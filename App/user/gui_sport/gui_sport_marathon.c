#include "gui_sport_config.h"
#include "gui_sport_marathon.h"

#include "drv_lcd.h"
#include "drv_key.h"

#include "task_sport.h"
#include "task_gps.h"

#include "time_notify.h"
#include "timer_traceback.h"
#include "timer_app.h"

#include "algo_hdr.h"
#include "algo_sport.h"

#include "com_data.h"
#include "com_sport.h"

#include "gui_sport.h"
#include "gui_run.h"

#include "font_config.h"
#include "gui_tool_gps.h"

#if DEBUG_ENABLED == 1 && GUI_SPORT_MARATHON_LOG_ENABLED == 1
	#define GUI_SPORT_MARATHON_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_SPORT_MARATHON_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define GUI_SPORT_MARATHON_WRITESTRING(...)
	#define GUI_SPORT_MARATHON_PRINTF(...)		        
#endif
#if defined WATCH_COM_SPORT


#define	GUI_SPORT_MARATHON_SCREEN_DATA			0	//登山高度数据界面
#define	GUI_SPORT_MARATHON_SCREEN_HEART			1	//登山心率界面
#define	GUI_SPORT_MARATHON_SCREEN_TRACK			2	//登山轨迹界面
#define	GUI_SPORT_MARATHON_SCREEN_TIME		    3	//登山实时时间
#define GUI_SPORT_MARATHON_SCREEN_MAX		    4


#define	GUI_SPORT_MARATHON_PAUSE_CONTINUE	0	//继续
#define	GUI_SPORT_MARATHON_PAUSE_SAVE		1	//结束运动
#define	GUI_SPORT_MARATHON_PAUSE_WAIT		2	//稍后继续
#define	GUI_SPORT_MARATHON_PAUSE_TRACKBACK	3	//循迹返航
#define	GUI_SPORT_MARATHON_PAUSE_CANCEL		4	//放弃

#define	GUI_SPORT_MARATHON_PAUSE_CANCEL_NO	0	//否
#define	GUI_SPORT_MARATHON_PAUSE_CANCEL_YES	1	//是


#define	GUI_SPORT_MARATHON_DETAIL_1				0	//详细数据界面1
#define	GUI_SPORT_MARATHON_DETAIL_2				1	//详细数据界面2
#define GUI_SPORT_MARATHON_DETAIL_TRACK			2	//轨迹界面
#define	GUI_SPORT_MARATHON_DETAIL_HEARTRATEZONE	3	//心率区间界面


static uint8_t m_gui_sport_marathon_index;		//登山界面索引
static uint8_t m_gui_sport_marathon_pause_index;	//登山暂停菜单界面索引
static uint16_t m_gui_sport_marathon_last_circle_pace;	//马拉松上圈配速

void gui_sport_marathon_last_pace_set(uint16_t pace)
{
	//得到上一圈配速
	m_gui_sport_marathon_last_circle_pace = pace;
}
extern volatile ScreenState_t Save_Detail_Index;

//按ok键运动开始提示
void gui_marathon_prompt_paint(void)
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

}


//马拉松数据界面
void gui_sport_marathon_data_paint(void)
{
		SetWord_t word = {0};
		//设置背景黑色
		LCD_SetBackgroundColor(LCD_BLACK);
	
		if(g_sport_status == SPORT_STATUS_READY)
		{
			gui_marathon_prompt_paint();
		}
		else
		{
			//圆点索引
			gui_sport_index_circle_paint(GUI_SPORT_MARATHON_SCREEN_DATA, GUI_SPORT_MARATHON_SCREEN_MAX, 1);
	
			gui_sport_data_display_total_distance_paint(SPORT_DATA_DISPALY_INDEX_UP_CIRCLE,ActivityData.ActivityDetails.RunningDetail.Distance);
			
		}
	
		
		//分割线
		word.x_axis = 80;
		LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,LCD_AUQAMARIN,0,0,LCD_FILL_ENABLE);
	
		
		//GPS状态,防止遮挡，采用靠左位置
		gui_sport_gps_status(SPORT_GPS_LEFT1);
	
		gui_sport_data_display_realtime_pace_paint(SPORT_DATA_DISPALY_INDEX_MID_GPS,ActivityData.ActivityDetails.RunningDetail.Pace);
	
		//分割线
		word.x_axis = 160;	
		LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,LCD_AUQAMARIN,0,0,LCD_FILL_ENABLE);
	
		gui_sport_data_display_total_time_paint(SPORT_DATA_DISPALY_INDEX_DOWN_NOHINT,ActivityData.ActTime);
	
		
	
}


//徒步越野心率界面
void gui_sport_marathon_heart_paint(void)
{
	//设置背景黑色
	LCD_SetBackgroundColor(LCD_BLACK);
	//圆点索引
    gui_sport_index_circle_paint(GUI_SPORT_MARATHON_SCREEN_HEART, GUI_SPORT_MARATHON_SCREEN_MAX, 1);

	gui_sport_gps_status(SPORT_GPS_LEFT1);
	
	//心率值
//	gui_sport_heart_1_paint(Get_Sport_Heart());

}


//徒步越野轨迹更新
void gui_marathon_track_update_paint(void)
{
        DrawLoadTrack();
        DrawTrack();

		//航迹标点与指针切换显示
		if (dottrack_draw_time > 0)
		{
		  gui_sport_dottrack_nums(dot_track_nums); //航迹打点标号
		}
		else if (TrackMarkIndex <= TRACK_MARK_NUM_MAX )
		{
			draw_track_mark();
		}
		else 
		{
			Drawsport_arrows();
		}	
	
}

//距离
static void gui_sport_marathon_1_distance(uint32_t distance)
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
//运动计时
static void gui_sport_marathon_1_time(void)
{
	SetWord_t word = {0};
	char str[10] = {0};
	
	word.x_axis = 180;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_32_SIZE;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 1;
	memset(str,0,sizeof(str));
	if(ActivityData.Act_Stop_Time.Hour > 0)
	{
		//时分秒
		sprintf(str,"%d:%02d:%02d",ActivityData.Act_Stop_Time.Hour, ActivityData.Act_Stop_Time.Minute, ActivityData.Act_Stop_Time.Second);
	}
	else
	{
		//分秒
		sprintf(str,"%d:%02d",ActivityData.Act_Stop_Time.Minute, ActivityData.Act_Stop_Time.Second);
	}
	LCD_SetNumber(str, &word);
}

//剩余距离
static void gui_sport_marathon_track_distance(uint8_t type, uint32_t distance)
{
	gui_sport_every_1_distance(type,distance);
}
static void gui_sport_marathon_track_time(void)
{
	SetWord_t word = {0};
	char str[10] = {0};
	
	word.x_axis = 184;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 1;
	memset(str,0,sizeof(str));
	if(ActivityData.Act_Stop_Time.Hour > 0)
	{
		//时分秒
		sprintf(str,"%d:%02d:%02d",ActivityData.Act_Stop_Time.Hour, ActivityData.Act_Stop_Time.Minute, ActivityData.Act_Stop_Time.Second);
	}
	else
	{
		//分秒
		sprintf(str,"%d:%02d",ActivityData.Act_Stop_Time.Minute, ActivityData.Act_Stop_Time.Second);
	}
	LCD_SetNumber(str, &word);
}

//徒步越野轨迹界面
void gui_sport_marathon_track_paint(void)
{
		static uint8_t last_status = 2;
	
		//设置背景黑色
		LCD_SetBackgroundColor(LCD_BLACK);
		 //轨迹
		gui_marathon_track_update_paint();

		gui_sport_index_circle_paint(GUI_SPORT_MARATHON_SCREEN_TRACK, GUI_SPORT_MARATHON_SCREEN_MAX, 1);
	
		
		if (dottrack_out_time != 0 || dottrack_nogps_time != 0)
    	{
			gui_sport_dot_draw_prompt();
    	}
		
		if((TRACE_BACK_DATA.is_traceback_flag||(loadflag ==1)) && (Get_PauseSta() == false)&& (GetGpsStatus()== true))
		{
			//运动状态且在循迹返航状态中或导航
			if((last_status == 0) && (TRACE_BACK_DATA.whether_in_track))
			{
				//已在轨迹上
				timer_notify_motor_start(300,300,1,false,NOTIFY_MODE_SPORTS);
				timer_notify_buzzer_start(300,300,1,false,NOTIFY_MODE_SPORTS);
				
				gui_sport_marathon_track_distance(2, 0);
			}
			else if((last_status == 1) && (TRACE_BACK_DATA.whether_in_track == 0))
			{
				//未在轨迹上
				timer_notify_motor_start(300,300,1,false,NOTIFY_MODE_SPORTS);
				timer_notify_buzzer_start(300,300,1,false,NOTIFY_MODE_SPORTS);
				
				gui_sport_marathon_track_distance(3, 0);
			}
			else
			{
				if(TRACE_BACK_DATA.sport_traceback_flag)
				{
					//计时
					gui_sport_marathon_track_time();
				}
				else
				{
					if(TRACE_BACK_DATA.whether_in_track)
					{
						//剩余距离
						gui_sport_marathon_track_distance(0, TRACE_BACK_DATA.remaining_distance);
					}
					else
					{
						//距离轨迹
						gui_sport_marathon_track_distance(1, TRACE_BACK_DATA.nearest_distance);
					}
				}
			}
			
			last_status = TRACE_BACK_DATA.whether_in_track;
		}
		else
		{
			//计时
			gui_sport_marathon_track_time();
			
			last_status = 2;
		}
		
	//	gui_crosscountry_realtime_battery_paint(205, g_bat_evt.level_percent);
		

		
		
		
		gui_sport_compass(0);
	
		gui_sport_gps_status(SPORT_GPS_LEFT2);		//使用通用GPS函数
	//	gui_crosscountry_gps_paint();
		
	
	
				
}


