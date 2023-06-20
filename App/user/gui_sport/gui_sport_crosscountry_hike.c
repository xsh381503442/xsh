#include "gui_sport_config.h"
#include "gui_sport_crosscountry_hike.h"
#include "gui_crosscountry.h"
#include "gui_motionsrecord.h"

#include "drv_lcd.h"
#include "drv_key.h"

#include "com_data.h"
#include "com_sport.h"

#include "gui_sport.h"
#include "gui_run.h"

#include "font_config.h"
#include "task_sport.h"
#include "task_gps.h"

#include "gui_tool_gps.h"
#include "time_notify.h"
#include "timer_traceback.h"
#include "algo_hdr.h"
#include "timer_app.h"
#include "drv_heartrate.h"
#if DEBUG_ENABLED == 1 && GUI_SPORT_CROSSCOUNTRY_HIKE_LOG_ENABLED == 1
	#define GUI_SPORT_CROSSCOUNTRY_HIKE_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_SPORT_CROSSCOUNTRY_HIKE_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define GUI_SPORT_CROSSCOUNTRY_HIKE_WRITESTRING(...)
	#define GUI_SPORT_CROSSCOUNTRY_HIKE_PRINTF(...)		        
#endif
uint8_t	vice_screen_walk_display[SPORT_DATA_DISPLAY_SUM] = {SPORT_DATA_DISPALY_TOTAL_DISTANCE,SPORT_DATA_DISPALY_AVE_PACE,SPORT_DATA_DISPALY_REALTIME_HEART,SPORT_DATA_DISPALY_TOTAL_TIME};

/*ͽ��ԽҰ������ʾ��ǰ��*/
 uint8_t m_hike_data_display_buf[SPORT_DATA_DISPLAY_SUM]={SPORT_DATA_DISPALY_TOTAL_DISTANCE,SPORT_DATA_DISPALY_REALTIME_PACE,SPORT_DATA_DISPALY_TOTAL_CLIMB,SPORT_DATA_DISPALY_TOTAL_TIME};
/*ͽ��ԽҰ������ʾѡ��*/
const uint8_t m_hike_data_display_option[HIKE_DATA_DISPLAY_SUM]= {SPORT_DATA_DISPALY_TOTAL_DISTANCE,SPORT_DATA_DISPALY_REALTIME_PACE,SPORT_DATA_DISPALY_TOTAL_CLIMB,SPORT_DATA_DISPALY_TOTAL_TIME,SPORT_DATA_DISPALY_AVE_PACE,
SPORT_DATA_DISPALY_TOTAL_STEP,SPORT_DATA_DISPALY_REALTIME_HEART,SPORT_DATA_DISPALY_REALTIME_FREQUENCY};

/*ͽ��ԽҰ��������ѡ��*/
const uint8_t m_hike_remind_set_option[HIKE_REMIND_SET_SUM]={SPORT_REMIND_SET_HAERT,SPORT_REMIND_SET_HRRECOVERY,SPORT_REMIND_SET_DISTANCE,SPORT_REMIND_SET_PACE,SPORT_REMIND_SET_TIME,SPORT_REMIND_SET_AUTOCIRCLE};

//extern _drv_bat_event g_bat_evt;

#define	GUI_SPORT_HIKE_SCREEN_DATA_1			0	//ͽ��ԽҰԤ������
#define	GUI_SPORT_HIKE_SCREEN_DATA_2			1	//ͽ��ԽҰ�������ݽ���
#define	GUI_SPORT_HIKE_SCREEN_DATA_3			2	//ͽ��ԽҰ�켣����
//#define	GUI_SPORT_HIKE_SCREEN_TIME		3	//��ɽʵʱʱ��
#define GUI_SPORT_HIKE_SCREEN_MAX		3

#define	GUI_SPORT_HIKE_PAUSE_CONTINUE	0	//����
#define	GUI_SPORT_HIKE_PAUSE_SAVE		1	//�����˶�
#define	GUI_SPORT_HIKE_PAUSE_WAIT		2	//�Ժ����
#define	GUI_SPORT_HIKE_PAUSE_TRACKBACK	3	//ѭ������
#define	GUI_SPORT_HIKE_PAUSE_CANCEL		4	//����

#define	GUI_SPORT_HIKE_PAUSE_CANCEL_NO	0	//��
#define	GUI_SPORT_HIKE_PAUSE_CANCEL_YES	1	//��


#define	GUI_SPORT_HIKE_DETAIL_1				0	//��ϸ���ݽ���1
#define	GUI_SPORT_HIKE_DETAIL_2				1	//��ϸ���ݽ���2
//#define	GUI_SPORT_HIKE_DETAIL_3				2	//��ϸ���ݽ���2
#define GUI_SPORT_HIKE_DETAIL_TRACK			2	//�켣����
//#define	GUI_SPORT_HIKE_DETAIL_HEARTRATEZONE	4	//�����������


static uint8_t m_gui_sport_hike_index;		//��ɽ��������
static uint8_t m_gui_sport_hike_pause_index;	//��ɽ��ͣ�˵���������

extern volatile ScreenState_t Save_Detail_Index;
extern uint8_t gui_sport_index;

extern uint8_t gui_sport_type;


//��ok���˶���ʼ��ʾ
void gui_hike_prompt_paint(void)
{
  SetWord_t word = {0};
 // uint8_t x1,y1,x2,y2,x3,y3;

   word.x_axis = 20;
   word.y_axis = 120;
   
   LCD_SetPicture(word.x_axis,word.y_axis,LCD_NONE,LCD_NONE,&Img_start_18X24);

   word.x_axis = 55;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	
	LCD_SetString((char *)SportsPromptStrs[SetValue.Language][0],&word);

}


//ͽ��ԽҰ���ݽ���
void gui_sport_hike_data_paint(void)
{

	SetWord_t word = {0};
	char str[10];
	uint8_t len;
	uint32_t time = ActivityData.ActTime;
	//���ñ�����ɫ
	LCD_SetBackgroundColor(LCD_BLACK);
	//Բ������
	gui_sport_index_circle_paint(1, 4, 1);

	//���
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 1;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.x_axis = 54;
	word.size = LCD_FONT_48_SIZE;
	#if defined (SPRINTF_FLOAT_TO_INT)
	sprintf(str,"%d.%02d",ActivityData.ActivityDetails.CrosscountryDetail.Distance/100000,ActivityData.ActivityDetails.CrosscountryDetail.Distance/1000%100);
	#else
	sprintf(str,"%0.2f",ActivityData.ActivityDetails.CrosscountryDetail.Distance/100000.f);
	#endif
	len = strlen(str);
	LCD_SetNumber(str,&word);
	
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 0;
	word.x_axis +=40;
	word.y_axis =120+(len*26)/2 + 5;
	LCD_SetString("km",&word);
	//gps
	gui_sport_gps_status(SPORT_GPS_LEFT3);
	
	//�ָ���
	word.x_axis = 120;
	LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,LCD_AUQAMARIN,0,0,LCD_FILL_ENABLE);
	
	//ʱ��
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 1;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.x_axis = 130;
	word.size = LCD_FONT_48_SIZE;

	memset(str,0,10);
	if(time >= 3600)
	{
					//ʱ����
		sprintf(str,"%d:%02d:%02d",time/3600, time/60%60, time%60);
	}
	else
	{
					//����
		sprintf(str,"%d:%02d",time/60%60, time%60);
	}		
	LCD_SetNumber(str,&word);


}




//ͽ��ԽҰ���ʽ���
void gui_sport_hike_heart_paint(void)
{
		//���ñ�����ɫ
		LCD_SetBackgroundColor(LCD_BLACK);
		//Բ������
		gui_sport_index_circle_vert_paint(1, 3);
		
		//����ֵ
		gui_sport_heart_1_paint(Get_Sport_Heart(),1);
		
}



