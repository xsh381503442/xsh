/*
*�������ܽ���:ѵ���ƻ�
*/
#include "gui_trainplan.h"
#include "gui_home.h"
#include "gui_heartrate.h"
#include "gui_sport_cycling.h"
#include "gui_sport.h"
#include "gui_run.h"

#include "task_display.h"
#include "task_key.h"
#include "task_sport.h"

#include "drv_lcd.h"
#include "drv_key.h"

#include "bsp_rtc.h"
#include "time_notify.h"

#include "mode_power_off.h"

#include "img_sport.h"
#include "font_config.h"
#include "com_data.h"
#include "com_sport.h"
#include "algo_trainplan.h"
#include "gui.h"
#if DEBUG_ENABLED == 1 && GUI_TRAINPLAN_LOG_ENABLED == 1
	#define GUI_TRAINPLAN_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_TRAINPLAN_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define GUI_TRAINPLAN_WRITESTRING(...)
	#define GUI_TRAINPLAN_PRINTF(...)		        
#endif
static uint8_t selectRunIndex = 0;

static uint8_t m_enter_trainplan_way = WAYS_ENTER_TRAINPLAN_SPORT_STANDBUY;//����ѵ���ƻ��˶��ķ�ʽ �������������ܲ��˶���������ѽ����

/*���� ���ַ�ʽ����ѵ���ƻ��˶�*/
void set_enter_trainplan_way(uint8_t ways)
{
	m_enter_trainplan_way = ways;
}
/*��ȡ ���ַ�ʽ����ѵ���ƻ��˶�*/
uint8_t get_enter_trainplan_way(void)
{
	return m_enter_trainplan_way;
}
/*ѵ���ƻ�����*/
static void gui_trainplan_title_paint(void)
{
	SetWord_t word = {0};
	char str[30];
	
	word.x_axis = 10;
	word.y_axis = LCD_CENTER_JUSTIFIED;
  LCD_SetPicture(word.x_axis, word.y_axis, LCD_AUQAMARIN, LCD_BLACK, &Img_Sport[1]);

	memset(str,0,30);
	sprintf(str,"%.20s",TrainPlanDownloadSettingStr.NameStr);
	word.x_axis = 72 - 24;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	word.kerning = 1;
	LCD_DisplayGTString(str,&word);
}

