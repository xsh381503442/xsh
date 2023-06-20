#include "gui_tool_config.h"
#include "gui_tool_countdown.h"

#include "drv_lcd.h"
#include "drv_key.h"

#include "task_tool.h"

#include "mode_power_off.h"

#include "time_notify.h"

#include "gui.h"
#include "gui_tool.h"

#include "font_config.h"

#include "com_data.h"

#if DEBUG_ENABLED == 1 && GUI_TOOL_COUNTDOWN_LOG_ENABLED == 1
	#define GUI_TOOL_COUNTDOWN_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_TOOL_COUNTDOWN_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define GUI_TOOL_COUNTDOWN_WRITESTRING(...)
	#define GUI_TOOL_COUNTDOWN_PRINTF(...)		        
#endif

#define GUI_TOOL_COUNTDOWN_SETTINE_HOUR	0
#define GUI_TOOL_COUNTDOWN_SETTINE_MIN	1
#define GUI_TOOL_COUNTDOWN_SETTINE_SEC	2

//#define GUI_TOOL_COUNTDOWN_MENU_TIME	0
//#define GUI_TOOL_COUNTDOWN_MENU_START	1

#define GUI_TOOL_COUNTDOWN_EXIT_NO		0
#define GUI_TOOL_COUNTDOWN_EXIT_YES		1

static uint8_t m_tool_countdown_hour = 0;
static uint8_t m_tool_countdown_min = 5;
static uint8_t m_tool_countdown_sec = 0;
static uint8_t m_tool_countdown_index;
static uint32_t m_tool_countdown_time;

static am_util_stopwatch_t m_tool_countdown;
void set_tool_countdown_index(uint8_t index) 
{
	m_tool_countdown_index = index;

}

uint32_t tool_countdown_real_time_get(void)
{
	//����ʱʱ��
	m_tool_countdown_time = am_util_stopwatch_elapsed_get(&m_tool_countdown, AM_UTIL_STOPWATCH_MS);
	m_tool_countdown_time /= 1000;
	
	return m_tool_countdown_time;
}

uint32_t tool_countdown_target_time_get(void)
{
	uint32_t targetSec;
	
	//Ŀ��ʱ��
	targetSec = (m_tool_countdown_hour * 3600) + (m_tool_countdown_min * 60) + m_tool_countdown_sec;
	
	return targetSec;
}

void tool_countdown_finish(void)
{
	m_tool_countdown_time = tool_countdown_target_time_get();
	
	//��ͣ��ʱ
	am_util_stopwatch_stop(&m_tool_countdown);
	
	//�ر�����
	mode_close();
	
	//����ʱ������ʾ
	timer_notify_motor_start(1000,500,30,false,NOTIFY_MODE_TOOLS);
	timer_notify_buzzer_start(1000,500,30,false,NOTIFY_MODE_TOOLS);
	ScreenState = DISPLAY_SCREEN_COUNTDOWN_FINISH;
}

//����ʱ
/*void gui_tool_countdown_paint(void)
{
	SetWord_t word = {0};
	
	//����������Ϊ��ɫ
	LCD_SetBackgroundColor(LCD_BLACK);
	
	//���ͼ��
	LCD_SetPicture(15,LCD_CENTER_JUSTIFIED,LCD_PERU,LCD_NONE,&Img_Countdown_36X36);
	
	//���뵹��ʱ
	word.x_axis = 108;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	LCD_SetString((char *)CountdownStrs[SetValue.Language][0],&word);
	
	//ʱ��
//	gui_bottom_time();
}*/

//����ʱ���ý���
void gui_tool_countdown_setting_paint(void)
{
	SetWord_t word = {0};
	char str[10] = {0};
	
	//����������Ϊ��ɫ
	LCD_SetBackgroundColor(LCD_BLACK);
	
	//LCD_SetRectangle(0, 80, 0, LCD_PIXEL_ONELINE_MAX, LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
	
	//ʱ������
	word.x_axis = 40;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	LCD_SetString((char *)CountdownStrs[SetValue.Language][1],&word);
	

	word.x_axis = 120 - Font_Number_49.height/2;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_49_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	memset(str,0,sizeof(str));
	sprintf(str,"%02d:%02d:%02d", m_tool_countdown_hour, m_tool_countdown_min, m_tool_countdown_sec);
	LCD_SetNumber(str, &word);
	
	//������
	word.x_axis += (Font_Number_49.height + 6);
#if defined WATCH_SIM_NUMBER
	word.y_axis = ((LCD_PIXEL_ONELINE_MAX - (Font_Number_49.width + word.kerning)*7)/2) 
					+ ((Font_Number_49.width*2 + (Font_Number_49.width/2) + (word.kerning*3))*m_tool_countdown_index);
#else
	word.y_axis = ((LCD_PIXEL_ONELINE_MAX - (Font_Number_49.width + word.kerning)*8)/2) 
					+ ((Font_Number_49.width*2 + Font_Number_49.width + (word.kerning*3))*m_tool_countdown_index);
#endif
	LCD_SetRectangle(word.x_axis, 4, word.y_axis, Font_Number_49.width*2 + word.kerning, LCD_CYAN, 0, 0, LCD_FILL_ENABLE);
}

