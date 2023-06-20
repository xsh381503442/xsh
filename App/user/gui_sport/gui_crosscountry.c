//Author: Jason Gong
//Time：20180206 V1.10
/*All Crosscountry Interface and Display are individual functions, except some common variables and 
pre-defined functions/data strings from task_sport.c task_sport.h, task_display.c, task_display.h 
gui_sport.c and com_sport.h. The first stage is all about drawing and displays   V1.10    */

/*All Crosscountry Data has been added into the system from com_sport.c, it also includes the determination 
of pause status,sport status,and continue afterward. The second stage is about data handling V1.40    */

#include "gui_sport_config.h"
#include "gui_sport.h"

#include "gui_crosscountry.h"

#include "bsp_timer.h"
#include "drv_lcd.h"

#include "task_sport.h"

#include "com_data.h"
#include "com_sport.h"
#include "font_config.h"
#include "drv_key.h"
#include "drv_heartrate.h"
#include "drv_battery.h"
#include "time_progress.h"
#include "algo_hdr.h"
#include "task_gps.h"
#include "gui_run.h"
#include "gui_tool_gps.h"
#include "gui.h"
#include "time_notify.h"
#include "timer_traceback.h"
#include "algo_hdr.h"
#include "timer_app.h"
#include "gui_sport_crosscountry_hike.h"

#if DEBUG_ENABLED == 1 && GUI_SPORT_CROSSCOUNTRY_LOG_ENABLED == 1
	#define GUI_SPORT_CROSSCOUNTRY_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_SPORT_CROSSCOUNTRY_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__) 
#else
	#define GUI_SPORT_CROSSCOUNTRY_WRITESTRING(...)
	#define GUI_SPORT_CROSSCOUNTRY_PRINTF(...)		          
#endif
/*越野跑数据显示当前项*/
 uint8_t m_crosscountry_data_display_buf[SPORT_DATA_DISPLAY_SUM]={SPORT_DATA_DISPALY_TOTAL_DISTANCE,SPORT_DATA_DISPALY_REALTIME_PACE,SPORT_DATA_DISPALY_TOTAL_CLIMB,SPORT_DATA_DISPALY_TOTAL_TIME};
/*越野跑数据显示选项*/
const uint8_t m_crosscountry_data_display_option[CROSSCOUNTRY_DATA_DISPLAY_SUM]= {SPORT_DATA_DISPALY_TOTAL_DISTANCE,SPORT_DATA_DISPALY_REALTIME_PACE,SPORT_DATA_DISPALY_TOTAL_CLIMB,SPORT_DATA_DISPALY_TOTAL_TIME,SPORT_DATA_DISPALY_AVE_PACE,
SPORT_DATA_DISPALY_TOTAL_STEP,SPORT_DATA_DISPALY_REALTIME_HEART,SPORT_DATA_DISPALY_REALTIME_FREQUENCY};

/*越野跑提醒设置选项*/
const uint8_t m_crosscountry_remind_set_option[CROSSCOUNTRY_REMIND_SET_SUM]={SPORT_REMIND_SET_HAERT,SPORT_REMIND_SET_HRRECOVERY,SPORT_REMIND_SET_DISTANCE,SPORT_REMIND_SET_PACE,SPORT_REMIND_SET_TIME,SPORT_REMIND_SET_AUTOCIRCLE};

extern uint8_t gui_sport_index;
	
extern uint8_t 	gui_sport_type;


//static bool cancel_index;//放弃选项索引(否,是)


#define	GUI_SPORT_CROSSCOUNTRY_SCREEN_DATA			0	//越野跑数据界面
#define	GUI_SPORT_CROSSCOUNTRY_SCREEN_HEART			1	//越野跑心率界面
#define	GUI_SPORT_CROSSCOUNTRY_SCREEN_TRACK			2	///越野跑轨迹界面

#define GUI_SPORT_CROSSCOUNTRY_SCREEN_MAX			3


#define	GUI_SPORT_CROSSCOUNTRY_PAUSE_CONTINUE	0	//继续
#define	GUI_SPORT_CROSSCOUNTRY_PAUSE_SAVE		1	//结束运动
#define	GUI_SPORT_CROSSCOUNTRY_PAUSE_WAIT		2	//稍后继续
#define	GUI_SPORT_CROSSCOUNTRY_PAUSE_TRACKBACK	3	//循迹返航
#define	GUI_SPORT_CROSSCOUNTRY_PAUSE_CANCEL		4	//放弃

#define	GUI_SPORT_CROSSCOUNTRY_PAUSE_CANCEL_NO	0	//否
#define	GUI_SPORT_CROSSCOUNTRY_PAUSE_CANCEL_YES	1	//是


#define	GUI_SPORT_CROSSCOUNTRY_DETAIL_1				0	//详细数据界面1
#define	GUI_SPORT_CROSSCOUNTRY_DETAIL_2				1	//详细数据界面2
#define GUI_SPORT_CROSSCOUNTRY_DETAIL_TRACK			2	//轨迹界面
//#define	GUI_SPORT_CROSSCOUNTRY_DETAIL_HEARTRATEZONE	4	//心率区间界面


static uint8_t m_gui_sport_crosscountry_index;		//越野跑界面索引
static uint8_t m_gui_sport_crosscountry_pause_index;	//越野跑暂停菜单界面索引

extern volatile ScreenState_t Save_Detail_Index;
static uint8_t m_gui_crosscountry_cloud_navigation_shink = 0;

//按ok键运动开始提示
void gui_crosscountry_prompt_paint(void)
{
  SetWord_t word = {0};
 // uint8_t x1,y1,x2,y2,x3,y3;

   word.x_axis = 16;
   word.y_axis = 118;
   
   LCD_SetPicture(word.x_axis,word.y_axis,LCD_NONE,LCD_NONE,&Img_start_18X24);

   word.x_axis = 50;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	
	LCD_SetString((char *)SportsPromptStrs[SetValue.Language][0],&word);

}
#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION

//徒步越野、越野跑等有云迹导航时候 图标闪烁
void gui_crosscountry_cloud_navigation_paint(void)
{
	if(m_gui_crosscountry_cloud_navigation_shink == 0)
	{
		m_gui_crosscountry_cloud_navigation_shink = 1;
	}
	else
	{
		m_gui_crosscountry_cloud_navigation_shink = 0;
	}
	if(m_gui_crosscountry_cloud_navigation_shink == 0)
	{
	SetWord_t word = {0};
   word.x_axis = 46;
   word.y_axis = 186;
   
	}
	LCD_SetPicture(word.x_axis,word.y_axis,LCD_NONE,LCD_NONE,&Img_new_cloud_navigation_54X54);
}

