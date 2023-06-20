#include "gui_time.h"
#include "SEGGER_RTT.h"
#include "drv_key.h"
#include "drv_lcd.h"
#include <string.h>
#include "font_displaystrs.h"
#include "task_display.h"
#include "com_data.h"
#include "time_progress.h"
#include "img_tool.h"
#include "img_menu.h"
#include "drv_ublox.h"
#include "task_gps.h"
#include "mode_power_off.h"
#include "lib_error.h"
#include "watch_config.h"
#include "font_config.h"
#if defined(WATCH_PRECISION_TIME_CALIBRATION)
#include "algo_time_calibration.h"
#endif
#if defined WATCH_HAS_SET_TIME_MANUALLY  
#include "gui_system.h"
#include "timer_app.h"
#include "time_notify.h"
#include "com_sport.h"
#if defined WATCH_TIMEZONE_SET
#include "gui.h"

#endif

typedef struct {
	uint8_t year_t;
	uint8_t mon_t;
	uint8_t day_t;
	uint8_t hour_t;
	uint8_t min_t;
	uint8_t sec_t;
}manual_time;

manual_time MANUAL_TIME; 
#endif

#define GUI_TIME_LOG_ENABLED 1

#if DEBUG_ENABLED == 1 && GUI_TIME_LOG_ENABLED == 1
	#define GUI_TIME_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_TIME_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define GUI_TIME_WRITESTRING(...)
	#define GUI_TIME_PRINTF(...)		        
#endif


//#define DELAY_US(X)                 am_util_delay_us(X)
//#define DELAY_MS(X)                 am_util_delay_ms(X)
#define DELAY_MS(X)                 vTaskDelay(X / portTICK_PERIOD_MS)
//#define DELAY_MS(X)                   do{TickType_t xLastWakeTime;xLastWakeTime = xTaskGetTickCount ();vTaskDelayUntil( &xLastWakeTime, X / portTICK_PERIOD_MS );}while(0)




extern SetValueStr SetValue;
#define BACKGROUND_COLOR      LCD_WHITE
#define FOREGROUND_COLOR      LCD_BLACK

static uint8_t m_time_index = 0;


_gui_time_utc_get_param g_gui_time_utc_get_param =
{
	.evt = GUI_TIME_UTC_GET_EVT_SEARCHING,
	.progress = 0,
};

#define GUI_TIME_ALARM_SET_INDEX_TIME_SET                  0
#define GUI_TIME_ALARM_SET_INDEX_BUZZER_SET                1
#define GUI_TIME_ALARM_SET_INDEX_REPITION_SET              2
#define GUI_TIME_ALARM_SET_INDEX_ENABLE_SET                3
static uint8_t m_gui_time_alarm_set_index = 0;
static uint8_t m_gui_time_alarm_index = 0;
static Alarm_time_t m_alarm_time = {0,0};
static uint8_t m_gui_time_alarm_set_time_index = 0;
#define GUI_TIME_ALARM_SET_TIME_HOUR           0
#define GUI_TIME_ALARM_SET_TIME_MINUTE         1
#define GUI_TIME_ALARM_SET_SOUND_N_VIBERATE_SOUND_N_VIBERATE         0
#define GUI_TIME_ALARM_SET_SOUND_N_VIBERATE_SOUND                    1
#define GUI_TIME_ALARM_SET_SOUND_N_VIBERATE_VIBERATE                 2
static uint8_t m_gui_time_alarm_set_sound_N_viberate_index = 0;
static uint8_t m_gui_time_alarm_set_repition_index = 0;

#if defined WATCH_TIMEZONE_SET
static uint8_t m_time_zone_select_index = 0;
static uint8_t m_time_zone_yesno_index = 0;

#endif


//设置闹钟个数，一共4个闹钟
Alarm_t g_alarm_instance[4] = 
{
	{
		.repetition_type = ALARM_REPITION_TYPE_SINGLE,
		.buzzer_enable = true,
		.motor_enable = true,
	},
	{
		.repetition_type = ALARM_REPITION_TYPE_SINGLE,		
		.buzzer_enable = true,
		.motor_enable = true,
	},
	{
		.repetition_type = ALARM_REPITION_TYPE_SINGLE,		
		.buzzer_enable = true,
		.motor_enable = true,
	},	
		{
		.repetition_type = ALARM_REPITION_TYPE_SINGLE,
		.buzzer_enable = true,
		.motor_enable = true,
	},
};


//时间设置菜单结构体，因为闹钟设置中的声音，重复与时间三个菜单结构类似，故设置结构体指针方便接下来的定义

static void time_paint( char* menu1, char* menu2, char* menu3)
{
	SetWord_t word = {0};
  /*  word.x_axis = 120 - 24/2 - 36 - 24 - 24;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_CYAN;
	word.bckgrndcolor = LCD_NONE;
	word.size = LCD_FONT_24_SIZE;
	word.kerning = 0;
	LCD_DisplayGTString(menu1,&word);	//时间
	LCD_SetPicture( word.x_axis,120 - 2*LCD_FONT_24_SIZE/2 - 8 - img_set_menu_b[1].width,LCD_RED,LCD_NONE,&img_set_menu_b[1]);//时间图标

	
	LCD_SetRectangle(120 - 24/2 - 36 - 1 ,2,LCD_LINE_CNT_MAX/2 - 60,120,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
	LCD_SetRectangle(120 + 24/2 + 36 - 1,2,LCD_LINE_CNT_MAX/2 - 60,120,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);*/
		
	LCD_SetRectangle(120 - 1 ,2,LCD_LINE_CNT_MAX/2 - 100,200,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
	

	word.x_axis = 120-24-24;	 
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.size = LCD_FONT_24_SIZE;
	LCD_DisplayGTString(menu2,&word);	
	

	
	word.x_axis = 120 + 24;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_LIGHTGRAY;
	word.bckgrndcolor = LCD_NONE;
	word.size = LCD_FONT_16_SIZE;
	LCD_DisplayGTString(menu3,&word);	



	
}

//时间设置界面画图：主界面，包括闹钟子界面以及卫星授时子界面

void gui_time_paint(void)
{
	LCD_SetBackgroundColor(LCD_BLACK);

	switch(m_time_index + DISPLAY_SCREEN_UTC_GET)
	{
		case DISPLAY_SCREEN_UTC_GET:{
			if(SetValue.Language == L_CHINESE)
			{
				time_paint("时间",
							(char *)TimeStrs[SetValue.Language][m_time_index],
							(char *)TimeStrs[SetValue.Language][m_time_index + 1]);			
			}
			else if(SetValue.Language == L_ENGLISH)
			{
				time_paint("TIME",
							(char *)TimeStrs[SetValue.Language][m_time_index],
							(char *)TimeStrs[SetValue.Language][m_time_index + 1]);				
			}
							
		}break;

		case DISPLAY_SCREEN_ALARM:{
			if(SetValue.Language == L_CHINESE)
			{
				time_paint("时间",
							(char *)TimeStrs[SetValue.Language][m_time_index],
							(char *)TimeStrs[SetValue.Language][0]);			
			}
			else if(SetValue.Language == L_ENGLISH)
			{
				time_paint("TIME",
							(char *)TimeStrs[SetValue.Language][m_time_index],
							(char *)TimeStrs[SetValue.Language][0]);				
			}							
		}break;				
	
	}	

	LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Down_12X8);
//	LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
}

