#include "gui_notify_ble.h"
#include "SEGGER_RTT.h"
#include "drv_key.h"
#include "drv_lcd.h"
#include <string.h>
#include "font_displaystrs.h"
#include "task_display.h"
#include "com_data.h"
#include "gui_theme.h"
#include "gui_time.h"
#include "gui_system.h"
#include "cmd_ble.h"
#include "lib_error.h"
#include "drv_ble.h"
#include "task_ble.h"
#include "img_tool.h"
#include "time_notify.h"
#include "img_notify.h"
#include "img_home.h"
#include "drv_heartrate.h"
#include "algo_hdr.h"
#include "font_number.h"
#include "gui_sport.h"
#include "com_sport.h"

#define GUI_NOTIFY_BLE_LOG_ENABLED 1

#if DEBUG_ENABLED == 1 && GUI_NOTIFY_BLE_LOG_ENABLED == 1
	#define GUI_NOTIFY_BLE_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_NOTIFY_BLE_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define GUI_NOTIFY_BLE_WRITESTRING(...)
	#define GUI_NOTIFY_BLE_PRINTF(...)		        
#endif

extern SetValueStr SetValue;
#define BACKGROUND_COLOR      LCD_WHITE
#define FOREGROUND_COLOR      LCD_BLACK
//#define DELAY_MS(X)                 am_util_delay_ms(X)
#define DELAY_MS(X)                 vTaskDelay(X / portTICK_PERIOD_MS)
//#define DELAY_MS(X)                   do{TickType_t xLastWakeTime;xLastWakeTime = xTaskGetTickCount ();vTaskDelayUntil( &xLastWakeTime, X / portTICK_PERIOD_MS );}while(0)

uint8_t m_pause_index = 0;
static uint8_t train_pause_index = 0; //暂停菜单界面索引
uint8_t sport_cast_pause_index = 0; //暂停菜单界面索引

PauseTimeStr train_pausetime;

extern _ble_passkey g_ble_passkey;
extern uint8_t g_peer_addr[BLE_GAP_ADDR_LEN];
extern uint8_t g_ble_disconnect_reason;
extern uint8_t get_action_name[10];
extern uint16_t get_action_name_len;
extern uint8_t gui_sport_index;
extern uint8_t gui_sport_type;

#ifdef COD 
extern cod_ble_sport_cast cod_user_sport_cast;


#endif
extern void timer_app_vice_pasue_stop(bool is_isr);
extern void  timer_app_vice_pasue_start(uint32_t num,uint8_t flag);

void gui_notify_ble_connect_paint(void)
{
//	LCD_SetCircle(LCD_CENTER_LINE, LCD_CENTER_ROW, 80, LCD_BLUE, 0, LCD_FILL_ENABLE);
	
	LCD_SetBackgroundColor(LCD_BLACK);
	LCD_SetPicture(30, LCD_CENTER_JUSTIFIED, LCD_NONE, LCD_NONE, &bluetooth_connect_30X60);
	
	SetWord_t word = {0};
//	word.x_axis = 108;
	
	word.x_axis = 128;
	
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	word.kerning = 0;
	LCD_SetString("蓝牙连接",&word);		
	
	
//	word.x_axis = 136;
//	char p_str[20];
//	memset(p_str,0,sizeof(p_str));
//	sprintf(p_str,"%02X:%02X:%02X:%02X:%02X:%02X",g_peer_addr[0],g_peer_addr[1],g_peer_addr[2],
//																						 g_peer_addr[3],g_peer_addr[4],g_peer_addr[5]);
//	word.size = LCD_FONT_16_SIZE;
//	word.kerning = 0;
//	LCD_SetString(p_str,&word);

//	LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
}



void gui_notify_ble_connect_btn_evt(uint32_t evt)
{
	switch(evt)
	{		
		case (KEY_BACK):
//		case (KEY_LIGHT):	
//		case (KEY_UP):
		case (KEY_OK):
//	  case (KEY_DOWN):
		{
			timer_notify_display_stop(false);
//			timer_notify_motor_stop(false);
//			timer_notify_buzzer_stop(false);
			DISPLAY_MSG  msg = {0,0};
			ScreenState = ScreenStateSave;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
		}break;						
	}
}



void gui_notify_ble_disconnect_paint(void)
{
//	LCD_SetCircle(LCD_CENTER_LINE, LCD_CENTER_ROW, 80, LCD_BLUE, 0, LCD_FILL_ENABLE);
		
	LCD_SetBackgroundColor(LCD_BLACK);
	LCD_SetPicture(30, LCD_CENTER_JUSTIFIED, LCD_NONE, LCD_NONE, &bluetooth_disconnect_30X60);
	
	SetWord_t word = {0};
//	word.x_axis = 108;
	word.x_axis = 128;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	word.kerning = 0;
	LCD_SetString("蓝牙断开",&word);		
	
	if(g_ble_disconnect_reason == TASK_BLE_CONN_DISCONNECT_REASON_WRONG_PASSKEY)
	{
	//	word.x_axis = 136;
		word.x_axis = 156;
		LCD_SetString("配对",&word);
	}
	
	
	
//	LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
	
	
	
}



