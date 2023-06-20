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

#define	GUI_SPORT_TRIATHLON_SWIMMING_SCREEN_1	0	//铁人三项游泳界面1
#define	GUI_SPORT_TRIATHLON_SWIMMING_SCREEN_2	1	//铁人三项游泳界面2

#define	GUI_SPORT_TRIATHLON_PAUSE_CONTINUE		0	//继续
#define	GUI_SPORT_TRIATHLON_PAUSE_SAVE			1	//结束运动
#define	GUI_SPORT_TRIATHLON_PAUSE_CANCEL		2	//放弃

#define	GUI_SPORT_TRIATHLON_PAUSE_CANCEL_NO		0	//否
#define	GUI_SPORT_TRIATHLON_PAUSE_CANCEL_YES	1	//是

static uint8_t m_gui_sport_triathlon_index;			//铁人三项界面索引
uint8_t m_gui_sport_triathlon_pause_index;	//铁人三项暂停菜单界面索引
static ScreenState_t m_gui_sport_triathlon_type;	//是哪项铁人三项
static uint8_t m_gui_sport_triathlon_status;			//铁人三项放弃状态

TriathlonDetailStr TriathlonData;			//用于保存铁人三项数据

void gui_sport_triathlon_status_set(uint8_t status)
{
	m_gui_sport_triathlon_status = status;
}

uint8_t gui_sport_triathlon_status_get(void)
{
	return m_gui_sport_triathlon_status;
}

//铁人三项准备界面
void gui_sport_triathlon_ready_paint(void)
{
	SetWord_t word = {0};
	
	//显示黑色背景
	LCD_SetBackgroundColor(LCD_BLACK);
	
	//铁人三项游泳图标
	LCD_SetPicture(18, LCD_CENTER_JUSTIFIED, LCD_NONE, LCD_NONE, &Img_Sport[Swimming]);
	
	//游泳
	word.x_axis = 108;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][35], &word);
	
	//提示语
	word.x_axis += LCD_FONT_24_SIZE + 10;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString((char *)SportsPromptStrs[SetValue.Language][4], &word);
	
	//GPS状态
	gui_sport_gps_status(SPORT_GPS_LEFT1);
}

//里程
static void gui_sport_triathlon_swimming_1_distance(uint32_t distance)
{
	SetWord_t word = {0};
	char str[10] = {0};
	uint8_t len;
	
	//里程数据
	word.x_axis = 30;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_32_SIZE;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 1;
	memset(str,0,sizeof(str));
	sprintf(str,"%d",distance/100);	//厘米装换为米
	LCD_SetNumber(str,&word);
	
	len = strlen(str)*(Font_Number_32.width + 1) - 1;
	word.x_axis += (Font_Number_32.height - 16);
	word.y_axis = (LCD_PIXEL_ONELINE_MAX - len)/2 + len + 8;
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 0;
	LCD_SetString("m",&word);
}