//时间设置界面：主界面按键处理，闹钟子界面与卫星授时子界面的上下切换
void gui_time_btn_evt(uint32_t evt)
{
	
	switch(evt)
	{		
		case (KEY_BACK):{
			m_time_index = 0;
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_MENU;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
		}break;			
		case (KEY_UP):{
			if(m_time_index <= 0)
			{
				m_time_index = GUI_TIME_INDEX_MAX - 1;
			}
			else
            {
				m_time_index--;
			}
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_TIME;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}break;	

		case (KEY_DOWN):
			{
			if(m_time_index >= (GUI_TIME_INDEX_MAX - 1))
			{
				m_time_index = 0;
			}
			else
     		{
				m_time_index++;
			}
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_TIME;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}break;		
		
		case (KEY_OK):{
			switch(m_time_index + DISPLAY_SCREEN_UTC_GET)
			{
				case DISPLAY_SCREEN_UTC_GET:{

					gui_time_utc_get_init();
					DISPLAY_MSG  msg = {0,0};
					ScreenState = DISPLAY_SCREEN_UTC_GET;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);

					CreateGPSTask();
		
				
				}break;
				case DISPLAY_SCREEN_ALARM:{
					memcpy(g_alarm_instance,SetValue.alarm_instance,sizeof(SetValue.alarm_instance));
					DISPLAY_MSG  msg = {0,0};
					ScreenState = DISPLAY_SCREEN_ALARM;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);											
				}break;								
			}				
		}break;		
			
	
	}
}

//UTC时间初始化函数，清除原有时间数据

void gui_time_utc_get_init(void)
{
	//清除数据
	memset(&g_gui_time_utc_get_param,0,sizeof(_gui_time_utc_get_param));
}

//卫星授时界面画图

void gui_time_utc_get_paint(void)
{
   uint8_t i;
	if(g_gui_time_utc_get_param.progress==0)
		{
	LCD_SetBackgroundColor(LCD_BLACK);
		}
	else;
	switch(g_gui_time_utc_get_param.evt)
	{
		case GUI_TIME_UTC_GET_EVT_SEARCHING:{
			SetWord_t word = {0};
			word.x_axis = 104;
			word.y_axis = LCD_CENTER_JUSTIFIED;
			word.size = LCD_FONT_32_SIZE;
			word.forecolor = LCD_WHITE;
			word.bckgrndcolor = LCD_BLACK;
			word.kerning = 0;	
			LCD_SetString("卫星授时中",&word);

			if(((g_gui_time_utc_get_param.progress%5)==0)&&(g_gui_time_utc_get_param.progress>0))
			{
			 if(g_gui_time_utc_get_param.progress>=10)
				{
                   
				   LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 14, 0, 360*4/GUI_TIME_UTC_TIMEOUT, LCD_CHOCOLATE);
				   for(i=1;i<g_gui_time_utc_get_param.progress/5;i++)
				   	{
				    // LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 8, 360*(g_gui_time_utc_get_param.progress-5)/GUI_TIME_UTC_TIMEOUT, 360*(g_gui_time_utc_get_param.progress-1)/GUI_TIME_UTC_TIMEOUT, LCD_CHOCOLATE);
				   
				   LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 14, 360*(i*5)/GUI_TIME_UTC_TIMEOUT, 360*((i+1)*5-1)/GUI_TIME_UTC_TIMEOUT, LCD_CHOCOLATE);
				   	}
				  }
			 else
				{
			     LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 14, 0, 360*(g_gui_time_utc_get_param.progress-1)/GUI_TIME_UTC_TIMEOUT, LCD_CHOCOLATE);
				}
			}
			else//从5开始
				{

				 
				  
                   for(i=0;i<g_gui_time_utc_get_param.progress/5;i++)
				   	{
				  
				   
				   LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 14, 360*(i*5)/GUI_TIME_UTC_TIMEOUT, 360*((i+1)*5-1)/GUI_TIME_UTC_TIMEOUT, LCD_CHOCOLATE);
				   	}


				   
				  }





			  
		}break;
		case GUI_TIME_UTC_GET_EVT_SUCCESS:{
			
			LCD_SetBackgroundColor(LCD_BLACK);
			SetWord_t word = {0};
			word.x_axis = 120+12;
			word.y_axis = LCD_CENTER_JUSTIFIED;
			word.size = LCD_FONT_32_SIZE;
			word.forecolor = LCD_WHITE;
			word.bckgrndcolor = LCD_BLACK;
			word.kerning = 0;	
    		LCD_SetString("授时完成",&word);		
			LCD_SetPicture(120 - 12 - img_sport_save_ok.height, LCD_CENTER_JUSTIFIED, LCD_CYAN, LCD_NONE, &img_sport_save_ok);
			 for (i = 1;i< 61;i++)
          	{
				if((i%5)==0)
				{
				 if(i>=10)
					{
	                   
					   LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 14, 360*(i-5)/GUI_TIME_UTC_TIMEOUT, 360*(i-1)/GUI_TIME_UTC_TIMEOUT, LCD_CYAN);
				    }
				 else
					{
				     LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 14, 0, 360*(i-1)/GUI_TIME_UTC_TIMEOUT, LCD_CYAN);
					}
				}
          	}
			timer_progress_start(1000);

		}break;
		case GUI_TIME_UTC_GET_EVT_FAIL:{
			
			LCD_SetBackgroundColor(LCD_BLACK);
			SetWord_t word = {0};
			word.x_axis = 104;
			word.y_axis = LCD_CENTER_JUSTIFIED;
			word.size = LCD_FONT_32_SIZE;
			word.forecolor = LCD_WHITE;
			word.bckgrndcolor = LCD_BLACK;
			word.kerning = 0;	
			LCD_SetString("授时失败",&word);		
			LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 14, 0, 360, LCD_RED);
			timer_progress_start(1000);
		}break;		
	}

			
/*
			else if(g_gui_time_utc_get_param.evt == GUI_TIME_UTC_GET_EVT_SEARCHING)
			{
//				timer_progress_stop();
//				dev_ublox_utc_read_uninit();
				DISPLAY_MSG  msg = {0,0};
				ScreenState = DISPLAY_SCREEN_TIME;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
			}
*/
//	LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
}

//卫星授时界面按键处理（返回）

