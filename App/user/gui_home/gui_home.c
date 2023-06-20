/*
*�������ܽ���:�������棺������ / ����״̬ / ���� / ���� / �������ٷֱȣ���
*/
#include "gui_home_config.h"
#include "gui_home.h"
#include "gui.h"
#include "gui_tool.h"
#include "gui_sport.h"
#include "gui_gps_test.h"
#include "gui_sports_record.h"
#include "gui_motionsrecord.h"
#include "gui_menu.h"
#include "gui_heartrate.h"

#include "bsp_timer.h"

#include "drv_lcd.h"
#include "drv_key.h"
#include "drv_battery.h"
#include "drv_gt24l24.h"
#include "drv_motor.h"
#include "drv_lsm6dsl.h"

#include "task_display.h"
#include "task_key.h"
#include "task_hrt.h"
#include "task_tool.h"
#include "task_hardware.h"
#include "task_ble.h"
#include "task_gps.h"
#include "algo_time_calibration.h"

#include "mode_power_off.h"

#include "com_data.h"
#include "com_sport.h"
#include "font_config.h"
#include "timer_app.h"
#include "time_notify.h"
#include "Ohrdriver.h"




#if DEBUG_ENABLED == 1 && GUI_HOME_LOG_ENABLED == 1
	#define GUI_HOME_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_HOME_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define GUI_HOME_WRITESTRING(...)
	#define GUI_HOME_PRINTF(...)		        
#endif

	
	#define NEED_SWITCH_SCREEN_INDEX			(6)
	
	//�໥�л�����
	const ScreenState_t m_switch_monitor_screen[NEED_SWITCH_SCREEN_INDEX] = 
	{
		DISPLAY_SCREEN_STEP,
		DISPLAY_SCREEN_CALORIES,
		DISPLAY_SCREEN_WEEK_AVERAGE_HDR,
		DISPLAY_SCREEN_MOTIONS_RECORD,
		DISPLAY_SCREEN_NOTIFICATION,
		DISPLAY_SCREEN_CALENDAR
	};
	
	//����ɾ������(���ϸ������еı��)
	const uint8_t m_no_delete_screen[3] = 
	{
		0,
		1,
		4,
	};
	
	
static uint32_t Custom_Home = 0x3FF; //�����Զ������ѡ�񣬺����Ų�������

static uint8_t Home_Nums;		//��ǰ��������(������)
static uint8_t Home_Nums_delete;//��ǰ��ɾ���Ľ�����(������)

static uint8_t Curr_Home_Num;	//��ǰ�������(������)
static ScreenState_t Select_Home_Index[NEED_SWITCH_SCREEN_INDEX];	//��ǰʹ�õĽ���


static uint8_t m_key_add_sub_flag = 0;

extern _drv_bat_event   g_bat_evt;
extern DialDataStr DialData;
extern ScreenState_t Select_Sport_Index[];//��ǰʹ�õĽ�������
extern uint8_t PostIndex;
 SWITCH_HOME_SCREEN_T switch_home_screen = BACK_HOME_SCREEN;//0back���أ�1��⹦���л�
HOME_OR_MINOR_SCREEN_T home_or_minor_switch = HOME_SCREEN_SWITCH;
static uint8_t POWER_ON = 1;//

/*˵��:��Ҫ�л��Ľ��������������*/
ScreenState_t s_need_switch_screen[NEED_SWITCH_SCREEN_INDEX];/*��Ҫ�л��Ľ����б�*/
ScreenState_t s_remain_sub_screen[NEED_SWITCH_SCREEN_INDEX];/*��Ҫ�Ƴ��Ľ����б�*/
uint16_t s_switch_screen_count = 0;
ScreenState_t back_monitor_screen = DISPLAY_SCREEN_STEP;

