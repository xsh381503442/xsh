#include "gui_sport_config.h"
#include "gui_sport.h"
#include "gui_run.h"
#include "gui_home.h"
#include "bsp_timer.h"

#include "drv_lcd.h"

#include "task_sport.h"
#include "algo_sport.h"
#include "timer_app.h"

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
#include "timer_traceback.h"
#include "time_notify.h"
#include "gui_trainplan.h"

#if DEBUG_ENABLED == 1 && GUI_RUN_LOG_ENABLED == 1
	#define GUI_RUN_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_RUN_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define GUI_RUN_WRITESTRING(...)
	#define GUI_RUN_PRINTF(...)		        
#endif
//#if defined WATCH_SIM_SPORT
#if defined WATCH_COM_SPORT

extern volatile ScreenState_t Save_Detail_Index;
extern uint8_t gui_sport_index;
extern uint8_t gui_sport_type;

uint8_t vice_screen_run_display[SPORT_DATA_DISPLAY_SUM] = {SPORT_DATA_DISPALY_TOTAL_DISTANCE,SPORT_DATA_DISPALY_AVE_PACE,SPORT_DATA_DISPALY_REALTIME_HEART,SPORT_DATA_DISPALY_TOTAL_TIME};

/*跑步数据显示当前项*/
uint8_t m_run_data_display_buf[SPORT_DATA_DISPLAY_SUM]={SPORT_DATA_DISPALY_REALTIME_HEART,SPORT_DATA_DISPALY_REALTIME_PACE,SPORT_DATA_DISPALY_TOTAL_TIME,SPORT_DATA_DISPALY_TOTAL_STEP};
/*跑步数据显示选项*/
/*const uint8_t m_run_data_display_option[RUN_DATA_DISPLAY_SUM]={SPORT_DATA_DISPALY_TOTAL_DISTANCE,SPORT_DATA_DISPALY_REALTIME_PACE,SPORT_DATA_DISPALY_TOTAL_TIME,SPORT_DATA_DISPALY_AVE_PACE,
	SPORT_DATA_DISPALY_OPTIMUM_PACE,SPORT_DATA_DISPALY_REALTIME_HEART,SPORT_DATA_DISPALY_AVE_HEART,SPORT_DATA_DISPALY_REALTIME_FREQUENCY,
	SPORT_DATA_DISPALY_AVE_FREQUENCY,SPORT_DATA_DISPALY_CALORIE};*/
const uint8_t m_run_data_display_option[RUN_DATA_DISPLAY_SUM]={SPORT_DATA_DISPALY_TOTAL_DISTANCE,SPORT_DATA_DISPALY_TOTAL_TIME,SPORT_DATA_DISPALY_AVE_PACE,SPORT_DATA_DISPALY_REALTIME_PACE,
	SPORT_DATA_DISPALY_TOTAL_STEP,SPORT_DATA_DISPALY_TOTAL_CLIMB,SPORT_DATA_DISPALY_REALTIME_HEART,SPORT_DATA_DISPALY_REALTIME_FREQUENCY};

/*跑步提醒设置选项*/
const uint8_t m_run_remind_set_option[RUN_REMIND_SET_SUM]={SPORT_REMIND_SET_HAERT,SPORT_REMIND_SET_HRRECOVERY,SPORT_REMIND_SET_PACE,SPORT_REMIND_SET_DISTANCE,SPORT_REMIND_SET_TIME,SPORT_REMIND_SET_AUTOCIRCLE};

#define	GUI_SPORT_RUN_SCREEN_DATA			0	//跑步数据界面
#define	GUI_SPORT_RUN_SCREEN_HEART			1	//跑步心率界面
//#define	GUI_SPORT_RUN_SCREEN_TIME			2	//跑步实时时间
#define	GUI_SPORT_RUN_SCREEN_TRACK			2	//跑步轨迹界面

#define	GUI_SPORT_RUN_PAUSE_CONTINUE	0	//继续
#define	GUI_SPORT_RUN_PAUSE_SAVE		1	//结束运动
#define	GUI_SPORT_RUN_PAUSE_WAIT		2	//稍后继续
#define	GUI_SPORT_RUN_PAUSE_TRACKBACK	3	//循迹返航
#define	GUI_SPORT_RUN_PAUSE_CANCEL		4	//放弃

#define	GUI_SPORT_RUN_PAUSE_CANCEL_NO	0	//否
#define	GUI_SPORT_RUN_PAUSE_CANCEL_YES	1	//是


#define	GUI_SPORT_RUN_DETAIL_1				0	//详细数据界面1
#define	GUI_SPORT_RUN_DETAIL_2				1	//详细数据界面2
#define GUI_SPORT_RUN_DETAIL_TRACK			2	//轨迹界面
#define	GUI_SPORT_RUN_DETAIL_HEARTRATEZONE	3	//心率区间界面


static uint8_t m_gui_sport_run_index;		//跑步界面索引
static uint8_t m_gui_sport_run_pause_index;	//跑步暂停菜单界面索引






	




//跑步实时时间界面
void gui_sport_run_realtime_paint(void)
{
	//设置背景黑色
	LCD_SetBackgroundColor(LCD_BLACK);
	
	//圆点索引
    gui_sport_index_circle_paint(2, 4, 1);
	
	//实时时间
	gui_sport_realtime_paint(1);
		
}
//剩余距离
static void gui_run_track_distance(uint8_t type, uint32_t distance)
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

//跑步轨迹更新
void gui_run_track_update_paint(void)
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
//跑步轨迹界面
void gui_sport_run_track_paint(void)
{

  	static uint8_t last_status = 2;
	
	//显示黑色背景
	LCD_SetBackgroundColor(LCD_WHITE);
    //轨迹
	gui_run_track_update_paint();

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
			
			gui_run_track_distance(2, 0);
		}
		else if((last_status == 1) && (TRACE_BACK_DATA.whether_in_track == 0))
		{
			//未在轨迹上
			timer_notify_motor_start(300,300,1,false,NOTIFY_MODE_SPORTS);
			timer_notify_buzzer_start(300,300,1,false,NOTIFY_MODE_SPORTS);
			
			gui_run_track_distance(3, 0);
		}
		else
		{
			if(TRACE_BACK_DATA.whether_in_track)
			{
				//剩余距离
				gui_run_track_distance(0, TRACE_BACK_DATA.remaining_distance);
			}
			else
			{
				//距离轨迹
				gui_run_track_distance(1, TRACE_BACK_DATA.nearest_distance);
			}
		}
		
		last_status = TRACE_BACK_DATA.whether_in_track;
	}
	else
	{
		last_status = 2;
	}
		
		
	//指北针
	gui_sport_compass(0);
		
	//GPS状态

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
}
//跑步界面
void gui_sport_run_paint(void)
{
   gui_sport_type = 0;
	if(m_gui_sport_run_index ==GUI_SPORT_RUN_SCREEN_DATA )
	{
       gui_sport_data_display_preview_paint(SPORT_DATA_SHOW);
     
		
	}
	else if (m_gui_sport_run_index ==GUI_SPORT_RUN_SCREEN_HEART)
	{
		gui_sport_heart_paint(1);
	}
	/*else if (m_gui_sport_run_index ==GUI_SPORT_RUN_SCREEN_TIME)
	{
		gui_sport_run_realtime_paint();
	}*/
	else
	{
		gui_sport_run_track_paint();
	}
	
	if(g_sport_status == SPORT_STATUS_PAUSE)
	{
      
	 // LCD_SetPicture(75,210,LCD_RED,LCD_NONE,&img_sport_pause_small);//右上角箭头,指示ok键
		
	}

}