//徒步越野实时时间界面
void gui_sport_marathon_realtime_paint(void)
{
	//设置背景黑色
	LCD_SetBackgroundColor(LCD_BLACK);
	
	//圆点索引
    gui_sport_index_circle_paint(GUI_SPORT_MARATHON_SCREEN_TIME, GUI_SPORT_MARATHON_SCREEN_MAX, 1);
	
	//实时时间
//	gui_sport_realtime_1_paint(1);
		
}
#if 1
//运动开始提示
static void gui_sport_marathon_1_start(void)
{
	SetWord_t word = {0};

	//图标
	word.x_axis = 20;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	LCD_SetPicture(word.x_axis, word.y_axis, LCD_NONE, LCD_NONE, &Img_start_18X24);
	
	//按ok键开始运动
	word.x_axis += (Img_start_18X24.height + 6);
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	LCD_SetString((char *)SportsPromptStrs[SetValue.Language][0],&word);
}

//速度
static void gui_sport_marathon_2_speed(uint16_t last_speed,uint16_t speed)
{
	SetWord_t word = {0};
	char str[10] = {0};
	
	word.x_axis = 90;
	word.y_axis = 80 - 40;
	word.size = LCD_FONT_16_SIZE;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 0;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][41],&word);
	
	word.y_axis = 180 - 16;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][39],&word);
	
	//上一圈配速数据
	memset(str,0,sizeof(str));
    sprintf(str,"%d:%02d",(m_gui_sport_marathon_last_circle_pace/60),(m_gui_sport_marathon_last_circle_pace%60));
	
	word.x_axis += 16 + 6;
	word.y_axis = 80 - (strlen(str)*(Font_Number_32.width + 1)/2);
	word.size = LCD_FONT_32_SIZE;
	word.kerning = 1;
	LCD_SetNumber(str,&word);
	
	//配速数据
	memset(str,0,sizeof(str));
    sprintf(str,"%d:%02d",(speed/60),(speed%60));
	
	word.y_axis = 180 - (strlen(str)*(Font_Number_32.width + 1)/2);
	LCD_SetNumber(str,&word);
}


//马拉松界面1
static void gui_sport_marathon_1_paint(void)
{
	//显示黑色背景
	LCD_SetBackgroundColor(LCD_BLACK);

	
	if(g_sport_status == SPORT_STATUS_READY)
	{
		//提示
		gui_sport_marathon_1_start();
	}
	else
	{
		//圆点索引
		gui_sport_index_circle_paint(m_gui_sport_marathon_index, GUI_SPORT_MARATHON_SCREEN_MAX, 0);
		
		//里程
		gui_sport_marathon_1_distance(ActivityData.ActivityDetails.RunningDetail.Distance);
	}
	
	//分割线
	LCD_SetRectangle(80, 1, 0, LCD_LINE_CNT_MAX ,LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);
	
	//GPS状态
	gui_sport_gps_status(SPORT_GPS_LEFT1);
	
	//配速
	gui_sport_marathon_2_speed(ActivityData.ActivityDetails.RunningDetail.Pace,ActivityData.ActivityDetails.RunningDetail.Pace);
	
	//分割线
	LCD_SetRectangle(160, 1, 0, LCD_LINE_CNT_MAX, LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);
	
	//计时
	gui_sport_marathon_1_time();
}

#endif

//徒步越野界面
void gui_sport_marathon_paint(void)
{
	if(m_gui_sport_marathon_index ==GUI_SPORT_MARATHON_SCREEN_DATA )
	{
	 	//gui_sport_marathon_data_paint();
	 	gui_sport_marathon_1_paint();
	}
	else if (m_gui_sport_marathon_index ==GUI_SPORT_MARATHON_SCREEN_HEART)
	{
		gui_sport_marathon_heart_paint();
	}
	else if (m_gui_sport_marathon_index ==GUI_SPORT_MARATHON_SCREEN_TIME)
	{
		gui_sport_marathon_realtime_paint();
	}
	else
	{
		gui_sport_marathon_track_paint();
	}
	
	if(g_sport_status == SPORT_STATUS_PAUSE)
	{
		//暂时增加红色圆圈，表示暂停状态
		LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_RED);
	}

}


//暂停菜单选项
static void gui_sport_marathon_pause_options(uint8_t index)
{
	SetWord_t word = {0};
	
	//菜单第一项
	word.x_axis = 133;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 0;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString((char *)ClimbPauseStrs[SetValue.Language][index],&word); //都有稍后继续项
	
	//菜单第二项
	if(index >= GUI_SPORT_MARATHON_PAUSE_CANCEL)
	{
		index = GUI_SPORT_MARATHON_PAUSE_CONTINUE;
	}
	else
	{

		index += 1;
		if((TRACE_BACK_DATA.is_traceback_flag) && (index == GUI_SPORT_MARATHON_PAUSE_TRACKBACK))
		{
				//如果是在循迹返航过程，则无循迹返航选项
				index = GUI_SPORT_MARATHON_PAUSE_CANCEL;
		}
		

	}
	word.x_axis = 183;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString((char *)ClimbPauseStrs[SetValue.Language][index],&word);
}


//徒步越野暂停菜单界面
void gui_sport_marathon_pause_paint(void)
{
	//上半部分为白色背景
	LCD_SetRectangle(0, 120, 0, LCD_PIXEL_ONELINE_MAX, LCD_WHITE, 0, 0, LCD_FILL_ENABLE);
	
	//暂停计时
	gui_sport_pause_time();
	
	//运动计时
	gui_sport_pause_acttime();
	
	//下半部分为黑色背景
	LCD_SetRectangle(120, 120, 0, LCD_PIXEL_ONELINE_MAX, LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
	
	//选项
	gui_sport_marathon_pause_options(m_gui_sport_marathon_pause_index);
	
	//分割线
	LCD_SetRectangle(170, 1, 0, LCD_LINE_CNT_MAX, LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);
	
	//下翻提示
	gui_sport_page_prompt(SPORT_PAGE_DOWN);
	
	//红色圆圈
	LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_RED);
}

//徒步越野放弃菜单界面
void gui_sport_marathon_cancel_paint(void)
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
	gui_sport_cancel_options(m_gui_sport_marathon_pause_index);
	
	//分割线
	LCD_SetRectangle(170, 1, 0, LCD_LINE_CNT_MAX, LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);
	
	//下翻提示
	gui_sport_page_prompt(SPORT_PAGE_DOWN);
	
	//红色圆圈
	LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_RED);
}
//详细数据界面1
static void gui_sport_marathon_detail_1_paint(uint8_t backgroundcolor)
{
		SetWord_t word = {0};
		char str[20] = {0};
		uint32_t second;
		uint8_t charlen;
		
		//计算时长 = 总时长 - 暂停时长
		second = ActivityData.ActTime; //get_time_diff(ActivityData.Act_Start_Time, ActivityData.Act_Stop_Time) - Get_PauseTotolTime();
		
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

		word.size = LCD_FONT_20_SIZE;

		word.bckgrndcolor = LCD_NONE;
		word.kerning = 0;
		memset(str,0,sizeof(str));
		sprintf(str,"%02d-%02d-%02d %02d:%02d",ActivityData.Act_Start_Time.Year,ActivityData.Act_Start_Time.Month,
												ActivityData.Act_Start_Time.Day,ActivityData.Act_Start_Time.Hour,
												ActivityData.Act_Start_Time.Minute);
		LCD_SetNumber(str,&word);
		
		//分割线
		word.x_axis = 70;
		LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,LCD_AUQAMARIN,0,0,LCD_FILL_ENABLE);
		
		//里程
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
		sprintf(str,"%0.2f",ActivityData.ActivityDetails.RunningDetail.Distance/100000.f);	//厘米装换为千米，并保留2位小数
		#endif
		LCD_SetNumber(str,&word);
		
		word.y_axis = SPORT_DETAIL_DATA_UNITS_Y_AXIS;
		word.kerning = 0;
		LCD_SetString("km",&word);
		
		//时长
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
			//时分秒
			sprintf(str,"%d:%02d:%02d",second/3600,second/60%60,second%60);
		}
		else
		{
			//分秒
			sprintf(str,"%d:%02d",second/60%60,second%60);
		}
		LCD_SetNumber(str,&word);
		
		//平均配速
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
		
		//平均步频
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
	//	sprintf(str,"%d",ActivityData.ActivityDetails.RunningDetail.frequency);
		LCD_SetNumber(str,&word);
		
		word.y_axis = SPORT_DETAIL_DATA_UNITS_Y_AXIS;
		word.kerning = 0;
		LCD_SetString("spm",&word);
		
		//下翻提示
		gui_sport_page_prompt(SPORT_PAGE_DOWN);
}

//详细数据界面2
static void gui_sport_marathon_detail_2_paint(uint8_t backgroundcolor)
{
	SetWord_t word = {0};
	char str[20] = {0};
	uint8_t charlen;
	
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

	//卡路里
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
    sprintf(str,"%d",ActivityData.ActivityDetails.RunningDetail.Calorie/1000);	//将卡装换为千卡，并保留一位小数点
    LCD_SetNumber(str,&word);
	
    word.y_axis = SPORT_DETAIL_DATA_UNITS_Y_AXIS;
    word.kerning = 0;
    LCD_SetString("kcal",&word);
	
    //平均心率
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
	
	//最大心率
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

	//心率恢复率
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
	
	
	//有氧效果
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
    sprintf(str,"%.1f",ActivityData.ActivityDetails.RunningDetail.AerobicEffect/10.f);		//保留一位小数点
    #endif
    LCD_SetNumber(str,&word);
	
    //无氧效果
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
	sprintf(str,"%d.%01d",ActivityData.ActivityDetails.RunningDetail.AnaerobicEffect/10,ActivityData.ActivityDetails.RunningDetail.AnaerobicEffecty%10);
	#else
    sprintf(str,"%.1f",ActivityData.ActivityDetails.RunningDetail.AnaerobicEffect/10.f);	//保留一位小数点
    #endif
    LCD_SetNumber(str,&word);
	
	//恢复时间
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
	
	//下翻提示
	gui_sport_page_prompt(SPORT_PAGE_DOWN);
}	

