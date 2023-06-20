#include "task_battery.h"
#include <string.h>

#include "task_config.h"
#include "task_pc.h"

#include "am_mcu_apollo.h"


#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "portmacro.h"
#include "portable.h"
#include "event_groups.h"
#include "semphr.h"

#include "time_notify.h"
#include "mode_power_off.h"
#include "mode_power_on.h"
#include "lib_error.h"

#include "com_sport.h"
#include "com_data.h"
#include "drv_ms5837.h"
#include "lib_app_data.h"
#include "drv_battery.h"


#if DEBUG_ENABLED == 1 && TASK_BATTERY_LOG_ENABLED == 1
	#define TASK_BATTERY_LOG_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define TASK_BATTERY_LOG_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else       
	#define TASK_BATTERY_LOG_WRITESTRING(...)
	#define TASK_BATTERY_LOG_PRINTF(...)		        
#endif


static TaskHandle_t task_handle;
static EventGroupHandle_t evt_handle;


extern ScreenState_t   ScreenState;
extern _drv_bat_event   g_bat_evt;
extern SetValueStr SetValue;
extern uint8_t m_bat_init_flag;

static bool start_charge = false;
//BaseType_t xHigherPriorityTaskWoken = false;	


#define OSTIMER_WAIT_FOR_QUEUE           100
static TimerHandle_t m_timer; 
#define TIMER_INIT_INTERVAL_MS        1000
uint32_t shutdown_val = SHUTDOWN_VAL + 1;

static void callback(TimerHandle_t xTimer)
{
	if(ScreenState == DISPLAY_SCREEN_NOTIFY_BAT_CHG)
	{
		if(CHARGE_PWON)
		{	
			CHARGE_PWON = 0;
			am_hal_rtc_time_get(&RTC_time);
			m_charging_sec = RTC_time.ui32Hour*3600+RTC_time.ui32Minute*60+RTC_time.ui32Second;
		}
	
		timer_battery_start(1000);
		
		drv_bat_adc_enable();
/*		DISPLAY_MSG  msg = {0,0};	
		msg.cmd = MSG_DISPLAY_SCREEN;
		xQueueSendFromISR(DisplayQueue, &msg, &xHigherPriorityTaskWoken);		*/	
	}

}


void timer_battery_init(void)
{
	
	m_timer = xTimerCreate("BAT",
												 ( TIMER_INIT_INTERVAL_MS / portTICK_PERIOD_MS ),
												 pdFALSE,
												 NULL,
												 callback);
	if (NULL == m_timer)
	{
		ERR_HANDLER(ERR_NO_MEM);
		TASK_BATTERY_LOG_PRINTF("[TASK_BATTERY]:xTimerCreate fail\n");
	}
}




void timer_battery_uninit(void)
{

	if (pdPASS != xTimerDelete(m_timer, OSTIMER_WAIT_FOR_QUEUE))
	{
		ERR_HANDLER(ERR_NO_MEM);
		TASK_BATTERY_LOG_PRINTF("[TASK_BATTERY]:xTimerDelete fail\n");
	}

}




void timer_battery_start(uint32_t timeout_msec)
{

	if( xTimerChangePeriod( m_timer, timeout_msec / portTICK_PERIOD_MS, OSTIMER_WAIT_FOR_QUEUE ) == pdPASS )
	{
		TASK_BATTERY_LOG_PRINTF("[TASK_BATTERY]:xTimerChangePeriod success\n");
	}
	else
	{
		
		TASK_BATTERY_LOG_PRINTF("[TASK_BATTERY]:xTimerChangePeriod fail\n");
	}		
}

void timer_battery_stop(void)
{
	if (pdPASS != xTimerStop(m_timer, OSTIMER_WAIT_FOR_QUEUE))
	{
		ERR_HANDLER(ERR_NO_MEM);
		TASK_BATTERY_LOG_PRINTF("[TASK_BATTERY]:xTimerStop fail\n");
	}
}






