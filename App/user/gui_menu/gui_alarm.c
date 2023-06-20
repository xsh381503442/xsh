#include "gui_alarm.h"
#include "SEGGER_RTT.h"
#include "drv_key.h"
#include "drv_lcd.h"
#include <string.h>
#include "font_displaystrs.h"
#include "task_display.h"
#include "com_data.h"



#define GUI_ALARM_LOG_ENABLED 1

#if DEBUG_ENABLED == 1 && GUI_ALARM_LOG_ENABLED == 1
	#define GUI_ALARM_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_ALARM_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define GUI_ALARM_WRITESTRING(...)
	#define GUI_ALARM_PRINTF(...)		        
#endif

extern SetValueStr SetValue;
#define BACKGROUND_COLOR      LCD_WHITE
#define FOREGROUND_COLOR      LCD_BLACK


#define GUI_ALARM_SET_INDEX_TIME_SET                  0
#define GUI_ALARM_SET_INDEX_BUZZER_SET                1
#define GUI_ALARM_SET_INDEX_REPITION_SET              2
#define GUI_ALARM_SET_INDEX_ENABLE_SET                3
static uint8_t m_gui_alarm_set_index = 0;
static uint8_t m_gui_alarm_index = 0;
static _alarm_time m_alarm_time = {0,0};
static uint8_t m_gui_alarm_set_time_index = 0;
#define GUI_ALARM_SET_TIME_HOUR           0
#define GUI_ALARM_SET_TIME_MINUTE         1
#define GUI_ALARM_SET_SOUND_N_VIBERATE_SOUND_N_VIBERATE         0
#define GUI_ALARM_SET_SOUND_N_VIBERATE_SOUND                    1
#define GUI_ALARM_SET_SOUND_N_VIBERATE_VIBERATE                 2
static uint8_t m_gui_alarm_set_sound_N_viberate_index = 0;
static uint8_t m_gui_alarm_set_repition_index = 0;

_alarm_instance g_alarm_instance[4] = 
{
	{
		.repetition_type = GUI_ALARM_SET_REPITION_SINGLE,
		.buzzer_enable = true,
		.motor_enable = true,
	},
	{
		.repetition_type = GUI_ALARM_SET_REPITION_SINGLE,		
		.buzzer_enable = true,
		.motor_enable = true,
	},
	{
		.repetition_type = GUI_ALARM_SET_REPITION_SINGLE,		
		.buzzer_enable = true,
		.motor_enable = true,
	},	
};




static void alarm(_alarm_instance *instance1, _alarm_instance *instance2, _alarm_instance *instance3)
{
	SetWord_t word = {0};
	word.x_axis = 32;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = FOREGROUND_COLOR;
	word.bckgrndcolor = BACKGROUND_COLOR;
	word.kerning = 0;
	
	char p_str[20];
	memset(p_str,0,sizeof(p_str));
	sprintf(p_str,"%02d:%02d",instance1->time.hour,instance1->time.minute);
	LCD_SetNumber(p_str,&word);	

	word.x_axis = 80;
	LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,FOREGROUND_COLOR,0,0,LCD_FILL_ENABLE);	

	word.x_axis = 90;
	word.size = LCD_FONT_32_SIZE;
	memset(p_str,0,sizeof(p_str));
	sprintf(p_str,"%02d:%02d",instance2->time.hour,instance1->time.minute);	
	LCD_SetNumber(p_str,&word);	
	if(instance2->enable)
	{
		word.x_axis = 134;
		word.y_axis = 84;
		word.size = LCD_FONT_16_SIZE;		
		if(SetValue.Language == L_CHINESE)
		{
			LCD_SetString("开启（单次）",&word);				
		}
		else if(SetValue.Language == L_ENGLISH)
		{
			word.y_axis = LCD_CENTER_JUSTIFIED;
			LCD_SetString("ON(SINGLE)",&word);				
		}		
		
	}
	else
	{
		word.x_axis = 134;
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
	
	word.x_axis = 160;
	LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,FOREGROUND_COLOR,0,0,LCD_FILL_ENABLE);		
	
	word.x_axis = 182;
	word.size = LCD_FONT_24_SIZE;
	memset(p_str,0,sizeof(p_str));
	sprintf(p_str,"%02d:%02d",instance3->time.hour,instance1->time.minute);
	LCD_SetNumber(p_str,&word);		

}


