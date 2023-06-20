#include "gui_tool_config.h"
#include "gui_tool_stopwatch.h"

#include "drv_lcd.h"
#include "drv_key.h"

#include "task_tool.h"

#include "time_notify.h"

#include "gui.h"
#include "gui_tool.h"

#include "font_config.h"

#include "com_data.h"

#if DEBUG_ENABLED == 1 && GUI_TOOL_STOPWATCH_LOG_ENABLED == 1
	#define GUI_TOOL_STOPWATCH_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_TOOL_STOPWATCH_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define GUI_TOOL_STOPWATCH_WRITESTRING(...)
	#define GUI_TOOL_STOPWATCH_PRINTF(...)		        
#endif

#define	TOOL_STOPWATCH_COUNT		1	//�ƴ�
#define	TOOL_STOPWATCH_LAP			2	//��Ȧ

#define	GUI_TOOL_STOPWATCH_MENU_CONTINUE	0	//����
#define	GUI_TOOL_STOPWATCH_MENU_FINISH		1	//���
#define	GUI_TOOL_STOPWATCH_MENU_RECORD		2	//�鿴
#define	GUI_TOOL_STOPWATCH_MENU_RESET		3	//����
#define GUI_TOOL_STOPWATCH_MENU_MAX			GUI_TOOL_STOPWATCH_MENU_RESET

static uint8_t m_gui_stopwatch_menu_index = GUI_TOOL_STOPWATCH_MENU_CONTINUE;

static am_util_stopwatch_t m_tool_stopwatch;
static tool_Stopwatch_t m_tool_stopwatch_data = {0};	//�������
static uint8_t m_tool_stopwatch_record_page;

extern SetValueStr SetValue;

uint8_t tool_stopwatch_status_get(void)
{
	//���״̬
	return m_tool_stopwatch_data.status;
}

uint32_t tool_stopwatch_time_get(void)
{
	//����ʱʱ��
	m_tool_stopwatch_data.time = am_util_stopwatch_elapsed_get(&m_tool_stopwatch,AM_UTIL_STOPWATCH_MS);
	
	return m_tool_stopwatch_data.time;
}

void tool_stopwatch_overtime(void)
{
	//����ʱ��ʱ
	m_tool_stopwatch_data.status = TOOL_STOPWATCH_STATUS_OVERTIME;
	m_tool_stopwatch_data.time = TOOL_STOPWATCH_TIME_MAX - 1;
	
	//��ͣ��ʱ
	am_util_stopwatch_stop(&m_tool_stopwatch);
	
	//���ز˵�����
	ScreenState = DISPLAY_SCREEN_STOPWATCH_MENU;
	m_gui_stopwatch_menu_index = GUI_TOOL_STOPWATCH_MENU_FINISH;
}

uint8_t gui_tool_stopwatch_time(uint8_t color)
	
{

   	SetWord_t word = {0};
	char str[10] = {0};
	uint16_t len;
	uint32_t ms = m_tool_stopwatch_data.time;
  if(ScreenState == DISPLAY_SCREEN_STOPWATCH_MENU)
  	{
	     
		
		//��:��
		memset(str,0,sizeof(str));
		sprintf(str,"%d:%02d",ms / 60000,ms / 1000 % 60);
		
		//len = ((Font_Number_49.width+2)*strlen(str)) + 6 + (2*Font_Number_29.width) + 2;
		
		len = ((Font_Number_56.width+2)*strlen(str)) + 6 + (2*Font_Number_37.width) + 2;

		//:Ϊһ����
		len -= (Font_Number_56.width/2);
		//word.x_axis = 52;
		word.x_axis = 32;
		
		word.y_axis = (LCD_PIXEL_ONELINE_MAX - len)/2;
		
		
		word.size = LCD_FONT_56_SIZE;
		word.kerning = 2;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_NONE;
		LCD_SetNumber(str,&word);
		
		//����
		word.y_axis += (len - (2*Font_Number_37.width) - 2);
		word.size = LCD_FONT_37_SIZE;
		memset(str,0,sizeof(str));
		sprintf(str,"%02d",ms % 1000 / 10);
		LCD_SetNumber(str,&word);


    }
  else
  	{

	
	//��:��
	memset(str,0,sizeof(str));
	sprintf(str,"%d:%02d",ms / 60000,ms / 1000 % 60);
	
	len = ((Font_Number_56.width+2)*strlen(str)) + 6 + (2*Font_Number_37.width) + 2;

	//:Ϊһ����
	len -= (Font_Number_56.width/2);
	word.x_axis = 120 - Font_Number_56.height/2;
	
	word.y_axis = (LCD_PIXEL_ONELINE_MAX - len)/2;
	
	LCD_SetRectangle(word.x_axis, Font_Number_56.height, word.y_axis, len, (~color)&0x77, 0, 0, LCD_FILL_ENABLE);
	
	word.size = LCD_FONT_56_SIZE;
	word.kerning = 2;
	word.forecolor = color;
	word.bckgrndcolor = LCD_NONE;
	LCD_SetNumber(str,&word);
	
	//����
	word.y_axis += (len - (2*Font_Number_37.width) - 2);
	word.size = LCD_FONT_37_SIZE;
	memset(str,0,sizeof(str));
	sprintf(str,"%02d",ms % 1000 / 10);
	LCD_SetNumber(str,&word);
  	}
	return word.x_axis;
}

