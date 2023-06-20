#include "gui_customized_tools.h"
#include "SEGGER_RTT.h"
#include "drv_key.h"
#include "drv_lcd.h"
#include <string.h>
#include "font_displaystrs.h"
#include "task_display.h"
#include "com_data.h"
#include "img_tool.h"
#include "img_menu.h"
#include "gui.h"
#include "task_ble.h"
#include "time_notify.h"


#define GUI_CUSTOMIZED_TOOLS_LOG_ENABLED 1

#if DEBUG_ENABLED == 1 && GUI_CUSTOMIZED_TOOLS_LOG_ENABLED == 1
	#define GUI_CUSTOMIZED_TOOLS_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_CUSTOMIZED_TOOLS_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define GUI_CUSTOMIZED_TOOLS_WRITESTRING(...)
	#define GUI_CUSTOMIZED_TOOLS_PRINTF(...)		        
#endif

extern SetValueStr SetValue;
#define BACKGROUND_COLOR      LCD_WHITE
#define FOREGROUND_COLOR      LCD_BLACK

#define GUI_CUSTOMIZED_TOOLS_STEP				0
#define GUI_CUSTOMIZED_TOOLS_CALORIES			1
#define GUI_CUSTOMIZED_TOOLS_HEARTRATE			2
#define GUI_CUSTOMIZED_TOOLS_MOTIONS_RECORD		3
#define GUI_CUSTOMIZED_TOOLS_NOTIFICATION		4
#define GUI_CUSTOMIZED_TOOLS_CALENDAR			5

   


#define GUI_CUSTOMIZED_TOOLS_RECOVERY			6
#define GUI_CUSTOMIZED_TOOLS_VO2MAX			    7
#define GUI_CUSTOMIZED_TOOLS_LACTATE_THRESHOLD	8
#define GUI_CUSTOMIZED_TOOLS_CUTDOWN			9
#define GUI_CUSTOMIZED_TOOLS_STOPWATCH			10
#define GUI_CUSTOMIZED_TOOLS_GPS			    11
#define GUI_CUSTOMIZED_TOOLS_COMPASS			12
#define GUI_CUSTOMIZED_TOOLS_AMBIENT			13
/*#if defined WATCH_TOOL_COUNTDOWN
#define GUI_CUSTOMIZED_TOOLS_COUNTDOWN			5
#define GUI_CUSTOMIZED_TOOLS_PHONE_FINDER		6
#else
#define GUI_CUSTOMIZED_TOOLS_PHONE_FINDER		5
#endif*/

       
#define GUI_CUSTOMIZED_MONITOR_TOTAL		6		//有6+8项运动
#define GUI_CUSTOMIZED_TOOLS_TOTAL			8		//有6+8项运动


        #define	GUI_CUSTOMIZED_TOOLS_SUM			(6+8)		//有6+8项运动
        const uint8_t m_customized_tools_buf[GUI_CUSTOMIZED_TOOLS_SUM] = {
        			GUI_CUSTOMIZED_TOOLS_STEP, GUI_CUSTOMIZED_TOOLS_CALORIES,
        			GUI_CUSTOMIZED_TOOLS_HEARTRATE, GUI_CUSTOMIZED_TOOLS_MOTIONS_RECORD,
        			GUI_CUSTOMIZED_TOOLS_NOTIFICATION, GUI_CUSTOMIZED_TOOLS_CALENDAR,
        			GUI_CUSTOMIZED_TOOLS_RECOVERY,GUI_CUSTOMIZED_TOOLS_VO2MAX,
        			GUI_CUSTOMIZED_TOOLS_LACTATE_THRESHOLD,GUI_CUSTOMIZED_TOOLS_CUTDOWN,
        			GUI_CUSTOMIZED_TOOLS_STOPWATCH,GUI_CUSTOMIZED_TOOLS_GPS,
        			GUI_CUSTOMIZED_TOOLS_COMPASS,GUI_CUSTOMIZED_TOOLS_AMBIENT};


uint8_t img_custom_monitor_color[] = {LCD_CYAN,LCD_RED,LCD_RED,LCD_PERU,LCD_CORNFLOWERBLUE,LCD_PERU};

