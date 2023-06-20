#include "gui_sport_config.h"
#include "gui_sport_triathlon.h"

#include "drv_lcd.h"
#include "drv_key.h"

#include "time_notify.h"
#include "time_progress.h"

#include "task_sport.h"

#include "com_data.h"
#include "com_sport.h"

#include "algo_hdr.h"

#include "gui_sport.h"
#include "gui_run.h"

#include "font_config.h"

#if DEBUG_ENABLED == 1 && GUI_SPORT_TRIATHLON_LOG_ENABLED == 1
	#define GUI_SPORT_TRIATHLON_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_SPORT_TRIATHLON_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define GUI_SPORT_TRIATHLON_WRITESTRING(...)
	#define GUI_SPORT_TRIATHLON_PRINTF(...)		        
#endif

#define	GUI_SPORT_TRIATHLON_SWIMMING_SCREEN_1	0	//����������Ӿ����1
#define	GUI_SPORT_TRIATHLON_SWIMMING_SCREEN_2	1	//����������Ӿ����2

#define	GUI_SPORT_TRIATHLON_PAUSE_CONTINUE		0	//����
#define	GUI_SPORT_TRIATHLON_PAUSE_SAVE			1	//�����˶�
#define	GUI_SPORT_TRIATHLON_PAUSE_CANCEL		2	//����

#define	GUI_SPORT_TRIATHLON_PAUSE_CANCEL_NO		0	//��
#define	GUI_SPORT_TRIATHLON_PAUSE_CANCEL_YES	1	//��

static uint8_t m_gui_sport_triathlon_index;			//���������������
uint8_t m_gui_sport_triathlon_pause_index;	//����������ͣ�˵���������
static ScreenState_t m_gui_sport_triathlon_type;	//��������������
static uint8_t m_gui_sport_triathlon_status;			//�����������״̬

TriathlonDetailStr TriathlonData;			//���ڱ���������������

void gui_sport_triathlon_status_set(uint8_t status)
{
	m_gui_sport_triathlon_status = status;
}

uint8_t gui_sport_triathlon_status_get(void)
{
	return m_gui_sport_triathlon_status;
}

//��������׼������
void gui_sport_triathlon_ready_paint(void)
{
	SetWord_t word = {0};
	
	//��ʾ��ɫ����
	LCD_SetBackgroundColor(LCD_BLACK);
	
	//����������Ӿͼ��
	LCD_SetPicture(18, LCD_CENTER_JUSTIFIED, LCD_NONE, LCD_NONE, &Img_Sport[Swimming]);
	
	//��Ӿ
	word.x_axis = 108;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][35], &word);
	
	//��ʾ��
	word.x_axis += LCD_FONT_24_SIZE + 10;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString((char *)SportsPromptStrs[SetValue.Language][4], &word);
	
	//GPS״̬
	gui_sport_gps_status(SPORT_GPS_LEFT1);
}

//���
static void gui_sport_triathlon_swimming_1_distance(uint32_t distance)
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
	sprintf(str,"%d",distance/100);	//����װ��Ϊ��
	LCD_SetNumber(str,&word);
	
	len = strlen(str)*(Font_Number_32.width + 1) - 1;
	word.x_axis += (Font_Number_32.height - 16);
	word.y_axis = (LCD_PIXEL_ONELINE_MAX - len)/2 + len + 8;
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 0;
	LCD_SetString("m",&word);
}

//��ʱ
static void gui_sport_triathlon_swimming_time(uint8_t x, uint8_t y, uint8_t forecolor, 
												uint8_t hour, uint8_t min, uint8_t sec)
{
	SetWord_t word = {0};
	char str[10];
	
	memset(str,0,sizeof(str));
	if(hour > 0)
	{
		//ʱ����
		sprintf(str,"%d:%02d:%02d",hour, min ,sec);
	}
	else
	{
		//����
		sprintf(str,"%d:%02d",min, sec);
	}
	
	word.x_axis = x;
#if defined WATCH_COM_SPORT 
	word.y_axis = LCD_CENTER_JUSTIFIED;
#else
	word.y_axis = y  - ((strlen(str)*(Font_Number_48.width + 1) - 1)/2);
#endif
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = forecolor;
#if defined WATCH_COM_SPORT
	if (x > 160)
	{
		word.size = LCD_FONT_32_SIZE;
	}
	else
	{
		word.size = LCD_FONT_48_SIZE;
	}
	
#else
	word.size = LCD_FONT_48_SIZE;
#endif
	word.kerning = 1;
	LCD_SetNumber(str,&word);
}

//����
static void gui_sport_triathlon_swimming_1_heart(uint8_t heart)
{
	SetWord_t word = {0};
    char str[10];
	
	//����ͼ��
    word.x_axis = 97;
	word.y_axis = 214 - (Img_heartrate_18X16.width/2);
	LCD_SetPicture(word.x_axis,word.y_axis,LCD_NONE,LCD_NONE,&Img_heartrate_18X16);
	
	//��������
	memset(str,0,sizeof(str));
    sprintf(str,"%d",heart);
    word.x_axis += Img_heartrate_18X16.height + 4;
	word.y_axis = 214 - (((Font_Number_24.width + 1)*strlen(str) - 1)/2);
    word.bckgrndcolor = LCD_NONE;
	word.forecolor = get_hdr_range_color(heart);
	word.size = LCD_FONT_24_SIZE;
	word.kerning = 1;
    LCD_SetNumber(str,&word);
}

//Ӿ��
static void gui_sport_triathlon_swimming_1_speed(uint16_t speed)
{
	SetWord_t word = {0};
	char str[10] = {0};
	uint8_t len;
	
	//�ٶ�����
#if defined WATCH_COM_SPORT
	word.x_axis = 92;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_48_SIZE;
#else
	word.x_axis = 170;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_32_SIZE;
#endif 
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 1;
	memset(str,0,sizeof(str));
    sprintf(str,"%d:%02d",(speed/60),(speed%60));
	LCD_SetNumber(str,&word);
#if defined WATCH_COM_SPORT
	len = strlen(str)*(Font_Number_48.width + 1) - 1;
	word.x_axis += (Font_Number_48.height - 16);
	word.y_axis = (LCD_PIXEL_ONELINE_MAX - len)/2 + len + 8;
#else
	len = strlen(str)*(Font_Number_32.width + 1) - 1;
	word.x_axis += (Font_Number_32.height - 16);
	word.y_axis = (LCD_PIXEL_ONELINE_MAX - len)/2 + len + 8;
#endif
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 0;
	LCD_SetString("/100m",&word);
}