/*�����ѵ���ƻ���Ϣ 1:��������ʱ�����£�0ʱ�����ϣ���������*/
static void gui_trainplan_today_content_paint(bool flag)
{
	SetWord_t word = {0};
	char str[30];
	
	memset(str,0,30);
	
	if(flag)
	{
		sprintf(str,"%.20s",Download[1].ContentStr);
		word.size = LCD_FONT_24_SIZE;
		if(Download[1].Type != Rest)
		{
			word.x_axis = LCD_LINE_CNT_MAX/2 - 24;
		}
		else
		{
			word.x_axis = LCD_LINE_CNT_MAX/2 - 12;
		}
	}
	else
	{
		sprintf(str,"%d-%02d-%02d %02d:%02d",RTC_time.ui32Year+2000,RTC_time.ui32Month,RTC_time.ui32DayOfMonth,
	                             TrainPlanDownloadSettingStr.Hint.Hour,TrainPlanDownloadSettingStr.Hint.Minute);
		word.size = LCD_FONT_16_SIZE;
		word.x_axis = LCD_LINE_CNT_MAX/2 - 16;
	}
	
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	word.kerning = 1;
	LCD_DisplayGTString(str,&word);
	
	memset(str,0,30);
	if(flag)
	{
		if(Get_IsCompleteTodayTrainPlan() == 1)
		{
			if(Download[1].Type != Rest)
			{
				sprintf(str,"�����");
				word.size = LCD_FONT_24_SIZE;
				word.y_axis = LCD_CENTER_JUSTIFIED;
				word.x_axis = LCD_LINE_CNT_MAX/2 + 8;
				word.forecolor = LCD_WHITE;
				word.bckgrndcolor = LCD_BLACK;
				word.kerning = 1;
				LCD_DisplayGTString(str,&word);
			}
		}
		else
		{
			sprintf(str,"%d-%02d-%02d %02d:%02d",RTC_time.ui32Year+2000,RTC_time.ui32Month,RTC_time.ui32DayOfMonth,
	                             TrainPlanDownloadSettingStr.Hint.Hour,TrainPlanDownloadSettingStr.Hint.Minute);
			word.size = LCD_FONT_16_SIZE;
			word.y_axis = LCD_CENTER_JUSTIFIED;
		word.x_axis = LCD_LINE_CNT_MAX/2 + 8;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_BLACK;
		word.kerning = 1;
		LCD_DisplayGTString(str,&word);
		}
	}
	else
	{
		sprintf(str,"%.20s",Download[1].ContentStr);
		word.size = LCD_FONT_24_SIZE;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.x_axis = LCD_LINE_CNT_MAX/2 + 8;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_BLACK;
		word.kerning = 1;
		LCD_DisplayGTString(str,&word);
	}

}
/*�����ѵ���ƻ���Ϣ
��� true �������ʾ����ѵ���ƻ����� �в���ʾ
     false �����������ʾ������
*/
static void gui_trainplan_yesterday_tommorow_paint(bool flag)
{
	SetWord_t word = {0};
	uint16_t day_sub = 0;
	char str[30];
	
	day_sub = DaysDiff(TrainPlanDownloadSettingStr.StopTime.Year,TrainPlanDownloadSettingStr.StopTime.Month,TrainPlanDownloadSettingStr.StopTime.Day
												,RTC_time.ui32Year,RTC_time.ui32Month,RTC_time.ui32DayOfMonth);

	if(flag)
	{
		memset(str,0,20);
		sprintf(str,"���쿪ʼ��һ��ѵ��");
		word.x_axis = 120;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_16_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_BLACK;
		word.kerning = 1;
		LCD_DisplayGTString(str,&word);
		
		word.x_axis += 24;
		memset(str,0,30);
		sprintf(str,"%.20s",Download[2].ContentStr);
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_16_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_BLACK;
		word.kerning = 1;
		LCD_DisplayGTString(str,&word);
	}
	else
	{
		if(day_sub > 0)
		{//ѵ���ƻ��������ǰ����ʾ����������ѵ���ƻ�����ʾ
			memset(str,0,30);
			sprintf(str,"����:");
			word.x_axis = 160 + 16;
			word.y_axis = 42;
			word.size = LCD_FONT_16_SIZE;
			word.forecolor = LCD_WHITE;
			word.bckgrndcolor = LCD_BLACK;
			word.kerning = 1;
			LCD_DisplayGTString(str,&word);
			
			memset(str,0,30);
			sprintf(str,"%.20s",Download[2].ContentStr);
			word.y_axis += 40;
			word.size = LCD_FONT_16_SIZE;
			word.forecolor = LCD_WHITE;
			word.bckgrndcolor = LCD_BLACK;
			word.kerning = 1;
			LCD_DisplayGTString(str,&word);
			
			word.x_axis = 160;
			LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX, LCD_LIGHTGRAY, 0, 0, LCD_FILL_ENABLE);
		}
	}
}
/*������ʾ:ʱ�仹δ��ѵ���ƻ���ʼʱ��*/
static void gui_trainplan_beforedays_paint(uint16_t days)
{
	SetWord_t word = {0};
	char str[30];
	
	gui_trainplan_title_paint();//����
	
	if(days < 100)
	{
#if defined WATCH_SIM_NUMBER
		char str_day[30];
		memset(str,0,15);
		sprintf(str,"�ƻ�����");
		memset(str_day,0,15);
		sprintf(str_day,"%d",days);
		word.x_axis = 136;
		word.y_axis = LCD_LINE_CNT_MAX/2 - strlen(str)*Font_Number_16.width - strlen(str_day)*Font_Number_32.width/2 - 6;
		word.size = LCD_FONT_16_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_BLACK;
		word.kerning = 1;
		LCD_DisplayGTString(str,&word);
		
		word.x_axis = 116;
		word.y_axis += (strlen(str)*Font_Number_16.width + 6);
		word.size = LCD_FONT_32_SIZE;
		LCD_SetNumber(str_day,&word);
		word.x_axis = 136;
		word.y_axis += (strlen(str_day)*Font_Number_32.width + 6);
		word.size = LCD_FONT_16_SIZE;
		memset(str,0,15);
		sprintf(str,"���ʼ");
		LCD_DisplayGTString(str,&word);
#else
		memset(str,0,15);
		sprintf(str,"�ƻ�����");
		word.x_axis = 136;
		word.y_axis = 38;
		word.size = LCD_FONT_16_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_BLACK;
		word.kerning = 1;
		LCD_DisplayGTString(str,&word);
		
		word.x_axis = 116;
		memset(str,0,15);
		sprintf(str,"%d",days);
		word.y_axis += 72;
		word.size = LCD_FONT_32_SIZE;
		LCD_SetNumber(str,&word);
		word.x_axis = 136;
		word.y_axis += 20+(strlen(str) - 1)*16;
		word.size = LCD_FONT_16_SIZE;
		memset(str,0,15);
		sprintf(str,"���ʼ");
		LCD_DisplayGTString(str,&word);
#endif
	}
	else if(days >= 100)
	{
		word.x_axis = 136;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_16_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_BLACK;
		word.kerning = 1;
		memset(str,0,30);
		sprintf(str,"�ƻ�����������ʼ");
		LCD_DisplayGTString(str,&word);
	}

	
	word.x_axis = 80;
	LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX, LCD_LIGHTGRAY, 0, 0, LCD_FILL_ENABLE);
}
static void gui_trainplan_hint_paint(void)
{
	SetWord_t word = {0};
	
	word.x_axis = 30;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	word.kerning = 1;
	LCD_DisplayGTString("ѵ���ƻ�",&word);
	
	word.size = LCD_FONT_16_SIZE;
	word.x_axis = 104;
	LCD_DisplayGTString("�����ؾ����˶�APP",&word);
	word.x_axis = 128;
	LCD_DisplayGTString("�ƶ������Լ��ļƻ���!",&word);
}
static void gui_trainplan_enddays_paint(void)
{
	SetWord_t word = {0};
	char str[30];
	
	if(Get_IsCompleteTodayTrainPlan() == 1)
	{
		word.x_axis = 120;
		memset(str,0,30);
		sprintf(str,"%.20s",Download[1].ContentStr);
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_24_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_BLACK;
		word.kerning = 1;
		LCD_DisplayGTString(str,&word);
		
		word.x_axis += 32;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_16_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_BLACK;
		word.kerning = 1;
		LCD_DisplayGTString("�����",&word);
	}
	else
	{
		word.x_axis = 120;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_16_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_BLACK;
		word.kerning = 1;
		LCD_DisplayGTString("������ѵ���ƻ����һ��",&word);
		
		word.x_axis += 24;
		memset(str,0,30);
		sprintf(str,"%.20s",Download[1].ContentStr);
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_16_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_BLACK;
		word.kerning = 1;
		LCD_DisplayGTString(str,&word);
	}

}

