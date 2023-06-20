#include "gui_tool_config.h"
#include "gui_tool_calendar.h"
#include "gui_home.h"
#include "bsp_rtc.h"

#include "drv_lcd.h"
#include "drv_key.h"

#include "task_tool.h"

#include "gui.h"
#include "gui_tool.h"

#include "font_config.h"

#include "com_data.h"
#include "watch_config.h"

#if DEBUG_ENABLED == 1 && GUI_TOOL_CALENDAR_LOG_ENABLED == 1
	#define GUI_TOOL_CALENDAR_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_TOOL_CALENDAR_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define GUI_TOOL_CALENDAR_WRITESTRING(...)
	#define GUI_TOOL_CALENDAR_PRINTF(...)		        
#endif



//下面是张琛对天气界面的处理，需要张云最后修改确定
//进入界面之前读出FLASH天气数据，如果CRC不对或者天数等于0说明没有数据，或者超出天数数组，显示无天气数据请连接APP同步
//7天数据总共239个字节，两中RAM处理方式：一，用RTOS ALLOC出动态RAM，用完释放，二，用静态RAM
//下面用，用静态RAM

#include "drv_extFlash.h"
#include "am_bootloader.h"

//AppWeatherInfo m_weather_info = {.days = 0, .day_index = 0};
#ifdef COD 
CodWeatherSync g_weather_info;
#endif
const unsigned int LunarCalendarTable[199] =  
{  
    0x04AE53,0x0A5748,0x5526BD,0x0D2650,0x0D9544,0x46AAB9,0x056A4D,0x09AD42,0x24AEB6,0x04AE4A,/*1901-1910*/  
    0x6A4DBE,0x0A4D52,0x0D2546,0x5D52BA,0x0B544E,0x0D6A43,0x296D37,0x095B4B,0x749BC1,0x049754,/*1911-1920*/  
    0x0A4B48,0x5B25BC,0x06A550,0x06D445,0x4ADAB8,0x02B64D,0x095742,0x2497B7,0x04974A,0x664B3E,/*1921-1930*/  
    0x0D4A51,0x0EA546,0x56D4BA,0x05AD4E,0x02B644,0x393738,0x092E4B,0x7C96BF,0x0C9553,0x0D4A48,/*1931-1940*/  
    0x6DA53B,0x0B554F,0x056A45,0x4AADB9,0x025D4D,0x092D42,0x2C95B6,0x0A954A,0x7B4ABD,0x06CA51,/*1941-1950*/  
    0x0B5546,0x555ABB,0x04DA4E,0x0A5B43,0x352BB8,0x052B4C,0x8A953F,0x0E9552,0x06AA48,0x6AD53C,/*1951-1960*/  
    0x0AB54F,0x04B645,0x4A5739,0x0A574D,0x052642,0x3E9335,0x0D9549,0x75AABE,0x056A51,0x096D46,/*1961-1970*/  
    0x54AEBB,0x04AD4F,0x0A4D43,0x4D26B7,0x0D254B,0x8D52BF,0x0B5452,0x0B6A47,0x696D3C,0x095B50,/*1971-1980*/  
    0x049B45,0x4A4BB9,0x0A4B4D,0xAB25C2,0x06A554,0x06D449,0x6ADA3D,0x0AB651,0x093746,0x5497BB,/*1981-1990*/  
    0x04974F,0x064B44,0x36A537,0x0EA54A,0x86B2BF,0x05AC53,0x0AB647,0x5936BC,0x092E50,0x0C9645,/*1991-2000*/  
    0x4D4AB8,0x0D4A4C,0x0DA541,0x25AAB6,0x056A49,0x7AADBD,0x025D52,0x092D47,0x5C95BA,0x0A954E,/*2001-2010*/  
    0x0B4A43,0x4B5537,0x0AD54A,0x955ABF,0x04BA53,0x0A5B48,0x652BBC,0x052B50,0x0A9345,0x474AB9,/*2011-2020*/  
    0x06AA4C,0x0AD541,0x24DAB6,0x04B64A,0x69573D,0x0A4E51,0x0D2646,0x5E933A,0x0D534D,0x05AA43,/*2021-2030*/  
    0x36B537,0x096D4B,0xB4AEBF,0x04AD53,0x0A4D48,0x6D25BC,0x0D254F,0x0D5244,0x5DAA38,0x0B5A4C,/*2031-2040*/  
    0x056D41,0x24ADB6,0x049B4A,0x7A4BBE,0x0A4B51,0x0AA546,0x5B52BA,0x06D24E,0x0ADA42,0x355B37,/*2041-2050*/  
    0x09374B,0x8497C1,0x049753,0x064B48,0x66A53C,0x0EA54F,0x06B244,0x4AB638,0x0AAE4C,0x092E42,/*2051-2060*/  
    0x3C9735,0x0C9649,0x7D4ABD,0x0D4A51,0x0DA545,0x55AABA,0x056A4E,0x0A6D43,0x452EB7,0x052D4B,/*2061-2070*/  
    0x8A95BF,0x0A9553,0x0B4A47,0x6B553B,0x0AD54F,0x055A45,0x4A5D38,0x0A5B4C,0x052B42,0x3A93B6,/*2071-2080*/  
    0x069349,0x7729BD,0x06AA51,0x0AD546,0x54DABA,0x04B64E,0x0A5743,0x452738,0x0D264A,0x8E933E,/*2081-2090*/  
    0x0D5252,0x0DAA47,0x66B53B,0x056D4F,0x04AE45,0x4A4EB9,0x0A4D4C,0x0D1541,0x2D92B5          /*2091-2099*/  
};

