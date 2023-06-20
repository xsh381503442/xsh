#include "gui_sport_config.h"
#include "gui_sport_indoorrun.h"

#include "drv_lcd.h"
#include "drv_key.h"

#include "task_sport.h"

#include "time_notify.h"
#include "timer_traceback.h"
#include "timer_app.h"
#include "time_progress.h"

#include "algo_hdr.h"
#include "algo_sport.h"

#include "com_data.h"
#include "com_sport.h"

#include "gui_sport.h"
#include "gui_run.h"
#include "gui_tool_gps.h"

#include "font_config.h"


#if DEBUG_ENABLED == 1 && GUI_SPORT_INDOORRUN_LOG_ENABLED == 1
	#define GUI_SPORT_INDOORRUN_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_SPORT_INDOORRUN_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define GUI_SPORT_INDOORRUN_WRITESTRING(...)
	#define GUI_SPORT_INDOORRUN_PRINTF(...)		        
#endif
#if defined WATCH_COM_SPORT

#define	GUI_SPORT_INDOORRUN_SCREEN_DATA			0	//���������ݽ���
#define	GUI_SPORT_INDOORRUN_SCREEN_HEART		1	//���������ʽ���
#define	GUI_SPORT_INDOORRUN_SCREEN_TIME			2	//������ʵʱʱ��
#define GUI_SPORT_INDOORRUN_SCREEN_MAX			3


#define	GUI_SPORT_INDOORRUN_PAUSE_CONTINUE	0	//����
#define	GUI_SPORT_INDOORRUN_PAUSE_SAVE		1	//�����˶�
#define	GUI_SPORT_INDOORRUN_PAUSE_WAIT		2	//�Ժ����
#define	GUI_SPORT_INDOORRUN_PAUSE_CANCEL	3	//����

#define	GUI_SPORT_INDOORRUN_PAUSE_CANCEL_NO	0	//��
#define	GUI_SPORT_INDOORRUN_PAUSE_CANCEL_YES	1	//��


#define	GUI_SPORT_INDOORRUN_DETAIL_1				0	//��ϸ���ݽ���1
#define	GUI_SPORT_INDOORRUN_DETAIL_2				1	//��ϸ���ݽ���2
#define	GUI_SPORT_INDOORRUN_DETAIL_HEARTRATEZONE	3	//�����������


static uint8_t m_gui_sport_indoorrun_index;		//�����ܽ�������
static uint8_t m_gui_sport_indoorrun_pause_index;	//��������ͣ�˵���������

/*�����ܳ�������*/
#define GUI_SPORT_SET_INDOORLENG_TENS                		0	//ʮλ
#define GUI_SPORT_SET_INDOORLENG_ONES						1	//��λ
static uint8_t m_gui_sport_set_indoorleng_index = 0;
_indoorleng m_indoorleng;
static uint8_t leng_set_flicker = 0;
extern volatile ScreenState_t Save_Detail_Index;


//��ok���˶���ʼ��ʾ
void gui_indoorrun_prompt_paint(void)
{
  SetWord_t word = {0};
 // uint8_t x1,y1,x2,y2,x3,y3;

   word.x_axis = 13;
   word.y_axis = 120;
   
   LCD_SetPicture(word.x_axis,word.y_axis,LCD_NONE,LCD_NONE,&Img_start_18X24);

   word.x_axis = 50;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	
	LCD_SetString((char *)SportsPromptStrs[SetValue.Language][0],&word);

	if  (leng_set_flicker==0)
	{
		leng_set_flicker=1;
		word.x_axis = 50;
		word.y_axis = 200;
		LCD_SetPicture(word.x_axis,word.y_axis,LCD_NONE,LCD_NONE,&Img_setting_16X16);
	}

	else 
	{
		leng_set_flicker=0;
		LCD_SetRectangle(50,16,200,16,LCD_BLACK,0,0,LCD_FILL_ENABLE);	
		//timer_progress_start(1000);	
	}
	

}


//���������ݽ���
void gui_sport_indoorrun_data_paint(void)
{

	SetWord_t word = {0};

	//���ñ�����ɫ
	LCD_SetBackgroundColor(LCD_BLACK);

	
	if(g_sport_status == SPORT_STATUS_READY)
	{
		gui_indoorrun_prompt_paint();
	}
	else
	{
		//Բ������
   		gui_sport_index_circle_paint(GUI_SPORT_INDOORRUN_SCREEN_DATA, GUI_SPORT_INDOORRUN_SCREEN_MAX, 1);

		gui_sport_data_display_total_distance_paint(SPORT_DATA_DISPALY_INDEX_UP_CIRCLE,ActivityData.ActivityDetails.RunningDetail.Distance);
		
	}

	
	//�ָ���
	word.x_axis = 80;
	LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,LCD_AUQAMARIN,0,0,LCD_FILL_ENABLE);

	
	//GPS״̬,��ֹ�ڵ������ÿ���λ��
	//gui_sport_gps_status(SPORT_GPS_LEFT1);

	gui_sport_data_display_realtime_pace_paint(SPORT_DATA_DISPALY_INDEX_MID_GPS,ActivityData.ActivityDetails.RunningDetail.Pace);

	//�ָ���
	word.x_axis = 160;	
	LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,LCD_AUQAMARIN,0,0,LCD_FILL_ENABLE);

	gui_sport_data_display_total_time_paint(SPORT_DATA_DISPALY_INDEX_DOWN_NOHINT,ActivityData.ActTime);


}




//���������ʽ���
void gui_sport_indoorrun_heart_paint(void)
{
	SetWord_t word = {0};
	char str[10];
	uint8_t heart = Get_Sport_Heart();
	
	//���ñ�����ɫ
	LCD_SetBackgroundColor(LCD_BLACK);
	//Բ������
    gui_sport_index_circle_paint(GUI_SPORT_INDOORRUN_SCREEN_HEART, GUI_SPORT_INDOORRUN_SCREEN_MAX, 1);

	//gui_sport_gps_status(SPORT_GPS_LEFT3);
	
	//����ͼ
	word.x_axis = 56;
	word.y_axis = 170;
	LCD_SetPicture(word.x_axis, word.y_axis, LCD_RED, LCD_NONE, &Img_heartrate_18X16);
	
	//����ֵ
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = get_hdr_range_color(heart);
	word.kerning = 1;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.x_axis = 54;
	word.size = LCD_FONT_48_SIZE;

	if(heart == 0)
	{	
		LCD_SetRectangle(80,3,93,26,LCD_WHITE,0,0,LCD_FILL_ENABLE);

		LCD_SetRectangle(80,3,121,26,LCD_WHITE,0,0,LCD_FILL_ENABLE);
	}
	else
	{
		memset(str,0,10);
		sprintf(str,"%d",heart);
		LCD_SetNumber(str,&word);
		
		word.y_axis = 170;
		word.x_axis = 93;
		word.size = LCD_FONT_16_SIZE;
		word.forecolor = LCD_WHITE;
		word.kerning = 0;
		LCD_SetString("bpm",&word);
	}
	
	//�ָ���
	word.x_axis = 120;
	LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,LCD_AUQAMARIN,0,0,LCD_FILL_ENABLE);
	
	//��·��
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 1;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.x_axis = 130;
	word.size = LCD_FONT_48_SIZE;

	memset(str,0,10);
	sprintf(str,"%d",ActivityData.ActivityDetails.RunningDetail.Calorie/1000);
	LCD_SetNumber(str,&word);

	word.x_axis =195;
	word.y_axis =LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
	word.kerning = 0;
	LCD_SetString("kcal",&word);
	
	
	

	
}






//������ʵʱʱ�����
void gui_sport_indoorrun_realtime_paint(void)
{
	//���ñ�����ɫ
	LCD_SetBackgroundColor(LCD_BLACK);
	
	//Բ������
    gui_sport_index_circle_paint(GUI_SPORT_INDOORRUN_SCREEN_TIME, GUI_SPORT_INDOORRUN_SCREEN_MAX, 1);
	
	//ʵʱʱ��
	//gui_sport_realtime_1_paint(0);
		
}


//�����ܽ���
void gui_sport_indoorrun_paint(void)
{
	if(m_gui_sport_indoorrun_index ==GUI_SPORT_INDOORRUN_SCREEN_DATA )
	{
	 	gui_sport_indoorrun_data_paint();
	}
	else if (m_gui_sport_indoorrun_index ==GUI_SPORT_INDOORRUN_SCREEN_HEART)
	{
		gui_sport_indoorrun_heart_paint();
	}
	else
	{
		gui_sport_indoorrun_realtime_paint();
	}
	
	if(g_sport_status == SPORT_STATUS_PAUSE)
	{
		//��ʱ���Ӻ�ɫԲȦ����ʾ��ͣ״̬
		LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_RED);
	}

}


//��ͣ�˵�ѡ��
static void gui_sport_indoorrun_pause_options(uint8_t index)
{
	SetWord_t word = {0};
	
	//�˵���һ��
	word.x_axis = 133;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 0;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString((char *)IndoorrunPauseStrs[SetValue.Language][index],&word); //
	
	//�˵��ڶ���
	if(index >= GUI_SPORT_INDOORRUN_PAUSE_CANCEL)
	{
		index = GUI_SPORT_INDOORRUN_PAUSE_CONTINUE;
	}
	else
	{

		index += 1;

	}
	word.x_axis = 183;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString((char *)IndoorrunPauseStrs[SetValue.Language][index],&word);
}


//��������ͣ�˵�����
void gui_sport_indoorrun_pause_paint(void)
{
	//�ϰ벿��Ϊ��ɫ����
	LCD_SetRectangle(0, 120, 0, LCD_PIXEL_ONELINE_MAX, LCD_WHITE, 0, 0, LCD_FILL_ENABLE);
	
	//��ͣ��ʱ
	gui_sport_pause_time();
	
	//�˶���ʱ
	gui_sport_pause_acttime();
	
	//�°벿��Ϊ��ɫ����
	LCD_SetRectangle(120, 120, 0, LCD_PIXEL_ONELINE_MAX, LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
	
	//ѡ��
	gui_sport_indoorrun_pause_options(m_gui_sport_indoorrun_pause_index);
	
	//�ָ���
	LCD_SetRectangle(170, 1, 0, LCD_LINE_CNT_MAX, LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);
	
	//�·���ʾ
	gui_sport_page_prompt(SPORT_PAGE_DOWN);
	
	//��ɫԲȦ
	LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_RED);
}