void gui_time_utc_get_btn_evt(uint32_t evt)
{	
	switch(evt)
	{		
		case (KEY_BACK):
			{
			timer_progress_stop();
//			timer_progress_uninit();
			mode_close();
			DISPLAY_MSG  msg = {0,0};
			ScreenStateSave = ScreenState;
			ScreenState = DISPLAY_SCREEN_TIME;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);				
			}
			break;			

	}
}

//闹钟界面画图：单个闹钟菜单

static void alarm(Alarm_t *instance1, Alarm_t *instance2, Alarm_t *instance3)
{
	LCD_SetPicture(10, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Up_12X8);
	
	LCD_SetRectangle(120 - 24/2 - 36 - 1 ,2,LCD_LINE_CNT_MAX/2 - 90,180,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
	LCD_SetRectangle(120 + 24/2 + 36 - 1,2,LCD_LINE_CNT_MAX/2 - 90,180,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
	
	SetWord_t word = {0};
	word.x_axis = 32;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_29_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;
	char p_str[20];
	memset(p_str,0,sizeof(p_str));
	sprintf(p_str,"%02d:%02d",instance1->time.hour,instance1->time.minute);
	LCD_SetNumber(p_str,&word);	

	

	//word.x_axis = 90;
	
	word.x_axis = 72+18;
	word.size = LCD_FONT_37_SIZE;
	memset(p_str,0,sizeof(p_str));
	sprintf(p_str,"%02d:%02d",instance2->time.hour,instance2->time.minute);	
	LCD_SetNumber(p_str,&word);	
	if(instance2->enable)
	{
		//word.x_axis = 134;
		
		word.x_axis += 37 + 8;
		word.y_axis = 84;
		word.size = LCD_FONT_16_SIZE;		
		if(SetValue.Language == L_CHINESE)
		{
			switch(instance2->repetition_type)
			{
				case ALARM_REPITION_TYPE_SINGLE:{
					LCD_SetString("开启（单次）",&word);	
				}break;
				case ALARM_REPITION_TYPE_DAILY:{
					LCD_SetString("开启（每日）",&word);	
				}break;
				case ALARM_REPITION_TYPE_WEEK_DAY:{
					LCD_SetString("开启（工作日）",&word);	
				}break;
				case ALARM_REPITION_TYPE_WEEKEND:{
					LCD_SetString("开启（周末）",&word);	
				}break;				
			}		
		}
		else if(SetValue.Language == L_ENGLISH)
		{
			word.y_axis = LCD_CENTER_JUSTIFIED;
			switch(instance2->repetition_type)
			{
				case ALARM_REPITION_TYPE_SINGLE:{
					LCD_SetString("ON(SINGLE)",&word);	
				}break;
				case ALARM_REPITION_TYPE_DAILY:{
					LCD_SetString("ON(DAILY)",&word);	
				}break;
				case ALARM_REPITION_TYPE_WEEK_DAY:{
					LCD_SetString("ON(WEEKDAY)",&word);		
				}break;
				case ALARM_REPITION_TYPE_WEEKEND:{
					LCD_SetString("ON(WEEKEND)",&word);	
				}break;				
			}					
						
		}		
		
	}
	else
	{
		//word.x_axis = 134;
		
		word.x_axis += 37 + 8;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_16_SIZE;		
		if(SetValue.Language == L_CHINESE)
		{
			LCD_SetString("关闭",&word);						
		}
		else if(SetValue.Language == L_ENGLISH)
		{
			LCD_SetString("OFF",&word);			
		}			
			
	}
	
		
	
	word.x_axis = 182;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_29_SIZE;
	memset(p_str,0,sizeof(p_str));
	sprintf(p_str,"%02d:%02d",instance3->time.hour,instance3->time.minute);
	LCD_SetNumber(p_str,&word);		

	
	LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Down_12X8);
}

//闹钟界面画图：闹钟选择菜单 （手表一次只能显示3个界面）

void gui_time_alarm_paint(void)
{
	//读取最新闹钟状态
	memcpy(g_alarm_instance,SetValue.alarm_instance,sizeof(SetValue.alarm_instance));
	
	LCD_SetBackgroundColor(LCD_BLACK);


	switch(m_gui_time_alarm_index)
	{
		case 0:{
			alarm(&g_alarm_instance[3],&g_alarm_instance[0],&g_alarm_instance[1]);			
		}break;
		case 1:{
			alarm(&g_alarm_instance[0],&g_alarm_instance[1],&g_alarm_instance[2]);					
		}break;		
		case 2:{
			alarm(&g_alarm_instance[1],&g_alarm_instance[2],&g_alarm_instance[3]);			
		}break;
		case 3:{
			alarm(&g_alarm_instance[2],&g_alarm_instance[3],&g_alarm_instance[0]);				
		}break;	
	}		
//	LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
}

//闹钟选择菜单按键处理

void gui_time_alarm_btn_evt(uint32_t evt)
{
	switch(evt)
	{		
		case (KEY_BACK):{
			m_gui_time_alarm_index = 0;
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_TIME;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
		}break;			
		case (KEY_UP):{
			if(m_gui_time_alarm_index <= 0)
			{
				m_gui_time_alarm_index = 3;
			}else{
				m_gui_time_alarm_index--;
			}
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_ALARM;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}break;	
			
		case (KEY_OK):{
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_ALARM_SET;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
		}break;		
		case (KEY_DOWN):{
			if(m_gui_time_alarm_index >= 3)
			{
				m_gui_time_alarm_index = 0;
			}else{
				m_gui_time_alarm_index++;
			}
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_ALARM;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}break;						
	}
}

//闹钟设置菜单画图：显示标题（设置时间，重复，声音）

static void alarm_set( char* menu1, char* menu2, char* menu3, char* status, const LCD_ImgStruct *pic)
{
	LCD_SetPicture(10, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Up_12X8);
	
	SetWord_t word = {0};
	word.x_axis = 40;			//16号字体对齐线
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_LIGHTGRAY;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;
	LCD_DisplayGTString(menu1,&word);	//更改使用字库字体	
	
		
	
	word.x_axis = 94;		//24号字体对齐线
	word.forecolor = LCD_WHITE;
	word.size = LCD_FONT_24_SIZE;
	LCD_DisplayGTString(menu2,&word);	//更改使用字库字体	
	word.x_axis += 24 + 8;
	word.size = LCD_FONT_16_SIZE;
	if(m_gui_time_alarm_set_index == GUI_TIME_ALARM_SET_INDEX_TIME_SET)
	{
		word.kerning = 1;
		
		word.size = LCD_FONT_29_SIZE;
		LCD_SetNumber(status,&word);
	}
	else

	{
		LCD_SetString(status,&word);
	}
	
	if(pic != NULL)
	{
		LCD_SetPicture(100, 200, LCD_NONE, LCD_NONE, pic);				
	}		
	
	
	
	
	word.x_axis = 186;
	word.forecolor = LCD_LIGHTGRAY;
	word.size = LCD_FONT_16_SIZE;
	LCD_DisplayGTString(menu3,&word);	//更改使用字库字体	
	
	LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Down_12X8);
}