/*����˵��:��ȡ��ǰ�л�����Ľ���������
���:�л��ĵڼ���
*/
ScreenState_t get_current_switch_screen(uint32_t index)
{
uint8_t i;
 ScreenState_t switch_screen_s;
 if( home_or_minor_switch==HOME_SCREEN_SWITCH)
 
 	{
 		if(POWER_ON==1)
 			{
 			 POWER_ON=0;
			   switch_screen_s = m_switch_monitor_screen[0];
 			}
		else
			{
	         switch_screen_s = back_monitor_screen;
			}
	  /*2��ѭ�������õ�ֻ��һ��*/
	   for(i=0;i<Home_Nums;i++)
	 	{
	     if(switch_screen_s ==s_need_switch_screen[i])
	     	{
			  s_switch_screen_count = i;
	          break;

		  }
	       
	    }
  
	   for(i=0;i<Home_Nums_delete;i++)
	 	{
	     if(switch_screen_s ==s_remain_sub_screen[i])
	     	{
	          switch_screen_s = s_need_switch_screen[0];
			  s_switch_screen_count = 0;
	          break;

		  }
	       
	    }
 	}
 else
 	{

     
	switch_screen_s = s_need_switch_screen[index];

    }

    return switch_screen_s;
}


void set_select_monitor_index(void)
{
    uint8_t i;
    uint8_t j = 0, k = 0;
    
    memset(s_need_switch_screen,0,sizeof(s_need_switch_screen));
	memset(s_remain_sub_screen,0,sizeof(s_remain_sub_screen));

		for (i=0; i<NEED_SWITCH_SCREEN_INDEX; i++)
		{
			if (is_custom_monitor(m_switch_monitor_screen[i]) == 1) 
			{
			//����ӵĽ���
			s_need_switch_screen[j] = (ScreenState_t)m_switch_monitor_screen[i];
			j++;
		     }
	         else
		    {
			//���Ƴ��Ľ���
			s_remain_sub_screen[k] = (ScreenState_t)m_switch_monitor_screen[i];
			k++;
		     }
		}
	  Home_Nums = j; 
      Home_Nums_delete = k;
	
}
uint8_t is_custom_monitor(ScreenState_t index)
{
	bool ret = 0;
	switch(index)
	{

	    case DISPLAY_SCREEN_STEP:				
			if (((SetValue.U_CustomMonitor.val) & 0x01) == 0x01) 
			{
				ret =1;
			}
			break;
		case DISPLAY_SCREEN_CALORIES:				
			if (((SetValue.U_CustomMonitor.val>>1) & 0x01) == 0x01) 
			{
				ret =1;
			}
			break;
		case DISPLAY_SCREEN_WEEK_AVERAGE_HDR:		
			if (((SetValue.U_CustomMonitor.val>>2) & 0x01) == 0x01) 
			{
				ret =1;
			}
			break;
		case DISPLAY_SCREEN_MOTIONS_RECORD:	
			if (((SetValue.U_CustomMonitor.val>>3) & 0x01) == 0x01) 
			{
				ret =1;
			}
			break;
		case DISPLAY_SCREEN_NOTIFICATION:			
			if (((SetValue.U_CustomMonitor.val>>4) & 0x01) == 0x01) 
			{
				ret =1;
			}
			break;
		case DISPLAY_SCREEN_CALENDAR:		
			if (((SetValue.U_CustomMonitor.val>>5) & 0x01) == 0x01) 
			{
				ret =1;
			}
			break;
	
		 default:
			break;
	}
	
	return ret;

}


/*�����¼��л�����*/
void set_switch_screen_count_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	uint16_t m_total_count = Home_Nums;
	//for(i = 0; i < m_total_count;i++)
	//{
		if(Key_Value == KEY_UP)
		{
			if(s_switch_screen_count > 0)
			{
				s_switch_screen_count --;
			}
			else
			{
				s_switch_screen_count = m_total_count-1;
			}
		}
		else if(Key_Value == KEY_DOWN)
		{
			if(s_switch_screen_count < m_total_count-1)
			{
				s_switch_screen_count ++;
			}
			else
			{
				s_switch_screen_count = 0;
			}	
		}
		   ScreenState = get_current_monitor_screen();
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
	//}
	/*	if(((SetValue.custom_shortcut_up_switch_val >> (s_switch_screen_count)) & 0x01) == 1)
		{
			if(i == Home_Nums)
			s_switch_screen_count = 0;
			}
			ScreenState = get_current_monitor_screen();
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		return;
	     }
		else
		{
			GUI_HOME_PRINTF("[GUI_HOME]:----s_switch_screen_count=%d------val=%d,value=%d\n",s_switch_screen_count
					,((SetValue.custom_shortcut_up_switch_val >> (s_switch_screen_count)) & 0x01),SetValue.custom_shortcut_up_switch_val);
		}*/
	

 }

