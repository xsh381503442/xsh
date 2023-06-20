/*
*待机功能界面:最大摄氧量（6个区间中显示当前区间）
*/
#include "gui_VO2max.h"
#include "gui_home_config.h"
#include "gui_home.h"
#include "gui_sport.h"
#include "gui_run.h"
#include "gui_sport.h"
#include "gui_step.h"
#include "gui.h"
#include "gui_tool.h"

#include "drv_lcd.h"
#include "drv_key.h"
#include "drv_battery.h"

#include "task_display.h"
#include "task_key.h"
#include "task_gps.h"
#include "task_sport.h"

#include "font_config.h"
#include "com_data.h"
#include "com_sport.h"
#include "algo_hdr.h"
#include "timer_app.h"
#include "time_notify.h"

extern _drv_bat_event g_bat_evt;
extern void set_gps_hint_motor_flag(bool flag);


static void gui_VO2max_histogram_paint(uint16_t VO2MaxValue)
{
	SetWord_t word = {0};
	uint8_t range = get_VO2max_range(VO2MaxValue,SetValue.AppSet.Sex,SetValue.AppSet.Age);
	
	for(int i = 0;i < 6;i++)
	{
		if((range == i) && (SetValue.IsMeasuredVO2max == 2)&& (VO2MaxValue != 0))
		
		{
			//word.x_axis = LCD_LINE_CNT_MAX/2 + 12;
			
			word.x_axis = LCD_LINE_CNT_MAX/2 + 10;
		}
		else
		{
			word.x_axis = LCD_LINE_CNT_MAX/2 + 26;
			
		}
		//word.y_axis = 14 + i * 32 + i * 4;
		
		word.y_axis =  i * 37 + i * 4;
		LCD_SetRectangle(word.x_axis, LCD_LINE_CNT_MAX - 62 -word.x_axis, word.y_axis, 32, get_VO2max_range_color(i), 1, 1, LCD_FILL_ENABLE);
	}
}
/*界面显示:最大摄氧量数值显示界面*/
void gui_VO2max_value_paint(uint16_t VO2MaxValue)
{
	SetWord_t word = {0};
  char str[10];
	uint8_t range = get_VO2max_range(VO2MaxValue,SetValue.AppSet.Sex,SetValue.AppSet.Age);
	//设置背景黑色
	LCD_SetBackgroundColor(LCD_BLACK);

	word.x_axis = 24;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;


	
	if((SetValue.IsMeasuredVO2max != 2)|| (VO2MaxValue == 0))
		
	{//第一次进入后 测试时间低于20分钟 无移动距离 认为数据无效
	 
		 if(SetValue.IsMeasuredVO2max==0)
			{//最大摄氧量恢复出厂第一次

			
			LCD_SetString("最大摄氧量",&word);
		   word.x_axis = LCD_LINE_CNT_MAX/2 - img_tools_big_lists[0].height/2;
		   word.y_axis = LCD_LINE_CNT_MAX/2 - img_tools_big_lists[0].width/2;
		   word.forecolor = LCD_RED;
		   word.bckgrndcolor = LCD_NONE;
		   LCD_SetPicture(word.x_axis,word.y_axis ,word.forecolor,word.bckgrndcolor,&img_tools_big_lists[0]);
	       }
           else if(SetValue.IsMeasuredVO2max==1)
	    	{
	    	
			LCD_SetString((char *)GuiHomeLockStrs[SetValue.Language][73],&word);//"(ml/kg/min)"
		     LCD_SetRectangle(LCD_LINE_CNT_MAX/2-42,6,LCD_LINE_CNT_MAX/2-48,44,LCD_GRAY,0,0,LCD_FILL_ENABLE);
		     LCD_SetRectangle(LCD_LINE_CNT_MAX/2-42,6,LCD_LINE_CNT_MAX/2+4,44,LCD_GRAY,0,0,LCD_FILL_ENABLE);
		/*矩形底端178*/
	         gui_VO2max_histogram_paint(VO2MaxValue);
	      }  
		   else;

	}

		
	else
	{
		memset(str,0,10);
		sprintf(str,"%d",VO2MaxValue);
		word.x_axis += 28;
		word.y_axis = (LCD_LINE_CNT_MAX - strlen(str) * Font_Number_72.width)/2;
		word.size = LCD_FONT_72_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_NONE;
		word.kerning = 1;
		LCD_SetNumber(str,&word);
		
		word.x_axis += (Font_Number_72.height - Font_Number_24.height);
		word.y_axis += strlen(str)*Font_Number_72.width + 18;
		word.size = LCD_FONT_24_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_NONE;
		word.kerning = 1;
		
		switch(range)
		{
			case 0:
				LCD_SetString((char *)GuiHomeLockStrs[SetValue.Language][89],&word);//"差"
				break;
			case 1:
				LCD_SetString((char *)GuiHomeLockStrs[SetValue.Language][88],&word);//"下"
				break;
			case 2:
				LCD_SetString((char *)GuiHomeLockStrs[SetValue.Language][87],&word);//"中"
				break;
			case 3:
				LCD_SetString((char *)GuiHomeLockStrs[SetValue.Language][86],&word);//"良"
				break;
			case 4:
				LCD_SetString((char *)GuiHomeLockStrs[SetValue.Language][9],&word);//"优"
				break;
			case 5:
				LCD_SetString((char *)GuiHomeLockStrs[SetValue.Language][85],&word);//"超优"
				break;
			default:
				break;
		}
		/*矩形底端178*/
	gui_VO2max_histogram_paint(VO2MaxValue);
	}

	/*矩形框*/
	word.x_axis = LCD_LINE_CNT_MAX/2 + 10 + 16*3 + 12;//178+12
	word.y_axis = LCD_CENTER_JUSTIFIED;
	LCD_SetRectangle(word.x_axis,22,0,LCD_LINE_CNT_MAX,LCD_GRAY,1,1,LCD_FILL_ENABLE);
	word.x_axis += 3;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	LCD_SetString("OK键进入VO2max测试",&word);//"OK键进入测试"

	/*矩形底端178*/
//	gui_VO2max_histogram_paint(VO2MaxValue);
}
/*界面显示:最大摄氧量测试提醒界面*/
void gui_VO2max_hint_mesuring_paint(void)
{
	SetWord_t word = {0};
	//设置背景黑色
	LCD_SetBackgroundColor(LCD_BLACK);
	gui_sport_gps_status(SPORT_GPS_LEFT1);//gps图标	//稳定心率
	
	word.x_axis = 20;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	LCD_SetString("VO2max",&word);//"VO2max"
	
	word.x_axis += Font_Number_24.height + 4;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	LCD_SetString((char *)GuiHomeLockStrs[SetValue.Language][11],&word);//"最大摄氧量测试"
	
	//LCD_SetPicture(56,206,LCD_RED,LCD_NONE,&step_cal_arrow_youshang);//右上角箭头
    
	LCD_SetPicture(58,205,LCD_RED,LCD_NONE,&step_cal_arrow_youshang);//右上角箭头
	
	word.x_axis = LCD_LINE_CNT_MAX/2 - Font_Number_24.height + 12;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_CYAN;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	LCD_SetString("热身后",&word);//"热身后"
	
	word.x_axis = LCD_LINE_CNT_MAX/2 + 12;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_CYAN;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	LCD_SetString("全力跑12分钟",&word);//"全力跑12分钟"

  
    LCD_SetRectangle(200-4,4+16+4,0,240,LCD_GRAY,1,1,LCD_FILL_ENABLE);
	
	word.x_axis = 200;
	word.forecolor = LCD_BLACK;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString("OK键开始",&word);//"OK键开始"

}
/*界面显示:最大摄氧量倒计时界面*/
void gui_VO2max_countdown_paint(uint8_t value)
{
	SetWord_t word = {0};
  char str[5];
	//设置背景黑色
	LCD_SetBackgroundColor(LCD_BLACK);
	//清屏
	//word.x_axis = 25;
	
	word.x_axis = 120 + 64;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	LCD_SetString((char *)GuiHomeLockStrs[SetValue.Language][14],&word);//"倒计时"
	
	gui_sport_gps_status(SPORT_GPS_LEFT2);//gps图标	
	memset(str,0,5);
	sprintf(str,"%d",value);
	word.x_axis = LCD_LINE_CNT_MAX/2 - 44;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_88_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	LCD_SetNumber(str,&word);
	switch(value)
	{
	  case 3:
		LCD_SetArc(120,120,120,14,225,135,LCD_RED);
		break;
		case 2:
			
		LCD_SetArc(120,120,120,14,225,225+90,LCD_GRAY);
		LCD_SetArc(120,120,120,14,225+90,135,LCD_RED);
		break;
		case 1:
			
		LCD_SetArc(120,120,120,14,225,225+180,LCD_GRAY);
		LCD_SetArc(120,120,120,14,225+180,135,LCD_RED);
		break;

	};
}
/*界面显示:最大摄氧量测试界面*/
void gui_VO2max_mesuring_paint(float mile,Countdown_time_t countdown_time,uint8_t hdrValue,float min_km)
{
	SetWord_t word = {0};
  
	//设置背景黑色
	LCD_SetBackgroundColor(LCD_BLACK);
	//清屏
	
	word.x_axis = 80;
	//LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX, LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);
	
	LCD_SetRectangle(word.x_axis,2,LCD_LINE_CNT_MAX/2 - 90 ,180, LCD_LIGHTGRAY, 0, 0, LCD_FILL_ENABLE);


	word.x_axis = 160;
	//LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX, LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);
	
	LCD_SetRectangle(word.x_axis,2,LCD_LINE_CNT_MAX/2 - 90,180, LCD_LIGHTGRAY, 0, 0, LCD_FILL_ENABLE);
	gui_sport_gps_status(SPORT_GPS_LEFT1);


	gui_sport_data_display_total_distance_paint(SPORT_DATA_DISPALY_INDEX_UP_CIRCLE,mile);
	gui_sport_data_display_total_time_paint(SPORT_DATA_DISPALY_INDEX_MID_GPS,(countdown_time.hour *3600 + countdown_time.minute *60 + countdown_time.second));
	gui_sport_data_display_realtime_pace_paint(SPORT_DATA_DISPALY_INDEX_DOWN_NOHINT,min_km);

}
/*界面显示:最大摄氧量或乳酸阈提前结束时的提醒界面*/
void gui_VO2max_lactate_eraly_end_hint_paint(void)
{
	SetWord_t word = {0};
	//设置背景黑色
	LCD_SetBackgroundColor(LCD_BLACK);
	//清屏

	word.x_axis = 25;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	LCD_SetString("提示",&word);//"提示"
    
	LCD_SetPicture(word.x_axis,LCD_PIXEL_ONELINE_MAX/2 - 16 - 4 - 16,LCD_RED,LCD_NONE,&img_tool_vo2max_ahead_quit_warn);


	
	word.x_axis = LCD_LINE_CNT_MAX/2-28;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	LCD_SetString("提前结束将会影响",&word);//"提前结束将会"
	
	word.x_axis = LCD_LINE_CNT_MAX/2+4;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	LCD_SetString("测试结果哦~",&word);//"影响测试结果哦~"


	LCD_SetRectangle(200-4,24,0,240,LCD_GRAY,1,1,LCD_FILL_ENABLE);
	word.x_axis = 200;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	LCD_SetString("按OK键退出测试",&word);//"按OK键退出测试"
}
/*
界面显示:还有未完成运动 稍后继续后进入最大摄氧量等测试模式时的提醒界面
*/
void gui_end_unfinished_sport_hint_paint(uint8_t m_screenflag)
{
	SetWord_t word = {0};
	//设置背景黑色
	LCD_SetBackgroundColor(LCD_BLACK);
	//清屏
	
	word.x_axis = 30;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	if(m_screenflag == DISTINGUISH_VO2)
	{
		//LCD_SetString("VO2max",&word);
		LCD_SetPicture(word.x_axis,word.y_axis ,LCD_RED,LCD_NONE,&img_tools_big_lists[0]);
	}
	else if(m_screenflag == DISTINGUISH_LACTATE)
	{
		//LCD_SetString("乳酸阈值",&word);
		
		LCD_SetPicture(word.x_axis,word.y_axis ,LCD_RED,LCD_NONE,&img_tools_big_lists[1]);
	}
	else if(m_screenflag == DISTINGUISH_STEP)
	{
		LCD_SetString("步长校准",&word);
	}
	
	word.x_axis = LCD_LINE_CNT_MAX/2-24;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	LCD_SetString("当前有未完成的运动",&word);
	
	word.x_axis = LCD_LINE_CNT_MAX/2+8;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	LCD_SetString("请结束运动后再进行测试",&word);
}

