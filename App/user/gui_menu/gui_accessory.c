#include "gui_accessory.h"
#include "SEGGER_RTT.h"
#include "drv_key.h"
#include "drv_lcd.h"
#include <string.h>
#include "font_displaystrs.h"
#include "task_display.h"
#include "com_data.h"
#include "img_sport.h"
#include "img_menu.h"
#include "time_progress.h"
#include "lib_error.h"


#include "FreeRTOS.h"
#include "task.h"
#include "portmacro.h"
#include "portable.h"
#include "semphr.h"
#include "event_groups.h"


#include "cmd_ble.h"
#include "drv_ble.h"
#include "task_ble.h"

#include "gui_notify_warning.h"
#include "time_notify.h"
#include "img_tool.h"

#define GUI_ACCESSORY_LOG_ENABLED 1

#if DEBUG_ENABLED == 1 && GUI_ACCESSORY_LOG_ENABLED == 1
	#define GUI_ACCESSORY_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_ACCESSORY_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define GUI_ACCESSORY_WRITESTRING(...)
	#define GUI_ACCESSORY_PRINTF(...)		        
#endif

extern SetValueStr SetValue;
#define BACKGROUND_COLOR      LCD_WHITE
#define FOREGROUND_COLOR      LCD_BLACK

//#define DELAY_MS(X)                 am_util_delay_ms(X)
#define DELAY_MS(X)                 vTaskDelay(X / portTICK_PERIOD_MS)
//#define DELAY_MS(X)                   do{TickType_t xLastWakeTime;xLastWakeTime = xTaskGetTickCount ();vTaskDelayUntil( &xLastWakeTime, X / portTICK_PERIOD_MS );}while(0)



#define GUI_ACCESSORY_ENABLE                      0
#define GUI_ACCESSORY_HEARTRATE                   1
#define GUI_ACCESSORY_RUNNING_CADENCE             2
#define GUI_ACCESSORY_CYCLING                     3


static uint8_t m_gui_accessory_index = 0;



_ble_accessory g_ble_accessory = 
{
	.enable = false,
	.running = {
		.status = ACCESSORY_STATUS_DISCONNECT,
	},
	.heartrate = {
		.status = ACCESSORY_STATUS_DISCONNECT,
	},
	.cycling = {
		.status = ACCESSORY_STATUS_DISCONNECT,
	},	
};


