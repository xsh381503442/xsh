#include "gui_hwt.h"
#include <string.h>
#include <stdio.h>
#include "am_util.h"
#include "drv_pc.h"
#include "cmd_pc.h"
#include "drv_lcd.h"
#include "task_ble.h"
#include "drv_battery.h"
#include "drv_key.h"
#include "lib_app_data.h"


extern _drv_bat_event   g_bat_evt;
#define BACKGROUND_COLOR      LCD_WHITE
#define FOREGROUND_COLOR      LCD_BLACK

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
#ifdef WATCH_CHIP_TEST
	.hardware_test_enable = false,
#endif
};




void gui_hwt_paint(void)
{
	LCD_SetBackgroundColor(LCD_BLACK);
	
	SetWord_t word = {0};
	word.x_axis = 26;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_32_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	word.kerning = 0;
	LCD_SetString("Ó²¼þ²âÊÔ",&word);	
	
	word.x_axis += 40;
	
	if(g_gui_hwt.lcd_enable)
	{
		LCD_SetString("ÆÁÍ¨¹ý",&word);	
	}
	
	
	LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
}



void gui_hwt_btn_evt(uint32_t evt)
{
	
	switch(evt)
	{		
		case (KEY_BACK):{
			if(g_gui_hwt.button_pwr.enable)
			{
				g_gui_hwt.button_pwr.press_cnt++;
				gui_hwt_paint();
				uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_BUTTON_PWR_PRESS | PC_UART_COMMAND_MASK_HARDWARE_TEST};
				drv_pc_send_data((uint8_t *)command, sizeof(command));				
			}

			
		}break;
		case (KEY_BACK<<1):{

		}break;		
		case (KEY_LIGHT):{
			if(g_gui_hwt.button_bl.enable)
			{
				g_gui_hwt.button_bl.press_cnt++;
				gui_hwt_paint();
				uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_BUTTON_BL_PRESS | PC_UART_COMMAND_MASK_HARDWARE_TEST};
				drv_pc_send_data((uint8_t *)command, sizeof(command));				
			}				
		}break;
		case (KEY_LIGHT<<1):{
	
			
			
		}break;									
		case (KEY_UP):{
			if(g_gui_hwt.button_up.enable)
			{
				g_gui_hwt.button_up.press_cnt++;
				gui_hwt_paint();
				uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_BUTTON_UP_PRESS | PC_UART_COMMAND_MASK_HARDWARE_TEST};
				drv_pc_send_data((uint8_t *)command, sizeof(command));				
			}	
		
		}break;	
		case (KEY_UP<<1):{
		}break;				
		case (KEY_OK):{
			if(g_gui_hwt.button_ok.enable)
			{
				g_gui_hwt.button_ok.press_cnt++;
				gui_hwt_paint();
				uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_BUTTON_OK_PRESS | PC_UART_COMMAND_MASK_HARDWARE_TEST};
				drv_pc_send_data((uint8_t *)command, sizeof(command));				
			}	
			
		}break;
		case (KEY_OK<<1):{

		}break;		
		case (KEY_DOWN):{
			if(g_gui_hwt.button_dwn.enable)
			{
				g_gui_hwt.button_dwn.press_cnt++;
				gui_hwt_paint();
				uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_BUTTON_DWN_PRESS | PC_UART_COMMAND_MASK_HARDWARE_TEST};
				drv_pc_send_data((uint8_t *)command, sizeof(command));				
			}	
		}break;
		
		case (KEY_DOWN<<1):{
	
		}break;					
	
	}

	

}



