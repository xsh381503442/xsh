
#include "task_button.h"
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

#include "com_apollo2.h"
#include "drv_button.h"
#include "dev_notify.h"
#include "dev_dfu.h"
#include "dev_extFlash.h"

#include "lib_boot.h"

#include "lib_boot_setting.h"
#include "drv_ble.h"
#include <string.h>

#include "cmd_pc.h"
#include "cmd_ble.h"

#include "am_util_delay.h"


#include "drv_lcd.h"
#include "dev_lcd.h"
#include <stdio.h>

#include "task_display.h"

#include "gui_boot.h"
#include "gui_post_watch_info.h"
#include "gui_exception.h"
#include "gui_back_door.h"



#define MOUDLE_LOG_ENABLED 							1
#define MOUDLE_DEBUG_ENABLED   					1
#define MOUDLE_NAME                     "[TASK_BUTTON]:"

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


extern ScreenState_t   ScreenState;
static TaskHandle_t task_handle;
static EventGroupHandle_t evt_handle;


static void task_cb(uint32_t evt)
{
	BaseType_t xHigherPriorityTaskWoken, xResult;

	xHigherPriorityTaskWoken = pdFALSE;

	xResult = xEventGroupSetBitsFromISR(evt_handle, (evt),
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

void  KeyEventHandler( uint32_t Key_Value )
{
	switch( ScreenState )
	{
	    case DISPLAY_SCREEN_BOOTLOADER:
				DEBUG_PRINTF(MOUDLE_NAME"DISPLAY_SCREEN_BOOTLOADER\n");
				gui_boot_btn_evt(Key_Value);
				break;
			case DISPLAY_SCREEN_POST_WATCH_INFO:
				DEBUG_PRINTF(MOUDLE_NAME"DISPLAY_SCREEN_POST_WATCH_INFO\n");
				gui_post_watch_info_btn_evt(Key_Value);
			break;
			case DISPLAY_SCREEN_EXCEPTION:
				DEBUG_PRINTF(MOUDLE_NAME"DISPLAY_SCREEN_EXCEPTION\n");
				gui_exception_btn_evt(Key_Value);
			break;			
			case DISPLAY_SCREEN_BACK_DOOR:
				DEBUG_PRINTF(MOUDLE_NAME"DISPLAY_SCREEN_BACK_DOOR\n");
				gui_back_door_btn_evt(Key_Value);
			break;	
		default:
			break;
	}
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
			case (BTN_PIN_PWR):{
				DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_PWR release\n");

				
			}break;
			case (BTN_PIN_PWR<<1):{
				DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_PWR long push\n");

			}break;		
			case (BTN_PIN_BL):{
				DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_BL release\n");
					
			}break;
			case (BTN_PIN_BL<<1):{
				DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_BL long push\n");
		
				
				
			}break;									
			case (BTN_PIN_UP):{
				DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_UP release\n");

				
			}break;	
			case (BTN_PIN_UP<<1):{
				DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_UP long push\n");
				if(am_hal_gpio_input_bit_read(BTN_PIN_DOWN))
				{
					DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_UP_DOWN\n");
					bitSet = BTN_PIN_UP_DOWN;
				}				
			}break;				
			case (BTN_PIN_OK):{
				DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_OK release\n");

			}break;
			case (BTN_PIN_OK<<1):{
				DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_OK long push\n");

			}break;		
			case (BTN_PIN_DOWN):{
				DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_DOWN release\n");

			}break;
			
			case (BTN_PIN_DOWN<<1):{
				DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_DOWN long push\n");
				if(am_hal_gpio_input_bit_read(BTN_PIN_UP))
				{
					DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_UP_DOWN\n");
					bitSet = BTN_PIN_UP_DOWN;
				}		
			}break;					
		
	  }
		
		KeyEventHandler(bitSet);

		
		
	}
	
	
	
	
	
}





void task_button_init(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"task_button_init\n");
	
	drv_button_init(task_cb);
	
	evt_handle = xEventGroupCreate();

	if(evt_handle == NULL)
	{
		DEBUG_PRINTF(MOUDLE_NAME"xEventGroupCreate failed\n");
		ERR_HANDLER(ERR_NULL);
	}
	BaseType_t xResult;
	xResult = xTaskCreate(task, "BUTTON", 1024, 0, 5, &task_handle);
	if (xResult == pdFAIL)
	{
		ERR_HANDLER(ERR_FORBIDDEN);
	}	
}










