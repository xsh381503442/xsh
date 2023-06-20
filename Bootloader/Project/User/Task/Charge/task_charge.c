#include "task_charge.h"
#include "rtt_log.h"
#include "lib_error.h"
#include "SEGGER_RTT.h"

#include "am_mcu_apollo.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "portmacro.h"
#include "portable.h"
#include "event_groups.h"
#include "semphr.h"


#include "dev_bat.h"

#include <string.h>

#include "com_apollo2.h"



#include "gui_boot.h"
#include "task_display.h"



#define MOUDLE_LOG_ENABLED 							0
#define MOUDLE_DEBUG_ENABLED   					0
#define MOUDLE_NAME                     "[TASK_CHARGE]:"

#if MOUDLE_LOG_ENABLED == 1 && RTT_LOG_ENABLED == 1
#define LOG_PRINTF(...)                      SEGGER_RTT_printf(0, ##__VA_ARGS__) 
#define LOG_HEXDUMP(p_data, len)             rtt_log_hexdump(p_data, len) 
#else
#define LOG_PRINTF(...) 
#define LOG_HEXDUMP(p_data, len)             
#endif

#if MOUDLE_DEBUG_ENABLED == 1 && RTT_LOG_ENABLED == 1
#define DEBUG_PRINTF(...)                    SEGGER_RTT_printf(0, ##__VA_ARGS__)
#define DEBUG_HEXDUMP(p_data, len)           rtt_log_hexdump(p_data, len)
#else
#define DEBUG_PRINTF(...)   
#define DEBUG_HEXDUMP(p_data, len) 
#endif






static TaskHandle_t task_handle;
static EventGroupHandle_t evt_handle;


extern _dev_bat_event   g_bat_evt;


extern ScreenState_t   ScreenState;
extern QueueHandle_t 	DisplayQueue;

static void task_cb(_dev_bat_event * p_event)
{
	DEBUG_PRINTF(MOUDLE_NAME"task_cb\n");
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
	DEBUG_PRINTF(MOUDLE_NAME"task\n");

	uint32_t bitSet;
	
	while (1)
	{
		bitSet = xEventGroupWaitBits(evt_handle, 0xFF, pdTRUE,
												pdFALSE, portMAX_DELAY);
		DEBUG_PRINTF(MOUDLE_NAME"bitSet : %d\n",bitSet);

		switch(bitSet)
		{		
			case DEV_BAT_EVENT_DATA:{
				DEBUG_PRINTF(MOUDLE_NAME"DEV_BAT_EVENT_DATA\n");
				
			}break;
			case DEV_BAT_EVENT_LOW:{
				DEBUG_PRINTF(MOUDLE_NAME"DEV_BAT_EVENT_LOW\n");

				
			}break;
			case DEV_BAT_EVENT_FULL:{
				DEBUG_PRINTF(MOUDLE_NAME"DEV_BAT_EVENT_FULL\n");

				
			}break;
			case DEV_BAT_EVENT_USB_CONN_CHARGING:{
				DEBUG_PRINTF(MOUDLE_NAME"DEV_BAT_EVENT_USB_CONN_CHARGING\n");
				
			}break;
			case DEV_BAT_EVENT_USB_CONN_CHARGING_FINISHED:{
				DEBUG_PRINTF(MOUDLE_NAME"DEV_BAT_EVENT_USB_CONN_CHARGING_FINISHED\n");
			
			}break;
			case DEV_BAT_EVENT_USB_DISCONN:{
				DEBUG_PRINTF(MOUDLE_NAME"DEV_BAT_EVENT_USB_DISCONN\n");
		
			}break;			
			
			default:
				break;
		}
		
		
	}
}

void task_charge_init(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"task_charge_init\n");
	voltage_divider_t   voltage_divider = 
	{
     .r_1_ohm = 620000,
     .r_2_ohm = 510000,	
	
	};
	_dev_bat_param  param = 
	{
    .adc_pin = TASK_CHARGE_ADC_PIN,                     
    .usb_detect_pin = TASK_CHARGE_USB_DETECT_PIN, 
		.usb_detect_pin_polarity = g_bat_evt.usb_detect_pin_polarity,
    .batt_chg_stat_pin = TASK_CHARGE_BAT_CHG_STATE_PIN,               
    .batt_voltage_limit_low = 3400,       
    .batt_voltage_limit_full = 4200,
		.voltage_divider = voltage_divider,	
	};
	_dev_bat p_dev_bat = 
	{
		.param = param,
		.evt_handler = task_cb,
	};
	
	dev_bat_init(&p_dev_bat);
	
	evt_handle = xEventGroupCreate();

	if(evt_handle == NULL)
	{
		DEBUG_PRINTF(MOUDLE_NAME"xEventGroupCreate failed\n");
		ERR_HANDLER(ERR_NULL);
	}
	BaseType_t xResult;
	xResult = xTaskCreate(task, "CHARGE", 256, 0, 5, &task_handle);
	if (xResult == pdFAIL)
	{
		ERR_HANDLER(ERR_FORBIDDEN);
	}	
}