//ͽ��ԽҰ�켣����
void gui_hike_track_update_paint(void)
{
        DrawLoadTrack();
        DrawTrack();

		//���������ָ���л���ʾ
		if (dottrack_draw_time > 0)
		{
		  gui_sport_dottrack_nums(dot_track_nums); //���������
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

//ʣ�����
static void gui_sport_hike_1_distance(uint8_t type, uint32_t distance)
{
	gui_sport_every_1_distance(type,distance);
}

static void gui_sport_hike_1_time(void)
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


//ͽ��ԽҰ�켣����
void gui_sport_hike_track_paint(void)
{
		static uint8_t last_status = 2;
	
		//���ñ�����ɫ
		LCD_SetBackgroundColor(LCD_WHITE);
		 //�켣
		gui_hike_track_update_paint();

		if(g_sport_status == SPORT_STATUS_READY)
		{
			gui_hike_prompt_paint();
		}
		else
		{
			
			gui_sport_index_circle_vert_paint(2, 3);
			
		}
		
		if (dottrack_out_time != 0 || dottrack_nogps_time != 0)
    	{
			gui_sport_dot_draw_prompt();
    	}
		
		if((TRACE_BACK_DATA.is_traceback_flag||(loadflag ==1)) && (Get_PauseSta() == false)&& (GetGpsStatus()== true))
		{
			//�˶�״̬����ѭ������״̬�л򵼺�
			if((last_status == 0) && (TRACE_BACK_DATA.whether_in_track))
			{
				//���ڹ켣��
				timer_notify_motor_start(300,300,1,false,NOTIFY_MODE_SPORTS);
				timer_notify_buzzer_start(300,300,1,false,NOTIFY_MODE_SPORTS);
				
				gui_sport_hike_1_distance(2, 0);
			}
			else if((last_status == 1) && (TRACE_BACK_DATA.whether_in_track == 0))
			{
				//δ�ڹ켣��
				timer_notify_motor_start(300,300,1,false,NOTIFY_MODE_SPORTS);
				timer_notify_buzzer_start(300,300,1,false,NOTIFY_MODE_SPORTS);
				
				gui_sport_hike_1_distance(3, 0);
			}
			else
			{
				if(TRACE_BACK_DATA.sport_traceback_flag)
				{
					//��ʱ
					gui_sport_hike_1_time();
				}
				else
				{
					if(TRACE_BACK_DATA.whether_in_track)
					{
						//ʣ�����
						gui_sport_hike_1_distance(0, TRACE_BACK_DATA.remaining_distance);
					}
					else
					{
						//����켣
						gui_sport_hike_1_distance(1, TRACE_BACK_DATA.nearest_distance);
					}
				}
			}
			
			last_status = TRACE_BACK_DATA.whether_in_track;
		}
		else
		{
			//��ʱ
			gui_sport_hike_1_time();
			
			last_status = 2;
		}
		
	//	gui_crosscountry_realtime_battery_paint(205, g_bat_evt.level_percent);
		

		
		
		
		gui_sport_compass(0);
	
  	if((g_sport_status == SPORT_STATUS_PAUSE)&&(ScreenState!=DISPLAY_SCREEN_REMIND_PAUSE))
			{
				//��ʾ��ͣ״̬
		     LCD_SetPicture(110,200,LCD_MAROON,LCD_NONE,&img_sport_pause_cod);//���ϽǼ�ͷ,ָʾok��
		      
			 // gui_sport_gps_status(SPORT_GPS_RIGHT_UP_6);
				
			}
		else
			{
	        gui_sport_gps_status(SPORT_GPS_RIGHT_UP_6);
			}
	//	gui_crosscountry_gps_paint();
		
	
	
				
}

//ͽ��ԽҰʵʱʱ�����
void gui_sport_hike_realtime_paint(void)
{
	//���ñ�����ɫ
	LCD_SetBackgroundColor(LCD_BLACK);
	
	//Բ������
    gui_sport_index_circle_paint(3, 4, 1);
	
	//ʵʱʱ��
	//gui_sport_realtime_1_paint(1);
		
}
//ͽ��ԽҰû��������ʾ������ �켣
void gui_sport_event_hike_three_track_paint(void)
{
		static uint8_t last_status = 2;
	
		//���ñ�����ɫ
		LCD_SetBackgroundColor(LCD_BLACK);
		 //�켣
		gui_hike_track_update_paint();
#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
	if(get_share_track_back_discreen() != GOTO_SHARE_TRACK_BY_GPS)
	{//�Ƽ�����
		gui_sport_index_circle_paint(2, 4, 1);
	}
#else
		gui_sport_index_circle_paint(2, 4, 1);
#endif
		if (dottrack_out_time != 0 || dottrack_nogps_time != 0)
    	{
			gui_sport_dot_draw_prompt();
    	}
		
		if((TRACE_BACK_DATA.is_traceback_flag||(loadflag ==1)) && (Get_PauseSta() == false)&& (GetGpsStatus()== true))
		{
			//�˶�״̬����ѭ������״̬�л򵼺�
			if((last_status == 0) && (TRACE_BACK_DATA.whether_in_track))
			{
				//���ڹ켣��
				timer_notify_motor_start(300,300,1,false,NOTIFY_MODE_SPORTS);
				timer_notify_buzzer_start(300,300,1,false,NOTIFY_MODE_SPORTS);
				
				gui_sport_hike_1_distance(2, 0);
			}
			else if((last_status == 1) && (TRACE_BACK_DATA.whether_in_track == 0))
			{
				//δ�ڹ켣��
				timer_notify_motor_start(300,300,1,false,NOTIFY_MODE_SPORTS);
				timer_notify_buzzer_start(300,300,1,false,NOTIFY_MODE_SPORTS);
				
				gui_sport_hike_1_distance(3, 0);
			}
			else
			{
				if(TRACE_BACK_DATA.sport_traceback_flag)
				{
					//��ʱ
					gui_sport_hike_1_time();
				}
				else
				{
					if(TRACE_BACK_DATA.whether_in_track)
					{
						//ʣ�����
						gui_sport_hike_1_distance(0, TRACE_BACK_DATA.remaining_distance);
					}
					else
					{
						//����켣
						gui_sport_hike_1_distance(1, TRACE_BACK_DATA.nearest_distance);
					}
				}
			}
			
			last_status = TRACE_BACK_DATA.whether_in_track;
		}
		else
		{
			//��ʱ
			gui_sport_hike_1_time();
			
			last_status = 2;
		}
		
	//	gui_crosscountry_realtime_battery_paint(205, g_bat_evt.level_percent);
		
		gui_sport_compass(0);
	
		gui_sport_gps_status(SPORT_GPS_LEFT2);		//ʹ��ͨ��GPS����
	//	gui_crosscountry_gps_paint();
		
#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
	if(get_share_track_back_discreen() == GOTO_SHARE_TRACK_BY_GPS)
	{//�Ƽ�����
			if(g_sport_status == SPORT_STATUS_READY)
			{
				gui_crosscountry_prompt_paint();
			}
			else
			{
				gui_sport_index_circle_paint(0, 4, 1);
			}
	}
#endif
	
				
}

//ͽ��ԽҰû��������ʾ������ ʵʱ���ٵ�
void gui_sport_event_hike_three_pace_paint(void)
{
	SetWord_t word = {0};
	//���ñ�����ɫ
	LCD_SetBackgroundColor(LCD_BLACK);
#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
	if(get_share_track_back_discreen() != GOTO_SHARE_TRACK_BY_GPS)
	{//�����Ƽ�����
		//�ָ���
		word.x_axis = 80;
		LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,LCD_AUQAMARIN,0,0,LCD_FILL_ENABLE);
		
		if(g_sport_status == SPORT_STATUS_READY)
		{
			gui_crosscountry_prompt_paint();
			if(get_is_sport_event_cloud_valid())
			{//���Ƽ�����
				gui_crosscountry_cloud_navigation_paint();
			}
		}
		else
		{
			//Բ������
			gui_sport_index_circle_paint(0, 4, 1);

			gui_sport_data_display_total_distance_paint(SPORT_DATA_DISPALY_INDEX_UP_CIRCLE,ActivityData.ActivityDetails.RunningDetail.Distance);
			
		}
	}
	else
	{
		//�ָ���
		word.x_axis = 80;
		LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,LCD_AUQAMARIN,0,0,LCD_FILL_ENABLE);
		//Բ������
		gui_sport_index_circle_paint(2, 4, 1);

		gui_sport_data_display_total_distance_paint(SPORT_DATA_DISPALY_INDEX_UP_CIRCLE,ActivityData.ActivityDetails.RunningDetail.Distance);
			
		
	}
#else
	if(g_sport_status == SPORT_STATUS_READY)
	{
		gui_crosscountry_prompt_paint();
	}
	else
	{
		//Բ������
   	gui_sport_index_circle_paint(m_gui_sport_hike_index, 4, 1);

		gui_sport_data_display_total_distance_paint(SPORT_DATA_DISPALY_INDEX_UP_CIRCLE,ActivityData.ActivityDetails.RunningDetail.Distance);
		
	}

	
	//�ָ���
	word.x_axis = 80;
	LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,LCD_AUQAMARIN,0,0,LCD_FILL_ENABLE);