const int MonthAdd[12] = {0,31,59,90,120,151,181,212,243,273,304,334};

static unsigned int LunarCalendarDay;

extern am_hal_rtc_time_t RTC_time;
extern SetValueStr SetValue;

extern SWITCH_HOME_SCREEN_T switch_home_screen;
extern HOME_OR_MINOR_SCREEN_T home_or_minor_switch;
extern uint16_t s_switch_screen_count;
extern ScreenState_t back_monitor_screen;
static uint8_t flag;//用于指示第一次进入天气详细界面
uint8_t calendar_weather_data[]  = {LCD_RED,LCD_YELLOW,LCD_NONE,LCD_NONE,LCD_NONE,LCD_BLUE,LCD_DARKTURQUOISE,LCD_CYAN,LCD_LIGHTGRAY,

LCD_CYAN,LCD_WHITE,LCD_WHEAT,LCD_WHITE,LCD_PERU,LCD_WHITE};

uint8_t calendar_weather_index = 0;

static int LunarCalendar(int year,int month,int day)
{  
    int Spring_NY,Sun_NY,StaticDayCount;  
    int index,flag;  
    //Spring_NY 记录春节离当年元旦的天数。  
    //Sun_NY 记录阳历日离当年元旦的天数。  
    if ( ((LunarCalendarTable[year-1901] & 0x0060) >> 5) == 1)  
        Spring_NY = (LunarCalendarTable[year-1901] & 0x001F) - 1;  
    else  
        Spring_NY = (LunarCalendarTable[year-1901] & 0x001F) - 1 + 31;  
    Sun_NY = MonthAdd[month-1] + day - 1;  
    if ( (!(year % 4)) && (month > 2))  
        Sun_NY++;  
    //StaticDayCount记录大小月的天数 29 或30  
    //index 记录从哪个月开始来计算。  
    //flag 是用来对闰月的特殊处理。  
    //判断阳历日在春节前还是春节后  
    if (Sun_NY >= Spring_NY)//阳历日在春节后（含春节那天）  
    {  
        Sun_NY -= Spring_NY;  
        month = 1;  
        index = 1;  
        flag = 0;  
        if ( ( LunarCalendarTable[year - 1901] & (0x80000 >> (index-1)) ) ==0)  
            StaticDayCount = 29;  
        else  
            StaticDayCount = 30;  
        while (Sun_NY >= StaticDayCount)  
        {  
            Sun_NY -= StaticDayCount;  
            index++;  
            if (month == ((LunarCalendarTable[year - 1901] & 0xF00000) >> 20) )  
            {  
                flag = ~flag;  
                if (flag == 0)  
                    month++;  
            }  
            else  
                month++;  
            if ( ( LunarCalendarTable[year - 1901] & (0x80000 >> (index-1)) ) ==0)  
                StaticDayCount=29;  
            else  
                StaticDayCount=30;  
        }  
        day = Sun_NY + 1;  
    }  
    else //阳历日在春节前  
    {  
        Spring_NY -= Sun_NY;  
        year--;  
        month = 12;  
        if ( ((LunarCalendarTable[year - 1901] & 0xF00000) >> 20) == 0)  
            index = 12;  
        else  
            index = 13;  
        flag = 0;  
        if ( ( LunarCalendarTable[year - 1901] & (0x80000 >> (index-1)) ) ==0)  
            StaticDayCount = 29;  
        else  
            StaticDayCount = 30;  
        while (Spring_NY > StaticDayCount)  
        {  
            Spring_NY -= StaticDayCount;  
            index--;  
            if (flag == 0)  
                month--;  
            if (month == ((LunarCalendarTable[year - 1901] & 0xF00000) >> 20))  
                flag = ~flag;  
            if ( ( LunarCalendarTable[year - 1901] & (0x80000 >> (index-1)) ) ==0)  
                StaticDayCount = 29;  
            else  
                StaticDayCount = 30;  
        }  
        day = StaticDayCount - Spring_NY + 1;  
    }  
    LunarCalendarDay |= day;  
    LunarCalendarDay |= (month << 6);  
    if (month == ((LunarCalendarTable[year - 1901] & 0xF00000) >> 20))
	{
		//润月
        return 1;  
	}
    else  
	{
        return 0;  
	}
} 

