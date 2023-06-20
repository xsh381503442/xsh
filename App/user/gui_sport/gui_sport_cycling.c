#include "gui_sport_config.h"
#include "gui_sport_cycling.h"

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

#if DEBUG_ENABLED == 1 && GUI_SPORT_CYCLING_LOG_ENABLED == 1
	#define GUI_SPORT_CYCLING_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_SPORT_CYCLING_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define GUI_SPORT_CYCLING_WRITESTRING(...)
	#define GUI_SPORT_CYCLING_PRINTF(...)		        
#endif




#define	GUI_SPORT_CYCLING_SCREEN_1			0	//骑行第一个界面
#define	GUI_SPORT_CYCLING_SCREEN_2			1	//骑行第二个界面
#define	GUI_SPORT_CYCLING_SCREEN_3			2	//骑行第三个界面
//#define	GUI_SPORT_CYCLING_SCREEN_4			3	//骑行第四个界面
#define	GUI_SPORT_CYCLING_SCREEN_MAX		3


#define	GUI_SPORT_CYCLING_PAUSE_CONTINUE	0	//继续
#define	GUI_SPORT_CYCLING_PAUSE_SAVE		1	//结束运动
#define	GUI_SPORT_CYCLING_PAUSE_WAIT		2	//稍后继续
#define	GUI_SPORT_CYCLING_PAUSE_TRACKBACK	3	//循迹返航
#define	GUI_SPORT_CYCLING_PAUSE_CANCEL		4	//放弃

#define	GUI_SPORT_CYCLING_PAUSE_CANCEL_NO	0	//否
#define	GUI_SPORT_CYCLING_PAUSE_CANCEL_YES	1	//是

#define	GUI_SPORT_CYCLING_DETAIL_1				0	//详细数据界面1
#define	GUI_SPORT_CYCLING_DETAIL_2				1	//详细数据界面2
//#define	GUI_SPORT_CYCLING_DETAIL_3				2	//详细数据界面3
#define GUI_SPORT_CYCLING_DETAIL_TRACK			2	//轨迹界面
//#define	GUI_SPORT_CYCLING_DETAIL_HEARTRATEZONE	4	//心率区间界面

//#if defined WATCH_SIM_SPORT
extern volatile ScreenState_t Save_Detail_Index;
uint8_t	vice_screen_cycling_display[SPORT_DATA_DISPLAY_SUM] = {SPORT_DATA_DISPALY_TOTAL_DISTANCE,SPORT_DATA_DISPALY_REALTIME_SPEED,SPORT_DATA_DISPALY_REALTIME_HEART,SPORT_DATA_DISPALY_TOTAL_TIME};

	/*骑行数据显示当前项*/
	 uint8_t m_cycling_data_display_buf[SPORT_DATA_DISPLAY_SUM]={SPORT_DATA_DISPALY_TOTAL_DISTANCE,SPORT_DATA_DISPALY_REALTIME_SPEED,SPORT_DATA_DISPALY_TOTAL_CLIMB,SPORT_DATA_DISPALY_TOTAL_TIME};
	/*骑行数据显示选项*/
	const uint8_t m_cycling_data_display_option[CYCLING_DATA_DISPLAY_SUM]= {SPORT_DATA_DISPALY_TOTAL_DISTANCE,SPORT_DATA_DISPALY_REALTIME_SPEED,SPORT_DATA_DISPALY_TOTAL_CLIMB,SPORT_DATA_DISPALY_TOTAL_TIME,
	SPORT_DATA_DISPALY_AVE_SPEED,SPORT_DATA_DISPALY_REALTIME_HEART};

	/*骑行提醒设置选项*/
	const uint8_t m_cycling_remind_set_option[CYCLING_REMIND_SET_SUM]={SPORT_REMIND_SET_HAERT,SPORT_REMIND_SET_HRRECOVERY,SPORT_REMIND_SET_SPEED,SPORT_REMIND_SET_DISTANCE,SPORT_REMIND_SET_TIME,SPORT_REMIND_SET_AUTOCIRCLE};

	static uint32_t m_gui_sport_cycling_last_circle_time;
//#endif

static uint8_t m_gui_sport_cycling_index;		//骑行界面索引
static uint8_t m_gui_sport_cycling_pause_index;	//骑行暂停菜单界面索引
extern uint8_t gui_sport_index;
extern uint8_t gui_sport_type;

void gui_sport_cycling_last_time_set(uint32_t time)
{
	//得到上一圈时间
	m_gui_sport_cycling_last_circle_time = time;
}
	
