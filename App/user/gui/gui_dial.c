#include "gui_config.h"
#include "gui_dial.h"

#include "drv_lcd.h"
#include "drv_battery.h"
#include "drv_ms5837.h"

#include "task_ble.h"

#include "timer_app.h"

#include "gui.h"
#include "gui_clockline.h"
#include "gui_home.h"
#include "gui_notification.h"
#include "gui_tool_calendar.h"
#include "gui_system.h"

#include "com_data.h"
#include "com_sport.h"
#include "com_dial.h"

#include "font_config.h"
#include "gui_theme.h"




#if DEBUG_ENABLED == 1 && GUI_DIAL_LOG_ENABLED == 1
	#define GUI_DIAL_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_DIAL_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define GUI_DIAL_WRITESTRING(...)
	#define GUI_DIAL_PRINTF(...)		        
#endif

extern DialDataStr DialData;
extern SetValueStr SetValue;
extern am_hal_rtc_time_t RTC_time;
extern _drv_bat_event g_bat_evt;
#ifdef COD 
extern CodWeatherSync g_weather_info;
#endif
extern uint8_t calendar_weather_data[];
extern ThemeDataStr m_theme_data[DIAL_MAX_NUM];
extern uint8_t m_theme_index;
 
static uint8_t m_dial_num;				//外部flash中主题存储第几空间
static uint32_t m_gial_addr_offset = 0;	//主题地址偏移量

static uint8_t dial_weather_index = 0;

extern float m_pres,m_temp,m_alt;


static void gui_dial_addr_offset_get(DialDataStr *dial)
{
	if((dial->Number >= 100) && (dial->Number != 0xFFFF))
	{
		//对于ID>=100的主题，存于外部flash中的图片数据地址整体向后偏移4K(不包括主题参数数据地址)
		m_gial_addr_offset = 0x1000;
	}
	else
	{
		m_gial_addr_offset = 0;
	}
}

void gui_dial_paint(void)
{
	if(0 == system_voltage_flag_get())	//开机获取完电量后再显示主界面
	{
		if( SetValue.SwAutoLockScreen == 1 && SetValue.AutoLockScreenFlag != 1)
		{
			timer_app_LOCK_start();//开始锁屏
		}
		Set_Curr_Home_Num(0);
		//根据主题设置判断是第几个表盘
		if((SetValue.Theme & 0x80) && (SetValue.Theme != 0xFF))
		{
			//节日表盘
			m_dial_num = DIAL_FESTIVAL_NUM - 1;
		}
		else if((SetValue.Theme >= DIAL_DEFAULT_MAX_NUM) && (SetValue.Theme < DIAL_MAX_NUM))
		{
			m_dial_num = SetValue.Theme - DIAL_DEFAULT_MAX_NUM;
		}
		else
		{
			m_dial_num = 0;
		}
		
		//获取当前时间
		am_hal_rtc_time_get(&RTC_time);
		
		//计算地址偏移量
		gui_dial_addr_offset_get(&DialData);
		
		//背景
		gui_dial_background(&DialData);
		
		//日期
		gui_dial_date(&DialData);
		
		//工具栏:蓝牙,闹钟,勿扰状态
		gui_dial_widget(&DialData);
		
		//电池
			gui_dial_battery(&DialData);
	
		
		//状态栏:步数
	    gui_dial_status(&DialData);
		//卡路里
		if(DialData.Setting.reserve1[0] == DIAL_STYLE_DEFAULT)
		{
	      gui_dial_kcal(&DialData);
		}
		//环境:气压，高度等
		
		if(DialData.Setting.reserve1[0] == DIAL_STYLE_DEFAULT)
			{
	   gui_dial_ambient(&DialData);
			}
		//心率
		
		if(DialData.Setting.reserve1[0] == DIAL_STYLE_DEFAULT)
       {		
        gui_dial_heart(&DialData);
			}
		
		//天气
		
         gui_dial_weather(&DialData);

		//气压图表
		if(DialData.Setting.reserve1[0] == DIAL_STYLE_DEFAULT)
			{
	         gui_dial_graph(&DialData);
			}
		//时间
	gui_dial_time(&DialData, &RTC_time);

	}
}

//预览表盘
void gui_dial_preview(uint8_t number, DialDataStr *dial)
{
	am_hal_rtc_time_t time;
	
	if((number >= DIAL_DEFAULT_MAX_NUM) && (number < DIAL_MAX_NUM))
	{
		m_dial_num = number - DIAL_DEFAULT_MAX_NUM;
	}
	else
	{
		m_dial_num = 0;
		
		//gui_progess_toothed(360,24);
	}
	
	//获取当前时间
	am_hal_rtc_time_get(&time);
	
	//计算地址偏移量
	gui_dial_addr_offset_get(dial);
	
	//背景
	gui_dial_background(dial);
	
	//日期
	gui_dial_date(dial);
	
	//工具栏
	gui_dial_widget(dial);
	
	//电池
	if(dial->Background.type != DIAL_BACKGROUND_TYPE_BATTERY)
	{
		gui_dial_battery(dial);
	}
	
	//状态栏:步数
	gui_dial_status(dial);
		//卡路里
		if(dial->Setting.reserve1[0] == DIAL_STYLE_DEFAULT)
		{
	      gui_dial_kcal(dial);
		}

		//环境:气压，高度等
		
		if(dial->Setting.reserve1[0] == DIAL_STYLE_DEFAULT)
			{
	   gui_dial_ambient(dial);
			}
		//心率
		
		if(dial->Setting.reserve1[0] == DIAL_STYLE_DEFAULT)
       {		
        gui_dial_heart(dial);
			}
	
	//天气

	gui_dial_weather(dial);
	
	
	
	//图表
	if(dial->Setting.reserve1[0] == DIAL_STYLE_DEFAULT)
			{
	         gui_dial_graph(dial);
			}
	
	//时间
	if((dial->Time.type == DIAL_TIME_TYPE_WATCHHAND_1)
		|| (dial->Time.type == DIAL_TIME_TYPE_WATCHHAND_2)
		|| (dial->Time.type == DIAL_TIME_TYPE_WATCHHAND_3)
		|| (dial->Time.type == DIAL_TIME_TYPE_WATCHHAND_4)
		|| (dial->Time.type == DIAL_TIME_TYPE_WATCHHAND_5)||(dial->Time.type == DIAL_TIME_TYPE_WATCHHAND_6))
	{
		//指针表盘，使用默认时间10:09
		time.ui32Hour = 10;
		time.ui32Minute = 9;
	}
	gui_dial_time(dial, &time);	
	
	
}




static void gui_dial_background(DialDataStr *dial)
{
	//设置背景色
	LCD_SetBackgroundColor(dial->Background.color);
	
	if(dial->Background.type == DIAL_BACKGROUND_TYPE_IMG)
	{
		//带图片的背景
		LCD_SetPicture_ExtFlash(DIAL_BACKGROUND_IMG_ADDRESS + (m_dial_num * DIAL_DATA_SIZE) + m_gial_addr_offset, &dial->Background.img, dial->Background.img.y_axis);
	}
	else if(dial->Background.type == DIAL_BACKGROUND_TYPE_BATTERY)
	{
		//电量
		gui_dial_battery(dial);
		
		//背景
		LCD_SetPicture_ExtFlash(DIAL_BACKGROUND_IMG_ADDRESS + (m_dial_num * DIAL_DATA_SIZE) + m_gial_addr_offset, &dial->Background.img, dial->Background.img.y_axis);
	}
}

static void gui_dial_time(DialDataStr *dial, am_hal_rtc_time_t *time)
{
	SetWord_t word = {0};
	char str[10] = {0};
	char str2[10] = {0};
	uint8_t hour_y,min_y,temp;
	uint16_t len,len2;
	int kern1,kern2,time_gap;
	switch(dial->Time.type)
	{
		case DIAL_TIME_TYPE_DEFAULT:	//默认表盘数字
			word.size = LCD_FONT_47_SIZE;
			word.x_axis = dial->Time.hourNum.numImg.x_axis;
			if((dial->Time.hourNum.numImg.y_axis == LCD_CENTER_JUSTIFIED) 
				&& (dial->Time.minNum.numImg.y_axis == LCD_CENTER_JUSTIFIED))
			{
				/*word.y_axis = (LCD_PIXEL_ONELINE_MAX - (dial->Time.hourNum.numImg.width*2) - dial->Time.hourNum.kerning 
								- (dial->Time.minNum.numImg.width*2) - dial->Time.minNum.kerning - dial->Time.gap) / 2;*/
				/*word.y_axis = (LCD_PIXEL_ONELINE_MAX/2 - (dial->Time.hourNum.numImg.width*3 + dial->Time.hourNum.kerning*2 
								- dial->Time.hourNum.numImg.width/2));*/
					word.y_axis = (LCD_PIXEL_ONELINE_MAX - (dial->Time.hourNum.numImg.width*2) - dial->Time.hourNum.kerning 
								- (dial->Time.minNum.numImg.width*2) - dial->Time.minNum.kerning - dial->Time.gap) / 2 - 8;
		      //  word.y_axis = LCD_CENTER_JUSTIFIED;
				//时&分
				word.kerning = dial->Time.hourNum.kerning;
				word.bckgrndcolor = dial->Time.hourNum.numImg.bckgrndcolor;
				word.forecolor = dial->Time.hourNum.numImg.forecolor;
				memset(str,0,sizeof(str));
				sprintf(str,"%02d:%02d", time->ui32Hour,time->ui32Minute);
				LCD_SetNumber(str, &word);

                
				
				//分
			/*	word.x_axis = dial->Time.minNum.numImg.x_axis;

				word.y_axis = LCD_PIXEL_ONELINE_MAX/2 + dial->Time.minNum.numImg.width/2 + dial->Time.minNum.kerning;
				word.kerning = dial->Time.minNum.kerning;
				word.bckgrndcolor = dial->Time.minNum.numImg.bckgrndcolor;
				word.forecolor = dial->Time.minNum.numImg.forecolor;
				memset(str,0,sizeof(str));
				sprintf(str,"%02d", time->ui32Minute);
				LCD_SetNumber(str, &word);*/
			}
			else
			{
				//时
				
				word.x_axis = dial->Time.hourNum.numImg.x_axis;
				word.y_axis = dial->Time.hourNum.numImg.y_axis;
				word.kerning = dial->Time.hourNum.kerning;
				word.bckgrndcolor = dial->Time.hourNum.numImg.bckgrndcolor;
				word.forecolor = dial->Time.hourNum.numImg.forecolor;
				memset(str,0,sizeof(str));
				sprintf(str,"%02d", time->ui32Hour);
				LCD_SetNumber(str, &word);
				
				//分
				word.x_axis = dial->Time.minNum.numImg.x_axis;
				word.y_axis = dial->Time.minNum.numImg.y_axis;
				word.kerning = dial->Time.minNum.kerning;
				word.bckgrndcolor = dial->Time.minNum.numImg.bckgrndcolor;
				word.forecolor = dial->Time.minNum.numImg.forecolor;
				memset(str,0,sizeof(str));
				sprintf(str,"%02d", time->ui32Minute);
				LCD_SetNumber(str, &word);
			}
			break;
		case DIAL_TIME_TYPE_NUMBER_SECOND:
			
		
			break;
		case DIAL_TIME_TYPE_NUMBER_NO_SECOND:		//01:01
		case DIAL_TIME_TYPE_NUMBER_NO_SECOND_1:		//1:01
		case DIAL_TIME_TYPE_NUMBER_NO_SECOND_2:		//1:1
			//时
			memset(str,0,sizeof(str));
			if((dial->Time.type == DIAL_TIME_TYPE_NUMBER_NO_SECOND_1)
				|| (dial->Time.type == DIAL_TIME_TYPE_NUMBER_NO_SECOND_2))
			{
				sprintf(str,"%d", time->ui32Hour);
			}
			else
			{
				sprintf(str,"%02d", time->ui32Hour);
			}
			len = strlen(str)*(dial->Time.hourNum.numImg.width + dial->Time.hourNum.kerning) - dial->Time.hourNum.kerning;
		
			//分
			memset(str2,0,sizeof(str2));
			if(dial->Time.type == DIAL_TIME_TYPE_NUMBER_NO_SECOND_2)
			{
				sprintf(str2,"%d", time->ui32Minute);
			}
			else
			{
				sprintf(str2,"%02d", time->ui32Minute);
			}
			len2 = strlen(str2)*(dial->Time.minNum.numImg.width + dial->Time.minNum.kerning) - dial->Time.minNum.kerning;
			
			if(dial->Time.align == DIAL_ACTIVEX_ALIGN_LEFT_0)
			{
				//合并左对齐
				hour_y = dial->Time.hourNum.numImg.y_axis;
				
				min_y = hour_y + len + dial->Time.gap;
			}
			else if(dial->Time.align == DIAL_ACTIVEX_ALIGN_RIGHT_0)
			{
				//合并右对齐
				hour_y = dial->Time.hourNum.numImg.y_axis - len - dial->Time.gap - len2;
				
				min_y = hour_y + len + dial->Time.gap;
			}
			else if(dial->Time.align == DIAL_ACTIVEX_ALIGN_CENTER_1)
			{
				//居中
				hour_y = dial->Time.hourNum.numImg.y_axis - (len/2);
				
				min_y = dial->Time.minNum.numImg.y_axis - (len2/2);
			}
			else if(dial->Time.align == DIAL_ACTIVEX_ALIGN_LEFT_1)
			{
				//左对齐
				hour_y = dial->Time.hourNum.numImg.y_axis;
				
				min_y = dial->Time.minNum.numImg.y_axis;
			}
			else if(dial->Time.align == DIAL_ACTIVEX_ALIGN_RIGHT_1)
			{
				//右对齐
				hour_y = dial->Time.hourNum.numImg.y_axis - len;
				
				min_y = dial->Time.minNum.numImg.y_axis - len2;
			}
			else
			{
				//合并居中
				if(dial->Time.hourNum.numImg.y_axis == LCD_CENTER_JUSTIFIED)
				{
					hour_y = (LCD_PIXEL_ONELINE_MAX - len - dial->Time.gap - len2)/2;
				}
				else
				{
					if(dial->Number <= 17)
					{
						//兼容以前的表盘
						hour_y = dial->Time.hourNum.numImg.y_axis;
					}
					else
					{
						hour_y = (dial->Time.hourNum.numImg.y_axis - len - dial->Time.gap - len2)/2;
					}
				}
				
				min_y = hour_y + len + dial->Time.gap;
			}
			
				//时
				word.size = LCD_FONT_dial_72_SIZE;
				word.x_axis = dial->Time.hourNum.numImg.x_axis;
				word.y_axis = hour_y;
				word.kerning = dial->Time.hourNum.kerning;
				word.bckgrndcolor = dial->Time.hourNum.numImg.bckgrndcolor;
				word.forecolor = dial->Time.hourNum.numImg.forecolor;
				LCD_SetNumber(str, &word);
				
				//分
				word.x_axis = dial->Time.minNum.numImg.x_axis;
				word.y_axis = min_y;
				word.kerning = dial->Time.minNum.kerning;
				word.bckgrndcolor = dial->Time.minNum.numImg.bckgrndcolor;
				word.forecolor = dial->Time.minNum.numImg.forecolor;
				LCD_SetNumber(str2, &word);
			//时
			//LCD_SetNumber_ExtFlash(str, DIAL_HOUR_IMG_ADDRESS + (m_dial_num * DIAL_DATA_SIZE) + m_gial_addr_offset, &dial->Time.hourNum, hour_y);
			
			//冒号
			if(dial->Time.colon == 1)
			{
				temp = hour_y + len;
				if(dial->Time.gap >= dial->Time.hourNum.numImg.width)
				{
					temp += ((dial->Time.gap - dial->Time.hourNum.numImg.width)/2);
				}
				else
				{
					temp -= ((dial->Time.hourNum.numImg.width - dial->Time.gap)/2);
				}
				LCD_SetNumber_ExtFlash(":", DIAL_HOUR_IMG_ADDRESS + (m_dial_num * DIAL_DATA_SIZE) + m_gial_addr_offset, &dial->Time.hourNum, temp);
			}
			
			//分
			LCD_SetNumber_ExtFlash(str2, DIAL_MIN_IMG_ADDRESS + (m_dial_num * DIAL_DATA_SIZE) + m_gial_addr_offset, &dial->Time.minNum, min_y);
			break;

		case	DIAL_TIME_TYPE_NUMBER_NO_SECOND_3:
            word.size = LCD_FONT_41_SIZE;
			word.x_axis = dial->Time.hourNum.numImg.x_axis;
			
			
			if(time->ui32Hour==11)
				{
                 
				 kern1 =  dial->Time.hourNum.kerning - 2*14;

			   }
				
			else if((time->ui32Hour==1)||(time->ui32Hour==21)||((time->ui32Hour>=10)&&(time->ui32Hour<=19)))
					{
					kern1 =  dial->Time.hourNum.kerning - 14 - 4;
                     
          
				   }
			
			else
				{
			       kern1 = dial->Time.hourNum.kerning - 2*4;
				}
                 if(time->ui32Minute==11)
                 	{

                      
					  kern2 =  dial->Time.minNum.kerning - 2*14;
				    }
				else if((time->ui32Minute==1)||(time->ui32Minute==21)||(time->ui32Minute==31)||(time->ui32Minute==41)||(time->ui32Minute==51)
					||((time->ui32Minute>=10)&&(time->ui32Minute<=19)))
					{
                     kern2 =  dial->Time.minNum.kerning - 14 - 4;

				   }
				else
					{
                   
                     
                     kern2 = dial->Time.minNum.kerning - 2*4;
				   }
				if(((time->ui32Hour==1)||(time->ui32Hour==11)||(time->ui32Hour==21))&&((time->ui32Minute>=10)&&(time->ui32Minute<=19)))
					{
                   time_gap = dial->Time.gap - 2* 14;


				  }
				else if(((time->ui32Hour==1)||(time->ui32Hour==11)||(time->ui32Hour==21))||((time->ui32Minute>=10)&&(time->ui32Minute<=19)))
					{
                     
					 time_gap = dial->Time.gap - 14 - 4;
				    }
				
				else
					{
                       time_gap = dial->Time.gap;
				   }

          	word.y_axis = 120 - ((dial->Time.hourNum.numImg.width*2) + kern1 + (dial->Time.minNum.numImg.width*2) + kern2 + time_gap)/2 ;
           			//时
				
				word.bckgrndcolor = dial->Time.hourNum.numImg.bckgrndcolor;
				word.forecolor = dial->Time.hourNum.numImg.forecolor;
				word.kerning = kern1;
				memset(str,0,sizeof(str));
				sprintf(str,"%02d", time->ui32Hour);
				
				LCD_SetNumber(str, &word);
				
				//分
				word.x_axis = dial->Time.minNum.numImg.x_axis;
				word.y_axis += dial->Time.hourNum.numImg.width*2 + kern1 + time_gap;
				word.kerning = kern2;
				word.bckgrndcolor = dial->Time.minNum.numImg.bckgrndcolor;
				word.forecolor = dial->Time.minNum.numImg.forecolor;
				memset(str,0,sizeof(str));
				sprintf(str,"%02d", time->ui32Minute);
			
				LCD_SetNumber(str, &word);
			
			break;
		case DIAL_TIME_TYPE_WATCHHAND_1:
			//时针
			gui_clockline_1(time->ui32Hour%12*30 + (time->ui32Minute/2), 5, 30, 60);
			
			//分针
			gui_clockline_1(time->ui32Minute*6, 5, 30, 100);
			
			//圆点
			LCD_SetCircle(LCD_CENTER_LINE - 1, LCD_CENTER_ROW, 5, LCD_WHITE, 0, LCD_FILL_ENABLE);
			LCD_SetCircle(LCD_CENTER_LINE - 1, LCD_CENTER_ROW, 7, LCD_LIGHTGRAY, 2, LCD_FILL_DISABLE);
			break;
		case DIAL_TIME_TYPE_WATCHHAND_2:
			//时针
			gui_clockline_1(time->ui32Hour%12*30 + (time->ui32Minute/2), 7, 30, 60);
			
			//分针
			gui_clockline_1(time->ui32Minute*6, 7, 30, 100);
			
			//圆点
			LCD_SetCircle(LCD_CENTER_LINE - 1, LCD_CENTER_ROW, 7, LCD_RED, 0, LCD_FILL_ENABLE);
			LCD_SetCircle(LCD_CENTER_LINE - 1, LCD_CENTER_ROW, 9, LCD_LIGHTGRAY, 2, LCD_FILL_DISABLE);
			break;
		case DIAL_TIME_TYPE_WATCHHAND_3:
			//时针
			gui_clockline_3(time->ui32Hour%12*30 + (time->ui32Minute/2), 5, 20, 74, dial->Time.hourNum.numImg.forecolor);
		
			//分针
			gui_clockline_3(time->ui32Minute*6, 5, 20, 110, dial->Time.hourNum.numImg.forecolor);
		
			//圆点
			LCD_SetCircle(LCD_CENTER_LINE - 1, LCD_CENTER_ROW, 10, dial->Time.hourNum.numImg.bckgrndcolor, 0, LCD_FILL_ENABLE);
			LCD_SetCircle(LCD_CENTER_LINE - 1, LCD_CENTER_ROW, 9, dial->Time.hourNum.numImg.forecolor, 0, LCD_FILL_ENABLE);
			LCD_SetCircle(LCD_CENTER_LINE - 1, LCD_CENTER_ROW, 6, dial->Time.secNum.numImg.bckgrndcolor, 0, LCD_FILL_ENABLE);
			LCD_SetCircle(LCD_CENTER_LINE - 1, LCD_CENTER_ROW, 5, dial->Time.secNum.numImg.forecolor, 0, LCD_FILL_ENABLE);
			break;
		case DIAL_TIME_TYPE_WATCHHAND_4:
			//时针
			gui_clockline_4(time->ui32Hour%12*30 + (time->ui32Minute/2), 60, 30, 18, dial->Time.hourNum.numImg.forecolor);

			//分针
			gui_clockline_4(time->ui32Minute*6, 90, 40, 18, dial->Time.minNum.numImg.forecolor);
		
			//圆点
			LCD_SetCircle(LCD_CENTER_LINE, LCD_CENTER_ROW, 7, LCD_BLACK, 0, LCD_FILL_ENABLE);
			LCD_SetCircle(LCD_CENTER_LINE, LCD_CENTER_ROW, 6, LCD_GRAY, 0, LCD_FILL_ENABLE);
			LCD_SetCircle(LCD_CENTER_LINE, LCD_CENTER_ROW, 3, LCD_WHITE, 0, LCD_FILL_ENABLE);
			break;
		case DIAL_TIME_TYPE_WATCHHAND_5:
			//时针
			gui_clockline_6(time->ui32Hour%12*30 + (time->ui32Minute/2), 56, 14, 
							dial->Time.hourNum.numImg.bckgrndcolor, dial->Background.color);
					
			//分针
			gui_clockline_5(time->ui32Minute*6, 76, 14, dial->Time.minNum.numImg.bckgrndcolor, dial->Background.color);
			
			//圆点
			LCD_SetCircle(LCD_CENTER_LINE, LCD_CENTER_ROW, 9, dial->Time.hourNum.numImg.bckgrndcolor, 0, LCD_FILL_ENABLE);
			LCD_SetCircle(LCD_CENTER_LINE, LCD_CENTER_ROW, 2, LCD_GRAY, 0, LCD_FILL_ENABLE);
			LCD_SetCircle(LCD_CENTER_LINE, LCD_CENTER_ROW, 6, LCD_GRAY, 1, LCD_FILL_DISABLE);
			LCD_SetCircle(LCD_CENTER_LINE, LCD_CENTER_ROW, 9, LCD_GRAY,1, LCD_FILL_DISABLE);
			break;
		case DIAL_TIME_TYPE_WATCHHAND_6:
				//时针
			gui_clockline_7(time->ui32Hour%12*30 + (time->ui32Minute/2), 5, 30, 60);
			
			//分针
			gui_clockline_7(time->ui32Minute*6, 5, 30, 90);
			
			//圆点
			LCD_SetCircle(LCD_CENTER_LINE, LCD_CENTER_ROW, 4, LCD_WHITE, 4, LCD_FILL_ENABLE);

			//圆环
			LCD_SetCircle(LCD_CENTER_LINE, LCD_CENTER_ROW, 8, LCD_GRAY, 4, LCD_FILL_DISABLE);

			//圆环黑边
			LCD_SetCircle(LCD_CENTER_LINE, LCD_CENTER_ROW, 10, LCD_BLACK, 2, LCD_FILL_DISABLE);
			break;
		default:
			break;
	}
}