void gui_notify_ble_disconnect_btn_evt(uint32_t evt)
{
	switch(evt)
	{		
		case (KEY_BACK):
//		case (KEY_LIGHT):	
//		case (KEY_UP):
		case (KEY_OK):
//	  case (KEY_DOWN):
		{
			timer_notify_display_stop(false);
//			timer_notify_motor_stop(false);
//			timer_notify_buzzer_stop(false);
			DISPLAY_MSG  msg = {0,0};
			ScreenState = ScreenStateSave;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
		}break;				
	}
}



void gui_notify_ble_bond_paint(void)
{
//	LCD_SetCircle(LCD_CENTER_LINE, LCD_CENTER_ROW, 80, LCD_BLUE, 0, LCD_FILL_ENABLE);
	
	LCD_SetBackgroundColor(LCD_BLACK);
	LCD_SetPicture(30, LCD_CENTER_JUSTIFIED, LCD_NONE, LCD_NONE, &bluetooth_connect_30X60);
	
	SetWord_t word = {0};
//	word.x_axis = 108;
	word.x_axis = 128;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	word.kerning = 0;
	LCD_SetString("蓝牙绑定",&word);		
	
	
//	LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
}



void gui_notify_ble_bond_btn_evt(uint32_t evt)
{
	switch(evt)
	{		
		case (KEY_BACK):
//		case (KEY_LIGHT):	
//		case (KEY_UP):
		case (KEY_OK):
//	  case (KEY_DOWN):
		{
			timer_notify_display_stop(false);
//			timer_notify_motor_stop(false);
//			timer_notify_buzzer_stop(false);
			DISPLAY_MSG  msg = {0,0};
			ScreenState = ScreenStateSave;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
		}break;				
	}
}


void gui_notify_ble_passkey_paint(void)
{
//	LCD_SetCircle(LCD_CENTER_LINE, LCD_CENTER_ROW, 80, LCD_BLUE, 0, LCD_FILL_ENABLE);
	LCD_SetBackgroundColor(LCD_BLACK);
	LCD_SetPicture(30, LCD_CENTER_JUSTIFIED, LCD_NONE, LCD_NONE, &bluetooth_connect_30X60);
	
	SetWord_t word = {0};
//	word.x_axis = 100;
	word.x_axis = 110;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	word.kerning = 0;
	LCD_SetString("蓝牙配对",&word);		
	
//	word.x_axis = 130;
	word.x_axis = 150;
	char p_str[20];
	memset(p_str,0,sizeof(p_str));
	memcpy(p_str,g_ble_passkey.passkey,sizeof(g_ble_passkey.passkey));
	LCD_SetNumber(p_str,&word);	
	
	
//	LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
}



void gui_notify_ble_passkey_btn_evt(uint32_t evt)
{
	switch(evt)
	{		
		case (KEY_BACK):
//		case (KEY_LIGHT):	
//		case (KEY_UP):
		case (KEY_OK):
//	  case (KEY_DOWN):
		{
			timer_notify_display_stop(false);
//			timer_notify_motor_stop(false);
//			timer_notify_buzzer_stop(false);
			DISPLAY_MSG  msg = {0,0};
			ScreenState = ScreenStateSave;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
		}break;				
	}
}

#ifdef COD 
void gui_notify_on_bind_device_paint(void)
{	
//具体需要UI交互确认
	LCD_SetBackgroundColor(LCD_BLACK);
	SetWord_t word = {0};
	word.x_axis = 108;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	word.kerning = 0;
	LCD_SetString("绑定确认",&word);	

		LCD_SetPicture(58,205,LCD_RED,LCD_NONE,&step_cal_arrow_youshang);//右上角箭头

	    word.x_axis = 120 + 13 + 56 + 8;//197
	    word.y_axis = LCD_CENTER_JUSTIFIED;
  	    LCD_SetRectangle(word.x_axis,22,0,LCD_LINE_CNT_MAX,LCD_GRAY,1,1,LCD_FILL_ENABLE);
	    word.x_axis += 3;
	    word.size = LCD_FONT_16_SIZE;
	    word.forecolor = LCD_BLACK;
	    word.bckgrndcolor = LCD_NONE;
	    word.kerning = 1;
	    LCD_SetString("请按OK键",&word);//"OK键进入测试"
	
}



