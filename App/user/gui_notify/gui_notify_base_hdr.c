#include "gui_notify_base_hdr.h"
#include "gui_home.h"
#include "drv_key.h"
#include "drv_lcd.h"

#include "font_config.h"
#include "com_data.h"
#include "time_notify.h"
#include "mode_power_off.h"
#include "task_hrt.h"
#include "gui_tool.h"
#include "gui_heartrate.h"

#if DEBUG_ENABLED == 1 && GUI_NOTIFY_BASE_HDR_LOG_ENABLED == 1
	#define GUI_NOTIFY_BASE_HDR_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_NOTIFY_BASE_HDR_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define GUI_NOTIFY_BASE_HDR_WRITESTRING(...)
	#define GUI_NOTIFY_BASE_HDR_PRINTF(...)		        
#endif

/*界面显示:监测心率时间到弹出框界面*/
void gui_monitor_hdr_time_paint(void)
{
	SetWord_t word = {0};
	
	LCD_SetBackgroundColor(LCD_BLACK);
	
	//左箭头
	//gui_point_direction(IMG_POINT_LEFT,LCD_WHITE);
	
	//LCD_SetPicture(45,200,LCD_RED,LCD_NONE,&step_cal_arrow_youshang);//右上角箭头
	
	LCD_SetPicture(58,205,LCD_RED,LCD_NONE,&step_cal_arrow_youshang);//右上角箭头
#if defined WATCH_FREEONE_MANGOFUN
	word.x_axis = 40;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	LCD_SetPicture(word.x_axis, word.y_axis, LCD_PERU, LCD_NONE, &Img_MangoSlogan_108X102);
	
	word.x_axis += (Img_MangoSlogan_108X102.height + 26);
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;
	LCD_SetString("芒果小助手提醒您",&word);
	
	word.x_axis += 16 + 6;
	LCD_SetString("测量基础心率啦",&word);
#else
	word.x_axis = 15;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	LCD_SetPicture(word.x_axis, word.y_axis, LCD_NONE, LCD_NONE, &Img_heartrate_32X28);
	
	//word.x_axis = LCD_LINE_CNT_MAX/2-24;
	word.x_axis = LCD_LINE_CNT_MAX/2-48;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_CYAN;
	word.bckgrndcolor = LCD_BLACK;
	word.kerning = 0;
	LCD_SetString("主人,",&word);
	word.x_axis += 32;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	LCD_SetString("监测基础心率",&word);
	
	word.x_axis += 32;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	LCD_SetString("的时间到啦!",&word);

	word.x_axis += 60;
	LCD_SetRectangle(word.x_axis,4+16+4,0,LCD_LINE_CNT_MAX,LCD_GRAY,0,0,1);
	
	word.x_axis += 4;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;
	LCD_SetString("按OK键进入监测",&word);
	
#endif
}
/*界面显示:倒计时的时间到显示的完成测试提醒界面*/
void gui_time_done_hint_paint(void)
{
  SetWord_t word = {0};

	//设置背景黑色
	LCD_SetBackgroundColor(LCD_BLACK);

	LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 120, 14, 0, 360, LCD_GRAY);
	

	//word.x_axis = 150;
	//word.y_axis = LCD_CENTER_JUSTIFIED;
	//word.size = LCD_FONT_32_SIZE;
	word.x_axis = 120 - 24/2 ;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	LCD_SetString("完成测试",&word);
	
	LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 80, 4, 100, 80, LCD_CYAN);
	/*中心:120,160*/
	LCD_SetPicture(120 - 6, 200 - 7, LCD_CYAN, LCD_NONE, &img_tool_finish_test);
 
}
/*按键处理:监测心率时间到弹出框界面*/
void gui_monitor_hdr_time_btn_evt(uint32_t Key_Value)
{
	switch(Key_Value)
	{
		case (KEY_OK):
		{
			//关闭上个界面的任务
			mode_close();
			
			timer_notify_display_stop(false);
			timer_notify_motor_stop(false);
			timer_notify_buzzer_stop(false);
			timer_notify_buzzer_start(100,100,1,false,NOTIFY_MODE_SYSTEM_TIME_REACH);

			DISPLAY_MSG  msg = {0,0};
		
#if defined WATCH_COM_SHORTCUNT_UP
			//初始化心率
			gui_heartrate_data_init();
			timer_app_tool_start();
			
			//进入心率界面
			ScreenState = DISPLAY_SCREEN_HEARTRATE;
#else
			ScreenState = DISPLAY_SCREEN_TIME_IS_MONITOR_HDR_EXPLAIN;
			msg.cmd = MSG_DISPLAY_SCREEN;
#endif
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}break;
		case (KEY_BACK):
		{
			timer_notify_motor_stop(false);
			timer_notify_buzzer_stop(false);	
			timer_notify_buzzer_start(100,100,1,false,NOTIFY_MODE_SYSTEM_TIME_REACH);
			DISPLAY_MSG  msg = {0,0};
			ScreenState = ScreenStateSave;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
		}break;
		default:
			break;
	}
}
/*按键处理:倒计时的时间到显示的完成测试提醒界面*/
void gui_time_done_hint_btn_evt(uint32_t Key_Value)
{
	switch(Key_Value)
	{		
		case (KEY_BACK):
		{
			DISPLAY_MSG  msg = {0,0};
#if  defined WATCH_COM_SHORTCUNT_UP
			ScreenState = get_current_monitor_screen();
#else
			ScreenState = Get_Curr_Home_Index();
#endif
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
			timer_notify_display_stop(false);
			timer_notify_motor_stop(false);
			timer_notify_buzzer_stop(false);	
		}break;
		default:
			break;
	}
}