static void gui_dial_date(DialDataStr *dial)
{
	SetWord_t word = {0};
	char str[20] = {0};
	
//	char str1[10] = {0};
	
	//char str2[10] = {0};
	uint8_t y;
	uint8_t temp;
	uint16_t len1,len2,len;
	
	switch(dial->Date.type)
	{
		case DIAL_DATE_TYPE_DEFAULT:	//1-19 周五 (自制字体)
		case DIAL_DATE_TYPE_CENTER_5:	//1-19 五 (自制字体)
		case DIAL_DATE_TYPE_CENTER_7:	//19 五 (自制字体)
		
		
			memset(str,0,sizeof(str));
			if(dial->Date.type == DIAL_DATE_TYPE_CENTER_7)
			{
				sprintf(str,"%d", RTC_time.ui32DayOfMonth);
			}
			else
			{
				sprintf(str,"%d-%d", RTC_time.ui32Month,RTC_time.ui32DayOfMonth);
				
				
			}
			
			//日期长度
			len1 = (dial->Date.dateStr.width + dial->Date.dateStr.kerning)*strlen(str) - dial->Date.dateStr.kerning; 
			       
			if((dial->Date.type == DIAL_DATE_TYPE_DEFAULT)||(dial->Date.type == DIAL_DATE_TYPE_CENTER_11))
			{
				len2 = dial->Date.weekStr.width*2 + dial->Date.weekStr.kerning;
			}
			else
			{
				len2 = dial->Date.weekStr.width;
			}
			
			if(dial->Date.align == DIAL_ACTIVEX_ALIGN_CENTER_0)
			{
				//合并居中显示
				if(dial->Date.dateStr.y_axis == LCD_CENTER_JUSTIFIED)
				{
					y = LCD_CENTER_ROW - ((len1 + len2 + dial->Date.gap)/2);
				}
				else
				{
					y = dial->Date.dateStr.y_axis - ((len1 + len2 + dial->Date.gap)/2);
				}
					
				temp = y + len1 + dial->Date.gap;
			}
			else if(dial->Date.align == DIAL_ACTIVEX_ALIGN_LEFT_0)
			{
				//合并左对齐
				y = dial->Date.dateStr.y_axis;
				temp = y + len1 + dial->Date.gap;
			}
			else if(dial->Date.align == DIAL_ACTIVEX_ALIGN_RIGHT_0)
			{
				//合并右对齐
				y = dial->Date.dateStr.y_axis - len1 - len2 - dial->Date.gap;
				temp = y + len1 + dial->Date.gap;
			}
			else if(dial->Date.align == DIAL_ACTIVEX_ALIGN_CENTER_1)
			{
				//分散居中对齐
				y = dial->Date.dateStr.y_axis - (len1/2);
				temp = dial->Date.weekStr.y_axis - (len2/2);
			}
			else if(dial->Date.align == DIAL_ACTIVEX_ALIGN_RIGHT_1)
			{
				//分散右对齐
				y = dial->Date.dateStr.y_axis - len1;
				temp = dial->Date.weekStr.y_axis - len2;
			}
			else if(dial->Date.align ==DIAL_ACTIVEX_ALIGN_6)
				{
                   //分散左对齐同时中间对齐
				y = dial->Date.dateStr.y_axis;
				temp = y + len1/2 - len2/2;
			    }
			else
			{
				//分散左对齐
				y = dial->Date.dateStr.y_axis;
				temp = dial->Date.weekStr.y_axis;
			}
			
			//边框
			if((dial->Date.borderType == 1)
				|| (dial->Date.borderType == 3))
			{
				//日期
				if(dial->Date.borderType == 3)
				{
					LCD_SetRectangle(dial->Date.dateStr.x_axis - 2 - dial->Date.borderThick, dial->Date.dateStr.height + 4 + (dial->Date.borderThick*2), 
										y - 3 - dial->Date.borderThick, len1 + 6 + (dial->Date.borderThick*2), 
										dial->Date.dateStr.bckgrndcolor, 0, 0, LCD_FILL_ENABLE);
				}
				LCD_SetRectangle(dial->Date.dateStr.x_axis - 2 - dial->Date.borderThick, dial->Date.dateStr.height + 4 + (dial->Date.borderThick*2), 
									y - 3 - dial->Date.borderThick, len1 + 6 + (dial->Date.borderThick*2), 
									dial->Date.borderColor, dial->Date.borderThick, dial->Date.borderThick, LCD_FILL_DISABLE);
			}
			else if((dial->Date.borderType == 2)
				|| (dial->Date.borderType == 4))
			{
				//日期 + 星期
				if(dial->Date.borderType == 4)
				{
					LCD_SetRectangle(dial->Date.dateStr.x_axis - 2 - dial->Date.borderThick, dial->Date.dateStr.height + 4 + (dial->Date.borderThick*2),
										y - 3 - dial->Date.borderThick, len1 + len2 + dial->Date.gap + 6 + (dial->Date.borderThick*2), 
										dial->Date.dateStr.bckgrndcolor, 0, 0, LCD_FILL_ENABLE);
				}
				LCD_SetRectangle(dial->Date.dateStr.x_axis - 2 - dial->Date.borderThick, dial->Date.dateStr.height + 4 + (dial->Date.borderThick*2),
									y - 3 - dial->Date.borderThick, len1 + len2 + dial->Date.gap + 6 + (dial->Date.borderThick*2), 
									dial->Date.borderColor, dial->Date.borderThick, dial->Date.borderThick, LCD_FILL_DISABLE);
			}
			
			//日期
			if(dial->Date.dateStr.x_axis != 0xFF)
			{
				word.x_axis = dial->Date.dateStr.x_axis;
				word.y_axis = y;
				word.size = (LCD_FONTSIZE)dial->Date.dateStr.size;
				//为了兼容以前的主题，将16和18的字号替换
				/*if(dial->Date.dateStr.size == LCD_FONT_16_SIZE)
				{
					//细长字体
					word.size = LCD_FONT_18_SIZE;
				}
				else if(dial->Date.dateStr.size == LCD_FONT_18_SIZE)
				{
					//粗字体
					word.size = LCD_FONT_16_SIZE;
				}*/
				word.kerning = dial->Date.dateStr.kerning;
				word.bckgrndcolor = dial->Date.dateStr.bckgrndcolor;
				word.forecolor = dial->Date.dateStr.forecolor;
				LCD_SetNumber(str,&word);
			
			}
			
			//周
			if(dial->Date.weekStr.x_axis != 0xFF)
			{
				word.x_axis = dial->Date.weekStr.x_axis;
				word.y_axis = temp;
				if(dial->Date.type == DIAL_DATE_TYPE_DEFAULT)
				{
					LCD_SetPicture(word.x_axis, word.y_axis, dial->Date.weekStr.forecolor, dial->Date.weekStr.bckgrndcolor, &img_week_13x13[7]);
					if(dial->Date.weekStr.kerning == 0)
					{
						//防止旧主题显示错误
						dial->Date.weekStr.kerning = 2;
					}
					word.y_axis += (img_week_13x13[0].width + dial->Date.weekStr.kerning);
					
					LCD_SetPicture(word.x_axis, word.y_axis, dial->Date.weekStr.forecolor, dial->Date.weekStr.bckgrndcolor, &img_week_13x13[RTC_time.ui32Weekday]);
				}
				else if(dial->Date.type == DIAL_DATE_TYPE_CENTER_11)
				{
					LCD_SetPicture(word.x_axis, word.y_axis, dial->Date.weekStr.forecolor, dial->Date.weekStr.bckgrndcolor, &img_week_20x19[7]);
					if(dial->Date.weekStr.kerning == 0)
					{
						//防止旧主题显示错误
						dial->Date.weekStr.kerning = 2;
					}
					word.y_axis += (img_week_20x19[0].width + dial->Date.weekStr.kerning);
					
					LCD_SetPicture(word.x_axis, word.y_axis, dial->Date.weekStr.forecolor, dial->Date.weekStr.bckgrndcolor, &img_week_20x19[RTC_time.ui32Weekday]);
				}
				else;
			}
			break;
		case DIAL_DATE_TYPE_CENTER_1:	//1-19 周五 (宋体)
		case DIAL_DATE_TYPE_CENTER_2:	//1-19 五 (宋体)
		case DIAL_DATE_TYPE_CENTER_3:	//1月19日 周五 (宋体)
		case DIAL_DATE_TYPE_CENTER_4:	//1月19日 五 (宋体)
		case DIAL_DATE_TYPE_CENTER_8:	//19 五 (宋体)
		case DIAL_DATE_TYPE_CENTER_9:	//FIR 19 (宋体)
		{
			char str2[10] = {0};
			
			memset(str,0,sizeof(str));
			memset(str2,0,sizeof(str));
			
			//兼容以前旧主题
			if((dial->Date.dateStr.type == DIAL_STRING_TYPE_SONG) 
				&& (dial->Date.dateStr.size == LCD_FONT_16_SIZE))
			{
				dial->Date.dateStr.width = 8;
				dial->Date.dateStr.height = 16;
			}
			if((dial->Date.weekStr.type == DIAL_STRING_TYPE_SONG) 
				&& (dial->Date.weekStr.size == LCD_FONT_16_SIZE))
			{
				dial->Date.weekStr.width = 8;
				dial->Date.weekStr.height = 16;
			}
			
			if(dial->Date.type == DIAL_DATE_TYPE_CENTER_1)
			{
				//日期长度
				sprintf(str,"%d-%d", RTC_time.ui32Month,RTC_time.ui32DayOfMonth);
				len1 = (dial->Date.dateStr.width*strlen(str)) + (dial->Date.dateStr.kerning*(strlen(str)-1));
				
				//星期长度
				sprintf(str2,"%s", (char *)WeekStrs[L_CHINESE][RTC_time.ui32Weekday]);
				len2 = (dial->Date.weekStr.width*strlen(str2)) + (dial->Date.weekStr.kerning*(strlen(str2)/2 - 1));
			}
			else if(dial->Date.type == DIAL_DATE_TYPE_CENTER_2)
			{
				//日期长度
				sprintf(str,"%d-%d", RTC_time.ui32Month,RTC_time.ui32DayOfMonth);
				len1 = (dial->Date.dateStr.width*strlen(str)) + (dial->Date.dateStr.kerning*(strlen(str)-1));
				
				//星期长度
				sprintf(str2,"%s", (char *)WeekofDayStrs[L_CHINESE][RTC_time.ui32Weekday]);
				len2 = (dial->Date.weekStr.width*strlen(str2)) + (dial->Date.weekStr.kerning*(strlen(str2)/2 - 1));
			}
			else if(dial->Date.type == DIAL_DATE_TYPE_CENTER_3)
			{
				//日期长度
				sprintf(str,"%d月%d日", RTC_time.ui32Month,RTC_time.ui32DayOfMonth);
				len1 = (dial->Date.dateStr.width*strlen(str)) + (dial->Date.dateStr.kerning*(strlen(str)-3));
				
				//星期长度
				sprintf(str2,"%s", (char *)WeekStrs[L_CHINESE][RTC_time.ui32Weekday]);
				len2 = (dial->Date.weekStr.width*strlen(str2)) + (dial->Date.weekStr.kerning*(strlen(str2)/2 - 1));
			}
			else if(dial->Date.type == DIAL_DATE_TYPE_CENTER_4)
			{
				//日期长度
				sprintf(str,"%d月%d日", RTC_time.ui32Month,RTC_time.ui32DayOfMonth);
				len1 = (dial->Date.dateStr.width*strlen(str)) + (dial->Date.dateStr.kerning*(strlen(str)-3));
				
				//星期长度
				sprintf(str2,"%s", (char *)WeekofDayStrs[L_CHINESE][RTC_time.ui32Weekday]);
				len2 = (dial->Date.weekStr.width*strlen(str2)) + (dial->Date.weekStr.kerning*(strlen(str2)/2 - 1));
			}
			else if(dial->Date.type == DIAL_DATE_TYPE_CENTER_8)
			{
				//日期长度
				sprintf(str,"%d", RTC_time.ui32DayOfMonth);
				len1 = (dial->Date.dateStr.width*strlen(str)) + (dial->Date.dateStr.kerning*(strlen(str)-1));
				
				//星期长度
				sprintf(str2,"%s", (char *)WeekofDayStrs[L_CHINESE][RTC_time.ui32Weekday]);
				len2 = (dial->Date.weekStr.width*strlen(str2)) + (dial->Date.weekStr.kerning*(strlen(str2)/2 - 1));
			}
			else if(dial->Date.type == DIAL_DATE_TYPE_CENTER_9)
			{
				//星期长度
				const char* weekBuf[] = {"SUN","MON","TUE","WED","THU","FRI","SAT"};//固定字符，不根据语言设置变化
				sprintf(str,"%s", (char *)weekBuf[RTC_time.ui32Weekday%7]);
				len1 = (dial->Date.weekStr.width*strlen(str)) + (dial->Date.weekStr.kerning*(strlen(str) - 1));
				
				//日期长度
				sprintf(str2,"%d", RTC_time.ui32DayOfMonth);
				len2 = (dial->Date.dateStr.width*strlen(str2)) + (dial->Date.dateStr.kerning*(strlen(str2)-1));
			}
			
			//计算日期和星期的位置
			if(dial->Date.align == DIAL_ACTIVEX_ALIGN_CENTER_0)
			{
				//合并居中显示
				if(dial->Date.dateStr.y_axis == LCD_CENTER_JUSTIFIED)
				{
					y = LCD_CENTER_ROW - ((len1 + len2 + dial->Date.gap)/2);
				}
				else
				{
					y = dial->Date.dateStr.y_axis - ((len1 + len2 + dial->Date.gap)/2);
				}
					
				temp = y + len1 + dial->Date.gap;
			}
			else if(dial->Date.align == DIAL_ACTIVEX_ALIGN_LEFT_0)
			{
				//合并左对齐
				y = dial->Date.dateStr.y_axis;
				temp = y + len1 + dial->Date.gap;
			}
			else if(dial->Date.align == DIAL_ACTIVEX_ALIGN_RIGHT_0)
			{
				//合并右对齐
				y = dial->Date.dateStr.y_axis - len1 - len2 - dial->Date.gap;
				temp = y + len1 + dial->Date.gap;
			}
			else if(dial->Date.align == DIAL_ACTIVEX_ALIGN_CENTER_1)
			{
				//分散居中对齐
				y = dial->Date.dateStr.y_axis - (len1/2);
				temp = dial->Date.weekStr.y_axis - (len2/2);
			}
			else if(dial->Date.align == DIAL_ACTIVEX_ALIGN_RIGHT_1)
			{
				//分散右对齐
				y = dial->Date.dateStr.y_axis - len1;
				temp = dial->Date.weekStr.y_axis - len2;
			}
			else
			{
				//分散左对齐
				y = dial->Date.dateStr.y_axis;
				temp = dial->Date.weekStr.y_axis;
			}
			
			//边框
			if((dial->Date.borderType == 1)
				|| (dial->Date.borderType == 3))
			{
				//日期
				if(dial->Date.borderType == 3)
				{
					LCD_SetRectangle(dial->Date.dateStr.x_axis - 1 - dial->Date.borderThick, dial->Date.dateStr.height + 2 + (dial->Date.borderThick*2), 
										y - 3 - dial->Date.borderThick, len1 + 6 + (dial->Date.borderThick*2), 
										dial->Date.dateStr.bckgrndcolor, 0, 0, LCD_FILL_ENABLE);
				}
				LCD_SetRectangle(dial->Date.dateStr.x_axis - 1 - dial->Date.borderThick, dial->Date.dateStr.height + 2 + (dial->Date.borderThick*2), 
									y - 3 - dial->Date.borderThick, len1 + 6 + (dial->Date.borderThick*2), 
									dial->Date.borderColor, dial->Date.borderThick, dial->Date.borderThick, LCD_FILL_DISABLE);
			}
			else if((dial->Date.borderType == 2)
				|| (dial->Date.borderType == 4))
			{
				//日期 + 星期
				if(dial->Date.borderType == 4)
				{
					LCD_SetRectangle(dial->Date.dateStr.x_axis - 1 - dial->Date.borderThick, dial->Date.dateStr.height + 2 + (dial->Date.borderThick*2),
										y - 3 - dial->Date.borderThick, len1 + len2 + dial->Date.gap + 6 + (dial->Date.borderThick*2), 
										dial->Date.dateStr.bckgrndcolor, 0, 0, LCD_FILL_ENABLE);
				}
				LCD_SetRectangle(dial->Date.dateStr.x_axis - 1 - dial->Date.borderThick, dial->Date.dateStr.height + 2 + (dial->Date.borderThick*2),
									y - 3 - dial->Date.borderThick, len1 + len2 + dial->Date.gap + 6 + (dial->Date.borderThick*2), 
									dial->Date.borderColor, dial->Date.borderThick, dial->Date.borderThick, LCD_FILL_DISABLE);
			}
			
			//日期
			if(dial->Date.dateStr.x_axis != 0xFF)
			{
				LCD_SetString_ExtFlash(str, &dial->Date.dateStr, y);
			}
		
			//星期
			if(dial->Number == 6)
			{
				//兼容活力世界杯的主题
				dial->Date.weekStr.x_axis = dial->Date.dateStr.x_axis;
			}
			if(dial->Date.weekStr.x_axis != 0xFF)
			{
				LCD_SetString_ExtFlash(str2, &dial->Date.weekStr, temp);
			}
		}
		
		break;
	
      case DIAL_DATE_TYPE_CENTER_10://商务指针主题日期
       
        
		memset(str,0,sizeof(str));
		sprintf(str,"%d", RTC_time.ui32DayOfMonth);
		
		len = (dial->Date.dateStr.width + dial->Date.dateStr.kerning)*strlen(str) - dial->Date.dateStr.kerning;
		
		//LCD_SetRectangle(dial->Date.dateStr.x_axis - 2 - 2,  2 ,word.y_axis - 4 - 2, len1 + 6*2,dial->Date.dateStr.forecolor, 0, 0, LCD_FILL_ENABLE);
		
		//LCD_SetRectangle(dial->Date.dateStr.x_axis - 4,  dial->Date.dateStr.height + 8 ,dial->Date.dateStr.y_axis + 4 , 2,dial->Date.dateStr.forecolor, 0, 0, LCD_FILL_ENABLE);

		//LCD_SetRectangle(dial->Date.dateStr.x_axis + dial->Date.dateStr.height + 2, 2 ,word.y_axis - 4 -2, len1 + 6*2,dial->Date.dateStr.forecolor, 0, 0, LCD_FILL_ENABLE);
		//len1 = (dial->Date.dateStr.width + dial->Date.dateStr.kerning)*strlen(str) - dial->Date.dateStr.kerning;
		
		//y = dial->Date.dateStr.y_axis - len1;
		word.x_axis = dial->Date.dateStr.x_axis;
		word.y_axis = dial->Date.dateStr.y_axis;
		word.forecolor = dial->Date.dateStr.forecolor;
	    word.bckgrndcolor = dial->Date.dateStr.bckgrndcolor;
		word.size = dial->Date.dateStr.size;
	    word.kerning = dial->Date.dateStr.kerning;

		LCD_SetRectangle(dial->Date.dateStr.x_axis - 6,  dial->Date.dateStr.height + 6*2 ,word.y_axis - 14, 2,LCD_WHITE, 0, 0, LCD_FILL_ENABLE);
		
		LCD_SetRectangle(dial->Date.dateStr.x_axis - 6,  2 ,word.y_axis - 14, len + 14*2,LCD_WHITE, 0, 0, LCD_FILL_ENABLE);
		
		LCD_SetRectangle(dial->Date.dateStr.x_axis - 6,  dial->Date.dateStr.height + 6*2 ,dial->Date.dateStr.y_axis + len + 14 - 2 , 2,LCD_WHITE, 0, 0, LCD_FILL_ENABLE);

		LCD_SetRectangle(dial->Date.dateStr.x_axis + dial->Date.dateStr.height + 6 - 2, 2 ,word.y_axis - 14, len + 14*2,LCD_WHITE, 0, 0, LCD_FILL_ENABLE);

		LCD_SetRectangle(dial->Date.dateStr.x_axis - 2 ,  dial->Date.dateStr.height + 2*2 ,word.y_axis - 10, len+ 10*2,LCD_WHITE, 0, 0, LCD_FILL_ENABLE);
       
	    LCD_SetNumber(str,&word);

	  	break;
		
		case DIAL_DATE_TYPE_CENTER_11://炫酷运动主题日期
			memset(str,0,sizeof(str));
			if(dial->Date.type == DIAL_DATE_TYPE_CENTER_7)
			{
				sprintf(str,"%d", RTC_time.ui32DayOfMonth);
			}
			else
			{
				
				sprintf(str,"%d-%d-%d", RTC_time.ui32Year+2000,RTC_time.ui32Month,RTC_time.ui32DayOfMonth);
			}
			
			//日期长度
			len1 = (dial->Date.dateStr.width + dial->Date.dateStr.kerning)*strlen(str) - dial->Date.dateStr.kerning;
		
			len2 = dial->Date.weekStr.width*2 + dial->Date.weekStr.kerning;
		
		
			
			if(dial->Date.align == DIAL_ACTIVEX_ALIGN_CENTER_0)
			{
				//合并居中显示
				if(dial->Date.dateStr.y_axis == LCD_CENTER_JUSTIFIED)
				{
					y = LCD_CENTER_ROW - ((len1 + len2 + dial->Date.gap)/2);
				}
				else
				{
					y = dial->Date.dateStr.y_axis - ((len1 + len2 + dial->Date.gap)/2);
				}
					
				temp = y + len1 + dial->Date.gap;
			}
		
			else
			{
				//分散左对齐
				y = dial->Date.dateStr.y_axis;
				temp = dial->Date.weekStr.y_axis;
			}
			
		
			
			//日期
			if(dial->Date.dateStr.x_axis != 0xFF)
			{
				word.x_axis = dial->Date.dateStr.x_axis;
				word.y_axis = y;
				word.size = (LCD_FONTSIZE)dial->Date.dateStr.size;
			
				word.kerning = dial->Date.dateStr.kerning;
				word.bckgrndcolor = dial->Date.dateStr.bckgrndcolor;
				word.forecolor = dial->Date.dateStr.forecolor;
				LCD_SetNumber(str,&word);
			}
			
			//周
			if(dial->Date.weekStr.x_axis != 0xFF)
			{
				word.x_axis = dial->Date.weekStr.x_axis;
				word.y_axis = temp;
					LCD_SetPicture(word.x_axis, word.y_axis, dial->Date.weekStr.forecolor, dial->Date.weekStr.bckgrndcolor, &img_week_13x13[7]);
					if(dial->Date.weekStr.kerning == 0)
					{
						//防止旧主题显示错误
						dial->Date.weekStr.kerning = 2;
					}
					word.y_axis += (img_week_13x13[0].width + dial->Date.weekStr.kerning);
					
					LCD_SetPicture(word.x_axis, word.y_axis, dial->Date.weekStr.forecolor, dial->Date.weekStr.bckgrndcolor, &img_week_13x13[RTC_time.ui32Weekday]);

				
			}
		break;
		case DIAL_DATE_TYPE_CENTER_12://炫酷运动主题日期
			memset(str,0,sizeof(str));
			if(dial->Date.type == DIAL_DATE_TYPE_CENTER_7)
			{
				sprintf(str,"%d", RTC_time.ui32DayOfMonth);
			}
			else
			{
				
				sprintf(str,"%d-%d-%d", RTC_time.ui32Year+2000,RTC_time.ui32Month,RTC_time.ui32DayOfMonth);
			}
			
			//日期长度
			len1 = (dial->Date.dateStr.width + dial->Date.dateStr.kerning)*strlen(str) - dial->Date.dateStr.kerning;
		
			len2 = dial->Date.weekStr.width*2 + dial->Date.weekStr.kerning;
		
		
			
			if(dial->Date.align == DIAL_ACTIVEX_ALIGN_CENTER_0)
			{
				//合并居中显示
				if(dial->Date.dateStr.y_axis == LCD_CENTER_JUSTIFIED)
				{
					y = LCD_CENTER_ROW - ((len1 + len2 + dial->Date.gap)/2);
				}
				else
				{
					y = dial->Date.dateStr.y_axis - ((len1 + len2 + dial->Date.gap)/2);
				}
					
				temp = y + len1 + dial->Date.gap;
			}
		
			else
			{
				//分散左对齐
				y = dial->Date.dateStr.y_axis;
				temp = dial->Date.weekStr.y_axis;
			}
			
		
			
			//日期
			if(dial->Date.dateStr.x_axis != 0xFF)
			{
				word.x_axis = dial->Date.dateStr.x_axis;
				word.y_axis = y;
				word.size = (LCD_FONTSIZE)dial->Date.dateStr.size;
			
				word.kerning = dial->Date.dateStr.kerning;
				word.bckgrndcolor = dial->Date.dateStr.bckgrndcolor;
				word.forecolor = dial->Date.dateStr.forecolor;
				LCD_SetNumber(str,&word);
			}
			
			//周
			if(dial->Date.weekStr.x_axis != 0xFF)
			{
				word.x_axis = dial->Date.weekStr.x_axis;
				word.y_axis = temp;
					//LCD_SetPicture(word.x_axis, word.y_axis, dial->Date.weekStr.forecolor, dial->Date.weekStr.bckgrndcolor, &img_week_13x13[7]);
					if(dial->Date.weekStr.kerning == 0)
					{
						//防止旧主题显示错误
						dial->Date.weekStr.kerning = 2;
					}
					word.y_axis += (img_week_13x13[0].width + dial->Date.weekStr.kerning);
					
					//LCD_SetPicture(word.x_axis, word.y_axis, dial->Date.weekStr.forecolor, dial->Date.weekStr.bckgrndcolor, &img_week_13x13[RTC_time.ui32Weekday]);

						//星期长度
				sprintf(str,"%s", (char *)WeekStrs[L_CHINESE][RTC_time.ui32Weekday]);
				//len2 = (dial->Date.weekStr.width*strlen(str2)) + (dial->Date.weekStr.kerning*(strlen(str2)/2 - 1));
				LCD_SetString_ExtFlash(str, &dial->Date.weekStr, temp);
				
			}
		break;
		default:
			break;
	}
}
	/*天气(或训练计划)-勿扰状态-蓝牙-闹钟*/