void gui_alarm_paint(void)
{
	LCD_SetBackgroundColor(BACKGROUND_COLOR);

	switch(m_gui_alarm_index)
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
	LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
}


void gui_alarm_btn_evt(uint32_t evt)
{
	switch(evt)
	{		
		case (KEY_BACK):{
			DISPLAY_MSG  msg = {0,0};
			ScreenStateSave = ScreenState;
			ScreenState = DISPLAY_SCREEN_TIME;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
		}break;			
		case (KEY_UP):{
			if(m_gui_alarm_index <= 0)
			{
				m_gui_alarm_index = 3;
			}else{
				m_gui_alarm_index--;
			}
			DISPLAY_MSG  msg = {0,0};
			ScreenStateSave = ScreenState;
			ScreenState = DISPLAY_SCREEN_ALARM;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}break;	
			
		case (KEY_OK):{
			m_gui_alarm_set_index = 0;
			DISPLAY_MSG  msg = {0,0};
			ScreenStateSave = ScreenState;
			ScreenState = DISPLAY_SCREEN_ALARM_SET;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
		}break;		
		case (KEY_DOWN):{
			if(m_gui_alarm_index >= 3)
			{
				m_gui_alarm_index = 0;
			}else{
				m_gui_alarm_index++;
			}
			DISPLAY_MSG  msg = {0,0};
			ScreenStateSave = ScreenState;
			ScreenState = DISPLAY_SCREEN_ALARM;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}break;						
	}
}



static void alarm_set( char* menu1, char* menu2, char* menu3, char* status)
{
	SetWord_t word = {0};
	word.x_axis = 26;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = FOREGROUND_COLOR;
	word.bckgrndcolor = BACKGROUND_COLOR;
	word.kerning = 0;
	LCD_SetString(menu1,&word);	
	
	word.x_axis = 80;
	LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,FOREGROUND_COLOR,0,0,LCD_FILL_ENABLE);
		
	word.x_axis = 90;
	word.size = LCD_FONT_32_SIZE;
	LCD_SetString(menu2,&word);	
	
	word.x_axis = 134;
	word.size = LCD_FONT_16_SIZE;
	if(m_gui_alarm_set_index == GUI_ALARM_SET_INDEX_TIME_SET)
	{
		word.kerning = 1;
		LCD_SetNumber(status,&word);
		word.kerning = 0;
	}else{
		LCD_SetString(status,&word);
	}
	
	word.x_axis = 160;
	LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,FOREGROUND_COLOR,0,0,LCD_FILL_ENABLE);	
	
	word.x_axis = 186;	
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString(menu3,&word);		
}