static void gui_trainplan_morethran_paint(void)
{
	SetWord_t word = {0};
	char str[30];
	
	word.x_axis = 30;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	word.kerning = 1;
	LCD_DisplayGTString("ѵ���ƻ�",&word);
	memset(str,0,30);
	sprintf(str,"%.20s������",TrainPlanDownloadSettingStr.NameStr);
	word.x_axis = 104;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	word.kerning = 1;
	LCD_DisplayGTString(str,&word);
	
	word.x_axis += 24;
	memset(str,0,30);
	sprintf(str,"��鿴�����˶�APP");
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	word.kerning = 1;
	LCD_DisplayGTString(str,&word);
}
/*������ʾ:ѵ���ƻ�֮������Ŀ����ʾ����
��ͬ��ѵ���ƻ�ʱ,�����ɾ������,������ʾ����ѵ���ƻ�
APPͬ�����ݺ�,����ɾ������ʱ�仹δ��ѵ���ƻ�ʱ�� ��ʾ��������;��ʱ����ǽ�������ʾ�����������
*/
void gui_trainplan_target_paint(void)
{
	SetWord_t word = {0};
	int16_t beforeDays = 0;
	char str[30];
	am_hal_rtc_time_get(&RTC_time);
	//���ñ�����ɫ
	LCD_SetBackgroundColor(LCD_BLACK);
	//����
	LCD_SetRectangle(0, LCD_LINE_CNT_MAX, 0, LCD_LINE_CNT_MAX, LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
	
	if(get_trainplan_valid_flag())
	{//��ѵ���ƻ�ͬ��
		beforeDays = Get_TrainPlanBeforeDays();
		GUI_TRAINPLAN_PRINTF("[GUI_TRAINPLAN]:beforeDays=%d\n",beforeDays);
		if(Is_ValidTimeTrainPlan())
		{
			if(beforeDays > 0)
			{//ʱ�仹δ��ѵ���ƻ���ʼʱ��
				gui_trainplan_beforedays_paint(beforeDays);
			}
		else
		{
		  if(Is_TodayTimeTrainPlan())
			{//ѵ���ƻ����ڼƻ�������
				if(Get_SomeDayofTrainPlan() == Get_TrainPlanTotalDays())
				{//ѵ���ƻ��������һ��
					gui_trainplan_title_paint();//����
					gui_trainplan_enddays_paint();
				}
				else
				{
					if(Download[1].Type <= Rest_Run_Distance)
					{
						gui_trainplan_title_paint();//����
						gui_trainplan_today_content_paint(true);//�����ѵ���ƻ���Ϣ
						gui_trainplan_yesterday_tommorow_paint(false);//����/�����ѵ���ƻ���Ϣ
						word.x_axis = 80;
						LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX, LCD_LIGHTGRAY, 0, 0, LCD_FILL_ENABLE);
					}
					else
					{
						memset(str,0,30);
						sprintf(str,"ѵ���ƻ�������������!");
						word.y_axis = LCD_CENTER_JUSTIFIED;
						word.x_axis = LCD_LINE_CNT_MAX/2;
						word.size = LCD_FONT_16_SIZE;
						word.forecolor = LCD_WHITE;
						word.bckgrndcolor = LCD_BLACK;
						word.kerning = 1;
						LCD_DisplayGTString(str,&word);
					}
				}
			}
			else
			{//ѵ���ƻ��ѳ�һ�죬��ʾ��������� ѵ���ƻ���1�����ϵ�,�������ʾѵ���ƻ���� ��û����ʾѵ���ƻ�δ���������
				gui_trainplan_morethran_paint();
			}
		}
		}
		else
		{
			memset(str,0,30);
			sprintf(str,"ѵ���ƻ�����ʱ������!");
			word.y_axis = LCD_CENTER_JUSTIFIED;
			word.x_axis = LCD_LINE_CNT_MAX/2;
			word.size = LCD_FONT_16_SIZE;
			word.forecolor = LCD_WHITE;
			word.bckgrndcolor = LCD_BLACK;
			word.kerning = 1;
			LCD_DisplayGTString(str,&word);
		}
	}
	else
	{//��ѵ���ƻ�ͬ��
		gui_trainplan_hint_paint();
	}
}
/*������ʾ:ѵ���ƻ���������ʾ����*/
void gui_trainplan_today_hint_paint(void)
{
	SetWord_t word = {0};
	char str[60],p_str[60],p_str_end[60];
	//���ñ�����ɫ
	LCD_SetBackgroundColor(LCD_BLACK);
	gui_trainplan_title_paint();//����
	gui_trainplan_today_content_paint(false);//����ƻ�
	word.x_axis = 80;
	LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX, LCD_LIGHTGRAY, 0, 0, LCD_FILL_ENABLE);
	
	GUI_TRAINPLAN_PRINTF("[gui_trainplan]:trainplan Type=%d,Download[1].Type=%d\n",
	Get_TrainPlanType(Download[1].Type),Download[1].Type);
	memset(str,0,60);
	sprintf(str,"%.30s",Download[1].SkillsStr);//���ݻ��߼���
	if(Get_TrainPlanType(Download[1].Type) == 0)
	{
		if(strlen(str) > 20)
		{
			memset(p_str,0,60);
			memset(p_str_end,0,60);
			memcpy(p_str,str,20);
			for(uint8_t i=20;i<sizeof(Download[1].SkillsStr);i++)
			{
				p_str_end[i-20] = Download[1].SkillsStr[i];
			}
			word.size = LCD_FONT_16_SIZE;
			word.x_axis = 166;
			word.y_axis = LCD_CENTER_JUSTIFIED;
			word.forecolor = LCD_WHITE;
			word.bckgrndcolor = LCD_BLACK;
			word.kerning = 1;
			LCD_DisplayGTString(p_str,&word);
			sprintf(str,"%.60s",p_str_end);//���ݻ��߼���
			word.x_axis += 24;
			LCD_DisplayGTString(str,&word);
			
		}
		else
		{
			word.size = LCD_FONT_16_SIZE;
			word.x_axis = 166;
			word.y_axis = LCD_CENTER_JUSTIFIED;
			word.forecolor = LCD_WHITE;
			word.bckgrndcolor = LCD_BLACK;
			word.kerning = 1;
			LCD_DisplayGTString(str,&word);
		}

	}
	else
	{
		memset(str,0,30);
		sprintf(str,"��ok����ʼ�˶���!");
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.x_axis = 166;
		word.size = LCD_FONT_16_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_BLACK;
		word.kerning = 1;
		LCD_DisplayGTString(str,&word);
	}
}
/*������ʾ:ѵ���ƻ�֮������Ŀ�����������ʾ����*/
void gui_trainplan_target_hint_paint(void)
{
	SetWord_t word = {0};
	char str[30];
	//���ñ�����ɫ
	LCD_SetBackgroundColor(LCD_BLACK);
	
	word.x_axis = 72 - 24;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	word.kerning = 1;
	LCD_DisplayGTString(TrainPlanDownloadSettingStr.NameStr,&word);//����
	
	memset(str,0,30);
	sprintf(str,"%.20s",Download[1].ContentStr);
	word.size = LCD_FONT_24_SIZE;
	word.x_axis = LCD_LINE_CNT_MAX/2 - 24;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	word.kerning = 1;
	LCD_DisplayGTString(str,&word);
	
	word.x_axis += 40;
	LCD_DisplayGTString("�����",&word);
	word.x_axis += 64;
	word.size = LCD_FONT_16_SIZE;
	LCD_DisplayGTString("��OK������ѵ��!",&word);
	gui_point_direction(IMG_POINT_LEFT,LCD_WHITE);
	LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 8, 0, 360, LCD_GREEN);
}
/*������ʾ:ѵ���ƻ�ÿ�����ͼ����� ����*/
void gui_trainplan_today_type_content_paint(void)
{
	SetWord_t word = {0};
	char str[60],p_str[60],p_str_end[60];
	//���ñ�����ɫ
	LCD_SetBackgroundColor(LCD_BLACK);
	
	memset(str,0,30);
	sprintf(str,"%d-%02d-%02d",RTC_time.ui32Year,RTC_time.ui32Month,RTC_time.ui32DayOfMonth);//��������
	word.size = LCD_FONT_24_SIZE;
	word.x_axis = 36;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	word.kerning = 1;
	LCD_DisplayGTString(str,&word);
	
	memset(str,0,30);
	sprintf(str,"%.20s",Download[1].ContentStr);//�����
	word.size = LCD_FONT_24_SIZE;
	word.x_axis = LCD_LINE_CNT_MAX/2 - 24;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	word.kerning = 1;
	LCD_DisplayGTString(str,&word);
	
	memset(str,0,60);
	
	sprintf(str,"%.30s",Download[1].SkillsStr);//���ݻ��߼���
	if(strlen(str) > 20)
	{
		memset(p_str,0,60);
		memset(p_str_end,0,60);
		memcpy(p_str,str,20);
		for(uint8_t i=20;i<sizeof(Download[1].SkillsStr);i++)
		{
			p_str_end[i-20] = Download[1].SkillsStr[i];
		}
		word.size = LCD_FONT_16_SIZE;
		word.x_axis += 40;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_BLACK;
		word.kerning = 1;
		LCD_DisplayGTString(p_str,&word);
		sprintf(str,"%.60s",p_str_end);//���ݻ��߼���
		word.x_axis += 24;
		LCD_DisplayGTString(str,&word);
		
	}
	else
	{
		word.size = LCD_FONT_16_SIZE;
		word.x_axis += 40;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_BLACK;
		word.kerning = 1;
		LCD_DisplayGTString(str,&word);
	}
}
/*������ʾ:ѵ���ƻ��Ƿ���ɵ���Ŀ��*/
void gui_trainplan_iscomplete_paint(void)
{
	SetWord_t word = {0};
	char str[30];
	//���ñ�����ɫ
	LCD_SetBackgroundColor(LCD_BLACK);
	
	word.size = LCD_FONT_16_SIZE;
	word.x_axis = LCD_LINE_CNT_MAX/2 - 16;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	word.kerning = 1;
	LCD_DisplayGTString("�Ƿ���ɵ���",&word);
	memset(str,0,30);
	sprintf(str,"%.20s?",Download[1].ContentStr);//�����
	word.size = LCD_FONT_16_SIZE;
	word.x_axis = LCD_LINE_CNT_MAX/2 + 12;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	word.kerning = 1;
	LCD_DisplayGTString(str,&word);
	
	gui_tick_cross();
}
/*������ʾ:ѵ���ƻ��˶�����������*/
void gui_trainplan_pace_up_down_paint(uint16_t now_pace)
{
	SetWord_t word = {0};
	char str[60],p_str[60],p_str_end[60];
	//���ñ�����ɫ
	LCD_SetBackgroundColor(LCD_BLACK);
	
	word.size = LCD_FONT_24_SIZE;
	word.x_axis = 36;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	word.kerning = 1;
	LCD_DisplayGTString("ѵ���ƻ�����",&word);
	
	memset(str,0,30);
	sprintf(str,"%d:%02d",now_pace/60,now_pace%60);//����
	word.size = LCD_FONT_32_SIZE;
	word.x_axis = 90;
	word.y_axis = 90;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	word.kerning = 1;
	LCD_SetNumber(str,&word);
	
	word.x_axis += 12;
	word.y_axis += strlen(str)*16;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString("/km",&word);
	
	memset(str,0,60);
	sprintf(str,"%.30s",Download[1].SkillsStr);//���ݻ��߼���
	if(strlen(str) > 20)
	{
		memset(p_str,0,60);
		memset(p_str_end,0,60);
		memcpy(p_str,str,20);
		for(uint8_t i=20;i<sizeof(Download[1].SkillsStr);i++)
		{
			p_str_end[i-20] = Download[1].SkillsStr[i];
		}
		word.size = LCD_FONT_16_SIZE;
		word.x_axis += 40;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_BLACK;
		word.kerning = 1;
		LCD_DisplayGTString(p_str,&word);
		sprintf(str,"%.60s",p_str_end);//���ݻ��߼���
		word.x_axis += 24;
		LCD_DisplayGTString(str,&word);
		
	}
	else
	{
		word.size = LCD_FONT_16_SIZE;
		word.x_axis += 40;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_BLACK;
		word.kerning = 1;
		LCD_DisplayGTString(str,&word);
	}
}
/*������ʾ:ѵ���ƻ��˶��м�Ъ�ܵ���������*/
void gui_trainplan_pace_inter_paint(uint16_t num)
{
	SetWord_t word = {0};
	uint16_t singleDis = 0,total_num = 0,num_up = 0,num_down = 0;
	char str[30];
	
	num_up = (num+1)/2;
	singleDis = Get_Intervals_TrainPlan_Single_Distance();
	num_down = num;
	total_num = Download[1].IntermittentRun;
	
	//���ñ�����ɫ
	LCD_SetBackgroundColor(LCD_BLACK);
	
	word.size = LCD_FONT_24_SIZE;
	word.x_axis = 36;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	word.kerning = 1;
	LCD_DisplayGTString("��Ъѵ��",&word);
	
	word.x_axis += 32;
	LCD_DisplayGTString("������",&word);
	if((num+1)%2 == 0)
	{//��400...
		memset(str,0,30);
		sprintf(str,"%dx%dm ��",num_up,Download[1].RestTime);//4x400...
	}
	else
	{//��200...
		memset(str,0,30);
		sprintf(str,"%dx%dm ��",num_up,singleDis);//4x400...
	}
	word.size = LCD_FONT_24_SIZE;
	word.x_axis += 64;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	word.kerning = 1;
	LCD_SetString(str,&word);
	
	gui_point_direction(IMG_POINT_LEFT,LCD_WHITE);
	
	memset(str,0,20);
	sprintf(str,"(%d/%d)",num_down,total_num);//(1/8)
	word.size = LCD_FONT_16_SIZE;
	word.x_axis += 32;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	word.kerning = 1;
	LCD_SetString(str,&word);
	LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 8, 0, 360, LCD_GREEN);
}
/*������ʾ:ѵ���ƻ�APP�򿪻��߹ر�����
���:0�ر�ѵ���ƻ� 1��ѵ���ƻ���������  2��ѵ���ƻ����ر�����
*/
void gui_trainplan_sw_hint_paint(uint16_t hintFlag)
{
	SetWord_t word = {0};
	char str[30];
	
	//���ñ�����ɫ
	LCD_SetBackgroundColor(LCD_BLACK);
	
	word.x_axis = 10;
	word.y_axis = LCD_CENTER_JUSTIFIED;
  LCD_SetPicture(word.x_axis, word.y_axis, LCD_AUQAMARIN, LCD_BLACK, &Img_Sport[1]);

	memset(str,0,30);
	sprintf(str,"%s","ѵ���ƻ�");
	word.x_axis = 72 - 24;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	word.kerning = 1;
	LCD_DisplayGTString(str,&word);
	
	word.x_axis = LCD_CENTER_JUSTIFIED/2-20;
	switch(hintFlag)
	{
		case 0:
		{
			LCD_DisplayGTString("ѵ���ƻ��ѹر�",&word);
		}
			break;
		case 1:
		{
			LCD_DisplayGTString("����ѵ���ƻ���",&word);
			word.x_axis = LCD_CENTER_JUSTIFIED/2+20;
			LCD_DisplayGTString("����ÿ������",&word);
		}
			break;
		case 2:
		{
			LCD_DisplayGTString("����ѵ���ƻ���",&word);
			word.x_axis = LCD_CENTER_JUSTIFIED/2+20;
			LCD_DisplayGTString("�ر�ÿ������",&word);
		}
			break;
		default:
			break;
	}
}
/*������ʾ:�˶�ģʽ����ʱ��ѵ���ƻ�ѡ�� ִ��ѵ���ƻ����ܲ�����ѡ��*/
void gui_trainplan_run_select_paint(void)
{
	SetWord_t word = {0};
	char str[30];
	
	LCD_SetBackgroundColor(LCD_WHITE);
	LCD_SetRectangle(0,LCD_LINE_CNT_MAX/2,0,LCD_LINE_CNT_MAX,LCD_BLACK,0,0,LCD_FILL_ENABLE);

#if defined(WATCH_SIM_SPORT)
	if(selectRunIndex == 0)
	{
		word.x_axis = 40;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_16_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_BLACK;
		word.kerning = 1;
		LCD_DisplayGTString("��������ѵ���ƻ�",&word);
		
		word.x_axis += 32;
		memset(str,0,30);
		sprintf(str,"%.20s",Download[1].ContentStr);
		word.size = LCD_FONT_24_SIZE;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_BLACK;
		word.kerning = 1;
		LCD_DisplayGTString(str,&word);
	}
	else
	{
		word.x_axis = 60;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_24_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_BLACK;
		word.kerning = 1;
		LCD_DisplayGTString("��ʼ�ܲ���",&word);
	}
#else
	word.x_axis = 40;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	word.kerning = 1;
	LCD_DisplayGTString("��������ѵ���ƻ�",&word);
	
	word.x_axis += 32;
	memset(str,0,30);
	sprintf(str,"%.20s",Download[1].ContentStr);
	word.size = LCD_FONT_24_SIZE;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	word.kerning = 1;
	LCD_DisplayGTString(str,&word);
#endif
	//ѡ��
	if(selectRunIndex == 0)
	{
		word.x_axis = 128;
		word.y_axis = 50;
		LCD_SetPicture(word.x_axis, word.y_axis, LCD_AUQAMARIN, LCD_WHITE, &Img_Sport[1]);
		word.x_axis += 8;
		word.y_axis += 46;
		word.size = LCD_FONT_24_SIZE;
		word.forecolor = LCD_BLACK;
		word.bckgrndcolor = LCD_WHITE;
		word.kerning = 1;
		if(Download[1].Type == Rest)
		{
			LCD_DisplayGTString("��Ϣ",&word);
		}
		else
		{
			LCD_DisplayGTString("��ʼѵ��",&word);
		}
		
		word.x_axis = LCD_LINE_CNT_MAX/2+58;
		word.y_axis = 50;
		LCD_SetPicture(word.x_axis, word.y_axis, LCD_AUQAMARIN, LCD_WHITE, &Img_Sport[0]);
		word.x_axis += 8;
		word.y_axis += 46;
		word.size = LCD_FONT_24_SIZE;
		word.forecolor = LCD_BLACK;
		word.bckgrndcolor = LCD_WHITE;
		word.kerning = 1;
		LCD_DisplayGTString("�ܲ�",&word);
	}
	else
	{
		word.x_axis = 128;
		word.y_axis = 50;
		LCD_SetPicture(word.x_axis, word.y_axis, LCD_AUQAMARIN, LCD_WHITE, &Img_Sport[0]);
		word.x_axis += 8;
		word.y_axis += 46;
		word.size = LCD_FONT_24_SIZE;
		word.forecolor = LCD_BLACK;
		word.bckgrndcolor = LCD_WHITE;
		word.kerning = 1;
		LCD_DisplayGTString("�ܲ�",&word);
		
		word.x_axis = LCD_LINE_CNT_MAX/2+58;
		word.y_axis = 50;
		LCD_SetPicture(word.x_axis, word.y_axis, LCD_AUQAMARIN, LCD_WHITE, &Img_Sport[1]);
		word.x_axis += 8;
		word.y_axis += 46;
		word.size = LCD_FONT_24_SIZE;
		word.forecolor = LCD_BLACK;
		word.bckgrndcolor = LCD_WHITE;
		word.kerning = 1;
		if(Download[1].Type == Rest)
		{
			LCD_DisplayGTString("��Ϣ",&word);
		}
		else
		{
			LCD_DisplayGTString("��ʼѵ��",&word);
		}
	}

	word.x_axis = LCD_LINE_CNT_MAX/2+56;
	LCD_SetRectangle(word.x_axis,2,0,LCD_LINE_CNT_MAX, LCD_LIGHTGRAY, 0, 0, LCD_FILL_ENABLE);
	//�¼�ͷ
	LCD_SetPicture(230, 116, LCD_BLACK, LCD_NONE, &Img_Pointing_Down_12X8);
}
/*��������:ѵ���ƻ�֮������Ŀ����ʾ����*/
void gui_trainplan_target_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	switch( Key_Value )
	{
		case KEY_OK:
			{
				if(Is_TodayTimeTrainPlan() && get_trainplan_valid_flag())
				{//ѵ���ƻ�ʱ����
					ScreenState = DISPLAY_SCREEN_TRAINPLAN_HOME_RUN_HINT;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				}
			}
			break;
#if  defined WATCH_COM_SHORTCUNT_UP
		case KEY_UP:
		case KEY_DOWN:
			set_switch_screen_count_evt(Key_Value);
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
			ScreenState = DISPLAY_SCREEN_HOME;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}