void gui_notify_on_bind_device_btn_evt(uint32_t evt)
{
	switch(evt)
	{		
		case (KEY_BACK):
		{
			timer_notify_display_stop(false);
		    timer_notify_motor_stop(false);
			timer_notify_buzzer_stop(false);
			DISPLAY_MSG  msg = {0,0};
			ScreenState = ScreenStateSave;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			ble_uart_response_application_on_bind_device(false);
		}
		break;
		case (KEY_OK):
		{
			timer_notify_display_stop(false);
		    timer_notify_motor_stop(false);
			timer_notify_buzzer_stop(false);
			DISPLAY_MSG  msg = {0,0};
			ScreenState = ScreenStateSave;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			ble_uart_response_application_on_bind_device(true);
		}break;	
		default:
		break;
	}
}
void gui_notify_locatin_ok_paint()
	{
	SetWord_t word = {0};

  if(ScreenStateSave==DISPLAY_SCREEN_SPORT_READY)
  	{
  	 
	   gui_sport_ready_paint();
	    //底圈
		LCD_SetCircle(LCD_CENTER_LINE, LCD_CENTER_ROW, 45, LCD_BLACK, 45, LCD_FILL_ENABLE);
		LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 45, 5, 0, 360, LCD_WHITE);
		word.forecolor = LCD_WHITE;
  	}
  else
  	{

	  
      gui_sport_location_ok_data_paint(gui_sport_type,gui_sport_index);
	   	//底圈
		LCD_SetCircle(LCD_CENTER_LINE, LCD_CENTER_ROW, 45, LCD_WHITE, 45, LCD_FILL_ENABLE);
		LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 45, 5, 0, 360, LCD_BLACK);
		word.forecolor = LCD_BLACK;

   }
	
	
		word.x_axis = 120 - 4 - 24 ;
		word.y_axis = LCD_CENTER_JUSTIFIED;
    
        word.bckgrndcolor= LCD_NONE;
		word.size = LCD_FONT_24_SIZE;
		word.kerning = 0;

		LCD_SetString("定位",&word);
		
		word.x_axis = 120 + 4;
		
		LCD_SetString("成功",&word);
		
	
	}


void gui_notify_bind_status_paint(cod_bind_status status)
{	
//具体需要UI交互确认
	LCD_SetBackgroundColor(LCD_BLACK);
	SetWord_t word = {0};
//	word.x_axis = 108;
	word.x_axis = 108;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	word.kerning = 0;
	if(status == STATUS_BINDING )
	{
		LCD_SetString("绑定中",&word);
	}
	else if (status == STATUS_BIND_SUCCESS)
	{
		LCD_SetString("绑定成功",&word);
	}
	else
	{
		LCD_SetString("绑定失败",&word);
	}
	
}



void gui_notify_bind_status_btn_evt(uint32_t evt)
{
	switch(evt)
	{		
		case (KEY_BACK):
		case (KEY_OK):
		{
			timer_notify_display_stop(false);
		    timer_notify_motor_stop(false);
			timer_notify_buzzer_stop(false);
			DISPLAY_MSG  msg = {0,0};
			ScreenState = ScreenStateSave;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}break;	
		default:
		break;
	}
}
void gui_notify_locatin_ok_btn_evt(uint32_t evt)
{
	switch(evt)
		{		
			case (KEY_BACK):
			case (KEY_OK):
			{
				timer_notify_display_stop(false);
				timer_notify_motor_stop(false);
				timer_notify_buzzer_stop(false);
				DISPLAY_MSG  msg = {0,0};
				ScreenState = ScreenStateSave;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}break; 
			default:
			break;
		}


}

//即将开始运动提醒
void gui_training_cast_start_remind_paint(void)
{	
	LCD_SetBackgroundColor(LCD_BLACK);

	LCD_SetPicture(120-img_train.height, LCD_CENTER_JUSTIFIED, LCD_CYAN, LCD_NONE, &img_train);
	
	SetWord_t word = {0};
	word.x_axis = 120+16;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	word.kerning = 0;
	LCD_SetString((char *)TrainStrs[SetValue.Language][0],&word);
	
}
//按键提醒
void gui_training_cast_press_remind_paint(void)
{	
	LCD_SetBackgroundColor(LCD_BLACK);
	
	SetWord_t word = {0};

	word.x_axis = 25;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	LCD_SetString("提示",&word);//"提示"
    
	LCD_SetPicture(word.x_axis,LCD_PIXEL_ONELINE_MAX/2 - 16 - 4 - 16,LCD_RED,LCD_NONE,&img_tool_vo2max_ahead_quit_warn);
	
	word.x_axis = 108;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	word.kerning = 0;

	LCD_SetString((char *)TrainStrs[SetValue.Language][5],&word);

	//LCD_SetArc(LCD_CENTER_LINE,LCD_CENTER_ROW,122,8,255,285,LCD_GREEN);
}