//计时
static void gui_sport_triathlon_swimming_time(uint8_t x, uint8_t y, uint8_t forecolor, 
												uint8_t hour, uint8_t min, uint8_t sec)
{
	SetWord_t word = {0};
	char str[10];
	
	memset(str,0,sizeof(str));
	if(hour > 0)
	{
		//时分秒
		sprintf(str,"%d:%02d:%02d",hour, min ,sec);
	}
	else
	{
		//分秒
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

//心率
static void gui_sport_triathlon_swimming_1_heart(uint8_t heart)
{
	SetWord_t word = {0};
    char str[10];
	
	//心率图标
    word.x_axis = 97;
	word.y_axis = 214 - (Img_heartrate_18X16.width/2);
	LCD_SetPicture(word.x_axis,word.y_axis,LCD_NONE,LCD_NONE,&Img_heartrate_18X16);
	
	//心率数据
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

//泳速
static void gui_sport_triathlon_swimming_1_speed(uint16_t speed)
{
	SetWord_t word = {0};
	char str[10] = {0};
	uint8_t len;
	
	//速度数据
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

//铁人三项游泳界面1
static void gui_sport_triathlon_swimming_1_paint(void)
{
#if defined WATCH_COM_SPORT 
	//显示黑色背景
	LCD_SetBackgroundColor(LCD_BLACK);

	//里程
	gui_sport_triathlon_swimming_1_distance(ActivityData.ActivityDetails.SwimmingDetail.Distance);
	
	//分割线
	LCD_SetRectangle(80, 1, 0, LCD_LINE_CNT_MAX ,LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);

	//泳速
	gui_sport_triathlon_swimming_1_speed(ActivityData.ActivityDetails.SwimmingDetail.Speed);

	//分割线
	LCD_SetRectangle(160, 1, 0, LCD_LINE_CNT_MAX, LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);

	//计时
	gui_sport_triathlon_swimming_time(170, LCD_CENTER_ROW, LCD_WHITE, ActivityData.Act_Stop_Time.Hour, 
										ActivityData.Act_Stop_Time.Minute, ActivityData.Act_Stop_Time.Second);

	//GPS状态
	gui_sport_gps_status(SPORT_GPS_LEFT1);

#else
	//显示黑色背景
	LCD_SetBackgroundColor(LCD_BLACK);
	
	//圆点索引
	gui_sport_index_circle_paint(m_gui_sport_triathlon_index, 2, 0);
	
	//里程
	gui_sport_triathlon_swimming_1_distance(ActivityData.ActivityDetails.SwimmingDetail.Distance);
	
	//分割线
	LCD_SetRectangle(80, 1, 0, LCD_LINE_CNT_MAX ,LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);
	
	//计时
	gui_sport_triathlon_swimming_time(97, LCD_CENTER_ROW, LCD_WHITE, ActivityData.Act_Stop_Time.Hour, 
										ActivityData.Act_Stop_Time.Minute, ActivityData.Act_Stop_Time.Second);
	
	//心率
	gui_sport_triathlon_swimming_1_heart(Get_Sport_Heart());
	
	//分割线
	LCD_SetRectangle(160, 1, 0, LCD_LINE_CNT_MAX, LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);
	
	//泳速
	gui_sport_triathlon_swimming_1_speed(ActivityData.ActivityDetails.SwimmingDetail.Speed);
	
	//时间和电量
	gui_sport_realtime_battery(0);
	
	//GPS状态
	gui_sport_gps_status(SPORT_GPS_LEFT1);
#endif
}

//划数
static void gui_sport_triathlon_swimming_2_stroke(uint32_t stroke)
{
	SetWord_t word = {0};
	char str[10] = {0};
	
	//划数
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

//划频
static void gui_sport_triathlon_swimming_2_frequency(uint16_t frequency)
{
	SetWord_t word = {0};
	char str[10] = {0};
	uint8_t len;
	
	//划频
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
	
	//次/min
	len = strlen(str)*(Font_Number_32.width + 1) - 1;
	word.x_axis += (Font_Number_32.height - 16);
	word.y_axis = (LCD_PIXEL_ONELINE_MAX - len)/2 + len + 8;
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 0;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][26],&word);
}

static void gui_sport_triathlon_swimming_2_paint(void)
{
	//显示黑色背景
	LCD_SetBackgroundColor(LCD_BLACK);
	
	//圆点索引
	gui_sport_index_circle_paint(m_gui_sport_triathlon_index, 2, 0);
	
	//划数
	gui_sport_triathlon_swimming_2_stroke(ActivityData.ActivityDetails.SwimmingDetail.Strokes);
	
	//分割线
	LCD_SetRectangle(120, 1, 0, LCD_LINE_CNT_MAX, LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);
	
	//划频
	gui_sport_triathlon_swimming_2_frequency(ActivityData.ActivityDetails.SwimmingDetail.Frequency);
	
	//时间和电量
	gui_sport_realtime_battery(0);
	
	//GPS状态
	gui_sport_gps_status(SPORT_GPS_LEFT3);
}

//铁人三项游泳界面
void gui_sport_triathlon_swimming_paint(void)
{
#if defined WATCH_COM_SPORT
	gui_sport_triathlon_swimming_1_paint();
#else
	if(m_gui_sport_triathlon_index == GUI_SPORT_TRIATHLON_SWIMMING_SCREEN_1)
	{
		//铁人三项游泳界面1
		gui_sport_triathlon_swimming_1_paint();
	}
	else
	{
		//铁人三项游泳界面2
		gui_sport_triathlon_swimming_2_paint();
	}
#endif
	if(g_sport_status == SPORT_STATUS_PAUSE)
	{
		//暂时增加红色圆圈，表示暂停状态
		LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_RED);
	}
}

//第一换项界面
void gui_sport_triathlon_first_change_paint(void)
{
	//总时长
	SetWord_t word = {0};
	uint32_t second;
	
	//计算总时长
	second = (ActivityData.Act_Stop_Time.Hour*3600) + (ActivityData.Act_Stop_Time.Minute*60) 
				+ ActivityData.Act_Stop_Time.Second + TriathlonData.SwimmingTime;
	
	//设置背景黑色
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
	
	//分割线
	LCD_SetRectangle(120, 1, 0, LCD_LINE_CNT_MAX, LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);
	
	//转换时长
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
	//时间和电量
	gui_sport_realtime_battery(0);
	#endif 
	
	if(g_sport_status == SPORT_STATUS_PAUSE)
	{
		//暂时增加红色圆圈，表示暂停状态
		LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_RED);
	}
	
	//GPS状态
	gui_sport_gps_status(SPORT_GPS_LEFT3);
}

//铁人三项骑行准备界面
void gui_sport_triathlon_cycling_ready_paint(void)
{
	SetWord_t word = {0};
	char str[10] = {0};
	uint32_t second;
	
	//计算时长
	second = TriathlonData.FirstWaitTime;
	
	//显示黑色背景
	LCD_SetBackgroundColor(LCD_BLACK);
	
	//铁人三项骑行图标
	LCD_SetPicture(18, LCD_CENTER_JUSTIFIED, LCD_NONE, LCD_NONE, &Img_Sport[Cycling]);
	
	//骑行
	word.x_axis = 108;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;
	LCD_SetString((char *)SportStrs[SetValue.Language][Cycling], &word);
	
	//转换时长
	memset(str,0,sizeof(str));
    if(second >= 3600)
	{
		//时分秒
		sprintf(str,"%d:%02d:%02d",second/3600,second/60%60,second%60);
	}
	else
	{
		//分秒
		sprintf(str,"%d:%02d",second/60%60,second%60);
	}
	
	word.x_axis += LCD_FONT_24_SIZE + 10;
	word.y_axis = (LCD_PIXEL_ONELINE_MAX - (strlen(str)*8)- (4*LCD_FONT_16_SIZE) - 4)/2;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][28], &word);
	
	word.y_axis += ((4*LCD_FONT_16_SIZE) + 4);
	LCD_SetString(str, &word);
	
	//GPS状态
	gui_sport_gps_status(SPORT_GPS_LEFT1);
}

