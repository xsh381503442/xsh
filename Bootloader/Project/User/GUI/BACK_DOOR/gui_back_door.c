#ifdef LAYER_BOOTLOADER 
#include "gui_back_door.h"
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

#include "drv_ble.h"
#include "com_apollo2.h"
#include "cmd_pc.h"
#include "dev_ble.h"
#include "task_ble.h"
#include "lib_error.h"


//#define DELAY_MS(X)                 am_util_delay_ms(X)
#define DELAY_MS(X)                 vTaskDelay(X / portTICK_PERIOD_MS)
//#define DELAY_MS(X)                   do{TickType_t xLastWakeTime;xLastWakeTime = xTaskGetTickCount ();vTaskDelayUntil( &xLastWakeTime, X / portTICK_PERIOD_MS );}while(0)


extern ScreenState_t   ScreenState;
extern QueueHandle_t 	DisplayQueue;


void gui_back_door_paint(void)
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
	sprintf(p_char,"back_door");
	word.y_axis = LCD_CENTER_JUSTIFIED;
	LCD_SetString(p_char,&word);
	
}



void gui_back_door_btn_evt(uint32_t evt)
{
	switch(evt)
	{		
		case (BTN_PIN_PWR):{
			ScreenState = DISPLAY_SCREEN_BOOTLOADER;
			DISPLAY_MSG  msg = {0,0};
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);				
		}break;			
		case (BTN_PIN_UP_DOWN):{
			
			uint32_t command[1] = {PC_UART_CCOMMAND_BOOTLOADER_BLE_JUMP_BOOTLOADER | PC_UART_CCOMMAND_MASK_BOOTLOADER};
			_uart_data_param param;
			param.user_callback = NULL;
			param.p_data = (uint8_t *)command;
			param.length = sizeof(command);
			uint8_t count = TASK_UART_DELAY_COUNT_OUT;
			while(drv_ble_send_data(&param) != ERR_SUCCESS && count--)
			{
				DELAY_MS(TASK_UART_DELAY);
			}				
			
	
		}break;	
					
	
	}

}


void gui_back_door_task_display(void)
{
	if(ScreenState == DISPLAY_SCREEN_BACK_DOOR)
	{
		DISPLAY_MSG  msg = {0,0};
		msg.cmd = MSG_DISPLAY_SCREEN;
		xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
	}

}


#endif