//闹钟设置菜单画图：子界面

void gui_time_alarm_set_paint(void)
{
	LCD_SetBackgroundColor(LCD_BLACK);
	LCD_SetRectangle(120 - 24/2 - 36 - 1 ,2,LCD_LINE_CNT_MAX/2 - 90,180,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
	LCD_SetRectangle(120 + 24/2 + 36 - 1,2,LCD_LINE_CNT_MAX/2 - 90,180,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
  char p_str[23];
	memset(p_str,0,sizeof(p_str));
	switch(m_gui_time_alarm_set_index)
	{
		case GUI_TIME_ALARM_SET_INDEX_TIME_SET:{
			sprintf(p_str,"%02d:%02d",g_alarm_instance[m_gui_time_alarm_index].time.hour,g_alarm_instance[m_gui_time_alarm_index].time.minute);	
			alarm_set((char *)AlarmSetStrs[SetValue.Language][GUI_ALARM_SET_INDEX_MAX - 1],
						(char *)AlarmSetStrs[SetValue.Language][m_gui_time_alarm_set_index],
						(char *)AlarmSetStrs[SetValue.Language][m_gui_time_alarm_set_index + 1],
							p_str,NULL);					
		}break;
		case GUI_TIME_ALARM_SET_INDEX_BUZZER_SET:{
			
			if(g_alarm_instance[m_gui_time_alarm_index].buzzer_enable && !g_alarm_instance[m_gui_time_alarm_index].motor_enable)
			{
				if(SetValue.Language == L_CHINESE)
				{
					sprintf(p_str,"声音");					
				}
				else if(SetValue.Language == L_ENGLISH)
				{
					sprintf(p_str,"SOUND");		
				}						
			}
			else if(!g_alarm_instance[m_gui_time_alarm_index].buzzer_enable && g_alarm_instance[m_gui_time_alarm_index].motor_enable)
			{
				if(SetValue.Language == L_CHINESE)
				{
					sprintf(p_str,"震动");					
				}
				else if(SetValue.Language == L_ENGLISH)
				{
					sprintf(p_str,"VIBERATE");		
				}					
			}
			else if(g_alarm_instance[m_gui_time_alarm_index].buzzer_enable && g_alarm_instance[m_gui_time_alarm_index].motor_enable)
			{
				if(SetValue.Language == L_CHINESE)
				{
					sprintf(p_str,"声音加震动");					
				}
				else if(SetValue.Language == L_ENGLISH)
				{
					sprintf(p_str,"SOUND AND VIBERATE");		
				}				
			}			
			alarm_set((char *)AlarmSetStrs[SetValue.Language][m_gui_time_alarm_set_index - 1],
						(char *)AlarmSetStrs[SetValue.Language][m_gui_time_alarm_set_index],
						(char *)AlarmSetStrs[SetValue.Language][m_gui_time_alarm_set_index + 1],
							p_str,NULL);									
		}break;		
		case GUI_TIME_ALARM_SET_INDEX_REPITION_SET:{
			switch(g_alarm_instance[m_gui_time_alarm_index].repetition_type)
			{
				case ALARM_REPITION_TYPE_SINGLE:{
					if(SetValue.Language == L_CHINESE)
					{
						sprintf(p_str,"单次");					
					}
					else if(SetValue.Language == L_ENGLISH)
					{
						sprintf(p_str,"SINGLE");		
					}					
				}break;
				case ALARM_REPITION_TYPE_DAILY:{
					if(SetValue.Language == L_CHINESE)
					{
						sprintf(p_str,"每日");					
					}
					else if(SetValue.Language == L_ENGLISH)
					{
						sprintf(p_str,"DAILY");		
					}				
				}break;
				case ALARM_REPITION_TYPE_WEEK_DAY:{
					if(SetValue.Language == L_CHINESE)
					{
						sprintf(p_str,"工作日");					
					}
					else if(SetValue.Language == L_ENGLISH)
					{
						sprintf(p_str,"WEEK DAY");		
					}				
				}break;
				case ALARM_REPITION_TYPE_WEEKEND:{
					if(SetValue.Language == L_CHINESE)
					{
						sprintf(p_str,"周末");					
					}
					else if(SetValue.Language == L_ENGLISH)
					{
						sprintf(p_str,"WEEKEND");		
					}				
				}break;				
			}
			
			alarm_set((char *)AlarmSetStrs[SetValue.Language][m_gui_time_alarm_set_index - 1],
						(char *)AlarmSetStrs[SetValue.Language][m_gui_time_alarm_set_index],
						(char *)AlarmSetStrs[SetValue.Language][m_gui_time_alarm_set_index + 1],
							p_str,NULL);						
		}break;
		case GUI_TIME_ALARM_SET_INDEX_ENABLE_SET:{
			if(g_alarm_instance[m_gui_time_alarm_index].enable)
			{
				if(SetValue.Language == L_CHINESE)
				{
					sprintf(p_str,"开启");					
				}
				else if(SetValue.Language == L_ENGLISH)
				{
					sprintf(p_str,"ON");		
				}			
				alarm_set((char *)AlarmSetStrs[SetValue.Language][m_gui_time_alarm_set_index - 1],
							(char *)AlarmSetStrs[SetValue.Language][m_gui_time_alarm_set_index],
							(char *)AlarmSetStrs[SetValue.Language][0],
								p_str, &img_v_switch_on);	
				
			}
			else{
				if(SetValue.Language == L_CHINESE)
				{
					sprintf(p_str,"关闭");					
				}
				else if(SetValue.Language == L_ENGLISH)
				{
					sprintf(p_str,"OFF");		
				}				
			alarm_set((char *)AlarmSetStrs[SetValue.Language][m_gui_time_alarm_set_index - 1],
						(char *)AlarmSetStrs[SetValue.Language][m_gui_time_alarm_set_index],
						(char *)AlarmSetStrs[SetValue.Language][0],
							p_str, &img_v_switch_off);					
				
			}
							
		}break;	
	}		
	
		
//	LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
}

void gui_time_alarm_set_btn_evt(uint32_t evt)
{
	switch(evt)
	{		
		case (KEY_BACK):{
			m_gui_time_alarm_set_index = 0;
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_ALARM;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);						
		}break;			
		case (KEY_UP):{
			if(m_gui_time_alarm_set_index <= 0)
			{
				m_gui_time_alarm_set_index = 3;
			}else{
				m_gui_time_alarm_set_index--;
			}
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_ALARM_SET;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
		}break;	
		case (KEY_OK):{
			switch(m_gui_time_alarm_set_index)
			{
				case GUI_TIME_ALARM_SET_INDEX_TIME_SET:{		
					m_gui_time_alarm_set_time_index = 0;
					memcpy(&m_alarm_time,&g_alarm_instance[m_gui_time_alarm_index].time,sizeof(m_alarm_time));
					DISPLAY_MSG  msg = {0,0};
					ScreenState = DISPLAY_SCREEN_ALARM_SET_TIME;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);					
				}break;
				case GUI_TIME_ALARM_SET_INDEX_BUZZER_SET:{
					if((g_alarm_instance[m_gui_time_alarm_index].buzzer_enable == true) 
						&& (g_alarm_instance[m_gui_time_alarm_index].motor_enable == false))
					{
						//声音
						m_gui_time_alarm_set_sound_N_viberate_index = GUI_TIME_ALARM_SET_SOUND_N_VIBERATE_SOUND;
					}
					else if((g_alarm_instance[m_gui_time_alarm_index].buzzer_enable == false) 
							&& (g_alarm_instance[m_gui_time_alarm_index].motor_enable == true))
					{
						//震动
						m_gui_time_alarm_set_sound_N_viberate_index = GUI_TIME_ALARM_SET_SOUND_N_VIBERATE_VIBERATE;
					}
					else
					{
						//声音加震动
						m_gui_time_alarm_set_sound_N_viberate_index = GUI_TIME_ALARM_SET_SOUND_N_VIBERATE_SOUND_N_VIBERATE;
					}
					
					DISPLAY_MSG  msg = {0,0};
					ScreenState = DISPLAY_SCREEN_ALARM_SET_SOUND_N_VIBERATE;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);															
				}break;		
				case GUI_TIME_ALARM_SET_INDEX_REPITION_SET:{
					if(g_alarm_instance[m_gui_time_alarm_index].repetition_type >= GUI_ALARM_SET_REPITION_INDEX_MAX)
					{
						m_gui_time_alarm_set_repition_index = 0;
					}
					else
					{
						m_gui_time_alarm_set_repition_index = g_alarm_instance[m_gui_time_alarm_index].repetition_type;
					}
									
					DISPLAY_MSG  msg = {0,0};
					ScreenState = DISPLAY_SCREEN_ALARM_SET_REPITION;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);		
				}break;
				case GUI_TIME_ALARM_SET_INDEX_ENABLE_SET:{
					g_alarm_instance[m_gui_time_alarm_index].enable = !g_alarm_instance[m_gui_time_alarm_index].enable;
					memcpy(SetValue.alarm_instance,g_alarm_instance,sizeof(SetValue.alarm_instance));
					DISPLAY_MSG  msg = {0,0};
					ScreenState = DISPLAY_SCREEN_ALARM_SET;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
				}break;	
			}					
		}break;		
		case (KEY_DOWN):{
			if(m_gui_time_alarm_set_index >= 3)
			{
				m_gui_time_alarm_set_index = 0;
			}else{
				m_gui_time_alarm_set_index++;
			}
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_ALARM_SET;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
		}break;					
	}
}

