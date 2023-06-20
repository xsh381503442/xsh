#ifdef LAYER_BOOTLOADER 
#include "gui_post_watch_info.h"
#include "drv_button.h"
#include "task_display.h"


#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "portmacro.h"
#include "portable.h"
#include "event_groups.h"
#include "semphr.h"

#include "drv_lcd.h"
#include <string.h>
#include <stdio.h>
#include "dev_ble.h"


#define YEAR ((((__DATE__ [7] - '0') * 10 + (__DATE__ [8] - '0')) * 10  + (__DATE__ [9] - '0')) * 10 + (__DATE__ [10] - '0'))  
  
#define MONTH (__DATE__ [2] == 'n' ? (__DATE__ [1] == 'a' ? 0 : 5)  : __DATE__ [2] == 'b' ? 1  : __DATE__ [2] == 'r' ? (__DATE__ [0] == 'M' ? 2 : 3)  : __DATE__ [2] == 'y' ? 4  : __DATE__ [2] == 'n' ? 5  : __DATE__ [2] == 'l' ? 6 : __DATE__ [2] == 'g' ? 7 : __DATE__ [2] == 'p' ? 8 : __DATE__ [2] == 't' ? 9 : __DATE__ [2] == 'v' ? 10 : 11)  
  
#define DAY ((__DATE__ [4] == ' ' ? 0 : __DATE__ [4] - '0') * 10+ (__DATE__ [5] - '0'))  
  
#define DATE_AS_INT (((YEAR - 2000) * 12 + MONTH) * 31 + DAY)  

#define HOUR ((__TIME__ [0] == ' ' ? 0 : __TIME__ [0] - '0') * 10+ (__TIME__ [1] - '0')) 
#define MINUTE ((__TIME__ [3] == ' ' ? 0 : __TIME__ [3] - '0') * 10+ (__TIME__ [4] - '0'))
#define SECOND ((__TIME__ [6] == ' ' ? 0 : __TIME__ [6] - '0') * 10+ (__TIME__ [7] - '0'))



extern ScreenState_t   ScreenState;
extern QueueHandle_t 	DisplayQueue;
extern _dev_ble_info g_ble_info;

void gui_post_watch_info_paint(void)
{

	LCD_SetBackgroundColor(LCD_WHITE);
	SetWord_t word = {0};
	word.x_axis = 15;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_WHITE;
	word.kerning = 0;	
	char p_char[20];
	
	
	memset(p_char,0,sizeof(p_char));
	sprintf(p_char,"build time");
	word.y_axis = LCD_CENTER_JUSTIFIED;
	LCD_SetString(p_char,&word);		
	
	word.x_axis += 20;
	memset(p_char,0,sizeof(p_char));
	sprintf(p_char,"%d-%02d-%02d %02d:%02d:%02d",YEAR, MONTH + 1, DAY,HOUR,MINUTE,SECOND);
	word.y_axis = LCD_CENTER_JUSTIFIED;
	LCD_SetString(p_char,&word);			

	{
		uint16_t year = (uint16_t)(g_ble_info.build_date>>16);
		uint8_t month = (uint8_t)(g_ble_info.build_date>>8);
		uint8_t day = (uint8_t)(g_ble_info.build_date>>0);
		uint8_t hour = (uint8_t)(g_ble_info.build_time>>16);	
		uint8_t minute = (uint8_t)(g_ble_info.build_time>>8);
		uint8_t second = (uint8_t)(g_ble_info.build_time>>0);
		
		if(year != 0)
		{
			word.x_axis += 20;
			memset(p_char,0,sizeof(p_char));
			sprintf(p_char,"ble build time");
			LCD_SetString(p_char,&word);			
			
			word.x_axis += 20;
			memset(p_char,0,sizeof(p_char));
			sprintf(p_char,"%d-%02d-%02d %02d:%02d:%02d",year, month + 1, day,hour,minute,second);
			LCD_SetString(p_char,&word);				
		}
	}
		
	
	
	
}



void gui_post_watch_info_btn_evt(uint32_t evt)
{
	switch(evt)
	{		
		case (BTN_PIN_PWR):{
			ScreenState = DISPLAY_SCREEN_BOOTLOADER;
			DISPLAY_MSG  msg = {0,0};
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
		}break;			
		
	
	}

}

#endif