#endif
	
	//GPS״̬,��ֹ�ڵ������ÿ���λ��
	gui_sport_gps_status(SPORT_GPS_LEFT1);

	gui_sport_data_display_realtime_pace_paint(SPORT_DATA_DISPALY_INDEX_MID_GPS,ActivityData.ActivityDetails.RunningDetail.Pace);

	//�ָ���
	word.x_axis = 160;	
	LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,LCD_AUQAMARIN,0,0,LCD_FILL_ENABLE);

	gui_sport_data_display_total_time_paint(SPORT_DATA_DISPALY_INDEX_DOWN_NOHINT,ActivityData.ActTime);

}
//ͽ��ԽҰû��������ʾ������ �ۼ�������
/*void gui_sport_event_hike_three_heart_paint(void)
{
	SetWord_t word = {0};
	char str[10];
	//uint8_t len;
	//���ñ�����ɫ
	LCD_SetBackgroundColor(LCD_BLACK);

	//Բ������
	gui_sport_index_circle_paint(m_gui_sport_hike_index, 4, 1);

	//�����߶�ͼ��
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
	
	//�ָ���
	word.x_axis = 80;
	LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,LCD_AUQAMARIN,0,0,LCD_FILL_ENABLE);

	
	//GPS״̬,��ֹ�ڵ������ÿ���λ��
	gui_sport_gps_status(SPORT_GPS_LEFT1);
	
	//����
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 1;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.x_axis = 92;
	word.size = LCD_FONT_48_SIZE;
	memset(str,0,10);

	sprintf(str,"%d",(int32_t)drv_getHdrValue());
	//len = strlen(str);
	LCD_SetNumber(str,&word);

	word.size = LCD_FONT_16_SIZE;
	word.kerning = 0;
	
	word.x_axis += 20;
	word.y_axis = 190;
	LCD_SetPicture(word.x_axis,word.y_axis,LCD_NONE,LCD_NONE,&Img_heartrate_32X28);
	

	//�ָ���
	word.x_axis = 160;	
	LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,LCD_AUQAMARIN,0,0,LCD_FILL_ENABLE);

	//�½��߶�ͼ��
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
//ͽ��ԽҰû��������ʾ�Ĺ��� ����������ʱ���
void gui_sport_event_hike_four_data_paint(void)
{
	SetWord_t word = {0};
	uint32_t second = ActivityData.ActTime;
	char p_str[20];
	
	LCD_SetBackgroundColor(LCD_BLACK);

	LCD_SetRectangle(80, 1, 0, LCD_LINE_CNT_MAX, LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);
	
	LCD_SetRectangle(160, 1, 0, LCD_LINE_CNT_MAX, LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);
	
	LCD_SetRectangle(80, 80, 120, 1, LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);
	
	//GPS״̬,��ֹ�ڵ������ÿ���λ��
	gui_sport_gps_status(SPORT_GPS_LEFT1);
	
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
	if(get_share_track_back_discreen() != GOTO_SHARE_TRACK_BY_GPS)
	{//�Ƽ�����
		if(g_sport_status == SPORT_STATUS_READY)
		{
			gui_hike_prompt_paint();
	#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
			if(get_is_sport_event_cloud_valid())
			{//���Ƽ�����
				gui_crosscountry_cloud_navigation_paint();
			}
	#endif
		}
		else
		{
			gui_sport_index_circle_paint(0, 4, 1);
			//ʱ��
			word.forecolor = LCD_WHITE;
			word.bckgrndcolor = LCD_NONE;
			word.x_axis = 28;
			word.y_axis = LCD_CENTER_JUSTIFIED;
			word.size = LCD_FONT_16_SIZE;
			word.kerning = 0;
			LCD_SetString((char *)GuiSportEventStrs[SetValue.Language][11], &word);
			word.kerning = 1;
			memset(p_str,0,sizeof(p_str));
			if(second >= 3600)
			{
				//ʱ����
				sprintf(p_str,"%d:%02d:%02d",second/3600,second/60%60,second%60);
			}
			else
			{
				//����
				sprintf(p_str,"%d:%02d",second/60%60,second%60);
			}
			word.x_axis += 20;
			word.size = LCD_FONT_24_SIZE;
			LCD_SetNumber(p_str,&word);
		}
	}
	else
	{
		gui_sport_index_circle_paint(2, 4, 1);
		//ʱ��
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_NONE;
		word.x_axis = 28;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_16_SIZE;
		word.kerning = 0;
		LCD_SetString((char *)GuiSportEventStrs[SetValue.Language][11], &word);
		word.kerning = 1;
		memset(p_str,0,sizeof(p_str));
		if(second >= 3600)
		{
			//ʱ����
			sprintf(p_str,"%d:%02d:%02d",second/3600,second/60%60,second%60);
		}
		else
		{
			//����
			sprintf(p_str,"%d:%02d",second/60%60,second%60);
		}
		word.x_axis += 20;
		word.size = LCD_FONT_24_SIZE;
		LCD_SetNumber(p_str,&word);
		
	}
#else
	if(g_sport_status == SPORT_STATUS_READY)
	{
		gui_hike_prompt_paint();
#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
		if(get_is_sport_event_cloud_valid())
		{//���Ƽ�����
			gui_crosscountry_cloud_navigation_paint();
		}
#endif
	}
	else
	{
		gui_sport_index_circle_paint(0, 4, 1);
		//ʱ��
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_NONE;
		word.x_axis = 28;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_16_SIZE;
		word.kerning = 0;
		LCD_SetString((char *)GuiSportEventStrs[SetValue.Language][11], &word);
		word.kerning = 1;
		memset(p_str,0,sizeof(p_str));
		if(second >= 3600)
		{
			//ʱ����
			sprintf(p_str,"%d:%02d:%02d",second/3600,second/60%60,second%60);
		}
		else
		{
			//����
			sprintf(p_str,"%d:%02d",second/60%60,second%60);
		}
		word.x_axis += 20;
		word.size = LCD_FONT_24_SIZE;
		LCD_SetNumber(p_str,&word);
	}
#endif


	//���
	word.x_axis = 90;
	word.y_axis = (LCD_LINE_CNT_MAX/2 - strlen((char *)GuiSportEventStrs[SetValue.Language][12]) * 8)/2;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString((char *)GuiSportEventStrs[SetValue.Language][12], &word);
	
	memset(p_str,0,sizeof(p_str));
	sprintf(p_str,"%0.2f",ActivityData.ActivityDetails.RunningDetail.Distance/100000.f);
	word.x_axis += 24;
	word.y_axis = (LCD_LINE_CNT_MAX/2 - strlen(p_str) * 8)/2;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.size = LCD_FONT_24_SIZE;
	word.kerning = 1;	
	LCD_SetNumber(p_str,&word);	
	
	//�����
	word.x_axis = 90;
	word.y_axis = LCD_LINE_CNT_MAX/2 + (LCD_LINE_CNT_MAX/2 - strlen((char *)GuiSportEventStrs[SetValue.Language][13]) * 8)/2;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString((char *)GuiSportEventStrs[SetValue.Language][13], &word);
	
	memset(p_str,0,sizeof(p_str));
#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
	sprintf(p_str,"%0.2f",(g_sport_event_schedule_detail_cal.total_distance + ActivityData.ActivityDetails.RunningDetail.Distance)/100000.f);
#else
	sprintf(p_str,"%0.2f",(ActivityData.ActivityDetails.RunningDetail.Distance)/100000.f);
#endif
	word.x_axis += 24;
	word.y_axis = LCD_LINE_CNT_MAX/2 + (LCD_LINE_CNT_MAX/2 - strlen(p_str) * 8)/2;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.size = LCD_FONT_24_SIZE;
	word.kerning = 1;	
	LCD_SetNumber(p_str,&word);	

	//��ʱ��
	word.x_axis = 170;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString((char *)GuiSportEventStrs[SetValue.Language][14], &word);
	
	word.x_axis += 24;
	memset(p_str,0,sizeof(p_str));
#if defined WATCH_SPORT_EVENT_SCHEDULE
	second += g_sport_event_schedule_detail_cal.total_time;
#endif
	if(second >= 3600)
	{
		//ʱ����
		sprintf(p_str,"%d:%02d:%02d",second/3600,second/60%60,second%60);
	}
	else
	{
		//����
		sprintf(p_str,"%d:%02d",second/60%60,second%60);
	}
	word.size = LCD_FONT_24_SIZE;
	LCD_SetNumber(p_str,&word);
}
//ͽ��ԽҰû��������ʾ�Ĺ��� ʵʱ�����ۼ������ۼ��½���
/*void gui_sport_event_hike_four_heart_paint(void)
{
	SetWord_t word = {0};
	uint32_t second = ActivityData.ActTime;
	char p_str[20];
	
	LCD_SetBackgroundColor(LCD_BLACK);

	LCD_SetRectangle(80, 1, 0, LCD_LINE_CNT_MAX, LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);
	
	LCD_SetRectangle(160, 1, 0, LCD_LINE_CNT_MAX, LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);
	
	LCD_SetRectangle(80, 80, 120, 1, LCD_AUQAMARIN, 0, 0, LCD_FILL_ENABLE);
	
	//GPS״̬,��ֹ�ڵ������ÿ���λ��
	gui_sport_gps_status(SPORT_GPS_LEFT1);
	
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	gui_sport_index_circle_paint(1, 4, 1);
	//ʵʱ����
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.x_axis = 28;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 0;
	LCD_SetString((char *)GuiSportEventStrs[SetValue.Language][15], &word);
	word.kerning = 1;
	memset(p_str,0,sizeof(p_str));
	sprintf(p_str,"%d:%02d",(ActivityData.ActivityDetails.RunningDetail.Pace/60),(ActivityData.ActivityDetails.RunningDetail.Pace%60));
	word.x_axis += 20;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetNumber(p_str,&word);
	

	//�ۼ�����
	word.x_axis = 90;
	word.y_axis = (LCD_LINE_CNT_MAX/2 - strlen((char *)GuiSportEventStrs[SetValue.Language][16]) * 8)/2;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString((char *)GuiSportEventStrs[SetValue.Language][16], &word);
	
	memset(p_str,0,sizeof(p_str));
	sprintf(p_str,"%d",ActivityData.ActivityDetails.ClimbingDetail.RiseHeight);
	word.x_axis += 24;
	word.y_axis = (LCD_LINE_CNT_MAX/2 - strlen(p_str) * 8)/2;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.size = LCD_FONT_24_SIZE;
	word.kerning = 1;	
	LCD_SetNumber(p_str,&word);	
	
	//�ۼ��½�
	word.x_axis = 90;
	word.y_axis = LCD_LINE_CNT_MAX/2 + (LCD_LINE_CNT_MAX/2 - strlen((char *)GuiSportEventStrs[SetValue.Language][17]) * 8)/2;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString((char *)GuiSportEventStrs[SetValue.Language][17], &word);
	
	memset(p_str,0,sizeof(p_str));
	sprintf(p_str,"%d",ActivityData.ActivityDetails.ClimbingDetail.DropHeight);
	word.x_axis += 24;
	word.y_axis = LCD_LINE_CNT_MAX/2 + (LCD_LINE_CNT_MAX/2 - strlen(p_str) * 8)/2;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.size = LCD_FONT_24_SIZE;
	word.kerning = 1;	
	LCD_SetNumber(p_str,&word);	

	//����
	word.x_axis = 170;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString((char *)GuiSportEventStrs[SetValue.Language][18], &word);
	
	word.x_axis += 24;
	memset(p_str,0,sizeof(p_str));
	sprintf(p_str,"%d",drv_getHdrValue());
	word.size = LCD_FONT_24_SIZE;
	LCD_SetNumber(p_str,&word);
	word.x_axis = 190;
	word.y_axis = 50;
	LCD_SetPicture(word.x_axis,word.y_axis,LCD_NONE,LCD_NONE,&Img_heartrate_18X16);

}*/
//ͽ��ԽҰ����
void gui_sport_hike_paint(void)
{
    
	gui_sport_type = 3;
	if(m_gui_sport_hike_index ==GUI_SPORT_HIKE_SCREEN_DATA_1 )
	{
	 	//gui_sport_hike_data_paint();
	 	
		gui_sport_data_display_preview_paint(SPORT_DATA_SHOW);
	}
	else if (m_gui_sport_hike_index ==GUI_SPORT_HIKE_SCREEN_DATA_2)
	{
		//gui_sport_hike_heart_paint();
		
		gui_sport_heart_paint(1);
	}
	/*else if (m_gui_sport_hike_index ==GUI_SPORT_HIKE_SCREEN_TIME)
	{
		gui_sport_hike_realtime_paint();
	}*/
	else
	{
		gui_sport_hike_track_paint();
	}
	
	if(g_sport_status == SPORT_STATUS_PAUSE)
	{
		//��ʱ���Ӻ�ɫԲȦ����ʾ��ͣ״̬
		//LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_RED);
	}

}
//ͽ��ԽҰ���½��� 3����
/*void gui_sport_event_hike_three_grid_paint(void)
{
#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
	if(get_share_track_back_discreen() == GOTO_SHARE_TRACK_BY_GPS)
	{//�Ƽ�����
		if(m_gui_sport_hike_index == GUI_SPORT_HIKE_SCREEN_DATA )
		{//��ʾ�ۼ�����
			gui_sport_event_hike_three_heart_paint();
		}
		else if (m_gui_sport_hike_index == GUI_SPORT_HIKE_SCREEN_HEART)
		{
			gui_sport_event_hike_three_pace_paint();
		}
		else if (m_gui_sport_hike_index == GUI_SPORT_HIKE_SCREEN_TIME)
		{
			gui_sport_hike_realtime_paint();
		}
		else
		{
			gui_sport_event_hike_three_track_paint();
		}
		
		if(g_sport_status == SPORT_STATUS_PAUSE)
		{
			//��ʱ���Ӻ�ɫԲȦ����ʾ��ͣ״̬
			LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_RED);
		}
	}
	else
	{
		if(m_gui_sport_hike_index ==GUI_SPORT_HIKE_SCREEN_DATA )
		{//��ʾ�ۼ�����
			gui_sport_event_hike_three_heart_paint();
		}
		else if (m_gui_sport_hike_index ==GUI_SPORT_HIKE_SCREEN_HEART)
		{
			gui_sport_event_hike_three_track_paint();
		}
		else if (m_gui_sport_hike_index ==GUI_SPORT_HIKE_SCREEN_TIME)
		{
			gui_sport_hike_realtime_paint();
		}
		else
		{
			gui_sport_event_hike_three_pace_paint();
		}
		
		if(g_sport_status == SPORT_STATUS_PAUSE)
		{
			//��ʱ���Ӻ�ɫԲȦ����ʾ��ͣ״̬
			LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_RED);
		}
	}
#else
	if(m_gui_sport_hike_index ==GUI_SPORT_HIKE_SCREEN_DATA )
	{//��ʾ�ۼ�����
	 	gui_sport_event_hike_three_heart_paint();
	}
	else if (m_gui_sport_hike_index ==GUI_SPORT_HIKE_SCREEN_HEART)
	{
		gui_sport_event_hike_three_track_paint();
	}
	else if (m_gui_sport_hike_index ==GUI_SPORT_HIKE_SCREEN_TIME)
	{
		gui_sport_hike_realtime_paint();
	}
	else
	{
		gui_sport_event_hike_three_pace_paint();
	}
	
	if(g_sport_status == SPORT_STATUS_PAUSE)
	{
		//��ʱ���Ӻ�ɫԲȦ����ʾ��ͣ״̬
		LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_RED);
	}
#endif
}*/
//ͽ��ԽҰ���½��� 4����
/*void gui_sport_event_hike_four_grid_paint(void)
{
#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
	if(get_share_track_back_discreen() == GOTO_SHARE_TRACK_BY_GPS)
	{//�Ƽ�����
		if(m_gui_sport_hike_index == GUI_SPORT_HIKE_SCREEN_DATA )
		{//2
			gui_sport_event_hike_four_heart_paint();
		}
		else if (m_gui_sport_hike_index ==GUI_SPORT_HIKE_SCREEN_HEART)
		{//3
			gui_sport_event_hike_four_data_paint();
		}
		else if (m_gui_sport_hike_index ==GUI_SPORT_HIKE_SCREEN_TIME)
		{//4
			gui_sport_hike_realtime_paint();
		}
		else
		{//1
			gui_sport_event_hike_three_track_paint();
		}
		
		if(g_sport_status == SPORT_STATUS_PAUSE)
		{
			//��ʱ���Ӻ�ɫԲȦ����ʾ��ͣ״̬
			LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_RED);
		}
	}
	else
	{
		if(m_gui_sport_hike_index == GUI_SPORT_HIKE_SCREEN_DATA )
		{//2
			gui_sport_event_hike_four_heart_paint();
		}
		else if (m_gui_sport_hike_index ==GUI_SPORT_HIKE_SCREEN_HEART)
		{//3
			gui_sport_event_hike_three_track_paint();
		}
		else if (m_gui_sport_hike_index ==GUI_SPORT_HIKE_SCREEN_TIME)
		{//4
			gui_sport_hike_realtime_paint();
		}
		else
		{//1
			gui_sport_event_hike_four_data_paint();
		}
		
		if(g_sport_status == SPORT_STATUS_PAUSE)
		{
			//��ʱ���Ӻ�ɫԲȦ����ʾ��ͣ״̬
			LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_RED);
		}
	}
#else
	if(m_gui_sport_hike_index == GUI_SPORT_HIKE_SCREEN_DATA )
	{//2
	 	gui_sport_event_hike_four_heart_paint();
	}
	else if (m_gui_sport_hike_index ==GUI_SPORT_HIKE_SCREEN_HEART)
	{//3
		gui_sport_event_hike_three_track_paint();
	}
	else if (m_gui_sport_hike_index ==GUI_SPORT_HIKE_SCREEN_TIME)
	{//4
		gui_sport_hike_realtime_paint();
	}
	else
	{//1
		gui_sport_event_hike_four_data_paint();
	}
	
	if(g_sport_status == SPORT_STATUS_PAUSE)
	{
		//��ʱ���Ӻ�ɫԲȦ����ʾ��ͣ״̬
		LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_RED);
	}
#endif
}*/