//�����ܷ����˵�����
void gui_sport_indoorrun_cancel_paint(void)
{
	SetWord_t word = {0};
	
	//�ϰ벿��Ϊ��ɫ����
	LCD_SetRectangle(0, 120, 0, LCD_PIXEL_ONELINE_MAX, LCD_WHITE, 0, 0, LCD_FILL_ENABLE);
	
	//����?
	word.x_axis = 48;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_BLACK;
	word.kerning = 0;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString((char *)RunPauseStrs[SetValue.Language][3],&word);
    word.y_axis = 104 + 32 + 2;
    LCD_SetString("?",&word);
	
	//�°벿��Ϊ��ɫ����
	LCD_SetRectangle(120, 120, 0, LCD_PIXEL_ONELINE_MAX, LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
	
	//ѡ��
	gui_sport_cancel_options(m_gui_sport_indoorrun_pause_index);
	
	//�ָ���
	LCD_SetRectangle(170, 1, 0, LCD_LINE_CNT_MAX, LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);
	
	//�·���ʾ
	gui_sport_page_prompt(SPORT_PAGE_DOWN);
	
	//��ɫԲȦ
	LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_RED);
}
//��ϸ���ݽ���1
static void gui_sport_indoorrun_detail_1_paint(uint8_t backgroundcolor)
{
		SetWord_t word = {0};
		char str[20] = {0};
		uint32_t second;
		uint8_t charlen;
		
		//����ʱ�� = ��ʱ�� - ��ͣʱ��
		second = ActivityData.ActTime; //get_time_diff(ActivityData.Act_Start_Time, ActivityData.Act_Stop_Time) - Get_PauseTotolTime();
		
		//���ñ�����ɫ
		LCD_SetBackgroundColor(backgroundcolor);
		
		if (backgroundcolor == LCD_BLACK)
		{
			//�ڵװ���
			word.forecolor = LCD_WHITE;
		}
		else
		{
			//�׵׺���
			word.forecolor = LCD_BLACK;
		}
	
		//ʱ��
		word.x_axis = 34;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_20_SIZE;

		word.bckgrndcolor = LCD_NONE;
		word.kerning = 0;
		memset(str,0,sizeof(str));
		sprintf(str,"%02d-%02d-%02d %02d:%02d",ActivityData.Act_Start_Time.Year,ActivityData.Act_Start_Time.Month,
												ActivityData.Act_Start_Time.Day,ActivityData.Act_Start_Time.Hour,
												ActivityData.Act_Start_Time.Minute);
		LCD_SetNumber(str,&word);
		
		//�ָ���
		word.x_axis = 70;
		LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,LCD_AUQAMARIN,0,0,LCD_FILL_ENABLE);
		
		//���
		charlen = strlen(SportDetailStrs[SetValue.Language][0]);
		word.x_axis = 90;
		word.y_axis = 95 - charlen*8;
		word.kerning = 0;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString((char *)SportDetailStrs[SetValue.Language][0],&word);
		word.y_axis = 98;
		LCD_SetString(":",&word);
		
		word.y_axis = 110;
		word.kerning = 1;
		memset(str,0,sizeof(str));
		#if defined (SPRINTF_FLOAT_TO_INT)
		sprintf(str,"%d.%02d",ActivityData.ActivityDetails.RunningDetail.Distance/100000,ActivityData.ActivityDetails.RunningDetail.Distance/1000%100);
		#else
		sprintf(str,"%0.2f",ActivityData.ActivityDetails.RunningDetail.Distance/100000.f);	//����װ��Ϊǧ�ף�������2λС��
		#endif
		LCD_SetNumber(str,&word);
		
		word.y_axis = SPORT_DETAIL_DATA_UNITS_Y_AXIS;
		word.kerning = 0;
		LCD_SetString("km",&word);
		
		//ʱ��
		charlen = strlen(SportDetailStrs[SetValue.Language][1]);
		word.x_axis = 120;
		word.y_axis = 95 - charlen*8;
		word.kerning = 0;
		LCD_SetString((char *)SportDetailStrs[SetValue.Language][1],&word);
		word.y_axis = 98;
		LCD_SetString(":",&word);
		
		word.y_axis = 110;
		word.kerning = 1;
		memset(str,0,sizeof(str));
		if(second >= 3600)
		{
			//ʱ����
			sprintf(str,"%d:%02d:%02d",second/3600,second/60%60,second%60);
		}
		else
		{
			//����
			sprintf(str,"%d:%02d",second/60%60,second%60);
		}
		LCD_SetNumber(str,&word);
		
		//ƽ������
		charlen = strlen(SportDetailStrs[SetValue.Language][2]);
		word.x_axis = 150;
		word.y_axis = 95 - charlen*8;
		word.kerning = 0;
		LCD_SetString((char *)SportDetailStrs[SetValue.Language][2],&word);
		word.y_axis = 98;
		LCD_SetString(":",&word);
		
		word.y_axis = 110;
		word.kerning = 1;
		memset(str,0,sizeof(str));
		sprintf(str,"%d:%02d",(ActivityData.ActivityDetails.RunningDetail.Pace/60),
								(ActivityData.ActivityDetails.RunningDetail.Pace%60));
		LCD_SetNumber(str,&word);
		
		word.y_axis = SPORT_DETAIL_DATA_UNITS_Y_AXIS;
		word.kerning = 0;
		LCD_SetString("/km",&word);
		
		//ƽ����Ƶ
		charlen = strlen(SportDetailStrs[SetValue.Language][24]);
		word.x_axis = 180;
		word.y_axis = 95 - charlen*8;
		word.kerning = 0;
		LCD_SetString((char *)SportDetailStrs[SetValue.Language][24],&word);
		word.y_axis = 98;
		LCD_SetString(":",&word);
		
		word.y_axis = 110;
		word.kerning = 1;
		memset(str,0,sizeof(str));
		if(second != 0)
		{
			sprintf(str,"%d",
			   ActivityData.ActivityDetails.RunningDetail.Steps*60/(ActivityData.ActTime));//��/min
		}
		else
		{
			sprintf(str,"%d",0);
		}
		LCD_SetNumber(str,&word);
		
		word.y_axis = SPORT_DETAIL_DATA_UNITS_Y_AXIS;
		word.kerning = 0;
		LCD_SetString("spm",&word);
		
		//�·���ʾ
		gui_sport_page_prompt(SPORT_PAGE_DOWN);
}

//��ϸ���ݽ���2
static void gui_sport_indoorrun_detail_2_paint(uint8_t backgroundcolor)
{
	SetWord_t word = {0};
	char str[20] = {0};
	uint8_t charlen;
	
	//���ñ�����ɫ
    LCD_SetBackgroundColor(backgroundcolor);
	
	if (backgroundcolor == LCD_BLACK)
    {
        //�ڵװ���
        word.forecolor = LCD_WHITE;
    }
    else
    {
        //�׵׺���
        word.forecolor = LCD_BLACK;
    }
	
	//�Ϸ���ʾ
	gui_sport_page_prompt(SPORT_PAGE_UP);

	//��·��
	charlen = strlen(SportDetailStrs[SetValue.Language][4]);
	word.x_axis = 30;
    word.y_axis = 95 - charlen*8;
	word.kerning = 0;
	word.bckgrndcolor = LCD_NONE;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][4],&word);
	word.y_axis = 98;
    LCD_SetString(":",&word);
	
	word.y_axis = 110;
    word.kerning = 1;
    memset(str,0,sizeof(str));
    sprintf(str,"%d",ActivityData.ActivityDetails.RunningDetail.Calorie/1000);	//����װ��Ϊǧ����������һλС����
    LCD_SetNumber(str,&word);
	
    word.y_axis = SPORT_DETAIL_DATA_UNITS_Y_AXIS;
    word.kerning = 0;
    LCD_SetString("kcal",&word);
	
    //ƽ������
	charlen = strlen(SportDetailStrs[SetValue.Language][5]);
	word.x_axis = 55;
    word.y_axis = 95 - charlen*8;
	word.kerning = 0;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][5],&word);
	word.y_axis = 98;
    LCD_SetString(":",&word);
	
	word.y_axis = 110;
    word.kerning = 1;
    memset(str,0,sizeof(str));
    sprintf(str,"%d",ActivityData.ActivityDetails.RunningDetail.AvgHeart);
    LCD_SetNumber(str,&word);
	
    word.y_axis = SPORT_DETAIL_DATA_UNITS_Y_AXIS;
    word.kerning = 0;
    LCD_SetString("bpm",&word);
	
	//�������
	charlen = strlen(SportDetailStrs[SetValue.Language][6]);
	word.x_axis = 80;
    word.y_axis = 95 - charlen*8;
	word.kerning = 0;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][6],&word);
	word.y_axis = 98;
    LCD_SetString(":",&word);
	
	word.y_axis = 110;
    word.kerning = 1;
    memset(str,0,sizeof(str));
    sprintf(str,"%d",ActivityData.ActivityDetails.RunningDetail.MaxHeart);
    LCD_SetNumber(str,&word);
	
    word.y_axis = SPORT_DETAIL_DATA_UNITS_Y_AXIS;
    word.kerning = 0;
    LCD_SetString("bpm",&word);

	//���ʻָ���
	charlen = strlen(SportDetailStrs[SetValue.Language][42]);
	word.x_axis = 105;
	word.y_axis = 95 - charlen*8;
	word.kerning = 0;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][42],&word);
	word.y_axis = 98;
	LCD_SetString(":",&word);
	if (ActivityData.ActivityDetails.CommonDetail.HRRecovery == 250)
	{
		word.y_axis = 110;
		word.kerning = 1;
		LCD_SetString("--",&word);
	}
	else
	{
		memset(str,0,20);
		#if defined (SPRINTF_FLOAT_TO_INT)
		sprintf(str,"%d.%01d",ActivityData.ActivityDetails.CommonDetail.HRRecovery/10,ActivityData.ActivityDetails.CommonDetail.HRRecovery%10);
		#else
		sprintf(str,"%0.1f",ActivityData.ActivityDetails.CommonDetail.HRRecovery/10.f);
		#endif
		word.y_axis = 110;
		word.kerning = 1;
		LCD_SetNumber(str,&word);
	}
	
	
	//����Ч��
	charlen = strlen(SportDetailStrs[SetValue.Language][7]);
	word.x_axis += 25;
    word.y_axis = 95 - charlen*8;
	word.kerning = 0;
	word.bckgrndcolor = LCD_NONE;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][7],&word);
	word.y_axis = 98;
    LCD_SetString(":",&word);
	
	word.y_axis = 110;
    word.kerning = 1;
    memset(str,0,sizeof(str));
	#if defined (SPRINTF_FLOAT_TO_INT)
	sprintf(str,"%d.%01d",ActivityData.ActivityDetails.RunningDetail.AerobicEffect/10,ActivityData.ActivityDetails.RunningDetail.AerobicEffect%10);
	#else
    sprintf(str,"%.1f",ActivityData.ActivityDetails.RunningDetail.AerobicEffect/10.f);		//����һλС����
    #endif
    LCD_SetNumber(str,&word);
	
    //����Ч��
	charlen = strlen(SportDetailStrs[SetValue.Language][8]);
	word.x_axis += 25;
    word.y_axis = 95 - charlen*8;
	word.kerning = 0;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][8],&word);
	word.y_axis = 98;
    LCD_SetString(":",&word);
	
	word.y_axis = 110;
    word.kerning = 1;
    memset(str,0,sizeof(str));
	#if defined (SPRINTF_FLOAT_TO_INT)
	sprintf(str,"%d.%01d",ActivityData.ActivityDetails.RunningDetail.AnaerobicEffect/10,ActivityData.ActivityDetails.RunningDetail.AnaerobicEffect%10);
	#else
    sprintf(str,"%.1f",ActivityData.ActivityDetails.RunningDetail.AnaerobicEffect/10.f);	//����һλС����
    #endif
    LCD_SetNumber(str,&word);
	
	//�ָ�ʱ��
	charlen = strlen(SportDetailStrs[SetValue.Language][9]);
	word.x_axis += 25;
	word.y_axis = 95 - charlen*8;
	word.kerning = 0;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][9],&word);
	word.y_axis = 98;
	LCD_SetString(":",&word);
	
	word.y_axis = 110;
	word.kerning = 1;
	memset(str,0,sizeof(str));
	#if defined (SPRINTF_FLOAT_TO_INT)
	sprintf(str,"%d.%01d",ActivityData.ActivityDetails.RunningDetail.RecoveryTime/60,ActivityData.ActivityDetails.RunningDetail.RecoveryTime%60);
	#else
	sprintf(str,"%.1f",ActivityData.ActivityDetails.RunningDetail.RecoveryTime/60.f);
	#endif
	LCD_SetNumber(str,&word);
	
	word.y_axis = SPORT_DETAIL_DATA_UNITS_Y_AXIS;
	word.kerning = 0;
	LCD_SetString("h",&word);
	
	//�·���ʾ
	gui_sport_page_prompt(SPORT_PAGE_DOWN);
}	