//飞腕骑行数据显示项
void gui_sport_cycling_data_detail_paint(uint8_t state ,uint8_t m_sport_data_display)
{
	uint16_t ave_speed = 0;
	uint16_t ave_cadence = 0;
   	switch(m_sport_data_display)
   	{
		case SPORT_DATA_DISPALY_TOTAL_TIME:
			gui_sport_data_display_total_time_paint(state,ActivityData.ActTime);
			break;
		case SPORT_DATA_DISPALY_TOTAL_DISTANCE:
			gui_sport_data_display_total_distance_paint(state,ActivityData.ActivityDetails.CyclingDetail.Distance);
			break;
		case SPORT_DATA_DISPALY_REALTIME_SPEED:
			gui_sport_data_display_realtime_speed_paint(state,ActivityData.ActivityDetails.CyclingDetail.Speed);
			break;
		case SPORT_DATA_DISPALY_AVE_SPEED:
			ave_speed = CalculateSpeed(ActivityData.ActTime,0,ActivityData.ActivityDetails.CyclingDetail.Distance,ACT_CYCLING);
			gui_sport_data_display_ave_speed_paint(state,ave_speed);
			break;
		case SPORT_DATA_DISPALY_OPTIMUM_SPEED:
			gui_sport_data_display_optimum_speed_paint(state,ActivityData.ActivityDetails.CyclingDetail.OptimumSpeed);
			break;
		case SPORT_DATA_DISPALY_REALTIME_HEART:
			gui_sport_data_display_realtime_heart_paint(state,Get_Sport_Heart());
			break;
		case SPORT_DATA_DISPALY_AVE_HEART:
			gui_sport_data_display_ave_heart_paint(state,ActivityData.ActivityDetails.CyclingDetail.AvgHeart);
			break;
		case SPORT_DATA_DISPALY_REALTIME_CADENCE:
			gui_sport_data_display_realtime_cadence_paint(state,ActivityData.ActivityDetails.CyclingDetail.Cadence);
			break;
		case SPORT_DATA_DISPALY_AVE_CADENCE:
			ave_cadence = Get_Cycling_AvgCadence();
			gui_sport_data_display_ave_cadence_paint(state,ave_cadence);
			break;
		case SPORT_DATA_DISPALY_CALORIE:
			gui_sport_data_display_calorie_paint(state,ActivityData.ActivityDetails.CyclingDetail.Calorie);
			break;
		case SPORT_DATA_DISPALY_LAST_TIME:
			gui_sport_data_display_last_time_paint(state,m_gui_sport_cycling_last_circle_time);
			break;
		default:
			break;
   	}
}

//跑步数据界面
void gui_sport_cycling_data_paint(void)
{
	SetWord_t word = {0};
	//设置背景黑色
	LCD_SetBackgroundColor(LCD_BLACK);

	//圆点索引
    gui_sport_index_circle_paint(0, GUI_SPORT_CYCLING_SCREEN_MAX, 1);

	
	//第一栏
	gui_sport_cycling_data_detail_paint(SPORT_DATA_DISPALY_INDEX_UP_CIRCLE,m_cycling_data_display_buf[0]);
	
	//分割线
	word.x_axis = 80;
	LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,LCD_AUQAMARIN,0,0,LCD_FILL_ENABLE);

	//第二栏
	gui_sport_cycling_data_detail_paint(SPORT_DATA_DISPALY_INDEX_MID_GPS,m_cycling_data_display_buf[1]);

	//分割线
	word.x_axis = 160;	
	LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,LCD_AUQAMARIN,0,0,LCD_FILL_ENABLE);

	//第三栏
	gui_sport_cycling_data_detail_paint(SPORT_DATA_DISPALY_INDEX_DOWN_NOHINT,m_cycling_data_display_buf[2]);

	
	//GPS状态,防止遮挡，采用靠左位置
	gui_sport_gps_status(SPORT_GPS_LEFT1);

}

//骑行心率界面
void gui_sport_cycling_heart_paint(void)
{
	//设置背景黑色
	LCD_SetBackgroundColor(LCD_BLACK);
	//圆点索引
    gui_sport_index_circle_vert_paint(1, 3);
	
	//心率值
	gui_sport_heart_1_paint(Get_Sport_Heart(),1);
	
}

//跑步实时时间界面
void gui_sport_cycling_realtime_paint(void)
{
	//设置背景黑色
	LCD_SetBackgroundColor(LCD_BLACK);
	
	//圆点索引
    gui_sport_index_circle_paint(2, GUI_SPORT_CYCLING_SCREEN_MAX, 1);
	
	//实时时间
	gui_sport_realtime_paint(1);
		
}

	