//��ͣ�˵�ѡ��
static void gui_sport_hike_pause_options(uint8_t index)
{
		SetWord_t word = {0};
	
	//�˵���һ��
	word.x_axis = 120 + 18;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;
	word.size = LCD_FONT_24_SIZE;
	
	LCD_SetString((char *)ClimbPauseStrs[SetValue.Language][index],&word); //�����Ժ������
	
		//�˵��ڶ���
	if(index >= GUI_SPORT_HIKE_PAUSE_CANCEL)
	{
		index = GUI_SPORT_HIKE_PAUSE_CONTINUE;
	}
	else
	{

		index += 1;
		if((TRACE_BACK_DATA.is_traceback_flag) && (index == GUI_SPORT_HIKE_PAUSE_TRACKBACK))
		{
				//�������ѭ���������̣�����ѭ������ѡ��
				index = GUI_SPORT_HIKE_PAUSE_CANCEL;
		}
	}

	word.x_axis = 180 + 18;
	word.forecolor = LCD_BLACK;
	word.size = LCD_FONT_16_SIZE;

	LCD_SetString((char *)ClimbPauseStrs[SetValue.Language][index],&word); //�����Ժ������
}


//ͽ��ԽҰ��ͣ�˵�����
void gui_sport_hike_pause_paint(void)
{

		LCD_SetBackgroundColor(LCD_BLACK);
		
		//��ͣ��ʱ
		gui_sport_pause_time();
		
		//�˶���ʱ
		gui_sport_pause_acttime();
		
		LCD_SetRectangle(120 - 1 ,122,0,240,LCD_WHITE,0,0,LCD_FILL_ENABLE);
		LCD_SetRectangle(120 + 60 - 1,2,0,240,LCD_BLACK,0,0,LCD_FILL_ENABLE);
	
		//ѡ��
		gui_sport_hike_pause_options(m_gui_sport_hike_pause_index);
		
		
		//�·���ʾ
		gui_sport_page_prompt(SPORT_PAGE_DOWN);
	
		LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_RED);

	
}