//��ϸ���ݽ���
void gui_sport_indoorrun_detail_paint(void)
{
	switch(m_gui_sport_indoorrun_pause_index)
	{
		case GUI_SPORT_INDOORRUN_DETAIL_1:
			gui_sport_indoorrun_detail_1_paint(LCD_BLACK);
			break;
		case GUI_SPORT_INDOORRUN_DETAIL_2:
			gui_sport_indoorrun_detail_2_paint(LCD_BLACK);
			break;	
		case GUI_SPORT_INDOORRUN_DETAIL_HEARTRATEZONE:
			gui_sport_detail_heartratezone_paint(LCD_BLACK);
			break;
		default:
			break;
	}
}
//�����ܲ�������
void gui_sport_set_indoorleng_paint(void)
{
	SetWord_t word = {0};
	char str[10];
	uint16_t tmp_value;
	
	//���ñ�����ɫ
	LCD_SetBackgroundColor(LCD_WHITE);
	//�ϰ벿��ɫ
	LCD_SetRectangle(0,80,0,LCD_LINE_CNT_MAX,LCD_BLACK,0,0,LCD_FILL_ENABLE);
		
	word.x_axis = 36;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;
	
	LCD_SetString((char *)SportRemindStrs[SetValue.Language][9],&word);
	
	
	word.x_axis = 100;
	word.forecolor = LCD_BLACK; 

	tmp_value = 10*m_indoorleng.tens + m_indoorleng.ones;
	
	word.x_axis = 110;
	word.size = LCD_FONT_88_SIZE;
	memset(str,0,10);	
	sprintf(str,"%d",tmp_value);
	LCD_SetNumber(str,&word);
	
	word.x_axis = 180;
	word.y_axis = 185;
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 1;
	LCD_SetString("cm",&word);

	switch(m_gui_sport_set_indoorleng_index)
	{

		case GUI_SPORT_SET_INDOORLENG_TENS:{
			if(m_indoorleng.tens >=10)
			{
				LCD_SetRectangle(200,6,61,76,LCD_BLUE,0,0,LCD_FILL_ENABLE);	
			}
			else
			{
				LCD_SetRectangle(200,6,81,38,LCD_BLUE,0,0,LCD_FILL_ENABLE);	
			}
		
		}break; 
		case GUI_SPORT_SET_INDOORLENG_ONES:{
			if(m_indoorleng.tens >=10)
			{
				LCD_SetRectangle(200,6,141,38,LCD_BLUE,0,0,LCD_FILL_ENABLE);	
			}
			else
			{	
			LCD_SetRectangle(200,6,121,38,LCD_BLUE,0,0,LCD_FILL_ENABLE);
			}
		}break;
		default:
			break;
	}	

}




void gui_sport_indoorrun_init(void)
{
	//Ĭ��Ϊ�ܲ����ݽ���
	m_gui_sport_indoorrun_index = GUI_SPORT_INDOORRUN_SCREEN_DATA;
	leng_set_flicker=0;
	timer_progress_start(1000);
}


void gui_sport_indoorrun_btn_evt(uint32_t Key_Value)
{
	switch( Key_Value )
	{
		case KEY_OK:
			if(g_sport_status == SPORT_STATUS_READY)
			{
				//��ʼ��ʱ
				g_sport_status = SPORT_STATUS_START;
				timer_progress_stop();
				gui_swich_msg();
				
				//����ʾ
				gui_sport_motor_prompt();
				
				//�����˶�����
				StartSport();
			}
			 else if(g_sport_status == SPORT_STATUS_START)
			{
				//��ͣ�˶���������ͣ�˵�����
				g_sport_status = SPORT_STATUS_PAUSE;
				Set_PauseSta(true);
				
				ScreenState = DISPLAY_SCREEN_INDOORRUN_PAUSE;
				m_gui_sport_indoorrun_pause_index = GUI_SPORT_INDOORRUN_PAUSE_CONTINUE;
				//gui_swich_msg();
				Save_Detail_Index = ScreenState;
				ScreenState = DISPLAY_SCREEN_REMIND_PAUSE;
				timer_app_pasue_start();
				//����ʾ
				gui_sport_motor_prompt();
			}
			else if(g_sport_status == SPORT_STATUS_PAUSE)
			{
				//������ʱ
				g_sport_status = SPORT_STATUS_START;
				Set_PauseSta(false);
				
				gui_swich_msg();
				Save_Detail_Index = ScreenState;
				timer_app_pasue_start();
				//����ʾ
				gui_sport_motor_prompt();
			}
			break;
		case KEY_DOWN:
			if (g_sport_status != SPORT_STATUS_READY)
			{
             //��ҳ�鿴�ܲ�����
				if(m_gui_sport_indoorrun_index < (GUI_SPORT_INDOORRUN_SCREEN_MAX -1))
				{
					m_gui_sport_indoorrun_index++;
				}
				else
				{
					m_gui_sport_indoorrun_index = 0;
				}
				
				gui_swich_msg();
			}
			break;
		case KEY_UP:
			if (g_sport_status != SPORT_STATUS_READY)
			{
           		//��ҳ�鿴�ܲ�����
				if(m_gui_sport_indoorrun_index > 0)
				{
					m_gui_sport_indoorrun_index--;
				}
				else
				{
					m_gui_sport_indoorrun_index = (GUI_SPORT_INDOORRUN_SCREEN_MAX -1);
				}
				
				gui_swich_msg();
			}
			else if (g_sport_status == SPORT_STATUS_READY)
			{
				m_indoorleng.tens = SetValue.AppGet.StepLengthIndoorRun/10;
				m_indoorleng.ones = SetValue.AppGet.StepLengthIndoorRun%10;

				
				m_gui_sport_set_indoorleng_index = 0;
			
				ScreenState = DISPLAY_SCREEN_SPORT_REMIND_SET_INDOORLENG;
				gui_swich_msg();

			}
		    break;
		case KEY_BACK:
 			if(g_sport_status == SPORT_STATUS_PAUSE)
			{
				//������ͣ�˵�����
				ScreenState = DISPLAY_SCREEN_INDOORRUN_PAUSE;
				m_gui_sport_indoorrun_index = GUI_SPORT_INDOORRUN_PAUSE_CONTINUE;
			
				gui_swich_msg();
			
			}
			else if(g_sport_status == SPORT_STATUS_READY)
			{
			
			  timer_progress_stop();	
			  if(0 == ENTER_NAVIGATION && 0 == ENTER_MOTION_TRAIL)    //�ɵ�����������˶�ready������ж�
				{


					
					//�����˶�ѡ��˵�����
					ScreenState = DISPLAY_SCREEN_SPORT;
					
				
					gui_swich_msg();
					
					//�ر�GPS����
					ReadySportBack();
				}
				else
				{
					if(1 == ENTER_NAVIGATION)
					{
						ScreenState = DISPLAY_SCREEN_NAVIGATION_OPS;
					}
					else
					{
						ScreenState = DISPLAY_SCREEN_MOTION_TRAIL_DETAIL;
					}
					
					ENTER_NAVIGATION = 0;
					ENTER_MOTION_TRAIL = 0;
					
					gui_swich_msg();
				}
			}
			break;
		default:
			break;
	}
}

//��������ͣ�˵����水���¼�
void gui_sport_indoorrun_pause_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
			if(m_gui_sport_indoorrun_pause_index == GUI_SPORT_INDOORRUN_PAUSE_CONTINUE)
			{
				//������ʱ����ʼ�˶�
				g_sport_status = SPORT_STATUS_START;
				Set_PauseSta(false);
				
				ScreenState = DISPLAY_SCREEN_INDOORRUN;
				gui_swich_msg();
				Save_Detail_Index = ScreenState;
				timer_app_pasue_start();
				//����ʾ
				gui_sport_motor_prompt();
			}
			else if(m_gui_sport_indoorrun_pause_index == GUI_SPORT_INDOORRUN_PAUSE_SAVE)
			{
				//�����˶�����������
				g_sport_status = SPORT_STATUS_READY;
				
				//Ĭ��ѡ��
				gui_sport_feel_init();
				
				ScreenState = DISPLAY_SCREEN_SPORT_FEEL;
				m_gui_sport_indoorrun_pause_index = GUI_SPORT_INDOORRUN_DETAIL_1;
				sport_end_flag = 1;
				sport_end_hrvalue = get_10s_heartrate(); //��ȡǰ10sƽ������
				g_HeartNum = 0;
				extern volatile ScreenState_t Save_Detail_Index;
				Save_Detail_Index = DISPLAY_SCREEN_INDOORRUN_DETAIL;
				gui_swich_msg();
			}
			else if(m_gui_sport_indoorrun_pause_index == GUI_SPORT_INDOORRUN_PAUSE_WAIT)
			{
				m_gui_sport_indoorrun_pause_index = GUI_SPORT_INDOORRUN_PAUSE_CONTINUE;
				
				//�Ժ����
				sport_continue_later_start();
				
				//�Ժ���������˶�����
				ContinueLaterDataSave.SportType = DISPLAY_SCREEN_INDOORRUN;
				
				//�Ժ�������˳��˶����棬����������
				ScreenState = DISPLAY_SCREEN_HOME;
				gui_swich_msg();
			}		
			else
			{
				//�����˶�
				ScreenState = DISPLAY_SCREEN_INDOORRUN_CANCEL;
				
				m_gui_sport_indoorrun_pause_index = GUI_SPORT_INDOORRUN_PAUSE_CANCEL_NO;
				gui_swich_msg();
			}
			
			break;
		case KEY_UP:
			if(m_gui_sport_indoorrun_pause_index > GUI_SPORT_INDOORRUN_PAUSE_CONTINUE)
			{

				m_gui_sport_indoorrun_pause_index--;

			}
			else
			{
				m_gui_sport_indoorrun_pause_index = GUI_SPORT_INDOORRUN_PAUSE_CANCEL;
			}
			
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		    break;
		case KEY_DOWN:
			if(m_gui_sport_indoorrun_pause_index < GUI_SPORT_INDOORRUN_PAUSE_CANCEL)
			{
		
				m_gui_sport_indoorrun_pause_index++;

			}
			else
			{
				m_gui_sport_indoorrun_pause_index = GUI_SPORT_INDOORRUN_PAUSE_CONTINUE;
			}
			
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		case KEY_BACK:
			//�������н���
			ScreenState = DISPLAY_SCREEN_INDOORRUN;
		
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}

