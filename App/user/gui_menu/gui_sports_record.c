#include "gui_menu_config.h"
#include "gui_sports_record.h"
#include "gui_motionsrecord.h"
#include "gui_heartrate.h"
#include "gui_home.h"
#include "gui_sport.h"
#include "gui_climbing.h"
#include "gui_swimming.h"
#include "gui_sport_triathlon.h"
#include "gui_run.h"
#include "gui.h"
#include "gui_sport_crosscountry_hike.h"

#include "SEGGER_RTT.h"
#include <string.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "portmacro.h"
#include "portable.h"
#include "event_groups.h"
#include "semphr.h"

#include "drv_key.h"
#include "drv_lcd.h"
#include "drv_extFlash.h"

#include "font_displaystrs.h"
#include "time_notify.h"
#include "font_config.h"
#include "img_tool.h"
#include "img_menu.h"

#include "com_sport.h"
#include "com_data.h"

#include "task.h"
#include "task_sport.h"
#include "task_display.h"
#include "task_timer.h"

#define GUI_SPORTS_RECORD_LOG_ENABLED 1

#if DEBUG_ENABLED == 1 && GUI_SPORTS_RECORD_LOG_ENABLED == 1
	#define GUI_SPORTS_RECORD_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_SPORTS_RECORD_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define GUI_SPORTS_RECORD_WRITESTRING(...)
	#define GUI_SPORTS_RECORD_PRINTF(...)		        
#endif


//#define DELAY_MS(X)                 am_util_delay_ms(X)
#define DELAY_MS(X)                 vTaskDelay(X / portTICK_PERIOD_MS)
//#define DELAY_MS(X)                   do{TickType_t xLastWakeTime;xLastWakeTime = xTaskGetTickCount ();vTaskDelayUntil( &xLastWakeTime, X / portTICK_PERIOD_MS );}while(0)




extern SetValueStr SetValue;
#define BACKGROUND_COLOR      LCD_WHITE
#define FOREGROUND_COLOR      LCD_BLACK

#define GUI_SPORTS_RECORD_DETAIL_PAGE_ONE        0
#define GUI_SPORTS_RECORD_DETAIL_PAGE_TWO        1
#define GUI_SPORTS_RECORD_DETAIL_PAGE_THREE      2
//#define GUI_SPORTS_RECORD_DETAIL_PAGE_FOUR       3
//#define GUI_SPORTS_RECORD_DETAIL_PAGE_FIVE       4
static uint8_t m_gui_sports_record_detail_index = 0;
extern _gui_sports_record g_gui_sports_record[2];
extern TriathlonDetailStr TriathlonData;			//���ڱ���������������
extern uint8_t m_gui_sport_triathlon_pause_index;

uint32_t activity_data_index = 0;//�����˶���¼�������¼��л����
uint8_t data_up_down_flag = 0;//1up 2down
uint32_t sport_data_addr[2] = {0};
uint32_t g_SportRecord_Going_Screen_Flag = 0;//1 ������������˶���¼
ActivityDataStr g_gpstrack_data;

