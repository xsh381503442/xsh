#ifdef LAYER_APPLICATION 
#include "gui_hwt.h"
#include "rtt_log.h"
#include "lib_error.h"
#include "SEGGER_RTT.h"

#include "lib_display.h"
#include "lib_gfx.h"
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
#include "cmd_pc.h"
#include "drv_pc.h"
#include "dev_bat.h"

#define MOUDLE_LOG_ENABLED 							1
#define MOUDLE_DEBUG_ENABLED   					1
#define MOUDLE_NAME                     "[GUI_HWT]:"

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
extern uint8_t lib_display_color_bank[64];
extern _dev_bat_event   g_bat_evt;


_gui_hwt g_gui_hwt = 
{
	.button_pwr = 
	{
		.enable = false,
		.press_cnt = 0,
	},
	.button_bl = 
	{
		.enable = false,
		.press_cnt = 0,
	},
	.button_up = 
	{
		.enable = false,
		.press_cnt = 0,
	},
	.button_ok = 
	{
		.enable = false,
		.press_cnt = 0,
	},
	.button_dwn = 
	{
		.enable = false,
		.press_cnt = 0,
	},
	.lcd_enable = false,
	.backlight_enable = false,
	.motor_enable = false,		
	.buzzer_enable = false,	
		
	.extFlash = 
	{
		.enable = false,
		.type = 0,
		.address = 0,
		.size = 0,
	},
	.font_enable = false,	

	.g_sensor = 
	{
		.enable = false,
		.x = 0,
		.y = 0,
		.z = 0,
		.int1 = false,
		.int2 = false,
	},
	
	.gyro = 
	{
		.enable = false,
		.x = 0,
		.y = 0,
		.z = 0,
		.int1 = false,
		.int2 = false,
	},	
	
	.compass = 
	{
		.enable = false,
		.x = 0,
		.y = 0,
		.z = 0,
		.int1 = false,
		.int2 = false,
	},

	.hr = 
	{
		.enable = false,
		.value = 0,
	},
	
	.gps_enable = false,
	
	.ble = 
	{
		.enable = false,
		.read_info = false,
		.read_extFlash = false,
	},	
	
	
};


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
	char p_char[] = "HWT";	
	lib_display_ascii(p_char, strlen(p_char), &param);	
}

static void button(void)
{
	_lib_display_param param = 
	{
		.x = 16+32*5,
		.y = 38,
		.height = 16,
		.gap = 0,
		.bg_color = BACKGROUND_COLOR,
		.fg_color = FOREGROUND_COLOR,
	};	
	char p_char[23];
	if(g_gui_hwt.button_pwr.enable)
	{
		memset(p_char,0,sizeof(p_char));
		sprintf(p_char,"PWR BUTTON");	
		lib_display_ascii(p_char, strlen(p_char), &param);	
		param.x += 32*2;
		param.y += 14;
		memset(p_char,0,sizeof(p_char));
		sprintf(p_char,"%d",g_gui_hwt.button_pwr.press_cnt);		
		lib_display_ascii(p_char, strlen(p_char), &param);	
	}
	else if(g_gui_hwt.button_bl.enable)
	{
		memset(p_char,0,sizeof(p_char));
		sprintf(p_char,"BL BUTTON");	
		lib_display_ascii(p_char, strlen(p_char), &param);	
		param.x += 32*2;
		param.y += 14;
		memset(p_char,0,sizeof(p_char));
		sprintf(p_char,"%d",g_gui_hwt.button_bl.press_cnt);		
		lib_display_ascii(p_char, strlen(p_char), &param);	
	}
	else if(g_gui_hwt.button_up.enable)
	{
		memset(p_char,0,sizeof(p_char));
		sprintf(p_char,"UP BUTTON");	
		lib_display_ascii(p_char, strlen(p_char), &param);	
		param.x += 32*2;
		param.y += 14;
		memset(p_char,0,sizeof(p_char));
		sprintf(p_char,"%d",g_gui_hwt.button_up.press_cnt);		
		lib_display_ascii(p_char, strlen(p_char), &param);	
	}	
	else if(g_gui_hwt.button_ok.enable)
	{
		memset(p_char,0,sizeof(p_char));
		sprintf(p_char,"OK BUTTON");	
		lib_display_ascii(p_char, strlen(p_char), &param);	
		param.x += 32*2;
		param.y += 14;
		memset(p_char,0,sizeof(p_char));
		sprintf(p_char,"%d",g_gui_hwt.button_ok.press_cnt);		
		lib_display_ascii(p_char, strlen(p_char), &param);	
	}		
	else if(g_gui_hwt.button_dwn.enable)
	{
		memset(p_char,0,sizeof(p_char));
		sprintf(p_char,"DWN BUTTON");	
		lib_display_ascii(p_char, strlen(p_char), &param);	
		param.x += 32*2;
		param.y += 14;
		memset(p_char,0,sizeof(p_char));
		sprintf(p_char,"%d",g_gui_hwt.button_dwn.press_cnt);		
		lib_display_ascii(p_char, strlen(p_char), &param);	
	}		
}