void gui_tool_stopwatch_record(void)
{
	SetWord_t word = {0};
	char str[20] = {0};
	uint8_t i,page;
	
	//����
	
	LCD_SetRectangle(160, 58, 0, 240, LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
	
	if(m_tool_stopwatch_data.status == TOOL_STOPWATCH_STATUS_RESET)
	{
		//��OK����ʼ/��ͣ
		word.x_axis = 190;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		LCD_SetRectangle(word.x_axis,22,0,240,LCD_GRAY,0,0,LCD_FILL_ENABLE);
		
		word.x_axis += 3;
		word.forecolor = LCD_BLACK;
		word.bckgrndcolor = LCD_NONE;
		word.size = LCD_FONT_16_SIZE;
		word.kerning = 1;
		LCD_SetString((char *)StopwatchStrs[SetValue.Language][2],&word);
		
		//UP���ƴ�,DOWN����Ȧ
		word.x_axis = 190 - 10 - 16;
		
		word.forecolor = LCD_WHITE;
		LCD_SetString((char *)StopwatchStrs[SetValue.Language][3],&word);
	}
//	else if(m_tool_stopwatch_data.status == TOOL_STOPWATCH_STATUS_OVERTIME)
//	{
//		//��ʱ��99:59:99���ӣ���ʾ
//		word.x_axis = 138;
//		word.y_axis = LCD_CENTER_JUSTIFIED;
//		word.bckgrndcolor = LCD_BLACK;
//		word.forecolor = LCD_WHITE;
//		word.size = LCD_FONT_16_SIZE;
//		LCD_SetString("��OK�����¼�ʱ",&word);
//	}
	else
	{
		//�������ļƴκͼ�Ȧ��¼
		//word.x_axis = 130;
		
		word.x_axis = 145 +8;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_BLACK;
		
		if(m_tool_stopwatch_data.page > 0)
		{
			page = m_tool_stopwatch_data.page;
			
			for(i = 0;i < 4;i++)
			{
				word.y_axis = (LCD_PIXEL_ONELINE_MAX - 112)/2;
				LCD_SetRectangle(word.x_axis, 22, word.y_axis, 112, LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
				
				if(page > 0)
				{
					page -= 1;
					word.size = LCD_FONT_16_SIZE;
					word.kerning = 0;
					if(m_tool_stopwatch_data.record[page].type == TOOL_STOPWATCH_COUNT)
					{
						//�ƴ�
						LCD_SetString((char *)StopwatchStrs[SetValue.Language][0],&word);
					}
					else
					{
						//��Ȧ
						LCD_SetString((char *)StopwatchStrs[SetValue.Language][1],&word);
					}
								
					//���
					word.x_axis += 2;
					word.y_axis += (2*16);
					word.size = LCD_FONT_16_SIZE;
					word.kerning = 0;
					memset(str,0,sizeof(str));
					sprintf(str,"%d ",m_tool_stopwatch_data.record[page].num);
					LCD_SetString(str,&word);

					word.y_axis += 16;
					if(m_tool_stopwatch_data.record[page].min < 10)
					{
						word.y_axis += 8;
					}
					memset(str,0,sizeof(str));
					sprintf(str,"%d:%02d.%02d",m_tool_stopwatch_data.record[page].min,m_tool_stopwatch_data.record[page].sec,
													m_tool_stopwatch_data.record[page].ms);
					LCD_SetString(str,&word);
					
					word.x_axis += 20;
				}
				else
				{
					break;
				}		
			}
		}
		else
		{
			//�޼�Ȧ�ƴμ�¼
			word.x_axis += 9;
			word.y_axis = LCD_CENTER_JUSTIFIED;
			word.size = LCD_FONT_16_SIZE;
			word.kerning = 0;
			//LCD_SetString("--------------",&word);
		}
	}
}

void gui_tool_stopwatch_button(void)
{
	uint8_t color;
	
	if(m_tool_stopwatch_data.status == TOOL_STOPWATCH_STATUS_START)
	{
		//��ʾUP���ƴ�
		if(m_tool_stopwatch_data.count < TOOL_STOPWATCH_COUNT_MAX)
		{
			color = LCD_RED;
		}
		else
		{
			color = LCD_BLACK;
		}
		
		//gui_button_up(color);

		
		gui_button_middle(color);
		//��ʾDOWN����Ȧ
		if(m_tool_stopwatch_data.lap < TOOL_STOPWATCH_LAP_MAX)
		{
			color = LCD_RED;
		}
		else
		{
			color = LCD_BLACK;
		}
		
		//gui_button_down(color);
		
		gui_button_back(color);
	}
	else if(m_tool_stopwatch_data.status >= TOOL_STOPWATCH_STATUS_STOP)
	{
		//��ʾ�Ϸ�
		if(m_tool_stopwatch_data.page < (m_tool_stopwatch_data.count + m_tool_stopwatch_data.lap))
		{
			color = LCD_WHITE;
		}
		else
		{
			color = LCD_BLACK;
		}
		//gui_button_up(color);
		
		gui_button_middle(color);
		
		//��ʾ�·�
		if(m_tool_stopwatch_data.page > 4)
		{
			color = LCD_WHITE;
		}
		else
		{
			color = LCD_BLACK;
		}
		//gui_button_down(color);
		
		gui_button_back(color);
	}
}

void gui_tool_stopwatch_init(void)
{
	//�������
	memset(&m_tool_stopwatch_data, 0, sizeof(tool_Stopwatch_t));
	
	am_util_stopwatch_clear(&m_tool_stopwatch);
}

void gui_tool_stopwatch_paint(void)
{
	//����������Ϊ��ɫ
	LCD_SetBackgroundColor(LCD_BLACK);
	
	//���ͼ��
	
	LCD_SetPicture(15,LCD_CENTER_JUSTIFIED,LCD_NONE,LCD_NONE,&img_tools_big_lists[3]);

	//ʱ��
	gui_tool_stopwatch_time(LCD_WHITE);
	
	
	//�ƴμ�Ȧ��¼
	gui_tool_stopwatch_record();
	
	//������ʾ
	gui_tool_stopwatch_button();
	
	//�ײ�ʱ��
//	gui_bottom_time();
}

static void gui_tool_stopwatch_menu(uint8_t index)
{
	SetWord_t word = {0};
	uint8_t index1,index2;
	
	switch(index)
	{
		case GUI_TOOL_STOPWATCH_MENU_CONTINUE:
			index1 = 4;
			index2 = 5;
			break;
		case GUI_TOOL_STOPWATCH_MENU_FINISH:
			index1 = 5;
			index2 = 6;
			break;
		case GUI_TOOL_STOPWATCH_MENU_RECORD:
			index1 = 6;
			index2 = 7;
			break;
		default:
			index1 = 7;
			break;
	}
	
//	word.x_axis = 120 + 12;
	/*81*/
	word.x_axis = 81 + 12;
	LCD_SetRectangle(word.x_axis, 2, LCD_LINE_CNT_MAX/2 - 90, 180, LCD_LIGHTGRAY, 0, 0, LCD_FILL_ENABLE);

	word.x_axis += 2 + 24;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_CYAN;
	word.bckgrndcolor = LCD_NONE;
	word.size = LCD_FONT_24_SIZE;
	word.kerning = 0;
	LCD_SetString((char *)StopwatchStrs[SetValue.Language][index1],&word);
	
	//�ָ���
	word.x_axis +=(LCD_FONT_24_SIZE + 24);
	
	LCD_SetRectangle(word.x_axis, 2, LCD_LINE_CNT_MAX/2 - 90, 180, LCD_LIGHTGRAY, 0, 0, LCD_FILL_ENABLE);
	
	if(index != GUI_TOOL_STOPWATCH_MENU_MAX)
	{
		word.x_axis += (2 + 12);
		word.forecolor = LCD_GRAY;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString((char *)StopwatchStrs[SetValue.Language][index2],&word);
	}
}

void gui_tool_stopwatch_menu_paint(void)
{
	//����
	LCD_SetRectangle(0, 240, 0, LCD_PIXEL_ONELINE_MAX, LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
	
	//���ͼ��
	//LCD_SetPicture(15,108,LCD_NONE,LCD_NONE,&img_tools_lists[4]);
	
	//����ʱ
	gui_tool_stopwatch_time(LCD_BLACK);
	
	
	//�˵�ѡ��
	gui_tool_stopwatch_menu(m_gui_stopwatch_menu_index);
	
	//�·���ʾ
	gui_page_down(LCD_PERU);
}

void gui_tool_stopwatch_menu_init(void)
{
	if(m_tool_stopwatch_data.status == TOOL_STOPWATCH_STATUS_OVERTIME)
	{
		//Ĭ��"���"ѡ��
		m_gui_stopwatch_menu_index = GUI_TOOL_STOPWATCH_MENU_FINISH;
	}
	else
	{
		//Ĭ��"����"ѡ��
		m_gui_stopwatch_menu_index = GUI_TOOL_STOPWATCH_MENU_CONTINUE;
	}
}

void gui_tool_stopwatch_record_paint(void)
{
	SetWord_t word = {0};
	char str[10] = {0};
	uint8_t page;
	uint32_t ms = m_tool_stopwatch_data.time;
	
	//����������Ϊ��ɫ
	LCD_SetBackgroundColor(LCD_BLACK);
	

	
	if(m_tool_stopwatch_record_page > 0)
	{

	    	//��:��
		memset(str,0,sizeof(str));
		sprintf(str,"%d:%02d",ms / 60000,ms / 1000 % 60);
		
		word.x_axis = 20;
		word.y_axis = (LCD_PIXEL_ONELINE_MAX - ((Font_Number_29.width+2)*strlen(str) + (Font_Number_29.width/2)- 6) 
						- (2*Font_Number_19.width) - 2)/2;
		word.size = LCD_FONT_29_SIZE;
		word.kerning = 2;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_NONE;
		LCD_SetNumber(str,&word);
		
		//����
		word.y_axis += ((Font_Number_29.width+2)*strlen(str) - (Font_Number_29.width/2) + 6);
		word.size = LCD_FONT_19_SIZE;
		memset(str,0,sizeof(str));
		sprintf(str,"%02d",ms % 1000 / 10);
		LCD_SetNumber(str,&word);

		
		word.x_axis += (Font_Number_32.height + 8);
		page = m_tool_stopwatch_record_page;
	
		if(m_tool_stopwatch_record_page < (m_tool_stopwatch_data.count + m_tool_stopwatch_data.lap))
		{
			//�Ϸ���ʾ
			word.x_axis += 6;
			LCD_SetPicture(word.x_axis, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Up_12X8);
			word.x_axis += (Img_Pointing_Up_12X8.height + 6);
		}
		else
		{
			word.x_axis += 20;
		}
		
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_BLACK;
		for(uint8_t i = 0;i < 6;i++)
		{
			word.y_axis = (LCD_PIXEL_ONELINE_MAX - 112)/2;
			
			if(page > 0)
			{
				page -= 1;
				word.size = LCD_FONT_16_SIZE;
				word.kerning = 0;
				if(m_tool_stopwatch_data.record[page].type == TOOL_STOPWATCH_COUNT)
				{
					//�ƴ�
					LCD_SetString((char *)StopwatchStrs[SetValue.Language][0],&word);
				}
				else
				{
					//��Ȧ
					LCD_SetString((char *)StopwatchStrs[SetValue.Language][1],&word);
				}
							
				//���
				word.x_axis += 2;
				word.y_axis += (2*16);
				word.size = LCD_FONT_16_SIZE;
				word.kerning = 0;
				memset(str,0,sizeof(str));
				sprintf(str,"%d ",m_tool_stopwatch_data.record[page].num);
				LCD_SetString(str,&word);

				word.y_axis += 16;
				if(m_tool_stopwatch_data.record[page].min < 10)
				{
					word.y_axis += 8;
				}
				memset(str,0,sizeof(str));
				sprintf(str,"%d:%02d.%02d",m_tool_stopwatch_data.record[page].min,m_tool_stopwatch_data.record[page].sec,
												m_tool_stopwatch_data.record[page].ms);
				LCD_SetString(str,&word);
				
				word.x_axis += 20;
			}
			else
			{
				break;
			}
		}
		
		if(m_tool_stopwatch_record_page > 6)
		{
			//�·���ʾ
			gui_page_down(LCD_PERU);
		}
	}
	else
	{
		//�޼�Ȧ�ƴμ�¼

			    	//��:��
		memset(str,0,sizeof(str));
		sprintf(str,"%d:%02d",ms / 60000,ms / 1000 % 60);
		
		word.x_axis = 120 - Font_Number_49.height/2;
		word.y_axis = (LCD_PIXEL_ONELINE_MAX - ((Font_Number_49.width+2)*strlen(str) + (Font_Number_49.width/2)- 6) 
						- (2*Font_Number_29.width) - 2)/2;
		word.size = LCD_FONT_49_SIZE;
		word.kerning = 2;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_NONE;
		LCD_SetNumber(str,&word);
		
		//����
		word.y_axis += ((Font_Number_49.width+2)*strlen(str) - (Font_Number_49.width/2) + 6);
		word.size = LCD_FONT_29_SIZE;
		memset(str,0,sizeof(str));
		sprintf(str,"%02d",ms % 1000 / 10);
		LCD_SetNumber(str,&word);
		
		
	}
}

void gui_tool_stopwatch_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch(Key_Value)
	{
		case KEY_OK:
			if(m_tool_stopwatch_data.status == TOOL_STOPWATCH_STATUS_RESET)
			{
				//��ʼ��ʱ
				m_tool_stopwatch_data.status = TOOL_STOPWATCH_STATUS_START;
				am_util_stopwatch_init(&m_tool_stopwatch);
				am_util_stopwatch_restart(&m_tool_stopwatch);
				
				msg.cmd = MSG_UPDATE_STOPWATCH_RECORD;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
			else if(m_tool_stopwatch_data.status == TOOL_STOPWATCH_STATUS_START)
			{
				//��ͣ��ʱ
				m_tool_stopwatch_data.status = TOOL_STOPWATCH_STATUS_STOP;
				am_util_stopwatch_stop(&m_tool_stopwatch);
				
				//�������˵�����
				ScreenState = DISPLAY_SCREEN_STOPWATCH_MENU;
				gui_tool_stopwatch_menu_init();
				
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
			else if(m_tool_stopwatch_data.status == TOOL_STOPWATCH_STATUS_STOP)
			{
				//������ʼ��ʱ
				m_tool_stopwatch_data.status = TOOL_STOPWATCH_STATUS_START;
				
				am_util_stopwatch_start(&m_tool_stopwatch);
				
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
//			else
//			{
//				//���¿�ʼ��ʱ
//				gui_tool_stopwatch_init();
//				m_tool_stopwatch_data.status = TOOL_STOPWATCH_STATUS_START;
//				
//				am_util_stopwatch_init(&m_tool_stopwatch);
//				am_util_stopwatch_restart(&m_tool_stopwatch);

//				msg.cmd = MSG_UPDATE_STOPWATCH_RECORD;
//				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
//			}				
			break;
		case KEY_UP:
			if(m_tool_stopwatch_data.status == TOOL_STOPWATCH_STATUS_START)
			{
				//�ƴ�
				if(m_tool_stopwatch_data.count < TOOL_STOPWATCH_COUNT_MAX)
				{
					uint32_t i,ms;
					i = m_tool_stopwatch_data.count + m_tool_stopwatch_data.lap;
					ms = am_util_stopwatch_elapsed_get(&m_tool_stopwatch,AM_UTIL_STOPWATCH_MS);
					m_tool_stopwatch_data.record[i].type = TOOL_STOPWATCH_COUNT;
					m_tool_stopwatch_data.record[i].num = ++m_tool_stopwatch_data.count;
					m_tool_stopwatch_data.record[i].min = ms / 60000;
					m_tool_stopwatch_data.record[i].sec = ms / 1000 % 60;
					m_tool_stopwatch_data.record[i].ms = ms % 1000 / 10;
					m_tool_stopwatch_data.page = i + 1;
					
					msg.cmd = MSG_UPDATE_STOPWATCH_RECORD;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
					
					//������
					timer_notify_motor_start(300,300,1,false,NOTIFY_MODE_TOOLS);
					timer_notify_buzzer_start(300,300,1,false,NOTIFY_MODE_TOOLS);
				}
			}
			else if(m_tool_stopwatch_data.status >= TOOL_STOPWATCH_STATUS_STOP)
			{
				//�Ϸ�
				if((m_tool_stopwatch_data.page + 4) <= (m_tool_stopwatch_data.count + m_tool_stopwatch_data.lap))
				{
					m_tool_stopwatch_data.page += 4;
					msg.cmd = MSG_UPDATE_STOPWATCH_RECORD;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				}
			}
			break;
		case KEY_DOWN:
			if(m_tool_stopwatch_data.status == TOOL_STOPWATCH_STATUS_START)
			{
				//��Ȧ
				if(m_tool_stopwatch_data.lap < TOOL_STOPWATCH_LAP_MAX)
				{
					uint32_t i,ms,tmp;
					i = m_tool_stopwatch_data.count + m_tool_stopwatch_data.lap;
					ms = am_util_stopwatch_elapsed_get(&m_tool_stopwatch,AM_UTIL_STOPWATCH_MS);
					tmp = ms - m_tool_stopwatch_data.last_time;
					m_tool_stopwatch_data.record[i].type = TOOL_STOPWATCH_LAP;
					m_tool_stopwatch_data.record[i].num = ++m_tool_stopwatch_data.lap;
					m_tool_stopwatch_data.record[i].min = tmp / 60000;
					m_tool_stopwatch_data.record[i].sec = tmp / 1000 % 60;
					m_tool_stopwatch_data.record[i].ms = tmp % 1000 / 10;
					m_tool_stopwatch_data.last_time = ms;
					m_tool_stopwatch_data.page = i + 1;
					
					msg.cmd = MSG_UPDATE_STOPWATCH_RECORD;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
					
					//������
					timer_notify_motor_start(300,300,1,false,NOTIFY_MODE_TOOLS);
					timer_notify_buzzer_start(300,300,1,false,NOTIFY_MODE_TOOLS);
				}
			}
			else if(m_tool_stopwatch_data.status >= TOOL_STOPWATCH_STATUS_STOP)
			{
				//�·�
				if(m_tool_stopwatch_data.page > 4)
				{
					m_tool_stopwatch_data.page -= 4;
					
					msg.cmd = MSG_UPDATE_STOPWATCH_RECORD;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				}
			}
			break;
		case KEY_BACK:
			if(m_tool_stopwatch_data.status == TOOL_STOPWATCH_STATUS_RESET)
			{
				//���ع���ѡ�����
				tool_task_close();
				ScreenState = DISPLAY_SCREEN_TOOL;
				msg.cmd = MSG_DISPLAY_SCREEN;
				msg.value =Get_Curr_Tool_Index();
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
			else if(m_tool_stopwatch_data.status >= TOOL_STOPWATCH_STATUS_STOP)				
			{
				//�������˵�����
				ScreenState = DISPLAY_SCREEN_STOPWATCH_MENU;
				gui_tool_stopwatch_menu_init();
				
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
			break;
		default:
			break;
	}
}

void gui_tool_stopwatch_menu_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch(Key_Value)
	{
		case KEY_OK:
			if(m_gui_stopwatch_menu_index == GUI_TOOL_STOPWATCH_MENU_CONTINUE)
			{
				//������ʱ
				m_tool_stopwatch_data.status = TOOL_STOPWATCH_STATUS_START;
				
				ScreenState = DISPLAY_SCREEN_STOPWATCH;
				
				am_util_stopwatch_start(&m_tool_stopwatch);
			}
			else if(m_gui_stopwatch_menu_index == GUI_TOOL_STOPWATCH_MENU_FINISH)
			{
				//���ش�������
				tool_task_close();
				
				gui_tool_stopwatch_init();
				
				ScreenState = DISPLAY_SCREEN_HOME;
			}
			else if(m_gui_stopwatch_menu_index == GUI_TOOL_STOPWATCH_MENU_RECORD)
			{
				//�鿴��ʱ��¼
				ScreenState = DISPLAY_SCREEN_STOPWATCH_RECORD;
				
				m_tool_stopwatch_record_page = m_tool_stopwatch_data.count + m_tool_stopwatch_data.lap;
			}
			else
			{
				//�������
				gui_tool_stopwatch_init();
				m_tool_stopwatch_data.status = TOOL_STOPWATCH_STATUS_RESET;
				
				ScreenState = DISPLAY_SCREEN_STOPWATCH;
			}
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		case KEY_UP:
			if(m_gui_stopwatch_menu_index > GUI_TOOL_STOPWATCH_MENU_CONTINUE)
			{
				m_gui_stopwatch_menu_index--;
				if((m_tool_stopwatch_data.status == TOOL_STOPWATCH_STATUS_OVERTIME)
					&& (m_gui_stopwatch_menu_index == GUI_TOOL_STOPWATCH_MENU_CONTINUE))
				{
					//���ʱ������"����"ѡ��
					m_gui_stopwatch_menu_index = GUI_TOOL_STOPWATCH_MENU_MAX;
				}
			}
			else
			{
				m_gui_stopwatch_menu_index = GUI_TOOL_STOPWATCH_MENU_MAX;
			}
			
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		case KEY_DOWN:
			if(m_gui_stopwatch_menu_index < GUI_TOOL_STOPWATCH_MENU_MAX)
			{
				m_gui_stopwatch_menu_index++;
			}
			else
			{
				if(m_tool_stopwatch_data.status == TOOL_STOPWATCH_STATUS_OVERTIME)
				{
					//���ʱ������"����"ѡ��
					m_gui_stopwatch_menu_index = GUI_TOOL_STOPWATCH_MENU_FINISH;
				}
				else
				{
					m_gui_stopwatch_menu_index = GUI_TOOL_STOPWATCH_MENU_CONTINUE;
				}
			}
			
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		case KEY_BACK:
			//����������
			ScreenState = DISPLAY_SCREEN_STOPWATCH;
			
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}

void gui_tool_stopwatch_record_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch(Key_Value)
	{
		case KEY_UP:
			//�Ϸ�
			if((m_tool_stopwatch_record_page + 6) <= (m_tool_stopwatch_data.count + m_tool_stopwatch_data.lap))
			{
				m_tool_stopwatch_record_page += 6;
				
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
			break;
		case KEY_DOWN:
			//�·�
			if(m_tool_stopwatch_record_page > 6)
			{
				m_tool_stopwatch_record_page -= 6;
				
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
			break;
		case KEY_BACK:
			//�����ϼ��˵�����
			ScreenState = DISPLAY_SCREEN_STOPWATCH_MENU;
			m_gui_stopwatch_menu_index = GUI_TOOL_STOPWATCH_MENU_RECORD;
			
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}