//����������Ӿ����1
static void gui_sport_triathlon_swimming_1_paint(void)
{
#if defined WATCH_COM_SPORT 
	//��ʾ��ɫ����
	LCD_SetBackgroundColor(LCD_BLACK);

	//���
	gui_sport_triathlon_swimming_1_distance(ActivityData.ActivityDetails.SwimmingDetail.Distance);
	
	//�ָ���
	LCD_SetRectangle(80, 1, 0, LCD_LINE_CNT_MAX ,LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);

	//Ӿ��
	gui_sport_triathlon_swimming_1_speed(ActivityData.ActivityDetails.SwimmingDetail.Speed);

	//�ָ���
	LCD_SetRectangle(160, 1, 0, LCD_LINE_CNT_MAX, LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);

	//��ʱ
	gui_sport_triathlon_swimming_time(170, LCD_CENTER_ROW, LCD_WHITE, ActivityData.Act_Stop_Time.Hour, 
										ActivityData.Act_Stop_Time.Minute, ActivityData.Act_Stop_Time.Second);

	//GPS״̬
	gui_sport_gps_status(SPORT_GPS_LEFT1);

#else
	//��ʾ��ɫ����
	LCD_SetBackgroundColor(LCD_BLACK);
	
	//Բ������
	gui_sport_index_circle_paint(m_gui_sport_triathlon_index, 2, 0);
	
	//���
	gui_sport_triathlon_swimming_1_distance(ActivityData.ActivityDetails.SwimmingDetail.Distance);
	
	//�ָ���
	LCD_SetRectangle(80, 1, 0, LCD_LINE_CNT_MAX ,LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);
	
	//��ʱ
	gui_sport_triathlon_swimming_time(97, LCD_CENTER_ROW, LCD_WHITE, ActivityData.Act_Stop_Time.Hour, 
										ActivityData.Act_Stop_Time.Minute, ActivityData.Act_Stop_Time.Second);
	
	//����
	gui_sport_triathlon_swimming_1_heart(Get_Sport_Heart());
	
	//�ָ���
	LCD_SetRectangle(160, 1, 0, LCD_LINE_CNT_MAX, LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);
	
	//Ӿ��
	gui_sport_triathlon_swimming_1_speed(ActivityData.ActivityDetails.SwimmingDetail.Speed);
	
	//ʱ��͵���
	gui_sport_realtime_battery(0);
	
	//GPS״̬
	gui_sport_gps_status(SPORT_GPS_LEFT1);
#endif
}

//����
static void gui_sport_triathlon_swimming_2_stroke(uint32_t stroke)
{
	SetWord_t word = {0};
	char str[10] = {0};
	
	//����
	word.x_axis = 36;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 0;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][23],&word);
	
	word.x_axis += LCD_FONT_16_SIZE + 12;
	word.size = LCD_FONT_32_SIZE;
	word.kerning = 1;
	memset(str,0,sizeof(str));
    sprintf(str,"%d",stroke);
	LCD_SetNumber(str,&word);
}

//��Ƶ
static void gui_sport_triathlon_swimming_2_frequency(uint16_t frequency)
{
	SetWord_t word = {0};
	char str[10] = {0};
	uint8_t len;
	
	//��Ƶ
	word.x_axis = 132;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 0;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][25],&word);
	
	word.x_axis += LCD_FONT_16_SIZE + 12;
	word.size = LCD_FONT_32_SIZE;
	word.kerning = 1;
	memset(str,0,sizeof(str));
    sprintf(str,"%d",frequency);
	LCD_SetNumber(str,&word);
	
	//��/min
	len = strlen(str)*(Font_Number_32.width + 1) - 1;
	word.x_axis += (Font_Number_32.height - 16);
	word.y_axis = (LCD_PIXEL_ONELINE_MAX - len)/2 + len + 8;
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 0;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][26],&word);
}

static void gui_sport_triathlon_swimming_2_paint(void)
{
	//��ʾ��ɫ����
	LCD_SetBackgroundColor(LCD_BLACK);
	
	//Բ������
	gui_sport_index_circle_paint(m_gui_sport_triathlon_index, 2, 0);
	
	//����
	gui_sport_triathlon_swimming_2_stroke(ActivityData.ActivityDetails.SwimmingDetail.Strokes);
	
	//�ָ���
	LCD_SetRectangle(120, 1, 0, LCD_LINE_CNT_MAX, LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);
	
	//��Ƶ
	gui_sport_triathlon_swimming_2_frequency(ActivityData.ActivityDetails.SwimmingDetail.Frequency);
	
	//ʱ��͵���
	gui_sport_realtime_battery(0);
	
	//GPS״̬
	gui_sport_gps_status(SPORT_GPS_LEFT3);
}

//����������Ӿ����
void gui_sport_triathlon_swimming_paint(void)
{
#if defined WATCH_COM_SPORT
	gui_sport_triathlon_swimming_1_paint();
#else
	if(m_gui_sport_triathlon_index == GUI_SPORT_TRIATHLON_SWIMMING_SCREEN_1)
	{
		//����������Ӿ����1
		gui_sport_triathlon_swimming_1_paint();
	}
	else
	{
		//����������Ӿ����2
		gui_sport_triathlon_swimming_2_paint();
	}
#endif
	if(g_sport_status == SPORT_STATUS_PAUSE)
	{
		//��ʱ���Ӻ�ɫԲȦ����ʾ��ͣ״̬
		LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_RED);
	}
}

//��һ�������
void gui_sport_triathlon_first_change_paint(void)
{
	//��ʱ��
	SetWord_t word = {0};
	uint32_t second;
	
	//������ʱ��
	second = (ActivityData.Act_Stop_Time.Hour*3600) + (ActivityData.Act_Stop_Time.Minute*60) 
				+ ActivityData.Act_Stop_Time.Second + TriathlonData.SwimmingTime;
	
	//���ñ�����ɫ
	LCD_SetBackgroundColor(LCD_BLACK);
	
	word.x_axis = 30;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 0;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][27],&word);
	
	word.x_axis += LCD_FONT_16_SIZE + 12;
	gui_sport_triathlon_swimming_time(word.x_axis, LCD_CENTER_ROW, LCD_WHITE, 
										second/3600, second/60%60, second%60);
	
	//�ָ���
	LCD_SetRectangle(120, 1, 0, LCD_LINE_CNT_MAX, LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);
	
	//ת��ʱ��
	word.x_axis = 132;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 0;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][28],&word);
	
	word.x_axis += LCD_FONT_16_SIZE + 12;
	gui_sport_triathlon_swimming_time(word.x_axis, LCD_CENTER_ROW, LCD_WHITE, ActivityData.Act_Stop_Time.Hour, 
										ActivityData.Act_Stop_Time.Minute, ActivityData.Act_Stop_Time.Second);
	#if !defined WATCH_COM_SPORT									
	//ʱ��͵���
	gui_sport_realtime_battery(0);
	#endif 
	
	if(g_sport_status == SPORT_STATUS_PAUSE)
	{
		//��ʱ���Ӻ�ɫԲȦ����ʾ��ͣ״̬
		LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_RED);
	}
	
	//GPS״̬
	gui_sport_gps_status(SPORT_GPS_LEFT3);
}

//������������׼������
void gui_sport_triathlon_cycling_ready_paint(void)
{
	SetWord_t word = {0};
	char str[10] = {0};
	uint32_t second;
	
	//����ʱ��
	second = TriathlonData.FirstWaitTime;
	
	//��ʾ��ɫ����
	LCD_SetBackgroundColor(LCD_BLACK);
	
	//������������ͼ��
	LCD_SetPicture(18, LCD_CENTER_JUSTIFIED, LCD_NONE, LCD_NONE, &Img_Sport[Cycling]);
	
	//����
	word.x_axis = 108;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;
	LCD_SetString((char *)SportStrs[SetValue.Language][Cycling], &word);
	
	//ת��ʱ��
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
	
	word.x_axis += LCD_FONT_24_SIZE + 10;
	word.y_axis = (LCD_PIXEL_ONELINE_MAX - (strlen(str)*8)- (4*LCD_FONT_16_SIZE) - 4)/2;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][28], &word);
	
	word.y_axis += ((4*LCD_FONT_16_SIZE) + 4);
	LCD_SetString(str, &word);
	
	//GPS״̬
	gui_sport_gps_status(SPORT_GPS_LEFT1);
}