//ͽ��ԽҰ�����˵�����
void gui_sport_hike_cancel_paint(void)
{
	SetWord_t word = {0};
		
	//�ϰ벿��Ϊ��ɫ����
	LCD_SetRectangle(0, 120, 0, LCD_PIXEL_ONELINE_MAX, LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
		
	//����?
	word.x_axis = 50;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 0;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString((char *)RunPauseStrs[SetValue.Language][3],&word);
	word.y_axis = 112 + 32 + 2;
	LCD_SetString("?",&word);
		
	//�°벿��Ϊ��ɫ����
	LCD_SetRectangle(120, 120, 0, LCD_PIXEL_ONELINE_MAX, LCD_WHITE, 0, 0, LCD_FILL_ENABLE);
		
		//ѡ��
	gui_sport_cancel_options(m_gui_sport_hike_pause_index);
	

	
		
	//�ָ���
	LCD_SetRectangle(170, 1, 0, LCD_LINE_CNT_MAX, LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
		
	//�·���ʾ
	gui_sport_page_prompt(SPORT_PAGE_DOWN);	


	
}
//��ϸ���ݽ���1
static void gui_sport_crosscountry_hike_detail_1_paint(uint8_t backgroundcolor)
	{
	   SetWord_t word = {0};
	   char str[20];
	   char *colon = ":";
	   uint8_t charlen;
	   
	   uint32_t second;
	
		//����ʱ��
		second = ActivityData.ActTime;//get_time_diff(ActivityData.Act_Start_Time, ActivityData.Act_Stop_Time) - Get_PauseTotolTime();
	
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
		word.x_axis = 16;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_24_SIZE;
		word.bckgrndcolor = LCD_NONE;
		LCD_SetString("ͽ��ԽҰ", &word);
	
	
		
	   //����ʱ��
		memset(str,0,20);
		sprintf(str,"%02d-%02d-%02d %02d:%02d",ActivityData.Act_Start_Time.Year+2000,ActivityData.Act_Start_Time.Month,ActivityData.Act_Start_Time.Day,ActivityData.Act_Start_Time.Hour,ActivityData.Act_Start_Time.Minute);
		word.x_axis = 40+8;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_13_SIZE;
		word.bckgrndcolor = LCD_NONE;
		word.kerning = 0;
		LCD_SetNumber(str,&word);
	
	
		
	
		//����
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
	//#if defined (SPRINTF_FLOAT_TO_INT)
		sprintf(str,"%d.%02d",ActivityData.ActivityDetails.CrosscountryDetail.Distance/100000,ActivityData.ActivityDetails.CrosscountryDetail.Distance/1000%100);
//	#else
	//	sprintf(str,"%0.2f",ActivityData.ActivityDetails.CrosscountryDetail.Distance/100000.f);
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
	
	
	//ʱ��	   
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
			//ʱ����
			sprintf(str,"%d:%02d:%02d",second/3600,second/60%60,second%60);
		}
		else
		{
			//����
			sprintf(str,"%d:%02d",second/60%60,second%60);
		}
		word.kerning = 1;
		LCD_SetNumber(str,&word);
	
	
	
	//ƽ������ 
		charlen = strlen(sportdetailstrsgod[SetValue.Language][2]);
		word.x_axis =  120 + 4 + 12;
		word.y_axis = 35;
		word.kerning = 0;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][2],&word);
		
		word.y_axis += charlen*8+1;
		LCD_SetString(colon,&word);
		//�����ٶȴ���
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
	
	   
	   //ƽ������
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
	
	
		word.x_axis = 163+36;//������
		LCD_SetRectangle(word.x_axis,4+8+4,0,240,LCD_MAROON,0,0,LCD_FILL_ENABLE);
	   
	   LCD_SetPicture(word.x_axis+4, LCD_CENTER_JUSTIFIED, LCD_WHITE, LCD_NONE, &Img_Pointing_Down_12X8);
	
		
	 
	}