//运动计时
static void gui_sport_cycling_1_time(void)
{
	SetWord_t word = {0};
	char str[10] = {0};
	
#if defined WATCH_SIM_SPORT
	word.x_axis = 200;
	word.size = LCD_FONT_20_SIZE;
#else
	word.x_axis = 176;
	word.size = LCD_FONT_24_SIZE;
#endif
	word.y_axis = LCD_CENTER_JUSTIFIED;
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

//剩余距离
static void gui_sport_cycling_1_distance(uint8_t type, uint32_t distance)
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
	word.forecolor = LCD_BLACK;
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
static void gui_sport_cycling_1_track(uint16_t angle)
{
	//需判断是否是循迹返航
	
	
	//指向箭头
	//gui_sport_arrows(angle, LCD_AQUA);
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

//骑行界面1
 void gui_sport_cycling_track_paint(void)
{
	static uint8_t last_status = 2;
	
	//显示黑色背景
	LCD_SetBackgroundColor(LCD_WHITE);
    //轨迹
	gui_sport_cycling_1_track(0);
	
	//圆点索引
	gui_sport_index_circle_vert_paint(2, 3);
	
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
			
			gui_sport_cycling_1_distance(2, 0);
		}
		else if((last_status == 1) && (TRACE_BACK_DATA.whether_in_track == 0))
		{
			//未在轨迹上
			timer_notify_motor_start(300,300,1,false,NOTIFY_MODE_SPORTS);
			timer_notify_buzzer_start(300,300,1,false,NOTIFY_MODE_SPORTS);
			
			gui_sport_cycling_1_distance(3, 0);
		}
		else
		{

			if(TRACE_BACK_DATA.sport_traceback_flag)
			{
				//计时
				gui_sport_cycling_1_time();
			}
			else
			{
				if(TRACE_BACK_DATA.whether_in_track)
				{
					//剩余距离
					gui_sport_cycling_1_distance(0, TRACE_BACK_DATA.remaining_distance);
				}
				else
				{
					//距离轨迹
					gui_sport_cycling_1_distance(1, TRACE_BACK_DATA.nearest_distance);
				}
			}
		}
		
		last_status = TRACE_BACK_DATA.whether_in_track;
	}
	else
	{
		//计时
		gui_sport_cycling_1_time();
		
		last_status = 2;
	}
	
#if !defined WATCH_SIM_SPORT
	//时间和电量
//	gui_sport_realtime_battery(1);
#endif
	
	//指北针
	gui_sport_compass(0);
	
	//GPS状态
  	if((g_sport_status == SPORT_STATUS_PAUSE)&&(ScreenState!=DISPLAY_SCREEN_REMIND_PAUSE))
			{
				//表示暂停状态
		     LCD_SetPicture(110,200,LCD_MAROON,LCD_NONE,&img_sport_pause_cod);//右上角箭头,指示ok键
		      
			  //gui_sport_gps_status(SPORT_GPS_RIGHT_UP_6);
				
			}
		else
			{
	        gui_sport_gps_status(SPORT_GPS_RIGHT_UP_6);
			}

}


//骑行界面
void gui_sport_cycling_paint(void)
{

   
   gui_sport_type = 1;
	if(m_gui_sport_cycling_index == GUI_SPORT_CYCLING_SCREEN_1)
	{
		//数据显示
		gui_sport_data_display_preview_paint(SPORT_DATA_SHOW);
	}
	else if(m_gui_sport_cycling_index == GUI_SPORT_CYCLING_SCREEN_2)
	{
		//心率、电量
		//gui_sport_cycling_heart_paint();
		
		gui_sport_heart_paint(1);
	}

	else
	{
		//轨迹
		gui_sport_cycling_track_paint();
	}


	if(g_sport_status == SPORT_STATUS_PAUSE)
	{
		//暂时增加红色圆圈，表示暂停状态
		//LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_RED);
	}
}

//暂停计时
/*static void gui_sport_cycling_pause_time(void)
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
}*/

//运动时间
/*static void gui_sport_cycling_time(void)
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
}*/

//暂停菜单选项
static void gui_sport_cycling_pause_options(uint8_t index)
{
	SetWord_t word = {0};
	
	//菜单第一项
	word.x_axis = 120 + 18;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString((char *)ClimbPauseStrs[SetValue.Language][index],&word);
	
	//菜单第二项
	if(index >= GUI_SPORT_CYCLING_PAUSE_CANCEL)
	{
		index = GUI_SPORT_CYCLING_PAUSE_CONTINUE;
	}
	else
	{
		index += 1;
		if((TRACE_BACK_DATA.is_traceback_flag) && (index == GUI_SPORT_CYCLING_PAUSE_TRACKBACK))
		{
			//如果是在循迹返航过程，则无循迹返航选项
			index = GUI_SPORT_CYCLING_PAUSE_CANCEL;
		}
	}
	word.x_axis = 180 + 18;
	word.forecolor = LCD_BLACK;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString((char *)ClimbPauseStrs[SetValue.Language][index],&word);
}

//暂停菜单界面
void gui_sport_cycling_pause_paint(void)
{
	LCD_SetBackgroundColor(LCD_BLACK);
	
	//暂停计时
	gui_sport_pause_time();
	
	//运动计时
	gui_sport_pause_acttime();
	
	LCD_SetRectangle(120 - 1 ,122,0,240,LCD_WHITE,0,0,LCD_FILL_ENABLE);
	LCD_SetRectangle(120 + 60 - 1,2,0,240,LCD_BLACK,0,0,LCD_FILL_ENABLE);
	
		//选项
	gui_sport_cycling_pause_options(m_gui_sport_cycling_pause_index);
	
	
	//下翻提示
	gui_sport_page_prompt(SPORT_PAGE_DOWN);
	
	LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_RED);

}

//放弃菜单选项
/*static void gui_sport_cycling_cancel_options(uint8_t index)
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
	if(index >= GUI_SPORT_CYCLING_PAUSE_CANCEL_YES)
	{
		index = GUI_SPORT_CYCLING_PAUSE_CANCEL_NO;
	}
	else
	{
		index += 1;
	}
	word.x_axis = 183;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString((char *)CancelStrs[SetValue.Language][index],&word);
}*/