void gui_alarm_set_paint(void)
{
	LCD_SetBackgroundColor(BACKGROUND_COLOR);
  char p_str[23];
	memset(p_str,0,sizeof(p_str));
	switch(m_gui_alarm_set_index)
	{
		case GUI_ALARM_SET_INDEX_TIME_SET:{
			sprintf(p_str,"%02d:%02d",g_alarm_instance[m_gui_alarm_index].time.hour,g_alarm_instance[m_gui_alarm_index].time.minute);	
			alarm_set((char *)AlarmSetStrs[SetValue.Language][GUI_ALARM_SET_INDEX_MAX - 1],
						(char *)AlarmSetStrs[SetValue.Language][m_gui_alarm_set_index],
						(char *)AlarmSetStrs[SetValue.Language][m_gui_alarm_set_index + 1],
							p_str);						
		}break;
		case GUI_ALARM_SET_INDEX_BUZZER_SET:{
			
			if(g_alarm_instance[m_gui_alarm_index].buzzer_enable && !g_alarm_instance[m_gui_alarm_index].motor_enable)
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
			else if(!g_alarm_instance[m_gui_alarm_index].buzzer_enable && g_alarm_instance[m_gui_alarm_index].motor_enable)
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
			else if(g_alarm_instance[m_gui_alarm_index].buzzer_enable && g_alarm_instance[m_gui_alarm_index].motor_enable)
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
			alarm_set((char *)AlarmSetStrs[SetValue.Language][m_gui_alarm_set_index - 1],
						(char *)AlarmSetStrs[SetValue.Language][m_gui_alarm_set_index],
						(char *)AlarmSetStrs[SetValue.Language][m_gui_alarm_set_index + 1],
							p_str);									
		}break;		
		case GUI_ALARM_SET_INDEX_REPITION_SET:{
			switch(g_alarm_instance[m_gui_alarm_index].repetition_type)
			{
				case GUI_ALARM_SET_REPITION_SINGLE:{
					if(SetValue.Language == L_CHINESE)
					{
						sprintf(p_str,"单次");					
					}
					else if(SetValue.Language == L_ENGLISH)
					{
						sprintf(p_str,"SINGLE");		
					}					
				}break;
				case GUI_ALARM_SET_REPITION_DAILY:{
					if(SetValue.Language == L_CHINESE)
					{
						sprintf(p_str,"每日");					
					}
					else if(SetValue.Language == L_ENGLISH)
					{
						sprintf(p_str,"DAILY");		
					}				
				}break;
				case GUI_ALARM_SET_REPITION_WEEK_DAY:{
					if(SetValue.Language == L_CHINESE)
					{
						sprintf(p_str,"工作日");					
					}
					else if(SetValue.Language == L_ENGLISH)
					{
						sprintf(p_str,"WEEK DAY");		
					}				
				}break;
				case GUI_ALARM_SET_REPITION_WEEKEND:{
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
			
			alarm_set((char *)AlarmSetStrs[SetValue.Language][m_gui_alarm_set_index - 1],
						(char *)AlarmSetStrs[SetValue.Language][m_gui_alarm_set_index],
						(char *)AlarmSetStrs[SetValue.Language][m_gui_alarm_set_index + 1],
							p_str);						
		}break;
		case GUI_ALARM_SET_INDEX_ENABLE_SET:{
			if(g_alarm_instance[m_gui_alarm_index].enable)
			{
				if(SetValue.Language == L_CHINESE)
				{
					sprintf(p_str,"开启");					
				}
				else if(SetValue.Language == L_ENGLISH)
				{
					sprintf(p_str,"ON");		
				}				
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
			}
			alarm_set((char *)AlarmSetStrs[SetValue.Language][m_gui_alarm_set_index - 1],
						(char *)AlarmSetStrs[SetValue.Language][m_gui_alarm_set_index],
						(char *)AlarmSetStrs[SetValue.Language][0],
							p_str);							
		}break;	
	}		
	
		
	LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
}

void gui_alarm_set_btn_evt(uint32_t evt)
{
	switch(evt)
	{		
		case (KEY_BACK):{
			DISPLAY_MSG  msg = {0,0};
			ScreenStateSave = ScreenState;
			ScreenState = DISPLAY_SCREEN_ALARM;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);						
		}break;			
		case (KEY_UP):{
			if(m_gui_alarm_set_index <= 0)
			{
				m_gui_alarm_set_index = 3;
			}else{
				m_gui_alarm_set_index--;
			}
			DISPLAY_MSG  msg = {0,0};
			ScreenStateSave = ScreenState;
			ScreenState = DISPLAY_SCREEN_ALARM_SET;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
		}break;	
		case (KEY_OK):{
			switch(m_gui_alarm_set_index)
			{
				case GUI_ALARM_SET_INDEX_TIME_SET:{		
					m_gui_alarm_set_time_index = 0;
					memset(&m_alarm_time,0,sizeof(m_alarm_time));
					DISPLAY_MSG  msg = {0,0};
					ScreenStateSave = ScreenState;
					ScreenState = DISPLAY_SCREEN_ALARM_SET_TIME;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);					
				}break;
				case GUI_ALARM_SET_INDEX_BUZZER_SET:{
					m_gui_alarm_set_sound_N_viberate_index = 0;
					DISPLAY_MSG  msg = {0,0};
					ScreenStateSave = ScreenState;
					ScreenState = DISPLAY_SCREEN_ALARM_SET_SOUND_N_VIBERATE;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);															
				}break;		
				case GUI_ALARM_SET_INDEX_REPITION_SET:{
					m_gui_alarm_set_repition_index = 0;				
					DISPLAY_MSG  msg = {0,0};
					ScreenStateSave = ScreenState;
					ScreenState = DISPLAY_SCREEN_ALARM_SET_REPITION;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);		
				}break;
				case GUI_ALARM_SET_INDEX_ENABLE_SET:{
					g_alarm_instance[m_gui_alarm_index].enable = !g_alarm_instance[m_gui_alarm_index].enable;
					DISPLAY_MSG  msg = {0,0};
					ScreenStateSave = ScreenState;
					ScreenState = DISPLAY_SCREEN_ALARM_SET;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
				}break;	
			}					
		}break;		
		case (KEY_DOWN):{
			if(m_gui_alarm_set_index >= 3)
			{
				m_gui_alarm_set_index = 0;
			}else{
				m_gui_alarm_set_index++;
			}
			DISPLAY_MSG  msg = {0,0};
			ScreenStateSave = ScreenState;
			ScreenState = DISPLAY_SCREEN_ALARM_SET;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
		}break;					
	}
}