//暂停菜单选项
static void gui_sport_run_pause_options(uint8_t index)
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
	if(index >= GUI_SPORT_RUN_PAUSE_CANCEL)
	{
		index = GUI_SPORT_RUN_PAUSE_CONTINUE;
	}
	else
	{
#if defined(WATCH_SIM_SPORT)
		if(Get_IsTrainPlanOnGoing() == true)
		{//训练计划 暂停界面
			if(index == GUI_SPORT_RUN_PAUSE_SAVE)
			{
				index += 3;
			}
			else if(index == GUI_SPORT_RUN_PAUSE_WAIT)
			{
				index += 2;
			}
			else
			{
				index += 1;
			}
		}
		else
		{
			index += 1;
			if((TRACE_BACK_DATA.is_traceback_flag) && (index == GUI_SPORT_RUN_PAUSE_TRACKBACK))
			{
				//如果是在循迹返航过程，则无循迹返航选项
				index = GUI_SPORT_RUN_PAUSE_CANCEL;
			}
		}
#else
		index += 1;
		if((TRACE_BACK_DATA.is_traceback_flag) && (index == GUI_SPORT_RUN_PAUSE_TRACKBACK))
		{
			//如果是在循迹返航过程，则无循迹返航选项
			index = GUI_SPORT_RUN_PAUSE_CANCEL;
		}
#endif
	}
	word.x_axis = 180 + 18;
	word.forecolor = LCD_BLACK;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString((char *)ClimbPauseStrs[SetValue.Language][index],&word);
}


//跑步暂停菜单界面
void gui_sport_run_pause_paint(void)
{
	
	LCD_SetBackgroundColor(LCD_BLACK);
	
	//暂停计时
	gui_sport_pause_time();
	
	//运动计时
	gui_sport_pause_acttime();
	
	LCD_SetRectangle(120 - 1 ,122,0,240,LCD_WHITE,0,0,LCD_FILL_ENABLE);
	LCD_SetRectangle(120 + 60 - 1,2,0,240,LCD_BLACK,0,0,LCD_FILL_ENABLE);

	
	//选项
	gui_sport_run_pause_options(m_gui_sport_run_pause_index);
	
	
	
	//下翻提示
	gui_sport_page_prompt(SPORT_PAGE_DOWN);

	LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_RED);
	
	
}

//跑步放弃菜单界面
void gui_sport_run_cancel_paint(void)
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
	gui_sport_cancel_options(m_gui_sport_run_pause_index);
		
	//分割线
	LCD_SetRectangle(170, 1, 0, LCD_LINE_CNT_MAX, LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
		
	//下翻提示
	gui_sport_page_prompt(SPORT_PAGE_DOWN);	

}
//跑步详细数据界面
void gui_sport_run_detail_paint(void)
{
	switch(m_gui_sport_run_pause_index)
	{
		case GUI_SPORT_RUN_DETAIL_1:
			gui_run_save_detail_1_paint(LCD_WHITE);
			break;
		case GUI_SPORT_RUN_DETAIL_2:
			gui_run_save_detail_2_paint(LCD_WHITE);
			break;
		case GUI_SPORT_RUN_DETAIL_TRACK:
			gui_sport_detail_track_paint(LCD_WHITE);
			break;
		/*case GUI_SPORT_RUN_DETAIL_HEARTRATEZONE:
			gui_sport_detail_heartratezone_paint(LCD_BLACK);
			break;*/
		default:
			break;
	}
}