//����ʱ�˵�����
void gui_tool_countdown_menu_paint(void)
{
	char str[10] = {0};
	SetWord_t word = {0};
	
	
	LCD_SetBackgroundColor(LCD_BLACK);
	LCD_SetRectangle(79, 3, 120 - 90, 180, LCD_LIGHTGRAY, 0, 0, LCD_FILL_ENABLE);//80
	LCD_SetRectangle(159, 3, 120 - 90, 180, LCD_LIGHTGRAY, 0, 0, LCD_FILL_ENABLE);//160�ָ���
       /*����ʱ*/
	    word.x_axis = 40;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_NONE;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString((char *)ToolStrs[SetValue.Language][3], &word);
	memset(str,0,sizeof(str));
	if(m_tool_countdown_hour > 0)
	{
		sprintf(str,"%d:%02d:%02d", m_tool_countdown_hour, m_tool_countdown_min, m_tool_countdown_sec);
	}
	else
	{
		sprintf(str,"%02d:%02d", m_tool_countdown_min, m_tool_countdown_sec);
	}
	
	if(m_tool_countdown_index == GUI_TOOL_COUNTDOWN_MENU_START)
	{
		
	
		//��ʼ
		word.x_axis = 108;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.forecolor = LCD_CYAN;
		word.bckgrndcolor = LCD_NONE;
		word.size = LCD_FONT_24_SIZE;
		LCD_SetString((char *)CountdownStrs[SetValue.Language][2], &word);
		
        		
		//ʱ��
		word.x_axis = 184;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.forecolor = LCD_LIGHTGRAY;
		word.bckgrndcolor = LCD_NONE;
		word.size = LCD_FONT_24_SIZE;
		LCD_SetNumber(str, &word);
	}
	else
	{
		//�Ϸ���ʾ
		//gui_page_up(LCD_BLACK);
       //ʱ��
       // word.x_axis = 108;
		
        word.x_axis = 120 - Font_Number_37.height/2;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.forecolor = LCD_CYAN;
		word.bckgrndcolor = LCD_NONE;
		//word.size = LCD_FONT_24_SIZE;
		
		word.size = LCD_FONT_37_SIZE;
		LCD_SetNumber(str, &word);
		
		//��ʼ
		word.x_axis = 184;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.forecolor = LCD_LIGHTGRAY;
		word.bckgrndcolor = LCD_NONE;
		word.size = LCD_FONT_24_SIZE;
		LCD_SetString((char *)CountdownStrs[SetValue.Language][2], &word);
	
				
	}
	
	//�·���ʾ
	gui_page_down(LCD_BROWN);
}

//����ʱʱ�����
void gui_tool_countdown_time_paint(void)
{
	SetWord_t word = {0};
	char str[10] = {0};
	uint32_t realSec,targetSec;
	uint8_t hour,min,sec;
	uint16_t ang;
	
	//����������Ϊ��ɫ
	LCD_SetBackgroundColor(LCD_BLACK);
	
	//����ʱͼ��
	LCD_SetPicture(30,LCD_CENTER_JUSTIFIED,LCD_RED,LCD_NONE,&img_tools_big_lists[2]);
	
	//Ŀ��ʱ��
	targetSec = tool_countdown_target_time_get();
	//ʵ��ʱ��
	realSec = m_tool_countdown_time;
	if(realSec > targetSec)
	{
		realSec = targetSec;
	}
	
	hour = (targetSec - realSec)/3600;
	min = (targetSec - realSec)/60%60;
	sec = (targetSec - realSec)%60;
	
	memset(str,0,sizeof(str));
	if(hour > 0)
	{
		sprintf(str,"%d:%02d:%02d", hour, min, sec);
	}
	else
	{
		sprintf(str,"%02d:%02d", min, sec);
	}
	
	word.x_axis = (LCD_LINE_CNT_MAX - Font_Number_56.height)/2;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_56_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	LCD_SetNumber(str, &word);
	
	//����Ȧ
	if(realSec < targetSec)
	{
		ang = realSec*360/targetSec;
		if((realSec > 0) && (ang == 0))
		{
			//����ʱ��ʼ�󣬷�ֹ�޽���
			ang = 1;
		}
	//	LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 14, ang, 360, LCD_RED);
		
		LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 14, 0, 360 - ang , LCD_RED);
	}
}