static void menu( char* menu1, char* menu2, char* menu3, char* option, const LCD_ImgStruct *pic)
{
	SetWord_t word = {0};
	word.x_axis = 32;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	
	 
	 word.forecolor = LCD_LIGHTGRAY;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;
	if((m_gui_accessory_index==GUI_ACCESSORY_ENABLE)&&(g_ble_accessory.enable==0));//关闭配件连接只显示关闭和图标
	else
		{
	       LCD_SetString(menu1,&word);	//配件连接或开启
	       LCD_SetRectangle(120 - 24/2 - 36 - 1 ,2,LCD_LINE_CNT_MAX/2 - 90,180,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
		}
 
	
	if(g_ble_accessory.enable==0)//关闭连接；关闭配件连接只显示关闭和图标
		{
         word.x_axis = 120 - 24/2;
        word.y_axis = LCD_CENTER_JUSTIFIED;
	     word.size = LCD_FONT_24_SIZE;
	     word.forecolor = LCD_WHITE;
	     word.bckgrndcolor = LCD_NONE;
	     word.kerning = 0;
		 LCD_SetString(menu2,&word);	//配件连接

	    if(pic != NULL)
	     {
		  LCD_SetPicture(120 - 34/2, 200, LCD_NONE, LCD_NONE, pic);				
	      }
	   }
	else   //开启
		{

		
		  LCD_SetRectangle(120 + 24/2 + 36 - 1,2,LCD_LINE_CNT_MAX/2 - 90,180,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);


			 if(option != NULL)
			  {
				 word.x_axis = 120 - (24 + 8 + 16)/2;
				 word.y_axis = LCD_CENTER_JUSTIFIED;
				 word.forecolor = LCD_WHITE;
		         word.bckgrndcolor = LCD_NONE;
				 word.size = LCD_FONT_24_SIZE;
		         word.kerning = 0;
	            LCD_SetString(menu2,&word);	//更改使用字库字体
				word.x_axis += 24 + 8;
				word.y_axis = LCD_CENTER_JUSTIFIED;
				word.forecolor = LCD_WHITE;
			    word.bckgrndcolor = LCD_NONE;
				word.size = LCD_FONT_16_SIZE;		
				LCD_SetString(option,&word);				
			 }
			else
				{

				 word.x_axis = 120 - 24/2;
				 word.y_axis = LCD_CENTER_JUSTIFIED;
				 word.forecolor = LCD_WHITE;
		         word.bckgrndcolor = LCD_NONE;
				 word.size = LCD_FONT_24_SIZE;
		         word.kerning = 0;
	             LCD_SetString(menu2,&word);	//更改使用字库字体


				  }
	
		   if(pic != NULL)
		     {
			LCD_SetPicture(103, 200, LCD_NONE, LCD_NONE, pic);				
		      }	
		  	




		     word.x_axis = 120 + 24/2 + 36 + 24;
			 word.forecolor = LCD_LIGHTGRAY;
	         word.bckgrndcolor = LCD_NONE;
			 word.size = LCD_FONT_16_SIZE;
		     LCD_SetString(menu3,&word);  //更改使用字库字体
			
           

	   }
	
	
}

void gui_accessory_paint(void)
{
	LCD_SetBackgroundColor(LCD_BLACK);
	char p_str[40];
	switch(m_gui_accessory_index)
	{
		case GUI_ACCESSORY_ENABLE:{
			if(SetValue.Language == L_CHINESE)
			{
				if(g_ble_accessory.enable)
				{
					menu((char *)AccessoryStrs[SetValue.Language][GUI_ACCESSORY_INDEX_MAX -1],(char *)AccessoryStrs[SetValue.Language][m_gui_accessory_index],(char *)AccessoryStrs[SetValue.Language][m_gui_accessory_index + 1],
						NULL,&img_v_switch_on);	
					
				}
				else
				{
					
					menu((char *)AccessoryStrs[SetValue.Language][GUI_ACCESSORY_INDEX_MAX -1],"关闭",NULL,NULL,&img_v_switch_off);
				
				}
			}
			else if(SetValue.Language == L_ENGLISH)
			{

					if(g_ble_accessory.enable)
					{
						menu((char *)AccessoryStrs[SetValue.Language][m_gui_accessory_index -1],
									(char *)AccessoryStrs[SetValue.Language][m_gui_accessory_index],
									(char *)AccessoryStrs[SetValue.Language][m_gui_accessory_index + 1],
									"ON",&img_v_switch_on);						
					}
					else{
						menu((char *)AccessoryStrs[SetValue.Language][m_gui_accessory_index -1],
									(char *)AccessoryStrs[SetValue.Language][m_gui_accessory_index],
									NULL,"OFF",&img_v_switch_off);			
					}		
			}			
		}break;		
		case GUI_ACCESSORY_HEARTRATE:{
			 LCD_SetPicture(10, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Up_12X8);
			 
			 LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Down_12X8);
			if(SetValue.Language == L_CHINESE)
			{
				if(g_ble_accessory.heartrate.status == ACCESSORY_STATUS_CONNECT)
				{
					memset(p_str,0,sizeof(p_str));
					sprintf(p_str,"已连接:%02X:%02X:%02X:%02X:%02X:%02X",g_ble_accessory.heartrate.mac[0],
																																g_ble_accessory.heartrate.mac[1],
																																g_ble_accessory.heartrate.mac[2],
																																g_ble_accessory.heartrate.mac[3],
																																g_ble_accessory.heartrate.mac[4],
																																g_ble_accessory.heartrate.mac[5]);					
					menu((char *)AccessoryStrs[SetValue.Language][m_gui_accessory_index -1],
						
								(char *)AccessoryStrs[SetValue.Language][m_gui_accessory_index],
								(char *)AccessoryStrs[SetValue.Language][m_gui_accessory_index + 1],p_str,NULL);					
				}
				else if(g_ble_accessory.heartrate.status == ACCESSORY_STATUS_DISCONNECT)
				{
					menu((char *)AccessoryStrs[SetValue.Language][m_gui_accessory_index -1],
								(char *)AccessoryStrs[SetValue.Language][m_gui_accessory_index],
								(char *)AccessoryStrs[SetValue.Language][m_gui_accessory_index + 1],"搜索中...",NULL);					
				}
				
		
			}
			else if(SetValue.Language == L_ENGLISH)
			{
				if(g_ble_accessory.heartrate.status == ACCESSORY_STATUS_CONNECT)
				{
					memset(p_str,0,sizeof(p_str));
					sprintf(p_str,"CONNECT:%02X:%02X:%02X:%02X:%02X:%02X",g_ble_accessory.heartrate.mac[0],
																																g_ble_accessory.heartrate.mac[1],
																																g_ble_accessory.heartrate.mac[2],
																																g_ble_accessory.heartrate.mac[3],
																																g_ble_accessory.heartrate.mac[4],
																																g_ble_accessory.heartrate.mac[5]);
					menu((char *)AccessoryStrs[SetValue.Language][m_gui_accessory_index -1],
								(char *)AccessoryStrs[SetValue.Language][m_gui_accessory_index],
								(char *)AccessoryStrs[SetValue.Language][m_gui_accessory_index + 1],p_str,NULL);					
				}
				else if(g_ble_accessory.heartrate.status == ACCESSORY_STATUS_DISCONNECT)
				{
					menu((char *)AccessoryStrs[SetValue.Language][m_gui_accessory_index -1],
								(char *)AccessoryStrs[SetValue.Language][m_gui_accessory_index],
								(char *)AccessoryStrs[SetValue.Language][m_gui_accessory_index + 1],"SEARCHING...",NULL);					
				}				
					
			}
							
		}break;
		case GUI_ACCESSORY_RUNNING_CADENCE:{
			 LCD_SetPicture(10, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Up_12X8);
			 
			 LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Down_12X8);
			if(SetValue.Language == L_CHINESE)
			{
				if(g_ble_accessory.running.status == ACCESSORY_STATUS_CONNECT)
				{
					memset(p_str,0,sizeof(p_str));
					sprintf(p_str,"已连接:%02X:%02X:%02X:%02X:%02X:%02X",g_ble_accessory.running.mac[0],
																																g_ble_accessory.running.mac[1],
																																g_ble_accessory.running.mac[2],
																																g_ble_accessory.running.mac[3],
																																g_ble_accessory.running.mac[4],
																																g_ble_accessory.running.mac[5]);					
					menu((char *)AccessoryStrs[SetValue.Language][m_gui_accessory_index -1],
								(char *)AccessoryStrs[SetValue.Language][m_gui_accessory_index],
								(char *)AccessoryStrs[SetValue.Language][m_gui_accessory_index + 1],p_str,NULL);					
				}
				else if(g_ble_accessory.running.status == ACCESSORY_STATUS_DISCONNECT)
				{
					menu((char *)AccessoryStrs[SetValue.Language][m_gui_accessory_index -1],
								(char *)AccessoryStrs[SetValue.Language][m_gui_accessory_index],
								(char *)AccessoryStrs[SetValue.Language][m_gui_accessory_index + 1],"搜索中...",NULL);					
				}					
			}
			else if(SetValue.Language == L_ENGLISH)
			{
				if(g_ble_accessory.running.status == ACCESSORY_STATUS_CONNECT)
				{
					memset(p_str,0,sizeof(p_str));
					sprintf(p_str,"CONNECT:%02X:%02X:%02X:%02X:%02X:%02X",g_ble_accessory.running.mac[0],
																																g_ble_accessory.running.mac[1],
																																g_ble_accessory.running.mac[2],
																																g_ble_accessory.running.mac[3],
																																g_ble_accessory.running.mac[4],
																																g_ble_accessory.running.mac[5]);
					menu((char *)AccessoryStrs[SetValue.Language][m_gui_accessory_index -1],
								(char *)AccessoryStrs[SetValue.Language][m_gui_accessory_index],
								(char *)AccessoryStrs[SetValue.Language][m_gui_accessory_index + 1],p_str,NULL);					
				}
				else if(g_ble_accessory.heartrate.status == ACCESSORY_STATUS_DISCONNECT)
				{
					menu((char *)AccessoryStrs[SetValue.Language][m_gui_accessory_index -1],
								(char *)AccessoryStrs[SetValue.Language][m_gui_accessory_index],
								(char *)AccessoryStrs[SetValue.Language][m_gui_accessory_index + 1],"SEARCHING...",NULL);					
				}				
					
			}			
		}break;				
		
		case GUI_ACCESSORY_CYCLING:{
			 LCD_SetPicture(10, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Up_12X8);
			if(SetValue.Language == L_CHINESE)
			{
				if(g_ble_accessory.cycling.status == ACCESSORY_STATUS_CONNECT)
				{
					memset(p_str,0,sizeof(p_str));
					sprintf(p_str,"已连接:%02X:%02X:%02X:%02X:%02X:%02X",g_ble_accessory.cycling.mac[0],
																																g_ble_accessory.cycling.mac[1],
																																g_ble_accessory.cycling.mac[2],
																																g_ble_accessory.cycling.mac[3],
																																g_ble_accessory.cycling.mac[4],
																																g_ble_accessory.cycling.mac[5]);					
					menu((char *)AccessoryStrs[SetValue.Language][m_gui_accessory_index -1],
								(char *)AccessoryStrs[SetValue.Language][m_gui_accessory_index],
								(char *)AccessoryStrs[SetValue.Language][0],p_str,NULL);					
				}
				else if(g_ble_accessory.cycling.status == ACCESSORY_STATUS_DISCONNECT)
				{
					menu((char *)AccessoryStrs[SetValue.Language][m_gui_accessory_index -1],
								(char *)AccessoryStrs[SetValue.Language][m_gui_accessory_index],
								(char *)AccessoryStrs[SetValue.Language][0],"搜索中...",NULL);					
				}					
			}
			else if(SetValue.Language == L_ENGLISH)
			{
				if(g_ble_accessory.cycling.status == ACCESSORY_STATUS_CONNECT)
				{
					memset(p_str,0,sizeof(p_str));
					sprintf(p_str,"CONNECT:%02X:%02X:%02X:%02X:%02X:%02X",g_ble_accessory.cycling.mac[0],
																																g_ble_accessory.cycling.mac[1],
																																g_ble_accessory.cycling.mac[2],
																																g_ble_accessory.cycling.mac[3],
																																g_ble_accessory.cycling.mac[4],
																																g_ble_accessory.cycling.mac[5]);
					menu((char *)AccessoryStrs[SetValue.Language][m_gui_accessory_index -1],
								(char *)AccessoryStrs[SetValue.Language][m_gui_accessory_index],
								NULL,p_str,NULL);					
				}
				else if(g_ble_accessory.cycling.status == ACCESSORY_STATUS_DISCONNECT)
				{
					menu((char *)AccessoryStrs[SetValue.Language][m_gui_accessory_index -1],
								(char *)AccessoryStrs[SetValue.Language][m_gui_accessory_index],
								NULL,"SEARCHING...",NULL);					
				}				
					
			}						
		}break;						
		
	}


	if(g_ble_accessory.enable)
		{
          LCD_SetPicture(10, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Up_12X8);
          LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Down_12X8);

      }
	//LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
}