//�����˵����水���¼�
void gui_sport_indoorrun_cancel_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
			if(m_gui_sport_indoorrun_pause_index == GUI_SPORT_INDOORRUN_PAUSE_CANCEL_NO)
			{
				//ȡ�������˶��������ϼ��˵�����
				ScreenState = DISPLAY_SCREEN_INDOORRUN_PAUSE;
				m_gui_sport_indoorrun_pause_index = GUI_SPORT_INDOORRUN_PAUSE_CANCEL;
			}
			else
			{
				//ȷ��ȡ���˶�������������
				g_sport_status = SPORT_STATUS_READY;
				
				ScreenState = DISPLAY_SCREEN_HOME;
				
				//�����˶�������
				Set_SaveSta(0);
				CloseSportTask();
				
				//����ʾ
				gui_sport_motor_prompt();
			}
			
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		case KEY_UP:
		case KEY_DOWN:
			if(m_gui_sport_indoorrun_pause_index == GUI_SPORT_INDOORRUN_PAUSE_CANCEL_NO)
			{
				m_gui_sport_indoorrun_pause_index = GUI_SPORT_INDOORRUN_PAUSE_CANCEL_YES;
			}
			else
			{
				m_gui_sport_indoorrun_pause_index = GUI_SPORT_INDOORRUN_PAUSE_CANCEL_NO;
			}

			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		    break;
		case KEY_BACK:
			//�����ϼ��˵�
			ScreenState = DISPLAY_SCREEN_INDOORRUN_PAUSE;
			m_gui_sport_indoorrun_pause_index = GUI_SPORT_INDOORRUN_PAUSE_CANCEL;
			
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}

//��ϸ���ݽ��水���¼�
void gui_sport_indoorrun_detail_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_UP:
			//�Ϸ�
			if(m_gui_sport_indoorrun_pause_index > GUI_SPORT_INDOORRUN_DETAIL_1)
			{
				m_gui_sport_indoorrun_pause_index--;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
		    break;
		case KEY_DOWN:
			//�·�
			if(m_gui_sport_indoorrun_pause_index < GUI_SPORT_INDOORRUN_DETAIL_HEARTRATEZONE)
			{
				m_gui_sport_indoorrun_pause_index++;

				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
			break;
		case KEY_BACK:
			//����������
			ScreenState = DISPLAY_SCREEN_HOME;
			
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}
void gui_sport_set_indoorleng_btn_evt(uint32_t Key_Value)
{
	switch( Key_Value )
	{
		case KEY_OK:
			switch(m_gui_sport_set_indoorleng_index)
			{
			
				case GUI_SPORT_SET_INDOORLENG_TENS:{
					m_gui_sport_set_indoorleng_index = GUI_SPORT_SET_INDOORLENG_ONES;						
				}break;		
				case GUI_SPORT_SET_INDOORLENG_ONES:{
						m_gui_sport_set_indoorleng_index = GUI_SPORT_SET_INDOORLENG_TENS;

						SetValue.AppGet.StepLengthIndoorRun = m_indoorleng.tens*10 + m_indoorleng.ones;
						SetValue.IndoorLengSet = 1;
						
						ScreenState = DISPLAY_SCREEN_INDOORRUN;
				
				}break;		
			}
			gui_swich_msg();
			break;
		case KEY_DOWN:
			switch(m_gui_sport_set_indoorleng_index)
			{
				case GUI_SPORT_SET_INDOORLENG_TENS:{
					if(m_indoorleng.tens <= 1)
					{
						m_indoorleng.tens = 24;
					}else{
						m_indoorleng.tens--;
					}						
				}break;		
				case GUI_SPORT_SET_INDOORLENG_ONES:{
					if(m_indoorleng.ones <= 0)
					{
						m_indoorleng.ones = 9;
					}else{
						m_indoorleng.ones--;
					}		
		
				}break;		
			}
			gui_swich_msg();
			break;
		case KEY_UP:
			switch(m_gui_sport_set_indoorleng_index)
			{			
				case GUI_SPORT_SET_INDOORLENG_TENS:{
					if(m_indoorleng.tens >= 24)
					{
						m_indoorleng.tens = 1;
					}else{
						m_indoorleng.tens++;
					}						
				}break;		
				case GUI_SPORT_SET_INDOORLENG_ONES:{
					if(m_indoorleng.ones >= 9)
					{
						m_indoorleng.ones = 0;
					}else{
						m_indoorleng.ones++;
					}		
		
				}break;		
			}
			gui_swich_msg();
            break; 
		case KEY_BACK:
			switch(m_gui_sport_set_indoorleng_index)
			{
				case GUI_SPORT_SET_INDOORLENG_TENS:{
					ScreenState = DISPLAY_SCREEN_INDOORRUN;						
				}break;		
				case GUI_SPORT_SET_INDOORLENG_ONES:{
					m_gui_sport_set_indoorleng_index = GUI_SPORT_SET_INDOORLENG_TENS;	
				}break;		
			}
			gui_swich_msg();
			break;
		default:
			break;
		}

}


#else

#if defined WATCH_SIM_SPORT
	#define	GUI_SPORT_INDOORRUN_SCREEN_1		0	//�����ܵ�һ������
	#define	GUI_SPORT_INDOORRUN_SCREEN_2		1	//�����ܵڶ�������
	#define	GUI_SPORT_INDOORRUN_SCREEN_3		2	//�����ܵ���������
	
	#define	GUI_SPORT_INDOORRUN_PAUSE_CONTINUE		0	//����
	#define	GUI_SPORT_INDOORRUN_PAUSE_SAVE			1	//�����˶�
	#define	GUI_SPORT_INDOORRUN_PAUSE_WAIT			2	//�Ժ����
	#define	GUI_SPORT_INDOORRUN_PAUSE_CANCEL		3	//����
#else
	#define	GUI_SPORT_INDOORRUN_PAUSE_CONTINUE		0	//����
	#define	GUI_SPORT_INDOORRUN_PAUSE_SAVE			1	//�����˶�
	#define	GUI_SPORT_INDOORRUN_PAUSE_CANCEL		2	//����
#endif

#define	GUI_SPORT_INDOORRUN_PAUSE_CANCEL_NO		0	//��
#define	GUI_SPORT_INDOORRUN_PAUSE_CANCEL_YES	1	//��

#define	GUI_SPORT_INDOORRUN_DETAIL_1				0	//��ϸ���ݽ���1
#define	GUI_SPORT_INDOORRUN_DETAIL_2				1	//��ϸ���ݽ���2
#define	GUI_SPORT_INDOORRUN_DETAIL_HEARTRATEZONE	2	//�����������

//#if defined WATCH_SIM_SPORT
extern volatile ScreenState_t Save_Detail_Index;

	/*������������ʾ��ǰ��*/
	uint8_t m_indoorrun_data_display_buf[SPORT_DATA_DISPLAY_SUM]={SPORT_DATA_DISPALY_TOTAL_DISTANCE,SPORT_DATA_DISPALY_REALTIME_PACE,SPORT_DATA_DISPALY_TOTAL_TIME};
	/*������������ʾѡ��*/
	const uint8_t m_indoorrun_data_display_option[INDOORRUN_DATA_DISPLAY_SUM]={SPORT_DATA_DISPALY_TOTAL_DISTANCE,SPORT_DATA_DISPALY_REALTIME_PACE,SPORT_DATA_DISPALY_TOTAL_TIME,SPORT_DATA_DISPALY_AVE_PACE,
		SPORT_DATA_DISPALY_OPTIMUM_PACE,SPORT_DATA_DISPALY_REALTIME_HEART,SPORT_DATA_DISPALY_AVE_HEART,SPORT_DATA_DISPALY_REALTIME_FREQUENCY,
		SPORT_DATA_DISPALY_AVE_FREQUENCY,SPORT_DATA_DISPALY_CALORIE};

	/*��������������ѡ��*/
	const uint8_t m_indoorrun_remind_set_option[INDOORRUN_REMIND_SET_SUM]={SPORT_REMIND_SET_HAERT,SPORT_REMIND_SET_HRRECOVERY,SPORT_REMIND_SET_INDOORRUNLENG};

	static uint8_t m_gui_sport_indoorrun_index;		//�����ܽ�������
//#endif

static uint8_t m_gui_sport_indoorrun_pause_index;	//��������ͣ�˵���������

#if defined WATCH_SIM_SPORT
//����������������ʾ��
void gui_sport_indoorrun_data_detail_paint(uint8_t state ,uint8_t m_sport_data_display)
{
	uint16_t ave_pace = 0;
   	switch(m_sport_data_display)
   	{
		case SPORT_DATA_DISPALY_TOTAL_TIME:
			gui_sport_data_display_total_time_paint(state,ActivityData.ActTime);
			break;
		case SPORT_DATA_DISPALY_TOTAL_DISTANCE:
			gui_sport_data_display_total_distance_paint(state,ActivityData.ActivityDetails.RunningDetail.Distance);
			break;
		case SPORT_DATA_DISPALY_REALTIME_PACE:
			gui_sport_data_display_realtime_pace_paint(state,ActivityData.ActivityDetails.RunningDetail.Pace);
			break;
		case SPORT_DATA_DISPALY_AVE_PACE:
			ave_pace = CalculateSpeed(ActivityData.ActTime,0,ActivityData.ActivityDetails.RunningDetail.Distance,ACT_INDOORRUN);
			gui_sport_data_display_ave_pace_paint(state,ave_pace);
			break;
		case SPORT_DATA_DISPALY_OPTIMUM_PACE:
			gui_sport_data_display_optimum_pace_paint(state,ActivityData.ActivityDetails.RunningDetail.OptimumPace);
			break;
		case SPORT_DATA_DISPALY_REALTIME_HEART:
			gui_sport_data_display_realtime_heart_paint(state,Get_Sport_Heart());
			break;
		case SPORT_DATA_DISPALY_AVE_HEART:
			gui_sport_data_display_ave_heart_paint(state,ActivityData.ActivityDetails.RunningDetail.AvgHeart);
			break;
		case SPORT_DATA_DISPALY_REALTIME_FREQUENCY:
			gui_sport_data_display_realtime_frequency_paint(state,ActivityData.ActivityDetails.RunningDetail.frequency);
			break;
		case SPORT_DATA_DISPALY_AVE_FREQUENCY:
			gui_sport_data_display_ave_frequency_paint(state,get_ave_frequency());
			break;
		case SPORT_DATA_DISPALY_CALORIE:
			gui_sport_data_display_calorie_paint(state,ActivityData.ActivityDetails.RunningDetail.Calorie);
			break;
		default:
			break;
   	}
}

//���������ݽ���
void gui_sport_indoorrun_data_paint(void)
{
	SetWord_t word = {0};
	//���ñ�����ɫ
	LCD_SetBackgroundColor(LCD_BLACK);

	//Բ������
    gui_sport_index_circle_paint(0, 3, 1);

	
	//��һ��
	gui_sport_indoorrun_data_detail_paint(SPORT_DATA_DISPALY_INDEX_UP_CIRCLE,m_indoorrun_data_display_buf[0]);
	
	//�ָ���
	word.x_axis = 80;
	LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,LCD_AUQAMARIN,0,0,LCD_FILL_ENABLE);

	//�ڶ���
	gui_sport_indoorrun_data_detail_paint(SPORT_DATA_DISPALY_INDEX_MID_GPS,m_indoorrun_data_display_buf[1]);

	//�ָ���
	word.x_axis = 160;	
	LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,LCD_AUQAMARIN,0,0,LCD_FILL_ENABLE);

	//������
	gui_sport_indoorrun_data_detail_paint(SPORT_DATA_DISPALY_INDEX_DOWN_NOHINT,m_indoorrun_data_display_buf[2]);

	
	//GPS״̬,��ֹ�ڵ������ÿ���λ��
//	gui_sport_gps_status(SPORT_GPS_LEFT1);

}

//���������ʽ���
void gui_sport_indoorrun_heart_paint(void)
{
	//���ñ�����ɫ
	LCD_SetBackgroundColor(LCD_BLACK);
	//Բ������
    gui_sport_index_circle_paint(1, 3, 1);
	
	//����ֵ
	gui_sport_heart_1_paint(Get_Sport_Heart(),0);
	
}

//�ܲ�ʵʱʱ�����
void gui_sport_indoorrun_realtime_paint(void)
{
	//���ñ�����ɫ
	LCD_SetBackgroundColor(LCD_BLACK);
	
	//Բ������
    gui_sport_index_circle_paint(2, 3, 1);
	
	//ʵʱʱ��
	gui_sport_realtime_paint(0);
		
}
 
#else
 
//�˶���ʼ��ʾ
static void gui_sport_indoorrun_start(void)
{
	SetWord_t word = {0};

	//ͼ��
	word.x_axis = 20;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	LCD_SetPicture(word.x_axis, word.y_axis, LCD_NONE, LCD_NONE, &Img_start_18X24);
	
	//��ok����ʼ�˶�
	word.x_axis += (Img_start_18X24.height + 6);
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	LCD_SetString((char *)SportsPromptStrs[SetValue.Language][0],&word);

}

//����
static void gui_sport_indoorrun_distance(uint32_t distance)
{
	SetWord_t word = {0};
	char str[10] = {0};
	uint8_t len;
	
	//�������
	word.x_axis = 30;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_32_SIZE;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 1;
	memset(str,0,sizeof(str));
	#if defined (SPRINTF_FLOAT_TO_INT)
	sprintf(str,"%d.%02d",distance/100000,distance/1000%100);
	#else
	sprintf(str,"%0.2f",distance/100000.f);	//����װ��Ϊǧ�ף�������2λС��
	#endif
	LCD_SetNumber(str,&word);
	
	len = strlen(str)*(Font_Number_32.width + 1) - 1;
	word.x_axis += (Font_Number_32.height - 16);
	word.y_axis = (LCD_PIXEL_ONELINE_MAX - len)/2 + len + 8;
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 0;
	LCD_SetString("km",&word);
}

//����
static void gui_sport_indoorrun_heart(uint8_t heart)
{
	SetWord_t word = {0};
	char str[10] = {0};
	
	memset(str,0,sizeof(str));
    sprintf(str,"%d",heart);
	
	//����ͼ��
	word.x_axis = 112;
	word.y_axis = 46 - ((strlen(str)*Font_Number_32.width + Img_heartrate_18X16.width + 8)/2);
	LCD_SetPicture(word.x_axis,word.y_axis,LCD_NONE,LCD_NONE,&Img_heartrate_18X16);
	
	//��������
	word.x_axis = 100;
	word.y_axis += (Img_heartrate_18X16.width + 8);
	word.size = LCD_FONT_32_SIZE;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = get_hdr_range_color(heart);
	word.kerning = 0;
	LCD_SetNumber(str,&word);
}

//�˶���ʱ
static void gui_sport_indoorrun_1_time(void)
{
	SetWord_t word = {0};
	char str[10] = {0};
	
	memset(str,0,sizeof(str));
	if(ActivityData.Act_Stop_Time.Hour > 0)
	{
		//ʱ����
		sprintf(str,"%d:%02d:%02d",ActivityData.Act_Stop_Time.Hour, ActivityData.Act_Stop_Time.Minute, ActivityData.Act_Stop_Time.Second);
	}
	else
	{
		//����
		sprintf(str,"%d:%02d",ActivityData.Act_Stop_Time.Minute, ActivityData.Act_Stop_Time.Second);
	}
	
	word.x_axis = 100;
	word.y_axis = 162 - ((strlen(str)*Font_Number_32.width)/2);
	word.size = LCD_FONT_32_SIZE;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 1;
	LCD_SetNumber(str, &word);
}

//�ٶ�
static void gui_sport_indoorrun_speed(uint16_t speed)
{
	SetWord_t word = {0};
	char str[10] = {0};
	uint8_t len;
	
	//��������
	word.x_axis = 170;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_32_SIZE;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 1;
	memset(str,0,sizeof(str));
    sprintf(str,"%d:%02d",(speed/60),(speed%60));
	LCD_SetNumber(str,&word);
	
	len = strlen(str)*(Font_Number_32.width + 1) - 1;
	word.x_axis += (Font_Number_32.height - 16);
	word.y_axis = (LCD_PIXEL_ONELINE_MAX - len)/2 + len + 8;
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 0;
	LCD_SetString("/km",&word);
}
#endif

//�����ܽ���
void gui_sport_indoorrun_paint(void)
{	
#if defined WATCH_SIM_SPORT
	if(m_gui_sport_indoorrun_index == GUI_SPORT_INDOORRUN_SCREEN_1)
	{
		//������ʾ
	 	gui_sport_indoorrun_data_paint();
	}
	else if (m_gui_sport_indoorrun_index == GUI_SPORT_INDOORRUN_SCREEN_2)
	{
		//���ʡ�����
		gui_sport_indoorrun_heart_paint();
	}
	else
	{
		//ʱ�䡢����
		gui_sport_indoorrun_realtime_paint();
	}
#else
	//��ʾ��ɫ����
	LCD_SetBackgroundColor(LCD_BLACK);

	if(g_sport_status == SPORT_STATUS_READY)
	{
		//��ʾ
		gui_sport_indoorrun_start();
	}
	else
	{
		//���
		gui_sport_indoorrun_distance(ActivityData.ActivityDetails.RunningDetail.Distance);
	}
	
	//�ָ���
	LCD_SetRectangle(80, 1, 0, LCD_LINE_CNT_MAX ,LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);
	
	//����
	gui_sport_indoorrun_heart(Get_Sport_Heart());
	
	//��ʱ
	gui_sport_indoorrun_1_time();
	
	//�ָ���
	LCD_SetRectangle(160, 1, 0, LCD_LINE_CNT_MAX, LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);
	
	//����
	gui_sport_indoorrun_speed(ActivityData.ActivityDetails.RunningDetail.Pace);
	
	//ʱ��͵���
	gui_sport_realtime_battery(2);
#endif
	if(g_sport_status == SPORT_STATUS_PAUSE)
	{
		//��ʱ���Ӻ�ɫԲȦ����ʾ��ͣ״̬
		LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_RED);
	}
}

//��ͣ��ʱ
static void gui_sport_indoorrun_pause_time(void)
{
	SetWord_t word = {0};
	char str[10] = {0};
	uint32_t second;

	//������ʱ��
	second = PauseTime.Pause_Stop_Time.Hour*60*60 + PauseTime.Pause_Stop_Time.Minute*60 + PauseTime.Pause_Stop_Time.Second;
	
	if(second >= 6000)
	{
		//��ͣʱ��������99:59
		second = 5999;
	}
	
	memset(str,0,sizeof(str));
	sprintf(str,"%d:%02d",second/60,second%60);
	
	//��ͣʱ��
	word.x_axis = 20;
	word.y_axis = (LCD_PIXEL_ONELINE_MAX - 48 - 5 - strlen(str)*(Font_Number_16.width+1))/2;
	word.size = LCD_FONT_16_SIZE;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_BLACK;
	word.kerning = 0;
	LCD_SetString((char *)SportsPromptStrs[SetValue.Language][1],&word);
	
	word.y_axis += 48 + 6;
	word.kerning = 1;
	LCD_SetNumber(str,&word);
}

//�˶�ʱ��
static void gui_sport_indoorrun_time(void)
{
	SetWord_t word = {0};
	char str[10] = {0};
	
	word.x_axis = 57;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_48_SIZE;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_BLACK;
	word.kerning = 1;
	memset(str,0,sizeof(str));
	if(ActivityData.Act_Stop_Time.Hour > 0)
	{
		//ʱ����
		sprintf(str,"%d:%02d:%02d",ActivityData.Act_Stop_Time.Hour, ActivityData.Act_Stop_Time.Minute, ActivityData.Act_Stop_Time.Second);	
	}
	else
	{
		//����
		sprintf(str,"%d:%02d",ActivityData.Act_Stop_Time.Minute, ActivityData.Act_Stop_Time.Second);
	}
	LCD_SetNumber(str, &word);
}