//放弃菜单界面
void gui_sport_cycling_cancel_paint(void)
{
	SetWord_t word = {0};
		
	//上半部分为白色背景
	LCD_SetRectangle(0, 120, 0, LCD_PIXEL_ONELINE_MAX, LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
		
	//放弃?
	word.x_axis = 50;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 0;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString((char *)RunPauseStrs[SetValue.Language][3],&word);
	word.y_axis = 112 + 32 + 2;
	LCD_SetString("?",&word);
		
	//下半部分为黑色背景
	LCD_SetRectangle(120, 120, 0, LCD_PIXEL_ONELINE_MAX, LCD_WHITE, 0, 0, LCD_FILL_ENABLE);
		
		//选项
	gui_sport_cancel_options(m_gui_sport_cycling_pause_index);
	

	
		
	//分割线
	LCD_SetRectangle(170, 1, 0, LCD_LINE_CNT_MAX, LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
		
	//下翻提示
	gui_sport_page_prompt(SPORT_PAGE_DOWN);	
}

//详细数据界面1
static void gui_sport_cycling_detail_1_paint(uint8_t backgroundcolor)
	{
	   SetWord_t word = {0};
	   char str[20];
	   char *colon = ":";
	   uint8_t charlen;
	   
	   uint32_t second;
	
		//计算时长
		second = ActivityData.ActTime;//get_time_diff(ActivityData.Act_Start_Time, ActivityData.Act_Stop_Time) - Get_PauseTotolTime();
	
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
		word.x_axis = 16;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_24_SIZE;
		word.bckgrndcolor = LCD_NONE;
		LCD_SetString("骑行", &word);
	
	
		
	   //结束时间
		memset(str,0,20);
		sprintf(str,"%02d-%02d-%02d %02d:%02d",ActivityData.Act_Start_Time.Year+2000,ActivityData.Act_Start_Time.Month,ActivityData.Act_Start_Time.Day,ActivityData.Act_Start_Time.Hour,ActivityData.Act_Start_Time.Minute);
		word.x_axis = 40+8;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_13_SIZE;
		word.bckgrndcolor = LCD_NONE;
		word.kerning = 0;
		LCD_SetNumber(str,&word);
	
	
		
	
		//距离
		charlen = strlen(sportdetailstrsgod[SetValue.Language][0]);
		word.x_axis = 120 - 4 - 19 - 4 - 16;
		word.y_axis = 35;
		word.size = LCD_FONT_16_SIZE;
		word.kerning = 0;
		LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][0],&word);
		//:
		word.y_axis += charlen*8 + 1;
		LCD_SetString(colon,&word);
		memset(str,0,20);
//	#if defined (SPRINTF_FLOAT_TO_INT)
		sprintf(str,"%d.%02d",ActivityData.ActivityDetails.CyclingDetail.Distance/100000,ActivityData.ActivityDetails.CyclingDetail.Distance/1000%100);
	//#else
//		sprintf(str,"%0.2f",ActivityData.ActivityDetails.CyclingDetail.Distance/100000.f);
//	#endif
		word.x_axis = 120 - 4 - 19;
		word.y_axis = 35;
		word.kerning = 1;
		word.size = LCD_FONT_19_SIZE;
		LCD_SetNumber(str,&word);
	
		
		word.x_axis = 120 - 4 - 16;
		word.y_axis = 35 + strlen(str)*10 - 0.5*10 + 8;
		word.kerning = 0;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString("km",&word);
	
	
	//时长	   
		charlen = strlen(sportdetailstrsgod[SetValue.Language][1]);
		word.x_axis = 120 - 4 - 19 - 4 - 16;
		word.y_axis = 130;
		word.kerning = 0;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][1],&word);
	   word.y_axis += charlen*8 + 1;
		LCD_SetString(colon,&word);
	
	
		
		word.x_axis = 120 - 4 - 19;
		word.y_axis = 130;
		word.size = LCD_FONT_19_SIZE;
		memset(str,0,20);
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
		word.kerning = 1;
		LCD_SetNumber(str,&word);
	
	
	
	//平均速度
		charlen = strlen(sportdetailstrsgod[SetValue.Language][14]);
		word.x_axis =  120 + 4 + 12;
		word.y_axis = 35;
		word.kerning = 0;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][14],&word);
		
		word.y_axis += charlen*8+1;
		LCD_SetString(colon,&word);
		
		memset(str,0,20);
		sprintf(str,"%d.%01d",(ActivityData.ActivityDetails.CyclingDetail.Speed/10),(ActivityData.ActivityDetails.CyclingDetail.Speed%10));
		word.x_axis =  120 + 4 + 12 + 16 + 4;
		word.y_axis = 35;
		word.kerning = 1;
		word.size = LCD_FONT_19_SIZE;
		LCD_SetNumber(str,&word);
	
		
		word.x_axis =  120 + 4 + 12 + 16 + 4 + 19 - 16;
		word.y_axis += strlen(str)*10 - 0.5*10 + 8;
		word.kerning = 0;
	   word.size = LCD_FONT_16_SIZE;
		LCD_SetString("km/h",&word);
	
	   
	   //平均心率
		charlen = strlen(sportdetailstrsgod[SetValue.Language][3]);
		word.x_axis = 120 + 4 + 12;
		word.y_axis = 130;
		word.kerning = 0;
		
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][3],&word);
		word.y_axis += charlen*8+1;
		LCD_SetString(colon,&word);
		
		memset(str,0,20);
		sprintf(str,"%d",ActivityData.ActivityDetails.CyclingDetail.AvgHeart);
		
		word.x_axis = 120 + 4 + 12 + 16 + 4;
		word.y_axis = 130;
		word.kerning = 1;
		
		word.size = LCD_FONT_19_SIZE;
		LCD_SetNumber(str,&word);
		word.y_axis += strlen(str)*10  + 8;
		word.kerning = 0;
	
		
		word.x_axis =  120 + 4 + 12 + 16 + 4 + 19 - 16;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString("bpm",&word);
	
	
		word.x_axis = 163+36;//框坐标
		LCD_SetRectangle(word.x_axis,4+8+4,0,240,LCD_MAROON,0,0,LCD_FILL_ENABLE);
	   
	   LCD_SetPicture(word.x_axis+4, LCD_CENTER_JUSTIFIED, LCD_WHITE, LCD_NONE, &Img_Pointing_Down_12X8);
	

}


