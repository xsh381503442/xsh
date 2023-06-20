#include "time_progress.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "portmacro.h"
#include "portable.h"
#include "event_groups.h"
#include "semphr.h"


#include "task_display.h"
#include "gui_time.h"
#include "drv_ublox.h"
#include "lib_error.h"
#include "gui_sport.h"
#include "timer_app.h"
#include "task_sport.h"

#define TIME_PROGRESS_LOG_ENABLED 1

#if DEBUG_ENABLED == 1 && TIME_PROGRESS_LOG_ENABLED == 1
	#define TIME_PROGRESS_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define TIME_PROGRESS_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define TIME_PROGRESS_WRITESTRING(...)
	#define TIME_PROGRESS_PRINTF(...)		        
#endif


#define OSTIMER_WAIT_FOR_QUEUE           100
static TimerHandle_t m_timer; 
#define TIMER_INIT_INTERVAL_MS        1000
//#define UTC_GET_TIMEOUT               60
static uint32_t m_timeout = 0;
extern _gui_time_utc_get_param g_gui_time_utc_get_param;
extern am_hal_rtc_time_t RTC_time;
extern volatile ScreenState_t Save_Detail_Index;

static void callback(TimerHandle_t xTimer)
{
	switch(ScreenState)
	{	


        case DISPLAY_SCREEN_SPORT_SAVING:
            {
            DISPLAY_MSG  msg = {0,0};
	        m_timeout++;
            if (m_timeout < 6)
            {
                ScreenStateSave = ScreenState;
			    ScreenState = DISPLAY_SCREEN_SPORT_SAVING;
                msg.cmd = MSG_DISPLAY_SCREEN;
                msg.value = m_timeout;
			    xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
            }
            else 
            { 
              
                m_timeout = 0;
                ScreenStateSave = ScreenState;
                ScreenState = DISPLAY_SCREEN_SPORT_SAVED;
                msg.cmd = MSG_DISPLAY_SCREEN;
			    xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
            }

		  }
		break;
        case DISPLAY_SCREEN_SPORT_SAVED:
            {
            DISPLAY_MSG  msg = {0,0};
	        m_timeout++;
            if (m_timeout < 6)
            {
            }
            else 
            { 
                m_timeout = 0;
                timer_progress_stop();
                ScreenStateSave = ScreenState;
                ScreenState = Save_Detail_Index;
                msg.cmd = MSG_DISPLAY_SCREEN;
								xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
								//开启运动详情界面超时定时器
								timer_app_sport_record_detail_start();
							

            }

		}break;
				case DISPLAY_SCREEN_SWIMMING:
				{
					if (g_sport_status == SPORT_STATUS_READY)
					{
						DISPLAY_MSG  msg = {0,0};
						ScreenStateSave = ScreenState;
			    		ScreenState = DISPLAY_SCREEN_SWIMMING;
                		msg.cmd = MSG_DISPLAY_SCREEN;
                		msg.value = m_timeout;
			   			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
					}
				}
				break;
				case DISPLAY_SCREEN_INDOORRUN:
					{
						if (g_sport_status == SPORT_STATUS_READY)
						{
							DISPLAY_MSG  msg = {0,0};
							ScreenStateSave = ScreenState;
							ScreenState = DISPLAY_SCREEN_INDOORRUN;
							msg.cmd = MSG_DISPLAY_SCREEN;
							msg.value = m_timeout;
							xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
						}
					}
					break;


					
//Addin by Jason for UTC Time Setting Interface delay		V1.20					 
				case DISPLAY_SCREEN_UTC_GET:
						{
						if	( (m_timeout ==0) && (g_gui_time_utc_get_param.evt == GUI_TIME_UTC_GET_EVT_SUCCESS))
						{
								DISPLAY_MSG  msg = {0,0};
								ScreenStateSave = ScreenState;
								ScreenState = DISPLAY_SCREEN_HOME;
								msg.cmd = MSG_DISPLAY_SCREEN;
								msg.value = m_timeout;
								xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
					 }
						
				else if((m_timeout ==0) && (g_gui_time_utc_get_param.evt == GUI_TIME_UTC_GET_EVT_FAIL))
								{
								DISPLAY_MSG  msg = {0,0};
								ScreenStateSave = ScreenState;
								ScreenState = DISPLAY_SCREEN_TIME;
								msg.cmd = MSG_DISPLAY_SCREEN;
								msg.value = m_timeout;
								xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
								}
					}
				break;
					
				case DISPLAY_SCREEN_NOTIFY_ALARM:
						 timer_progress_stop();
				break;
						
//Jason's code end in here		<<<<<<< .mine//Addin by Jason for UTC Time Setting Interface delay		V1.20					 


				case DISPLAY_SCREEN_TRIATHLON_FINISH:
		{
				DISPLAY_MSG msg = {0};
				m_timeout++;
				if(m_timeout <= 6)
				{
					msg.cmd = MSG_DISPLAY_SCREEN;
					msg.value = m_timeout;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				}
				else
				{
					//关闭定时器
					m_timeout = 0;
					timer_progress_stop();
					
					//显示铁人三项详细数据
					ScreenState = DISPLAY_SCREEN_TRIATHLON_DETAIL;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
					//开启运动详情界面超时定时器
					timer_app_sport_record_detail_start();
				}
			}

			break;

		default:
			break;
	}
	
}



void timer_progress_init(void)
{
	
	m_timer = xTimerCreate("PROG",
												 ( TIMER_INIT_INTERVAL_MS / portTICK_PERIOD_MS ),
												 pdTRUE,
												 NULL,
												 callback);
	if (NULL == m_timer)
	{
		ERR_HANDLER(ERR_NO_MEM);
		TIME_PROGRESS_PRINTF("[TIME_PROGRESS]:xTimerCreate fail\n");
	}
}



void timer_progress_uninit(void)
{

	if (pdPASS != xTimerDelete(m_timer, OSTIMER_WAIT_FOR_QUEUE))
	{
		ERR_HANDLER(ERR_NO_MEM);
		TIME_PROGRESS_PRINTF("[TIME_PROGRESS]:xTimerDelete fail\n");
	}

}

void timer_progress_start(uint32_t timeout_msec)
{
	m_timeout = 0;
	if( xTimerChangePeriod( m_timer, timeout_msec / portTICK_PERIOD_MS, OSTIMER_WAIT_FOR_QUEUE ) == pdPASS )
	{
		TIME_PROGRESS_PRINTF("[TIME_PROGRESS]:xTimerChangePeriod success\n");
	}
	else
	{
		ERR_HANDLER(ERR_NO_MEM);
		TIME_PROGRESS_PRINTF("[TIME_PROGRESS]:xTimerChangePeriod fail\n");
	}		
}


void timer_progress_stop(void)
{
	if (pdPASS != xTimerStop(m_timer, OSTIMER_WAIT_FOR_QUEUE))
	{
		ERR_HANDLER(ERR_NO_MEM);
		TIME_PROGRESS_PRINTF("[TIME_PROGRESS]:xTimerStop fail\n");
	}
}





