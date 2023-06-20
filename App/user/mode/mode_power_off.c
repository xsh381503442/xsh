#include "mode_config.h"
#include "mode_power_off.h"

#include "drv_lcd.h"
#include "drv_lsm6dsl.h"
#include "drv_alarm.h"

#include "time_notify.h"
#include "timer_app.h"
#include "time_progress.h"

#include "task_display.h"
#include "task_ble.h"
#include "task_hrt.h"
#include "task_gps.h"
#include "task_tool.h"
#include "task_sport.h"
#include "task_step.h"

#include "gui_home.h"
#include "gui_time.h"
#include "gui_tool_gps.h"
#include "gui_heartrate.h"

#include "com_data.h"
#include "com_sport.h"
#include "drv_battery.h"
#include "drv_lis3mdl.h"

#include "algo_time_calibration.h"
#include "bsp.h"
#include "bsp_iom.h"
#include "drv_lcd.h"
#include "drv_motor.h"
#include "drv_hrt.h"
#include "drv_ble.h"


#if DEBUG_ENABLED == 1 && MODE_POWER_OFF_LOG_ENABLED == 1
	#define MODE_POWER_OFF_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define MODE_POWER_OFF_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define MODE_POWER_OFF_WRITESTRING(...)
	#define MODE_POWER_OFF_PRINTF(...)		        
#endif

extern SetValueStr SetValue;
extern _drv_bat_event g_bat_evt;

void mode_button_pof(void)
{

	//关闭相应界面下的功能
	mode_close();
	
	timer_app_uninit();
	ScreenState = DISPLAY_SCREEN_LOGO;

	
	
    //关计步
	DayMainData.CommonSteps = get_filt_stepcount();
	drv_lsm6dsl_reset();
	CloseStepTask();
	//关外设
	drv_lsm6dsl_acc_powerdown();
	drv_lsm6dsl_gyro_powerdown();
	//关地磁
	drv_lis3mdl_standby();
	//关GPS
	//drv_ublox_poweroff();
	//am_hal_gpio_pin_config(BSP_GPIO_UBLOX_PWR,AM_HAL_PIN_OUTPUT);
	//am_hal_gpio_out_bit_clear(BSP_GPIO_UBLOX_PWR);

	//关蜂鸣器
	am_hal_gpio_pin_config(BSP_GPIO_BUZZER_PWR,AM_HAL_GPIO_OUTPUT);
	am_hal_gpio_out_bit_clear(BSP_GPIO_BUZZER_PWR);

	
	//关马达
	drv_motor_disable();
	
#ifdef WATCH_COD_BSP	
	am_hal_gpio_out_bit_set(BSP_GPIO_BUZZ_EN);
	
   //关心率
    task_ohr_timer_stop(NULL);
	drv_hrt_disable();

	//显示黑屏	
	LCD_SetBackgroundColor(LCD_BLACK);
	LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);

	LCD_Poweroff();

#else	
	task_hrt_close();//关闭心率
	//显示黑屏	
	LCD_SetBackgroundColor(LCD_BLACK);
	LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);

	//关闭LCD电源
	LCD_Init();

	LCD_Poweroff();
#endif
	if(SetValue.SwBle >= BLE_DISCONNECT)
	{
		MODE_POWER_OFF_PRINTF("[MODE_POWER_OFF]:SetValue.SwBle >= BLE_DISCONNECT\n");
		dev_ble_system_off();
		SetValue.SwBle = BLE_DISCONNECT;
	}

	
	timer_notify_remind_stop(false);
	timer_notify_display_stop(false);
	timer_notify_motor_stop(false);
	timer_notify_buzzer_stop(false);	
	timer_notify_backlight_stop(false);
	//timer_app_uninit();
	timer_progress_uninit();
	timer_notify_remind_uninit();
	drv_ble_timer_uninit();
	
	mode_power_off_store_set_value();

	bsp_iom_disable(0);
	bsp_iom_disable(1);
	bsp_iom_disable(2);
	bsp_iom_disable(3);
	bsp_iom_disable(4);


}


