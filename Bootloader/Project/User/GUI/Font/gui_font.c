#ifdef LAYER_APPLICATION 
#include "gui_font.h"
#include "rtt_log.h"
#include "lib_error.h"
#include "SEGGER_RTT.h"

#include "lib_display.h"
#include <string.h>
#include <stdio.h>


#include "drv_lcd.h"
#include "drv_button.h"
#include "gui_util.h"


#include "am_util.h"
#include "gui_home.h"
#include "gui_menu.h"
#include "dev_extFlash.h"
#include "dev_font.h"
#include "am_mcu_apollo.h"
#include "dev_ble.h"
#include "task_ble.h"
#include "drv_ble.h"
#include "cmd_ble.h"


#include "dev_font.h"
#include "dev_extFlash.h"


#define MOUDLE_LOG_ENABLED 							1
#define MOUDLE_DEBUG_ENABLED   					1
#define MOUDLE_NAME                     "[GUI_FONT]:"

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
extern uint32_t g_gui_index;
extern uint32_t g_gui_index_save;

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
	char p_char[] = "FONT";	
	lib_display_ascii(p_char, strlen(p_char), &param);	
}
void gui_font_paint(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"gui_font_paint\n");
	memset(g_lcd_ram,BACKGROUND_COLOR,sizeof(g_lcd_ram));
	
	title();
	
	_drv_lcd_graph graph =
	{
			.x = 0,
			.y = 0,
			.height = DRV_LCD_HIGHT*2,
			.width = DRV_LCD_WIDTH/2,
	};
	drv_lcd_set_one_frame(&graph);
}


void gui_font_btn_evt(uint32_t evt)
{
	
	switch(evt)
	{		
		case (BTN_PIN_PWR):{
			DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_PWR release\n");
			g_gui_index = GUI_INDEX_MENU;
			g_gui_index_save = g_gui_index;
			gui_menu_paint();	

			
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
	
		}break;					
	
	}

	

}




#endif