//徒步越野详细数据界面
void gui_sport_marathon_detail_paint(void)
{
	switch(m_gui_sport_marathon_pause_index)
	{
		case GUI_SPORT_MARATHON_DETAIL_1:
			gui_sport_marathon_detail_1_paint(LCD_BLACK);
			break;
		case GUI_SPORT_MARATHON_DETAIL_2:
			gui_sport_marathon_detail_2_paint(LCD_BLACK);
			break;	
		case GUI_SPORT_MARATHON_DETAIL_TRACK:
			gui_sport_detail_track_paint(LCD_BLACK);
			break;
		case GUI_SPORT_MARATHON_DETAIL_HEARTRATEZONE:
			gui_sport_detail_heartratezone_paint(LCD_BLACK);
			break;
		default:
			break;
	}
}


void gui_sport_marathon_init(void)
{
	//默认为跑步数据界面
	m_gui_sport_marathon_index = GUI_SPORT_MARATHON_SCREEN_DATA;
	m_gui_sport_marathon_last_circle_pace = 0;
}


void gui_sport_marathon_btn_evt(uint32_t Key_Value)
{
	switch( Key_Value )
	{
		case KEY_OK:
			if(g_sport_status == SPORT_STATUS_READY)
			{
				//开始计时
				g_sport_status = SPORT_STATUS_START;
				
				gui_swich_msg();
				
				//振动提示
				gui_sport_motor_prompt();
				
				//创建运动任务
				StartSport();
			}
			 else if(g_sport_status == SPORT_STATUS_START)
			{
				//暂停运动，进入暂停菜单界面
				g_sport_status = SPORT_STATUS_PAUSE;
				Set_PauseSta(true);
				
				ScreenState = DISPLAY_SCREEN_MARATHON_PAUSE;
				m_gui_sport_marathon_pause_index = GUI_SPORT_MARATHON_PAUSE_CONTINUE;
				//gui_swich_msg();
				Save_Detail_Index = ScreenState;
				ScreenState = DISPLAY_SCREEN_REMIND_PAUSE;
				timer_app_pasue_start();
				//振动提示
				gui_sport_motor_prompt();
			}
			else if(g_sport_status == SPORT_STATUS_PAUSE)
			{
				//继续计时
				g_sport_status = SPORT_STATUS_START;
				Set_PauseSta(false);
				
				gui_swich_msg();
				Save_Detail_Index = ScreenState;
				timer_app_pasue_start();
				//振动提示
				gui_sport_motor_prompt();
			}
			break;
		case KEY_DOWN:
			if (g_sport_status != SPORT_STATUS_READY)
			{
             //翻页查看跑步界面
				if(m_gui_sport_marathon_index < (GUI_SPORT_MARATHON_SCREEN_MAX -1))
				{
					m_gui_sport_marathon_index++;
				}
				else
				{
					m_gui_sport_marathon_index = 0;
				}
				
				gui_swich_msg();
			}
			break;
		case KEY_UP:
			if (g_sport_status != SPORT_STATUS_READY)
			{
           		//翻页查看跑步界面
				if(m_gui_sport_marathon_index > 0)
				{
					m_gui_sport_marathon_index--;
				}
				else
				{
					m_gui_sport_marathon_index = (GUI_SPORT_MARATHON_SCREEN_MAX -1);
				}
				
				gui_swich_msg();
			}
		    break;
		case KEY_BACK:
 			if(g_sport_status == SPORT_STATUS_PAUSE)
			{
				//返回暂停菜单界面
				ScreenState = DISPLAY_SCREEN_MARATHON_PAUSE;
				m_gui_sport_marathon_index = GUI_SPORT_MARATHON_PAUSE_CONTINUE;
			
				gui_swich_msg();
			
			}
			else if(g_sport_status == SPORT_STATUS_READY)
			{
			  if(0 == ENTER_NAVIGATION && 0 == ENTER_MOTION_TRAIL)    //由导航界面进入运动ready界面的判断
				{


					
					//返回运动选项菜单界面
					ScreenState = DISPLAY_SCREEN_SPORT;
					
				
					gui_swich_msg();
					
					//关闭GPS任务
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
		case KEY_LONG_UP:
			if(g_sport_status == SPORT_STATUS_START)
			{
				//轨迹界面保存航迹打点数据
				if(m_gui_sport_marathon_index == GUI_SPORT_MARATHON_SCREEN_TRACK)
				{	
					sport_dottrack();

				}	
			}
			break;
		default:
			break;
	}
}

//徒步越野暂停菜单界面按键事件
void gui_sport_marathon_pause_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
			if(m_gui_sport_marathon_pause_index == GUI_SPORT_MARATHON_PAUSE_CONTINUE)
			{
				//继续计时，开始运动
				g_sport_status = SPORT_STATUS_START;
				Set_PauseSta(false);
				
				ScreenState = DISPLAY_SCREEN_MARATHON;
				gui_swich_msg();
				Save_Detail_Index = ScreenState;
				timer_app_pasue_start();
				//振动提示
				gui_sport_motor_prompt();
			}
			else if(m_gui_sport_marathon_pause_index == GUI_SPORT_MARATHON_PAUSE_SAVE)
			{
				//结束运动并保存数据
				g_sport_status = SPORT_STATUS_READY;
				
				//默认选项
				gui_sport_feel_init();
				
				ScreenState = DISPLAY_SCREEN_SPORT_FEEL;
				m_gui_sport_marathon_pause_index = GUI_SPORT_MARATHON_DETAIL_1;
				sport_end_flag = 1;
				sport_end_hrvalue = get_10s_heartrate(); //获取前10s平均心率
				g_HeartNum = 0;
				extern volatile ScreenState_t Save_Detail_Index;
				Save_Detail_Index = DISPLAY_SCREEN_MARATHON_DETAIL;
				gui_swich_msg();
			}
			else if(m_gui_sport_marathon_pause_index == GUI_SPORT_MARATHON_PAUSE_WAIT)
			{
				m_gui_sport_marathon_pause_index = GUI_SPORT_MARATHON_PAUSE_CONTINUE;
				
				//稍后继续
				sport_continue_later_start();
				
				//稍后继续保存运动类型
				ContinueLaterDataSave.SportType = DISPLAY_SCREEN_MARATHON;
				
				//稍后继续，退出运动界面，返回主界面
				ScreenState = DISPLAY_SCREEN_HOME;
				gui_swich_msg();
			}
			else if(m_gui_sport_marathon_pause_index == GUI_SPORT_MARATHON_PAUSE_TRACKBACK)
			{
				//开始循迹返航
				g_sport_status = SPORT_STATUS_START;
				
				//保存数据
				TRACE_BACK_DATA.is_traceback_flag = 1;
				TRACE_BACK_DATA.sport_distance = ActivityData.ActivityDetails.RunningDetail.Distance;
				TRACE_BACK_DATA.remaining_distance = ActivityData.ActivityDetails.RunningDetail.Distance;
				
				//开启循迹返航定时器
				trace_back_timer_start();
				
				//循迹返航轨迹标记
                TrackBackFlag();
				
				//取消暂停
				Set_PauseSta(false);
				
				ScreenState = DISPLAY_SCREEN_MARATHON;
				gui_swich_msg();
				Save_Detail_Index = ScreenState;
				timer_app_pasue_start();
				gui_sport_motor_prompt();
			}
			else
			{
				//放弃运动
				ScreenState = DISPLAY_SCREEN_MARATHON_CANCEL;
				
				m_gui_sport_marathon_pause_index = GUI_SPORT_MARATHON_PAUSE_CANCEL_NO;
				gui_swich_msg();
			}
			
			break;
		case KEY_UP:
			if(m_gui_sport_marathon_pause_index > GUI_SPORT_MARATHON_PAUSE_CONTINUE)
			{

			
			
				m_gui_sport_marathon_pause_index--;
				if((TRACE_BACK_DATA.is_traceback_flag) && (m_gui_sport_marathon_pause_index == GUI_SPORT_MARATHON_PAUSE_TRACKBACK))
				{
						//如果是在循迹返航过程，则无循迹返航选项
						m_gui_sport_marathon_pause_index = GUI_SPORT_MARATHON_PAUSE_WAIT;
				}
			

			}
			else
			{
				m_gui_sport_marathon_pause_index = GUI_SPORT_MARATHON_PAUSE_CANCEL;
			}
			
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		    break;
		case KEY_DOWN:
			if(m_gui_sport_marathon_pause_index < GUI_SPORT_MARATHON_PAUSE_CANCEL)
			{

			
				m_gui_sport_marathon_pause_index++;
				if((TRACE_BACK_DATA.is_traceback_flag) && (m_gui_sport_marathon_pause_index == GUI_SPORT_MARATHON_PAUSE_TRACKBACK))
				{
					//如果是在循迹返航过程，则无循迹返航选项
					m_gui_sport_marathon_pause_index = GUI_SPORT_MARATHON_PAUSE_CANCEL;
				}
				

			}
			else
			{
				m_gui_sport_marathon_pause_index = GUI_SPORT_MARATHON_PAUSE_CONTINUE;
			}
			
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		case KEY_BACK:
			//返回骑行界面
			ScreenState = DISPLAY_SCREEN_MARATHON;
		
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}

//放弃菜单界面按键事件
void gui_sport_marathon_cancel_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
			if(m_gui_sport_marathon_pause_index == GUI_SPORT_MARATHON_PAUSE_CANCEL_NO)
			{
				//取消放弃运动，返回上级菜单界面
				ScreenState = DISPLAY_SCREEN_MARATHON_PAUSE;
				m_gui_sport_marathon_pause_index = GUI_SPORT_MARATHON_PAUSE_CANCEL;
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
			if(m_gui_sport_marathon_pause_index == GUI_SPORT_MARATHON_PAUSE_CANCEL_NO)
			{
				m_gui_sport_marathon_pause_index = GUI_SPORT_MARATHON_PAUSE_CANCEL_YES;
			}
			else
			{
				m_gui_sport_marathon_pause_index = GUI_SPORT_MARATHON_PAUSE_CANCEL_NO;
			}

			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		    break;
		case KEY_BACK:
			//返回上级菜单
			ScreenState = DISPLAY_SCREEN_MARATHON_PAUSE;
			m_gui_sport_marathon_pause_index = GUI_SPORT_MARATHON_PAUSE_CANCEL;
			
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}

//详细数据界面按键事件
void gui_sport_marathon_detail_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_UP:
			//上翻
			if(m_gui_sport_marathon_pause_index > GUI_SPORT_MARATHON_DETAIL_1)
			{
				m_gui_sport_marathon_pause_index--;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
		    break;
		case KEY_DOWN:
			//下翻
			if(m_gui_sport_marathon_pause_index < GUI_SPORT_MARATHON_DETAIL_HEARTRATEZONE)
			{
				m_gui_sport_marathon_pause_index++;

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


#else
#if defined WATCH_SIM_SPORT
	#define	GUI_SPORT_MARATHON_SCREEN_1			0	//马拉松第一个界面
	#define	GUI_SPORT_MARATHON_SCREEN_2			1	//马拉松第二个界面
	#define	GUI_SPORT_MARATHON_SCREEN_3			2	//马拉松第三个界面
	#define	GUI_SPORT_MARATHON_SCREEN_4			3	//马拉松第四个界面
	#define GUI_SPORT_MARATHON_SCREEN_MAX		4

	#define	GUI_SPORT_MARATHON_PAUSE_CONTINUE	0	//继续
	#define	GUI_SPORT_MARATHON_PAUSE_SAVE		1	//结束运动
	#define	GUI_SPORT_MARATHON_PAUSE_WAIT		2	//稍后继续
	#define	GUI_SPORT_MARATHON_PAUSE_TRACKBACK	3	//循迹返航
	#define	GUI_SPORT_MARATHON_PAUSE_CANCEL		4	//放弃
#else
	#define	GUI_SPORT_MARATHON_SCREEN_1			0	//马拉松第一个界面
	#define	GUI_SPORT_MARATHON_SCREEN_2			1	//马拉松第二个界面
	#define	GUI_SPORT_MARATHON_SCREEN_3			2	//马拉松第三个界面
	#define GUI_SPORT_MARATHON_SCREEN_MAX		3

	#define	GUI_SPORT_MARATHON_PAUSE_CONTINUE	0	//继续
	#define	GUI_SPORT_MARATHON_PAUSE_SAVE		1	//结束运动
	#define	GUI_SPORT_MARATHON_PAUSE_TRACKBACK	2	//循迹返航
	#define	GUI_SPORT_MARATHON_PAUSE_CANCEL		3	//放弃
#endif

#define	GUI_SPORT_MARATHON_PAUSE_CANCEL_NO	0	//否
#define	GUI_SPORT_MARATHON_PAUSE_CANCEL_YES	1	//是

#define	GUI_SPORT_MARATHON_DETAIL_1				0	//详细数据界面1
#define	GUI_SPORT_MARATHON_DETAIL_2				1	//详细数据界面2
#define GUI_SPORT_MARATHON_DETAIL_TRACK			2	//轨迹界面
#define	GUI_SPORT_MARATHON_DETAIL_HEARTRATEZONE	3	//心率区间界面

//#if defined WATCH_SIM_SPORT
extern volatile ScreenState_t Save_Detail_Index;

	/*马拉松数据显示当前项*/
	uint8_t m_marathon_data_display_buf[SPORT_DATA_DISPLAY_SUM]={SPORT_DATA_DISPALY_TOTAL_DISTANCE,SPORT_DATA_DISPALY_REALTIME_PACE,SPORT_DATA_DISPALY_TOTAL_TIME};
	/*马拉松数据显示选项*/
	const uint8_t m_marathon_data_display_option[MARATHON_DATA_DISPLAY_SUM]={SPORT_DATA_DISPALY_TOTAL_DISTANCE,SPORT_DATA_DISPALY_REALTIME_PACE,SPORT_DATA_DISPALY_TOTAL_TIME,SPORT_DATA_DISPALY_LAST_TIME,
		SPORT_DATA_DISPALY_AVE_PACE,SPORT_DATA_DISPALY_OPTIMUM_PACE,SPORT_DATA_DISPALY_LAST_PACE,SPORT_DATA_DISPALY_REALTIME_HEART,SPORT_DATA_DISPALY_AVE_HEART,
		SPORT_DATA_DISPALY_REALTIME_FREQUENCY,SPORT_DATA_DISPALY_AVE_FREQUENCY,SPORT_DATA_DISPALY_CALORIE};

	/*马拉松提醒设置选项*/
	const uint8_t m_marathon_remind_set_option[MARATHON_REMIND_SET_SUM]={SPORT_REMIND_SET_HAERT,SPORT_REMIND_SET_HRRECOVERY,SPORT_REMIND_SET_PACE,SPORT_REMIND_SET_DISTANCE,SPORT_REMIND_SET_AUTOCIRCLE};
//#endif

static uint8_t m_gui_sport_marathon_index;				//马拉松界面索引
static uint8_t m_gui_sport_marathon_pause_index;		//马拉松暂停菜单界面索引
static uint16_t m_gui_sport_marathon_last_circle_pace;	//马拉松上圈配速
static uint32_t m_gui_sport_marathon_last_circle_time;	//马拉松上圈时间

void gui_sport_marathon_last_pace_set(uint16_t pace)
{
	//得到上一圈配速
	m_gui_sport_marathon_last_circle_pace = pace;
}

void gui_sport_marathon_last_time_set(uint32_t time)
{
	//得到上一圈时间
	m_gui_sport_marathon_last_circle_time = time;
}

#if defined WATCH_SIM_SPORT
//飞腕马拉松数据显示项
void gui_sport_marathon_data_detail_paint(uint8_t state ,uint8_t m_sport_data_display)
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
			ave_pace = CalculateSpeed(ActivityData.ActTime,0,ActivityData.ActivityDetails.RunningDetail.Distance,ACT_MARATHON);
			gui_sport_data_display_ave_pace_paint(state,ave_pace);
			break;
		case SPORT_DATA_DISPALY_OPTIMUM_PACE:
			gui_sport_data_display_optimum_pace_paint(state,ActivityData.ActivityDetails.RunningDetail.OptimumPace);
			break;
		case SPORT_DATA_DISPALY_LAST_PACE:
			gui_sport_data_display_last_pace_paint(state,m_gui_sport_marathon_last_circle_pace);
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
		case SPORT_DATA_DISPALY_LAST_TIME:
			gui_sport_data_display_last_time_paint(state,m_gui_sport_marathon_last_circle_time);
			break;
		default:
			break;
   	}
}

//马拉松数据界面
void gui_sport_marathon_data_paint(void)
{
	SetWord_t word = {0};
	//设置背景黑色
	LCD_SetBackgroundColor(LCD_BLACK);

	//圆点索引,有上一圈时间界面，需要靠上
    gui_sport_index_circle_paint(m_gui_sport_marathon_index, GUI_SPORT_MARATHON_SCREEN_MAX, 0);

	
	//第一栏
	gui_sport_marathon_data_detail_paint(SPORT_DATA_DISPALY_INDEX_UP_CIRCLE,m_marathon_data_display_buf[0]);
	
	//分割线
	word.x_axis = 80;
	LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,LCD_AUQAMARIN,0,0,LCD_FILL_ENABLE);

	//第二栏
	gui_sport_marathon_data_detail_paint(SPORT_DATA_DISPALY_INDEX_MID_GPS,m_marathon_data_display_buf[1]);

	//分割线
	word.x_axis = 160;	
	LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,LCD_AUQAMARIN,0,0,LCD_FILL_ENABLE);

	//第三栏
	gui_sport_marathon_data_detail_paint(SPORT_DATA_DISPALY_INDEX_DOWN_NOHINT,m_marathon_data_display_buf[2]);

	
	//GPS状态,防止遮挡，采用靠左位置
	gui_sport_gps_status(SPORT_GPS_LEFT1);
}

