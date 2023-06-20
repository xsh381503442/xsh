#include "gui_time_calibration_test.h"
#include "drv_lcd.h"
#include "task_display.h"
#include "task_gps.h"
#include "drv_key.h"
#include "algo_time_calibration.h"
#include "bsp_timer.h"
#include "com_data.h"
#include "time_notify.h"
#include "timer_app.h"
#include "com_sport.h"

static uint8_t s_up_down_index = 0;/*精密时间界面按键显示下一页标记*/
#if defined(WATCH_PRECISION_TIME_JUMP_TEST)
static uint32_t s_jump_normal_time_count = 0;/*精密授时时间 计数*/
static uint32_t s_jump_exception_time_count = 0;/*精密授时时间跳变异常 计数*/
#endif
void gui_time_calibration_realtime_test_paint(void)
{
	SetWord_t word = {0};
  char str[30];

	//设置背景
	LCD_SetBackgroundColor(LCD_WHITE);
	am_hal_rtc_time_t RTC_time_Show;
	am_hal_rtc_time_get(&RTC_time_Show);
	memset(str,0,30);
	sprintf(str,"%04d-%02d-%02d",RTC_time_Show.ui32Year+2000,RTC_time_Show.ui32Month,
																				RTC_time_Show.ui32DayOfMonth);
	word.x_axis = 90;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_WHITE;
	word.kerning = 0;
	LCD_SetString(str,&word);
	
	memset(str,0,30);
	sprintf(str,"%d:%02d:%02d.%02d0",RTC_time_Show.ui32Hour,RTC_time_Show.ui32Minute,
																				RTC_time_Show.ui32Second,RTC_time_Show.ui32Hundredths);
	word.x_axis += 36;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_WHITE;
	word.kerning = 0;
	LCD_SetString(str,&word);
	
#if defined(WATCH_GPS_OR_NO_GPS_RESOURCE_SET_TIME)
	word.x_axis = 200;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_WHITE;
	word.kerning = 0;
	switch(get_s_set_time_resource())
	{
		case SET_TIME_RESOURCE_BY_RESET:
			LCD_SetString("来源:重启授时",&word);
			break;
		case SET_TIME_RESOURCE_BY_GPS:
			LCD_SetString("来源:GPS授时",&word);
			break;
		case SET_TIME_RESOURCE_BY_MANUAL:
			LCD_SetString("来源:手动授时",&word);
			break;
		case SET_TIME_RESOURCE_BY_BLE:
			LCD_SetString("来源:蓝牙授时",&word);
			break;
		case SET_TIME_RESOURCE_BY_CALIBRATION:
			LCD_SetString("来源:补偿授时",&word);
			break;
		default:
			break;
	}
#endif
}
/*界面显示:精密授时方案测试状态显示*/
void gui_time_calibration_status_test_paint(void)
{
	SetWord_t word = {0};
  char str[30];
		//清屏
	LCD_SetRectangle(0, LCD_LINE_CNT_MAX, 0, LCD_LINE_CNT_MAX, LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
	//设置背景
	LCD_SetBackgroundColor(LCD_WHITE);
	
	word.x_axis = 15;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_WHITE;
	word.kerning = 0;
	LCD_SetString("精密时间状态",&word);
	
	uint8_t m_timecalibration_status = ReadCalibrationValueAndConfigAdjValue();
	Set_TimeCalibrationStatus(m_timecalibration_status);//设置精密校准状态
	TimeCalibrationStr m_time_calibration = Get_TimeCalibrationStr();//calibration值
	TimeAdjStr m_time_adj = Get_TimeAdjStr();//Adj值
	if(s_up_down_index == 0)
	{
		word.x_axis += 24;
		word.size = LCD_FONT_16_SIZE;
		switch(Get_TimeCalibrationStatus())
		{
			case STATUS_FLASH_NO_ADJ_NO_TCALIBRATION:
				LCD_SetString("NO_ADJ_NO_TCALIBRATION",&word);
				break;
			case STATUS_FLASH_NO_ADJ_HAS_TCALIBRATION:
				LCD_SetString("NO_ADJ_HAS_TCALIBRATION",&word);
				break;
			case STATUS_FLASH_HAS_ADJ_NO_TCALIBRATION:
				LCD_SetString("HAS_ADJ_NO_TCALIBRATION",&word);
				break;
			case STATUS_FLASH_HAS_ADJ_HAS_TCALIBRATION:
				LCD_SetString("校准完成",&word);
				break;
			default:
				break;
		}
		am_hal_rtc_time_t m_T1_gps_time = Get_T1GPSStr();

		word.x_axis += 24;
		memset(str,0,30);
		sprintf(str,"T1 %d-%02d-%02d %d:%02d:%02d.%02d0"
			,m_T1_gps_time.ui32Year,m_T1_gps_time.ui32Month,m_T1_gps_time.ui32DayOfMonth,m_T1_gps_time.ui32Hour
			,m_T1_gps_time.ui32Minute,m_T1_gps_time.ui32Second,m_T1_gps_time.ui32Hundredths);
		LCD_SetString(str,&word);
		
		int16_t xt_value = 0;
		xt_value = AM_BFR(CLKGEN,CALXT,CALXT);//读取晶振补偿寄存器值
		xt_value &= 0x7FF;
		xt_value <<= 5;
		xt_value >>= 5;
		word.x_axis += 24;
		memset(str,0,30);
		sprintf(str,"XT:%d Adj:%d Cal:%d",xt_value,m_time_adj.AdjValue,(int32_t)(m_time_calibration.CalibrationValue));
		LCD_SetString(str,&word);
		if(IsTimeValid(m_time_adj.GpsFirstTime))
		{
			word.x_axis += 24;
			memset(str,0,30);
			sprintf(str,"Adj %d-%02d-%02d",m_time_adj.GpsFirstTime.Year,m_time_adj.GpsFirstTime.Month,
							m_time_adj.GpsFirstTime.Day);
			LCD_SetString(str,&word);
			word.x_axis += 24;
			memset(str,0,30);
			sprintf(str,"%d:%02d:%02d.%02d0",m_time_adj.GpsFirstTime.Hour,m_time_adj.GpsFirstTime.Minute,
							m_time_adj.GpsFirstTime.Second,m_time_adj.GpsFirstTime.Msecond);
			LCD_SetString(str,&word);
		}
		else
		{
			word.x_axis += 24;
			LCD_SetString("No_TimeAdj",&word);
		}

		
		if(IsTimeValid(m_time_calibration.CalibrationSaveTime))
		{
			word.x_axis += 24;
			memset(str,0,30);
			sprintf(str,"cal %d-%02d-%02d",m_time_calibration.CalibrationSaveTime.Year,m_time_calibration.CalibrationSaveTime.Month,
							m_time_calibration.CalibrationSaveTime.Day);
			LCD_SetString(str,&word);
			word.x_axis += 24;
			memset(str,0,30);
			sprintf(str,"%d:%02d:%02d.%02d0",m_time_calibration.CalibrationSaveTime.Hour,m_time_calibration.CalibrationSaveTime.Minute,
							m_time_calibration.CalibrationSaveTime.Second,m_time_calibration.CalibrationSaveTime.Msecond);
			LCD_SetString(str,&word);
		}
		else
		{
			word.x_axis += 24;
			LCD_SetString("No_TimeCalibration",&word);
		}

		word.x_axis += 24;
		TimeCalibrationStatus m_hint_status = Get_TimeCalibrationHintStatus();//获取显示状态
		switch(m_hint_status.status)
		{
			case EXCEPTION_ADJ_VALUE_VALID:
				LCD_SetString("ADJ值有效",&word);
				break;
			case EXCEPTION_ADJ_VALUE_INVALID:
				LCD_SetString("ADJ值无效",&word);
				break;
			case EXCEPTION_CALIBRATION_VALUE_VALID:
				LCD_SetString("Calibration值有效",&word);
				break;
			case EXCEPTION_CALIBRATION_VALUE_INVALID:
				LCD_SetString("Calibration值无效",&word);
				break;
			case EXCEPTION_ADJ_TIMEOUT:
				LCD_SetString("ADJ校准超时",&word);
				break;
			case EXCEPTION_CALIBRATION_TIMEOUT:
				LCD_SetString("Calibration校准超时",&word);
				break;
			case EXCEPTION_CALIBRATION_WRONG:
				LCD_SetString("无效的补偿",&word);
				break;
			case EXCEPTION_MOUNT_GPS_CALIBRATION:
			case EXCEPTION_CALIBRATION_CORRECT:
				LCD_SetString("校准完成时间补偿阶段",&word);
				break;
			default:
				LCD_SetString("请开始校准",&word);
				break;
		}
	}
	else if(s_up_down_index == 1)
	{
		am_hal_rtc_time_t m_T1_gps_time = Get_T1GPSStr();
		word.x_axis += 24;
		word.size = LCD_FONT_16_SIZE;
		memset(str,0,30);
		sprintf(str,"T1 %d-%02d-%02d",m_T1_gps_time.ui32Year,m_T1_gps_time.ui32Month,m_T1_gps_time.ui32DayOfMonth);
		LCD_SetString(str,&word);
		
		word.x_axis += 24;
		memset(str,0,30);
		sprintf(str," %d:%02d:%02d.%02d0",m_T1_gps_time.ui32Hour,m_T1_gps_time.ui32Minute,m_T1_gps_time.ui32Second,m_T1_gps_time.ui32Hundredths);
		LCD_SetString(str,&word);
		
		word.x_axis += 24;
		memset(str,0,30);
		if(IsTimeValid(m_time_adj.AdjSaveTime))
		{
			sprintf(str,"T2 %d-%02d-%02d %d:%02d:%02d.%02d0"
				,m_time_adj.AdjSaveTime.Year,m_time_adj.AdjSaveTime.Month,m_time_adj.AdjSaveTime.Day
				,m_time_adj.AdjSaveTime.Hour,m_time_adj.AdjSaveTime.Minute,m_time_adj.AdjSaveTime.Second,m_time_adj.AdjSaveTime.Msecond);
			LCD_SetString(str,&word);
		}
		else
		{
			LCD_SetString("T2:NONE",&word);
		}
		word.x_axis += 24;
		memset(str,0,30);
		if(IsTimeValid(m_time_adj.GpsFirstTime))
		{
			sprintf(str,"T2s %d-%02d-%02d %d:%02d:%02d.%02d0"
				,m_time_adj.GpsFirstTime.Year,m_time_adj.GpsFirstTime.Month,m_time_adj.GpsFirstTime.Day
				,m_time_adj.GpsFirstTime.Hour,m_time_adj.GpsFirstTime.Minute,m_time_adj.GpsFirstTime.Second,m_time_adj.GpsFirstTime.Msecond);
			LCD_SetString(str,&word);
		}
		else
		{
			LCD_SetString("T2s:NONE",&word);
		}
		
//		int64_t m_T2_T1 = Get_T2_T1_Value(),m_Ts_T2 = Get_Ts_T2_Value();
//		word.x_axis += 24;
//		memset(str,0,30);
//		sprintf(str,"T2_T1=%d,Ts_T2=%d",m_T2_T1,m_Ts_T2);
//		LCD_SetString(str,&word);
		
		word.x_axis += 24;
		memset(str,0,30);

		if(IsTimeValid(m_time_calibration.CalibrationSaveTime))
		{
			sprintf(str,"T3 %d-%02d-%02d %d:%02d:%02d.%02d0"
				,m_time_calibration.CalibrationSaveTime.Year,m_time_calibration.CalibrationSaveTime.Month,m_time_calibration.CalibrationSaveTime.Day
				,m_time_calibration.CalibrationSaveTime.Hour,m_time_calibration.CalibrationSaveTime.Minute,m_time_calibration.CalibrationSaveTime.Second
				,m_time_calibration.CalibrationSaveTime.Msecond);
			LCD_SetString(str,&word);
		}
		else
		{
			LCD_SetString("T3:NONE",&word);
		}
		word.x_axis += 24;
		memset(str,0,30);
		if(IsTimeValid(m_time_calibration.GpsSecondTime))
		{
			sprintf(str,"T3s %d-%02d-%02d %d:%02d:%02d.%02d0"
				,m_time_calibration.GpsSecondTime.Year,m_time_calibration.GpsSecondTime.Month,m_time_calibration.GpsSecondTime.Day
				,m_time_calibration.GpsSecondTime.Hour,m_time_calibration.GpsSecondTime.Minute,m_time_calibration.GpsSecondTime.Second
				,m_time_calibration.GpsSecondTime.Msecond);
			LCD_SetString(str,&word);
		}
		else
		{
			LCD_SetString("T3s:NONE",&word);
		}
#if defined(WATCH_NOT_LEAP_SECOND_SET_TIME)
		word.x_axis += 24;
		if(s_is_open_nav_time_gps == TIME_NAV_GPS_VALID)
		{
			LCD_SetString("闰秒:有效",&word);
		}
		else
		{
			LCD_SetString("闰秒:无效",&word);
		}
#endif
	}
}
#if defined(WATCH_PRECISION_TIME_CALIBRATION)
/*实时时间与GPS时间误差值显示界面*/
void gui_time_realtime_difftime_paint(void)
{
	SetWord_t word = {0};
	am_hal_rtc_time_t m_rtc_time;
  char str[30];
		//清屏
	LCD_SetRectangle(0, LCD_LINE_CNT_MAX, 0, LCD_LINE_CNT_MAX, LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
	//设置背景
	LCD_SetBackgroundColor(LCD_WHITE);
	//获取GPS及rtc时间差 及GPS最新时间
	difftime_gps_str m_difftime = get_difftime_gps_time();
	
	word.x_axis = 15;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_WHITE;
	word.kerning = 0;
	LCD_SetString("实时时间详情",&word);
	
	if(get_open_difftime_status() == STATUS_OPEN_DIFFITME_COMPLETE)
	{//定到位后显示当时rtc时间，否则刷新rtc
		m_rtc_time = Get_TimeRTCStr();
	}
	else
	{
		am_hal_rtc_time_get(&m_rtc_time);
	}
	word.x_axis += 24;
	LCD_SetString("RTC实时时间:",&word);
	
	word.x_axis += 24;
	memset(str,0,30);
	sprintf(str,"%d-%02d-%02d %d:%02d:%02d.%02d0"
		,m_rtc_time.ui32Year,m_rtc_time.ui32Month,m_rtc_time.ui32DayOfMonth,m_rtc_time.ui32Hour
		,m_rtc_time.ui32Minute,m_rtc_time.ui32Second,m_rtc_time.ui32Hundredths);
	LCD_SetString(str,&word);
	
	word.x_axis += 24;
	if(get_open_difftime_status() == STATUS_OPEN_DIFFITME_COMPLETE
		 && m_difftime.m_time.ui32Year != 0xFF && m_difftime.m_time.ui32Month != 0xFF 
	   && m_difftime.m_time.ui32DayOfMonth != 0xFF && m_difftime.m_time.ui32Hour != 0xFF
	   && m_difftime.m_time.ui32Minute != 0xFF && m_difftime.m_time.ui32Second != 0xFF
	   && m_difftime.m_time.ui32Hundredths != 0xFF)
	{
		
		LCD_SetString("GPS时间:",&word);
		word.x_axis += 24;
		memset(str,0,30);
		sprintf(str,"%d-%02d-%02d %d:%02d:%02d.%02d0"
			,m_difftime.m_time.ui32Year,m_difftime.m_time.ui32Month,m_difftime.m_time.ui32DayOfMonth
		  ,m_difftime.m_time.ui32Hour,m_difftime.m_time.ui32Minute,m_difftime.m_time.ui32Second
		  ,m_difftime.m_time.ui32Hundredths);
		LCD_SetString(str,&word);
	}
	else
	{
		LCD_SetString("获取GPS时间中...",&word);
	}

	word.x_axis += 24;
	memset(str,0,30);
	sprintf(str,"RTC比GPS时间少%d",(int32_t)(m_difftime.gps_rtc_value));
	LCD_SetString(str,&word);
#if defined(WATCH_PRECISION_TIME_JUMP_TEST)
	word.x_axis += 24;
	memset(str,0,30);
	sprintf(str,"GPS开启次数:%d",get_precision_time_jump_open_gps_count());
	LCD_SetString(str,&word);
	word.x_axis += 24;
	memset(str,0,30);
	sprintf(str,"时间差值异常次数:%d",get_precision_time_compare_exception_count());
	LCD_SetString(str,&word);
#endif
	word.x_axis += 24;
	memset(str,0,30);
	sprintf(str,"ReRMCCount:%d",get_s_reset_gpsrmc_count());
	LCD_SetString(str,&word);
}
#endif
void gui_time_calibration_realtime_test_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:{
#if defined(WATCH_PRECISION_TIME_CALIBRATION)
			StopTimerB2();
			s_up_down_index = 0;
			uint8_t m_timecalibration_status = ReadCalibrationValueAndConfigAdjValue();//精密时间校准状态
			Set_TimeCalibrationStatus(m_timecalibration_status);
			ScreenState = DISPLAY_SCREEN_TEST_CALIBRATION_VIEW_STATUS;
#else
			StopTimerB2();
			ScreenState = DISPLAY_SCREEN_POST_SETTINGS;
#endif
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}
		break;
		case KEY_LONG_DOWN:
		{
#if defined(WATCH_PRECISION_TIME_CALIBRATION)
			//实时时间与GPS时间误差值界面
			open_difftime_rtc_gps_time();
			ScreenState = DISPLAY_SCREEN_TEST_REAL_DIFF_TIME;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
#endif
		}
			break;
#if defined(WATCH_AUTO_BACK_HOME)
		case KEY_LONG_OK:
		{
			ScreenState = DISPLAY_SCREEN_POST_AUTO_BACK_HOME;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}
		break;
#endif
		case KEY_BACK:{
			StopTimerB2();
			ScreenState = DISPLAY_SCREEN_POST_SETTINGS;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}break;
		default:
			break;
	}
	
}
/*按键处理:精密授时方案测试状态显示*/
void gui_time_calibration_status_test_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_BACK:{
			SetupTimerB2(1);//开启500ms定时器
			ScreenState = DISPLAY_SCREEN_TIME_CALIBRATION_REALTIME;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}break;
		case KEY_UP:
		case KEY_DOWN:
		{
			if(s_up_down_index == 0)
			{
				s_up_down_index = 1;
			}
			else
			{
				s_up_down_index = 0;
			}
			ScreenState = DISPLAY_SCREEN_TEST_CALIBRATION_VIEW_STATUS;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}break;
		case KEY_UP_DOWN:{
			//删除精密时间各数据
			Reset_TimeCalibration();
			uint8_t m_timecalibration_status = ReadCalibrationValueAndConfigAdjValue();
			Set_TimeCalibrationStatus(m_timecalibration_status);//设置精密校准状态
			ScreenState = DISPLAY_SCREEN_TEST_CALIBRATION_VIEW_STATUS;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			timer_notify_motor_start(1000,500,1,false,NOTIFY_MODE_DIRECT_RESPONSE);
			timer_notify_buzzer_start(1000,500,1,false,NOTIFY_MODE_DIRECT_RESPONSE);
		}break;
		default:
			break;
	}
}
/*实时时间与GPS时间误差值按键处理*/
void gui_time_realtime_difftime_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	switch( Key_Value )
	{
		case KEY_BACK:{
#if defined(WATCH_PRECISION_TIME_JUMP_TEST)
			timer_app_precision_time_jump_test_stop(false);
#endif
			close_difftime_rtc_gps_time();
			SetupTimerB2(1);//开启500ms定时器
			ScreenState = DISPLAY_SCREEN_TIME_CALIBRATION_REALTIME;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}break;
		default:
			break;
	}
}
#if defined(WATCH_AUTO_BACK_HOME)
/*界面说明:无数据刷新页面超过10分钟无任何操作（无按键操作）返回待机界面 状态显示*/
void gui_post_auto_back_home_paint(void)
{
	AutoBackScreenStr m_get_autobackhome_str;	
	SetWord_t word = {0};
  char str[50];
	
	//清屏
	LCD_SetRectangle(0, LCD_LINE_CNT_MAX, 0, LCD_LINE_CNT_MAX, LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
	//设置背景
	LCD_SetBackgroundColor(LCD_WHITE);
	m_get_autobackhome_str = get_m_AutoBackScreen();
	
	word.x_axis = 34;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_WHITE;
	word.kerning = 0;
	LCD_SetString("Auto Back Home",&word);
	

	if(m_get_autobackhome_str.status == true)
	{
		word.x_axis = 60;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		memset(str,0,50);
		sprintf(str,"status:%d",m_get_autobackhome_str.status);
		LCD_SetString(str,&word);
		
		word.x_axis += 34;
		memset(str,0,50);
		sprintf(str,"Before screen:%d",m_get_autobackhome_str.screen_before_index);
		LCD_SetString(str,&word);
		
		word.x_axis += 34;
		memset(str,0,50);
		sprintf(str,"Before time:%d-%02d-%02d"
		,m_get_autobackhome_str.before_time.ui32Year,m_get_autobackhome_str.before_time.ui32Month
		,m_get_autobackhome_str.before_time.ui32DayOfMonth);
		LCD_SetString(str,&word);
			word.x_axis += 20;
		memset(str,0,50);
		sprintf(str,"%d:%02d:%02d.%02d0"
		,m_get_autobackhome_str.before_time.ui32Hour,m_get_autobackhome_str.before_time.ui32Minute
		,m_get_autobackhome_str.before_time.ui32Second,m_get_autobackhome_str.before_time.ui32Hundredths);
		LCD_SetString(str,&word);
	}
	else
	{
		word.x_axis = 120;
		LCD_SetString("NO AUTO BACK HOME!",&word);
	}
	word.x_axis = 200;
	LCD_SetString("OK to BACK",&word);
}
/*按键处理:无数据刷新页面超过10分钟无任何操作（无按键操作）返回待机界面*/
void gui_post_auto_back_home_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	switch( Key_Value )
	{
		case KEY_BACK:
		case KEY_OK:{
			SetupTimerB2(1);//开启500ms定时器
			ScreenState = DISPLAY_SCREEN_TIME_CALIBRATION_REALTIME;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}break;
		default:
			break;
	}
}
#endif