//闹钟时间设置界面画图：设置时间

static void alarm_set_time(Alarm_time_t alarm_time)
{
	SetWord_t word = {0};
	word.x_axis = LCD_PIXEL_ONELINE_MAX/2 - Font_Number_56.height/2;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_56_SIZE;
	
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;
	
    char p_str[23];
	memset(p_str,0,sizeof(p_str));	
	sprintf(p_str,"%02d:%02d",alarm_time.hour,alarm_time.minute);
	LCD_SetNumber(p_str,&word);

	if(m_gui_time_alarm_set_time_index == GUI_TIME_ALARM_SET_TIME_HOUR)
	{
		word.x_axis = LCD_PIXEL_ONELINE_MAX/2 + Font_Number_56.height/2 + 4;
		word.y_axis = LCD_PIXEL_ONELINE_MAX/2 - Font_Number_56.width*2 - 8;
		LCD_SetRectangle(word.x_axis,4,word.y_axis,Font_Number_56.width*2,LCD_CYAN,0,0,LCD_FILL_ENABLE);	
	}
	else if(m_gui_time_alarm_set_time_index == GUI_TIME_ALARM_SET_TIME_MINUTE)
	{
		word.x_axis = LCD_PIXEL_ONELINE_MAX/2 + Font_Number_56.height/2 + 4;
		word.y_axis = LCD_PIXEL_ONELINE_MAX/2 + 8;
		LCD_SetRectangle(word.x_axis,4,word.y_axis,Font_Number_56.width*2,LCD_CYAN,0,0,LCD_FILL_ENABLE);	
	}

}

//闹钟时间设置界面画图：主界面

void gui_time_alarm_set_time_paint(void)
{
	LCD_SetBackgroundColor(LCD_BLACK);
	
	SetWord_t word = {0};
	word.x_axis = 20;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;
	if(SetValue.Language == L_CHINESE)
	{
		LCD_SetString("设置时间",&word);						
	}
	else if(SetValue.Language == L_ENGLISH)
	{
		LCD_SetString("TIME SET",&word);		
	}		
	
	
	alarm_set_time(m_alarm_time);
	
//	LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
}

//闹钟时间设置界面按键处理

void gui_time_alarm_set_time_btn_evt(uint32_t evt)
{
	switch(evt)
	{		
		case (KEY_BACK):{
			if(m_gui_time_alarm_set_time_index == GUI_TIME_ALARM_SET_TIME_MINUTE)
			{
				m_gui_time_alarm_set_time_index = GUI_TIME_ALARM_SET_TIME_HOUR;
				DISPLAY_MSG  msg = {0,0};
				ScreenState = DISPLAY_SCREEN_ALARM_SET_TIME;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);				
			}else{
				DISPLAY_MSG  msg = {0,0};
				ScreenState = DISPLAY_SCREEN_ALARM_SET;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);				
			}
					
		}break;			
		case (KEY_UP):{
			if(m_gui_time_alarm_set_time_index == GUI_TIME_ALARM_SET_TIME_HOUR)
			{
				if(m_alarm_time.hour >= 23)
				{
					m_alarm_time.hour = 0;
				}else{
					m_alarm_time.hour++;
				}								
			}
			else if(m_gui_time_alarm_set_time_index == GUI_TIME_ALARM_SET_TIME_MINUTE)
			{
				if(m_alarm_time.minute >= 59)
				{
					m_alarm_time.minute = 0;
				}else{
					m_alarm_time.minute++;
				}				
			}				
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_ALARM_SET_TIME;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}break;	
		case (KEY_OK):{
			if(m_gui_time_alarm_set_time_index == GUI_TIME_ALARM_SET_TIME_HOUR)
			{
				m_gui_time_alarm_set_time_index = GUI_TIME_ALARM_SET_TIME_MINUTE;
				DISPLAY_MSG  msg = {0,0};
				ScreenState = DISPLAY_SCREEN_ALARM_SET_TIME;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);					
			}
			else if(m_gui_time_alarm_set_time_index == GUI_TIME_ALARM_SET_TIME_MINUTE)
			{
				g_alarm_instance[m_gui_time_alarm_index].time = m_alarm_time;
				memcpy(SetValue.alarm_instance,g_alarm_instance,sizeof(SetValue.alarm_instance));
				DISPLAY_MSG  msg = {0,0};
				ScreenState = DISPLAY_SCREEN_ALARM_SET;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);				
			}
			
		}break;		
		case (KEY_DOWN):{
			if(m_gui_time_alarm_set_time_index == GUI_TIME_ALARM_SET_TIME_HOUR)
			{
				if(m_alarm_time.hour <= 0)
				{
					m_alarm_time.hour = 23;
				}else{
					m_alarm_time.hour--;
				}					
			}
			else if(m_gui_time_alarm_set_time_index == GUI_TIME_ALARM_SET_TIME_MINUTE)
			{
				if(m_alarm_time.minute <= 0)
				{
					m_alarm_time.minute = 59;
				}else{
					m_alarm_time.minute--;
				}										
			}					
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_ALARM_SET_TIME;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);					
		}break;					
	}

}

