#ifdef LAYER_APPLICATION 
#include "gui_home.h"
#include "rtt_log.h"
#include "lib_error.h"
#include "SEGGER_RTT.h"

#include "lib_display.h"
#include <string.h>
#include <stdio.h>

#include "lib_boot_setting.h"
#include "drv_lcd.h"
#include "dev_bat.h"
#include "task_ble.h"
#include "drv_button.h"
#include "gui_util.h"
#include "gui_menu.h"
#include "lib_gfx.h"
#include "dev_extFlash.h"
#include "dev_font.h"


#define MOUDLE_LOG_ENABLED 							1
#define MOUDLE_DEBUG_ENABLED   					1
#define MOUDLE_NAME                     "[GUI_HOME]:"

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
extern am_hal_rtc_time_t g_rtc_time;
extern uint8_t g_lcd_ram[DRV_LCD_HIGHT*2][DRV_LCD_WIDTH/2];

extern _dev_bat_event   g_bat_evt;

extern _bluetooth_status g_bluetooth_status;

extern uint32_t g_gui_index;
extern uint32_t g_gui_index_save;


#define BACKGROUND_COLOR      LIB_DISPLAY_COLOR_00
#define FOREGROUND_COLOR      LIB_DISPLAY_COLOR_77

static void title(void)
{
	_lib_display_param param = 
	{
		.x = 16,
		.y = 35,
		.height = 16,
		.gap = 0,
		.bg_color = BACKGROUND_COLOR,
		.fg_color = FOREGROUND_COLOR,	
	};
	char p_char[] = "Application";	
	lib_display_ascii(p_char, strlen(p_char), &param);	
}

static void version(void)
{
	_boot_setting boot_setting;
	lib_boot_setting_read(&boot_setting);	
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
	version = (uint8_t *)&boot_setting.application_version;
	sprintf(p_char,"A:V%d.%d.%dB%d",version[3],version[2],version[1],version[0]);
	lib_display_ascii(p_char, strlen(p_char), &param);	
	param.x += 32;
	memset(p_char,0,sizeof(p_char));
	version = (uint8_t *)&boot_setting.bootloader_version;
	sprintf(p_char,"B:V%d.%d.%dB%d",version[3],version[2],version[1],version[0]);
	lib_display_ascii(p_char, strlen(p_char), &param);
	param.x += 32;
	memset(p_char,0,sizeof(p_char));
	version = (uint8_t *)&boot_setting.update_version;
	sprintf(p_char,"U:V%d.%d.%dB%d",version[3],version[2],version[1],version[0]);
	lib_display_ascii(p_char, strlen(p_char), &param);
	param.x += 32;
	memset(p_char,0,sizeof(p_char));
	version = (uint8_t *)&boot_setting.pcb_version;
	sprintf(p_char,"PCB:V%d.%d.%dB%d",version[3],version[2],version[1],version[0]);
	lib_display_ascii(p_char, strlen(p_char), &param);
	
}
static void battery(void)
{

	
	_lib_display_param param = 
	{
		.x = 176,
		.y = 20,
		.height = 16,
		.gap = 0,
		.bg_color = BACKGROUND_COLOR,
		.fg_color = FOREGROUND_COLOR,	
	};	
	char p_char[23];
	memset(p_char,0,sizeof(p_char));
		
	switch(g_bat_evt.type)
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

	}	

	switch(g_bat_evt.cable_status)
	{
		case DEV_BAT_EVENT_USB_CONN_CHARGING:{
			sprintf(p_char,"Bat : charging");
		}break;		
		
		
		case DEV_BAT_EVENT_USB_CONN_CHARGING_FINISHED:{
			sprintf(p_char,"Bat : charging full");
		}break;		
		
		case DEV_BAT_EVENT_USB_DISCONN:{
			sprintf(p_char,"Chg : disconnect");
		}break;	
	
	}
	
	
	lib_display_ascii(p_char, strlen(p_char), &param);	
	param.x += 32;
	memset(p_char,0,sizeof(p_char));
	sprintf(p_char,"Bat : %dmv",g_bat_evt.voltage_mv);	
	lib_display_ascii(p_char, strlen(p_char), &param);	
	param.x += 32;
	memset(p_char,0,sizeof(p_char));
	sprintf(p_char,"Bat : %d percent",g_bat_evt.level_percent);	
	lib_display_ascii(p_char, strlen(p_char), &param);	
	
}
static void bluetooth(void)
{
	_lib_display_param param = 
	{
		.x = 272,
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


static void chip_id(void)
{
	_lib_display_param param = 
	{
		.x = 272+32,
		.y = 20,
		.height = 16,
		.gap = 0,
		.bg_color = BACKGROUND_COLOR,
		.fg_color = FOREGROUND_COLOR,	
	};
	char p_char[23];
	memset(p_char,0,sizeof(p_char));
	sprintf(p_char,"ID:%08X%08X",AM_REG(MCUCTRL, CHIPID0),AM_REG(MCUCTRL, CHIPID1));
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
void gui_home_paint(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"gui_home_paint\n");
	memset(g_lcd_ram,BACKGROUND_COLOR,sizeof(g_lcd_ram));
	title();
	version();
	battery();
	bluetooth();
	chip_id();
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


void gui_home_btn_evt(uint32_t evt)
{
	switch(evt)
	{		
		case (BTN_PIN_PWR):{
			DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_PWR release\n");

			
		}break;
		case (BTN_PIN_PWR<<1):{
			DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_PWR long push\n");
			
		}break;		
		case (BTN_PIN_BL):{
			DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_BL release\n");
				
			uint8_t buffer[256];		
			dev_extFlash_read(0, buffer, sizeof(buffer));	
			DEBUG_HEXDUMP(buffer, 32);
			dev_extFlash_erase(0, 128);
			dev_extFlash_read(0, buffer, sizeof(buffer));	
			DEBUG_HEXDUMP(buffer, 32);		
			for(uint32_t i=0;i<sizeof(buffer);i++)
			{
				buffer[i] = i;
			}				
			dev_extFlash_write(0, buffer, sizeof(buffer));	
			dev_extFlash_read(0, buffer, sizeof(buffer));	
			DEBUG_HEXDUMP(buffer, 32);					
			
			
			
			
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
			g_gui_index = GUI_INDEX_MENU;
			g_gui_index_save = g_gui_index;
			gui_menu_paint();
			
			
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