void gui_tool_calendar_date(void)
{
	SetWord_t word = {0};
	char str[5] = {0};
	int8_t i;
	uint8_t week,day;
	uint8_t buf[7] = {0};
	uint8_t lunar_month,lunar_day;
	
	//获取当前时间
	am_hal_rtc_time_get(&RTC_time);
	

	//sprintf(str,"%d",RTC_time.ui32Month);
/*	word.kerning = 1;
	word.y_axis = (LCD_PIXEL_ONELINE_MAX - strlen(str)*(Font_Number_29.width + word.kerning) + word.kerning)/2;
	word.size = LCD_FONT_29_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	LCD_SetNumber(str,&word);*/

	
	gui_tool_calendar_weather_info_init();
	calendar_weather_index = gui_tool_calendar_weather_get(RTC_time.ui32Year, RTC_time.ui32Month, RTC_time.ui32DayOfMonth,RTC_time.ui32Hour);

	//calendar_weather_index = 3;
	memset(str,0,sizeof(str));
    word.x_axis = 70;
	#ifdef COD 
	if((calendar_weather_index < 24)&&(g_weather_info.cod_weather[calendar_weather_index].weather<=14))
	{
	   
       sprintf(str,"%d",g_weather_info.cod_weather[calendar_weather_index].temperature);//温度
       
       word.kerning = 1;
	   word.y_axis = LCD_PIXEL_ONELINE_MAX /2 - (strlen(str)*(Font_Number_29.width + word.kerning) - word.kerning) - 2;
	   word.size = LCD_FONT_29_SIZE;
	   word.forecolor = LCD_WHITE;
	   word.bckgrndcolor = LCD_NONE;
       LCD_SetNumber(str,&word);


      word.x_axis += 1;
	  word.y_axis = LCD_PIXEL_ONELINE_MAX/2 +1;
      word.forecolor = LCD_WHITE;
	  word.bckgrndcolor = LCD_NONE;
	  LCD_SetPicture( word.x_axis,word.y_axis,word.forecolor, word.bckgrndcolor,&img_calendar_temp);//度


		//天气图标
		LCD_SetPicture(word.x_axis + Font_Number_29.height/2 - img_Weather_lists[0].height/2,
					LCD_PIXEL_ONELINE_MAX /2 - (strlen(str)*(Font_Number_29.width + word.kerning) - word.kerning) - 2 - 8 - img_Weather_lists[0].width,
						calendar_weather_data[g_weather_info.cod_weather[calendar_weather_index].weather],LCD_NONE,&img_Weather_lists[g_weather_info.cod_weather[calendar_weather_index].weather]);

				//农历月份
			LunarCalendarDay = 0;
			LunarCalendar(RTC_time.ui32Year+2000,RTC_time.ui32Month,RTC_time.ui32DayOfMonth);
			lunar_month = (LunarCalendarDay & 0x3C0) >> 6;
			lunar_day = (LunarCalendarDay & 0x3F);
			word.x_axis += (img_calendar_temp.height/2  - 2 - 16);
			word.y_axis += img_calendar_temp.width + 12;
			word.size = LCD_FONT_16_SIZE;
			word.kerning = 1;
			LCD_SetString((char *)MonthStrs[SetValue.Language][lunar_month-1],&word);
			
			//农历日期
			word.x_axis += 16 + 4;
			LCD_SetString((char *)LunarStrs[SetValue.Language][lunar_day-1],&word);


			
	}
  else 
  	{
        //无天气信息


     	//天气图标
		LCD_SetPicture(word.x_axis + img_calendar_temp.height/2 - img_weather_update_big.height/2,120 - 4 - img_weather_update_big.width,LCD_WHITE,LCD_NONE,&img_weather_update_big);

				//农历月份
			LunarCalendarDay = 0;
			LunarCalendar(RTC_time.ui32Year+2000,RTC_time.ui32Month,RTC_time.ui32DayOfMonth);
			lunar_month = (LunarCalendarDay & 0x3C0) >> 6;
			lunar_day = (LunarCalendarDay & 0x3F);
			word.x_axis += (img_calendar_temp.height/2  - 2 - 16);
			word.y_axis = 120 + 4;
			word.size = LCD_FONT_16_SIZE;
			word.forecolor = LCD_WHITE;
	        word.bckgrndcolor = LCD_NONE;
			word.kerning = 1;
			LCD_SetString((char *)MonthStrs[SetValue.Language][lunar_month-1],&word);
			
			//农历日期
			word.x_axis += 16 + 4;
			LCD_SetString((char *)LunarStrs[SetValue.Language][lunar_day-1],&word);
		/*
     		//农历月份
			LunarCalendarDay = 0;
			LunarCalendar(RTC_time.ui32Year+2000,RTC_time.ui32Month,RTC_time.ui32DayOfMonth);
			lunar_month = (LunarCalendarDay & 0x3C0) >> 6;
			lunar_day = (LunarCalendarDay & 0x3F);
			word.x_axis += (img_calendar_temp.height/2  - 2 - 16);
			word.y_axis = LCD_CENTER_JUSTIFIED;
			word.size = LCD_FONT_16_SIZE;
			 word.forecolor = LCD_WHITE;
	        word.bckgrndcolor = LCD_NONE;
			word.kerning = 1;
			
			LCD_SetString((char *)MonthStrs[SetValue.Language][lunar_month-1],&word);
			
			//农历日期
			word.x_axis += 16 + 4;
			LCD_SetString((char *)LunarStrs[SetValue.Language][lunar_day-1],&word);*/
			

   }

#endif
	

	week = RTC_time.ui32Weekday%7;
	day = RTC_time.ui32DayOfMonth;
	for(i = week;i >= 0;i--)
	{
		buf[i] = day;
		if(day > 1)
		{
			day--;
		}
		else
		{
			if( RTC_time.ui32Month == 1 )
			{
				day = 31;
			}
			else
			{
				day = MonthMaxDay(RTC_time.ui32Year,RTC_time.ui32Month-1);
			}
		}
	}
	day = RTC_time.ui32DayOfMonth;
	for(i = week;i < 7;i++)
	{
		buf[i] = day;
		if(day < (MonthMaxDay(RTC_time.ui32Year,RTC_time.ui32Month)))
		{
			day++;
		}
		else
		{
			day = 1;
		}
	}
	
#if defined WATCH_SIM_NUMBER
	uint8_t width;
	
	//一周
	//word.x_axis = 140;
	word.x_axis = 120 + 8;
	word.y_axis = (LCD_PIXEL_ONELINE_MAX - (7*img_week_13x13[0].width*2-img_week_13x13[0].width))/2;
	for(i = 0;i < 7;i++)
	{
		//word.y_axis += 3;
		LCD_SetPicture(word.x_axis, word.y_axis, LCD_GRAY, LCD_NONE, &img_week_13x13[i]);
		word.y_axis += (img_week_13x13[0].width*2);
	}
		
	for(i = 0;i < 7;i++)
	{
		if(buf[i] != 0)
		{
			word.x_axis = 120 + 8;
			width = (LCD_PIXEL_ONELINE_MAX - (7*img_week_13x13[0].width*2 - img_week_13x13[0].width))/2 + i*(img_week_13x13[0].width*2);
			
			if(i == week)
			{
				//选中当前日期
				//画选中的红线
				word.y_axis = width;
				LCD_SetRectangle(word.x_axis-4,1, word.y_axis - 6,6+img_week_13x13[i].width+6, 
									LCD_RED, 0, 0, LCD_FILL_ENABLE);	//上端横线
				LCD_SetRectangle(word.x_axis-4, 4+img_week_13x13[0].height + 9 + Font_Number_13.height+4, word.y_axis - 6, 1, 
									LCD_RED, 0, 0, LCD_FILL_ENABLE);	//左端竖线
				LCD_SetRectangle(word.x_axis-4, 4+img_week_13x13[0].height + 9 + Font_Number_13.height+4, word.y_axis + (img_week_13x13[i].width) + 6, 1, 
									LCD_RED, 0, 0, LCD_FILL_ENABLE);	//右端竖线
				LCD_SetRectangle(word.x_axis+img_week_13x13[0].height + 9 + Font_Number_13.height+4, 1, word.y_axis - 6, 6+img_week_13x13[i].width+6, 
								   LCD_RED, 0, 0, LCD_FILL_ENABLE);	//下端横线
				
				word.size = LCD_FONT_13_SIZE;
				word.x_axis += (img_week_13x13[0].height + 9);
				word.kerning = 1;
				word.forecolor = LCD_WHITE;
				word.bckgrndcolor = LCD_NONE;
				memset(str,0,sizeof(str));
				sprintf(str,"%d",buf[i]);
			
				word.y_axis += ((Font_Number_13.width/2)*(strlen(str)%2));
				LCD_SetNumber(str,&word);
				
				word.x_axis += (Font_Number_13.height + 12);
				word.y_axis = width;
				LCD_SetPicture(word.x_axis,word.y_axis,LCD_RED,LCD_NONE,&Img_Pointing_Up_12X8);

				 word.x_axis = 120 + 8;
		         word.y_axis = (LCD_PIXEL_ONELINE_MAX - (7*img_week_13x13[0].width*2-img_week_13x13[0].width))/2 + week*(img_week_13x13[0].width*2) ;
                    
				LCD_SetPicture(word.x_axis, word.y_axis, LCD_WHITE, LCD_NONE, &img_week_13x13[week]);
			}
			else
			{
				if(((i < week) && (buf[i] > buf[week]))
					|| ((i > week) && (buf[i] < buf[week])))
				{
					//其他月份的显示灰色
					word.forecolor = LCD_GRAY;
				}
				else
				{
					word.forecolor = LCD_WHITE;
				}
				
				word.forecolor = LCD_GRAY;
				word.bckgrndcolor = LCD_NONE;
				word.size = LCD_FONT_13_SIZE;
				word.x_axis += (img_week_13x13[0].height + 9);
				word.kerning = 0;
				memset(str,0,sizeof(str));
				sprintf(str,"%d",buf[i]);
				word.y_axis = width + ((Font_Number_13.width/2)*(strlen(str)%2));
				LCD_SetNumber(str,&word);
			}
		}
	}
#else
	//一周
	word.x_axis = 134;
	word.y_axis = (LCD_PIXEL_ONELINE_MAX - 7*16 - 6*8)/2;
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 8;
	LCD_SetString("日一二三四五六",&word);
		
	for(i = 0;i < 7;i++)
	{
		if(buf[i] != 0)
		{
			if(i == week)
			{
				//选中当前日期
				//画选中的红线
				word.y_axis = (LCD_PIXEL_ONELINE_MAX - 7*16 - 6*8)/2 + i*(8 + 2*8);
				LCD_SetRectangle(153, 1, 0, word.y_axis - 4, LCD_RED, 0, 0, LCD_FILL_ENABLE);		//左端横线
				LCD_SetRectangle(134, 42, word.y_axis - 4, 1, LCD_RED, 0, 0, LCD_FILL_ENABLE);		//左端竖线
				LCD_SetRectangle(134, 42, word.y_axis + 16 + 3, 1, LCD_RED, 0, 0, LCD_FILL_ENABLE);	//右端竖线
				LCD_SetRectangle(153, 1, word.y_axis + 16 + 3, LCD_PIXEL_ONELINE_MAX - (word.y_axis + 16), 
									LCD_RED, 0, 0, LCD_FILL_ENABLE);	//右端横线
				word.size = LCD_FONT_16_SIZE;
				word.x_axis  = 162;
				word.forecolor = LCD_WHITE;
				word.bckgrndcolor = LCD_BLACK;
				word.kerning = 0;
				memset(str,0,5);
				sprintf(str,"%d",buf[i]);
				word.y_axis = (LCD_PIXEL_ONELINE_MAX - 7*16 - 6*8)/2 
								+ i*(8 + 2*8) + 4*(strlen(str)%2);
				LCD_SetString(str,&word);
				
				word.x_axis = 182;
				word.y_axis = (LCD_PIXEL_ONELINE_MAX - 7*16 - 6*8)/2 + i*(8 + 2*8) + 2;
				LCD_SetPicture(word.x_axis,word.y_axis,LCD_RED,LCD_NONE,&Img_Pointing_Up_12X8);
			}
			else
			{
				word.size = LCD_FONT_16_SIZE;
				word.x_axis  = 162;
				word.forecolor = LCD_WHITE;
				word.bckgrndcolor = LCD_BLACK;
				word.kerning = 0;
				memset(str,0,5);
				sprintf(str,"%d",buf[i]);
				word.y_axis = (LCD_PIXEL_ONELINE_MAX - 7*16 - 6*8)/2 
								+ i*(8 + 2*(8)) + 4*(strlen(str)%2);
				LCD_SetString(str,&word);
			}
		}
	}
#endif

}
 void gui_tool_calendar_weather_no_info(void)
{
	SetWord_t word = {0};
	
	LCD_SetPicture(15,106,LCD_PERU,LCD_NONE,&img_monitor_calendar);
	if(SetValue.Language == L_CHINESE)
	{
		word.x_axis = 120-8-24;
		
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_24_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_BLACK;
		LCD_SetString("无天气信息",&word);
		word.x_axis = 120+8;
		LCD_SetString("请连接APP同步",&word);				
	}
	else if(SetValue.Language == L_ENGLISH)
	{
		word.x_axis = 120-8-24;
		word.size = LCD_FONT_16_SIZE;
		word.bckgrndcolor = LCD_BLACK;
		word.forecolor = LCD_WHITE;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		LCD_SetString("NO WEATHER INFO",&word);
		word.x_axis = 120+8;
		LCD_SetString("PLZ CONNECT APP",&word);						
	}
}

