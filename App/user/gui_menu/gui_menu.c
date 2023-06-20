#include "gui_menu.h"
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
#include "drv_battery.h"

#include "task_ble.h"
#include "img_tool.h"
#include "img_dial.h"

#include "lib_app_data.h"
#include "gui_motionsrecord.h"
#include "gui_sports_record.h"
#include "com_sport.h"
#include "time_notify.h"
#include "gui_notify_warning.h"
#include "img_menu.h"


#define GUI_MENU_LOG_ENABLED 1

#if DEBUG_ENABLED == 1 && GUI_MENU_LOG_ENABLED == 1
	#define GUI_MENU_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_MENU_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define GUI_MENU_WRITESTRING(...)
	#define GUI_MENU_PRINTF(...)		        
#endif

extern _drv_bat_event   g_bat_evt;
extern SetValueStr SetValue;
#define BACKGROUND_COLOR      LCD_WHITE
#define FOREGROUND_COLOR      LCD_BLACK
//#define DELAY_MS(X)                 am_util_delay_ms(X)
#define DELAY_MS(X)                 vTaskDelay(X / portTICK_PERIOD_MS)
//#define DELAY_MS(X)                   do{TickType_t xLastWakeTime;xLastWakeTime = xTaskGetTickCount ();vTaskDelayUntil( &xLastWakeTime, X / portTICK_PERIOD_MS );}while(0)

extern _task_ble_info g_task_info;
static uint8_t m_menu_index = 0;
const ScreenState_t m_select_menu_buf[GUI_MENU_INDEX_MAX] = {  
	    DISPLAY_SCREEN_THEME, DISPLAY_SCREEN_TIME,  
		DISPLAY_SCREEN_SYSTEM, DISPLAY_SCREEN_CUSTOMIZED_SPORTS,  
		DISPLAY_SCREEN_CUSTOMIZED_TOOLS, DISPLAY_SCREEN_ACCESSORY,  
		DISPLAY_SCREEN_WATCH_INFO};

