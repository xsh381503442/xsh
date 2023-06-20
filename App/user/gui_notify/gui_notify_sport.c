//Author: Jason Gong
//Time��20180302 V1.40
/*This is the file for drawing sport notify interfaces, which would be including 4 sport
goal notify interfaces including time,distance,speed,lap and alititude notify interface*/

#include "gui_notify_sport.h"
#include "drv_lcd.h"
#include "drv_key.h"
#include "font_config.h"
#include "task_display.h"
#include "img_home.h"
#include <string.h>
#include "time_notify.h"
#include "com_data.h"
#include "com_sport.h"
#include "watch_config.h"

#define GUI_NOTIFY_SPORT_LOG_ENASPORTD 1

#if DEBUG_ENASPORTD == 1 && GUI_NOTIFY_SPORT_LOG_ENASPORTD == 1
	#define GUI_NOTIFY_SPORT_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_NOTIFY_SPORT_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define GUI_NOTIFY_SPORT_WRITESTRING(...)
	#define GUI_NOTIFY_SPORT_PRINTF(...)		        
#endif

last_circle_t last_circle_data;

void gui_notify_sport_circle_paint(uint32_t circle)			//Ŀ��Ȧ������ 
{
	GUI_NOTIFY_SPORT_PRINTF("[GUI_NOTIFY_ALARM]:gui_notify_sport_circle_paint\n"); 
	
	SetWord_t word = {0};
	char p_str[20];

	LCD_SetBackgroundColor(LCD_WHITE);
	
	LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 121, 5, 0, 360, LCD_CYAN);
	
	//��ǰȦ��
	  word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_NONE;
	word.x_axis = 120 - 72/2;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_72_SIZE;
	word.kerning = 1;
	memset(p_str,0,sizeof(p_str));
	sprintf(p_str,"%d",circle);
	LCD_SetNumber(p_str,&word);	
	
#if defined WATCH_SIM_NUMBER
	word.x_axis = 30 + 16 + 4;
	word.y_axis = LCD_CENTER_JUSTIFIED;
#else
	word.x_axis += (Font_Number_88.height - Font_Char_16.height);
	word.y_axis = (LCD_PIXEL_ONELINE_MAX - (strlen(p_str)*(Font_Number_88.width + 1)) + 1)/2 
					+ (strlen(p_str)*(Font_Number_88.width + 1)) + 3;
#endif
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 0;
	LCD_SetString("laps",&word);	
	
	//Ŀ��Ȧ��
	word.x_axis = 30;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	LCD_SetString((char *)NotifyMenuStrs[SetValue.Language][4],&word);
	
	//����ɣ�
	word.x_axis = 194;
	word.forecolor = LCD_CYAN;
	LCD_SetString((char *)NotifyMenuStrs[SetValue.Language][9],&word);
}

void gui_notify_sport_time_paint(uint32_t time)		//Ŀ��ʱ�����
{
	GUI_NOTIFY_SPORT_PRINTF("[GUI_NOTIFY_ALARM]:gui_notify_sport_time_paint\n"); 	
	
	SetWord_t word = {0};
	char p_str[20];

	LCD_SetBackgroundColor(LCD_WHITE);
	
	LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 121, 5, 0, 360, LCD_CYAN);
	
	//��ǰʱ��
	   word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_NONE;
	word.x_axis = 120 - 72/2;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_72_SIZE;
	word.kerning = 1;
	memset(p_str,0,sizeof(p_str));
	sprintf(p_str,"%d",time/60);
	LCD_SetNumber(p_str,&word);	
	
#if defined WATCH_SIM_NUMBER
	word.x_axis = 30 + 16 + 4;
	word.y_axis = LCD_CENTER_JUSTIFIED;
#else
	word.x_axis += (Font_Number_88.height - Font_Char_16.height);
	word.y_axis = (LCD_PIXEL_ONELINE_MAX - (strlen(p_str)*(Font_Number_88.width + 1)) + 1)/2 
					+ (strlen(p_str)*(Font_Number_88.width + 1)) + 3;