void gui_sport_run_init(void)
{
	//默认为跑步数据界面
	m_gui_sport_run_index = GUI_SPORT_RUN_SCREEN_DATA;
}
/*跑步界面按OK键暂停*/
void gui_sport_pause_evt(void)
{
	if(g_sport_status == SPORT_STATUS_START)
	{
		//暂停运动，进入暂停菜单界面
		g_sport_status = SPORT_STATUS_PAUSE;
		Set_PauseSta(true);
		
		ScreenState = DISPLAY_SCREEN_RUN_PAUSE;
		m_gui_sport_run_pause_index = GUI_SPORT_RUN_PAUSE_CONTINUE;
		gui_swich_msg();
		
		//振动提示
		gui_sport_motor_prompt();
	}
	else if(g_sport_status == SPORT_STATUS_PAUSE)
	{
		//继续计时
		g_sport_status = SPORT_STATUS_START;
		Set_PauseSta(false);
		
		gui_swich_msg();
		
		//振动提示
		gui_sport_motor_prompt();
	}
}
//跑步界面按键事件
void gui_sport_run_btn_evt(uint32_t Key_Value)
{
	switch( Key_Value )
	{
		case KEY_OK:
			 if(g_sport_status == SPORT_STATUS_START)
			{
				//暂停运动，进入暂停菜单界面
				g_sport_status = SPORT_STATUS_PAUSE;
				Set_PauseSta(true);
				
				ScreenState = DISPLAY_SCREEN_RUN_PAUSE;
				m_gui_sport_run_pause_index = GUI_SPORT_RUN_PAUSE_CONTINUE;
				Save_Detail_Index = ScreenState;
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
             //翻页查看跑步界面
			if(m_gui_sport_run_index < GUI_SPORT_RUN_SCREEN_TRACK)
			{
				m_gui_sport_run_index++;
			}
			else
			{
				m_gui_sport_run_index = GUI_SPORT_RUN_SCREEN_DATA;
			}
				
			gui_swich_msg();

			break;
		case KEY_UP:
           	//翻页查看跑步界面
			if(m_gui_sport_run_index > GUI_SPORT_RUN_SCREEN_DATA)
			{
				m_gui_sport_run_index--;
			}
			else
			{
				m_gui_sport_run_index = GUI_SPORT_RUN_SCREEN_TRACK;
			}
				
			gui_swich_msg();
		    break;
		case KEY_BACK:
 			if(g_sport_status == SPORT_STATUS_PAUSE)
			{
				//返回暂停菜单界面
				ScreenState = DISPLAY_SCREEN_RUN_PAUSE;
				m_gui_sport_run_index = GUI_SPORT_RUN_PAUSE_CONTINUE;
			
				gui_swich_msg();
			
			}
			break;
		case KEY_LONG_UP:
			if(g_sport_status == SPORT_STATUS_START)
			{
				//轨迹界面保存航迹打点数据
				if(m_gui_sport_run_index == GUI_SPORT_RUN_SCREEN_TRACK)
				{	
					sport_dottrack();

				}	
			}
			break;
		default:
			break;
	}

	gui_sport_index = m_gui_sport_run_index;
	gui_sport_type = 0;
}


//跑步暂停菜单界面按键事件
void gui_sport_run_pause_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
			if(m_gui_sport_run_pause_index == GUI_SPORT_RUN_PAUSE_CONTINUE)
			{
				//继续计时，开始运动
				g_sport_status = SPORT_STATUS_START;
				Set_PauseSta(false);
				
				ScreenState = DISPLAY_SCREEN_RUN;
				gui_swich_msg();
				Save_Detail_Index = ScreenState;
				timer_app_pasue_start();
				//振动提示
				gui_sport_motor_prompt();
			}
			else if(m_gui_sport_run_pause_index == GUI_SPORT_RUN_PAUSE_SAVE)
			{
				//结束运动并保存数据
				g_sport_status = SPORT_STATUS_READY;
				
				//默认选项
				gui_sport_feel_init();
				
				ScreenState = DISPLAY_SCREEN_SPORT_FEEL;
				m_gui_sport_run_pause_index = GUI_SPORT_RUN_DETAIL_1;
				sport_end_flag = 1;
				sport_end_hrvalue = get_10s_heartrate(); //获取前10s平均心率
				g_HeartNum = 0;
				extern volatile ScreenState_t Save_Detail_Index;
				Save_Detail_Index = DISPLAY_SCREEN_RUN_DETAIL;
				gui_swich_msg();
			}
			else if(m_gui_sport_run_pause_index == GUI_SPORT_RUN_PAUSE_WAIT)
			{
				m_gui_sport_run_pause_index = GUI_SPORT_RUN_PAUSE_CONTINUE;
				
				//稍后继续
				sport_continue_later_start();
				
				//稍后继续保存运动类型
				ContinueLaterDataSave.SportType = DISPLAY_SCREEN_RUN;
				
				//稍后继续，退出运动界面，返回主界面
				ScreenState = DISPLAY_SCREEN_HOME;
				gui_swich_msg();
			}
			else if(m_gui_sport_run_pause_index == GUI_SPORT_RUN_PAUSE_TRACKBACK)
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
				
				ScreenState = DISPLAY_SCREEN_RUN;
				gui_swich_msg();
				Save_Detail_Index = ScreenState;
				timer_app_pasue_start();
				gui_sport_motor_prompt();
			}
			else
			{
				//放弃运动
				ScreenState = DISPLAY_SCREEN_RUN_CANCEL;
				
				m_gui_sport_run_pause_index = GUI_SPORT_RUN_PAUSE_CANCEL_NO;
				gui_swich_msg();
			}
			
			break;
		case KEY_UP:
			if(m_gui_sport_run_pause_index > GUI_SPORT_RUN_PAUSE_CONTINUE)
			{
	#if defined(WATCH_SIM_SPORT)
				if(Get_IsTrainPlanOnGoing() == true)
				{//若处于训练计划中,暂停界面过滤稍后继续、循迹返航
					if(m_gui_sport_run_pause_index == GUI_SPORT_RUN_PAUSE_CANCEL
						|| m_gui_sport_run_pause_index == GUI_SPORT_RUN_PAUSE_WAIT
						|| m_gui_sport_run_pause_index == GUI_SPORT_RUN_PAUSE_TRACKBACK)
					{
						m_gui_sport_run_pause_index = GUI_SPORT_RUN_PAUSE_SAVE;
					}
					else
					{
						m_gui_sport_run_pause_index--;
						if((TRACE_BACK_DATA.is_traceback_flag) && (m_gui_sport_run_pause_index == GUI_SPORT_RUN_PAUSE_TRACKBACK))
						{
							//如果是在循迹返航过程，则无循迹返航选项
							m_gui_sport_run_pause_index = GUI_SPORT_RUN_PAUSE_WAIT;
						}
					}
				}
				else
				{
					m_gui_sport_run_pause_index--;
					if((TRACE_BACK_DATA.is_traceback_flag) && (m_gui_sport_run_pause_index == GUI_SPORT_RUN_PAUSE_TRACKBACK))
					{
						//如果是在循迹返航过程，则无循迹返航选项
						m_gui_sport_run_pause_index = GUI_SPORT_RUN_PAUSE_WAIT;
					}
				}
#else		
				m_gui_sport_run_pause_index--;
				if((TRACE_BACK_DATA.is_traceback_flag) && (m_gui_sport_run_pause_index == GUI_SPORT_RUN_PAUSE_TRACKBACK))
				{
					//如果是在循迹返航过程，则无循迹返航选项
					m_gui_sport_run_pause_index = GUI_SPORT_RUN_PAUSE_WAIT;
				}
#endif			
           }
			else
			{
				m_gui_sport_run_pause_index = GUI_SPORT_RUN_PAUSE_CANCEL;
			}
			
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		    break;
		case KEY_DOWN:
			if(m_gui_sport_run_pause_index < GUI_SPORT_RUN_PAUSE_CANCEL)
			{
	#if defined(WATCH_SIM_SPORT)
				if(Get_IsTrainPlanOnGoing() == true)
				{//若处于训练计划中,暂停界面过滤稍后继续、循迹返航
					if(m_gui_sport_run_pause_index == GUI_SPORT_RUN_PAUSE_SAVE
						|| m_gui_sport_run_pause_index == GUI_SPORT_RUN_PAUSE_WAIT
						|| m_gui_sport_run_pause_index == GUI_SPORT_RUN_PAUSE_TRACKBACK)
					{
						m_gui_sport_run_pause_index = GUI_SPORT_RUN_PAUSE_CANCEL;
					}
					else
					{
						m_gui_sport_run_pause_index++;
						if((TRACE_BACK_DATA.is_traceback_flag) && (m_gui_sport_run_pause_index == GUI_SPORT_RUN_PAUSE_TRACKBACK))
						{
							//如果是在循迹返航过程，则无循迹返航选项
							m_gui_sport_run_pause_index = GUI_SPORT_RUN_PAUSE_CANCEL;
						}
					}
				}
				else
				{
					m_gui_sport_run_pause_index++;
					if((TRACE_BACK_DATA.is_traceback_flag) && (m_gui_sport_run_pause_index == GUI_SPORT_RUN_PAUSE_TRACKBACK))
					{
						//如果是在循迹返航过程，则无循迹返航选项
						m_gui_sport_run_pause_index = GUI_SPORT_RUN_PAUSE_CANCEL;
					}
				}
#else			
				m_gui_sport_run_pause_index++;
				if((TRACE_BACK_DATA.is_traceback_flag) && (m_gui_sport_run_pause_index == GUI_SPORT_RUN_PAUSE_TRACKBACK))
				{
					//如果是在循迹返航过程，则无循迹返航选项
					m_gui_sport_run_pause_index = GUI_SPORT_RUN_PAUSE_CANCEL;
				}
#endif			
			}
			else
			{
				m_gui_sport_run_pause_index = GUI_SPORT_RUN_PAUSE_CONTINUE;
			}
			
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		case KEY_BACK:
			ScreenState = DISPLAY_SCREEN_RUN;
		
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}

//放弃菜单界面按键事件
void gui_sport_run_cancel_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
			if(m_gui_sport_run_pause_index == GUI_SPORT_RUN_PAUSE_CANCEL_NO)
			{
				//取消放弃运动，返回上级菜单界面
				ScreenState = DISPLAY_SCREEN_RUN_PAUSE;
				m_gui_sport_run_pause_index = GUI_SPORT_RUN_PAUSE_CANCEL;
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
			if(m_gui_sport_run_pause_index == GUI_SPORT_RUN_PAUSE_CANCEL_NO)
			{
				m_gui_sport_run_pause_index = GUI_SPORT_RUN_PAUSE_CANCEL_YES;
			}
			else
			{
				m_gui_sport_run_pause_index = GUI_SPORT_RUN_PAUSE_CANCEL_NO;
			}

			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		    break;
		case KEY_BACK:
			//返回上级菜单
			ScreenState = DISPLAY_SCREEN_RUN_PAUSE;
			m_gui_sport_run_pause_index = GUI_SPORT_RUN_PAUSE_CANCEL;
			
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}

