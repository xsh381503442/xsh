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

	//�ر���Ӧ�����µĹ���
	mode_close();
	
	timer_app_uninit();
	ScreenState = DISPLAY_SCREEN_LOGO;

	
	
    //�ؼƲ�
	DayMainData.CommonSteps = get_filt_stepcount();
	drv_lsm6dsl_reset();
	CloseStepTask();
	//������
	drv_lsm6dsl_acc_powerdown();
	drv_lsm6dsl_gyro_powerdown();
	//�صش�
	drv_lis3mdl_standby();
	//��GPS
	//drv_ublox_poweroff();
	//am_hal_gpio_pin_config(BSP_GPIO_UBLOX_PWR,AM_HAL_PIN_OUTPUT);
	//am_hal_gpio_out_bit_clear(BSP_GPIO_UBLOX_PWR);

	//�ط�����
	am_hal_gpio_pin_config(BSP_GPIO_BUZZER_PWR,AM_HAL_GPIO_OUTPUT);
	am_hal_gpio_out_bit_clear(BSP_GPIO_BUZZER_PWR);

	
	//�����
	drv_motor_disable();
	
#ifdef WATCH_COD_BSP	
	am_hal_gpio_out_bit_set(BSP_GPIO_BUZZ_EN);
	
   //������
    task_ohr_timer_stop(NULL);
	drv_hrt_disable();

	//��ʾ����	
	LCD_SetBackgroundColor(LCD_BLACK);
	LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);

	LCD_Poweroff();

#else	
	task_hrt_close();//�ر�����
	//��ʾ����	
	LCD_SetBackgroundColor(LCD_BLACK);
	LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);

	//�ر�LCD��Դ
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
	//������ѹ�͸߶�����
	Store_AmbientData();
	
	//��ȡ��ǰ�ػ�ʱ��
	am_hal_rtc_time_get(&RTC_time);
	SetValue.CurrentAddress.PowerOffHour |= 0x80;
	SetValue.PowerOffTime.Year   = RTC_time.ui32Year;
	SetValue.PowerOffTime.Month  = RTC_time.ui32Month;
	SetValue.PowerOffTime.Day    = RTC_time.ui32DayOfMonth;
	SetValue.PowerOffTime.Hour   = RTC_time.ui32Hour;
	SetValue.PowerOffTime.Minute = RTC_time.ui32Minute;
	SetValue.PowerOffTime.Second = RTC_time.ui32Second;
	MODE_POWER_OFF_PRINTF("[MODE_POWER_OFF]:SetValue.CurrentAddress.PowerOffHour=%d\n",SetValue.CurrentAddress.PowerOffHour);
    //�ػ��˶����ճ�����
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
	//�ػ�����Ϊ�˿�������	
	SetValue.AutoLockScreenFlag = 0;
	SetValue.PowerPercentRecord = g_bat_evt.level_percent;  //����ػ�ǰ�ĵ����ٷֱ�
		
	//��������
	Store_SetData();

}