void gui_tool_calendar_paint(void)
{
	//将背景设置为黑色
	LCD_SetBackgroundColor(LCD_BLACK);
	
	//日历图标
	
	LCD_SetPicture(15,106,LCD_PERU,LCD_NONE,&img_monitor_calendar);
	

	//日期
	gui_tool_calendar_date();
	
	//ok键提示
	//gui_button_ok(LCD_WHITE);
	
	LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_LINE, 112, 6, 150, 210, LCD_RED);//下环圈
	//底部时间
	gui_bottom_time();
}

uint8_t gui_tool_calendar_weather_get(uint8_t year, uint8_t month, uint8_t day,uint8_t Hour)
{
#ifdef COD 	
	
	if((g_weather_info.date[0] != year)||(g_weather_info.date[1] != month)||((day - g_weather_info.date[2])>=2 ))//不同年或月或超过一天
	{
	    return 24;
		
	}
	else if((day - g_weather_info.date[2]) == 1) //第二天
		{
           if(Hour>=g_weather_info.date[3])
           	{
              return 24;
		    }
		   else
		   	{
              
            return  (Hour + 24 - g_weather_info.date[3]);
		    }


	   }
	else
		{
           return (Hour - g_weather_info.date[3]); 

	   }
#endif
}


//对外接口，用于进入界面前初始化天气数据，
//当处于该界面时手机APP更新天气数据时调用和按键进入时调用
void gui_tool_calendar_weather_info_init(void)
{
#ifdef COD 
	dev_extFlash_enable();			
	dev_extFlash_read(WEATHER_DATA_START_ADDRESS, (uint8_t *)&g_weather_info, sizeof(CodWeatherSync));			
	dev_extFlash_disable();	
#endif
		


}