static void gui_dial_widget_type_1(uint8_t alarm_flag, uint8_t ble_flag, uint8_t dnd_flag, uint8_t bat_flag, uint8_t plan_or_weather_flag, DialDataStr *dial)
{
	char str[20] = {0};
	uint8_t y,len = 0;
	uint8_t temp,color;
	uint8_t percent = g_bat_evt.level_percent;
	if(percent > 100)
		{
			percent = 100;
		}
    	if(percent <= 10)
			{
				color = dial->Battery.batImg.lowColor;
			}
			else
			{
				color = dial->Battery.batImg.normalColor;
			}

	#if defined COD
	//温度值的长度
	
	if(plan_or_weather_flag)
    {	
   
      memset(str,0,sizeof(str));
	   sprintf(str,"%d",g_weather_info.cod_weather[dial_weather_index].temperature);//温度
      len  = strlen(str)*(dial->Widget.dial_weather.d_weatherStr.width + dial->Widget.dial_weather.d_weatherStr.kerning) - dial->Widget.dial_weather.d_weatherStr.kerning;
	}

	
	if((dial_weather_index < 24)&&(g_weather_info.cod_weather[dial_weather_index].weather<=14))
		{
	 temp = (dial->Widget.dial_weather.d_weatherImg.width + dial->Widget.gap + len +  2 +  img_dial_temp.width + dial->Widget.gap)*plan_or_weather_flag//天气
	        + (dial->Widget.ble.connectImg.width + dial->Widget.gap)*ble_flag//蓝牙
	        + (dial->Widget.DNDImg.width + dial->Widget.gap)*dnd_flag //勿扰
			+ (dial->Widget.alarmImg.width + dial->Widget.gap)*alarm_flag //闹钟
	  	 	+ (dial->Battery.batImg.batImg.width+ dial->Widget.gap)*bat_flag//电池
         	-  dial->Widget.gap
			;
		}
	else
		{
          temp = (img_weather_update_small.width + dial->Widget.gap)*plan_or_weather_flag//天气
	        + (dial->Widget.ble.connectImg.width + dial->Widget.gap)*ble_flag//蓝牙
	        + (dial->Widget.DNDImg.width + dial->Widget.gap)*dnd_flag //勿扰
			+ (dial->Widget.alarmImg.width + dial->Widget.gap)*alarm_flag //闹钟
	  	 	+ (dial->Battery.batImg.batImg.width+ dial->Widget.gap)*bat_flag//电池
         	-  dial->Widget.gap
			;
 


	   }
	
	#else


	
	if(plan_or_weather_flag)//训练计划
	{
		//训练计划时间
		memset(str,0,sizeof(str));
		sprintf(str,"%02d:%02d", TrainPlanDownloadSettingStr.Hint.Hour,TrainPlanDownloadSettingStr.Hint.Minute);
		len = strlen(str)*(dial->Widget.plan.timeStr.width + dial->Widget.plan.timeStr.kerning) - dial->Widget.plan.timeStr.kerning;
		//冒号宽度
		len -= (dial->Widget.plan.timeStr.width/2);
	}
    temp = (dial->Widget.DNDImg.width + dial->Widget.gap)*dnd_flag 
				+ (dial->Widget.alarmImg.width + dial->Widget.gap)*alarm_flag
				+ (dial->Widget.ble.connectImg.width + dial->Widget.gap)*ble_flag
				+ (dial->Widget.msgImg.width + dial->Widget.gap)*bat_flag
				+ (dial->Widget.plan.gap + dial->Widget.plan.sportImg.width + 4 + len + dial->Widget.gap)*plan_or_weather_flag
				- dial->Widget.gap;
	
	#endif
	
		
	if(dial->Widget.align == DIAL_ACTIVEX_ALIGN_CENTER_0)
	{
		//居中显示
		if(dial->Widget.ble.connectImg.y_axis == LCD_CENTER_JUSTIFIED)
		{
			y = (LCD_PIXEL_ONELINE_MAX - temp)/2;
		}
		else
		{
			y = dial->Widget.ble.connectImg.y_axis - (temp/2);
		}
	}
	else if(dial->Widget.align == DIAL_ACTIVEX_ALIGN_RIGHT_0)
	{
		//右对齐
		y = dial->Widget.ble.connectImg.y_axis - temp;
	}
	else if(dial->Widget.align == DIAL_ACTIVEX_ALIGN_7)
		{
        
		y = dial->Widget.ble.connectImg.y_axis - temp/2;
          

	   }
	else
	{
		//左对齐
		y = dial->Widget.ble.connectImg.y_axis;
	}	
  //天气或训练计划
#if defined COD
     
	if(plan_or_weather_flag)
		{
        
		 if((dial_weather_index < 24)&&(g_weather_info.cod_weather[dial_weather_index].weather<=14))
		 	{
           memset(str,0,sizeof(str));
	       sprintf(str,"%d",g_weather_info.cod_weather[dial_weather_index].temperature);//温度
           
		   //天气图标
			  LCD_SetPicture(dial->Widget.dial_weather.d_weatherImg.x_axis, y,calendar_weather_data[g_weather_info.cod_weather[dial_weather_index].weather],
			   LCD_NONE,&img_dial_weather_lists[g_weather_info.cod_weather[dial_weather_index].weather]);

		
		   y += (img_dial_weather_lists[0].width + dial->Widget.gap);
          //温度值
          LCD_SetString_ExtFlash(str,&dial->Widget.dial_weather.d_weatherStr,y);
		   y += len + 4;
		   //度图标
		   LCD_SetPicture(dial->Widget.dial_weather.d_weatherImg.x_axis + dial->Widget.dial_weather.d_weatherImg.height/2 - img_dial_temp.height/2,y,LCD_WHITE,LCD_NONE,&img_dial_temp);
		   y += img_dial_temp.width + dial->Widget.gap;
		 	}
		 else
		 	{
              LCD_SetPicture(dial->Widget.dial_weather.d_weatherImg.x_axis, y,LCD_WHITE, LCD_NONE,&img_weather_update_small);
			  
			  y += img_weather_update_small.width + dial->Widget.gap;

		   }
	    }
   #else
	if(plan_or_weather_flag)
	{
		if(alarm_flag || ble_flag || dnd_flag)
		{
			y += (dial->Widget.gap + dial->Widget.plan.gap);
		}
		
		//马拉松图标
		if(dial->Widget.type == DIAL_WIDGET_TYPE_1)
		{
			//使用外部flash图片
			LCD_SetPicture_ExtFlash(DIAL_PLAN_IMG_ADDRESS + (m_dial_num * DIAL_DATA_SIZE) + m_gial_addr_offset, &dial->Widget.plan.sportImg, y);
		}
		else
		{
			//使用内部flash图片
			LCD_SetPicture(dial->Widget.plan.sportImg.x_axis, y, dial->Widget.plan.sportImg.forecolor, dial->Widget.plan.sportImg.bckgrndcolor, &Img_Marathon_21X18);
		}
		
		//时间
		y += (dial->Widget.plan.sportImg.width + 4);
		LCD_SetString_ExtFlash(str, &dial->Widget.plan.timeStr, y);
	}
#endif
	
	
	//蓝牙图标
	if(ble_flag)
	{
		if(SetValue.SwBle == BLE_DISCONNECT)
		{
			//蓝牙未连接
			if(dial->Widget.type == DIAL_WIDGET_TYPE_1)
			{
				//使用外部flash图片
				LCD_SetPicture_ExtFlash(DIAL_BLE_DISCONN_IMG_ADDRESS + (m_dial_num * DIAL_DATA_SIZE) + m_gial_addr_offset, &dial->Widget.ble.disconnectImg, y);
			}
			else
			{
				//使用内部flash图片
				LCD_SetPicture(dial->Widget.ble.disconnectImg.x_axis, y, dial->Widget.ble.disconnectImg.forecolor, dial->Widget.ble.disconnectImg.bckgrndcolor, &img_dial_ble);
			}
			y += (dial->Widget.ble.disconnectImg.width + dial->Widget.gap);
		}
		else
		{
			//蓝牙已连接
			if(dial->Widget.type == DIAL_WIDGET_TYPE_1)
			{
				//使用外部flash图片
				LCD_SetPicture_ExtFlash(DIAL_BLE_CONN_IMG_ADDRESS + (m_dial_num * DIAL_DATA_SIZE) + m_gial_addr_offset, &dial->Widget.ble.connectImg, y);
			}
			else
			{
				//使用内部flash图片
				LCD_SetPicture(dial->Widget.ble.connectImg.x_axis, y, dial->Widget.ble.connectImg.forecolor, dial->Widget.ble.connectImg.bckgrndcolor, &img_dial_ble);
			}
			y += (dial->Widget.ble.connectImg.width+ dial->Widget.gap);
		}	
	}

		//勿扰图标
	if(dnd_flag)
	{
		if(dial->Widget.type == DIAL_WIDGET_TYPE_1)
		{
			//使用外部flash图片
			LCD_SetPicture_ExtFlash(DIAL_DND_IMG_ADDRESS + (m_dial_num * DIAL_DATA_SIZE) + m_gial_addr_offset, &dial->Widget.DNDImg, y);
		}
		else
		{
			//使用内部flash图片
			LCD_SetPicture(dial->Widget.DNDImg.x_axis, y, dial->Widget.DNDImg.forecolor, dial->Widget.DNDImg.bckgrndcolor, &img_dial_dndisturb);
		}
		
		y += (dial->Widget.DNDImg.width + dial->Widget.gap);
	}
	
		//闹钟图标
	if(alarm_flag)
	{
		if(dial->Widget.type == DIAL_WIDGET_TYPE_1)
		{
			//使用外部flash图片
			LCD_SetPicture_ExtFlash(DIAL_ALARM_IMG_ADDRESS + (m_dial_num * DIAL_DATA_SIZE) + m_gial_addr_offset, &dial->Widget.alarmImg, y);
		}
		else
		{
			//使用内部flash图片
			LCD_SetPicture(dial->Widget.alarmImg.x_axis+1, y, dial->Widget.alarmImg.forecolor, dial->Widget.alarmImg.bckgrndcolor, &img_dial_alarm);
		}
		y += (dial->Widget.alarmImg.width + dial->Widget.gap);
	}

	//电池
	if(bat_flag)
	{
		if(dial->Widget.type == DIAL_WIDGET_TYPE_1)
		{
			//使用外部flash图片
			LCD_SetPicture_ExtFlash(DIAL_MSG_IMG_ADDRESS + (m_dial_num * DIAL_DATA_SIZE) + m_gial_addr_offset, &dial->Widget.msgImg, y);
		}
		else
		{
			//使用内部flash图片
			LCD_SetPicture(dial->Battery.batImg.batImg.x_axis, y,dial->Battery.batImg.batImg.forecolor, dial->Battery.batImg.batImg.bckgrndcolor, &img_dial_bat_28x13);

			LCD_SetRectangle(dial->Battery.batImg.batImg.x_axis + 1, dial->Battery.batImg.batImg.height - 2, y + 1, percent * dial->Battery.batImg.width / 100, 
								color, 0, 0, LCD_FILL_ENABLE);
		}
		y += (dial->Battery.batImg.batImg.width + dial->Widget.gap);
	}
	
}