//详细数据界面按键事件
void gui_sport_run_detail_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_UP:
			//上翻
			if(m_gui_sport_run_pause_index > GUI_SPORT_RUN_DETAIL_1)
			{
				m_gui_sport_run_pause_index--;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
		    break;
		case KEY_DOWN:
			//下翻
			//if(m_gui_sport_run_pause_index < GUI_SPORT_RUN_DETAIL_HEARTRATEZONE)
				
			if(m_gui_sport_run_pause_index < GUI_SPORT_RUN_DETAIL_TRACK)
				
			{
				m_gui_sport_run_pause_index++;

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


//#elif defined WATCH_COM_SPORT

/*#define	GUI_SPORT_RUN_SCREEN_DATA			0	//跑步数据界面
#define	GUI_SPORT_RUN_SCREEN_HEART			1	//跑步心率界面
#define	GUI_SPORT_RUN_SCREEN_TRACK			2	//跑步轨迹界面
#define	GUI_SPORT_RUN_SCREEN_TIME			3	//跑步实时时间
#define GUI_SPORT_RUN_SCREEN_MAX			4


#define	GUI_SPORT_RUN_PAUSE_CONTINUE	0	//继续
#define	GUI_SPORT_RUN_PAUSE_SAVE		1	//结束运动
#define	GUI_SPORT_RUN_PAUSE_WAIT		2	//稍后继续
#define	GUI_SPORT_RUN_PAUSE_TRACKBACK	3	//循迹返航
#define	GUI_SPORT_RUN_PAUSE_CANCEL		4	//放弃

#define	GUI_SPORT_RUN_PAUSE_CANCEL_NO	0	//否
#define	GUI_SPORT_RUN_PAUSE_CANCEL_YES	1	//是


#define	GUI_SPORT_RUN_DETAIL_1				0	//详细数据界面1
#define	GUI_SPORT_RUN_DETAIL_2				1	//详细数据界面2
#define GUI_SPORT_RUN_DETAIL_TRACK			2	//轨迹界面
#define	GUI_SPORT_RUN_DETAIL_HEARTRATEZONE	3	//心率区间界面


static uint8_t m_gui_sport_run_index;		//跑步界面索引
static uint8_t m_gui_sport_run_pause_index;*/	//跑步暂停菜单界面索引

//extern volatile ScreenState_t Save_Detail_Index;


//按ok键运动开始提示
/*void gui_run_prompt_paint(void)
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

}*/


#else

#endif
//跑步运动详细_1界面
void gui_run_save_detail_1_paint(uint8_t backgroundcolor)
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
	LCD_SetString("跑步", &word);


	
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
	sprintf(str,"%d.%02d",ActivityData.ActivityDetails.RunningDetail.Distance/100000,ActivityData.ActivityDetails.RunningDetail.Distance/1000%100);
	//#else
  //  sprintf(str,"%0.2f",ActivityData.ActivityDetails.RunningDetail.Distance/100000.f);
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
    sprintf(str,"%d:%02d",(ActivityData.ActivityDetails.RunningDetail.Pace/60),(ActivityData.ActivityDetails.RunningDetail.Pace%60));
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
    sprintf(str,"%d",ActivityData.ActivityDetails.RunningDetail.AvgHeart);
	
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
//最佳配速 
  /*  charlen = strlen(SportDetailStrs[SetValue.Language][3]);
    word.x_axis = 180;
    word.y_axis = 95 - charlen*8;
    word.kerning = 0;
    LCD_SetString((char *)SportDetailStrs[SetValue.Language][3],&word);
    word.y_axis = 100;
    LCD_SetString(colon,&word);
    memset(str,0,20);
    sprintf(str,"%d:%02d",(ActivityData.ActivityDetails.RunningDetail.OptimumPace/60),(ActivityData.ActivityDetails.RunningDetail.OptimumPace%60));
    word.y_axis = 112;
    word.kerning = 1;
    LCD_SetNumber(str,&word);
    word.y_axis = SPORT_DETAIL_DATA_UNITS_Y_AXIS;
    word.kerning = 0;
    LCD_SetString("/km",&word);

    LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, word.forecolor, LCD_NONE, &Img_Pointing_Down_12X8);*/
    
 
}

//跑步运动详细_2界面
void gui_run_save_detail_2_paint(uint8_t backgroundcolor)
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
    word.x_axis = 120 - 4 - 19 - 4 - 16 - 16;
	word.y_axis = 35;
    word.size = LCD_FONT_16_SIZE;
    word.kerning = 0;
    LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][4],&word);
    word.y_axis += charlen*8 + 1;
    LCD_SetString(colon,&word);
    memset(str,0,20);
    sprintf(str,"%d",ActivityData.ActivityDetails.RunningDetail.Calorie/1000);
    word.x_axis = 120 - 4 - 19 - 16;
    word.y_axis = 35;
    word.kerning = 1;
    word.size = LCD_FONT_19_SIZE;
    LCD_SetNumber(str,&word);
    word.x_axis = 120 - 4 - 16 - 16;
    word.y_axis = 35 + strlen(str)*10 - 0.5*10 + 8;
    word.kerning = 0;
    word.size = LCD_FONT_16_SIZE;
    LCD_SetString("kcal",&word);


	//总步数
    charlen = strlen(sportdetailstrsgod[SetValue.Language][5]);
    word.x_axis = 120 - 4 - 19 - 4 - 16 - 16;
    word.y_axis = 130;
    word.kerning = 0;
    word.size = LCD_FONT_16_SIZE;
    LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][5],&word);
	word.y_axis += charlen*8 + 1;
    LCD_SetString(colon,&word);
    memset(str,0,20);
    sprintf(str,"%d",ActivityData.ActivityDetails.RunningDetail.Steps);
    word.x_axis = 120 - 4 - 19 - 16;
    word.y_axis = 130;
    word.size = LCD_FONT_19_SIZE;
    word.kerning = 1;
    LCD_SetNumber(str,&word);
   // word.y_axis = SPORT_DETAIL_DATA_UNITS_Y_AXIS;
   // word.kerning = 0;
   // LCD_SetString("bpm",&word);

    //平均步频
    charlen = strlen(sportdetailstrsgod[SetValue.Language][6]);
     word.x_axis =  120;
    word.y_axis = 35;
    word.kerning = 0;
    word.size = LCD_FONT_16_SIZE;
    LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][6],&word);
    word.y_axis += charlen*8+1;
    LCD_SetString(colon,&word);
    memset(str,0,20);
    sprintf(str,"%d",(uint16_t)((ActivityData.ActivityDetails.RunningDetail.Steps*60) / ActivityData.ActTime));
     word.x_axis =  120  + 16 + 4;
    word.y_axis = 35;
    word.kerning = 1;
    word.size = LCD_FONT_19_SIZE;
    LCD_SetNumber(str,&word);
    word.x_axis =  120  + 16 + 4 + 19 - 16;
    word.y_axis += strlen(str)*10 - 0.5*10 + 8;
    word.kerning = 0;
   word.size = LCD_FONT_16_SIZE;
    LCD_SetString("spm",&word);



    //平均步幅
    charlen = strlen(sportdetailstrsgod[SetValue.Language][7]);
    word.x_axis = 120;
    word.y_axis = 130;
    word.kerning = 0;
	
    word.size = LCD_FONT_16_SIZE;
    LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][7],&word);
    word.y_axis += charlen*8+1;
    LCD_SetString(colon,&word);
    memset(str,0,20);
	#if defined (SPRINTF_FLOAT_TO_INT)
	sprintf(str,"%d.%01d",ActivityData.ActivityDetails.RunningDetail.RecoveryTime/60,ActivityData.ActivityDetails.RunningDetail.RecoveryTime%60);
	#else
	
    sprintf(str,"%d",(uint16_t)(ActivityData.ActivityDetails.RunningDetail.Distance/ActivityData.ActivityDetails.RunningDetail.Steps));
	#endif
	//sprintf(str,"%d",120);//步幅测试值
    word.x_axis = 120 + 16 + 4;
    word.y_axis = 130;
    word.kerning = 1;
	
    word.size = LCD_FONT_19_SIZE;
    LCD_SetNumber(str,&word);
    word.y_axis += strlen(str)*10  + 8;
    word.kerning = 0;
	word.x_axis =  120  + 16 + 4 + 19 - 16;
    word.size = LCD_FONT_16_SIZE;
    LCD_SetString("cm",&word);



    //累积爬升
    charlen = strlen(sportdetailstrsgod[SetValue.Language][8]);
    word.x_axis =  120 + 4 + 16 + 4 - 16 + 19 + 32; //179
    word.y_axis = 35;
    word.kerning = 0;
    word.size = LCD_FONT_16_SIZE;
    LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][8],&word);
	word.y_axis += charlen*8+1;
    LCD_SetString(colon,&word);

    memset(str,0,20);
    sprintf(str,"%d",(uint16_t)ActivityData.ActivityDetails.RunningDetail.RiseHeight);
    word.x_axis =  120 + 4 + 16 + 4 - 16 + 19 + 32 + 16 - 19;
    word.y_axis += 8;
    word.kerning = 1;
    word.size = LCD_FONT_19_SIZE;
    LCD_SetNumber(str,&word);
	
     word.x_axis = 179;
     word.y_axis += strlen(str)*10  + 8;
	 word.kerning = 0;
	 word.size = LCD_FONT_16_SIZE;
	 LCD_SetString("m",&word);


    LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, LCD_BLACK, LCD_NONE, &Img_Pointing_Down_12X8);
    
 
}
#if defined WATCH_SIM_SPORT
#elif defined WATCH_COM_SPORT
#else
//跑步运动详细轨迹界面
void gui_run_save_track_paint(uint8_t backgroundcolor)
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
  
    LCD_SetPicture(10, LCD_CENTER_JUSTIFIED, word.forecolor, LCD_NONE, &Img_Pointing_Up_12X8);

    word.x_axis = 25;
    word.y_axis = LCD_CENTER_JUSTIFIED;
    word.size = LCD_FONT_16_SIZE;
   // word.forecolor = LCD_WHITE;
    word.bckgrndcolor = LCD_NONE;
    LCD_SetString((char *)SportDetailStrs[SetValue.Language][12],&word);
    //保存轨迹显示
    DrawSaveTrack();
 

    LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, word.forecolor, LCD_NONE, &Img_Pointing_Down_12X8);  
 
}