//��ϸ���ݽ���2
static void gui_sport_crosscountry_hike_detail_2_paint(uint8_t backgroundcolor)
	{
		SetWord_t word = {0};
		uint32_t sec = 0;
		char str[20] = {0};
		
		char *colon = ":";
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
		
	
    LCD_SetPicture(10, LCD_CENTER_JUSTIFIED, LCD_BLACK, LCD_NONE, &Img_Pointing_Up_12X8);
	
	word.bckgrndcolor = LCD_NONE;
	//�ܲ���
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

	
	//�ۻ�����
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
	
     word.x_axis =  120 + 8 + 16 + 4 + 19 - 16;
     word.y_axis += strlen(str)*10  + 8;
	 word.kerning = 0;
	 word.size = LCD_FONT_16_SIZE;
	 LCD_SetString("m",&word);
	
		
		
	
    LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, LCD_BLACK, LCD_NONE, &Img_Pointing_Down_12X8);
	
}

/*
ͽ��ԽҰ������ϸ���ݽ���1
ActivityDataStr     �����
SportScheduleCalStr �������������
backgroundcolor     
*/
#if defined WATCH_SPORT_EVENT_SCHEDULE
void gui_sport_event_crosscountry_hike_detail_1_paint(ActivityDataStr m_ActivityData
																										 ,SportScheduleCalStr m_sport_str
																										 ,uint8_t backgroundcolor)
{
	SetWord_t word = {0};
	char str[30] = {0};
	uint32_t second;
	uint8_t charlen;
	
	//����ʱ��
	second = m_ActivityData.ActTime;
	
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
	LCD_SetPicture(16, LCD_CENTER_JUSTIFIED, LCD_GREEN, backgroundcolor
									,&Img_Sport[getImgSport(m_sport_str.schedule.sport_type)]);
	//ʱ��
	word.x_axis = 56;

	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_20_SIZE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;
	memset(str,0,sizeof(str));
	sprintf(str,"%02d-%02d-%02d %02d:%02d",m_ActivityData.Act_Start_Time.Year+2000,m_ActivityData.Act_Start_Time.Month,
										m_ActivityData.Act_Start_Time.Day,m_ActivityData.Act_Start_Time.Hour,
										m_ActivityData.Act_Start_Time.Minute);
	LCD_SetNumber(str,&word);
	
	//�ָ���
	word.x_axis = 80;
	LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,LCD_AUQAMARIN,0,0,LCD_FILL_ENABLE);

	//��������(��X����)
	memset(str,0,sizeof(str));
	sprintf(str,"%s%d%s",(char *)GuiSportEventStrs[SetValue.Language][5]
		                  ,m_sport_str.schedule.nums
	                    ,(char *)GuiSportEventStrs[SetValue.Language][6]);
	word.x_axis = 88;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.kerning = 0;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString(str,&word);
	//���
	charlen = strlen(SportDetailStrs[SetValue.Language][0]);
	word.x_axis += 26;
	word.y_axis = 95 - charlen*8;
	word.kerning = 0;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][0],&word);
	word.y_axis = 98;
	LCD_SetString(":",&word);

	word.y_axis = 110;
	word.kerning = 1;
	memset(str,0,sizeof(str));
	sprintf(str,"%0.2f",m_ActivityData.ActivityDetails.CrosscountryDetail.Distance/100000.f);	//����װ��Ϊǧ�ף�������2λС��
	LCD_SetNumber(str,&word);

	word.y_axis = 160;
	word.kerning = 0;
	LCD_SetString("km",&word);

	//ʱ��
	charlen = strlen(SportDetailStrs[SetValue.Language][1]);
	word.x_axis += 24;
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
	word.x_axis += 24;
	word.y_axis = 95 - charlen*8;
	word.kerning = 0;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][2],&word);
	word.y_axis = 98;
	LCD_SetString(":",&word);

	word.y_axis = 110;
	word.kerning = 1;
	memset(str,0,sizeof(str));
	sprintf(str,"%02d:%02d",(m_ActivityData.ActivityDetails.CrosscountryDetail.Speed)/60,
													(m_ActivityData.ActivityDetails.CrosscountryDetail.Speed)%60);	
	LCD_SetNumber(str,&word);

	word.y_axis = 160;
	word.kerning = 0;
	LCD_SetString("/km",&word);

	//�������
	charlen = strlen(SportDetailStrs[SetValue.Language][3]);
	word.x_axis += 24;
	word.y_axis = 95 - charlen*8;
	word.kerning = 0;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][3],&word);
	word.y_axis = 98;
	LCD_SetString(":",&word);

	word.y_axis = 110;
	word.kerning = 1;
	memset(str,0,sizeof(str));
	sprintf(str,"%02d:%02d",(m_ActivityData.ActivityDetails.CrosscountryDetail.OptimumSpeed)/60,
													(m_ActivityData.ActivityDetails.CrosscountryDetail.OptimumSpeed)%60);
	LCD_SetNumber(str,&word);

	word.y_axis = 160;
	word.kerning = 0;
	LCD_SetString("/km",&word);

	//�·���ʾ
	gui_sport_page_prompt(SPORT_PAGE_DOWN);
}
/*
ͽ��ԽҰ������ϸ���ݽ���3
ActivityDataStr     �����
SportScheduleCalStr �������������
backgroundcolor     
*/
void gui_sport_event_crosscountry_hike_detail_3_paint(ActivityDataStr m_ActivityData
																										 ,SportScheduleCalStr m_sport_str
																										 ,uint8_t backgroundcolor)
{
		SetWord_t word = {0};
		uint32_t second = 0;
		char str[30] = {0};
		uint8_t charlen;
	
		second = m_sport_str.total_time;//��ʱ��
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
	
		//����Ч��
		charlen = strlen(SportDetailStrs[SetValue.Language][7]);
		word.x_axis = 40;
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
		sprintf(str,"%.1f",ActivityData.ActivityDetails.CrosscountryDetail.AerobicEffect/10.f); 	//����һλС����
		LCD_SetNumber(str,&word);
	
		//����Ч��
		charlen = strlen(SportDetailStrs[SetValue.Language][8]);
		word.x_axis += 24;
		word.y_axis = 95 - charlen*8;
		word.kerning = 0;
		LCD_SetString((char *)SportDetailStrs[SetValue.Language][8],&word);
		word.y_axis = 98;
		LCD_SetString(":",&word);
	
		word.y_axis = 110;
		word.kerning = 1;
		memset(str,0,sizeof(str));
		sprintf(str,"%.1f",ActivityData.ActivityDetails.CrosscountryDetail.AnaerobicEffect/10.f);	//����һλС����
		LCD_SetNumber(str,&word);
	
		//�ָ�ʱ��
		charlen = strlen(SportDetailStrs[SetValue.Language][9]);
		word.x_axis += 24;
		word.y_axis = 95 - charlen*8;
		word.kerning = 0;
		LCD_SetString((char *)SportDetailStrs[SetValue.Language][9],&word);
		word.y_axis = 98;
		LCD_SetString(":",&word);
	
		word.y_axis = 110;
		word.kerning = 1;
		memset(str,0,sizeof(str));
		sprintf(str,"%.1f",ActivityData.ActivityDetails.CrosscountryDetail.RecoveryTime/60.f);
		LCD_SetNumber(str,&word);
	
		word.y_axis = 160;
		word.kerning = 0;
		LCD_SetString("h",&word);
		//�����ۼ�
		word.x_axis += 24;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString((char *)GuiSportEventStrs[SetValue.Language][7],&word);
	
		//�����
		charlen = strlen(GuiSportEventStrs[SetValue.Language][8]);
		word.x_axis += 24;
		word.y_axis = 95 - charlen*8;
		word.kerning = 0;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString((char *)GuiSportEventStrs[SetValue.Language][8],&word);
		word.y_axis = 98;
		LCD_SetString(":",&word);

		word.y_axis = 110;
		word.kerning = 1;
		memset(str,0,sizeof(str));
		sprintf(str,"%0.2f",m_sport_str.total_distance/100000.f);	//����װ��Ϊǧ�ף�������2λС��
		LCD_SetNumber(str,&word);

		word.y_axis = 160;
		word.kerning = 0;
		LCD_SetString("km",&word);
		//��ʱ��
		charlen = strlen((char *)GuiSportEventStrs[SetValue.Language][9]);
		word.x_axis += 24;
		word.y_axis = 95 - charlen*8;
		word.kerning = 0;
		LCD_SetString((char *)GuiSportEventStrs[SetValue.Language][9],&word);
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
		//��ƽ������
		charlen = strlen(GuiSportEventStrs[SetValue.Language][10]);
		word.x_axis += 24;
		word.y_axis = 95 - charlen*8;
		word.kerning = 0;
		LCD_SetString((char *)GuiSportEventStrs[SetValue.Language][10],&word);
		word.y_axis = 98;
		LCD_SetString(":",&word);

		word.y_axis = 110;
		word.kerning = 1;
		memset(str,0,sizeof(str));
		sprintf(str,"%02d:%02d",(m_sport_str.total_avg_pace)/60,
														(m_sport_str.total_avg_pace)%60);	
		LCD_SetNumber(str,&word);

		word.y_axis = 160;
		word.kerning = 0;
		LCD_SetString("/km",&word);
		//�·���ʾ
		gui_sport_page_prompt(SPORT_PAGE_DOWN);
}
#endif
//ͽ��ԽҰ��ϸ���ݽ���
void gui_sport_hike_detail_paint(void)
{
	switch(m_gui_sport_hike_pause_index)
	{
		case GUI_SPORT_HIKE_DETAIL_1:
		{
		
		
			gui_sport_crosscountry_hike_detail_1_paint(LCD_WHITE);
		}
			break;
		case GUI_SPORT_HIKE_DETAIL_2:
			gui_sport_crosscountry_hike_detail_2_paint(LCD_WHITE);
			break;
		/*case GUI_SPORT_HIKE_DETAIL_3:
		{
		
			gui_sport_crosscountry_hike_detail_3_paint(LCD_BLACK);
		}
			break;*/
		case GUI_SPORT_HIKE_DETAIL_TRACK:
			gui_sport_detail_track_paint(LCD_WHITE);
			break;
		/*case GUI_SPORT_HIKE_DETAIL_HEARTRATEZONE:
			gui_sport_detail_heartratezone_paint(LCD_BLACK);
			break;*/
		default:
			break;
	}
}