static void List_Up(void)
{
	SetWord_t word = {0};
//	if(activity_data_index == 0)
//	{
		word.x_axis = 39 -8;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_24_SIZE;
		word.forecolor = BACKGROUND_COLOR;
		word.bckgrndcolor = FOREGROUND_COLOR;
		word.kerning = 0;
		
		//LCD_SetRectangle(0,80,0,LCD_LINE_CNT_MAX,FOREGROUND_COLOR,0,0,LCD_FILL_ENABLE);
		
		LCD_SetString("�˶���¼",&word);
//	}
//	else
//	{
//		word.x_axis = 36;
//		word.y_axis = LCD_CENTER_JUSTIFIED;	
//		word.size = LCD_FONT_16_SIZE;
//		word.forecolor = FOREGROUND_COLOR;
//		word.bckgrndcolor = BACKGROUND_COLOR;
//		word.kerning = 1;	

//		char p_str[30];
//		memset(p_str,0,sizeof(p_str));

//		if(IsActivityDataValid(&Sport_Record[0]) == 1)
//		{
//			sprintf(p_str,"%d-%02d-%02d",Sport_Record[0].Act_Start_Time.Year+2000,
//																	 Sport_Record[0].Act_Start_Time.Month,
//																	 Sport_Record[0].Act_Start_Time.Day);
//			LCD_SetNumber(p_str,&word);
//		}		
//	}

}
static void List_Current(void)
{
	if(g_gui_sports_record[0].year != 0 && g_gui_sports_record[0].year != 0xFF
		 && g_gui_sports_record[0].month != 0 && g_gui_sports_record[0].month != 0xFF
	   && g_gui_sports_record[0].date != 0 && g_gui_sports_record[0].date != 0xFF)
	{
	SetWord_t word = {0};
	word.forecolor = LCD_CYAN;
	word.bckgrndcolor = LCD_BLACK;
	word.kerning = 0;		
	char *colon = ":";

	//word.x_axis = 102;
	//word.y_axis = 5;
	/*���ں�ʱ��*/
	char p_str[30];
	


   word.x_axis = 120 - 12 - 16 - 13 + 13/2 - img_sports_record[0].height/2;
   word.y_axis = 45;
   LCD_SetPicture(word.x_axis,word.y_axis,LCD_PERU,LCD_NONE,&img_sports_record[getImgSport(g_gui_sports_record[0].type)]);

  	memset(p_str,0,sizeof(p_str));
	sprintf(p_str,"%d-%02d-%02d %02d:%02d",g_gui_sports_record[0].year, g_gui_sports_record[0].month,g_gui_sports_record[0].date,
			g_gui_sports_record[0].time_stamp.hour, g_gui_sports_record[0].time_stamp.minute);
	word.x_axis = 120 - 12 - 16 - 13;
	word.y_axis += img_sports_record[0].width + 8;
	word.size = LCD_FONT_13_SIZE;
	word.forecolor = LCD_CYAN;
	word.bckgrndcolor = LCD_BLACK;
	word.kerning = 0;		
	LCD_SetNumber(p_str,&word);	

	/*ʱ��:*/
   word.x_axis = 120 - 12;
   word.y_axis = 45;
   word.size = LCD_FONT_24_SIZE;
   word.forecolor = LCD_CYAN;
   word.bckgrndcolor = LCD_BLACK;
   LCD_SetString("ʱ��",&word);	
	 
   word.y_axis += 2*24+1;
   LCD_SetString(colon,&word);
	/*ʱ��ֵ*/
	memset(p_str,0,sizeof(p_str));
 		
	sprintf(p_str,"%02d:%02d:%02d",g_gui_sports_record[0].time_elapse.hour, g_gui_sports_record[0].time_elapse.minute, g_gui_sports_record[0].time_elapse.second);
	word.x_axis = 120 + 12 - 24;
    word.y_axis +=  12+1 ;
	word.size = LCD_FONT_25_SIZE;
	LCD_SetNumber(p_str,&word);		
	
  
	/*����*/
   word.x_axis = 120 + 12 + 16;
   word.y_axis = 45;
   word.size = LCD_FONT_24_SIZE;
   word.forecolor = LCD_CYAN;
   word.bckgrndcolor = LCD_BLACK;
   LCD_SetString("����",&word);	
   word.y_axis += 2*24+1;
   LCD_SetString(colon,&word);


     /*����ֵ*/    
	memset(p_str,0,sizeof(p_str));
	
	sprintf(p_str,"%d.%02d",(uint32_t)g_gui_sports_record[0].distance,(uint32_t)(g_gui_sports_record[0].distance*100)%100);
  //  sprintf(p_str,"%.2f",g_gui_sports_record[0].distance);
    word.x_axis =  120 + 12 + 16 + 24 - 25;
    word.y_axis += 12+1 ;
	word.size = LCD_FONT_25_SIZE;
	LCD_SetNumber(p_str,&word);	
   
       word.kerning = 0;	   
	   word.x_axis = 120 + 12 + 16 + 24 - 16;
	   word.y_axis += strlen(p_str)*Font_Number_16.width+16;
	   
	   word.size = LCD_FONT_16_SIZE;
	   LCD_SetString("km",&word);		   
	}

}
static void List_Down(void)
{
	SetWord_t word = {0};
	word.x_axis = 180 + 8 +8;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_LIGHTGRAY;
	word.bckgrndcolor = LCD_BLACK;
	word.kerning = 1;	

	char p_str[30];
	memset(p_str,0,sizeof(p_str));

	if(g_gui_sports_record[1].year != 0 && g_gui_sports_record[1].year != 0xFF
		 && g_gui_sports_record[1].month != 0 && g_gui_sports_record[1].month != 0xFF
	   && g_gui_sports_record[1].date != 0 && g_gui_sports_record[1].date != 0xFF)
	{
		sprintf(p_str,"%d-%02d-%02d",g_gui_sports_record[1].year,
																 g_gui_sports_record[1].month,
																 g_gui_sports_record[1].date);
		
		LCD_SetNumber(p_str,&word);
	}		
}
void List_None(void)
{
	//���ñ�����ɫ
	LCD_SetBackgroundColor(LCD_BLACK);
	if(SetValue.IsFirstSport == 1)
	{
		SetWord_t word = {0};
		word.x_axis = 72;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_24_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_NONE;
		word.kerning = 1;
		LCD_SetString("�����˶���¼",&word);
		word.x_axis += 24 + 12;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_24_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_NONE;
		word.kerning = 1;
		LCD_SetString("������ĵ�һ��",&word);
		word.x_axis += 24 + 12;
		LCD_SetString("�˶���!",&word);
	}
	else
	{
		SetWord_t word = {0};
		word.x_axis = 90;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_24_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_NONE;
		word.kerning = 1;
		LCD_SetString("�����˶���¼",&word);
		word.x_axis += 24 + 12;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_24_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_NONE;
		word.kerning = 1;
		LCD_SetString("�Ͽ�ȥ�˶���!",&word);
	}
}
void gui_sports_record_paint(void)
{
	LCD_SetBackgroundColor(LCD_BLACK);
	
	if(g_ActivityData_Total_Num != 0)
	{//���˶���¼ʱ
		List_Up();//��һҳ
		List_Current();//��ǰ
		List_Down();//��һҳ

		if((activity_data_index == 0 && g_ActivityData_Total_Num >= 2 && data_up_down_flag == 0) ||
			 (activity_data_index > 0 && data_up_down_flag == 2 && g_ActivityData_Total_Num >= 2 && activity_data_index < g_ActivityData_Total_Num-1)
			||(activity_data_index+1 == 0 && g_ActivityData_Total_Num >= 2))
		{
			//�¼�ͷ
			LCD_SetPicture(222, 116, LCD_PERU, LCD_NONE, &Img_Pointing_Down_12X8);
		}
		else if(activity_data_index+1 > 0 && data_up_down_flag == 1 && g_ActivityData_Total_Num >= 2 && activity_data_index < g_ActivityData_Total_Num-1)
		{
			//�ϼ�ͷ
			LCD_SetPicture(222, 116, LCD_PERU, LCD_NONE, &Img_Pointing_Up_12X8);
		}
		LCD_SetRectangle(70-8,2,120 - 80,160,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
		LCD_SetRectangle(179+8,2,120 - 80,160,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
	}
	else
	{
		List_None();//���˶�����
	}

}
#if defined WATCH_SPORT_EVENT_SCHEDULE
/*�������� ͽ��ԽҰ ԽҰ�������� �˶���¼���鲻ͬ*/
void search_sport_event_schedule_record(ActivityDataStr m_ActivityDataStr)
{
	uint8_t sport_type = 0;
	memset(&g_sport_event_status,0,sizeof(SportEventStatusStr));
	memset(&g_sport_event_schedule_detail_cal,0,sizeof(SportScheduleCalStr));
	//�Ƿ�������
	g_sport_event_status.event_str = ActivityRecordData.sport_event;//��ȡ��ǰ�Ƿ���ͽ��ԽҰ����
	g_sport_event_status.event_id = get_sport_event_id(ActivityRecordData.sport_event);
	g_sport_event_status.nums = ActivityRecordData.sport_event.U_EventSport.bits.Shedule_Nums;
	g_sport_event_status.sport_type = ActivityRecordData.Activity_Type;
	g_sport_event_status.status = ActivityRecordData.sport_event.U_EventSport.bits.Event_Status;
	g_sport_event_status.total_nums = ActivityRecordData.sport_event.U_EventSport.bits.Shedule_Nums;
	
	if(is_sport_event_schedule_valid(g_sport_event_status) == true)
	{//��Ч����
		g_sport_event_schedule_detail_cal = get_sport_event_detail_total(g_sport_event_status);
	}
}
#endif

void gui_sports_record_btn_evt(uint32_t evt)
{
	switch(evt)
	{		


       	case (KEY_OK):{
			if(g_ActivityData_Total_Num != 0)
			{
				
				m_gui_sports_record_detail_index = 0;
				m_gui_sport_triathlon_pause_index = 0;//���������
				DISPLAY_MSG  msg = {0,0};
				ScreenState = DISPLAY_SCREEN_SPORTS_RECORD_DETAIL;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
	
		}break;	


	
		case (KEY_BACK):{
			DISPLAY_MSG  msg = {0,0};
			activity_data_index = 0;
			data_up_down_flag = 0;
			if(g_SportRecord_Going_Screen_Flag == 1)
			{//����������˶���¼����
				g_SportRecord_Going_Screen_Flag = 0;
				ScreenState = DISPLAY_SCREEN_HOME;
			}
			else
			{//�˵�������˶���¼����
				ScreenState = DISPLAY_SCREEN_MENU;
			}
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}break;			
		case (KEY_UP):{
			//�Ϸ�ҳ
			if(activity_data_index > 0)
			{
				activity_data_index --;
				data_up_down_flag = 1;
				if(activity_data_index+1 >= g_ActivityData_Total_Num)
				{
					GetSportDetail(g_ActivityData_Total_Num-1,0);//��ʾ���һ��
				}
				else
				{
					GetSportDetail(activity_data_index,activity_data_index+1);
				}
				if(g_gui_sports_record[0].year != 0 && g_gui_sports_record[0].year != 0xFF
				 && g_gui_sports_record[0].month != 0 && g_gui_sports_record[0].month != 0xFF
				 && g_gui_sports_record[0].date != 0 && g_gui_sports_record[0].date != 0xFF)
				{
					DISPLAY_MSG  msg = {0,0};
					ScreenState = DISPLAY_SCREEN_SPORTS_RECORD;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				}
			}
		
		}break;
			
		
		case (KEY_DOWN):{
			//�·�ҳ
			if(activity_data_index < (g_ActivityData_Total_Num-1))
			{
				activity_data_index ++;
				data_up_down_flag = 2;
				if(activity_data_index+1 >= g_ActivityData_Total_Num)
				{
					GetSportDetail(g_ActivityData_Total_Num-1,0);//��ʾ���һ��
				}
				else
				{
					GetSportDetail(activity_data_index,activity_data_index+1);
				}
				if(g_gui_sports_record[0].year != 0 && g_gui_sports_record[0].year != 0xFF
				 && g_gui_sports_record[0].month != 0 && g_gui_sports_record[0].month != 0xFF
				 && g_gui_sports_record[0].date != 0 && g_gui_sports_record[0].date != 0xFF)
				{
					DISPLAY_MSG  msg = {0,0};
					ScreenState = DISPLAY_SCREEN_SPORTS_RECORD;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				}
			}
			
		}break;					
		default:
			break;
	}
}
/*�˶���¼�鿴�켣��GPS��ַ*/
void Get_SportTrack_GpsAddress(uint32_t addr)
{
	memset(&g_gpstrack_data,0,sizeof(g_gpstrack_data));
	dev_extFlash_enable();
	dev_extFlash_read(addr,(uint8_t *)&g_gpstrack_data,sizeof(g_gpstrack_data));
	dev_extFlash_disable();
}
static void GetGpsTrack(void)
{
	DISPLAY_MSG  msg = {0,0};
	
	LOADINGFLAG = 1;       //�����б�־
	ScreenState = DISPLAY_SCREEN_TRACK_LOAD;      
	msg.cmd = MSG_DISPLAY_SCREEN;
	xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
	Send_Timer_Cmd(CMD_SPORTS_TRAIL_LOAD);
}
void gui_sports_track_paint(uint8_t backgroundcolor)
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
	LCD_SetPicture(10, LCD_CENTER_JUSTIFIED, word.forecolor, LCD_NONE, &Img_Pointing_Up_12X8);

	word.x_axis = 25;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	// word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][12],&word);
	
	if(g_gui_sports_record[0].type == ACT_TRIATHLON_SWIM
		 || g_gui_sports_record[0].type == ACT_TRIATHLON_CYCLING
	   || g_gui_sports_record[0].type == ACT_TRIATHLON_RUN)
	{//���������ʾ�¼�ͷ
		DrawLoadALLTrack();
//		DrawLoadTrack();
	}
	else
	{
		DrawLoadALLTrack();//�鿴�켣
//		DrawLoadTrack();
		//LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, word.forecolor, LCD_NONE, &Img_Pointing_Down_12X8); 
	}

}
/*����Ӿ�� ����*/
static void sports_record_detail_page_one(void)
	{
	   SetWord_t word = {0};
	   char str[20];
	   char *colon = ":";
	   uint8_t charlen;
	   
	  // uint32_t second;
	
		//����ʱ��
		//second = ActivityData.ActTime;//get_time_diff(ActivityData.Act_Start_Time, ActivityData.Act_Stop_Time) - Get_PauseTotolTime();
	
		//���ñ�����ɫ
		LCD_SetBackgroundColor(LCD_BLACK);
	
	
		word.forecolor = LCD_WHITE;
		word.x_axis = 16;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_24_SIZE;
		word.bckgrndcolor = LCD_NONE;
		if(g_gui_sports_record[0].type==ACT_RUN)
			{
           
		   LCD_SetString("�ܲ�", &word);
		   }
		else if(g_gui_sports_record[0].type==ACT_CYCLING)
			{
		    LCD_SetString("����", &word);
			}
		else if(g_gui_sports_record[0].type==ACT_CLIMB)
			{
		    LCD_SetString("��ɽ", &word);
			}
		else if(g_gui_sports_record[0].type==ACT_SWIMMING)
			{
		    LCD_SetString("��Ӿ", &word);
			}
		else if(g_gui_sports_record[0].type==ACT_HIKING)
			{
		    LCD_SetString("ͽ��ԽҰ", &word);
			}
		else if(g_gui_sports_record[0].type==ACT_CROSSCOUNTRY)
			{
		    LCD_SetString("ԽҰ��", &word);
			}
	
		
	   //����ʱ��
		memset(str,0,20);
		//sprintf(str,"%02d-%02d-%02d %02d:%02d",ActivityData.Act_Start_Time.Year,ActivityData.Act_Start_Time.Month,ActivityData.Act_Start_Time.Day,ActivityData.Act_Start_Time.Hour,ActivityData.Act_Start_Time.Minute);

		sprintf(str,"%d-%02d-%02d %02d:%02d",g_gui_sports_record[0].year, g_gui_sports_record[0].month, g_gui_sports_record[0].date, g_gui_sports_record[0].time_stamp.hour,g_gui_sports_record[0].time_stamp.minute);	
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
		//sprintf(str,"%0.2f",ActivityData.ActivityDetails.RunningDetail.Distance/100000.f);
		
		sprintf(str,"%d.%02d",(uint32_t)g_gui_sports_record[0].distance,(uint32_t)(g_gui_sports_record[0].distance*100)%100);
		//sprintf(str,"%.2f",g_gui_sports_record[0].distance);	
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
		sprintf(str,"%02d:%02d:%02d",g_gui_sports_record[0].time_elapse.hour, g_gui_sports_record[0].time_elapse.minute, g_gui_sports_record[0].time_elapse.second);
		/*if(second >= 3600)
		{
			//ʱ����
			sprintf(str,"%d:%02d:%02d",second/3600,second/60%60,second%60);
		}
		else
		{
			//����
			sprintf(str,"%d:%02d",second/60%60,second%60);
		}*/
		word.kerning = 1;
		LCD_SetNumber(str,&word);
	
	
	
	//ƽ������&ƽ���ٶ�
		charlen = strlen(sportdetailstrsgod[SetValue.Language][2]);
		word.x_axis =  120 + 4 + 12;
		word.y_axis = 35;
		word.kerning = 0;
		word.size = LCD_FONT_16_SIZE;
		if((g_gui_sports_record[0].type==ACT_RUN)||(g_gui_sports_record[0].type==ACT_HIKING)||(g_gui_sports_record[0].type==ACT_CROSSCOUNTRY))
			{
           
		   LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][2],&word);
		   }
		else if(g_gui_sports_record[0].type==ACT_CYCLING)
			{
			
			LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][14],&word);
			}
		else if(g_gui_sports_record[0].type==ACT_CLIMB)
			{
		    LCD_SetString("��ֱ�ٶ�", &word);
			}
		
		
		
		word.y_axis += charlen*8+1;
		LCD_SetString(colon,&word);
		
		memset(str,0,20);
		//sprintf(str,"%d:%02d",(ActivityData.ActivityDetails.RunningDetail.Pace/60),(ActivityData.ActivityDetails.RunningDetail.Pace%60));
		if((g_gui_sports_record[0].type==ACT_CYCLING)||(g_gui_sports_record[0].type==ACT_CLIMB))//���к͵�ɽΪƽ���ٶ�
			{
            
			sprintf(str,"%.1f",g_gui_sports_record[0].avg_speed/10.f);
		   }
		
		else
			{
		sprintf(str,"%02d:%02d",(int)(g_gui_sports_record[0].avg_speed/60),(int)(g_gui_sports_record[0].avg_speed)%60);	
			}
		word.x_axis =  120 + 4 + 12 + 16 + 4;
		word.y_axis = 35;
		word.kerning = 1;
		word.size = LCD_FONT_19_SIZE;
		LCD_SetNumber(str,&word);
	
		
		word.x_axis =  120 + 4 + 12 + 16 + 4 + 19 - 16;
		word.y_axis += strlen(str)*10 - 0.5*10 + 8;
		word.kerning = 0;
	   word.size = LCD_FONT_16_SIZE;
	   if(g_gui_sports_record[0].type==ACT_CYCLING)
	   	{
         LCD_SetString("km/h",&word);
	   	}
	   else if(g_gui_sports_record[0].type==ACT_CLIMB)
	   	{
           
		   LCD_SetString("m/h",&word);

	    }
	   else
	   	{
		 LCD_SetString("/km",&word);
	   	}
	   
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
		
		sprintf(str,"%d",g_gui_sports_record[0].avg_hr);	
		
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
		LCD_SetRectangle(word.x_axis,4+8+4,0,240,LCD_PERU,0,0,LCD_FILL_ENABLE);
	   
	   LCD_SetPicture(word.x_axis+4, LCD_CENTER_JUSTIFIED, LCD_BLACK, LCD_NONE, &Img_Pointing_Down_12X8);

		
	 
}