static void alarm_set_time(_alarm_time alarm_time)
{
	SetWord_t word = {0};
	word.x_axis = 70;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_88_SIZE;
	word.forecolor = FOREGROUND_COLOR;
	word.bckgrndcolor = BACKGROUND_COLOR;
	word.kerning = 0;
	
  char p_str[23];
	memset(p_str,0,sizeof(p_str));	
	sprintf(p_str,"%02d:%02d",alarm_time.hour,alarm_time.minute);
	LCD_SetNumber(p_str,&word);
	
	if(m_gui_alarm_set_time_index == GUI_ALARM_SET_TIME_HOUR)
	{
		word.x_axis = 170;
		word.y_axis = 48;
		LCD_SetRectangle(word.x_axis,6,word.y_axis,58,LCD_BLUE,0,0,LCD_FILL_ENABLE);	
	}
	else if(m_gui_alarm_set_time_index == GUI_ALARM_SET_TIME_MINUTE)
	{
		word.x_axis = 170;
		word.y_axis = 134;
		LCD_SetRectangle(word.x_axis,6,word.y_axis,58,LCD_BLUE,0,0,LCD_FILL_ENABLE);	
	}
}

void gui_alarm_set_time_paint(void)
{
	LCD_SetBackgroundColor(BACKGROUND_COLOR);
	
	SetWord_t word = {0};
	word.x_axis = 20;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = FOREGROUND_COLOR;
	word.bckgrndcolor = BACKGROUND_COLOR;
	word.kerning = 0;
	if(SetValue.Language == L_CHINESE)
	{
		LCD_SetString("时间设置",&word);						
	}
	else if(SetValue.Language == L_ENGLISH)
	{
		LCD_SetString("TIME SET",&word);		
	}		
	
	
	alarm_set_time(m_alarm_time);
	
	LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
}

