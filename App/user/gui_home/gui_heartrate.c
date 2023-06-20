/*
*待机功能界面:心率监测：显示一周基础心率、平均基础心率；
*/
#include "gui_home_config.h"
#include "gui_home.h"
#include "gui.h"
#include "gui_tool.h"
#include "gui_heartrate.h"
#include "gui_gps_test.h"

#include "drv_lcd.h"
#include "drv_heartrate.h"
#include "drv_key.h"
#include "drv_extflash.h"
#include "drv_buzzer.h"
#include "drv_motor.h"

#include "task_display.h"
#include "task_key.h"
#include "task_timer.h"
#include "task_hrt.h"
#include "task_gps.h"
#include "task_tool.h"

#include "font_config.h"

#include "algo_hdr.h"

#include "bsp_rtc.h"
#include "bsp_timer.h"
#include "com_data.h"
#include "bsp.h"

#include "Ohrdriver.h"




//#define GUI_HEARTRATE_LOG_ENABLED   1
#if DEBUG_ENABLED == 1 && GUI_HEARTRATE_LOG_ENABLED == 1
	#define GUI_HEARTRATE_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_HEARTRATE_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define GUI_HEARTRATE_WRITESTRING(...)
	#define GUI_HEARTRATE_PRINTF(...)		        
#endif

extern SWITCH_HOME_SCREEN_T switch_home_screen;
extern HOME_OR_MINOR_SCREEN_T home_or_minor_switch;
extern uint16_t s_switch_screen_count;

extern ScreenState_t back_monitor_screen;
static uint8_t weekHdrValue[7] = {0};
uint8_t PostIndex = 0;//自检界面索引

uint8_t stableHdr = 0;

#if defined WATCH_COM_SHORTCUNT_UP
static uint8_t m_heartrate_status;	//历史心率数据状态

void gui_heartrate_data_init(void)
{
	m_heartrate_status = 0;
	
	//获取历史基础心率数据
	GetDayMainData_BasedHDRWeekData(weekHdrValue);
	
	for(uint8_t i = 0; i < 7; i++)
	{
		if(weekHdrValue[i] != 0)
		{
			//有效的历史基础心率值
			m_heartrate_status = 1;
			break;
		}
	}
}

uint8_t gui_heartrate_statu_get(void)
{
	//返回基础心率状态
	return m_heartrate_status;
}
#endif

/*绘制心率波峰图*/
 void gui_heartrate_wave_paint(void)
{
	uint8_t temp = 0,hdrColor = 0;
	for(int i = 0;i < g_HeartNum;i++)
	{
		hdrColor = get_hdr_range_color(g_HeartBuf[i]);
        temp = g_HeartBuf[i]*0.75;
		
		LCD_SetRectangle(240-temp,temp,i*4,2, hdrColor, 0, 2, LCD_FILL_ENABLE);
	}
}