//��ͣ�˵�ѡ��
static void gui_sport_indoorrun_pause_options(uint8_t index)
{
	SetWord_t word = {0};
	
	//�˵���һ��
	word.x_axis = 133;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 0;
	word.size = LCD_FONT_24_SIZE;
#if defined WATCH_SIM_SPORT
	LCD_SetString((char *)IndoorrunPauseStrs[SetValue.Language][index],&word);
#else
	LCD_SetString((char *)SwimPauseStrs[SetValue.Language][index],&word);
#endif
	
	//�˵��ڶ���
	if(index >= GUI_SPORT_INDOORRUN_PAUSE_CANCEL)
	{
		index = GUI_SPORT_INDOORRUN_PAUSE_CONTINUE;
	}
	else
	{
		index += 1;
	}
	word.x_axis = 183;
	word.size = LCD_FONT_16_SIZE;
#if defined WATCH_SIM_SPORT
	LCD_SetString((char *)IndoorrunPauseStrs[SetValue.Language][index],&word);
#else
	LCD_SetString((char *)SwimPauseStrs[SetValue.Language][index],&word);
#endif
}

//��ͣ�˵�����
void gui_sport_indoorrun_pause_paint(void)
{
	//�ϰ벿��Ϊ��ɫ����
	LCD_SetRectangle(0, 120, 0, LCD_PIXEL_ONELINE_MAX, LCD_WHITE, 0, 0, LCD_FILL_ENABLE);
	
	//��ͣ��ʱ
	gui_sport_indoorrun_pause_time();
	
	//�˶���ʱ
	gui_sport_indoorrun_time();
	
	//�°벿��Ϊ��ɫ����
	LCD_SetRectangle(120, 120, 0, LCD_PIXEL_ONELINE_MAX, LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
	
	//ѡ��
	gui_sport_indoorrun_pause_options(m_gui_sport_indoorrun_pause_index);
	
	//�ָ���
	LCD_SetRectangle(170, 1, 0, LCD_LINE_CNT_MAX, LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);
	
	//�·���ʾ
	gui_sport_page_prompt(SPORT_PAGE_DOWN);
	
	//��ɫԲȦ
	LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_RED);
}

//�����˵�ѡ��
static void gui_sport_indoorrun_cancel_options(uint8_t index)
{
	SetWord_t word = {0};
	
	//�˵���һ��
	word.x_axis = 133;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 0;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString((char *)CancelStrs[SetValue.Language][index],&word);
	
	//�˵��ڶ���
	if(index >= GUI_SPORT_INDOORRUN_PAUSE_CANCEL_YES)
	{
		index = GUI_SPORT_INDOORRUN_PAUSE_CANCEL_NO;
	}
	else
	{
		index += 1;
	}
	word.x_axis = 183;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString((char *)CancelStrs[SetValue.Language][index],&word);
}

//�����˵�����
void gui_sport_indoorrun_cancel_paint(void)
{
	SetWord_t word = {0};
	
	//�ϰ벿��Ϊ��ɫ����
	LCD_SetRectangle(0, 120, 0, LCD_PIXEL_ONELINE_MAX, LCD_WHITE, 0, 0, LCD_FILL_ENABLE);
	
	//����?
	word.x_axis = 48;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_BLACK;
	word.kerning = 0;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString((char *)RunPauseStrs[SetValue.Language][3],&word);
    word.y_axis = 104 + 32 + 2;
    LCD_SetString("?",&word);
	
	//�°벿��Ϊ��ɫ����
	LCD_SetRectangle(120, 120, 0, LCD_PIXEL_ONELINE_MAX, LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
	
	//ѡ��
	gui_sport_indoorrun_cancel_options(m_gui_sport_indoorrun_pause_index);
	
	//�ָ���
	LCD_SetRectangle(170, 1, 0, LCD_LINE_CNT_MAX, LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);
	
	//�·���ʾ
	gui_sport_page_prompt(SPORT_PAGE_DOWN);
	
	//��ɫԲȦ
	LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_RED);
}

//��ϸ���ݽ���1
static void gui_sport_indoorrun_detail_1_paint(uint8_t backgroundcolor)
{
	SetWord_t word = {0};
	char str[20] = {0};
	uint32_t second;
	uint8_t charlen;
	
	//����ʱ�� = ��ʱ�� - ��ͣʱ��
	second = ActivityData.ActTime; //get_time_diff(ActivityData.Act_Start_Time, ActivityData.Act_Stop_Time) - Get_PauseTotolTime();
	
	//���ñ�����ɫ
    LCD_SetBackgroundColor(backgroundcolor);
	
	if (backgroundcolor == LCD_BLACK)
    {
        //�ڵװ���
        word.forecolor = LCD_WHITE;
    }
    else
    {
        //�׵׺���
        word.forecolor = LCD_BLACK;
    }

	//ʱ��
	word.x_axis = 34;
    word.y_axis = LCD_CENTER_JUSTIFIED;
#if defined WATCH_SIM_SPORT
	word.size = LCD_FONT_20_SIZE;
#else
    word.size = LCD_FONT_24_SIZE;
#endif
    word.bckgrndcolor = LCD_NONE;
    word.kerning = 0;
	memset(str,0,sizeof(str));
    sprintf(str,"%02d-%02d-%02d %02d:%02d",ActivityData.Act_Start_Time.Year,ActivityData.Act_Start_Time.Month,
											ActivityData.Act_Start_Time.Day,ActivityData.Act_Start_Time.Hour,
											ActivityData.Act_Start_Time.Minute);
    LCD_SetNumber(str,&word);
	
	//�ָ���
    word.x_axis = 70;
    LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,LCD_AUQAMARIN,0,0,LCD_FILL_ENABLE);
	
    //���
	charlen = strlen(SportDetailStrs[SetValue.Language][0]);
    word.x_axis = 90;
    word.y_axis = 95 - charlen*8;
	word.kerning = 0;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][0],&word);
	word.y_axis = 98;
    LCD_SetString(":",&word);
	
	word.y_axis = 110;
    word.kerning = 1;
    memset(str,0,sizeof(str));
	#if defined (SPRINTF_FLOAT_TO_INT)
	sprintf(str,"%d.%02d",ActivityData.ActivityDetails.RunningDetail.Distance/100000,ActivityData.ActivityDetails.RunningDetail.Distance/1000%100);
	#else
	sprintf(str,"%0.2f",ActivityData.ActivityDetails.RunningDetail.Distance/100000.f);	//����װ��Ϊǧ�ף�������2λС��
	#endif
    LCD_SetNumber(str,&word);
	
    word.y_axis = SPORT_DETAIL_DATA_UNITS_Y_AXIS;
    word.kerning = 0;
    LCD_SetString("km",&word);
	
	//ʱ��
	charlen = strlen(SportDetailStrs[SetValue.Language][1]);
	word.x_axis = 120;
    word.y_axis = 95 - charlen*8;
	word.kerning = 0;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][1],&word);
	word.y_axis = 98;
    LCD_SetString(":",&word);
	
	word.y_axis = 110;
    word.kerning = 1;
    memset(str,0,sizeof(str));
	if(second >= 3600)
	{
		//ʱ����
		sprintf(str,"%d:%02d:%02d",second/3600,second/60%60,second%60);
	}
	else
	{
		//����
		sprintf(str,"%d:%02d",second/60%60,second%60);
	}
    LCD_SetNumber(str,&word);
	
	//ƽ������
	charlen = strlen(SportDetailStrs[SetValue.Language][2]);
	word.x_axis = 150;
    word.y_axis = 95 - charlen*8;
	word.kerning = 0;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][2],&word);
	word.y_axis = 98;
    LCD_SetString(":",&word);
	
	word.y_axis = 110;
    word.kerning = 1;
    memset(str,0,sizeof(str));
    sprintf(str,"%d:%02d",(ActivityData.ActivityDetails.RunningDetail.Pace/60),
							(ActivityData.ActivityDetails.RunningDetail.Pace%60));
    LCD_SetNumber(str,&word);
	
    word.y_axis = SPORT_DETAIL_DATA_UNITS_Y_AXIS;
    word.kerning = 0;
    LCD_SetString("/km",&word);
	
	//ƽ����Ƶ
	charlen = strlen(SportDetailStrs[SetValue.Language][24]);
    word.x_axis = 180;
    word.y_axis = 95 - charlen*8;
	word.kerning = 0;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][24],&word);
	word.y_axis = 98;
    LCD_SetString(":",&word);
	
	word.y_axis = 110;
    word.kerning = 1;
    memset(str,0,sizeof(str));
	if(second != 0)
	{
		sprintf(str,"%d",
		   ActivityData.ActivityDetails.RunningDetail.Steps*60/(ActivityData.ActTime));//��/min
	}
	else
	{
		sprintf(str,"%d",0);
	}
    LCD_SetNumber(str,&word);
	
    word.y_axis = SPORT_DETAIL_DATA_UNITS_Y_AXIS;
    word.kerning = 0;
    LCD_SetString("spm",&word);
	
	//�·���ʾ
	gui_sport_page_prompt(SPORT_PAGE_DOWN);
}