/*��ȡ�л��Ĵ���*/
uint16_t get_switch_screen_count(void)
{

     
	return s_switch_screen_count;
}


ScreenState_t get_current_monitor_screen(void)
{
	ScreenState_t m_screen = get_current_switch_screen(get_switch_screen_count());
	//GUI_HOME_PRINTF("m_screen:%d\r\n",m_screen);
	if(m_screen == DISPLAY_SCREEN_MOTIONS_RECORD)
	{
//		Com_Flash_Common_Sport();
#if defined(WATCH_SPORT_RECORD_TEST)
		//��ѯ�����˶���¼ ����� 
		if(search_sport_record() != 0)
		{
			data_up_down_flag = 0;
			GetSportDetail(0,1);
		}
#else
		Com_Flash_Common_Sport();
		g_motion_record_addr = ActivityAddressForwardLookup(DAY_MAIN_DATA_START_ADDRESS);//��ȡ���»���ݵ�ַ
		sport_data_addr[0] = g_motion_record_addr;
		sport_data_addr[1] = 0;
		GetSportDetail(sport_data_addr[0],sport_data_addr[1]);
#endif
	}
	else if(m_screen == DISPLAY_SCREEN_TRAINPLAN_MARATHON)
	{
		if(get_trainplan_valid_flag())
		{//��ѵ���ƻ�ͬ��
			Get_TrainPlanDayRecord();
		}
	}
	else if(m_screen == DISPLAY_SCREEN_LACTATE_THRESHOLD || m_screen == DISPLAY_SCREEN_VO2MAX)
	{//�������������������  �����㷨��ʼ��
		//�㷨��ʼ��
//		PhysicalParamCalculateInit();
	}
	else if(m_screen == DISPLAY_SCREEN_WEEK_AVERAGE_HDR)
	{
		//���ʽ���
		gui_heartrate_data_init();
		if(gui_heartrate_statu_get() != 1)
		{
			//����ʷ�����������ݣ�����2S��ʱ��
			timer_app_tool_start();
			m_screen =  DISPLAY_SCREEN_HEARTRATE;
		}
	}
	
	return m_screen;
}


ScreenState_t get_current_longtool_screen(void)

{
	ScreenState_t m_screen = Get_Curr_Tool_Index();
	GUI_HOME_PRINTF("Curr_Tool_Num:%d\r\n",m_screen);
	if(m_screen == DISPLAY_SCREEN_MOTIONS_RECORD)
	{
//		Com_Flash_Common_Sport();
#if defined(WATCH_SPORT_RECORD_TEST)
		//��ѯ�����˶���¼ ����� 
		if(search_sport_record() != 0)
		{
			data_up_down_flag = 0;
			GetSportDetail(0,1);
		}
#else
		Com_Flash_Common_Sport();
		g_motion_record_addr = ActivityAddressForwardLookup(DAY_MAIN_DATA_START_ADDRESS);//��ȡ���»���ݵ�ַ
		sport_data_addr[0] = g_motion_record_addr;
		sport_data_addr[1] = 0;
		GetSportDetail(sport_data_addr[0],sport_data_addr[1]);
#endif
	}
	else if(m_screen == DISPLAY_SCREEN_TRAINPLAN_MARATHON)
	{
		if(get_trainplan_valid_flag())
		{//��ѵ���ƻ�ͬ��
			Get_TrainPlanDayRecord();
		}
	}
	else if(m_screen == DISPLAY_SCREEN_LACTATE_THRESHOLD || m_screen == DISPLAY_SCREEN_VO2MAX)
	{//�������������������  �����㷨��ʼ��
		//�㷨��ʼ��
//		PhysicalParamCalculateInit();
	}
#if defined WATCH_COM_SHORTCUNT_UP
	else if(m_screen == DISPLAY_SCREEN_WEEK_AVERAGE_HDR)
	{
		//���ʽ���
		gui_heartrate_data_init();
		if(gui_heartrate_statu_get() != 1)
		{
			//����ʷ�����������ݣ�����2S��ʱ��
			timer_app_tool_start();
			m_screen =  DISPLAY_SCREEN_HEARTRATE;
		}
	}
	
#endif
	
	return m_screen;
}



