#ifdef LAYER_APPLICATION 
#include "gui_menu.h"
#include "rtt_log.h"
#include "lib_error.h"
#include "SEGGER_RTT.h"

#include "lib_display.h"
#include <string.h>
#include <stdio.h>

#include "am_mcu_apollo.h"

#include "drv_lcd.h"
#include "drv_button.h"
#include "gui_util.h"
#include "gui_ble.h"
#include "gui_axis6.h"
#include "gui_mag.h"
#include "gui_gps.h"
#include "gui_heartrate.h"
#include "gui_home.h"
#include "gui_font.h"


#include "drv_ble.h"
#include "cmd_ble.h"
#include "task_ble.h"
#include "dev_ble.h"
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
#define MOUDLE_NAME                     "[GUI_MENU]:"

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

#define BACKGROUND_COLOR      LIB_DISPLAY_COLOR_00
#define FOREGROUND_COLOR      LIB_DISPLAY_COLOR_77

extern uint8_t g_lcd_ram[DRV_LCD_HIGHT*2][DRV_LCD_WIDTH/2];

#define MENU_INDEX_BLE           1
#define MENU_INDEX_AXIS6         2
#define MENU_INDEX_MAG           3
#define MENU_INDEX_GPS           4
#define MENU_INDEX_HEARTRATE     5
#define MENU_INDEX_FONT          6

#define MENU_INDEX_MIN          MENU_INDEX_BLE
#define MENU_INDEX_MAX          MENU_INDEX_FONT
extern am_hal_rtc_time_t g_rtc_time;
static uint32_t m_menu_index = MENU_INDEX_BLE;
extern uint32_t g_gui_index;
extern uint32_t g_gui_index_save;
extern _bluetooth_status g_bluetooth_status;
extern _dev_ble_info g_ble_info;



static void title(void)
{
	_lib_display_param param = 
	{
		.x = 16,
		.y = 50,
		.height = 16,
		.gap = 0,
		.bg_color = BACKGROUND_COLOR,
		.fg_color = FOREGROUND_COLOR,
	};
	char p_char[] = "Menu";	
	lib_display_ascii(p_char, strlen(p_char), &param);	
}


static void ble(void)
{
	_lib_display_param param = 
	{
		.x = 150,
		.y = 30,
		.height = 16,
		.gap = 0,
		.bg_color = BACKGROUND_COLOR,
		.fg_color = FOREGROUND_COLOR,
	};
	char p_char[23];	
	memset(p_char,0,sizeof(p_char));
	sprintf(p_char,"FONT");
	lib_display_ascii(p_char, strlen(p_char), &param);	
	param.x += 32;
	memset(p_char,0,sizeof(p_char));
	sprintf(p_char,"BLE");
	lib_display_ascii(p_char, strlen(p_char), &param);	
	param.x += 32;
	memset(p_char,0,sizeof(p_char));
	sprintf(p_char,"AXIS6");
	lib_display_ascii(p_char, strlen(p_char), &param);		
	
}
static void axis6(void)
{
	_lib_display_param param = 
	{
		.x = 150,
		.y = 30,
		.height = 16,
		.gap = 0,
		.bg_color = BACKGROUND_COLOR,
		.fg_color = FOREGROUND_COLOR,
	};
	char p_char[23];	
	memset(p_char,0,sizeof(p_char));
	sprintf(p_char,"BLE");
	lib_display_ascii(p_char, strlen(p_char), &param);	
	param.x += 32;
	memset(p_char,0,sizeof(p_char));
	sprintf(p_char,"AXIS6");
	lib_display_ascii(p_char, strlen(p_char), &param);	
	param.x += 32;
	memset(p_char,0,sizeof(p_char));
	sprintf(p_char,"MAG");
	lib_display_ascii(p_char, strlen(p_char), &param);

}
static void mag(void)
{
	_lib_display_param param = 
	{
		.x = 150,
		.y = 30,
		.height = 16,
		.gap = 0,
		.bg_color = BACKGROUND_COLOR,
		.fg_color = FOREGROUND_COLOR,
	};
	char p_char[23];	
	memset(p_char,0,sizeof(p_char));
	sprintf(p_char,"AXIS6");
	lib_display_ascii(p_char, strlen(p_char), &param);	
	param.x += 32;
	memset(p_char,0,sizeof(p_char));
	sprintf(p_char,"MAG");
	lib_display_ascii(p_char, strlen(p_char), &param);	
	param.x += 32;
	memset(p_char,0,sizeof(p_char));
	sprintf(p_char,"GPS");
	lib_display_ascii(p_char, strlen(p_char), &param);

}
static void gps(void)
{
	_lib_display_param param = 
	{
		.x = 150,
		.y = 30,
		.height = 16,
		.gap = 0,
		.bg_color = BACKGROUND_COLOR,
		.fg_color = FOREGROUND_COLOR,
	};
	char p_char[23];	
	memset(p_char,0,sizeof(p_char));
	sprintf(p_char,"MAG");
	lib_display_ascii(p_char, strlen(p_char), &param);	
	param.x += 32;
	memset(p_char,0,sizeof(p_char));
	sprintf(p_char,"GPS");
	lib_display_ascii(p_char, strlen(p_char), &param);	
	param.x += 32;
	memset(p_char,0,sizeof(p_char));
	sprintf(p_char,"HEARTRATE");
	lib_display_ascii(p_char, strlen(p_char), &param);

}

