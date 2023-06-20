#include "gui_watch_info.h"
#include "SEGGER_RTT.h"
#include "drv_key.h"
#include "drv_lcd.h"
#include <string.h>
#include "font_displaystrs.h"
#include "task_display.h"
#include "com_data.h"
#include "task_ble.h"
#include "lib_boot_setting.h"
#include "lib_boot.h"
#include "img_tool.h"
#include "lib_app_data.h"
#include "com_sport.h"
#include "drv_extflash.h"
#include "cmd_ble.h"
#include "lib_error.h"
#include "drv_ble.h"
#include "task_ble.h"
#include "img_menu.h"

#define GUI_WATCH_INFO_LOG_ENABLED 1

#if DEBUG_ENABLED == 1 && GUI_WATCH_INFO_LOG_ENABLED == 1
	#define GUI_WATCH_INFO_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_WATCH_INFO_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define GUI_WATCH_INFO_WRITESTRING(...)
	#define GUI_WATCH_INFO_PRINTF(...)		        
#endif

//#define DELAY_MS(X)                 am_util_delay_ms(X)
#define DELAY_MS(X)                 vTaskDelay(X / portTICK_PERIOD_MS)
//#define DELAY_MS(X)                   do{TickType_t xLastWakeTime;xLastWakeTime = xTaskGetTickCount ();vTaskDelayUntil( &xLastWakeTime, X / portTICK_PERIOD_MS );}while(0)


extern SetValueStr SetValue;
#define BACKGROUND_COLOR      LCD_WHITE
#define FOREGROUND_COLOR      LCD_BLACK

extern uint8_t rtc_hw_flag;


#define GUI_WATCH_INFO_PAGE_ONE        0
#define GUI_WATCH_INFO_PAGE_TWO        1
static uint8_t m_gui_watch_info_index = 0;