//马拉松上一圈配速、心率、步频界面
void gui_sport_marathon_heart_spm_paint(uint8_t heart,uint16_t frequency)
{
	SetWord_t word = {0};
	char str[10];
	//设置背景黑色
	LCD_SetBackgroundColor(LCD_BLACK);
	
	//圆点索引,有上一圈时间界面，需要靠上
    gui_sport_index_circle_paint(m_gui_sport_marathon_index, GUI_SPORT_MARATHON_SCREEN_MAX, 0);
	
	//上一圈配速
	word.x_axis = 20;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 0;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][41],&word);
	
	word.x_axis += 16 + 4;
	word.size = LCD_FONT_32_SIZE;
	word.kerning = 1;
	memset(str,0,sizeof(str));
    sprintf(str,"%d:%02d",(m_gui_sport_marathon_last_circle_pace/60),(m_gui_sport_marathon_last_circle_pace%60));
	LCD_SetNumber(str,&word);
	
	word.x_axis = 62;
	word.y_axis = 170;
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 0;
	LCD_SetString("/km",&word);
	
	//分割线
	word.x_axis = 80;
	LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,LCD_AUQAMARIN,0,0,LCD_FILL_ENABLE);

	//心率图
	word.x_axis = 96;
	word.y_axis = 170;
	LCD_SetPicture(word.x_axis, word.y_axis, LCD_RED, LCD_NONE, &Img_heartrate_18X16);
	
	//心率值
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = get_hdr_range_color(heart);
	word.kerning = 1;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.x_axis = 94;
	word.size = LCD_FONT_48_SIZE;

	if(heart == 0)
	{	
		LCD_SetRectangle(120,3,93,26,LCD_WHITE,0,0,LCD_FILL_ENABLE);

		LCD_SetRectangle(120,3,121,26,LCD_WHITE,0,0,LCD_FILL_ENABLE);
	}
	else
	{
		memset(str,0,10);
		sprintf(str,"%d",heart);
		LCD_SetNumber(str,&word);
		
		word.y_axis = 170;
		word.x_axis = 133;
		word.size = LCD_FONT_16_SIZE;
		word.forecolor = LCD_WHITE;
		word.kerning = 0;
		LCD_SetString("bpm",&word);
	}
	
	//分割线
	word.x_axis = 160;
	LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,LCD_AUQAMARIN,0,0,LCD_FILL_ENABLE);

	//步频
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 1;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.x_axis = 170;
	word.size = LCD_FONT_32_SIZE;

	memset(str,0,10);
	sprintf(str,"%d",frequency);
	LCD_SetNumber(str,&word);

	word.x_axis = 180;
	word.y_axis = 170;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
	word.kerning = 0;
	LCD_SetString("spm",&word);
	
	//GPS状态
	gui_sport_gps_status(SPORT_GPS_LEFT1);
}