void gui_alarm_set_time_btn_evt(uint32_t evt)
{
	switch(evt)
	{		
		case (KEY_BACK):{
			DISPLAY_MSG  msg = {0,0};
			ScreenStateSave = ScreenState;
			ScreenState = DISPLAY_SCREEN_ALARM_SET;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);						
		}break;			
		case (KEY_UP):{
			if(m_gui_alarm_set_time_index == GUI_ALARM_SET_TIME_HOUR)
			{
				if(m_alarm_time.hour >= 23)
				{
					m_alarm_time.hour = 0;
				}else{
					m_alarm_time.hour++;
				}								
			}
			else if(m_gui_alarm_set_time_index == GUI_ALARM_SET_TIME_MINUTE)
			{
				if(m_alarm_time.minute >= 59)
				{
					m_alarm_time.minute = 0;
				}else{
					m_alarm_time.minute++;
				}				
			}				
			DISPLAY_MSG  msg = {0,0};
			ScreenStateSave = ScreenState;
			ScreenState = DISPLAY_SCREEN_ALARM_SET_TIME;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}break;	
		case (KEY_OK):{
			if(m_gui_alarm_set_time_index == GUI_ALARM_SET_TIME_HOUR)
			{
				m_gui_alarm_set_time_index = GUI_ALARM_SET_TIME_MINUTE;
				DISPLAY_MSG  msg = {0,0};
				ScreenStateSave = ScreenState;
				ScreenState = DISPLAY_SCREEN_ALARM_SET_TIME;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);					
			}
			else if(m_gui_alarm_set_time_index == GUI_ALARM_SET_TIME_MINUTE)
			{
				g_alarm_instance[m_gui_alarm_index].time = m_alarm_time;
				DISPLAY_MSG  msg = {0,0};
				ScreenStateSave = ScreenState;
				ScreenState = DISPLAY_SCREEN_ALARM_SET;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);				
			}
			

								
		}break;		
		case (KEY_DOWN):{
			if(m_gui_alarm_set_time_index == GUI_ALARM_SET_TIME_HOUR)
			{
				if(m_alarm_time.hour <= 0)
				{
					m_alarm_time.hour = 23;
				}else{
					m_alarm_time.hour--;
				}					
			}
			else if(m_gui_alarm_set_time_index == GUI_ALARM_SET_TIME_MINUTE)
			{
				if(m_alarm_time.minute <= 0)
				{
					m_alarm_time.minute = 59;
				}else{
					m_alarm_time.minute--;
				}										
			}					
			DISPLAY_MSG  msg = {0,0};
			ScreenStateSave = ScreenState;
			ScreenState = DISPLAY_SCREEN_ALARM_SET_TIME;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);					
		}break;					
	}

}


static void alarm_set_sound_N_viberate( char* menu1, char* menu2, char* menu3)
{
	SetWord_t word = {0};
	word.x_axis = 26;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = FOREGROUND_COLOR;
	word.bckgrndcolor = BACKGROUND_COLOR;
	word.kerning = 0;
	LCD_SetString(menu1,&word);	
	
	word.x_axis = 80;
	LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,FOREGROUND_COLOR,0,0,LCD_FILL_ENABLE);
		
	word.x_axis = 100;
	word.size = LCD_FONT_32_SIZE;
	LCD_SetString(menu2,&word);	
	
	word.x_axis = 160;
	LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,FOREGROUND_COLOR,0,0,LCD_FILL_ENABLE);	
	
	word.x_axis = 186;	
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString(menu3,&word);		

}
void gui_alarm_set_sound_N_viberate_paint(void)
{
	LCD_SetBackgroundColor(BACKGROUND_COLOR);
	
	switch(m_gui_alarm_set_sound_N_viberate_index)
	{
		case GUI_ALARM_SET_SOUND_N_VIBERATE_SOUND_N_VIBERATE:{
			alarm_set_sound_N_viberate((char *)AlarmSetSoundNViberateStrs[SetValue.Language][GUI_ALARM_SET_SOUND_N_VIBERATE_INDEX_MAX - 1],
						(char *)AlarmSetSoundNViberateStrs[SetValue.Language][m_gui_alarm_set_sound_N_viberate_index],
						(char *)AlarmSetSoundNViberateStrs[SetValue.Language][m_gui_alarm_set_sound_N_viberate_index + 1]);			
		}break;
		case GUI_ALARM_SET_SOUND_N_VIBERATE_SOUND:{
			alarm_set_sound_N_viberate((char *)AlarmSetSoundNViberateStrs[SetValue.Language][m_gui_alarm_set_sound_N_viberate_index - 1],
						(char *)AlarmSetSoundNViberateStrs[SetValue.Language][m_gui_alarm_set_sound_N_viberate_index],
						(char *)AlarmSetSoundNViberateStrs[SetValue.Language][m_gui_alarm_set_sound_N_viberate_index + 1]);			
		}break;
		case GUI_ALARM_SET_SOUND_N_VIBERATE_VIBERATE:{
			alarm_set_sound_N_viberate((char *)AlarmSetSoundNViberateStrs[SetValue.Language][m_gui_alarm_set_sound_N_viberate_index - 1],
						(char *)AlarmSetSoundNViberateStrs[SetValue.Language][m_gui_alarm_set_sound_N_viberate_index],
						(char *)AlarmSetSoundNViberateStrs[SetValue.Language][0]);			
		}break;		
	}		
	
	LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
}