#endif
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 0;
	LCD_SetString("min",&word);	
	
	//Ŀ��ʱ��
	word.x_axis = 30;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	LCD_SetString((char *)NotifyMenuStrs[SetValue.Language][5],&word);
	
	//����ɣ�
	word.x_axis = 194;
	word.forecolor = LCD_CYAN;
	LCD_SetString((char *)NotifyMenuStrs[SetValue.Language][9],&word);
}

void gui_notify_sport_distance_paint(uint32_t distance)			//Ŀ��������
{
	GUI_NOTIFY_SPORT_PRINTF("[GUI_NOTIFY_ALARM]:gui_notify_sport_time_paint\n"); 	
	
	SetWord_t word = {0};
	char p_str[20];

	LCD_SetBackgroundColor(LCD_WHITE);
	
	LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 121, 5, 0, 360, LCD_CYAN);
	
	//��ǰ����
	word.forecolor = LCD_BLACK;
	   word.bckgrndcolor = LCD_NONE;

	word.x_axis = 120 - 72/2;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_72_SIZE;
	word.kerning = 1;
	memset(p_str,0,sizeof(p_str));
	#if defined (SPRINTF_FLOAT_TO_INT)
	sprintf(p_str,"%d.%01d",distance/1000,distance/100%10);
	#else
	sprintf(p_str,"%.1f",distance/1000.f);
	#endif
	LCD_SetNumber(p_str,&word);	
	
#if defined WATCH_SIM_NUMBER
	word.x_axis = 30 + 16 + 4;
	word.y_axis = LCD_CENTER_JUSTIFIED;
#else
	word.x_axis += (Font_Number_88.height - Font_Char_16.height);
	word.y_axis = (LCD_PIXEL_ONELINE_MAX - (strlen(p_str)*(Font_Number_88.width + 1)) + 1)/2 
					+ (strlen(p_str)*(Font_Number_88.width + 1)) + 3;
#endif
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 0;
	LCD_SetString("km",&word);	
	
	//Ŀ�����
	word.x_axis = 30;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	LCD_SetString((char *)NotifyMenuStrs[SetValue.Language][6],&word);
	
	//����ɣ�
	word.x_axis = 194;
	word.forecolor = LCD_CYAN;
	LCD_SetString((char *)NotifyMenuStrs[SetValue.Language][9],&word);
}

void gui_notify_sport_pace_paint(uint32_t pace)			//Ŀ�����ٽ��� ��Сʱ/ǧ�ף�
{
	GUI_NOTIFY_SPORT_PRINTF("[GUI_NOTIFY_ALARM]:gui_notify_sport_time_paint\n");

	SetWord_t word = {0};
	char p_str[20];
	uint16_t now_pace;
	uint16_t goal_pace;
	
	//��ǰֵΪ���ε�16λ��Ŀ��ֵΪ���θ�16λ
	now_pace=0x0000FFFF&pace;
	goal_pace=(0xFFFF0000&pace)>>16;

	LCD_SetBackgroundColor(LCD_WHITE);

	//��Ȧ
	LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 121, 5, 0, 360, LCD_RED);
	
	//��ǰ����
	word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_NONE;
	word.x_axis = 120 - 72/2;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_72_SIZE;
	word.kerning = 1;
	memset(p_str,0,sizeof(p_str));
	sprintf(p_str,"%d:%02d",now_pace/60,now_pace%60);
	LCD_SetNumber(p_str,&word);	
	
#if defined WATCH_SIM_NUMBER
	word.x_axis = 30 + 16 + 4;
	word.y_axis = LCD_CENTER_JUSTIFIED;
#else
	word.x_axis += (Font_Number_88.height - Font_Char_16.height);
	word.y_axis = (LCD_PIXEL_ONELINE_MAX - (strlen(p_str)*(Font_Number_88.width + 1)) + 1)/2 
					+ (strlen(p_str)*(Font_Number_88.width + 1)) + 3;
#endif
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 0;
	LCD_SetString("/km",&word);	
	
	//Ŀ������
	word.x_axis = 30;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	LCD_SetString((char *)NotifyMenuStrs[SetValue.Language][7],&word);
	
	word.x_axis = 194;
	word.forecolor = LCD_RED;
	memset(p_str,0,sizeof(p_str));
	sprintf(p_str,"Ŀ��:%d:%02d/km",goal_pace/60,goal_pace%60);
	LCD_SetString(p_str,&word);
}