//马拉松实时时间界面
void gui_sport_marathon_realtime_paint(void)
{
	//设置背景黑色
	LCD_SetBackgroundColor(LCD_BLACK);
	
	//圆点索引,有上一圈时间界面，需要靠上
    gui_sport_index_circle_paint(m_gui_sport_marathon_index, GUI_SPORT_MARATHON_SCREEN_MAX, 0);
	
	//实时时间
	gui_sport_realtime_paint(1);
		
}
 
#else

//运动开始提示
static void gui_sport_marathon_1_start(void)
{
	SetWord_t word = {0};

	//图标
	word.x_axis = 20;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	LCD_SetPicture(word.x_axis, word.y_axis, LCD_NONE, LCD_NONE, &Img_start_18X24);
	
	//按ok键开始运动
	word.x_axis += (Img_start_18X24.height + 6);
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	LCD_SetString((char *)SportsPromptStrs[SetValue.Language][0],&word);
}

//距离
static void gui_sport_marathon_1_distance(uint32_t distance)
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
static void gui_sport_marathon_2_speed(uint16_t last_speed,uint16_t speed)
{
	SetWord_t word = {0};
	char str[10] = {0};
	
	word.x_axis = 90;
	word.y_axis = 80 - 40;
	word.size = LCD_FONT_16_SIZE;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 0;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][41],&word);
	
	word.y_axis = 180 - 16;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][39],&word);
	
	//上一圈配速数据
	memset(str,0,sizeof(str));
    sprintf(str,"%d:%02d",(m_gui_sport_marathon_last_circle_pace/60),(m_gui_sport_marathon_last_circle_pace%60));
	
	word.x_axis += 16 + 6;
	word.y_axis = 80 - (strlen(str)*(Font_Number_32.width + 1)/2);
	word.size = LCD_FONT_32_SIZE;
	word.kerning = 1;
	LCD_SetNumber(str,&word);
	
	//配速数据
	memset(str,0,sizeof(str));
    sprintf(str,"%d:%02d",(speed/60),(speed%60));
	
	word.y_axis = 180 - (strlen(str)*(Font_Number_32.width + 1)/2);
	LCD_SetNumber(str,&word);
}

//运动计时
static void gui_sport_marathon_1_time(void)
{
	SetWord_t word = {0};
	char str[10] = {0};
	
	word.x_axis = 170;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_32_SIZE;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 1;
	memset(str,0,sizeof(str));
	if(ActivityData.Act_Stop_Time.Hour > 0)
	{
		//时分秒
		sprintf(str,"%d:%02d:%02d",ActivityData.Act_Stop_Time.Hour, ActivityData.Act_Stop_Time.Minute, ActivityData.Act_Stop_Time.Second);
	}
	else
	{
		//分秒
		sprintf(str,"%d:%02d",ActivityData.Act_Stop_Time.Minute, ActivityData.Act_Stop_Time.Second);
	}
	LCD_SetNumber(str, &word);
}

//马拉松界面1
static void gui_sport_marathon_1_paint(void)
{
	//显示黑色背景
	LCD_SetBackgroundColor(LCD_BLACK);

	
	if(g_sport_status == SPORT_STATUS_READY)
	{
		//提示
		gui_sport_marathon_1_start();
	}
	else
	{
		//圆点索引
		gui_sport_index_circle_paint(m_gui_sport_marathon_index, GUI_SPORT_MARATHON_SCREEN_MAX, 0);
		
		//里程
		gui_sport_marathon_1_distance(ActivityData.ActivityDetails.RunningDetail.Distance);
	}
	
	//分割线
	LCD_SetRectangle(80, 1, 0, LCD_LINE_CNT_MAX ,LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);
	
	//GPS状态
	gui_sport_gps_status(SPORT_GPS_LEFT1);
	
	//配速
	gui_sport_marathon_2_speed(ActivityData.ActivityDetails.RunningDetail.Pace,ActivityData.ActivityDetails.RunningDetail.Pace);
	
	//分割线
	LCD_SetRectangle(160, 1, 0, LCD_LINE_CNT_MAX, LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);
	
	//计时
	gui_sport_marathon_1_time();
	
	//时间和电量
	gui_sport_realtime_battery(2);
}

//心率
static void gui_sport_marathon_2_heart(uint8_t heart)
{
	SetWord_t word = {0};
	char str[10] = {0};
	
	//心率
	word.x_axis = 36;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	LCD_SetPicture(word.x_axis,word.y_axis,LCD_NONE,LCD_NONE,&Img_heartrate_18X16);
	
	word.x_axis += Img_heartrate_18X16.height + 12;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = get_hdr_range_color(heart);
	word.size = LCD_FONT_32_SIZE;
	word.kerning = 1;
	memset(str,0,sizeof(str));
    sprintf(str,"%d",heart);
	LCD_SetNumber(str,&word);
}

//步频
static void gui_sport_marathon_2_stride(uint16_t stride)
{	
	SetWord_t word = {0};
	char str[10] = {0};
	
	//步频
	word.x_axis = 132;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 0;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][40],&word);
	
	word.x_axis += LCD_FONT_16_SIZE + 12;
	word.size = LCD_FONT_32_SIZE;
	word.kerning = 1;
	memset(str,0,sizeof(str));
    sprintf(str,"%d",stride);
	LCD_SetNumber(str,&word);
}

//马拉松界面2
static void gui_sport_marathon_2_paint(void)
{
	//显示黑色背景
	LCD_SetBackgroundColor(LCD_BLACK);
	
	//圆点索引
	gui_sport_index_circle_paint(m_gui_sport_marathon_index, GUI_SPORT_MARATHON_SCREEN_MAX, 0);
	
	//心率
	gui_sport_marathon_2_heart(Get_Sport_Heart());
	
	//分割线
	LCD_SetRectangle(120, 1, 0, LCD_LINE_CNT_MAX, LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);
	
	//步频
	gui_sport_marathon_2_stride(ActivityData.ActivityDetails.RunningDetail.frequency);
	
	//时间和电量
	gui_sport_realtime_battery(2);
	
	//GPS状态
	gui_sport_gps_status(SPORT_GPS_LEFT3);
}
#endif

//剩余距离
static void gui_sport_marathon_3_distance(uint8_t type, uint32_t distance)
{
	SetWord_t word = {0};
	char str[10] = {0};
	
	//里程数据
	memset(str,0,sizeof(str));
	#if defined (SPRINTF_FLOAT_TO_INT)
	sprintf(str,"%d.%02dkm",distance/100000,distance/1000%100);
	#else
	sprintf(str,"%0.2fkm",distance/100000.f);	//厘米装换为千米，并保留2位小数
	#endif
	
	word.x_axis = 184;
	word.y_axis = (LCD_PIXEL_ONELINE_MAX - (4*LCD_FONT_16_SIZE) - (strlen(str)*8) - 4)/2;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.size = LCD_FONT_16_SIZE;
	if(type == 2)
	{
		//已在轨迹上
		word.y_axis = LCD_CENTER_JUSTIFIED;
		LCD_SetString((char *)SportDetailStrs[SetValue.Language][37], &word);
		
		return;
	}
	else if(type == 3)
	{
		//未在轨迹上
		word.y_axis = LCD_CENTER_JUSTIFIED;
		LCD_SetString((char *)SportDetailStrs[SetValue.Language][38], &word);
		
		return;
	}
	else if(type == 0)
	{
		//剩余距离
		LCD_SetString((char *)SportDetailStrs[SetValue.Language][34], &word);
	}
	else
	{
		//距离轨迹
		LCD_SetString((char *)SportDetailStrs[SetValue.Language][36], &word);
	}
	
	word.y_axis += (4*LCD_FONT_16_SIZE) + 4;
	LCD_SetString(str, &word);
}