static void sports_record_detail_page_two(void)
	{
	   SetWord_t word = {0};
	   char str[20];
	   char *colon = ":";
	   uint8_t charlen;
	  
		//���ñ�����ɫ 
		LCD_SetBackgroundColor(LCD_BLACK);
	
		{
			//�ڵװ���
			word.forecolor = LCD_WHITE;
		
	
	  
		LCD_SetPicture(10, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Up_12X8);
		
		//��·��
		charlen = strlen(sportdetailstrsgod[SetValue.Language][4]);
		word.x_axis = 120 - 4 - 19 - 4 - 16 - 16;
		word.y_axis = 35;
		word.size = LCD_FONT_16_SIZE;
		word.kerning = 0;
		LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][4],&word);
		word.y_axis += charlen*8 + 1;
		LCD_SetString(colon,&word);
		memset(str,0,20);
		//sprintf(str,"%d",ActivityData.ActivityDetails.RunningDetail.Calorie/1000);
		sprintf(str,"%d",(uint32_t)g_gui_sports_record[0].calory);
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
	
	
		//�ܲ���
		charlen = strlen(sportdetailstrsgod[SetValue.Language][5]);
		word.x_axis = 120 - 4 - 19 - 4 - 16 - 16;
		word.y_axis = 130;
		word.kerning = 0;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][5],&word);
		word.y_axis += charlen*8 + 1;
		LCD_SetString(colon,&word);
		memset(str,0,20);
		//sprintf(str,"%d",ActivityData.ActivityDetails.RunningDetail.Steps);
		
		sprintf(str,"%d",(uint32_t)g_gui_sports_record[0].steps);
		word.x_axis = 120 - 4 - 19 - 16;
		word.y_axis = 130;
		word.size = LCD_FONT_19_SIZE;
		word.kerning = 1;
		LCD_SetNumber(str,&word);
	   // word.y_axis = SPORT_DETAIL_DATA_UNITS_Y_AXIS;
	   // word.kerning = 0;
	   // LCD_SetString("bpm",&word);
	
		//ƽ����Ƶ
		charlen = strlen(sportdetailstrsgod[SetValue.Language][6]);
		 word.x_axis =	120;
		word.y_axis = 35;
		word.kerning = 0;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][6],&word);
		word.y_axis += charlen*8+1;
		LCD_SetString(colon,&word);
		memset(str,0,20);
		//sprintf(str,"%d",(uint16_t)(ActivityData.ActivityDetails.RunningDetail.Steps / ActivityData.ActTime));
		
		sprintf(str,"%d",(uint16_t)((g_gui_sports_record[0].steps*60)/(g_gui_sports_record[0].time_elapse.hour*60*60 + g_gui_sports_record[0].time_elapse.minute*60 + g_gui_sports_record[0].time_elapse.second)));
		
		 word.x_axis =	120  + 16 + 4;
		word.y_axis = 35;
		word.kerning = 1;
		word.size = LCD_FONT_19_SIZE;
		LCD_SetNumber(str,&word);
		word.x_axis =  120	+ 16 + 4 + 19 - 16;
		word.y_axis += strlen(str)*10 - 0.5*10 + 8;
		word.kerning = 0;
	   word.size = LCD_FONT_16_SIZE;
		LCD_SetString("spm",&word);
	
	
	
		//ƽ������
		charlen = strlen(sportdetailstrsgod[SetValue.Language][7]);
		word.x_axis = 120;
		word.y_axis = 130;
		word.kerning = 0;
		
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][7],&word);
		word.y_axis += charlen*8+1;
		LCD_SetString(colon,&word);
		memset(str,0,20);
		/*#if defined (SPRINTF_FLOAT_TO_INT)
		sprintf(str,"%d.%01d",ActivityData.ActivityDetails.RunningDetail.RecoveryTime/60,ActivityData.ActivityDetails.RunningDetail.RecoveryTime%60);
	#else
		sprintf(str,"%0.1f",ActivityData.ActivityDetails.RunningDetail.RecoveryTime/60.f);
	#endif*/
		sprintf(str,"%d",(uint8_t)((g_gui_sports_record[0].distance*100000) /g_gui_sports_record[0].steps));//��������ֵ
		word.x_axis = 120 + 16 + 4;
		word.y_axis = 130;
		word.kerning = 1;
		
		word.size = LCD_FONT_19_SIZE;
		LCD_SetNumber(str,&word);
		word.y_axis += strlen(str)*10  + 8;
		word.kerning = 0;
		word.x_axis =  120	+ 16 + 4 + 19 - 16;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString("cm",&word);
	
	
	
		//�ۻ�����
		charlen = strlen(sportdetailstrsgod[SetValue.Language][8]);
		word.x_axis =  120 + 4 + 16 + 4 - 16 + 19 + 32; //179
		word.y_axis = 35;
		word.kerning = 0;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][8],&word);
		word.y_axis += charlen*8+1;
		LCD_SetString(colon,&word);
	
		memset(str,0,20);
		sprintf(str,"%d",(uint16_t)(g_gui_sports_record[0].riseHeight));
		word.x_axis =  120 + 4 + 16 + 4 - 16 + 19 + 32 + 16 - 19;
		word.y_axis += 8;
		word.kerning = 1;
		word.size = LCD_FONT_19_SIZE;
		LCD_SetNumber(str,&word);
		
		 word.x_axis = 179;
		 word.y_axis += strlen(str)*10	+ 8;
		 word.kerning = 0;
		 word.size = LCD_FONT_16_SIZE;
		 LCD_SetString("m",&word);
	
	
		LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Down_12X8);
		
	 
	}


}

