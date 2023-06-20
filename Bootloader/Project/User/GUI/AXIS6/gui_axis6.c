#ifdef LAYER_APPLICATION 
#include "gui_axis6.h"
#include "rtt_log.h"
#include "lib_error.h"
#include "SEGGER_RTT.h"

#include "lib_display.h"
#include <string.h>
#include <stdio.h>


#include "drv_lcd.h"
#include "drv_button.h"
#include "gui_util.h"
#include "gui_menu.h"
#include "dev_lcd.h"

#include "am_util.h"
#include "drv_ble.h"
#include "com_apollo2.h"
#ifdef BOSH_OFFICIAL_SITE_API
#include "drv_bosh_official_api.h"
#endif
#ifdef BOSH_KEIL_API
#include "drv_bosh_keil_api.h"
#endif


#include "timer_sensor.h"


#define MOUDLE_LOG_ENABLED 							0
#define MOUDLE_DEBUG_ENABLED   					0
#define MOUDLE_NAME                     "[GUI_AXIS6]:"

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




#define DELAY_MS(X)                 am_util_delay_ms(X)
//#define DELAY_MS(X)                 vTaskDelay(X / portTICK_PERIOD_MS)
//#define DELAY_MS(X)                   do{TickType_t xLastWakeTime;xLastWakeTime = xTaskGetTickCount ();vTaskDelayUntil( &xLastWakeTime, X / portTICK_PERIOD_MS );}while(0)

extern uint8_t g_lcd_ram[DRV_LCD_HIGHT*2][DRV_LCD_WIDTH/2];
extern uint32_t g_gui_index;
extern uint32_t g_gui_index_save;

#define BACKGROUND_COLOR      LIB_DISPLAY_COLOR_00
#define FOREGROUND_COLOR      LIB_DISPLAY_COLOR_77

static bool m_axis6_switch_select = false;

extern _bmi160_data bmi160_data;

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
	char p_char[] = "AXIS6";	
	lib_display_ascii(p_char, strlen(p_char), &param);	
}

static void axis6_switch(void)
{
	_lib_display_param param = 
	{
		.x = 16+32,
		.y = 20,
		.height = 16,
		.gap = 0,
		.bg_color = BACKGROUND_COLOR,
		.fg_color = FOREGROUND_COLOR,
	};	
	char p_char[23];
	memset(p_char,0,sizeof(p_char));
	if(m_axis6_switch_select)
	{
		sprintf(p_char,"Axis6 : on");	
	}else{
		sprintf(p_char,"Axis6 : off");	
	}	
	
	lib_display_ascii(p_char, strlen(p_char), &param);
}

static void axis6_switch_paint(void)
{
	_lib_display_param param = 
	{
		.x = 16+32,
		.y = 20,
		.height = 16,
		.gap = 0,
		.bg_color = BACKGROUND_COLOR,
		.fg_color = FOREGROUND_COLOR,
	};	
	for(uint16_t i=0;i<param.height*2;i++)
	{
		memset(g_lcd_ram[param.x+i],BACKGROUND_COLOR,DRV_LCD_WIDTH/2);
	}	
	char p_char[23];
	memset(p_char,0,sizeof(p_char));
	if(m_axis6_switch_select)
	{
		sprintf(p_char,"Axis6 : on");	
	}else{
		sprintf(p_char,"Axis6 : off");	
	}	
	
	lib_display_ascii(p_char, strlen(p_char), &param);
	_drv_lcd_graph graph =
	{
		.x = 16+32,
		.y = 20,
		.height = 16*2,
	};	
	drv_lcd_set_one_frame(&graph);
}
static void axis6_switch_confirm_paint(void)
{
	_lib_display_param param = 
	{
		.x = 16+32,
		.y = 20,
		.height = 16,
		.gap = 0,
		.bg_color = BACKGROUND_COLOR,
		.fg_color = FOREGROUND_COLOR,
	};	
	for(uint16_t i=0;i<param.height*2;i++)
	{
		memset(g_lcd_ram[param.x+i],BACKGROUND_COLOR,DRV_LCD_WIDTH/2);
	}	
	char p_char[23];
	memset(p_char,0,sizeof(p_char));
	if(m_axis6_switch_select)
	{
		sprintf(p_char,"Axis6 : on OK");	
	}else{
		sprintf(p_char,"Axis6 : off OK");	
	}	
	
	lib_display_ascii(p_char, strlen(p_char), &param);
	_drv_lcd_graph graph =
	{
		.x = 16+32,
		.y = 20,
		.height = 16*2,
	};	
	drv_lcd_set_one_frame(&graph);
}

void gui_axis6_sensor_data_paint(void)
{
	_lib_display_param param = 
	{
		.x = 16+32+32,
		.y = 15,
		.height = 16,
		.gap = 0,
		.bg_color = BACKGROUND_COLOR,
		.fg_color = FOREGROUND_COLOR,
	};	
	for(uint16_t i=0;i<param.height*2;i++)
	{
		memset(g_lcd_ram[param.x+i],BACKGROUND_COLOR,DRV_LCD_WIDTH/2);
	}	
	char p_char[23];
	memset(p_char,0,sizeof(p_char));
	sprintf(p_char,"X:%d,Y:%d,Z:%d",bmi160_data.accel.x,bmi160_data.accel.y,bmi160_data.accel.z);	
	lib_display_ascii(p_char, strlen(p_char), &param);
	_drv_lcd_graph graph =
	{
		.x = 16+32+32,
		.y = 15,
		.height = 16*2,
	};	
	drv_lcd_set_one_frame(&graph);

}

void gui_axis6_paint(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"gui_axis6_paint\n");
	memset(g_lcd_ram,BACKGROUND_COLOR,sizeof(g_lcd_ram));

	title();
	axis6_switch();

	_drv_lcd_graph graph =
	{
		.x = 0,
		.y = 0,
		.height = DRV_LCD_HIGHT*2,
		.width = DRV_LCD_WIDTH/2,
	};
	drv_lcd_set_one_frame(&graph);
}

void gui_axis6_btn_evt(uint32_t evt)
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
			m_axis6_switch_select = !m_axis6_switch_select;
			axis6_switch_paint();


			
		}break;	
		case (BTN_PIN_UP<<1):{
			DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_UP long push\n");
			
		}break;				
		case (BTN_PIN_OK):{
			DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_OK release\n");
			axis6_switch_confirm_paint();
			
			if(m_axis6_switch_select)
			{
				#ifdef BOSH_OFFICIAL_SITE_API
				drv_bosh_official_api_init();
				timer_sensor_start();
				#endif
				
				#ifdef BOSH_KEIL_API
				drv_bosh_keil_api_init();
				#endif				
				
				
				
			}
			else{
				#ifdef BOSH_OFFICIAL_SITE_API
				timer_sensor_stop();
				drv_bosh_official_api_uninit();
				#endif
				
				#ifdef BOSH_KEIL_API
				drv_bosh_keil_api_uninit();
				#endif					
				
			}
			
			
			
		}break;
		case (BTN_PIN_OK<<1):{
			DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_OK long push\n");

			

			
				
			
			
			
		}break;		
		case (BTN_PIN_DOWN):{
			DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_DOWN release\n");
			m_axis6_switch_select = !m_axis6_switch_select;
			axis6_switch_paint();
		}break;
		
		case (BTN_PIN_DOWN<<1):{
			DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_DOWN long push\n");

		}break;					
	
	}



}

#endif