void mode_power_off_store_set_value(void)
{
	//保存气压和高度数据
	Store_AmbientData();
	
	//获取当前关机时间
	am_hal_rtc_time_get(&RTC_time);
	SetValue.CurrentAddress.PowerOffHour |= 0x80;
	SetValue.PowerOffTime.Year   = RTC_time.ui32Year;
	SetValue.PowerOffTime.Month  = RTC_time.ui32Month;
	SetValue.PowerOffTime.Day    = RTC_time.ui32DayOfMonth;
	SetValue.PowerOffTime.Hour   = RTC_time.ui32Hour;
	SetValue.PowerOffTime.Minute = RTC_time.ui32Minute;
	SetValue.PowerOffTime.Second = RTC_time.ui32Second;
	MODE_POWER_OFF_PRINTF("[MODE_POWER_OFF]:SetValue.CurrentAddress.PowerOffHour=%d\n",SetValue.CurrentAddress.PowerOffHour);
    //关机运动、日常数据
	SetValue.PowerOffMianData.CommonSteps = DayMainData.CommonSteps;
    SetValue.PowerOffMianData.CommonEnergy = DayMainData.CommonEnergy;
    SetValue.PowerOffMianData.CommonDistance = DayMainData.CommonDistance;
    SetValue.PowerOffMianData.SportSteps = DayMainData.SportSteps;
    SetValue.PowerOffMianData.SportEnergy = DayMainData.SportEnergy;
    SetValue.PowerOffMianData.SportDistance = DayMainData.SportDistance;
	SetValue.PowerOffMianData.BasedHeartrate = DayMainData.BasedHeartrate;
	SetValue.PowerOffMianData.VO2MaxMeasuring = DayMainData.VO2MaxMeasuring;
	SetValue.PowerOffMianData.LTBmpMeasuring = DayMainData.LTBmpMeasuring;
	SetValue.PowerOffMianData.LTSpeedMeasuring = DayMainData.LTSpeedMeasuring;
	//关机解锁为了开机无锁	
	SetValue.AutoLockScreenFlag = 0;
	SetValue.PowerPercentRecord = g_bat_evt.level_percent;  //保存关机前的电量百分比
		
	//保存数据
	Store_SetData();

}