static void gui_cycling_sport_detail_2_paint(uint8_t backgroundcolor)

{
	   SetWord_t word = {0};
	   char str[20];
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
	  
		LCD_SetPicture(10, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Up_12X8);
		
		//��·��
		charlen = strlen(sportdetailstrsgod[SetValue.Language][4]);
		word.x_axis = 120 - 4 - 19 - 4 - 16 - 16 - 18;
		word.y_axis = 35;
		word.size = LCD_FONT_16_SIZE;
		word.kerning = 0;
		LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][4],&word);
		word.y_axis += charlen*8 + 1;
		LCD_SetString(colon,&word);
		memset(str,0,20);
		sprintf(str,"%d",ActivityRecordData.ActivityDetails.CyclingDetail.Calorie/1000);
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
	
	
		//����ٶ�
		charlen = strlen(sportdetailstrsgod[SetValue.Language][15]);
		word.x_axis = 120 - 4 - 19 - 4 - 16 - 16 - 18;
		word.y_axis = 130;
		word.kerning = 0;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][15],&word);
		word.y_axis += charlen*8 + 1;
		LCD_SetString(colon,&word);
		memset(str,0,20);
		//sprintf(str,"%d",ActivityData.ActivityDetails.CyclingDetail.OptimumSpeed);
		
		sprintf(str,"%.1f",g_gui_sports_record[0].max_speed/10.f);
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
		//��ߺ���(��ʱδ����)
		charlen = strlen(sportdetailstrsgod[SetValue.Language][47]);
		 word.x_axis =	120 - 18;
		word.y_axis = 35;
		word.kerning = 0;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][47],&word);
		word.y_axis += charlen*8+1;
		LCD_SetString(colon,&word);
		memset(str,0,20);
		sprintf(str,"%d",ActivityRecordData.ActivityDetails.CyclingDetail.MaxAlt);//���������߶ȴ���
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

	
		//�ۻ�����
		charlen = strlen(sportdetailstrsgod[SetValue.Language][8]);
		word.x_axis = 120 - 18;
		word.y_axis = 130;
		word.kerning = 0;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][8],&word);
		
		word.y_axis += charlen*8+1;
		LCD_SetString(colon,&word);
		
		memset(str,0,20);
		sprintf(str,"%d",(uint16_t)(g_gui_sports_record[0].riseHeight));
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
	
		//��������(��ʱδ����)
		charlen = strlen(sportdetailstrsgod[SetValue.Language][20]);
		word.x_axis =  120 + 4 + 16 + 4 - 16 + 19 + 32 - 18; 
		word.y_axis = 35;
		word.kerning = 0;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][20],&word);
		word.y_axis += charlen*8+1;
		LCD_SetString(colon,&word);
	
		memset(str,0,20);
		sprintf(str,"%d",ActivityRecordData.ActivityDetails.CyclingDetail.RiseAlt);
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
	


        //�������(��ʱδ����)
		charlen = strlen(sportdetailstrsgod[SetValue.Language][48]);
		word.x_axis =  120 + 4 + 16 + 4 - 16 + 19 + 32 - 18; 
		word.y_axis = 130;
		word.kerning = 0;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][48],&word);
		word.y_axis += charlen*8+1;
		LCD_SetString(colon,&word);
	
		memset(str,0,20);
		sprintf(str,"%d.%02d",ActivityRecordData.ActivityDetails.CyclingDetail.UphillDistance/100000,ActivityRecordData.ActivityDetails.CyclingDetail.UphillDistance/1000%100);
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
		LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Down_12X8);
		
	 
}

//ԽҰ��/ͽ��ԽҰ ��ϸ���ݽ���2
static void gui_crosscountry_hike_sport_detail_2_paint(uint8_t backgroundcolor)
	{
			SetWord_t word = {0};
			//uint32_t sec = 0;
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
			
		
		LCD_SetPicture(10, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Up_12X8);
		
		
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
		sprintf(str,"%d",g_gui_sports_record[0].steps);
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
		sprintf(str,"%d",(uint16_t)(g_gui_sports_record[0].riseHeight));
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
		
			
			
		
		LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Down_12X8);
		
	}

//ԽҰ��/ͽ��ԽҰ/���� ��ϸ���ݽ���3