#endif

//越野跑心率界面
void gui_sport_crosscountry_heart_paint(void)
	{
			//设置背景黑色
			LCD_SetBackgroundColor(LCD_BLACK);
			//圆点索引
			gui_sport_index_circle_vert_paint(1, 3);
			
			//心率值
			gui_sport_heart_1_paint(Get_Sport_Heart(),1);
			
	}



//越野跑轨迹更新
void gui_crosscountry_track_update_paint(void)
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

//剩余距离
static void gui_sport_crosscountry_1_distance(uint8_t type, uint32_t distance)
{
	gui_sport_every_1_distance(type,distance);
}

static void gui_sport_crosscountry_1_time(void)
{
	SetWord_t word = {0};
	char str[10] = {0};
	
	word.x_axis = 184;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
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


//越野跑轨迹界面
void gui_sport_crosscountry_track_paint(void)
{
		static uint8_t last_status = 2;
	
		//设置背景黑色
		LCD_SetBackgroundColor(LCD_WHITE);
	
		 //轨迹
		gui_crosscountry_track_update_paint();

	    
		gui_sport_index_circle_vert_paint(2, 3);
		//gui_sport_index_circle_paint(1, 4, 1);
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
				
				gui_sport_crosscountry_1_distance(2, 0);
			}
			else if((last_status == 1) && (TRACE_BACK_DATA.whether_in_track == 0))
			{
				//未在轨迹上
				timer_notify_motor_start(300,300,1,false,NOTIFY_MODE_SPORTS);
				timer_notify_buzzer_start(300,300,1,false,NOTIFY_MODE_SPORTS);
				
				gui_sport_crosscountry_1_distance(3, 0);
			}
			else
			{
				if(TRACE_BACK_DATA.sport_traceback_flag)
				{
					//计时
					gui_sport_crosscountry_1_time();
				}
				else
				{
					if(TRACE_BACK_DATA.whether_in_track)
					{
						//剩余距离
						gui_sport_crosscountry_1_distance(0, TRACE_BACK_DATA.remaining_distance);
					}
					else
					{
						//距离轨迹
						gui_sport_crosscountry_1_distance(1, TRACE_BACK_DATA.nearest_distance);
					}
				}
			}
			
			last_status = TRACE_BACK_DATA.whether_in_track;
		}
		else
		{
			//计时
			gui_sport_crosscountry_1_time();
			
			last_status = 2;
		}
		
	//	gui_crosscountry_realtime_battery_paint(205, g_bat_evt.level_percent);
		

		
		
		
		gui_sport_compass(0);
	
  		if((g_sport_status == SPORT_STATUS_PAUSE)&&(ScreenState!=DISPLAY_SCREEN_REMIND_PAUSE))
			{
				//表示暂停状态
		      LCD_SetPicture(110,200,LCD_MAROON,LCD_NONE,&img_sport_pause_cod);//右上角箭头,指示ok键
		      
			 // gui_sport_gps_status(SPORT_GPS_RIGHT_UP_6);
				
			}
		else
			{
	        gui_sport_gps_status(SPORT_GPS_RIGHT_UP_6);
			}
	//	gui_crosscountry_gps_paint();
		
	
				
}


//越野跑实时时间界面
void gui_sport_crosscountry_realtime_paint(void)
{
	//设置背景黑色
	LCD_SetBackgroundColor(LCD_BLACK);
	
	//圆点索引
    gui_sport_index_circle_paint(3, 4, 1);
	
	//实时时间
	//gui_sport_realtime_1_paint(1);
		
}


//越野跑界面
void gui_sport_crosscountry_paint(void)
{
	
	gui_sport_type = 4;
	if(m_gui_sport_crosscountry_index ==GUI_SPORT_CROSSCOUNTRY_SCREEN_DATA )
	{
		
		gui_sport_data_display_preview_paint(SPORT_DATA_SHOW);
	}
	else if (m_gui_sport_crosscountry_index ==GUI_SPORT_CROSSCOUNTRY_SCREEN_HEART)
	{
		//gui_sport_crosscountry_heart_paint();
		
		gui_sport_heart_paint(1);
		
	}
	/*else if (m_gui_sport_crosscountry_index ==GUI_SPORT_CROSSCOUNTRY_SCREEN_TIME)
	{
		gui_sport_crosscountry_realtime_paint();
	}*/
	else
	{
		gui_sport_crosscountry_track_paint();
	}
	
	if(g_sport_status == SPORT_STATUS_PAUSE)
	{
		//暂时增加红色圆圈，表示暂停状态
		//LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_RED);
	}
}


//暂停菜单选项
static void gui_sport_crosscountry_pause_options(uint8_t index)
	{
			SetWord_t word = {0};
		
		//菜单第一项
		word.x_axis = 120 + 18;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.forecolor = LCD_BLACK;
		word.bckgrndcolor = LCD_NONE;
		word.kerning = 0;
		word.size = LCD_FONT_24_SIZE;
		
		LCD_SetString((char *)ClimbPauseStrs[SetValue.Language][index],&word); //都有稍后继续项
		
			//菜单第二项
		if(index >= GUI_SPORT_CROSSCOUNTRY_PAUSE_CANCEL)
		{
			index = GUI_SPORT_CROSSCOUNTRY_PAUSE_CONTINUE;
		}
		else
		{
	
			index += 1;
			if((TRACE_BACK_DATA.is_traceback_flag) && (index == GUI_SPORT_CROSSCOUNTRY_PAUSE_TRACKBACK))
			{
					//如果是在循迹返航过程，则无循迹返航选项
					index = GUI_SPORT_CROSSCOUNTRY_PAUSE_CANCEL;
			}
		}
	
		word.x_axis = 180 + 18;
		word.forecolor = LCD_BLACK;
		word.size = LCD_FONT_16_SIZE;
	
		LCD_SetString((char *)ClimbPauseStrs[SetValue.Language][index],&word); //都有稍后继续项
	}