//����ʱ�˳�����
void gui_tool_countdown_exit_paint(void)
{
	char str[10] = {0};
	SetWord_t word = {0};


	LCD_SetBackgroundColor(LCD_BLACK);
	LCD_SetRectangle(79, 3, 120 - 90, 180, LCD_LIGHTGRAY, 0, 0, LCD_FILL_ENABLE);//80
	LCD_SetRectangle(159, 3, 120 - 90, 180, LCD_LIGHTGRAY, 0, 0, LCD_FILL_ENABLE);//160�ָ�


	//�Ƿ��˳�?
	word.x_axis = 40;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString((char *)CountdownStrs[SetValue.Language][3],&word);
	
	
	
	word.x_axis = 108;
	word.forecolor = LCD_CYAN;
	word.size = LCD_FONT_24_SIZE;
	if(m_tool_countdown_index == GUI_TOOL_COUNTDOWN_EXIT_NO)
	{
		//��
		
		LCD_SetString((char *)CountdownStrs[SetValue.Language][4],&word);
	}
	else
	{
		//��
		
		LCD_SetString((char *)CountdownStrs[SetValue.Language][5],&word);
	}
	
	
	word.x_axis = 184;
	word.forecolor = LCD_GRAY;
	word.size = LCD_FONT_16_SIZE;
	if(m_tool_countdown_index == GUI_TOOL_COUNTDOWN_EXIT_NO)
	{
		//��
		LCD_SetString((char *)CountdownStrs[SetValue.Language][5],&word);
	}
	else
	{
		//��
		LCD_SetString((char *)CountdownStrs[SetValue.Language][4],&word);
	}
	//�·���ʾ
	gui_page_down(LCD_BROWN);
}

void gui_tool_countdown_finish_paint(void)
{
	SetWord_t word = {0};
	
	//����������Ϊ��ɫ
	LCD_SetBackgroundColor(LCD_BLACK);

		//���ͼ��
	//LCD_SetPicture(30,LCD_CENTER_JUSTIFIED,LCD_NONE,LCD_NONE,&Img_Countdown_28X28);
		
		LCD_SetPicture(30,LCD_CENTER_JUSTIFIED,LCD_RED,LCD_NONE,&img_tools_big_lists[2]);
	
	word.x_axis = 108;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	LCD_SetString((char *)CountdownStrs[SetValue.Language][6],&word);
	
	//ʱ��
}

void gui_tool_countdown_init(void)
{
	//�������
	m_tool_countdown_time = 0;
}

void gui_tool_countdown_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch(Key_Value)
	{
		case KEY_OK:
			//���뵹��ʱ�˵�����
			ScreenState = DISPLAY_SCREEN_COUNTDOWN_MENU;
			m_tool_countdown_index = GUI_TOOL_COUNTDOWN_MENU_START;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		case KEY_UP:
		case KEY_DOWN:
		case (KEY_DOWN<<1):
			break;
		case KEY_BACK:
			//���ش�������
			ScreenState = DISPLAY_SCREEN_HOME;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}