static void gui_dial_widget(DialDataStr *dial)
{
	uint8_t dnd_flag =0,alarm_flag=0,ble_flag=0,plan_flag=0;
	uint8_t bat_flag=0,weather_flag = 0;
	
	//勿扰状态
	if((system_silent_mode_get())&& (dial->Widget.DNDImg.x_axis != 0xFF))
	{
		//开启勿扰模式
		dnd_flag = 1;
	}
	else
	{
		//关闭勿扰模式
		dnd_flag = 0;
	}
	
	//闹钟状态
	if((SetValue.alarm_instance[0].enable | SetValue.alarm_instance[1].enable |
		SetValue.alarm_instance[2].enable | SetValue.alarm_instance[3].enable)
		&& (dial->Widget.alarmImg.x_axis != 0xFF))
	{
		//开启闹钟
		alarm_flag = 1;
	}
	else
	{
		//关闭闹钟
		alarm_flag = 0;
	}
	
	//蓝牙状态
	if((SetValue.SwBle > 0)
		&& (dial->Widget.ble.connectImg.x_axis != 0xFF))
	{
		//开启蓝牙
		ble_flag = 1;
	}
	else
	{
		//关闭蓝牙
		ble_flag = 0;
	}
	
	//电池信息
	if(dial->Battery.type ==DIAL_BATTERY_TYPE_4)
		
	{
		bat_flag = 1;
	}
	else
	{
		bat_flag = 0;
	}
	
	//天气或训练计划
	#if defined COD
	
	if((dial->Widget.dial_weather.d_weatherImg.x_axis!= 0xFF)&&(dial->Weather.type == DIAL_WEATHER_TYPE_3))
		 {
			weather_flag = 1;
			gui_dial_get_weather();
		}
		else
		{
			weather_flag = 0;
		} 

	#else
	

  	if((Get_TrainPlanFlag() == 1)
			&& (dial->Widget.plan.sportImg.x_axis != 0xFF))
		{
			//训练计划未完成
			plan_flag = 1;
		}
		else
		{
			//无训练计划或者训练计划已完成
			plan_flag = 0;
		}

	
	#endif
	
	switch(dial->Widget.type)
	{
		case DIAL_WIDGET_TYPE_DEFAULT:	//默认图标 勿扰 闹钟 蓝牙 训练计划 (居中显示)
		case DIAL_WIDGET_TYPE_1:		//外部flash图标 勿扰 闹钟 蓝牙 训练计划 (居中显示)
		 
       #if defined COD
			gui_dial_widget_type_1(alarm_flag, ble_flag, dnd_flag, bat_flag, weather_flag, dial);
	   #else
	   
	      gui_dial_widget_type_1(alarm_flag, ble_flag, dnd_flag, bat_flag, plan_flag, dial);
	   #endif
			break;
		case DIAL_WIDGET_TYPE_2:
			//蓝牙 (居中显示)
			//蓝牙 电量
			//此蓝牙图标在电量函数中一起显示
			
			break;
		default:
			break;
	}
}