void gui_notify_sport_height_paint(uint32_t height)			//�߶����ѽ���
{
	GUI_NOTIFY_SPORT_PRINTF("[GUI_NOTIFY_ALARM]:gui_notify_sport_time_paint\n"); 	
	
	SetWord_t word = {0};
	char p_str[20];

	LCD_SetBackgroundColor(LCD_WHITE);
	
	LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 121, 5, 0, 360, LCD_CYAN);
	
	//��ǰ�߶�
	word.forecolor = LCD_BLACK;
	 word.bckgrndcolor = LCD_NONE;

	word.x_axis = 120 - 72/2;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_72_SIZE;
	word.kerning = 1;
	memset(p_str,0,sizeof(p_str));
	sprintf(p_str,"%d",height);
	LCD_SetNumber(p_str,&word);

	//��ǰ�߶�
	word.x_axis = 30;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 0;
	LCD_SetString((char *)NotifyMenuStrs[SetValue.Language][8],&word);
	
	//m
	word.x_axis = 190;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString("m",&word);
}

void gui_notify_sport_speed_paint(uint32_t speed)			//Ŀ���ٶȽ��� ��ǧ��/Сʱ�� 
{
	GUI_NOTIFY_SPORT_PRINTF("[GUI_NOTIFY_ALARM]:gui_notify_sport_speed_paint\n");

	SetWord_t word = {0};
	char p_str[20];
	uint16_t now_speed;
	uint16_t goal_speed;
	
	//��ǰֵΪ���ε�16λ��Ŀ��ֵΪ���θ�16λ
	now_speed=0x0000FFFF&speed;
	goal_speed=(0xFFFF0000&speed)>>16;

	LCD_SetBackgroundColor(LCD_WHITE);
	
	LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 121, 5, 0, 360, LCD_RED);
	
	//��ǰ�ٶ�
	word.forecolor = LCD_BLACK;
		word.bckgrndcolor = LCD_NONE;

	word.x_axis = 120 - 72/2;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_72_SIZE;
	word.kerning = 1;
	memset(p_str,0,sizeof(p_str));
	#if defined (SPRINTF_FLOAT_TO_INT)
	sprintf(p_str,"%d.%01d",now_speed/10,now_speed%10);
	#else
	sprintf(p_str,"%.1f",now_speed/10.f);
	#endif
	LCD_SetNumber(p_str,&word);	
	
#if defined WATCH_SIM_NUMBER
	word.x_axis = 30 + 16 + 4;
	word.y_axis = LCD_CENTER_JUSTIFIED;
#else
	word.x_axis += (Font_Number_88.height - Font_Char_16.height);
	word.y_axis = (LCD_PIXEL_ONELINE_MAX - (strlen(p_str)*(Font_Number_88.width + 1)) + 1)/2 
					+ (strlen(p_str)*(Font_Number_88.width + 1)) + 3;
#endif
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 0;
	LCD_SetString("km/h",&word);	
	
	//Ŀ���ٶ�
	word.x_axis = 30;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	LCD_SetString((char *)NotifyMenuStrs[SetValue.Language][11],&word);
	
	word.x_axis = 194;
	memset(p_str,0,sizeof(p_str));
	word.forecolor = LCD_RED;
	#if defined (SPRINTF_FLOAT_TO_INT)
	sprintf(p_str,"Ŀ��:%d.%01dkm/h",goal_speed/10,goal_speed%10);
	#else
	sprintf(p_str,"Ŀ��:%.1fkm/h",goal_speed/10.f);
	#endif
	LCD_SetString(p_str,&word);
}

void gui_notify_sport_calory_paint(uint32_t calory)			//�˶�ȼ֬���� 
{
	GUI_NOTIFY_SPORT_PRINTF("[GUI_NOTIFY_ALARM]:gui_notify_sport_calory_paint\n");

	SetWord_t word = {0};
	char p_str[20];

	LCD_SetBackgroundColor(LCD_WHITE);
	
	LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 121, 5, 0, 360, LCD_CYAN);
	
	//��ǰ��·��
	word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_NONE;
	word.x_axis = 120 - 72/2;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_72_SIZE;
	word.kerning = 1;
	memset(p_str,0,sizeof(p_str));
	sprintf(p_str,"%d",calory);
	LCD_SetNumber(p_str,&word);	
	