/*平均基础心率柱状图显示*/
static void gui_histogram_hdr_paint(uint8_t day)
{
	SetWord_t word = {0};
	char str[10];
	uint8_t mDayNum = 0,mNewDayNum = 0;
	int max ,min,max_pos = 0,min_pos = 0;
	uint32_t hdrtotal = 0;
	float pixMultiple = 0;
	
	//找出最大最小值
	max = weekHdrValue[0];
	min = 0; 
	for(uint8_t i = 0;i < 7;i++)
	{
		if(weekHdrValue[i] >= max)
		{
			max = weekHdrValue[i];
			max_pos = i;
		}
		hdrtotal += weekHdrValue[i];
		if(hdrtotal == 0)
		{
			min = weekHdrValue[i];
			min_pos = i;
		}
		else
		{
			if(weekHdrValue[i] != 0)
			{
				if(min > weekHdrValue[i] && min != 0 && weekHdrValue[i] != 0)
				{
					min = weekHdrValue[i];
					min_pos = i;
				}
				else if(min == 0)
				{
					if(weekHdrValue[i] != 0)
					{
						min = weekHdrValue[i];
						min_pos = i;
					}
				}
			}
		}
	}
	if(max <= 84)
	{
		pixMultiple = 10;
	}
	else if(max > 84)
	{
		pixMultiple = 84 * 10 / max;
	}
	
	GUI_HEARTRATE_PRINTF("max=%d,min=%d,pixMultiple=%d\n",max,min,pixMultiple);
	//绘制柱状图及顶部最大最小数字
	for(uint8_t i = 0;i < 7;i++)
	{
		mDayNum = day + 1 + i;
		if(mDayNum >= 7)
		{
			mNewDayNum = mDayNum - 7;
		}
		else
		{
			mNewDayNum = mDayNum;
		}

		word.x_axis = 180 - 10 - weekHdrValue[mNewDayNum]*pixMultiple/10;
		word.y_axis = (LCD_LINE_CNT_MAX - 7 * 13 - 6 * 12) / 2 + i * (13+12);
		//weekHdrValue[mNewDayNum]*pixMultiple按比例缩小柱状图
		LCD_SetRectangle(word.x_axis, weekHdrValue[mNewDayNum]*pixMultiple/10, word.y_axis + 3, 7, LCD_RED, 1, 1, LCD_FILL_ENABLE);

		if(mNewDayNum == max_pos && weekHdrValue[max_pos] != 0)
		{
			word.x_axis = 180 - 10 - weekHdrValue[mNewDayNum]*pixMultiple/10 - 20;
			memset(str,0,10);
			sprintf(str,"%d",weekHdrValue[max_pos]);
			word.size = LCD_FONT_16_SIZE;
			word.forecolor = LCD_WHITE;
			word.bckgrndcolor = LCD_NONE;
			word.kerning = 1;
			LCD_SetString(str,&word);//柱状图顶部 最大值
		}
		else if(mNewDayNum == min_pos && weekHdrValue[min_pos] != 0)
		{
			word.x_axis = 180 - 10 - weekHdrValue[mNewDayNum]*pixMultiple/10 - 20;
			memset(str,0,10);
			sprintf(str,"%d",weekHdrValue[min_pos]);
			word.size = LCD_FONT_16_SIZE;
			word.forecolor = LCD_WHITE;
			word.bckgrndcolor = LCD_NONE;
			word.kerning = 1;
			LCD_SetString(str,&word);//柱状图顶部 最小值
		}
		/*if(mNewDayNum == min_pos && weekHdrValue[min_pos] != 0)
		{
	      	
			word.x_axis = 150 + 13 + 20;
			word.y_axis = LCD_CENTER_JUSTIFIED;
			word.size = LCD_FONT_16_SIZE;
			word.forecolor = LCD_WHITE;
			word.bckgrndcolor = LCD_NONE;
			word.kerning = 1;
			LCD_SetString("最低",&word);

			memset(str,0,10);
			sprintf(str,"%d",weekHdrValue[min_pos]);
			word.y_axis = 120 + 24;
			word.size = LCD_FONT_13_SIZE;
			word.forecolor = LCD_WHITE;
			word.bckgrndcolor = LCD_NONE;
			word.kerning = 1;
			LCD_SetNumber(str,&word);
		}
		else if(mNewDayNum == max_pos && weekHdrValue[max_pos] != 0)
		{
		    word.x_axis = 150 + 13 + 20 +16;
			word.y_axis = LCD_CENTER_JUSTIFIED;
			word.size = LCD_FONT_16_SIZE;
			word.forecolor = LCD_WHITE;
			word.bckgrndcolor = LCD_NONE;
			word.kerning = 1;
			LCD_SetString("最高",&word);

			memset(str,0,10);
			sprintf(str,"%d",weekHdrValue[max_pos]);
			word.y_axis = 120 + 24;
			word.size = LCD_FONT_13_SIZE;
			word.forecolor = LCD_WHITE;
			word.bckgrndcolor = LCD_NONE;
			word.kerning = 1;
			LCD_SetNumber(str,&word);
		}*/
	}
	
}
/*界面显示:平均基础心率界面*/
void gui_heartrate_average_based_paint(void)
	{
		SetWord_t word = {0};
	    char str[10];
		uint16_t totalHdrValue = 0;
		uint8_t average_hdr_value = 0;
		uint8_t count_hdr = 0;
		
		am_hal_rtc_time_get(&RTC_time);
		//设置背景黑色
		LCD_SetBackgroundColor(LCD_BLACK);
		//过去一周
		word.x_axis = 15;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_16_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_NONE;
		word.kerning = 1;
		LCD_SetString((char *)GuiHomeLockStrs[SetValue.Language][1],&word);//"过去一周"
		
		word.x_axis += LCD_FONT_16_SIZE + 16;
		word.y_axis = 44;
		word.size = LCD_FONT_16_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_NONE;
		word.kerning = 1;
		LCD_SetString((char *)GuiHomeLockStrs[SetValue.Language][2],&word);//"平均基础心率"
		
	
	  GetDayMainData_BasedHDRWeekData(weekHdrValue);
	
		for(uint8_t i = 0;i < 7; i++)
		{
			if(weekHdrValue[i] > 0)
			{
				count_hdr ++;
				totalHdrValue += weekHdrValue[i];
			}
		}
		if(count_hdr != 0)
		{
			average_hdr_value = totalHdrValue / count_hdr;//周平均基础心率
		}
		else
		{
			average_hdr_value = 0;
		}

		
		memset(str,0,10);
		sprintf(str,"%d",average_hdr_value);

		word.y_axis = 152;
		LCD_SetPicture(word.x_axis, word.y_axis, LCD_RED, LCD_NONE, &Img_heartrate_18X16);


	    word.x_axis += 16/2 - Font_Number_25.height/2;
		word.y_axis = 180;
		word.size = LCD_FONT_25_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_NONE;
		word.kerning = 1;
		LCD_SetNumber(str,&word);//平均心率值
		
	/*	word.y_axis = 152;
		word.size = LCD_FONT_16_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_NONE;
		word.kerning = 1;
		LCD_SetString(str,&word);//平均心率值
		
		word.y_axis = 180;
		LCD_SetPicture(word.x_axis, word.y_axis, LCD_RED, LCD_NONE, &Img_heartrate_18X16);*/
		
		gui_histogram_hdr_paint(RTC_time.ui32Weekday);
		gui_daysofweek_paint(RTC_time.ui32Weekday);//week
	}