//详细数据界面2
static void gui_sport_cycling_detail_2_paint(uint8_t backgroundcolor)
	{
	   SetWord_t word = {0};
	   char str[20];
	   char *colon = ":";
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
	  
		LCD_SetPicture(10, LCD_CENTER_JUSTIFIED, LCD_BLACK, LCD_NONE, &Img_Pointing_Up_12X8);
		word.bckgrndcolor = LCD_NONE;
		//卡路里
		charlen = strlen(sportdetailstrsgod[SetValue.Language][4]);
		word.x_axis = 120 - 4 - 19 - 4 - 16 - 16 - 18;
		word.y_axis = 35;
		word.size = LCD_FONT_16_SIZE;
		word.kerning = 0;
		LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][4],&word);
		word.y_axis += charlen*8 + 1;
		LCD_SetString(colon,&word);
		memset(str,0,20);
		sprintf(str,"%d",ActivityData.ActivityDetails.CyclingDetail.Calorie/1000);
		word.x_axis = 120 - 4 - 19 - 16 - 18;
		word.y_axis = 35;
		word.kerning = 1;
		word.size = LCD_FONT_19_SIZE;
		LCD_SetNumber(str,&word);
		word.x_axis = 120 - 4 - 16 - 16 - 18;
		word.y_axis = 35 + strlen(str)*10 - 0.5*10 + 8;
		word.kerning = 0;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString("kcal",&word);
	
	
		//最大速度
		charlen = strlen(sportdetailstrsgod[SetValue.Language][15]);
		word.x_axis = 120 - 4 - 19 - 4 - 16 - 16 - 18;
		word.y_axis = 130;
		word.kerning = 0;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][15],&word);
		word.y_axis += charlen*8 + 1;
		LCD_SetString(colon,&word);
		memset(str,0,20);
		
		sprintf(str,"%.1f",ActivityData.ActivityDetails.CyclingDetail.OptimumSpeed/10.f);
		word.x_axis = 120 - 4 - 19 - 16 - 18;
		word.y_axis = 130;
		word.size = LCD_FONT_19_SIZE;
		word.kerning = 1;
		LCD_SetNumber(str,&word);		
		word.x_axis = 120 - 4 - 16 - 16 - 18;
		word.y_axis += strlen(str)*10 - 0.5*10 + 8;
	    word.kerning = 0;		
		word.size = LCD_FONT_16_SIZE;
	    LCD_SetString("km/h",&word);
		#ifdef COD 
		//最高海拔
		charlen = strlen(sportdetailstrsgod[SetValue.Language][47]);
		 word.x_axis =	120 - 18;
		word.y_axis = 35;
		word.kerning = 0;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][47],&word);
		word.y_axis += charlen*8+1;
		LCD_SetString(colon,&word);
		memset(str,0,20);
		sprintf(str,"%d",(ActivityData.ActivityDetails.CyclingDetail.MaxAlt));//先用上升高度代替
		 word.x_axis =	120  + 16 + 4 - 18;
		word.y_axis = 35;
		word.kerning = 1;
		word.size = LCD_FONT_19_SIZE;
		LCD_SetNumber(str,&word);
		word.x_axis =  120	+ 16 + 4 + 19 - 16 - 18;
		word.y_axis += strlen(str)*10 - 0.5*10 + 8;
		word.kerning = 0;
	   word.size = LCD_FONT_16_SIZE;
		LCD_SetString("m",&word);
		#endif

	
		//累积爬升
		charlen = strlen(sportdetailstrsgod[SetValue.Language][8]);
		word.x_axis = 120 - 18;
		word.y_axis = 130;
		word.kerning = 0;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][8],&word);
		
		word.y_axis += charlen*8+1;
		LCD_SetString(colon,&word);
		
		memset(str,0,20);
		sprintf(str,"%d",ActivityData.ActivityDetails.CyclingDetail.RiseHeight);
		word.x_axis = 120 + 16 + 4 - 18;
		word.y_axis = 130;
		word.kerning = 1;
		word.size = LCD_FONT_19_SIZE;
		LCD_SetNumber(str,&word);
		
		word.y_axis += strlen(str)*10  + 8;
		word.kerning = 0;
		word.x_axis =  120	+ 16 + 4 + 19 - 16 - 18;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString("m",&word);
	
		#ifdef COD 
	
		//海拔上升
		charlen = strlen(sportdetailstrsgod[SetValue.Language][20]);
		word.x_axis =  120 + 4 + 16 + 4 - 16 + 19 + 32 - 18; 
		word.y_axis = 35;
		word.kerning = 0;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][20],&word);
		word.y_axis += charlen*8+1;
		LCD_SetString(colon,&word);
	
		memset(str,0,20);
		sprintf(str,"%d",ActivityData.ActivityDetails.CyclingDetail.RiseAlt);
		word.x_axis =  120 + 4 + 16 + 4 - 16 + 19 + 32 - 18 + 16 + 4;
		word.y_axis = 35;
		word.kerning = 1;
		word.size = LCD_FONT_19_SIZE;
		LCD_SetNumber(str,&word);
		
		 word.x_axis = 120 + 4 + 16 + 4 - 16 + 19 + 32 - 18 + 16 + 4 + 19 - 16;
		 word.y_axis += strlen(str)*10	+ 8;
		 word.kerning = 0;
		 word.size = LCD_FONT_16_SIZE;
		 LCD_SetString("m",&word);
	


        //上坡里程
		charlen = strlen(sportdetailstrsgod[SetValue.Language][48]);
		word.x_axis =  120 + 4 + 16 + 4 - 16 + 19 + 32 - 18; 
		word.y_axis = 130;
		word.kerning = 0;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][48],&word);
		word.y_axis += charlen*8+1;
		LCD_SetString(colon,&word);
	
		memset(str,0,20);
		sprintf(str,"%d.%02d",ActivityData.ActivityDetails.CyclingDetail.UphillDistance/100000,ActivityData.ActivityDetails.CyclingDetail.UphillDistance/1000%100);
		word.x_axis =  120 + 4 + 16 + 4 - 16 + 19 + 32 - 18 + 16 + 4;
		word.y_axis = 130;
		word.kerning = 1;
		word.size = LCD_FONT_19_SIZE;
		LCD_SetNumber(str,&word);
		
		word.x_axis = 120 + 4 + 16 + 4 - 16 + 19 + 32 - 18 + 16 + 4 + 19 - 16;
		 word.y_axis += strlen(str)*10	+ 8;
		 word.kerning = 0;
		 word.size = LCD_FONT_16_SIZE;
		 LCD_SetString("km",&word);
		#endif
		LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, LCD_BLACK, LCD_NONE, &Img_Pointing_Down_12X8);
		
	 
}


