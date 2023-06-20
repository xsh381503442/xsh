#include "task_config.h"
#include "task_step.h"

#include "bsp_timer.h"

#include "drv_lcd.h"
#include "drv_spiflash.h"
#include "drv_ms5837.h"
#include "drv_lis3mdl.h"
#include "drv_motor.h"

#include "task_hardware.h"
#include "task_display.h"
#include "task_key.h"
#include "task_uart.h"
#include "task_tool.h"
#include "task_timer.h"
#include "task_hrt.h"
#include "task_ble.h"

#include "time_notify.h"
#include "timer_app.h"

//#include "ndof/drv_ndof.h"
#include "Pedometer/Pedometer.h"
#include "Pedometer/PedoSupport.h"

#include "gui_tool.h"
#include "gui_tool_stopwatch.h"
#include "gui_tool_compass.h"
#include "gui_tool_gps.h"
#include "gui_tool_countdown.h"
#include "gui_heartrate.h"
#include "algo_magn_cal.h"

#include "gui_gps_test.h"




#if DEBUG_ENABLED == 1 && TASK_TOOL_LOG_ENABLED == 1
	#define TASK_TOOL_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define TASK_TOOL_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define TASK_TOOL_WRITESTRING(...)
	#define TASK_TOOL_PRINTF(...)		        
#endif

TaskHandle_t TaskToolHandle = NULL;

SemaphoreHandle_t Tool_Delete_Semaphore = NULL;
SemaphoreHandle_t Tool_Start_Semaphore = NULL;
extern uint8_t		g_CompassCaliStatus;

 float m_pres = 0;	//气压
 float m_alt = 0;		//高度
 float m_temp = 0;	//温度
static uint8_t	m_CompassCaliNum;	//指北针校准次数
static float m_Angle = 0;			//指北针角度

void CreateToolTask(void)
{
	//创建工具任务
   	if( TaskToolHandle == NULL )
	{
		xTaskCreate(TaskTools,"Task Tools",TaskTools_StackDepth,0,TaskTools_Priority,&TaskToolHandle );
    }
}

ScreenState_t tool_task_init(ScreenState_t ScreenSta)
{
	ScreenState_t temp = ScreenSta;
	
	//初始化相应数据
    switch(ScreenSta)
	{
		case DISPLAY_SCREEN_COMPASS:
			TASK_TOOL_PRINTF("gui_tool_compass_init\r\n");
			gui_tool_compass_init();
			break;
		default:
			break;
	}
	
	
	return temp;
}

void tool_task_start(ScreenState_t ScreenSta)
{
	
	//初始化相应数据
	
	TASK_TOOL_PRINTF("go to tool_task_init\r\n");
	tool_task_init(ScreenSta);

	if( TaskToolHandle != NULL )
	{
		xSemaphoreGive(	Tool_Start_Semaphore );
	}
}

void tool_task_close(void)
{
	
	if (TaskToolHandle !=NULL)
	{
		xSemaphoreGive(Tool_Delete_Semaphore);
	}
}

static ScreenState_t tool_drv_init(ScreenState_t ScreenSta)
{
	//初始相应外设
	switch(ScreenSta)
	{
		case DISPLAY_SCREEN_GPS:
		case DISPLAY_SCREEN_GPS_DETAIL:
		case DISPLAY_SCREEN_PROJECTION_ZONING:
			ScreenSta = DISPLAY_SCREEN_GPS;
			//创建GPS任务
			CreateGPSTask();
			break;
		case DISPLAY_SCREEN_COMPASS:
        case DISPLAY_SCREEN_COMPASS_CALIBRATION:
			ScreenSta = DISPLAY_SCREEN_COMPASS;
	        drv_lis3mdl_reset();
            am_util_delay_ms(30);
           // drv_lis3mdl_init();
			magn_dev_para_init();
			break;
        case DISPLAY_SCREEN_AMBIENT:
            ReadSeaLevelPressure(RTC_time.ui32Year,RTC_time.ui32Month,RTC_time.ui32DayOfMonth);
            break;
		default:
			ScreenSta = Get_Curr_Tool_Index();
			break;		
	}
	
	return ScreenSta;
}

float tool_compass_angle_get(void)
{
	//获取指北针角度
	return m_Angle;
}

void tool_ambient_value_get(float *pPres,float *pAlt,float *pTemp)
{
	//获取气压、高度、温度
	*pPres = m_pres;
	*pAlt = m_alt;
	*pTemp = m_temp;
}

