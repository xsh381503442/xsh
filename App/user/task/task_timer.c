#include "task_timer.h"
#include "task_config.h"
#include "task_step.h"
#include "drv_heartrate.h"
#include "drv_lsm6dsl.h"
#include "drv_ms5837.h"
#include "drv_alarm.h"

#include "bsp_timer.h"
#include "isr_gpio.h"
#include "drv_light.h"
#include "algo_hdr.h"
#include "com_data.h"
#include "com_sport.h"
#include "time_notify.h"

#include "gui_tool_gps.h"
#include "gui_sports_record.h"

#if DEBUG_ENABLED == 1 && TASK_TIMER_LOG_ENABLED == 1
	#define TASK_TIMER_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define TASK_TIMER_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define TASK_TIMER_WRITESTRING(...)
	#define TASK_TIMER_PRINTF(...)		        
#endif

TaskHandle_t	TaskTimerHandle = NULL;
QueueHandle_t 	TimerQueue;
extern uint8_t LOADINGFLAG;
extern uint32_t sport_data_addr[2];
extern _gui_sports_record g_gui_sports_record[2];
void CTimer1TickHandler(void )
{
	//用于检测和运动模式开启的2秒计时
	#if 0
	DISPLAY_MSG  msg = {0};
	BaseType_t xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken = pdFALSE;

	if( ScreenState == DISPLAY_SCREEN_MEASURING )
	{
		ScreenState = Get_Curr_Measuring_Index();
		msg.cmd = MSG_START_MEASURING;
		msg.value = Get_Curr_Measuring_Index();
		xQueueSendFromISR(DisplayQueue, &msg, &xHigherPriorityTaskWoken);
	}
	#endif
	
}

void CTimerLightHandler( void )
{
    drv_light_disable();//关闭背光
}

/*1秒定时器任务处理函数*/
void CTimerB0TickHandler(void)
{

}
void CTimerB2TickHandler(void )
{
	DISPLAY_MSG  msg = {0};
	BaseType_t xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken = pdFALSE;
	if(ScreenState == DISPLAY_SCREEN_TIME_CALIBRATION_REALTIME)
	{
		TASK_TIMER_PRINTF("[task_timer]:CTimerB2TickHandler --DISPLAY_SCREEN_TIME_CALIBRATION\n");
		msg.cmd = MSG_UPDATE_TIME_CALIBRATION;
		xQueueSendFromISR(DisplayQueue, &msg, &xHigherPriorityTaskWoken);
		ResetTimerB2(1);
	}
}
void CreatTimerTask(void)
{
	if( TaskTimerHandle == NULL )
	{
	    xTaskCreate(TaskTimer, "Task Timer",TaskTimer_StackDepth, 0, TaskTimer_Priority, &TaskTimerHandle); 
		TimerQueue = xQueueCreate( 5,sizeof( TIMER_MSG ));
	}
}

void Send_Timer_Cmd(uint32_t cmd)
{

	TIMER_MSG  msg = {0};

	msg.cmd = cmd;
	xQueueSend(TimerQueue, &msg, portMAX_DELAY );
}