//里程
static void gui_sport_triathlon_cycling_distance(uint32_t distance)
{
	SetWord_t word = {0};
	char str[10] = {0};
	uint8_t len;
	
	//里程数据
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
	sprintf(str,"%0.2f",distance/100000.f);	//厘米装换为千米，并保留2位小数
	#endif
	LCD_SetNumber(str,&word);
	
	len = strlen(str)*(Font_Number_32.width + 1) - 1;
	word.x_axis += (Font_Number_32.height - 16);
	word.y_axis = (LCD_PIXEL_ONELINE_MAX - len)/2 + len + 8;
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 0;
	LCD_SetString("km",&word);
}

//速度
static void gui_sport_triathlon_cycling_speed(uint16_t speed)
{
	SetWord_t word = {0};
	char str[10] = {0};
	uint8_t len;
	
	//速度数据
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
    sprintf(str,"%.1f",speed/10.f);		//速度保留一位小数点
    #endif
	
	LCD_SetNumber(str,&word);
	
	len = strlen(str)*(Font_Number_32.width + 1) - 1;
	word.x_axis += (Font_Number_32.height - 16);
	word.y_axis = (LCD_PIXEL_ONELINE_MAX - len)/2 + len + 8;
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 0;
	LCD_SetString("km/h",&word);
}

//铁人三项骑行界面
void gui_sport_triathlon_cycling_paint(void)
{
	//显示黑色背景
	LCD_SetBackgroundColor(LCD_BLACK);
	
	//里程
	gui_sport_triathlon_cycling_distance(ActivityData.ActivityDetails.CyclingDetail.Distance);
	
	//分割线
	LCD_SetRectangle(80, 1, 0, LCD_LINE_CNT_MAX ,LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);
	
	//计时
	gui_sport_triathlon_swimming_time(97, LCD_CENTER_ROW, LCD_WHITE, ActivityData.Act_Stop_Time.Hour, 
										ActivityData.Act_Stop_Time.Minute, ActivityData.Act_Stop_Time.Second);
	#if !defined WATCH_COM_SPORT
	//心率
	gui_sport_triathlon_swimming_1_heart(Get_Sport_Heart());
	#endif
	//分割线
	LCD_SetRectangle(160, 1, 0, LCD_LINE_CNT_MAX, LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);
	
	//速度
	gui_sport_triathlon_cycling_speed(ActivityData.ActivityDetails.CyclingDetail.Speed);
	#if !defined WATCH_COM_SPORT
	//时间和电量
	gui_sport_realtime_battery(0);
	#endif 
	//GPS状态
	gui_sport_gps_status(SPORT_GPS_LEFT1);
	
	if(g_sport_status == SPORT_STATUS_PAUSE)
	{
		//暂时增加红色圆圈，表示暂停状态
		LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_RED);
	}
}

//第二换项界面
void gui_sport_triathlon_second_change_paint(void)
{
	//总时长
	SetWord_t word = {0};
	uint32_t second;
	
	//计算总时长
	second = (ActivityData.Act_Stop_Time.Hour*3600) + (ActivityData.Act_Stop_Time.Minute*60) 
				+ ActivityData.Act_Stop_Time.Second + TriathlonData.SwimmingTime
				+ TriathlonData.FirstWaitTime + TriathlonData.CyclingTime;
	
	//设置背景黑色
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
	
	//分割线
	LCD_SetRectangle(120, 1, 0, LCD_LINE_CNT_MAX, LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);
	
	//转换时长
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
	//时间和电量
	gui_sport_realtime_battery(0);
	#endif
	if(g_sport_status == SPORT_STATUS_PAUSE)
	{
		//暂时增加红色圆圈，表示暂停状态
		LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_RED);
	}
	
	//GPS状态
	gui_sport_gps_status(SPORT_GPS_LEFT3);
}

//铁人三项跑步准备界面
void gui_sport_triathlon_running_ready_paint(void)
{
	SetWord_t word = {0};
	char str[10] = {0};
	uint32_t second;
	
	//计算时长
	second = TriathlonData.SecondWaitTime;
	
	//显示黑色背景
	LCD_SetBackgroundColor(LCD_BLACK);
	
	//铁人三项跑步图标
	LCD_SetPicture(18, LCD_CENTER_JUSTIFIED, LCD_NONE, LCD_NONE, &Img_Sport[Run]);
	
	//跑步
	word.x_axis = 108;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;
	LCD_SetString((char *)SportStrs[SetValue.Language][Run], &word);
	
	//转换时长
	memset(str,0,sizeof(str));
    if(second >= 3600)
	{
		//时分秒
		sprintf(str,"%d:%02d:%02d",second/3600,second/60%60,second%60);
	}
	else
	{
		//分秒
		sprintf(str,"%d:%02d",second/60%60,second%60);
	}
	
	word.x_axis += LCD_FONT_24_SIZE + 10;
	word.y_axis = (LCD_PIXEL_ONELINE_MAX - (strlen(str)*8)- (4*LCD_FONT_16_SIZE) - 4)/2;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][28], &word);
	
	word.y_axis += ((4*LCD_FONT_16_SIZE) + 4);
	LCD_SetString(str, &word);
	
	//GPS状态
	gui_sport_gps_status(SPORT_GPS_LEFT1);
}

