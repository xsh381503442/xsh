#include "gui_sport_config.h"
#include "gui_sport.h"
#include "gui_climbing.h"

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
#include "gui_tool_gps.h"
#include "algo_trackjudge.h"
#include "timer_traceback.h"
#include "time_notify.h"
#include "timer_app.h"


#if DEBUG_ENABLED == 1 && GUI_RUN_LOG_ENABLED == 1
	#define GUI_RUN_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_RUN_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define GUI_RUN_WRITESTRING(...)
	#define GUI_RUN_PRINTF(...)		        
#endif

uint8_t vice_screen_climbing_display[SPORT_DATA_DISPLAY_SUM] = {SPORT_DATA_DISPALY_TOTAL_DISTANCE,SPORT_DATA_DISPALY_ALTITUDE,SPORT_DATA_DISPALY_REALTIME_HEART,SPORT_DATA_DISPALY_TOTAL_TIME};

/*登山数据显示当前项*/
 uint8_t m_climbing_data_display_buf[SPORT_DATA_DISPLAY_SUM]={SPORT_DATA_DISPALY_REALTIME_HEART,SPORT_DATA_DISPALY_ALTITUDE,SPORT_DATA_DISPALY_TOTAL_CLIMB,SPORT_DATA_DISPALY_TOTAL_TIME};
/*登山数据显示选项*/
const uint8_t m_climbing_data_display_option[CLIMBING_DATA_DISPLAY_SUM]= {SPORT_DATA_DISPALY_TOTAL_DISTANCE,SPORT_DATA_DISPALY_ALTITUDE,SPORT_DATA_DISPALY_TOTAL_CLIMB,SPORT_DATA_DISPALY_TOTAL_TIME,
SPORT_DATA_DISPALY_VER_AVE_SPEED,SPORT_DATA_DISPALY_REALTIME_HEART,SPORT_DATA_DISPALY_REALTIME_FREQUENCY};

/*登山提醒设置选项*/
const uint8_t m_climbing_remind_set_option[CLIMBING_REMIND_SET_SUM]={SPORT_REMIND_SET_HAERT,SPORT_REMIND_SET_HRRECOVERY,SPORT_REMIND_SET_DISTANCE,SPORT_REMIND_SET_ALTITUDE,SPORT_REMIND_SET_TIME,SPORT_REMIND_SET_AUTOCIRCLE};

extern _drv_bat_event g_bat_evt;

#define	GUI_SPORT_CLIMBING_SCREEN_DATA_1		0	//登山预览界面
#define	GUI_SPORT_CLIMBING_SCREEN_DATA_2		1	//登山心率界面
#define	GUI_SPORT_CLIMBING_SCREEN_TRACK			2	//登山轨迹界面
#define GUI_SPORT_CLIMBING_SCREEN_MAX			3


#define	GUI_SPORT_CLIMBING_PAUSE_CONTINUE	0	//继续
#define	GUI_SPORT_CLIMBING_PAUSE_SAVE		1	//结束运动
#define	GUI_SPORT_CLIMBING_PAUSE_WAIT		2	//稍后继续
#define	GUI_SPORT_CLIMBING_PAUSE_TRACKBACK	3	//循迹返航
#define	GUI_SPORT_CLIMBING_PAUSE_CANCEL		4	//放弃

#define	GUI_SPORT_CLIMBING_PAUSE_CANCEL_NO	0	//否
#define	GUI_SPORT_CLIMBING_PAUSE_CANCEL_YES	1	//是


#define	GUI_SPORT_CLIMBING_DETAIL_1				0	//详细数据界面1
#define	GUI_SPORT_CLIMBING_DETAIL_2				1	//详细数据界面2
#define GUI_SPORT_CLIMBING_DETAIL_TRACK			2	//轨迹界面
//#define	GUI_SPORT_CLIMBING_DETAIL_HEARTRATEZONE	3	//心率区间界面


static uint8_t m_gui_sport_climbing_index;		//登山界面索引
static uint8_t m_gui_sport_climbing_pause_index;	//登山暂停菜单界面索引

extern volatile ScreenState_t Save_Detail_Index;
extern float sport_alt;

extern uint8_t gui_sport_index;
extern uint8_t gui_sport_type;
//按ok键运动开始提示
void gui_climbing_prompt_paint(void)
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