//跑步运动详细心率区间界面
void gui_run_save_heartratezone_paint(uint8_t backgroundcolor)
{
   SetWord_t word = {0};
   char str[10];
   uint8_t width,i;
   uint32_t MaxHeartRateZone = 0;
   uint8_t charlen;
   
   //假数据，调试示例，后面去掉
  // ActivityData.ActivityDetails.RunningDetail.HeartRateZone[0] = 533;
  //ActivityData.ActivityDetails.RunningDetail.HeartRateZone[1] = 2222;
  // ActivityData.ActivityDetails.RunningDetail.HeartRateZone[2] = 12220;
  // ActivityData.ActivityDetails.RunningDetail.HeartRateZone[3] = 1233;
   //ActivityData.ActivityDetails.RunningDetail.HeartRateZone[4] = 333;


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

//循迹返航数据界面绘制
void gui_run_data_trackback_paint(void)
{
    SetWord_t word = {0};

    //设置背景黑色
	LCD_SetBackgroundColor(LCD_BLACK);
    
    gui_sport_index_circle_paint(0, 2, 1);
    
    gui_run_distance_paint(ActivityData.ActivityDetails.RunningDetail.Distance);

    
    word.x_axis = 80;
	LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,LCD_AUQAMARIN,0,0,LCD_FILL_ENABLE);

    gui_sport_gps_status(SPORT_GPS_LEFT1);

    gui_run_time_paint(97,30,LCD_WHITE);

    gui_run_heartrate_paint(Get_Sport_Heart());
    
    word.x_axis = 160;
	LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,LCD_AUQAMARIN,0,0,LCD_FILL_ENABLE);

    gui_run_speed_paint(ActivityData.ActivityDetails.RunningDetail.Pace);

    gui_sport_realtime_battery(0);
    
    //循迹返航没有定义数据暂停索引，暂时通过状态来获取，与run_data处理方式不一样
    //if (Get_PauseSta() == true)
    //{
    //    LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_RED);
    //}
    
}

//循迹返航轨迹界面绘制
void gui_run_track_trackback_paint(void)
{
    //SetWord_t word = {0};

    //设置背景黑色
		LCD_SetBackgroundColor(LCD_BLACK);

    gui_run_track_update_paint();
     
    gui_sport_index_circle_paint(1, 2, 1);

	gui_run_distance_paint(ActivityData.ActivityDetails.RunningDetail.Distance);
	
    gui_sport_gps_status(SPORT_GPS_LEFT2);
  
    gui_sport_realtime_battery(1); 
    gui_sport_compass(0);

   //循迹返航没有定义数据暂停索引，暂时通过状态来获取，与run_track处理方式不一样
    if (Get_PauseSta() == true)
    {
        LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_RED);
    }
    
    
}

//循迹返航暂停界面绘制
void gui_run_trackback_pause_paint(void)
{
	SetWord_t word = {0};

	//设置背景白色
	LCD_SetBackgroundColor(LCD_WHITE);
	gui_run_pause_time_paint();
	gui_run_time_paint(57,45,LCD_BLACK);

	LCD_SetRectangle(120,120,0,LCD_LINE_CNT_MAX,LCD_BLACK,0,0,LCD_FILL_ENABLE);

	gui_run_trackback_pause_options_paint(run_pause_index);

	word.x_axis = 170;
	LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,LCD_AUQAMARIN,0,0,LCD_FILL_ENABLE);

	LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, LCD_WHEAT, LCD_NONE, &Img_Pointing_Down_12X8);

	LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_RED);

}

/*
//界面切换消息
void gui_swich_msg(void)
{
    DISPLAY_MSG  msg = {0};
    msg.cmd = MSG_DISPLAY_SCREEN;
    xQueueSend(DisplayQueue, &msg, portMAX_DELAY);

}


//运动后下键切换。FirstIndex : 运动中第一个子界面， LastIndex:运动中最后个子界面
void gui_sport_index_down(ScreenState_t FirstIndex,ScreenState_t LastIndex)
{
   // DISPLAY_MSG  msg = {0};
    if( Sport_Index == FirstIndex )
    {
        Sport_Index = LastIndex;
    }
    else
    {
        Sport_Index--;
    }
    ScreenState = Sport_Index;
    
    gui_swich_msg();

}

//运动后上键切换。FirstIndex : 运动中第一个子界面， LastIndex:运动中最后个子界面
void gui_sport_index_up(ScreenState_t FirstIndex,ScreenState_t LastIndex)
{
    //DISPLAY_MSG  msg = {0};
    if( Sport_Index == LastIndex )
    {
        Sport_Index = FirstIndex;
    }
    else
    {
        Sport_Index++;
    }
    ScreenState = Sport_Index ;
    gui_swich_msg();

}
*/