//���
static void gui_sport_triathlon_cycling_distance(uint32_t distance)
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

//�ٶ�
static void gui_sport_triathlon_cycling_speed(uint16_t speed)
{
	SetWord_t word = {0};
	char str[10] = {0};
	uint8_t len;
	
	//�ٶ�����
	word.x_axis = 170;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_32_SIZE;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 1;
	memset(str,0,sizeof(str));
	#if defined (SPRINTF_FLOAT_TO_INT)
	sprintf(str,"%d.%01d",speed/10,speed%10);
	#else
    sprintf(str,"%.1f",speed/10.f);		//�ٶȱ���һλС����
    #endif
	
	LCD_SetNumber(str,&word);
	
	len = strlen(str)*(Font_Number_32.width + 1) - 1;
	word.x_axis += (Font_Number_32.height - 16);
	word.y_axis = (LCD_PIXEL_ONELINE_MAX - len)/2 + len + 8;
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 0;
	LCD_SetString("km/h",&word);
}

//�����������н���
void gui_sport_triathlon_cycling_paint(void)
{
	//��ʾ��ɫ����
	LCD_SetBackgroundColor(LCD_BLACK);
	
	//���
	gui_sport_triathlon_cycling_distance(ActivityData.ActivityDetails.CyclingDetail.Distance);
	
	//�ָ���
	LCD_SetRectangle(80, 1, 0, LCD_LINE_CNT_MAX ,LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);
	
	//��ʱ
	gui_sport_triathlon_swimming_time(97, LCD_CENTER_ROW, LCD_WHITE, ActivityData.Act_Stop_Time.Hour, 
										ActivityData.Act_Stop_Time.Minute, ActivityData.Act_Stop_Time.Second);
	#if !defined WATCH_COM_SPORT
	//����
	gui_sport_triathlon_swimming_1_heart(Get_Sport_Heart());
	#endif
	//�ָ���
	LCD_SetRectangle(160, 1, 0, LCD_LINE_CNT_MAX, LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);
	
	//�ٶ�
	gui_sport_triathlon_cycling_speed(ActivityData.ActivityDetails.CyclingDetail.Speed);
	#if !defined WATCH_COM_SPORT
	//ʱ��͵���
	gui_sport_realtime_battery(0);
	#endif 
	//GPS״̬
	gui_sport_gps_status(SPORT_GPS_LEFT1);
	
	if(g_sport_status == SPORT_STATUS_PAUSE)
	{
		//��ʱ���Ӻ�ɫԲȦ����ʾ��ͣ״̬
		LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_RED);
	}
}

//�ڶ��������
void gui_sport_triathlon_second_change_paint(void)
{
	//��ʱ��
	SetWord_t word = {0};
	uint32_t second;
	
	//������ʱ��
	second = (ActivityData.Act_Stop_Time.Hour*3600) + (ActivityData.Act_Stop_Time.Minute*60) 
				+ ActivityData.Act_Stop_Time.Second + TriathlonData.SwimmingTime
				+ TriathlonData.FirstWaitTime + TriathlonData.CyclingTime;
	
	//���ñ�����ɫ
	LCD_SetBackgroundColor(LCD_BLACK);
	
	word.x_axis = 30;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 0;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][27],&word);
	
	word.x_axis += LCD_FONT_16_SIZE + 12;
	gui_sport_triathlon_swimming_time(word.x_axis, LCD_CENTER_ROW, LCD_WHITE,  
										second/3600, second/60%60, second%60);
	
	//�ָ���
	LCD_SetRectangle(120, 1, 0, LCD_LINE_CNT_MAX, LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);
	
	//ת��ʱ��
	word.x_axis = 132;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 0;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][28],&word);
	
	word.x_axis += LCD_FONT_16_SIZE + 12;
	gui_sport_triathlon_swimming_time(word.x_axis, LCD_CENTER_ROW, LCD_WHITE, ActivityData.Act_Stop_Time.Hour, 
										ActivityData.Act_Stop_Time.Minute, ActivityData.Act_Stop_Time.Second);
	#if !defined WATCH_COM_SPORT									
	//ʱ��͵���
	gui_sport_realtime_battery(0);
	#endif
	if(g_sport_status == SPORT_STATUS_PAUSE)
	{
		//��ʱ���Ӻ�ɫԲȦ����ʾ��ͣ״̬
		LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_RED);
	}
	
	//GPS״̬
	gui_sport_gps_status(SPORT_GPS_LEFT3);
}

//���������ܲ�׼������
void gui_sport_triathlon_running_ready_paint(void)
{
	SetWord_t word = {0};
	char str[10] = {0};
	uint32_t second;
	
	//����ʱ��
	second = TriathlonData.SecondWaitTime;
	
	//��ʾ��ɫ����
	LCD_SetBackgroundColor(LCD_BLACK);
	
	//���������ܲ�ͼ��
	LCD_SetPicture(18, LCD_CENTER_JUSTIFIED, LCD_NONE, LCD_NONE, &Img_Sport[Run]);
	
	//�ܲ�
	word.x_axis = 108;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;
	LCD_SetString((char *)SportStrs[SetValue.Language][Run], &word);
	
	//ת��ʱ��
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
	
	word.x_axis += LCD_FONT_24_SIZE + 10;
	word.y_axis = (LCD_PIXEL_ONELINE_MAX - (strlen(str)*8)- (4*LCD_FONT_16_SIZE) - 4)/2;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][28], &word);
	
	word.y_axis += ((4*LCD_FONT_16_SIZE) + 4);
	LCD_SetString(str, &word);
	
	//GPS״̬
	gui_sport_gps_status(SPORT_GPS_LEFT1);
}

//����
static void gui_sport_triathlon_running_speed(uint16_t speed)
{
	SetWord_t word = {0};
	char str[10] = {0};
	uint8_t len;
	
	//��������
#if defined WATCH_COM_SPORT
	word.x_axis = 92;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_48_SIZE;
#else
	word.x_axis = 170;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_32_SIZE;
#endif
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 1;
	memset(str,0,sizeof(str));
    sprintf(str,"%d:%02d",(speed/60),(speed%60));
	LCD_SetNumber(str,&word);
#if defined WATCH_COM_SPORT
	len = strlen(str)*(Font_Number_48.width + 1) - 1;
	word.x_axis += (Font_Number_48.height - 16);
	word.y_axis = (LCD_PIXEL_ONELINE_MAX - len)/2 + len + 8;

#else
	len = strlen(str)*(Font_Number_32.width + 1) - 1;
	word.x_axis += (Font_Number_32.height - 16);
	word.y_axis = (LCD_PIXEL_ONELINE_MAX - len)/2 + len + 8;
#endif
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 0;
	LCD_SetString("/km",&word);
}