/*ѵ���ƻ�Log��ӡ*/
static void Gui_Home_TrainPlan_Log(void)
{
	GUI_HOME_PRINTF("--------------------Gui_Home_TrainPlan_Log------------------\n");
	GUI_HOME_PRINTF("TrainPlan-->>RTC:year=%d,month=%d,day=%d,\n Hint:Hour=%d,minute=%d,Get_SomeDayofTrainPlan=%d\n"
									,RTC_time.ui32Year,RTC_time.ui32Month,RTC_time.ui32DayOfMonth,TrainPlanDownloadSettingStr.Hint.Hour,TrainPlanDownloadSettingStr.Hint.Minute,Get_SomeDayofTrainPlan());
	GUI_HOME_PRINTF("TrainPlan-->>SetValue.TrainPlanFlag=%d,SetValue.TrainPlanHintFlag=%d,Get_TrainPlanFlag()=%d,Get_TrainPlanBeforeDays()=%d,\r\n start:year=%d,month=%d,day=%d\n"
	                ,SetValue.TrainPlanFlag,SetValue.TrainPlanHintFlag,Get_TrainPlanFlag(),Get_TrainPlanBeforeDays(),TrainPlanDownloadSettingStr.StartTime.Year
	                ,TrainPlanDownloadSettingStr.StartTime.Month,TrainPlanDownloadSettingStr.StartTime.Day);
	GUI_HOME_PRINTF("TrainPlan-->>stop:year=%d,month=%d,day=%d\n"
									,TrainPlanDownloadSettingStr.StopTime.Year,TrainPlanDownloadSettingStr.StopTime.Month
									,TrainPlanDownloadSettingStr.StopTime.Day);
	GUI_HOME_PRINTF("TrainPlan-->>Download[0].type=%d,Download[1].type=%d,Download[2].type=%d,get_trainplan_valid_flag()=%d\n"
	                ,Download[0].Type,Download[1].Type,Download[2].Type,get_trainplan_valid_flag());
	
	GUI_HOME_PRINTF("--------------------End------------------\n");
}
ScreenState_t Get_Curr_Home_Index(void)
{
	uint8_t m_curr_home_num = 0;
	
	for(int i = 0;i<Home_Nums;i++)
	{
		if(Select_Home_Index[Curr_Home_Num] == 0)
		{
			if(m_key_add_sub_flag == 1)
			{//add
				if(Curr_Home_Num >= Home_Nums)
				{
					Curr_Home_Num = 0;
				}
				else
				{
					Curr_Home_Num++;
				}
			}
			else if(m_key_add_sub_flag == 2)
			{//sub
				if(Curr_Home_Num == 0)
				{
					Curr_Home_Num = Home_Nums-1;
				}
				else
				{
					Curr_Home_Num--;
				}
			}

		}
		else
		{
			if(Curr_Home_Num >= Home_Nums)
			{
				Curr_Home_Num = 0;
			}
			m_curr_home_num = Curr_Home_Num;
			
			break;
		}
	}
	if(m_curr_home_num == ((uint8_t)DISPLAY_SCREEN_MOTIONS_RECORD -1))//�˶���¼����
	{
#if defined(WATCH_SPORT_RECORD_TEST)
		//��ѯ�����˶���¼ ����� 
		if(search_sport_record() != 0)
		{
			data_up_down_flag = 0;
			GetSportDetail(0,1);
		}
#else
		Com_Flash_Common_Sport();
		g_motion_record_addr = ActivityAddressForwardLookup(DAY_MAIN_DATA_START_ADDRESS);//��ȡ���»���ݵ�ַ
		sport_data_addr[0] = g_motion_record_addr;
		sport_data_addr[1] = 0;
		GetSportDetail(sport_data_addr[0],sport_data_addr[1]);
#endif
	}
	else if(m_curr_home_num == ((uint8_t)DISPLAY_SCREEN_TRAINPLAN_MARATHON -1))//ѵ���ƻ�����
	{
//		Gui_Home_TrainPlan_Log();
		if(get_trainplan_valid_flag())
		{//��ѵ���ƻ�ͬ��
			Get_TrainPlanDayRecord();
			Gui_Home_TrainPlan_Log();
//			Get_TranPlanUploadData(Get_SomeDayofTrainPlan()-1);//��ȡǰһ���ѵ�����
		}
		else
		{//��ѵ���ƻ�ͬ��
			
		}
	}

#if defined WATCH_COM_SHORTCUNT_UP
	if(m_curr_home_num == ((uint8_t)DISPLAY_SCREEN_WEEK_AVERAGE_HDR -1))
	{
		//���ʽ���
		gui_heartrate_data_init();
		if(gui_heartrate_statu_get())
		{
			//����ʷ������������
			return Select_Home_Index[m_curr_home_num];
		}
		else
		{
			//����ʷ�����������ݣ�����2S��ʱ��
			timer_app_tool_start();
			return DISPLAY_SCREEN_HEARTRATE;
		}
	}
	else
#endif
	{
		return Select_Home_Index[m_curr_home_num];
	}
}