//闹钟响铃设置界面画图：字符串

static void alarm_set_sound_N_viberate( char* menu1, char* menu2, char* menu3)
{
	LCD_SetRectangle(120 - 24/2 - 36 - 1 ,2,LCD_LINE_CNT_MAX/2 - 90,180,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
	LCD_SetRectangle(120 + 24/2 + 36 - 1,2,LCD_LINE_CNT_MAX/2 - 90,180,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
	LCD_SetPicture(10, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Up_12X8);
	
	SetWord_t word = {0};
	//	word.x_axis = 26; //24号字体对齐线
	word.x_axis = 40;			//16号字体对齐线
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;
	
//	LCD_SetString(menu1,&word);	
	LCD_DisplayGTString(menu1,&word);	//更改使用字库字体
	
	
		
	//	word.x_axis = 100; //32号字体对齐线		
	word.x_axis = 108;	 //24号字体对齐线
	word.size = LCD_FONT_24_SIZE;
//	LCD_SetString(menu2,&word);	
	LCD_DisplayGTString(menu2,&word);	//更改使用字库字体
	
	
	
	word.x_axis = 186;	
	word.size = LCD_FONT_16_SIZE;
//	LCD_SetString(menu3,&word);		
	LCD_DisplayGTString(menu3,&word);	//更改使用字库字体
	
	LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Down_12X8);

}

//闹钟响铃设置界面画图：菜单索引与字符串显示
void gui_time_alarm_set_sound_N_viberate_paint(void)
{
	LCD_SetBackgroundColor(LCD_BLACK);
	
	switch(m_gui_time_alarm_set_sound_N_viberate_index)
	{
		case GUI_TIME_ALARM_SET_SOUND_N_VIBERATE_SOUND_N_VIBERATE:{
			alarm_set_sound_N_viberate((char *)AlarmSetSoundNViberateStrs[SetValue.Language][GUI_ALARM_SET_SOUND_N_VIBERATE_INDEX_MAX - 1],
						(char *)AlarmSetSoundNViberateStrs[SetValue.Language][m_gui_time_alarm_set_sound_N_viberate_index],
						(char *)AlarmSetSoundNViberateStrs[SetValue.Language][m_gui_time_alarm_set_sound_N_viberate_index + 1]);			
		}break;
		case GUI_TIME_ALARM_SET_SOUND_N_VIBERATE_SOUND:{
			alarm_set_sound_N_viberate((char *)AlarmSetSoundNViberateStrs[SetValue.Language][m_gui_time_alarm_set_sound_N_viberate_index - 1],
						(char *)AlarmSetSoundNViberateStrs[SetValue.Language][m_gui_time_alarm_set_sound_N_viberate_index],
						(char *)AlarmSetSoundNViberateStrs[SetValue.Language][m_gui_time_alarm_set_sound_N_viberate_index + 1]);			
		}break;
		case GUI_TIME_ALARM_SET_SOUND_N_VIBERATE_VIBERATE:{
			alarm_set_sound_N_viberate((char *)AlarmSetSoundNViberateStrs[SetValue.Language][m_gui_time_alarm_set_sound_N_viberate_index - 1],
						(char *)AlarmSetSoundNViberateStrs[SetValue.Language][m_gui_time_alarm_set_sound_N_viberate_index],
						(char *)AlarmSetSoundNViberateStrs[SetValue.Language][0]);			
		}break;		
	}		
	
//	LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
}

//闹钟响铃设置界面按键处理

void gui_time_alarm_set_sound_N_viberate_btn_evt(uint32_t evt)
{
	switch(evt)
	{		
		case (KEY_BACK):{
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_ALARM_SET;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);						
		}break;			
		case (KEY_UP):{
			if(m_gui_time_alarm_set_sound_N_viberate_index <= 0)
			{
				m_gui_time_alarm_set_sound_N_viberate_index = GUI_ALARM_SET_SOUND_N_VIBERATE_INDEX_MAX - 1;
			}else{
				m_gui_time_alarm_set_sound_N_viberate_index--;
			}
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_ALARM_SET_SOUND_N_VIBERATE;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
		}break;	
		case (KEY_OK):{
			switch(m_gui_time_alarm_set_sound_N_viberate_index)
			{
				case GUI_TIME_ALARM_SET_SOUND_N_VIBERATE_SOUND_N_VIBERATE:{
					g_alarm_instance[m_gui_time_alarm_index].buzzer_enable = true;			
					g_alarm_instance[m_gui_time_alarm_index].motor_enable = true;	
				}break;
				case GUI_TIME_ALARM_SET_SOUND_N_VIBERATE_SOUND:{
					g_alarm_instance[m_gui_time_alarm_index].buzzer_enable = true;			
					g_alarm_instance[m_gui_time_alarm_index].motor_enable = false;								
				}break;
				case GUI_TIME_ALARM_SET_SOUND_N_VIBERATE_VIBERATE:{
					g_alarm_instance[m_gui_time_alarm_index].buzzer_enable = false;			
					g_alarm_instance[m_gui_time_alarm_index].motor_enable = true;						
				}break;		
			}									
			memcpy(SetValue.alarm_instance,g_alarm_instance,sizeof(SetValue.alarm_instance));
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_ALARM_SET;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
		}break;		
		case (KEY_DOWN):{
			if(m_gui_time_alarm_set_sound_N_viberate_index >= ( GUI_ALARM_SET_SOUND_N_VIBERATE_INDEX_MAX - 1))
			{
				m_gui_time_alarm_set_sound_N_viberate_index = 0;
			}else{
				m_gui_time_alarm_set_sound_N_viberate_index++;
			}
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_ALARM_SET_SOUND_N_VIBERATE;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);								
		}break;					
	}

}

//闹钟重复界面画图：字符串处理