#if defined WATCH_SIM_NUMBER
	word.x_axis = 30 + 16 + 4;
	word.y_axis = LCD_CENTER_JUSTIFIED;
#else
	word.x_axis += (Font_Number_88.height - Font_Char_16.height);
	word.y_axis = (LCD_PIXEL_ONELINE_MAX - (strlen(p_str)*(Font_Number_88.width + 1)) + 1)/2 
					+ (strlen(p_str)*(Font_Number_88.width + 1)) + 3;
#endif
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 0;
	LCD_SetString("kcal",&word);	
	
	//Ŀ�꿨·��
	word.x_axis = 30;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	LCD_SetString((char *)NotifyMenuStrs[SetValue.Language][10],&word);
	
	//����ɣ�
	word.x_axis = 194;
	word.forecolor = LCD_CYAN;
	LCD_SetString((char *)NotifyMenuStrs[SetValue.Language][9],&word);
}

//�ܲ��Զ���Ȧ���ѽ���
void gui_notify_run_atuocircle_paint(last_circle_t data)
{
	SetWord_t word = {0};
	char p_str[20];
	
	LCD_SetBackgroundColor(LCD_WHITE);

	LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 121, 5, 0, 360, LCD_LIGHTGREEN);
		
	LCD_SetRectangle(80, 2, 0, LCD_LINE_CNT_MAX, LCD_LIGHTGREEN, 0, 0, LCD_FILL_ENABLE);
	
	LCD_SetRectangle(158, 2, 0, LCD_LINE_CNT_MAX, LCD_LIGHTGREEN, 0, 0, LCD_FILL_ENABLE);
	
	LCD_SetRectangle(80, 80, 120, 2, LCD_LIGHTGREEN, 0, 0, LCD_FILL_ENABLE);
	//Ȧ��
	word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_NONE;
	word.x_axis = 20;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_48_SIZE;
	word.kerning = 1;
	memset(p_str,0,sizeof(p_str));
	sprintf(p_str,"%d",data.lap);
	
	LCD_SetNumber(p_str,&word);	

	word.x_axis = 50;
	word.y_axis = 170;
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 0;
	LCD_SetString("lap",&word);

	//��һȦ��ʱ
	memset(p_str,0,sizeof(p_str));
	sprintf(p_str,"%d:%02d",data.time/60,data.time%60);
	word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_NONE;
	word.x_axis = 110;
	word.y_axis = (120 -(strlen(p_str)*(Font_Number_32.width + 1) ))/2+5;
	word.size = LCD_FONT_32_SIZE;
	word.kerning = 1;	
	LCD_SetNumber(p_str,&word);	
	
	word.x_axis = 90;
	word.y_axis =48;
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 0;
	LCD_SetString((char *)NotifyMenuStrs[SetValue.Language][12],&word);

	//��һȦ����
	memset(p_str,0,sizeof(p_str));
	sprintf(p_str,"%d:%02d",data.pace/60,data.pace%60);
	word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_NONE;
	word.x_axis = 110;
	word.y_axis =120 + (120 -(strlen(p_str)*(Font_Number_32.width + 1) ))/2;
	word.size = LCD_FONT_32_SIZE;
	word.kerning = 1;	
	LCD_SetNumber(p_str,&word);	
	
	word.x_axis = 90;
	word.y_axis =150;
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 0;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][39],&word);

	word.y_axis +=35;
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 0;
	LCD_SetString("/km",&word);

	//��һȦ����
	memset(p_str,0,sizeof(p_str));
	sprintf(p_str,"%d",data.heart);
	word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_NONE;
	word.x_axis = 175;
	word.y_axis =LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_32_SIZE;
	word.kerning = 1;	
	LCD_SetNumber(p_str,&word);	

	LCD_SetPicture(185,75 , LCD_RED, LCD_NONE, &Img_heartrate_18X16);
	

}