static void watch_info_page_one(_boot_setting *boot_setting,_task_ble_info *task_ble_info)
{
	SetWord_t word = {0};
	//word.x_axis = 105;
	
	word.x_axis = 32;
	word.y_axis = 40;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;	
	char p_char[20];
	uint8_t *version;
	
	if(SetValue.Language == L_CHINESE)
	{
		LCD_SetString("型号    : ",&word);						
	}
	else if(SetValue.Language == L_ENGLISH)
	{
		LCD_SetString("MODEL:",&word);								
	}			
	
	memset(p_char,0,sizeof(p_char));
	memcpy(p_char,boot_setting->model,sizeof(boot_setting->model));
	word.y_axis = 120;
	#ifdef COD
	LCD_SetString("K2",&word);
	
	#else
	LCD_SetString(p_char,&word);
	#endif
	
	word.x_axis += 20;
	word.y_axis = 40;
	if(SetValue.Language == L_CHINESE)
	{
		LCD_SetString("版本号  : ",&word);						
	}
	else if(SetValue.Language == L_ENGLISH)
	{
		LCD_SetString("VERSION:",&word);								
	}

	memset(p_char,0,sizeof(p_char));
	version = (uint8_t *)&boot_setting->application_version;
	sprintf(p_char,"V%d.%d.%dB%d",version[3],version[2],version[1],version[0]);
	word.y_axis = 120;	
	LCD_SetString(p_char,&word);	
	
	word.x_axis += 20;
	word.y_axis = 40;
	if(SetValue.Language == L_CHINESE)
	{
		LCD_SetString("设备名称: ",&word);						
	}
	else if(SetValue.Language == L_ENGLISH)
	{
		LCD_SetString("NAME:",&word);								
	}	
	
	memset(p_char,0,sizeof(p_char));
	memcpy(p_char,task_ble_info->dev_name.name,task_ble_info->dev_name.len);
	word.y_axis = 120;
	#ifdef COD 
	LCD_SetString("COD_WATCH_K2",&word);
	
	#else
	LCD_SetString(p_char,&word);
	#endif
	
	word.x_axis += 20;
	word.y_axis = 40;
	if(SetValue.Language == L_CHINESE)
	{
		LCD_SetString("蓝牙地址: ",&word);						
	}
	else if(SetValue.Language == L_ENGLISH)
	{
		LCD_SetString("MAC:",&word);								
	}	
	
	memset(p_char,0,sizeof(p_char));
	uint8_t *mac_f4b = (uint8_t *)&task_ble_info->mac_address_f4b;
	uint8_t *mac_l2b = (uint8_t *)&task_ble_info->mac_address_l2b;
	sprintf(p_char,"%02X:%02X:%02X:%02X:%02X:%02X",mac_l2b[1],mac_l2b[0],mac_f4b[3],mac_f4b[2],mac_f4b[1],mac_f4b[0]);
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.x_axis += 20;
	LCD_SetString(p_char,&word);		
	
	//GUI_WATCH_INFO_PRINTF("V=%d\r\n",word.x_axis);

	//word.x_axis = +20;
	word.x_axis = 132;
	//GUI_WATCH_INFO_PRINTF("V=%d\r\n",word.x_axis);
	word.y_axis = 40;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;	
	if(SetValue.Language == L_CHINESE)
	{
		LCD_SetString("蓝牙版本: ",&word);						
	}
	else if(SetValue.Language == L_ENGLISH)
	{
		LCD_SetString("BLE APP:",&word);								
	}			
	
	memset(p_char,0,sizeof(p_char));
	version = (uint8_t *)&task_ble_info->application_version;
	sprintf(p_char,"V%d.%d.%dB%d",version[3],version[2],version[1],version[0]);
	word.y_axis = 120;
	LCD_SetString(p_char,&word);
	
	word.x_axis += 20;
	word.y_axis = 40;	
	if(SetValue.Language == L_CHINESE)
	{
		LCD_SetString("硬件版本: ",&word);						
	}
	else if(SetValue.Language == L_ENGLISH)
	{
		LCD_SetString("PCB:",&word);								
	}			
	
	memset(p_char,0,sizeof(p_char));
	
	version = (uint8_t *)&boot_setting->pcb_version;
	if (rtc_hw_flag == 1)
	{
		version[1] = version[1] | 0x01;
	}
	
	sprintf(p_char,"V%d.%d.%dB%d",version[3],version[2],version[1],version[0]);
	word.y_axis = 120;
	LCD_SetString(p_char,&word);	
	
	word.x_axis += 20;
	word.y_axis = 40;	
	if(SetValue.Language == L_CHINESE)
	{
		LCD_SetString("主引导  : ",&word);						
	}
	else if(SetValue.Language == L_ENGLISH)
	{
		LCD_SetString("APOLLO BOOT:",&word);								
	}			
	
	memset(p_char,0,sizeof(p_char));
	version = (uint8_t *)&boot_setting->bootloader_version;
	sprintf(p_char,"V%d.%d.%dB%d",version[3],version[2],version[1],version[0]);
	word.y_axis = 120;
	LCD_SetString(p_char,&word);		
	word.x_axis += 20;
	word.y_axis = 40;	
	if(SetValue.Language == L_CHINESE)
	{
		LCD_SetString("蓝牙引导: ",&word);						
	}
	else if(SetValue.Language == L_ENGLISH)
	{
		LCD_SetString("BLE BOOT:",&word);								
	}			
	
	memset(p_char,0,sizeof(p_char));
	version = (uint8_t *)&task_ble_info->bootloader_version;
	sprintf(p_char,"V%d.%d.%dB%d",version[3],version[2],version[1],version[0]);
	word.y_axis = 120;
	LCD_SetString(p_char,&word);			
	
	
}
static void watch_info_page_two(_boot_setting *boot_setting, _task_ble_info *task_ble_info)
{
	LCD_SetPicture(90, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Up_12X8);
	
	SetWord_t word = {0};
	word.x_axis = 105;
	word.y_axis = 40;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;	
	char p_char[20];
	uint8_t *version;
#ifndef WATCH_HAS_NO_BLE 
	if(SetValue.Language == L_CHINESE)
	{
		LCD_SetString("蓝牙版本: ",&word);						
	}
	else if(SetValue.Language == L_ENGLISH)
	{
		LCD_SetString("BLE APP:",&word);								
	}			
	
	memset(p_char,0,sizeof(p_char));
	version = (uint8_t *)&task_ble_info->application_version;
	sprintf(p_char,"V%d.%d.%dB%d",version[3],version[2],version[1],version[0]);
	word.y_axis = 120;
	LCD_SetString(p_char,&word);
#endif
	
	word.x_axis += 20;
	word.y_axis = 40;	
	if(SetValue.Language == L_CHINESE)
	{
		LCD_SetString("硬件版本: ",&word);						
	}
	else if(SetValue.Language == L_ENGLISH)
	{
		LCD_SetString("PCB:",&word);								
	}			
	
	memset(p_char,0,sizeof(p_char));
	version = (uint8_t *)&boot_setting->pcb_version;
	if (rtc_hw_flag == 1)
	{
		version[1] = version[1] | 0x01;
	}
	sprintf(p_char,"V%d.%d.%dB%d",version[3],version[2],version[1],version[0]);
	word.y_axis = 120;
	LCD_SetString(p_char,&word);	
	
	
	word.x_axis += 20;
	word.y_axis = 40;	
	if(SetValue.Language == L_CHINESE)
	{
		LCD_SetString("主引导  : ",&word);						
	}
	else if(SetValue.Language == L_ENGLISH)
	{
		LCD_SetString("APOLLO BOOT:",&word);								
	}			
	
	memset(p_char,0,sizeof(p_char));
	version = (uint8_t *)&boot_setting->bootloader_version;
	sprintf(p_char,"V%d.%d.%dB%d",version[3],version[2],version[1],version[0]);
	word.y_axis = 120;
	LCD_SetString(p_char,&word);		
#ifndef WATCH_HAS_NO_BLE 
	word.x_axis += 20;
	word.y_axis = 40;	
	if(SetValue.Language == L_CHINESE)
	{
		LCD_SetString("蓝牙引导: ",&word);						
	}
	else if(SetValue.Language == L_ENGLISH)
	{
		LCD_SetString("BLE BOOT:",&word);								
	}			
	
	memset(p_char,0,sizeof(p_char));
	version = (uint8_t *)&task_ble_info->bootloader_version;
	sprintf(p_char,"V%d.%d.%dB%d",version[3],version[2],version[1],version[0]);
	word.y_axis = 120;
	LCD_SetString(p_char,&word);			
#endif	
	

	
	//LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, FOREGROUND_COLOR, BACKGROUND_COLOR, &Img_Pointing_Down_12X8);
}