//���������ܲ�����
void gui_sport_triathlon_running_paint(void)
{
	//��ʾ��ɫ����
	LCD_SetBackgroundColor(LCD_BLACK);
	
	//���
	gui_sport_triathlon_cycling_distance(ActivityData.ActivityDetails.RunningDetail.Distance);
	
	//�ָ���
	LCD_SetRectangle(80, 1, 0, LCD_LINE_CNT_MAX ,LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);
	#if defined WATCH_COM_SPORT
		gui_sport_triathlon_swimming_time(170, LCD_CENTER_ROW, LCD_WHITE, ActivityData.Act_Stop_Time.Hour, 
										ActivityData.Act_Stop_Time.Minute, ActivityData.Act_Stop_Time.Second);
	#else
	//��ʱ
	gui_sport_triathlon_swimming_time(97, LCD_CENTER_ROW, LCD_WHITE, ActivityData.Act_Stop_Time.Hour, 
										ActivityData.Act_Stop_Time.Minute, ActivityData.Act_Stop_Time.Second);
	
	//����
	gui_sport_triathlon_swimming_1_heart(Get_Sport_Heart());
	#endif
	
	//�ָ���
	LCD_SetRectangle(160, 1, 0, LCD_LINE_CNT_MAX, LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);
	
	//����
	gui_sport_triathlon_running_speed(ActivityData.ActivityDetails.RunningDetail.Pace);
	#if !defined WATCH_COM_SPORT
	//ʱ��͵���
	gui_sport_realtime_battery(0);
	#endif
	
	//GPS״̬
	gui_sport_gps_status(SPORT_GPS_LEFT1);
	
	if(g_sport_status == SPORT_STATUS_PAUSE)
	{
		//��ʱ���Ӻ�ɫԲȦ����ʾ��ͣ״̬
		LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_RED);
	}
}

//������������ɽ���
void gui_sport_triathlon_finish_paint(uint8_t endangle)
{
	SetWord_t word = {0};
	
	//���ñ�����ɫ
	LCD_SetBackgroundColor(LCD_BLACK);
	
	word.x_axis = 108;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	LCD_SetString((char *)SportsPromptStrs[SetValue.Language][5],&word);
	
	if (endangle > 6)
	{
		endangle = 6;
	}
	LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 8, 0, 60*endangle, LCD_CHOCOLATE);
}

//��ͣ��ʱ
static void gui_sport_triathlon_pause_time(void)
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
static void gui_sport_triathlon_time(void)
{
	SetWord_t word = {0};
	char str[10] = {0};
	uint32_t second;
	
	//������ʱ��
	second = (ActivityData.Act_Stop_Time.Hour*3600) + (ActivityData.Act_Stop_Time.Minute*60) 
				+ ActivityData.Act_Stop_Time.Second + TriathlonData.SwimmingTime 
				+ TriathlonData.FirstWaitTime + TriathlonData.CyclingTime 
				+ TriathlonData.SecondWaitTime + TriathlonData.RunningTime;
	
	word.x_axis = 57;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_48_SIZE;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_BLACK;
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
	LCD_SetNumber(str, &word);
}

//��ͣ�˵�ѡ��
static void gui_sport_triathlon_pause_options(uint8_t index)
{
	SetWord_t word = {0};
	
	//�˵���һ��
	word.x_axis = 133;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 0;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString((char *)SwimPauseStrs[SetValue.Language][index],&word);
	
	//�˵��ڶ���
	if(index >= GUI_SPORT_TRIATHLON_PAUSE_CANCEL)
	{
		index = GUI_SPORT_TRIATHLON_PAUSE_CONTINUE;
	}
	else
	{
		index += 1;
	}
	word.x_axis = 183;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString((char *)SwimPauseStrs[SetValue.Language][index],&word);
}

//��ͣ�˵�����
void gui_sport_triathlon_pause_paint(void)
{
	//�ϰ벿��Ϊ��ɫ����
	LCD_SetRectangle(0, 120, 0, LCD_PIXEL_ONELINE_MAX, LCD_WHITE, 0, 0, LCD_FILL_ENABLE);
	
	//��ͣ��ʱ
	gui_sport_triathlon_pause_time();
	
	//�˶���ʱ
	gui_sport_triathlon_time();
	
	//�°벿��Ϊ��ɫ����
	LCD_SetRectangle(120, 120, 0, LCD_PIXEL_ONELINE_MAX, LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
	
	//ѡ��
	gui_sport_triathlon_pause_options(m_gui_sport_triathlon_pause_index);
	
	//�ָ���
	LCD_SetRectangle(170, 1, 0, LCD_LINE_CNT_MAX, LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);
	
	//�·���ʾ
	gui_sport_page_prompt(SPORT_PAGE_DOWN);
	
	//��ɫԲȦ
	LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_RED);
}

//�����˵�ѡ��
static void gui_sport_triathlon_cancel_options(uint8_t index)
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
	if(index >= GUI_SPORT_TRIATHLON_PAUSE_CANCEL_YES)
	{
		index = GUI_SPORT_TRIATHLON_PAUSE_CANCEL_NO;
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
void gui_sport_triathlon_cancel_paint(void)
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
	gui_sport_triathlon_cancel_options(m_gui_sport_triathlon_pause_index);
	
	//�ָ���
	LCD_SetRectangle(170, 1, 0, LCD_LINE_CNT_MAX, LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);
	
	//�·���ʾ
	gui_sport_page_prompt(SPORT_PAGE_DOWN);
	
	//��ɫԲȦ
	LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_RED);
}

//��ϸ���ݽ���1
void gui_sport_triathlon_detail_1_paint(uint8_t backgroundcolor)
{
	SetWord_t word = {0};
	char str[20] = {0};
	uint32_t second;
	uint8_t charlen;
	
	//����ʱ��
	second = TriathlonData.SwimmingTime + TriathlonData.FirstWaitTime + TriathlonData.CyclingTime 
				+ TriathlonData.SecondWaitTime + TriathlonData.RunningTime;
	
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
#if defined WATCH_SIM_NUMBER
	word.size = LCD_FONT_20_SIZE;
#else
    word.size = LCD_FONT_24_SIZE;
#endif
    word.bckgrndcolor = LCD_NONE;
    word.kerning = 0;
	memset(str,0,sizeof(str));
    sprintf(str,"%02d-%02d-%02d %02d:%02d",TriathlonData.StartTime.Year,TriathlonData.StartTime.Month,
											TriathlonData.StartTime.Day,TriathlonData.StartTime.Hour,
											TriathlonData.StartTime.Minute);
    LCD_SetNumber(str,&word);
	
	//�ָ���
    word.x_axis = 70;
    LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,LCD_AUQAMARIN,0,0,LCD_FILL_ENABLE);
	
	//��ʱ��
	charlen = strlen(SportDetailStrs[SetValue.Language][27]);
	word.x_axis = 100;
    word.y_axis = 95 - charlen*8;
	word.kerning = 0;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][27],&word);
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
	
	//��Ӿ���
	charlen = strlen(SportDetailStrs[SetValue.Language][35]);
	word.x_axis += 50;
    word.y_axis = 95 - charlen*8;
	word.kerning = 0;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][35],&word);
	word.y_axis = 98;
    LCD_SetString(":",&word);
	
	word.y_axis = 110;
    word.kerning = 1;
    memset(str,0,sizeof(str));
    sprintf(str,"%d",TriathlonData.SwimmingDistance/100);
    LCD_SetNumber(str,&word);
	
    word.y_axis = 160;
    word.kerning = 0;
    LCD_SetString("m",&word);
	
	//��Ӿʱ��
	second = TriathlonData.SwimmingTime;
	word.x_axis += 24;
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
	
	//�·���ʾ
	gui_sport_page_prompt(SPORT_PAGE_DOWN);
}