//跑步准备界面按键处理
void gui_run_ready_btn_evt(uint32_t Key_Value)
{
   // DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
						gui_sport_motor_prompt();	
						ScreenState = DISPLAY_SCREEN_RUN_DATA;
             Sport_Index = DISPLAY_SCREEN_RUN_DATA;
             Save_Detail_Index = DISPLAY_SCREEN_RUN_SAVE_DETIAL_1;
             gui_swich_msg();
             StartSport();
			break;
	
		case KEY_BACK:
			Set_IsTrainPlanOnGoing(false);
			if(0 == ENTER_NAVIGATION && 0 == ENTER_MOTION_TRAIL)    //由导航界面进入运动ready界面的判断
			{
				if(Get_IsCompleteTodayTrainPlan() != 1 && get_trainplan_valid_flag()
					 && Get_SomeDayofTrainPlan() > 0 && Get_TrainPlanBeforeDays() == 0 )
				{//训练计划进入的跑步运动
						if(get_enter_trainplan_way() == WAYS_ENTER_TRAINPLAN_SPORT_STANDBUY)
					 {//从待机快捷界面进入的训练计划 原路返回
						ScreenState = DISPLAY_SCREEN_TRAINPLAN_MARATHON;
					 }
					 else if(get_enter_trainplan_way() == WAYS_ENTER_TRAINPLAN_SPORT_RUN)
					 {//从跑步急么界面进入的训练计划 原路返回
						 ScreenState = DISPLAY_SCREEN_TRAINPLAN_RUN_SELECT;
					 }
					 else
					 {//提醒进入的训练计划等 返回运动界面
					 	 ScreenState = DISPLAY_SCREEN_SPORT;
					 }
				}
				else
				{//非训练计划进入的正常跑步运动
					ScreenState = DISPLAY_SCREEN_SPORT;
				}
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
			}
			
			gui_swich_msg();
			break;
		default:
			break;
	}	
}
/*用于按键进入运动暂停界面*/
void Go_to_Run_Pause(void)
{
	gui_sport_motor_prompt();
	ScreenState = DISPLAY_SCREEN_RUN_PAUSE;
	run_pause_index = RunPauseContinue; 
	Set_PauseSta(true);
	gui_swich_msg();
}

//跑步轨迹界面按键处理
void gui_run_track_btn_evt(uint32_t Key_Value)
{
   //DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
						gui_sport_motor_prompt();
						ScreenState = DISPLAY_SCREEN_RUN_PAUSE;
             run_pause_index = RunPauseContinue; 
            Set_PauseSta(true);
             gui_swich_msg();
			break;
		case KEY_DOWN:
            gui_sport_index_down(DISPLAY_SCREEN_RUN_DATA,DISPLAY_SCREEN_RUN_TRACK);

			break;
		case KEY_UP:
            gui_sport_index_up(DISPLAY_SCREEN_RUN_DATA,DISPLAY_SCREEN_RUN_TRACK);

            break; 
		case KEY_BACK:
 
			break;
		case KEY_LONG_UP:
			//保存航迹打点数据
			sport_dottrack();

			break;
		default:
			break;
	}	
}


//跑步数据界面按键处理
void gui_run_data_btn_evt(uint32_t Key_Value)
{
   //DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
							gui_sport_motor_prompt();				
							ScreenState = DISPLAY_SCREEN_RUN_PAUSE;
             run_pause_index = RunPauseContinue ;
            Set_PauseSta(true);
             gui_swich_msg();
			break;
		case KEY_DOWN:
            gui_sport_index_down(DISPLAY_SCREEN_RUN_DATA,DISPLAY_SCREEN_RUN_TRACK);

			break;
		case KEY_UP:
            gui_sport_index_up(DISPLAY_SCREEN_RUN_DATA,DISPLAY_SCREEN_RUN_TRACK);

		    break;
		case KEY_BACK:
 
			break;
		default:
			break;
	}	
}

//跑步暂停界面按键处理
void gui_run_pause_btn_evt(uint32_t Key_Value)
{
    //DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:  
            if (run_pause_index == RunPauseContinue)
            {
								gui_sport_motor_prompt();           

                if (Sport_Index == DISPLAY_SCREEN_RUN_PAUSE_DATA)
                {
                    Sport_Index = DISPLAY_SCREEN_RUN_DATA;

                }
                else if (Sport_Index ==DISPLAY_SCREEN_RUN_PAUSE_TRACK)
                {
                    Sport_Index = DISPLAY_SCREEN_RUN_TRACK;
                }

                ScreenState = Sport_Index;
               Set_PauseSta(false);

            }
            else if (run_pause_index == RunPauseSave)
            {
               ScreenState = DISPLAY_SCREEN_SPORT_FEEL;
               gui_sport_feel_init();
			   sport_end_flag = 1;
			   sport_end_hrvalue = get_10s_heartrate(); //获取前10s平均心率
				g_HeartNum = 0;
              // sport_feel_index = FeelAbitTired;
            }
	        else if (run_pause_index == RunPauseTrackback && (Get_IsTrainPlanOnGoing() == false))
            {//非训练计划下有循迹返航
							TRACE_BACK_DATA.is_traceback_flag = 1;
							TRACE_BACK_DATA.sport_distance = ActivityData.ActivityDetails.RunningDetail.Distance;
							TRACE_BACK_DATA.remaining_distance = ActivityData.ActivityDetails.RunningDetail.Distance;
							trace_back_timer_start();
							gui_sport_motor_prompt();
							ScreenState = DISPLAY_SCREEN_RUN_DATA_TRACKBACK;
							Set_PauseSta(false); 
                            //循迹返航轨迹标记
                            TrackBackFlag();
            }
            else if (run_pause_index == RunPauseCancel)
            {
                Pause_Save_Index = ScreenState;
                ScreenState = DISPLAY_SCREEN_SPORT_CANCEL;
				gui_sport_cancel_init();
            }
            gui_swich_msg();
			break;
		case KEY_UP:
				if(Get_IsTrainPlanOnGoing())
				{//若处于训练计划活动进行中时不显示循迹返航
					if (run_pause_index == RunPauseContinue)
					{
						run_pause_index = RunPauseCancel ;
					}
					else if(run_pause_index == RunPauseCancel)
					{
						run_pause_index = RunPauseSave;
					}
					else
					{
						run_pause_index--;
					}
				}
				else
				{
					if (run_pause_index == RunPauseContinue)
					{
						run_pause_index = RunPauseCancel ;

					}
					else
					{
						run_pause_index--;
					}
				}
        gui_swich_msg();
 
			break;
		case KEY_DOWN:
				if(Get_IsTrainPlanOnGoing())
				{//若处于训练计划活动进行中时不显示循迹返航
					if (run_pause_index == RunPauseCancel)
					{
						run_pause_index = RunPauseContinue ;
					}
					else if(run_pause_index == RunPauseSave)
					{
						run_pause_index = RunPauseCancel ;
					}
					else
					{
						run_pause_index++;
					}
				}
				else
				{
					if (run_pause_index == RunPauseCancel)
					{
						run_pause_index = RunPauseContinue ;
					}
					else
					{
						run_pause_index++;
					}
				}

         gui_swich_msg();

            break; 
		case KEY_BACK:
					ScreenState = DISPLAY_SCREEN_RUN_PAUSE_DATA;//add DISPLAY_SCREEN_RUN_PAUSE_DATA
					Sport_Index = DISPLAY_SCREEN_RUN_PAUSE_DATA;
					gui_swich_msg();
			break;
		default:
			break;
	}	
}

