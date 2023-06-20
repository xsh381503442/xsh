//Author: Jason Gong
//Time��20180130 V1.00
/*All Swimming Interface and Display are individual functions, except some common variables and 
pre-defined functions/data strings from task_sport.c task_sport.h, task_display.c, task_display.h 
gui_sport.c and com_sport.h. The first stage is all about drawing and displays   V1.00    */

/*All Swimming Data has been added into the system from com_sport.c, it also includes the determination 
of pause status,sport status,and continue afterward. The second stage is about data handling V1.30    */

#include "gui_sport_config.h"
#include "gui_sport.h"
#include "gui_swimming.h"
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
#include "timer_app.h"

#include "algo_hdr.h"
#include "task_gps.h"
#include "gui_run.h"
#include "gui_accessory.h"

#include "gui_tool_gps.h"


	     extern volatile ScreenState_t Save_Detail_Index;
	          extern uint8_t gui_sport_index;
			  
	          extern uint8_t gui_sport_type;
	           /*��Ӿ������ʾ��ǰ��*/
	     uint8_t m_swim_data_display_buf[SPORT_DATA_DISPLAY_SUM]={SPORT_DATA_DISPALY_TOTAL_DISTANCE,SPORT_DATA_DISPALY_SWIM_LAPS,SPORT_DATA_DISPALY_PREVIOUS_SWOLF,SPORT_DATA_DISPALY_TOTAL_TIME};
	           /*��Ӿ������ʾѡ��*/
	      const uint8_t m_swim_data_display_option[SWIM_DATA_DISPLAY_SUM]={SPORT_DATA_DISPALY_TOTAL_DISTANCE,SPORT_DATA_DISPALY_SWIM_LAPS,SPORT_DATA_DISPALY_PREVIOUS_SWOLF,SPORT_DATA_DISPALY_TOTAL_TIME,
	      SPORT_DATA_DISPALY_AVE_SWIM_FREQUENCY,SPORT_DATA_DISPALY_AVE_SWIM_PACE,SPORT_DATA_DISPALY_REALTIME_HEART};
	           
	           /*��Ӿ��������ѡ��*/
	           const uint8_t m_swim_remind_set_option[SWIM_REMIND_SET_SUM]={SPORT_REMIND_SET_HAERT,SPORT_REMIND_SET_SWIMPOOL,SPORT_REMIND_SET_TIME,SPORT_REMIND_SET_DISTANCE,SPORT_REMIND_SET_GOALCIRCLE};
	           
	           
	       #define	GUI_SPORT_SWIM_SCREEN_DATA_1		0	//��Ӿ���ݽ���1(ͬԤ��4������)
	       #define	GUI_SPORT_SWIM_SCREEN_DATA_2		1	//��Ӿ���ݽ���2(��Ϊʵʱ������ɫ���漰���ѽ���)
	      // #define	GUI_SPORT_SWIM_SCREEN_HEART			2	//��Ӿ���ʽ���(��Ϊ�����ӵ��������)
         //  #define	GUI_SPORT_SWIM_SCREEN_TIME			3	//��Ӿʵʱʱ��
           
           
           #define	GUI_SPORT_SWIM_PAUSE_CONTINUE		0	//����
           #define	GUI_SPORT_SWIM_PAUSE_SAVE			1	//�����˶�
           #define	GUI_SPORT_SWIM_PAUSE_CANCEL			2	//����
           
           #define	GUI_SPORT_SWIM_PAUSE_CANCEL_NO		0	//��
           #define	GUI_SPORT_SWIM_PAUSE_CANCEL_YES		1	//��
           
           
           #define	GUI_SPORT_SWIM_DETAIL_1				0	//��ϸ���ݽ���1
           #define	GUI_SPORT_SWIM_DETAIL_2				1	//��ϸ���ݽ���2 
          // #define	GUI_SPORT_SWIM_DETAIL_HEARTRATEZONE	2	//�����������
           
           
           static uint8_t m_gui_sport_swim_index;			//��Ӿ��������
           static uint8_t m_gui_sport_swim_pause_index;	//��Ӿ��ͣ�˵���������
           //static uint16_t m_gui_sport_swim_laps = 0;	
           uint16_t m_gui_sport_swim_laps = 0;
           static uint16_t m_gui_sport_swim_ave_stroke = 0;	
           
		 //  static uint8_t pool_set_flicker = 0;
           
           extern _ble_accessory g_ble_accessory;
           
           //����
           void gui_sport_swim_laps_set(uint16_t laps)
           {
           	m_gui_sport_swim_laps = laps;
           }
           
           //ƽ������
           void gui_sport_swim_ave_stroke_set(uint16_t stroke)
           {
           	m_gui_sport_swim_ave_stroke = stroke;
           }
           
           
           //������������ʾ��
           void gui_sport_swim_data_detail_paint(uint8_t state ,uint8_t m_sport_data_display)
           {
              	switch(m_sport_data_display)
              	{
           		case SPORT_DATA_DISPALY_TOTAL_TIME:
           			gui_sport_data_display_total_time_paint(state,ActivityData.ActTime);
           			break;
           		case SPORT_DATA_DISPALY_TOTAL_DISTANCE:
           			gui_sport_data_display_total_distance_paint(state,ActivityData.ActivityDetails.SwimmingDetail.Distance);
           			break;
           		case SPORT_DATA_DISPALY_AVE_SWIM_PACE:
           			gui_sport_data_display_ave_swim_pace_paint(state,ActivityData.ActivityDetails.SwimmingDetail.Speed);
           			break;
           		case SPORT_DATA_DISPALY_REALTIME_HEART:
           			gui_sport_data_display_realtime_heart_paint(state,Get_Sport_Heart());
           			break;
           		case SPORT_DATA_DISPALY_AVE_HEART:
           			gui_sport_data_display_ave_heart_paint(state,ActivityData.ActivityDetails.SwimmingDetail.AvgHeart);
           			break;
           		case SPORT_DATA_DISPALY_CALORIE:
           			gui_sport_data_display_calorie_paint(state,ActivityData.ActivityDetails.SwimmingDetail.Calorie);
           			break;
           			case SPORT_DATA_DISPALY_SWIM_LAPS:
           			gui_sport_data_display_swim_laps_paint(state,m_gui_sport_swim_laps);
           			break;
           		case SPORT_DATA_DISPALY_TOTAL_STROKES:
           			gui_sport_data_display_total_strokes_paint(state,ActivityData.ActivityDetails.SwimmingDetail.Strokes);
           			break;
           		case SPORT_DATA_DISPALY_AVE_STROKES:
           			gui_sport_data_display_ave_strokes_paint(state,m_gui_sport_swim_ave_stroke);
           			break;
           		case SPORT_DATA_DISPALY_AVE_SWOLF:
           			gui_sport_data_display_ave_swlof_paint(state,ActivityData.ActivityDetails.SwimmingDetail.Swolf/10);
           			break;
           		case SPORT_DATA_DISPALY_AVE_SWIM_FREQUENCY:
           			gui_sport_data_display_ave_swim_frequency_paint(state,ActivityData.ActivityDetails.SwimmingDetail.Frequency);
           			break;
           		default:
           			break;
              	}
            
           }
           
           void gui_sport_swim_data_1_paint(void)
           {
           	SetWord_t word = {0};
           	//���ñ�����ɫ
           	LCD_SetBackgroundColor(LCD_BLACK);
           
           	//Բ������,����һȦʱ����棬��Ҫ����
               gui_sport_index_circle_paint(m_gui_sport_swim_index, 4, 0);
           	
           	//��һ��
           	gui_sport_swim_data_detail_paint(SPORT_DATA_DISPALY_INDEX_UP_CIRCLE,m_swim_data_display_buf[0]);
           	
           	//�ָ���
           	word.x_axis = 80;
           	LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,LCD_AUQAMARIN,0,0,LCD_FILL_ENABLE);
           
           	//�ڶ���
           	gui_sport_swim_data_detail_paint(SPORT_DATA_DISPALY_INDEX_MID_GPS,m_swim_data_display_buf[1]);
           
           	//�ָ���
           	word.x_axis = 160;	
           	LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,LCD_AUQAMARIN,0,0,LCD_FILL_ENABLE);
           
           	//������
           	gui_sport_swim_data_detail_paint(SPORT_DATA_DISPALY_INDEX_DOWN_NOHINT,m_swim_data_display_buf[2]);
           
           }
           //��Ӿ������swlof����
           void gui_sport_swim_data_2_paint(uint32_t strokes,uint16_t swlof)
           {
           	SetWord_t word = {0};
           	char str[10];
           	//���ñ�����ɫ
           	LCD_SetBackgroundColor(LCD_BLACK);
           	//Բ������,����һȦʱ����棬��Ҫ����
               gui_sport_index_circle_paint(m_gui_sport_swim_index, 4, 0);
           	//����
           	word.size = LCD_FONT_16_SIZE;
           	word.bckgrndcolor = LCD_NONE;
           	word.forecolor = LCD_WHITE;
           	word.kerning = 1;
           	word.x_axis = 25;	
           	word.y_axis = LCD_CENTER_JUSTIFIED;
           	LCD_SetString((char *)SportDataDisStrs[SetValue.Language][SPORT_DATA_DISPALY_TOTAL_STROKES],&word);
           	
           	word.x_axis = 54;
           	word.size = LCD_FONT_48_SIZE;
           	memset(str,0,10);
           	sprintf(str,"%d",strokes);
           	LCD_SetNumber(str,&word);
           
           	//�ָ���
           	word.x_axis = 120;
           	LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,LCD_AUQAMARIN,0,0,LCD_FILL_ENABLE);
           	
           	//swlof
           	word.x_axis = 130;
           	memset(str,0,10);
           	sprintf(str,"%d",swlof/10);
           	LCD_SetNumber(str,&word);
           	
           	word.x_axis = 195;
           	word.size = LCD_FONT_16_SIZE;
           	LCD_SetString((char *)SportDataDisStrs[SetValue.Language][SPORT_DATA_DISPALY_AVE_SWOLF],&word);
           
           
           }
           //��Ӿ���ʽ���
         void gui_sport_swim_heart_paint(void)
          {
           	SetWord_t word = {0};
           	//���ñ�����ɫ
           	LCD_SetBackgroundColor(LCD_WHITE);
           	//Բ������
               gui_sport_index_circle_vert_paint(1, 2);
           	
           	//����ֵ
           	gui_sport_heart_1_paint(Get_Sport_Heart(),0);
           	
           	//û�����ʱ��ʾ��
           	if(g_ble_accessory.heartrate.status == ACCESSORY_STATUS_DISCONNECT)
           	{
              
			  LCD_SetRectangle(0,200,0,240,LCD_WHITE,0,0,LCD_FILL_ENABLE);//�����ϲ���
			   
               gui_sport_index_circle_vert_paint(1, 2);
			   
           		word.size = LCD_FONT_16_SIZE;
           		word.bckgrndcolor = LCD_NONE;
           		word.forecolor = LCD_BLACK;
           		word.kerning = 1;
           		word.x_axis = 120 - 16/2;	
           		word.y_axis = LCD_CENTER_JUSTIFIED;
           		LCD_SetString((char *)SportReadyStrs[SetValue.Language][10],&word);

		      
			  LCD_SetPicture(120 - 16/2 - 32 - 28, LCD_CENTER_JUSTIFIED, LCD_RED, LCD_NONE, &Img_heartrate_32X28);
				
           
           	}
           	
          }
           
           //��Ӿʵʱʱ�����
           void gui_sport_swim_realtime_paint(void)
           {
           	//���ñ�����ɫ
           	LCD_SetBackgroundColor(LCD_BLACK);
           	
           	//Բ������
               gui_sport_index_circle_paint(m_gui_sport_swim_index, 4, 0);
           	
           	//ʵʱʱ��
           	gui_sport_realtime_paint(0);
           		
           }
           
           //��Ӿ����
           void gui_sport_swim_paint(void)
           {

		   
		   gui_sport_type = 5;
           	if(m_gui_sport_swim_index ==GUI_SPORT_SWIM_SCREEN_DATA_1 )
           	{
           	 	//gui_sport_swim_data_1_paint();
				
				gui_sport_data_display_preview_paint(SPORT_DATA_SHOW);
           	}
           	else if (m_gui_sport_swim_index ==GUI_SPORT_SWIM_SCREEN_DATA_2)
           	{
           		//gui_sport_swim_data_2_paint(ActivityData.ActivityDetails.SwimmingDetail.Strokes,ActivityData.ActivityDetails.SwimmingDetail.Swolf);
           		//gui_sport_heart_paint(0);
				
           		gui_sport_swim_heart_paint();
           	}
          /* 	else if (m_gui_sport_swim_index ==GUI_SPORT_SWIM_SCREEN_HEART)
           	{
           		gui_sport_swim_heart_paint();
           	}*/
           	else;
           	//{
           		//gui_sport_swim_realtime_paint();
           	//}
           	
           	if(g_sport_status == SPORT_STATUS_PAUSE)
           	{
           		//��ʱ���Ӻ�ɫԲȦ����ʾ��ͣ״̬
           		//LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_RED);
           	}
           
           }
           
           //��ͣ�˵�ѡ��
      static void gui_sport_swim_pause_options(uint8_t index)
           {
           	SetWord_t word = {0};
           	
           	//�˵���һ��
           word.x_axis = 120 + 18;
	       word.y_axis = LCD_CENTER_JUSTIFIED;
	       word.forecolor = LCD_BLACK;
	       word.bckgrndcolor = LCD_NONE;
	       word.kerning = 0;
	       word.size = LCD_FONT_24_SIZE;
           	LCD_SetString((char *)SwimPauseStrs[SetValue.Language][index],&word); 
           	
           	//�˵��ڶ���
           	if(index >= GUI_SPORT_SWIM_PAUSE_CANCEL)
           	{
           		index = GUI_SPORT_SWIM_PAUSE_CONTINUE;
           	}
           	else
           	{
           		index += 1;
           	}
            word.x_axis = 180 + 18;
	        word.forecolor = LCD_BLACK;
	        word.size = LCD_FONT_16_SIZE;
           	LCD_SetString((char *)SwimPauseStrs[SetValue.Language][index],&word);
           }
           
           
           //��Ӿ��ͣ�˵�����
      void gui_sport_swim_pause_paint(void)
           {
			   LCD_SetBackgroundColor(LCD_BLACK);
			   
			   //��ͣ��ʱ
			   gui_sport_pause_time();
			   
			   //�˶���ʱ
			   gui_sport_pause_acttime();
			   
			   LCD_SetRectangle(120 - 1 ,122,0,240,LCD_WHITE,0,0,LCD_FILL_ENABLE);
			   LCD_SetRectangle(120 + 60 - 1,2,0,240,LCD_BLACK,0,0,LCD_FILL_ENABLE);
			   
			
	       		//ѡ��
		  		 gui_sport_swim_pause_options(m_gui_sport_swim_pause_index);
			   
			   
			   //�·���ʾ
			   gui_sport_page_prompt(SPORT_PAGE_DOWN);
			   
			   LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_RED);
           }
           
           //��Ӿ�����˵�����
void gui_sport_swim_cancel_paint(void)
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
			gui_sport_cancel_options(m_gui_sport_swim_pause_index);
			
			
			
				
			//�ָ���
			LCD_SetRectangle(170, 1, 0, LCD_LINE_CNT_MAX, LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
				
			//�·���ʾ
			gui_sport_page_prompt(SPORT_PAGE_DOWN); 


}
           
           //��Ӿ��ϸ���ݽ���
           void gui_sport_swim_detail_paint(void)
           {
           	switch(m_gui_sport_swim_pause_index)
           	{
           		case GUI_SPORT_SWIM_DETAIL_1:
           			gui_swimming_save_detail_1_paint(LCD_WHITE);
           			break;
           		case GUI_SPORT_SWIM_DETAIL_2:
           			gui_swimming_save_detail_2_paint(LCD_WHITE);
           			break;
           		/*case GUI_SPORT_SWIM_DETAIL_HEARTRATEZONE:
           			gui_sport_detail_heartratezone_paint(LCD_BLACK);
           			break;*/
           		default:
           			break;
           	}
           
           }
           void gui_sport_swim_init(void)
           {
           	//Ĭ��Ϊ�ܲ����ݽ���
         	m_gui_sport_swim_index = GUI_SPORT_SWIM_SCREEN_DATA_1;
           	m_gui_sport_swim_laps = 0;
           	m_gui_sport_swim_ave_stroke =0 ;
	
			
           }
           
           
           //��Ӿ���水���¼�
           void gui_sport_swim_btn_evt(uint32_t Key_Value)
           {
           	switch( Key_Value )
           	{
           		case KEY_OK:
           			 if(g_sport_status == SPORT_STATUS_START)
           			{
           				//��ͣ�˶���������ͣ�˵�����
           				g_sport_status = SPORT_STATUS_PAUSE;
           				Set_PauseSta(true);
           				
           				ScreenState = DISPLAY_SCREEN_SWIM_PAUSE;
           				m_gui_sport_swim_pause_index = GUI_SPORT_SWIM_PAUSE_CONTINUE;
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
                        //��ҳ�鿴�ܲ�����
           			if(m_gui_sport_swim_index < GUI_SPORT_SWIM_SCREEN_DATA_2)
           			{
           				m_gui_sport_swim_index++;
           			}
           			else
           			{
           				m_gui_sport_swim_index = GUI_SPORT_SWIM_SCREEN_DATA_1;
           			}
           				
           			gui_swich_msg();
           
           			break;
           		case KEY_UP:
                      	//��ҳ�鿴�ܲ�����
           			if(m_gui_sport_swim_index > GUI_SPORT_SWIM_SCREEN_DATA_1)
           			{
           				m_gui_sport_swim_index--;
           			}
           			else
           			{
           				m_gui_sport_swim_index = GUI_SPORT_SWIM_SCREEN_DATA_2;
           			}
           				
           			gui_swich_msg();
           		    break;
           		case KEY_BACK:
            			if(g_sport_status == SPORT_STATUS_PAUSE)
           			{
           				//������ͣ�˵�����
           				ScreenState = DISPLAY_SCREEN_SWIM_PAUSE;
           				m_gui_sport_swim_index = GUI_SPORT_SWIM_PAUSE_CONTINUE;
           			
           				gui_swich_msg();
           			
           			}
           			break;
           		default:
           			break;
           	}
			gui_sport_index = m_gui_sport_swim_index;
			gui_sport_type = 5;
           }
           
           //��Ӿ��ͣ�˵����水���¼�
           void gui_sport_swim_pause_btn_evt(uint32_t Key_Value)
           {
           	DISPLAY_MSG  msg = {0};
           	
           	switch( Key_Value )
           	{
           		case KEY_OK:
           			if(m_gui_sport_swim_pause_index == GUI_SPORT_SWIM_PAUSE_CONTINUE)
           			{
           				//������ʱ����ʼ�˶�
           				g_sport_status = SPORT_STATUS_START;
           				Set_PauseSta(false);
           				
           				ScreenState = DISPLAY_SCREEN_SWIMMING;
           				gui_swich_msg();
           				Save_Detail_Index = ScreenState;
           				timer_app_pasue_start();
           				//����ʾ
           				gui_sport_motor_prompt();
           			}
           			else if(m_gui_sport_swim_pause_index == GUI_SPORT_SWIM_PAUSE_SAVE)
           			{
           				//�����˶�����������
           				g_sport_status = SPORT_STATUS_READY;
           				
           				//Ĭ��ѡ��
           				gui_sport_feel_init();
           				
           				ScreenState = DISPLAY_SCREEN_SPORT_FEEL;
           				m_gui_sport_swim_pause_index = GUI_SPORT_SWIM_DETAIL_1;
           				
           				extern volatile ScreenState_t Save_Detail_Index;
           				Save_Detail_Index = DISPLAY_SCREEN_SWIM_DETAIL;
           				gui_swich_msg();
           			}
           			else
           			{
           				//�����˶�
           				ScreenState = DISPLAY_SCREEN_SWIM_CANCEL;
           				
           				m_gui_sport_swim_pause_index = GUI_SPORT_SWIM_PAUSE_CANCEL_NO;
           				gui_swich_msg();
           			}
           			
           			//msg.cmd = MSG_DISPLAY_SCREEN;
           			//xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
           			break;
           		case KEY_UP:
           			if(m_gui_sport_swim_pause_index > GUI_SPORT_SWIM_PAUSE_CONTINUE)
           			{
           				m_gui_sport_swim_pause_index--;
           			
           			}
           			else
           			{
           				m_gui_sport_swim_pause_index = GUI_SPORT_SWIM_PAUSE_CANCEL;
           			}
           			
           			msg.cmd = MSG_DISPLAY_SCREEN;
           			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
           		    break;
           		case KEY_DOWN:
           			if(m_gui_sport_swim_pause_index < GUI_SPORT_SWIM_PAUSE_CANCEL)
           			{
           				m_gui_sport_swim_pause_index++;
           		
           			}
           			else
           			{
           				m_gui_sport_swim_pause_index = GUI_SPORT_SWIM_PAUSE_CONTINUE;
           			}
           			
           			msg.cmd = MSG_DISPLAY_SCREEN;
           			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
           			break;
           		case KEY_BACK:
           			//������Ӿ����
           			ScreenState = DISPLAY_SCREEN_SWIMMING;
           		
           			msg.cmd = MSG_DISPLAY_SCREEN;
           			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
           			break;
           		default:
           			break;
           	}
			
           }
           
           //�����˵����水���¼�
           void gui_sport_swim_cancel_btn_evt(uint32_t Key_Value)
           {
           	DISPLAY_MSG  msg = {0};
           	
           	switch( Key_Value )
           	{
           		case KEY_OK:
           			if(m_gui_sport_swim_pause_index == GUI_SPORT_SWIM_PAUSE_CANCEL_NO)
           			{
           				//ȡ�������˶��������ϼ��˵�����
           				ScreenState = DISPLAY_SCREEN_SWIM_PAUSE;
           				m_gui_sport_swim_pause_index = GUI_SPORT_SWIM_PAUSE_CANCEL;
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
           			if(m_gui_sport_swim_pause_index == GUI_SPORT_SWIM_PAUSE_CANCEL_NO)
           			{
           				m_gui_sport_swim_pause_index = GUI_SPORT_SWIM_PAUSE_CANCEL_YES;
           			}
           			else
           			{
           				m_gui_sport_swim_pause_index = GUI_SPORT_SWIM_PAUSE_CANCEL_NO;
           			}
           
           			msg.cmd = MSG_DISPLAY_SCREEN;
           			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
           		    break;
           		case KEY_BACK:
           			//�����ϼ��˵�
           			ScreenState = DISPLAY_SCREEN_SWIM_PAUSE;
           			m_gui_sport_swim_pause_index = GUI_SPORT_SWIM_PAUSE_CANCEL;
           			
           			msg.cmd = MSG_DISPLAY_SCREEN;
           			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
           			break;
           		default:
           			break;
           	}
           }
           
           //��ϸ���ݽ��水���¼�
           void gui_sport_swim_detail_btn_evt(uint32_t Key_Value)
           {
           	DISPLAY_MSG  msg = {0};
           	
           	switch( Key_Value )
           	{
           		case KEY_UP:
           			//�Ϸ�
           			if(m_gui_sport_swim_pause_index > GUI_SPORT_SWIM_DETAIL_1)
           			{
           				m_gui_sport_swim_pause_index--;
           				msg.cmd = MSG_DISPLAY_SCREEN;
           				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
           			}
           		    break;
           		case KEY_DOWN:
           			//�·�
           			if(m_gui_sport_swim_pause_index < GUI_SPORT_SWIM_DETAIL_2)
           			{
           				m_gui_sport_swim_pause_index++;
           
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
           
           



//��Ӿ�˶���ϸ_����1 Swimming Saving Details 1st Page
void gui_swimming_save_detail_1_paint(uint8_t backgroundcolor)
{
   SetWord_t word = {0};
   char str[20];
   char *colon = ":";
   uint8_t charlen;
   
   uint32_t second;
   
   //����ʱ�� =��ʱ��-��ͣʱ��
   second = ActivityData.ActTime;//get_time_diff(ActivityData.Act_Start_Time, ActivityData.Act_Stop_Time)-  Get_PauseTotolTime();


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
	sprintf(str,"%d.%02d",ActivityData.ActivityDetails.SwimmingDetail.Distance/100000,ActivityData.ActivityDetails.SwimmingDetail.Distance/1000%100);
	//#else
  //  sprintf(str,"%0.2f",ActivityData.ActivityDetails.SwimmingDetail.Distance/100000.f);
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
    sprintf(str,"%d",ActivityData.ActivityDetails.SwimmingDetail.Calorie/1000);
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
	 sprintf(str,"%d",ActivityData.ActivityDetails.SwimmingDetail.Strokes);
	 word.x_axis = 120 + 4 + 12 + 16 + 4;
	 word.y_axis = 130;
	 word.kerning = 1;
	 word.size = LCD_FONT_19_SIZE;
	 LCD_SetNumber(str,&word);
	// word.y_axis += strlen(str)*10	+ 8;
	// word.kerning = 0;

	 
	// word.x_axis =	120 + 4 + 12 + 16 + 4 + 19 - 16;
	// word.size = LCD_FONT_16_SIZE;
	// LCD_SetString("bpm",&word);

    word.x_axis = 163+36;//������
	LCD_SetRectangle(word.x_axis,4+8+4,0,240,LCD_MAROON,0,0,LCD_FILL_ENABLE);
   
   LCD_SetPicture(word.x_axis+4, LCD_CENTER_JUSTIFIED, LCD_WHITE, LCD_NONE, &Img_Pointing_Down_12X8);
    
 
}
//��Ӿ�˶���ϸ_����2 Swimming Saving Details 2nd Page
void gui_swimming_save_detail_2_paint(uint8_t backgroundcolor)
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
  
    LCD_SetPicture(10, LCD_CENTER_JUSTIFIED, LCD_BLACK, LCD_NONE, &Img_Pointing_Up_12X8);
    
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
    sprintf(str,"%d:%02d",(ActivityData.ActivityDetails.SwimmingDetail.Speed/60),(ActivityData.ActivityDetails.SwimmingDetail.Speed%60));
	
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



    //(ƽ��)��ˮ���� ("��Ӿ����"����)
    charlen = strlen(sportdetailstrsgod[SetValue.Language][25]);
    word.x_axis = 120 - 4 - 16;
    word.y_axis = 60;
	
    word.size = LCD_FONT_16_SIZE;
    LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][25],&word);
    
    word.y_axis += charlen*8+1;
    LCD_SetString(colon,&word);
	
    memset(str,0,20);
   // sprintf(str,"%d",ActivityData.ActivityDetails.SwimmingDetail.Strokes);// 
    
	sprintf(str,"%d",(uint16_t)(ActivityData.ActivityDetails.SwimmingDetail.Strokes*60/ActivityData.ActTime));
    
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
    sprintf(str,"%d",(uint16_t)(ActivityData.ActivityDetails.SwimmingDetail.Swolf/10));
	
    word.x_axis = 120 + 19 + 32 + 16 + 4;
    word.y_axis = 60;
    word.kerning = 1;
	
    word.size = LCD_FONT_19_SIZE;
    LCD_SetNumber(str,&word);
   



  //  LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Down_12X8);
    
 
}


