#ifdef LAYER_APPLICATION 
#include "gui_ble.h"
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
#include "gui_menu.h"
#include "dev_ble.h"
#include "task_ble.h"
#include "drv_ble.h"
#include "cmd_ble.h"
#include "com_apollo2.h"

#include "am_util.h"

#define MOUDLE_LOG_ENABLED 							0
#define MOUDLE_DEBUG_ENABLED   					0
#define MOUDLE_NAME                     "[GUI_BLE]:"

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

#define BACKGROUND_COLOR      LIB_DISPLAY_COLOR_00
#define FOREGROUND_COLOR      LIB_DISPLAY_COLOR_77

extern uint8_t g_lcd_ram[DRV_LCD_HIGHT*2][DRV_LCD_WIDTH/2];
extern uint32_t g_gui_index;
extern uint32_t g_gui_index_save;
extern _dev_ble_info g_ble_info;
extern am_hal_rtc_time_t g_rtc_time;
extern _bluetooth_status g_bluetooth_status;

static bool m_ble_switch_temp = true;


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
	char p_char[] = "BLE";	
	lib_display_ascii(p_char, strlen(p_char), &param);	
}

static void version(void)
{
	uint8_t *version;
	
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
	version = (uint8_t *)&g_ble_info.application_version;
	sprintf(p_char,"A:V%d.%d.%dB%d",version[3],version[2],version[1],version[0]);
	lib_display_ascii(p_char, strlen(p_char), &param);	
	param.x += 32;
	memset(p_char,0,sizeof(p_char));
	version = (uint8_t *)&g_ble_info.bootloader_version;
	sprintf(p_char,"B:V%d.%d.%dB%d",version[3],version[2],version[1],version[0]);
	lib_display_ascii(p_char, strlen(p_char), &param);
	param.x += 32;
	memset(p_char,0,sizeof(p_char));
	version = (uint8_t *)&g_ble_info.softdevice_version;
	sprintf(p_char,"S:V%d.%d.%dB%d",version[3],version[2],version[1],version[0]);
	lib_display_ascii(p_char, strlen(p_char), &param);
	param.x += 32;
	memset(p_char,0,sizeof(p_char));
	version = (uint8_t *)&g_ble_info.pcb_version;
	sprintf(p_char,"PCB:V%d.%d.%dB%d",version[3],version[2],version[1],version[0]);
	lib_display_ascii(p_char, strlen(p_char), &param);
	param.x += 32;
	memset(p_char,0,sizeof(p_char));
	char name[10];
	memset(name,0,sizeof(name));
	memcpy(name,g_ble_info.dev_name.name,g_ble_info.dev_name.len);
	sprintf(p_char,"Name:%s",name);
	lib_display_ascii(p_char, strlen(p_char), &param);	
	param.x += 32;
	memset(p_char,0,sizeof(p_char));
	uint8_t *mac_f4b = (uint8_t *)&g_ble_info.mac_address_f4b;
	uint8_t *mac_l2b = (uint8_t *)&g_ble_info.mac_address_l2b;
	sprintf(p_char,"MAC:%02X:%02X:%02X:%02X:%02X:%02X",mac_l2b[1],mac_l2b[0],mac_f4b[3],mac_f4b[2],mac_f4b[1],mac_f4b[0]);
	lib_display_ascii(p_char, strlen(p_char), &param);	
}

static void ble_switch(void)
{
	_lib_display_param param = 
	{
		.x = 176+32+32,
		.y = 20,
		.height = 16,
		.gap = 0,
		.bg_color = BACKGROUND_COLOR,
		.fg_color = FOREGROUND_COLOR,
	};	
	char p_char[23];
	memset(p_char,0,sizeof(p_char));
	if(g_bluetooth_status.is_on)
	{
		switch(g_bluetooth_status.is_conn)
		{
			case BLE_CONNECT:{
				sprintf(p_char,"Ble : on & connect");
			}break;
			case BLE_DISCONNECT:{
				sprintf(p_char,"Ble : on & disconnect");
			}break;
			case BLE_BONDED:{
				sprintf(p_char,"Ble : on & bond");
			}break;			
		}

	}else{
	
		sprintf(p_char,"Ble : off");	
	}	
	
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

static void ble_switch_temp_paint(void)
{
	
	_lib_display_param param = 
	{
		.x = 176+32+32,
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
	if(m_ble_switch_temp)
	{
		switch(g_bluetooth_status.is_conn)
		{
			case BLE_CONNECT:{
				sprintf(p_char,"Ble : on & connect");
			}break;
			case BLE_DISCONNECT:{
				sprintf(p_char,"Ble : on & disconnect");
			}break;
			case BLE_BONDED:{
				sprintf(p_char,"Ble : on & bond");
			}break;			
		}

	}else{
	
		sprintf(p_char,"Ble : off");	
	}	
	
	lib_display_ascii(p_char, strlen(p_char), &param);	
	_drv_lcd_graph graph =
	{
		.x = 176+32+32,
		.y = 20,
		.height = 16*2,
	};	
	drv_lcd_set_one_frame(&graph);
}
void gui_ble_paint(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"gui_ble_paint\n");
	m_ble_switch_temp = g_bluetooth_status.is_on;
	memset(g_lcd_ram,BACKGROUND_COLOR,sizeof(g_lcd_ram));

	title();
	version();
	ble_switch();
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

void gui_ble_btn_evt(uint32_t evt)
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
			m_ble_switch_temp = !m_ble_switch_temp;
			ble_switch_temp_paint();
		
		}break;	
		case (BTN_PIN_UP<<1):{
			DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_UP long push\n");
		}break;				
		case (BTN_PIN_OK):{
			DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_OK release\n");
			if(m_ble_switch_temp)
			{
				DEBUG_PRINTF(MOUDLE_NAME"BLE_UART_CCOMMAND_APPLICATION_BLE_SYSTEM_ON\n");
				dev_ble_system_on();
			}
			else{
				DEBUG_PRINTF(MOUDLE_NAME"BLE_UART_CCOMMAND_APPLICATION_BLE_SYSTEM_OFF\n");
				dev_ble_system_off();
			}
			g_gui_index = GUI_INDEX_MENU;
			g_gui_index_save = g_gui_index;
			gui_menu_paint();	

			
		}break;
		case (BTN_PIN_OK<<1):{
			DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_OK long push\n");

		}break;		
		case (BTN_PIN_DOWN):{
			DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_DOWN release\n");
			m_ble_switch_temp = !m_ble_switch_temp;
			ble_switch_temp_paint();
		}break;
		
		case (BTN_PIN_DOWN<<1):{
			DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_DOWN long push\n");
	
		}break;					
	
	}



}





#endif