void gui_sport_triathlon_detail_1(uint8_t x, uint8_t forecolor)
{
	SetWord_t word = {0};
	char str[20] = {0};
	uint8_t charlen;
	uint32_t second;
	
	//����ʱ��
	second = TriathlonData.FirstWaitTime;

	//��һת��
	charlen = strlen(SportDetailStrs[SetValue.Language][29]);
	word.x_axis = x;
    word.y_axis = 95 - charlen*8;
	word.kerning = 0;
	word.forecolor = forecolor;
	word.bckgrndcolor = LCD_NONE;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][29],&word);
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
}

void gui_sport_triathlon_detail_2(uint8_t x, uint8_t forecolor)
{
	SetWord_t word = {0};
	char str[20] = {0};
	uint8_t charlen;
	
    //����
	charlen = strlen(SportStrs[SetValue.Language][Cycling]);
	word.x_axis = x;
    word.y_axis = 95 - charlen*8;
	word.kerning = 0;
	word.forecolor = forecolor;
	word.bckgrndcolor = LCD_NONE;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString((char *)SportStrs[SetValue.Language][Cycling],&word);
	word.y_axis = 98;
    LCD_SetString(":",&word);
	
	word.y_axis = 110;
    word.kerning = 1;
    memset(str,0,sizeof(str));
	#if defined (SPRINTF_FLOAT_TO_INT)
	sprintf(str,"%d.%02d",TriathlonData.CyclingDistance/100000,TriathlonData.CyclingDistance/1000%100);
	#else
	sprintf(str,"%0.2f",TriathlonData.CyclingDistance/100000.f);	//����װ��Ϊǧ�ף�������2λС��
	#endif
    LCD_SetNumber(str,&word);
	
    word.y_axis = 160;
    word.kerning = 0;
    LCD_SetString("km",&word);
}

void gui_sport_triathlon_detail_3(uint8_t x, uint8_t forecolor)
{
	SetWord_t word = {0};
	char str[20] = {0};
	uint32_t second;
	
	//����ʱ��
	second = TriathlonData.CyclingTime;
	
	//����ʱ��
	word.x_axis = x;
	word.y_axis = 110;
    word.kerning = 1;
	word.forecolor = forecolor;
	word.bckgrndcolor = LCD_NONE;
	word.size = LCD_FONT_16_SIZE;
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
}


void gui_sport_triathlon_detail_4(uint8_t x, uint8_t forecolor)
{
	SetWord_t word = {0};
	char str[20] = {0};
	uint8_t charlen;
	uint32_t second;
	
	//����ʱ��
	second = TriathlonData.SecondWaitTime;

	//�ڶ�ת��
	charlen = strlen(SportDetailStrs[SetValue.Language][30]);
	word.x_axis = x;
    word.y_axis = 95 - charlen*8;
	word.kerning = 0;
	word.forecolor = forecolor;
	word.bckgrndcolor = LCD_NONE;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][30],&word);
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
}

void gui_sport_triathlon_detail_5(uint8_t x, uint8_t forecolor)
{
	SetWord_t word = {0};
	char str[20] = {0};
	uint8_t charlen;
	
    //�ܲ�
	charlen = strlen(SportStrs[SetValue.Language][Run]);
	word.x_axis = x;
    word.y_axis = 95 - charlen*8;
	word.kerning = 0;
	word.forecolor = forecolor;
	word.bckgrndcolor = LCD_NONE;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString((char *)SportStrs[SetValue.Language][Run],&word);
	word.y_axis = 98;
    LCD_SetString(":",&word);
	
	word.y_axis = 110;
    word.kerning = 1;
    memset(str,0,sizeof(str));
	#if defined (SPRINTF_FLOAT_TO_INT)
	sprintf(str,"%d.%02d",TriathlonData.RunningDistance/100000,TriathlonData.RunningDistance/1000%100);
	#else
	sprintf(str,"%0.2f",TriathlonData.RunningDistance/100000.f);	//����װ��Ϊǧ�ף�������2λС��
	#endif
    LCD_SetNumber(str,&word);
	
    word.y_axis = 160;
    word.kerning = 0;
    LCD_SetString("km",&word);
}

void gui_sport_triathlon_detail_6(uint8_t x, uint8_t forecolor)
{
	SetWord_t word = {0};
	char str[20] = {0};
	uint32_t second;
	
	//����ʱ��
	second = TriathlonData.RunningTime;
	
	//�ܲ�ʱ��
	word.x_axis = x;
	word.y_axis = 110;
    word.kerning = 1;
	word.forecolor = forecolor;
	word.bckgrndcolor = LCD_NONE;
	word.size = LCD_FONT_16_SIZE;
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
}
void gui_sport_triathlon_detail_7(uint8_t x, uint8_t forecolor)
{
	SetWord_t word = {0};
	char str[20] = {0};
	uint8_t charlen;

	//ƽ������
	charlen = strlen(SportDetailStrs[SetValue.Language][5]);
    word.x_axis = x;
    word.y_axis = 95 - charlen*8;
	word.kerning = 0;
	word.forecolor = forecolor;
	word.bckgrndcolor = LCD_NONE;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][5],&word);
	word.y_axis = 98;
    LCD_SetString(":",&word);
	
	word.y_axis = 110;
    word.kerning = 1;
    memset(str,0,sizeof(str));
    sprintf(str,"%d",TriathlonData.HeartAvg);
    LCD_SetNumber(str,&word);
	
    word.y_axis = 160;
    word.kerning = 0;
    LCD_SetString("bpm",&word);
}

void gui_sport_triathlon_detail_8(uint8_t x, uint8_t forecolor)
{
	SetWord_t word = {0};
	char str[20] = {0};
	uint8_t charlen;
	
	//�������
	charlen = strlen(SportDetailStrs[SetValue.Language][6]);
    word.x_axis = x;
    word.y_axis = 95 - charlen*8;
	word.kerning = 0;
	word.forecolor = forecolor;
	word.bckgrndcolor = LCD_NONE;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][6],&word);
	word.y_axis = 98;
    LCD_SetString(":",&word);
	
	word.y_axis = 110;
    word.kerning = 1;
    memset(str,0,sizeof(str));
    sprintf(str,"%d",TriathlonData.HeartMax);
    LCD_SetNumber(str,&word);
	
    word.y_axis = 160;
    word.kerning = 0;
    LCD_SetString("bpm",&word);
}

//�˶��켣����
void gui_sport_triathlon_detail_track_paint(uint8_t backgroundcolor)
{
	SetWord_t word = {0};
	
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
	
	//�켣
	word.x_axis = 25;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	//	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][12],&word);

	//����켣��ʾ
	DrawSaveTrack();
}