//登山数据界面
/*void gui_sport_climbing_data_1_paint(void)
{
	SetWord_t word = {0};
	char str[10];
	//uint8_t len;
	//设置背景黑色
	LCD_SetBackgroundColor(LCD_BLACK);

	if(g_sport_status == SPORT_STATUS_READY)
	{
		gui_climbing_prompt_paint();
	}
	else
	{
		//圆点索引
   		gui_sport_index_circle_paint(0, 5, 1);

		//上升高度图标
		word.x_axis = 40;
		word.y_axis = 50;
		LCD_SetPicture(word.x_axis, word.y_axis, LCD_NONE, LCD_NONE, &Img_RiseHeight_22X22);

		word.bckgrndcolor = LCD_NONE;
		word.forecolor = LCD_WHITE;
		word.kerning = 1;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.x_axis = 34;
		word.size = LCD_FONT_32_SIZE;

		memset(str,0,10);
		sprintf(str,"%d",ActivityData.ActivityDetails.ClimbingDetail.RiseHeight);
		LCD_SetNumber(str,&word);

		word.y_axis = 170;
		word.x_axis = 52;
		word.size = LCD_FONT_16_SIZE;	
		word.forecolor = LCD_WHITE;
		word.kerning = 0;
		
		LCD_SetString("m",&word);
		
	}

	
	//分割线
	word.x_axis = 80;
	LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,LCD_AUQAMARIN,0,0,LCD_FILL_ENABLE);

	
	//GPS状态,防止遮挡，采用靠左位置
	gui_sport_gps_status(SPORT_GPS_LEFT1);
	
	//海拔
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 1;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.x_axis = 92;
	word.size = LCD_FONT_48_SIZE;
	memset(str,0,10);

	sprintf(str,"%d",(int32_t)sport_alt);
	//len = strlen(str);
	LCD_SetNumber(str,&word);

	word.size = LCD_FONT_16_SIZE;
	word.kerning = 0;
	
	word.x_axis +=40;
	word.y_axis =190;
	LCD_SetString("m",&word);
	

	//分割线
	word.x_axis = 160;	
	LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,LCD_AUQAMARIN,0,0,LCD_FILL_ENABLE);

	//下降高度图标
	word.x_axis = 177;
	word.y_axis = 50;
	LCD_SetPicture(word.x_axis, word.y_axis, LCD_NONE, LCD_NONE, &Img_DropHeight_22X22);

	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 1;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.x_axis = 170;
	word.size = LCD_FONT_32_SIZE;

	memset(str,0,10);
	sprintf(str,"%d",ActivityData.ActivityDetails.ClimbingDetail.DropHeight);
	LCD_SetNumber(str,&word);

	word.y_axis = 170;
	word.x_axis = 188;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
	word.kerning = 0;
	LCD_SetString("m",&word);

	

}*/
//登山数据界面2
/*void gui_sport_climbing_data_2_paint(void)
{
	SetWord_t word = {0};
	char str[10];
	uint8_t len;
	//设置背景黑色
	LCD_SetBackgroundColor(LCD_BLACK);

	
		//圆点索引
   	gui_sport_index_circle_paint(1, 5, 1);

	gui_sport_data_display_total_distance_paint(SPORT_DATA_DISPALY_INDEX_UP_CIRCLE,ActivityData.ActivityDetails.ClimbingDetail.Distance);
		
	
	//分割线
	word.x_axis = 80;
	LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,LCD_AUQAMARIN,0,0,LCD_FILL_ENABLE);

	
	//GPS状态,防止遮挡，采用靠左位置
	gui_sport_gps_status(SPORT_GPS_LEFT1);

	gui_sport_data_display_total_time_paint(SPORT_DATA_DISPALY_INDEX_MID_GPS,ActivityData.ActTime);

	//分割线
	word.x_axis = 160;	
	LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,LCD_AUQAMARIN,0,0,LCD_FILL_ENABLE);

	//垂直高度图标
	LCD_SetPicture(175, 70, LCD_NONE, LCD_NONE, &Img_VerticalHeight_10X28);

	//速度数据
	word.x_axis = 170;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_32_SIZE;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 1;
	memset(str,0,sizeof(str));
    sprintf(str,"%d",ActivityData.ActivityDetails.ClimbingDetail.Speed/10);
	LCD_SetNumber(str,&word);
	
	len = strlen(str)*(Font_Number_32.width + 1) - 1;
	word.x_axis += 20;
	word.y_axis = (LCD_PIXEL_ONELINE_MAX - len)/2 + len + 8;
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 0;
	LCD_SetString("m/h",&word);

}*/