void mode_close(void)
{
	ScreenState_t index;
	
	if(ScreenState >= DISPLAY_SCREEN_NOTIFY)
	{
		//�����֪ͨ���棬��Ϊ�ϸ�����
		index = ScreenStateSave;
		
		//�ر�֪ͨ��ʱ��
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
		//��������ӽ���
		index = ScreenStateAlarm;
		
		//�ر����Ӷ�ʱ��
		timer_notify_remind_stop(false);
	}
	
	switch(index)
	{
		case DISPLAY_SCREEN_GPS:		//λ�ù���
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
			
		case DISPLAY_SCREEN_COMPASS:	//ָ���빤��			
		case DISPLAY_SCREEN_COMPASS_CALIBRATION:
			
		case DISPLAY_SCREEN_AMBIENT:	//������⹤��
		case DISPLAY_SCREEN_AMBIENT_MENU:
		case DISPLAY_SCREEN_AMBIENT_PRESSURE:
		case DISPLAY_SCREEN_AMBIENT_ALTITUDE:
		case DISPLAY_SCREEN_AMBIENT_ALTITUDE_CALIBRATION:
		
		case DISPLAY_SCREEN_STOPWATCH:	//�����
		case DISPLAY_SCREEN_STOPWATCH_MENU:
		case DISPLAY_SCREEN_STOPWATCH_RECORD:

	
	
#if defined WATCH_TOOL_COUNTDOWN
		case DISPLAY_SCREEN_COUNTDOWN_TIME:	//����ʱ
		case DISPLAY_SCREEN_COUNTDOWN_EXIT:
#endif
		case DISPLAY_SCREEN_CALENDAR:	//������������
		case DISPLAY_SCREEN_WEATHER_INFO:
		case DISPLAY_SCREEN_FINDPHONE:	//Ѱ���ֻ�����
			//���𹤾�����
			tool_task_close();
			break;
		case DISPLAY_SCREEN_BLE_DFU:
		#ifdef COD 
		case DISPLAY_SCREEN_UPDATE_BP:
		#endif
			gui_dfu_timer_stop();
		    break;

		case DISPLAY_SCREEN_UTC_GET:	//��ʱ����
		case DISPLAY_SCREEN_POST_GPS://GPS+BD
		case DISPLAY_SCREEN_POST_GPS_DETAILS://GPS+BD����
#if defined WATCH_SIM_SPORT
		case DISPLAY_SCREEN_SPORT://�˶�ѡ�����
#endif
			//�ر�GPSģ�飬����GPS����
			CloseGPSTask();
			break;
#if defined(WATCH_PRECISION_TIME_CALIBRATION)
		case DISPLAY_SCREEN_TEST_REAL_DIFF_TIME:
			close_difftime_rtc_gps_time();//�Լ�����ʵʱʱ����GPSʱ�书�ܹر�
			break;
#endif
		case DISPLAY_SCREEN_VO2MAX_MEASURING://��������������н���
			//�رն�ʱ��
			timer_app_VO2max_Countdown_stop(false);
			timer_app_VO2max_stop(false);
			break;
		case DISPLAY_SCREEN_STEP_WEEK_STEPS:
		case DISPLAY_SCREEN_STEP_WEEK_MILEAGE:
			if(SetValue.IsFirstCalibrationStep == 1)
			{//��δУ׼��ʱ,ֹͣУ׼ͼ����˸
				timer_app_step_calibration_stop(false);
			}
			break;
		case DISPLAY_SCREEN_CUTDOWN:
			timer_app_countdown_stop(false);
			break;
		case DISPLAY_SCREEN_HEARTRATE:
			timer_app_tool_stop(false);
		
			//�ر�����
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
			//����ɾ������
			{
				uint8_t screen = get_switch_screen_count();
				if((get_current_switch_screen(screen) == DISPLAY_SCREEN_WEEK_AVERAGE_HDR)
					&& (gui_heartrate_statu_get() != 1))
				{
					//�ر�����
					timer_app_tool_stop(false);
					task_hrt_close();
				}
			}
			break;
#endif

		default:
			if(IsSportReady(index))
			{
				//�˶�׼������
				ReadySportBack();
#if defined WATCH_SIM_SPORT
				CloseGPSTask();
#endif

				//����Ǵ��Ƽ����������˶��켣��ʼ���˶�����Ҫ���𹤾�����
				if((ENTER_NAVIGATION == 1) || (ENTER_MOTION_TRAIL == 1))
				{
					ENTER_NAVIGATION = 0;
					ENTER_MOTION_TRAIL = 0;
					
					//���𹤾�����
					tool_task_close();
				}
			}
			else if(IsSportMode(index))
			{
				//�˶�����
				Set_PauseSta(true);
				Set_SaveSta(1);
				CloseSportTask();
			}
			if(is_sport_record_detail_screen(ScreenStateSave))
			{
				//�˶���������˳� �ص���ʱ��ⶨʱ��
				timer_app_sport_record_detail_stop(false);
			}
			break;
	}
	
	//���˶������Ժ����״̬
//	if(1 == IS_SPORT_CONTINUE_LATER)
//	{
//		//�˶�����
//		Set_SaveSta(1);
//		CloseSportTask();
//	}
}