//��ϸ���ݽ���2
static void gui_sport_indoorrun_detail_2_paint(uint8_t backgroundcolor)
{
	SetWord_t word = {0};
	char str[20] = {0};
	uint8_t charlen;
	
	//���ñ�����ɫ
    LCD_SetBackgroundColor(backgroundcolor);
	
	if (backgroundcolor == LCD_BLACK)
    {
        //�ڵװ���
        word.forecolor = LCD_WHITE;
    }
    else
    {
        //�׵׺���
        word.forecolor = LCD_BLACK;
    }
	
	//�Ϸ���ʾ
	gui_sport_page_prompt(SPORT_PAGE_UP);

	//��·��
	charlen = strlen(SportDetailStrs[SetValue.Language][4]);
	word.x_axis = 30;
    word.y_axis = 95 - charlen*8;
	word.kerning = 0;
	word.bckgrndcolor = LCD_NONE;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][4],&word);
	word.y_axis = 98;
    LCD_SetString(":",&word);
	
	word.y_axis = 110;
    word.kerning = 1;
    memset(str,0,sizeof(str));
    sprintf(str,"%d",ActivityData.ActivityDetails.RunningDetail.Calorie/1000);	//����װ��Ϊǧ����������һλС����
    LCD_SetNumber(str,&word);
	
    word.y_axis = SPORT_DETAIL_DATA_UNITS_Y_AXIS;
    word.kerning = 0;
    LCD_SetString("kcal",&word);
	
    //ƽ������
	charlen = strlen(SportDetailStrs[SetValue.Language][5]);
	word.x_axis = 55;
    word.y_axis = 95 - charlen*8;
	word.kerning = 0;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][5],&word);
	word.y_axis = 98;
    LCD_SetString(":",&word);
	
	word.y_axis = 110;
    word.kerning = 1;
    memset(str,0,sizeof(str));
    sprintf(str,"%d",ActivityData.ActivityDetails.RunningDetail.AvgHeart);
    LCD_SetNumber(str,&word);
	
    word.y_axis = SPORT_DETAIL_DATA_UNITS_Y_AXIS;
    word.kerning = 0;
    LCD_SetString("bpm",&word);
	
	//�������
	charlen = strlen(SportDetailStrs[SetValue.Language][6]);
	word.x_axis = 80;
    word.y_axis = 95 - charlen*8;
	word.kerning = 0;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][6],&word);
	word.y_axis = 98;
    LCD_SetString(":",&word);
	
	word.y_axis = 110;
    word.kerning = 1;
    memset(str,0,sizeof(str));
    sprintf(str,"%d",ActivityData.ActivityDetails.RunningDetail.MaxHeart);
    LCD_SetNumber(str,&word);
	
    word.y_axis = SPORT_DETAIL_DATA_UNITS_Y_AXIS;
    word.kerning = 0;
    LCD_SetString("bpm",&word);

	//���ʻָ���
	charlen = strlen(SportDetailStrs[SetValue.Language][42]);
	word.x_axis = 105;
	word.y_axis = 95 - charlen*8;
	word.kerning = 0;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][42],&word);
	word.y_axis = 98;
	LCD_SetString(":",&word);
	if (ActivityData.ActivityDetails.CommonDetail.HRRecovery == 250)
	{
		word.y_axis = 110;
		word.kerning = 1;
		LCD_SetString("--",&word);
	}
	else
	{
		memset(str,0,20);
		#if defined (SPRINTF_FLOAT_TO_INT)
		sprintf(str,"%d.%01d",ActivityData.ActivityDetails.CommonDetail.HRRecovery/10,ActivityData.ActivityDetails.CommonDetail.HRRecovery%10);
		#else
		sprintf(str,"%0.1f",ActivityData.ActivityDetails.CommonDetail.HRRecovery/10.f);
		#endif
		word.y_axis = 110;
		word.kerning = 1;
		LCD_SetNumber(str,&word);
	}
	
	
	//����Ч��
	charlen = strlen(SportDetailStrs[SetValue.Language][7]);
	word.x_axis += 25;
    word.y_axis = 95 - charlen*8;
	word.kerning = 0;
	word.bckgrndcolor = LCD_NONE;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][7],&word);
	word.y_axis = 98;
    LCD_SetString(":",&word);
	
	word.y_axis = 110;
    word.kerning = 1;
    memset(str,0,sizeof(str));
	#if defined (SPRINTF_FLOAT_TO_INT)
	sprintf(str,"%d.%01d",ActivityData.ActivityDetails.RunningDetail.AerobicEffect/10,ActivityData.ActivityDetails.RunningDetail.AerobicEffect%10);
	#else
    sprintf(str,"%.1f",ActivityData.ActivityDetails.RunningDetail.AerobicEffect/10.f);		//����һλС����
    #endif
    LCD_SetNumber(str,&word);
	
    //����Ч��
	charlen = strlen(SportDetailStrs[SetValue.Language][8]);
	word.x_axis += 25;
    word.y_axis = 95 - charlen*8;
	word.kerning = 0;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][8],&word);
	word.y_axis = 98;
    LCD_SetString(":",&word);
	
	word.y_axis = 110;
    word.kerning = 1;
    memset(str,0,sizeof(str));
	#if defined (SPRINTF_FLOAT_TO_INT)
	sprintf(str,"%d.%01d",ActivityData.ActivityDetails.RunningDetail.AnaerobicEffect/10,ActivityData.ActivityDetails.RunningDetail.AnaerobicEffect%10);
	#else
    sprintf(str,"%.1f",ActivityData.ActivityDetails.RunningDetail.AnaerobicEffect/10.f);	//����һλС����
    #endif
    LCD_SetNumber(str,&word);
	
	//�ָ�ʱ��
	charlen = strlen(SportDetailStrs[SetValue.Language][9]);
	word.x_axis += 25;
	word.y_axis = 95 - charlen*8;
	word.kerning = 0;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][9],&word);
	word.y_axis = 98;
	LCD_SetString(":",&word);
	
	word.y_axis = 110;
	word.kerning = 1;
	memset(str,0,sizeof(str));
	#if defined (SPRINTF_FLOAT_TO_INT)
	sprintf(str,"%d.%01d",ActivityData.ActivityDetails.RunningDetail.RecoveryTime/60,ActivityData.ActivityDetails.RunningDetail.RecoveryTime%60);
	#else
	sprintf(str,"%.1f",ActivityData.ActivityDetails.RunningDetail.RecoveryTime/60.f);
	#endif
	LCD_SetNumber(str,&word);
	
	word.y_axis = SPORT_DETAIL_DATA_UNITS_Y_AXIS;
	word.kerning = 0;
	LCD_SetString("h",&word);
	
	//�·���ʾ
	gui_sport_page_prompt(SPORT_PAGE_DOWN);
}
#if 0
//�˶������������
static void gui_sport_indoorrun_detail_heartratezone_paint(uint8_t backgroundcolor)
{
	SetWord_t word = {0};
	char str[10] = {0};
	uint8_t width,i;
	uint32_t MaxHeartRateZone = 0;
	uint8_t charlen;

	for (i = 0; i < 5;i++)
	{
		if(MaxHeartRateZone < ActivityData.ActivityDetails.RunningDetail.HeartRateZone[i])
		{
			MaxHeartRateZone = ActivityData.ActivityDetails.RunningDetail.HeartRateZone[i];
		} 
	}
	
	//���ñ�����ɫ
	LCD_SetBackgroundColor(backgroundcolor);
	if (backgroundcolor == LCD_BLACK)
	{
        //�ڵװ���
        word.forecolor = LCD_WHITE;
    }
    else
    {
        //�׵׺���
        word.forecolor = LCD_BLACK;
    }
  
    //�Ϸ���ʾ
	gui_sport_page_prompt(SPORT_PAGE_UP);
	
	//��������ͼ
    word.x_axis = 25;
    word.y_axis = LCD_CENTER_JUSTIFIED;
    word.size = LCD_FONT_16_SIZE;
    word.bckgrndcolor = LCD_NONE;
    LCD_SetString((char *)SportDetailStrs[SetValue.Language][13],&word);
    
	//����5
	charlen = strlen(SportDetailStrs[SetValue.Language][14]);
    word.x_axis = 60;
    word.y_axis = 35;
    word.size = LCD_FONT_16_SIZE;
    word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;
    LCD_SetString((char *)SportDetailStrs[SetValue.Language][14],&word);
    word.y_axis += (charlen*8+1);
    LCD_SetString("5",&word);
	
    memset(str,0,sizeof(str));
    sprintf(str,"%d:%02d",(ActivityData.ActivityDetails.RunningDetail.HeartRateZone[4]/60),
							(ActivityData.ActivityDetails.RunningDetail.HeartRateZone[4]%60));
    word.y_axis += (8 + 8);
	word.kerning = 1;
    LCD_SetNumber(str,&word);
	
    word.y_axis += 44;
    width = 80 * ActivityData.ActivityDetails.RunningDetail.HeartRateZone[4]/MaxHeartRateZone;
    LCD_SetRectangle(word.x_axis, 16, word.y_axis, width, LCD_RED, 0, 0, LCD_FILL_ENABLE);

    //����4
    word.x_axis = 90;
    word.y_axis = 35;
	word.kerning = 0;
    LCD_SetString((char *)SportDetailStrs[SetValue.Language][14],&word);
    word.y_axis += (charlen*8+1);
    LCD_SetString("4",&word);
	
    memset(str,0,sizeof(str));
    sprintf(str,"%d:%02d",(ActivityData.ActivityDetails.RunningDetail.HeartRateZone[3]/60),
							(ActivityData.ActivityDetails.RunningDetail.HeartRateZone[3]%60));
    word.y_axis += (8 + 8);
	word.kerning = 1;
    LCD_SetNumber(str,&word);
	
    word.y_axis += 44;
    width = 80 * ActivityData.ActivityDetails.RunningDetail.HeartRateZone[3]/MaxHeartRateZone;
    LCD_SetRectangle(word.x_axis, 16, word.y_axis, width, LCD_CHOCOLATE, 0, 0, LCD_FILL_ENABLE);

	//����3
    word.x_axis = 120;
    word.y_axis = 35;
	word.kerning = 0;
    LCD_SetString((char *)SportDetailStrs[SetValue.Language][14],&word);
    word.y_axis += (charlen*8+1);
    LCD_SetString("3",&word);
	
    memset(str,0,sizeof(str));
    sprintf(str,"%d:%02d",(ActivityData.ActivityDetails.RunningDetail.HeartRateZone[2]/60),
							(ActivityData.ActivityDetails.RunningDetail.HeartRateZone[2]%60));
    word.y_axis += (8 + 8);
	word.kerning = 1;
    LCD_SetNumber(str,&word);
	
    word.y_axis += 44;
    width = 80 * ActivityData.ActivityDetails.RunningDetail.HeartRateZone[2]/MaxHeartRateZone;
    LCD_SetRectangle(word.x_axis, 16, word.y_axis, width, LCD_GREEN, 0, 0, LCD_FILL_ENABLE);

	//����2
    word.x_axis = 150;
    word.y_axis = 35;
	word.kerning = 0;
    LCD_SetString((char *)SportDetailStrs[SetValue.Language][14],&word);
    word.y_axis += (charlen*8+1);
    LCD_SetString("2",&word);
	
    memset(str,0,sizeof(str));
    sprintf(str,"%d:%02d",(ActivityData.ActivityDetails.RunningDetail.HeartRateZone[1]/60),
							(ActivityData.ActivityDetails.RunningDetail.HeartRateZone[1]%60));
    word.y_axis += (8 + 8);
	word.kerning = 1;
    LCD_SetNumber(str,&word);
	
    word.y_axis += 44;
    width = 80 * ActivityData.ActivityDetails.RunningDetail.HeartRateZone[1]/MaxHeartRateZone;
    LCD_SetRectangle(word.x_axis, 16, word.y_axis, width, LCD_CORNFLOWERBLUE, 0, 0, LCD_FILL_ENABLE);

    //����1
    word.x_axis = 180;
    word.y_axis = 35;
	word.kerning = 0;
    LCD_SetString((char *)SportDetailStrs[SetValue.Language][14],&word);
    word.y_axis += (charlen*8+1);
    LCD_SetString("1",&word);
	
    memset(str,0,sizeof(str));
    sprintf(str,"%d:%02d",(ActivityData.ActivityDetails.RunningDetail.HeartRateZone[0]/60),
							(ActivityData.ActivityDetails.RunningDetail.HeartRateZone[0]%60));
    word.y_axis += (8 + 8);
	word.kerning = 1;
    LCD_SetNumber(str,&word);
	
    word.y_axis += 44;
    width = 80 * ActivityData.ActivityDetails.RunningDetail.HeartRateZone[0]/MaxHeartRateZone;
    LCD_SetRectangle(word.x_axis, 16, word.y_axis, width, LCD_LIGHTGRAY, 0, 0, LCD_FILL_ENABLE);
}
#endif
//��ϸ���ݽ���
void gui_sport_indoorrun_detail_paint(void)
{
	switch(m_gui_sport_indoorrun_pause_index)
	{
		case GUI_SPORT_INDOORRUN_DETAIL_1:
			gui_sport_indoorrun_detail_1_paint(LCD_BLACK);
			break;
		case GUI_SPORT_INDOORRUN_DETAIL_2:
			gui_sport_indoorrun_detail_2_paint(LCD_BLACK);
			break;
		default:

			gui_sport_detail_heartratezone_paint(LCD_BLACK);
			break;
	}
}