/*{




	GUI_HEARTRATE_PRINTF("heartrate_week_average_based_paint\r\n");

	SetWord_t word = {0};
    char str[10];
	uint16_t totalHdrValue = 0;
	uint8_t average_hdr_value = 0;
	uint8_t count_hdr = 0;
	
	am_hal_rtc_time_get(&RTC_time);
	//设置背景黑色
	LCD_SetBackgroundColor(LCD_BLACK);
	//清屏

	//过去一周
	word.x_axis = 15;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	LCD_SetString((char *)GuiHomeLockStrs[SetValue.Language][1],&word);//"过去一周"
	
	word.x_axis += LCD_FONT_16_SIZE + 16;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	LCD_SetString((char *)GuiHomeLockStrs[SetValue.Language][2],&word);//"平均基础心率"
	
	
	
  GetDayMainData_BasedHDRWeekData(weekHdrValue);

	for(uint8_t i = 0;i < 7; i++)
	{
		if(weekHdrValue[i] > 0)
		{
			count_hdr ++;
			totalHdrValue += weekHdrValue[i];
		}
	}
	if(count_hdr != 0)
	{
		average_hdr_value = totalHdrValue / count_hdr;//周平均基础心率
	}
	else
	{
		average_hdr_value = 0;
	}

    word.x_axis = (LCD_LINE_CNT_MAX - Font_Number_56.height)/2;
	if(average_hdr_value >= 100)
	{
		word.y_axis = 136;
	}
	else if(average_hdr_value >= 10)
	{
		word.y_axis = 144;
	}
	else
	{
		word.y_axis = 152;
	}
	
	memset(str,0,10);
	sprintf(str,"%d",average_hdr_value);
	word.size = LCD_FONT_56_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	//LCD_SetString(str,&word);//平均心率值
	LCD_SetNumber(str,&word);//平均心率值

	word.x_axis -= (4 + 16);
	//word.y_axis = 180;
	word.y_axis += (strlen(str)*(Font_Number_56.width + word.kerning) - word.kerning)/2;
	word.y_axis -= (18/2);
	LCD_SetPicture(word.x_axis, word.y_axis, LCD_RED, LCD_NONE, &Img_heartrate_18X16);
	
	gui_histogram_hdr_paint(RTC_time.ui32Weekday);
	gui_daysofweek_paint(RTC_time.ui32Weekday);//week
}*/