ScreenState_t Get_Home_Index(uint8_t Num)
{
    return Select_Home_Index[Num];
}

uint8_t Get_Curr_Home_Num(ScreenState_t ScreenSta)
{
	uint8_t i;
   
	for (i=0;i<Home_Nums;i++)
	{
		if (Select_Home_Index[i]==ScreenSta)
        {
			break;
		}
	}
	
	return i;
}

uint8_t Get_Home_Nums(void)
{
    return Home_Nums;
}

void Set_Curr_Home_Num(uint8_t Num)
{
     Curr_Home_Num = Num;
}

ScreenState_t Set_Curr_Home_Index( ScreenState_t ScreenSta)
{
  uint8_t i;
   
	for (i=0;i<Home_Nums;i++)
	{
		if (Select_Home_Index[i]==ScreenSta)
    {
			Curr_Home_Num = i;
			break;
		}
	}
	return Select_Home_Index[Curr_Home_Num];
}

void Set_Select_Home_Index(void)
{
    uint8_t i;
    uint8_t j = 0;
    ScreenState_t Indextmp = DISPLAY_SCREEN_HOME;
    
    memset(Select_Home_Index,0,sizeof(Select_Home_Index));
	
    for(i=0; i<Home_Nums; i++)
    {
		if (((Custom_Home>>i) & 0x01) == 0x01) 
		{
			Select_Home_Index[j]= Indextmp;
			j++;
		}
		Indextmp++;
    }
	Home_Nums = j;
}

void set_home_index_up(void)
{
	m_key_add_sub_flag = 1;
	if( Curr_Home_Num == Home_Nums -1 )
	{
		Curr_Home_Num = 0;
	}
	else
	{

		Curr_Home_Num++;

	}
}

void set_home_index_down(void)
{
	m_key_add_sub_flag = 2;
	if( Curr_Home_Num == 0 )
	{
		Curr_Home_Num = Home_Nums -1;
	}
	else
  {

		Curr_Home_Num--;

	}
}