//训练结束提醒
void gui_training_cast_end_remind_paint(void)
{	
	char p_str[20];
    uint8_t len;
	LCD_SetBackgroundColor(LCD_BLACK);

	LCD_SetPicture(20, LCD_CENTER_JUSTIFIED, LCD_CYAN, LCD_NONE, &img_train);

	SetWord_t word = {0};
	word.x_axis = 90+4;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	word.kerning = 0;
	LCD_SetString((char *)TrainStrs[SetValue.Language][11],&word);

	word.x_axis = 130;
	word.y_axis = 25;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][5],&word);

	word.x_axis = 160;
	memset(p_str,0,sizeof(p_str));
	sprintf(p_str,"%d",cod_user_training_cast.heart_rate);
	len = strlen(p_str);
	word.y_axis = 25 + 2*24 - len/2;
	LCD_SetNumber(p_str,&word);
	
	word.x_axis = 130;
	word.y_axis = 150;
	LCD_SetString((char *)SportDetailStrs[SetValue.Language][4],&word);
	
	word.x_axis = 160;
	memset(p_str,0,sizeof(p_str));
	//sprintf(p_str,"%d",(uint32_t)cod_user_training_cast.calorie);
	sprintf(p_str,"%d.%01d",(uint32_t)(cod_user_training_cast.calorie),(uint32_t)(cod_user_training_cast.calorie*10)%10);
	len = strlen(p_str);
	word.y_axis = 150 + 24 - len/2;
	LCD_SetNumber(p_str,&word);
	
}

void gui_training_cast_end_remind_btn_evt(uint32_t evt)
{
	switch(evt)
	{		
		case (KEY_BACK):
		case (KEY_DOWN):
		{
			timer_notify_display_stop(false);
		    timer_notify_motor_stop(false);
			timer_notify_buzzer_stop(false);
			DISPLAY_MSG  msg = {0,0};
			ScreenState = ScreenStateSave;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}break;	
		
		default:
		break;
	}

}

void gui_training_cast_title_paint(SetWord_t *word)
{

	char p_str[40];
	char p_tmp[20];
	uint8_t len = 0;
	if (cod_user_training_cast.training_type == TRAINING_TYPE_INTERACT)
	{
	    memset(p_str,0,sizeof(p_str));
		if (get_action_name_len > 8)
		{
			get_action_name_len = 8;
		}
	
		memcpy(p_str,get_action_name,get_action_name_len);

		
		if (cod_user_training_cast.action_type ==TRAINING_ACTION_TYPE_COUNT)
		{
			memset(p_tmp,0,sizeof(p_tmp));
			sprintf(p_tmp,"%d",cod_user_training_cast.action_target);
			len = (int)log10(cod_user_training_cast.action_target) + 1;
			if (len > 5)
			{
				len = 5;
			}
			memcpy(p_str +get_action_name_len,p_tmp,len);
				
			memcpy(p_str +get_action_name_len +len,(char *)TrainStrs[SetValue.Language][3],2);
		}
		else
		{
			memset(p_tmp,0,sizeof(p_tmp));
			sprintf(p_tmp,"%d",cod_user_training_cast.action_target/60);
			len = (int)log10(cod_user_training_cast.action_target/60) + 1;
			if (len > 5)
			{
				len =5;
			}
			memcpy(p_str +get_action_name_len,p_tmp,len);
				
			memcpy(p_str +get_action_name_len +len,(char *)TrainStrs[SetValue.Language][4],4);
		}
			
		  LCD_SetString(p_str,word);
		  
	
	}

	else if (cod_user_training_cast.training_type == TRAINING_TYPE_LIVE)
	{
		LCD_SetString((char *)TrainStrs[SetValue.Language][6],word);
	}

	else if (cod_user_training_cast.training_type == TRAINING_TYPE_NORMAL)
	{
		LCD_SetString((char *)TrainStrs[SetValue.Language][7],word);
	}
	
}