//越野跑暂停菜单界面
void gui_sport_crosscountry_pause_paint(void)
{
	LCD_SetBackgroundColor(LCD_BLACK);
	
	//暂停计时
	gui_sport_pause_time();
	
	//运动计时
	gui_sport_pause_acttime();
	
	LCD_SetRectangle(120 - 1 ,122,0,240,LCD_WHITE,0,0,LCD_FILL_ENABLE);
	LCD_SetRectangle(120 + 60 - 1,2,0,240,LCD_BLACK,0,0,LCD_FILL_ENABLE);

	
	//选项
	gui_sport_crosscountry_pause_options(m_gui_sport_crosscountry_pause_index);
	
	
	//下翻提示
	gui_sport_page_prompt(SPORT_PAGE_DOWN);

	LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_RED);

	

	


	
	
}

//越野跑放弃菜单界面
void gui_sport_crosscountry_cancel_paint(void)
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
	gui_sport_cancel_options(m_gui_sport_crosscountry_pause_index);

	
		
	//分割线
	LCD_SetRectangle(170, 1, 0, LCD_LINE_CNT_MAX, LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
		
	//下翻提示
	gui_sport_page_prompt(SPORT_PAGE_DOWN);	
	
	
}
//越野跑详细数据界面
void gui_sport_crosscountry_detail_paint(void)
{
	switch(m_gui_sport_crosscountry_pause_index)
	{
		case GUI_SPORT_CROSSCOUNTRY_DETAIL_1:
		
			gui_crosscountry_save_detail_1_paint(LCD_WHITE);
			break;
		case GUI_SPORT_CROSSCOUNTRY_DETAIL_2:
			gui_crosscountry_save_detail_2_paint(LCD_WHITE);
			break;
		
		case GUI_SPORT_CROSSCOUNTRY_DETAIL_TRACK:
			gui_sport_detail_track_paint(LCD_WHITE);
			break;
		
		default:
			break;
	}
}


void gui_sport_crosscountry_init(void)
{
	//默认为跑步数据界面
	m_gui_sport_crosscountry_index = GUI_SPORT_CROSSCOUNTRY_SCREEN_DATA;
}


void gui_sport_crosscountry_btn_evt(uint32_t Key_Value)
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
				
				ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY_PAUSE;
				m_gui_sport_crosscountry_pause_index = GUI_SPORT_CROSSCOUNTRY_PAUSE_CONTINUE;
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
				if(m_gui_sport_crosscountry_index < GUI_SPORT_CROSSCOUNTRY_SCREEN_TRACK)
				{
					m_gui_sport_crosscountry_index++;
				}
				else
				{
					m_gui_sport_crosscountry_index = GUI_SPORT_CROSSCOUNTRY_SCREEN_DATA;
				}
				
				gui_swich_msg();
			}
			break;
		case KEY_UP:
			if (g_sport_status != SPORT_STATUS_READY)
			{
           		//翻页查看跑步界面
				if(m_gui_sport_crosscountry_index > GUI_SPORT_CROSSCOUNTRY_SCREEN_DATA)
				{
					m_gui_sport_crosscountry_index--;
				}
				else
				{
					m_gui_sport_crosscountry_index = GUI_SPORT_CROSSCOUNTRY_SCREEN_TRACK;
				}
				
				gui_swich_msg();
			}
		    break;
		case KEY_BACK:
		{
			
 			if(g_sport_status == SPORT_STATUS_PAUSE)
			{
				//返回暂停菜单界面
				ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY_PAUSE;
				m_gui_sport_crosscountry_index = GUI_SPORT_CROSSCOUNTRY_PAUSE_CONTINUE;
			
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
		}
			break;
		case KEY_LONG_UP:
			if(g_sport_status == SPORT_STATUS_START)
			{
				//轨迹界面保存航迹打点数据
				if(m_gui_sport_crosscountry_index == GUI_SPORT_CROSSCOUNTRY_SCREEN_TRACK)
				{	
					sport_dottrack();

				}	
			}
			break;
		default:
			break;
	}
	gui_sport_index = m_gui_sport_crosscountry_index;
	
	gui_sport_type = 4;
}
//越野跑暂停菜单界面按键事件
void gui_sport_crosscountry_pause_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
			if(m_gui_sport_crosscountry_pause_index == GUI_SPORT_CROSSCOUNTRY_PAUSE_CONTINUE)
			{
				//继续计时，开始运动
				g_sport_status = SPORT_STATUS_START;
				Set_PauseSta(false);
				
				ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY;
				gui_swich_msg();
				Save_Detail_Index = ScreenState;
				timer_app_pasue_start();
				//振动提示
				gui_sport_motor_prompt();
			}
#if defined WATCH_SPORT_EVENT_SCHEDULE
			else if(m_gui_sport_crosscountry_pause_index == GUI_SPORT_CROSSCOUNTRY_PAUSE_SCHEDULE)
			{//赛段结束
				set_sport_pause_event_status(1);
				//结束运动并保存数据
				g_sport_status = SPORT_STATUS_READY;
				
				//默认选项
				gui_sport_feel_init();
				
				ScreenState = DISPLAY_SCREEN_SPORT_FEEL;
				m_gui_sport_crosscountry_pause_index = GUI_SPORT_CROSSCOUNTRY_DETAIL_1;
				sport_end_flag = 1;
				sport_end_hrvalue = get_10s_heartrate(); //获取前10s平均心率
				g_HeartNum = 0;
				extern volatile ScreenState_t Save_Detail_Index;
				Save_Detail_Index = DISPLAY_SCREEN_CROSSCOUNTRY_DETAIL;
				gui_swich_msg();
			}