#if defined WATCH_SIM_SPORT
void gui_sport_indoorrun_init(void)
{
	//Ĭ��Ϊ�����ܽ���1
	m_gui_sport_indoorrun_index = GUI_SPORT_INDOORRUN_SCREEN_1;
}
#endif

//�����ܽ��水���¼�
void gui_sport_indoorrun_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
			if(g_sport_status == SPORT_STATUS_READY)
			{
				//��ʼ��ʱ
				g_sport_status = SPORT_STATUS_START;
				
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				
				//����ʾ
				gui_sport_motor_prompt();
				
				//�����˶�����
				StartSport();
			}
			else if(g_sport_status == SPORT_STATUS_START)
			{
				//��ͣ�˶���������ͣ�˵�����
				g_sport_status = SPORT_STATUS_PAUSE;
				Set_PauseSta(true);
				
				ScreenState = DISPLAY_SCREEN_INDOORRUN_PAUSE;
				m_gui_sport_indoorrun_pause_index = GUI_SPORT_INDOORRUN_PAUSE_CONTINUE;
#if defined WATCH_SIM_SPORT
				Save_Detail_Index = ScreenState;
				ScreenState = DISPLAY_SCREEN_REMIND_PAUSE;
				timer_app_pasue_start();
#else
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				
#endif

				
				//����ʾ
				gui_sport_motor_prompt();
			}
			else if(g_sport_status == SPORT_STATUS_PAUSE)
			{
				//������ʱ
				g_sport_status = SPORT_STATUS_START;
				Set_PauseSta(false);
				
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
#if defined WATCH_SIM_SPORT
				Save_Detail_Index = ScreenState;
				timer_app_pasue_start();
#endif
				//����ʾ
				gui_sport_motor_prompt();
			}
			break;
#if defined WATCH_SIM_SPORT
		case KEY_UP:
				if(m_gui_sport_indoorrun_index > GUI_SPORT_INDOORRUN_SCREEN_1)
				{
					m_gui_sport_indoorrun_index --;
				}
				else
				{
					m_gui_sport_indoorrun_index = GUI_SPORT_INDOORRUN_SCREEN_3;
				}
				
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				break;
		case KEY_DOWN:
				if(m_gui_sport_indoorrun_index < GUI_SPORT_INDOORRUN_SCREEN_3)
				{
					m_gui_sport_indoorrun_index ++;
				}
				else
				{
					m_gui_sport_indoorrun_index = GUI_SPORT_INDOORRUN_SCREEN_1;
				}
				
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
#endif
		case KEY_BACK:
			if(g_sport_status == SPORT_STATUS_READY)
			{
				if(0 == ENTER_NAVIGATION && 0 == ENTER_MOTION_TRAIL)    //�ɵ�����������˶�ready������ж�
				{

//				if(Get_Curr_Home_Index() == DISPLAY_SCREEN_TRAINPLAN_MARATHON)
//				{
//					//����ѵ���ƻ�����
//					ScreenState = DISPLAY_SCREEN_TRAINPLAN_MARATHON;
//				}
//				else
					{
						//�����˶�ѡ��˵�����
						ScreenState = DISPLAY_SCREEN_SPORT;
					}
					
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
					
					//�ر�GPS����
					ReadySportBack();
				}
				else
				{
					if(1 == ENTER_NAVIGATION)
					{
						ScreenState = DISPLAY_SCREEN_NAVIGATION_OPS;
					}
					else
					{
						ScreenState = DISPLAY_SCREEN_MOTION_TRAIL_DETAIL;
					}
					
					ENTER_NAVIGATION = 0;
					ENTER_MOTION_TRAIL = 0;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				}
			}
			else if(g_sport_status == SPORT_STATUS_PAUSE)
			{
				//������ͣ�˵�����
				ScreenState = DISPLAY_SCREEN_INDOORRUN_PAUSE;
				m_gui_sport_indoorrun_pause_index = GUI_SPORT_INDOORRUN_PAUSE_CONTINUE;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
			break;
		default:
			break;
	}
}

//��ͣ�˵����水���¼�
void gui_sport_indoorrun_pause_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
			if(m_gui_sport_indoorrun_pause_index == GUI_SPORT_INDOORRUN_PAUSE_CONTINUE)
			{
				//������ʱ����ʼ�˶�
				g_sport_status = SPORT_STATUS_START;
				Set_PauseSta(false);
				
				ScreenState = DISPLAY_SCREEN_INDOORRUN;
				gui_swich_msg();
#if defined WATCH_SIM_SPORT
				Save_Detail_Index = ScreenState;
				timer_app_pasue_start();
#endif				
				//����ʾ
				gui_sport_motor_prompt();
			}
			else if(m_gui_sport_indoorrun_pause_index == GUI_SPORT_INDOORRUN_PAUSE_SAVE)
			{
				//�����˶�����������
				g_sport_status = SPORT_STATUS_READY;
				
				//Ĭ��ѡ��
				gui_sport_feel_init();
				
				ScreenState = DISPLAY_SCREEN_SPORT_FEEL;
				m_gui_sport_indoorrun_pause_index = GUI_SPORT_INDOORRUN_DETAIL_1;
				sport_end_flag = 1;
				sport_end_hrvalue = get_10s_heartrate(); //��ȡǰ10sƽ������
				g_HeartNum = 0;
				extern volatile ScreenState_t Save_Detail_Index;
				Save_Detail_Index = DISPLAY_SCREEN_INDOORRUN_DETAIL;
				gui_swich_msg();
			}
#if defined WATCH_SIM_SPORT
			else if(m_gui_sport_indoorrun_pause_index == GUI_SPORT_INDOORRUN_PAUSE_WAIT)
			{
				//�Ժ����
				m_gui_sport_indoorrun_pause_index = GUI_SPORT_INDOORRUN_PAUSE_CONTINUE;
				
				//�Ժ����
				sport_continue_later_start();
				
				//�Ժ���������˶�����
				ContinueLaterDataSave.SportType = DISPLAY_SCREEN_INDOORRUN;
				
				//�Ժ�������˳��˶����棬����������
				ScreenState = DISPLAY_SCREEN_HOME;
				gui_swich_msg();
			}
#endif
			else
			{
				//�����˶�
				ScreenState = DISPLAY_SCREEN_INDOORRUN_CANCEL;
				
				m_gui_sport_indoorrun_pause_index = GUI_SPORT_INDOORRUN_PAUSE_CANCEL_NO;
				gui_swich_msg();
			}
			
			//msg.cmd = MSG_DISPLAY_SCREEN;
			//xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		case KEY_UP:
			if(m_gui_sport_indoorrun_pause_index > GUI_SPORT_INDOORRUN_PAUSE_CONTINUE)
			{
				m_gui_sport_indoorrun_pause_index--;
			}
			else
			{
				m_gui_sport_indoorrun_pause_index = GUI_SPORT_INDOORRUN_PAUSE_CANCEL;
			}
			
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		    break;
		case KEY_DOWN:
			if(m_gui_sport_indoorrun_pause_index < GUI_SPORT_INDOORRUN_PAUSE_CANCEL)
			{
				m_gui_sport_indoorrun_pause_index++;
			}
			else
			{
				m_gui_sport_indoorrun_pause_index = GUI_SPORT_INDOORRUN_PAUSE_CONTINUE;
			}
			
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		case KEY_BACK:
			//���������ܽ���
			ScreenState = DISPLAY_SCREEN_INDOORRUN;
		
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}

//�����˵����水���¼�
void gui_sport_indoorrun_cancel_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
			if(m_gui_sport_indoorrun_pause_index == GUI_SPORT_INDOORRUN_PAUSE_CANCEL_NO)
			{
				//ȡ�������˶��������ϼ��˵�����
				ScreenState = DISPLAY_SCREEN_INDOORRUN_PAUSE;
				m_gui_sport_indoorrun_pause_index = GUI_SPORT_INDOORRUN_PAUSE_CANCEL;
			}
			else
			{
				//ȷ��ȡ���˶�������������
				g_sport_status = SPORT_STATUS_READY;
				
				ScreenState = DISPLAY_SCREEN_HOME;
				
				//�����˶�������
				Set_SaveSta(0);
				CloseSportTask();
				
				//����ʾ
				gui_sport_motor_prompt();
			}
			
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		case KEY_UP:
		case KEY_DOWN:
			if(m_gui_sport_indoorrun_pause_index == GUI_SPORT_INDOORRUN_PAUSE_CANCEL_NO)
			{
				m_gui_sport_indoorrun_pause_index = GUI_SPORT_INDOORRUN_PAUSE_CANCEL_YES;
			}
			else
			{
				m_gui_sport_indoorrun_pause_index = GUI_SPORT_INDOORRUN_PAUSE_CANCEL_NO;
			}

			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		    break;
		case KEY_BACK:
			//�����ϼ��˵�
			ScreenState = DISPLAY_SCREEN_INDOORRUN_PAUSE;
			m_gui_sport_indoorrun_pause_index = GUI_SPORT_INDOORRUN_PAUSE_CANCEL;
			
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}

//��ϸ���ݽ��水���¼�
void gui_sport_indoorrun_detail_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_UP:
			//�Ϸ�
			if(m_gui_sport_indoorrun_pause_index > GUI_SPORT_INDOORRUN_DETAIL_1)
			{
				m_gui_sport_indoorrun_pause_index--;
				
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
		    break;
		case KEY_DOWN:
			//�·�
			if(m_gui_sport_indoorrun_pause_index < GUI_SPORT_INDOORRUN_DETAIL_HEARTRATEZONE)
			{
				m_gui_sport_indoorrun_pause_index++;
				
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
			break;
		case KEY_BACK:
			//����������
			ScreenState = DISPLAY_SCREEN_HOME;
			
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}

#endif