//�����Զ���Ȧ���ѽ���
void gui_notify_walk_atuocircle_paint(last_circle_t data)
{
	
		SetWord_t word = {0};
		char p_str[20];
		
		LCD_SetBackgroundColor(LCD_WHITE);
			
		LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 121, 5, 0, 360, LCD_LIGHTGREEN);
	
	
		LCD_SetRectangle(80, 2, 0, LCD_LINE_CNT_MAX, LCD_LIGHTGREEN, 0, 0, LCD_FILL_ENABLE);
	
		LCD_SetRectangle(158, 2, 0, LCD_LINE_CNT_MAX, LCD_LIGHTGREEN, 0, 0, LCD_FILL_ENABLE);
	
		LCD_SetRectangle(80, 80, 120, 2, LCD_LIGHTGREEN, 0, 0, LCD_FILL_ENABLE);
		//Ȧ��
	 word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_NONE;
		word.x_axis = 20;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_48_SIZE;
		word.kerning = 1;
		memset(p_str,0,sizeof(p_str));
		sprintf(p_str,"%d",data.lap);
		
		LCD_SetNumber(p_str,&word); 
	
		word.x_axis = 50;
		word.y_axis = 170;
		word.size = LCD_FONT_16_SIZE;
		word.kerning = 0;
		LCD_SetString("lap",&word);
	
		//��һȦ��ʱ
		memset(p_str,0,sizeof(p_str));
		sprintf(p_str,"%d:%02d",data.time/60,data.time%60);
		 word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_NONE;
		word.x_axis = 110;
		word.y_axis = (120 -(strlen(p_str)*(Font_Number_32.width + 1) ))/2+5;
		word.size = LCD_FONT_32_SIZE;
		word.kerning = 1;	
		LCD_SetNumber(p_str,&word); 
		
		word.x_axis = 90;
		word.y_axis =48;
		word.size = LCD_FONT_16_SIZE;
		word.kerning = 0;
		LCD_SetString((char *)NotifyMenuStrs[SetValue.Language][12],&word);
	
		//��һȦ��Ƶ
		memset(p_str,0,sizeof(p_str));
		sprintf(p_str,"%d",data.frequency);
		 word.forecolor = LCD_BLACK;
	     word.bckgrndcolor = LCD_NONE;
		word.x_axis = 110;
		word.y_axis =120 + (120 -(strlen(p_str)*(Font_Number_32.width + 1) ))/2;
		word.size = LCD_FONT_32_SIZE;
		word.kerning = 1;	
		LCD_SetNumber(p_str,&word); 
		
		word.x_axis = 90;
		word.y_axis =165;
		word.size = LCD_FONT_16_SIZE;
		word.kerning = 0;
		LCD_SetString((char *)SportDetailStrs[SetValue.Language][40],&word);
	

	
		//��һȦ����
		memset(p_str,0,sizeof(p_str));
		sprintf(p_str,"%d",data.heart);
		 word.forecolor = LCD_BLACK;
	    word.bckgrndcolor = LCD_NONE;
		word.x_axis = 175;
		word.y_axis =LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_32_SIZE;
		word.kerning = 1;	
		LCD_SetNumber(p_str,&word); 
	
		LCD_SetPicture(185,75 , LCD_RED, LCD_NONE, &Img_heartrate_18X16);


}