/*按键处理:最大摄氧量数值显示界面*/
void gui_VO2max_value_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	switch( Key_Value )
	{
		case KEY_OK:
			if(1 == IS_SPORT_CONTINUE_LATER)
			{//如果有稍后继续,必须结束后才可以进入
				ScreenState = DISPLAY_SCREEN_END_UNFINISH_SPORT;
				msg.cmd = MSG_DISPLAY_SCREEN;
				msg.value = DISTINGUISH_VO2;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
			else
			{
				total_hdr_VO2max_value = 0;
				total_hdr_VO2max_count = 0;
				set_gps_hint_motor_flag(false);
				ScreenState = DISPLAY_SCREEN_VO2MAX_HINT;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				CreateGPSTask();
			}
			break;
#if  defined WATCH_COM_SHORTCUNT_UP
		case KEY_UP:
		case KEY_DOWN:
			//set_switch_screen_count_evt(Key_Value);
		
			//gui_tool_switch(Key_Value);
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
/*按键处理:最大摄氧量测试提醒界面*/
void gui_VO2max_hint_mesuring_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	switch( Key_Value )
	{
		case KEY_OK:
			set_gps_hint_motor_flag(false);//退出该界面需清除图标闪动标记
			ScreenState = DISPLAY_SCREEN_CUTDOWN;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			timer_app_countdown_start();
			break;
		case KEY_BACK:
			CloseGPSTask();
			ScreenState = DISPLAY_SCREEN_VO2MAX;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}

/*按键处理:最大摄氧量测试界面*/
void gui_VO2max_mesuring_btn_evt(uint32_t Key_Value)
{
	//DISPLAY_MSG  msg = {0};
	switch( Key_Value )
	{
		case KEY_BACK:
			if((ScreenState != DISPLAY_SCREEN_LOGO)
				&& (ScreenState != DISPLAY_SCREEN_NOTIFY_BAT_CHG)
				&& time_notify_is_ok())
			{
				if(ScreenState < DISPLAY_SCREEN_NOTIFY)
				{
					ScreenStateSave = ScreenState;
				}
				timer_notify_display_start(3000,1,false);
				timer_notify_motor_start(1000,500,1,false,NOTIFY_MODE_TOOLS);
				timer_notify_buzzer_start(1000,500,1,false,NOTIFY_MODE_TOOLS);
				DISPLAY_MSG  msg = {0,0};
				ScreenState = DISPLAY_SCREEN_NOTIFY_VO2MAX_LACTATE_ERALY_END;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);							
			}	
			break;
		default:
			break;
	}
}
/*界面显示:最大摄氧量或乳酸阈提前结束时的提醒界面*/
void gui_VO2max_lactate_eraly_end_hint_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	switch( Key_Value )
	{
		case KEY_OK:
			timer_notify_display_stop(false);
			timer_notify_motor_stop(false);
			timer_notify_buzzer_stop(false);
			UninitVO2maxMeasing();
			if(Distinguish_VO2_Lactate == DISTINGUISH_LACTATE)
			{
				ScreenState = DISPLAY_SCREEN_LACTATE_THRESHOLD;
			}
			else if(Distinguish_VO2_Lactate == DISTINGUISH_VO2)
			{
				ScreenState = DISPLAY_SCREEN_VO2MAX;
			}
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		case KEY_BACK:
			{
				timer_notify_display_stop(false);
				timer_notify_motor_stop(false);
				timer_notify_buzzer_stop(false);	
				DISPLAY_MSG  msg = {0,0};
				ScreenState = DISPLAY_SCREEN_VO2MAX_MEASURING;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
			break;
		default:
			break;
	}
}
/*
按键处理:还有未完成运动 稍后继续后进入最大摄氧量等测试模式时的提醒界面
*/
void gui_end_unfinished_sport_hint_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0,0};
	switch( Key_Value )
	{
		case KEY_OK:
			SportContinueLaterDisplay();
			break;
		case KEY_BACK:
		{
#if  defined WATCH_COM_SHORTCUNT_UP
			ScreenState = DISPLAY_SCREEN_HOME;
#else			
			uint32_t screen = Get_Curr_Home_Index();
			if(screen == DISPLAY_SCREEN_VO2MAX)
			{
				ScreenState = DISPLAY_SCREEN_VO2MAX;
			}
			else if(screen == DISPLAY_SCREEN_LACTATE_THRESHOLD)
			{
				ScreenState = DISPLAY_SCREEN_LACTATE_THRESHOLD;
			}
			else if(screen == DISPLAY_SCREEN_STEP)
			{
				if(menuIndex == 1)
				{
					ScreenState = DISPLAY_SCREEN_STEP_WEEK_STEPS;
				}
				else if(menuIndex == 2)
				{
					ScreenState = DISPLAY_SCREEN_STEP_WEEK_MILEAGE;
				}
			}
#endif
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
		}
			break;
		default:
			break;
	}
}