//详细数据界面3
/*static void gui_sport_cycling_detail_3_paint(uint8_t backgroundcolor)
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

	//有氧效果
	charlen = strlen(SportDetailStrs[SetValue.Language][7]);
	word.x_axis = 60;
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
	sprintf(str,"%d.%01d",ActivityData.ActivityDetails.CyclingDetail.AerobicEffect/10,ActivityData.ActivityDetails.CyclingDetail.AerobicEffect%10);
	#else
    sprintf(str,"%.1f",ActivityData.ActivityDetails.CyclingDetail.AerobicEffect/10.f);		//保留一位小数点
    #endif
    LCD_SetNumber(str,&word);
	
    //无氧效果
	charlen = strlen(SportDetailStrs[SetValue.Language][8]);
	word.x_axis += 30;
    word.y_axis = 95 - charlen*8;
	word.kerning = 0;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][8],&word);
	word.y_axis = 98;
    LCD_SetString(":",&word);
	
	word.y_axis = 110;
    word.kerning = 1;
    memset(str,0,sizeof(str));
	#if defined (SPRINTF_FLOAT_TO_INT)
	sprintf(str,"%d.%01d",ActivityData.ActivityDetails.CyclingDetail.AnaerobicEffect/10,ActivityData.ActivityDetails.CyclingDetail.AnaerobicEffect%10);
	#else
    sprintf(str,"%.1f",ActivityData.ActivityDetails.CyclingDetail.AnaerobicEffect/10.f);	//保留一位小数点
    #endif
    LCD_SetNumber(str,&word);
	
//	//恢复时间
//	charlen = strlen(SportDetailStrs[SetValue.Language][9]);
//	word.x_axis += 30;
//	word.y_axis = 95 - charlen*8;
//	word.kerning = 0;
//	LCD_SetString((char *)SportDetailStrs[SetValue.Language][9],&word);
//	word.y_axis = 98;
//	LCD_SetString(":",&word);
//	
//	word.y_axis = 110;
//	word.kerning = 1;
//	memset(str,0,sizeof(str));
//	sprintf(str,"%.f",ActivityData.ActivityDetails.CyclingDetail.RecoveryTime/60.f);
//	LCD_SetNumber(str,&word);
//	
//	word.y_axis = 160;
//	word.kerning = 0;
//	LCD_SetString("h",&word);
	
	//下翻提示
	gui_sport_page_prompt(SPORT_PAGE_DOWN);
}*/