void gui_alarm_set_sound_N_viberate_btn_evt(uint32_t evt)
{
	switch(evt)
	{		
		case (KEY_BACK):{
			DISPLAY_MSG  msg = {0,0};
			ScreenStateSave = ScreenState;
			ScreenState = DISPLAY_SCREEN_ALARM_SET;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);						
		}break;			
		case (KEY_UP):{
			if(m_gui_alarm_set_sound_N_viberate_index <= 0)
			{
				m_gui_alarm_set_sound_N_viberate_index = GUI_ALARM_SET_SOUND_N_VIBERATE_INDEX_MAX - 1;
			}else{
				m_gui_alarm_set_sound_N_viberate_index--;
			}
			DISPLAY_MSG  msg = {0,0};
			ScreenStateSave = ScreenState;
			ScreenState = DISPLAY_SCREEN_ALARM_SET_SOUND_N_VIBERATE;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
		}break;	
		case (KEY_OK):{
			switch(m_gui_alarm_set_sound_N_viberate_index)
			{
				case GUI_ALARM_SET_SOUND_N_VIBERATE_SOUND_N_VIBERATE:{
					g_alarm_instance[m_gui_alarm_index].buzzer_enable = true;			
					g_alarm_instance[m_gui_alarm_index].motor_enable = true;	
				}break;
				case GUI_ALARM_SET_SOUND_N_VIBERATE_SOUND:{
					g_alarm_instance[m_gui_alarm_index].buzzer_enable = true;			
					g_alarm_instance[m_gui_alarm_index].motor_enable = false;								
				}break;
				case GUI_ALARM_SET_SOUND_N_VIBERATE_VIBERATE:{
					g_alarm_instance[m_gui_alarm_index].buzzer_enable = false;			
					g_alarm_instance[m_gui_alarm_index].motor_enable = true;						
				}break;		
			}									
			DISPLAY_MSG  msg = {0,0};
			ScreenStateSave = ScreenState;
			ScreenState = DISPLAY_SCREEN_ALARM_SET;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
		}break;		
		case (KEY_DOWN):{
			if(m_gui_alarm_set_sound_N_viberate_index >= ( GUI_ALARM_SET_SOUND_N_VIBERATE_INDEX_MAX - 1))
			{
				m_gui_alarm_set_sound_N_viberate_index = 0;
			}else{
				m_gui_alarm_set_sound_N_viberate_index++;
			}
			DISPLAY_MSG  msg = {0,0};
			ScreenStateSave = ScreenState;
			ScreenState = DISPLAY_SCREEN_ALARM_SET_SOUND_N_VIBERATE;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);								
		}break;					
	}

}