void gui_training_cast_heart_paint(uint8_t x_axis ,uint8_t Range)
{
	SetWord_t word = {0};
	char p_str[30];
	uint8_t hdrRange,heart;
	
		     heart	= drv_getHdrValue();//cod_user_training_cast.heart_rate;
			 word.forecolor = get_hdr_range_color(heart);
	if(Range ==0)
		{
         
		
					 //心率
			 LCD_SetPicture(x_axis, LCD_CENTER_JUSTIFIED, word.forecolor, LCD_NONE, &Img_heartrate_18X16);
			 
			 word.x_axis =x_axis+Img_heartrate_18X16.height+8;
			 word.y_axis = LCD_CENTER_JUSTIFIED;
			 word.size = LCD_FONT_24_SIZE;
			 word.forecolor = LCD_WHITE;
			 word.kerning = 0;
			 if (heart != 0)
			 {
				 memset(p_str,0,sizeof(p_str));
				 sprintf(p_str,"%d",heart);
				 LCD_SetNumber(p_str,&word);
			 }
			 else
			 {	
		 
				 sprintf(p_str,"%d",heart);
				 LCD_SetNumber(p_str,&word);
			   
				 //LCD_SetRectangle(word.x_axis,2,120-2-8,8,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);  
				 
				 //LCD_SetRectangle(word.x_axis,2,120+2,8,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);	 
			 }


	   }

	else if (Range ==1)
	{

	    //心率图标
		
			LCD_SetPicture(x_axis, 120-2*16-8 -Img_heartrate_18X16.width, word.forecolor, LCD_NONE, &Img_heartrate_18X16);
		//心率值	
			word.x_axis =x_axis+Img_heartrate_18X16.height+8;
			word.y_axis = LCD_CENTER_JUSTIFIED;
			word.size = LCD_FONT_24_SIZE;
			word.forecolor = LCD_WHITE;
			word.kerning = 0;
			if (heart != 0)
			{
				memset(p_str,0,sizeof(p_str));
				sprintf(p_str,"%d",heart);
				LCD_SetNumber(p_str,&word);
			}
			else
			{  
		
				sprintf(p_str,"%d",heart);
				LCD_SetNumber(p_str,&word);
			  
				//LCD_SetRectangle(word.x_axis,2,120-2-8,8,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);	
				
				//LCD_SetRectangle(word.x_axis,2,120+2,8,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);	
			}

        //心率区间
		word.x_axis = x_axis + Img_heartrate_18X16.height/2 - 16/2;
		word.y_axis = 120 - 2*16;
		word.forecolor = get_hdr_range_color(heart);
		word.size = LCD_FONT_16_SIZE;
		word.kerning = 1;
		hdrRange = get_hdr_range(heart);
		if (hdrRange != 0)
		{		
	
		 LCD_SetString((char *)heartzonegodstrs[SetValue.Language][hdrRange -1],&word);
		}
   }
}

void gui_training_cast_time_paint(void)
{

	SetWord_t word = {0};
	char p_str[30];
	word.x_axis =20;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	word.kerning = 0;
	memset(p_str,0,sizeof(p_str));
	sprintf(p_str,"%d:%02d",RTC_time.ui32Hour,RTC_time.ui32Minute);
	LCD_SetNumber(p_str,&word);	
}
void gui_train_options(uint8_t index)
	{
				SetWord_t word = {0};
				
				//菜单第一项
			   word.x_axis = 120 + 18;
			   word.y_axis = LCD_CENTER_JUSTIFIED;
			   word.forecolor = LCD_CYAN;
			   word.bckgrndcolor = LCD_NONE;
			   word.kerning = 0;
			   word.size = LCD_FONT_24_SIZE;
				LCD_SetString((char *)trainpauseStrs[SetValue.Language][index],&word); 
				
				//菜单第二项
				if(index >= 1)
				{
					index = 0;
				}
				else
				{
					index += 1;
				}
				word.x_axis = 180 + 18;
				word.forecolor = LCD_LIGHTGRAY;
				word.size = LCD_FONT_16_SIZE;
				LCD_SetString((char *)trainpauseStrs[SetValue.Language][index],&word);
 }