uint8_t img_custom_tools_color[] = {LCD_RED,LCD_RED,LCD_RED,LCD_RED,LCD_RED,LCD_RED,LCD_RED,LCD_NONE};

			

static uint8_t m_gui_customized_tools_index = 0;



static void menu_monitor_and_tools( char* menu1, char* menu2, char* menu3, uint8_t pic_index, bool enable, uint8_t flag)
{
	SetWord_t word = {0};
	word.x_axis = 40;			//16号字体对齐线
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_LIGHTGRAY;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;
	LCD_DisplayGTString(menu1,&word);	//更改使用字库字体	


	LCD_SetRectangle(120 - 24/2 - 36 - 1 ,2,LCD_LINE_CNT_MAX/2 - 90,180,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
	LCD_SetRectangle(120 + 24/2 + 36 - 1,2,LCD_LINE_CNT_MAX/2 - 90,180,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
	
	
	word.x_axis = 120 - 24/2;	 //24号字体对齐线
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.size = LCD_FONT_24_SIZE;
	LCD_DisplayGTString(menu2,&word);	//更改使用字库字体
	if(flag==0)
		{
		
		LCD_SetPicture(120 -30/2,120 - (strlen(menu2)/2)*LCD_FONT_24_SIZE/2 - 8 - 30, img_custom_monitor_color[pic_index], LCD_NONE, &img_custom_monitor[pic_index]);
		}
		else
		{
       
		LCD_SetPicture(120 -img_custom_tools[pic_index - GUI_CUSTOMIZED_MONITOR_TOTAL].height/2, 120 - (strlen(menu2)/2)*LCD_FONT_24_SIZE/2 - 8 - img_custom_tools[pic_index - GUI_CUSTOMIZED_MONITOR_TOTAL].width, img_custom_tools_color[pic_index - GUI_CUSTOMIZED_MONITOR_TOTAL], LCD_NONE, &img_custom_tools[pic_index - GUI_CUSTOMIZED_MONITOR_TOTAL]);

		}
	
	if(enable)
	{
		LCD_SetPicture(120 -25/2, 200, LCD_CYAN, LCD_NONE, &img_custom_tool_yes);
	}
	else{
		LCD_SetPicture(120 -25/2, 200, LCD_RED, LCD_NONE, &img_custom_tool_delete);
	}
	

	
	word.x_axis = 120 + 24/2 + 36 + 24;
	word.y_axis = LCD_CENTER_JUSTIFIED;
    word.forecolor = LCD_GRAY;
	word.bckgrndcolor = LCD_NONE;
	word.size = LCD_FONT_16_SIZE;
	LCD_DisplayGTString(menu3,&word);	//更改使用字库字体
}

void gui_customized_tools_paint(void)
{
	uint8_t tools_enable;
	
	LCD_SetBackgroundColor(LCD_BLACK);
	uint32_t index_value;
	index_value = m_customized_tools_buf[m_gui_customized_tools_index];
	switch(m_gui_customized_tools_index)
	{
		case GUI_CUSTOMIZED_TOOLS_STEP:
			tools_enable = SetValue.U_CustomMonitor.bits.step;
			break;
		case GUI_CUSTOMIZED_TOOLS_CALORIES:
			tools_enable = SetValue.U_CustomMonitor.bits.calories;
			break;
		case GUI_CUSTOMIZED_TOOLS_HEARTRATE:
			tools_enable = SetValue.U_CustomMonitor.bits.heartrate;
			break;
		case GUI_CUSTOMIZED_TOOLS_MOTIONS_RECORD:
			tools_enable = SetValue.U_CustomMonitor.bits.motions_record;
			break;
		case GUI_CUSTOMIZED_TOOLS_NOTIFICATION:
			tools_enable = SetValue.U_CustomMonitor.bits.notification;
			break;
		case GUI_CUSTOMIZED_TOOLS_CALENDAR:
	
			tools_enable = SetValue.U_CustomMonitor.bits.Calendar;
			break;


      	case GUI_CUSTOMIZED_TOOLS_RECOVERY:
			tools_enable = SetValue.U_CustomTool.bits.recover;
			break;
		case GUI_CUSTOMIZED_TOOLS_VO2MAX:
			tools_enable = SetValue.U_CustomTool.bits.vo2max;
			break;
		case GUI_CUSTOMIZED_TOOLS_LACTATE_THRESHOLD:
			tools_enable = SetValue.U_CustomTool.bits.threshold;
			break;
		case GUI_CUSTOMIZED_TOOLS_CUTDOWN:
			tools_enable = SetValue.U_CustomTool.bits.Countdown;
			break;
		case GUI_CUSTOMIZED_TOOLS_STOPWATCH:
			tools_enable = SetValue.U_CustomTool.bits.StopWatch;
			break;
		case GUI_CUSTOMIZED_TOOLS_GPS:
	
			tools_enable = SetValue.U_CustomTool.bits.Location;
			break;
		case GUI_CUSTOMIZED_TOOLS_COMPASS:
			tools_enable = SetValue.U_CustomTool.bits.Compass;
			break;
		case GUI_CUSTOMIZED_TOOLS_AMBIENT:
	
			tools_enable = SetValue.U_CustomTool.bits.ambient;
			break;
		default:
			break;
	}
	GUI_CUSTOMIZED_TOOLS_PRINTF("m_gui_customized_tools_index:%d,customized_tools:%d\r\n",m_gui_customized_tools_index, m_customized_tools_buf[m_gui_customized_tools_index]);
	//上翻提示
	gui_page_up(LCD_PERU);
	if(m_gui_customized_tools_index<=5)
	{
			if(m_gui_customized_tools_index == 0)
			{
				//第一项
				menu_monitor_and_tools((char *)CustomizedToolsStrs[SetValue.Language][GUI_CUSTOMIZED_TOOLS_TOTAL -1],
								(char *)CustomizedMonitorStrs[SetValue.Language][m_customized_tools_buf[0]],
								(char *)CustomizedMonitorStrs[SetValue.Language][m_customized_tools_buf[1]],
									m_customized_tools_buf[0], tools_enable,0);
			}
			else if(m_gui_customized_tools_index >= (GUI_CUSTOMIZED_TOOLS_SUM - GUI_CUSTOMIZED_TOOLS_TOTAL-1))
			{
				//最后一项
				menu_monitor_and_tools((char *)CustomizedMonitorStrs[SetValue.Language][m_customized_tools_buf[GUI_CUSTOMIZED_TOOLS_SUM - GUI_CUSTOMIZED_TOOLS_TOTAL-2]],
								(char *)CustomizedMonitorStrs[SetValue.Language][m_customized_tools_buf[GUI_CUSTOMIZED_TOOLS_SUM - GUI_CUSTOMIZED_TOOLS_TOTAL-1]],
								(char *)CustomizedToolsStrs[SetValue.Language][0],
									m_customized_tools_buf[m_gui_customized_tools_index], tools_enable,0);
			}
			else
			{
				menu_monitor_and_tools((char *)CustomizedMonitorStrs[SetValue.Language][m_customized_tools_buf[m_gui_customized_tools_index - 1]],
								(char *)CustomizedMonitorStrs[SetValue.Language][m_customized_tools_buf[m_gui_customized_tools_index]],
								(char *)CustomizedMonitorStrs[SetValue.Language][m_customized_tools_buf[m_gui_customized_tools_index + 1]],
									m_customized_tools_buf[m_gui_customized_tools_index], tools_enable,0);
			}
    }
	else
		{

         if(m_gui_customized_tools_index == (GUI_CUSTOMIZED_TOOLS_SUM - GUI_CUSTOMIZED_TOOLS_TOTAL))
			{
				//第一项
				menu_monitor_and_tools((char *)CustomizedMonitorStrs[SetValue.Language][GUI_CUSTOMIZED_MONITOR_TOTAL - 1],
								(char *)CustomizedToolsStrs[SetValue.Language][m_customized_tools_buf[m_gui_customized_tools_index]-GUI_CUSTOMIZED_MONITOR_TOTAL],
								(char *)CustomizedToolsStrs[SetValue.Language][m_customized_tools_buf[m_gui_customized_tools_index+1]-GUI_CUSTOMIZED_MONITOR_TOTAL],
									m_customized_tools_buf[m_gui_customized_tools_index], tools_enable,1);
			}
			else if(m_gui_customized_tools_index >= (GUI_CUSTOMIZED_TOOLS_SUM - 1))
			{
				//最后一项
				menu_monitor_and_tools((char *)CustomizedToolsStrs[SetValue.Language][m_customized_tools_buf[GUI_CUSTOMIZED_TOOLS_SUM - 2]-GUI_CUSTOMIZED_MONITOR_TOTAL],
								(char *)CustomizedToolsStrs[SetValue.Language][m_customized_tools_buf[GUI_CUSTOMIZED_TOOLS_SUM - 1]-GUI_CUSTOMIZED_MONITOR_TOTAL],
								(char *)CustomizedMonitorStrs[SetValue.Language][0],
									m_customized_tools_buf[m_gui_customized_tools_index], tools_enable,1);
			}
			else
			{
				menu_monitor_and_tools((char *)CustomizedToolsStrs[SetValue.Language][m_customized_tools_buf[m_gui_customized_tools_index - 1]-GUI_CUSTOMIZED_MONITOR_TOTAL],
								(char *)CustomizedToolsStrs[SetValue.Language][m_customized_tools_buf[m_gui_customized_tools_index]-GUI_CUSTOMIZED_MONITOR_TOTAL],
								(char *)CustomizedToolsStrs[SetValue.Language][m_customized_tools_buf[m_gui_customized_tools_index + 1]-GUI_CUSTOMIZED_MONITOR_TOTAL],
									m_customized_tools_buf[m_gui_customized_tools_index], tools_enable,1);
			}




	  }
	//下翻提示
	gui_page_down(LCD_PERU);
}

void gui_customized_tools_hint_paint(void)
{
   
    LCD_SetBackgroundColor(LCD_BLACK);
    SetWord_t word = {0};
	word.x_axis = 20;			//16号字体对齐线
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_LIGHTGRAY;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;
	LCD_DisplayGTString("删除提示",&word);

	LCD_SetPicture(word.x_axis,120 - 2*LCD_FONT_16_SIZE - 4 - img_custom_tool_hint.width,LCD_RED,LCD_NONE,&img_custom_tool_hint);
    

   	word.x_axis = 120 - 8 - 24;			
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	LCD_DisplayGTString("至少保留",&word);


    word.x_axis = 120 + 8;			
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	if(ScreenState == DISPLAY_SCREEN_CUSTOM_MON_HINT)
	{
     
	 LCD_DisplayGTString("一个监测功能",&word);
	}

	else if(ScreenState == DISPLAY_SCREEN_CUSTOM_TOOL_HINT)
		{
         
		 LCD_DisplayGTString("一个工具功能",&word);

	    }
	else if(ScreenState == DISPLAY_SCREEN_CUSTOM_SPORT_HINT)
		{

         
		 LCD_DisplayGTString("一个运动项目",&word);

	    }
	else;

	

	

	
}

void gui_customized_tools_btn_evt(uint32_t evt)
{
uint8_t screen_flag=0;	 
	switch(evt)
	{		
		case (KEY_BACK):{
			m_gui_customized_tools_index = 0;
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_MENU;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
		}break;			
		case (KEY_UP):{
			if(m_gui_customized_tools_index <= 0)
			{
				m_gui_customized_tools_index = GUI_CUSTOMIZED_TOOLS_SUM - 1;
			}else{
				m_gui_customized_tools_index--;
			}
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_CUSTOMIZED_TOOLS;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}break;	
			
		case (KEY_OK):{
			   

			// b_value = (SetValue.U_CustomTool&0x3F);
			 
				//back_monitor_screen = s_need_switch_screen[s_switch_screen_count];
				switch(m_customized_tools_buf[m_gui_customized_tools_index])
				{

                    case GUI_CUSTOMIZED_TOOLS_STEP:

					{
						if(SetValue.U_CustomMonitor.bits.step && ( SetValue.U_CustomMonitor.bits.calories || SetValue.U_CustomMonitor.bits.heartrate ||SetValue.U_CustomMonitor.bits.motions_record ||
															 SetValue.U_CustomMonitor.bits.notification||SetValue.U_CustomMonitor.bits.Calendar))
						{
							SetValue.U_CustomMonitor.bits.step = 0;
						}
						else
						{
                          if(SetValue.U_CustomMonitor.bits.step == 1)
                          	{
                             screen_flag =1;
						    }
						
							SetValue.U_CustomMonitor.bits.step = 1;
						}

					
							
					}

					break;
			  case GUI_CUSTOMIZED_TOOLS_CALORIES:

					{
						if(SetValue.U_CustomMonitor.bits.calories && ( SetValue.U_CustomMonitor.bits.step || SetValue.U_CustomMonitor.bits.heartrate ||SetValue.U_CustomMonitor.bits.motions_record ||
															 SetValue.U_CustomMonitor.bits.notification|| SetValue.U_CustomMonitor.bits.Calendar))
						{
							SetValue.U_CustomMonitor.bits.calories = 0;
						}
						else
							{

							if(SetValue.U_CustomMonitor.bits.calories == 1)
                          	{
                             screen_flag =1;
						    }
							SetValue.U_CustomMonitor.bits.calories = 1;
						}
					}

					break;
					    case GUI_CUSTOMIZED_TOOLS_HEARTRATE:

					{
						if(SetValue.U_CustomMonitor.bits.heartrate && ( SetValue.U_CustomMonitor.bits.calories || SetValue.U_CustomMonitor.bits.step ||SetValue.U_CustomMonitor.bits.motions_record ||
															 SetValue.U_CustomMonitor.bits.notification|| SetValue.U_CustomMonitor.bits.Calendar))
						{
							SetValue.U_CustomMonitor.bits.heartrate = 0;
						}else{

						      if(SetValue.U_CustomMonitor.bits.heartrate == 1)
                          	{
                             screen_flag =1;
						    }
							SetValue.U_CustomMonitor.bits.heartrate = 1;
						}
					}

					break;
						  case GUI_CUSTOMIZED_TOOLS_MOTIONS_RECORD:

					{
						if(SetValue.U_CustomMonitor.bits.motions_record && ( SetValue.U_CustomMonitor.bits.calories || SetValue.U_CustomMonitor.bits.heartrate ||SetValue.U_CustomMonitor.bits.step ||
															 SetValue.U_CustomMonitor.bits.notification|| SetValue.U_CustomMonitor.bits.Calendar))
						{
							SetValue.U_CustomMonitor.bits.motions_record = 0;
						}else{

						        if(SetValue.U_CustomMonitor.bits.motions_record  == 1)
                          	{
                             screen_flag =1;
						    }
							SetValue.U_CustomMonitor.bits.motions_record = 1;
						}
					}

					break;
				  case GUI_CUSTOMIZED_TOOLS_NOTIFICATION:

					{
						if(SetValue.U_CustomMonitor.bits.notification && ( SetValue.U_CustomMonitor.bits.calories || SetValue.U_CustomMonitor.bits.heartrate ||SetValue.U_CustomMonitor.bits.motions_record ||
															 SetValue.U_CustomMonitor.bits.step|| SetValue.U_CustomMonitor.bits.Calendar))
						{
							SetValue.U_CustomMonitor.bits.notification = 0;
						}else{

						      if(SetValue.U_CustomMonitor.bits.notification  == 1)
                          	{
                             screen_flag =1;
						    }
							SetValue.U_CustomMonitor.bits.notification = 1;
						}
					}

					break;
			 case GUI_CUSTOMIZED_TOOLS_CALENDAR:

					{
						if(SetValue.U_CustomMonitor.bits.Calendar && ( SetValue.U_CustomMonitor.bits.calories || SetValue.U_CustomMonitor.bits.heartrate ||SetValue.U_CustomMonitor.bits.motions_record ||
															 SetValue.U_CustomMonitor.bits.notification|| SetValue.U_CustomMonitor.bits.step))
						{
							SetValue.U_CustomMonitor.bits.Calendar = 0;
						}else{
						    
						      if(SetValue.U_CustomMonitor.bits.Calendar  == 1)
                          	{
                             screen_flag =1;
						    }
							SetValue.U_CustomMonitor.bits.Calendar = 1;
						}
					}

					break;
							  




					
					case GUI_CUSTOMIZED_TOOLS_RECOVERY:{
						if(SetValue.U_CustomTool.bits.recover && ( SetValue.U_CustomTool.bits.Location || SetValue.U_CustomTool.bits.vo2max ||SetValue.U_CustomTool.bits.threshold ||
																	SetValue.U_CustomTool.bits.Countdown|| SetValue.U_CustomTool.bits.StopWatch||SetValue.U_CustomTool.bits.Compass||
																	SetValue.U_CustomTool.bits.ambient))
						{
							SetValue.U_CustomTool.bits.recover = 0;
						}else{
						       if(SetValue.U_CustomTool.bits.recover  == 1)
                          	{
                             screen_flag =2;
						    }
							SetValue.U_CustomTool.bits.recover = 1;
						}
					}break;


							case GUI_CUSTOMIZED_TOOLS_VO2MAX:{
						if(SetValue.U_CustomTool.bits.vo2max && ( SetValue.U_CustomTool.bits.recover || SetValue.U_CustomTool.bits.Location ||SetValue.U_CustomTool.bits.threshold ||
																	SetValue.U_CustomTool.bits.Countdown|| SetValue.U_CustomTool.bits.StopWatch||SetValue.U_CustomTool.bits.Compass||
																	SetValue.U_CustomTool.bits.ambient))
						{
							SetValue.U_CustomTool.bits.vo2max = 0;
						}
						else
							{
                               if(SetValue.U_CustomTool.bits.vo2max  == 1)
                          	{
                             screen_flag =2;
						    }
						      
							SetValue.U_CustomTool.bits.vo2max = 1;
						}
					}break;
							case GUI_CUSTOMIZED_TOOLS_LACTATE_THRESHOLD:{
						if(SetValue.U_CustomTool.bits.threshold && ( SetValue.U_CustomTool.bits.recover || SetValue.U_CustomTool.bits.vo2max || SetValue.U_CustomTool.bits.Location ||
																	SetValue.U_CustomTool.bits.Countdown|| SetValue.U_CustomTool.bits.StopWatch||SetValue.U_CustomTool.bits.Compass ||
																	SetValue.U_CustomTool.bits.ambient))
						{
							SetValue.U_CustomTool.bits.threshold = 0;
						}
						else{
						       if(SetValue.U_CustomTool.bits.threshold  == 1)
                          	  {
                               screen_flag =2;
						       }
							SetValue.U_CustomTool.bits.threshold = 1;
						}
					}break;
								case GUI_CUSTOMIZED_TOOLS_CUTDOWN:{
						if(SetValue.U_CustomTool.bits.Countdown && ( SetValue.U_CustomTool.bits.recover || SetValue.U_CustomTool.bits.vo2max ||SetValue.U_CustomTool.bits.threshold ||
																	SetValue.U_CustomTool.bits.Location|| SetValue.U_CustomTool.bits.StopWatch||SetValue.U_CustomTool.bits.Compass||
																	SetValue.U_CustomTool.bits.ambient))
						{
							SetValue.U_CustomTool.bits.Countdown = 0;
						}
						else{
						       if(SetValue.U_CustomTool.bits.Countdown  == 1)
                          	  {
                               screen_flag =2;
						       }
							SetValue.U_CustomTool.bits.Countdown = 1;
						}
					}break;
								case GUI_CUSTOMIZED_TOOLS_STOPWATCH:{
						if(SetValue.U_CustomTool.bits.StopWatch && ( SetValue.U_CustomTool.bits.recover || SetValue.U_CustomTool.bits.vo2max ||SetValue.U_CustomTool.bits.threshold ||
																	SetValue.U_CustomTool.bits.Countdown|| SetValue.U_CustomTool.bits.Location||SetValue.U_CustomTool.bits.Compass||
																	SetValue.U_CustomTool.bits.ambient))
						{
							SetValue.U_CustomTool.bits.StopWatch = 0;
						}else{
						      if(SetValue.U_CustomTool.bits.StopWatch  == 1)
                          	  {
                               screen_flag =2;
						       }
							SetValue.U_CustomTool.bits.StopWatch = 1;
						}
					}break;
								case GUI_CUSTOMIZED_TOOLS_GPS:{
						if(SetValue.U_CustomTool.bits.Location && ( SetValue.U_CustomTool.bits.recover || SetValue.U_CustomTool.bits.vo2max ||SetValue.U_CustomTool.bits.threshold ||
																	SetValue.U_CustomTool.bits.Countdown|| SetValue.U_CustomTool.bits.StopWatch||SetValue.U_CustomTool.bits.Compass||
																	SetValue.U_CustomTool.bits.ambient))
						{
							SetValue.U_CustomTool.bits.Location = 0;
						}else{
						      if(SetValue.U_CustomTool.bits.Location == 1)
                          	  {
                               screen_flag =2;
						       }
							SetValue.U_CustomTool.bits.Location = 1;
						}
					}break;
								case GUI_CUSTOMIZED_TOOLS_COMPASS:{
						if(SetValue.U_CustomTool.bits.Compass && ( SetValue.U_CustomTool.bits.recover || SetValue.U_CustomTool.bits.vo2max ||SetValue.U_CustomTool.bits.threshold ||
																	SetValue.U_CustomTool.bits.Countdown|| SetValue.U_CustomTool.bits.StopWatch||SetValue.U_CustomTool.bits.Location||
																	SetValue.U_CustomTool.bits.ambient))
						{
							SetValue.U_CustomTool.bits.Compass = 0;
						}else{

						  if(SetValue.U_CustomTool.bits.Compass == 1)
                          	  {
                               screen_flag =2;
						       }
							SetValue.U_CustomTool.bits.Compass = 1;
						}
					}break;
								case GUI_CUSTOMIZED_TOOLS_AMBIENT:{
						if(SetValue.U_CustomTool.bits.ambient && ( SetValue.U_CustomTool.bits.recover || SetValue.U_CustomTool.bits.vo2max ||SetValue.U_CustomTool.bits.threshold ||
																	SetValue.U_CustomTool.bits.Countdown|| SetValue.U_CustomTool.bits.StopWatch||SetValue.U_CustomTool.bits.Compass||
																	SetValue.U_CustomTool.bits.Location))
						{
							SetValue.U_CustomTool.bits.ambient = 0;
						}else{
						     if(SetValue.U_CustomTool.bits.ambient == 1)
                          	  {
                               screen_flag =2;
						       }
							SetValue.U_CustomTool.bits.ambient = 1;
						}
					}break;
					default:
						break;

						

					
				}

				
			//	a_value = (SetValue.U_CustomTool&0x3F);
			/*	if(b_value==a_value)//自定义监测无修改
					{
					 minitor_change_flag = VALUE_NOT_CHANGE;
					}

				else
					{
                        minitor_change_flag = VALUE_BY_CHANGE;
				    }*/
				DISPLAY_MSG  msg = {0,0};
			if(screen_flag==0)
				{
                ScreenState = DISPLAY_SCREEN_CUSTOMIZED_TOOLS;
				  msg.cmd = MSG_DISPLAY_SCREEN;
				 xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
			    }
			else if(screen_flag==1)
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
					ScreenState = DISPLAY_SCREEN_CUSTOM_MON_HINT; 
					msg.cmd = MSG_DISPLAY_SCREEN; 
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);      
					} 
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
					ScreenState = DISPLAY_SCREEN_CUSTOM_TOOL_HINT; 
					msg.cmd = MSG_DISPLAY_SCREEN; 
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);      
					}

			    }
			
						
			
			
		
		}break;		
		case (KEY_DOWN):{
			if(m_gui_customized_tools_index >= ( GUI_CUSTOMIZED_TOOLS_SUM - 1))
			{
				m_gui_customized_tools_index = 0;
			}else{
				m_gui_customized_tools_index++;
			}
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_CUSTOMIZED_TOOLS;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}break;					
	
	}

	



}


void gui_customized_tools_hint_btn_evt(uint32_t evt)
{
   
   DISPLAY_MSG	msg = {0,0};
	switch(evt)
		{
       case (KEY_UP):
	   	case (KEY_DOWN):
			case (KEY_OK):
			ScreenState = DISPLAY_SCREEN_MENU;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
         
       default:
	   	ScreenState = DISPLAY_SCREEN_MENU;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);

	   break;
	  }



}