//配速
static void gui_sport_triathlon_running_speed(uint16_t speed)
{
	SetWord_t word = {0};
	char str[10] = {0};
	uint8_t len;
	
	//配速数据
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

//铁人三项跑步界面
void gui_sport_triathlon_running_paint(void)
{
	//显示黑色背景
	LCD_SetBackgroundColor(LCD_BLACK);
	
	//里程
	gui_sport_triathlon_cycling_distance(ActivityData.ActivityDetails.RunningDetail.Distance);
	
	//分割线
	LCD_SetRectangle(80, 1, 0, LCD_LINE_CNT_MAX ,LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);
	#if defined WATCH_COM_SPORT
		gui_sport_triathlon_swimming_time(170, LCD_CENTER_ROW, LCD_WHITE, ActivityData.Act_Stop_Time.Hour, 
										ActivityData.Act_Stop_Time.Minute, ActivityData.Act_Stop_Time.Second);
	#else
	//计时
	gui_sport_triathlon_swimming_time(97, LCD_CENTER_ROW, LCD_WHITE, ActivityData.Act_Stop_Time.Hour, 
										ActivityData.Act_Stop_Time.Minute, ActivityData.Act_Stop_Time.Second);
	
	//心率
	gui_sport_triathlon_swimming_1_heart(Get_Sport_Heart());
	#endif
	
	//分割线
	LCD_SetRectangle(160, 1, 0, LCD_LINE_CNT_MAX, LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);
	
	//配速
	gui_sport_triathlon_running_speed(ActivityData.ActivityDetails.RunningDetail.Pace);
	#if !defined WATCH_COM_SPORT
	//时间和电量
	gui_sport_realtime_battery(0);
	#endif
	
	//GPS状态
	gui_sport_gps_status(SPORT_GPS_LEFT1);
	
	if(g_sport_status == SPORT_STATUS_PAUSE)
	{
		//暂时增加红色圆圈，表示暂停状态
		LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_RED);
	}
}

//铁人三项已完成界面
void gui_sport_triathlon_finish_paint(uint8_t endangle)
{
	SetWord_t word = {0};
	
	//设置背景黑色
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

//暂停计时
static void gui_sport_triathlon_pause_time(void)
{
	SetWord_t word = {0};
	char str[10] = {0};
	uint32_t second;

	//计算总时长
	second = PauseTime.Pause_Stop_Time.Hour*60*60 + PauseTime.Pause_Stop_Time.Minute*60 + PauseTime.Pause_Stop_Time.Second;
	
	if(second >= 6000)
	{
		//暂停时长不超过99:59
		second = 5999;
	}
	
	memset(str,0,sizeof(str));
    sprintf(str,"%d:%02d",second/60,second%60);
	
	//暂停时长
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

//运动时间
static void gui_sport_triathlon_time(void)
{
	SetWord_t word = {0};
	char str[10] = {0};
	uint32_t second;
	
	//计算总时长
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
		//时分秒
		sprintf(str,"%d:%02d:%02d",second/3600,second/60%60,second%60);
	}
	else
	{
		//分秒
		sprintf(str,"%d:%02d",second/60%60,second%60);
	}
	LCD_SetNumber(str, &word);
}

//暂停菜单选项
static void gui_sport_triathlon_pause_options(uint8_t index)
{
	SetWord_t word = {0};
	
	//菜单第一项
	word.x_axis = 133;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 0;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString((char *)SwimPauseStrs[SetValue.Language][index],&word);
	
	//菜单第二项
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

//暂停菜单界面
void gui_sport_triathlon_pause_paint(void)
{
	//上半部分为白色背景
	LCD_SetRectangle(0, 120, 0, LCD_PIXEL_ONELINE_MAX, LCD_WHITE, 0, 0, LCD_FILL_ENABLE);
	
	//暂停计时
	gui_sport_triathlon_pause_time();
	
	//运动计时
	gui_sport_triathlon_time();
	
	//下半部分为黑色背景
	LCD_SetRectangle(120, 120, 0, LCD_PIXEL_ONELINE_MAX, LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
	
	//选项
	gui_sport_triathlon_pause_options(m_gui_sport_triathlon_pause_index);
	
	//分割线
	LCD_SetRectangle(170, 1, 0, LCD_LINE_CNT_MAX, LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);
	
	//下翻提示
	gui_sport_page_prompt(SPORT_PAGE_DOWN);
	
	//红色圆圈
	LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_RED);
}

//放弃菜单选项
static void gui_sport_triathlon_cancel_options(uint8_t index)
{
	SetWord_t word = {0};
	
	//菜单第一项
	word.x_axis = 133;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 0;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString((char *)CancelStrs[SetValue.Language][index],&word);
	
	//菜单第二项
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

//放弃菜单界面
void gui_sport_triathlon_cancel_paint(void)
{
	SetWord_t word = {0};
	
	//上半部分为白色背景
	LCD_SetRectangle(0, 120, 0, LCD_PIXEL_ONELINE_MAX, LCD_WHITE, 0, 0, LCD_FILL_ENABLE);
	
	//放弃?
	word.x_axis = 48;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_BLACK;
	word.kerning = 0;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString((char *)RunPauseStrs[SetValue.Language][3],&word);
    word.y_axis = 104 + 32 + 2;
    LCD_SetString("?",&word);
	
	//下半部分为黑色背景
	LCD_SetRectangle(120, 120, 0, LCD_PIXEL_ONELINE_MAX, LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
	
	//选项
	gui_sport_triathlon_cancel_options(m_gui_sport_triathlon_pause_index);
	
	//分割线
	LCD_SetRectangle(170, 1, 0, LCD_LINE_CNT_MAX, LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);
	
	//下翻提示
	gui_sport_page_prompt(SPORT_PAGE_DOWN);
	
	//红色圆圈
	LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_RED);
}

//详细数据界面1
void gui_sport_triathlon_detail_1_paint(uint8_t backgroundcolor)
{
	SetWord_t word = {0};
	char str[20] = {0};
	uint32_t second;
	uint8_t charlen;
	
	//计算时长
	second = TriathlonData.SwimmingTime + TriathlonData.FirstWaitTime + TriathlonData.CyclingTime 
				+ TriathlonData.SecondWaitTime + TriathlonData.RunningTime;
	
	//设置背景黑色
    LCD_SetBackgroundColor(backgroundcolor);
	
	if (backgroundcolor == LCD_BLACK)
    {
        //黑底白字
        word.forecolor = LCD_WHITE;
    }
    else
    {
        //白底黑字
        word.forecolor = LCD_BLACK;
    }

	//时间
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
	
	//分割线
    word.x_axis = 70;
    LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,LCD_AUQAMARIN,0,0,LCD_FILL_ENABLE);
	
	//总时长
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
		//时分秒
		sprintf(str,"%d:%02d:%02d",second/3600,second/60%60,second%60);
	}
	else
	{
		//分秒
		sprintf(str,"%d:%02d",second/60%60,second%60);
	}
    LCD_SetNumber(str,&word);
	
	//游泳里程
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
	
	//游泳时长
	second = TriathlonData.SwimmingTime;
	word.x_axis += 24;
	word.y_axis = 110;
    word.kerning = 1;
    memset(str,0,sizeof(str));
	if(second >= 3600)
	{
		//时分秒
		sprintf(str,"%d:%02d:%02d",second/3600,second/60%60,second%60);
	}
	else
	{
		//分秒
		sprintf(str,"%d:%02d",second/60%60,second%60);
	}
    LCD_SetNumber(str,&word);
	
	//下翻提示
	gui_sport_page_prompt(SPORT_PAGE_DOWN);
}

void gui_sport_triathlon_detail_1(uint8_t x, uint8_t forecolor)
{
	SetWord_t word = {0};
	char str[20] = {0};
	uint8_t charlen;
	uint32_t second;
	
	//计算时长
	second = TriathlonData.FirstWaitTime;

	//第一转换
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
		//时分秒
		sprintf(str,"%d:%02d:%02d",second/3600,second/60%60,second%60);
	}
	else
	{
		//分秒
		sprintf(str,"%d:%02d",second/60%60,second%60);
	}
    LCD_SetNumber(str,&word);
}

