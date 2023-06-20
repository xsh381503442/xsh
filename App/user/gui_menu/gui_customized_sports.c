#include "gui_customized_sports.h"
#include "SEGGER_RTT.h"
#include "drv_key.h"
#include "drv_lcd.h"
#include <string.h>
#include "font_displaystrs.h"
#include "task_display.h"
#include "com_data.h"
#include "img_sport.h"
#include "img_menu.h"
#include "gui.h"
#include "time_notify.h"

#define GUI_CUSTOMIZED_SPORTS_LOG_ENABLED 1

#if DEBUG_ENABLED == 1 && GUI_CUSTOMIZED_SPORTS_LOG_ENABLED == 1
	#define GUI_CUSTOMIZED_SPORTS_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_CUSTOMIZED_SPORTS_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define GUI_CUSTOMIZED_SPORTS_WRITESTRING(...)
	#define GUI_CUSTOMIZED_SPORTS_PRINTF(...)		        
#endif

extern SetValueStr SetValue;
#define BACKGROUND_COLOR      LCD_WHITE
#define FOREGROUND_COLOR      LCD_BLACK

/*#define GUI_CUSTOMIZED_SPORTS_HIKING					0
#define GUI_CUSTOMIZED_SPORTS_CROSSCOUNTRY				1
#define GUI_CUSTOMIZED_SPORTS_RUN						2
#define GUI_CUSTOMIZED_SPORTS_MAROTHON					3
#define GUI_CUSTOMIZED_SPORTS_INDOOR_RUN				4
#define GUI_CUSTOMIZED_SPORTS_WALK						5
#define GUI_CUSTOMIZED_SPORTS_BIKE						6
#define GUI_CUSTOMIZED_SPORTS_MOUNTAIN					7
#define GUI_CUSTOMIZED_SPORTS_SWIM						8
#define GUI_CUSTOMIZED_SPORTS_TRIATHONAL				9*/

#define GUI_CUSTOMIZED_SPORTS_RUN						0
#define GUI_CUSTOMIZED_SPORTS_BIKE						1
#define GUI_CUSTOMIZED_SPORTS_MOUNTAIN					2
#define GUI_CUSTOMIZED_SPORTS_SWIM						3
#define GUI_CUSTOMIZED_SPORTS_HIKING					4
#define GUI_CUSTOMIZED_SPORTS_CROSSCOUNTRY				5





#define GUI_CUSTOMIZED_SPORTS_MAROTHON					6
#define GUI_CUSTOMIZED_SPORTS_INDOOR_RUN				7
#define GUI_CUSTOMIZED_SPORTS_WALK						8
#define GUI_CUSTOMIZED_SPORTS_TRIATHONAL				9


#define	GUI_CUSTOMIZED_SPORTS_SUM						6		//有6项运动
const uint8_t m_customized_sport_buf[GUI_CUSTOMIZED_SPORTS_SUM] = {
			GUI_CUSTOMIZED_SPORTS_RUN, GUI_CUSTOMIZED_SPORTS_BIKE,
			GUI_CUSTOMIZED_SPORTS_MOUNTAIN,GUI_CUSTOMIZED_SPORTS_SWIM,
			 GUI_CUSTOMIZED_SPORTS_HIKING,GUI_CUSTOMIZED_SPORTS_CROSSCOUNTRY
			 };			


static uint8_t m_gui_customized_sports_index = 0;