static void alarm_set_repition( char* menu1, char* menu2, char* menu3)
{
     LCD_SetRectangle(120 - 24/2 - 36 - 1 ,2,LCD_LINE_CNT_MAX/2 - 90,180,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
	LCD_SetRectangle(120 + 24/2 + 36 - 1,2,LCD_LINE_CNT_MAX/2 - 90,180,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
	LCD_SetPicture(10, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Up_12X8);
	
	SetWord_t word = {0};
	//	word.x_axis = 26; //24号字体对齐线
	word.x_axis = 40;			//16号字体对齐线
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;
	
//	LCD_SetString(menu1,&word);	
	LCD_DisplayGTString(menu1,&word);	//更改使用字库字体
	//word.x_axis = 80;
	
	//LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,FOREGROUND_COLOR,0,0,LCD_FILL_ENABLE);
		
//	word.x_axis = 100; //32号字体对齐线		
	word.x_axis = 108;	 //24号字体对齐线
	word.size = LCD_FONT_24_SIZE;
//	LCD_SetString(menu2,&word);	
	LCD_DisplayGTString(menu2,&word);	//更改使用字库字体

	//word.x_axis = 160;
	//LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,FOREGROUND_COLOR,0,0,LCD_FILL_ENABLE);	
	
	word.x_axis = 186;	
	word.size = LCD_FONT_16_SIZE;
	LCD_DisplayGTString(menu3,&word);	//更改使用字库字体
//	LCD_SetString(menu3,&word);		
	
	LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Down_12X8);

}

//闹钟重复界面画图：逻辑

void gui_time_alarm_set_repition_paint(void)
{
	LCD_SetBackgroundColor(LCD_BLACK);
	
	switch(m_gui_time_alarm_set_repition_index)
	{
		case ALARM_REPITION_TYPE_SINGLE:{
			alarm_set_repition((char *)AlarmSetRepitionStrs[SetValue.Language][GUI_ALARM_SET_REPITION_INDEX_MAX - 1],
						(char *)AlarmSetRepitionStrs[SetValue.Language][m_gui_time_alarm_set_repition_index],
						(char *)AlarmSetRepitionStrs[SetValue.Language][m_gui_time_alarm_set_repition_index + 1]);			
		}break;
		case ALARM_REPITION_TYPE_DAILY:{
			alarm_set_repition((char *)AlarmSetRepitionStrs[SetValue.Language][m_gui_time_alarm_set_repition_index - 1],
						(char *)AlarmSetRepitionStrs[SetValue.Language][m_gui_time_alarm_set_repition_index],
						(char *)AlarmSetRepitionStrs[SetValue.Language][m_gui_time_alarm_set_repition_index + 1]);			
		}break;
		case ALARM_REPITION_TYPE_WEEK_DAY:{
			alarm_set_repition((char *)AlarmSetRepitionStrs[SetValue.Language][m_gui_time_alarm_set_repition_index - 1],
						(char *)AlarmSetRepitionStrs[SetValue.Language][m_gui_time_alarm_set_repition_index],
						(char *)AlarmSetRepitionStrs[SetValue.Language][m_gui_time_alarm_set_repition_index + 1]);			
		}break;
		case ALARM_REPITION_TYPE_WEEKEND:{
			alarm_set_repition((char *)AlarmSetRepitionStrs[SetValue.Language][m_gui_time_alarm_set_repition_index - 1],
						(char *)AlarmSetRepitionStrs[SetValue.Language][m_gui_time_alarm_set_repition_index],
						(char *)AlarmSetRepitionStrs[SetValue.Language][0]);			
		}break;		
	}			
	
//	LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
}

//闹钟重复界面按键处理

void gui_time_alarm_set_repition_btn_evt(uint32_t evt)
{
	switch(evt)
	{		
		case (KEY_BACK):{
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_ALARM_SET;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);						
		}break;			
		case (KEY_UP):{
			if(m_gui_time_alarm_set_repition_index <= 0)
			{
				m_gui_time_alarm_set_repition_index = GUI_ALARM_SET_REPITION_INDEX_MAX - 1;
			}else{
				m_gui_time_alarm_set_repition_index--;
			}
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_ALARM_SET_REPITION;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);					
		}break;	
		case (KEY_OK):{
			g_alarm_instance[m_gui_time_alarm_index].repetition_type = m_gui_time_alarm_set_repition_index;		
			memcpy(SetValue.alarm_instance,g_alarm_instance,sizeof(SetValue.alarm_instance));
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_ALARM_SET;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
		}break;		
		case (KEY_DOWN):{
			if(m_gui_time_alarm_set_repition_index >= ( GUI_ALARM_SET_REPITION_INDEX_MAX - 1))
			{
				m_gui_time_alarm_set_repition_index = 0;
			}else{
				m_gui_time_alarm_set_repition_index++;
			}
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_ALARM_SET_REPITION;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);												
		}break;					
	}

}

#if defined WATCH_TIMEZONE_SET
/*
函数说明:根据不同的国家,显示各国家地区缩写、时区信息
入参:国家  
     TIMEZONE_ZERO 0时区
	
返回值:地区缩写、时区字符串

*/

char* get_timezone_string(uint16_t country)
{
	char* value = "";
	
	switch(country)
	{
	case TIMEZONE_MDY:
		value = "MDY一-11:00";
		break;
	case TIMEZONE_HNL:
		value = "HNL一-10:00";
		break;
	case TIMEZONE_ANC:
		value = "ANC一-09:00";
		break;
	case TIMEZONE_LAX:
		value = "LAX一-08:00";
		break;
	case TIMEZONE_DEN:
		value = "DEN一-07:00";
		break;
	case TIMEZONE_CHI:
		value = "CHI一-06:00";
		break;
	case TIMEZONE_NYC:
		value = "NYC一-05:00";
		break;
	case TIMEZONE_CCS:
		value = "CCS一-04:30";
		break;
	case TIMEZONE_SCL:
		value = "SCL一-04:00";
		break;
	case TIMEZONE_YYT:
		value = "YYT一-03:30";
		break;
	case TIMEZONE_RIO:
		value ="RIO一-03:00";
		break;
	case TIMEZONE_FEN:
		value = "FEN一-02:00";
		break;
	case TIMEZONE_PDL:
		value = "PDL一-01:00";
		break;
	case TIMEZONE_ZERO://零时区,伦敦时间
		value = "LON一00:00" ;
		break;
	case TIMEZONE_PAR:
		value = "PAR一+01:00";
		break;
	case TIMEZONE_CAI:
		value = "CAI一+02:00";
		break;
	case TIMEZONE_MOW:
		value = "MOW一+03:00";
		break;
	case TIMEZONE_THR:
		value = "THR一+03:30";
		break;
	case TIMEZONE_DXB:
		value = "DXB一+04:00";
		break;
	case TIMEZONE_KBL:
		value = "KBL一+04:30";
		break;
	case TIMEZONE_KHI:
		value = "KHI一+05:00";
		break;
	case TIMEZONE_DEL:
		value ="DEL一+05:30";
		break;
	case TIMEZONE_KTM:
		value = "KTM一+05:45";
		break;
	case TIMEZONE_DAC:
		value = "DAC一+06:00";
		break;
	case TIMEZONE_RGN:
		value = "RGN一+06:30";
		break;
	case TIMEZONE_BKK:
		value = "BKK一+07:00";
		break;
	case TIMEZONE_BJS:
		value = "BJS一+08:00";
		break;
	case TIMEZONE_TYO:
		value = "TYO一+09:00";
		break;
	case TIMEZONE_ADL:
		value ="ADL一+09:30";
		break;
	case TIMEZONE_SYD:
		value = "SYD一+10:00";
		break;
	case TIMEZONE_NOU:
		value = "NOU一+11:00";
		break;
	case TIMEZONE_WLG:
		value = "WLG一+12:00";
		break;
		default://默认北京时间
			value = "BJS一+08:00";
			break;
	}
	
	
	return value;
}