void gui_sport_triathlon_detail_2(uint8_t x, uint8_t forecolor)
{
	SetWord_t word = {0};
	char str[20] = {0};
	uint8_t charlen;
	
    //骑行
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
	sprintf(str,"%0.2f",TriathlonData.CyclingDistance/100000.f);	//厘米装换为千米，并保留2位小数
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
	
	//计算时长
	second = TriathlonData.CyclingTime;
	
	//骑行时长
	word.x_axis = x;
	word.y_axis = 110;
    word.kerning = 1;
	word.forecolor = forecolor;
	word.bckgrndcolor = LCD_NONE;
	word.size = LCD_FONT_16_SIZE;
    memset(str,0,sizeof(str));
    if(second >= 3600)
	{
		//时分秒
		sprintf(str,"%d:%02d:%02d",second/3600,second/60%60,second%60);
	}
	else
	{
		//分秒
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
	
	//计算时长
	second = TriathlonData.SecondWaitTime;

	//第二转换
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
		//时分秒
		sprintf(str,"%d:%02d:%02d",second/3600,second/60%60,second%60);
	}
	else
	{
		//分秒
		sprintf(str,"%d:%02d",second/60%60,second%60);
	}
    LCD_SetNumber(str,&word);
}

void gui_sport_triathlon_detail_5(uint8_t x, uint8_t forecolor)
{
	SetWord_t word = {0};
	char str[20] = {0};
	uint8_t charlen;
	
    //跑步
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
	sprintf(str,"%0.2f",TriathlonData.RunningDistance/100000.f);	//厘米装换为千米，并保留2位小数
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
	
	//计算时长
	second = TriathlonData.RunningTime;
	
	//跑步时长
	word.x_axis = x;
	word.y_axis = 110;
    word.kerning = 1;
	word.forecolor = forecolor;
	word.bckgrndcolor = LCD_NONE;
	word.size = LCD_FONT_16_SIZE;
    memset(str,0,sizeof(str));
	if(second >= 3600)
	{
		//时分秒
		sprintf(str,"%d:%02d:%02d",second/3600,second/60%60,second%60);
	}
	else
	{
		//分秒
		sprintf(str,"%d:%02d",second/60%60,second%60);
	}
    LCD_SetNumber(str,&word);
}
void gui_sport_triathlon_detail_7(uint8_t x, uint8_t forecolor)
{
	SetWord_t word = {0};
	char str[20] = {0};
	uint8_t charlen;

	//平均心率
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
	
	//最大心率
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

//运动轨迹界面
void gui_sport_triathlon_detail_track_paint(uint8_t backgroundcolor)
{
	SetWord_t word = {0};
	
	//设置背景黑色
	LCD_SetBackgroundColor(backgroundcolor);
	if (backgroundcolor == LCD_BLACK)
	{
		//黑底白字
		word.forecolor = LCD_WHITE;
	}
	else
	{
		//白底黑字
		word.forecolor = LCD_BLACK;
	}
	//上翻提示
	gui_sport_page_prompt(SPORT_PAGE_UP);
	
	//轨迹
	word.x_axis = 25;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	//	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][12],&word);

	//保存轨迹显示
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
		//设置背景黑色
		LCD_SetBackgroundColor(LCD_BLACK);
		
		//上翻提示
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
		
		//下翻提示
		gui_sport_page_prompt(SPORT_PAGE_DOWN);
	}
}