//轨迹
static void gui_sport_marathon_3_track(uint16_t angle)
{
	//指向箭头
//	gui_sport_arrows(angle, LCD_AQUA);
	DrawLoadTrack(); 
	DrawTrack();
	//航迹标点与指针切换显示
	if (dottrack_draw_time > 0)
	{
	  gui_sport_dottrack_nums(dot_track_nums); //航迹打点标号
	}
	else if (TrackMarkIndex <= TRACK_MARK_NUM_MAX )
	{
		draw_track_mark();
	}
	else 
	{
		Drawsport_arrows();
	}

}

//马拉松界面3
static void gui_sport_marathon_3_paint(void)
{
	static uint8_t last_status = 2;
	
	//显示黑色背景
	LCD_SetBackgroundColor(LCD_BLACK);
	
	//圆点索引
	gui_sport_index_circle_paint(m_gui_sport_marathon_index, GUI_SPORT_MARATHON_SCREEN_MAX, 1);

	if (dottrack_out_time != 0 || dottrack_nogps_time != 0)
    {
		gui_sport_dot_draw_prompt();
    }
	
	//轨迹
	gui_sport_marathon_3_track(0);
	
	if((TRACE_BACK_DATA.is_traceback_flag||(loadflag ==1)) && (Get_PauseSta() == false)&& (GetGpsStatus()== true))
	{
		//运动状态且在循迹返航状态中或导航
		if((last_status == 0) && (TRACE_BACK_DATA.whether_in_track))
		{
			//已在轨迹上
			timer_notify_motor_start(300,300,1,false,NOTIFY_MODE_SPORTS);
			timer_notify_buzzer_start(300,300,1,false,NOTIFY_MODE_SPORTS);
			
			gui_sport_marathon_3_distance(2, 0);
		}
		else if((last_status == 1) && (TRACE_BACK_DATA.whether_in_track == 0))
		{
			//未在轨迹上
			timer_notify_motor_start(300,300,1,false,NOTIFY_MODE_SPORTS);
			timer_notify_buzzer_start(300,300,1,false,NOTIFY_MODE_SPORTS);
			
			gui_sport_marathon_3_distance(3, 0);
		}
		else
		{
			if(TRACE_BACK_DATA.whether_in_track)
			{
				//剩余距离
				gui_sport_marathon_3_distance(0, TRACE_BACK_DATA.remaining_distance);
			}
			else
			{
				//距离轨迹
				gui_sport_marathon_3_distance(1, TRACE_BACK_DATA.nearest_distance);
			}
		}
		
		last_status = TRACE_BACK_DATA.whether_in_track;
	}
	else
	{
		last_status = 2;
	}
	
#if !defined WATCH_SIM_SPORT
	//时间和电量
	gui_sport_realtime_battery(1);
#endif
	
	//GPS状态
	gui_sport_gps_status(SPORT_GPS_LEFT2);
	
	//指北针
	gui_sport_compass(0);
}
//马拉松界面
void gui_sport_marathon_paint(void)
{
#if defined WATCH_SIM_SPORT
	if(m_gui_sport_marathon_index == GUI_SPORT_MARATHON_SCREEN_1)
	{
		//数据显示
		gui_sport_marathon_data_paint();
		
	}
	else if(m_gui_sport_marathon_index == GUI_SPORT_MARATHON_SCREEN_2)
	{
		//心率，步频
		gui_sport_marathon_heart_spm_paint(Get_Sport_Heart(),ActivityData.ActivityDetails.WalkingDetail.frequency);	
	}
	else if(m_gui_sport_marathon_index == GUI_SPORT_MARATHON_SCREEN_3)
	{
		//时间电量
		gui_sport_marathon_realtime_paint();
	}
	else
	{
		//马拉松界面3
		gui_sport_marathon_3_paint();
	}
#else
	if(m_gui_sport_marathon_index == GUI_SPORT_MARATHON_SCREEN_1)
	{
		//马拉松界面1
		gui_sport_marathon_1_paint();
	}
	else if(m_gui_sport_marathon_index == GUI_SPORT_MARATHON_SCREEN_2)
	{
		//马拉松界面2
		gui_sport_marathon_2_paint();	
	}
	else
	{
		//马拉松界面3
		gui_sport_marathon_3_paint();
	}
#endif
	
	if(g_sport_status == SPORT_STATUS_PAUSE)
	{
		//暂时增加红色圆圈，表示暂停状态
		LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_RED);
	}
}

//暂停计时
static void gui_sport_marathon_pause_time(void)
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
static void gui_sport_marathon_time(void)
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
		//时分秒
		sprintf(str,"%d:%02d:%02d",ActivityData.Act_Stop_Time.Hour, ActivityData.Act_Stop_Time.Minute, ActivityData.Act_Stop_Time.Second);	
	}
	else
	{
		//分秒
		sprintf(str,"%d:%02d",ActivityData.Act_Stop_Time.Minute, ActivityData.Act_Stop_Time.Second);
	}
	LCD_SetNumber(str, &word);
}

//暂停菜单选项
static void gui_sport_marathon_pause_options(uint8_t index)
{
	SetWord_t word = {0};
	
	//菜单第一项
	word.x_axis = 133;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 0;
	word.size = LCD_FONT_24_SIZE;
#if defined WATCH_SIM_SPORT
	LCD_SetString((char *)ClimbPauseStrs[SetValue.Language][index],&word);
#else
	LCD_SetString((char *)RunPauseStrs[SetValue.Language][index],&word);
#endif
	
	//菜单第二项
	if(index >= GUI_SPORT_MARATHON_PAUSE_CANCEL)
	{
		index = GUI_SPORT_MARATHON_PAUSE_CONTINUE;
	}
	else
	{
		index += 1;
		if((TRACE_BACK_DATA.is_traceback_flag) && (index == GUI_SPORT_MARATHON_PAUSE_TRACKBACK))
		{
			//如果是在循迹返航过程，则无循迹返航选项
			index = GUI_SPORT_MARATHON_PAUSE_CANCEL;
		}
	}
	word.x_axis = 183;
	word.size = LCD_FONT_16_SIZE;
#if defined WATCH_SIM_SPORT
	LCD_SetString((char *)ClimbPauseStrs[SetValue.Language][index],&word);
#else
	LCD_SetString((char *)RunPauseStrs[SetValue.Language][index],&word);
#endif
}

//暂停菜单界面
void gui_sport_marathon_pause_paint(void)
{
	//上半部分为白色背景
	LCD_SetRectangle(0, 120, 0, LCD_PIXEL_ONELINE_MAX, LCD_WHITE, 0, 0, LCD_FILL_ENABLE);
	
	//暂停计时
	gui_sport_marathon_pause_time();
	
	//运动计时
	gui_sport_marathon_time();
	
	//下半部分为黑色背景
	LCD_SetRectangle(120, 120, 0, LCD_PIXEL_ONELINE_MAX, LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
	
	//选项
	gui_sport_marathon_pause_options(m_gui_sport_marathon_pause_index);
	
	//分割线
	LCD_SetRectangle(170, 1, 0, LCD_LINE_CNT_MAX, LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);
	
	//下翻提示
	gui_sport_page_prompt(SPORT_PAGE_DOWN);
	
	//红色圆圈
	LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_RED);
}

//放弃菜单选项
static void gui_sport_marathon_cancel_options(uint8_t index)
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
	if(index >= GUI_SPORT_MARATHON_PAUSE_CANCEL_YES)
	{
		index = GUI_SPORT_MARATHON_PAUSE_CANCEL_NO;
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
void gui_sport_marathon_cancel_paint(void)
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
	gui_sport_marathon_cancel_options(m_gui_sport_marathon_pause_index);
	
	//分割线
	LCD_SetRectangle(170, 1, 0, LCD_LINE_CNT_MAX, LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);
	
	//下翻提示
	gui_sport_page_prompt(SPORT_PAGE_DOWN);
	
	//红色圆圈
	LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_RED);
}

//详细数据界面1
static void gui_sport_marathon_detail_1_paint(uint8_t backgroundcolor)
{
	SetWord_t word = {0};
	char str[20] = {0};
	uint32_t second;
	uint8_t charlen;
	
	//计算时长 = 总时长 - 暂停时长
	second = ActivityData.ActTime; //get_time_diff(ActivityData.Act_Start_Time, ActivityData.Act_Stop_Time) - Get_PauseTotolTime();
	
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
	
	//分割线
    word.x_axis = 70;
    LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,LCD_AUQAMARIN,0,0,LCD_FILL_ENABLE);
	
    //里程
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
	sprintf(str,"%0.2f",ActivityData.ActivityDetails.RunningDetail.Distance/100000.f);	//厘米装换为千米，并保留2位小数
	#endif
    LCD_SetNumber(str,&word);
	
    word.y_axis = SPORT_DETAIL_DATA_UNITS_Y_AXIS;
    word.kerning = 0;
    LCD_SetString("km",&word);
	
	//时长
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
		//时分秒
		sprintf(str,"%d:%02d:%02d",second/3600,second/60%60,second%60);
	}
	else
	{
		//分秒
		sprintf(str,"%d:%02d",second/60%60,second%60);
	}
    LCD_SetNumber(str,&word);
	
	//平均配速
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
	
	//平均步频
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
	sprintf(str,"%d",ActivityData.ActivityDetails.RunningDetail.frequency);
    LCD_SetNumber(str,&word);
	
    word.y_axis = SPORT_DETAIL_DATA_UNITS_Y_AXIS;
    word.kerning = 0;
    LCD_SetString("spm",&word);
	
	//下翻提示
	gui_sport_page_prompt(SPORT_PAGE_DOWN);
}