//��ɽ�Զ���Ȧ���ѽ���
void gui_notify_climb_atuocircle_paint(last_circle_t data)
{
	
		SetWord_t word = {0};
		char p_str[20];
		
		LCD_SetBackgroundColor(LCD_WHITE);
			
		LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 121, 5, 0, 360, LCD_LIGHTGREEN);
	
		LCD_SetRectangle(80, 2, 0, LCD_LINE_CNT_MAX, LCD_LIGHTGREEN, 0, 0, LCD_FILL_ENABLE);
	
		LCD_SetRectangle(158, 2, 0, LCD_LINE_CNT_MAX, LCD_LIGHTGREEN, 0, 0, LCD_FILL_ENABLE);
	
		LCD_SetRectangle(80, 160, 120, 2, LCD_LIGHTGREEN, 0, 0, LCD_FILL_ENABLE);
	
		//Ȧ��
	    word.forecolor = LCD_BLACK;
	    word.bckgrndcolor = LCD_NONE;
		word.x_axis = 20;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_48_SIZE;
		word.kerning = 1;
		memset(p_str,0,sizeof(p_str));
		sprintf(p_str,"%d",data.lap);
		
		LCD_SetNumber(p_str,&word); 
	
		word.x_axis = 50;
		word.y_axis = 170;
		word.size = LCD_FONT_16_SIZE;
		word.kerning = 0;
		LCD_SetString("lap",&word);
	
		//��һȦ��ʱ
		memset(p_str,0,sizeof(p_str));
		sprintf(p_str,"%d:%02d",data.time/60,data.time%60);
		  word.forecolor = LCD_BLACK;
	    word.bckgrndcolor = LCD_NONE;
		word.x_axis = 110;
		word.y_axis = (120 -(strlen(p_str)*(Font_Number_32.width + 1) ))/2+5;
		word.size = LCD_FONT_32_SIZE;
		word.kerning = 1;	
		LCD_SetNumber(p_str,&word); 
		
		word.x_axis = 90;
		word.y_axis =48;
		word.size = LCD_FONT_16_SIZE;
		word.kerning = 0;
		LCD_SetString((char *)NotifyMenuStrs[SetValue.Language][12],&word);
	
		//��һȦ����
		memset(p_str,0,sizeof(p_str));
		sprintf(p_str,"%d",data.heart);
		word.forecolor = LCD_BLACK;
	    word.bckgrndcolor = LCD_NONE;
		word.x_axis = 110;
		word.y_axis =120 + (120 -(strlen(p_str)*(Font_Number_32.width + 1) ))/2;
		word.size = LCD_FONT_32_SIZE;
		word.kerning = 1;	
		LCD_SetNumber(p_str,&word); 
	
		LCD_SetPicture(90,170 , LCD_RED, LCD_NONE, &Img_heartrate_18X16);

		//�����߶�
		LCD_SetPicture(161,70,LCD_NONE, LCD_NONE, &Img_RiseHeight_22X22);
		
		memset(p_str,0,sizeof(p_str));
		sprintf(p_str,"%d",data.riseheight);
		word.forecolor = LCD_BLACK;
	    word.bckgrndcolor = LCD_NONE;
		word.x_axis = 185;
		word.y_axis =(120 -(strlen(p_str)*(Font_Number_32.width + 1) ))/2+25;
		word.size = LCD_FONT_32_SIZE;
		word.kerning = 1;	
		LCD_SetNumber(p_str,&word); 
	
		//�½��߶�
		LCD_SetPicture(161,140, LCD_NONE, LCD_NONE, &Img_DropHeight_22X22);
		
		memset(p_str,0,sizeof(p_str));
		sprintf(p_str,"%d",data.dropheight);
		word.forecolor = LCD_BLACK;
	    word.bckgrndcolor = LCD_NONE;
		word.x_axis = 185;
		word.y_axis =120 + (120 -(strlen(p_str)*(Font_Number_32.width + 1) ))/2-25;
		word.size = LCD_FONT_32_SIZE;
		word.kerning = 1;	
		LCD_SetNumber(p_str,&word); 


}

