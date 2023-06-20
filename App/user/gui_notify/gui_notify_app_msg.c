#include "gui_notification.h"
#include "gui_notify_app_msg.h"
#include "SEGGER_RTT.h"
#include "drv_key.h"
#include "drv_lcd.h"
#include "task_ble.h"
#include "time_notify.h"
#include "task_display.h"
#include "img_home.h"
#include "drv_extFlash.h"
#include <string.h>
#include "com_data.h"
#include "cmd_ble.h"
#include "drv_ble.h"
#include "task_ble.h"
#include "lib_error.h"
#include "watch_config.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "portmacro.h"
#include "portable.h"
#include "event_groups.h"
#include "semphr.h"

#define GUI_NOTIFY_APP_MSG_LOG_ENABLED 0

#if DEBUG_ENABLED == 1 && GUI_NOTIFY_APP_MSG_LOG_ENABLED == 1
	#define GUI_NOTIFY_APP_MSG_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_NOTIFY_APP_MSG_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define GUI_NOTIFY_APP_MSG_WRITESTRING(...)
	#define GUI_NOTIFY_APP_MSG_PRINTF(...)		        
#endif

//#define USE_TYPE_CALL_IOS

//#define DELAY_MS(X)                 am_util_delay_ms(X)
#define DELAY_MS(X)                 vTaskDelay(X / portTICK_PERIOD_MS)
//#define DELAY_MS(X)                   do{TickType_t xLastWakeTime;xLastWakeTime = xTaskGetTickCount ();vTaskDelayUntil( &xLastWakeTime, X / portTICK_PERIOD_MS );}while(0)


