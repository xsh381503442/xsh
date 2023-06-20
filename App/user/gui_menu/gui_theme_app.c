#include "gui_theme_app.h"
#include "SEGGER_RTT.h"
#include "drv_key.h"
#include "drv_lcd.h"
#include <string.h>
#include "font_displaystrs.h"
#include "task_display.h"
#include "com_data.h"
#define GUI_THEME_APP_DOWNLOAD_LOG_ENABLED 1

#if DEBUG_ENABLED == 1 && GUI_THEME_APP_DOWNLOAD_LOG_ENABLED == 1
	#define GUI_APP_DOWNLOAD_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_APP_DOWNLOAD_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define GUI_APP_DOWNLOAD_WRITESTRING(...)
	#define GUI_APP_DOWNLOAD_PRINTF(...)		        
#endif

extern SetValueStr SetValue;
#define BACKGROUND_COLOR      LCD_WHITE
#define FOREGROUND_COLOR      LCD_BLACK



void gui_theme_app_download_paint(void)
{
	LCD_SetBackgroundColor(BACKGROUND_COLOR);
	
	SetWord_t word = {0};
	word.x_axis = 110;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_32_SIZE;
	word.forecolor = FOREGROUND_COLOR;
	word.bckgrndcolor = BACKGROUND_COLOR;
	word.kerning = 0;
	LCD_SetString("ÍøÂçÖ÷Ìâ",&word);		
		
	
	LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
}


void gui_theme_app_download_btn_evt(uint32_t evt)
{
	
	switch(evt)
	{		
		case (KEY_BACK):{
			DISPLAY_MSG  msg = {0,0};
			ScreenStateSave = ScreenState;
			ScreenState = DISPLAY_SCREEN_THEME;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
		}break;			
		case (KEY_UP):{

		}break;	
			
		case (KEY_OK):{

		}break;		
		case (KEY_DOWN):{

		}break;					
	
	}

	



}








