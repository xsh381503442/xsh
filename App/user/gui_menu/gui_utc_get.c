#include "gui_utc_get.h"
#include "SEGGER_RTT.h"
#include "drv_key.h"
#include "drv_lcd.h"
#include <string.h>
#include "font_displaystrs.h"
#include "task_display.h"
#include "com_data.h"
#include "time_progress.h"

#define GUI_UTC_GET_LOG_ENABLED 1

#if DEBUG_ENABLED == 1 && GUI_UTC_GET_LOG_ENABLED == 1
	#define GUI_UTC_GET_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_UTC_GET_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define GUI_UTC_GET_WRITESTRING(...)
	#define GUI_UTC_GET_PRINTF(...)		        
#endif

extern SetValueStr SetValue;
#define BACKGROUND_COLOR      LCD_BLACK
#define FOREGROUND_COLOR      LCD_WHITE

_gui_utc_get_param g_gui_utc_get_param =
{
	.evt = GUI_UTC_GET_EVT_SEARCHING,
	.progress = 0,
};

void gui_utc_get_paint(void)
{
	LCD_SetBackgroundColor(BACKGROUND_COLOR);
	
	switch(g_gui_utc_get_param.evt)
	{
		case GUI_UTC_GET_EVT_SEARCHING:{
			SetWord_t word = {0};
			word.x_axis = 104;
			word.y_axis = LCD_CENTER_JUSTIFIED;
			word.size = LCD_FONT_32_SIZE;
			word.forecolor = FOREGROUND_COLOR;
			word.bckgrndcolor = BACKGROUND_COLOR;
			word.kerning = 0;	
			LCD_SetString("卫星授时中",&word);

			word.x_axis = 140;
			word.size = LCD_FONT_32_SIZE;
			char p_str[3];
			memset(p_str,0,sizeof(p_str));
			sprintf(p_str,"%d",g_gui_utc_get_param.progress);
			LCD_SetNumber((char *)p_str,&word);
		}break;
		case GUI_UTC_GET_EVT_SUCCESS:{
			SetWord_t word = {0};
			word.x_axis = 160;
			word.y_axis = LCD_CENTER_JUSTIFIED;
			word.size = LCD_FONT_32_SIZE;
			word.forecolor = FOREGROUND_COLOR;
			word.bckgrndcolor = BACKGROUND_COLOR;
			word.kerning = 0;	
			LCD_SetString("授时完成",&word);		
		}break;
		case GUI_UTC_GET_EVT_FAIL:{
			SetWord_t word = {0};
			word.x_axis = 160;
			word.y_axis = LCD_CENTER_JUSTIFIED;
			word.size = LCD_FONT_32_SIZE;
			word.forecolor = FOREGROUND_COLOR;
			word.bckgrndcolor = BACKGROUND_COLOR;
			word.kerning = 0;	
			LCD_SetString("授时失败",&word);		
		}break;		
	}
	
	LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
}


void gui_utc_get_btn_evt(uint32_t evt)
{	
	switch(evt)
	{		
		case (KEY_BACK):{
			timer_progress_stop();
			timer_progress_uninit();
			
			if(g_gui_utc_get_param.evt == GUI_UTC_GET_EVT_SUCCESS)
			{
				memset(&g_gui_utc_get_param,0,sizeof(g_gui_utc_get_param));
				DISPLAY_MSG  msg = {0,0};
				ScreenStateSave = ScreenState;
				ScreenState = DISPLAY_SCREEN_HOME;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
			
			}else if(g_gui_utc_get_param.evt == GUI_UTC_GET_EVT_FAIL)
			{
				memset(&g_gui_utc_get_param,0,sizeof(g_gui_utc_get_param));
				DISPLAY_MSG  msg = {0,0};
				ScreenStateSave = ScreenState;
				ScreenState = DISPLAY_SCREEN_TIME;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
			}else if(g_gui_utc_get_param.evt == GUI_UTC_GET_EVT_SEARCHING)
			{
				memset(&g_gui_utc_get_param,0,sizeof(g_gui_utc_get_param));
				DISPLAY_MSG  msg = {0,0};
				ScreenStateSave = ScreenState;
				ScreenState = DISPLAY_SCREEN_TIME;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
			}
			
			
		}break;			

	}
}