static void lcd(void)
{
	if(g_gui_hwt.lcd_enable)
	{
		memset(g_lcd_ram,LIB_DISPLAY_COLOR_77,sizeof(g_lcd_ram));
	  lib_gfx_circle_t p_circle_1 = LIB_GFX_CIRCLE(120-2, 120-2, 10);
		lib_gfx_circle_draw(&p_circle_1,LIB_DISPLAY_COLOR_11,false);
	  lib_gfx_circle_t p_circle_2 = LIB_GFX_CIRCLE(120-2, 120-2, 20);
		lib_gfx_circle_draw(&p_circle_2,LIB_DISPLAY_COLOR_11,false);
	  lib_gfx_circle_t p_circle_3 = LIB_GFX_CIRCLE(120-2, 120-2, 30);
		lib_gfx_circle_draw(&p_circle_3,LIB_DISPLAY_COLOR_11,false);
	  lib_gfx_circle_t p_circle_4 = LIB_GFX_CIRCLE(120-2, 120-2, 40);
		lib_gfx_circle_draw(&p_circle_4,LIB_DISPLAY_COLOR_11,false);		
		lib_gfx_rect_t p_rect_1 = LIB_GFX_RECT(240-2-100+60, 120-2-50+30, 100-60, 100*2-60*2);
		lib_gfx_rect_draw(&p_rect_1,2,LIB_DISPLAY_COLOR_22,false);
		lib_gfx_rect_t p_rect_2 = LIB_GFX_RECT(240-2-100+40, 120-2-50+20, 100-40, 100*2-40*2);
		lib_gfx_rect_draw(&p_rect_2,2,LIB_DISPLAY_COLOR_22,false);
		lib_gfx_rect_t p_rect_3 = LIB_GFX_RECT(240-2-100+20, 120-2-50+10, 100-20, 100*2-20*2);
		lib_gfx_rect_draw(&p_rect_3,2,LIB_DISPLAY_COLOR_22,false);
		lib_gfx_rect_t p_rect_4 = LIB_GFX_RECT(240-2-100, 120-2-50, 100, 100*2);
		lib_gfx_rect_draw(&p_rect_4,2,LIB_DISPLAY_COLOR_22,false);
		lib_gfx_line_t p_line_1 = LIB_GFX_LINE(50, 120-2, 480-50, 120-2, 2);
		lib_gfx_line_draw(&p_line_1,LIB_DISPLAY_COLOR_44);		
		lib_gfx_line_t p_line_2 = LIB_GFX_LINE(240, 20, 240, 240-20, 2);
		lib_gfx_line_draw(&p_line_2,LIB_DISPLAY_COLOR_44);		
		lib_gfx_line_t p_line_3 = LIB_GFX_LINE(80, 40, 400, 200, 2);
		lib_gfx_line_draw(&p_line_3,LIB_DISPLAY_COLOR_44);
	}
}

static void backlight(void)
{
	if(g_gui_hwt.backlight_enable)
	{
		_lib_display_param param = 
		{
			.x = 16+32*5,
			.y = 38,
			.height = 16,
			.gap = 0,
			.bg_color = BACKGROUND_COLOR,
			.fg_color = FOREGROUND_COLOR,
		};	
		char p_char[23];	
		memset(p_char,0,sizeof(p_char));
		sprintf(p_char,"BACKLIGHT");	
		lib_display_ascii(p_char, strlen(p_char), &param);	
	}
}
static void motor(void)
{
	if(g_gui_hwt.motor_enable)
	{
		_lib_display_param param = 
		{
			.x = 16+32*5,
			.y = 38,
			.height = 16,
			.gap = 0,
			.bg_color = BACKGROUND_COLOR,
			.fg_color = FOREGROUND_COLOR,
		};	
		char p_char[23];	
		memset(p_char,0,sizeof(p_char));
		sprintf(p_char,"MOTOR");	
		lib_display_ascii(p_char, strlen(p_char), &param);	
	}
}
static void buzzer(void)
{
	if(g_gui_hwt.buzzer_enable)
	{
		_lib_display_param param = 
		{
			.x = 16+32*5,
			.y = 38,
			.height = 16,
			.gap = 0,
			.bg_color = BACKGROUND_COLOR,
			.fg_color = FOREGROUND_COLOR,
		};	
		char p_char[23];	
		memset(p_char,0,sizeof(p_char));
		sprintf(p_char,"BUZZER");	
		lib_display_ascii(p_char, strlen(p_char), &param);	
	}
}