//登山轨迹更新
void gui_climbing_track_update_paint(void)
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
static void gui_sport_climbing_1_distance(uint8_t type, uint32_t distance)
{
	gui_sport_every_1_distance(type,distance);
}

static void gui_sport_climbing_1_time(void)
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


//登山轨迹界面
void gui_sport_climbing_track_paint(void)
{
		static uint8_t last_status = 2;
	
		//设置背景黑色
		LCD_SetBackgroundColor(LCD_WHITE);
		 //轨迹
		gui_climbing_track_update_paint();
		//圆点索引
	gui_sport_index_circle_vert_paint(2, 3);
		//gui_sport_index_circle_paint(3, 5, 1);
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
				
				gui_sport_climbing_1_distance(2, 0);
			}
			else if((last_status == 1) && (TRACE_BACK_DATA.whether_in_track == 0))
			{
				//未在轨迹上
				timer_notify_motor_start(300,300,1,false,NOTIFY_MODE_SPORTS);
				timer_notify_buzzer_start(300,300,1,false,NOTIFY_MODE_SPORTS);
				
				gui_sport_climbing_1_distance(3, 0);
			}
			else
			{
				if(TRACE_BACK_DATA.sport_traceback_flag)
				{
					//计时
					gui_sport_climbing_1_time();
				}
				else
				{
					if(TRACE_BACK_DATA.whether_in_track)
					{
						//剩余距离
						gui_sport_climbing_1_distance(0, TRACE_BACK_DATA.remaining_distance);
					}
					else
					{
						//距离轨迹
						gui_sport_climbing_1_distance(1, TRACE_BACK_DATA.nearest_distance);
					}
				}
			}
			
			last_status = TRACE_BACK_DATA.whether_in_track;
		}
		else
		{
			//计时
			gui_sport_climbing_1_time();
			
			last_status = 2;
		}
		
	//	gui_crosscountry_realtime_battery_paint(205, g_bat_evt.level_percent);
		

		
		
		
		gui_sport_compass(0);
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
	//	gui_crosscountry_gps_paint();
		
	
	
				
}


//登山实时时间界面
void gui_sport_climbing_realtime_paint(void)
{
	//设置背景黑色
	LCD_SetBackgroundColor(LCD_BLACK);
	
	//圆点索引
    gui_sport_index_circle_paint(4, 5, 1);
	
	//实时时间
	//gui_sport_realtime_1_paint(1);
		
}
void gui_sport_climbing_heart_paint()
{

//设置背景黑色
	LCD_SetBackgroundColor(LCD_BLACK);
	//圆点索引
    gui_sport_index_circle_vert_paint(1, 3);
	
	//心率值
	gui_sport_heart_1_paint(Get_Sport_Heart(),1);







}

//登山界面
void gui_sport_climbing_paint(void)
{

   
   gui_sport_type = 2;
	if(m_gui_sport_climbing_index ==GUI_SPORT_CLIMBING_SCREEN_DATA_1 )
	{
	 	//gui_sport_climbing_data_1_paint();
		
		gui_sport_data_display_preview_paint(SPORT_DATA_SHOW);
	}
	else if(m_gui_sport_climbing_index ==GUI_SPORT_CLIMBING_SCREEN_DATA_2 )
	{
	 	//gui_sport_climbing_data_2_paint();
	 	
		//gui_sport_climbing_heart_paint();
		
		gui_sport_heart_paint(1);
	}
	/*else if (m_gui_sport_climbing_index ==GUI_SPORT_CLIMBING_SCREEN_HEART)
	{
		gui_sport_climbing_heart_paint();
	}
	else if (m_gui_sport_climbing_index ==GUI_SPORT_CLIMBING_SCREEN_TIME)
	{
		gui_sport_climbing_realtime_paint();
	}*/
	else
	{
		gui_sport_climbing_track_paint();
	}
	
	if(g_sport_status == SPORT_STATUS_PAUSE)
	{
		//暂时增加红色圆圈，表示暂停状态
		//LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_RED);
	}

}