static void task_cb(_drv_bat_event * p_event)
{
	BaseType_t xHigherPriorityTaskWoken, xResult;

	xHigherPriorityTaskWoken = pdFALSE;
	
	xResult = xEventGroupSetBitsFromISR(evt_handle, (p_event->type),
																			&xHigherPriorityTaskWoken);
	if (xResult == pdPASS)
	{
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
//	else
//	{
//		ERR_HANDLER(ERR_FORBIDDEN);
//	}
}


static void task(void *pvParameters)
{
	uint32_t bitSet;
	static uint8_t last_percent = 0;
	
	while (1)
	{		
		bitSet = xEventGroupWaitBits(evt_handle, 0xFF, pdTRUE,
												pdFALSE, portMAX_DELAY);
		switch(bitSet)
		{	
/***********************************************	
			case DRV_BAT_EVENT_DATA:{
				TASK_BATTERY_LOG_PRINTF("[TASK_BATTERY]:DRV_BAT_EVENT_DATA\n");
				
				if(ScreenState == DISPLAY_SCREEN_HOME)
				{
					DISPLAY_MSG  msg = {0,0};

					ScreenState = DISPLAY_SCREEN_HOME;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				}		
			}break;
			case DRV_BAT_EVENT_5_PERCENT:{
				TASK_BATTERY_LOG_PRINTF("[TASK_BATTERY]:DRV_BAT_EVENT_5_PERCENT\n");
				
				if(ScreenState != DISPLAY_SCREEN_LOGO)
				{
					if(ScreenState < DISPLAY_SCREEN_NOTIFY)
					{
						ScreenStateSave = ScreenState;
					}
					timer_notify_display_start(10000,1,false);
					timer_notify_motor_start(1000,500,3,false,NOTIFY_MODE_NORMAL);
					timer_notify_buzzer_start(1000,500,3,false,NOTIFY_MODE_NORMAL);
					DISPLAY_MSG  msg = {0,0};
					ScreenState = DISPLAY_SCREEN_NOTIFY_LOW_BAT;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);								
				}					
				
			}break;
			case DRV_BAT_EVENT_10_PERCENT:{
				TASK_BATTERY_LOG_PRINTF("[TASK_BATTERY]:DRV_BAT_EVENT_10_PERCENT\n");
				//改变电池图标颜色
				if(ScreenState == DISPLAY_SCREEN_HOME)
				{
					DISPLAY_MSG  msg = {0,0};

					ScreenState = DISPLAY_SCREEN_HOME;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				}					
			}break;		************************************************/		
			case DRV_BAT_EVENT_FULL:{
				TASK_BATTERY_LOG_PRINTF("[TASK_BATTERY]:DRV_BAT_EVENT_FULL\n");	
				if(ScreenState == DISPLAY_SCREEN_HOME)
				{
					DISPLAY_MSG  msg = {0,0};

					ScreenState = DISPLAY_SCREEN_HOME;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				}				
			}break;
			case DRV_BAT_EVENT_USB_CONN_CHARGING:
			case DRV_BAT_EVENT_USB_CONN_CHARGING_FINISHED:{
				TASK_BATTERY_LOG_PRINTF("[TASK_BATTERY]:DRV_BAT_EVENT_USB_CONN_CHARGING\n");	
				TASK_BATTERY_LOG_PRINTF("[TASK_BATTERY]:ScreenStateSave --> %d\n",ScreenStateSave);	
				TASK_BATTERY_LOG_PRINTF("[TASK_BATTERY]:ScreenState --> %d\n",ScreenState);	
				TASK_BATTERY_LOG_PRINTF("[TASK_BATTERY]:DISPLAY_SCREEN_NOTIFY_BAT_CHG --> %d\n",DISPLAY_SCREEN_NOTIFY_BAT_CHG);	
					
				if(ScreenState == DISPLAY_SCREEN_LOGO)
				{
					mode_button_pon();
				}
				else
				{
					if(ScreenState != DISPLAY_SCREEN_BLE_DFU)
					{
						mode_close();		//充电时关闭正在进行的运动
						InitLoadTrack(); 	//初始化加载轨迹参数，防止轨迹状态异常
						task_pc_init();
						timer_battery_start(1000);
						g_bat_evt.cable_status = DRV_BAT_EVENT_USB_CONN_CHARGING;		
						DISPLAY_MSG  msg = {0,0};
						if(ScreenState < DISPLAY_SCREEN_NOTIFY)
						{
							ScreenStateSave = ScreenState;
						}							
						ScreenState = DISPLAY_SCREEN_NOTIFY_BAT_CHG;
						msg.cmd = MSG_DISPLAY_SCREEN;
						xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
					}
				}
				
			}break;
			case DRV_BAT_EVENT_USB_DISCONN:{
				TASK_BATTERY_LOG_PRINTF("[TASK_BATTERY]:DRV_BAT_EVENT_USB_DISCONN\n");	
				TASK_BATTERY_LOG_PRINTF("[TASK_BATTERY]:ScreenStateSave --> %d\n",ScreenStateSave);	
				TASK_BATTERY_LOG_PRINTF("[TASK_BATTERY]:ScreenState --> %d\n",ScreenState);	
				TASK_BATTERY_LOG_PRINTF("[TASK_BATTERY]:DISPLAY_SCREEN_NOTIFY_BAT_CHG --> %d\n",DISPLAY_SCREEN_NOTIFY_BAT_CHG);	
				
				SetValue.PowerPercentRecord = g_bat_evt.level_percent;		//切换充电状态时保存一次电量百分比
				//保存数据
				Store_SetData();
				
				if(charging_timer_get() != NULL)
					charging_timer_delete();
				
				task_pc_uninit();
				timer_battery_stop();		
				g_bat_evt.cable_status = DRV_BAT_EVENT_USB_DISCONN;	
				
				if(ScreenState == DISPLAY_SCREEN_NOTIFY_BAT_CHG)
				{
					DISPLAY_MSG  msg = {0,0};	
					ScreenState = DISPLAY_SCREEN_HOME;            //充电结束时，返回主界面
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);					
				}
						
			}break;			
			case DRV_BAT_EVENT_CHARGING:											//充电状态
			{	
				DISPLAY_MSG  msg = {0,0};	
				
				drv_bat_management(&TMP_ADC);
				
				if(1 == system_voltage_flag_get())						//第一次从flash获取电量记录时
				{
					system_voltage_flag_clear();
				}
				
				if(ScreenState == DISPLAY_SCREEN_PC_HWT)
				{
					//硬件测试不显示充电界面
					break;
				}
				
  				else if (ScreenState == DISPLAY_SCREEN_NOTIFY_FIND_WATCH) 
				{
					//寻找腕表界面时不显示充电界面
					break;
				}
                #if DEBUG_ENABLED == 0 //调试时不需要显示充电界面
                    ScreenState = DISPLAY_SCREEN_NOTIFY_BAT_CHG;
                    msg.cmd = MSG_DISPLAY_SCREEN;
                    xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
                #endif                

			}
			break;
			case DRV_BAT_EVENT_DISCHARGING:										//放电状态
			{				
				drv_bat_management(&TMP_ADC);
			

				if(start_charge == false)
				{	
				     
					//if(TMP_ADC <= 11360)       //低电量自动关机11694

					if(shutdown_val<SHUTDOWN_VAL)
					{
						if(ScreenState != DISPLAY_SCREEN_LOGO && ScreenState != DISPLAY_SCREEN_NOTIFY_BAT_CHG)
						{
							mode_button_pof();
						}
					}

					if (1 == system_voltage_flag_get())
					{
						system_voltage_flag_clear();
						DISPLAY_MSG  msg = {0,0};
						msg.cmd = MSG_DISPLAY_SCREEN;
						xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
					}
				
				}

				if (start_charge)
				{
					start_charge = false;
				}
				
				if(1 == system_voltage_flag_get())	  //第一次从flash获取电量记录时
				{
					system_voltage_flag_clear();
				}

			
				
//				else if(g_bat_evt.level_percent <= 5)
//				{
//					if(ScreenState != DISPLAY_SCREEN_LOGO)
//					{
//						if(ScreenState < DISPLAY_SCREEN_NOTIFY)
//						{
//							ScreenStateSave = ScreenState;
//						}
//						timer_notify_display_start(3000,1,false);
//						timer_notify_motor_start(1000,500,1,false,NOTIFY_MODE_NORMAL);
//						timer_notify_buzzer_start(1000,500,1,false,NOTIFY_MODE_NORMAL);
//						DISPLAY_MSG  msg = {0,0};
//						ScreenState = DISPLAY_SCREEN_NOTIFY_LOW_BAT;
//						msg.cmd = MSG_DISPLAY_SCREEN;
//						xQueueSend(DisplayQueue, &msg, portMAX_DELAY);								
//					}					
//				}
//				else if(g_bat_evt.level_percent <= 10)
//				{
//									//改变电池图标颜色
//					if(ScreenState == DISPLAY_SCREEN_HOME)
//					{
//						DISPLAY_MSG  msg = {0,0};
//	
//						ScreenState = DISPLAY_SCREEN_HOME;
//						msg.cmd = MSG_DISPLAY_SCREEN;
//						xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
//					}					
//				}
//				else
//				{
//					if(ScreenState == DISPLAY_SCREEN_HOME)
//					{
//						DISPLAY_MSG  msg = {0,0};
//	
//						ScreenState = DISPLAY_SCREEN_HOME;
//						msg.cmd = MSG_DISPLAY_SCREEN;
//						xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
//					}
//				}
				else
				{
					if(last_percent != g_bat_evt.level_percent)
					{
						if(((last_percent > 10) && (g_bat_evt.level_percent <= 10))
							|| ((last_percent > 5) && (g_bat_evt.level_percent <= 5)))
						{

							//剩余电量10%和5%的时候提醒一次
							if(ScreenState != DISPLAY_SCREEN_LOGO)
							{
								if(ScreenState < DISPLAY_SCREEN_NOTIFY)
								{
									ScreenStateSave = ScreenState;
								}
								timer_notify_display_start(3000,1,false);
								timer_notify_motor_start(1000,500,1,false,NOTIFY_MODE_NORMAL);
								timer_notify_buzzer_start(1000,500,1,false,NOTIFY_MODE_NORMAL);
								DISPLAY_MSG  msg = {0,0};
								ScreenState = DISPLAY_SCREEN_NOTIFY_LOW_BAT;
								msg.cmd = MSG_DISPLAY_SCREEN;
								xQueueSend(DisplayQueue, &msg, portMAX_DELAY);								
							}
						}
						else
						{
							if(ScreenState == DISPLAY_SCREEN_HOME)
							{
								DISPLAY_MSG  msg = {0,0};
			
								ScreenState = DISPLAY_SCREEN_HOME;
								msg.cmd = MSG_DISPLAY_SCREEN;
								xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
							}
						}
					}
				}
				
				last_percent = 	g_bat_evt.level_percent;
			}
			break;
			default:
				break;
		}			
	}
}