static void extFlash(void)
{
	if(g_gui_hwt.extFlash.enable)
	{
		_lib_display_param param = 
		{
			.x = 16+32*5,
			.y = 38,
			.height = 16,
			.gap = 0,
			.bg_color = BACKGROUND_COLOR,
			.fg_color = FOREGROUND_COLOR,
		};	
		char p_char[23];	
		memset(p_char,0,sizeof(p_char));
		sprintf(p_char,"EXT_FLASH");	
		lib_display_ascii(p_char, strlen(p_char), &param);	
		if(g_gui_hwt.extFlash.type == HWT_EXTFLASH_TYPE_SECTOR)
		{
			param.x += 32*2;
			param.y = 20;
			memset(p_char,0,sizeof(p_char));
			sprintf(p_char,"SECTOR:0x%08X",g_gui_hwt.extFlash.address);		
			lib_display_ascii(p_char, strlen(p_char), &param);	
		}
			
	}

}

static void font(void)
{
	if(g_gui_hwt.font_enable)
	{
		_lib_display_param param = 
		{
			.x = 16+32*5,
			.y = 38,
			.height = 16,
			.gap = 0,
			.bg_color = BACKGROUND_COLOR,
			.fg_color = FOREGROUND_COLOR,
		};	
		char p_char[23];	
		memset(p_char,0,sizeof(p_char));
		sprintf(p_char,"FONT");	
		lib_display_ascii(p_char, strlen(p_char), &param);	
	}

}
static void g_sensor(void)
{
	if(g_gui_hwt.g_sensor.enable)
	{
		_lib_display_param param = 
		{
			.x = 16+32*5,
			.y = 38,
			.height = 16,
			.gap = 0,
			.bg_color = BACKGROUND_COLOR,
			.fg_color = FOREGROUND_COLOR,
		};	
		char p_char[23];	
		memset(p_char,0,sizeof(p_char));
		sprintf(p_char,"GSENSOR");	
		lib_display_ascii(p_char, strlen(p_char), &param);

		param.x += 32;	
		for(uint16_t i=0;i<param.height*2;i++)
		{
			memset(g_lcd_ram[param.x+i],BACKGROUND_COLOR,DRV_LCD_WIDTH/2);
		}	
		memset(p_char,0,sizeof(p_char));
		sprintf(p_char,"X:%d",g_gui_hwt.g_sensor.x);	
		lib_display_ascii(p_char, strlen(p_char), &param);	

		param.x += 32;	
		for(uint16_t i=0;i<param.height*2;i++)
		{
			memset(g_lcd_ram[param.x+i],BACKGROUND_COLOR,DRV_LCD_WIDTH/2);
		}	
		memset(p_char,0,sizeof(p_char));
		sprintf(p_char,"Y:%d",g_gui_hwt.g_sensor.y);	
		lib_display_ascii(p_char, strlen(p_char), &param);	
		
		param.x += 32;	
		for(uint16_t i=0;i<param.height*2;i++)
		{
			memset(g_lcd_ram[param.x+i],BACKGROUND_COLOR,DRV_LCD_WIDTH/2);
		}	
		memset(p_char,0,sizeof(p_char));
		sprintf(p_char,"Z:%d",g_gui_hwt.g_sensor.z);	
		lib_display_ascii(p_char, strlen(p_char), &param);	
	}

}
static void gyro(void)
{
	if(g_gui_hwt.gyro.enable)
	{
		_lib_display_param param = 
		{
			.x = 16+32*5,
			.y = 38,
			.height = 16,
			.gap = 0,
			.bg_color = BACKGROUND_COLOR,
			.fg_color = FOREGROUND_COLOR,
		};	
		char p_char[23];	
		memset(p_char,0,sizeof(p_char));
		sprintf(p_char,"GYRO");	
		lib_display_ascii(p_char, strlen(p_char), &param);

		param.x += 32;	
		for(uint16_t i=0;i<param.height*2;i++)
		{
			memset(g_lcd_ram[param.x+i],BACKGROUND_COLOR,DRV_LCD_WIDTH/2);
		}	
		memset(p_char,0,sizeof(p_char));
		sprintf(p_char,"X:%d",g_gui_hwt.gyro.x);	
		lib_display_ascii(p_char, strlen(p_char), &param);	

		param.x += 32;	
		for(uint16_t i=0;i<param.height*2;i++)
		{
			memset(g_lcd_ram[param.x+i],BACKGROUND_COLOR,DRV_LCD_WIDTH/2);
		}	
		memset(p_char,0,sizeof(p_char));
		sprintf(p_char,"Y:%d",g_gui_hwt.gyro.y);	
		lib_display_ascii(p_char, strlen(p_char), &param);	
		
		param.x += 32;	
		for(uint16_t i=0;i<param.height*2;i++)
		{
			memset(g_lcd_ram[param.x+i],BACKGROUND_COLOR,DRV_LCD_WIDTH/2);
		}	
		memset(p_char,0,sizeof(p_char));
		sprintf(p_char,"Z:%d",g_gui_hwt.gyro.z);	
		lib_display_ascii(p_char, strlen(p_char), &param);	
	}

}
static void compass(void)
{
	if(g_gui_hwt.compass.enable)
	{
		_lib_display_param param = 
		{
			.x = 16+32*5,
			.y = 38,
			.height = 16,
			.gap = 0,
			.bg_color = BACKGROUND_COLOR,
			.fg_color = FOREGROUND_COLOR,
		};	
		char p_char[23];	
		memset(p_char,0,sizeof(p_char));
		sprintf(p_char,"COMPASS");	
		lib_display_ascii(p_char, strlen(p_char), &param);

		param.x += 32;	
		for(uint16_t i=0;i<param.height*2;i++)
		{
			memset(g_lcd_ram[param.x+i],BACKGROUND_COLOR,DRV_LCD_WIDTH/2);
		}	
		memset(p_char,0,sizeof(p_char));
		sprintf(p_char,"X:%d",g_gui_hwt.compass.x);	
		lib_display_ascii(p_char, strlen(p_char), &param);	

		param.x += 32;	
		for(uint16_t i=0;i<param.height*2;i++)
		{
			memset(g_lcd_ram[param.x+i],BACKGROUND_COLOR,DRV_LCD_WIDTH/2);
		}	
		memset(p_char,0,sizeof(p_char));
		sprintf(p_char,"Y:%d",g_gui_hwt.compass.y);	
		lib_display_ascii(p_char, strlen(p_char), &param);	
		
		param.x += 32;	
		for(uint16_t i=0;i<param.height*2;i++)
		{
			memset(g_lcd_ram[param.x+i],BACKGROUND_COLOR,DRV_LCD_WIDTH/2);
		}	
		memset(p_char,0,sizeof(p_char));
		sprintf(p_char,"Z:%d",g_gui_hwt.compass.z);	
		lib_display_ascii(p_char, strlen(p_char), &param);	
	}

}
static void heartrate(void)
{
	if(g_gui_hwt.hr.enable)
	{
		_lib_display_param param = 
		{
			.x = 16+32*5,
			.y = 38,
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
		for(uint16_t i=0;i<param.height*2;i++)
		{
			memset(g_lcd_ram[param.x+i],BACKGROUND_COLOR,DRV_LCD_WIDTH/2);
		}	
		memset(p_char,0,sizeof(p_char));
		sprintf(p_char,"X:%d",g_gui_hwt.hr.value);	
		lib_display_ascii(p_char, strlen(p_char), &param);	
	}

}

static void gps(void)
{
	if(g_gui_hwt.gps_enable)
	{
		_lib_display_param param = 
		{
			.x = 16+32*5,
			.y = 38,
			.height = 16,
			.gap = 0,
			.bg_color = BACKGROUND_COLOR,
			.fg_color = FOREGROUND_COLOR,
		};	
		char p_char[23];	
		memset(p_char,0,sizeof(p_char));
		sprintf(p_char,"GPS");	
		lib_display_ascii(p_char, strlen(p_char), &param);
	}
}

static void ble(void)
{
	if(g_gui_hwt.ble.enable)
	{
		extern _dev_ble_info g_ble_info;
		_lib_display_param param = 
		{
			.x = 48+32*3,
			.y = 38,
			.height = 16,
			.gap = 0,
			.bg_color = BACKGROUND_COLOR,
			.fg_color = FOREGROUND_COLOR,
		};	
		char p_char[23];	
		memset(p_char,0,sizeof(p_char));
		sprintf(p_char,"BLE");	
		lib_display_ascii(p_char, strlen(p_char), &param);
		
		if(g_gui_hwt.ble.read_info)
		{
			uint8_t *version;
			param.x += 32;
			param.y = 20;
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
		
		if(g_gui_hwt.ble.read_extFlash)
		{
		
		}		
		
	
		
		
	}
}

static void battery(void)
{

	
	_lib_display_param param = 
	{
		.x = 48,
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

void gui_hwt_paint(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"gui_hwt_paint\n");
	memset(g_lcd_ram,BACKGROUND_COLOR,sizeof(g_lcd_ram));
	
	title();
	button();
	lcd();
	backlight();
	motor();
	buzzer();
	extFlash();
	font();
	g_sensor();
	gyro();
	compass();
	heartrate();
	gps();
	ble();
	battery();
	
	_drv_lcd_graph graph =
	{
			.x = 0,
			.y = 0,
			.height = DRV_LCD_HIGHT*2,
			.width = DRV_LCD_WIDTH/2,
	};
	drv_lcd_set_one_frame(&graph);
}



void gui_hwt_btn_evt(uint32_t evt)
{
	
	switch(evt)
	{		
		case (BTN_PIN_PWR):{
			DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_PWR release\n");
			if(g_gui_hwt.button_pwr.enable)
			{
				g_gui_hwt.button_pwr.press_cnt++;
				gui_hwt_paint();
				uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_BUTTON_PWR_PRESS | PC_UART_COMMAND_MASK_HARDWARE_TEST};
				drv_pc_send_data((uint8_t *)command, sizeof(command));				
			}

			
		}break;
		case (BTN_PIN_PWR<<1):{
			DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_PWR long push\n");

		}break;		
		case (BTN_PIN_BL):{
			DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_BL release\n");
			if(g_gui_hwt.button_bl.enable)
			{
				g_gui_hwt.button_bl.press_cnt++;
				gui_hwt_paint();
				uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_BUTTON_BL_PRESS | PC_UART_COMMAND_MASK_HARDWARE_TEST};
				drv_pc_send_data((uint8_t *)command, sizeof(command));				
			}				
		}break;
		case (BTN_PIN_BL<<1):{
			DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_BL long push\n");
	
			
			
		}break;									
		case (BTN_PIN_UP):{
			DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_UP release\n");
			if(g_gui_hwt.button_up.enable)
			{
				g_gui_hwt.button_up.press_cnt++;
				gui_hwt_paint();
				uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_BUTTON_UP_PRESS | PC_UART_COMMAND_MASK_HARDWARE_TEST};
				drv_pc_send_data((uint8_t *)command, sizeof(command));				
			}	
		
		}break;	
		case (BTN_PIN_UP<<1):{
			DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_UP long push\n");
		}break;				
		case (BTN_PIN_OK):{
			DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_OK release\n");
			if(g_gui_hwt.button_ok.enable)
			{
				g_gui_hwt.button_ok.press_cnt++;
				gui_hwt_paint();
				uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_BUTTON_OK_PRESS | PC_UART_COMMAND_MASK_HARDWARE_TEST};
				drv_pc_send_data((uint8_t *)command, sizeof(command));				
			}	
			
		}break;
		case (BTN_PIN_OK<<1):{
			DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_OK long push\n");

		}break;		
		case (BTN_PIN_DOWN):{
			DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_DOWN release\n");
			if(g_gui_hwt.button_dwn.enable)
			{
				g_gui_hwt.button_dwn.press_cnt++;
				gui_hwt_paint();
				uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_BUTTON_DWN_PRESS | PC_UART_COMMAND_MASK_HARDWARE_TEST};
				drv_pc_send_data((uint8_t *)command, sizeof(command));				
			}	
		}break;
		
		case (BTN_PIN_DOWN<<1):{
			DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_DOWN long push\n");
	
		}break;					
	
	}

	

}




#endif