//暂停菜单选项
static void gui_sport_climbing_pause_options(uint8_t index)
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
	if(index >= GUI_SPORT_CLIMBING_PAUSE_CANCEL)
	{
		index = GUI_SPORT_CLIMBING_PAUSE_CONTINUE;
	}
	else
	{

		index += 1;
		if((TRACE_BACK_DATA.is_traceback_flag) && (index == GUI_SPORT_CLIMBING_PAUSE_TRACKBACK))
		{
				//如果是在循迹返航过程，则无循迹返航选项
				index = GUI_SPORT_CLIMBING_PAUSE_CANCEL;
		}
		

	}
	word.x_axis = 180 + 18;
	word.forecolor = LCD_BLACK;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString((char *)ClimbPauseStrs[SetValue.Language][index],&word);
}


//登山暂停菜单界面
void gui_sport_climbing_pause_paint(void)
{
	LCD_SetBackgroundColor(LCD_BLACK);
	
	//暂停计时
	gui_sport_pause_time();
	
	//运动计时
	gui_sport_pause_acttime();
	
	LCD_SetRectangle(120 - 1 ,122,0,240,LCD_WHITE,0,0,LCD_FILL_ENABLE);
	LCD_SetRectangle(120 + 60 - 1,2,0,240,LCD_BLACK,0,0,LCD_FILL_ENABLE);

	
	//选项
	gui_sport_climbing_pause_options(m_gui_sport_climbing_pause_index);
	
	
	
	//下翻提示
	gui_sport_page_prompt(SPORT_PAGE_DOWN);

	LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_RED);

}

//登山放弃菜单界面
void gui_sport_climbing_cancel_paint(void)
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
	gui_sport_cancel_options(m_gui_sport_climbing_pause_index);
	
		
	//分割线
	LCD_SetRectangle(170, 1, 0, LCD_LINE_CNT_MAX, LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
		
	//下翻提示
	gui_sport_page_prompt(SPORT_PAGE_DOWN);	

	
}
//登山详细数据界面
void gui_sport_climbing_detail_paint(void)
{
	switch(m_gui_sport_climbing_pause_index)
	{
		case GUI_SPORT_CLIMBING_DETAIL_1:
			gui_climbing_save_detail_1_paint(LCD_WHITE);
			break;
		case GUI_SPORT_CLIMBING_DETAIL_2:
			gui_climbing_save_detail_2_paint(LCD_WHITE);
			break;	
		case GUI_SPORT_CLIMBING_DETAIL_TRACK:
			gui_sport_detail_track_paint(LCD_WHITE);
			break;
		/*case GUI_SPORT_CLIMBING_DETAIL_HEARTRATEZONE:
			gui_sport_detail_heartratezone_paint(LCD_BLACK);
			break;*/
		default:
			break;
	}
}


void gui_sport_climbing_init(void)
{
	//默认为跑步数据界面
	m_gui_sport_climbing_index = GUI_SPORT_CLIMBING_SCREEN_DATA_1;
}


void gui_sport_climbing_btn_evt(uint32_t Key_Value)
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
				
				ScreenState = DISPLAY_SCREEN_CLIMBING_PAUSE;
				m_gui_sport_climbing_pause_index = GUI_SPORT_CLIMBING_PAUSE_CONTINUE;
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
				if(m_gui_sport_climbing_index < (GUI_SPORT_CLIMBING_SCREEN_MAX -1))
				{
					m_gui_sport_climbing_index++;
				}
				else
				{
					m_gui_sport_climbing_index = 0;
				}
				
				gui_swich_msg();
			}
			break;
		case KEY_UP:
			if (g_sport_status != SPORT_STATUS_READY)
			{
           		//翻页查看跑步界面
				if(m_gui_sport_climbing_index > 0)
				{
					m_gui_sport_climbing_index--;
				}
				else
				{
					m_gui_sport_climbing_index = (GUI_SPORT_CLIMBING_SCREEN_MAX -1);
				}
				
				gui_swich_msg();
			}
		    break;
		case KEY_BACK:
 			if(g_sport_status == SPORT_STATUS_PAUSE)
			{
				//返回暂停菜单界面
				ScreenState = DISPLAY_SCREEN_CLIMBING_PAUSE;
				m_gui_sport_climbing_index = GUI_SPORT_CLIMBING_PAUSE_CONTINUE;
			
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
				if(m_gui_sport_climbing_index == GUI_SPORT_CLIMBING_SCREEN_TRACK)
				{	
					sport_dottrack();

				}	
			}
			break;
		default:
			break;
	}
	gui_sport_index = m_gui_sport_climbing_index;
	gui_sport_type = 2;
}