#endif
			else if(m_gui_sport_crosscountry_pause_index == GUI_SPORT_CROSSCOUNTRY_PAUSE_SAVE)
			{
#if defined WATCH_SPORT_EVENT_SCHEDULE
				set_sport_pause_event_status(0);//赛事结束
#endif
				//结束运动并保存数据
				g_sport_status = SPORT_STATUS_READY;
				
				//默认选项
				gui_sport_feel_init();
				
				ScreenState = DISPLAY_SCREEN_SPORT_FEEL;
				m_gui_sport_crosscountry_pause_index = GUI_SPORT_CROSSCOUNTRY_DETAIL_1;
				sport_end_flag = 1;
				sport_end_hrvalue = get_10s_heartrate(); //获取前10s平均心率
				g_HeartNum = 0;
				extern volatile ScreenState_t Save_Detail_Index;
				Save_Detail_Index = DISPLAY_SCREEN_CROSSCOUNTRY_DETAIL;
				gui_swich_msg();
			}
			else if(m_gui_sport_crosscountry_pause_index == GUI_SPORT_CROSSCOUNTRY_PAUSE_WAIT)
			{
				m_gui_sport_crosscountry_pause_index = GUI_SPORT_CROSSCOUNTRY_PAUSE_CONTINUE;
				
				//稍后继续
				sport_continue_later_start();
				
				//稍后继续保存运动类型
				ContinueLaterDataSave.SportType = DISPLAY_SCREEN_CROSSCOUNTRY;
				
				//稍后继续，退出运动界面，返回主界面
				ScreenState = DISPLAY_SCREEN_HOME;
				gui_swich_msg();
			}
			else if(m_gui_sport_crosscountry_pause_index == GUI_SPORT_CROSSCOUNTRY_PAUSE_TRACKBACK)
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
#if defined WATCH_SPORT_EVENT_SCHEDULE
				//加载的轨迹数据初始化
				InitLoadTrack();
#endif
				ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY;
				gui_swich_msg();
				Save_Detail_Index = ScreenState;
				timer_app_pasue_start();
				gui_sport_motor_prompt();
			}
			else
			{
				//放弃运动
				ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY_CANCEL;
				
				m_gui_sport_crosscountry_pause_index = GUI_SPORT_CROSSCOUNTRY_PAUSE_CANCEL_NO;
				gui_swich_msg();
			}
			
			break;
		case KEY_UP:
			if(m_gui_sport_crosscountry_pause_index > GUI_SPORT_CROSSCOUNTRY_PAUSE_CONTINUE)
			{

			
			
				m_gui_sport_crosscountry_pause_index--;
				if((TRACE_BACK_DATA.is_traceback_flag) && (m_gui_sport_crosscountry_pause_index == GUI_SPORT_CROSSCOUNTRY_PAUSE_TRACKBACK))
				{
						//如果是在循迹返航过程，则无循迹返航选项
						m_gui_sport_crosscountry_pause_index = GUI_SPORT_CROSSCOUNTRY_PAUSE_WAIT;
				}
			


			}
			else
			{
				m_gui_sport_crosscountry_pause_index = GUI_SPORT_CROSSCOUNTRY_PAUSE_CANCEL;
			}
			
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		    break;
		case KEY_DOWN:
			if(m_gui_sport_crosscountry_pause_index < GUI_SPORT_CROSSCOUNTRY_PAUSE_CANCEL)
			{

			
				m_gui_sport_crosscountry_pause_index++;
				if((TRACE_BACK_DATA.is_traceback_flag) && (m_gui_sport_crosscountry_pause_index == GUI_SPORT_CROSSCOUNTRY_PAUSE_TRACKBACK))
				{
					//如果是在循迹返航过程，则无循迹返航选项
					m_gui_sport_crosscountry_pause_index = GUI_SPORT_CROSSCOUNTRY_PAUSE_CANCEL;
				}
				

			}
			else
			{
				m_gui_sport_crosscountry_pause_index = GUI_SPORT_CROSSCOUNTRY_PAUSE_CONTINUE;
			}
			
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		case KEY_BACK:
			//返回骑行界面
			ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY;
		
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
	
}

//放弃菜单界面按键事件
void gui_sport_crosscountry_cancel_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
			if(m_gui_sport_crosscountry_pause_index == GUI_SPORT_CROSSCOUNTRY_PAUSE_CANCEL_NO)
			{
				//取消放弃运动，返回上级菜单界面
				ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY_PAUSE;
				m_gui_sport_crosscountry_pause_index = GUI_SPORT_CROSSCOUNTRY_PAUSE_CANCEL;
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
			if(m_gui_sport_crosscountry_pause_index == GUI_SPORT_CROSSCOUNTRY_PAUSE_CANCEL_NO)
			{
				m_gui_sport_crosscountry_pause_index = GUI_SPORT_CROSSCOUNTRY_PAUSE_CANCEL_YES;
			}
			else
			{
				m_gui_sport_crosscountry_pause_index = GUI_SPORT_CROSSCOUNTRY_PAUSE_CANCEL_NO;
			}

			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		    break;
		case KEY_BACK:
			//返回上级菜单
			ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY_PAUSE;
			m_gui_sport_crosscountry_pause_index = GUI_SPORT_CROSSCOUNTRY_PAUSE_CANCEL;
			
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}

//详细数据界面按键事件
void gui_sport_crosscountry_detail_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_UP:
			//上翻
			if(m_gui_sport_crosscountry_pause_index > GUI_SPORT_CROSSCOUNTRY_DETAIL_1)
			{
				m_gui_sport_crosscountry_pause_index--;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
		    break;
		case KEY_DOWN:
			//下翻
			if(m_gui_sport_crosscountry_pause_index < GUI_SPORT_CROSSCOUNTRY_DETAIL_TRACK)
			{
				m_gui_sport_crosscountry_pause_index++;

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



//越野保存数据详情_界面1
void gui_crosscountry_save_detail_1_paint(uint8_t backgroundcolor)
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
			LCD_SetString("越野跑", &word);
		
		
			
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
			sprintf(str,"%d.%02d",ActivityData.ActivityDetails.CrosscountryDetail.Distance/100000,ActivityData.ActivityDetails.CrosscountryDetail.Distance/1000%100);
	//#else
	//		sprintf(str,"%0.2f",ActivityData.ActivityDetails.CrosscountryDetail.Distance/100000.f);
	//#endif
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
		
		
		
		//平均配速 
			charlen = strlen(sportdetailstrsgod[SetValue.Language][2]);
			word.x_axis =  120 + 4 + 12;
			word.y_axis = 35;
			word.kerning = 0;
			word.size = LCD_FONT_16_SIZE;
			LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][2],&word);
			
			word.y_axis += charlen*8+1;
			LCD_SetString(colon,&word);
			memset(str,0,20);
			sprintf(str,"%d:%02d",(ActivityData.ActivityDetails.CrosscountryDetail.Speed/60),(ActivityData.ActivityDetails.CrosscountryDetail.Speed%60));
			word.x_axis =  120 + 4 + 12 + 16 + 4;
			word.y_axis = 35;
			word.kerning = 1;
			word.size = LCD_FONT_19_SIZE;
			LCD_SetNumber(str,&word);
		
			
			word.x_axis =  120 + 4 + 12 + 16 + 4 + 19 - 16;
			word.y_axis += strlen(str)*10 - 0.5*10 + 8;
			word.kerning = 0;
		   word.size = LCD_FONT_16_SIZE;
			LCD_SetString("/km",&word);
		
		   
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
			sprintf(str,"%d",ActivityData.ActivityDetails.CrosscountryDetail.AvgHeart);
			
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