static void gui_dial_status(DialDataStr *dial)
{
	char str1[20] = {0};
	char str2[20] = {0};
	uint16_t y,temp;
	uint16_t len,len1,len2;
	float s_percent,d_percent,c_percent;
	uint32_t step = Get_TotalStep();
	uint32_t distance = Get_TotalDistance();	//厘米
	uint32_t calorie = Get_TotalEnergy();		//卡
	SetWord_t word = {0};
	uint8_t width_percent;
	//步数完成度
	if(step > SetValue.AppSet.Goal_Steps)
	{
		//达到目标值
		s_percent = 100;
	}
	else
	{
		if(SetValue.AppSet.Goal_Steps > 0)
		{
			s_percent = (step*100.f/SetValue.AppSet.Goal_Steps);
		}
		else
		{
			s_percent = 0;
		}
	}
	
	//里程完成度
	if(distance > (SetValue.AppSet.Goal_Distance*100000))
	{
		//达到目标值
		d_percent = 100;
	}
	else
	{
		if((SetValue.AppSet.Goal_Distance > 0) && (distance >= 10000)) //因为保留一位小数,>=0.1千米是才有显示
		{
			d_percent = (distance*100.f/(SetValue.AppSet.Goal_Distance*100000));
		}
		else
		{
			d_percent = 0;
		}
	}
	
	//热量完成度
	if(calorie > (SetValue.AppSet.Goal_Energy*1000))
	{
		//达到目标值
		c_percent = 100;
	}
	else
	{
		if((SetValue.AppSet.Goal_Energy > 0) && (calorie >= 50))//因为保留一位小数,>=0.1千卡是才有显示
		{
			c_percent = (calorie*100.f/(SetValue.AppSet.Goal_Energy*1000));
		}
		else
		{
			c_percent = 0;
		}
	}
	switch(dial->Status.Type1.type)
	{
		case DIAL_STATUS_TYPE_DEFAULT:	//步数 完成度
			//步数完成度
	            width_percent = (uint8_t)(dial->Status.Type1.stepDeg.width*d_percent/100);
				
				LCD_SetRectangle(120 - 20 -2, 24,0, 240, LCD_GRAY, 0, 0, LCD_FILL_ENABLE);
				
			
			memset(str1,0,sizeof(str1));
			sprintf(str1,"%d", step);		
			memset(str2,0,sizeof(SetValue.AppSet.Goal_Steps));
			sprintf(str2,"%d", SetValue.AppSet.Goal_Steps);
			len1 = strlen(str1)*(dial->Status.Type1.step.stepStr.width + dial->Status.Type1.step.stepStr.kerning) - dial->Status.Type1.step.stepStr.kerning;
			len2 = strlen(str2)*(dial->Status.Type1.step.stepStr.width + dial->Status.Type1.step.stepStr.kerning) - dial->Status.Type1.step.stepStr.kerning;
			len =  len1  + len2  + img_dial_bias_5x17.width + 2*dial->Status.Type1.step.gap;
			if(dial->Status.Type1.align1 == DIAL_ACTIVEX_ALIGN_CENTER_1)
			{
				//居中
				y = dial->Status.Type1.step.stepStr.y_axis - (len/2);
			}
			else if(dial->Status.Type1.align1 == DIAL_ACTIVEX_ALIGN_RIGHT_1)
			{
				//右对齐
				y = dial->Status.Type1.step.stepStr.y_axis - len;
			}
		   else if(dial->Status.Type1.align1 == DIAL_ACTIVEX_ALIGN_CENTER_0)
		   	{

           
		      y = 120 - img_dial_bias_5x17.width/2 - 2 - len1;

		    }
			else
			{
				y = dial->Status.Type1.step.stepStr.y_axis;
			}
			//步数图标
			LCD_SetPicture(dial->Status.Type1.step.stepImg.x_axis, y - 4 - img_dial_step_16x16.width, dial->Status.Type1.step.stepImg.forecolor, dial->Status.Type1.step.stepImg.bckgrndcolor, &img_dial_step_16x16);
            //步数
			LCD_SetString_ExtFlash(str1, &dial->Status.Type1.step.stepStr, y);
			
           //斜杠
		   
		   LCD_SetPicture(dial->Status.Type1.step.stepStr.x_axis, LCD_CENTER_JUSTIFIED, dial->Status.Type1.step.stepStr.forecolor, dial->Status.Type1.step.stepStr.bckgrndcolor, &img_dial_bias_5x17);
              y = 120 + img_dial_bias_5x17.width/2 + 2;
			  //步数目标值
			LCD_SetString_ExtFlash(str2, &dial->Status.Type1.step.stepStr, y);
			break;

			
			case DIAL_STATUS_TYPE_DEFAULT_1:

				
			//步数
			memset(str1,0,sizeof(str1));
			sprintf(str1,"%d", step);
			len = strlen(str1)*(dial->Status.Type1.step.stepStr.width + dial->Status.Type1.step.stepStr.kerning)
					- dial->Status.Type1.step.stepStr.kerning;
			
			if(dial->Status.Type1.align1 == DIAL_ACTIVEX_ALIGN_CENTER_1)
			{
				//居中
				y = dial->Status.Type1.step.stepStr.y_axis - (len/2);
			}
			else if(dial->Status.Type1.align1 == DIAL_ACTIVEX_ALIGN_CENTER_0)
				{

                 
				 y = 120 - (len/2);

			     }
			else if(dial->Status.Type1.align1 == DIAL_ACTIVEX_ALIGN_RIGHT_1)
			{
				//右对齐
				y = dial->Status.Type1.step.stepStr.y_axis - len;
			}
			else
			{
				y = dial->Status.Type1.step.stepStr.y_axis;
			}
			
			LCD_SetString_ExtFlash(str1, &dial->Status.Type1.step.stepStr, y);
		
			//步数图标
			LCD_SetPicture(dial->Status.Type1.step.stepImg.x_axis, y-4-dial->Status.Type1.step.stepImg.height, 
							dial->Status.Type1.step.stepImg.forecolor, dial->Status.Type1.step.stepImg.bckgrndcolor, &img_dial_step_16x16);
				break;

			case DIAL_STATUS_TYPE_DEFAULT_2:
				
			//步数
			memset(str1,0,sizeof(str1));
			sprintf(str1,"%d", step);
			len = strlen(str1)*(dial->Status.Type1.step.stepStr.width + dial->Status.Type1.step.stepStr.kerning)
					- dial->Status.Type1.step.stepStr.kerning;
			
			if(dial->Status.Type1.align1 == DIAL_ACTIVEX_ALIGN_CENTER_1)
			{
				//居中
				y = dial->Status.Type1.step.stepStr.y_axis - (len/2);
			}
			else if(dial->Status.Type1.align1 == DIAL_ACTIVEX_ALIGN_CENTER_0)
				{

                 
				 y = 120 - (len/2);

			     }
			else if(dial->Status.Type1.align1 == DIAL_ACTIVEX_ALIGN_RIGHT_1)
			{
				//右对齐
				y = dial->Status.Type1.step.stepStr.y_axis - len;
			}
			else
			{
				y = dial->Status.Type1.step.stepStr.y_axis;
			}
			
			LCD_SetString_ExtFlash(str1, &dial->Status.Type1.step.stepStr, y);
		
			//步数图标
			LCD_SetPicture(dial->Status.Type1.step.stepImg.x_axis, dial->Status.Type1.step.stepImg.y_axis-(dial->Status.Type1.step.stepImg.height/2), 
							dial->Status.Type1.step.stepImg.forecolor, dial->Status.Type1.step.stepImg.bckgrndcolor, &img_dial_step_16x16);
				break;
		case DIAL_STATUS_TYPE_1:	//步数 里程
		case DIAL_STATUS_TYPE_2:	//步数 目标（内部flash）
		case DIAL_STATUS_TYPE_7:	//步数 目标（外部flash）
			//步数完成度
			gui_progess_bar(s_percent, &dial->Status.Type1.stepDeg);
			
			//步数
			memset(str1,0,sizeof(str1));
			sprintf(str1,"%d", step);
			if(dial->Status.Type1.step.stepStr.width == 0)
			{
				//兼容以前表盘
				dial->Status.Type1.step.stepStr.width = 8;
			}
			len = strlen(str1)*(dial->Status.Type1.step.stepStr.width + dial->Status.Type1.step.stepStr.kerning)
					- dial->Status.Type1.step.stepStr.kerning + dial->Status.Type1.step.stepImg.width + dial->Status.Type1.step.gap;
		
			memset(str2,0,sizeof(str2));
			if((dial->Status.Type1.type == DIAL_STATUS_TYPE_2)
				|| (dial->Status.Type1.type == DIAL_STATUS_TYPE_7))
			{
				//目标
				sprintf(str2,"%d", SetValue.AppSet.Goal_Steps);
			}
			else
			{
				//里程
				#if defined (SPRINTF_FLOAT_TO_INT)
				sprintf(str2,"%d.%02d",distance/100000,distance/1000%100);
				#else
				sprintf(str2,"%.2f", distance/100000.f);
				#endif
				if(dial->Status.Type1.distance.stepStr.width == 0)
				{
					//兼容以前表盘
					dial->Status.Type1.distance.stepStr.width = 8;
					dial->Status.Type1.gap = 8;
				}
			}
			len2 = strlen(str2)*(dial->Status.Type1.distance.stepStr.width + dial->Status.Type1.distance.stepStr.kerning)
					- dial->Status.Type1.distance.stepStr.kerning + dial->Status.Type1.distance.stepImg.width + dial->Status.Type1.distance.gap;
		
			if(dial->Status.Type1.align1 == DIAL_ACTIVEX_ALIGN_LEFT_0)
			{
				//合并左对齐
				y = dial->Status.Type1.step.stepStr.y_axis;
				
				temp = y + len + dial->Status.Type1.gap;
			}
			else if(dial->Status.Type1.align1 == DIAL_ACTIVEX_ALIGN_RIGHT_0)
			{
				//合并右对齐
				y = dial->Status.Type1.step.stepStr.y_axis - (len + dial->Status.Type1.gap + len2);
				
				temp = y + len + dial->Status.Type1.gap;
			}
			else if(dial->Status.Type1.align1 == DIAL_ACTIVEX_ALIGN_CENTER_1)
			{
				//居中
				y = dial->Status.Type1.step.stepStr.y_axis - (len/2);
				
				temp = dial->Status.Type1.distance.stepStr.y_axis - (len2/2);
			}
			else if(dial->Status.Type1.align1 == DIAL_ACTIVEX_ALIGN_LEFT_1)
			{
				//左对齐
				y = dial->Status.Type1.step.stepStr.y_axis;
				
				temp = dial->Status.Type1.distance.stepStr.y_axis;
			}
			else if(dial->Status.Type1.align1 == DIAL_ACTIVEX_ALIGN_RIGHT_1)
			{
				//右对齐
				y = dial->Status.Type1.step.stepStr.y_axis - len;
				
				temp = dial->Status.Type1.distance.stepStr.y_axis - len2;
			}
			else
			{
				//合并居中，兼容以前表盘
				if(dial->Status.Type1.step.stepStr.y_axis == LCD_CENTER_JUSTIFIED)
				{
					y = (LCD_PIXEL_ONELINE_MAX  - len - dial->Status.Type1.gap - len2)/2;
				}
				else
				{
					y = dial->Status.Type1.step.stepStr.y_axis - ((len + dial->Status.Type1.gap + len2)/2);
				}
				
				temp = y + len + dial->Status.Type1.gap;
			}
			
			//步数图标
			if(dial->Status.Type1.type == DIAL_STATUS_TYPE_2)
			{
				LCD_SetPicture(dial->Status.Type1.step.stepImg.x_axis, y, dial->Status.Type1.step.stepImg.forecolor,
								dial->Status.Type1.step.stepImg.bckgrndcolor, &Img_Step_12X16);
			}
			else
			{
				LCD_SetPicture_ExtFlash(DIAL_STEP_IMG_ADDRESS + (m_dial_num * DIAL_DATA_SIZE) + m_gial_addr_offset, &dial->Status.Type1.step.stepImg, y);
			}
		
			//步数
			y += (dial->Status.Type1.step.stepImg.width + dial->Status.Type1.step.gap);
			LCD_SetString_ExtFlash(str1, &dial->Status.Type1.step.stepStr, y);
			
			
			if(dial->Status.Type1.type == DIAL_STATUS_TYPE_2)
			{
				//目标步数图标
				LCD_SetPicture(dial->Status.Type1.distance.stepImg.x_axis, temp, dial->Status.Type1.distance.stepImg.forecolor,
								dial->Status.Type1.distance.stepImg.bckgrndcolor, &Img_StepTarget_12X16);
			}
			else
			{
				//里程图标
				LCD_SetPicture_ExtFlash(DIAL_DISTANCE_IMG_ADDRESS + (m_dial_num * DIAL_DATA_SIZE) + m_gial_addr_offset, &dial->Status.Type1.distance.stepImg, temp);
			}
			
			//里程
			temp += (dial->Status.Type1.distance.stepImg.width + dial->Status.Type1.distance.gap);
			LCD_SetString_ExtFlash(str2, &dial->Status.Type1.distance.stepStr, temp);
			break;
		case DIAL_STATUS_TYPE_3:
			//步数
			gui_progress_circle(LCD_CENTER_LINE, 52, 44, 5, 360, 360, s_percent, dial->Status.Type1.stepDeg.forecolor, dial->Status.Type1.stepDeg.bckgrndcolor);
			LCD_SetPicture(94 - 2, 52 - 10, LCD_WHITE, LCD_NONE, &img_dial_step_big);
		
			memset(str1,0,sizeof(str1));
			sprintf(str1,"%d", step);
		
			word.size = LCD_FONT_16_SIZE;
			word.forecolor = LCD_WHITE;
			word.bckgrndcolor = LCD_NONE;
			word.kerning = 1;
			word.x_axis = LCD_CENTER_LINE + 4;
			word.y_axis = 52 - ((Font_Number_16.width + word.kerning)*strlen(str1) - word.kerning)/2;
			LCD_SetNumber(str1,&word);
			
			//卡路里
			gui_progress_circle(LCD_CENTER_LINE, 68 + 120, 44, 5, 360, 360, c_percent, dial->Status.Type1.stepDeg.forecolor, dial->Status.Type1.stepDeg.bckgrndcolor);
			LCD_SetPicture(94 - 2, 68 + 120 - 10, LCD_WHITE, LCD_NONE, &Img_Calorie_20X24);
			
			memset(str1,0,sizeof(str1));
			sprintf(str1,"%d", calorie/1000);
			
			word.y_axis = 68 + 120 - ((Font_Number_16.width + word.kerning)*strlen(str1) - word.kerning)/2;
			LCD_SetNumber(str1,&word);
		
			//里程
			gui_progress_circle(192, LCD_CENTER_ROW, 44, 5, 360, 360, d_percent, dial->Status.Type1.stepDeg.forecolor, dial->Status.Type1.stepDeg.bckgrndcolor);
			LCD_SetPicture(168 - 2, LCD_CENTER_JUSTIFIED, LCD_WHITE, LCD_NONE, &Img_Distance_28X20);
		
			memset(str1,0,sizeof(str1));
			#if defined (SPRINTF_FLOAT_TO_INT)
			sprintf(str1,"%d.%01d",distance/100000,distance/10000%10);
			#else
			sprintf(str1,"%.1f", distance/100000.f);
			#endif
			
			word.x_axis = 192 + 4;
			word.y_axis = LCD_CENTER_JUSTIFIED;
			LCD_SetNumber(str1,&word);
			break;
		case DIAL_STATUS_TYPE_4:	//步数进度条 步数图标 步数数据 目标步数图标 目标步数 里程图标 里程数据 卡路里图标 卡路里数据
		case DIAL_STATUS_TYPE_5:    //无里程图标，有里程单位 (0.0km)
			//步数进度条
			gui_progess_bar(s_percent, &dial->Status.Type2.stepDeg);
			if(dial->Status.Type2.stepDeg.type == DIAL_PROGRESSBAR_TYPE_6)
			{
				//进度条上有图标
				if(step > 0)
				{
					dial->Graph.Type2.stepImg.forecolor = dial->Status.Type2.stepDeg.forecolor;
				}
				else
				{
					dial->Graph.Type2.stepImg.forecolor = dial->Status.Type2.stepDeg.bckgrndcolor;
				}
				LCD_SetPicture_ExtFlash(DIAL_STEPDEG_IMG_ADDRESS + (m_dial_num * DIAL_DATA_SIZE) + m_gial_addr_offset, 
											&dial->Graph.Type2.stepImg, dial->Graph.Type2.stepImg.y_axis);
			}
			
			//步数数据
			if((dial->Status.Type2.step.stepImg.x_axis != 0xFF)
				|| (dial->Status.Type2.step.stepStr.x_axis != 0xFF))
			{
				if(dial->Status.Type1.type == DIAL_STATUS_TYPE_6)
				{
					//步数/目标步数 (999/1000)
					memset(str1,0,sizeof(str1));
					sprintf(str1,"%d|%d", step,SetValue.AppSet.Goal_Steps);
				}
				else
				{
					memset(str1,0,sizeof(str1));
					sprintf(str1,"%d", step);
				}
				len = (strlen(str1)*(dial->Status.Type2.step.stepStr.width + dial->Status.Type2.step.stepStr.kerning)
						- dial->Status.Type2.step.stepStr.kerning);
				
				if((dial->Status.Type2.align1&0x0F) == DIAL_ACTIVEX_ALIGN_CENTER_0)
				{
					//合并居中
					if(dial->Status.Type2.step.stepStr.y_axis == LCD_CENTER_JUSTIFIED)
					{
						y = (LCD_PIXEL_ONELINE_MAX - (len + dial->Status.Type2.step.gap + dial->Status.Type2.step.stepImg.width))/2;
					}
					else
					{
						y = dial->Status.Type2.step.stepStr.y_axis - ((len + dial->Status.Type2.step.gap + dial->Status.Type2.step.stepImg.width)/2);
					}
					
					temp = y + dial->Status.Type2.step.stepImg.width + dial->Status.Type2.step.gap;
				}
				else if((dial->Status.Type2.align1&0x0F) == DIAL_ACTIVEX_ALIGN_LEFT_0)
				{
					//合并左对齐
					y = dial->Status.Type2.step.stepStr.y_axis;
					
					temp = y + dial->Status.Type2.step.stepImg.width + dial->Status.Type2.step.gap;
				}
				else if((dial->Status.Type2.align1&0x0F) == DIAL_ACTIVEX_ALIGN_RIGHT_0)
				{
					//合并右对齐
					y = dial->Status.Type2.step.stepStr.y_axis - (len + dial->Status.Type2.step.gap + dial->Status.Type2.step.stepImg.width);
					
					temp = y + dial->Status.Type2.step.stepImg.width + dial->Status.Type2.step.gap;
				}
				else if((dial->Status.Type2.align1&0x0F) == DIAL_ACTIVEX_ALIGN_CENTER_1)
				{
					//居中
					y = dial->Status.Type2.step.stepImg.y_axis - (dial->Status.Type2.step.stepImg.width/2);
					
					temp = dial->Status.Type2.step.stepStr.y_axis - (len/2);
				}
				else if((dial->Status.Type2.align1&0x0F) == DIAL_ACTIVEX_ALIGN_RIGHT_1)
				{
					//右对齐
					y = dial->Status.Type2.step.stepImg.y_axis - dial->Status.Type2.step.stepImg.width;
					
					temp = dial->Status.Type2.step.stepStr.y_axis - len;
				}
				else
				{
					y = dial->Status.Type2.step.stepImg.y_axis;
					
					temp = dial->Status.Type2.step.stepStr.y_axis;
				}
				
				if(dial->Status.Type2.step.stepImg.x_axis != 0xFF)
				{
					//图标
					LCD_SetPicture_ExtFlash(DIAL_STEP_IMG_ADDRESS + (m_dial_num * DIAL_DATA_SIZE) + m_gial_addr_offset, 
											&dial->Status.Type2.step.stepImg, y);
				}
				
				if(dial->Status.Type2.step.stepStr.x_axis != 0xFF)
				{
					//数据
					LCD_SetString_ExtFlash(str1, &dial->Status.Type2.step.stepStr, temp);
				}
			}
			
			//目标步数
			if((dial->Status.Type2.stepGoal.stepImg.x_axis != 0xFF)
				|| (dial->Status.Type2.stepGoal.stepStr.x_axis != 0xFF))
			{
				memset(str1,0,sizeof(str1));
				sprintf(str1,"%d", SetValue.AppSet.Goal_Steps);
				len = (strlen(str1)*(dial->Status.Type2.stepGoal.stepStr.width + dial->Status.Type2.stepGoal.stepStr.kerning)
						- dial->Status.Type2.stepGoal.stepStr.kerning);
				
				if(((dial->Status.Type2.align1&0xF0) >> 4) == DIAL_ACTIVEX_ALIGN_CENTER_0)
				{
					//合并居中
					if(dial->Status.Type2.stepGoal.stepStr.y_axis == LCD_CENTER_JUSTIFIED)
					{
						y = (LCD_PIXEL_ONELINE_MAX - (len + dial->Status.Type2.stepGoal.gap + dial->Status.Type2.stepGoal.stepImg.width))/2;
					}
					else
					{
						y = dial->Status.Type2.stepGoal.stepStr.y_axis - ((len + dial->Status.Type2.stepGoal.gap + dial->Status.Type2.stepGoal.stepImg.width)/2);
					}
					temp = y + dial->Status.Type2.stepGoal.stepImg.width + dial->Status.Type2.stepGoal.gap;
				}
				else if(((dial->Status.Type2.align1&0xF0) >> 4) == DIAL_ACTIVEX_ALIGN_LEFT_0)
				{
					//合并左对齐
					y = dial->Status.Type2.stepGoal.stepStr.y_axis;
					
					temp = y + dial->Status.Type2.stepGoal.stepImg.width + dial->Status.Type2.stepGoal.gap;
				}
				else if(((dial->Status.Type2.align1&0xF0) >> 4) == DIAL_ACTIVEX_ALIGN_RIGHT_0)
				{
					//合并右对齐
					y = dial->Status.Type2.stepGoal.stepStr.y_axis - (len + dial->Status.Type2.stepGoal.gap + dial->Status.Type2.stepGoal.stepImg.width);
					
					temp = y + dial->Status.Type2.stepGoal.stepImg.width + dial->Status.Type2.stepGoal.gap;
				}
				else if(((dial->Status.Type2.align1&0xF0) >> 4) == DIAL_ACTIVEX_ALIGN_CENTER_1)
				{
					//居中
					y = dial->Status.Type2.stepGoal.stepImg.y_axis - (dial->Status.Type2.stepGoal.stepImg.width/2);
					
					temp = dial->Status.Type2.stepGoal.stepStr.y_axis - (len/2);
				}
				else if(((dial->Status.Type2.align1&0xF0) >> 4) == DIAL_ACTIVEX_ALIGN_RIGHT_1)
				{
					//右对齐
					y = dial->Status.Type2.stepGoal.stepImg.y_axis - dial->Status.Type2.stepGoal.stepImg.width;
					
					temp = dial->Status.Type2.stepGoal.stepStr.y_axis - len;
				}
				else
				{
					y = dial->Status.Type2.stepGoal.stepImg.y_axis;
					
					temp = dial->Status.Type2.stepGoal.stepStr.y_axis;
				}
				
				if(dial->Status.Type2.stepGoal.stepImg.x_axis != 0xFF)
				{
					//图标
					LCD_SetPicture_ExtFlash(DIAL_STEPGOAL_IMG_ADDRESS + (m_dial_num * DIAL_DATA_SIZE) + m_gial_addr_offset, 
											&dial->Status.Type2.stepGoal.stepImg, y);
				}
				
				if(dial->Status.Type2.stepGoal.stepStr.x_axis != 0xFF)
				{
					//数据
					LCD_SetString_ExtFlash(str1, &dial->Status.Type2.stepGoal.stepStr, temp);
				}
			}
			
			//里程
			if((dial->Status.Type2.distance.stepImg.x_axis != 0xFF)
				|| (dial->Status.Type2.distance.stepStr.x_axis != 0xFF))
			{
				if(dial->Status.Type1.type == DIAL_STATUS_TYPE_5)
				{
					//无里程图标，有里程单位 (0.0km)
					memset(str1,0,sizeof(str1));
					#if defined (SPRINTF_FLOAT_TO_INT)
					sprintf(str1,"%d.%02dkm",distance/100000,distance/1000%100);
					#else
					sprintf(str1,"%.2fkm", distance/100000.f);
					#endif
				}
				else
				{
					memset(str1,0,sizeof(str1));
					#if defined (SPRINTF_FLOAT_TO_INT)
					sprintf(str1,"%d.%02d",distance/100000,distance/1000%100);
					#else
					sprintf(str1,"%.2f", distance/100000.f);
					#endif
				}
				len = (strlen(str1)*(dial->Status.Type2.distance.stepStr.width + dial->Status.Type2.distance.stepStr.kerning)
						- dial->Status.Type2.distance.stepStr.kerning);
				
				if((dial->Status.Type2.align2&0x0F) == DIAL_ACTIVEX_ALIGN_CENTER_0)
				{
					//合并居中
					if(dial->Status.Type2.distance.stepStr.y_axis == LCD_CENTER_JUSTIFIED)
					{
						y = (LCD_PIXEL_ONELINE_MAX - (len + dial->Status.Type2.distance.gap + dial->Status.Type2.distance.stepImg.width))/2;
					}
					else
					{
						y = dial->Status.Type2.distance.stepStr.y_axis - ((len + dial->Status.Type2.distance.gap + dial->Status.Type2.distance.stepImg.width)/2);
					}
					
					temp = y + dial->Status.Type2.distance.stepImg.width + dial->Status.Type2.distance.gap;
				}
				else if((dial->Status.Type2.align2&0x0F) == DIAL_ACTIVEX_ALIGN_LEFT_0)
				{
					//合并左对齐
					y = dial->Status.Type2.distance.stepStr.y_axis;
					
					temp = y + dial->Status.Type2.distance.stepImg.width + dial->Status.Type2.distance.gap;
				}
				else if((dial->Status.Type2.align2&0x0F) == DIAL_ACTIVEX_ALIGN_RIGHT_0)
				{
					//合并右对齐
					y = dial->Status.Type2.distance.stepStr.y_axis - (len + dial->Status.Type2.distance.gap + dial->Status.Type2.distance.stepImg.width);
					
					temp = y + dial->Status.Type2.distance.stepImg.width + dial->Status.Type2.distance.gap;
				}
				else if((dial->Status.Type2.align2&0x0F) == DIAL_ACTIVEX_ALIGN_CENTER_1)
				{
					//居中
					y = dial->Status.Type2.distance.stepImg.y_axis - (dial->Status.Type2.distance.stepImg.width/2);
					
					temp = dial->Status.Type2.distance.stepStr.y_axis - (len/2);
				}
				else if((dial->Status.Type2.align2&0x0F) == DIAL_ACTIVEX_ALIGN_RIGHT_1)
				{
					//右对齐
					y = dial->Status.Type2.distance.stepImg.y_axis - dial->Status.Type2.distance.stepImg.width;
					
					temp = dial->Status.Type2.distance.stepStr.y_axis - len;
				}
				else
				{
					y = dial->Status.Type2.distance.stepImg.y_axis;
					
					temp = dial->Status.Type2.distance.stepStr.y_axis;
				}
				
				if(dial->Status.Type2.distance.stepImg.x_axis != 0xFF)
				{
					//图标
					LCD_SetPicture_ExtFlash(DIAL_DISTANCE_IMG_ADDRESS + (m_dial_num * DIAL_DATA_SIZE) + m_gial_addr_offset, 
											&dial->Status.Type2.distance.stepImg, y);
				}
				
				if(dial->Status.Type2.distance.stepStr.x_axis != 0xFF)
				{
					//数据
					LCD_SetString_ExtFlash(str1, &dial->Status.Type2.distance.stepStr, temp);
				}
			}
			
			//卡路里
			if((dial->Status.Type2.calorie.stepImg.x_axis != 0xFF)
				|| (dial->Status.Type2.calorie.stepStr.x_axis != 0xFF))
			{
				memset(str1,0,sizeof(str1));
				sprintf(str1,"%d", calorie/1000);
				len = (strlen(str1)*(dial->Status.Type2.calorie.stepStr.width + dial->Status.Type2.calorie.stepStr.kerning)
						- dial->Status.Type2.calorie.stepStr.kerning);
				
				if(((dial->Status.Type2.align2&0xF0)>>4) == DIAL_ACTIVEX_ALIGN_CENTER_0)
				{
					//合并居中
					if(dial->Status.Type2.calorie.stepStr.y_axis == LCD_CENTER_JUSTIFIED)
					{
						y = (LCD_PIXEL_ONELINE_MAX - (len + dial->Status.Type2.calorie.gap + dial->Status.Type2.calorie.stepImg.width))/2;
					}
					else
					{
						y = dial->Status.Type2.calorie.stepStr.y_axis - ((len + dial->Status.Type2.calorie.gap + dial->Status.Type2.calorie.stepImg.width)/2);
					}
					
					temp = y + dial->Status.Type2.calorie.stepImg.width + dial->Status.Type2.calorie.gap;
				}
				else if(((dial->Status.Type2.align2&0xF0)>>4) == DIAL_ACTIVEX_ALIGN_LEFT_0)
				{
					//合并左对齐
					y = dial->Status.Type2.calorie.stepStr.y_axis;
					
					temp = y + dial->Status.Type2.calorie.stepImg.width + dial->Status.Type2.calorie.gap;
				}
				else if(((dial->Status.Type2.align2&0xF0)>>4) == DIAL_ACTIVEX_ALIGN_RIGHT_0)
				{
					//合并右对齐
					y = dial->Status.Type2.calorie.stepStr.y_axis - (len + dial->Status.Type2.calorie.gap + dial->Status.Type2.calorie.stepImg.width);
					
					temp = y + dial->Status.Type2.calorie.stepImg.width + dial->Status.Type2.calorie.gap;
				}
				else if(((dial->Status.Type2.align2&0xF0)>>4) == DIAL_ACTIVEX_ALIGN_CENTER_1)
				{
					//居中
					y = dial->Status.Type2.calorie.stepImg.y_axis - (dial->Status.Type2.calorie.stepImg.width/2);
					
					temp = dial->Status.Type2.calorie.stepStr.y_axis - (len/2);
				}
				else if(((dial->Status.Type2.align2&0xF0)>>4) == DIAL_ACTIVEX_ALIGN_RIGHT_1)
				{
					//右对齐
					y = dial->Status.Type2.calorie.stepImg.y_axis - dial->Status.Type2.calorie.stepImg.width;
					
					temp = dial->Status.Type2.calorie.stepStr.y_axis - len;
				}
				else
				{
					y = dial->Status.Type2.calorie.stepImg.y_axis;
					
					temp = dial->Status.Type2.calorie.stepStr.y_axis;
				}
				
				if(dial->Status.Type2.calorie.stepImg.x_axis != 0xFF)
				{
					//图标
					LCD_SetPicture_ExtFlash(DIAL_CALORIE_IMG_ADDRESS + (m_dial_num * DIAL_DATA_SIZE) + m_gial_addr_offset, 
											&dial->Status.Type2.calorie.stepImg, y);
				}
				
				if(dial->Status.Type2.calorie.stepStr.x_axis != 0xFF)
				{
					//数据
					LCD_SetString_ExtFlash(str1, &dial->Status.Type2.calorie.stepStr, temp);
				}
			}
			break;
			case DIAL_STATUS_TYPE_6:	//步数/目标步数 (999/1000)(运动休闲)
          	//步数完成度
	            width_percent = (uint8_t)((dial->Status.Type1.stepDeg.width-2)*d_percent/100);
				
				/*LCD_SetRectangle(dial->Status.Type1.stepDeg.star_x, dial->Status.Type1.stepDeg.height,dial->Status.Type1.stepDeg.star_y, 
				width_percent, dial->Status.Type1.stepDeg.forecolor, 0, 0, LCD_FILL_ENABLE);*/
				
				/*LCD_SetRectangle(dial->Status.Type1.stepDeg.star_x, dial->Status.Type1.stepDeg.height,dial->Status.Type1.stepDeg.star_y+width_percent, 
				dial->Status.Type1.stepDeg.width-width_percent, dial->Status.Type1.stepDeg.bckgrndcolor, 0, 0, LCD_FILL_ENABLE);*/

				LCD_SetRectangle(dial->Status.Type1.stepDeg.star_x, dial->Status.Type1.stepDeg.height,dial->Status.Type1.stepDeg.star_y, 
				dial->Status.Type1.stepDeg.width, LCD_WHITE, 0, 0, LCD_FILL_ENABLE);

				LCD_SetRectangle(dial->Status.Type1.stepDeg.star_x+1, dial->Status.Type1.stepDeg.height-2,dial->Status.Type1.stepDeg.star_y+1, 
				dial->Status.Type1.stepDeg.width-2, LCD_BLACK, 0, 0, LCD_FILL_ENABLE);

				LCD_SetRectangle(dial->Status.Type1.stepDeg.star_x+1, dial->Status.Type1.stepDeg.height-2,dial->Status.Type1.stepDeg.star_y+1, 
				width_percent, dial->Status.Type1.stepDeg.forecolor, 0, 0, LCD_FILL_ENABLE);

				//步数图标
			LCD_SetPicture(dial->Status.Type1.step.stepImg.x_axis, dial->Status.Type1.step.stepImg.y_axis, 
							dial->Status.Type1.step.stepImg.forecolor, dial->Status.Type1.step.stepImg.bckgrndcolor, &img_dial_step_16x16);
			//步数
		    memset(str1,0,sizeof(str1));
			sprintf(str1,"%d", step);		
			memset(str2,0,sizeof(SetValue.AppSet.Goal_Steps));
			sprintf(str2,"%d", SetValue.AppSet.Goal_Steps);
			len1 = strlen(str1)*(dial->Status.Type1.step.stepStr.width + dial->Status.Type1.step.stepStr.kerning) - dial->Status.Type1.step.stepStr.kerning;
			len2 = strlen(str2)*(dial->Status.Type1.step.stepStr.width + dial->Status.Type1.step.stepStr.kerning) - dial->Status.Type1.step.stepStr.kerning;
			len =  len1  + len2  + img_dial_bias_5x17.width + 2*dial->Status.Type1.step.gap;
			if(dial->Status.Type1.align1 == DIAL_ACTIVEX_ALIGN_CENTER_1)
			{
				//居中
				y = dial->Status.Type1.step.stepStr.y_axis - (len/2);
			}
			else if(dial->Status.Type1.align1 == DIAL_ACTIVEX_ALIGN_RIGHT_1)
			{
				//右对齐
				y = dial->Status.Type1.step.stepStr.y_axis - len;
			}
		   else if(dial->Status.Type1.align1 == DIAL_ACTIVEX_ALIGN_CENTER_0)
		   	{

           
		      y = 120 - img_dial_bias_5x17.width/2 - dial->Status.Type1.step.gap - len1;

		    }
			else
			{
				y = dial->Status.Type1.step.stepStr.y_axis;
			}
			word.x_axis = dial->Status.Type1.step.stepStr.x_axis;
			word.y_axis = y;
			word.forecolor = LCD_WHITE;
			word.bckgrndcolor = dial->Status.Type1.step.stepStr.bckgrndcolor;
			word.size = dial->Status.Type1.step.stepStr.size;
			word.kerning = dial->Status.Type1.step.stepStr.kerning;
			LCD_SetNumber(str1, &word);
		
			   //斜杠
		   
		   LCD_SetPicture(dial->Status.Type1.step.stepStr.x_axis, LCD_CENTER_JUSTIFIED, dial->Status.Type1.step.stepStr.forecolor, dial->Status.Type1.step.stepStr.bckgrndcolor, &img_dial_bias_5x17);
              y = 120 + img_dial_bias_5x17.width/2 + dial->Status.Type1.step.gap;
			  
			  word.forecolor = dial->Status.Type1.step.stepStr.forecolor;
			  word.y_axis = y;
			  //步数目标值
			LCD_SetNumber(str2, &word);
           
           //卡路里
            memset(str1,0,sizeof(str1));
			sprintf(str1,"%d", calorie/1000);
          
        	LCD_SetRectangle(dial->Status.Type2.stepDeg.star_x, dial->Status.Type2.stepDeg.height,dial->Status.Type2.stepDeg.star_y, dial->Status.Type2.stepDeg.width, LCD_RED, 0, 0, LCD_FILL_ENABLE);

	
					//卡路里图标
			LCD_SetPicture(dial->Status.Type2.calorie.stepImg.x_axis, dial->Status.Type2.calorie.stepImg.y_axis, LCD_RED, LCD_NONE, &img_dial_cal_15x20);

			word.size = LCD_FONT_16_1_SIZE;
			word.forecolor = LCD_WHITE;
			word.bckgrndcolor = LCD_NONE;
			word.kerning = 1;
			word.x_axis = dial->Status.Type2.stepDeg.star_x  + dial->Status.Type2.stepDeg.height + 4;
			word.y_axis = LCD_CENTER_JUSTIFIED;
			LCD_SetNumber(str1,&word);
		   break;
			case DIAL_STATUS_TYPE_8: //经典数字
				//步数
			gui_progress_circle(LCD_CENTER_LINE, (120 - Font_Number_dial_72.width - 8*2)/2 + 8, (120 - Font_Number_dial_72.width - 8*2)/2, 3, 360, 360, s_percent, dial->Status.Type1.stepDeg.forecolor, dial->Status.Type1.stepDeg.bckgrndcolor);

				
			LCD_SetPicture(120 - 2 - img_dial_step_big.height, (120 - Font_Number_dial_72.width - 8*2)/2 + 8 -img_dial_step.width/2, dial->Status.Type1.step.stepImg.forecolor, LCD_NONE, &img_dial_step);
		
			memset(str1,0,sizeof(str1));
			sprintf(str1,"%d", step);
		
			word.size = (LCD_FONTSIZE)dial->Status.Type1.step.stepStr.size;
			word.forecolor = dial->Status.Type1.step.stepStr.forecolor;
			word.bckgrndcolor = dial->Status.Type1.step.stepStr.bckgrndcolor;
			word.kerning = 1;
			word.x_axis = LCD_CENTER_LINE + 2;
			word.y_axis = (120 - Font_Number_dial_72.width - 8*2)/2 + 8 - ((dial->Status.Type1.step.stepStr.width + word.kerning)*strlen(str1) - word.kerning)/2;
			LCD_SetNumber(str1,&word);
			

           
			/*卡路里*/
            gui_progress_circle(LCD_CENTER_LINE, 120 +  Font_Number_dial_72.width + 8 + (120 - Font_Number_dial_72.width - 8*2)/2, (120 - Font_Number_dial_72.width - 8*2)/2, 3, 360, 360, 100, LCD_WHITE, LCD_NONE);
			
			LCD_SetPicture(120 - 2 - img_dial_kcal.height, 120 +  Font_Number_dial_72.width + 8 + (120 - Font_Number_dial_72.width - 8*2)/2 - img_dial_kcal.width/2, LCD_RED, LCD_NONE, &img_dial_kcal);

            memset(str1,0,sizeof(str1));
			sprintf(str1,"%d", calorie/1000);
		
			word.size = (LCD_FONTSIZE)dial->Status.Type1.step.stepStr.size ;
			word.forecolor = dial->Status.Type1.step.stepStr.forecolor;
			word.bckgrndcolor = dial->Status.Type1.step.stepStr.bckgrndcolor;
			word.kerning = 1;
			word.x_axis = LCD_CENTER_LINE + 2;
			word.y_axis = 120 +  Font_Number_dial_72.width + 8 + (120 - Font_Number_dial_72.width - 8*2)/2 - ((dial->Status.Type1.step.stepStr.width + word.kerning)*strlen(str1) - word.kerning)/2;
			LCD_SetNumber(str1,&word);
			
			
			break;


			case DIAL_STATUS_TYPE_9://数字游戏对应的步数
            /*步数*/
				width_percent = (uint8_t)(dial->Status.Type1.stepDeg.width*s_percent/100);
			
			/*LCD_SetRectangle(dial->Status.Type1.stepDeg.star_x, dial->Status.Type1.stepDeg.height,dial->Status.Type1.stepDeg.star_y, 
			2, dial->Status.Type1.stepDeg.forecolor, 0, 0, LCD_FILL_ENABLE);

            LCD_SetRectangle(dial->Status.Type1.stepDeg.star_x, 2,dial->Status.Type1.stepDeg.star_y, 
			dial->Status.Type1.stepDeg.width, dial->Status.Type1.stepDeg.forecolor, 0, 0, LCD_FILL_ENABLE);
			
			LCD_SetRectangle(dial->Status.Type1.stepDeg.star_x+dial->Status.Type1.stepDeg.height-2, 2,dial->Status.Type1.stepDeg.star_y, 
			dial->Status.Type1.stepDeg.width, dial->Status.Type1.stepDeg.forecolor, 0, 0, LCD_FILL_ENABLE);
			
			LCD_SetRectangle(dial->Status.Type1.stepDeg.star_x, dial->Status.Type1.stepDeg.height,dial->Status.Type1.stepDeg.star_y+dial->Status.Type1.stepDeg.width-2, 
			2, dial->Status.Type1.stepDeg.forecolor, 0, 0, LCD_FILL_ENABLE);*/
			
			 
			
			LCD_SetRectangle(dial->Status.Type1.stepDeg.star_x, dial->Status.Type1.stepDeg.height,dial->Status.Type1.stepDeg.star_y, 
			width_percent, dial->Status.Type1.stepDeg.forecolor, 0, 0, LCD_FILL_ENABLE);
			
			LCD_SetRectangle(dial->Status.Type1.stepDeg.star_x, dial->Status.Type1.stepDeg.height,dial->Status.Type1.stepDeg.star_y+width_percent, 
			dial->Status.Type1.stepDeg.width-width_percent, dial->Status.Type1.stepDeg.bckgrndcolor, 0, 0, LCD_FILL_ENABLE);

			LCD_SetCircle(dial->Status.Type1.stepDeg.star_x  + dial->Status.Type1.stepDeg.height/2,dial->Status.Type1.stepDeg.star_y - 2 - img_dial_step.width/2,3 + img_dial_step.width/2,dial->Status.Type1.step.stepStr.forecolor,0,1);
		   LCD_SetPicture(dial->Status.Type1.stepDeg.star_x + dial->Status.Type1.stepDeg.height/2 - img_dial_step.height/2,dial->Status.Type1.stepDeg.star_y - 3 - img_dial_step.width,LCD_NONE,LCD_NONE,&img_dial_step);
           
             memset(str1,0,sizeof(str1));
			sprintf(str1,"%d", step);
		
			word.size = (LCD_FONTSIZE)dial->Status.Type1.step.stepStr.size;
			word.forecolor = dial->Status.Type1.step.stepStr.forecolor;
			word.bckgrndcolor = dial->Status.Type1.step.stepStr.bckgrndcolor;
			word.kerning = 1;
			word.x_axis = dial->Status.Type1.stepDeg.star_x + dial->Status.Type1.stepDeg.height + 8;
			word.y_axis = dial->Status.Type1.stepDeg.star_y + dial->Status.Type1.stepDeg.width/2 - ((dial->Status.Type1.step.stepStr.width + word.kerning)*strlen(str1) - word.kerning)/2;
			LCD_SetNumber(str1,&word);
			
			
				break;
		default:
			break;
	}
}