void gui_tool_calendar_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	home_or_minor_switch = MINOR_SCREEN_SWITCH;
	switch(Key_Value)
	{
		case KEY_OK:

			flag = 1;
			GUI_TOOL_CALENDAR_PRINTF("[GUI_TOOL_CALENDAR]:KEY_OK\n");
			ScreenState = DISPLAY_SCREEN_WEATHER_INFO;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);				
		
			break;
		case KEY_UP:
			
			if(((SetValue.U_CustomMonitor.val & 0x01)== 0) &&(((SetValue.U_CustomMonitor.val>>1) & 0x01)== 0)
				&&(((SetValue.U_CustomMonitor.val>>2) & 0x01)== 0)&&(((SetValue.U_CustomMonitor.val>>3) & 0x01)== 0)&&(((SetValue.U_CustomMonitor.val>>4) & 0x01)== 0))
				{
				   
					s_switch_screen_count = 0;
					switch_home_screen = MONITOR_HOME_SCREEN;
					
					back_monitor_screen = DISPLAY_SCREEN_CALENDAR;
                    ScreenState = DISPLAY_SCREEN_HOME;
			         //返回待机界面
			         msg.cmd = MSG_DISPLAY_SCREEN;
			         xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			    }
			else
				{
                  
				set_switch_screen_count_evt(Key_Value);
			    }
			break;
		case KEY_DOWN:
			//set_switch_screen_count_evt(Key_Value);
			
			switch_home_screen = MONITOR_HOME_SCREEN;
			
			back_monitor_screen = DISPLAY_SCREEN_CALENDAR;
			ScreenState = DISPLAY_SCREEN_HOME;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		case (KEY_DOWN<<1):
			set_switch_screen_count_evt(Key_Value);
			break;
		case KEY_BACK:
			//返回待机界面
			
			switch_home_screen = BACK_HOME_SCREEN;
			tool_task_close();
			ScreenState = DISPLAY_SCREEN_HOME;
			
			back_monitor_screen = DISPLAY_SCREEN_CALENDAR;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}