//越野保存数据详情_界面2
void gui_crosscountry_save_detail_2_paint(uint8_t backgroundcolor)
	{
			SetWord_t word = {0};
			uint32_t sec = 0;
			char str[20] = {0};
			
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
		//总步数
		charlen = strlen(sportdetailstrsgod[SetValue.Language][5]);
		word.x_axis = 120 - 8 - 19 - 4 - 16;
		word.y_axis = 120 - 2*16;
		word.kerning = 0;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][5],&word);
		  word.y_axis =  120 - 2*16 + charlen*8 + 1;
		LCD_SetString(colon,&word);
		memset(str,0,20);
		sprintf(str,"%d",ActivityData.ActivityDetails.RunningDetail.Steps);
		word.x_axis = 120 - 8 - 19;
		word.y_axis = 120 - 2*16;
		word.size = LCD_FONT_19_SIZE;
		word.kerning = 1;
		LCD_SetNumber(str,&word);
	
		
		//累积爬升
		charlen = strlen(sportdetailstrsgod[SetValue.Language][8]);
		word.x_axis =  120 + 8; 
		word.y_axis = 120 - 2*16;
		word.kerning = 0;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][8],&word);
		word.y_axis = 120 - 2*16 + charlen*8 + 1;
		LCD_SetString(colon,&word);
	
		memset(str,0,20);
		sprintf(str,"%d",(uint16_t)(ActivityData.ActivityDetails.RunningDetail.RiseHeight));
		word.x_axis =  120 + 8 + 16 + 4;
		word.y_axis = 120 - 2*16;
		word.kerning = 1;
		word.size = LCD_FONT_19_SIZE;
		LCD_SetNumber(str,&word);
		
		 word.x_axis =	120 + 8 + 16 + 4 + 19 - 16;
		 word.y_axis += strlen(str)*10	+ 8;
		 word.kerning = 0;
		 word.size = LCD_FONT_16_SIZE;
		 LCD_SetString("m",&word);
		
			
			
		
		LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, LCD_BLACK, LCD_NONE, &Img_Pointing_Down_12X8);
		
	}



//越野保存数据详情_界面3
void gui_crosscountry_save_detail_3_paint(uint8_t backgroundcolor)
{
//	GUI_SPORT_CROSSCOUNTRY_PRINTF("gui_crosscountry_save_detail_3_paint"); 
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
  
    LCD_SetPicture(10, LCD_CENTER_JUSTIFIED, word.forecolor, LCD_NONE, &Img_Pointing_Up_12X8);
    
    word.size = LCD_FONT_16_SIZE;
//  word.forecolor = LCD_WHITE;
    word.bckgrndcolor = LCD_NONE;		
    //有氧效果
    charlen = strlen(SportDetailStrs[SetValue.Language][7]);
    word.x_axis = 60;
    word.y_axis = 95 - charlen*8;
    LCD_SetString((char *)SportDetailStrs[SetValue.Language][7],&word);
    word.y_axis = 100;
    word.kerning = 0;
    LCD_SetString(colon,&word);
    memset(str,0,20);
	#if defined (SPRINTF_FLOAT_TO_INT)
	sprintf(str,"%d.%01d",ActivityData.ActivityDetails.RunningDetail.AerobicEffect/10,ActivityData.ActivityDetails.RunningDetail.AerobicEffect%10);
	#else
    sprintf(str,"%0.1f",ActivityData.ActivityDetails.RunningDetail.AerobicEffect/10.f);
		#endif
    word.y_axis = 112;
    word.kerning = 1;
    LCD_SetNumber(str,&word);

   //无氧效果
    charlen = strlen(SportDetailStrs[SetValue.Language][8]);
    word.x_axis = 90;
    word.y_axis = 95 - charlen*8;
    LCD_SetString((char *)SportDetailStrs[SetValue.Language][8],&word);
    word.y_axis = 100;
    word.kerning = 0;
    LCD_SetString(colon,&word);
    memset(str,0,20);
	#if defined (SPRINTF_FLOAT_TO_INT)
		sprintf(str,"%d.%01d",ActivityData.ActivityDetails.RunningDetail.AnaerobicEffect/10,ActivityData.ActivityDetails.RunningDetail.AnaerobicEffect%10);
	#else
    sprintf(str,"%0.1f",ActivityData.ActivityDetails.RunningDetail.AnaerobicEffect/10.f);
	#endif
    word.y_axis = 112;
    word.kerning = 1;
    LCD_SetNumber(str,&word);

    //恢复时间
    charlen = strlen(SportDetailStrs[SetValue.Language][9]);
    word.x_axis = 120;
    word.y_axis = 95 - charlen*8;
    LCD_SetString((char *)SportDetailStrs[SetValue.Language][9],&word);
    word.y_axis = 100;
    word.kerning = 0;
    LCD_SetString(colon,&word);
    memset(str,0,20);
	#if defined (SPRINTF_FLOAT_TO_INT)
	sprintf(str,"%d.%01d",ActivityData.ActivityDetails.RunningDetail.RecoveryTime/60,ActivityData.ActivityDetails.RunningDetail.RecoveryTime%60);
	#else
    sprintf(str,"%0.1f",ActivityData.ActivityDetails.RunningDetail.RecoveryTime/60.f);
	#endif
    word.y_axis = 112;
    word.kerning = 1;
    LCD_SetNumber(str,&word);
    word.y_axis = 160;
    word.kerning = 0;
    LCD_SetString("h",&word);


    LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, word.forecolor, LCD_NONE, &Img_Pointing_Down_12X8);
}    
 


#if defined WATCH_COM_SPORT
#else