//详细数据界面2
static void gui_sport_marathon_detail_2_paint(uint8_t backgroundcolor)
{
	SetWord_t word = {0};
	char str[20] = {0};
	uint8_t charlen;
	
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

	//卡路里
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
    sprintf(str,"%d",ActivityData.ActivityDetails.RunningDetail.Calorie/1000);	//将卡装换为千卡，并保留一位小数点
    LCD_SetNumber(str,&word);
	
    word.y_axis = SPORT_DETAIL_DATA_UNITS_Y_AXIS;
    word.kerning = 0;
    LCD_SetString("kcal",&word);
	
    //平均心率
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
	
	//最大心率
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

	//心率恢复率
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
	
	//有氧效果
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
    sprintf(str,"%.1f",ActivityData.ActivityDetails.RunningDetail.AerobicEffect/10.f);		//保留一位小数点
    #endif
    LCD_SetNumber(str,&word);
	
    //无氧效果
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
    sprintf(str,"%.1f",ActivityData.ActivityDetails.RunningDetail.AnaerobicEffect/10.f);	//保留一位小数点
    #endif
    LCD_SetNumber(str,&word);
	
	//恢复时间
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
	
	//下翻提示
	gui_sport_page_prompt(SPORT_PAGE_DOWN);
}

//运动轨迹界面
static void gui_sport_marathon_detail_track_paint(void)
{
	SetWord_t word = {0};
	
	//设置背景黑色
    LCD_SetBackgroundColor(LCD_BLACK);
	
	//上翻提示
	gui_sport_page_prompt(SPORT_PAGE_UP);
	
	//轨迹
	word.x_axis = 25;
    word.y_axis = LCD_CENTER_JUSTIFIED;
    word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
    word.bckgrndcolor = LCD_NONE;
    LCD_SetString((char *)SportDetailStrs[SetValue.Language][12],&word);

    //保存轨迹显示
    DrawSaveTrack();
	
	//下翻提示
	gui_sport_page_prompt(SPORT_PAGE_DOWN);
}
#if 0
//运动心率区间界面
static void gui_sport_marathon_detail_heartratezone_paint(uint8_t backgroundcolor)
{
	SetWord_t word = {0};
	char str[10] = {0};
	uint8_t width,i;
	uint32_t MaxHeartRateZone = 0;
	uint8_t charlen;

	for (i = 0; i < 5;i++)
	{
		if(MaxHeartRateZone < ActivityData.ActivityDetails.CyclingDetail.HeartRateZone[i])
		{
			MaxHeartRateZone = ActivityData.ActivityDetails.CyclingDetail.HeartRateZone[i];
		} 
	}
	
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
	
	//心率区间图
    word.x_axis = 25;
    word.y_axis = LCD_CENTER_JUSTIFIED;
    word.size = LCD_FONT_16_SIZE;
    word.bckgrndcolor = LCD_NONE;
    LCD_SetString((char *)SportDetailStrs[SetValue.Language][13],&word);
    
	//区间5
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
    sprintf(str,"%d:%02d",(ActivityData.ActivityDetails.CyclingDetail.HeartRateZone[4]/60),
							(ActivityData.ActivityDetails.CyclingDetail.HeartRateZone[4]%60));
    word.y_axis += (8 + 8);
	word.kerning = 1;
    LCD_SetNumber(str,&word);
	
    word.y_axis += 44;
    width = 80 * ActivityData.ActivityDetails.CyclingDetail.HeartRateZone[4]/MaxHeartRateZone;
    LCD_SetRectangle(word.x_axis, 16, word.y_axis, width, LCD_RED, 0, 0, LCD_FILL_ENABLE);

    //区间4
    word.x_axis = 90;
    word.y_axis = 35;
	word.kerning = 0;
    LCD_SetString((char *)SportDetailStrs[SetValue.Language][14],&word);
    word.y_axis += (charlen*8+1);
    LCD_SetString("4",&word);
	
    memset(str,0,sizeof(str));
    sprintf(str,"%d:%02d",(ActivityData.ActivityDetails.CyclingDetail.HeartRateZone[3]/60),
							(ActivityData.ActivityDetails.CyclingDetail.HeartRateZone[3]%60));
    word.y_axis += (8 + 8);
	word.kerning = 1;
    LCD_SetNumber(str,&word);
	
    word.y_axis += 44;
    width = 80 * ActivityData.ActivityDetails.CyclingDetail.HeartRateZone[3]/MaxHeartRateZone;
    LCD_SetRectangle(word.x_axis, 16, word.y_axis, width, LCD_CHOCOLATE, 0, 0, LCD_FILL_ENABLE);

	//区间3
    word.x_axis = 120;
    word.y_axis = 35;
	word.kerning = 0;
    LCD_SetString((char *)SportDetailStrs[SetValue.Language][14],&word);
    word.y_axis += (charlen*8+1);
    LCD_SetString("3",&word);
	
    memset(str,0,sizeof(str));
    sprintf(str,"%d:%02d",(ActivityData.ActivityDetails.CyclingDetail.HeartRateZone[2]/60),
							(ActivityData.ActivityDetails.CyclingDetail.HeartRateZone[2]%60));
    word.y_axis += (8 + 8);
	word.kerning = 1;
    LCD_SetNumber(str,&word);
	
    word.y_axis += 44;
    width = 80 * ActivityData.ActivityDetails.CyclingDetail.HeartRateZone[2]/MaxHeartRateZone;
    LCD_SetRectangle(word.x_axis, 16, word.y_axis, width, LCD_GREEN, 0, 0, LCD_FILL_ENABLE);

	//区间2
    word.x_axis = 150;
    word.y_axis = 35;
	word.kerning = 0;
    LCD_SetString((char *)SportDetailStrs[SetValue.Language][14],&word);
    word.y_axis += (charlen*8+1);
    LCD_SetString("2",&word);
	
    memset(str,0,sizeof(str));
    sprintf(str,"%d:%02d",(ActivityData.ActivityDetails.CyclingDetail.HeartRateZone[1]/60),
							(ActivityData.ActivityDetails.CyclingDetail.HeartRateZone[1]%60));
    word.y_axis += (8 + 8);
	word.kerning = 1;
    LCD_SetNumber(str,&word);
	
    word.y_axis += 44;
    width = 80 * ActivityData.ActivityDetails.CyclingDetail.HeartRateZone[1]/MaxHeartRateZone;
    LCD_SetRectangle(word.x_axis, 16, word.y_axis, width, LCD_CORNFLOWERBLUE, 0, 0, LCD_FILL_ENABLE);

    //区间1
    word.x_axis = 180;
    word.y_axis = 35;
	word.kerning = 0;
    LCD_SetString((char *)SportDetailStrs[SetValue.Language][14],&word);
    word.y_axis += (charlen*8+1);
    LCD_SetString("1",&word);
	
    memset(str,0,sizeof(str));
    sprintf(str,"%d:%02d",(ActivityData.ActivityDetails.CyclingDetail.HeartRateZone[0]/60),
							(ActivityData.ActivityDetails.CyclingDetail.HeartRateZone[0]%60));
    word.y_axis += (8 + 8);
	word.kerning = 1;
    LCD_SetNumber(str,&word);
	
    word.y_axis += 44;
    width = 80 * ActivityData.ActivityDetails.CyclingDetail.HeartRateZone[0]/MaxHeartRateZone;
    LCD_SetRectangle(word.x_axis, 16, word.y_axis, width, LCD_LIGHTGRAY, 0, 0, LCD_FILL_ENABLE);
}
#endif
//详细数据界面
void gui_sport_marathon_detail_paint(void)
{
	switch(m_gui_sport_marathon_pause_index)
	{
		case GUI_SPORT_MARATHON_DETAIL_1:
			gui_sport_marathon_detail_1_paint(LCD_BLACK);
			break;
		case GUI_SPORT_MARATHON_DETAIL_2:
			gui_sport_marathon_detail_2_paint(LCD_BLACK);
			break;
		case GUI_SPORT_MARATHON_DETAIL_TRACK:
			gui_sport_marathon_detail_track_paint();
			break;
		default:

			gui_sport_detail_heartratezone_paint(LCD_BLACK);

			break;
	}
}

//void gui_sport_marathon_feel_paint(void)
//{
//	SetWord_t word = {0};

//	//设置背景黑色
//	LCD_SetBackgroundColor(LCD_BLACK);

//	//上翻提示
//	gui_sport_page_prompt(SPORT_PAGE_UP);
//	
//	//分割线
//	LCD_SetRectangle(80, 1, 0, LCD_LINE_CNT_MAX,LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);
//	
//	//选项
//	gui_sport_feel_options_paint(m_gui_sport_marathon_pause_index);
//	
//	//分割线
//	LCD_SetRectangle(160, 1, 0, LCD_LINE_CNT_MAX, LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);

//	//下翻提示
//	gui_sport_page_prompt(SPORT_PAGE_DOWN);
//}

void gui_sport_marathon_init(void)
{
	//默认为马拉松界面1
	m_gui_sport_marathon_index = GUI_SPORT_MARATHON_SCREEN_1;
	m_gui_sport_marathon_last_circle_pace = 0;
	m_gui_sport_marathon_last_circle_time = 0;
}