//����������ϸ���ݽ���1
static void gui_triathlon_sport_detail_1_paint(uint8_t backgroundcolor)
{
	SetWord_t word = {0};
	char str[20] = {0};
	uint32_t second;
	uint8_t charlen;

	//����ʱ�� = ��ʱ�� - ��ͣʱ��
	second = TriathlonData.SwimmingTime + TriathlonData.FirstWaitTime + TriathlonData.CyclingTime
			+ TriathlonData.SecondWaitTime + TriathlonData.RunningTime;

	//���ñ�����ɫ
	LCD_SetBackgroundColor(backgroundcolor);

	word.forecolor = FOREGROUND_COLOR;
	word.bckgrndcolor = BACKGROUND_COLOR;
	LCD_SetPicture(16, LCD_CENTER_JUSTIFIED, LCD_GREEN, BACKGROUND_COLOR, &Img_Sport[getImgSport(g_gui_sports_record[0].type)]);	
	
	char p_str[30];
	memset(p_str,0,sizeof(p_str));
	sprintf(p_str,"%d-%02d-%02d %02d:%02d",g_gui_sports_record[0].year,
																				 g_gui_sports_record[0].month,
																				 g_gui_sports_record[0].date,
																				 g_gui_sports_record[0].time_stamp.hour,
																				 g_gui_sports_record[0].time_stamp.minute);	
	word.x_axis = 56;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 1;
	LCD_SetNumber(p_str,&word);	
	word.kerning = 0;
	
	word.x_axis = 80;
	LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,FOREGROUND_COLOR,0,0,LCD_FILL_ENABLE);

	//��Ӿ���
	charlen = strlen(SportStrs[SetValue.Language][Swimming]);
	word.x_axis += 50;
	word.y_axis = 95 - charlen*8;
	word.kerning = 0;
	LCD_SetString("��Ӿ",&word);
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
	sprintf(str,"%02d:%02d:%02d",second/3600,second/60%60,second%60);
	LCD_SetNumber(str,&word);

	//�·���ʾ
	gui_page_down(FOREGROUND_COLOR);
}
/*static void gui_triathlon_sport_detail_paint(void)
{
	uint8_t x = 50;
	uint8_t pageMax = TriathlonData.FininshNum > 2 ? 2 : TriathlonData.FininshNum;
	
	if(m_gui_sport_triathlon_pause_index == 0)
	{
		gui_triathlon_sport_detail_1_paint(LCD_WHITE);
	}
	else if(m_gui_sport_triathlon_pause_index > pageMax)
	{
		GetGpsTrack();
	}
	else
	{
		//���ñ�����ɫ
		LCD_SetBackgroundColor(LCD_WHITE);
		
		//�Ϸ���ʾ
		gui_page_up(FOREGROUND_COLOR);
		
		if(TriathlonData.FininshNum == 1)
		{
			gui_sport_triathlon_detail_7(x, LCD_BLACK);
			x += 24;

			gui_sport_triathlon_detail_8(x, LCD_BLACK);
		}
		else if(TriathlonData.FininshNum == 2)
		{
			if(m_gui_sport_triathlon_pause_index == 1)
			{
				gui_sport_triathlon_detail_1(x, LCD_BLACK);
				x += 50;
				
				gui_sport_triathlon_detail_2(x, LCD_BLACK);
				x += 24;
				
				gui_sport_triathlon_detail_3(x, LCD_BLACK);
				x += 50;
				
				gui_sport_triathlon_detail_7(x, LCD_BLACK);
			}
			else
			{
				gui_sport_triathlon_detail_8(x, LCD_BLACK);
			}
		}
		else
		{
			if(m_gui_sport_triathlon_pause_index == 1)
			{
				gui_sport_triathlon_detail_1(x, LCD_BLACK);
				x += 50;
				
				gui_sport_triathlon_detail_2(x, LCD_BLACK);
				x += 24;
				
				gui_sport_triathlon_detail_3(x, LCD_BLACK);
				x += 50;
				
				gui_sport_triathlon_detail_4(x, LCD_BLACK);
			}
			else
			{
				gui_sport_triathlon_detail_5(x, LCD_BLACK);
				x += 24;
				
				gui_sport_triathlon_detail_6(x, LCD_BLACK);
				x += 50;
				
				gui_sport_triathlon_detail_7(x, LCD_BLACK);
				x += 24;
				
				gui_sport_triathlon_detail_8(x, LCD_BLACK);
			}
		}
		
		//�·���ʾ
		gui_page_down(FOREGROUND_COLOR);
	}
}*/
//��Ӿ�˶���ϸ_����1 Swimming Saving Details 1st Page
static void gui_swimming_sport_detail_1_paint(uint8_t backgroundcolor)
	{
	   SetWord_t word = {0};
	   char str[20];
	   char *colon = ":";
	   uint8_t charlen;
	   
	  // uint32_t second;
	   
	   //����ʱ�� =��ʱ��-��ͣʱ��
	  // second = ActivityData.ActTime;//get_time_diff(ActivityData.Act_Start_Time, ActivityData.Act_Stop_Time)-	Get_PauseTotolTime();
	
	
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
		  LCD_SetString("Ӿ����Ӿ", &word);
	//ʱ��
		memset(str,0,20);
		//sprintf(str,"%02d-%02d-%02d %02d:%02d",ActivityData.Act_Start_Time.Year,ActivityData.Act_Start_Time.Month,ActivityData.Act_Start_Time.Day,ActivityData.Act_Start_Time.Hour,ActivityData.Act_Start_Time.Minute);
        sprintf(str,"%d-%02d-%02d %02d:%02d",g_gui_sports_record[0].year,g_gui_sports_record[0].month,g_gui_sports_record[0].date,g_gui_sports_record[0].time_stamp.hour,g_gui_sports_record[0].time_stamp.minute);	
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
		//sprintf(str,"%0.2f",ActivityData.ActivityDetails.SwimmingDetail.Distance/100000.f);
		
		sprintf(str,"%.0f",ActivityRecordData.ActivityDetails.SwimmingDetail.Distance/100.f);
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

		
		sprintf(str,"%02d:%02d:%02d",g_gui_sports_record[0].time_elapse.hour, g_gui_sports_record[0].time_elapse.minute, g_gui_sports_record[0].time_elapse.second);
		/*if(second >= 3600)
		{
			//ʱ����
			sprintf(str,"%d:%02d:%02d",second/3600,second/60%60,second%60);
		}
		else
		{
			//����
			sprintf(str,"%d:%02d",second/60%60,second%60);
		}*/
		word.kerning = 1;
		LCD_SetNumber(str,&word);
	
	
	//��·��
		charlen = strlen(sportdetailstrsgod[SetValue.Language][4]);
		word.x_axis =  120 + 4 + 12;
		word.y_axis = 35;
		word.kerning = 0;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][4],&word);
		
		word.y_axis += charlen*8+1;
		
		LCD_SetString(colon,&word);
		memset(str,0,20);
		sprintf(str,"%d",ActivityRecordData.ActivityDetails.SwimmingDetail.Calorie/1000);
		word.x_axis =  120 + 4 + 12 + 16 + 4;
		word.y_axis = 35;
		word.kerning = 1;
		word.size = LCD_FONT_19_SIZE;
		LCD_SetNumber(str,&word);
		word.x_axis =  120 + 4 + 12 + 16 + 4 + 19 - 16;
		word.y_axis += strlen(str)*10 - 0.5*10 + 8;
		word.kerning = 0;
		
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString("kcal",&word);
	
	
	//��ˮ����
		 charlen = strlen(sportdetailstrsgod[SetValue.Language][27]);
		 word.x_axis = 120 + 4 + 12;
		 word.y_axis = 130;
		 word.kerning = 0;
		 word.size = LCD_FONT_16_SIZE;
		 LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][27],&word);
		 
		 word.y_axis += charlen*8+1;
		 LCD_SetString(colon,&word);
		 
		 memset(str,0,20);
		 sprintf(str,"%d",ActivityRecordData.ActivityDetails.SwimmingDetail.Strokes);
		 word.x_axis = 120 + 4 + 12 + 16 + 4;
		 word.y_axis = 130;
		 word.kerning = 1;
		 word.size = LCD_FONT_19_SIZE;
		 LCD_SetNumber(str,&word);
		
	
		word.x_axis = 163+36;//������
		LCD_SetRectangle(word.x_axis,4+8+4,0,240,LCD_PERU,0,0,LCD_FILL_ENABLE);
	   
	   LCD_SetPicture(word.x_axis+4, LCD_CENTER_JUSTIFIED, LCD_BLACK, LCD_NONE, &Img_Pointing_Down_12X8);
		
	 
	}


