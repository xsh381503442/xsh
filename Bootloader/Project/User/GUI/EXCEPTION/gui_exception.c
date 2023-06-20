#ifdef LAYER_BOOTLOADER 
#include "gui_exception.h"
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



extern ScreenState_t   ScreenState;
extern QueueHandle_t 	DisplayQueue;
extern ScreenState_exception g_screen_state_exception_type;

void gui_exception_paint(void)
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
	sprintf(p_char,"exception");
	word.y_axis = LCD_CENTER_JUSTIFIED;
	LCD_SetString(p_char,&word);
	
	word.x_axis += 16;
	memset(p_char,0,sizeof(p_char));
	switch(g_screen_state_exception_type)
	{
	
		case SCREEN_STATE_EXCEPTION_TYPE_NONE:
		{
			sprintf(p_char,"fine");
		}break;
		case SCREEN_STATE_EXCEPTION_TYPE_APPLICATION_START_ADDRESS_ERROR:
		{
			sprintf(p_char,"error:app addr");
		}break;
		case SCREEN_STATE_EXCEPTION_TYPE_APPLICATION_CRC_ERROR:
		{
			sprintf(p_char,"error:app crc");
		}break;
		case SCREEN_STATE_EXCEPTION_TYPE_UPDATE_ALGORITHM_START_ADDRESS_ERROR:
		{
			sprintf(p_char,"error:update addr");
		}break;
		case SCREEN_STATE_EXCEPTION_TYPE_UPDATE_ALGORITHM_CRC_ERROR:
		{
			sprintf(p_char,"error:update crc");
		}break;
		case SCREEN_STATE_EXCEPTION_TYPE_BOOTSETTING_CRC_ERROR:
		{
			sprintf(p_char,"error:bootsetting crc");
		}break;		
		
		
		default:
			break;
	}
	
	word.y_axis = LCD_CENTER_JUSTIFIED;
	LCD_SetString(p_char,&word);	
	
	
}



void gui_exception_btn_evt(uint32_t evt)
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


void gui_exception_task_display(void)
{
	if(ScreenState == DISPLAY_SCREEN_EXCEPTION)
	{
		DISPLAY_MSG  msg = {0,0};
		msg.cmd = MSG_DISPLAY_SCREEN;
		xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
	}

}


#endif