/*������ʾ:�Զ������������ѽ���*/
void gui_home_lock_hint_paint(void)
{


  	SetWord_t word = {0};
   	char *code;
  	//��ɫ����
  	LCD_SetBackgroundColor(LCD_BLACK);
  	
  	if(SetValue.AutoLockScreenFlag == 0)
  	{
  		//����ͼ��
  		word.x_axis = 54;
  		LCD_SetPicture(word.x_axis, LCD_CENTER_JUSTIFIED, LCD_NONE, LCD_NONE, &Home_img_unlock);
  
  		//�����ɹ�
  		word.x_axis += (Home_img_unlock.height + 30);
  		word.y_axis = LCD_CENTER_JUSTIFIED;
  		word.size = LCD_FONT_24_SIZE;
  		word.forecolor = LCD_WHITE;
  		word.bckgrndcolor = LCD_NONE;
  		LCD_SetString((char *)GuiHomeLockStrs[SetValue.Language][92],&word);
  	}
  else
  	{
  		//����ͼ��
  		word.x_axis = 54;

		
  		LCD_SetPicture(word.x_axis, LCD_CENTER_JUSTIFIED, LCD_NONE, LCD_NONE, &Home_img_lock);
  
  		//BACK+UP
  		word.x_axis += (Home_img_lock.height + 30);
  		//word.y_axis = LCD_CENTER_JUSTIFIED;
  		word.y_axis = 40;
  		word.size = LCD_FONT_24_SIZE;
  		word.forecolor = LCD_WHITE;
  		word.bckgrndcolor = LCD_NONE;
  		LCD_SetString("OK+DOWN",&word);
  		code = "OK+DOWN";
  		//word.x_axis += 24 + 8;

		word.y_axis =  120 + 20;
  		LCD_SetString((char *)GuiHomeLockStrs[SetValue.Language][91],&word);
  			
  		//back+up����ʾ
  		gui_button_back(LCD_WHITE);
  		gui_button_up(LCD_WHITE);
  	}
	
}

/*��ͷ������������ʾ*/
void gui_point_direction(uint8_t direction,uint8_t color)
{
	SetWord_t word = {0};
	switch(direction)
	{
		case IMG_POINT_LEFT://���ͷ
			word.x_axis = 116;
			word.y_axis = 2;
			LCD_SetPicture(word.x_axis, word.y_axis, color, LCD_NONE, &Img_Pointing_Left_12X8);
			break;
		case IMG_POINT_RIGHT://�Ҽ�ͷ
			word.x_axis = 116;
			word.y_axis = 222;
			LCD_SetPicture(word.x_axis, word.y_axis, color, LCD_NONE, &Img_Pointing_Right_12X8);
			break;
		case IMG_POINT_UP:
			word.x_axis = 10;
			word.y_axis = 116;
			LCD_SetPicture(word.x_axis, word.y_axis, color, LCD_NONE, &Img_Pointing_Up_12X8);
			break;
		case IMG_POINT_DOWN:
			word.x_axis = 222;
			word.y_axis = 116;
			LCD_SetPicture(word.x_axis, word.y_axis, color, LCD_NONE, &Img_Pointing_Down_12X8);
			break;
		default:
			break;
	}
}








void notify_low_bat_start(void)
{

#if defined WATCH_HAS_SOS
		//SOS�����͵������ѽ���
		if((ScreenState != DISPLAY_SCREEN_LOGO) && (ScreenState != DISPLAY_SCREEN_SOS)
			&& (ScreenState != DISPLAY_SCREEN_SOS_MENU))
#else
		//ʣ�����10%��5%��ʱ������һ��
		if(ScreenState != DISPLAY_SCREEN_LOGO)
#endif
		{
			if(ScreenState < DISPLAY_SCREEN_NOTIFY)
			{
				ScreenStateSave = ScreenState;
			}
			timer_notify_display_start(3000,1,false);
			timer_notify_motor_start(300,300,1,false,NOTIFY_MODE_SYSTEM_TIME_REACH);
			timer_notify_buzzer_start(300,300,1,false,NOTIFY_MODE_SYSTEM_TIME_REACH);
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_NOTIFY_LOW_BAT;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);								
		}
}