//��Ӿ�˶���ϸ_����2 Swimming Saving Details 2nd Page
static void gui_swimming_sport_detail_2_paint(uint8_t backgroundcolor)
	{
	   SetWord_t word = {0};
	   char str[20];
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
	  
		LCD_SetPicture(10, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Up_12X8);
		
		//(����)ƽ������
		charlen = strlen(sportdetailstrsgod[SetValue.Language][2]);
		word.x_axis = 120 - 4 - 16 - 32 - 19 - 4 - 16;//39
		word.y_axis = 60;
		word.size = LCD_FONT_16_SIZE;
		word.bckgrndcolor = LCD_NONE;
		LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][2],&word);
		
		word.y_axis += charlen*8+1;
		LCD_SetString(colon,&word);
	
		memset(str,0,20);
		//sprintf(str,"%d:%02d",(ActivityData.ActivityDetails.SwimmingDetail.Speed/60),(ActivityData.ActivityDetails.SwimmingDetail.Speed%60));
		
		sprintf(str,"%d:%02d",(ActivityRecordData.ActivityDetails.SwimmingDetail.Speed/60),(ActivityRecordData.ActivityDetails.SwimmingDetail.Speed%60));
		word.x_axis = 120 - 4 - 16 - 32 - 19;
		word.y_axis = 60;
		word.kerning = 1;
		
		word.size = LCD_FONT_19_SIZE;
		LCD_SetNumber(str,&word);
	
		
		word.x_axis = 120 - 4 - 16 - 32 - 16;
		word.y_axis = 60 + strlen(str)*10 - 0.5*10 + 8;
		word.kerning = 0;
		 word.size = LCD_FONT_16_SIZE;
		LCD_SetString("/100m",&word);
	
	
	
		//(ƽ��)��ˮ���� 
		charlen = strlen(sportdetailstrsgod[SetValue.Language][25]);
		word.x_axis = 120 - 4 - 16;
		word.y_axis = 60;
		
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][25],&word);
		
		word.y_axis += charlen*8+1;
		LCD_SetString(colon,&word);
		
		memset(str,0,20);
		sprintf(str,"%d",(uint16_t)(ActivityRecordData.ActivityDetails.SwimmingDetail.Strokes*60/(g_gui_sports_record[0].time_elapse.hour*60*60 + g_gui_sports_record[0].time_elapse.minute*60 + g_gui_sports_record[0].time_elapse.second)));
		
		word.x_axis = 120;
		word.y_axis = 60;
		word.kerning = 1;
		word.size = LCD_FONT_19_SIZE;
		LCD_SetNumber(str,&word);
		
		word.x_axis = 120 + 19 - 16;
		word.y_axis = 60 + strlen(str)*10 - 0.5*10 + 8;
		word.kerning = 0;
		 word.size = LCD_FONT_16_SIZE;
		LCD_SetString("/min",&word);
	
		//ƽ��swolf
		charlen = strlen(sportdetailstrsgod[SetValue.Language][26]);
		word.x_axis = 120 + 19 + 32;
		word.y_axis = 60;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][26],&word);
		word.y_axis += charlen*8+1;
		word.kerning = 0;
		LCD_SetString(colon,&word);
		memset(str,0,20);
		sprintf(str,"%d",(ActivityRecordData.ActivityDetails.SwimmingDetail.Swolf/10)); 
		
		word.x_axis = 120 + 19 + 32 + 16 + 4;
		word.y_axis = 60;
		word.kerning = 1;
		
		word.size = LCD_FONT_19_SIZE;
		LCD_SetNumber(str,&word);
	   
	

		
	 
	}

//��ɽ�˶���ϸ_2����
static void gui_climbing_sport_detail_2_paint(uint8_t backgroundcolor)
	{
		   SetWord_t word = {0};
		   char str[20];
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
		  
			LCD_SetPicture(10, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Up_12X8);
			
			//��·��
			charlen = strlen(sportdetailstrsgod[SetValue.Language][4]);
			word.x_axis = 120 - 4 - 19 - 4 - 16;
			word.y_axis = 35;
			word.size = LCD_FONT_16_SIZE;
			word.kerning = 0;
			LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][4],&word);
			word.y_axis += charlen*8 + 1;
			LCD_SetString(colon,&word);
			memset(str,0,20);
			sprintf(str,"%d",ActivityRecordData.ActivityDetails.ClimbingDetail.Calorie/1000);
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
		
		
			//�ܲ���
			charlen = strlen(sportdetailstrsgod[SetValue.Language][5]);
			word.x_axis = 120 - 4 - 19 - 4 - 16;
			word.y_axis = 130;
			word.kerning = 0;
			word.size = LCD_FONT_16_SIZE;
			LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][5],&word);
			word.y_axis += charlen*8 + 1;
			LCD_SetString(colon,&word);
			memset(str,0,20);
			sprintf(str,"%d",g_gui_sports_record[0].steps);
			word.x_axis = 120 - 4 - 19;
			word.y_axis = 130;
			word.size = LCD_FONT_19_SIZE;
			word.kerning = 1;
			LCD_SetNumber(str,&word);
		
		
			//�ۻ�����
		   charlen = strlen(sportdetailstrsgod[SetValue.Language][8]);
			word.x_axis =  120 + 4 + 12;
			word.y_axis = 35;
			word.kerning = 0;
			word.size = LCD_FONT_16_SIZE;
			LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][8],&word);
			word.y_axis += charlen*8+1;
			LCD_SetString(colon,&word);
			memset(str,0,20);
			sprintf(str,"%d",(uint16_t)(g_gui_sports_record[0].riseHeight));
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
			//��ߺ���(���������߶ȴ���)
			charlen = strlen(sportdetailstrsgod[SetValue.Language][47]);
			word.x_axis = 120 + 4 + 12;
			word.y_axis = 130;
			word.kerning = 0;
			
			word.size = LCD_FONT_16_SIZE;
			LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][47],&word);
			word.y_axis += charlen*8+1;
			LCD_SetString(colon,&word);
			memset(str,0,20);
	
			sprintf(str,"%d",(ActivityRecordData.ActivityDetails.ClimbingDetail.MaxAlt));//
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
		
		
			
		
		
			LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Down_12X8);
			
		 
		}