static void gui_dial_battery(DialDataStr *dial)
{
	char str[10] = {0};
	uint8_t y,color,status;
	uint16_t temp,len;
	uint8_t percent = g_bat_evt.level_percent;
   
   uint8_t width_percent;
	if(percent > 100)
	{
		percent = 100;
	}
	
	switch(dial->Battery.type)
	{
		case DIAL_BATTERY_TYPE_DEFAULT:	//百分比 电量图标
		case DIAL_BATTERY_TYPE_1:		//蓝牙 电量图标
			if(dial->Battery.type == DIAL_BATTERY_TYPE_DEFAULT)
			{
				if(dial->Battery.batStr.x_axis != 0xFF)
				{
					//有百分比
					status = 1;
				}
				else
				{
					//无百分比
					status = 0;
				}
			
				memset(str,0,sizeof(str));
				sprintf(str,"%d", percent);
			
				len = ((dial->Battery.batStr.width + dial->Battery.batStr.kerning)*strlen(str)
						- dial->Battery.batStr.kerning)*status;
				if(dial->Battery.batStr.size == LCD_FONT_8_SIZE)
				{
					//8#的%宽2像素
					len += 2;
				}
			}
			else
			{
				if(SetValue.SwBle > 0)
				{
					//蓝牙为开
					status = 1;
				}
				else
				{
					//蓝牙为关
					status = 0;
				}
				
				len = dial->Widget.ble.connectImg.width*status;
			}
			
			if(dial->Battery.align == DIAL_ACTIVEX_ALIGN_CENTER_0)
			{
				//合并居中对齐
				if(dial->Battery.batImg.batImg.y_axis == LCD_CENTER_JUSTIFIED)
				{
					y = (LCD_PIXEL_ONELINE_MAX - len - 2- img_dial_bat_per_8x12.width - dial->Battery.gap - dial->Battery.batImg.batImg.width)/2;
				}
				else
				{
					y = dial->Battery.batImg.batImg.y_axis - ((len + dial->Battery.gap + dial->Battery.batImg.batImg.width)/2);
				}
				
				temp = y + len + 2 + img_dial_bat_per_8x12.width + dial->Battery.gap;//图标的起始坐标
			}
			else if(dial->Battery.align == DIAL_ACTIVEX_ALIGN_LEFT_0)
			{
				//合并左对齐
				y = dial->Battery.batImg.batImg.y_axis;
				temp =  y + len + dial->Battery.gap;
			}
			else if(dial->Battery.align == DIAL_ACTIVEX_ALIGN_RIGHT_0)
			{
				//合并右对齐
				y = dial->Battery.batImg.batImg.y_axis - len - dial->Battery.gap - dial->Battery.batImg.batImg.width;
				temp =  y + len + dial->Battery.gap;
			}
			else if(dial->Battery.align == DIAL_ACTIVEX_ALIGN_CENTER_1)
			{
				//分散居中对齐
				y = dial->Battery.batStr.y_axis - (len/2);
				temp = dial->Battery.batImg.batImg.y_axis - (dial->Battery.batImg.batImg.width/2);
			}
			else if(dial->Battery.align == DIAL_ACTIVEX_ALIGN_RIGHT_1)
			{
				//分散右对齐
				y = dial->Battery.batStr.y_axis - len;
				temp = dial->Battery.batImg.batImg.y_axis - dial->Battery.batImg.batImg.width;
			}
			else
			{
				y = dial->Battery.batStr.y_axis;
				temp = dial->Battery.batImg.batImg.y_axis;
			}
			
			if(status)
			{
				if(dial->Battery.type == DIAL_BATTERY_TYPE_DEFAULT)
				{
					//百分比
					LCD_SetString_ExtFlash(str, &dial->Battery.batStr, y);
				}
				else
				{
					//蓝牙
					if(SetValue.SwBle == BLE_DISCONNECT)
					{
						//蓝牙未连接
						//LCD_SetPicture(dial->Widget.ble.disconnectImg.x_axis, y, dial->Widget.ble.disconnectImg.forecolor, dial->Widget.ble.disconnectImg.bckgrndcolor, &Img_BleDisconn_12X18);
					}
					else
					{
						//蓝牙已连接
						//LCD_SetPicture(dial->Widget.ble.connectImg.x_axis, y, dial->Widget.ble.connectImg.forecolor, dial->Widget.ble.connectImg.bckgrndcolor, &Img_BleConn_12X18);
					}
				}
			}

           //电量百分比图标
			LCD_SetPicture(dial->Battery.batStr.x_axis, y + len + 2 , dial->Battery.batStr.forecolor, 
							dial->Battery.batStr.bckgrndcolor, &img_dial_bat_per_8x12);
		
			//电量图标
			LCD_SetPicture(dial->Battery.batImg.batImg.x_axis, temp, dial->Battery.batImg.batImg.forecolor, 
							dial->Battery.batImg.batImg.bckgrndcolor, &img_dial_bat_28x13);
			
			//填充图标
			if(percent <= 10)
			{
				color = dial->Battery.batImg.lowColor;
			}
			else
			{
				color = dial->Battery.batImg.normalColor;
			}
			
			LCD_SetRectangle(dial->Battery.batImg.batImg.x_axis + 1, dial->Battery.batImg.batImg.height - 2, temp + 1, percent * dial->Battery.batImg.width / 100, color, 0, 0, LCD_FILL_ENABLE);
			break;
		case DIAL_BATTERY_TYPE_2:	//电池图标 电量进度条(JW600默认主题)
			if(dial->Battery.batStr.x_axis != 0xFF)
			{
				//有百分比
				status = 1;
			}
			else
			{
				//无百分比
				status = 0;
			}
		
			memset(str,0,sizeof(str));
			sprintf(str,"%d", percent);
		
			len = ((dial->Battery.batStr.width + dial->Battery.batStr.kerning)*strlen(str)
					- dial->Battery.batStr.kerning)*status;
			if(dial->Battery.batStr.size == LCD_FONT_8_SIZE)
			{
				//8#的%宽2像素
				len += 2;
			}
			
			if(dial->Battery.align == DIAL_ACTIVEX_ALIGN_CENTER_0)
			{
				//合并居中对齐
				if(dial->Battery.batImg.batImg.y_axis == LCD_CENTER_JUSTIFIED)
				{
					y = (LCD_PIXEL_ONELINE_MAX - len - dial->Battery.gap - dial->Battery.batImg.batImg.width)/2;
				}
				else
				{
					y = dial->Battery.batImg.batImg.y_axis - ((len + dial->Battery.gap + dial->Battery.batImg.batImg.width)/2);
				}
				
				temp = y + dial->Battery.batImg.batImg.width + dial->Battery.gap;
			}
			else if(dial->Battery.align == DIAL_ACTIVEX_ALIGN_LEFT_0)
			{
				//合并左对齐
				y = dial->Battery.batImg.batImg.y_axis;
				temp =  y + dial->Battery.batImg.batImg.width + dial->Battery.gap;
			}
			else if(dial->Battery.align == DIAL_ACTIVEX_ALIGN_RIGHT_0)
			{
				//合并右对齐
				y = dial->Battery.batImg.batImg.y_axis - len - dial->Battery.gap - dial->Battery.batImg.batImg.width;
				temp =  y + dial->Battery.batImg.batImg.width + dial->Battery.gap;
			}
			else if(dial->Battery.align == DIAL_ACTIVEX_ALIGN_CENTER_1)
			{
				//分散居中对齐
				y = dial->Battery.batImg.batImg.y_axis - (dial->Battery.batImg.batImg.width/2);
				temp = dial->Battery.batStr.y_axis - (len/2);
			}
			else if(dial->Battery.align == DIAL_ACTIVEX_ALIGN_RIGHT_1)
			{
				//分散右对齐
				y = dial->Battery.batImg.batImg.y_axis - dial->Battery.batImg.batImg.width;
				temp = dial->Battery.batStr.y_axis - len;
			}
			else
			{
				y = dial->Battery.batImg.batImg.y_axis;
				temp = dial->Battery.batStr.y_axis;
			}
			
			//电池图标
			if(dial->Battery.batImg.x_axis != 0xFF)
			{
				if(percent <= 10)
				{
					color = dial->Battery.batImg.lowColor;
				}
				else
				{
					color = dial->Battery.batImg.normalColor;
				}
				if(dial->Battery.type == DIAL_BATTERY_TYPE_2)
				{
					//内部flash图标
					LCD_SetPicture(dial->Battery.batImg.batImg.x_axis, dial->Battery.batImg.batImg.y_axis, 
									color, dial->Battery.batImg.batImg.bckgrndcolor, &img_dial_bat_charge);
				}
				else
				{
					//外部flash图标
					dial->Battery.batImg.batImg.forecolor = color;
					/*LCD_SetPicture_ExtFlash(DIAL_BATTERY_IMG_ADDRESS + (m_dial_num * DIAL_DATA_SIZE) + m_gial_addr_offset, 
												&dial->Battery.batImg.batImg, y);*/
				}
			}
			
			if(status)
			{
				//百分比
				
				LCD_SetString_ExtFlash(str, &dial->Battery.batStr, temp);
				
				LCD_SetPicture(dial->Battery.batStr.x_axis+dial->Battery.batStr.height-img_dial_percent.height,temp+strlen(str)*dial->Battery.batStr.width+1+2,dial->Battery.batStr.forecolor,dial->Battery.batStr.bckgrndcolor,&img_dial_percent);
			}
			
			//电量进度条
				/*LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, dial->Battery.batDeg.height, dial->Battery.batDeg.width, 
						dial->Battery.batDeg.star_ang, dial->Battery.batDeg.range, LCD_GRAY);*/
			gui_progess_bar(percent, &dial->Battery.batDeg);
			
			break;
			
			case DIAL_BATTERY_TYPE_3:	//电池图标 电量进度条
            
            
			LCD_SetArc(LCD_CENTER_LINE,LCD_CENTER_ROW,dial->Battery.batDeg.height,dial->Battery.batDeg.width*3,210,212,dial->Battery.batDeg.forecolor); 
			
			LCD_SetArc(LCD_CENTER_LINE,LCD_CENTER_ROW,dial->Battery.batDeg.height,dial->Battery.batDeg.width*3,270-2,270,dial->Battery.batDeg.bckgrndcolor); 
			gui_progess_bar(percent, &dial->Battery.batDeg);

              LCD_SetPicture(212,66,LCD_WHITE,LCD_NONE,&img_dial_bat_charge);
			
			break;
			case DIAL_BATTERY_TYPE_5:	//数字运动对应的电池进度条
             	width_percent = (uint8_t)(dial->Battery.batDeg.width*percent/100);
			
			LCD_SetRectangle(dial->Battery.batDeg.star_x, dial->Battery.batDeg.height,dial->Battery.batDeg.star_y, 
			width_percent, dial->Battery.batDeg.forecolor, 0, 0, LCD_FILL_ENABLE);
			
			LCD_SetRectangle(dial->Battery.batDeg.star_x, dial->Battery.batDeg.height,dial->Battery.batDeg.star_y+width_percent, 
			dial->Battery.batDeg.width - width_percent, dial->Battery.batDeg.bckgrndcolor, 0, 0, LCD_FILL_ENABLE);

			LCD_SetCircle(dial->Battery.batDeg.star_x + dial->Battery.batDeg.height/2,dial->Battery.batDeg.star_y - 2 - img_dial_bat_charge.width/2,3 + img_dial_bat_charge.width/2,dial->Battery.batStr.forecolor,0,1);
		   LCD_SetPicture(dial->Battery.batDeg.star_x + dial->Battery.batDeg.height/2 - img_dial_bat_charge.height/2,dial->Battery.batDeg.star_y - 3 - img_dial_bat_charge.width,LCD_NONE,LCD_NONE,&img_dial_bat_charge);
            memset(str,0,sizeof(str));
			 sprintf(str,"%d", percent);
		
               y = dial->Battery.batDeg.star_y + dial->Battery.batDeg.width/2 - ((dial->Battery.batStr.width + dial->Battery.batStr.kerning)*strlen(str) -dial->Battery.batStr.kerning)/2;
			
			LCD_SetString_ExtFlash(str, &dial->Battery.batStr, y);
			y = dial->Battery.batDeg.star_y + dial->Battery.batDeg.width/2 +  ((dial->Battery.batStr.width + dial->Battery.batStr.kerning)*strlen(str) -dial->Battery.batStr.kerning)/2 +4;

			   LCD_SetPicture(dial->Battery.batStr.x_axis+dial->Battery.batStr.height-img_dial_percent_13x13.height,y,dial->Battery.batStr.forecolor,dial->Battery.batStr.bckgrndcolor,&img_dial_percent_13x13);
			break;

			
			case DIAL_BATTERY_TYPE_6:	//商务指针对应的电池
			 y = (LCD_PIXEL_ONELINE_MAX -  dial->Battery.batImg.batImg.width) / 2;
				if(percent <= 10)
				{
					color = dial->Battery.batImg.lowColor;
				}
				else
				{
					color = dial->Battery.batImg.normalColor;
				}
             LCD_SetPicture(dial->Battery.batImg.batImg.x_axis, dial->Battery.batImg.batImg.y_axis,dial->Battery.batImg.batImg.forecolor, dial->Battery.batImg.batImg.bckgrndcolor, &img_dial_bat_big);
			 LCD_SetRectangle(dial->Battery.batImg.batImg.x_axis, dial->Battery.batImg.batImg.height, y, percent * dial->Battery.batImg.width / 100, 
								color, 0, 0, LCD_FILL_ENABLE);

			   memset(str,0,sizeof(str));
			   sprintf(str,"%d", percent);
			 	len = (dial->Battery.batStr.width + dial->Battery.batStr.kerning)*strlen(str)- dial->Battery.batStr.kerning;
				
			 LCD_SetString_ExtFlash(str, &dial->Battery.batStr, y-4-img_dial_percent.width-2-len);
			 
			 LCD_SetPicture(dial->Battery.batStr.x_axis + dial->Battery.batStr.height/2 - img_dial_percent.height/2,y-4-img_dial_percent.width,dial->Battery.batStr.forecolor,dial->Battery.batStr.bckgrndcolor,&img_dial_percent);

			
			break;
		default:
			break;
	}
}
static void gui_dial_kcal(DialDataStr *dial)
{

	if(dial->Ambient.type != DIAL_AMBIENT_TYPE_2)
	{
  float pres,alt,temp;

  char str[10] = {0};
	
	SetWord_t word = {0};
	uint32_t calorie = Get_TotalEnergy();		
	memset(str,0,sizeof(str));
	sprintf(str,"%d", calorie/1000);
	
	word.size = LCD_FONT_16_1_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	word.x_axis = 110 - img_dial_cal_15x20.height/2;
	word.y_axis = 180;
//卡路里
  LCD_SetPicture(word.x_axis, word.y_axis, LCD_PERU, LCD_NONE, &img_dial_cal_15x20);
  word.y_axis += img_dial_cal_15x20.width +4;
  
  word.x_axis = 110 - 16/2;
  LCD_SetNumber(str,&word);
}




}
static void gui_dial_heart(DialDataStr *dial)
{
	char str[10] = {0};
	uint8_t y,heart;
	uint16_t len,temp;
	
	SetWord_t word = {0};
	//当天基础心率
	if((DayMainData.BasedHeartrate != 0)
		&& (DayMainData.BasedHeartrate != 0xFF))
	{
		heart = DayMainData.BasedHeartrate;
	}
	else
	{
		heart = 0;
	}
	         word.x_axis = dial->Heart.heartStr.x_axis;
        	  word.y_axis = dial->Heart.heartStr.y_axis;
			  word.forecolor = dial->Heart.heartStr.forecolor;
			  word.bckgrndcolor = dial->Heart.heartStr.bckgrndcolor;
			  word.size = LCD_FONT_16_1_SIZE;
			  word.kerning = 1;
	switch(dial->Heart.type)
	{
		case DIAL_HEART_TYPE_DEFAULT:
			//心率图标 心率数据
			memset(str,0,sizeof(str));
			if(heart != 0)
			{
				sprintf(str,"%d", heart);
			}
			else
			{
				sprintf(str,"--");
			}
		
			len = (dial->Heart.heartStr.width + dial->Heart.heartStr.kerning)*strlen(str) - dial->Heart.heartStr.kerning;
		
			if(dial->Heart.align == DIAL_ACTIVEX_ALIGN_CENTER_0)
			{
				//合并居中
				if(dial->Heart.heartStr.y_axis == LCD_CENTER_JUSTIFIED)
				{
					y = (LCD_PIXEL_ONELINE_MAX - (dial->Heart.gap + dial->Heart.heartImg.width + len))/2;
				}
				else
				{
					y = dial->Heart.heartStr.y_axis - ((dial->Heart.gap + dial->Heart.heartImg.width + len)/2);
				}
				
				temp = y + dial->Heart.heartImg.width + dial->Heart.gap;
			}
			else if(dial->Heart.align == DIAL_ACTIVEX_ALIGN_LEFT_0)
			{
				//合并左对齐
				y = dial->Heart.heartStr.y_axis;
				
				temp = y + dial->Heart.heartImg.width + dial->Heart.gap;
			}
			else if(dial->Heart.align == DIAL_ACTIVEX_ALIGN_RIGHT_0)
			{
				//合并右对齐
				y = dial->Heart.heartStr.y_axis - len - dial->Heart.gap - dial->Heart.heartImg.width;
				
				temp = y + dial->Heart.heartImg.width + dial->Heart.gap;
			}
			else if(dial->Heart.align == DIAL_ACTIVEX_ALIGN_CENTER_1)
			{
				//居中
				y = dial->Heart.heartImg.y_axis - (dial->Heart.heartImg.width/2);
				
				temp = dial->Heart.heartStr.y_axis - (len/2);
			}
			else if(dial->Heart.align == DIAL_ACTIVEX_ALIGN_RIGHT_1)
			{
				//右对齐
				y = dial->Heart.heartImg.y_axis - dial->Heart.heartImg.width;
				
				temp = dial->Heart.heartStr.y_axis - len;
			}
			else
			{
				y = dial->Heart.heartImg.y_axis;
				
				temp = dial->Heart.heartStr.y_axis;
			}
		
			//心率图标
			if(dial->Heart.heartImg.x_axis != 0xFF)
			{
				LCD_SetPicture(dial->Heart.heartImg.x_axis, dial->Heart.heartImg.y_axis,dial->Heart.heartImg.forecolor,dial->Heart.heartImg.bckgrndcolor,&img_dial_heartrate_16x15);
			}
			
			//心率数据
			if(dial->Heart.heartStr.x_axis != 0xFF)
			{
			
			if(heart != 0)
				{
				
				LCD_SetNumber(str,&word);
				}
			else
				{
                 LCD_SetRectangle(110 -2,4,temp,8,LCD_WHITE,0,0,LCD_FILL_ENABLE);
                 
                 LCD_SetRectangle(110 -2,4,temp+8+4,8,LCD_WHITE,0,0,LCD_FILL_ENABLE);

			    }
			}
			break;
		default:
			break;
	}
}