/*��������:���������ѵ���ƻ���������ʾ����*/
void gui_trainplan_today_hint_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	switch( Key_Value )
	{
		case KEY_OK:
			timer_notify_motor_stop(false);
			timer_notify_buzzer_stop(false);
			if(Get_TrainPlanType(Download[1].Type) == 0)
			{
				//�ر��ϸ����������
				mode_close();
				
				ScreenState = DISPLAY_SCREEN_HOME;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
			else if(Get_TrainPlanType(Download[1].Type) == 1 && Get_IsCompleteTodayTrainPlan() != 1)
			{
				//�ر��ϸ����������
				mode_close();
				set_enter_trainplan_way(WAYS_ENTER_TRAINPLAN_SPORT_REMIND);//���ѽ������ѵ���ƻ�
				//��Ϣ���߽���ѵ���Ƚ���  ȷ���Ƿ����
				ScreenState = DISPLAY_SCREEN_TRAINPLAN_IS_COMPLETE;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
			else if((Get_TrainPlanType(Download[1].Type) == 2
				      || Get_TrainPlanType(Download[1].Type) == 3
			        || Get_TrainPlanType(Download[1].Type) == 4
			        || Get_TrainPlanType(Download[1].Type) == 5) && Get_IsCompleteTodayTrainPlan() != 1)
			{
				//�ر��ϸ����������
				mode_close();
				
				//�ܲ�
				if(1 == IS_SPORT_CONTINUE_LATER)
				{//������Ժ����,���������ſ��Խ���
					SportContinueLaterDisplay();
				}
				else
				{
					set_enter_trainplan_way(WAYS_ENTER_TRAINPLAN_SPORT_REMIND);//���ѽ������ѵ���ƻ�
					ReadySportBack();
					ReadySport(DISPLAY_SCREEN_TRAINPLAN_RUN);
#if defined WATCH_SIM_SPORT
					ScreenState = DISPLAY_SCREEN_RUN;
					ScreenSportSave = DISPLAY_SCREEN_RUN;
					gui_sport_start_init();
#elif defined WATCH_COM_SPORT
					ScreenState = DISPLAY_SCREEN_RUN;
					gui_sport_run_init();

#else
					ScreenState = DISPLAY_SCREEN_RUN_READY;
#endif
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
					timer_notify_motor_start(1000,500,1,false,NOTIFY_MODE_TOOLS);
					timer_notify_buzzer_start(1000,500,1,false,NOTIFY_MODE_TOOLS);
				}
			}
			break;
		case KEY_BACK:
			timer_notify_motor_stop(false);
			timer_notify_buzzer_stop(false);

			if(Get_TrainPlanType(Download[1].Type) == 0)
			{
//				//�ر��ϸ����������
//				mode_close();
				
				ScreenState = DISPLAY_SCREEN_HOME;
			}
      else
			{
				ScreenState = ScreenStateSave;
			}
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
			break;
		default:
			break;
	}
}
/*��������:����������ѵ���ƻ�������������ʾ����*/
void gui_trainplan_home_today_hint_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	switch( Key_Value )
	{
		case KEY_OK:
			if(get_trainplan_valid_flag())
			{
				if(Get_TrainPlanType(Download[1].Type) == 1 && Get_IsCompleteTodayTrainPlan() != 1)
				{//��Ϣ���߽���ѵ���Ƚ���  ȷ���Ƿ����
					Set_IsTrainPlanOnGoing(true);
					set_enter_trainplan_way(WAYS_ENTER_TRAINPLAN_SPORT_STANDBUY);//������ݽ������ѵ���ƻ�
					ScreenState = DISPLAY_SCREEN_TRAINPLAN_IS_COMPLETE;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				}
				else if((Get_TrainPlanType(Download[1].Type) == 2
								|| Get_TrainPlanType(Download[1].Type) == 3
								|| Get_TrainPlanType(Download[1].Type) == 4
								|| Get_TrainPlanType(Download[1].Type) == 5) && Get_IsCompleteTodayTrainPlan() != 1)
				{//�ܲ�
					if(1 == IS_SPORT_CONTINUE_LATER)
					{//������Ժ����,���������ſ��Խ���
						SportContinueLaterDisplay();
					}
					else
					{
						set_enter_trainplan_way(WAYS_ENTER_TRAINPLAN_SPORT_STANDBUY);//������ݽ������ѵ���ƻ�
						ReadySportBack();
						ReadySport(DISPLAY_SCREEN_TRAINPLAN_RUN);
#if defined WATCH_SIM_SPORT
						ScreenState = DISPLAY_SCREEN_RUN;
						ScreenSportSave = DISPLAY_SCREEN_RUN;
						gui_sport_start_init();
#elif defined WATCH_COM_SPORT
						ScreenState = DISPLAY_SCREEN_RUN;
						gui_sport_run_init();

#else
						ScreenState = DISPLAY_SCREEN_RUN_READY;
#endif
						msg.cmd = MSG_DISPLAY_SCREEN;
						xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
						timer_notify_motor_start(1000,500,1,false,NOTIFY_MODE_TOOLS);
						timer_notify_buzzer_start(1000,500,1,false,NOTIFY_MODE_TOOLS);
					}
				}
			}
			break;
		case KEY_BACK:
			ScreenState = DISPLAY_SCREEN_TRAINPLAN_MARATHON;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}
/*��������:�˶�������ѵ���ƻ�������������ʾ����*/
void gui_trainplan_sport_today_hint_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	switch( Key_Value )
	{
		case KEY_OK:
			//״̬��Ϊ0
			if(get_trainplan_valid_flag())
			{
				if(Get_TrainPlanType(Download[1].Type) == 1 && Get_IsCompleteTodayTrainPlan() != 1)
				{//��Ϣ���߽���ѵ���Ƚ���  ȷ���Ƿ����
					Set_IsTrainPlanOnGoing(true);
					ScreenState = DISPLAY_SCREEN_TRAINPLAN_IS_COMPLETE;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				}
				else if((Get_TrainPlanType(Download[1].Type) == 2
								|| Get_TrainPlanType(Download[1].Type) == 3
								|| Get_TrainPlanType(Download[1].Type) == 4
								|| Get_TrainPlanType(Download[1].Type) == 5) && Get_IsCompleteTodayTrainPlan() != 1)
				{//�ܲ�
					if(1 == IS_SPORT_CONTINUE_LATER)
					{//������Ժ����,���������ſ��Խ���
						SportContinueLaterDisplay();
					}
					else
					{
						ReadySportBack();
						ReadySport(DISPLAY_SCREEN_TRAINPLAN_RUN);
#if defined WATCH_SIM_SPORT
						ScreenState = DISPLAY_SCREEN_RUN;
						ScreenSportSave = DISPLAY_SCREEN_RUN;
						gui_sport_start_init();
#elif defined WATCH_COM_SPORT
					ScreenState = DISPLAY_SCREEN_RUN;
					gui_sport_run_init();
#else

						ScreenState = DISPLAY_SCREEN_RUN_READY;
#endif
						msg.cmd = MSG_DISPLAY_SCREEN;
						xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
						timer_notify_motor_start(1000,500,1,false,NOTIFY_MODE_TOOLS);
						timer_notify_buzzer_start(1000,500,1,false,NOTIFY_MODE_TOOLS);
					}
				}
				else
				{
					break;
				}
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
			break;
		case KEY_BACK:
			//�����˶�ѡ�����
			ScreenState = DISPLAY_SCREEN_SPORT;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			ReadySportBack();
			break;
		default:
			break;
	}
}
/*��������:ѵ���ƻ�Ŀ��������ʾ����*/
void gui_trainplan_target_hint_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	switch( Key_Value )
	{
		case KEY_OK:
			timer_notify_motor_stop(false);
			timer_notify_buzzer_stop(false);
			//�����˶���ͣ����
#if defined WATCH_SIM_SPORT
			gui_sport_pause_evt();
#elif defined WATCH_COM_SPORT
			gui_sport_pause_evt();

#else
			Go_to_Run_Pause();
#endif
			break;
		case KEY_BACK:
//			timer_notify_display_stop(false);
			timer_notify_motor_stop(false);
			timer_notify_buzzer_stop(false);	
			//Ŀ����ɺ������ͣ����
			ScreenState = ScreenStateSave;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
			break;
		default:
			break;
	}
}
/*��������:����������ѵ���ƻ�ȷ���Ƿ���ɽ���*/
void gui_trainplan_is_complete_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
			Set_IsTrainPlanOnGoing(false);
			if(Get_IsCompleteTodayTrainPlan() != 1)
			{
				Set_IsCompleteTodayTrainPlan(1);
				Store_TranPlanUploadData();
			}

			ScreenState = DISPLAY_SCREEN_TRAINPLAN_MARATHON;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		case KEY_BACK:
			Set_IsTrainPlanOnGoing(false);
			if(Get_IsCompleteTodayTrainPlan() != 1)
			{
				Set_IsCompleteTodayTrainPlan(0);
//				Store_TranPlanUploadData();
			}
			if(Get_Curr_Home_Index() == DISPLAY_SCREEN_TRAINPLAN_MARATHON)
			{
				ScreenState = DISPLAY_SCREEN_TRAINPLAN_MARATHON;
			}
			else
			{
				ScreenState = DISPLAY_SCREEN_TRAINPLAN_RUN_SELECT;
			}
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}
/*��������:ѵ���ƻ��˶�����������*/
void gui_trainplan_pace_up_down_btn_evt(uint32_t Key_Value)
{
	switch( Key_Value )
	{
		case KEY_OK:
		case KEY_BACK:
		{
			timer_notify_display_stop(false);
			timer_notify_motor_stop(false);
			timer_notify_buzzer_stop(false);	
			DISPLAY_MSG  msg = {0,0};
			ScreenState = ScreenStateSave;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
		}
			break;
		default:
			break;
	}
}
/*��������:ѵ���ƻ���Ъ���˶�����������*/
void gui_trainplan_pace_inter_btn_evt(uint32_t Key_Value)
{
	switch( Key_Value )
	{
		case KEY_OK:
		case KEY_BACK:
		{
			timer_notify_display_stop(false);
			timer_notify_motor_stop(false);
			timer_notify_buzzer_stop(false);	
			DISPLAY_MSG  msg = {0,0};
			ScreenState = ScreenStateSave;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
		}
			break;
		default:
			break;
	}
}
/*������ʾ:ѵ���ƻ�APP�򿪻��߹ر�����
*/
void gui_trainplan_sw_hint_btn_evt(uint32_t Key_Value)
{
	switch( Key_Value )
	{
		case KEY_OK:
		case KEY_BACK:
		{
			timer_notify_display_stop(false);
			timer_notify_motor_stop(false);
			timer_notify_buzzer_stop(false);	
			DISPLAY_MSG  msg = {0,0};
			ScreenState = ScreenStateSave;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
		}
			break;
		default:
			break;
	}
}
/*��������:�˶�ģʽ����ʱ��ѵ���ƻ�ѡ�� ִ��ѵ���ƻ����ܲ�����ѡ��*/
void gui_trainplan_run_select_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0,0};
	switch( Key_Value )
	{
		case KEY_OK:
			if(selectRunIndex == 0)
			{//ִ��ѵ���ƻ�
				if(get_trainplan_valid_flag())
				{
					if(Get_TrainPlanType(Download[1].Type) == 1 && Get_IsCompleteTodayTrainPlan() != 1)
					{//��Ϣ���߽���ѵ���Ƚ���  ȷ���Ƿ����
						set_enter_trainplan_way(WAYS_ENTER_TRAINPLAN_SPORT_RUN);//�ܲ��˶��������ѵ���ƻ�
						Set_IsTrainPlanOnGoing(true);
						ScreenState = DISPLAY_SCREEN_TRAINPLAN_IS_COMPLETE;
						msg.cmd = MSG_DISPLAY_SCREEN;
						xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
					}
					else if((Get_TrainPlanType(Download[1].Type) == 2
									|| Get_TrainPlanType(Download[1].Type) == 3
									|| Get_TrainPlanType(Download[1].Type) == 4
									|| Get_TrainPlanType(Download[1].Type) == 5) && Get_IsCompleteTodayTrainPlan() != 1)
					{//�ܲ�
						if(1 == IS_SPORT_CONTINUE_LATER)
						{//������Ժ����,���������ſ��Խ���
							SportContinueLaterDisplay();
						}
						else
						{
							set_enter_trainplan_way(WAYS_ENTER_TRAINPLAN_SPORT_RUN);//�ܲ��˶��������ѵ���ƻ�
							ReadySportBack();
							ReadySport(DISPLAY_SCREEN_TRAINPLAN_RUN);
#if defined WATCH_SIM_SPORT
							ScreenState = DISPLAY_SCREEN_RUN;
							ScreenSportSave = DISPLAY_SCREEN_RUN;
							gui_sport_start_init();
#elif defined WATCH_COM_SPORT
							ScreenState = DISPLAY_SCREEN_RUN;
							gui_sport_run_init();

#else

							ScreenState = DISPLAY_SCREEN_RUN_READY;
#endif
							msg.cmd = MSG_DISPLAY_SCREEN;
							xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
							timer_notify_motor_start(1000,500,1,false,NOTIFY_MODE_TOOLS);
							timer_notify_buzzer_start(1000,500,1,false,NOTIFY_MODE_TOOLS);
						}
					}
					else if(Get_TrainPlanType(Download[1].Type) == 0)
					{
						ScreenState = DISPLAY_SCREEN_HOME;
						msg.cmd = MSG_DISPLAY_SCREEN;
						xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
					}
					else
					{
						break;
					}
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				}
			}
			else
			{//�ܲ�
					set_enter_trainplan_way(WAYS_ENTER_TRAINPLAN_SPORT_RUN);//�ܲ��˶����� ѵ���ƻ�ѡ����ѡ�������ܲ��˶�
					ReadySportBack();
					ReadySport(DISPLAY_SCREEN_RUN);
					
					
					ScreenState = DISPLAY_SCREEN_SPORT_READY;
					ScreenSportSave = DISPLAY_SCREEN_RUN;

					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
					timer_notify_motor_start(1000,500,1,false,NOTIFY_MODE_TOOLS);
			}
			break;
		case KEY_UP:
		case KEY_DOWN:
		{
			if(selectRunIndex == 0)
			{
				selectRunIndex = 1;
			}
			else
			{
				selectRunIndex = 0;
			}
			ScreenState = DISPLAY_SCREEN_TRAINPLAN_RUN_SELECT;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}
			break;
		case KEY_BACK:
		{
			ScreenState = DISPLAY_SCREEN_SPORT;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			ReadySportBack();
		}
			break;
		default:
			break;
	}
}