void gui_tool_countdown_setting_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch(Key_Value)
	{
		case KEY_OK:
			if(m_tool_countdown_index < GUI_TOOL_COUNTDOWN_SETTINE_SEC)
			{
				m_tool_countdown_index++;
				if(m_tool_countdown_index == GUI_TOOL_COUNTDOWN_SETTINE_SEC)
				{
					if((m_tool_countdown_hour == 0) && (m_tool_countdown_min == 0)
						&& (m_tool_countdown_sec == 0))
					{
						//��ֹʱ��Ϊ0
						m_tool_countdown_sec = 1;
					}
				}
			}
			else
			{
				//�������
				ScreenState = DISPLAY_SCREEN_COUNTDOWN_MENU;
				m_tool_countdown_index = GUI_TOOL_COUNTDOWN_MENU_START;
			}
			
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		case KEY_UP:
			if(m_tool_countdown_index == GUI_TOOL_COUNTDOWN_SETTINE_HOUR)
			{
				//ʱ
				if(m_tool_countdown_hour < 23)
				{
					m_tool_countdown_hour++;
				}
				else
				{
					m_tool_countdown_hour = 0;
				}
			}
			else if(m_tool_countdown_index == GUI_TOOL_COUNTDOWN_SETTINE_MIN)
			{
				//��
				if(m_tool_countdown_min < 59)
				{
					m_tool_countdown_min++;
				}
				else
				{
					m_tool_countdown_min = 0;
				}
			}
			else
			{
				//��
				if(m_tool_countdown_sec < 59)
				{
					m_tool_countdown_sec++;
				}
				else
				{
					//��ֹʱ��Ϊ0
					if((m_tool_countdown_hour == 0) && (m_tool_countdown_min == 0))
					{
						m_tool_countdown_sec = 1;
					}
					else
					{
						m_tool_countdown_sec = 0;
					}
				}
			}
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		case KEY_DOWN:
			if(m_tool_countdown_index == GUI_TOOL_COUNTDOWN_SETTINE_HOUR)
			{
				//ʱ
				if(m_tool_countdown_hour > 0)
				{
					m_tool_countdown_hour--;
				}
				else
				{
					m_tool_countdown_hour = 23;
				}
			}
			else if(m_tool_countdown_index == GUI_TOOL_COUNTDOWN_SETTINE_MIN)
			{
				//��
				if(m_tool_countdown_min > 0)
				{
					m_tool_countdown_min--;
				}
				else
				{
					m_tool_countdown_min = 59;
				}
			}
			else
			{
				//��
				uint8_t minSec;
				if((m_tool_countdown_hour == 0) && (m_tool_countdown_min == 0))
				{
					minSec = 1;
				}
				else
				{
					minSec = 0;
				}
				
				if(m_tool_countdown_sec > minSec)
				{
					m_tool_countdown_sec--;
				}
				else
				{
					m_tool_countdown_sec = 59;
				}
			}
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		case KEY_BACK:
			if(m_tool_countdown_index > GUI_TOOL_COUNTDOWN_SETTINE_HOUR)
			{
				m_tool_countdown_index--;
			}
			else
			{
				ScreenState = DISPLAY_SCREEN_COUNTDOWN_MENU;
				m_tool_countdown_index = GUI_TOOL_COUNTDOWN_MENU_TIME;
			}
			
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}

void gui_tool_countdown_menu_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch(Key_Value)
	{
		case KEY_OK:
			if(m_tool_countdown_index == GUI_TOOL_COUNTDOWN_MENU_TIME)
			{
				//����ʱ�����
				ScreenState = DISPLAY_SCREEN_COUNTDOWN_SETTING;
				m_tool_countdown_index = GUI_TOOL_COUNTDOWN_SETTINE_HOUR;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
			else
			{
				//��ʼ����ʱ
				gui_tool_countdown_init();
				am_util_stopwatch_init(&m_tool_countdown);
				am_util_stopwatch_restart(&m_tool_countdown);
				
				ScreenState = DISPLAY_SCREEN_COUNTDOWN_TIME;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				
				//��������
				tool_task_start(DISPLAY_SCREEN_COUNTDOWN);
			}
			break;
		case KEY_UP:
		case KEY_DOWN:
			if(m_tool_countdown_index == GUI_TOOL_COUNTDOWN_MENU_TIME)
			{
				m_tool_countdown_index = GUI_TOOL_COUNTDOWN_MENU_START;
			}
			else
			{
				m_tool_countdown_index = GUI_TOOL_COUNTDOWN_MENU_TIME;
			}
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		case KEY_BACK:
			//���ص���ʱ����
			ScreenState = DISPLAY_SCREEN_TOOL;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}

void gui_tool_countdown_time_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch(Key_Value)
	{
		//case KEY_BACK:
		case KEY_OK:
			//��ͣ����ʱ
			am_util_stopwatch_stop(&m_tool_countdown);
		
			ScreenState = DISPLAY_SCREEN_COUNTDOWN_EXIT;
			m_tool_countdown_index = GUI_TOOL_COUNTDOWN_EXIT_YES;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}

void gui_tool_countdown_exit_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch(Key_Value)
	{
		case KEY_OK:
			if(m_tool_countdown_index == GUI_TOOL_COUNTDOWN_EXIT_YES)
			{
				//�ر�����
				tool_task_close();
				//���ص���ʱ����
				ScreenState = DISPLAY_SCREEN_TOOL;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
			else
			{
				//��������ʱ
				am_util_stopwatch_start(&m_tool_countdown);
				
				ScreenState = DISPLAY_SCREEN_COUNTDOWN_TIME;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
			break;
		case KEY_UP:
		case KEY_DOWN:
			if(m_tool_countdown_index == GUI_TOOL_COUNTDOWN_EXIT_YES)
			{
				m_tool_countdown_index = GUI_TOOL_COUNTDOWN_EXIT_NO;
			}
			else
			{
				m_tool_countdown_index = GUI_TOOL_COUNTDOWN_EXIT_YES;
			}
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}

void gui_tool_countdown_finish_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch(Key_Value)
	{
		case KEY_BACK:
			timer_notify_motor_stop(false);
			timer_notify_buzzer_stop(false);
		
			//���ش�������
			
			ScreenState = DISPLAY_SCREEN_TOOL;

			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}