//运动轨迹界面
static void gui_sport_cycling_detail_track_paint(void)
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
//详细数据界面
void gui_sport_cycling_detail_paint(void)
{
	switch(m_gui_sport_cycling_pause_index)
	{
		case GUI_SPORT_CYCLING_DETAIL_1:
			gui_sport_cycling_detail_1_paint(LCD_WHITE);
			break;
		case GUI_SPORT_CYCLING_DETAIL_2:
			gui_sport_cycling_detail_2_paint(LCD_WHITE);
			break;
		/*case GUI_SPORT_CYCLING_DETAIL_3:
			gui_sport_cycling_detail_3_paint(LCD_BLACK);
			break;*/
		case GUI_SPORT_CYCLING_DETAIL_TRACK:
			gui_sport_detail_track_paint(LCD_WHITE);
			break;
		default:
		//gui_sport_detail_heartratezone_paint(LCD_BLACK);

			break;
	}
}

//void gui_sport_cycling_feel_paint(void)
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
//	gui_sport_feel_options_paint(m_gui_sport_cycling_pause_index);
//	
//	//分割线
//	LCD_SetRectangle(160, 1, 0, LCD_LINE_CNT_MAX, LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);

//	//下翻提示
//	gui_sport_page_prompt(SPORT_PAGE_DOWN);
//}

void gui_sport_cycling_init(void)
{
	//默认为骑行界面1
	m_gui_sport_cycling_index = GUI_SPORT_CYCLING_SCREEN_1;
	
#if defined WATCH_SIM_SPORT
	m_gui_sport_cycling_last_circle_time = 0;
#endif
}

//骑行界面按键事件
void gui_sport_cycling_btn_evt(uint32_t Key_Value)
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
				
				ScreenState = DISPLAY_SCREEN_CYCLING_PAUSE;
				m_gui_sport_cycling_pause_index = GUI_SPORT_CYCLING_PAUSE_CONTINUE;
//#if defined WATCH_SIM_SPORT
				Save_Detail_Index = ScreenState;
				ScreenState = DISPLAY_SCREEN_REMIND_PAUSE;
				timer_app_pasue_start();
//#else
				//msg.cmd = MSG_DISPLAY_SCREEN;
				//xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				