static void gui_dial_get_weather(void)

{
   //获取当前时间
	am_hal_rtc_time_get(&RTC_time);
	gui_tool_calendar_weather_info_init();//读取天气和温度同步信息
	dial_weather_index	= gui_tool_calendar_weather_get(RTC_time.ui32Year, RTC_time.ui32Month, RTC_time.ui32DayOfMonth,RTC_time.ui32Hour);


}
static void gui_dial_weather(DialDataStr *dial)
{
	char str1[10];
	char str2[10];
	SetWord_t word = {0};
	uint8_t index;
	uint16_t len,y,temp;
	 uint8_t x1,x2,axix_x,axix_y;
	char str[20] = {0};
	//获取天气索引
	index = gui_tool_calendar_weather_get(RTC_time.ui32Year, RTC_time.ui32Month, RTC_time.ui32DayOfMonth,RTC_time.ui32Hour);
	//获取当天天气索引
	gui_dial_get_weather();
	switch(dial->Weather.type)
	{
		case DIAL_WEATHER_TYPE_1:	//天气图标 气温 日出 日落
		case DIAL_WEATHER_TYPE_2:	//无天气时不显示
			//天气图标
			if(dial->Weather.weather.stepImg.x_axis != 0xFF)
			{
				if(index < 7)
				{
					//有天气
					if(dial->Weather.align1 == DIAL_ACTIVEX_ALIGN_CENTER_1)
					{
						//居中
						y = dial->Weather.weather.stepImg.y_axis - (dial->Weather.weather.stepImg.width/2);
					}
					else if(dial->Weather.align1 == DIAL_ACTIVEX_ALIGN_RIGHT_1)
					{
						//右对齐
						y = dial->Weather.weather.stepImg.y_axis - dial->Weather.weather.stepImg.width;
					}
					else
					{
						y = dial->Weather.weather.stepImg.y_axis;
					}
	
				}
				else
				{
					if(dial->Weather.type == DIAL_WEATHER_TYPE_1)
					{
						//无天气
						if(dial->Weather.align1 == DIAL_ACTIVEX_ALIGN_CENTER_1)
						{
							//居中
							y = dial->Weather.weather.stepImg.y_axis - (28/2);
						}
						else if(dial->Weather.align1 == DIAL_ACTIVEX_ALIGN_RIGHT_1)
						{
							//右对齐
							y = dial->Weather.weather.stepImg.y_axis - 28;
						}
						else
						{
							y = dial->Weather.weather.stepImg.y_axis + ((dial->Weather.weather.stepImg.width - 28)/2);
						}
						
						LCD_SetRectangle(dial->Weather.weather.stepImg.x_axis + (dial->Weather.weather.stepImg.height*2/3), 4, 
											y, 12, LCD_GRAY, 0, 0, LCD_FILL_ENABLE);
						LCD_SetRectangle(dial->Weather.weather.stepImg.x_axis + (dial->Weather.weather.stepImg.height*2/3), 4, 
											y + 16, 12, LCD_GRAY, 0, 0, LCD_FILL_ENABLE);
					}
				}
			}
			
			//温度
			if(dial->Weather.weather.stepStr.x_axis != 0xFF)
			{
				if(index < 7)
				{
					memset(str1,0,sizeof(str1));
				//	sprintf(str1,"%d",m_weather_info.appWeatherSync.weather_array[index].lowest_temp);
					memset(str2,0,sizeof(str2));
				//	sprintf(str2,"%d",m_weather_info.appWeatherSync.weather_array[index].highest_temp);
					
					if(dial->Weather.align1 == DIAL_ACTIVEX_ALIGN_CENTER_1)
					{
						//居中
						y = dial->Weather.weather.stepStr.y_axis - (((strlen(str1)+strlen(str2))*dial->Weather.weather.stepStr.width 
																+ (strlen(str1)+strlen(str2)-2)*dial->Weather.weather.stepStr.kerning 
																+ 26)/2);
					}
					else if(dial->Weather.align1 == DIAL_ACTIVEX_ALIGN_RIGHT_1)
					{
						//右对齐
						y = dial->Weather.weather.stepStr.y_axis - ((strlen(str1)+strlen(str2))*dial->Weather.weather.stepStr.width 
																+ (strlen(str1)+strlen(str2)-2)*dial->Weather.weather.stepStr.kerning 
																+ 26);
					}
					else
					{
						y = dial->Weather.weather.stepStr.y_axis;
					}
					
					//最低温度
					LCD_SetString_ExtFlash(str1, &dial->Weather.weather.stepStr, y);
					
					//度
					y += (strlen(str1)*(dial->Weather.weather.stepStr.width + dial->Weather.weather.stepStr.kerning)- dial->Weather.weather.stepStr.kerning + 2);
//					LCD_SetPicture(dial->Weather.weather.stepStr.x_axis, y, 
//									dial->Weather.weather.stepStr.forecolor , dial->Weather.weather.stepStr.bckgrndcolor, &Img_compass_4X4);
					
					//~
//					y += (Img_compass_4X4.width + 2);
					word.x_axis = dial->Weather.weather.stepStr.x_axis + 6;
					word.y_axis = y;
					word.size = (LCD_FONTSIZE)dial->Weather.weather.stepStr.size;
					word.forecolor = dial->Weather.weather.stepStr.forecolor;
					word.bckgrndcolor = dial->Weather.weather.stepStr.bckgrndcolor;
					word.kerning = dial->Weather.weather.stepStr.kerning;
					LCD_SetString("~",&word);
					
					//最高温度
					y += (8 + 3);
					LCD_SetString_ExtFlash(str2, &dial->Weather.weather.stepStr, y);
					
					//度
					y += (strlen(str2)*(dial->Weather.weather.stepStr.width + dial->Weather.weather.stepStr.kerning)- dial->Weather.weather.stepStr.kerning + 2);
					LCD_SetPicture(dial->Weather.weather.stepStr.x_axis, y, 
									dial->Weather.weather.stepStr.forecolor , dial->Weather.weather.stepStr.bckgrndcolor, &Img_compass_4X4);
									
					//C
					y += (Img_compass_4X4.width + 1);
					LCD_SetPicture(dial->Weather.weather.stepStr.x_axis, y, 
									dial->Weather.weather.stepStr.forecolor , dial->Weather.weather.stepStr.bckgrndcolor, &Img_TempC_6X16);
				}
			}
			
			//日出
			if((dial->Weather.sunrise.stepImg.x_axis != 0xFF)
				|| (dial->Weather.sunrise.stepStr.x_axis != 0xFF))
			{
				memset(str1,0,sizeof(str1));
				if(index < 7)
				{
					//有数据
					/*sprintf(str1,"%02d:%02d", m_weather_info.appWeatherSync.weather_array[index].sun_rise_time[0],
												m_weather_info.appWeatherSync.weather_array[index].sun_rise_time[1]);*/
				}
				else
				{
					//无数据
					sprintf(str1,"--");
				}
				len = (strlen(str1)*(dial->Weather.sunrise.stepStr.width + dial->Weather.sunrise.stepStr.kerning)
						- dial->Weather.sunrise.stepStr.kerning);
				
				if((dial->Weather.align2&0x0F) == DIAL_ACTIVEX_ALIGN_CENTER_0)
				{
					//合并居中
					if(dial->Weather.sunrise.stepStr.y_axis == LCD_CENTER_JUSTIFIED)
					{
						y = (LCD_PIXEL_ONELINE_MAX - (len + dial->Weather.sunrise.gap + dial->Weather.sunrise.stepImg.width))/2;
					}
					else
					{
						y = dial->Weather.sunrise.stepStr.y_axis - ((len + dial->Weather.sunrise.gap + dial->Weather.sunrise.stepImg.width)/2);
					}
					
					temp = y + dial->Weather.sunrise.stepImg.width + dial->Weather.sunrise.gap;
				}
				else if((dial->Weather.align2&0x0F) == DIAL_ACTIVEX_ALIGN_LEFT_0)
				{
					//合并左对齐
					y = dial->Weather.sunrise.stepStr.y_axis;
					
					temp = y + dial->Weather.sunrise.stepImg.width + dial->Weather.sunrise.gap;
				}
				else if((dial->Weather.align2&0x0F) == DIAL_ACTIVEX_ALIGN_RIGHT_0)
				{
					//合并右对齐
					y = dial->Weather.sunrise.stepStr.y_axis - (len + dial->Weather.sunrise.gap + dial->Weather.sunrise.stepImg.width);
					
					temp = y + dial->Weather.sunrise.stepImg.width + dial->Weather.sunrise.gap;
				}
				else if((dial->Weather.align2&0x0F) == DIAL_ACTIVEX_ALIGN_CENTER_1)
				{
					//居中
					y = dial->Weather.sunrise.stepImg.y_axis - (dial->Weather.sunrise.stepImg.width/2);
					
					temp = dial->Weather.sunrise.stepStr.y_axis - (len/2);
				}
				else if((dial->Weather.align2&0x0F) == DIAL_ACTIVEX_ALIGN_RIGHT_1)
				{
					//右对齐
					y = dial->Weather.sunrise.stepImg.y_axis - dial->Weather.sunrise.stepImg.width;
					
					temp = dial->Weather.sunrise.stepStr.y_axis - len;
				}
				else
				{
					y = dial->Weather.sunrise.stepImg.y_axis;
					
					temp = dial->Weather.sunrise.stepStr.y_axis;
				}
				
				if(dial->Weather.sunrise.stepImg.x_axis != 0xFF)
				{
					//图标
					LCD_SetPicture_ExtFlash(DIAL_SUNRISE_IMG_ADDRESS + (m_dial_num * DIAL_DATA_SIZE) + m_gial_addr_offset, 
											&dial->Weather.sunrise.stepImg, y);
				}
				
				if(dial->Weather.sunrise.stepStr.x_axis != 0xFF)
				{
					//数据
					LCD_SetString_ExtFlash(str1, &dial->Weather.sunrise.stepStr, temp);
				}
			}
			
			//日落
			if((dial->Weather.sunset.stepImg.x_axis != 0xFF)
				|| (dial->Weather.sunset.stepStr.x_axis != 0xFF))
			{
				memset(str1,0,sizeof(str1));
				if(index < 7)
				{
					//有数据
					/*sprintf(str1,"%02d:%02d", m_weather_info.appWeatherSync.weather_array[index].sun_set_time[0],
												m_weather_info.appWeatherSync.weather_array[index].sun_set_time[1]);*/
				}
				else
				{
					//无数据
					sprintf(str1,"--");
				}
				len = (strlen(str1)*(dial->Weather.sunset.stepStr.width + dial->Weather.sunset.stepStr.kerning)
						- dial->Weather.sunset.stepStr.kerning);
				
				if(((dial->Weather.align2&0xF0)>>4) == DIAL_ACTIVEX_ALIGN_CENTER_0)
				{
					//合并居中
					if(dial->Weather.sunset.stepStr.y_axis == LCD_CENTER_JUSTIFIED)
					{
						y = (LCD_PIXEL_ONELINE_MAX - (len + dial->Weather.sunset.gap + dial->Weather.sunset.stepImg.width))/2;
					}
					else
					{
						y = dial->Weather.sunset.stepStr.y_axis - ((len + dial->Weather.sunset.gap + dial->Weather.sunset.stepImg.width)/2);
					}
					
					temp = y + dial->Weather.sunset.stepImg.width + dial->Weather.sunset.gap;
				}
				else if(((dial->Weather.align2&0xF0)>>4) == DIAL_ACTIVEX_ALIGN_LEFT_0)
				{
					//合并左对齐
					y = dial->Weather.sunset.stepStr.y_axis;
					
					temp = y + dial->Weather.sunset.stepImg.width + dial->Weather.sunset.gap;
				}
				else if(((dial->Weather.align2&0xF0)>>4) == DIAL_ACTIVEX_ALIGN_RIGHT_0)
				{
					//合并右对齐
					y = dial->Weather.sunset.stepStr.y_axis - (len + dial->Weather.sunset.gap + dial->Weather.sunset.stepImg.width);
					
					temp = y + dial->Weather.sunset.stepImg.width + dial->Weather.sunset.gap;
				}
				else if(((dial->Weather.align2&0xF0)>>4) == DIAL_ACTIVEX_ALIGN_CENTER_1)
				{
					//居中
					y = dial->Weather.sunset.stepImg.y_axis - (dial->Weather.sunset.stepImg.width/2);
					
					temp = dial->Weather.sunset.stepStr.y_axis - (len/2);
				}
				else if(((dial->Weather.align2&0xF0)>>4) == DIAL_ACTIVEX_ALIGN_RIGHT_1)
				{
					//右对齐
					y = dial->Weather.sunset.stepImg.y_axis - dial->Weather.sunset.stepImg.width;
					
					temp = dial->Weather.sunset.stepStr.y_axis - len;
				}
				else
				{
					y = dial->Weather.sunset.stepImg.y_axis;
					
					temp = dial->Weather.sunset.stepStr.y_axis;
				}
				
				if(dial->Weather.sunset.stepImg.x_axis != 0xFF)
				{
					//图标
					LCD_SetPicture_ExtFlash(DIAL_SUNSET_IMG_ADDRESS + (m_dial_num * DIAL_DATA_SIZE) + m_gial_addr_offset, 
											&dial->Weather.sunset.stepImg, y);
				}
				
				if(dial->Weather.sunset.stepStr.x_axis != 0xFF)
				{
					//数据
					LCD_SetString_ExtFlash(str1, &dial->Weather.sunset.stepStr, temp);
				}
			}
			break;


		case DIAL_WEATHER_TYPE_0:
			#ifdef COD 
			if(dial->Weather.weather.stepImg.x_axis != 0xFF)
				{
					
		        if((dial_weather_index < 24)&&(g_weather_info.cod_weather[dial_weather_index].weather<=14))
		        	{
		           memset(str,0,sizeof(str));
			       sprintf(str,"%d",g_weather_info.cod_weather[dial_weather_index].temperature);//温度
		      
				   len	= strlen(str)*(dial->Widget.dial_weather.d_weatherStr.width + dial->Widget.dial_weather.d_weatherStr.kerning) - dial->Widget.dial_weather.d_weatherStr.kerning;
				

		          //温度值
		          LCD_SetString_ExtFlash(str,&dial->Widget.dial_weather.d_weatherStr,dial->Widget.dial_weather.d_weatherStr.y_axis);
				   //度图标
				   LCD_SetPicture(dial->Widget.dial_weather.d_weatherStr.x_axis,dial->Widget.dial_weather.d_weatherStr.y_axis + len + 4 ,LCD_WHITE,LCD_NONE,&img_dial_temp_10x16);

		            len += (4 + img_dial_temp_10x16.width);

					dial->Widget.dial_weather.d_weatherImg.y_axis = dial->Widget.dial_weather.d_weatherStr.y_axis + len/2 - img_dial_weather_lists[g_weather_info.cod_weather[dial_weather_index].weather].width/2;
				      //天气图标
					  LCD_SetPicture(dial->Widget.dial_weather.d_weatherImg.x_axis, dial->Widget.dial_weather.d_weatherImg.y_axis,calendar_weather_data[g_weather_info.cod_weather[dial_weather_index].weather],
					   LCD_NONE,&img_dial_weather_lists[g_weather_info.cod_weather[dial_weather_index].weather]);

		        	}
				else
					{
		            LCD_SetPicture(dial->Widget.dial_weather.d_weatherStr.x_axis + Font_Number_16_1.height - img_weather_update_small.height, dial->Widget.dial_weather.d_weatherImg.y_axis,LCD_WHITE, LCD_NONE,&img_weather_update_small);
		                    

				  }
				}
			else;
			#endif
				break;
		case DIAL_WEATHER_TYPE_4:
			#ifdef COD 
			if(dial->Weather.weather.stepImg.x_axis != 0xFF)
				{
			x1 =  dial->Widget.dial_weather.d_weatherStr.x_axis + dial->Widget.dial_weather.d_weatherStr.height + 8;
			x2 = dial->Widget.dial_weather.d_weatherImg.x_axis - 8;
			axix_x = (x1 + x2)/2;
			axix_y = 120;
         gui_progress_circle(axix_x,axix_y,(x1-x2)/2,3,0,360,100,LCD_WHITE,LCD_NONE);
			
        if((dial_weather_index < 24)&&(g_weather_info.cod_weather[dial_weather_index].weather<=14))
        	{
           memset(str,0,sizeof(str));
	       sprintf(str,"%d",g_weather_info.cod_weather[dial_weather_index].temperature);//温度
           
		   len	= strlen(str)*(dial->Widget.dial_weather.d_weatherStr.width + dial->Widget.dial_weather.d_weatherStr.kerning) - dial->Widget.dial_weather.d_weatherStr.kerning;
		   

			word.x_axis =  dial->Widget.dial_weather.d_weatherStr.x_axis;
			word.y_axis = 120 - (len+ 4 + img_dial_temp_10x16.width)/2 ;
			word.size =  dial->Widget.dial_weather.d_weatherStr.size;
			word.forecolor = dial->Widget.dial_weather.d_weatherStr.forecolor;
			word.bckgrndcolor = dial->Widget.dial_weather.d_weatherStr.bckgrndcolor;
			word.kerning = dial->Widget.dial_weather.d_weatherStr.kerning;
        
		
		//温度值
         LCD_SetNumber(str,&word);
		   //度图标
		   LCD_SetPicture(dial->Widget.dial_weather.d_weatherStr.x_axis,word.y_axis + len + 2 ,LCD_WHITE,LCD_NONE,&img_dial_temp_10x16);

            len += (2 + img_dial_temp_10x16.width);

			dial->Widget.dial_weather.d_weatherImg.y_axis = word.y_axis + len/2 - img_Weather_lists[g_weather_info.cod_weather[dial_weather_index].weather].width/2;
		      //天气图标
			  LCD_SetPicture(dial->Widget.dial_weather.d_weatherImg.x_axis, dial->Widget.dial_weather.d_weatherImg.y_axis,calendar_weather_data[g_weather_info.cod_weather[dial_weather_index].weather],
			   LCD_NONE,&img_Weather_lists[g_weather_info.cod_weather[dial_weather_index].weather]);

        	}
		else
			{
            LCD_SetPicture(dial->Widget.dial_weather.d_weatherImg.x_axis, dial->Widget.dial_weather.d_weatherImg.y_axis,LCD_WHITE, LCD_NONE,&img_weather_update_big);
                    

		  }
				}
			else;
			#endif
				break;
			
		default:
			break;
	}
}