//登山暂停菜单界面按键事件
void gui_sport_climbing_pause_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
			if(m_gui_sport_climbing_pause_index == GUI_SPORT_CLIMBING_PAUSE_CONTINUE)
			{
				//继续计时，开始运动
				g_sport_status = SPORT_STATUS_START;
				Set_PauseSta(false);
				
				ScreenState = DISPLAY_SCREEN_CLIMBING;
				gui_swich_msg();
				Save_Detail_Index = ScreenState;
				timer_app_pasue_start();
				//振动提示
				gui_sport_motor_prompt();
			}
			else if(m_gui_sport_climbing_pause_index == GUI_SPORT_CLIMBING_PAUSE_SAVE)
			{
				//结束运动并保存数据
				g_sport_status = SPORT_STATUS_READY;
				
				//默认选项
				gui_sport_feel_init();
				
				ScreenState = DISPLAY_SCREEN_SPORT_FEEL;
				m_gui_sport_climbing_pause_index = GUI_SPORT_CLIMBING_DETAIL_1;
				sport_end_flag = 1;
				sport_end_hrvalue = get_10s_heartrate(); //获取前10s平均心率
				g_HeartNum = 0;
				extern volatile ScreenState_t Save_Detail_Index;
				Save_Detail_Index = DISPLAY_SCREEN_CLIMBING_DETAIL;
				gui_swich_msg();
			}
			else if(m_gui_sport_climbing_pause_index == GUI_SPORT_CLIMBING_PAUSE_WAIT)
			{
				m_gui_sport_climbing_pause_index = GUI_SPORT_CLIMBING_PAUSE_CONTINUE;
				
				//稍后继续
				sport_continue_later_start();
				
				//稍后继续保存运动类型
				ContinueLaterDataSave.SportType = DISPLAY_SCREEN_CLIMBING;
				
				//稍后继续，退出运动界面，返回主界面
				ScreenState = DISPLAY_SCREEN_HOME;
				gui_swich_msg();
			}
			else if(m_gui_sport_climbing_pause_index == GUI_SPORT_CLIMBING_PAUSE_TRACKBACK)
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
				
				ScreenState = DISPLAY_SCREEN_CLIMBING;
				gui_swich_msg();
				Save_Detail_Index = ScreenState;
				timer_app_pasue_start();
				gui_sport_motor_prompt();
			}
			else
			{
				//放弃运动
				ScreenState = DISPLAY_SCREEN_CLIMBING_CANCEL;
				
				m_gui_sport_climbing_pause_index = GUI_SPORT_CLIMBING_PAUSE_CANCEL_NO;
				gui_swich_msg();
			}
			
			break;
		case KEY_UP:
			if(m_gui_sport_climbing_pause_index > GUI_SPORT_CLIMBING_PAUSE_CONTINUE)
			{

			
			
				m_gui_sport_climbing_pause_index--;
				if((TRACE_BACK_DATA.is_traceback_flag) && (m_gui_sport_climbing_pause_index == GUI_SPORT_CLIMBING_PAUSE_TRACKBACK))
				{
						//如果是在循迹返航过程，则无循迹返航选项
						m_gui_sport_climbing_pause_index = GUI_SPORT_CLIMBING_PAUSE_WAIT;
				}
			


			}
			else
			{
				m_gui_sport_climbing_pause_index = GUI_SPORT_CLIMBING_PAUSE_CANCEL;
			}
			
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		    break;
		case KEY_DOWN:
			if(m_gui_sport_climbing_pause_index < GUI_SPORT_CLIMBING_PAUSE_CANCEL)
			{

			
				m_gui_sport_climbing_pause_index++;
				if((TRACE_BACK_DATA.is_traceback_flag) && (m_gui_sport_climbing_pause_index == GUI_SPORT_CLIMBING_PAUSE_TRACKBACK))
				{
					//如果是在循迹返航过程，则无循迹返航选项
					m_gui_sport_climbing_pause_index = GUI_SPORT_CLIMBING_PAUSE_CANCEL;
				}
				

			}
			else
			{
				m_gui_sport_climbing_pause_index = GUI_SPORT_CLIMBING_PAUSE_CONTINUE;
			}
			
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		case KEY_BACK:
			//返回骑行界面
			ScreenState = DISPLAY_SCREEN_CLIMBING;
		
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}