/*界面显示:心率测量提示界面*/
void gui_heartrate_hint_measuring_paint(void)
{
	SetWord_t word = {0};
	//设置背景黑色
	LCD_SetBackgroundColor(LCD_BLACK);
	//清屏
	LCD_SetRectangle(0, LCD_LINE_CNT_MAX, 0, LCD_LINE_CNT_MAX, LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
	
	gui_point_direction(IMG_POINT_LEFT,LCD_WHITE);//左箭头
	
	word.x_axis = LCD_LINE_CNT_MAX/2 - 6 - LCD_FONT_24_SIZE;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	LCD_SetString((char *)GuiHomeLockStrs[SetValue.Language][4],&word);//"请佩戴好腕表"
	
	word.x_axis = LCD_LINE_CNT_MAX/2 + 6;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	LCD_SetString((char *)GuiHomeLockStrs[SetValue.Language][79],&word);//"按OK键监测心率"
}




/*界面显示:心率跳动监测界面*/
void gui_heartrate_paint(uint8_t value)
{


	SetWord_t word = {0};
  char str[30];
	static uint8_t n = 0;
		//设置背景黑色
	LCD_SetBackgroundColor(LCD_BLACK);
  
	for(uint8_t i = 0;i < 2;i++)
	{
		//清屏
		
		LCD_SetRectangle(35, 47, 45, 118, LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
		if(n == i)
		{
			word.x_axis = 50;
			word.y_axis = 55;
			LCD_SetPicture(word.x_axis, word.y_axis, LCD_RED, LCD_NONE, &Img_heartrate_18X16);
		}
		else
		{
			word.x_axis = 40;
			word.y_axis = 45;
			LCD_SetPicture(word.x_axis, word.y_axis, LCD_RED, LCD_NONE, &Img_heartrate_32X28);
		}
	}
	n++;
	if(n == 2)
	{
		n = 0;
	}
	
	word.x_axis = 40;
	word.y_axis = 155;
	word.size = LCD_FONT_29_SIZE;//15X29
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	if(value == 0)
	{
		//心率为0
		LCD_SetRectangle(word.x_axis + Font_Number_29.height, 2, 
							word.y_axis, Font_Number_29.width - 1, 
							LCD_WHITE, 0, 0, LCD_FILL_ENABLE);
		
		LCD_SetRectangle(word.x_axis + Font_Number_29.height, 2, 
							word.y_axis + Font_Number_29.width + 2, Font_Number_29.width - 1, 
							LCD_WHITE, 0, 0, LCD_FILL_ENABLE);
	}
	else
	{
		memset(str,0,18);
		sprintf(str,"%d",value);
		LCD_SetNumber(str,&word);
	}

	#ifdef HRT_DEBUG_TEST	
	word.x_axis = 90;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;//15X29
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	extern uint32_t hrt_int_num;
	memset(str,0,30);
    sprintf(str,"%d:%d:%d",OHRL_data.amb,OHRL_data.ppg,hrt_int_num);
	LCD_SetNumber(str,&word);
	#endif
	
	//画心率波形图
	gui_heartrate_wave_paint();

}
/*
勾号与叉号绘制
*/
void gui_tick_cross(void)
{
	SetWord_t word = {0};
	
	
	word.x_axis = 38;
	word.y_axis = LCD_LINE_CNT_MAX - img_step_cal_ysave.width -12;
	LCD_SetPicture(word.x_axis, word.y_axis, LCD_NONE, LCD_NONE, &img_step_cal_ysave);
	

	word.x_axis = 120 - 38 + 120 - img_step_cal_nsave.height;
	word.y_axis = LCD_LINE_CNT_MAX - img_step_cal_nsave.width -12;
	LCD_SetPicture(word.x_axis, word.y_axis, LCD_NONE, LCD_NONE, &img_step_cal_nsave);
}
/*界面显示:稳定心率界面*/
void gui_heartrate_stable_paint(uint8_t hdrValue)
{
	SetWord_t word = {0};
	char str[10];
	uint8_t len;
	//设置背景黑色
	LCD_SetBackgroundColor(LCD_BLACK);
	
	gui_tick_cross();//勾号与叉号绘制
	//稳定心率
/*	word.x_axis = 15;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	LCD_SetString((char *)GuiHomeLockStrs[SetValue.Language][5],&word);*///"稳定心率"
	
/*#if defined WATCH_FREEONE_MANGOFUN
	word.x_axis = 54;
	word.y_axis = 77;
	LCD_SetPicture(word.x_axis, word.y_axis, LCD_NONE, LCD_NONE, &Img_Mangofun_34X36);
#else
	word.x_axis = 60;
	word.y_axis = 77;
	LCD_SetPicture(word.x_axis, word.y_axis, LCD_NONE, LCD_NONE, &Img_heartrate_32X28);
#endif*/
	//稳定心率值
	word.x_axis = 50;
	word.size = LCD_FONT_56_SIZE;
	word.kerning = 1;
	word.forecolor = LCD_CYAN;
	word.bckgrndcolor = LCD_NONE;
	memset(str,0,sizeof(str));
	sprintf(str,"%d", hdrValue);
	 len = strlen(str);
	word.y_axis = LCD_CENTER_JUSTIFIED;
	LCD_SetNumber(str, &word);
     
    word.x_axis = 50 + Font_Number_56.height - 24;
	word.y_axis =120 +(len*Font_Number_56.width - word.kerning)/2 + 8;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_CYAN;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	LCD_SetString("bpm",&word);
	
	word.x_axis = 120 + 4;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	LCD_SetString("是否作为基础",&word);
	
	word.x_axis += 32;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	LCD_SetString("心率保存?",&word);
}

/*界面显示:监测心率时间到释义界面*/
void gui_monitor_hdr_time_explain_paint(void)
{
	LCD_SetBackgroundColor(LCD_BLACK);
	//清屏
	LCD_SetRectangle(0, LCD_LINE_CNT_MAX, 0, LCD_LINE_CNT_MAX, LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
	
	gui_point_direction(IMG_POINT_LEFT,LCD_WHITE);//左箭头
	
	SetWord_t word = {0};
	word.x_axis = 15;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	word.kerning = 0;
	LCD_SetString("温馨提示",&word);
	
	word.x_axis += 40;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	word.kerning = 0;
	LCD_SetString("请佩戴好腕表",&word);
	word.x_axis += 32;
	LCD_SetString("早上取坐姿,清醒平静",&word);
	word.x_axis += 32;
	LCD_SetString("状态的监测值",&word);
	word.x_axis += 32;
	LCD_SetString("才更准确~",&word);
	
	//LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
}
/*测试心率自检显示界面*/
void DisplayPostHdr( uint8_t hdr_value )
{
	SetWord_t word = {0};
	char str[30];
	
	//将背景设置为白色
	LCD_SetBackgroundColor(LCD_WHITE);
	
	word.x_axis = 20;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_WHITE;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString("心率漏光",&word);
	
	memset(str,0,30);
	sprintf(str,"ppg:%d",OHRL_data.ppg);
	word.x_axis = 80;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_WHITE;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString(str,&word);

	word.x_axis = 130;

	if (OHRL_data.ppg < 250)
	{
		word.forecolor = LCD_GREEN;
		LCD_SetString("漏光测试成功",&word);
	}
	else
	{
		word.forecolor = LCD_RED;
		LCD_SetString("漏光测试失败",&word);
	}
	
}

/*测试心率自检显示界面*/
void DisplayPostHdr_SNR( uint8_t hdr_value )
{
	
	SetWord_t word = {0};
	char str[30];
	
	//将背景设置为白色
	LCD_SetBackgroundColor(LCD_WHITE);
	
	word.x_axis = 20;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_WHITE;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString("心率信噪比",&word);
	
	memset(str,0,30);
	sprintf(str,"ppg:%d",OHRL_data.ppg);
	word.x_axis = 65;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_WHITE;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString(str,&word);

	memset(str,0,30);
	sprintf(str,"ppg_maxmin:%d",OHRL_data.ppg_maxmin);
	word.x_axis = 95;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_WHITE;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString(str,&word);

	word.x_axis = 135;

	if ((OHRL_data.ppg >=6000) &&(OHRL_data.ppg<=12000) && (OHRL_data.ppg_maxmin < 8))
	{
		word.forecolor = LCD_GREEN;
		LCD_SetString("信噪比测试成功",&word);
	}
	else
	{
		word.forecolor = LCD_RED;
		LCD_SetString("信噪比测试失败",&word);
	}

}

/*函数说明:心率恢复出厂设置选择界面*/
void DisplayPostSettings(void)
{
	SetWord_t word = {0};

	uint8_t x_axis_start = 15;
	
	//将背景设置为白色
	LCD_SetBackgroundColor(LCD_WHITE);
	
	word.x_axis = x_axis_start;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_WHITE;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString("自检",&word);
	
	//选中色
	LCD_SetRectangle((word.x_axis+8 + 20*(PostIndex+1)),LCD_FONT_16_SIZE,0,LCD_LINE_CNT_MAX, LCD_BLUE, 0, 2, LCD_FILL_ENABLE);
	
	x_axis_start += 28;
	word.x_axis = x_axis_start;
	word.size = LCD_FONT_16_SIZE;
	if(PostIndex == 0)
	{
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_BLUE;
	}
	else
	{
		word.forecolor = LCD_BLACK;
		word.bckgrndcolor = LCD_WHITE;
	}
	LCD_SetString("心率漏光",&word);

	x_axis_start += 20;
	word.x_axis = x_axis_start;		
	if(PostIndex == 1)
	{
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_BLUE;
	}
	else
	{
		word.forecolor = LCD_BLACK;
		word.bckgrndcolor = LCD_WHITE;
	}
	LCD_SetString("心率信噪比",&word);
	
	
	x_axis_start += 20;
	word.x_axis = x_axis_start;		
	if(PostIndex == 2)
	{
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_BLUE;
	}
	else
	{
		word.forecolor = LCD_BLACK;
		word.bckgrndcolor = LCD_WHITE;
	}
	LCD_SetString("恢复出厂设置",&word);
	
	x_axis_start += 20;
	word.x_axis = x_axis_start;				
	if(PostIndex == 3)
	{
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_BLUE;
	}
	else
	{
		word.forecolor = LCD_BLACK;
		word.bckgrndcolor = LCD_WHITE;
	}
	LCD_SetString("蓝牙下载",&word);		


	x_axis_start += 20;
	word.x_axis = x_axis_start;		
	if(PostIndex == 4)
	{
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_BLUE;
	}
	else
	{
		word.forecolor = LCD_BLACK;
		word.bckgrndcolor = LCD_WHITE;
	}
	LCD_SetString("手表信息",&word);			
	
	x_axis_start += 20;
	word.x_axis = x_axis_start;		
	if(PostIndex == 5)
	{
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_BLUE;
	}
	else
	{
		word.forecolor = LCD_BLACK;
		word.bckgrndcolor = LCD_WHITE;
	}
	LCD_SetString("快速放电，后复位手表",&word);
	x_axis_start += 20;
	word.x_axis = x_axis_start;		
	if(PostIndex == 6)
	{
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_BLUE;
	}
	else
	{
		word.forecolor = LCD_BLACK;
		word.bckgrndcolor = LCD_WHITE;
	}
	LCD_SetString("当前实时时间",&word);
	x_axis_start += 20;
	word.x_axis = x_axis_start;		
	if(PostIndex == 7)
	{
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_BLUE;
	}
	else
	{
		word.forecolor = LCD_BLACK;
		word.bckgrndcolor = LCD_WHITE;
	}
	LCD_SetString("信号详情(GPS+BD)",&word);	
}
/*按键处理:平均基础心率界面*/
void gui_heartrate_average_based_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	home_or_minor_switch = MINOR_SCREEN_SWITCH;
	switch( Key_Value )
	{
	
#if defined WATCH_COM_SHORTCUNT_UP 
		case KEY_OK:{
			GUI_HEARTRATE_PRINTF("heartrate_test_screen\r\n");
			ScreenState = DISPLAY_SCREEN_HEARTRATE;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			
			task_hrt_start();//初始化心率
		}break;
		case KEY_UP:
			if(((SetValue.U_CustomMonitor.val & 0x01)== 0) &&(((SetValue.U_CustomMonitor.val>>1) & 0x01)== 0))
				{
				   
					s_switch_screen_count = 0;
					
					switch_home_screen = MONITOR_HOME_SCREEN;
					
					back_monitor_screen = DISPLAY_SCREEN_WEEK_AVERAGE_HDR;
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
               
			if((((SetValue.U_CustomMonitor.val>>3) & 0x01)== 0)
				&&(((SetValue.U_CustomMonitor.val>>4) & 0x01)== 0)&&(((SetValue.U_CustomMonitor.val>>5) & 0x01)== 0))
						{
							switch_home_screen = MONITOR_HOME_SCREEN;
							
							back_monitor_screen = DISPLAY_SCREEN_WEEK_AVERAGE_HDR;
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
#else
		case KEY_OK:{
			ScreenState = DISPLAY_SCREEN_HEARTRATE_HINT;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}break;
		case KEY_UP:
			set_home_index_up();
			ScreenState = Get_Curr_Home_Index();
			msg.cmd = MSG_DISPLAY_SCREEN;
//			msg.value =Get_Curr_Home_Index() ;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		case KEY_DOWN:
			set_home_index_down();
			ScreenState = Get_Curr_Home_Index();
			msg.cmd = MSG_DISPLAY_SCREEN;
//			msg.value =Get_Curr_Home_Index() ;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
#endif
		case KEY_BACK:
			
			switch_home_screen = BACK_HOME_SCREEN;
			ScreenState = DISPLAY_SCREEN_HOME;
		
			back_monitor_screen = DISPLAY_SCREEN_WEEK_AVERAGE_HDR;
			//返回待机界面
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}
/*按键处理:心率测量提示界面*/
void gui_heartrate_hint_measuring_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:{
			ScreenState = DISPLAY_SCREEN_HEARTRATE;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
			task_hrt_start();//初始化心率
		}break;
		case KEY_BACK:

			ScreenState = DISPLAY_SCREEN_WEEK_AVERAGE_HDR;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		break;
		default:
			break;
	}
}
/*按键处理:开始跳动监测界面*/
void gui_heartrate_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{




		case KEY_UP:
			
			if(gui_heartrate_statu_get() != 1)

				{
	                 timer_app_tool_stop(false);
					 task_hrt_close();
					// task_hdr_uninit();
					home_or_minor_switch = MINOR_SCREEN_SWITCH;
				  if(((SetValue.U_CustomMonitor.val & 0x01)== 0) &&(((SetValue.U_CustomMonitor.val>>1) & 0x01)== 0))
					{
					   
						s_switch_screen_count = 0;
						
						switch_home_screen = MONITOR_HOME_SCREEN;
						
						back_monitor_screen = DISPLAY_SCREEN_WEEK_AVERAGE_HDR;
	                    ScreenState = DISPLAY_SCREEN_HOME;
				         //返回待机界面
				         msg.cmd = MSG_DISPLAY_SCREEN;
				         xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				    }
				else
					{
	                  
						
						set_switch_screen_count_evt(Key_Value);
				    }
			

			  }
						
		
			break;
		case KEY_DOWN:
			        //关闭心率
			      if(gui_heartrate_statu_get() != 1)
				{
						timer_app_tool_stop(false);
						task_hrt_close();
						
						//task_hdr_uninit();
			      home_or_minor_switch = MINOR_SCREEN_SWITCH;
				if((((SetValue.U_CustomMonitor.val>>3) & 0x01)== 0)
				&&(((SetValue.U_CustomMonitor.val>>4) & 0x01)== 0)&&(((SetValue.U_CustomMonitor.val>>5) & 0x01)== 0))
						{
							switch_home_screen = MONITOR_HOME_SCREEN;
							
							back_monitor_screen = DISPLAY_SCREEN_WEEK_AVERAGE_HDR;
							ScreenState = DISPLAY_SCREEN_HOME;
							 //返回待机界面
							 msg.cmd = MSG_DISPLAY_SCREEN;
							 xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
						}
					else
						{
							
						
								
								set_switch_screen_count_evt(Key_Value);
						}
				}
			break;
   
     
		case (KEY_DOWN<<1):
//			gui_tool_switch(Key_Value);
			break;
		case KEY_BACK:{
			//关闭心率
			timer_app_tool_stop(false);
			task_hrt_close();
			
			//task_hdr_uninit();
			switch_home_screen = BACK_HOME_SCREEN;
			back_monitor_screen = DISPLAY_SCREEN_WEEK_AVERAGE_HDR;
			if(m_heartrate_status)
			{
				ScreenState = DISPLAY_SCREEN_WEEK_AVERAGE_HDR;
			}
			else
			{
				ScreenState = DISPLAY_SCREEN_HOME;
			}

			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
		}break;
	
		default:
			break;
	}
	
	
}
/*按键处理:稳定心率界面*/
void gui_heartrate_stable_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	switch( Key_Value )
	{
		case KEY_OK:
			//保存稳定心率
			am_hal_rtc_time_get(&RTC_time);
			GUI_HEARTRATE_PRINTF("----DayMainData.Index=%d,RTC_time.ui32Weekday=%d\n",DayMainData.Index,RTC_time.ui32Weekday);
			if(RTC_time.ui32Hour >= 5 && RTC_time.ui32Hour < 12 && (SetValue.IsAllowBaseHDR == 1))
			{//5点到12点才能保存
        DayMainData.BasedHeartrate = stableHdr;
				ScreenState = DISPLAY_SCREEN_WEEK_AVERAGE_HDR;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
			break;
		case KEY_BACK:
			ScreenState = DISPLAY_SCREEN_WEEK_AVERAGE_HDR;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}

/*按键处理:监测心率时间到释义界面*/
void gui_monitor_hdr_time_explain_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	switch( Key_Value )
	{
		case KEY_OK:
			task_hrt_start();//初始化心率
			ScreenState = DISPLAY_SCREEN_HEARTRATE;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		case KEY_BACK:

			ScreenState = DISPLAY_SCREEN_HOME;
			//返回待机界面
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}

void DisplayPostSettings_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	switch( Key_Value )
	{
		case KEY_UP:
		{
			if(PostIndex > 0)
			{
				PostIndex--;
			}
			GUI_HEARTRATE_PRINTF("KEY_UP:PostIndex=%d\n",PostIndex);
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}
			break;
		case KEY_DOWN:
		{
			if(PostIndex < 7)
			{
				PostIndex++;
			}
			GUI_HEARTRATE_PRINTF("KEY_DOWN:PostIndex=%d\n",PostIndex);
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}
			break;
		case KEY_OK:
		{
			GUI_HEARTRATE_PRINTF("KEY_OK:PostIndex=%d\n",PostIndex);
			if(PostIndex == 0)
			{//进入心率自检
				ScreenState = DISPLAY_SCREEN_POST_HDR;
				task_hrt_start();//初始化心率
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
			if(PostIndex == 1)
			{//进入心率自检
				ScreenState = DISPLAY_SCREEN_POST_HDR_SNR;
				task_hrt_start();//初始化心率
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
			else if(PostIndex == 2)
			{//恢复出厂设置
				Factory_Reset();
			}
			else if(PostIndex == 3)
			{//下载蓝牙模拟开关打开
			#ifdef WATCH_COD_BSP 
				//am_hal_gpio_pin_config(BSP_GPIO_BT_SEL,AM_HAL_GPIO_OUTPUT);
				am_hal_gpio_out_bit_clear(BSP_GPIO_BT_SEL);
			#else 
				am_hal_gpio_pin_config(DRV_BAT_CHG_STATE_PIN,AM_HAL_GPIO_OUTPUT);
				am_hal_gpio_out_bit_clear(DRV_BAT_CHG_STATE_PIN);
			#endif 
			}	
			else if(PostIndex == 4)
			{//显示手表软件硬件信息
				ScreenState = DISPLAY_SCREEN_POST_WATCH_INFO;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);				
			}		
			else if(PostIndex == 5)
			{//快速放电
				drv_motor_enable();
			}
			else if(PostIndex == 6)
			{
				SetupTimerB2(1);//开启500ms定时器
				ScreenState = DISPLAY_SCREEN_TIME_CALIBRATION_REALTIME;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
			else if(PostIndex == 7)
			{//GPS+BD测试信号详情界面
				g_Second = 0;
				g_status = 0;
				ScreenState = DISPLAY_SCREEN_POST_GPS;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				CreateGPSTask();
			}
		}
			break;
		case KEY_BACK:
			#ifdef WATCH_COD_BSP 
			//am_hal_gpio_pin_config(BSP_GPIO_BT_SEL,AM_HAL_GPIO_INPUT);
			am_hal_gpio_out_bit_set(BSP_GPIO_BT_SEL);
			#else
			am_hal_gpio_pin_config(DRV_BAT_CHG_STATE_PIN,AM_HAL_GPIO_INPUT);
			#endif
			ScreenState = DISPLAY_SCREEN_HOME;
			//返回待机界面
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}
void DisplayPostHdr_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	switch( Key_Value )
	{
		case KEY_BACK:
			task_hrt_close();//关闭心率
			PostIndex = 0;
			ScreenState = DISPLAY_SCREEN_POST_SETTINGS;
			//返回待机界面
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}

void DisplayPostHdr_SNR_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	switch( Key_Value )
	{
		case KEY_BACK:
			task_hrt_close();//关闭心率
			PostIndex = 1;
			ScreenState = DISPLAY_SCREEN_POST_SETTINGS;
			//返回待机界面
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}