void task_battery_init(void)
{

    if(am_hal_gpio_input_bit_read(DRV_BAT_USB_DETECT_PIN))
    {
		start_charge = true;
		task_pc_init();
		g_bat_evt.cable_status = DRV_BAT_EVENT_USB_CONN_CHARGING;
		g_bat_evt.usb_detect_pin_polarity = AM_HAL_GPIO_FALLING;
	}
	else{
		g_bat_evt.cable_status = DRV_BAT_EVENT_USB_DISCONN;
		g_bat_evt.usb_detect_pin_polarity = AM_HAL_GPIO_RISING;
	}
	
	
	voltage_divider_t   voltage_divider = 
	{
     .r_1_ohm = 620000,
     .r_2_ohm = 510000,		
	};
	_drv_bat_param  param = 
	{
    .adc_pin = DRV_BAT_ADC_PIN,                     
    .usb_detect_pin = DRV_BAT_USB_DETECT_PIN, 
		.usb_detect_pin_polarity = g_bat_evt.usb_detect_pin_polarity,
    .batt_chg_stat_pin = DRV_BAT_CHG_STATE_PIN,               
    .batt_voltage_limit_low = 12000,       
    .batt_voltage_limit_full = 16000,
		.voltage_divider = voltage_divider,	
	};
	_drv_bat p_drv_bat = 
	{
		.param = param,
		.evt_handler = task_cb,
	};	
//	_lib_app_data lib_app_data;
//	lib_app_data_read(&lib_app_data);
	if (SetValue.PowerPercentRecord  > 100)
	{
		SetValue.PowerPercentRecord = 100;
	}
	p_drv_bat.bat_pct = SetValue.PowerPercentRecord;	
//	drv_bat_init(&p_drv_bat);
	evt_handle = xEventGroupCreate();

	if(evt_handle == NULL)
	{
		ERR_HANDLER(ERR_NO_MEM);
	}

	BaseType_t xResult;
	xResult = xTaskCreate(task, "BATTERY", TaskBattery_StackDepth, NULL, TaskBattery_Priority, &task_handle);
	if (xResult == pdFAIL)
	{
		ERR_HANDLER(ERR_NO_MEM);
	}	
	timer_battery_init();
	
	drv_bat_init(&p_drv_bat, &start_charge);
}