//定时保存数据及其他杂项
void TaskTimer(void* pvParameter)
{
	static TIMER_MSG msg = {0,0};
	
	//开机时获取步数
	//DayMainData.CommonSteps = drv_lsm6dsl_get_stepcount() - DayMainData.SportSteps;

	while(1)
	{
		if (xQueueReceive(TimerQueue, &msg, portMAX_DELAY) == pdTRUE)
		{
			switch( msg.cmd ) 
			{ 
				case CMD_HOME_STEP:
					DayMainData.CommonSteps = get_filt_stepcount();
					Remind_DailyKcal(Get_TotalEnergy()/1000,SetValue.AppSet.Goal_Energy);
					Remind_DailyStep(Get_TotalStep(),SetValue.AppSet.Goal_Steps);
					break;
				case CMD_AMBIENT_DATA:
				{
					float pres,alt,temp;
					
					drv_ms5837_data_get(&pres,&alt,&temp);

					if((RTC_time.ui32Minute % 3) == 0)
					{
						//每3分钟存一次高度数据
						if(AmbientData.AltitudeNum >= AMBIENT_DATA_MAX_NUM)
						{
							AmbientData.AltitudeNum = 0;
						}
						AmbientData.AltitudeData[AmbientData.AltitudeNum++] = (int16_t)(alt + SetValue.Altitude_Offset);
						if(AmbientData.AltitudeSum < AMBIENT_DATA_MAX_NUM)
						{
							AmbientData.AltitudeSum++;
						}
						if(AmbientData.AltitudeNum >= AMBIENT_DATA_MAX_NUM)
						{
							AmbientData.AltitudeNum = 0;
						}
					}
					
					//每6分钟存一次气压数据
				if((RTC_time.ui32Minute % 2) == 0)
					{
						if ((pres > 300)&&(pres < 1300))
						{
							if(AmbientData.PressureNum >= AMBIENT_DATA_MAX_NUM)
							{
								AmbientData.PressureNum = 0;
							}
							AmbientData.PressureData[AmbientData.PressureNum++] = (int16_t)(pres*10);
							if(AmbientData.PressureSum < AMBIENT_DATA_MAX_NUM)
							{
								AmbientData.PressureSum++;
							}
							if(AmbientData.PressureNum >= AMBIENT_DATA_MAX_NUM)
							{
							AmbientData.PressureNum = 0;
							}
					   }
					}
						//每6分钟存一次气压数据
				/*	for(i= 0;i<90;i++)
					{
					     pres =1000 + rand()%21;
						
						if(AmbientData.PressureNum >= AMBIENT_DATA_MAX_NUM)
						{
							AmbientData.PressureNum = 0;
						}
						AmbientData.PressureData[AmbientData.PressureNum++] = (int16_t)(pres*10);
						if(AmbientData.PressureSum < AMBIENT_DATA_MAX_NUM)
						{
							AmbientData.PressureSum++;
						}
						if(AmbientData.PressureNum >= AMBIENT_DATA_MAX_NUM)
						{
							AmbientData.PressureNum = 0;
						}
						if(i==89)
							{
							i=0;
							pres -= 180;
							 }
					
					}*/
					
					//保存数据
//					Store_AmbientData();
					
					if((ScreenState == DISPLAY_SCREEN_AMBIENT_PRESSURE) 
						|| (ScreenState == DISPLAY_SCREEN_AMBIENT_ALTITUDE))
					{
						//如果在气压和高度界面，需更新界面
						msg.cmd = MSG_DISPLAY_SCREEN;
						xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
					}
					break;
				}
				case CMD_MOTION_TRAIL_LOAD:
                    vTaskDelay(300/portTICK_PERIOD_MS);
					InitLoadTrack();
					LoadMotiontTrail(get_motion_trail_startaddress(),get_motion_trail_endaddress());
					if (LOADINGFLAG ==1)
					{
					gui_motion_trail_btn_detail();
					}
				break;
				case CMD_NAVIGATION_LOAD:
				{
                    vTaskDelay(300/portTICK_PERIOD_MS);
					InitLoadTrack();
					LoadNavigation(get_navigation_gps_startaddress(),get_navigation_gps_endaddress());
					load_track_mark(get_track_point_startaddress(),get_track_point_endaddress());
					if (LOADINGFLAG ==1)
					{
					gui_cloud_navigation_btn_item();
					}
				}
				break;
				case CMD_MINUTE_DATA:
					//闹钟检查
					drv_alarm_handler();
#if defined WATCH_HAS_NO_BLE 
				
#else
					//训练计划提醒
					Remind_TodayTrainPlan();
#endif
				break;
				case CMD_SPORTS_TRAIL_LOAD:
					//运动记录中运动轨迹加载
				{
					uint32_t m_stop_gps_addr = 0;
          vTaskDelay(300/portTICK_PERIOD_MS);
					InitLoadTrack();
#if defined(WATCH_SPORT_RECORD_TEST)
					Get_SportTrack_GpsAddress(s_sport_record_select_str[activity_data_index].address);
#else
					Get_SportTrack_GpsAddress(sport_data_addr[0]);
#endif
					if(g_gui_sports_record[0].type == ACT_TRIATHLON_SWIM
					|| g_gui_sports_record[0].type == ACT_TRIATHLON_CYCLING
					|| g_gui_sports_record[0].type == ACT_TRIATHLON_RUN)
					{//铁人三项GPS获取
#if defined(WATCH_SPORT_RECORD_TEST)
						m_stop_gps_addr = GetTriathionGPSEndAddress(s_sport_record_select_str[activity_data_index].address);
#else
						m_stop_gps_addr = GetTriathionGPSEndAddress(sport_data_addr[0]);
#endif
						//if(m_stop_gps_addr <= g_gpstrack_data.GPS_Start_Address)
						//	m_stop_gps_addr = g_gpstrack_data.GPS_Start_Address;
						if(m_stop_gps_addr != 0)
						{
							LoadMotiontTrail(g_gpstrack_data.GPS_Start_Address,m_stop_gps_addr);
						}
					}
					else
					{
						LoadMotiontTrail(g_gpstrack_data.GPS_Start_Address,g_gpstrack_data.GPS_Stop_Address);
					}
					
					if (LOADINGFLAG ==1)
					{
						ScreenState = DISPLAY_SCREEN_SPORTS_RECORD_TRACK_DETAIL;
						msg.cmd = MSG_DISPLAY_SCREEN;
						xQueueSend(DisplayQueue, &msg, portMAX_DELAY );
					}
				}
					break;
				default:
					break;
			}
		}
	}
}