/*函数说明:选择时区界面*/
static void time_zone_paint( char* menu1, char* menu2, char* menu3,uint16_t country)
{
	SetWord_t word = {0};
	char str[30];
	//	word.x_axis = 26; //24号字体对齐线
	word.x_axis = 40;			//16号字体对齐线
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = BACKGROUND_COLOR;
	word.bckgrndcolor = FOREGROUND_COLOR;
	word.kerning = 0;
	
	LCD_SetRectangle(0,80,0,LCD_LINE_CNT_MAX,FOREGROUND_COLOR,0,0,LCD_FILL_ENABLE);
		
	LCD_SetString(menu1,&word);
	
	word.x_axis = 80;
	word.forecolor = FOREGROUND_COLOR;
	word.bckgrndcolor = BACKGROUND_COLOR;	
	LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,FOREGROUND_COLOR,0,0,LCD_FILL_ENABLE);
		
	word.x_axis = 97;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString(menu2,&word);
	memset(str,0,30);
	sprintf(str,"%s",get_timezone_string(country));
	word.x_axis += 24 + 6;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString(str,&word);
	
	word.x_axis = 160;
	LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,FOREGROUND_COLOR,0,0,LCD_FILL_ENABLE);	
	
	word.x_axis = 186;	
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString(menu3,&word);
	

	LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, LCD_BLACK, LCD_NONE, &Img_Pointing_Down_12X8);

}


/*界面显示:世界时钟 时区选择子界面*/
void gui_time_zone_set_paint(void)
{
	LCD_SetBackgroundColor(BACKGROUND_COLOR);
	
	
	if(m_time_zone_select_index == (GUI_TIME_ZONE_COUNTIES_INDEX_MAX-1) && GUI_TIME_ZONE_COUNTIES_INDEX_MAX > 1)
	{
		time_zone_paint((char *)TimeStrs[SetValue.Language][0],
					(char *)TimeZoneCountiesStrs[SetValue.Language][m_time_zone_select_index],(char *)TimeZoneCountiesStrs[SetValue.Language][0],m_time_zone_select_index);		
	}
	else if((m_time_zone_select_index < GUI_TIME_ZONE_COUNTIES_INDEX_MAX-1) )
	{
		time_zone_paint((char *)TimeStrs[SetValue.Language][0],
					(char *)TimeZoneCountiesStrs[SetValue.Language][m_time_zone_select_index],
					(char *)TimeZoneCountiesStrs[SetValue.Language][m_time_zone_select_index+1],m_time_zone_select_index);		
	}
}

/*界面显示:世界时钟 时区选择子界面*/
void gui_time_zone_select_paint(void)
{
		
	SetWord_t word = {0};
	uint8_t index = m_time_zone_yesno_index;
	//上半部分为白色背景
	LCD_SetRectangle(0, 120, 0, LCD_PIXEL_ONELINE_MAX, LCD_WHITE, 0, 0, LCD_FILL_ENABLE);
	
	//放弃?
	word.x_axis = 48;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_BLACK;
	word.kerning = 0;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString((char *)TimeZoneConfirmStrs[SetValue.Language][0],&word);
    
	
	//下半部分为黑色背景
	LCD_SetRectangle(120, 120, 0, LCD_PIXEL_ONELINE_MAX, LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
	

		//菜单第一项
	word.x_axis = 133;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 0;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString((char *)CancelStrs[SetValue.Language][index],&word);
	
	//菜单第二项
	if(index == 1)
	{
		index = 0;
	}
	else
	{
		index = 1;
	}
	word.x_axis = 183;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString((char *)CancelStrs[SetValue.Language][index],&word);
	
	//分割线
	LCD_SetRectangle(170, 1, 0, LCD_LINE_CNT_MAX, LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);

	gui_page_down(LCD_WHITE);
}

/*界面显示:世界时钟 时区选择子界面*/
void gui_time_zone_set_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0,0};

	switch(Key_Value)
	{		
		case (KEY_BACK):{
			ScreenState = DISPLAY_SCREEN_TIME;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}break;
		case (KEY_OK):{
			m_time_zone_yesno_index = 1;
			ScreenState = DISPLAY_SCREEN_TIMEZONE_SELECT;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}break;
		case (KEY_UP):{
			if(m_time_zone_select_index == 0)
			{
				m_time_zone_select_index = (GUI_TIME_ZONE_COUNTIES_INDEX_MAX-1);
			}
			else
			{
				m_time_zone_select_index--;
			}
			
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}break;
		case (KEY_DOWN):{
			if(m_time_zone_select_index >= (GUI_TIME_ZONE_COUNTIES_INDEX_MAX-1))
			{
				m_time_zone_select_index =0;
			}
			else
			{
				m_time_zone_select_index++;
			}
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}break;
		default:
			break;
	}
}

void gui_time_zone_select_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
			if(m_time_zone_yesno_index == 0)
			{
				//返回上级菜单界面
				ScreenState = DISPLAY_SCREEN_TIMEZONE_SET;
	
			}
			else
			{
				//设置时区返回主界面
				am_hal_rtc_time_t m_before_time,m_now_time; //更改前RTC时间和要设置到RTC的时间
				am_hal_rtc_time_get(&RTC_time);
				m_before_time = RTC_time;
				//更改时区时间
				RTC_time = get_local_time(m_before_time,SetValue.TimeZoneRecord,m_time_zone_select_index);
				m_now_time = RTC_time;
				am_hal_rtc_time_set(&RTC_time);
				RenewOneDay( m_before_time,m_now_time,true);
				//保存时区选择
				SetValue.TimeZoneRecord = m_time_zone_select_index;
				
				ScreenState = DISPLAY_SCREEN_HOME;	
				
			}
			
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		case KEY_UP:
		case KEY_DOWN:
			if(m_time_zone_yesno_index == 0)
			{
				m_time_zone_yesno_index = 1;
			}
			else
			{
				m_time_zone_yesno_index = 0;
			}

			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		    break;
		case KEY_BACK:
			//返回上级菜单
			ScreenState = DISPLAY_SCREEN_TIMEZONE_SET;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}

#endif