void gui_accessory_btn_evt(uint32_t evt)
{	

	switch(evt)
	{		
		case (KEY_BACK):{
			m_gui_accessory_index = 0;
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_MENU;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
		}break;			
		case (KEY_UP):{			
			if(g_ble_accessory.enable)
			{
			
				if(m_gui_accessory_index <= 0)
				{
					m_gui_accessory_index = GUI_ACCESSORY_INDEX_MAX - 1;
				}else{
					m_gui_accessory_index--;
				}
				DISPLAY_MSG  msg = {0,0};
				ScreenState = DISPLAY_SCREEN_ACCESSORY;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
			}
		}break;	
			
		case (KEY_OK):{
			switch(m_gui_accessory_index)
			{
				case GUI_ACCESSORY_ENABLE:{			
					if(g_ble_accessory.enable)
					{
						g_ble_accessory.enable = false;
						
						DISPLAY_MSG  msg = {0,0};
						ScreenState = DISPLAY_SCREEN_ACCESSORY;
						msg.cmd = MSG_DISPLAY_SCREEN;
						xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
						
						uint32_t command[1] = {BLE_UART_CCOMMAND_APPLICATION_SCAN_DISABLE | BLE_UART_CCOMMAND_MASK_APPLICATION};
						GUI_ACCESSORY_PRINTF("BLE_UART_CCOMMAND_APPLICATION_SCAN_DISABLE\r\n");
						_uart_data_param param;
						param.user_callback = NULL;
						param.p_data = (uint8_t *)command;
						param.length = sizeof(command);
						uint8_t count = TASK_UART_DELAY_COUNT_OUT;
					#ifndef WATCH_HAS_NO_BLE 
						while(drv_ble_send_data(&param) != ERR_SUCCESS && count--)
						{
							DELAY_MS(TASK_UART_DELAY);
						}		
					#endif
						g_ble_accessory.running.status = ACCESSORY_STATUS_DISCONNECT;
						g_ble_accessory.cycling.status = ACCESSORY_STATUS_DISCONNECT;
						g_ble_accessory.heartrate.status = ACCESSORY_STATUS_DISCONNECT;
						g_ble_accessory.data.cycle_data_valid = false;
						g_ble_accessory.data.hr_data_valid = false;
						g_ble_accessory.data.run_data_valid = false;											
					}
					else
					{
						
						if(SetValue.SwBle != false )
						{
							SetValue.SwBle = BLE_DISCONNECT;
							g_ble_accessory.enable = true;

							DISPLAY_MSG  msg = {0,0};
							ScreenState = DISPLAY_SCREEN_ACCESSORY;
							msg.cmd = MSG_DISPLAY_SCREEN;
							xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
							
							uint32_t command[1] = {BLE_UART_CCOMMAND_APPLICATION_SCAN_ENABLE | BLE_UART_CCOMMAND_MASK_APPLICATION};
							GUI_ACCESSORY_PRINTF("BLE_UART_CCOMMAND_APPLICATION_SCAN_ENABLE\r\n");
							_uart_data_param param;
							param.user_callback = NULL;
							param.p_data = (uint8_t *)command;
							param.length = sizeof(command);
							uint8_t count = TASK_UART_DELAY_COUNT_OUT;
						#ifndef WATCH_HAS_NO_BLE 
							while(drv_ble_send_data(&param) != ERR_SUCCESS && count--)
							{
								DELAY_MS(TASK_UART_DELAY);
							}	
						#endif														
						}
						else{
						
							
							timer_notify_display_start(2000,1,false);
							DISPLAY_MSG  msg = {0,0};
							ScreenStateSave = ScreenState;
							ScreenState = DISPLAY_SCREEN_NOTIFY_WARNING;
							msg.cmd = MSG_DISPLAY_SCREEN;
							msg.value = GUI_NOTIFY_WARNING_INDEX_ACCESSORY_BLE_OFF;
							xQueueSend(DisplayQueue, &msg, portMAX_DELAY);							
							
							
						}
						
							
					}

				}break;				
				
				
//				case GUI_ACCESSORY_HEARTRATE:{			
//					DISPLAY_MSG  msg = {0,0};
//					ScreenState = DISPLAY_SCREEN_ACCESSORY_HEARTRATE;
//					msg.cmd = MSG_DISPLAY_SCREEN;
//					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
//				}break;
//				case GUI_ACCESSORY_RUNNING_CADENCE:{
//					DISPLAY_MSG  msg = {0,0};
//					ScreenState = DISPLAY_SCREEN_ACCESSORY_RUNNING;
//					msg.cmd = MSG_DISPLAY_SCREEN;
//					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);				
//				}break;		
//				case GUI_ACCESSORY_CYCLING_WHEEL:{
//					DISPLAY_MSG  msg = {0,0};
//					ScreenState = DISPLAY_SCREEN_ACCESSORY_CYCLING_WHEEL;
//					msg.cmd = MSG_DISPLAY_SCREEN;
//					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);		
//				}break;
//				case GUI_ACCESSORY_CYCLING_CADENCE:{
//					DISPLAY_MSG  msg = {0,0};
//					ScreenState = DISPLAY_SCREEN_ACCESSORY_CYCLING_CADENCE;
//					msg.cmd = MSG_DISPLAY_SCREEN;
//					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);				
//				}break;	
			}	

		}break;		
		case (KEY_DOWN):{			
			if(g_ble_accessory.enable)
			{
				if(m_gui_accessory_index >= (GUI_ACCESSORY_INDEX_MAX - 1))
				{
					m_gui_accessory_index = 0;
				}else{
					m_gui_accessory_index++;
				}
				DISPLAY_MSG  msg = {0,0};
				ScreenState = DISPLAY_SCREEN_ACCESSORY;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
			}
		}break;					
	}
}





