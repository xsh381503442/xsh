/*
*待机功能界面:恢复时间倒计时
*/
#include "gui_recovery.h"
#include "gui_home_config.h"
#include "gui_home.h"
#include "gui.h"
/*界面显示:恢复时间倒计时界面*/
void gui_recovery_time_paint(void)
{
	SetWord_t word = {0};
	//float time = SetValue.RecoverTime/60.f;//恢复时间 h
	char str[10];
   
	//设置背景黑色
	LCD_SetBackgroundColor(LCD_BLACK);
	
	word.x_axis = 45;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	LCD_SetString("恢复时间",&word);

	if(SetValue.RecoverTime > 0)
	{
		memset(str,0,10);
		sprintf(str,"%02d:%02d",(uint32_t)(SetValue.RecoverTime/60),(uint32_t)(SetValue.RecoverTime%60));
	
		word.x_axis = LCD_LINE_CNT_MAX/2 - Font_Number_72.height/2;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_72_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_NONE;
		word.kerning = 1;
		LCD_SetNumber(str,&word);
		if(SetValue.RecoverTimeTotal > 0 && SetValue.RecoverTime <= SetValue.RecoverTimeTotal) 
		{
			LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 14, 0, SetValue.RecoverTime*360.f/SetValue.RecoverTimeTotal, LCD_CYAN);
			//LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 14, (360 - (SetValue.RecoverTime*360.f/SetValue.RecoverTimeTotal)),360, LCD_GRAY);
		}
		else
		{
			//LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 14, 0, 360, LCD_GRAY);
		}
	}
	else
	{
	
		LCD_SetRectangle(117,6,LCD_LINE_CNT_MAX/2-48,44,LCD_GRAY,0,0,LCD_FILL_ENABLE);
		LCD_SetRectangle(117,6,LCD_LINE_CNT_MAX/2+4,44,LCD_GRAY,0,0,LCD_FILL_ENABLE);
		
	}


}
/*按键处理:恢复时间倒计时界面*/
void gui_recovery_time_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	switch( Key_Value )
	{
		case KEY_OK:

			break;
#if  defined WATCH_COM_SHORTCUNT_UP
		case KEY_UP:
		case KEY_DOWN:
			//set_switch_screen_count_evt(Key_Value);
		
		//	gui_tool_switch(Key_Value);
			break;
#else	
		case KEY_UP:
			set_home_index_up();
			ScreenState = Get_Curr_Home_Index();
			msg.cmd = MSG_DISPLAY_SCREEN;
//			msg.value =Get_Curr_Home_Index() ;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		break;
		case KEY_DOWN:	
			set_home_index_down();
			ScreenState = Get_Curr_Home_Index();
			msg.cmd = MSG_DISPLAY_SCREEN;
//			msg.value =Get_Curr_Home_Index() ;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		break;
#endif
		case KEY_BACK:
			ScreenState = DISPLAY_SCREEN_TOOL;
			//返回待机界面
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}