//放弃菜单界面按键事件
void gui_sport_climbing_cancel_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
			if(m_gui_sport_climbing_pause_index == GUI_SPORT_CLIMBING_PAUSE_CANCEL_NO)
			{
				//取消放弃运动，返回上级菜单界面
				ScreenState = DISPLAY_SCREEN_CLIMBING_PAUSE;
				m_gui_sport_climbing_pause_index = GUI_SPORT_CLIMBING_PAUSE_CANCEL;
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
			if(m_gui_sport_climbing_pause_index == GUI_SPORT_CLIMBING_PAUSE_CANCEL_NO)
			{
				m_gui_sport_climbing_pause_index = GUI_SPORT_CLIMBING_PAUSE_CANCEL_YES;
			}
			else
			{
				m_gui_sport_climbing_pause_index = GUI_SPORT_CLIMBING_PAUSE_CANCEL_NO;
			}

			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		    break;
		case KEY_BACK:
			//返回上级菜单
			ScreenState = DISPLAY_SCREEN_CLIMBING_PAUSE;
			m_gui_sport_climbing_pause_index = GUI_SPORT_CLIMBING_PAUSE_CANCEL;
			
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}

//详细数据界面按键事件
void gui_sport_climbing_detail_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_UP:
			//上翻
			if(m_gui_sport_climbing_pause_index > GUI_SPORT_CLIMBING_DETAIL_1)
			{
				m_gui_sport_climbing_pause_index--;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
		    break;
		case KEY_DOWN:
			//下翻
			if(m_gui_sport_climbing_pause_index < GUI_SPORT_CLIMBING_DETAIL_TRACK)
			{
				m_gui_sport_climbing_pause_index++;

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



//登山运动详细_1界面
void gui_climbing_save_detail_1_paint(uint8_t backgroundcolor)
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
		LCD_SetString("登山", &word);
	
	
		
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
		sprintf(str,"%d.%02d",ActivityData.ActivityDetails.ClimbingDetail.Distance/100000,ActivityData.ActivityDetails.ClimbingDetail.Distance/1000%100);
	//#else
	//	sprintf(str,"%0.2f",ActivityData.ActivityDetails.ClimbingDetail.Distance/100000.f);
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
	
	
	
	//平均速度(登山使用垂直速度)
		charlen = strlen(sportdetailstrsgod[SetValue.Language][14]);
		word.x_axis =  120 + 4 + 12;
		word.y_axis = 35;
		word.kerning = 0;
		word.size = LCD_FONT_16_SIZE;
		//LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][14],&word);
		
		LCD_SetString("垂直速度",&word);
		
		word.y_axis += charlen*8+1;
		LCD_SetString(colon,&word);
		// m/h:ActivityData.ActivityDetails.ClimbingDetail.Speed/10;
		
		memset(str,0,20);
		//sprintf(str,"%d:%01d",(ActivityData.ActivityDetails.ClimbingDetail.Speed/10),(ActivityData.ActivityDetails.ClimbingDetail.Speed%10));
		
		sprintf(str,"%.1f",ActivityData.ActivityDetails.ClimbingDetail.Speed/10.f);
		word.x_axis =  120 + 4 + 12 + 16 + 4;
		word.y_axis = 35;
		word.kerning = 1;
		word.size = LCD_FONT_19_SIZE;
		LCD_SetNumber(str,&word);
	
		
		word.x_axis =  120 + 4 + 12 + 16 + 4 + 19 - 16;
		word.y_axis += strlen(str)*10 - 0.5*10 + 8;
		word.kerning = 0;
	   word.size = LCD_FONT_16_SIZE;
		LCD_SetString("m/h",&word);
	
	   
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
		sprintf(str,"%d",ActivityData.ActivityDetails.ClimbingDetail.AvgHeart);
		
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


//登山运动详细_2界面
void gui_climbing_save_detail_2_paint(uint8_t backgroundcolor)
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
	  	word.bckgrndcolor = LCD_NONE;
		LCD_SetPicture(10, LCD_CENTER_JUSTIFIED, LCD_BLACK, LCD_NONE, &Img_Pointing_Up_12X8);
		
		//卡路里
		charlen = strlen(sportdetailstrsgod[SetValue.Language][4]);
		word.x_axis = 120 - 4 - 19 - 4 - 16;
		word.y_axis = 35;
		word.size = LCD_FONT_16_SIZE;
		word.kerning = 0;
		LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][4],&word);
		word.y_axis += charlen*8 + 1;
		LCD_SetString(colon,&word);
		memset(str,0,20);
		sprintf(str,"%d",ActivityData.ActivityDetails.ClimbingDetail.Calorie/1000);
		word.x_axis = 120 - 4 - 19;
		word.y_axis = 35;
		word.kerning = 1;
		word.size = LCD_FONT_19_SIZE;
		LCD_SetNumber(str,&word);
		word.x_axis = 120 - 4 - 16;
		word.y_axis = 35 + strlen(str)*10 - 0.5*10 + 8;
		word.kerning = 0;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString("kcal",&word);
	
	
		//总步数
		charlen = strlen(sportdetailstrsgod[SetValue.Language][5]);
		word.x_axis = 120 - 4 - 19 - 4 - 16;
		word.y_axis = 130;
		word.kerning = 0;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][5],&word);
		word.y_axis += charlen*8 + 1;
		LCD_SetString(colon,&word);
		memset(str,0,20);
		sprintf(str,"%d",ActivityData.ActivityDetails.ClimbingDetail.Steps);
		word.x_axis = 120 - 4 - 19;
		word.y_axis = 130;
		word.size = LCD_FONT_19_SIZE;
		word.kerning = 1;
		LCD_SetNumber(str,&word);
	
	
		//累积爬升(上升高度)
	   charlen = strlen(sportdetailstrsgod[SetValue.Language][8]);
		word.x_axis =  120 + 4 + 12;
		word.y_axis = 35;
		word.kerning = 0;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][8],&word);
		word.y_axis += charlen*8+1;
		LCD_SetString(colon,&word);
		memset(str,0,20);
		sprintf(str,"%d",(uint16_t)(ActivityData.ActivityDetails.ClimbingDetail.RiseHeight));
		word.x_axis =  120 + 4 + 12 + 16 + 4;
		word.y_axis = 35;
		word.kerning = 1;
		word.size = LCD_FONT_19_SIZE;
		LCD_SetNumber(str,&word);
		word.x_axis =  120 + 4 + 12 + 16 + 4 + 19 - 16;
		word.y_axis += strlen(str)*10 - 0.5*10 + 8;
		word.kerning = 0;
	   word.size = LCD_FONT_16_SIZE;
	   LCD_SetString("m",&word);
	
		
		#ifdef COD 
		//最高海拔
		charlen = strlen(sportdetailstrsgod[SetValue.Language][47]);
		word.x_axis = 120 + 4 + 12;
		word.y_axis = 130;
		word.kerning = 0;
		
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][47],&word);
		word.y_axis += charlen*8+1;
		LCD_SetString(colon,&word);
		memset(str,0,20);

		sprintf(str,"%d",ActivityData.ActivityDetails.ClimbingDetail.MaxAlt);//先用上升高度代替
		word.x_axis = 120 + 4 + 12 + 16 + 4;
		word.y_axis = 130;
		word.kerning = 1;
		
		word.size = LCD_FONT_19_SIZE;
		LCD_SetNumber(str,&word);
		word.y_axis += strlen(str)*10  + 8;
		word.kerning = 0;
		word.x_axis =  120 + 4 + 12 + 16 + 4 + 19 - 16;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString("m",&word);
		#endif
	
	
		
	
	
		LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, LCD_BLACK, LCD_NONE, &Img_Pointing_Down_12X8);
		
	 
	}