/* 
//运动放弃界面处理
void gui_sport_cancel_btn_evt(uint32_t Key_Value)
{
    // DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
            if (cancel_index== true)
            {
                ScreenState = DISPLAY_SCREEN_HOME;
                //结束运动不保存
								SportContinueLaterFlagClr(); //清除稍后继续状态
                Set_SaveSta(0);
                CloseSportTask();

            }
            else
            {
                ScreenState = Pause_Save_Index;
            }
	        gui_swich_msg();
			break;
        case KEY_UP:
		case KEY_DOWN:
             cancel_index=!cancel_index;
             gui_swich_msg();
			break; 
		case KEY_BACK:
             ScreenState = Pause_Save_Index;
             gui_swich_msg();
			break;
		default:
			break;
	}	
}

//运动感受界面处理
void gui_sport_feel_btn_evt(uint32_t Key_Value)
{
    // DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
             ScreenState = DISPLAY_SCREEN_SPORT_SAVING;
	         gui_swich_msg();
             //结束运动，保存数据
             SetSubjectiveEvaluate(sport_feel_index);
             Set_SaveSta(1);
             CloseSportTask();
						//训练计划结果保存
						Store_TranPlanUploadData();
						//训练计划结束
						Set_IsTrainPlanOnGoing(false);
            timer_progress_start(200);
			break;
		case KEY_DOWN:
           if (sport_feel_index == FeelTired)
           {
             sport_feel_index = FeelEasy ;

           }
         else
           {
                sport_feel_index++;
           }
             gui_swich_msg();
			break;
		case KEY_UP:
           if (sport_feel_index == FeelEasy)
           {
             sport_feel_index = FeelTired ;

           }
         else
           {
                sport_feel_index--;
           }
             gui_swich_msg();

            break; 
		case KEY_BACK:
            // ScreenState = Pause_Save_Index;
            // gui_swich_msg();
			break;
		default:
			break;
	}	
}
*/

//跑步暂停界面返回至跑步暂停轨迹界面按键处理
void gui_run_pause_track_btn_evt(uint32_t Key_Value)
{
   //DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
	         gui_sport_motor_prompt();
					 ScreenState = DISPLAY_SCREEN_RUN_TRACK;
					 Sport_Index = DISPLAY_SCREEN_RUN_TRACK;
             //run_pause_index = RunPauseContinue;
             Set_PauseSta(false);
             gui_swich_msg();
			break;
		case KEY_DOWN:
            gui_sport_index_down(DISPLAY_SCREEN_RUN_PAUSE_DATA,DISPLAY_SCREEN_RUN_PAUSE_TRACK);

			break;
		case KEY_UP:
            gui_sport_index_up(DISPLAY_SCREEN_RUN_PAUSE_DATA,DISPLAY_SCREEN_RUN_PAUSE_TRACK);

            break; 
		case KEY_BACK:
		ScreenState = DISPLAY_SCREEN_RUN_PAUSE;
		gui_swich_msg();
			break;
		default:
			break;
	}	
}

//跑步暂停界面返回至跑步暂停数据界面按键处理
void gui_run_pause_data_btn_evt(uint32_t Key_Value)
{
   //DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
	         gui_sport_motor_prompt(); 
					 ScreenState = DISPLAY_SCREEN_RUN_DATA;
					 Sport_Index = DISPLAY_SCREEN_RUN_DATA;
           //  run_pause_index = RunPauseContinue ;
             Set_PauseSta(false);
             gui_swich_msg();
			break;
		case KEY_DOWN:
            gui_sport_index_down(DISPLAY_SCREEN_RUN_PAUSE_DATA,DISPLAY_SCREEN_RUN_PAUSE_TRACK);

			break;
		case KEY_UP:
            gui_sport_index_up(DISPLAY_SCREEN_RUN_PAUSE_DATA,DISPLAY_SCREEN_RUN_PAUSE_TRACK);

		    break;
		case KEY_BACK:
		ScreenState = DISPLAY_SCREEN_RUN_PAUSE;
		gui_swich_msg();
				break;
		default:
			break;
	}	
}