void gui_home_btn_evt(uint32_t Key_Value)
	{
		DISPLAY_MSG  msg = {0};
		ScreenState_t Curr_Tool_Index,Curr_Sport_Index; //��ѡ��⡢�˶����棬��������ʹ��ͳ�ƻ�ȡ
		switch( Key_Value )
		{
	
	
		  case KEY_UP://��⹦��
		  
		  case KEY_DOWN://��⹦��
        
			home_or_minor_switch = HOME_SCREEN_SWITCH;
			if(SetValue.AutoLockScreenFlag != 1)
				{
				
					timer_app_LOCK_stop(false);
					set_select_monitor_index();

					 if(switch_home_screen==BACK_HOME_SCREEN)
					 	{
                          ScreenState = get_current_monitor_screen();
						 
						  msg.cmd = MSG_DISPLAY_SCREEN;
						  xQueueSend(DisplayQueue, &msg, portMAX_DELAY);


					     }
					  else
						{
						 switch_home_screen=BACK_HOME_SCREEN;
						 
						 if((s_switch_screen_count==0)&&(Key_Value==KEY_UP))
							{
				
							 s_switch_screen_count=Home_Nums-1;
							   ScreenState = s_need_switch_screen[s_switch_screen_count];
							  if(ScreenState == DISPLAY_SCREEN_WEEK_AVERAGE_HDR)
									{
										//���ʽ���
										gui_heartrate_data_init();
										if(gui_heartrate_statu_get() != 1)
										{
											//����ʷ�����������ݣ�����2S��ʱ��
											timer_app_tool_start();
											ScreenState =  DISPLAY_SCREEN_HEARTRATE;
											
										}
									}
							   msg.cmd = MSG_DISPLAY_SCREEN;
						      xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				
				
						   }
						else if((s_switch_screen_count==Home_Nums-1)&&(Key_Value==KEY_DOWN))
							{
				
							  s_switch_screen_count=0;
							    ScreenState = s_need_switch_screen[s_switch_screen_count];
							  if(ScreenState == DISPLAY_SCREEN_WEEK_AVERAGE_HDR)
									{
										//���ʽ���
										gui_heartrate_data_init();
										if(gui_heartrate_statu_get() != 1)
										{
											//����ʷ�����������ݣ�����2S��ʱ��
											timer_app_tool_start();
											ScreenState =  DISPLAY_SCREEN_HEARTRATE;
											
										}
									}
							   msg.cmd = MSG_DISPLAY_SCREEN;
						      xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				
						   }
						 else
						 	{
                             
							 ScreenState = back_monitor_screen;
							  msg.cmd = MSG_DISPLAY_SCREEN;
						      xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
						     }
				
						}
						
				}
				else
				{
					ScreenState = DISPLAY_SCREEN_LARGE_LOCK;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				}
				break;
			case KEY_OK://�˶�����
				if(SetValue.AutoLockScreenFlag != 1)
				{
					if(0 == IS_SPORT_CONTINUE_LATER)
					{
						timer_app_LOCK_stop(false);
						Set_Select_Sport_Index();
						if(SportIndexRecAdjust())	 //��ֹ�Զ����˶���������������
						{
							Curr_Sport_Index = sports_menu_index_record();
						}
						else
						{
							Curr_Sport_Index = Select_Sport_Index[0];
						}
						
						Set_Curr_Sport_Index(Curr_Sport_Index);
						CreateSportDisplay();
						//vTaskDelay(150);
					        //�����˶�ѡ���Ǿʹ�gps������
					        CreateGPSTask();
						task_hrt_start();
						timer_timeout_back_start();
						#ifndef COD
						//��ȡѵ���ƻ�
						if(get_trainplan_valid_flag())
						{
							Get_TrainPlanDayRecord();
						}
						#endif
					}
					else
					{
						timer_app_LOCK_stop(false);
						SportContinueLaterDisplay();
					}
					//��ʾ������
					if (g_bat_evt.level_percent <= 10)
					{
				                 notify_low_bat_start();
					}
							
				}
				else
				{
					ScreenState = DISPLAY_SCREEN_LARGE_LOCK;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				}
				break;

		
			
		
			case KEY_LONG_UP://�˵�����
	

				if(SetValue.AutoLockScreenFlag != 1)
				{
				   
					switch_home_screen=BACK_HOME_SCREEN;
					timer_app_LOCK_stop(false);
					
					gui_menu_init();
					ScreenState = DISPLAY_SCREEN_MENU;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				}
				else
				{
					ScreenState = DISPLAY_SCREEN_LARGE_LOCK;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				}
				break;
			case KEY_LONG_DOWN://����
				if(SetValue.AutoLockScreenFlag != 1)
				{
					timer_app_LOCK_stop(false);
					Set_Select_Tool_Index();
					
					if(ToolIndexRecAdjust())  //��ֹ�Զ��幤������������������
						Curr_Tool_Index = tool_menu_index_record();
					else
						Curr_Tool_Index = AdjustToolIndex();
					Set_Curr_Tool_Index(Curr_Tool_Index);
					CreateToolDisplay();
					//��ʾ������
					if (g_bat_evt.level_percent <= 10)
					{
						notify_low_bat_start();
					}
				
				}
				else
				{
					ScreenState = DISPLAY_SCREEN_LARGE_LOCK;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				}
				break;
			case KEY_UP_DOWN:
			

				if(SetValue.AutoLockScreenFlag != 1)
				{
					timer_app_LOCK_stop(false);
					PostIndex = 0;//Ĭ��ѡ�������Լ�
					ScreenState = DISPLAY_SCREEN_POST_SETTINGS;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				}
				else
				{
					ScreenState = DISPLAY_SCREEN_LARGE_LOCK;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				}
	
				break;
			case (KEY_LONG_OK):
				if(SetValue.AutoLockScreenFlag != 1)
				{
					timer_app_LOCK_stop(false);
				}
			
				else
				{
					ScreenState = DISPLAY_SCREEN_LARGE_LOCK;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				}
				break;
			case KEY_BACK:
				
				if(SetValue.AutoLockScreenFlag != 1)
				{
					timer_app_LOCK_stop(false);
					
				}
				else
				{
					ScreenState = DISPLAY_SCREEN_LARGE_LOCK;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				}
				break;


				
			default:
				if(SetValue.AutoLockScreenFlag != 1)
				{
					timer_app_LOCK_stop(false);
				}
				break;
		}
	}