//gui_watch_info_pain会taskdisplay溢出，封装下读task_ble_info
void task_ble_info_set(_task_ble_info *task_ble_info)
{
	_lib_app_data lib_app_data;
	lib_app_data_read(&lib_app_data);	
	task_ble_info->setting_version = lib_app_data.ble.setting_version;
	task_ble_info->bootloader_version = lib_app_data.ble.bootloader_version;
	task_ble_info->application_version = lib_app_data.ble.application_version;
	task_ble_info->softdevice_version = lib_app_data.ble.softdevice_version;
	task_ble_info->pcb_version = lib_app_data.ble.pcb_version;
	task_ble_info->mac_address_f4b = lib_app_data.ble.mac_address_f4b;
	task_ble_info->mac_address_l2b = lib_app_data.ble.mac_address_l2b;
	memset(task_ble_info->dev_name.name,0,sizeof(task_ble_info->dev_name.name));
	memcpy(task_ble_info->dev_name.name,lib_app_data.ble.name,lib_app_data.ble.name_len);		
	task_ble_info->dev_name.len = lib_app_data.ble.name_len;	

}






void gui_watch_info_paint(void)
{	

	_task_ble_info task_ble_info;
    task_ble_info_set(&task_ble_info); 
	
	
	LCD_SetBackgroundColor(LCD_BLACK);
	SetWord_t word = {0};
	word.x_axis = 32;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_CYAN;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;
	
	_boot_setting boot_setting;
	lib_boot_setting_read(&boot_setting);	
	
/*	if(SetValue.Language == L_CHINESE)
	{

		LCD_SetString("关于手表",&word);	
		
	}
	else if(SetValue.Language == L_ENGLISH)
	{
		LCD_SetString("WATCH INFO",&word);		
    }

	LCD_SetPicture(word.x_axis,120 - 4*LCD_FONT_24_SIZE/2 - 8 - img_set_menu_b[6].width,LCD_RED,LCD_NONE,&img_set_menu_b[6]);//关于手表图标
		
	LCD_SetRectangle(120 - 24/2 - 36 - 1 ,2,LCD_LINE_CNT_MAX/2 - 60,120,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);*/

	
	word.size = LCD_FONT_16_SIZE;
    
	watch_info_page_one(&boot_setting,&task_ble_info);
	/*switch(m_gui_watch_info_index)
	{
		case GUI_WATCH_INFO_PAGE_ONE:{
			watch_info_page_one(&boot_setting,&task_ble_info);
		}break;
		case GUI_WATCH_INFO_PAGE_TWO:{
		//	watch_info_page_two(&boot_setting,&task_ble_info);
		}break;

	}	*/

}



void gui_watch_info_btn_evt(uint32_t evt)
{
	m_gui_watch_info_index = 0;
	DISPLAY_MSG  msg = {0,0};
	switch(evt)
	{		
		case (KEY_BACK):{
			ScreenState = DISPLAY_SCREEN_MENU;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
		}break;			
		/*case (KEY_UP):{
			if(m_gui_watch_info_index > 0)
			{
				m_gui_watch_info_index--;
			}
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_WATCH_INFO;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}break;	*/
			
		case (KEY_OK):{

			
//			uint32_t command[1] = {BLE_UART_CCOMMAND_APPLICATION_SOS_GPS | BLE_UART_CCOMMAND_MASK_APPLICATION};
//			_uart_data_param param;
//			param.user_callback = NULL;
//			param.p_data = (uint8_t *)command;
//			param.length = sizeof(command);
//			uint8_t count = TASK_UART_DELAY_COUNT_OUT;
//			while(drv_ble_send_data(&param) != ERR_SUCCESS && count--)
//			{
//				DELAY_MS(TASK_UART_DELAY);
//			}		

			
		}break;		
		/*case (KEY_DOWN):{
			if(m_gui_watch_info_index < 1)
			{
				m_gui_watch_info_index++;
			}
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_WATCH_INFO;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}break;	*/				
	   default:
	   break;
	   
	}
}