//#endif

				
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
				Save_Detail_Index = ScreenState;
				timer_app_pasue_start();
				//振动提示
				gui_sport_motor_prompt();
			}
			break;
		case KEY_UP:
			if(g_sport_status != SPORT_STATUS_READY)
			{
				//翻页查看马拉松界面
				if(m_gui_sport_cycling_index > GUI_SPORT_CYCLING_SCREEN_1)
				{
					m_gui_sport_cycling_index--;
				}
				else
				{
					m_gui_sport_cycling_index = GUI_SPORT_CYCLING_SCREEN_MAX - 1;
				}
				
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
			break;
		case KEY_DOWN:
			if(g_sport_status != SPORT_STATUS_READY)
			{
				//翻页查看骑行界面
				if(m_gui_sport_cycling_index < (GUI_SPORT_CYCLING_SCREEN_MAX - 1))
				{
					m_gui_sport_cycling_index++;
				}
				else
				{
					m_gui_sport_cycling_index = GUI_SPORT_CYCLING_SCREEN_1;
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
				ScreenState = DISPLAY_SCREEN_CYCLING_PAUSE;
				m_gui_sport_cycling_pause_index = GUI_SPORT_CYCLING_PAUSE_CONTINUE;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
			break;
		case KEY_LONG_UP:
			if(g_sport_status == SPORT_STATUS_START)
			{
				//轨迹界面保存航迹打点数据			
#if defined WATCH_SIM_SPORT
				if(m_gui_sport_cycling_index == GUI_SPORT_CYCLING_SCREEN_4)
#else
				if(m_gui_sport_cycling_index == GUI_SPORT_CYCLING_SCREEN_1)
#endif
				{	
					sport_dottrack();

				}	
			}
			break;
		default:
			break;
	}

	gui_sport_index = m_gui_sport_cycling_index;
	
	gui_sport_type = 1;
}

//暂停菜单界面按键事件
void gui_sport_cycling_pause_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
			if(m_gui_sport_cycling_pause_index == GUI_SPORT_CYCLING_PAUSE_CONTINUE)
			{
				//继续计时，开始运动
				g_sport_status = SPORT_STATUS_START;
				Set_PauseSta(false);
				
				ScreenState = DISPLAY_SCREEN_CYCLING;
				gui_swich_msg();
#if defined WATCH_SIM_SPORT
				Save_Detail_Index = ScreenState;
				timer_app_pasue_start();
#endif
				//振动提示
				gui_sport_motor_prompt();
			}
			else if(m_gui_sport_cycling_pause_index == GUI_SPORT_CYCLING_PAUSE_SAVE)
			{
				//结束运动并保存数据
				g_sport_status = SPORT_STATUS_READY;
				
				//默认选项
				gui_sport_feel_init();
				
				ScreenState = DISPLAY_SCREEN_SPORT_FEEL;
				m_gui_sport_cycling_pause_index = GUI_SPORT_CYCLING_DETAIL_1;
				sport_end_flag = 1;
				sport_end_hrvalue = get_10s_heartrate(); //获取前10s平均心率
				g_HeartNum = 0;
				extern volatile ScreenState_t Save_Detail_Index;
				Save_Detail_Index = DISPLAY_SCREEN_CYCLING_DETAIL;
				gui_swich_msg();
			}
			else if(m_gui_sport_cycling_pause_index == GUI_SPORT_CYCLING_PAUSE_WAIT)
			{
				m_gui_sport_cycling_pause_index = GUI_SPORT_CYCLING_PAUSE_CONTINUE;
				
				//稍后继续
				sport_continue_later_start();
				
				//稍后继续保存运动类型
				ContinueLaterDataSave.SportType = DISPLAY_SCREEN_CYCLING;
				
				//稍后继续，退出运动界面，返回主界面
				ScreenState = DISPLAY_SCREEN_HOME;
				gui_swich_msg();
			}
			else if(m_gui_sport_cycling_pause_index == GUI_SPORT_CYCLING_PAUSE_TRACKBACK)
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
				
				ScreenState = DISPLAY_SCREEN_CYCLING;
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
				ScreenState = DISPLAY_SCREEN_CYCLING_CANCEL;
				
				m_gui_sport_cycling_pause_index = GUI_SPORT_CYCLING_PAUSE_CANCEL_NO;
				gui_swich_msg();
			}
			
			//msg.cmd = MSG_DISPLAY_SCREEN;
			//xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		case KEY_UP:
			if(m_gui_sport_cycling_pause_index > GUI_SPORT_CYCLING_PAUSE_CONTINUE)
			{
				m_gui_sport_cycling_pause_index--;
				if((TRACE_BACK_DATA.is_traceback_flag) && (m_gui_sport_cycling_pause_index == GUI_SPORT_CYCLING_PAUSE_TRACKBACK))
				{
					//如果是在循迹返航过程，则无循迹返航选项
					m_gui_sport_cycling_pause_index = GUI_SPORT_CYCLING_PAUSE_WAIT;
				}
			}
			else
			{
				m_gui_sport_cycling_pause_index = GUI_SPORT_CYCLING_PAUSE_CANCEL;
			}
			
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		    break;
		case KEY_DOWN:
			if(m_gui_sport_cycling_pause_index < GUI_SPORT_CYCLING_PAUSE_CANCEL)
			{
				m_gui_sport_cycling_pause_index++;
				if((TRACE_BACK_DATA.is_traceback_flag) && (m_gui_sport_cycling_pause_index == GUI_SPORT_CYCLING_PAUSE_TRACKBACK))
				{
					//如果是在循迹返航过程，则无循迹返航选项
					m_gui_sport_cycling_pause_index = GUI_SPORT_CYCLING_PAUSE_CANCEL;
				}
			}
			else
			{
				m_gui_sport_cycling_pause_index = GUI_SPORT_CYCLING_PAUSE_CONTINUE;
			}
			
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		case KEY_BACK:
			//返回骑行界面
			ScreenState = DISPLAY_SCREEN_CYCLING;
		
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}

//放弃菜单界面按键事件
void gui_sport_cycling_cancel_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
			if(m_gui_sport_cycling_pause_index == GUI_SPORT_CYCLING_PAUSE_CANCEL_NO)
			{
				//取消放弃运动，返回上级菜单界面
				ScreenState = DISPLAY_SCREEN_CYCLING_PAUSE;
				m_gui_sport_cycling_pause_index = GUI_SPORT_CYCLING_PAUSE_CANCEL;
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
			if(m_gui_sport_cycling_pause_index == GUI_SPORT_CYCLING_PAUSE_CANCEL_NO)
			{
				m_gui_sport_cycling_pause_index = GUI_SPORT_CYCLING_PAUSE_CANCEL_YES;
			}
			else
			{
				m_gui_sport_cycling_pause_index = GUI_SPORT_CYCLING_PAUSE_CANCEL_NO;
			}

			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		    break;
		case KEY_BACK:
			//返回上级菜单
			ScreenState = DISPLAY_SCREEN_CYCLING_PAUSE;
			m_gui_sport_cycling_pause_index = GUI_SPORT_CYCLING_PAUSE_CANCEL;
			
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}

//详细数据界面按键事件
void gui_sport_cycling_detail_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_UP:
			//上翻
			if(m_gui_sport_cycling_pause_index > GUI_SPORT_CYCLING_DETAIL_1)
			{
				m_gui_sport_cycling_pause_index--;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
		    break;
		case KEY_DOWN:
			//下翻
			if(m_gui_sport_cycling_pause_index < GUI_SPORT_CYCLING_DETAIL_TRACK)
			{
				m_gui_sport_cycling_pause_index++;
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


