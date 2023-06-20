#ifdef LAYER_APPLICATION 
#include "gui_pof.h"
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
#include "dev_extFlash.h"
#include "dev_font.h"
#include "am_mcu_apollo.h"
#include "dev_ble.h"
#include "task_ble.h"
#include "drv_ble.h"
#include "cmd_ble.h"

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



extern uint8_t g_lcd_ram[DRV_LCD_HIGHT*2][DRV_LCD_WIDTH/2];
extern uint32_t g_gui_index;
extern uint32_t g_gui_index_save;
extern _bluetooth_status g_bluetooth_status;


static bool m_is_pof = false;
static bool m_ble_switch_temp = false;

void gui_pof_paint(void)
{
	
	memset(g_lcd_ram,LIB_DISPLAY_COLOR_00,sizeof(g_lcd_ram));
	_drv_lcd_graph graph =
	{
			.x = 0,
			.y = 0,
			.height = DRV_LCD_HIGHT*2,
			.width = DRV_LCD_WIDTH/2,
	};
	drv_lcd_set_one_frame(&graph);
	
	
	

}
void gui_pof_btn_evt(uint32_t evt)
{
	
	switch(evt)
	{		
		case (BTN_PIN_PWR):{
			DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_PWR release\n");


			
		}break;
		case (BTN_PIN_PWR<<1):{
			DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_PWR long push\n");
			if(m_is_pof)
			{
				
				if(m_ble_switch_temp)
				{
					m_ble_switch_temp = false;
					dev_ble_system_on();
				}				
				
				g_gui_index = GUI_INDEX_HOME;
				g_gui_index_save = g_gui_index;
				drv_lcd_init();
				gui_home_paint();				
				m_is_pof = false;
			}
			else{		

				if(g_bluetooth_status.is_on)
				{
					m_ble_switch_temp = true;
					dev_ble_system_off();
				}
			  
								
				
				gui_pof_paint();
				drv_lcd_uninit();
				m_is_pof = true;
			}
			
			


			
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