void gui_sport_triathlon_detail_paint(void)
{
	uint8_t x = 50;
	uint8_t pageMax = TriathlonData.FininshNum > 2 ? 2 : TriathlonData.FininshNum;
	
	if(m_gui_sport_triathlon_pause_index == 0)
	{
		gui_sport_triathlon_detail_1_paint(LCD_BLACK);
	}
	else if(m_gui_sport_triathlon_pause_index > pageMax)
	{
		gui_sport_triathlon_detail_track_paint(LCD_BLACK);
	}
	else
	{
		//���ñ�����ɫ
		LCD_SetBackgroundColor(LCD_BLACK);
		
		//�Ϸ���ʾ
		gui_sport_page_prompt(SPORT_PAGE_UP);
		
		if(TriathlonData.FininshNum == 1)
		{
			gui_sport_triathlon_detail_7(x, LCD_WHITE);
			x += 24;

			gui_sport_triathlon_detail_8(x, LCD_WHITE);
		}
		else if(TriathlonData.FininshNum == 2)
		{
			if(m_gui_sport_triathlon_pause_index == 1)
			{
				gui_sport_triathlon_detail_1(x, LCD_WHITE);
				x += 50;
				
				gui_sport_triathlon_detail_2(x, LCD_WHITE);
				x += 24;
				
				gui_sport_triathlon_detail_3(x, LCD_WHITE);
				x += 50;
				
				gui_sport_triathlon_detail_7(x, LCD_WHITE);
			}
			else
			{
				gui_sport_triathlon_detail_8(x, LCD_WHITE);
			}
		}
		else
		{
			if(m_gui_sport_triathlon_pause_index == 1)
			{
				gui_sport_triathlon_detail_1(x, LCD_WHITE);
				x += 50;
				
				gui_sport_triathlon_detail_2(x, LCD_WHITE);
				x += 24;
				
				gui_sport_triathlon_detail_3(x, LCD_WHITE);
				x += 50;
				
				gui_sport_triathlon_detail_4(x, LCD_WHITE);
			}
			else
			{
				gui_sport_triathlon_detail_5(x, LCD_WHITE);
				x += 24;
				
				gui_sport_triathlon_detail_6(x, LCD_WHITE);
				x += 50;
				
				gui_sport_triathlon_detail_7(x, LCD_WHITE);
				x += 24;
				
				gui_sport_triathlon_detail_8(x, LCD_WHITE);
			}
		}
		
		//�·���ʾ
		gui_sport_page_prompt(SPORT_PAGE_DOWN);
	}
}

//������������˶������󣬼���ʱ���;���
void gui_sport_triathlon_data_get(void)
{
	uint16_t heartAvg;
	
	switch (m_gui_sport_triathlon_type)
	{
		case DISPLAY_SCREEN_TRIATHLON_SWIMMING:
			//��Ӿʱ��
			TriathlonData.SwimmingTime = ActivityData.ActTime;
			
			//��Ӿ����
			TriathlonData.SwimmingDistance = ActivityData.ActivityDetails.SwimmingDetail.Distance;
		
			//�������
			TriathlonData.HeartMax = ActivityData.ActivityDetails.SwimmingDetail.MaxHeart;
		
			//ƽ������
			TriathlonData.HeartAvg = ActivityData.ActivityDetails.SwimmingDetail.AvgHeart;
			break;
		case DISPLAY_SCREEN_TRIATHLON_FIRST_CHANGE:
			//�ѿ�ʼ�����˶�
			if(TriathlonData.FininshNum >= 2)
			{
				//��һ��ת��ʱ��
				TriathlonData.FirstWaitTime = ActivityData.ActTime;
			}
			break;
		case DISPLAY_SCREEN_TRIATHLON_CYCLING:
			//����ʱ��
			TriathlonData.CyclingTime = ActivityData.ActTime;
			
			//���о���
			TriathlonData.CyclingDistance = ActivityData.ActivityDetails.CyclingDetail.Distance;
		
			//�������
			if(ActivityData.ActivityDetails.CyclingDetail.MaxHeart > TriathlonData.HeartMax)
			{
				TriathlonData.HeartMax = ActivityData.ActivityDetails.CyclingDetail.MaxHeart;
			}
		
			//ƽ������
			heartAvg = TriathlonData.HeartAvg;
			if((heartAvg == 0) || (ActivityData.ActivityDetails.CyclingDetail.AvgHeart == 0))
			{
				TriathlonData.HeartAvg = heartAvg + ActivityData.ActivityDetails.CyclingDetail.AvgHeart;
			}
			else
			{
				TriathlonData.HeartAvg = (heartAvg + ActivityData.ActivityDetails.CyclingDetail.AvgHeart)/2;
			}
			break;
		case DISPLAY_SCREEN_TRIATHLON_SECOND_CHANGE:
			//�ѿ�ʼ�ܲ��˶�
			if(TriathlonData.FininshNum >= 3)
			{
				//�ڶ���ת��ʱ��
				TriathlonData.SecondWaitTime = ActivityData.ActTime;
			}
			break;
		default:
			//�ܲ�ʱ��
			TriathlonData.RunningTime = ActivityData.ActTime;
			
			//�ܲ�����
			TriathlonData.RunningDistance = ActivityData.ActivityDetails.RunningDetail.Distance;
		
			//�������
			if(ActivityData.ActivityDetails.RunningDetail.MaxHeart > TriathlonData.HeartMax)
			{
				TriathlonData.HeartMax = ActivityData.ActivityDetails.RunningDetail.MaxHeart;
			}
		
			//ƽ������
			heartAvg = TriathlonData.HeartAvg;
			if((heartAvg == 0) || (ActivityData.ActivityDetails.RunningDetail.AvgHeart == 0))
			{
				TriathlonData.HeartAvg = heartAvg + ActivityData.ActivityDetails.RunningDetail.AvgHeart;
			}
			else
			{
				TriathlonData.HeartAvg = (heartAvg + ActivityData.ActivityDetails.RunningDetail.AvgHeart)/2;
			}
			break;
	}
}

//��������׼�����水���¼�
void gui_sport_triathlon_ready_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
			//��ʼ��Ӿ�˶�
			g_sport_status = SPORT_STATUS_START;
			
			//��ʼ������
			memset(&TriathlonData, 0, sizeof(TriathlonDetailStr));
			TriathlonData.FininshNum = 1;
			gui_sport_triathlon_status_set(0);
			
			ScreenState = DISPLAY_SCREEN_TRIATHLON_SWIMMING;
			m_gui_sport_triathlon_type = DISPLAY_SCREEN_TRIATHLON_SWIMMING;
			m_gui_sport_triathlon_index = GUI_SPORT_TRIATHLON_SWIMMING_SCREEN_1;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			
			//����ʾ
			gui_sport_motor_prompt();
		
			//�����˶�����
			StartSport();
			
			//�����������ʼʱ��
			memcpy(&TriathlonData.StartTime, &ActivityData.Act_Start_Time, sizeof(rtc_time_t));
			break;
		case KEY_BACK:
//			if(Get_Curr_Home_Index() == DISPLAY_SCREEN_TRAINPLAN_MARATHON)
//			{
//				//����ѵ���ƻ�����
//				ScreenState = DISPLAY_SCREEN_TRAINPLAN_MARATHON;
//			}
//			else
			{
				//�����˶�ѡ��˵�����
				ScreenState = DISPLAY_SCREEN_SPORT;
			}
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		
			//�ر�GPS����
			ReadySportBack();
			break;
		default:
			break;
	}
}

