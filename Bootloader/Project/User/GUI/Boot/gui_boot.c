#ifdef LAYER_BOOTLOADER  
#include "gui_boot.h"
#include "rtt_log.h"
#include "lib_error.h"
#include "SEGGER_RTT.h"

#include "lib_display.h"
#include <string.h>
#include <stdio.h>


#include "drv_lcd.h"
#include "lib_boot_setting.h"
#include "am_mcu_apollo.h"
#include "am_util_delay.h"
#include "dev_bat.h"
#include "task_ble.h"
#include "dev_dfu.h"
#include "lib_gfx.h"
#include "task_display.h"


#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "portmacro.h"
#include "portable.h"
#include "event_groups.h"
#include "semphr.h"


#include "drv_button.h"


#define MOUDLE_LOG_ENABLED 							0
#define MOUDLE_DEBUG_ENABLED   					0
#define MOUDLE_NAME                     "[GUI_BOOT]:"

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


#ifdef LAYER_BOOTLOADER  
extern _dev_dfu_progress g_dev_dfu_progress;
#endif 

extern am_hal_rtc_time_t g_rtc_time;

static uint8_t progress = 0;
static void progress_display(uint8_t progress);

extern ScreenState_t   ScreenState;
extern QueueHandle_t 	DisplayQueue;


void gui_boot_task_display(void)
{
	if(ScreenState == DISPLAY_SCREEN_BOOTLOADER)
	{
		DISPLAY_MSG  msg = {0,0};
		msg.cmd = MSG_DISPLAY_SCREEN;
		xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
	}

}

void gui_boot_paint(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"gui_boot_paint\n");

	LCD_SetBackgroundColor(LCD_BLACK);
	
//	static uint8_t m_waiting = 0;
//	
//	m_waiting++;
//	if(m_waiting >= 4)
//	{
//		m_waiting = 0;
//	}
//	
//	
//	
	SetWord_t word = {0};
	word.x_axis = 104;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	word.kerning = 0;
	LCD_SetString("正在升级",&word);	
//	
//	
//	for(uint8_t i=0;i<m_waiting;i++)
//	{
//		LCD_SetRectangle( word.x_axis+10, 4, word.y_axis+52+i*10, 4,LCD_WHITE, LCD_BLACK, 0, true);	
//	}
//	
	word.x_axis = word.x_axis+20;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_RED;
	LCD_SetString("请勿关机",&word);
//	

//	LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 8, 0, (g_dev_dfu_progress.percentage*360)/100, LCD_GREEN);
	
	
	
	if(progress >= 11)
	{
		progress = 0;
	}else{
		progress++;
	}	
	progress_display(progress);


	
	
	
}



static void progress_display(uint8_t progress)
{

	uint16_t start_degree = 0;
	uint16_t end_degree = 0;
	
	for(uint8_t i=0;i<12;i++)
	{
		start_degree = i*30+1;
		end_degree = start_degree+28;
		if(progress == i)
		{	
			LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 20, 
							start_degree, end_degree, LCD_YELLOW);
		}
		else
		{
			LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 20, 
							start_degree, end_degree, LCD_GRAY);
		}
		
	}
	
}




void gui_boot_btn_evt(uint32_t evt)
{
	switch(evt)
	{		
		
		case (BTN_PIN_UP_DOWN):{
			ScreenState = DISPLAY_SCREEN_POST_WATCH_INFO;
			DISPLAY_MSG  msg = {0,0};
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
		}break;	
			
	
	
	}

}



#endif