static void alarm_set_repition( char* menu1, char* menu2, char* menu3)
{
	SetWord_t word = {0};
	word.x_axis = 26;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = FOREGROUND_COLOR;
	word.bckgrndcolor = BACKGROUND_COLOR;
	word.kerning = 0;
	LCD_SetString(menu1,&word);	
	
	word.x_axis = 80;
	LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,FOREGROUND_COLOR,0,0,LCD_FILL_ENABLE);
		
	word.x_axis = 100;
	word.size = LCD_FONT_32_SIZE;
	LCD_SetString(menu2,&word);	
	
	word.x_axis = 160;
	LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,FOREGROUND_COLOR,0,0,LCD_FILL_ENABLE);	
	
	word.x_axis = 186;	
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString(menu3,&word);		

}
void gui_alarm_set_repition_paint(void)
{
	LCD_SetBackgroundColor(BACKGROUND_COLOR);
	
	switch(m_gui_alarm_set_repition_index)
	{
		case GUI_ALARM_SET_REPITION_SINGLE:{
			alarm_set_repition((char *)AlarmSetRepitionStrs[SetValue.Language][GUI_ALARM_SET_REPITION_INDEX_MAX - 1],
						(char *)AlarmSetRepitionStrs[SetValue.Language][m_gui_alarm_set_repition_index],
						(char *)AlarmSetRepitionStrs[SetValue.Language][m_gui_alarm_set_repition_index + 1]);			
		}break;
		case GUI_ALARM_SET_REPITION_DAILY:{
			alarm_set_repition((char *)AlarmSetRepitionStrs[SetValue.Language][m_gui_alarm_set_repition_index - 1],
						(char *)AlarmSetRepitionStrs[SetValue.Language][m_gui_alarm_set_repition_index],
						(char *)AlarmSetRepitionStrs[SetValue.Language][m_gui_alarm_set_repition_index + 1]);			
		}break;
		case GUI_ALARM_SET_REPITION_WEEK_DAY:{
			alarm_set_repition((char *)AlarmSetRepitionStrs[SetValue.Language][m_gui_alarm_set_repition_index - 1],
						(char *)AlarmSetRepitionStrs[SetValue.Language][m_gui_alarm_set_repition_index],
						(char *)AlarmSetRepitionStrs[SetValue.Language][m_gui_alarm_set_repition_index + 1]);			
		}break;
		case GUI_ALARM_SET_REPITION_WEEKEND:{
			alarm_set_repition((char *)AlarmSetRepitionStrs[SetValue.Language][m_gui_alarm_set_repition_index - 1],
						(char *)AlarmSetRepitionStrs[SetValue.Language][m_gui_alarm_set_repition_index],
						(char *)AlarmSetRepitionStrs[SetValue.Language][0]);			
		}break;		
	}			
	
	LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
}


void gui_alarm_set_repition_btn_evt(uint32_t evt)
{
	switch(evt)
	{		
		case (KEY_BACK):{
			DISPLAY_MSG  msg = {0,0};
			ScreenStateSave = ScreenState;
			ScreenState = DISPLAY_SCREEN_ALARM_SET;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);						
		}break;			
		case (KEY_UP):{
			if(m_gui_alarm_set_repition_index <= 0)
			{
				m_gui_alarm_set_repition_index = GUI_ALARM_SET_REPITION_INDEX_MAX - 1;
			}else{
				m_gui_alarm_set_repition_index--;
			}
			DISPLAY_MSG  msg = {0,0};
			ScreenStateSave = ScreenState;
			ScreenState = DISPLAY_SCREEN_ALARM_SET_REPITION;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);					
		}break;	
		case (KEY_OK):{
			g_alarm_instance[m_gui_alarm_index].repetition_type = m_gui_alarm_set_repition_index;		
			DISPLAY_MSG  msg = {0,0};
			ScreenStateSave = ScreenState;
			ScreenState = DISPLAY_SCREEN_ALARM_SET;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
		}break;		
		case (KEY_DOWN):{
			if(m_gui_alarm_set_repition_index >= ( GUI_ALARM_SET_REPITION_INDEX_MAX - 1))
			{
				m_gui_alarm_set_repition_index = 0;
			}else{
				m_gui_alarm_set_repition_index++;
			}
			DISPLAY_MSG  msg = {0,0};
			ScreenStateSave = ScreenState;
			ScreenState = DISPLAY_SCREEN_ALARM_SET_REPITION;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);												
		}break;					
	}

}





