//Author: Jason Gong
//Time：20180301 V1.40
/*This is the file for drawing train notify interfaces, which would be including 2 
goal notify interfaces without numbers and heartrate warning interface*/

#include "gui_notify_train.h"
#include "drv_lcd.h"
#include "drv_key.h"
#include "font_displaystrs.h"
#include "task_display.h"
#include "img_home.h"
#include <string.h>
#include "time_notify.h"
#include "com_data.h"
#include "com_sport.h"
#include "img_notify.h"
#include "task_sport.h"

#define GUI_NOTIFY_TRAIN_LOG_ENATRAIND 1

#if DEBUG_ENATRAIND == 1 && GUI_NOTIFY_TRAIN_LOG_ENATRAIND == 1
	#define GUI_NOTIFY_TRAIN_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_NOTIFY_TRAIN_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define GUI_NOTIFY_TRAIN_WRITESTRING(...)
	#define GUI_NOTIFY_TRAIN_PRINTF(...)		        
#endif

void gui_notify_train_step_paint(void)		//目标步数达成界面
{
	GUI_NOTIFY_TRAIN_PRINTF("[GUI_NOTIFY_ALARM]:gui_notify_train_step_paint\n"); 	
	
	SetWord_t word = {0};
	LCD_SetBackgroundColor(LCD_WHITE);
	//LCD_SetPicture(20, LCD_CENTER_JUSTIFIED, LCD_NONE, LCD_NONE, &Img_Goal_Step_58X78);  
	LCD_SetArc(120,120,120,8,225,135,LCD_CYAN);//缺口环
	LCD_SetPicture(20, LCD_CENTER_JUSTIFIED, LCD_CYAN, LCD_NONE, &img_monitor_step);
          

	word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;
	
	char p_str[20];
	memset(p_str,0,20);
	
	word.x_axis = 120;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;	
	LCD_SetString((char *)NotifyMenuStrs[SetValue.Language][1],&word);
	
	word.x_axis = 150;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;	
	LCD_SetString((char *)NotifyMenuStrs[SetValue.Language][3],&word);			
	
	
//	LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
}



void gui_notify_train_step_btn_evt(uint32_t Key_Value)
{
	switch(Key_Value)
	{
		case (KEY_BACK):
//		case (KEY_LIGHT):	
//		case (KEY_UP):
		case (KEY_OK):
//		case (KEY_DOWN):
		{
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

void gui_notify_train_calory_paint(void)			//目标卡路里达成界面
{
	GUI_NOTIFY_TRAIN_PRINTF("[GUI_NOTIFY_ALARM]:gui_notify_train_calory_paint\n"); 	
	
	LCD_SetBackgroundColor(LCD_WHITE);
	
	LCD_SetPicture(30, LCD_CENTER_JUSTIFIED, LCD_NONE, LCD_NONE, &Img_Goal_Calory_58X78);

	SetWord_t word = {0};
	word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_NONE;

	word.kerning = 0;
	
	char p_str[20];
	memset(p_str,0,20);

	word.x_axis = 120;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;	
	LCD_SetString((char *)NotifyMenuStrs[SetValue.Language][2],&word);
	
	word.x_axis = 150;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;	
	LCD_SetString((char *)NotifyMenuStrs[SetValue.Language][3],&word);			
	
	//	LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
	
}



void gui_notify_train_calory_btn_evt(uint32_t Key_Value)
{
	switch(Key_Value)
	{
		case (KEY_BACK):
//		case (KEY_LIGHT):	
//		case (KEY_UP):
		case (KEY_OK):
//		case (KEY_DOWN):
		{
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

void gui_notify_heartrate_warning_paint(uint32_t Heart)				//心率警告界面
{
	GUI_NOTIFY_TRAIN_PRINTF("[GUI_NOTIFY_ALARM]:gui_notify_heartrate_warning_paint\n"); 	
	LCD_SetBackgroundColor(LCD_WHITE);
	SetWord_t word = {0};
	LCD_SetPicture(42, 40, LCD_NONE, LCD_NONE, &Img_heartrate_32X28);
  
    word.x_axis = 42 + Img_heartrate_32X28.height/2 - 24/2;
	word.y_axis = 40 + Img_heartrate_32X28.width + 8;
	word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_NONE;
	word.size = LCD_FONT_24_SIZE;	
	LCD_SetString((char *)NotifyMenuStrs[SetValue.Language][0],&word);



	
	 word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;
	char p_str[20];
	memset(p_str,0,sizeof(p_str));
	sprintf(p_str,"%d",Get_Sport_Heart());

	word.x_axis = 100;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_72_SIZE;		
	LCD_SetNumber(p_str,&word);		
	
	
	

//	LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
}

void gui_notify_heartrate_warning_btn_evt(uint32_t Key_Value)
{
	switch(Key_Value)
	{
		case (KEY_BACK):
//		case (KEY_LIGHT):	
//		case (KEY_UP):
		case (KEY_OK):
//		case (KEY_DOWN):
		{
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
