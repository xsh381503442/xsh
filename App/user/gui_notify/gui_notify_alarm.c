#include "gui_notify_alarm.h"
#include "SEGGER_RTT.h"
#include "drv_key.h"
#include "drv_lcd.h"
#include "drv_alarm.h"
#include <string.h>
#include "font_config.h"
#include "task_display.h"
#include "com_data.h"
#include "gui_theme.h"
#include "gui_time.h"
#include "gui_system.h"
#include "cmd_ble.h"
#include "lib_error.h"
#include "drv_ble.h"
#include "task_ble.h"
#include "img_tool.h"
#include "time_notify.h"
#include "img_menu.h"
#include "time_progress.h"
#include "watch_config.h"

#define GUI_NOTIFY_ALARM_LOG_ENABLED 1

#if DEBUG_ENABLED == 1 && GUI_NOTIFY_ALARM_LOG_ENABLED == 1
	#define GUI_NOTIFY_ALARM_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_NOTIFY_ALARM_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define GUI_NOTIFY_ALARM_WRITESTRING(...)
	#define GUI_NOTIFY_ALARM_PRINTF(...)		        
#endif

extern SetValueStr SetValue;
#define BACKGROUND_COLOR      LCD_WHITE
#define FOREGROUND_COLOR      LCD_BLACK
//#define DELAY_MS(X)                 am_util_delay_ms(X)
#define DELAY_MS(X)                 vTaskDelay(X / portTICK_PERIOD_MS)
//#define DELAY_MS(X)                   do{TickType_t xLastWakeTime;xLastWakeTime = xTaskGetTickCount ();vTaskDelayUntil( &xLastWakeTime, X / portTICK_PERIOD_MS );}while(0)

extern am_hal_rtc_time_t RTC_time;


void gui_notify_alarm_paint(void)
{
	
	SetWord_t word = {0};
	char p_str[20];
	am_hal_rtc_time_get(&RTC_time);
	
	LCD_SetBackgroundColor(LCD_BLACK);
	
	LCD_SetPicture(20, LCD_CENTER_JUSTIFIED, LCD_RED, LCD_NONE, &img_set_time_alarm);

	
	
	memset(p_str,0,sizeof(p_str));
	sprintf(p_str,"%02d:%02d",SetValue.alarm_instance[g_drv_alarm_num].time.hour,SetValue.alarm_instance[g_drv_alarm_num].time.minute);
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;
	word.x_axis = (LCD_LINE_CNT_MAX - Font_Number_56.height)/2;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_56_SIZE;
	LCD_SetNumber(p_str,&word);
	

   LCD_SetRectangle(180 - 4,4 + 16 + 4,0,240,LCD_GRAY,0,0,LCD_FILL_ENABLE);

	word.x_axis = 180;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;	
	LCD_SetString("按任意键退出",&word);		
	
	
}



void gui_notify_alarm_btn_evt(uint32_t evt)
{
	switch(evt)
	{
		case (KEY_BACK):
//		case (KEY_LIGHT):	
		case (KEY_UP):
		case (KEY_OK):
		case (KEY_DOWN):
		{
			timer_progress_stop();
			timer_notify_remind_stop(false);
			timer_notify_display_stop(false);
			timer_notify_motor_stop(false);
			timer_notify_buzzer_stop(false);
			DISPLAY_MSG  msg = {0,0};
			ScreenState = ScreenStateAlarm;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}break;				
	}
}