//铁人三项各项运动结束后，计算时长和距离
void gui_sport_triathlon_data_get(void)
{
	uint16_t heartAvg;
	
	switch (m_gui_sport_triathlon_type)
	{
		case DISPLAY_SCREEN_TRIATHLON_SWIMMING:
			//游泳时长
			TriathlonData.SwimmingTime = ActivityData.ActTime;
			
			//游泳距离
			TriathlonData.SwimmingDistance = ActivityData.ActivityDetails.SwimmingDetail.Distance;
		
			//最大心率
			TriathlonData.HeartMax = ActivityData.ActivityDetails.SwimmingDetail.MaxHeart;
		
			//平均心率
			TriathlonData.HeartAvg = ActivityData.ActivityDetails.SwimmingDetail.AvgHeart;
			break;
		case DISPLAY_SCREEN_TRIATHLON_FIRST_CHANGE:
			//已开始骑行运动
			if(TriathlonData.FininshNum >= 2)
			{
				//第一次转项时长
				TriathlonData.FirstWaitTime = ActivityData.ActTime;
			}
			break;
		case DISPLAY_SCREEN_TRIATHLON_CYCLING:
			//骑行时长
			TriathlonData.CyclingTime = ActivityData.ActTime;
			
			//骑行距离
			TriathlonData.CyclingDistance = ActivityData.ActivityDetails.CyclingDetail.Distance;
		
			//最大心率
			if(ActivityData.ActivityDetails.CyclingDetail.MaxHeart > TriathlonData.HeartMax)
			{
				TriathlonData.HeartMax = ActivityData.ActivityDetails.CyclingDetail.MaxHeart;
			}
		
			//平均心率
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
			//已开始跑步运动
			if(TriathlonData.FininshNum >= 3)
			{
				//第二次转项时长
				TriathlonData.SecondWaitTime = ActivityData.ActTime;
			}
			break;
		default:
			//跑步时长
			TriathlonData.RunningTime = ActivityData.ActTime;
			
			//跑步距离
			TriathlonData.RunningDistance = ActivityData.ActivityDetails.RunningDetail.Distance;
		
			//最大心率
			if(ActivityData.ActivityDetails.RunningDetail.MaxHeart > TriathlonData.HeartMax)
			{
				TriathlonData.HeartMax = ActivityData.ActivityDetails.RunningDetail.MaxHeart;
			}
		
			//平均心率
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

//铁人三项准备界面按键事件
void gui_sport_triathlon_ready_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
			//开始游泳运动
			g_sport_status = SPORT_STATUS_START;
			
			//初始化数据
			memset(&TriathlonData, 0, sizeof(TriathlonDetailStr));
			TriathlonData.FininshNum = 1;
			gui_sport_triathlon_status_set(0);
			
			ScreenState = DISPLAY_SCREEN_TRIATHLON_SWIMMING;
			m_gui_sport_triathlon_type = DISPLAY_SCREEN_TRIATHLON_SWIMMING;
			m_gui_sport_triathlon_index = GUI_SPORT_TRIATHLON_SWIMMING_SCREEN_1;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			
			//振动提示
			gui_sport_motor_prompt();
		
			//创建运动任务
			StartSport();
			
			//保存铁人三项开始时间
			memcpy(&TriathlonData.StartTime, &ActivityData.Act_Start_Time, sizeof(rtc_time_t));
			break;
		case KEY_BACK:
//			if(Get_Curr_Home_Index() == DISPLAY_SCREEN_TRAINPLAN_MARATHON)
//			{
//				//返回训练计划界面
//				ScreenState = DISPLAY_SCREEN_TRAINPLAN_MARATHON;
//			}
//			else
			{
				//返回运动选项菜单界面
				ScreenState = DISPLAY_SCREEN_SPORT;
			}
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		
			//关闭GPS任务
			ReadySportBack();
			break;
		default:
			break;
	}
}

