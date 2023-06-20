#include "gui_notify_bat.h"
#include "drv_key.h"
#include "drv_lcd.h"

#include "font_config.h"
#include "com_data.h"
#include "time_notify.h"
#include "drv_battery.h"
#include "task_battery.h"

#if DEBUG_ENABLED == 1 && GUI_NOTIFY_LOW_BAT_LOG_ENABLED == 1
	#define GUI_NOTIFY_LOW_BAT_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_NOTIFY_LOW_BAT_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define GUI_NOTIFY_LOW_BAT_WRITESTRING(...)
	#define GUI_NOTIFY_LOW_BAT_PRINTF(...)		        
#endif


extern am_hal_rtc_time_t RTC_time;
extern _drv_bat_event   g_bat_evt;

void gui_monitor_bat_low_paint(void)
{
	LCD_SetBackgroundColor(LCD_BLACK);

	LCD_SetPicture(40, LCD_CENTER_JUSTIFIED, LCD_NONE, LCD_NONE, &Img_Low_Battery_32X15);

	
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
	
//	LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
}

void gui_monitor_bat_low_btn_evt(uint32_t Key_Value)
{
	switch(Key_Value)
	{	
		case KEY_OK:
		case KEY_BACK:{
			timer_notify_display_stop(false);
			timer_notify_motor_stop(false);
			timer_notify_buzzer_stop(false);
			DISPLAY_MSG  msg = {0,0};
			ScreenState = ScreenStateSave;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);				
		}break;			
	}
}


void gui_monitor_bat_chg_paint(void)
{
	LCD_SetBackgroundColor(LCD_BLACK);
//	LCD_SetPicture(35, LCD_CENTER_JUSTIFIED, LCD_NONE, LCD_NONE, &Img_Low_Battery_32X15);
	LCD_SetPicture(25, LCD_CENTER_JUSTIFIED, LCD_NONE, LCD_NONE, &Img_Charging_Battery_32X20);
	uint8_t percent = g_bat_evt.level_percent;
	if( percent > 100)
	{
		percent = 100;
	}
		
	if(0 == system_voltage_flag_get())
	{		
		SetWord_t word = {0};
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_BLACK;
		word.kerning = 0;
		
		uint8_t color = 0;
		if(percent <= 10)
		{
			color = LCD_RED;
		}
		else if(percent > 10 && percent < 20)
		{
			color = LCD_YELLOW;
		}
		else{
			color = 0xBB;
		}
		LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 8, 0, (percent*360)/100, color);
		
		char p_str[20];
		
		memset(p_str,0,sizeof(p_str));
		word.x_axis = 60;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_88_SIZE;
		word.kerning = 1;
		sprintf(p_str,"%d",percent);
		LCD_SetNumber(p_str,&word);	
		
		word.x_axis = 120;
#if defined WATCH_SIM_NUMBER
		word.y_axis = (LCD_PIXEL_ONELINE_MAX - (strlen(p_str)*(Font_Number_88.width + word.kerning)))/2 
							+ (strlen(p_str)*(Font_Number_88.width + word.kerning)) + 6;
		word.kerning = 1;
#else
		word.y_axis = 180;
		word.kerning = 0;
#endif
		word.size = LCD_FONT_32_SIZE;
		LCD_SetString("%",&word);
		
		
		memset(p_str,0,sizeof(p_str));
		word.x_axis = 170;
		word.size = LCD_FONT_32_SIZE;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		sprintf(p_str,"%02d:%02d",RTC_time.ui32Hour,RTC_time.ui32Minute);
		LCD_SetNumber(p_str,&word);		
	
	}
//	LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
}

void gui_monitor_bat_chg_btn_evt(uint32_t Key_Value)
{
	switch(Key_Value)
	{		
		case (KEY_BACK):{
			//充电时不允许操作按键
            #if DEBUG_ENABLED==1  //调试时可以操作按键
			timer_battery_stop();	   
			DISPLAY_MSG  msg = {0,0};
			ScreenState = ScreenStateSave;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
            #endif
		}break;			
	}
}