//越野运动详细心率区间界面
void gui_crosscountry_save_heartratezone_paint(uint8_t backgroundcolor)
{
   SetWord_t word = {0};
   char str[10];
   uint8_t width,i;
   uint32_t MaxHeartRateZone = 0;
   uint8_t charlen;
   
   //假数据，调试示例，后面去掉
//   ActivityData.ActivityDetails.RunningDetail.HeartRateZone[0] = 533;
//   ActivityData.ActivityDetails.RunningDetail.HeartRateZone[1] = 2222;
//   ActivityData.ActivityDetails.RunningDetail.HeartRateZone[2] = 1222;
//   ActivityData.ActivityDetails.RunningDetail.HeartRateZone[3] = 1233;
//   ActivityData.ActivityDetails.RunningDetail.HeartRateZone[4] = 333;


   for (i = 0; i < 5;i++)
   {
        if (MaxHeartRateZone < ActivityData.ActivityDetails.RunningDetail.HeartRateZone[i])
        {
           MaxHeartRateZone = ActivityData.ActivityDetails.RunningDetail.HeartRateZone[i];
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
  
    LCD_SetPicture(10, LCD_CENTER_JUSTIFIED, word.forecolor, LCD_NONE, &Img_Pointing_Up_12X8);

    word.x_axis = 25;
    word.y_axis = LCD_CENTER_JUSTIFIED;
    word.size = LCD_FONT_16_SIZE;
    //word.forecolor = LCD_WHITE;
    word.bckgrndcolor = LCD_NONE;
    LCD_SetString((char *)SportDetailStrs[SetValue.Language][13],&word);
    
    charlen = strlen(SportDetailStrs[SetValue.Language][14]);
    word.x_axis = 60;
    word.y_axis = 35;
    word.size = LCD_FONT_16_SIZE;
 // word.forecolor = LCD_WHITE;
    word.bckgrndcolor = LCD_NONE;
    LCD_SetString((char *)SportDetailStrs[SetValue.Language][14],&word);
    word.y_axis += (charlen*8+1);
    LCD_SetString("5",&word);
    memset(str,0,10);
    sprintf(str,"%d:%02d",(ActivityData.ActivityDetails.RunningDetail.HeartRateZone[4]/60),(ActivityData.ActivityDetails.RunningDetail.HeartRateZone[4]%60));
    word.y_axis += (8+4);
    LCD_SetString(str,&word);
    word.y_axis +=50;
    width = 80 * ActivityData.ActivityDetails.RunningDetail.HeartRateZone[4]/MaxHeartRateZone;
    LCD_SetRectangle(word.x_axis,16,word.y_axis,width,LCD_RED,0,0,LCD_FILL_ENABLE);

    

    word.x_axis = 90;
    word.y_axis = 35;
    LCD_SetString((char *)SportDetailStrs[SetValue.Language][14],&word);
    word.y_axis += (charlen*8+1);
    LCD_SetString("4",&word);
    memset(str,0,10);
    sprintf(str,"%d:%02d",(ActivityData.ActivityDetails.RunningDetail.HeartRateZone[3]/60),(ActivityData.ActivityDetails.RunningDetail.HeartRateZone[3]%60));
    word.y_axis += (8+4);
    LCD_SetString(str,&word);
    word.y_axis +=50;
    width = 80 * ActivityData.ActivityDetails.RunningDetail.HeartRateZone[3]/MaxHeartRateZone;
    LCD_SetRectangle(word.x_axis,16,word.y_axis,width,LCD_CHOCOLATE,0,0,LCD_FILL_ENABLE);


    word.x_axis = 120;
    word.y_axis = 35;
    LCD_SetString((char *)SportDetailStrs[SetValue.Language][14],&word);
    word.y_axis += (charlen*8+1);
    LCD_SetString("3",&word);
    memset(str,0,10);
    sprintf(str,"%d:%02d",(ActivityData.ActivityDetails.RunningDetail.HeartRateZone[2]/60),(ActivityData.ActivityDetails.RunningDetail.HeartRateZone[2]%60));
    word.y_axis += (8+4);
    LCD_SetString(str,&word);
    word.y_axis +=50;
    width = 80 * ActivityData.ActivityDetails.RunningDetail.HeartRateZone[2]/MaxHeartRateZone;
    LCD_SetRectangle(word.x_axis,16,word.y_axis,width,LCD_GREEN,0,0,LCD_FILL_ENABLE);

    word.x_axis = 150;
    word.y_axis = 35;
    LCD_SetString((char *)SportDetailStrs[SetValue.Language][14],&word);
    word.y_axis += (charlen*8+1);
    LCD_SetString("2",&word);
    memset(str,0,10);
    sprintf(str,"%d:%02d",(ActivityData.ActivityDetails.RunningDetail.HeartRateZone[1]/60),(ActivityData.ActivityDetails.RunningDetail.HeartRateZone[1]%60));
    word.y_axis += (8+4);
    LCD_SetString(str,&word);
    word.y_axis +=50;
    width = 80 * ActivityData.ActivityDetails.RunningDetail.HeartRateZone[1]/MaxHeartRateZone;
    LCD_SetRectangle(word.x_axis,16,word.y_axis,width,LCD_CORNFLOWERBLUE,0,0,LCD_FILL_ENABLE);

    word.x_axis = 180;
    word.y_axis = 35;
    LCD_SetString((char *)SportDetailStrs[SetValue.Language][14],&word);
    word.y_axis += (charlen*8+1);
    LCD_SetString("1",&word);
    memset(str,0,10);
    sprintf(str,"%d:%02d",(ActivityData.ActivityDetails.RunningDetail.HeartRateZone[0]/60),(ActivityData.ActivityDetails.RunningDetail.HeartRateZone[0]%60));
    word.y_axis += (8+4);
    LCD_SetString(str,&word);
    word.y_axis +=50;
    width = 80 * ActivityData.ActivityDetails.RunningDetail.HeartRateZone[0]/MaxHeartRateZone;
    LCD_SetRectangle(word.x_axis,16,word.y_axis,width,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);

 
}

//越野准备界面按键处理
void gui_crosscountry_ready_btn_evt(uint32_t Key_Value)
{
   // DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
	         ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY_TRACK;
					 Sport_Index = DISPLAY_SCREEN_CROSSCOUNTRY_TRACK;
             Save_Detail_Index = DISPLAY_SCREEN_CROSSCOUNTRY_SAVE_DETIAL_1;
             gui_swich_msg();
						 gui_sport_motor_prompt(); //开启震动提示
						 Set_PauseSta(false);				//增加稍后继续功能
					if(0 == IS_SPORT_CONTINUE_LATER)
						 StartSport();
			break;
	
		case KEY_BACK:
			if(0 == ENTER_NAVIGATION && 0 == ENTER_MOTION_TRAIL)    //由导航界面进入运动ready界面的判断
			{
            //返回运动选择界面
             ScreenState = DISPLAY_SCREEN_SPORT;
             gui_swich_msg();
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
			break;
		default:
			break;
	}	
}

//越野轨迹界面按键处理

void gui_crosscountry_track_btn_evt(uint32_t Key_Value)
{
   //DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
						if(TRACE_BACK_DATA.is_traceback_flag)
						{
							trace_back_timer_stop();
						}
						ScreenStateSave = ScreenState;
						ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY_PAUSE;
            cross_pause_index = CrossPauseContinue ;
            gui_swich_msg();
						gui_sport_motor_prompt(); //开启震动提示
						Set_PauseSta(true);			//开启暂停数据状态
			break;
		case KEY_DOWN:
            gui_sport_index_down(DISPLAY_SCREEN_CROSSCOUNTRY_TRACK,DISPLAY_SCREEN_CROSSCOUNTRY_DATA);

			break;
		case KEY_UP:
            gui_sport_index_up(DISPLAY_SCREEN_CROSSCOUNTRY_TRACK,DISPLAY_SCREEN_CROSSCOUNTRY_DATA);

		    break;
		case KEY_BACK:
 					// ScreenStateSave = ScreenState;
					// ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY_PAUSE;
          // cross_pause_index = CrossPauseContinue ;
          // gui_swich_msg();
				//	 gui_sport_motor_prompt(); //开启震动提示
			//		 Set_PauseSta(true);			//开启暂停数据状态
			break;
		case KEY_LONG_UP:
			//保存航迹打点数据
			sport_dottrack();

			break;
		default:
			break;
	}	

}

//越野数据界面按键处理

void gui_crosscountry_data_btn_evt(uint32_t Key_Value)
{
   //DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
			     if(TRACE_BACK_DATA.is_traceback_flag)
					 {
					 	 trace_back_timer_stop();
					 }
	         ScreenStateSave = ScreenState;
					 ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY_PAUSE;
           cross_pause_index = CrossPauseContinue ;
           gui_swich_msg();
					 gui_sport_motor_prompt(); //开启震动提示
					 Set_PauseSta(true);			//开启暂停数据状态
			break;
		case KEY_DOWN:
            gui_sport_index_down(DISPLAY_SCREEN_CROSSCOUNTRY_TRACK,DISPLAY_SCREEN_CROSSCOUNTRY_DATA);

			break;
		case KEY_UP:
            gui_sport_index_up(DISPLAY_SCREEN_CROSSCOUNTRY_TRACK,DISPLAY_SCREEN_CROSSCOUNTRY_DATA);

            break; 
		case KEY_BACK:
					// ScreenStateSave = ScreenState;
				//	 ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY_PAUSE;
          // cross_pause_index = CrossPauseContinue ;
         //  gui_swich_msg();
		//			 gui_sport_motor_prompt(); //开启震动提示
		//			 Set_PauseSta(true);			//开启暂停数据状态
			break;
		default:
			break;
	}	

}