static void menu( char* menu1, char* menu2, char* menu3,uint8_t index)
{
	uint8_t percent ;
	
	SetWord_t word = {0};
	percent = g_bat_evt.level_percent;
    //上翻提示
	gui_page_up(LCD_PERU);
		//下翻提示
	gui_page_down(LCD_PERU);

		
    LCD_SetRectangle(120 - 24/2 - 36 - 1 ,2,LCD_LINE_CNT_MAX/2 - 90,180,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
	LCD_SetRectangle(120 + 24/2 + 36 - 1,2,LCD_LINE_CNT_MAX/2 - 90,180,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
	//LCD_SetPicture(10, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Up_12X8);
   // LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Down_12X8);

		

		word.x_axis = 120 - 24/2 - 36 - 16 - 16 - 6 - img_dial_bat_28x13.height;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		 
	   LCD_SetPicture(word.x_axis,word.y_axis,LCD_WHITE,LCD_NONE,&img_dial_bat_28x13);
	   	if(percent > 100)
		{
			percent = 100;
		}
    	if(percent <= 10)
			{
				word.forecolor = LCD_RED;
			}
			else
			{
				word.forecolor = LCD_WHITE;
			}
		
		LCD_SetRectangle(word.x_axis + 1 ,img_dial_bat_28x13.height-2,120 - img_dial_bat_28x13.width/2 + 1,percent*(img_dial_bat_28x13.width - 1 - 2)/100 ,word.forecolor,0,0,LCD_FILL_ENABLE);
   
/*第一栏*/
   // word.x_axis = 120 - 24/2 - 36 - 24 - 16;
	word.x_axis = 120 - 24/2 - 36 - 16 - 16;

    word.y_axis = LCD_PIXEL_ONELINE_MAX/2 - (16*strlen(menu1)/2 + 6 + img_set_menu_l[0].width)/2 + img_set_menu_l[0].width + 6 ;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_LIGHTGRAY;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;
	
	LCD_SetString(menu1,&word);	//更改使用字库字体
	if(index==0)
		{
	//LCD_SetPicture(word.x_axis,120 - strlen(menu1)*LCD_FONT_16_SIZE/4 - 6 - img_set_menu_l[GUI_MENU_INDEX_MAX-1].width,LCD_LIGHTGRAY,LCD_NONE,&img_set_menu_l[GUI_MENU_INDEX_MAX-1]);
	LCD_SetPicture(word.x_axis,LCD_PIXEL_ONELINE_MAX/2 - (16*strlen(menu1)/2 + 6 + img_set_menu_l[GUI_MENU_INDEX_MAX-1].width)/2,LCD_LIGHTGRAY,LCD_NONE,&img_set_menu_l[GUI_MENU_INDEX_MAX-1]);
		}
	else
		{
        
	//	LCD_SetPicture(word.x_axis,120 - strlen(menu1)*LCD_FONT_16_SIZE/4 - 6 - img_set_menu_l[index-1].width,LCD_LIGHTGRAY,LCD_NONE,&img_set_menu_l[index-1]);
		
		LCD_SetPicture(word.x_axis,LCD_PIXEL_ONELINE_MAX/2 - (16*strlen(menu1)/2 + 6 + img_set_menu_l[index-1].width)/2,LCD_LIGHTGRAY,LCD_NONE,&img_set_menu_l[index-1]);
	  }




	
	/*第二栏*/
	word.x_axis = 120 - 24/2;	 //24号字体对齐线
	word.y_axis = LCD_PIXEL_ONELINE_MAX/2 - (24*strlen(menu2)/2 + 8 + img_set_menu_b[0].width)/2 + img_set_menu_b[index].width + 8 ;
	word.forecolor = LCD_CYAN;
	word.bckgrndcolor = LCD_NONE;
	word.size = LCD_FONT_24_SIZE;
	//LCD_DisplayGTString(menu2,&word);	//更改使用字库字体
	LCD_SetString(menu2,&word);
   // LCD_SetPicture(word.x_axis,120 - strlen(menu2)*LCD_FONT_24_SIZE/4 - 8 - img_set_menu_b[index].width,LCD_RED,LCD_NONE,&img_set_menu_b[index]);
	
    LCD_SetPicture(word.x_axis,LCD_PIXEL_ONELINE_MAX/2 - (24*strlen(menu2)/2 + 8 + img_set_menu_b[index].width)/2,LCD_RED,LCD_NONE,&img_set_menu_b[index]);


	
	/*第三栏*/
	word.x_axis = 120 + 24/2 + 36 + 24;
	word.y_axis = LCD_PIXEL_ONELINE_MAX/2 - (16*strlen(menu3)/2 + 6 + img_set_menu_l[0].width)/2 + img_set_menu_l[0].width + 6;
	word.forecolor = LCD_LIGHTGRAY;
	word.bckgrndcolor = LCD_NONE;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString(menu3,&word);		//更改使用字库字体
     if(index== (GUI_MENU_INDEX_MAX-1))
 	 {
    
	 // LCD_SetPicture(word.x_axis,120 - strlen(menu3)*LCD_FONT_16_SIZE/4 - 6 - img_set_menu_l[0].width,LCD_LIGHTGRAY,LCD_NONE,&img_set_menu_l[0]);
	  
	  LCD_SetPicture(word.x_axis,LCD_PIXEL_ONELINE_MAX/2 - (16*strlen(menu3)/2 + 6 + img_set_menu_l[0].width)/2,LCD_LIGHTGRAY,LCD_NONE,&img_set_menu_l[0]);

      }
    
    else
    {
	//LCD_SetPicture(word.x_axis,120 - strlen(menu3)*LCD_FONT_16_SIZE/4 - 6 - img_set_menu_l[index+1].width,LCD_LIGHTGRAY,LCD_NONE,&img_set_menu_l[index+1]);
	
	LCD_SetPicture(word.x_axis,LCD_PIXEL_ONELINE_MAX/2 - (16*strlen(menu3)/2 + 6 + img_set_menu_l[index+1].width)/2,LCD_LIGHTGRAY,LCD_NONE,&img_set_menu_l[index+1]);

     }
		
	

}

void gui_menu_battery_paint(void)
{
	SetWord_t word = {0};
	uint8_t percent = g_bat_evt.level_percent;
	uint8_t color;
	char str[20] = {0};

	if(percent > 100)
	{
		percent = 100;
	}
	memset(str,0,sizeof(str));
	sprintf(str,"%d", percent);
	
	//百分比
	word.x_axis = 35;
	word.y_axis = (LCD_PIXEL_ONELINE_MAX - strlen(str)*8 - 14 - Img_Battery_32X15.width) / 2;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	LCD_SetNumber(str,&word);
	word.y_axis +=(strlen(str)*8 + 6);
	LCD_SetString("%",&word);
	
	word.x_axis = 35;
	word.y_axis +=12;
	LCD_SetPicture(word.x_axis, word.y_axis, LCD_WHITE, LCD_NONE, &Img_Battery_32X15);


	
	if(percent <= 10)
	{
		color = LCD_RED;
	}
	else
	{
		color = LCD_WHITE;
	}
	
	//填充剩余电量
	LCD_SetRectangle(word.x_axis + 2, 11, word.y_axis + 2, percent * (Img_Battery_32X15.width - 6) / 100, color, 0, 0, LCD_FILL_ENABLE);
		
}

void gui_menu_paint(void)
{
	GUI_MENU_PRINTF("[GUI_MENU]:gui_menu_paint\n");
	uint8_t percent;
	uint8_t color;
	SetWord_t word = {0};
	LCD_SetBackgroundColor(LCD_BLACK);
	
    switch(m_select_menu_buf[m_menu_index])
	{
		case DISPLAY_SCREEN_THEME:
			{
			GUI_MENU_PRINTF("[GUI_MENU]:DISPLAY_SCREEN_THEME\n");
			menu((char *)MenuStrs[SetValue.Language][GUI_MENU_INDEX_MAX - 1],
						(char *)MenuStrs[SetValue.Language][m_menu_index],
						(char *)MenuStrs[SetValue.Language][m_menu_index + 1],m_menu_index);
		
		  }
		break;
		case DISPLAY_SCREEN_TIME:{
			GUI_MENU_PRINTF("[GUI_MENU]:DISPLAY_SCREEN_TIME\n");
			menu((char *)MenuStrs[SetValue.Language][m_menu_index - 1],
						(char *)MenuStrs[SetValue.Language][m_menu_index],
						(char *)MenuStrs[SetValue.Language][m_menu_index + 1],m_menu_index);				
		}break;		
	case DISPLAY_SCREEN_SYSTEM:{
			GUI_MENU_PRINTF("[GUI_MENU]:DISPLAY_SCREEN_SYSTEM\n");
			menu((char *)MenuStrs[SetValue.Language][m_menu_index - 1],
						(char *)MenuStrs[SetValue.Language][m_menu_index],
						(char *)MenuStrs[SetValue.Language][m_menu_index + 1],m_menu_index);				
		}break;


  	case DISPLAY_SCREEN_CUSTOMIZED_SPORTS:
			{
			GUI_MENU_PRINTF("[GUI_MENU]:DISPLAY_SCREEN_CUSTOMIZED_SPORTS\n");
			menu((char *)MenuStrs[SetValue.Language][m_menu_index - 1],
						(char *)MenuStrs[SetValue.Language][m_menu_index],
						(char *)MenuStrs[SetValue.Language][m_menu_index + 1],m_menu_index);
		
		  }
		break;
		case DISPLAY_SCREEN_CUSTOMIZED_TOOLS:
			{
			GUI_MENU_PRINTF("[GUI_MENU]:DISPLAY_SCREEN_CUSTOMIZED_TOOLS\n");
			menu((char *)MenuStrs[SetValue.Language][m_menu_index - 1],
						(char *)MenuStrs[SetValue.Language][m_menu_index],
						(char *)MenuStrs[SetValue.Language][m_menu_index + 1],m_menu_index);
		
		  }
		break;

		case DISPLAY_SCREEN_ACCESSORY:{
			GUI_MENU_PRINTF("[GUI_MENU]:DISPLAY_SCREEN_ACCESSORY\n");
			menu((char *)MenuStrs[SetValue.Language][m_menu_index - 1],
						(char *)MenuStrs[SetValue.Language][m_menu_index],
						(char *)MenuStrs[SetValue.Language][m_menu_index + 1],m_menu_index);				
		}break;	
		
		case DISPLAY_SCREEN_WATCH_INFO:{
			GUI_MENU_PRINTF("[GUI_MENU]:DISPLAY_SCREEN_WATCH_INFO\n");
			menu((char *)MenuStrs[SetValue.Language][m_menu_index - 1],
						(char *)MenuStrs[SetValue.Language][m_menu_index],
						(char *)MenuStrs[SetValue.Language][0],m_menu_index);
			
		}break;
      default:
	  	break;
	
	}
  


	
	
	
}

void gui_menu_init(void)
{
	//默认为主题选项
	m_menu_index = 0;
}

void gui_menu_btn_evt(uint32_t evt)
{
	switch(evt)
	{		
		case (KEY_BACK):{
			GUI_MENU_PRINTF("[GUI_MENU]:KEY_BACK\n");
			m_menu_index = 0;
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_HOME;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
		}break;			
		case (KEY_UP):{
			GUI_MENU_PRINTF("[GUI_MENU]:KEY_UP\n");
			if(m_menu_index <= 0)
			{
				m_menu_index = GUI_MENU_INDEX_MAX - 1;
			}else{
				m_menu_index--;
			}
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_MENU;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}break;	
	case (KEY_DOWN):{
			GUI_MENU_PRINTF("[GUI_MENU]:KEY_DOWN\n");
			if(m_menu_index >= ( GUI_MENU_INDEX_MAX - 1))
			{
				m_menu_index = 0;
			}else{
				m_menu_index++;
			}
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_MENU;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}break;	
		case (KEY_OK):{
			
            switch(m_select_menu_buf[m_menu_index])
            {
				case DISPLAY_SCREEN_THEME:{			
					DISPLAY_MSG  msg = {0,0};
					
					//初始化主题信息
					gui_theme_init(1);
					
					ScreenState = DISPLAY_SCREEN_THEME;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				}break;
				case DISPLAY_SCREEN_TIME:{
					DISPLAY_MSG  msg = {0,0};
					ScreenState = DISPLAY_SCREEN_TIME;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);				
				}break;	
					case DISPLAY_SCREEN_SYSTEM:{
					DISPLAY_MSG  msg = {0,0};
					ScreenState = DISPLAY_SCREEN_SYSTEM;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);				
				}break;
				/*case DISPLAY_SCREEN_SPORTS_RECORD:{
#if defined(WATCH_SPORT_RECORD_TEST)
					//查询遍历运动记录 
					if(search_sport_record() != 0)
					{
						data_up_down_flag = 0;
						GetSportDetail(0,1);
					}		
					DISPLAY_MSG  msg = {0,0};
					data_up_down_flag = 0;
					ScreenState = DISPLAY_SCREEN_SPORTS_RECORD;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
					
#else
					DISPLAY_MSG  msg = {0,0};
					data_up_down_flag = 0;
					g_motion_record_addr = ActivityAddressForwardLookup(DAY_MAIN_DATA_START_ADDRESS);//获取最新活动数据地址
					ActivityDataResult = ActivityDataReverseLookup(g_motion_record_addr, Sport_Record);
					sport_data_addr[0] = ActivityDataResult.address1;
					sport_data_addr[1] = ActivityDataResult.address0;
					GetSportDetail(sport_data_addr[0],sport_data_addr[1]);
					ScreenState = DISPLAY_SCREEN_SPORTS_RECORD;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);		
#endif
				}break;*/

				case DISPLAY_SCREEN_CUSTOMIZED_SPORTS:{
					DISPLAY_MSG  msg = {0,0};
					ScreenState = DISPLAY_SCREEN_CUSTOMIZED_SPORTS;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);				
				}break;
				
				case DISPLAY_SCREEN_CUSTOMIZED_TOOLS:{
					DISPLAY_MSG  msg = {0,0};
					ScreenState = DISPLAY_SCREEN_CUSTOMIZED_TOOLS;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);				
				}break;	
			

				case DISPLAY_SCREEN_ACCESSORY:{
					DISPLAY_MSG  msg = {0,0};
					ScreenState = DISPLAY_SCREEN_ACCESSORY;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);				
				}break;	
			
			
				case DISPLAY_SCREEN_WATCH_INFO:{					

		
					_lib_app_data lib_app_data;
					lib_app_data_read(&lib_app_data);					
					if(lib_app_data_crc_check_bootsetting(&lib_app_data))
					{
						GUI_MENU_PRINTF("[GUI_MENU]:app data already stored\n");
						DISPLAY_MSG  msg = {0,0};
						ScreenState = DISPLAY_SCREEN_WATCH_INFO;
						msg.cmd = MSG_DISPLAY_SCREEN;
						xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
					}
					else
					{
						GUI_MENU_PRINTF("[GUI_MENU]:have app data stored\n");
						if(SetValue.SwBle != false )
						{
							GUI_MENU_PRINTF("[GUI_MENU]:ble is enabled\n");									
							uint32_t command[1] = {BLE_UART_CCOMMAND_APPLICATION_BLE_INFO | BLE_UART_CCOMMAND_MASK_APPLICATION};
							_uart_data_param param;
							param.user_callback = NULL;
							param.p_data = (uint8_t *)command;
							param.length = sizeof(command);
							uint8_t count = TASK_UART_DELAY_COUNT_OUT;
							while(drv_ble_send_data(&param) != ERR_SUCCESS && count--)
							{
								DELAY_MS(TASK_UART_DELAY);
							}						
						}
						else
						{
							GUI_MENU_PRINTF("[GUI_MENU]:ble is disabled\n");
							//should never fall into here.
							timer_notify_display_start(2000,1,false);
							DISPLAY_MSG  msg = {0,0};
							ScreenStateSave = ScreenState;
							ScreenState = DISPLAY_SCREEN_NOTIFY_WARNING;
							msg.cmd = MSG_DISPLAY_SCREEN;
							msg.value = GUI_NOTIFY_WARNING_INDEX_WATCH_INFO_BLE_OFF;
							xQueueSend(DisplayQueue, &msg, portMAX_DELAY);								
							
						}					
					}
				}break;
			}	
		}break;		
					
	
	}

	

}