static void menu( char* menu1, char* menu2, char* menu3, uint8_t pic_index, bool enable)
{
	SetWord_t word = {0};
	
    word.x_axis = 120 - 24/2 - 36 - 24 - 16;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_LIGHTGRAY;
	word.bckgrndcolor = LCD_NONE;
	LCD_DisplayGTString(menu1,&word);	//更改使用字库字体	



    LCD_SetRectangle(120 - 24/2 - 36 - 1 ,2,LCD_LINE_CNT_MAX/2 - 90,180,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
	LCD_SetRectangle(120 + 24/2 + 36 - 1,2,LCD_LINE_CNT_MAX/2 - 90,180,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);

	

	
	   word.x_axis = 120 - 24/2;
	   word.y_axis = LCD_CENTER_JUSTIFIED;
	   word.size = LCD_FONT_24_SIZE;
	   word.forecolor = LCD_WHITE;
	   word.bckgrndcolor = LCD_NONE;

	LCD_DisplayGTString(menu2,&word);	//更改使用字库字体
	
	LCD_SetPicture(120 -60/2, 15, LCD_CYAN, LCD_NONE, &img_custom_sports[pic_index]);
	
	if(enable)
	{
		
		LCD_SetPicture(120 -25/2, 200, LCD_CYAN, LCD_NONE, &img_custom_tool_yes);
		
	}
	else{
		LCD_SetPicture(120 -25/2, 200, LCD_RED, LCD_NONE, &img_custom_tool_delete);
	}
	

	
	word.x_axis = 120 + 24/2 + 36 + 24;
    word.forecolor = LCD_LIGHTGRAY;
	word.size = LCD_FONT_16_SIZE;
	LCD_DisplayGTString(menu3,&word);	//更改使用字库字体	
}

void gui_customized_sports_paint(void)
{
	uint8_t sport_enable;
	
	GUI_CUSTOMIZED_SPORTS_PRINTF("[GUI_CUSTOMIZED_SPORTS]: gui_customized_sports_paint\r\n");
	GUI_CUSTOMIZED_SPORTS_PRINTF("[GUI_CUSTOMIZED_SPORTS]: m_gui_customized_sports_index-->%d\r\n",m_gui_customized_sports_index);
	LCD_SetBackgroundColor(LCD_BLACK);
	
	switch(m_customized_sport_buf[m_gui_customized_sports_index])
	{
		case GUI_CUSTOMIZED_SPORTS_HIKING:
			sport_enable = SetValue.U_CustomSport.bits.Hiking;
			break;
		case GUI_CUSTOMIZED_SPORTS_CROSSCOUNTRY:
			sport_enable = SetValue.U_CustomSport.bits.CountryRun;
			break;
		case GUI_CUSTOMIZED_SPORTS_RUN:
			sport_enable = SetValue.U_CustomSport.bits.Run;
			break;
		/*case GUI_CUSTOMIZED_SPORTS_MAROTHON:
			sport_enable = SetValue.U_CustomSport.bits.Marathon;
			break;*/
		/*case GUI_CUSTOMIZED_SPORTS_INDOOR_RUN:
			sport_enable = SetValue.U_CustomSport.bits.IndoorRun;
			break;*/
		/*case GUI_CUSTOMIZED_SPORTS_WALK:
			sport_enable = SetValue.U_CustomSport.bits.Walking;
			break;*/
		case GUI_CUSTOMIZED_SPORTS_BIKE:
			sport_enable = SetValue.U_CustomSport.bits.Cycling;
			break;
		case GUI_CUSTOMIZED_SPORTS_MOUNTAIN:
			sport_enable = SetValue.U_CustomSport.bits.Climbing;
			break;
		case GUI_CUSTOMIZED_SPORTS_SWIM:
			sport_enable = SetValue.U_CustomSport.bits.Swimming;
			break;
		default:
			//sport_enable = SetValue.U_CustomSport.bits.Triathlon;
			break;
	}
	
	//上翻提示
	gui_page_up(LCD_PERU);
	
	if(m_gui_customized_sports_index == 0)
	{
		//第一项
		menu((char *)CustomizedSportsStrs[SetValue.Language][m_customized_sport_buf[GUI_CUSTOMIZED_SPORTS_SUM - 1]],
						(char *)CustomizedSportsStrs[SetValue.Language][m_customized_sport_buf[0]],
						(char *)CustomizedSportsStrs[SetValue.Language][m_customized_sport_buf[1]],
							m_customized_sport_buf[0], sport_enable);
	}
	else if((m_gui_customized_sports_index + 1) >= GUI_CUSTOMIZED_SPORTS_SUM)
	{
		//最后一项
		menu((char *)CustomizedSportsStrs[SetValue.Language][m_customized_sport_buf[GUI_CUSTOMIZED_SPORTS_SUM - 2]],
						(char *)CustomizedSportsStrs[SetValue.Language][m_customized_sport_buf[GUI_CUSTOMIZED_SPORTS_SUM - 1]],
						(char *)CustomizedSportsStrs[SetValue.Language][m_customized_sport_buf[0]],
							m_customized_sport_buf[m_gui_customized_sports_index], sport_enable);
	}
	else
	{
		menu((char *)CustomizedSportsStrs[SetValue.Language][m_customized_sport_buf[m_gui_customized_sports_index - 1]],
						(char *)CustomizedSportsStrs[SetValue.Language][m_customized_sport_buf[m_gui_customized_sports_index]],
						(char *)CustomizedSportsStrs[SetValue.Language][m_customized_sport_buf[m_gui_customized_sports_index + 1]],
							m_customized_sport_buf[m_gui_customized_sports_index], sport_enable);
	}
	
	//下翻提示
	gui_page_down(LCD_PERU);
}


void gui_customized_sports_btn_evt(uint32_t evt)
{

  uint8_t flag = 0;
	switch(evt)
	{		
		case (KEY_BACK):{
			m_gui_customized_sports_index = 0;
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_MENU;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
		}break;			
		case (KEY_UP):{
			GUI_CUSTOMIZED_SPORTS_PRINTF("[GUI_CUSTOMIZED_SPORTS]: KEY_UP\n");
			GUI_CUSTOMIZED_SPORTS_PRINTF("[GUI_CUSTOMIZED_SPORTS]: m_gui_customized_sports_index-->%d\n",m_gui_customized_sports_index);
			if(m_gui_customized_sports_index <= 0)
			{
				m_gui_customized_sports_index = GUI_CUSTOMIZED_SPORTS_SUM - 1;
				
			}else{
				m_gui_customized_sports_index--;
			}
			GUI_CUSTOMIZED_SPORTS_PRINTF("[GUI_CUSTOMIZED_SPORTS]: m_gui_customized_sports_index-->%d\n",m_gui_customized_sports_index);
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_CUSTOMIZED_SPORTS;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}break;			
		case (KEY_OK):{
			switch(m_customized_sport_buf[m_gui_customized_sports_index])
			{	
				case GUI_CUSTOMIZED_SPORTS_RUN:{
					if(SetValue.U_CustomSport.bits.Run && (SetValue.U_CustomSport.bits.CountryRun || SetValue.U_CustomSport.bits.Hiking ||
												             SetValue.U_CustomSport.bits.Climbing || SetValue.U_CustomSport.bits.Cycling ||
															 SetValue.U_CustomSport.bits.Swimming ))
					{
						SetValue.U_CustomSport.bits.Run = 0;
					}
					else

					{

					  if(SetValue.U_CustomSport.bits.Run == 1)
					  	{
                        flag = 1;
					   }
						SetValue.U_CustomSport.bits.Run = 1;
					}
				}break;
				case GUI_CUSTOMIZED_SPORTS_CROSSCOUNTRY:{
					if(SetValue.U_CustomSport.bits.CountryRun && (SetValue.U_CustomSport.bits.Run || SetValue.U_CustomSport.bits.Hiking ||
												             SetValue.U_CustomSport.bits.Climbing || SetValue.U_CustomSport.bits.Cycling ||
															 SetValue.U_CustomSport.bits.Swimming ))
					{
						SetValue.U_CustomSport.bits.CountryRun = 0;
					}else{
					   if(SetValue.U_CustomSport.bits.CountryRun == 1)
					  	{
                        flag = 1;
					   }
						SetValue.U_CustomSport.bits.CountryRun = 1;
					}
				}break;
		
				case GUI_CUSTOMIZED_SPORTS_BIKE:{
					if(SetValue.U_CustomSport.bits.Cycling && (SetValue.U_CustomSport.bits.CountryRun || SetValue.U_CustomSport.bits.Hiking ||
												             SetValue.U_CustomSport.bits.Climbing || SetValue.U_CustomSport.bits.Run ||
															 SetValue.U_CustomSport.bits.Swimming ))
					{
						SetValue.U_CustomSport.bits.Cycling = 0;
					}else{

					   if(SetValue.U_CustomSport.bits.Cycling == 1)
					  	{
                        flag = 1;
					   }
						SetValue.U_CustomSport.bits.Cycling = 1;
					}
				}break;
		
				case GUI_CUSTOMIZED_SPORTS_MOUNTAIN:{
					if(SetValue.U_CustomSport.bits.Climbing && (SetValue.U_CustomSport.bits.CountryRun || SetValue.U_CustomSport.bits.Hiking ||
												             SetValue.U_CustomSport.bits.Run || SetValue.U_CustomSport.bits.Cycling ||
															 SetValue.U_CustomSport.bits.Swimming ))
					{
						SetValue.U_CustomSport.bits.Climbing = 0;
					}else{

					   if(SetValue.U_CustomSport.bits.Climbing == 1)
					  	{
                        flag = 1;
					   }
						SetValue.U_CustomSport.bits.Climbing = 1;
					}
				}break;
				case GUI_CUSTOMIZED_SPORTS_SWIM:{
					if(SetValue.U_CustomSport.bits.Swimming && (SetValue.U_CustomSport.bits.CountryRun || SetValue.U_CustomSport.bits.Hiking ||
												             SetValue.U_CustomSport.bits.Climbing || SetValue.U_CustomSport.bits.Cycling ||
															 SetValue.U_CustomSport.bits.Run ))
					{
						SetValue.U_CustomSport.bits.Swimming = 0;
					}else{

					   if(SetValue.U_CustomSport.bits.Swimming == 1)
					  	{
                        flag = 1;
					   }
						SetValue.U_CustomSport.bits.Swimming = 1;
					}
				}break;

				
				case GUI_CUSTOMIZED_SPORTS_HIKING:{
					if(SetValue.U_CustomSport.bits.Hiking && (SetValue.U_CustomSport.bits.CountryRun || SetValue.U_CustomSport.bits.Run ||
												             SetValue.U_CustomSport.bits.Climbing || SetValue.U_CustomSport.bits.Cycling ||
															 SetValue.U_CustomSport.bits.Swimming ))
					{
						SetValue.U_CustomSport.bits.Hiking = 0;
					}
					else
						{

					   if(SetValue.U_CustomSport.bits.Hiking == 1)
					  	{
                        flag = 1;
					   }
						SetValue.U_CustomSport.bits.Hiking = 1;
					}
				}break;
			}
			DISPLAY_MSG  msg = {0,0};

			if(flag == 0)
				{
                
				ScreenState = DISPLAY_SCREEN_CUSTOMIZED_SPORTS;
                 msg.cmd = MSG_DISPLAY_SCREEN;
			     xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			   }

			else
				{
                    if((ScreenState != DISPLAY_SCREEN_LOGO)  && (ScreenState != DISPLAY_SCREEN_NOTIFY_BAT_CHG)  && (ScreenState != DISPLAY_SCREEN_BLE_DFU)  && time_notify_is_ok()) 
				  	{  
				  	if(ScreenState < DISPLAY_SCREEN_NOTIFY) 
						{  
						ScreenStateSave = ScreenState; 
						} 
					timer_notify_display_start(2000,1,false);  
					timer_notify_motor_stop(false); 
					timer_notify_buzzer_stop(false);   
					DISPLAY_MSG  msg = {0,0}; 
					
					ScreenState = DISPLAY_SCREEN_CUSTOM_SPORT_HINT;
					
					msg.cmd = MSG_DISPLAY_SCREEN; 
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY); 
			    }
						
		}
			}break;		
		case (KEY_DOWN):{
			if(m_gui_customized_sports_index >= ( GUI_CUSTOMIZED_SPORTS_SUM - 1))
			{
				m_gui_customized_sports_index = 0;
			}else{
				m_gui_customized_sports_index++;
			}
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_CUSTOMIZED_SPORTS;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}break;					
	
	}

}