_app_notification_msg g_app_notification_msg;
extern SetValueStr SetValue;
extern uint8_t fore_color[6];
/*

电话:已存-标题,未存-内容就是电话号码
短信:标题+内容
QQ:只有内容
微信:标题+内容，标题是固定的微信



*/
void gui_notify_app_msg_paint(void)
{
	LCD_SetBackgroundColor(LCD_BLACK);
	int i;
	SetWord_t word = {0};
	word.x_axis = 24;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	word.kerning = 0;
	uint8_t index = 0,line = 0;
	uint8_t offset = 0,length = 0;
	uint8_t num_display_character = 0;
	
	
	uint8_t len = 0; 
	char p_str[64];
	uint8_t type = (g_app_notification_msg.type==APP_MSG_TYPE_CALL_IOS)?APP_MSG_TYPE_CALL:g_app_notification_msg.type;
#if defined WATCH_COM_SHORTCUNT_UP
	if(type>5)
		{
	     LCD_SetPicture(24, LCD_CENTER_JUSTIFIED, fore_color[type - 2], LCD_NONE, &img_notifi_msg[type - 2]);		//图标
		}
	else
		{
          
		  LCD_SetPicture(24, LCD_CENTER_JUSTIFIED, fore_color[type - 1], LCD_NONE, &img_notifi_msg[type - 1]);		  //图标

	    }
#else
	LCD_SetRectangle(0,80,0,LCD_LINE_CNT_MAX,LCD_LIME,0,0,LCD_FILL_ENABLE);
	
	if(type>5)
		{
	      LCD_SetPicture(24, LCD_CENTER_JUSTIFIED, fore_color[type - 2], LCD_NONE, &img_notifi_msg[type - 2]);		//图标
		}
	else
		{
       
	   LCD_SetPicture(24, LCD_CENTER_JUSTIFIED, fore_color[type - 1], LCD_NONE, &img_notifi_msg[type - 1]);	 //图标

	   }
#endif
	
#ifdef USE_TYPE_CALL_IOS	
	if(g_app_notification_msg.type == APP_MSG_TYPE_CALL_IOS)
	{
		word.x_axis = 60;
		word.size = LCD_FONT_16_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_CYAN;			
		memset(p_str,0,sizeof(p_str));
		memcpy(p_str,"按上键接听，按下键挂断",sizeof("按上键接听，按下键挂断"));		
		LCD_DisplayGTString(p_str,&word);		
	}
#endif	
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;	
	word.size = LCD_FONT_24_SIZE;
	
	word.x_axis = 24 +  img_notifi_msg[0].height;
	word.x_axis+= 8;
	if((g_app_notification_msg.title[0] != 0 ) && (g_app_notification_msg.type != APP_MSG_TYPE_QQ))//QQ的数据没有标题，标题在信息中
	{
		while(g_app_notification_msg.title[index] != 0)					//推送信息数据循环判断逻辑 
		{
			if(g_app_notification_msg.title[index] & 0x80)				//判断编码是否为中文
			{
			 //中文
			 index += 2;
			}
			else
			{
			 //英文字符
			 index++;
			}
		}
		/*
		* 当字数超过屏幕长度，只显示16个字符，当第16个字节是中文，只显示15个字符
		*/
		//len = sizeof(p_str);
		if(index > 16)
		{
			len = 16;
			if(g_app_notification_msg.title[16] & 0x80)
			{
				len = 15;
			}
		}

		else
			{
             
			 len = index;

		   }
		memset(p_str,0,sizeof(p_str));
		memcpy(p_str,g_app_notification_msg.title,len);		
		LCD_DisplayGTString(p_str,&word);
		word.x_axis += 34;
		index = 0;
		
	}

	if((g_app_notification_msg.subtitle[0] != 0)&&(g_app_notification_msg.type == APP_MSG_TYPE_CALL || g_app_notification_msg.type == APP_MSG_TYPE_CALL_IOS))
	{
		LCD_DisplayGTString((char *)g_app_notification_msg.subtitle,&word);	
		word.x_axis += 34;	
	}	

	
	word.size = LCD_FONT_24_SIZE;
	
	word.y_axis = LCD_CENTER_JUSTIFIED;

	while(g_app_notification_msg.message[index] != 0)					//推送信息数据循环判断逻辑 
	{
		
		if (g_app_notification_msg.message[index] & 0x80)				//判断编码是否为中文
		{
			//中文
			index += 2;
			length += 2;
			num_display_character++;
		}
		else
		{
			//英文字符
			index++;
			length++;
			num_display_character++;
		}
		if (line>2) //无法显示超过5行的信息
		{
		break;	
		
		}   

		if(g_app_notification_msg.type==APP_MSG_TYPE_CALL)
			{

			if((num_display_character%13 == 0 ) && (line < 2))
				{
		 	
						memset(p_str,0,sizeof(p_str));
						memcpy(p_str,g_app_notification_msg.message+offset,length);			
						LCD_DisplayGTString(p_str,&word);	
						offset = index;
						length = 0;
						word.x_axis += 28;
						line++;	
				}
			 }
		 else
			{
            if((num_display_character%8 == 0 ) && (line < 2))
				{
		 	
						memset(p_str,0,sizeof(p_str));
						if(line==0)
							{
                             memcpy(p_str,g_app_notification_msg.message+offset,length);
							 offset = index;
								length = 0;
								LCD_DisplayGTString(p_str,&word);	
								
						   }
						else
							{
                             memcpy(p_str,g_app_notification_msg.message+offset,length-2);
							 memcpy(p_str+length-2,"...",2);
							 offset = index;
							 length = 0;
							 LCD_DisplayGTString(p_str,&word);

						    }
						
						word.x_axis += 28;
						line++;	
				}

		   }
		  
			

	}

	if(length > 0 && line < 2)				
	{
		
		memset(p_str,0,sizeof(p_str));
		memcpy(p_str,g_app_notification_msg.message+offset,length);
		LCD_DisplayGTString(p_str,&word);
		word.x_axis += 28;
	}


/*	else if ((num_display_character%10 != 0) && (line<4))
	{
		
		memset(p_str,0,sizeof(p_str));
		memcpy(p_str,g_app_notification_msg.message+offset,20);
		LCD_DisplayGTString(p_str,&word);
		word.x_axis += 28;	
	}	

	else if ((num_display_character%10 != 0) && (line==4))
	{
		
		memset(p_str,0,sizeof(p_str));
		memcpy(p_str,g_app_notification_msg.message+offset,16);
		LCD_DisplayGTString(p_str,&word);
		word.x_axis += 28;	
	}*/
	
/*	if(g_app_notification_msg.date_time[0] != 0)				//显示消息推送时间
	{
		memset(p_str,0,sizeof(p_str));
		sprintf(p_str,"(%c%c:%c%c)",g_app_notification_msg.date_time[9],g_app_notification_msg.date_time[10],
		g_app_notification_msg.date_time[11],g_app_notification_msg.date_time[12]);
		LCD_SetString(p_str,&word);	
		//word.x_axis += 16;
	}	*/

	
	word.size = LCD_FONT_16_SIZE;
	if(g_app_notification_msg.type == APP_MSG_TYPE_CALL)
		{
        word.x_axis = 200;
        LCD_SetRectangle(word.x_axis - 3,word.size + 6,0,240,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
	    word.forecolor = LCD_BLACK;
	    word.bckgrndcolor = LCD_NONE;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		LCD_SetString("按back键拒接",&word);
		


	   }
	else;
}

void gui_notify_app_msg_btn_evt(uint32_t evt)
{
//	DISPLAY_MSG  msg = {0};
	switch(evt)
	{		
		case KEY_OK:
		case (KEY_UP):
	    case (KEY_DOWN):
		{
		#ifdef COD 
			if ((g_app_notification_msg.type == APP_MSG_TYPE_CALL_IOS) ||
				(g_app_notification_msg.type == APP_MSG_TYPE_CALL))
				{
				timer_notify_display_stop(false);
				timer_notify_motor_stop(false);
				timer_notify_buzzer_stop(false); 
				DISPLAY_MSG  msg = {0,0};
				ScreenState = ScreenStateSave;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				ble_uart_ccommand_application_phone_call_ctrl(IGNORE_PHONE_CALL);
				
			}
		#endif
		}
		break;

		
		case (KEY_BACK):
		{
			timer_notify_display_stop(false);
			timer_notify_motor_stop(false);
			timer_notify_buzzer_stop(false);
			DISPLAY_MSG  msg = {0,0};
			ScreenState = ScreenStateSave;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			#ifdef COD 
			if ((g_app_notification_msg.type == APP_MSG_TYPE_CALL_IOS) ||
				(g_app_notification_msg.type == APP_MSG_TYPE_CALL))
				{
				 ble_uart_ccommand_application_phone_call_ctrl(REJECT_PHONE_CALL);
			}
			#endif 
		}
			break;		
	}
}