void gui_train_pause_time(void)
{
	SetWord_t word = {0};
	char str[20] = {0};
	uint32_t second;

	//计算总时长
	second = train_pausetime.Pause_Stop_Time.Hour*60*60 + train_pausetime.Pause_Stop_Time.Minute*60 + train_pausetime.Pause_Stop_Time.Second;
	
	if(second >= 6000)
	{
		//暂停时长不超过99:59
		second = 5999;
	}
	
	memset(str,0,sizeof(str));
	sprintf(str,"%d:%02d",second/60,second%60);
	
	//已暂停
	word.x_axis = 20;
	word.y_axis = (LCD_PIXEL_ONELINE_MAX - 48 - 5 - strlen(str)*(Font_Number_19.width+1))/2;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;
	LCD_SetString((char *)SportsPromptStrs[SetValue.Language][1],&word);

	//暂停时长
	word.y_axis += 48 + 6;
	word.kerning = 1;
	LCD_SetNumber(str,&word);
}
void gui_sport_cast_pause_paint(void)
{
	
	char str[20];
	SetWord_t word = {0};
	LCD_SetBackgroundColor(LCD_BLACK);
	 gui_train_pause_time();//暂停时长自己统计
	if(train_pausetime.Pause_Stop_Time.Minute>=5)//大于5分钟自动关闭心率测量
	{
		  
	
	}
		 

//运动时长
    word.x_axis = 39+16;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_56_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	memset(str,0,sizeof(str));
	


	if(cod_user_sport_cast.time >= 3600)
	  {
						//时分秒
		sprintf(str,"%d:%02d:%02d",cod_user_sport_cast.time/3600, cod_user_sport_cast.time/60%60, cod_user_sport_cast.time%60);
	 }
	else
		{
						//分秒
	    sprintf(str,"%d:%02d",cod_user_sport_cast.time/60%60, cod_user_sport_cast.time%60);
	 }
	LCD_SetNumber(str, &word);

	 LCD_SetRectangle(120 - 1 ,2,LCD_LINE_CNT_MAX/2 - 90,180,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
	 LCD_SetRectangle(120 + 60 - 1,2,LCD_LINE_CNT_MAX/2 - 90,180,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
	   //选项
	  gui_train_options(sport_cast_pause_index);
		
    //下翻提示
	 gui_sport_page_prompt(1);
				 
			  
 }



//训练暂停提醒
void gui_training_cast_pause_paint(void)

{

    uint8_t hdrRange,heart;
	char str[20];
	SetWord_t word = {0};
    LCD_SetBackgroundColor(LCD_BLACK);
	 gui_train_pause_time();//暂停时长自己统计
	  if(train_pausetime.Pause_Stop_Time.Minute>=5)//大于5分钟自动关闭心率测量
	  	{
      

	    }
	// heart	= cod_user_training_cast.heart_rate;
	 
	 heart	= drv_getHdrValue();
     word.x_axis = 39+16;
	 word.y_axis = LCD_CENTER_JUSTIFIED;
	 word.size = LCD_FONT_56_SIZE;
	 word.forecolor = LCD_WHITE;
	 word.bckgrndcolor = LCD_NONE;
	 word.kerning = 1;
	 memset(str,0,sizeof(str));
	 sprintf(str,"%d",heart);
	  LCD_SetNumber(str, &word);
	  
	  LCD_SetPicture(39 + 16 + Font_Number_56.height/2 -Img_heartrate_18X16.height/2, 120 - 3*Font_Number_56.width/2 - 16 -Img_heartrate_18X16.width, word.forecolor, LCD_NONE, &Img_heartrate_18X16);
	
	       LCD_SetRectangle(120 - 1 ,2,LCD_LINE_CNT_MAX/2 - 90,180,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
	       LCD_SetRectangle(120 + 60 - 1,2,LCD_LINE_CNT_MAX/2 - 90,180,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);

	
	       //选项
		   gui_train_options(train_pause_index);
	
    
	
	       //下翻提示
	       gui_sport_page_prompt(1);
             
		  
  }
void set_train_pause_time()
{

train_pausetime.Pause_Stop_Time.Second = 0;
train_pausetime.Pause_Stop_Time.Minute = 0;
train_pausetime.Pause_Stop_Time.Hour = 0;





}
void gui_sport_cast_pause_btn_evt(uint32_t evt)
	{
		DISPLAY_MSG  msg = {0};
	
		switch(evt)
		{		
			
		 case KEY_UP:
						if(sport_cast_pause_index > 0)
						{
							sport_cast_pause_index--;
						
						}
						else
						{
							sport_cast_pause_index = 1;
						}
						
						ScreenState = DISPLAY_SCREEN_SPORT_CAST_PAUSE_REMIND;
						msg.cmd = MSG_DISPLAY_SCREEN;
						xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
						break;
			   case KEY_DOWN:
						if(sport_cast_pause_index < 1)
						{
							sport_cast_pause_index++;
					
						}
						else
						{
							sport_cast_pause_index = 0;
						}
						
						ScreenState = DISPLAY_SCREEN_SPORT_CAST_PAUSE_REMIND;
						msg.cmd = MSG_DISPLAY_SCREEN;
						xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
						break;
	
			
			case KEY_OK:
				timer_app_vice_pasue_stop(false);
				
				set_train_pause_time(); 
				if(sport_cast_pause_index == 0)//继续
				{
					ble_uart_response_application_sport_ctrl(SPORT_CTRL_RESUME);		
				}
				else if(sport_cast_pause_index == 1)//结束
				{
								   
			   ble_uart_response_application_sport_ctrl(SPORT_CTRL_STOP);
				
			   }
				
				break;
	
		}
	}


void gui_training_cast_pause_btn_evt(uint32_t evt)
{
	DISPLAY_MSG  msg = {0};

	switch(evt)
	{		
		
     case KEY_UP:
           			if(train_pause_index > 0)
           			{
           				train_pause_index--;
           			
           			}
           			else
           			{
           				train_pause_index = 1;
           			}
           			
           			ScreenState = DISPLAY_SCREEN_TRAIN_CAST_PAUSE_REMIND;
					msg.cmd = MSG_DISPLAY_SCREEN;
           			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
           		    break;
           case KEY_DOWN:
           			if(train_pause_index < 1)
           			{
           				train_pause_index++;
           		
           			}
           			else
           			{
           				train_pause_index = 0;
           			}
           			
					ScreenState = DISPLAY_SCREEN_TRAIN_CAST_PAUSE_REMIND;
					msg.cmd = MSG_DISPLAY_SCREEN;
           			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
           			break;

		
		case KEY_OK:
			timer_app_vice_pasue_stop(false);
			
            set_train_pause_time();	
            if(train_pause_index == 0)//继续
           	{
				ble_uart_response_application_sport_ctrl(SPORT_CTRL_RESUME);		
           	}
           	else if(train_pause_index == 1)//结束
           	{
           				       
		   ble_uart_response_application_sport_ctrl(SPORT_CTRL_STOP);
			
           }
			
			break;

	}
}
void gui_training_cast_remind_btn_evt(uint32_t evt)
{
	DISPLAY_MSG  msg = {0};

	
	switch(evt)
     {
     case  KEY_LONG_OK:
      // DISPLAY_MSG  msg = {0,0};
					ScreenState = DISPLAY_SCREEN_TRAIN_CAST_PAUSE_REMIND;
					
					timer_app_vice_pasue_start(300,2);
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	

		
	 	break;
		default:
			break;

		}
}


void gui_training_cast_paint(void)
{	
	LCD_SetBackgroundColor(LCD_BLACK);
	SetWord_t word = {0};
	char p_str[30];
	#if 0//test
	cod_user_training_cast.training_status = TRAINING_STATUS_TRAINING;
	cod_user_training_cast.training_type = TRAINING_TYPE_INTERACT;
	cod_user_training_cast.action_name_len = 8;
	memcpy(cod_user_training_cast.action_name,"平板支撑",cod_user_training_cast.action_name_len);
	cod_user_training_cast.action_target = 15;
	cod_user_training_cast.action_actual =12;
	cod_user_training_cast.time = 6000;
	cod_user_training_cast.action_type =TRAINING_ACTION_TYPE_COUNT;
	cod_user_training_cast.calorie = 345;
	cod_user_training_cast.heart_rate = 135;
	#endif 
	gui_training_cast_time_paint();//RTC时间
	//标题
	word.x_axis = 45;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	word.kerning = 1;		

	gui_training_cast_title_paint(&word);
	
	//准备
	if (cod_user_training_cast.training_status == TRAINING_STATUS_PREPARE )
	{
		word.x_axis = 108;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_24_SIZE;
		word.forecolor = LCD_CYAN;
		word.bckgrndcolor = LCD_NONE;
		word.kerning = 2;

		LCD_SetString((char *)TrainStrs[SetValue.Language][1],&word);

		LCD_SetCircle(LCD_CENTER_LINE,LCD_CENTER_ROW,120,LCD_CYAN,4,LCD_FILL_DISABLE);

		//心率
		gui_training_cast_heart_paint(120+12+36,0);
	}
	//训练中和恢复训练
	else if (cod_user_training_cast.training_status == TRAINING_STATUS_TRAINING  || cod_user_training_cast.training_status == TRAINING_STATUS_RESUME)
	{	
		//训练中
		if(cod_user_training_cast.training_type == TRAINING_TYPE_INTERACT) //互动课程
		{
			if(cod_user_training_cast.action_type == TRAINING_ACTION_TYPE_COUNT)
			{
				word.x_axis =120 - Font_Number_56.height/2;
				word.y_axis = LCD_CENTER_JUSTIFIED;
				word.size = LCD_FONT_56_SIZE;
				word.forecolor = LCD_WHITE;
				word.bckgrndcolor = LCD_NONE;
				word.kerning = 2;
				memset(p_str,0,sizeof(p_str));
				sprintf(p_str,"%d",cod_user_training_cast.action_actual);
				LCD_SetNumber(p_str,&word);	
				
				//LCD_SetCircle(LCD_CENTER_LINE,LCD_CENTER_ROW,120,LCD_CYAN,4,LCD_FILL_DISABLE);
				uint16_t endangle = 360 * cod_user_training_cast.action_actual/cod_user_training_cast.action_target;
				LCD_SetArc(LCD_CENTER_LINE,LCD_CENTER_ROW,120,4,0,endangle,LCD_CYAN);
				
			}
			else
			{
				word.x_axis =120 - Font_Number_56.height/2;
				word.y_axis = LCD_CENTER_JUSTIFIED;
				word.size = LCD_FONT_56_SIZE;
				word.forecolor = LCD_WHITE;
				word.bckgrndcolor = LCD_NONE;
				word.kerning = 2;
				memset(p_str,0,sizeof(p_str));
				if (cod_user_training_cast.action_actual > 3599)
				{
					cod_user_training_cast.action_actual = 3599;
				}
				sprintf(p_str,"%d:%02d",cod_user_training_cast.action_actual/60,cod_user_training_cast.action_actual%60);
				LCD_SetNumber(p_str,&word);	
				//添加时间百分比
				uint16_t endangle = 360 * cod_user_training_cast.action_actual/cod_user_training_cast.action_target;
				LCD_SetArc(LCD_CENTER_LINE,LCD_CENTER_ROW,120,4,0,endangle,LCD_CYAN);
			}
			
			gui_training_cast_heart_paint(120+12+36,1);
		}

		else if ((cod_user_training_cast.training_type == TRAINING_TYPE_NORMAL) ||(cod_user_training_cast.training_type == TRAINING_TYPE_LIVE))
		{
			word.x_axis =120 - Font_Number_56.height/2;
			word.y_axis = LCD_CENTER_JUSTIFIED;
			word.size = LCD_FONT_56_SIZE;
			word.forecolor = LCD_WHITE;
			word.bckgrndcolor = LCD_NONE;
			word.kerning = 2;
			memset(p_str,0,sizeof(p_str));
			if (cod_user_training_cast.action_actual > 3599)
			{
				sprintf(p_str,"%d:%02d:%02d",cod_user_training_cast.action_actual/3600,cod_user_training_cast.action_actual%3600/60,cod_user_training_cast.action_actual%60);
			}
			else
			{
				sprintf(p_str,"%d:%02d",cod_user_training_cast.action_actual/60,cod_user_training_cast.action_actual%60);
			}
			LCD_SetNumber(p_str,&word);	
             //添加时间百分比
			uint16_t endangle = 360 * cod_user_training_cast.action_actual/cod_user_training_cast.action_target;
			LCD_SetArc(LCD_CENTER_LINE,LCD_CENTER_ROW,120,4,0,endangle,LCD_CYAN);
		
			
		}
		
		gui_training_cast_heart_paint(120+12+36,1);
	}
	//休息

	else if (cod_user_training_cast.training_status == TRAINING_STATUS_REST )
	{
		
		word.x_axis = 108;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_24_SIZE;
		word.forecolor = LCD_RED;
		word.bckgrndcolor = LCD_NONE;
		word.kerning = 2;

		LCD_SetString((char *)TrainStrs[SetValue.Language][2],&word);

		uint16_t endangle = 360 * cod_user_training_cast.action_actual/cod_user_training_cast.action_target;
		
		LCD_SetArc(LCD_CENTER_LINE,LCD_CENTER_ROW,120,4,0,endangle,LCD_LIGHTGRAY);

		gui_training_cast_heart_paint(120+12+36,0);
	}
      //未知
	else if (cod_user_training_cast.training_status == TRAINING_STATUS_UNKNOWN)
	{
		word.x_axis = 108;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_24_SIZE;
		word.forecolor = LCD_RED;
		word.bckgrndcolor = LCD_NONE;
		word.kerning = 2;

		LCD_SetString((char *)TrainStrs[SetValue.Language][12],&word);
	}
  //开始
	else if (cod_user_training_cast.training_status == TRAINING_STATUS_START)
	{
		word.x_axis = 108;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_24_SIZE;
		word.forecolor = LCD_CYAN;
		word.bckgrndcolor = LCD_NONE;
		word.kerning = 2;

		LCD_SetString((char *)TrainStrs[SetValue.Language][0],&word);
	}
	//讲解

	else if (cod_user_training_cast.training_status == TRAINING_STATUS_INTRODUCT)
	{
		word.x_axis = 108;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_24_SIZE;
		word.forecolor = LCD_CYAN;
		word.bckgrndcolor = LCD_NONE;
		word.kerning = 2;

		LCD_SetString((char *)TrainStrs[SetValue.Language][13],&word);
	}
	
}



void gui_training_cast_btn_evt(uint32_t evt)
{
	switch(evt)
	{		
		case (KEY_BACK):
		case (KEY_OK):
		{
			if (cod_user_training_cast.training_type != TRAINING_TYPE_LIVE)
			{
				if((ScreenState != DISPLAY_SCREEN_LOGO)
				&& (ScreenState != DISPLAY_SCREEN_NOTIFY_BAT_CHG)
				&& time_notify_is_ok())
				{
					if(ScreenState < DISPLAY_SCREEN_NOTIFY)
					{
						ScreenStateSave = ScreenState;
					}
					
					timer_notify_display_start(3000,1,false);

					DISPLAY_MSG  msg = {0,0};
					ScreenState = DISPLAY_SCREEN_TRAIN_CAST_PRESS_REMIND;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);								
				}
			}
		
		}break;	
		case (KEY_LONG_OK):
		{	
			if (cod_user_training_cast.training_type != TRAINING_TYPE_LIVE)
			{
				ble_uart_response_application_sport_ctrl(SPORT_CTRL_PAUSE);
			}
		}break;	
		
		default:
		break;

	}
}
void gui_vice_sport_btn_evt(uint32_t evt)
{
	switch(evt)
	{
	case KEY_OK:
      ble_uart_response_application_sport_ctrl(SPORT_CTRL_PAUSE);
      break;

	  
    default:
  	break;
	}
}




#endif