void mode_close(void)
{
	ScreenState_t index;
	
	if(ScreenState >= DISPLAY_SCREEN_NOTIFY)
	{
		//如果在通知界面，则为上个界面
		index = ScreenStateSave;
		
		//关闭通知定时器
		timer_notify_display_stop(false);
		timer_notify_motor_stop(false);
		timer_notify_buzzer_stop(false);
	}
	else
	{
		index = ScreenState;
	}
	
	if(index == DISPLAY_SCREEN_NOTIFY_ALARM)
	{
		//如果是闹钟界面
		index = ScreenStateAlarm;
		
		//关闭闹钟定时器
		timer_notify_remind_stop(false);
	}
	
	switch(index)
	{
		case DISPLAY_SCREEN_GPS:		//位置工具
		case DISPLAY_SCREEN_GPS_DETAIL:
		case DISPLAY_SCREEN_PROJECTION_ZONING:
		case DISPLAY_SCREEN_CLOUD_NAVIGATION:
		case DISPLAY_SCREEN_MOTION_TRAIL:
		case DISPLAY_SCREEN_MOTION_TRAIL_DETAIL:
		case DISPLAY_SCREEN_VIEW_MOTION_TRAIL:
		case DISPLAY_SCREEN_NAVIGATION_OPS:
		case DISPLAY_SCREEN_CLOUD_NAVIGATION_TRAIL:
		case DISPLAY_SCREEN_CLOUD_NAVIGATION_DEL:
		case DISPLAY_SCREEN_START_NAVIGATION_READY:
		case DISPLAY_SCREEN_GPS_TRAIL_REMIND:
		case DISPLAY_SCREEN_TRACK_LOAD:
			
		case DISPLAY_SCREEN_COMPASS:	//指北针工具			
		case DISPLAY_SCREEN_COMPASS_CALIBRATION:
			
		case DISPLAY_SCREEN_AMBIENT:	//环境监测工具
		case DISPLAY_SCREEN_AMBIENT_MENU:
		case DISPLAY_SCREEN_AMBIENT_PRESSURE:
		case DISPLAY_SCREEN_AMBIENT_ALTITUDE:
		case DISPLAY_SCREEN_AMBIENT_ALTITUDE_CALIBRATION:
		
		case DISPLAY_SCREEN_STOPWATCH:	//秒表工具
		case DISPLAY_SCREEN_STOPWATCH_MENU:
		case DISPLAY_SCREEN_STOPWATCH_RECORD:

	
	
#if defined WATCH_TOOL_COUNTDOWN
		case DISPLAY_SCREEN_COUNTDOWN_TIME:	//倒计时
		case DISPLAY_SCREEN_COUNTDOWN_EXIT:
#endif
		case DISPLAY_SCREEN_CALENDAR:	//日历天气工具
		case DISPLAY_SCREEN_WEATHER_INFO:
		case DISPLAY_SCREEN_FINDPHONE:	//寻找手机工具
			//挂起工具任务
			tool_task_close();
			break;
		case DISPLAY_SCREEN_BLE_DFU:
		#ifdef COD 
		case DISPLAY_SCREEN_UPDATE_BP:
		#endif
			gui_dfu_timer_stop();
		    break;

		case DISPLAY_SCREEN_UTC_GET:	//授时界面
		case DISPLAY_SCREEN_POST_GPS://GPS+BD
		case DISPLAY_SCREEN_POST_GPS_DETAILS://GPS+BD详情
#if defined WATCH_SIM_SPORT
		case DISPLAY_SCREEN_SPORT://运动选择界面
#endif
			//关闭GPS模块，挂起GPS任务
			CloseGPSTask();
			break;
#if defined(WATCH_PRECISION_TIME_CALIBRATION)
		case DISPLAY_SCREEN_TEST_REAL_DIFF_TIME:
			close_difftime_rtc_gps_time();//自检界面的实时时间与GPS时间功能关闭
			break;
#endif
		case DISPLAY_SCREEN_VO2MAX_MEASURING://最大摄氧量乳酸阈界面
			//关闭定时器
			timer_app_VO2max_Countdown_stop(false);
			timer_app_VO2max_stop(false);
			break;
		case DISPLAY_SCREEN_STEP_WEEK_STEPS:
		case DISPLAY_SCREEN_STEP_WEEK_MILEAGE:
			if(SetValue.IsFirstCalibrationStep == 1)
			{//从未校准过时,停止校准图标闪烁
				timer_app_step_calibration_stop(false);
			}
			break;
		case DISPLAY_SCREEN_CUTDOWN:
			timer_app_countdown_stop(false);
			break;
		case DISPLAY_SCREEN_HEARTRATE:
			timer_app_tool_stop(false);
		
			//关闭心率
			task_hrt_close();
			break;
#if defined WATCH_HAS_SOS
		case DISPLAY_SCREEN_SOS:
		case DISPLAY_SCREEN_SOS_MENU:
			mode_sos_close();
			break;
#endif

#if defined WATCH_COM_SHORTCUNT_UP
		case DISPLAY_SCREEN_CUSTOM_ADD_SUB_MAIN:
		case DISPLAY_SCREEN_CUSTOM_ADD_SUB:
			//增加删除界面
			{
				uint8_t screen = get_switch_screen_count();
				if((get_current_switch_screen(screen) == DISPLAY_SCREEN_WEEK_AVERAGE_HDR)
					&& (gui_heartrate_statu_get() != 1))
				{
					//关闭心率
					timer_app_tool_stop(false);
					task_hrt_close();
				}
			}
			break;
#endif

		default:
			if(IsSportReady(index))
			{
				//运动准备界面
				ReadySportBack();
#if defined WATCH_SIM_SPORT
				CloseGPSTask();
#endif

				//如果是从云迹导航或者运动轨迹开始的运动，需要挂起工具任务
				if((ENTER_NAVIGATION == 1) || (ENTER_MOTION_TRAIL == 1))
				{
					ENTER_NAVIGATION = 0;
					ENTER_MOTION_TRAIL = 0;
					
					//挂起工具任务
					tool_task_close();
				}
			}
			else if(IsSportMode(index))
			{
				//运动界面
				Set_PauseSta(true);
				Set_SaveSta(1);
				CloseSportTask();
			}
			if(is_sport_record_detail_screen(ScreenStateSave))
			{
				//运动详情界面退出 关掉超时检测定时器
				timer_app_sport_record_detail_stop(false);
			}
			break;
	}
	
	//有运动属于稍后继续状态
//	if(1 == IS_SPORT_CONTINUE_LATER)
//	{
//		//运动界面
//		Set_SaveSta(1);
//		CloseSportTask();
//	}
}

