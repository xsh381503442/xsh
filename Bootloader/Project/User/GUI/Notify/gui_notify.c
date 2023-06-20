#ifdef LAYER_APPLICATION 
#include "gui_notify.h"
#include "rtt_log.h"
#include "lib_error.h"
#include "SEGGER_RTT.h"

#include "lib_display.h"
#include <string.h>
#include <stdio.h>


#include "drv_lcd.h"
#include "drv_button.h"
#include "gui_util.h"
#include "gui_home.h"
#include "gui_menu.h"
#include "gui_ble.h"
#include "gui_axis6.h"
#include "gui_mag.h"
#include "gui_gps.h"
#include "gui_heartrate.h"
#include "gui_hwt.h"

#include "drv_ble.h"
#include "cmd_ble.h"
#include "task_ble.h"

#include "timer_notify.h"
#include "am_util.h"


#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "portmacro.h"
#include "portable.h"
#include "event_groups.h"
#include "semphr.h"

#define MOUDLE_LOG_ENABLED 							0
#define MOUDLE_DEBUG_ENABLED   					0
#define MOUDLE_NAME                     "[GUI_NOTIFY]:"

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

//#define DELAY_MS(X)                 am_util_delay_ms(X)
#define DELAY_MS(X)                 vTaskDelay(X / portTICK_PERIOD_MS)
//#define DELAY_MS(X)                   do{TickType_t xLastWakeTime;xLastWakeTime = xTaskGetTickCount ();vTaskDelayUntil( &xLastWakeTime, X / portTICK_PERIOD_MS );}while(0)

#define BACKGROUND_COLOR      LIB_DISPLAY_COLOR_77
#define FOREGROUND_COLOR      LIB_DISPLAY_COLOR_00

extern uint8_t g_lcd_ram[DRV_LCD_HIGHT*2][DRV_LCD_WIDTH/2];
extern uint32_t g_gui_index;
extern uint32_t g_gui_index_save;
extern _bluetooth_status g_bluetooth_status;
extern _ble_passkey g_ble_passkey;


void gui_notify_connect_paint(void)
{
	_lib_display_param param = 
	{
		.x = 208,
		.y = 45,
		.height = 16,
		.gap = 0,
		.bg_color = BACKGROUND_COLOR,
		.fg_color = FOREGROUND_COLOR,	
	};	
	for(uint16_t i=0;i<param.height*2;i++)
	{
		memset(g_lcd_ram[param.x+i]+param.y-10,BACKGROUND_COLOR,10*4+20);
	}		
	char p_char[23];
	memset(p_char,0,sizeof(p_char));
	sprintf(p_char,"Connect");	
	lib_display_ascii(p_char, strlen(p_char), &param);	
	_drv_lcd_graph graph =
	{
		.x = 208,
		.y = 45,
		.height = 16*2,
	};	
	drv_lcd_set_one_frame(&graph);	

}


void gui_notify_disconnect_paint(void)
{
	_lib_display_param param = 
	{
		.x = 208,
		.y = 45,
		.height = 16,
		.gap = 0,
		.bg_color = BACKGROUND_COLOR,
		.fg_color = FOREGROUND_COLOR,		
	};	
	for(uint16_t i=0;i<param.height*2;i++)
	{
		memset(g_lcd_ram[param.x+i]+param.y-10,BACKGROUND_COLOR,10*4+20);
	}		
	char p_char[23];
	memset(p_char,0,sizeof(p_char));
	sprintf(p_char,"Disconnect");	
	lib_display_ascii(p_char, strlen(p_char), &param);	
	_drv_lcd_graph graph =
	{
		.x = 208,
		.y = 45,
		.height = 16*2,
	};	
	drv_lcd_set_one_frame(&graph);


}



