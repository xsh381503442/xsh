#include "drv_alarm.h"
#include "com_data.h"
#include "SEGGER_RTT.h"
#include <string.h>
#include "time_notify.h"
#include "am_mcu_apollo.h"
#include "am_util.h"
#include "time_progress.h"

#define DRV_ALARM_LOG_ENABLED 1

#if DEBUG_ENABLED == 1 && DRV_ALARM_LOG_ENABLED == 1
	#define DRV_ALARM_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define DRV_ALARM_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define DRV_ALARM_WRITESTRING(...)
	#define DRV_ALARM_PRINTF(...)		        
#endif

uint16_t repetition;

bool motor_enable_temp;
bool buzzer_enable_temp;

uint16_t motor_counter=0;
uint16_t buzzer_counter=0;

extern SetValueStr SetValue;
extern am_hal_rtc_time_t RTC_time;
ScreenState_t ScreenStateAlarm;			//用于保存闹钟上个界面
uint8_t g_drv_alarm_num = 0;			//响铃的闹钟编号

void drv_alarm_handler(void)
{
	bool motor_enable = false, buzzer_enable = false;
	
	am_hal_rtc_time_get(&RTC_time);

	uint32_t weekday = am_util_time_computeDayofWeek(RTC_time.ui32Year+2000, RTC_time.ui32Month, RTC_time.ui32DayOfMonth);
	uint32_t hour = RTC_time.ui32Hour;
	uint32_t minute = RTC_time.ui32Minute;


	for(uint8_t i=0;i<(sizeof(SetValue.alarm_instance)/sizeof(SetValue.alarm_instance[0]));i++)
	{
		repetition=10;
		motor_enable_temp=false;
		//第一层判断：手表时间是否与闹钟时间相同，闹钟是否打开
		if(SetValue.alarm_instance[i].enable && 
			 SetValue.alarm_instance[i].time.hour == hour && 
			 SetValue.alarm_instance[i].time.minute == minute
		  )
				{
				//第二层判断：闹钟重复的三种方式（一次，每天，工作日）
					if(SetValue.alarm_instance[i].repetition_type == ALARM_REPITION_TYPE_SINGLE)
						{
									//第三层判断：闹钟的震动与声音是否打开
									if(SetValue.alarm_instance[i].motor_enable)
								{
										motor_enable = true;
								}	
									if(SetValue.alarm_instance[i].buzzer_enable)
								{
										buzzer_enable = true;
								}		
							SetValue.alarm_instance[i].enable = false;
							g_drv_alarm_num = i;
							//将其他相同设置的闹钟都关闭
							for(;i<(sizeof(SetValue.alarm_instance)/sizeof(SetValue.alarm_instance[0]));i++)
							{
								if((SetValue.alarm_instance[i].enable) && 
									 (SetValue.alarm_instance[i].time.hour == hour) && 
									 (SetValue.alarm_instance[i].time.minute == minute) &&
									 (SetValue.alarm_instance[i].repetition_type == ALARM_REPITION_TYPE_SINGLE))
								{
									SetValue.alarm_instance[i].enable = false;
								}
							}
							break;			
						}
					else if(SetValue.alarm_instance[i].repetition_type == ALARM_REPITION_TYPE_DAILY)
						{
									if(SetValue.alarm_instance[i].motor_enable)
								{
										motor_enable = true;
								}	
									if(SetValue.alarm_instance[i].buzzer_enable)
								{
										buzzer_enable = true;
								}
							g_drv_alarm_num = i;
							break;
						}
					else if(SetValue.alarm_instance[i].repetition_type == ALARM_REPITION_TYPE_WEEK_DAY)
			{
				if((weekday > 0) && (weekday < 6))
				{
					//工作日
					if(SetValue.alarm_instance[i].motor_enable)
					{
						motor_enable = true;
					}	
					if(SetValue.alarm_instance[i].buzzer_enable)
					{
						buzzer_enable = true;
					}
					g_drv_alarm_num = i;
					break;
				}
			}
			else if(SetValue.alarm_instance[i].repetition_type == ALARM_REPITION_TYPE_WEEKEND)
			{
				if((weekday == 6) || (weekday == 0))
				{
					//周六、周日
					if(SetValue.alarm_instance[i].motor_enable)
					{
						motor_enable = true;
					}	
					if(SetValue.alarm_instance[i].buzzer_enable)
					{
						buzzer_enable = true;
					}
					g_drv_alarm_num = i;
					break;
				}			
			}			
		}
	}
	if(motor_enable || buzzer_enable)			//只有开启了马达或声音提醒，闹钟才会弹出提醒界面。（!=0,0）
	{
		if((ScreenState != DISPLAY_SCREEN_LOGO)
			&& (ScreenState != DISPLAY_SCREEN_NOTIFY_BAT_CHG)
			&& time_notify_is_ok())
		{
			if(ScreenState < DISPLAY_SCREEN_NOTIFY)
			{
				if(ScreenState != DISPLAY_SCREEN_NOTIFY_ALARM)
				{
					ScreenStateAlarm = ScreenState;
				}
			}
			else
			{
				if(ScreenStateSave != DISPLAY_SCREEN_NOTIFY_ALARM)
				{
					ScreenStateAlarm = ScreenStateSave;
				}
			}
//			timer_notify_display_start(335000,1,false);//多出5秒让分钟多走一//在此界面中一共停留5分30秒，以配合震铃 V1.0.2B103	
			timer_notify_display_stop(false);
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_NOTIFY_ALARM;
			msg.cmd = MSG_DISPLAY_SCREEN;
			//BaseType_t xHigherPrioritTaskWoken = pdFALSE;
			if( xQueueSendToFront( DisplayQueue, &msg, ( TickType_t ) 0) != pdPASS )
			{
				DRV_ALARM_PRINTF("[TIME_NOTIFY]:xQueueSendToFrontFromISR fail\n");
			}
		//	if( xHigherPrioritTaskWoken != pdFALSE )
		//	{
		//		portYIELD_FROM_ISR(xHigherPrioritTaskWoken);
		//	}							
		
			if	((motor_enable==true) && (buzzer_enable==true))				//1,1
			{
				timer_notify_remind_start(150000,30000,1000,500,1000,500,3,false);//一个周期150秒，一次提醒30秒种，一共提醒3次
//				timer_notify_remind_start(20000,10000,1000,500,1000,500,3,true);	//一个周期20秒，一次提醒10秒钟，一共提醒3次
			}	
			else if ((motor_enable==true) && (buzzer_enable==false))	//1,0
			{
				timer_notify_remind_start(150000,30000,1000,500,0,0,3,false);	//一个周期150秒，一次提醒30秒种，一共提醒3次
//				timer_notify_remind_start(20000,10000,1000,500,0,0,3,true);			//一个周期20秒，一次提醒10秒钟，一共提醒3次
			}
			else if ((motor_enable==false) && (buzzer_enable==true))	//0,1
			{
				timer_notify_remind_start(150000,30000,0,0,1000,500,3,false);			//一个周期150秒，一次提醒30秒种，一共提醒3次
//				timer_notify_remind_start(20000,10000,0,0,1000,500,3,true);	//一个周期20秒，一次提醒10秒钟，一共提醒3次

			}
		}
	}	
	
//if(motor_enable) 
//{
//	timer_notify_motor_start(1000,500,10,true,1);
//}
//	if(motor_enable)
//	{
//		
//		motor_enable_temp = true;
//		timer_notify_motor_start(1000,500,repetition,true,1);
//		motor_counter++;
//		DRV_ALARM_PRINTF( "motor viberate time 1st %d \n",motor_counter);
//			if ((repetition==0) && (motor_enable_temp == true))
//		//((motor_enable != motor_enable_temp) && (motor_enable==false))	
//		{
//		
////		motor_enable_temp = false;
//		motor_enable = true;
//		timer_notify_motor_start(2000,500,10,true,1);
//		motor_counter++;
//		DRV_ALARM_PRINTF( "motor viberate time 2nd %d \n",motor_counter);
//		}
//	}
//	if(buzzer_enable)
//	{
//		buzzer_enable_temp = true;
//		timer_notify_buzzer_start(1000,500,repetition,true,1);
//		buzzer_counter++;
//	}	
//	timer_progress_start(5000);
	

//		if ((buzzer_counter==10)&& (buzzer_enable==1))
		//((buzzer_enable != buzzer_enable_temp) && (buzzer_enable==false))	
//	{

//		buzzer_enable_temp = false;
//		buzzer_enable = true;
//		timer_notify_buzzer_start(2000,500,10,true,1);
//		buzzer_counter++;
//	}

}
