static void heartrate(void)
{
	_lib_display_param param = 
	{
		.x = 150,
		.y = 30,
		.height = 16,
		.gap = 0,
		.bg_color = BACKGROUND_COLOR,
		.fg_color = FOREGROUND_COLOR,
	};
	char p_char[23];	
	memset(p_char,0,sizeof(p_char));
	sprintf(p_char,"GPS");
	lib_display_ascii(p_char, strlen(p_char), &param);	
	param.x += 32;
	memset(p_char,0,sizeof(p_char));
	sprintf(p_char,"HEARTRATE");
	lib_display_ascii(p_char, strlen(p_char), &param);	
	param.x += 32;
	memset(p_char,0,sizeof(p_char));
	sprintf(p_char,"FONT");
	lib_display_ascii(p_char, strlen(p_char), &param);

}
static void font(void)
{
	_lib_display_param param = 
	{
		.x = 150,
		.y = 30,
		.height = 16,
		.gap = 0,
		.bg_color = BACKGROUND_COLOR,
		.fg_color = FOREGROUND_COLOR,
	};
	char p_char[23];	
	memset(p_char,0,sizeof(p_char));
	sprintf(p_char,"HEARTRATE");
	lib_display_ascii(p_char, strlen(p_char), &param);	
	param.x += 32;
	memset(p_char,0,sizeof(p_char));
	sprintf(p_char,"FONT");
	lib_display_ascii(p_char, strlen(p_char), &param);	
	param.x += 32;
	memset(p_char,0,sizeof(p_char));
	sprintf(p_char,"BLE");
	lib_display_ascii(p_char, strlen(p_char), &param);

}
static void timer(void)
{
	am_hal_rtc_time_get(&g_rtc_time);
	_lib_display_param param = 
	{
		.x = 440,
		.y = 42,
		.height = 16,
		.gap = 0,
		.bg_color = BACKGROUND_COLOR,
		.fg_color = FOREGROUND_COLOR,
	};	
	char p_char[23];
	memset(p_char,0,sizeof(p_char));
	sprintf(p_char,"%02d:%02d:%02d",g_rtc_time.ui32Hour,g_rtc_time.ui32Minute,g_rtc_time.ui32Second);	
	lib_display_ascii(p_char, strlen(p_char), &param);
}

void gui_menu_paint(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"gui_menu_paint\n");
	memset(g_lcd_ram,BACKGROUND_COLOR,sizeof(g_lcd_ram));

	title();
	
	switch(m_menu_index)
	{
		case MENU_INDEX_BLE:{
			ble();
		
		}break;
		case MENU_INDEX_AXIS6:{
			axis6();
		}break;
		case MENU_INDEX_MAG:{
			mag();
		}break;
		case MENU_INDEX_GPS:{
			gps();
		}break;		
		case MENU_INDEX_HEARTRATE:{
			heartrate();
		}break;	
		case MENU_INDEX_FONT:{
			font();
		}break;			
	}
	timer();
	_drv_lcd_graph graph =
	{
		.x = 0,
		.y = 0,
		.height = DRV_LCD_HIGHT*2,
		.width = DRV_LCD_WIDTH/2,
	};
	drv_lcd_set_one_frame(&graph);
}


void gui_menu_btn_evt(uint32_t evt)
{
	switch(evt)
	{		
		case (BTN_PIN_PWR):{
			DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_PWR release\n");
			g_gui_index = GUI_INDEX_HOME;
			g_gui_index_save = g_gui_index;
			gui_home_paint();	
			
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
			if(m_menu_index <= MENU_INDEX_MIN)
			{
				m_menu_index = MENU_INDEX_MAX;
			}else{
				m_menu_index--;
			}
			gui_menu_paint();	
		
		}break;	
		case (BTN_PIN_UP<<1):{
			DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_UP long push\n");
		}break;				
		case (BTN_PIN_OK):{
			DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_OK release\n");
			switch(m_menu_index)
			{			
				
				case MENU_INDEX_BLE:{
					
					if(g_bluetooth_status.is_on)
					{
						g_gui_index = GUI_INDEX_BLE;
						g_gui_index_save = g_gui_index;					
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
					else{
						g_gui_index = GUI_INDEX_BLE;
						g_gui_index_save = g_gui_index;						
						gui_ble_paint();		
					}

				}break;
				case MENU_INDEX_AXIS6:{
					g_gui_index = GUI_INDEX_AXIS6;
					g_gui_index_save = g_gui_index;
					gui_axis6_paint();
				}break;		
				case MENU_INDEX_MAG:{
					g_gui_index = GUI_INDEX_MAG;
					g_gui_index_save = g_gui_index;	
					gui_mag_paint();					
				}break;
				case MENU_INDEX_GPS:{
					g_gui_index = GUI_INDEX_GPS;
					g_gui_index_save = g_gui_index;
					gui_gps_paint();
				}break;	
				case MENU_INDEX_HEARTRATE:{
					g_gui_index = GUI_INDEX_HEARTRATE;
					g_gui_index_save = g_gui_index;
					gui_heartrate_paint();
				}break;	
				case MENU_INDEX_FONT:{
					g_gui_index = GUI_INDEX_FONT;
					g_gui_index_save = g_gui_index;
					gui_font_paint();
				}break;			
			}						
			
			
		}break;
		case (BTN_PIN_OK<<1):{
			DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_OK long push\n");

		}break;		
		case (BTN_PIN_DOWN):{
			DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_DOWN release\n");
			if(m_menu_index >= MENU_INDEX_MAX)
			{
				m_menu_index = MENU_INDEX_MIN;
			}else{
				m_menu_index++;
			}
			gui_menu_paint();	
		}break;
		
		case (BTN_PIN_DOWN<<1):{
			DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_DOWN long push\n");
	
		}break;					
	
	}



}



#endif 