void gui_notify_bond_paint(void)
{
	_lib_display_param param = 
	{
		.x = 208,
		.y = 45,
		.height = 16,
		.gap = 0,
		.bg_color = BACKGROUND_COLOR,
		.fg_color = FOREGROUND_COLOR,	
	};
	for(uint16_t i=0;i<param.height*2;i++)
	{
		memset(g_lcd_ram[param.x+i]+param.y-10,BACKGROUND_COLOR,10*4+20);
	}	
	char p_char[23];
	memset(p_char,0,sizeof(p_char));
	sprintf(p_char,"Bond");	
	lib_display_ascii(p_char, strlen(p_char), &param);	
	_drv_lcd_graph graph =
	{
		.x = 208,
		.y = 45,
		.height = 16*2,
	};	
	drv_lcd_set_one_frame(&graph);


}


void gui_notify_passkey_paint(void)
{
	_lib_display_param param = 
	{
		.x = 208,
		.y = 45,
		.height = 16,
		.gap = 0,
		.bg_color = BACKGROUND_COLOR,
		.fg_color = FOREGROUND_COLOR,	
	};	
	for(uint16_t i=0;i<param.height*2;i++)
	{
		memset(g_lcd_ram[param.x+i]+param.y-10,BACKGROUND_COLOR,10*4+20);
	}
	char p_char[23];
	memset(p_char,0,sizeof(p_char));
	memcpy(p_char,g_ble_passkey.passkey,BLE_GAP_PASSKEY_LEN);
	lib_display_ascii(p_char, strlen(p_char), &param);	
	_drv_lcd_graph graph =
	{
		.x = 208,
		.y = 45,
		.height = 16*2,
	};	
	drv_lcd_set_one_frame(&graph);


}

void gui_notify_rollback_paint(uint32_t gui_index)
{
	DEBUG_PRINTF(MOUDLE_NAME"gui_notify_rollback_paint\n");
	
	switch(g_gui_index)
	{
		case GUI_INDEX_HOME:{
			gui_home_paint();
		
		}break;
		case GUI_INDEX_MENU:{
			gui_menu_paint();
		
		}break;
		case GUI_INDEX_BLE:{					
			if(g_bluetooth_status.is_conn)
			{			
				uint32_t command[1] = {BLE_UART_CCOMMAND_APPLICATION_BLE_INFO | BLE_UART_CCOMMAND_MASK_APPLICATION};
				_uart_data_param param;
				param.user_callback = NULL;
				param.p_data = (uint8_t *)command;
				param.length = sizeof(command);
				uint8_t count = TASK_UART_DELAY_COUNT_OUT;
				while(drv_ble_send_data(&param) != ERR_SUCCESS && count--)
				{
					DELAY_MS(TASK_UART_DELAY);
				}					
			}	
		
		}break;		
		case GUI_INDEX_AXIS6:{
			gui_axis6_paint();
		
		}break;				
		case GUI_INDEX_MAG:{
			gui_mag_paint();
		
		}break;				
		case GUI_INDEX_GPS:{
			gui_gps_paint();
		
		}break;				
		case GUI_INDEX_HEARTRATE:{
			gui_heartrate_paint();
		
		}break;				
		case GUI_INDEX_HWT:{
			gui_hwt_paint();
		
		}break;			
		
		
		
		
	}			
	
	
	
}

void gui_notify_btn_evt(uint32_t evt)
{
	
	switch(g_gui_index)
	{
		case GUI_INDEX_NOTIFY_CONNECT:
		case GUI_INDEX_NOTIFY_DISCONNECT:
		case GUI_INDEX_NOTIFY_BOND:
		case GUI_INDEX_NOTIFY_PASSKEY:{
			if(evt == BTN_PIN_PWR ||
				 evt == BTN_PIN_BL ||
				 evt == BTN_PIN_UP ||
				 evt == BTN_PIN_OK ||
				 evt == BTN_PIN_DOWN)
			{
				DEBUG_PRINTF(MOUDLE_NAME"g_gui_index --> %d\n",g_gui_index);
				DEBUG_PRINTF(MOUDLE_NAME"g_gui_index_save --> %d\n",g_gui_index_save);					
				timer_notify_stop();
				g_gui_index = g_gui_index_save;
				g_gui_index_save = g_gui_index;
				gui_notify_rollback_paint(g_gui_index);
				
			}	
		}break;			
	}		

	

}




#endif