//越野暂停界面按键处理

void gui_crosscountry_pause_btn_evt(uint32_t Key_Value)
{
    //DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:  
            if (cross_pause_index == CrossPauseContinue)
            {
								if(1 == IS_SPORT_CONTINUE_LATER)			//增加稍后继续功能
								{
									OpenSensor();
									gui_sport_motor_prompt(); //开启震动提示									
									ScreenState = ContinueLaterDataSave.ScreenStateSave;   //暂时修改，用于防止训练计划干扰到Sport_Index
									Sport_Index = ContinueLaterDataSave.ScreenStateSave;   ///////////////////////////////////////////////
								}
								else
								{
									if(TRACE_BACK_DATA.is_traceback_flag)
									{
										trace_back_timer_restart();
									}
									ScreenState = Sport_Index;
								}
								gui_sport_motor_prompt(); //开启震动提示
								Set_PauseSta(false);			//关闭暂停数据状态

            }
            else if (cross_pause_index == CrossPauseSave)
            {
							 if(TRACE_BACK_DATA.is_traceback_flag)
							 {
								 trace_back_timer_delete();
							 }
               ScreenState = DISPLAY_SCREEN_SPORT_FEEL;
               gui_sport_feel_init();
			   sport_end_flag = 1;
			   sport_end_hrvalue = get_10s_heartrate(); //获取前10s平均心率
				g_HeartNum = 0;
							 SportContinueLaterFlagClr();
//               sport_feel_index = FeelAbitTired;
            }
						else if (cross_pause_index == CrossContinueLater)
						{
							 CloseSensor();//稍后继续关闭传感器	
						   ContinueLaterDataSave.ScreenStateSave = Sport_Index;
							 SportContinueLaterFlagSet();					   
							 ContinueLaterDataSave.SportType = DISPLAY_SCREEN_CROSSCOUNTRY;
							 ScreenState = DISPLAY_SCREEN_HOME;
						}
	        else if (cross_pause_index == CrossPauseTrackback)
            {
							//保存数据
							TRACE_BACK_DATA.sport_distance = ActivityData.ActivityDetails.CyclingDetail.Distance;
							TRACE_BACK_DATA.remaining_distance = ActivityData.ActivityDetails.CyclingDetail.Distance;
							
							//开启循迹返航定时器
							trace_back_timer_start();

							//开始循迹返航
							if(1 == IS_SPORT_CONTINUE_LATER)
								OpenSensor();		
							Set_PauseSta(false);
							TRACE_BACK_DATA.is_traceback_flag = 1;
							
							ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY_TRACK;
							//循迹返航轨迹标记
							TrackBackFlag();
							gui_sport_motor_prompt();
            }
            else if (cross_pause_index == CrossPauseCancel)
            {
                Pause_Save_Index = ScreenState;
                ScreenState = DISPLAY_SCREEN_SPORT_CANCEL;
								gui_sport_cancel_init();
            }
            gui_swich_msg();
			break;
		case KEY_UP:
         if (cross_pause_index == CrossPauseContinue)
           {
             cross_pause_index = CrossPauseCancel ;

           }

					 else
           {
              if ((TRACE_BACK_DATA.is_traceback_flag==1)&&(cross_pause_index == CrossPauseCancel))
						{
							cross_pause_index = cross_pause_index-1;
							cross_pause_index--;
						}   
						 else 
						 cross_pause_index--;
           }
           gui_swich_msg();
 
			break;
		case KEY_DOWN:
          if (cross_pause_index == CrossPauseCancel)
           {
             cross_pause_index = CrossPauseContinue ;

           }
         else
           {
             if ((TRACE_BACK_DATA.is_traceback_flag==1)&&(cross_pause_index == CrossContinueLater))
							{
							cross_pause_index = cross_pause_index+1;
							cross_pause_index++;
							}  
							else
							cross_pause_index++;
           }
          gui_swich_msg();

            break; 
		case KEY_BACK:
					
					if (Sport_Index == DISPLAY_SCREEN_CROSSCOUNTRY_DATA)
					{ 
//					ScreenStateSave = ScreenState;
					ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY_DATA_PAUSE;
					gui_swich_msg();
					}
					else if (Sport_Index == DISPLAY_SCREEN_CROSSCOUNTRY_TRACK)
					{ 
//					ScreenStateSave = ScreenState;
					ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY_TRACK_PAUSE;
					gui_swich_msg();
					}
			break;
					
		default:
			break;
	}	

}
//越野暂停后返回界面按键处理 （越野轨迹与越野数据）