void gui_sport_crosscountry_hike_init(void)
{
	//Ĭ��Ϊ�ܲ����ݽ���
	m_gui_sport_hike_index = GUI_SPORT_HIKE_SCREEN_DATA_1;
}


void gui_sport_hike_btn_evt(uint32_t Key_Value)
{
	switch( Key_Value )
	{
		case KEY_OK:
			if(g_sport_status == SPORT_STATUS_READY)
			{
				//��ʼ��ʱ
				g_sport_status = SPORT_STATUS_START;
				
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
				
				ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY_HIKE_PAUSE;
				m_gui_sport_hike_pause_index = GUI_SPORT_HIKE_PAUSE_CONTINUE;
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
				if(m_gui_sport_hike_index < (GUI_SPORT_HIKE_SCREEN_MAX -1))
				{
					m_gui_sport_hike_index++;
				}
				else
				{
					m_gui_sport_hike_index = 0;
				}
				
				gui_swich_msg();
			}
			break;
		case KEY_UP:
			if (g_sport_status != SPORT_STATUS_READY)
			{
           		//��ҳ�鿴�ܲ�����
				if(m_gui_sport_hike_index > 0)
				{
					m_gui_sport_hike_index--;
				}
				else
				{
					m_gui_sport_hike_index = (GUI_SPORT_HIKE_SCREEN_MAX -1);
				}
				
				gui_swich_msg();
			}
		    break;
		case KEY_BACK:
 			if(g_sport_status == SPORT_STATUS_PAUSE)
			{
				//������ͣ�˵�����
				ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY_HIKE_PAUSE;
				m_gui_sport_hike_index = GUI_SPORT_HIKE_PAUSE_CONTINUE;
			
				gui_swich_msg();
			
			}
			else if(g_sport_status == SPORT_STATUS_READY)
			{
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
		case KEY_LONG_UP:
			if(g_sport_status == SPORT_STATUS_START)
			{
				//�켣���汣�溽���������
				//if(m_gui_sport_hike_index == GUI_SPORT_HIKE_SCREEN_TRACK)
				if(m_gui_sport_hike_index == GUI_SPORT_HIKE_SCREEN_DATA_3)
				{	
					sport_dottrack();

				}	
			}
			break;
		default:
			break;
	}
	gui_sport_index = m_gui_sport_hike_index;
	
	gui_sport_type = 3;
	
}

//ͽ��ԽҰ��ͣ�˵����水���¼�
void gui_sport_hike_pause_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
			if(m_gui_sport_hike_pause_index == GUI_SPORT_HIKE_PAUSE_CONTINUE)
			{
				//������ʱ����ʼ�˶�
				g_sport_status = SPORT_STATUS_START;
				Set_PauseSta(false);
				
				ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY_HIKE;
				gui_swich_msg();
				Save_Detail_Index = ScreenState;
				timer_app_pasue_start();
				//����ʾ
				gui_sport_motor_prompt();
			}
			else if(m_gui_sport_hike_pause_index == GUI_SPORT_HIKE_PAUSE_SAVE)
			{
				//�����˶�����������
				g_sport_status = SPORT_STATUS_READY;
				
				//Ĭ��ѡ��
				gui_sport_feel_init();
				
				ScreenState = DISPLAY_SCREEN_SPORT_FEEL;
				m_gui_sport_hike_pause_index = GUI_SPORT_HIKE_DETAIL_1;
				sport_end_flag = 1;
				sport_end_hrvalue = get_10s_heartrate(); //��ȡǰ10sƽ������
				g_HeartNum = 0;
				extern volatile ScreenState_t Save_Detail_Index;
				Save_Detail_Index = DISPLAY_SCREEN_CROSSCOUNTRY_HIKE_DETAIL;
				gui_swich_msg();
			}
			else if(m_gui_sport_hike_pause_index == GUI_SPORT_HIKE_PAUSE_WAIT)
			{
				m_gui_sport_hike_pause_index = GUI_SPORT_HIKE_PAUSE_CONTINUE;
				
				//�Ժ����
				sport_continue_later_start();
				
				//�Ժ���������˶�����
				ContinueLaterDataSave.SportType = DISPLAY_SCREEN_CROSSCOUNTRY_HIKE;
				
				//�Ժ�������˳��˶����棬����������
				ScreenState = DISPLAY_SCREEN_HOME;
				gui_swich_msg();
			}
			else if(m_gui_sport_hike_pause_index == GUI_SPORT_HIKE_PAUSE_TRACKBACK)
			{
				//��ʼѭ������
				g_sport_status = SPORT_STATUS_START;
				
				//��������
				TRACE_BACK_DATA.is_traceback_flag = 1;
				TRACE_BACK_DATA.sport_distance = ActivityData.ActivityDetails.RunningDetail.Distance;
				TRACE_BACK_DATA.remaining_distance = ActivityData.ActivityDetails.RunningDetail.Distance;
				
				//����ѭ��������ʱ��
				trace_back_timer_start();
				
				//ѭ�������켣���
                TrackBackFlag();
				
				//ȡ����ͣ
				Set_PauseSta(false);
				ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY_HIKE;
				gui_swich_msg();
				Save_Detail_Index = ScreenState;
				timer_app_pasue_start();
				gui_sport_motor_prompt();
			}
			else
			{
				//�����˶�
				ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY_HIKE_CANCEL;
				
				m_gui_sport_hike_pause_index = GUI_SPORT_HIKE_PAUSE_CANCEL_NO;
				gui_swich_msg();
			}
			
			break;
		case KEY_UP:
			if(m_gui_sport_hike_pause_index > GUI_SPORT_HIKE_PAUSE_CONTINUE)
			{

			
			
				m_gui_sport_hike_pause_index--;
				if((TRACE_BACK_DATA.is_traceback_flag) && (m_gui_sport_hike_pause_index == GUI_SPORT_HIKE_PAUSE_TRACKBACK))
				{
						//�������ѭ���������̣�����ѭ������ѡ��
						m_gui_sport_hike_pause_index = GUI_SPORT_HIKE_PAUSE_WAIT;
				}
			

			}
			else
			{
				m_gui_sport_hike_pause_index = GUI_SPORT_HIKE_PAUSE_CANCEL;
			}
			
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		    break;
		case KEY_DOWN:
			if(m_gui_sport_hike_pause_index < GUI_SPORT_HIKE_PAUSE_CANCEL)
			{

			
				m_gui_sport_hike_pause_index++;
				if((TRACE_BACK_DATA.is_traceback_flag) && (m_gui_sport_hike_pause_index == GUI_SPORT_HIKE_PAUSE_TRACKBACK))
				{
					//�������ѭ���������̣�����ѭ������ѡ��
					m_gui_sport_hike_pause_index = GUI_SPORT_HIKE_PAUSE_CANCEL;
				}
				

			}
			else
			{
				m_gui_sport_hike_pause_index = GUI_SPORT_HIKE_PAUSE_CONTINUE;
			}
			
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		case KEY_BACK:
			//�������н���
			ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY_HIKE;
		
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}
#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
//�ӹ���GPS�н�����Ƽ����� ͽ��ԽҰ��ͣ�˵����水���¼�
void gui_sport_cloud_navigation_hike_pause_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
			if(m_gui_sport_hike_pause_index == GUI_SPORT_CLOUD_HIKE_PAUSE_CONTINUE)
			{
				//������ʱ����ʼ�˶�
				g_sport_status = SPORT_STATUS_START;
				Set_PauseSta(false);
				
				ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY_HIKE;
				gui_swich_msg();
				Save_Detail_Index = ScreenState;
				timer_app_pasue_start();
				//����ʾ
				gui_sport_motor_prompt();
			}
			else if(m_gui_sport_hike_pause_index == GUI_SPORT_CLOUD_HIKE_PAUSE_SAVE)
			{
				//�����˶�����������
				g_sport_status = SPORT_STATUS_READY;
				memset(&g_sport_event_status,0,sizeof(SportEventStatusStr));
				//Ĭ��ѡ��
				gui_sport_feel_init();
				
				ScreenState = DISPLAY_SCREEN_SPORT_FEEL;
				m_gui_sport_hike_pause_index = GUI_SPORT_HIKE_DETAIL_1;
				sport_end_flag = 1;
				sport_end_hrvalue = get_10s_heartrate(); //��ȡǰ10sƽ������
				g_HeartNum = 0;
				extern volatile ScreenState_t Save_Detail_Index;
				Save_Detail_Index = DISPLAY_SCREEN_CROSSCOUNTRY_HIKE_DETAIL;
				gui_swich_msg();
			}
			else if(m_gui_sport_hike_pause_index == GUI_SPORT_CLOUD_HIKE_PAUSE_WAIT)
			{
				m_gui_sport_hike_pause_index = GUI_SPORT_HIKE_PAUSE_CONTINUE;
				
				//�Ժ����
				sport_continue_later_start();
				
				//�Ժ���������˶�����
				ContinueLaterDataSave.SportType = DISPLAY_SCREEN_CROSSCOUNTRY_HIKE;
				
				//�Ժ�������˳��˶����棬����������
				ScreenState = DISPLAY_SCREEN_HOME;
				gui_swich_msg();
			}
			else if(m_gui_sport_hike_pause_index == GUI_SPORT_CLOUD_HIKE_PAUSE_TRACKBACK)
			{
				//��ʼѭ������
				g_sport_status = SPORT_STATUS_START;
				
				//��������
				TRACE_BACK_DATA.is_traceback_flag = 1;
				TRACE_BACK_DATA.sport_distance = ActivityData.ActivityDetails.RunningDetail.Distance;
				TRACE_BACK_DATA.remaining_distance = ActivityData.ActivityDetails.RunningDetail.Distance;
				
				//����ѭ��������ʱ��
				trace_back_timer_start();
				
				//ѭ�������켣���
        TrackBackFlag();
				
				//ȡ����ͣ
				Set_PauseSta(false);
#if defined WATCH_SPORT_EVENT_SCHEDULE
				//���صĹ켣���ݳ�ʼ��
				InitLoadTrack();
#endif
				ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY_HIKE;
				gui_swich_msg();
				Save_Detail_Index = ScreenState;
				timer_app_pasue_start();
				gui_sport_motor_prompt();
			}
			else
			{
				//�����˶�
				ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY_HIKE_CANCEL;
				
				m_gui_sport_hike_pause_index = GUI_SPORT_HIKE_PAUSE_CANCEL_NO;
				gui_swich_msg();
			}
			
			break;
		case KEY_UP:
			if(m_gui_sport_hike_pause_index > GUI_SPORT_CLOUD_HIKE_PAUSE_CONTINUE)
			{

			
			
				m_gui_sport_hike_pause_index--;
				if((TRACE_BACK_DATA.is_traceback_flag) && (m_gui_sport_hike_pause_index == GUI_SPORT_CLOUD_HIKE_PAUSE_TRACKBACK))
				{
						//�������ѭ���������̣�����ѭ������ѡ��
						m_gui_sport_hike_pause_index = GUI_SPORT_CLOUD_HIKE_PAUSE_WAIT;
				}
			

			}
			else
			{
				m_gui_sport_hike_pause_index = GUI_SPORT_CLOUD_HIKE_PAUSE_CANCEL;
			}
			
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		    break;
		case KEY_DOWN:
			if(m_gui_sport_hike_pause_index < GUI_SPORT_CLOUD_HIKE_PAUSE_CANCEL)
			{

			
				m_gui_sport_hike_pause_index++;
				if((TRACE_BACK_DATA.is_traceback_flag) && (m_gui_sport_hike_pause_index == GUI_SPORT_CLOUD_HIKE_PAUSE_TRACKBACK))
				{
					//�������ѭ���������̣�����ѭ������ѡ��
					m_gui_sport_hike_pause_index = GUI_SPORT_CLOUD_HIKE_PAUSE_CANCEL;
				}
				

			}
			else
			{
				m_gui_sport_hike_pause_index = GUI_SPORT_CLOUD_HIKE_PAUSE_CONTINUE;
			}
			
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		case KEY_BACK:
			//�������н���
			ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY_HIKE;
		
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}
#endif
//�����˵����水���¼�
void gui_sport_hike_cancel_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
			if(m_gui_sport_hike_pause_index == GUI_SPORT_HIKE_PAUSE_CANCEL_NO)
			{
				//ȡ�������˶��������ϼ��˵�����
				ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY_HIKE_PAUSE;
				m_gui_sport_hike_pause_index = GUI_SPORT_HIKE_PAUSE_CANCEL;
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
			if(m_gui_sport_hike_pause_index == GUI_SPORT_HIKE_PAUSE_CANCEL_NO)
			{
				m_gui_sport_hike_pause_index = GUI_SPORT_HIKE_PAUSE_CANCEL_YES;
			}
			else
			{
				m_gui_sport_hike_pause_index = GUI_SPORT_HIKE_PAUSE_CANCEL_NO;
			}

			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		    break;
		case KEY_BACK:
			//�����ϼ��˵�
			ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY_HIKE_PAUSE;
			m_gui_sport_hike_pause_index = GUI_SPORT_HIKE_PAUSE_CANCEL;
			
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}

//��ϸ���ݽ��水���¼�
void gui_sport_hike_detail_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_UP:
			//�Ϸ�
			if(m_gui_sport_hike_pause_index > GUI_SPORT_HIKE_DETAIL_1)
			{
				m_gui_sport_hike_pause_index--;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
		    break;
		case KEY_DOWN:
			//�·�
			if(m_gui_sport_hike_pause_index < GUI_SPORT_HIKE_DETAIL_TRACK)
			{
				m_gui_sport_hike_pause_index++;

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