//马拉松界面按键事件
void gui_sport_marathon_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
			if(g_sport_status == SPORT_STATUS_READY)
			{
				//开始计时
				g_sport_status = SPORT_STATUS_START;
				
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				
				//振动提示
				gui_sport_motor_prompt();
				
				//创建运动任务
				StartSport();
			}
			else if(g_sport_status == SPORT_STATUS_START)
			{
				//暂停运动，进入暂停菜单界面
				g_sport_status = SPORT_STATUS_PAUSE;
				Set_PauseSta(true);
				
				ScreenState = DISPLAY_SCREEN_MARATHON_PAUSE;
				m_gui_sport_marathon_pause_index = GUI_SPORT_MARATHON_PAUSE_CONTINUE;
#if defined WATCH_SIM_SPORT
				Save_Detail_Index = ScreenState;
				ScreenState = DISPLAY_SCREEN_REMIND_PAUSE;
				timer_app_pasue_start();
#else
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				
#endif			
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
#if defined WATCH_SIM_SPORT
				Save_Detail_Index = ScreenState;
				timer_app_pasue_start();
#endif				
				//振动提示
				gui_sport_motor_prompt();
			}
			break;
		case KEY_UP:
			if(g_sport_status != SPORT_STATUS_READY)
			{
				//翻页查看马拉松界面
				if(m_gui_sport_marathon_index > GUI_SPORT_MARATHON_SCREEN_1)
				{
					m_gui_sport_marathon_index--;
				}
				else
				{
					m_gui_sport_marathon_index = GUI_SPORT_MARATHON_SCREEN_MAX - 1;
				}
				
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
			break;
		case KEY_DOWN:
			if(g_sport_status != SPORT_STATUS_READY)
			{
				//翻页查看马拉松界面
				if(m_gui_sport_marathon_index < (GUI_SPORT_MARATHON_SCREEN_MAX - 1))
				{
					m_gui_sport_marathon_index++;
				}
				else
				{
					m_gui_sport_marathon_index = GUI_SPORT_MARATHON_SCREEN_1;
				}
				
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
			break;
		case KEY_BACK:
			if(g_sport_status == SPORT_STATUS_READY)
			{
				if(0 == ENTER_NAVIGATION && 0 == ENTER_MOTION_TRAIL)    //由导航界面进入运动ready界面的判断
				{

//				if(Get_Curr_Home_Index() == DISPLAY_SCREEN_TRAINPLAN_MARATHON)
//				{
//					//返回训练计划界面
//					ScreenState = DISPLAY_SCREEN_TRAINPLAN_MARATHON;
//				}
//				else
					{
						//返回运动选项菜单界面
						ScreenState = DISPLAY_SCREEN_SPORT;
					}
					
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
					
					//关闭GPS任务
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
				//返回暂停菜单界面
				ScreenState = DISPLAY_SCREEN_MARATHON_PAUSE;
				m_gui_sport_marathon_pause_index = GUI_SPORT_MARATHON_PAUSE_CONTINUE;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
			break;
		case KEY_LONG_UP:
			if(g_sport_status == SPORT_STATUS_START)
			{
				//轨迹界面保存航迹打点数据
#if defined WATCH_SIM_SPORT
				if(m_gui_sport_marathon_index == GUI_SPORT_MARATHON_SCREEN_4)
#else
				if(m_gui_sport_marathon_index == GUI_SPORT_MARATHON_SCREEN_3)
#endif
				{	
					sport_dottrack();

				}	
			}
			break;
		default:
			break;
	}
}

//暂停菜单界面按键事件
void gui_sport_marathon_pause_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
			if(m_gui_sport_marathon_pause_index == GUI_SPORT_MARATHON_PAUSE_CONTINUE)
			{
				//继续计时，开始运动
				g_sport_status = SPORT_STATUS_START;
				Set_PauseSta(false);
				
				ScreenState = DISPLAY_SCREEN_MARATHON;
				gui_swich_msg();
#if defined WATCH_SIM_SPORT
				Save_Detail_Index = ScreenState;
				timer_app_pasue_start();
#endif				
				//振动提示
				gui_sport_motor_prompt();
			}
			else if(m_gui_sport_marathon_pause_index == GUI_SPORT_MARATHON_PAUSE_SAVE)
			{
				//结束运动并保存数据
				g_sport_status = SPORT_STATUS_READY;
				
				//默认选项
				gui_sport_feel_init();
				
				ScreenState = DISPLAY_SCREEN_SPORT_FEEL;
				m_gui_sport_marathon_pause_index = GUI_SPORT_MARATHON_DETAIL_1;
				sport_end_flag = 1;
				sport_end_hrvalue = get_10s_heartrate(); //获取前10s平均心率
				g_HeartNum = 0;
				extern volatile ScreenState_t Save_Detail_Index;
				Save_Detail_Index = DISPLAY_SCREEN_MARATHON_DETAIL;
				gui_swich_msg();
			}
#if defined WATCH_SIM_SPORT
			else if (m_gui_sport_marathon_pause_index == GUI_SPORT_MARATHON_PAUSE_WAIT)
			{
				m_gui_sport_marathon_pause_index = GUI_SPORT_MARATHON_PAUSE_CONTINUE;
				
				//稍后继续
				sport_continue_later_start();
				
				//稍后继续保存运动类型
				ContinueLaterDataSave.SportType = DISPLAY_SCREEN_MARATHON;
				
				//稍后继续，退出运动界面，返回主界面
				ScreenState = DISPLAY_SCREEN_HOME;
				gui_swich_msg();
			}
#endif
			else if(m_gui_sport_marathon_pause_index == GUI_SPORT_MARATHON_PAUSE_TRACKBACK)
			{
				//开始循迹返航
				g_sport_status = SPORT_STATUS_START;
				
				//保存数据
				TRACE_BACK_DATA.is_traceback_flag = 1;
				TRACE_BACK_DATA.sport_distance = ActivityData.ActivityDetails.CyclingDetail.Distance;
				TRACE_BACK_DATA.remaining_distance = ActivityData.ActivityDetails.CyclingDetail.Distance;
				
				//开启循迹返航定时器
				trace_back_timer_start();
				
				//循迹返航轨迹标记
                TrackBackFlag();
				
				//取消暂停
				Set_PauseSta(false);
				
				ScreenState = DISPLAY_SCREEN_MARATHON;
				gui_swich_msg();
#if defined WATCH_SIM_SPORT
				Save_Detail_Index = ScreenState;
				timer_app_pasue_start();
#endif
				gui_sport_motor_prompt();
			}
			else
			{
				//放弃运动
				ScreenState = DISPLAY_SCREEN_MARATHON_CANCEL;
				
				m_gui_sport_marathon_pause_index = GUI_SPORT_MARATHON_PAUSE_CANCEL_NO;
				gui_swich_msg();
			}
			
			//msg.cmd = MSG_DISPLAY_SCREEN;
			//xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		case KEY_UP:
			if(m_gui_sport_marathon_pause_index > GUI_SPORT_MARATHON_PAUSE_CONTINUE)
			{
				m_gui_sport_marathon_pause_index--;
				if((TRACE_BACK_DATA.is_traceback_flag) && (m_gui_sport_marathon_pause_index == GUI_SPORT_MARATHON_PAUSE_TRACKBACK))
				{
					//如果是在循迹返航过程，则无循迹返航选项
					m_gui_sport_marathon_pause_index = GUI_SPORT_MARATHON_PAUSE_SAVE;
				}
			}
			else
			{
				m_gui_sport_marathon_pause_index = GUI_SPORT_MARATHON_PAUSE_CANCEL;
			}
			
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		    break;
		case KEY_DOWN:
			if(m_gui_sport_marathon_pause_index < GUI_SPORT_MARATHON_PAUSE_CANCEL)
			{
				m_gui_sport_marathon_pause_index++;
				if((TRACE_BACK_DATA.is_traceback_flag) && (m_gui_sport_marathon_pause_index == GUI_SPORT_MARATHON_PAUSE_TRACKBACK))
				{
					//如果是在循迹返航过程，则无循迹返航选项
					m_gui_sport_marathon_pause_index = GUI_SPORT_MARATHON_PAUSE_CANCEL;
				}
			}
			else
			{
				m_gui_sport_marathon_pause_index = GUI_SPORT_MARATHON_PAUSE_CONTINUE;
			}
			
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		case KEY_BACK:
			//返回马拉松界面
			ScreenState = DISPLAY_SCREEN_MARATHON;
		
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}

//放弃菜单界面按键事件
void gui_sport_marathon_cancel_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
			if(m_gui_sport_marathon_pause_index == GUI_SPORT_MARATHON_PAUSE_CANCEL_NO)
			{
				//取消放弃运动，返回上级菜单界面
				ScreenState = DISPLAY_SCREEN_MARATHON_PAUSE;
				m_gui_sport_marathon_pause_index = GUI_SPORT_MARATHON_PAUSE_CANCEL;
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
			if(m_gui_sport_marathon_pause_index == GUI_SPORT_MARATHON_PAUSE_CANCEL_NO)
			{
				m_gui_sport_marathon_pause_index = GUI_SPORT_MARATHON_PAUSE_CANCEL_YES;
			}
			else
			{
				m_gui_sport_marathon_pause_index = GUI_SPORT_MARATHON_PAUSE_CANCEL_NO;
			}

			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		    break;
		case KEY_BACK:
			//返回上级菜单
			ScreenState = DISPLAY_SCREEN_MARATHON_PAUSE;
			m_gui_sport_marathon_pause_index = GUI_SPORT_MARATHON_PAUSE_CANCEL;
			
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}

//详细数据界面按键事件
void gui_sport_marathon_detail_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_UP:
			//上翻
			if(m_gui_sport_marathon_pause_index > GUI_SPORT_MARATHON_DETAIL_1)
			{
				m_gui_sport_marathon_pause_index--;
				
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
		    break;
		case KEY_DOWN:
			//下翻
			if(m_gui_sport_marathon_pause_index < GUI_SPORT_MARATHON_DETAIL_HEARTRATEZONE)
			{
				m_gui_sport_marathon_pause_index++;
				
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
#endif