//�����Զ���Ȧ���ѽ���
void gui_notify_cycling_atuocircle_paint(last_circle_t data)
{
	
		SetWord_t word = {0};
		char p_str[20];
		
		LCD_SetBackgroundColor(LCD_WHITE);
			
		LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 121, 5, 0, 360, LCD_LIGHTGREEN);
	
		LCD_SetRectangle(80, 2, 0, LCD_LINE_CNT_MAX, LCD_LIGHTGREEN, 0, 0, LCD_FILL_ENABLE);
	
		LCD_SetRectangle(158, 2, 0, LCD_LINE_CNT_MAX, LCD_LIGHTGREEN, 0, 0, LCD_FILL_ENABLE);
	
		LCD_SetRectangle(80, 80, 120, 2, LCD_LIGHTGREEN, 0, 0, LCD_FILL_ENABLE);
	
		//Ȧ��
		  word.forecolor = LCD_BLACK;
	    word.bckgrndcolor = LCD_NONE;
		word.x_axis = 20;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_48_SIZE;
		word.kerning = 1;
		memset(p_str,0,sizeof(p_str));
		sprintf(p_str,"%d",data.lap);
		
		LCD_SetNumber(p_str,&word); 
	
		word.x_axis = 50;
		word.y_axis = 170;
		word.size = LCD_FONT_16_SIZE;
		word.kerning = 0;
		LCD_SetString("lap",&word);
	
		//��һȦ��ʱ
		memset(p_str,0,sizeof(p_str));
		sprintf(p_str,"%d:%02d",data.time/60,data.time%60);
		word.forecolor = LCD_BLACK;
		word.bckgrndcolor = LCD_NONE;
		word.x_axis = 110;
		word.y_axis = (120 -(strlen(p_str)*(Font_Number_32.width + 1) ))/2+5;
		word.size = LCD_FONT_32_SIZE;
		word.kerning = 1;	
		LCD_SetNumber(p_str,&word); 
		
		word.x_axis = 90;
		word.y_axis =48;
		word.size = LCD_FONT_16_SIZE;
		word.kerning = 0;
		LCD_SetString((char *)NotifyMenuStrs[SetValue.Language][12],&word);
	
		//��һȦ�ٶ�
		memset(p_str,0,sizeof(p_str));
		#if defined (SPRINTF_FLOAT_TO_INT)
		sprintf(p_str,"%d.%01d",data.pace/10,data.pace%10);
		#else
		sprintf(p_str,"%0.1f",data.pace/10.f);
		#endif
		word.forecolor = LCD_BLACK;
		word.bckgrndcolor = LCD_NONE;
		word.x_axis = 110;
		word.y_axis =120 + (120 -(strlen(p_str)*(Font_Number_32.width + 1) ))/2;
		word.size = LCD_FONT_32_SIZE;
		word.kerning = 1;	
		LCD_SetNumber(p_str,&word); 
		
		word.x_axis = 90;
		word.y_axis =150;
		word.size = LCD_FONT_16_SIZE;
		word.kerning = 0;
		LCD_SetString((char *)NotifyMenuStrs[SetValue.Language][13],&word);
	
		word.y_axis +=35;
		word.size = LCD_FONT_16_SIZE;
		word.kerning = 0;
		LCD_SetString("km/h",&word);
	
		//��һȦ����
		memset(p_str,0,sizeof(p_str));
		sprintf(p_str,"%d",data.heart);
		word.forecolor = LCD_BLACK;
		word.bckgrndcolor = LCD_NONE;
		word.x_axis = 175;
		word.y_axis =LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_32_SIZE;
		word.kerning = 1;	
		LCD_SetNumber(p_str,&word); 
	
		LCD_SetPicture(185,75 , LCD_RED, LCD_NONE, &Img_heartrate_18X16);
		


}

//�Զ���Ȧ���ѽ���
void gui_notify_sport_atuocircle_paint(void)
{
	switch(ActivityData.Activity_Type)
	{
		case ACT_RUN:
		case ACT_MARATHON:
		case ACT_CROSSCOUNTRY:
		case ACT_HIKING:
			gui_notify_run_atuocircle_paint(last_circle_data);
		break;
		case ACT_WALK:
			gui_notify_walk_atuocircle_paint(last_circle_data);
		break;
		case ACT_CLIMB:
			gui_notify_climb_atuocircle_paint(last_circle_data);
		break;
		case ACT_CYCLING:
			gui_notify_cycling_atuocircle_paint(last_circle_data);
			
		break;
		default:
		break;			
	}
}


////
void gui_notify_sport_circle_btn_evt(uint32_t Key_Value)
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

void gui_notify_sport_time_btn_evt(uint32_t Key_Value)
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

void gui_notify_sport_distance_btn_evt(uint32_t Key_Value)
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

void gui_notify_sport_pace_btn_evt(uint32_t Key_Value)
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

void gui_notify_sport_height_btn_evt(uint32_t Key_Value)
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

void gui_notify_sport_speed_btn_evt(uint32_t Key_Value)
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

void gui_notify_sport_calory_btn_evt(uint32_t Key_Value)
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

void gui_notify_sport_autocircle_btn_evt(uint32_t Key_Value)
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