void gui_crosscountry_pause_screen_btn_evt(uint32_t Key_Value)
	
{
	switch ( Key_Value )
	{
		case KEY_OK:  

						if(TRACE_BACK_DATA.is_traceback_flag)
						{
							trace_back_timer_restart();
						}

						if (ScreenState == DISPLAY_SCREEN_CROSSCOUNTRY_DATA_PAUSE)
            {
							if(1 == IS_SPORT_CONTINUE_LATER)
							{
								OpenSensor();
							}
							
							ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY_DATA;
							ScreenStateSave = ScreenState; 
							gui_swich_msg();
							gui_sport_motor_prompt(); //开启震动提示
							Set_PauseSta(false);			//关闭暂停数据状态
            }
            else if (ScreenState == DISPLAY_SCREEN_CROSSCOUNTRY_TRACK_PAUSE)
            {
								
							if(1 == IS_SPORT_CONTINUE_LATER)
							{
								OpenSensor();
							}
							
							ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY_TRACK;
							ScreenStateSave = ScreenState;
              gui_swich_msg();
							gui_sport_motor_prompt(); //开启震动提示
							Set_PauseSta(false);			//关闭暂停数据状态
            }

			break;
						
		case KEY_UP:
            if (ScreenState == DISPLAY_SCREEN_CROSSCOUNTRY_DATA_PAUSE)
            {
							ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY_TRACK_PAUSE;
							ScreenStateSave = DISPLAY_SCREEN_CROSSCOUNTRY_TRACK; 
							gui_swich_msg();
						}
						else if (ScreenState == DISPLAY_SCREEN_CROSSCOUNTRY_TRACK_PAUSE)
						{
							ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY_DATA_PAUSE;
							ScreenStateSave = DISPLAY_SCREEN_CROSSCOUNTRY_DATA; 
							gui_swich_msg();
						}
      break; 
		
		case KEY_DOWN:    
						if (ScreenState == DISPLAY_SCREEN_CROSSCOUNTRY_DATA_PAUSE)
            {
							ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY_TRACK_PAUSE;
							ScreenStateSave = DISPLAY_SCREEN_CROSSCOUNTRY_TRACK; 
							gui_swich_msg();
						}
						else if (ScreenState == DISPLAY_SCREEN_CROSSCOUNTRY_TRACK_PAUSE)
						{
							ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY_DATA_PAUSE;
							ScreenStateSave = DISPLAY_SCREEN_CROSSCOUNTRY_DATA; 
							gui_swich_msg();
						}
			break;

		case KEY_BACK:
							ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY_PAUSE;
							gui_swich_msg();
			break;
		default:
			break;
	}	
}


//越野运动详细_界面1按键处理
void gui_crosscountry_save_detail_1_btn_evt(uint32_t Key_Value)
{
    // DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_DOWN:
             ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY_SAVE_DETIAL_2;
             gui_swich_msg();
			break;
		case KEY_UP:
   
             gui_swich_msg();

            break; 
		case KEY_BACK:
             ScreenState = DISPLAY_SCREEN_HOME;
             gui_swich_msg();
			break;
		default:
			break;
	}	
}


//越野运动详细_界面2按键处理
void gui_crosscountry_save_detail_2_btn_evt(uint32_t Key_Value)
{
    // DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_DOWN:
             ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY_SAVE_DETIAL_3;
             gui_swich_msg();
             break;
        case KEY_UP:
             ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY_SAVE_DETIAL_1;
             gui_swich_msg();
             break;
		case KEY_BACK:
             ScreenState = DISPLAY_SCREEN_HOME;
             gui_swich_msg();
			break;
		default:
			break;
	}	
}
//越野运动详细_界面3按键处理
void gui_crosscountry_save_detail_3_btn_evt(uint32_t Key_Value)
{
    // DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_DOWN:
             ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY_SAVE_TRACK;
             gui_swich_msg();
             break;
        case KEY_UP:
             ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY_SAVE_DETIAL_2;
             gui_swich_msg();
             break;
		case KEY_BACK:
             ScreenState = DISPLAY_SCREEN_HOME;
             gui_swich_msg();
			break;
		default:
			break;
	}	
}
//越野运动详细轨迹界面按键处理
void gui_crosscountry_save_track_btn_evt(uint32_t Key_Value)
{
    // DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_DOWN:
             ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY_SAVE_HEARTRATEZONE;
             gui_swich_msg();
			break;
		case KEY_UP:
             ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY_SAVE_DETIAL_3;
             gui_swich_msg();
            break; 
		case KEY_BACK:
             ScreenState = DISPLAY_SCREEN_HOME;
             gui_swich_msg();
			break;
		default:
			break;
	}	
}

//越野运动详细心率区间界面按键处理
void gui_crosscountry_save_heartratezone_btn_evt(uint32_t Key_Value)
{
    // DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_UP:
             ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY_SAVE_TRACK;
             gui_swich_msg();
            break; 
		case KEY_BACK:
             ScreenState = DISPLAY_SCREEN_HOME;
             gui_swich_msg();
			break;
		default:
			break;
	}	
}
#endif