/*******************
�ܲ�/������/����
1.��� ʱ�� ƽ������ �������
2.��·�� ƽ������ ������� ����Ч�� ����Ч�� �ָ�ʱ��
3.��������
��Ӿ
1.��� ʱ�� ƽ������ ƽ��swolf
2.��·�� ƽ������ ������� ƽ����Ƶ ����Ч�� ����Ч��
3.��������
������
1.��� ʱ�� ƽ���ٶ� ����ٶ�
2.��·�� ƽ������ ������� ����Ч�� ����Ч�� �ָ�ʱ��
3.��������
ԽҰ��/ͽ��ԽҰ/����
1.��� ʱ�� ƽ������ �������
2.��·�� ƽ������ ������� ƽ����Ƶ �����߶� �½��߶�
3.����Ч�� ����Ч�� �ָ�ʱ��
4.��������
��ɽ
1.��� ʱ�� �ۼ����� �ۼ��½� 
2.��·�� ��ֱ�ٶ� ƽ������ ������� ����Ч�� ����Ч��
3.��������
��������
1.��ʱ�� ��Ӿ
2.��һת�� ���г� �ڶ�ת��
3.�ܲ� ƽ������ �������
4.��������
*************/
void gui_sports_record_detail_paint(void)
{
	LCD_SetBackgroundColor(LCD_BLACK);
	switch(g_gui_sports_record[0].type)
	{
		case ACT_RUN://�ܲ�
		//case ACT_WALK://����
		//case ACT_MARATHON://������
		{
			switch(m_gui_sports_record_detail_index)
			{
				case GUI_SPORTS_RECORD_DETAIL_PAGE_ONE:{
					sports_record_detail_page_one();
					//gui_run_save_detail_1_paint();
				}break;
				case GUI_SPORTS_RECORD_DETAIL_PAGE_TWO:{
					sports_record_detail_page_two();
				}break;
				case GUI_SPORTS_RECORD_DETAIL_PAGE_THREE:{
					GetGpsTrack();
				}break;
				/*case GUI_SPORTS_RECORD_DETAIL_PAGE_FOUR:{
					sports_record_detail_page_three();
				}break;*/
				default:
				break;
			}
		}
			break;
		/*case ACT_INDOORRUN://������
		{
			switch(m_gui_sports_record_detail_index)
			{
				case GUI_SPORTS_RECORD_DETAIL_PAGE_ONE:{
					sports_record_detail_page_one();
				}break;
				case GUI_SPORTS_RECORD_DETAIL_PAGE_TWO:{
					sports_record_detail_page_two();
				}break;
				case GUI_SPORTS_RECORD_DETAIL_PAGE_THREE:{
					sports_record_detail_page_three();
				}break;
				default:
				break;
			}
		}
			break;*/
		case ACT_SWIMMING://��Ӿ
		{
			switch(m_gui_sports_record_detail_index)
			{
				case GUI_SPORTS_RECORD_DETAIL_PAGE_ONE:{
					gui_swimming_sport_detail_1_paint(LCD_BLACK);
				}break;
				case GUI_SPORTS_RECORD_DETAIL_PAGE_TWO:{
					gui_swimming_sport_detail_2_paint(LCD_BLACK);
				}break;
				/*case GUI_SPORTS_RECORD_DETAIL_PAGE_THREE:{
					sports_record_detail_page_three();
				}break;*/
				default:
					break;
			}
		}
			break;
		case ACT_CROSSCOUNTRY://ԽҰ��
		case ACT_HIKING://ͽ��ԽҰ
		{
			switch(m_gui_sports_record_detail_index)
			{
				case GUI_SPORTS_RECORD_DETAIL_PAGE_ONE:{
				
					sports_record_detail_page_one();
				}break;
				case GUI_SPORTS_RECORD_DETAIL_PAGE_TWO:{
					gui_crosscountry_hike_sport_detail_2_paint(LCD_BLACK);
				}break;
				case GUI_SPORTS_RECORD_DETAIL_PAGE_THREE:{
				    GetGpsTrack();
					//gui_crosscountry_hike_sport_detail_3_paint(LCD_WHITE);
				}break;
				/*case GUI_SPORTS_RECORD_DETAIL_PAGE_FOUR:{
					GetGpsTrack();
				}break;*/
				/*case GUI_SPORTS_RECORD_DETAIL_PAGE_FIVE:{
					sports_record_detail_page_three();
				}break;*/
				default:
					break;
			}
		}
			break;
		case ACT_CYCLING://����
		{
			switch(m_gui_sports_record_detail_index)
			{
				case GUI_SPORTS_RECORD_DETAIL_PAGE_ONE:{
					sports_record_detail_page_one();
				}break;
				case GUI_SPORTS_RECORD_DETAIL_PAGE_TWO:{
					gui_cycling_sport_detail_2_paint(LCD_BLACK);
				}break;
			
				case GUI_SPORTS_RECORD_DETAIL_PAGE_THREE:{
					GetGpsTrack();
					//gui_crosscountry_hike_sport_detail_3_paint(LCD_WHITE);
				}break;
				/*case GUI_SPORTS_RECORD_DETAIL_PAGE_FOUR:{
					GetGpsTrack();
				}break;
				case GUI_SPORTS_RECORD_DETAIL_PAGE_FIVE:{
					sports_record_detail_page_three();
				}break;*/
				default:
					break;
			}
		}
			break;
		case ACT_CLIMB://��ɽ
		{
			switch(m_gui_sports_record_detail_index)
			{
				case GUI_SPORTS_RECORD_DETAIL_PAGE_ONE:{
					//gui_climbing_sport_detail_1_paint(LCD_WHITE);
					
					sports_record_detail_page_one();
				}break;
				case GUI_SPORTS_RECORD_DETAIL_PAGE_TWO:{
					gui_climbing_sport_detail_2_paint(LCD_BLACK);
				}break;
				case GUI_SPORTS_RECORD_DETAIL_PAGE_THREE:{
					GetGpsTrack();
				}break;
				/*case GUI_SPORTS_RECORD_DETAIL_PAGE_FOUR:{
					sports_record_detail_page_three();
				}break;*/
				default:
					break;
			}
		}
			break;
		/*case ACT_TRIATHLON_SWIM:	
		case ACT_TRIATHLON_CYCLING:
		case ACT_TRIATHLON_RUN://��������
		{
			gui_triathlon_sport_detail_paint();
		}
			break;*/
		default:
			break;
	}
}


