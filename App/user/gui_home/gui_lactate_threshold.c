/*
*�������ܽ���:������ʱ�����ʺ����� ��
*����������ʱ�����ʺ����ٽ����ܲ���������Ч�������������
*�ڽ׶��Ե��˶�ѵ���У�������ʱ�����ʺ����ټ�С����ʾ�˶�������ǿ��
*/
#include "gui_lactate_threshold.h"
#include "gui_home_config.h"
#include "gui_home.h"
#include "gui_run.h"
#include "gui_sport.h"
#include "gui.h"


#include "task_gps.h"
#include "com_sport.h"

extern void set_gps_hint_motor_flag(bool flag);
/*������ʾ:�����н������ǰ��ʾ����*/
static void gui_clactate_threshold_value_title_paint(uint8_t hdr_value,uint32_t pace)
{
	SetWord_t word = {0};
	char str[10];
	
	word.x_axis = 24;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	LCD_SetString("������ֵ",&word);
	//if(SetValue.IsMeasuredLthreshold == 0 && pace == 0 && hdr_value == 0)
		
	if(SetValue.IsMeasuredLthreshold == 0)
	{//�����лָ�������һ��
		word.x_axis = LCD_LINE_CNT_MAX/2 - img_tools_big_lists[1].height/2;
		word.y_axis = LCD_LINE_CNT_MAX/2 - img_tools_big_lists[1].width/2;
		word.forecolor = LCD_RED;
		word.bckgrndcolor = LCD_NONE;
		LCD_SetPicture(word.x_axis,word.y_axis ,word.forecolor,word.bckgrndcolor,&img_tools_big_lists[1]);
	}
	//else if(SetValue.IsMeasuredLthreshold == 1 && (pace == 0 && hdr_value == 0))
		
	else if(SetValue.IsMeasuredLthreshold == 1)
	{
		word.x_axis = LCD_LINE_CNT_MAX/2 - Font_Number_24.height/2;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_24_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_NONE;
		word.kerning = 1;
		LCD_SetString("����Ч����",&word);
		
		
	}
	else
	{

	   /*�м�ָ���*/
	   LCD_SetRectangle(LCD_LINE_CNT_MAX/2 -1,2,LCD_LINE_CNT_MAX/2 - 100,200, LCD_CYAN, 0, 1, LCD_FILL_ENABLE);
	   
	   /*����ֵ*/
		memset(str,0,10);
		sprintf(str,"%d",hdr_value);
		word.x_axis = LCD_LINE_CNT_MAX/2 -1 - 12 - Font_Number_56.height;
		//word.y_axis = (LCD_LINE_CNT_MAX -  strlen(str)*(Font_Number_56.width + word.kerning) + word.kerning)/2;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_56_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_NONE;
		word.kerning = 1;
		LCD_SetNumber(str,&word);
		
		/*bpm*/
		word.x_axis = LCD_LINE_CNT_MAX/2 - 1 - 12 - 16;
		word.y_axis = (LCD_LINE_CNT_MAX +  strlen(str)*(Font_Number_56.width + word.kerning) - word.kerning)/2 + 16;
		word.size = LCD_FONT_16_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_NONE;
		word.kerning = 1;
		LCD_SetString("bpm",&word);
	//	LCD_SetPicture(word.x_axis, word.y_axis, LCD_RED, LCD_NONE, &Img_heartrate_32X28);
		
		
	/*������ֵ*/
		memset(str,0,10);
		sprintf(str,"%d:%02d",pace/60,pace%60);
		word.x_axis = LCD_LINE_CNT_MAX/2 + 1 + 12;
		//word.y_axis = (LCD_LINE_CNT_MAX - (strlen(str)-1)*Font_Number_56.width - 12)/2;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_56_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_NONE;
		word.kerning = 1;
		LCD_SetNumber(str,&word);

		
		/*km*/
		word.x_axis = LCD_LINE_CNT_MAX/2 + 1 + 12 + Font_Number_56.height - Font_Number_16.height;
		word.y_axis = LCD_LINE_CNT_MAX/2 + (3*Font_Number_56.width + 0.5*Font_Number_56.width)/2 + 16;
		word.size = LCD_FONT_16_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_NONE;
		word.kerning = 1;
		LCD_SetString("/km",&word);
		
	
	}
	    word.x_axis = 120 + 13 + Font_Number_56.height + 8;//197
	    word.y_axis = LCD_CENTER_JUSTIFIED;
  	    LCD_SetRectangle(word.x_axis,22,0,LCD_LINE_CNT_MAX,LCD_GRAY,1,1,LCD_FILL_ENABLE);
	    word.x_axis += 3;
	    word.size = LCD_FONT_16_SIZE;
	    word.forecolor = LCD_BLACK;
	    word.bckgrndcolor = LCD_NONE;
	    word.kerning = 1;
	    LCD_SetString("OK���������",&word);//"OK���������"
		
}
/*������ʾ:�����в���ǰ����������ʾ����*/
static void gui_lactate_threshold_hint_content_paint(void)
{
	SetWord_t word = {0};
	
	LCD_SetPicture(58,205,LCD_RED,LCD_NONE,&step_cal_arrow_youshang);//���ϽǼ�ͷ
	word.x_axis = 40;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	LCD_SetString("�����в���",&word);
	word.x_axis = LCD_LINE_CNT_MAX/2 - 4 - Font_Number_24.height;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_CYAN;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	LCD_SetString("�����",&word);
	
	word.x_axis += Font_Number_24.height + 8;
	LCD_SetString("ȫ����30����",&word);
	
	word.x_axis = 190;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_GRAY;
	word.bckgrndcolor = LCD_NONE;
	LCD_SetRectangle(word.x_axis,22,0,240,word.forecolor,0,0,LCD_FILL_ENABLE);
	
	word.x_axis = 190 + 3;
	word.size = LCD_FONT_16_SIZE;
	
	word.forecolor = LCD_BLACK;
	word.kerning = 1;
	LCD_SetString("OK����ʼ",&word);
	
}
/*������ʾ:�����н������ǰ��ʾ����*/
void gui_lactate_threshold_value_paint(uint8_t hdr_value,uint32_t pace)
{
	//���ñ�����ɫ
	LCD_SetBackgroundColor(LCD_BLACK);
	gui_clactate_threshold_value_title_paint(hdr_value,pace);//����
}
/*������ʾ:�����в���ǰ������ʾ����*/
void gui_lactate_threshold_hint_paint(void)
{
	//���ñ�����ɫ
	LCD_SetBackgroundColor(LCD_BLACK);
	gui_sport_gps_status(SPORT_GPS_LEFT1);  
	gui_lactate_threshold_hint_content_paint();//��ʾ����
}
/*������ʾ:�����в��Խ���*/
void gui_lactate_mesuring_paint(float mile,Countdown_time_t countdown_time,uint8_t hdrValue,float min_km)
{
	SetWord_t word = {0};
  char str[5];
	//���ñ�����ɫ
	LCD_SetBackgroundColor(LCD_BLACK);
	//����
	
	word.x_axis = 120 - 1;
	
	LCD_SetRectangle(word.x_axis,2,LCD_LINE_CNT_MAX/2 - 100 ,200, LCD_CYAN, 0, 0, LCD_FILL_ENABLE);


	
	gui_sport_gps_status(SPORT_GPS_UP);


	gui_lactate_distance_paint(SPORT_DATA_DISPALY_INDEX_NEW_LACTATE,mile);//������

	gui_lactate_time_paint(SPORT_DATA_DISPALY_INDEX_NEW_LACTATE,(countdown_time.hour *3600 + countdown_time.minute *60 + countdown_time.second));

	gui_lactate_pace_paint(SPORT_DATA_DISPALY_INDEX_NEW_LACTATE,min_km);

	
	gui_lactate_bpm_paint(SPORT_DATA_DISPALY_INDEX_NEW_LACTATE,hdrValue);

}

