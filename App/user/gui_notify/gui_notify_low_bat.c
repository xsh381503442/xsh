#include "gui_notify_low_bat.h"
#include "drv_key.h"
#include "drv_lcd.h"

#include "font_config.h"
#include "com_data.h"
#include "time_notify.h"

#if DEBUG_ENABLED == 1 && GUI_NOTIFY_LOW_BAT_LOG_ENABLED == 1
	#define GUI_NOTIFY_LOW_BAT_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_NOTIFY_LOW_BAT_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define GUI_NOTIFY_LOW_BAT_WRITESTRING(...)
	#define GUI_NOTIFY_LOW_BAT_PRINTF(...)		        
#endif


void gui_monitor_low_bat_paint(void)
{
	LCD_SetBackgroundColor(LCD_BLACK);

	SetWord_t word = {0};
	word.x_axis = 88;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_32_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	word.kerning = 0;
	LCD_SetString("电量过低",&word);	
	word.x_axis = 120;
	LCD_SetString("请充电",&word);	
	
	LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
}

void gui_monitor_low_bat_btn_evt(uint32_t Key_Value)
{
	switch(Key_Value)
	{		
		case (KEY_BACK):{
			timer_notify_display_stop(false);
			timer_notify_motor_stop(false);
			timer_notify_buzzer_stop(false);
			DISPLAY_MSG  msg = {0,0};
			ScreenState = ScreenStateSave;
			ScreenStateSave = ScreenState;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);				
		}break;			
	}
}