void gui_sports_record_detail_btn_evt(uint32_t evt)
{

	uint8_t MaxPageIndex = 0;

	if(g_gui_sports_record[0].type == ACT_TRIATHLON_SWIM
		 || g_gui_sports_record[0].type == ACT_TRIATHLON_CYCLING
	   || g_gui_sports_record[0].type == ACT_TRIATHLON_RUN)
	{
		DISPLAY_MSG  msg = {0,0};
		uint8_t pageMax = TriathlonData.FininshNum > 2 ? 2 : TriathlonData.FininshNum;
	
		pageMax += 1;
		
		switch( evt )
		{
			case KEY_UP:
				//�Ϸ�
				if(m_gui_sport_triathlon_pause_index > 0)
				{
					m_gui_sport_triathlon_pause_index--;
					ScreenState = DISPLAY_SCREEN_SPORTS_RECORD_DETAIL;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				}
					break;
			case KEY_DOWN:
				//�·�
				if(m_gui_sport_triathlon_pause_index < pageMax)
				{
					m_gui_sport_triathlon_pause_index++;
					
					ScreenState = DISPLAY_SCREEN_SPORTS_RECORD_DETAIL;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				}
				break;
			case (KEY_BACK):{
				InitLoadTrack();
				m_gui_sport_triathlon_pause_index = 0;
				ScreenState = DISPLAY_SCREEN_SPORTS_RECORD;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
			}break;	
			case (KEY_OK):{
				
				DISPLAY_MSG  msg = {0,0};
				ScreenState = DISPLAY_SCREEN_SPORTS_RECORD_DELETE;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
			}break;
			default:
				break;
		}
	}
	else
	{
		//static uint8_t MaxPageIndex = 0;
		switch(g_gui_sports_record[0].type)
		{
			
			case ACT_RUN://�ܲ�
			case ACT_CYCLING://����
			case ACT_CLIMB://��ɽ
			case ACT_HIKING://ͽ��ԽҰ
			case ACT_CROSSCOUNTRY://ԽҰ��
				MaxPageIndex = 3;
				break;
			
			case ACT_SWIMMING://��Ӿ
				MaxPageIndex = 2;
				break;
			default:
				MaxPageIndex = 3;
				break;
		}
		switch(evt)
		{		
			case (KEY_BACK):{
				InitLoadTrack();
				DISPLAY_MSG  msg = {0,0};
				ScreenState = DISPLAY_SCREEN_SPORTS_RECORD;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
			}break;			
			case (KEY_UP):{
				if(m_gui_sports_record_detail_index > 0)
				{
					m_gui_sports_record_detail_index--;
				}
				DISPLAY_MSG  msg = {0,0};
				ScreenState = DISPLAY_SCREEN_SPORTS_RECORD_DETAIL;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
			}break;	
			case (KEY_OK):{
				
                      #if DEBUG_ENABLED == 1 && GUI_SPORTS_RECORD_LOG_ENABLED == 1
							 uint32_t i,n,timestamp,dis_g;
							 uint32_t startaddress,endaddress;
							  int32_t Lon,Lat;
							  uint32_t step,time,dis,speed;
								
							 startaddress = ActivityRecordData.CircleTime_Start_Address;

							 endaddress = ActivityRecordData.CircleTime_Stop_Address;

                      
							 
							  if (endaddress >= startaddress)//�Ƿ񳬳�������ַ
							 {
								 n = (endaddress - startaddress)/32;
							 }
							 else
							 {
								 n = (CIRCLETIME_STOP_ADDRESS +1 - startaddress + endaddress - CIRCLETIME_START_ADDRESS)/32;
							 }
							 GUI_SPORTS_RECORD_PRINTF("startaddress:%x,dendaddress:%x,%d\r\n",startaddress,endaddress,n);
							 for (i = 0;i < n;i++)
							 { 
								 if(startaddress + 32*i < CIRCLETIME_STOP_ADDRESS)
                                 												 
								 {
									 dev_extFlash_enable();
									 dev_extFlash_read(startaddress +32*i,(uint8_t*)&Lon,4); 
									 dev_extFlash_read(startaddress +32*i + 4,(uint8_t*)&Lat,4); 
									 dev_extFlash_read(startaddress +32*i + 8,(uint8_t*)&step,4);
									 dev_extFlash_read(startaddress +32*i + 8+4,(uint8_t*)&time,4);
									  dev_extFlash_read(startaddress +32*i + 8+8,(uint8_t*)&dis,4);
									  dev_extFlash_read(startaddress +32*i + 8+12,(uint8_t*)&speed,4); 
									 dev_extFlash_disable();
								 }
								 else
								 {
					
									  dev_extFlash_enable();
									  dev_extFlash_read(CIRCLETIME_START_ADDRESS +16*i -(CIRCLETIME_STOP_ADDRESS +1 - startaddress) ,(uint8_t*)&Lon,4 );
									  dev_extFlash_read(CIRCLETIME_START_ADDRESS +16*i -(CIRCLETIME_STOP_ADDRESS +1 - startaddress)+4 ,(uint8_t*)&Lat,4 );
									  dev_extFlash_read(CIRCLETIME_START_ADDRESS +16*i -(CIRCLETIME_STOP_ADDRESS +1 - startaddress)+8,(uint8_t*)&step,4 );
				                      dev_extFlash_read(CIRCLETIME_START_ADDRESS +16*i -(CIRCLETIME_STOP_ADDRESS +1 - startaddress)+8+4 ,(uint8_t*)&time,4 );
				                      dev_extFlash_disable();
					 
								 }
								 GUI_SPORTS_RECORD_PRINTF("%d,%d,%d,%d,%d,%d\r\n",Lon,Lat,step,time,dis,speed);
								 
								 vTaskDelay(10/portTICK_PERIOD_MS);
							 }
				          
						  GUI_SPORTS_RECORD_PRINTF("****raw data over*******************and opt data start********************************************\r\n");
				             startaddress = ActivityRecordData.GPS_Start_Address;

							 endaddress = ActivityRecordData.GPS_Stop_Address;
							 	  if (endaddress >= startaddress)//�Ƿ񳬳�������ַ
							 {
								 n = (endaddress - startaddress)/16;
							 }
							 else
							 {
								 n = (GPS_DATA_STOP_ADDRESS +1 - startaddress + endaddress - GPS_DATA_START_ADDRESS)/16;
							 }
							 GUI_SPORTS_RECORD_PRINTF("startaddress:%x,dendaddress:%x,%d\r\n",startaddress,endaddress,n);
							 for (i = 0;i < n;i++)
							 { 
								 if(startaddress + 16*i < GPS_DATA_STOP_ADDRESS)
                                 												 
								 {
									 dev_extFlash_enable();
									 
									 dev_extFlash_read(startaddress +16*i + 4,(uint8_t*)&timestamp,4);
									 dev_extFlash_read(startaddress +16*i + 8,(uint8_t*)&Lon,4);
									 dev_extFlash_read(startaddress +16*i + 8+4,(uint8_t*)&Lat,4); 															
									 dev_extFlash_disable();
								 }
								 else
								 {
					
									  dev_extFlash_enable();
									  
									  dev_extFlash_read(GPS_DATA_START_ADDRESS +16*i -(GPS_DATA_STOP_ADDRESS +1 - startaddress)+4,(uint8_t*)&timestamp,4);
									  dev_extFlash_read(GPS_DATA_START_ADDRESS +16*i -(GPS_DATA_STOP_ADDRESS +1 - startaddress)+8,(uint8_t*)&Lon,4 );
				                      dev_extFlash_read(GPS_DATA_START_ADDRESS +16*i -(GPS_DATA_STOP_ADDRESS +1 - startaddress)+8+4 ,(uint8_t*)&Lat,4 );
				                      dev_extFlash_disable();
					 
								 }
								 GUI_SPORTS_RECORD_PRINTF("%d,%d,%d\r\n",Lon,Lat,timestamp);
								 
								 vTaskDelay(10/portTICK_PERIOD_MS);
							 }
							GUI_SPORTS_RECORD_PRINTF("************************opt gps over and dis start***********************************\r\n");

                             startaddress = ActivityRecordData.DistancePoint_Start_Address;

							 endaddress = ActivityRecordData.DistancePoint_Stop_Address;
							 	  if (endaddress >= startaddress)//�Ƿ񳬳�������ַ
							 {
								 n = (endaddress - startaddress)/4;
							 }
							 else
							 {
								 n = (DISTANCEPOINT_STOP_ADDRESS +1 - startaddress + endaddress - DISTANCEPOINT_START_ADDRESS)/4;
							 }
							 GUI_SPORTS_RECORD_PRINTF("startaddress:%x,dendaddress:%x,%d\r\n",startaddress,endaddress,n);
							 for (i = 0;i < n;i++)
							 { 
								 if(startaddress + 4*i < DISTANCEPOINT_STOP_ADDRESS)
                                 												 
								 {
									 dev_extFlash_enable();
									 
									 dev_extFlash_read(startaddress + 4*i,(uint8_t*)&dis_g,4);
																								
									 dev_extFlash_disable();
								 }
								 else
								 {
					
									  dev_extFlash_enable();
									  dev_extFlash_read(DISTANCEPOINT_START_ADDRESS +4*i -(DISTANCEPOINT_STOP_ADDRESS +1 - startaddress),(uint8_t*)&dis_g,4 );
				                      dev_extFlash_disable();
					 
								 }
								 GUI_SPORTS_RECORD_PRINTF("%d\r\n",dis_g);
								 
								 vTaskDelay(10/portTICK_PERIOD_MS);
							 }
							
							GUI_SPORTS_RECORD_PRINTF("************************dis over***********************************\r\n");
				
			
				#else
				DISPLAY_MSG  msg = {0,0};
				ScreenState = DISPLAY_SCREEN_SPORTS_RECORD_DELETE;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
				#endif
			}break;		
			case (KEY_DOWN):{
				if(m_gui_sports_record_detail_index < (MaxPageIndex -1))
				{
					m_gui_sports_record_detail_index++;
						DISPLAY_MSG  msg = {0,0};
				ScreenState = DISPLAY_SCREEN_SPORTS_RECORD_DETAIL;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
				}
					
			}break;					
			default:
				break;
		}
	}
}


void gui_sports_record_delete_paint(void)
{
	LCD_SetBackgroundColor(LCD_BLACK);

	SetWord_t word = {0};
	word.kerning = 0;
	word.x_axis = 108;
	word.y_axis = 55;
	word.size = LCD_FONT_24_SIZE;	
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;	
	LCD_SetString("ɾ���˶���¼��",&word);	
	gui_tick_cross();
}


void gui_sports_record_delete_btn_evt(uint32_t evt)
{
	
	switch(evt)
	{		
		case (KEY_BACK):{
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_SPORTS_RECORD_DETAIL;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
		}break;			

		case (KEY_OK):{
			//ȷ��ɾ��
			InitLoadTrack();
			DeleteActivityRecord(s_sport_record_select_str[activity_data_index].address);
			memset(&ActivityRecordData,0,sizeof(ActivityRecordData));
			memset(&ActivityRecordDataDown,0,sizeof(ActivityRecordData));
			memset(&g_gui_sports_record[0],0,sizeof(_gui_sports_record));
			memset(&g_gui_sports_record[1],0,sizeof(_gui_sports_record));
			activity_data_index = 0;
			//��ѯ�����˶���¼  
			if(search_sport_record() != 0)
			{
				//DISPLAY_MSG  msg = {0,0};
				data_up_down_flag = 0;
				GetSportDetail(0,1);//����
			}
			if((ScreenState != DISPLAY_SCREEN_LOGO)
				&& (ScreenState != DISPLAY_SCREEN_NOTIFY_BAT_CHG)
				&& time_notify_is_ok())
			{
				ScreenState = DISPLAY_SCREEN_SPORTS_RECORD;
				if(ScreenState < DISPLAY_SCREEN_NOTIFY)
				{
					ScreenStateSave = ScreenState;
				}
				timer_notify_display_start(3000,1,false);
				DISPLAY_MSG  msg = {0,0};

				ScreenState = DISPLAY_SCREEN_NOTIFY_SPORTS_RECORD_DELETE;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}	
		}break;		
	}
}



void gui_sports_record_delete_notify_paint(void)
{
		
	
	
	  SetWord_t word = {0};
	
		//���ñ�����ɫ
		LCD_SetBackgroundColor(LCD_BLACK);
	
		LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 120, 6, 0, 360, LCD_GRAY);
		
	
		
		word.x_axis = 120 - 24/2 ;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_24_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_NONE;
		LCD_SetString("ɾ���ɹ�",&word);
		
		LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 80, 4, 100, 80, LCD_CYAN);
		/*����:120,160*/
		LCD_SetPicture(120 - 6, 200 - 7, LCD_CYAN, LCD_NONE, &img_tool_finish_test);
	 
	


}


void gui_sports_record_delete_notify_btn_evt(uint32_t evt)
{
	
	switch(evt)
	{		
		case (KEY_BACK):
		case (KEY_OK):{
//			timer_notify_display_stop(false);
//			DISPLAY_MSG  msg = {0,0};
//			ScreenState = DISPLAY_SCREEN_SPORTS_RECORD;
//			msg.cmd = MSG_DISPLAY_SCREEN;
//			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
		}break;		
	
	}
}