/*��������:�����н������ǰ��ʾ����*/
void gui_lactate_threshold_value_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	switch( Key_Value )
	{
		case KEY_OK:
			if(IS_SPORT_CONTINUE_LATER==1)
			{//������Ժ����,���������ſ��Խ���
				ScreenState = DISPLAY_SCREEN_END_UNFINISH_SPORT;
				msg.cmd = MSG_DISPLAY_SCREEN;
				msg.value = DISTINGUISH_LACTATE;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
			else
			{
				total_hdr_value = 0;
				total_hdr_count = 0;
				set_gps_hint_motor_flag(false);
				ScreenState = DISPLAY_SCREEN_LACTATE_THRESHOLD_HINT;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				CreateGPSTask();
			}
			break;
#if  defined WATCH_COM_SHORTCUNT_UP
		case KEY_UP:
		case KEY_DOWN:
			//set_switch_screen_count_evt(Key_Value);
		
			//gui_tool_switch(Key_Value);
			break;
#else	
		case KEY_UP:
			set_home_index_up();
			ScreenState = Get_Curr_Home_Index();
			msg.cmd = MSG_DISPLAY_SCREEN;
//			msg.value =Get_Curr_Home_Index() ;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		break;
		case KEY_DOWN:	
			set_home_index_down();
			ScreenState = Get_Curr_Home_Index();
			msg.cmd = MSG_DISPLAY_SCREEN;
//			msg.value =Get_Curr_Home_Index() ;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		break;
#endif
		case KEY_BACK:
			ScreenState = DISPLAY_SCREEN_TOOL;
			//���ش�������
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}
/*��������:�����в���ǰ������ʾ����*/
void gui_lactate_threshold_hint_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	switch( Key_Value )
	{
		case KEY_OK:
			timer_app_countdown_start();
			ScreenState = DISPLAY_SCREEN_CUTDOWN;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		case KEY_BACK:
			CloseGPSTask();
			ScreenState = DISPLAY_SCREEN_LACTATE_THRESHOLD;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}