//����������Ӿ����
void gui_sport_triathlon_swimming_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
			if(g_sport_status == SPORT_STATUS_START)
			{
				//��ͣ�˶���������ͣ�˵�����
				g_sport_status = SPORT_STATUS_PAUSE;
				Set_PauseSta(true);
				
				ScreenState = DISPLAY_SCREEN_TRIATHLON_PAUSE;
				m_gui_sport_triathlon_pause_index = GUI_SPORT_TRIATHLON_PAUSE_CONTINUE;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				
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
				
				//����ʾ
				gui_sport_motor_prompt();
			}
			break;
		case KEY_UP:
		case KEY_DOWN:
			#if !defined WATCH_COM_SPORT
			//��ҳ�鿴���н���
			if(m_gui_sport_triathlon_index == GUI_SPORT_TRIATHLON_SWIMMING_SCREEN_1)
			{
				m_gui_sport_triathlon_index = GUI_SPORT_TRIATHLON_SWIMMING_SCREEN_2;
			}
			else
			{
				m_gui_sport_triathlon_index = GUI_SPORT_TRIATHLON_SWIMMING_SCREEN_1;
			}
			
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			#endif 
			break;
		case KEY_BACK:
			if(g_sport_status == SPORT_STATUS_START)
			{
				//������Ӿ�˶�������������
				g_sport_status = SPORT_STATUS_START;
				sport_triathlon_change_start();
				
				//���뻻�����
				ScreenState = DISPLAY_SCREEN_TRIATHLON_FIRST_CHANGE;
				m_gui_sport_triathlon_type = DISPLAY_SCREEN_TRIATHLON_FIRST_CHANGE;
				
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
			else if(g_sport_status == SPORT_STATUS_PAUSE)
			{
				//������ͣ�˵�����
				ScreenState = DISPLAY_SCREEN_TRIATHLON_PAUSE;
				m_gui_sport_triathlon_pause_index = GUI_SPORT_TRIATHLON_PAUSE_CONTINUE;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
			break;
		default:
			break;
	}
}

//���������һ�������
void gui_sport_triathlon_first_change_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
			if(g_sport_status == SPORT_STATUS_START)
			{
				//��ͣ����
				g_sport_status = SPORT_STATUS_PAUSE;
				Set_PauseSta(true);
				
				//������ͣ�˵�����
				ScreenState = DISPLAY_SCREEN_TRIATHLON_PAUSE;
				m_gui_sport_triathlon_pause_index = GUI_SPORT_TRIATHLON_PAUSE_CONTINUE;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				
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
				
				//����ʾ
				gui_sport_motor_prompt();
			}
			break;
		case KEY_BACK:
			if(g_sport_status == SPORT_STATUS_START)
			{
				//���滻������
				TriathlonData.FininshNum = 2;
				sport_triathlon_change_start();
				
				//��ʼ�����˶�
				ScreenState = DISPLAY_SCREEN_TRIATHLON_CYCLING_READY;
				m_gui_sport_triathlon_type = DISPLAY_SCREEN_TRIATHLON_CYCLING;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			
//				if(ScreenState < DISPLAY_SCREEN_NOTIFY)
				{
					//5S��������н���
					ScreenStateSave = DISPLAY_SCREEN_TRIATHLON_CYCLING;
				}							
				timer_notify_display_start(5000,1,false);
			}
			else if(g_sport_status == SPORT_STATUS_PAUSE)
			{
				//������ͣ�˵�����
				ScreenState = DISPLAY_SCREEN_TRIATHLON_PAUSE;
				m_gui_sport_triathlon_pause_index = GUI_SPORT_TRIATHLON_PAUSE_CONTINUE;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
			break;
		default:
			break;
	}
}

//������������׼������
void gui_sport_triathlon_cycling_ready_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
			//�رն�ʱ��
			timer_notify_display_stop(false);
		
			//��ͣ�����˶�
			g_sport_status = SPORT_STATUS_PAUSE;
			Set_PauseSta(true);
			
			//������ͣ�˵�����
			ScreenState = DISPLAY_SCREEN_TRIATHLON_PAUSE;
			m_gui_sport_triathlon_pause_index = GUI_SPORT_TRIATHLON_PAUSE_CONTINUE;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			
			//����ʾ
			gui_sport_motor_prompt();
			break;
		default:
			break;
	}
}

//�����������н���
void gui_sport_triathlon_cycling_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
			if(g_sport_status == SPORT_STATUS_START)
			{
				//��ͣ�˶���������ͣ�˵�����
				g_sport_status = SPORT_STATUS_PAUSE;
				Set_PauseSta(true);
				
				ScreenState = DISPLAY_SCREEN_TRIATHLON_PAUSE;
				m_gui_sport_triathlon_pause_index = GUI_SPORT_TRIATHLON_PAUSE_CONTINUE;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				
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
				
				//����ʾ
				gui_sport_motor_prompt();
			}
			break;
		case KEY_BACK:
			if(g_sport_status == SPORT_STATUS_START)
			{
				//���������˶�
				g_sport_status = SPORT_STATUS_START;
				sport_triathlon_change_start();
				
				//���뻻�����
				ScreenState = DISPLAY_SCREEN_TRIATHLON_SECOND_CHANGE;
				m_gui_sport_triathlon_type = DISPLAY_SCREEN_TRIATHLON_SECOND_CHANGE;
				
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
			else if(g_sport_status == SPORT_STATUS_PAUSE)
			{
				//������ͣ�˵�����
				ScreenState = DISPLAY_SCREEN_TRIATHLON_PAUSE;
				m_gui_sport_triathlon_pause_index = GUI_SPORT_TRIATHLON_PAUSE_CONTINUE;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
			break;
		default:
			break;
	}
}

//��������ڶ��������
void gui_sport_triathlon_second_change_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
			if(g_sport_status == SPORT_STATUS_START)
			{
				//��ͣ����
				g_sport_status = SPORT_STATUS_PAUSE;
				Set_PauseSta(true);
				
				//������ͣ�˵�����
				ScreenState = DISPLAY_SCREEN_TRIATHLON_PAUSE;
				m_gui_sport_triathlon_pause_index = GUI_SPORT_TRIATHLON_PAUSE_CONTINUE;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				
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
				
				//����ʾ
				gui_sport_motor_prompt();
			}
			break;
		case KEY_BACK:
			//��ʼ�ܲ��˶�
			if(g_sport_status == SPORT_STATUS_START)
			{
				//���滻������
				TriathlonData.FininshNum = 3;
				sport_triathlon_change_start();
				
				ScreenState = DISPLAY_SCREEN_TRIATHLON_RUNNING_READY;
				m_gui_sport_triathlon_type = DISPLAY_SCREEN_TRIATHLON_RUNNING;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				
//				if(ScreenState < DISPLAY_SCREEN_NOTIFY)
				{
					//5S��������н���
					ScreenStateSave = DISPLAY_SCREEN_TRIATHLON_RUNNING;
				}							
				timer_notify_display_start(5000,1,false);
			}
			else if(g_sport_status == SPORT_STATUS_PAUSE)
			{
				//������ͣ�˵�����
				ScreenState = DISPLAY_SCREEN_TRIATHLON_PAUSE;
				m_gui_sport_triathlon_pause_index = GUI_SPORT_TRIATHLON_PAUSE_CONTINUE;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
			break;
		default:
			break;
	}
}