void gui_accessory_heartrate_paint(void)
{
	LCD_SetBackgroundColor(LCD_BLACK);

}


void gui_accessory_heartrate_btn_evt(uint32_t evt)
{

	switch(evt)
	{		
		case (KEY_BACK):{
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_ACCESSORY;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);				
		}break;			
		case (KEY_UP):{
						
		}break;	
		case (KEY_OK):{
			
		}break;		
		case (KEY_DOWN):{
					
		}break;			
	}

}




void gui_accessory_running_paint(void)
{
	LCD_SetBackgroundColor(LCD_BLACK);

}


void gui_accessory_running_btn_evt(uint32_t evt)
{

	switch(evt)
	{		
		case (KEY_BACK):{
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_ACCESSORY;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);				
		}break;			
		case (KEY_UP):{
						
		}break;	
		case (KEY_OK):{
			
		}break;		
		case (KEY_DOWN):{
					
		}break;			
	}

}




void gui_accessory_cycling_wheel_paint(void)
{
	LCD_SetBackgroundColor(LCD_BLACK);

}


void gui_accessory_cycling_wheel_btn_evt(uint32_t evt)
{

	switch(evt)
	{		
		case (KEY_BACK):{
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_ACCESSORY;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);				
		}break;			
		case (KEY_UP):{
						
		}break;	
		case (KEY_OK):{
			
		}break;		
		case (KEY_DOWN):{
					
		}break;			
	}

}




void gui_accessory_cycling_cadence_paint(void)
{
	LCD_SetBackgroundColor(LCD_BLACK);

}


void gui_accessory_cycling_cadence_btn_evt(uint32_t evt)
{

	switch(evt)
	{		
		case (KEY_BACK):{
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_ACCESSORY;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);				
		}break;			
		case (KEY_UP):{
						
		}break;	
		case (KEY_OK):{
			
		}break;		
		case (KEY_DOWN):{
					
		}break;			
	}

}

