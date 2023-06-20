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


//�������Ӹ�����һ��4������
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


//ʱ�����ò˵��ṹ�壬��Ϊ���������е��������ظ���ʱ�������˵��ṹ���ƣ������ýṹ��ָ�뷽��������Ķ���

static void time_paint( char* menu1, char* menu2, char* menu3)
{
	SetWord_t word = {0};
  /*  word.x_axis = 120 - 24/2 - 36 - 24 - 24;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_CYAN;
	word.bckgrndcolor = LCD_NONE;
	word.size = LCD_FONT_24_SIZE;
	word.kerning = 0;
	LCD_DisplayGTString(menu1,&word);	//ʱ��
	LCD_SetPicture( word.x_axis,120 - 2*LCD_FONT_24_SIZE/2 - 8 - img_set_menu_b[1].width,LCD_RED,LCD_NONE,&img_set_menu_b[1]);//ʱ��ͼ��

	
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

//ʱ�����ý��滭ͼ�������棬���������ӽ����Լ�������ʱ�ӽ���

void gui_time_paint(void)
{
	LCD_SetBackgroundColor(LCD_BLACK);

	switch(m_time_index + DISPLAY_SCREEN_UTC_GET)
	{
		case DISPLAY_SCREEN_UTC_GET:{
			if(SetValue.Language == L_CHINESE)
			{
				time_paint("ʱ��",
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
				time_paint("ʱ��",
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

//ʱ�����ý��棺�����水�����������ӽ�����������ʱ�ӽ���������л�
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

//UTCʱ���ʼ�����������ԭ��ʱ������

void gui_time_utc_get_init(void)
{
	//�������
	memset(&g_gui_time_utc_get_param,0,sizeof(_gui_time_utc_get_param));
}

//������ʱ���滭ͼ

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
			LCD_SetString("������ʱ��",&word);

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
			else//��5��ʼ
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
    		LCD_SetString("��ʱ���",&word);		
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
			LCD_SetString("��ʱʧ��",&word);		
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

//������ʱ���水���������أ�

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

//���ӽ��滭ͼ���������Ӳ˵�

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
					LCD_SetString("���������Σ�",&word);	
				}break;
				case ALARM_REPITION_TYPE_DAILY:{
					LCD_SetString("������ÿ�գ�",&word);	
				}break;
				case ALARM_REPITION_TYPE_WEEK_DAY:{
					LCD_SetString("�����������գ�",&word);	
				}break;
				case ALARM_REPITION_TYPE_WEEKEND:{
					LCD_SetString("��������ĩ��",&word);	
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
			LCD_SetString("�ر�",&word);						
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

//���ӽ��滭ͼ������ѡ��˵� ���ֱ�һ��ֻ����ʾ3�����棩

void gui_time_alarm_paint(void)
{
	//��ȡ��������״̬
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

//����ѡ��˵���������

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

//�������ò˵���ͼ����ʾ���⣨����ʱ�䣬�ظ���������

static void alarm_set( char* menu1, char* menu2, char* menu3, char* status, const LCD_ImgStruct *pic)
{
	LCD_SetPicture(10, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Up_12X8);
	
	SetWord_t word = {0};
	word.x_axis = 40;			//16�����������
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_LIGHTGRAY;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;
	LCD_DisplayGTString(menu1,&word);	//����ʹ���ֿ�����	
	
		
	
	word.x_axis = 94;		//24�����������
	word.forecolor = LCD_WHITE;
	word.size = LCD_FONT_24_SIZE;
	LCD_DisplayGTString(menu2,&word);	//����ʹ���ֿ�����	
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
	LCD_DisplayGTString(menu3,&word);	//����ʹ���ֿ�����	
	
	LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Down_12X8);
}

//�������ò˵���ͼ���ӽ���

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
					sprintf(p_str,"����");					
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
					sprintf(p_str,"��");					
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
					sprintf(p_str,"��������");					
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
						sprintf(p_str,"����");					
					}
					else if(SetValue.Language == L_ENGLISH)
					{
						sprintf(p_str,"SINGLE");		
					}					
				}break;
				case ALARM_REPITION_TYPE_DAILY:{
					if(SetValue.Language == L_CHINESE)
					{
						sprintf(p_str,"ÿ��");					
					}
					else if(SetValue.Language == L_ENGLISH)
					{
						sprintf(p_str,"DAILY");		
					}				
				}break;
				case ALARM_REPITION_TYPE_WEEK_DAY:{
					if(SetValue.Language == L_CHINESE)
					{
						sprintf(p_str,"������");					
					}
					else if(SetValue.Language == L_ENGLISH)
					{
						sprintf(p_str,"WEEK DAY");		
					}				
				}break;
				case ALARM_REPITION_TYPE_WEEKEND:{
					if(SetValue.Language == L_CHINESE)
					{
						sprintf(p_str,"��ĩ");					
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
					sprintf(p_str,"����");					
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
					sprintf(p_str,"�ر�");					
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
						//����
						m_gui_time_alarm_set_sound_N_viberate_index = GUI_TIME_ALARM_SET_SOUND_N_VIBERATE_SOUND;
					}
					else if((g_alarm_instance[m_gui_time_alarm_index].buzzer_enable == false) 
							&& (g_alarm_instance[m_gui_time_alarm_index].motor_enable == true))
					{
						//��
						m_gui_time_alarm_set_sound_N_viberate_index = GUI_TIME_ALARM_SET_SOUND_N_VIBERATE_VIBERATE;
					}
					else
					{
						//��������
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

//����ʱ�����ý��滭ͼ������ʱ��

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

//����ʱ�����ý��滭ͼ��������

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
		LCD_SetString("����ʱ��",&word);						
	}
	else if(SetValue.Language == L_ENGLISH)
	{
		LCD_SetString("TIME SET",&word);		
	}		
	
	
	alarm_set_time(m_alarm_time);
	
//	LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
}

//����ʱ�����ý��水������

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

//�����������ý��滭ͼ���ַ���

static void alarm_set_sound_N_viberate( char* menu1, char* menu2, char* menu3)
{
	LCD_SetRectangle(120 - 24/2 - 36 - 1 ,2,LCD_LINE_CNT_MAX/2 - 90,180,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
	LCD_SetRectangle(120 + 24/2 + 36 - 1,2,LCD_LINE_CNT_MAX/2 - 90,180,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
	LCD_SetPicture(10, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Up_12X8);
	
	SetWord_t word = {0};
	//	word.x_axis = 26; //24�����������
	word.x_axis = 40;			//16�����������
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;
	
//	LCD_SetString(menu1,&word);	
	LCD_DisplayGTString(menu1,&word);	//����ʹ���ֿ�����
	
	
		
	//	word.x_axis = 100; //32�����������		
	word.x_axis = 108;	 //24�����������
	word.size = LCD_FONT_24_SIZE;
//	LCD_SetString(menu2,&word);	
	LCD_DisplayGTString(menu2,&word);	//����ʹ���ֿ�����
	
	
	
	word.x_axis = 186;	
	word.size = LCD_FONT_16_SIZE;
//	LCD_SetString(menu3,&word);		
	LCD_DisplayGTString(menu3,&word);	//����ʹ���ֿ�����
	
	LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Down_12X8);

}

//�����������ý��滭ͼ���˵��������ַ�����ʾ
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

//�����������ý��水������

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

//�����ظ����滭ͼ���ַ�������

static void alarm_set_repition( char* menu1, char* menu2, char* menu3)
{
     LCD_SetRectangle(120 - 24/2 - 36 - 1 ,2,LCD_LINE_CNT_MAX/2 - 90,180,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
	LCD_SetRectangle(120 + 24/2 + 36 - 1,2,LCD_LINE_CNT_MAX/2 - 90,180,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
	LCD_SetPicture(10, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Up_12X8);
	
	SetWord_t word = {0};
	//	word.x_axis = 26; //24�����������
	word.x_axis = 40;			//16�����������
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;
	
//	LCD_SetString(menu1,&word);	
	LCD_DisplayGTString(menu1,&word);	//����ʹ���ֿ�����
	//word.x_axis = 80;
	
	//LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,FOREGROUND_COLOR,0,0,LCD_FILL_ENABLE);
		
//	word.x_axis = 100; //32�����������		
	word.x_axis = 108;	 //24�����������
	word.size = LCD_FONT_24_SIZE;
//	LCD_SetString(menu2,&word);	
	LCD_DisplayGTString(menu2,&word);	//����ʹ���ֿ�����

	//word.x_axis = 160;
	//LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,FOREGROUND_COLOR,0,0,LCD_FILL_ENABLE);	
	
	word.x_axis = 186;	
	word.size = LCD_FONT_16_SIZE;
	LCD_DisplayGTString(menu3,&word);	//����ʹ���ֿ�����
//	LCD_SetString(menu3,&word);		
	
	LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Down_12X8);

}

//�����ظ����滭ͼ���߼�

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

//�����ظ����水������

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
����˵��:���ݲ�ͬ�Ĺ���,��ʾ�����ҵ�����д��ʱ����Ϣ
���:����  
     TIMEZONE_ZERO 0ʱ��
	
����ֵ:������д��ʱ���ַ���

*/

char* get_timezone_string(uint16_t country)
{
	char* value = "";
	
	switch(country)
	{
	case TIMEZONE_MDY:
		value = "MDYһ-11:00";
		break;
	case TIMEZONE_HNL:
		value = "HNLһ-10:00";
		break;
	case TIMEZONE_ANC:
		value = "ANCһ-09:00";
		break;
	case TIMEZONE_LAX:
		value = "LAXһ-08:00";
		break;
	case TIMEZONE_DEN:
		value = "DENһ-07:00";
		break;
	case TIMEZONE_CHI:
		value = "CHIһ-06:00";
		break;
	case TIMEZONE_NYC:
		value = "NYCһ-05:00";
		break;
	case TIMEZONE_CCS:
		value = "CCSһ-04:30";
		break;
	case TIMEZONE_SCL:
		value = "SCLһ-04:00";
		break;
	case TIMEZONE_YYT:
		value = "YYTһ-03:30";
		break;
	case TIMEZONE_RIO:
		value ="RIOһ-03:00";
		break;
	case TIMEZONE_FEN:
		value = "FENһ-02:00";
		break;
	case TIMEZONE_PDL:
		value = "PDLһ-01:00";
		break;
	case TIMEZONE_ZERO://��ʱ��,�׶�ʱ��
		value = "LONһ00:00" ;
		break;
	case TIMEZONE_PAR:
		value = "PARһ+01:00";
		break;
	case TIMEZONE_CAI:
		value = "CAIһ+02:00";
		break;
	case TIMEZONE_MOW:
		value = "MOWһ+03:00";
		break;
	case TIMEZONE_THR:
		value = "THRһ+03:30";
		break;
	case TIMEZONE_DXB:
		value = "DXBһ+04:00";
		break;
	case TIMEZONE_KBL:
		value = "KBLһ+04:30";
		break;
	case TIMEZONE_KHI:
		value = "KHIһ+05:00";
		break;
	case TIMEZONE_DEL:
		value ="DELһ+05:30";
		break;
	case TIMEZONE_KTM:
		value = "KTMһ+05:45";
		break;
	case TIMEZONE_DAC:
		value = "DACһ+06:00";
		break;
	case TIMEZONE_RGN:
		value = "RGNһ+06:30";
		break;
	case TIMEZONE_BKK:
		value = "BKKһ+07:00";
		break;
	case TIMEZONE_BJS:
		value = "BJSһ+08:00";
		break;
	case TIMEZONE_TYO:
		value = "TYOһ+09:00";
		break;
	case TIMEZONE_ADL:
		value ="ADLһ+09:30";
		break;
	case TIMEZONE_SYD:
		value = "SYDһ+10:00";
		break;
	case TIMEZONE_NOU:
		value = "NOUһ+11:00";
		break;
	case TIMEZONE_WLG:
		value = "WLGһ+12:00";
		break;
		default://Ĭ�ϱ���ʱ��
			value = "BJSһ+08:00";
			break;
	}
	
	
	return value;
}





/*����˵��:ѡ��ʱ������*/
static void time_zone_paint( char* menu1, char* menu2, char* menu3,uint16_t country)
{
	SetWord_t word = {0};
	char str[30];
	//	word.x_axis = 26; //24�����������
	word.x_axis = 40;			//16�����������
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


/*������ʾ:����ʱ�� ʱ��ѡ���ӽ���*/
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

/*������ʾ:����ʱ�� ʱ��ѡ���ӽ���*/
void gui_time_zone_select_paint(void)
{
		
	SetWord_t word = {0};
	uint8_t index = m_time_zone_yesno_index;
	//�ϰ벿��Ϊ��ɫ����
	LCD_SetRectangle(0, 120, 0, LCD_PIXEL_ONELINE_MAX, LCD_WHITE, 0, 0, LCD_FILL_ENABLE);
	
	//����?
	word.x_axis = 48;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_BLACK;
	word.kerning = 0;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString((char *)TimeZoneConfirmStrs[SetValue.Language][0],&word);
    
	
	//�°벿��Ϊ��ɫ����
	LCD_SetRectangle(120, 120, 0, LCD_PIXEL_ONELINE_MAX, LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
	

		//�˵���һ��
	word.x_axis = 133;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 0;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString((char *)CancelStrs[SetValue.Language][index],&word);
	
	//�˵��ڶ���
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
	
	//�ָ���
	LCD_SetRectangle(170, 1, 0, LCD_LINE_CNT_MAX, LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);

	gui_page_down(LCD_WHITE);
}

/*������ʾ:����ʱ�� ʱ��ѡ���ӽ���*/
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
				//�����ϼ��˵�����
				ScreenState = DISPLAY_SCREEN_TIMEZONE_SET;
	
			}
			else
			{
				//����ʱ������������
				am_hal_rtc_time_t m_before_time,m_now_time; //����ǰRTCʱ���Ҫ���õ�RTC��ʱ��
				am_hal_rtc_time_get(&RTC_time);
				m_before_time = RTC_time;
				//����ʱ��ʱ��
				RTC_time = get_local_time(m_before_time,SetValue.TimeZoneRecord,m_time_zone_select_index);
				m_now_time = RTC_time;
				am_hal_rtc_time_set(&RTC_time);
				RenewOneDay( m_before_time,m_now_time,true);
				//����ʱ��ѡ��
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
			//�����ϼ��˵�
			ScreenState = DISPLAY_SCREEN_TIMEZONE_SET;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}

#endif