static void gui_dial_ambient(DialDataStr *dial)
{
	char str[10]={0};
	uint16_t len,strY,imgY;
	float pres,alt,temp;
	uint32_t calorie;
	SetWord_t word = {0};
	//获取海拔气压温度
	ambient_value_get(&pres, &alt, &temp);
	
	switch(dial->Ambient.type)
	{

         case DIAL_AMBIENT_TYPE_DEFAULT:

			  word.x_axis = 120 + 16 + 16 - img_dial_fiag_m.height;
			  word.y_axis = 16;
			  word.forecolor = LCD_CORNFLOWERBLUE;
			  word.bckgrndcolor = LCD_NONE;
			  word.size = LCD_FONT_16_1_SIZE;
			  word.kerning = 1;
			  
			  LCD_SetPicture(word.x_axis, word.y_axis,  word.forecolor,  word.bckgrndcolor, &img_dial_fiag_m);
			  word.x_axis = 120 + 16;
			  word.y_axis = 16 + img_dial_fiag_m.width + 8;
			  memset(str,0,sizeof(str));
			  sprintf(str,"%d",(int)(alt + SetValue.Altitude_Offset));
			  LCD_SetNumber(str,&word);


			  word.x_axis = 120 + 16 + 16 - img_dial_fiag_pa.height;
			  word.y_axis = 156;
			  word.forecolor = LCD_RED;
			  word.bckgrndcolor = LCD_NONE;
			  word.kerning = 1;
			  
			  LCD_SetPicture(word.x_axis, word.y_axis,  word.forecolor,  word.bckgrndcolor, &img_dial_fiag_pa);
			  word.x_axis = 120 + 16 ;
			  word.y_axis = 156 + img_dial_fiag_pa.width + 8;
			  memset(str,0,sizeof(str));
			  sprintf(str,"%.1f",pres);
			  LCD_SetNumber(str,&word);
		break;
	
		case DIAL_AMBIENT_TYPE_1:	//海拔 气压 温度
			//海拔
			if((dial->Ambient.altitude.stepImg.x_axis != 0xFF)
				|| (dial->Ambient.altitude.stepStr.x_axis != 0xFF))
			{
				memset(str,0,sizeof(str));
				if(alt != 0)
				{
					//有数据
					sprintf(str,"%d", (int)(alt + SetValue.Altitude_Offset));
				}
				else
				{
					//无数据
					sprintf(str,"--");
				}
				len = (strlen(str)*(dial->Ambient.altitude.stepStr.width + dial->Ambient.altitude.stepStr.kerning)
						- dial->Ambient.altitude.stepStr.kerning);
				
				if((dial->Ambient.align1&0x0F) == DIAL_ACTIVEX_ALIGN_CENTER_0)
				{
					//合并居中
					if(dial->Ambient.altitude.stepStr.y_axis == LCD_CENTER_JUSTIFIED)
					{
						strY = (LCD_PIXEL_ONELINE_MAX - (len + dial->Ambient.altitude.gap + dial->Ambient.altitude.stepImg.width))/2;
					}
					else
					{
						strY = dial->Ambient.altitude.stepStr.y_axis - ((len + dial->Ambient.altitude.gap + dial->Ambient.altitude.stepImg.width)/2);
					}
					
					imgY = strY + dial->Ambient.altitude.stepImg.width + dial->Ambient.altitude.gap;
				}
				else if((dial->Ambient.align1&0x0F) == DIAL_ACTIVEX_ALIGN_LEFT_0)
				{
					//合并左对齐
					strY = dial->Ambient.altitude.stepStr.y_axis;
					
					imgY = strY + dial->Ambient.altitude.stepImg.width + dial->Ambient.altitude.gap;
				}
				else if((dial->Ambient.align1&0x0F) == DIAL_ACTIVEX_ALIGN_RIGHT_0)
				{
					//合并右对齐
					strY = dial->Ambient.altitude.stepStr.y_axis - (len + dial->Ambient.altitude.gap + dial->Ambient.altitude.stepImg.width);
					
					imgY = strY + dial->Ambient.altitude.stepImg.width + dial->Ambient.altitude.gap;
				}
				else if((dial->Ambient.align1&0x0F) == DIAL_ACTIVEX_ALIGN_CENTER_1)
				{
					//居中
					strY = dial->Ambient.altitude.stepImg.y_axis - (dial->Ambient.altitude.stepImg.width/2);
					
					imgY = dial->Ambient.altitude.stepStr.y_axis - (len/2);
				}
				else if((dial->Ambient.align1&0x0F) == DIAL_ACTIVEX_ALIGN_RIGHT_1)
				{
					//右对齐
					strY = dial->Ambient.altitude.stepImg.y_axis - dial->Ambient.altitude.stepImg.width;
					
					imgY = dial->Ambient.altitude.stepStr.y_axis - len;
				}
				else
				{
					strY = dial->Ambient.altitude.stepImg.y_axis;
					
					imgY = dial->Ambient.altitude.stepStr.y_axis;
				}
				
				if(dial->Ambient.altitude.stepImg.x_axis != 0xFF)
				{
					//图标
					LCD_SetPicture_ExtFlash(DIAL_ALTITUDE_IMG_ADDRESS + (m_dial_num * DIAL_DATA_SIZE) + m_gial_addr_offset, 
											&dial->Ambient.altitude.stepImg, strY);
				}
				
				if(dial->Ambient.altitude.stepStr.x_axis != 0xFF)
				{
					//数据
					LCD_SetString_ExtFlash(str, &dial->Ambient.altitude.stepStr, imgY);
				}
			}
			
			//气压
			if((dial->Ambient.pressure.stepImg.x_axis != 0xFF)
				|| (dial->Ambient.pressure.stepStr.x_axis != 0xFF))
			{
				memset(str,0,sizeof(str));
				if(pres != 0)
				{
					//无气压数据
					#if defined (SPRINTF_FLOAT_TO_INT)
					sprintf(str,"%d.%01d",(int32_t)pres,(int32_t)(fabsf(pres)*10)%10);
					#else
					sprintf(str,"%.1f", pres);
					#endif
				}
				else
				{
					//有气压数据
					sprintf(str,"--");
				}
				len = (strlen(str)*(dial->Ambient.pressure.stepStr.width + dial->Ambient.pressure.stepStr.kerning)
						- dial->Ambient.pressure.stepStr.kerning);
				
				if((dial->Ambient.align2&0x0F) == DIAL_ACTIVEX_ALIGN_CENTER_0)
				{
					//合并居中
					if(dial->Ambient.pressure.stepStr.y_axis == LCD_CENTER_JUSTIFIED)
					{
						strY = (LCD_PIXEL_ONELINE_MAX - (len + dial->Ambient.pressure.gap + dial->Ambient.pressure.stepImg.width))/2;
					}
					else
					{
						strY = dial->Ambient.pressure.stepStr.y_axis - ((len + dial->Ambient.pressure.gap + dial->Ambient.pressure.stepImg.width)/2);
					}
					
					imgY = strY + dial->Ambient.pressure.stepImg.width + dial->Ambient.pressure.gap;
				}
				else if((dial->Ambient.align2&0x0F) == DIAL_ACTIVEX_ALIGN_LEFT_0)
				{
					//合并左对齐
					strY = dial->Ambient.pressure.stepStr.y_axis;
					
					imgY = strY + dial->Ambient.pressure.stepImg.width + dial->Ambient.pressure.gap;
				}
				else if((dial->Ambient.align2&0x0F) == DIAL_ACTIVEX_ALIGN_RIGHT_0)
				{
					//合并右对齐
					strY = dial->Ambient.pressure.stepStr.y_axis - (len + dial->Ambient.pressure.gap + dial->Ambient.pressure.stepImg.width);
					
					imgY = strY + dial->Ambient.pressure.stepImg.width + dial->Ambient.pressure.gap;
				}
				else if((dial->Ambient.align2&0x0F) == DIAL_ACTIVEX_ALIGN_CENTER_1)
				{
					//居中
					strY = dial->Ambient.pressure.stepImg.y_axis - (dial->Ambient.pressure.stepImg.width/2);
					
					imgY = dial->Ambient.pressure.stepStr.y_axis - (len/2);
				}
				else if((dial->Ambient.align2&0x0F) == DIAL_ACTIVEX_ALIGN_RIGHT_1)
				{
					//右对齐
					strY = dial->Ambient.pressure.stepImg.y_axis - dial->Ambient.pressure.stepImg.width;
					
					imgY = dial->Ambient.pressure.stepStr.y_axis - len;
				}
				else
				{
					strY = dial->Ambient.pressure.stepImg.y_axis;
					
					imgY = dial->Ambient.pressure.stepStr.y_axis;
				}
				
				if(dial->Ambient.pressure.stepImg.x_axis != 0xFF)
				{
					//图标
					LCD_SetPicture_ExtFlash(DIAL_PRESSURE_IMG_ADDRESS + (m_dial_num * DIAL_DATA_SIZE) + m_gial_addr_offset, 
											&dial->Ambient.pressure.stepImg, strY);
				}
				
				if(dial->Ambient.pressure.stepStr.x_axis != 0xFF)
				{
					//数据
					LCD_SetString_ExtFlash(str, &dial->Ambient.pressure.stepStr, imgY);
				}
			}
			
			//温度
			if((dial->Ambient.temp.stepImg.x_axis != 0xFF)
				|| (dial->Ambient.temp.stepStr.x_axis != 0xFF))
			{
				memset(str,0,sizeof(str));
				if(temp != 0)
				{
					//有温度数据
					#if defined (SPRINTF_FLOAT_TO_INT)
					sprintf(str,"%d.%01d",(int32_t)temp,(int32_t)(fabsf(temp)*10)%10);
					#else
					sprintf(str,"%.1f", temp);
					#endif
				}
				else
				{
					//无温度数据
					sprintf(str,"--");
				}
				len = (strlen(str)*(dial->Ambient.temp.stepStr.width + dial->Ambient.temp.stepStr.kerning)
						- dial->Ambient.temp.stepStr.kerning);
				
				if(((dial->Ambient.align2&0xF0)>>4) == DIAL_ACTIVEX_ALIGN_CENTER_0)
				{
					//合并居中
					if(dial->Ambient.temp.stepStr.y_axis == LCD_CENTER_JUSTIFIED)
					{
						strY = (LCD_PIXEL_ONELINE_MAX - (len + dial->Ambient.temp.gap + dial->Ambient.temp.stepImg.width))/2;
					}
					else
					{
						strY = dial->Ambient.temp.stepStr.y_axis - ((len + dial->Ambient.temp.gap + dial->Ambient.temp.stepImg.width)/2);
					}
					
					imgY = strY + dial->Ambient.temp.stepImg.width + dial->Ambient.temp.gap;
				}
				else if(((dial->Ambient.align2&0xF0)>>4) == DIAL_ACTIVEX_ALIGN_LEFT_0)
				{
					//合并左对齐
					strY = dial->Ambient.temp.stepStr.y_axis;
					
					imgY = strY + dial->Ambient.temp.stepImg.width + dial->Ambient.temp.gap;
				}
				else if(((dial->Ambient.align2&0xF0)>>4) == DIAL_ACTIVEX_ALIGN_RIGHT_0)
				{
					//合并右对齐
					strY = dial->Ambient.temp.stepStr.y_axis - (len + dial->Ambient.temp.gap + dial->Ambient.temp.stepImg.width);
					
					imgY = strY + dial->Ambient.temp.stepImg.width + dial->Ambient.temp.gap;
				}
				else if(((dial->Ambient.align2&0xF0)>>4) == DIAL_ACTIVEX_ALIGN_CENTER_1)
				{
					//居中
					strY = dial->Ambient.temp.stepImg.y_axis - (dial->Ambient.temp.stepImg.width/2);
					
					imgY = dial->Ambient.temp.stepStr.y_axis - (len/2);
				}
				else if(((dial->Ambient.align2&0xF0)>>4) == DIAL_ACTIVEX_ALIGN_RIGHT_1)
				{
					//右对齐
					strY = dial->Ambient.temp.stepImg.y_axis - dial->Ambient.temp.stepImg.width;
					
					imgY = dial->Ambient.temp.stepStr.y_axis - len;
				}
				else
				{
					strY = dial->Ambient.temp.stepImg.y_axis;
					
					imgY = dial->Ambient.temp.stepStr.y_axis;
				}
				
				if(dial->Ambient.temp.stepImg.x_axis != 0xFF)
				{
					//图标
					LCD_SetPicture_ExtFlash(DIAL_TEMP_IMG_ADDRESS + (m_dial_num * DIAL_DATA_SIZE) + m_gial_addr_offset, 
											&dial->Ambient.temp.stepImg, strY);
				}
				
				if(dial->Ambient.temp.stepStr.x_axis != 0xFF)
				{
					//数据
					LCD_SetString_ExtFlash(str, &dial->Ambient.temp.stepStr, imgY);
				}
			}
			break;
			case DIAL_AMBIENT_TYPE_2:
			  word.x_axis = 120 -10;
			  word.y_axis = 120 -(img_dial_fiag_pa.width /2) ;
			  word.forecolor = LCD_RED;
			  word.bckgrndcolor = LCD_NONE;
			  word.kerning = 1;
			  
			  LCD_SetPicture(word.x_axis, word.y_axis,  word.forecolor,  word.bckgrndcolor, &img_dial_fiag_pa);
			  word.x_axis = 120 -12 + img_dial_fiag_pa.height +10;
			 
			  word.size = LCD_FONT_17_SIZE;
			  memset(str,0,sizeof(str));
			  sprintf(str,"%.1f",pres);
			  len = (strlen(str)*(Font_Number_17.width + word.kerning));
			  word.y_axis = 120 -(len/2)+3;
			  word.forecolor = LCD_WHITE;
			  LCD_SetNumber(str,&word);
			


			calorie = Get_TotalEnergy();		
			memset(str,0,sizeof(str));
			sprintf(str,"%d", calorie/1000);
	
			word.size = LCD_FONT_17_SIZE;
			word.forecolor = LCD_WHITE;
			word.bckgrndcolor = LCD_NONE;
			word.kerning = 1;
			word.x_axis = 120 - 13;
			word.y_axis = 200 - (img_dial_cal_15x20.width /2);
			//卡路里
 			LCD_SetPicture(word.x_axis, word.y_axis, LCD_PERU, LCD_NONE, &img_dial_cal_15x20);
 		   //word.y_axis += img_dial_cal_15x20.width +4;
   
  		   word.x_axis = 120-12 +img_dial_cal_15x20.height +4 ;
		    len = (strlen(str)*(Font_Number_17.width + word.kerning));
			word.y_axis = 200 -(len/2);
 		   LCD_SetNumber(str,&word);
		break;
		default:
			break;
	}
}


void gui_dial_chart()
{
	//X轴刻度上限152,下限209,差57 除3 =19;Y轴 40~200;48*N
     LCD_SetRectangle(164-1,1,0,240,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
	
	LCD_SetRectangle(164+15-1,1,0,240,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
	
	LCD_SetRectangle(164+15*2-1,1,0,240,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
  
    LCD_SetRectangle(164+15*3-1,1,0,240,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
	
	LCD_SetRectangle(164,45,48-1,1,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
	
	LCD_SetRectangle(164,45,48*2-1,1,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
    
	LCD_SetRectangle(164,45,48*3-1,1,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
	
	LCD_SetRectangle(164,45,48*4-1,1,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
}

static void gui_dial_graph(DialDataStr *dial)
{
	switch(dial->Graph.Type1.type)
	{
		case DIAL_GRAPH_TYPE_WEEK_DISTANCE:
			//一周里程直方图
			gui_distance_histogram(dial->Graph.Type1.star_x, dial->Graph.Type1.height, 
									dial->Graph.Type1.star_y, dial->Graph.Type1.width);
			break;
		case DIAL_GRAPH_TYPE_VO2MAX:
			//最大摄氧量图
			gui_VO2max_histogram(dial->Graph.Type1.star_x);
			break;
		case DIAL_GRAPH_TYPE_12H_PRESSURE:
			//过去12H气压变化图
			gui_12h_pressure(dial->Graph.Type1.star_x, dial->Graph.Type1.height, 
								dial->Graph.Type1.star_y, dial->Graph.Type1.width, dial->Graph.Type1.forecolor, dial->Graph.Type1.bckgrndcolor);
		   
			break;
		case DIAL_GRAPH_TYPE_STEP_DEG:
			//用于显示步数进度条图标，在计步函数中实现
			break;
		default:
			break;
	}
}