//���������ܲ�׼������
void gui_sport_triathlon_running_ready_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
			//�رն�ʱ��
			timer_notify_display_stop(false);
		
			//��ͣ�ܲ��˶�
			g_sport_status = SPORT_STATUS_PAUSE;
			Set_PauseSta(true);
			
			//������ͣ�˵�����
			ScreenState = DISPLAY_SCREEN_TRIATHLON_PAUSE;
			m_gui_sport_triathlon_pause_index = GUI_SPORT_TRIATHLON_PAUSE_CONTINUE;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			
			//����ʾ
			gui_sport_motor_prompt();
			break;
		default:
			break;
	}
}

//���������ܲ�����
void gui_sport_triathlon_running_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
			if(g_sport_status == SPORT_STATUS_START)
			{
				//��ͣ�˶���������ͣ�˵�����
				g_sport_status = SPORT_STATUS_PAUSE;
				Set_PauseSta(true);
				
				ScreenState = DISPLAY_SCREEN_TRIATHLON_PAUSE;
				m_gui_sport_triathlon_pause_index = GUI_SPORT_TRIATHLON_PAUSE_CONTINUE;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				
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
				
				//����ʾ
				gui_sport_motor_prompt();
			}
			break;
		case KEY_BACK:
			if(g_sport_status == SPORT_STATUS_START)
			{
				//�����ܲ��˶������������˶������
				g_sport_status = SPORT_STATUS_READY;
				Set_PauseSta(true);
				//�����˶����ݣ��˳��˶�����
				Set_SaveSta(1);
				CloseSportTask();
				
				ScreenState = DISPLAY_SCREEN_TRIATHLON_FINISH;
				m_gui_sport_triathlon_pause_index = 0;
				
				timer_progress_start(400);
				
				//������
				timer_notify_motor_start(1000,500,3,false,NOTIFY_MODE_SPORTS);
		        timer_notify_buzzer_start(1000,500,3,false,NOTIFY_MODE_SPORTS);
			}
			else if(g_sport_status == SPORT_STATUS_PAUSE)
			{
				//������ͣ�˵�����
				ScreenState = DISPLAY_SCREEN_TRIATHLON_PAUSE;
				m_gui_sport_triathlon_pause_index = GUI_SPORT_TRIATHLON_PAUSE_CONTINUE;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
			break;
		default:
			break;
	}
}

//��ͣ�˵����水���¼�
void gui_sport_triathlon_pause_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
			if(m_gui_sport_triathlon_pause_index == GUI_SPORT_TRIATHLON_PAUSE_CONTINUE)
			{
				//������ʱ����ʼ�˶�
				g_sport_status = SPORT_STATUS_START;
				Set_PauseSta(false);
				
				ScreenState = m_gui_sport_triathlon_type;
				
				//����ʾ
				gui_sport_motor_prompt();
			}
			else if(m_gui_sport_triathlon_pause_index == GUI_SPORT_TRIATHLON_PAUSE_SAVE)
			{
				//���������˶������
				g_sport_status = SPORT_STATUS_READY;
				
				if((m_gui_sport_triathlon_type == DISPLAY_SCREEN_TRIATHLON_FIRST_CHANGE)
					||(m_gui_sport_triathlon_type == DISPLAY_SCREEN_TRIATHLON_SECOND_CHANGE))
				{
					//�ڻ����н����˶����򲻱��滻���˶�����
					gui_sport_triathlon_status_set(1);
					Set_SaveSta(0);
				}
				else
				{
					//�����˶�����
					Set_SaveSta(1);
				}
				
				//�˳�����
				CloseSportTask();
				
//				//Ĭ��Ϊ�е����ѡ��
//				gui_sport_feel_init();
				
				ScreenState = DISPLAY_SCREEN_SPORT_SAVING;
				m_gui_sport_triathlon_pause_index = 0;
				
				extern volatile ScreenState_t Save_Detail_Index;
				Save_Detail_Index = DISPLAY_SCREEN_TRIATHLON_DETAIL;
				
				timer_progress_start(200);
				
				//����ʾ
				gui_sport_motor_prompt();
			}
			else
			{
				//�����˶�
				ScreenState = DISPLAY_SCREEN_TRIATHLON_CANCEL;
				
				m_gui_sport_triathlon_pause_index = GUI_SPORT_TRIATHLON_PAUSE_CANCEL_NO;
			}
			
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		case KEY_UP:
			if(m_gui_sport_triathlon_pause_index > GUI_SPORT_TRIATHLON_PAUSE_CONTINUE)
			{
				m_gui_sport_triathlon_pause_index--;
			}
			else
			{
				m_gui_sport_triathlon_pause_index = GUI_SPORT_TRIATHLON_PAUSE_CANCEL;
			}
			
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		    break;
		case KEY_DOWN:
			if(m_gui_sport_triathlon_pause_index < GUI_SPORT_TRIATHLON_PAUSE_CANCEL)
			{
				m_gui_sport_triathlon_pause_index++;
			}
			else
			{
				m_gui_sport_triathlon_pause_index = GUI_SPORT_TRIATHLON_PAUSE_CONTINUE;
			}
			
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		case KEY_BACK:
			//���������������
			ScreenState = m_gui_sport_triathlon_type;
		
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}

//�����˵����水���¼�
void gui_sport_triathlon_cancel_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
			if(m_gui_sport_triathlon_pause_index == GUI_SPORT_TRIATHLON_PAUSE_CANCEL_NO)
			{
				//ȡ�������˶��������ϼ��˵�����
				ScreenState = DISPLAY_SCREEN_TRIATHLON_PAUSE;
				m_gui_sport_triathlon_pause_index = GUI_SPORT_TRIATHLON_PAUSE_CANCEL;
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
			if(m_gui_sport_triathlon_pause_index == GUI_SPORT_TRIATHLON_PAUSE_CANCEL_NO)
			{
				m_gui_sport_triathlon_pause_index = GUI_SPORT_TRIATHLON_PAUSE_CANCEL_YES;
			}
			else
			{
				m_gui_sport_triathlon_pause_index = GUI_SPORT_TRIATHLON_PAUSE_CANCEL_NO;
			}

			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		    break;
		case KEY_BACK:
			//�����ϼ��˵�
			ScreenState = DISPLAY_SCREEN_TRIATHLON_PAUSE;
			m_gui_sport_triathlon_pause_index = GUI_SPORT_TRIATHLON_PAUSE_CANCEL;
			
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}

//��ϸ���ݽ��水���¼�
void gui_sport_triathlon_detail_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	uint8_t pageMax = TriathlonData.FininshNum > 2 ? 2 : TriathlonData.FininshNum;
	
	pageMax += 1;
	
	switch( Key_Value )
	{
		case KEY_UP:
			//�Ϸ�
			if(m_gui_sport_triathlon_pause_index > 0)
			{
				m_gui_sport_triathlon_pause_index--;
				
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
		    break;
		case KEY_DOWN:
			//�·�
			if(m_gui_sport_triathlon_pause_index < pageMax)
			{
				m_gui_sport_triathlon_pause_index++;
				
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