void TaskTools( void* pvParameter)
{
	(void)pvParameter;
	DISPLAY_MSG  msg = {0};
	ScreenState_t ScreenSta = ScreenState;
	BaseType_t pdreturn = pdFALSE;
	uint32_t stopwatch_time;
	
	Tool_Start_Semaphore = xSemaphoreCreateCounting(10,0);
	Tool_Delete_Semaphore = xSemaphoreCreateCounting(10,0);
	while(1)
	{
		if( xSemaphoreTake(Tool_Start_Semaphore,portMAX_DELAY) == pdTRUE )
		{
			while( ( xSemaphoreTake(Tool_Delete_Semaphore,10/portTICK_PERIOD_MS) ) == pdTRUE )
			{
				xSemaphoreTake(Tool_Start_Semaphore,10/portTICK_PERIOD_MS);
			}
			if( ( ScreenState < DISPLAY_SCREEN_GPS ) || ( ScreenState > DISPLAY_SCREEN_FINDPHONE ) )
			{  //不处于六大监测主界面
				
				if( ( ScreenState < DISPLAY_SCREEN_COMPASS_CALIBRATION ) || ( ScreenState > DISPLAY_SCREEN_WEATHER_INFO ) )
				{  //不处于六大监测子界面
					xSemaphoreTake(Tool_Delete_Semaphore,10/portTICK_PERIOD_MS);
					pdreturn = pdTRUE;   //不等待信号量即退出
					
				TASK_TOOL_PRINTF("pdreturn = pdTRUE\r\n");
				}
				else
				{
					ScreenSta = tool_drv_init(ScreenState);
					pdreturn = pdFALSE;
					
				TASK_TOOL_PRINTF("tool_drv_init pdreturn = pdFALSE:%d\r\n",ScreenState);
				}
			}
			else
			{
				ScreenSta = tool_drv_init(ScreenState);       
				pdreturn = pdFALSE;
				
			TASK_TOOL_PRINTF("else tool_drv_init:%d\r\n",ScreenState);
			}
#if defined(DEBUG_FACTORY_HDR_AMBIENT_TEST)
			if(ScreenState == DISPLAY_SCREEN_FACTORY_HDR_AMBIENT_TEST)
			{
				ScreenSta = tool_drv_init(ScreenState);       
				pdreturn = pdFALSE;
			}
#endif			
			
			while( pdreturn == pdFALSE )
			{
				switch( ScreenSta )
				{
					case DISPLAY_SCREEN_FINDPHONE:
						
						pdreturn = xSemaphoreTake(Tool_Delete_Semaphore,1000/portTICK_PERIOD_MS);
						break;
					case DISPLAY_SCREEN_AMBIENT:
					case DISPLAY_SCREEN_AMBIENT_MENU:
					case DISPLAY_SCREEN_AMBIENT_PRESSURE:
					case DISPLAY_SCREEN_AMBIENT_ALTITUDE:
					case DISPLAY_SCREEN_AMBIENT_ALTITUDE_CALIBRATION:

						ScreenSta = DISPLAY_SCREEN_AMBIENT;
					
						//读取气压、高度、温度
						drv_ms5837_data_get(&m_pres,&m_alt,&m_temp);
					TASK_TOOL_PRINTF("drv_ms5837_data_get m_pres:%d,m_alt:%d,m_temp:%d\r\n",m_pres,m_alt,m_temp);
						if(ScreenState == DISPLAY_SCREEN_AMBIENT)
						{
							msg.cmd = MSG_UPDATE_AMBIENT_VALUE;
							xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
						}
						
						pdreturn = xSemaphoreTake(Tool_Delete_Semaphore,3000/portTICK_PERIOD_MS);
						break;
#if defined(DEBUG_FACTORY_HDR_AMBIENT_TEST)
					case DISPLAY_SCREEN_FACTORY_HDR_AMBIENT_TEST:
						//读取气压、高度、温度
						drv_ms5837_data_get(&m_pres,&m_alt,&m_temp);
						pdreturn = xSemaphoreTake(Tool_Delete_Semaphore,3000/portTICK_PERIOD_MS);
						break;
#endif
					case DISPLAY_SCREEN_STOPWATCH:
						if(ScreenState == DISPLAY_SCREEN_STOPWATCH)
						{
							if(tool_stopwatch_status_get() == TOOL_STOPWATCH_STATUS_START)
							{
								stopwatch_time = tool_stopwatch_time_get();
								if(stopwatch_time >= (TOOL_STOPWATCH_TIME_MAX - 1))
								{
									//秒表计时超时
									tool_stopwatch_overtime();
									
									msg.cmd = MSG_DISPLAY_SCREEN;
								}
								else
								{
									//更新秒表计时时间
									msg.cmd = MSG_UPDATE_STOPWATCH_TIME;
								}
								msg.value = stopwatch_time;
								xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
								
								pdreturn = xSemaphoreTake(Tool_Delete_Semaphore,100/portTICK_PERIOD_MS);
							}
							else
							{
								pdreturn = xSemaphoreTake(Tool_Delete_Semaphore,500/portTICK_PERIOD_MS);
							}
						}
						else
						{
							pdreturn = xSemaphoreTake(Tool_Delete_Semaphore,1000/portTICK_PERIOD_MS);
						}
						break;
					case DISPLAY_SCREEN_GPS:
					case DISPLAY_SCREEN_GPS_DETAIL:
					case DISPLAY_SCREEN_PROJECTION_ZONING:
						ScreenSta = DISPLAY_SCREEN_GPS;
						if(ScreenState == DISPLAY_SCREEN_GPS)
						{
							msg.cmd = MSG_UPDATE_GPS_VALUE;
							xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
						}
						
						pdreturn = xSemaphoreTake(Tool_Delete_Semaphore,1000/portTICK_PERIOD_MS);
						break;
					case DISPLAY_SCREEN_COMPASS:
					case DISPLAY_SCREEN_COMPASS_CALIBRATION:
						ScreenSta = DISPLAY_SCREEN_COMPASS;
						if(ScreenState == DISPLAY_SCREEN_COMPASS)
						{
							m_CompassCaliNum = 0;
							m_Angle = magn_angle_and_cal(0);

							msg.cmd = MSG_UPDATE_COMPASS_VALUE;
							msg.value = m_Angle;
							xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
 
							pdreturn = xSemaphoreTake(Tool_Delete_Semaphore,350/portTICK_PERIOD_MS);
						}
						else if(ScreenState == DISPLAY_SCREEN_COMPASS_CALIBRATION)
						{
							m_Angle = magn_angle_and_cal(1);
					    
							 if(magn_cal_flag_get() == 3)
							{
								//校准完成,马达振动
								timer_notify_motor_start(300,300,1,false,NOTIFY_MODE_TOOLS);
								timer_notify_buzzer_start(300,300,1,false,NOTIFY_MODE_TOOLS);
								magn_para_reset();
								//返回指北针界面
								ScreenState = DISPLAY_SCREEN_COMPASS;
								msg.cmd = MSG_DISPLAY_SCREEN;
								xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
							}
							else
							{
								m_CompassCaliNum++;
								if(m_CompassCaliNum % 20 == 0)
								{
									if(m_CompassCaliNum == 160)
									{
										m_CompassCaliNum = 0; 
									}
									msg.value = m_CompassCaliNum / 20;
									msg.cmd = MSG_UPDATE_COMPASS_CALIBRATION;
					   
									xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
								   
								}
								
								pdreturn = xSemaphoreTake(Tool_Delete_Semaphore,30/portTICK_PERIOD_MS);
							}
						}
						else
						{
							pdreturn = xSemaphoreTake(Tool_Delete_Semaphore,1000/portTICK_PERIOD_MS);
						}	
						break;
#if  defined (WATCH_TOOL_COUNTDOWN)
					case DISPLAY_SCREEN_COUNTDOWN:
						ScreenSta = DISPLAY_SCREEN_COUNTDOWN;
						
						//获取时间
						stopwatch_time = tool_countdown_real_time_get();
						if(stopwatch_time > tool_countdown_target_time_get())
						{
							//倒计时结束
							tool_countdown_finish();
						}
						
						if((ScreenState == DISPLAY_SCREEN_COUNTDOWN_TIME)
							|| (ScreenState == DISPLAY_SCREEN_COUNTDOWN_FINISH))
						{
							msg.cmd = MSG_DISPLAY_SCREEN;
							xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
						}
						
						pdreturn = xSemaphoreTake(Tool_Delete_Semaphore,1000/portTICK_PERIOD_MS);
						break;
#endif
					default:
						pdreturn = xSemaphoreTake(Tool_Delete_Semaphore,portMAX_DELAY);
						break;
				}
			}
		
			if(pdreturn == pdTRUE)
			{               
				//退出相应模式
				switch(ScreenSta)
				{
					case DISPLAY_SCREEN_GPS:
						if((0 == ENTER_NAVIGATION) && (0 == ENTER_MOTION_TRAIL))
						{
							//在运动导航和运动轨迹中时，不关闭GPS任务
							CloseGPSTask();
						}
						break;
					case DISPLAY_SCREEN_COMPASS:
						drv_lis3mdl_standby();
						break;
					case DISPLAY_SCREEN_STOPWATCH:
						gui_tool_stopwatch_init();
						break;
					default:
						break;
				}
			}
		}
	}
}