uint16_t get_air_standard(uint16_t value_aqi)
{
    uint16_t aqi,air_quality;
	aqi = value_aqi;
	if(aqi<=50)
		{
         air_quality = GOOD_AIR_QUALITY;
	    }
	else if((aqi>50)&&(aqi<=100))
		{
         
         air_quality = WELL_AIR_QUALITY;
	    }
	else if((aqi>100)&&(aqi<=150))
		{
         
         air_quality = LIGHT_POLUTION_AIR_QUALITY;

	    }
	else if((aqi>150)&&(aqi<=200))
		{
         
         air_quality = MILD_POLUTION_AIR_QUALITY;
	   }
	else if((aqi>200)&&(aqi<=300))
		{
        
		air_quality = HEAVY_POLUTION_AIR_QUALITY;
	   }
	else
		{
          
		  air_quality = INTENSE_POLLUTION_QUALITY;

	    }
   return air_quality;


}

void gui_tool_calendar_weather_paint(void)
{
#ifdef COD 
	
	uint16_t air_quality;
	SetWord_t word = {0};
	
	char str[20] = {0};
	LCD_SetBackgroundColor(LCD_BLACK);
	if((calendar_weather_index < 24)&&(g_weather_info.cod_weather[calendar_weather_index].weather<=14))
		{
   LCD_SetRectangle(LCD_LINE_CNT_MAX/2 - 1,2,LCD_LINE_CNT_MAX/2 - 90,90*2,LCD_WHITE,0,0,LCD_FILL_ENABLE);

	word.x_axis  = 18;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	word.size = LCD_FONT_24_SIZE;
	word.kerning = 1;
	memset(str,0,sizeof(str));
	memcpy(str,g_weather_info.city,sizeof(g_weather_info.city));
	LCD_SetString(str,&word);

     
	 word.x_axis  = LCD_LINE_CNT_MAX/2 - 12 - Font_Number_56.height;
	 memset(str,0,sizeof(str));
	 sprintf(str,"%d",g_weather_info.cod_weather[calendar_weather_index].temperature);//温度
     word.kerning = 1;
	 word.y_axis = LCD_CENTER_JUSTIFIED;
	 word.size = LCD_FONT_56_SIZE;
	 word.forecolor = LCD_WHITE;
	 word.bckgrndcolor = LCD_NONE;
     LCD_SetNumber(str,&word);


     // word.x_axis += 1;
	  word.y_axis = LCD_PIXEL_ONELINE_MAX/2 + (strlen(str)*(Font_Number_56.width + word.kerning) - word.kerning)/2 + 1;
      word.forecolor = LCD_WHITE;
	  word.bckgrndcolor = LCD_NONE;
	  LCD_SetPicture( word.x_axis,word.y_axis,word.forecolor, word.bckgrndcolor,&img_calendar_temp);//度




   LCD_SetPicture(word.x_axis + Font_Number_56.height/2 - img_Weather_lists[0].height/2,
					   LCD_PIXEL_ONELINE_MAX /2 - (strlen(str)*(Font_Number_56.width + word.kerning) - word.kerning)/2  - 8 - img_Weather_lists[0].width,
						   calendar_weather_data[g_weather_info.cod_weather[calendar_weather_index].weather],LCD_NONE,&img_Weather_lists[g_weather_info.cod_weather[calendar_weather_index].weather]);
   

	 word.x_axis  = 120+12;
	 memset(str,0,sizeof(str));
	 sprintf(str,"%d",g_weather_info.cod_weather[calendar_weather_index].aqi);//AQI值
     word.kerning = 1;
	 word.y_axis = LCD_CENTER_JUSTIFIED;
	 word.size = LCD_FONT_56_SIZE;
	 word.forecolor = LCD_WHITE;
	 word.bckgrndcolor = LCD_NONE;
     LCD_SetNumber(str,&word);


   
	word.x_axis  =  120 + 12 + Font_Number_56.height/2 - 12;
	word.y_axis = LCD_PIXEL_ONELINE_MAX /2 - (strlen(str)*(Font_Number_56.width + word.kerning) - word.kerning)/2  - 8 - 36;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	word.size = LCD_FONT_24_SIZE;
	word.kerning = 1;
    memset(str,0,sizeof(str));
	sprintf(str,"AQI");
	LCD_SetString(str,&word);

	air_quality = get_air_standard(g_weather_info.cod_weather[calendar_weather_index].aqi);

		memset(str,0,sizeof(str));
		word.x_axis =  120 + 12 + Font_Number_56.height + 12;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_16_SIZE;
		word.kerning = 1;
		word.forecolor = LCD_WHITE;
	    word.bckgrndcolor = LCD_NONE;
		switch(air_quality)
		{
			case GOOD_AIR_QUALITY:
				sprintf(str,"优");
				
				word.bckgrndcolor = LCD_GREEN;
				LCD_SetString(str,&word);	
			break;
			case WELL_AIR_QUALITY:
				sprintf(str,"良");
				
				word.bckgrndcolor = LCD_ORANGE;
				LCD_SetString(str,&word);
			break;
			case LIGHT_POLUTION_AIR_QUALITY:
				sprintf(str,"轻度污染");
				
				word.bckgrndcolor = LCD_PERU;
				LCD_SetString(str,&word);
			break;
			case MILD_POLUTION_AIR_QUALITY:
				sprintf(str,"中度污染");
				
				word.bckgrndcolor = LCD_RED;
				LCD_SetString(str,&word);
			break;
			case HEAVY_POLUTION_AIR_QUALITY:
				sprintf(str,"重度污染");
				
				word.bckgrndcolor = LCD_MAROON;
				LCD_SetString(str,&word);
			break;
			case INTENSE_POLLUTION_QUALITY:
				sprintf(str,"严重污染");
				
				word.bckgrndcolor = LCD_LIGHTGRAY;
				LCD_SetString(str,&word);
			break;
		}
		}
	else
		{

         gui_tool_calendar_weather_no_info();

	   }

#endif

}
void gui_tool_calendar_weather_btn_evt(uint32_t Key_Value)
{
	switch(Key_Value)
	{		
		case (KEY_BACK):{
			DISPLAY_MSG  msg = {0,0};
			//ScreenState = DISPLAY_SCREEN_CALENDAR;
		   ScreenState =DISPLAY_SCREEN_CALENDAR;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
		}break;			
		case (KEY_UP):{
	/*		if(m_weather_info.days > 0)
			{
				if(m_weather_info.day_index <= 0)
				{
					m_weather_info.day_index = m_weather_info.days - 1;
				}else{
					m_weather_info.day_index--;
				}
				GUI_TOOL_CALENDAR_PRINTF("[GUI_TOOL_CALENDAR]:m_weather_info.day_index --> %d\n",m_weather_info.day_index);	
				DISPLAY_MSG  msg = {0,0};
				ScreenState = DISPLAY_SCREEN_WEATHER_INFO;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}*/
		}break;	
			
		case (KEY_OK):{

		}break;		
		case (KEY_DOWN):{
		/*	if(m_weather_info.days > 0)
			{
				if(m_weather_info.day_index >= (m_weather_info.days - 1))
				{
					m_weather_info.day_index = 0;
				}else{
					m_weather_info.day_index++;
				}
				GUI_TOOL_CALENDAR_PRINTF("[GUI_TOOL_CALENDAR]:m_weather_info.day_index --> %d\n",m_weather_info.day_index);			
				DISPLAY_MSG  msg = {0,0};
				ScreenState = DISPLAY_SCREEN_WEATHER_INFO;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);				
			}*/
		}break;					
	
	}
}