//铁人三项游泳界面
void gui_sport_triathlon_swimming_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
			if(g_sport_status == SPORT_STATUS_START)
			{
				//暂停运动，进入暂停菜单界面
				g_sport_status = SPORT_STATUS_PAUSE;
				Set_PauseSta(true);
				
				ScreenState = DISPLAY_SCREEN_TRIATHLON_PAUSE;
				m_gui_sport_triathlon_pause_index = GUI_SPORT_TRIATHLON_PAUSE_CONTINUE;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				
				//振动提示
				gui_sport_motor_prompt();
			}
			else if(g_sport_status == SPORT_STATUS_PAUSE)
			{
				//继续计时
				g_sport_status = SPORT_STATUS_START;
				Set_PauseSta(false);
				
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				
				//振动提示
				gui_sport_motor_prompt();
			}
			break;
		case KEY_UP:
		case KEY_DOWN:
			#if !defined WATCH_COM_SPORT
			//翻页查看骑行界面
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
				//结束游泳运动，并保存数据
				g_sport_status = SPORT_STATUS_START;
				sport_triathlon_change_start();
				
				//进入换项界面
				ScreenState = DISPLAY_SCREEN_TRIATHLON_FIRST_CHANGE;
				m_gui_sport_triathlon_type = DISPLAY_SCREEN_TRIATHLON_FIRST_CHANGE;
				
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
			else if(g_sport_status == SPORT_STATUS_PAUSE)
			{
				//返回暂停菜单界面
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

//铁人三项第一换项界面
void gui_sport_triathlon_first_change_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
			if(g_sport_status == SPORT_STATUS_START)
			{
				//暂停换项
				g_sport_status = SPORT_STATUS_PAUSE;
				Set_PauseSta(true);
				
				//进入暂停菜单界面
				ScreenState = DISPLAY_SCREEN_TRIATHLON_PAUSE;
				m_gui_sport_triathlon_pause_index = GUI_SPORT_TRIATHLON_PAUSE_CONTINUE;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				
				//振动提示
				gui_sport_motor_prompt();
			}
			else if(g_sport_status == SPORT_STATUS_PAUSE)
			{
				//继续计时
				g_sport_status = SPORT_STATUS_START;
				Set_PauseSta(false);
				
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				
				//振动提示
				gui_sport_motor_prompt();
			}
			break;
		case KEY_BACK:
			if(g_sport_status == SPORT_STATUS_START)
			{
				//保存换项数据
				TriathlonData.FininshNum = 2;
				sport_triathlon_change_start();
				
				//开始骑行运动
				ScreenState = DISPLAY_SCREEN_TRIATHLON_CYCLING_READY;
				m_gui_sport_triathlon_type = DISPLAY_SCREEN_TRIATHLON_CYCLING;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			
//				if(ScreenState < DISPLAY_SCREEN_NOTIFY)
				{
					//5S后进入骑行界面
					ScreenStateSave = DISPLAY_SCREEN_TRIATHLON_CYCLING;
				}							
				timer_notify_display_start(5000,1,false);
			}
			else if(g_sport_status == SPORT_STATUS_PAUSE)
			{
				//返回暂停菜单界面
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

//铁人三项骑行准备界面
void gui_sport_triathlon_cycling_ready_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
			//关闭定时器
			timer_notify_display_stop(false);
		
			//暂停骑行运动
			g_sport_status = SPORT_STATUS_PAUSE;
			Set_PauseSta(true);
			
			//进入暂停菜单界面
			ScreenState = DISPLAY_SCREEN_TRIATHLON_PAUSE;
			m_gui_sport_triathlon_pause_index = GUI_SPORT_TRIATHLON_PAUSE_CONTINUE;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			
			//振动提示
			gui_sport_motor_prompt();
			break;
		default:
			break;
	}
}