//跑步运动详细_1界面按键处理
void gui_run_save_detail_1_btn_evt(uint32_t Key_Value)
{
    // DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_DOWN:
             ScreenState = DISPLAY_SCREEN_RUN_SAVE_DETIAL_2;
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

//跑步运动详细_2界面按键处理
void gui_run_save_detail_2_btn_evt(uint32_t Key_Value)
{
    // DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_DOWN:
             ScreenState = DISPLAY_SCREEN_RUN_SAVE_TRACK;
             gui_swich_msg();
             break;
        case KEY_UP:
             ScreenState = DISPLAY_SCREEN_RUN_SAVE_DETIAL_1;
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

//跑步运动详细轨迹界面按键处理
void gui_run_save_track_btn_evt(uint32_t Key_Value)
{
    // DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_DOWN:
             ScreenState = DISPLAY_SCREEN_RUN_SAVE_HEARTRATEZONE;
             gui_swich_msg();
			break;
		case KEY_UP:
             ScreenState = DISPLAY_SCREEN_RUN_SAVE_DETIAL_2;
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

//跑步运动详细心率区间界面按键处理
void gui_run_save_heartratezone_btn_evt(uint32_t Key_Value)
{
    // DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_UP:
             ScreenState = DISPLAY_SCREEN_RUN_SAVE_TRACK;
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






//循迹返航数据界面按键处理
void gui_run_data_trackback_btn_evt(uint32_t Key_Value)
{
   //DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:

             if (Get_PauseSta() == true)
             {
                 Set_PauseSta(false);
             }
             else
             {
                ScreenState = DISPLAY_SCREEN_RUN_TRACKBACK_PAUSE;
                run_pause_index = RunPauseContinue ;
                Set_PauseSta(true);
             }
             gui_swich_msg();
			break;

		case KEY_DOWN:
						Sport_Index = DISPLAY_SCREEN_RUN_DATA_TRACKBACK;
            gui_sport_index_down(DISPLAY_SCREEN_RUN_DATA_TRACKBACK,DISPLAY_SCREEN_RUN_TRACK_TRACKBACK);

			break;
		case KEY_UP:
						Sport_Index = DISPLAY_SCREEN_RUN_DATA_TRACKBACK;
            gui_sport_index_up(DISPLAY_SCREEN_RUN_DATA_TRACKBACK,DISPLAY_SCREEN_RUN_TRACK_TRACKBACK);

		    break;
		case KEY_BACK:
            if (Get_PauseSta() == true)
            { 
 		        ScreenState = DISPLAY_SCREEN_RUN_TRACKBACK_PAUSE;
		        gui_swich_msg();
            }
			break;
		default:
			break;
	}	
	
}


//循迹返航轨迹界面按键处理 
void gui_run_track_trackback_btn_evt(uint32_t Key_Value)
{
   //DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
            //循迹返航没有定义数据暂停索引，暂时通过状态来获取，与run_track处理方式不一样
             if (Get_PauseSta() == true)
             {
                 Set_PauseSta(false);
								
             }
             else
             {
							  trace_back_timer_stop();
                gui_sport_motor_prompt();   
								ScreenState = DISPLAY_SCREEN_RUN_TRACKBACK_PAUSE;
                 run_pause_index = RunPauseContinue;
                 Set_PauseSta(true);
             }
             gui_swich_msg();
			break;
		case KEY_DOWN:
						Sport_Index = DISPLAY_SCREEN_RUN_TRACK_TRACKBACK;
            gui_sport_index_down(DISPLAY_SCREEN_RUN_DATA_TRACKBACK,DISPLAY_SCREEN_RUN_TRACK_TRACKBACK);

			break;
		case KEY_UP:
						Sport_Index = DISPLAY_SCREEN_RUN_TRACK_TRACKBACK;
            gui_sport_index_up(DISPLAY_SCREEN_RUN_DATA_TRACKBACK,DISPLAY_SCREEN_RUN_TRACK_TRACKBACK);

            break; 
		case KEY_BACK:
            if (Get_PauseSta() == true)
            { 
 		        ScreenState = DISPLAY_SCREEN_RUN_TRACKBACK_PAUSE;
		        gui_swich_msg();
            }
			break;
		case KEY_LONG_UP:
			//保存航迹打点数据
			sport_dottrack();

			break;

		default:
			break;
	}	

}

//循迹返航暂停界面按键处理

void gui_run_trackback_pause_btn_evt(uint32_t Key_Value)
{
    //DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:  
            if (run_pause_index == RunPauseContinue)
            {
            #if 0
               if (Sport_Index == DISPLAY_SCREEN_RUN_PAUSE_DATA)
                {
                    Sport_Index = DISPLAY_SCREEN_RUN_DATA_TRACKBACK;

                }
                else if (Sport_Index ==DISPLAY_SCREEN_RUN_PAUSE_TRACK)
                {
                    Sport_Index = DISPLAY_SCREEN_RUN_DATA_TRACKBACK;
                }
              #endif
								trace_back_timer_restart();
                ScreenState = Sport_Index;
                Set_PauseSta(false);
								gui_sport_motor_prompt();  
            }
            else if (run_pause_index == RunPauseSave)
            {
							 trace_back_timer_delete();
               ScreenState = DISPLAY_SCREEN_SPORT_FEEL;
               gui_sport_feel_init();
			   sport_end_flag = 1;
			   sport_end_hrvalue = get_10s_heartrate(); //获取前10s平均心率
				g_HeartNum = 0;
             //  sport_feel_index = FeelAbitTired;
            }
	        /*else if (run_pause_index == RunPauseTraclback)
            {
							 ScreenState = DISPLAY_SCREEN_RUN_DATA_TRACLBACK;
            }*/
            else if (run_pause_index == RunPauseCancel)
            {
                Pause_Save_Index = ScreenState;
                ScreenState = DISPLAY_SCREEN_SPORT_CANCEL;
				gui_sport_cancel_init();
            }
            gui_swich_msg();
			break;
		case KEY_UP:
         if (run_pause_index == RunPauseContinue)
           {
             run_pause_index = RunPauseCancel ;

           }
         else
           {
                run_pause_index--;
								if(run_pause_index == RunPauseTrackback)
									run_pause_index--;
           }
           gui_swich_msg();
 
			break;
		case KEY_DOWN:
          if (run_pause_index == RunPauseCancel)
           {
             run_pause_index = RunPauseContinue ;

           }
         else
           {
                run_pause_index++;
								if(run_pause_index == RunPauseTrackback)
									run_pause_index++;
           }
          gui_swich_msg();

            break; 
		case KEY_BACK:
					ScreenState = DISPLAY_SCREEN_RUN_PAUSE_BACK_DATA_PAUSE;//add DISPLAY_SCREEN_RUN_PAUSE_DATA
					Sport_Index = DISPLAY_SCREEN_RUN_PAUSE_BACK_DATA_PAUSE;
					gui_swich_msg();
			break;
		default:
			break;
	}	

}




void gui_run_pause_back_data_trackback_pause_paint(void)
{
    SetWord_t word = {0};

    //设置背景黑色
	LCD_SetBackgroundColor(LCD_BLACK);
    
    gui_sport_index_circle_paint(0, 2, 1);
    
    gui_run_distance_paint(ActivityData.ActivityDetails.RunningDetail.Distance);

    
    word.x_axis = 80;
	LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,LCD_AUQAMARIN,0,0,LCD_FILL_ENABLE);

    gui_sport_gps_status(SPORT_GPS_LEFT1);

    gui_run_time_paint(97,30,LCD_WHITE);

    gui_run_heartrate_paint(Get_Sport_Heart());
    
    word.x_axis = 160;
	LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,LCD_AUQAMARIN,0,0,LCD_FILL_ENABLE);

    gui_run_speed_paint(ActivityData.ActivityDetails.RunningDetail.Pace);

    gui_sport_realtime_battery(0);

    LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_RED);
  
}

void gui_run_pause_back_data_trackback_pause_btn_env(uint32_t Key_Value)
{
	switch( Key_Value )
	{
		case KEY_OK:
		{
					 trace_back_timer_restart();
					 //振动提示
					 gui_sport_motor_prompt();
					 
	         ScreenState = DISPLAY_SCREEN_RUN_DATA_TRACKBACK;
					 Sport_Index = DISPLAY_SCREEN_RUN_DATA_TRACKBACK;
           //  run_pause_index = RunPauseContinue ;
					 Set_PauseSta(false);
             gui_swich_msg();
		}
			break;
		case KEY_DOWN:
            gui_sport_index_down(DISPLAY_SCREEN_RUN_PAUSE_BACK_DATA_PAUSE,DISPLAY_SCREEN_RUN_PAUSE_BACK_TRACK_PAUSE);

			break;
		case KEY_UP:
            gui_sport_index_up(DISPLAY_SCREEN_RUN_PAUSE_BACK_DATA_PAUSE,DISPLAY_SCREEN_RUN_PAUSE_BACK_TRACK_PAUSE);

		    break;
		case KEY_BACK:
					ScreenState = DISPLAY_SCREEN_RUN_TRACKBACK_PAUSE;
					Sport_Index = DISPLAY_SCREEN_RUN_TRACKBACK_PAUSE;
					gui_swich_msg();
			break;
		default:
			break;
	}

}

void gui_run_pause_back_track_trackback_pause_paint(uint32_t distance)
{
	//设置背景黑色
	LCD_SetBackgroundColor(LCD_BLACK);

	gui_sport_index_circle_paint(0, 2, 1);
	
	gui_sport_traceback_heartrate_paint(distance, Get_Sport_Heart());
	
	gui_sport_traceback_time();
	
	gui_sport_realtime_battery(1);
	
//	gui_sport_c_1_track(0);
	gui_sport_compass(0);
	
	gui_sport_gps_status(SPORT_GPS_LEFT2);
	
	LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_RED);

}


void gui_run_pause_back_track_trackback_pause_btn_env(uint32_t Key_Value)
{
	switch( Key_Value )
	{
		case KEY_OK:
		{
					 trace_back_timer_restart();
					 //振动提示
					 gui_sport_motor_prompt();
					 
	         ScreenState = DISPLAY_SCREEN_RUN_TRACK_TRACKBACK;
					 Sport_Index = DISPLAY_SCREEN_RUN_TRACK_TRACKBACK;
           //  run_pause_index = RunPauseContinue ;
					 Set_PauseSta(false);
             gui_swich_msg();
		}
			break;
		case KEY_DOWN:
            gui_sport_index_down(DISPLAY_SCREEN_RUN_PAUSE_BACK_DATA_PAUSE,DISPLAY_SCREEN_RUN_PAUSE_BACK_TRACK_PAUSE);

			break;
		case KEY_UP:
            gui_sport_index_up(DISPLAY_SCREEN_RUN_PAUSE_BACK_DATA_PAUSE,DISPLAY_SCREEN_RUN_PAUSE_BACK_TRACK_PAUSE);

		    break;
		case KEY_BACK:
					ScreenState = DISPLAY_SCREEN_RUN_TRACKBACK_PAUSE;
					Sport_Index = DISPLAY_SCREEN_RUN_TRACKBACK_PAUSE;
					gui_swich_msg();
			break;
		default:
			break;
	}

}
#endif