/*��������:�Զ������������ѽ���*/
void gui_home_lock_hint_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_BACK:{
			if(SetValue.AutoLockScreenFlag == 0)
			{
				//�����ɹ����棬��رն�ʱ��
				timer_notify_display_stop(false);
			}
			
			ScreenState = DISPLAY_SCREEN_HOME;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
			}
			break;
		default:
			break;
	}
}
#ifdef HEARTRAT_FPC_TEST
uint8_t test_press = 0;
uint8_t test_heart = 0;

void gui_hrt_test(void)
{

	SetWord_t word = {0};
	uint8_t x,y;
	char str[20];
	LCD_SetBackgroundColor(LCD_BLACK);
	float pres,alt,temp;
	
	//��ȡ������ѹ�¶�
	
	word.x_axis = 80;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.kerning = 1;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	memset(str,0,sizeof(str));
	if (test_press == 1)
	{
		word.forecolor = LCD_GREEN;
		LCD_SetString("��ѹ�ɹ�",&word);
	}
	else
	{
		word.forecolor = LCD_RED;
		LCD_SetString("��ѹʧ��",&word);
	}



	
	word.x_axis = 140;
	if (test_heart == 1)
	{
		word.forecolor = LCD_GREEN;
		LCD_SetString("���ʳɹ�",&word);
	}
	else
	{
		word.forecolor = LCD_RED;
		LCD_SetString("����ʧ��",&word);
	}


	

}
#endif