//铁人三项骑行界面
void gui_sport_triathlon_cycling_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
			if(g_sport_status == SPORT_STATUS_START)
			{
				//暂停运动，进入暂停菜单界面
				g_sport_status = SPORT_STATUS_PAUSE;
				Set_PauseSta(true);
				
				ScreenState = DISPLAY_SCREEN_TRIATHLON_PAUSE;
				m_gui_sport_triathlon_pause_index = GUI_SPORT_TRIATHLON_PAUSE_CONTINUE;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				
				//振动提示
				gui_sport_motor_prompt();
			}
			else if(g_sport_status == SPORT_STATUS_PAUSE)
			{
				//继续计时
				g_sport_status = SPORT_STATUS_START;
				Set_PauseSta(false);
				
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				
				//振动提示
				gui_sport_motor_prompt();
			}
			break;
		case KEY_BACK:
			if(g_sport_status == SPORT_STATUS_START)
			{
				//结束骑行运动
				g_sport_status = SPORT_STATUS_START;
				sport_triathlon_change_start();
				
				//进入换项界面
				ScreenState = DISPLAY_SCREEN_TRIATHLON_SECOND_CHANGE;
				m_gui_sport_triathlon_type = DISPLAY_SCREEN_TRIATHLON_SECOND_CHANGE;
				
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
			else if(g_sport_status == SPORT_STATUS_PAUSE)
			{
				//返回暂停菜单界面
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

//铁人三项第二换项界面
void gui_sport_triathlon_second_change_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
			if(g_sport_status == SPORT_STATUS_START)
			{
				//暂停换项
				g_sport_status = SPORT_STATUS_PAUSE;
				Set_PauseSta(true);
				
				//进入暂停菜单界面
				ScreenState = DISPLAY_SCREEN_TRIATHLON_PAUSE;
				m_gui_sport_triathlon_pause_index = GUI_SPORT_TRIATHLON_PAUSE_CONTINUE;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				
				//振动提示
				gui_sport_motor_prompt();
			}
			else if(g_sport_status == SPORT_STATUS_PAUSE)
			{
				//继续计时
				g_sport_status = SPORT_STATUS_START;
				Set_PauseSta(false);
				
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				
				//振动提示
				gui_sport_motor_prompt();
			}
			break;
		case KEY_BACK:
			//开始跑步运动
			if(g_sport_status == SPORT_STATUS_START)
			{
				//保存换项数据
				TriathlonData.FininshNum = 3;
				sport_triathlon_change_start();
				
				ScreenState = DISPLAY_SCREEN_TRIATHLON_RUNNING_READY;
				m_gui_sport_triathlon_type = DISPLAY_SCREEN_TRIATHLON_RUNNING;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				
//				if(ScreenState < DISPLAY_SCREEN_NOTIFY)
				{
					//5S后进入骑行界面
					ScreenStateSave = DISPLAY_SCREEN_TRIATHLON_RUNNING;
				}							
				timer_notify_display_start(5000,1,false);
			}
			else if(g_sport_status == SPORT_STATUS_PAUSE)
			{
				//返回暂停菜单界面
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

//铁人三项跑步准备界面
void gui_sport_triathlon_running_ready_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
			//关闭定时器
			timer_notify_display_stop(false);
		
			//暂停跑步运动
			g_sport_status = SPORT_STATUS_PAUSE;
			Set_PauseSta(true);
			
			//进入暂停菜单界面
			ScreenState = DISPLAY_SCREEN_TRIATHLON_PAUSE;
			m_gui_sport_triathlon_pause_index = GUI_SPORT_TRIATHLON_PAUSE_CONTINUE;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			
			//振动提示
			gui_sport_motor_prompt();
			break;
		default:
			break;
	}
}

//铁人三项跑步界面
void gui_sport_triathlon_running_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
			if(g_sport_status == SPORT_STATUS_START)
			{
				//暂停运动，进入暂停菜单界面
				g_sport_status = SPORT_STATUS_PAUSE;
				Set_PauseSta(true);
				
				ScreenState = DISPLAY_SCREEN_TRIATHLON_PAUSE;
				m_gui_sport_triathlon_pause_index = GUI_SPORT_TRIATHLON_PAUSE_CONTINUE;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				
				//振动提示
				gui_sport_motor_prompt();
			}
			else if(g_sport_status == SPORT_STATUS_PAUSE)
			{
				//继续计时
				g_sport_status = SPORT_STATUS_START;
				Set_PauseSta(false);
				
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				
				//振动提示
				gui_sport_motor_prompt();
			}
			break;
		case KEY_BACK:
			if(g_sport_status == SPORT_STATUS_START)
			{
				//结束跑步运动，铁人三项运动已完成
				g_sport_status = SPORT_STATUS_READY;
				Set_PauseSta(true);
				//保存运动数据，退出运动任务
				Set_SaveSta(1);
				CloseSportTask();
				
				ScreenState = DISPLAY_SCREEN_TRIATHLON_FINISH;
				m_gui_sport_triathlon_pause_index = 0;
				
				timer_progress_start(400);
				
				//振动提醒
				timer_notify_motor_start(1000,500,3,false,NOTIFY_MODE_SPORTS);
		        timer_notify_buzzer_start(1000,500,3,false,NOTIFY_MODE_SPORTS);
			}
			else if(g_sport_status == SPORT_STATUS_PAUSE)
			{
				//返回暂停菜单界面
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

//暂停菜单界面按键事件
void gui_sport_triathlon_pause_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
			if(m_gui_sport_triathlon_pause_index == GUI_SPORT_TRIATHLON_PAUSE_CONTINUE)
			{
				//继续计时，开始运动
				g_sport_status = SPORT_STATUS_START;
				Set_PauseSta(false);
				
				ScreenState = m_gui_sport_triathlon_type;
				
				//振动提示
				gui_sport_motor_prompt();
			}
			else if(m_gui_sport_triathlon_pause_index == GUI_SPORT_TRIATHLON_PAUSE_SAVE)
			{
				//铁人三项运动已完成
				g_sport_status = SPORT_STATUS_READY;
				
				if((m_gui_sport_triathlon_type == DISPLAY_SCREEN_TRIATHLON_FIRST_CHANGE)
					||(m_gui_sport_triathlon_type == DISPLAY_SCREEN_TRIATHLON_SECOND_CHANGE))
				{
					//在换项中结束运动，则不保存换项运动数据
					gui_sport_triathlon_status_set(1);
					Set_SaveSta(0);
				}
				else
				{
					//保存运动数据
					Set_SaveSta(1);
				}
				
				//退出任务
				CloseSportTask();
				
//				//默认为有点吃力选项
//				gui_sport_feel_init();
				
				ScreenState = DISPLAY_SCREEN_SPORT_SAVING;
				m_gui_sport_triathlon_pause_index = 0;
				
				extern volatile ScreenState_t Save_Detail_Index;
				Save_Detail_Index = DISPLAY_SCREEN_TRIATHLON_DETAIL;
				
				timer_progress_start(200);
				
				//振动提示
				gui_sport_motor_prompt();
			}
			else
			{
				//放弃运动
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
			//返回铁人三项界面
			ScreenState = m_gui_sport_triathlon_type;
		
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}

//放弃菜单界面按键事件
void gui_sport_triathlon_cancel_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
			if(m_gui_sport_triathlon_pause_index == GUI_SPORT_TRIATHLON_PAUSE_CANCEL_NO)
			{
				//取消放弃运动，返回上级菜单界面
				ScreenState = DISPLAY_SCREEN_TRIATHLON_PAUSE;
				m_gui_sport_triathlon_pause_index = GUI_SPORT_TRIATHLON_PAUSE_CANCEL;
			}
			else
			{
				//确认取消运动，返回主界面
				g_sport_status = SPORT_STATUS_READY;
				
				ScreenState = DISPLAY_SCREEN_HOME;
				
				//结束运动不保存
				Set_SaveSta(0);
				CloseSportTask();
				
				//振动提示
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
			//返回上级菜单
			ScreenState = DISPLAY_SCREEN_TRIATHLON_PAUSE;
			m_gui_sport_triathlon_pause_index = GUI_SPORT_TRIATHLON_PAUSE_CANCEL;
			
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}

//详细数据界面按键事件
void gui_sport_triathlon_detail_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	uint8_t pageMax = TriathlonData.FininshNum > 2 ? 2 : TriathlonData.FininshNum;
	
	pageMax += 1;
	
	switch( Key_Value )
	{
		case KEY_UP:
			//上翻
			if(m_gui_sport_triathlon_pause_index > 0)
			{
				m_gui_sport_triathlon_pause_index--;
				
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
		    break;
		case KEY_DOWN:
			//下翻
			if(m_gui_sport_triathlon_pause_index < pageMax)
			{
				m_gui_sport_triathlon_pause_index++;
				
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
			break;
		case KEY_BACK:
			//返回主界面
			ScreenState = DISPLAY_SCREEN_HOME;
			
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}

