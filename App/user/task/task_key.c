#include "task_config.h"
#include "task_key.h"

#include "bsp.h"
#include "bsp_uart.h"
#include "bsp_timer.h"

#include "drv_lcd.h"
#include "drv_key.h"
#include "drv_light.h"
#include "drv_spiflash.h"
#include "drv_buzzer.h"
#include "drv_spiflash.h"
#include "drv_heartrate.h"
//#include "ndof/drv_ndof.h"

#include "task_hardware.h"
#include "task_uart.h"

#include "mode_power_on.h"
#include "mode_power_off.h"

#include "gui_gps_test.h"
#include "gui_home.h"
#include "gui_tool.h"
#include "gui_tool_gps.h"
#include "gui_tool_compass.h"
#include "gui_tool_calendar.h"
#include "gui_tool_ambient.h"
#include "gui_tool_stopwatch.h"
#include "gui_tool_countdown.h"
#include "gui_hwt.h"
#include "gui_menu.h"
#include "gui_theme.h"
#include "gui_time.h"
#include "gui_heartrate.h"
#include "gui_sports_record.h"
#include "gui_step.h"
#include "gui_heartrate.h"
#include "gui_customized_tools.h"
#include "gui_customized_sports.h"
#include "gui_accessory.h"
#include "gui_system.h"
#include "gui_watch_info.h"
#include "gui_notify_ble.h"
#include "gui_notify_alarm.h"
#include "gui_time_calibration_test.h"
#include "gui_sport.h"
#include "gui_notify_base_hdr.h"
#include "gui_notify_bat.h"
#include "gui_run.h"
#include "gui_swimming.h"
#include "gui_VO2max.h"
#include "gui_notify_base_hdr.h"
#include "gui_trainplan.h"
#include "gui_motionsrecord.h"
#include "gui_notification.h"
#include "gui_calories.h"
#include "gui_lactate_threshold.h"
#include "gui_recovery.h"
#include "gui_notify_app_msg.h"
#include "gui_sport_cycling.h"
#include "gui_sport_indoorrun.h"
#include "gui_sport_walk.h"
#include "gui_sport_marathon.h"
#include "gui_sport_crosscountry_hike.h"
#include "gui_sport_triathlon.h"

#include "algo_time_calibration.h"
#include "time_notify.h"
#include "gui_climbing.h"
#include "gui_crosscountry.h"
#include "gui_notify_train.h"
#include "gui_notify_sport.h"


#include "gui_tool_findphone.h"
#include "gui_notify_warning.h"
#include "gui_post_watch_info.h"
#include "gui_dfu.h"

#include "gui_sport.h"


#if DEBUG_ENABLED == 1 && TASK_KEY_LOG_ENABLED == 1
	#define TASK_KEY_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define TASK_KEY_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define TASK_KEY_WRITESTRING(...)
	#define TASK_KEY_PRINTF(...)		        
#endif
#if defined WATCH_IMU_CALIBRATION 
extern uint8_t start_cal_flag;
extern uint8_t quit_cal_flag;
#endif

SemaphoreHandle_t KeySemaphore = NULL;
static EventGroupHandle_t evt_handle;
extern SWITCH_HOME_SCREEN_T switch_home_screen;
#ifdef WATCH_CHIP_TEST
extern _gui_hwt g_gui_hwt;
#endif
#if defined WATCH_SIM_LIGHTS_ON
uint8_t backlight_always_on = 0;  //背光常亮标志
#endif
static void task_cb(uint32_t evt)
{
	BaseType_t xHigherPriorityTaskWoken, xResult;

	xHigherPriorityTaskWoken = pdFALSE;

	xResult = xEventGroupSetBitsFromISR(evt_handle, (evt),
																			&xHigherPriorityTaskWoken);
	if (xResult == pdPASS)
	{
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
//	else
//	{
//		ERR_HANDLER(ERR_FORBIDDEN);
//	}
}
/*运动详情总结界面按键处理 无按键操作时自动退出到主界面  暂时放这里 后面统一规划后需删除
注:添加一项新运动 需要增加相关运动的运动详情
*/
void KeyEventHandlerScreenSportRecord(uint32_t Key_Value)
{
	if(is_sport_record_detail_screen(ScreenState) == true)
	{
		//运动总结详情界面的按键处理
		if(Key_Value == KEY_BACK)
		{
			//运动详情界面退出 关掉超时检测定时器
			timer_app_sport_record_detail_stop(false);
		}
		else
		{//否则重置定时器
			timer_app_sport_record_detail_stop(false);
			timer_app_sport_record_detail_start();
		}
	}
}
#if defined(WATCH_AUTO_BACK_HOME)
/*无数据刷新页面超过10分钟无任何操作（无按键操作）返回待机界面*/
void KeyEventHandlerScreenAutoBackHome(uint32_t Key_Value)
{
	if(is_auto_back_home(ScreenState) == true)
	{//不在刷新界面的，开启定时器
		TASK_KEY_PRINTF("[task_key]:<---KeyEventHandlerScreenAutoBackHome--key_value=%d,ScreenState=%d>\r\n",Key_Value,ScreenState);
		timer_app_auto_back_home_start();
	}
}
#endif
void CreateKeyTask(void)
{	
	drv_key_init(task_cb);
	evt_handle = xEventGroupCreate();
	xTaskCreate(TaskKey,"Task Key",TaskKey_StackDepth,0,TaskKey_Priority,NULL);	
}

void TaskKey( void *pvParameters )
{
	( void ) pvParameters;	
	uint32_t bitSet;
	
	while(1)
	{
		bitSet = xEventGroupWaitBits(evt_handle, 0xFF, pdTRUE,
												pdFALSE, portMAX_DELAY);
		
		if(ScreenState != DISPLAY_SCREEN_LOGO)
		{
			//按键音
			timer_notify_keytone_start();
		}
		
		switch(bitSet)
		{	
			
			case (KEY_BACK):
            case (KEY_UP):				
			case (KEY_OK):
			case (KEY_DOWN):{
				if(ScreenState != DISPLAY_SCREEN_LOGO)
				{
					if(timer_notify_backlight_is_enable())
					{
						timer_notify_backlight_start(SetValue.backlight_timeout_mssec,false);
					}
				}
			}break;
			
			case (KEY_LIGHT):	{
				if(ScreenState != DISPLAY_SCREEN_LOGO)
				{
					if(timer_notify_backlight_is_enable())
					{
						timer_notify_backlight_stop(false);
					}
					else
					{
						if(backlight_always_on == 1)
						{
							//关闭常亮背光
							drv_light_disable();
							
						}
						else
						{
							timer_notify_backlight_start(SetValue.backlight_timeout_mssec,false);
						}
					}
					
				}			
			}break;
			case (KEY_LIGHT<<1):
				if(ScreenState != DISPLAY_SCREEN_LOGO)
				{
					if(timer_notify_backlight_is_enable())
					{
						//关闭背光超时定时器
						timer_notify_backlight_stop(false);
					}
					
					//常亮背光
					drv_light_enable();
					backlight_always_on = 1;
				}	
				break;
			
			case (KEY_BACK<<1):{
//				if(ScreenState != DISPLAY_SCREEN_LOGO)
//				{
//					timer_notify_buzzer_start(100,100,1,false,0);			
//				}
				if(ScreenState == DISPLAY_SCREEN_LOGO)
				{
					//timer_notify_motor_start(50,50,1,false,NOTIFY_MODE_DIRECT_RESPONSE);	
					//timer_notify_buzzer_start(50,50,1,false,NOTIFY_MODE_DIRECT_RESPONSE);					
					//开机
					mode_button_pon();
				}
				else if (SetValue.AutoLockScreenFlag == 1 && (ScreenState == DISPLAY_SCREEN_HOME || 
                                               ScreenState == DISPLAY_SCREEN_LARGE_LOCK	))
				{
					mode_power_off_store_set_value();
					SetValue.AutoLockScreenFlag = 1;
				}
				else
				{
					timer_notify_motor_start(50,50,1,false,NOTIFY_MODE_DIRECT_RESPONSE);		
					timer_notify_buzzer_start(50,50,1,false,NOTIFY_MODE_DIRECT_RESPONSE);					
					//关机
					mode_button_pof();
				}				
			}break;		
			case (KEY_UNLOCK_BACK_UP):		//短按back+up键解锁
			
			{
				if(SetValue.AutoLockScreenFlag == 1 && (ScreenState == DISPLAY_SCREEN_HOME || 
                                               ScreenState == DISPLAY_SCREEN_LARGE_LOCK	))
				{	//解锁,振动提醒
					timer_notify_motor_start(300,300,1,false,NOTIFY_MODE_SYSTEM_TIME_REACH);
					timer_notify_buzzer_start(300,300,1,false,NOTIFY_MODE_SYSTEM_TIME_REACH);
					DISPLAY_MSG  msg = {0};
					SetValue.AutoLockScreenFlag = 0;
					ScreenState = DISPLAY_SCREEN_LARGE_LOCK;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				}


			}break;									

			case (KEY_UP<<1):{

					//快捷界面长按上键呼出添加移除界面
				//	gui_key_long_btn_evt(KEY_UP<<1);

					if(am_hal_gpio_input_bit_read(KEY_DOWN))
					{
						bitSet = KEY_UP_DOWN;
					}
				
			}break;				

			case (KEY_OK<<1):{
				if(ScreenState != DISPLAY_SCREEN_LOGO)
				{
				}				
			}break;		

			case (KEY_DOWN<<1):{
			
				if(am_hal_gpio_input_bit_read(KEY_UP))
				{
					bitSet = KEY_UP_DOWN;
				}

			}break;
	  }
		//运动详情总结界面按键处理 无按键操作时自动退出到主界面  暂时放这里
		KeyEventHandlerScreenSportRecord(bitSet);
#if defined(WATCH_AUTO_BACK_HOME)
		KeyEventHandlerScreenAutoBackHome(bitSet);
#endif
    KeyEventHandler(bitSet);
	}		
}

void  KeyEventHandler( uint32_t Key_Value )
{
#ifdef WATCH_CHIP_TEST
	if (g_gui_hwt.hardware_test_enable)
	{
		gui_hwt_btn_evt(Key_Value);
	}
#endif
	switch( ScreenState )
	{
	    case DISPLAY_SCREEN_HOME:

				gui_home_btn_evt(Key_Value);
			
				break;
			case DISPLAY_SCREEN_WEEK_AVERAGE_HDR:
				gui_heartrate_average_based_btn_evt(Key_Value);
			break;
			case DISPLAY_SCREEN_STEP:
				gui_step_btn_evt(Key_Value);
				break;
			case DISPLAY_SCREEN_VO2MAX:
				gui_VO2max_value_btn_evt(Key_Value);
				break;
			case DISPLAY_SCREEN_MOTIONS_RECORD:
				gui_motionsrecord_last_btn_evt(Key_Value);
				break;
			case DISPLAY_SCREEN_NOTIFICATION:
				gui_notification_btn_evt(Key_Value);
				break;
			case DISPLAY_SCREEN_CALORIES:
				gui_calories_sports_foundation_btn_evt(Key_Value);
				break;
			case DISPLAY_SCREEN_LACTATE_THRESHOLD:
				gui_lactate_threshold_value_btn_evt(Key_Value);
				break;
			case DISPLAY_SCREEN_TRAINPLAN_MARATHON:
				gui_trainplan_target_btn_evt(Key_Value);
				break;
			case DISPLAY_SCREEN_RECOVERY:
				gui_recovery_time_btn_evt(Key_Value);
				break;
			
			case DISPLAY_SCREEN_CUSTOM_ADD_SUB_MAIN:
				//gui_custom_add_sub_main_btn_evt(Key_Value);
				break;
			case DISPLAY_SCREEN_CUSTOM_ADD_SUB:
				//gui_custom_add_sub_btn_evt(Key_Value);
				break;
			case DISPLAY_SCREEN_LARGE_LOCK:
				gui_home_lock_hint_btn_evt(Key_Value);
				break;
			case DISPLAY_SCREEN_NOTIFY_TIME_IS_MONITOR_HDR:
				gui_monitor_hdr_time_btn_evt(Key_Value);
			break;
			case DISPLAY_SCREEN_TIME_IS_MONITOR_HDR_EXPLAIN:
				gui_monitor_hdr_time_explain_btn_evt(Key_Value);
			break;
		
			case DISPLAY_SCREEN_HEARTRATE_HINT:
				gui_heartrate_hint_measuring_btn_evt(Key_Value);
				break;
			case DISPLAY_SCREEN_HEARTRATE:
				gui_heartrate_btn_evt(Key_Value);
				break;
			case DISPLAY_SCREEN_HEARTRATE_STABLE:
				gui_heartrate_stable_btn_evt(Key_Value);
				break;
			case DISPLAY_SCREEN_STEP_WEEK_STEPS:
				gui_step_week_steps_btn_evt(Key_Value);
				break;
			case DISPLAY_SCREEN_STEP_WEEK_MILEAGE:
				gui_step_week_mileage_btn_evt(Key_Value);
				break;
			case DISPLAY_SCREEN_STEP_CALIBRATION_HINT:
				gui_step_calibration_hint_btn_evt(Key_Value);
			break;
			case DISPLAY_SCREEN_STEP_CALIBRATION_SHOW:
				gui_step_calibration_show_btn_evt(Key_Value);
			break;
			case DISPLAY_SCREEN_STEP_CALIBRATION_CONFIRM_SAVE:
				gui_step_calibration_confirm_save_btn_evt(Key_Value);
			break;
			case DISPLAY_SCREEN_VO2MAX_HINT:
				gui_VO2max_hint_mesuring_btn_evt(Key_Value);
				break;
			case DISPLAY_SCREEN_VO2MAX_MEASURING:
				gui_VO2max_mesuring_btn_evt(Key_Value);
				break;
			case DISPLAY_SCREEN_CALORIES_WEEK_SPORTS:
				gui_calories_week_sports_btn_evt(Key_Value);
			break;
			case DISPLAY_SCREEN_LACTATE_THRESHOLD_HINT:
				gui_lactate_threshold_hint_btn_evt(Key_Value);
			  break;
			case DISPLAY_SCREEN_TIME_DONE:
				gui_time_done_hint_btn_evt(Key_Value);
			break;
			
			
		case DISPLAY_SCREEN_TOOL:		//工具选择界面
			gui_tool_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_GPS:        //定位
			gui_tool_gps_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_GPS_DETAIL:
			gui_gps_detail_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_GPS_TRAIL_REMIND:
			gui_gps_trail_remind_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_PROJECTION_ZONING:
			gui_projection_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_CLOUD_NAVIGATION:
			gui_cloud_navigation_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_MOTION_TRAIL:
			gui_motion_trail_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_MOTION_TRAIL_DETAIL:
			gui_motion_trail_detail_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_VIEW_MOTION_TRAIL:
			gui_view_motion_trail_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_NAVIGATION_OPS:
			gui_navigation_item_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_CLOUD_NAVIGATION_TRAIL:
			gui_navigation_trail_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_CLOUD_NAVIGATION_DEL:
			gui_navigation_del_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_TRACK_LOAD:
			gui_track_load_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_COMPASS:    //指北针
			gui_tool_compass_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_COMPASS_CALIBRATION:
			gui_tool_compass_cali_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_CALENDAR:   //日历天气
			gui_tool_calendar_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_WEATHER_INFO:   //天气信息界面
			gui_tool_calendar_weather_btn_evt(Key_Value);
			break;		
		
		
		case DISPLAY_SCREEN_AMBIENT:    //环境监测
			gui_tool_ambient_btn_evt(Key_Value);
			break;
	
		case DISPLAY_SCREEN_COUNTDOWN:	//倒计时
				gui_tool_countdown_menu_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_COUNTDOWN_SETTING:
			gui_tool_countdown_setting_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_COUNTDOWN_MENU:
			gui_tool_countdown_menu_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_COUNTDOWN_TIME:
			gui_tool_countdown_time_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_COUNTDOWN_EXIT:
			gui_tool_countdown_exit_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_COUNTDOWN_FINISH:
			gui_tool_countdown_finish_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_FINDPHONE:  //寻找手机
			gui_tool_findphone_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_AMBIENT_MENU:
			gui_tool_ambient_menu_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_AMBIENT_PRESSURE:
			gui_tool_ambient_pressure_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_AMBIENT_ALTITUDE:
			gui_tool_ambient_altitude_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_AMBIENT_ALTITUDE_CALIBRATION:
			gui_tool_ambient_altitude_calibration_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_STOPWATCH:  //秒表计时
			gui_tool_stopwatch_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_STOPWATCH_MENU:
			gui_tool_stopwatch_menu_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_STOPWATCH_RECORD:
			gui_tool_stopwatch_record_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_MENU:       
				gui_menu_btn_evt(Key_Value);
				break;
		case DISPLAY_SCREEN_THEME:       
				gui_theme_btn_evt(Key_Value);
				break;
		case DISPLAY_SCREEN_THEME_PREVIEW:       
				gui_theme_preview_btn_evt(Key_Value);
				break;
		case DISPLAY_SCREEN_THEME_APP_MENU:
		case DISPLAY_SCREEN_THEME_APP_CUSTOM:
				gui_theme_app_menu_btn_evt(Key_Value);
				break;
		case DISPLAY_SCREEN_THEME_APP_SUCCESS:       
				gui_theme_app_success_btn_evt(Key_Value);
				break;
		case DISPLAY_SCREEN_THEME_INVALID:       
				gui_theme_invalid_btn_evt(Key_Value);
				break;
		case DISPLAY_SCREEN_TIME:       
				gui_time_btn_evt(Key_Value);
				break;	
		case DISPLAY_SCREEN_UTC_GET:{
				gui_time_utc_get_btn_evt(Key_Value);	
				}break;
				
		case DISPLAY_SCREEN_ALARM:{
				gui_time_alarm_btn_evt(Key_Value);	
		 }break;
		case DISPLAY_SCREEN_ALARM_SET:
				gui_time_alarm_set_btn_evt(Key_Value);	
		 break;	
		case DISPLAY_SCREEN_ALARM_SET_TIME:
				gui_time_alarm_set_time_btn_evt(Key_Value);	
		 break;		
		
		case DISPLAY_SCREEN_ALARM_SET_SOUND_N_VIBERATE:
				gui_time_alarm_set_sound_N_viberate_btn_evt(Key_Value);	
		 break;	
		case DISPLAY_SCREEN_ALARM_SET_REPITION:
				gui_time_alarm_set_repition_btn_evt(Key_Value);	
		 break;	

		case DISPLAY_SCREEN_SPORTS_RECORD:
				gui_sports_record_btn_evt(Key_Value);	
		 break;			
		case DISPLAY_SCREEN_SPORTS_RECORD_DETAIL:
				gui_sports_record_detail_btn_evt(Key_Value);	
		 break;			
		
		case DISPLAY_SCREEN_SPORTS_RECORD_DELETE:
				gui_sports_record_delete_btn_evt(Key_Value);	
		 break;					
		
		case DISPLAY_SCREEN_NOTIFY_SPORTS_RECORD_DELETE:
				gui_sports_record_delete_notify_btn_evt(Key_Value);	
		 break;		
		case DISPLAY_SCREEN_CUSTOMIZED_TOOLS:
				gui_customized_tools_btn_evt(Key_Value);	
		 break;

		 case DISPLAY_SCREEN_CUSTOM_MON_HINT:
		case DISPLAY_SCREEN_CUSTOM_TOOL_HINT:
		case DISPLAY_SCREEN_CUSTOM_SPORT_HINT:
			gui_customized_tools_hint_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_CUSTOMIZED_SPORTS:
				gui_customized_sports_btn_evt(Key_Value);	
		 break;

		case DISPLAY_SCREEN_ACCESSORY:
				gui_accessory_btn_evt(Key_Value);	
		 break;	
		case DISPLAY_SCREEN_ACCESSORY_HEARTRATE:
				gui_accessory_heartrate_btn_evt(Key_Value);	
		 break;		
		case DISPLAY_SCREEN_ACCESSORY_RUNNING:
				gui_accessory_running_btn_evt(Key_Value);	
		 break;	
		case DISPLAY_SCREEN_ACCESSORY_CYCLING_WHEEL:
				gui_accessory_cycling_wheel_btn_evt(Key_Value);	
		 break;			
		case DISPLAY_SCREEN_ACCESSORY_CYCLING_CADENCE:
				gui_accessory_cycling_cadence_btn_evt(Key_Value);	
		 break;	
		case DISPLAY_SCREEN_SYSTEM:
				gui_system_btn_evt(Key_Value);	
		 break;	
	case DISPLAY_SCREEN_SYSTEM_SILENT:
		gui_system_silent_btn_evt(Key_Value);
		break;
		case DISPLAY_SCREEN_SYSTEM_SILENT_TIME:
		gui_system_silent_time_btn_evt(Key_Value);
		break;
		case DISPLAY_SCREEN_SYSTEM_AUTOLIGHT:
				gui_system_autolight_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_SYSTEM_AUTOLIGHT_TIME:
				gui_system_autolight_time_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_SYSTEM_BASE_HEARTRATE:
				gui_system_base_heartrate_btn_evt(Key_Value);	
		 break;			
		case DISPLAY_SCREEN_SYSTEM_LANGUAGE:
				gui_system_language_btn_evt(Key_Value);	
		 break;				
		case DISPLAY_SCREEN_SYSTEM_BACKLIGHT:
				gui_system_backlight_btn_evt(Key_Value);	
		 break;			
		case DISPLAY_SCREEN_SYSTEM_SOUND:
				gui_system_sound_btn_evt(Key_Value);	
		 break;	
		case DISPLAY_SCREEN_WATCH_INFO:
				gui_watch_info_btn_evt(Key_Value);	
		 break;		
		case DISPLAY_SCREEN_NOTIFY_BLE_CONNECT:
				gui_notify_ble_connect_btn_evt(Key_Value);
		 break;		
		case DISPLAY_SCREEN_NOTIFY_BLE_DISCONNECT:
				gui_notify_ble_disconnect_btn_evt(Key_Value);
		 break;			
		case DISPLAY_SCREEN_NOTIFY_BLE_BOND:
				gui_notify_ble_bond_btn_evt(Key_Value);
		 break;				
		case DISPLAY_SCREEN_NOTIFY_BLE_PASSKEY:
				gui_notify_ble_passkey_btn_evt(Key_Value);
		 break;		

		case DISPLAY_SCREEN_NOTIFY_ALARM:
				gui_notify_alarm_btn_evt(Key_Value);
		 break;		
		case DISPLAY_SCREEN_NOTIFY_LOW_BAT:
				gui_monitor_bat_low_btn_evt(Key_Value);
		 break;	
		
		
		
		case DISPLAY_SCREEN_NOTIFY_BAT_CHG:
				gui_monitor_bat_chg_btn_evt(Key_Value);
		 break;	


		 
		case DISPLAY_SCREEN_SPORT://运动选择界面
				gui_sport_btn_evt(Key_Value);
		 break;	

	
			case DISPLAY_SCREEN_RUN:
			gui_sport_run_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_RUN_PAUSE:
			gui_sport_run_pause_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_RUN_CANCEL:
			gui_sport_run_cancel_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_RUN_DETAIL:
			gui_sport_run_detail_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_REMIND_PAUSE:
			gui_sport_pause_hint_btn_evt(Key_Value);
			break;
        


              /*新增*/

		case DISPLAY_SCREEN_SPORT_READY:
			gui_sport_ready_btn_evt(Key_Value);
		    break;
         //预览界面按键操作
           case DISPLAY_SCREEN_SPORT_DATA_DISPLAY_PREVIEW:
			gui_sport_data_display_preview_btn_evt(Key_Value);
		    break;

			case DISPLAY_SCREEN_SPORT_DATA_DISPLAY:
			gui_sport_data_display_btn_evt(Key_Value);
		    break;


		case DISPLAY_SCREEN_SPORT_DATA_DISPLAY_SET:
			gui_sport_data_display_set_btn_evt(Key_Value);
		    break;
		case DISPLAY_SCREEN_SPORT_REMIND_SET:
			gui_sport_remind_set_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_SPORT_REMIND_SET_HAERT:
			gui_sport_remind_set_heart_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_SPORT_REMIND_SET_PACE:
			gui_sport_remind_set_pace_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_SPORT_REMIND_SET_DISTANCE:
			gui_sport_remind_set_distance_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_SPORT_REMIND_SET_AUTOCIRCLE:
			gui_sport_remind_set_autocircle_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_SPORT_REMIND_SET_COALKCAL:
			gui_sport_remind_set_goalkcal_btn_evt( Key_Value);
			break;
		case DISPLAY_SCREEN_SPORT_REMIND_SET_TIME:
			gui_sport_remind_set_time_btn_evt( Key_Value);
			break;
		case DISPLAY_SCREEN_SPORT_REMIND_SET_SPEED:
			gui_sport_remind_set_speed_btn_evt( Key_Value);
			break;
		case DISPLAY_SCREEN_SPORT_REMIND_SET_SWIMPOOL:
			gui_sport_remind_set_swimpool_btn_evt( Key_Value);
			break;
		case DISPLAY_SCREEN_SPORT_REMIND_SET_GOALCIRCLE:
				gui_sport_remind_set_goalcircle_btn_evt( Key_Value);
			break;
		case DISPLAY_SCREEN_SPORT_REMIND_SET_INDOORLENG:
			gui_sport_remind_set_indoorleng_btn_evt( Key_Value);
			break;


      //(登山)高度设置
        case  DISPLAY_SCREEN_SPORT_REMIND_SET_ALTITUDE:
			
			gui_sport_remind_set_altitude_btn_evt( Key_Value);
            break;

			


		 case DISPLAY_SCREEN_SPORT_CANCEL:
                gui_sport_cancel_btn_evt(Key_Value);
		 break;	
        case DISPLAY_SCREEN_SPORT_FEEL:
                gui_sport_feel_btn_evt(Key_Value);
		 break;
		 case DISPLAY_SCREEN_SPORT_HRRECROVY:
            gui_sport_heart_recovery_btn_evt(Key_Value);
            break;
		case DISPLAY_SCREEN_CLIMBING:
			gui_sport_climbing_btn_evt(Key_Value);
		break;
		case DISPLAY_SCREEN_CLIMBING_PAUSE:
			gui_sport_climbing_pause_btn_evt(Key_Value);
		break;
		case DISPLAY_SCREEN_CLIMBING_CANCEL:
			gui_sport_climbing_cancel_btn_evt(Key_Value);
		break;
		case DISPLAY_SCREEN_CLIMBING_DETAIL:
		gui_sport_climbing_detail_btn_evt(Key_Value);
		break;

		case DISPLAY_SCREEN_CYCLING:	//骑行界面
			gui_sport_cycling_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_CYCLING_PAUSE:
			gui_sport_cycling_pause_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_CYCLING_CANCEL:
			gui_sport_cycling_cancel_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_CYCLING_DETAIL:
			gui_sport_cycling_detail_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_INDOORRUN:	//室内跑界面
			gui_sport_indoorrun_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_INDOORRUN_PAUSE:
			gui_sport_indoorrun_pause_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_INDOORRUN_CANCEL:
			gui_sport_indoorrun_cancel_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_INDOORRUN_DETAIL:
			gui_sport_indoorrun_detail_btn_evt(Key_Value);
			break;

		case DISPLAY_SCREEN_WALK:		//健走界面
			gui_sport_walk_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_WALK_PAUSE:
			gui_sport_walk_pause_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_WALK_CANCEL:
			gui_sport_walk_cancel_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_WALK_DETAIL:
			gui_sport_walk_detail_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_MARATHON:		//马拉松界面
			gui_sport_marathon_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_MARATHON_PAUSE:
			gui_sport_marathon_pause_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_MARATHON_CANCEL:
			gui_sport_marathon_cancel_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_MARATHON_DETAIL:
			gui_sport_marathon_detail_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_CROSSCOUNTRY_HIKE:	//徒步越野界面
			gui_sport_hike_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_CROSSCOUNTRY_HIKE_PAUSE:
		
			gui_sport_hike_pause_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_CROSSCOUNTRY_HIKE_CANCEL:
			gui_sport_hike_cancel_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_CROSSCOUNTRY_HIKE_DETAIL:
			gui_sport_hike_detail_btn_evt(Key_Value);
			break;
	
		case DISPLAY_SCREEN_TRIATHLON:	//铁人三项
			gui_sport_triathlon_ready_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_TRIATHLON_SWIMMING:
			gui_sport_triathlon_swimming_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_TRIATHLON_CYCLING:
			gui_sport_triathlon_cycling_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_TRIATHLON_RUNNING:
			gui_sport_triathlon_running_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_TRIATHLON_FIRST_CHANGE:
			gui_sport_triathlon_first_change_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_TRIATHLON_CYCLING_READY:
			gui_sport_triathlon_cycling_ready_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_TRIATHLON_SECOND_CHANGE:
			gui_sport_triathlon_second_change_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_TRIATHLON_RUNNING_READY:
			gui_sport_triathlon_running_ready_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_TRIATHLON_PAUSE:
			gui_sport_triathlon_pause_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_TRIATHLON_CANCEL:
			gui_sport_triathlon_cancel_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_TRIATHLON_DETAIL:
			gui_sport_triathlon_detail_btn_evt(Key_Value);
			break;
		 
         case DISPLAY_SCREEN_NOTIFY_APP_MSG:
                gui_notify_app_msg_btn_evt(Key_Value);
		 break;	
         
         case DISPLAY_SCREEN_APP_MSG_INFO:
                gui_notification_info_btn_evt(Key_Value);
		 break;	
		//训练计划
			case DISPLAY_SCREEN_TRAINPLAN_HOME_RUN_HINT:
				gui_trainplan_home_today_hint_btn_evt(Key_Value);
				break;
			case DISPLAY_SCREEN_TRAINPLAN_SPORT_RUN_HINT:
				gui_trainplan_sport_today_hint_btn_evt(Key_Value);
				break;
			case DISPLAY_SCREEN_TRAINPLAN_IS_COMPLETE:
				gui_trainplan_is_complete_btn_evt(Key_Value);
				break;				
			case DISPLAY_SCREEN_NOTIFY_TRAINING_DAY://训练计划日提醒
				gui_trainplan_today_hint_btn_evt(Key_Value);
				break;
			case DISPLAY_SCREEN_NOTIFY_TRAINING_COMPLETE://训练计划目标达成提醒
				gui_trainplan_target_hint_btn_evt(Key_Value);
				break;
			case DISPLAY_SCREEN_NOTIFY_PACE_TRAINPLAN:
				gui_trainplan_pace_up_down_btn_evt(Key_Value);
				break;
			case DISPLAY_SCREEN_NOTIFY_PACE_INTER_TRAINPLAN:
				gui_trainplan_pace_inter_btn_evt(Key_Value);
				break;
			case DISPLAY_SCREEN_NOTIFY_SW_HINT://训练计划 app打开或者关闭提醒界面
				gui_trainplan_sw_hint_btn_evt(Key_Value);
				break;
//Jason_V1.00:Addin Swimming Interface Trigger Cases in the following section:	
		case DISPLAY_SCREEN_SWIMMING:
			gui_sport_swim_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_SWIM_PAUSE:
			gui_sport_swim_pause_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_SWIM_CANCEL:
			gui_sport_swim_cancel_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_SWIM_DETAIL:
			gui_sport_swim_detail_btn_evt(Key_Value);
			break;
	

	
//Jason_V1.10:Addin CrossCountryRunning Interface Trigger Cases in the following section:
		case DISPLAY_SCREEN_CROSSCOUNTRY:
			gui_sport_crosscountry_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_CROSSCOUNTRY_PAUSE:
		
			gui_sport_crosscountry_pause_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_CROSSCOUNTRY_CANCEL:
			gui_sport_crosscountry_cancel_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_CROSSCOUNTRY_DETAIL:
			gui_sport_crosscountry_detail_btn_evt(Key_Value);
			break;
//Jason_V1.30:Addin NotifyMenu Interface Button Trigger Cases in the following section:
				 case DISPLAY_SCREEN_NOTIFY_HEARTRATE:     
								gui_notify_heartrate_warning_btn_evt(Key_Value);
     break;
				 case DISPLAY_SCREEN_NOTIFY_CALORY:
								gui_notify_train_calory_btn_evt(Key_Value);
     break;
				 case DISPLAY_SCREEN_NOTIFY_STEP:
								gui_notify_train_step_btn_evt(Key_Value);
     break;

				 
				 case DISPLAY_SCREEN_NOTIFY_GOALCIRCLE:     
								gui_notify_sport_circle_btn_evt(Key_Value);
     break;
				 case DISPLAY_SCREEN_NOTIFY_GOALTIME:
								gui_notify_sport_time_btn_evt(Key_Value);
     break;
				 case DISPLAY_SCREEN_NOTIFY_GOALDISTANCE:
								gui_notify_sport_distance_btn_evt(Key_Value);
     break;
	 	case DISPLAY_SCREEN_NOTIFY_CIRCLEDISTANCE:
	 			gui_notify_sport_autocircle_btn_evt(Key_Value);
	 	break;
				 case DISPLAY_SCREEN_NOTIFY_SPEED:
								gui_notify_sport_speed_btn_evt(Key_Value);
     break;
				 case DISPLAY_SCREEN_NOTIFY_PACE:
								gui_notify_sport_pace_btn_evt(Key_Value);
     break;
				 case DISPLAY_SCREEN_NOTIFY_ALTITUDE:
								gui_notify_sport_height_btn_evt(Key_Value);
     break;
				 case DISPLAY_SCREEN_NOTIFY_GOALKCAL:
								gui_notify_sport_calory_btn_evt(Key_Value);
     break;
//Jason's code end in here	
		
#if defined(HARDWARE_TEST)
		case DISPLAY_SCREEN_TEST_STEP:
		case DISPLAY_SCREEN_TEST_COMPASS:
		case DISPLAY_SCREEN_TEST_COMPASS_CALIBRATION:
		case DISPLAY_SCREEN_TEST_GPS:
		case DISPLAY_SCREEN_TEST_HDR:
		case DISPLAY_SCREEN_TEST_FLASH:
		case DISPLAY_SCREEN_TEST_PRESSURE:
		case DISPLAY_SCREEN_TEST_MOTOR:	
			MsgHardware(ScreenState,Key_Value);  //监测界面处理
			break;
		case DISPLAY_SCREEN_TEST_GPS_DETAILS:
			MsgPostGPS(Key_Value);
			break;
#endif
		case DISPLAY_SCREEN_POST_GPS:
			gui_post_gps_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_POST_GPS_DETAILS:
			gui_post_gps_details_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_TIME_CALIBRATION_REALTIME:
		{
			gui_time_calibration_realtime_test_evt(Key_Value);
		}
		break;			
#if defined(DEBUG_FACTORY_HDR_AMBIENT_TEST)
		case DISPLAY_SCREEN_FACTORY_HDR_AMBIENT_TEST:
			DisplayTestHdrAmbient_btn_evt(Key_Value);
			break;
#endif
		
		
		case DISPLAY_SCREEN_NOTIFY_WARNING:       
				gui_notify_warning_btn_evt(Key_Value);
				break;			
				
		case DISPLAY_SCREEN_PC_HWT:
#ifdef WATCH_CHIP_TEST
#else       
				gui_hwt_btn_evt(Key_Value);
#endif
				break;			
		
		
		case DISPLAY_SCREEN_POST_HDR:
			DisplayPostHdr_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_POST_HDR_SNR:
			DisplayPostHdr_SNR_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_POST_SETTINGS:
			DisplayPostSettings_btn_evt(Key_Value);
			break;
		
		
			case DISPLAY_SCREEN_POST_WATCH_INFO:
				gui_post_watch_info_btn_evt(Key_Value);
			break;		
		case DISPLAY_SCREEN_SPORTS_RECORD_TRACK_DETAIL:
			gui_sports_record_detail_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_NOTIFY_VO2MAX_LACTATE_ERALY_END:
			gui_VO2max_lactate_eraly_end_hint_btn_evt(Key_Value);
			break;

		
		case DISPLAY_SCREEN_BLE_DFU:
				gui_dfu_btn_evt(Key_Value);	
		 break;			
		

		case DISPLAY_SCREEN_TRAINPLAN_RUN_SELECT:
			gui_trainplan_run_select_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_TEST_CALIBRATION_VIEW_STATUS:
			gui_time_calibration_status_test_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_END_UNFINISH_SPORT:
			gui_end_unfinished_sport_hint_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_TEST_REAL_DIFF_TIME:
			gui_time_realtime_difftime_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_NOTIFY_FIND_WATCH:
			gui_notify_find_watch_btn_evt(Key_Value);
			break;
		case DISPLAY_SCREEN_POST_AUTO_BACK_HOME:
			gui_post_auto_back_home_btn_evt(Key_Value);
			break;
#ifdef COD
		case DISPLAY_SCREEN_NOTIFY_ON_BIND_DEVICE://
		{
			gui_notify_on_bind_device_btn_evt(Key_Value);
		}
		break;
		case DISPLAY_SCREEN_NOTIFY_BIND_STATUS://
		{
			gui_notify_bind_status_btn_evt(Key_Value);
		}
		break;
		case  DISPLAY_SCREEN_NOTIFY_LOCATION_OK:

			
			gui_notify_locatin_ok_btn_evt(Key_Value);
		 	break;
	case DISPLAY_SCREEN_TRAIN_CAST_END_REMIND:	//训练结束提醒
		gui_training_cast_end_remind_btn_evt(Key_Value);
	break;
	case DISPLAY_SCREEN_TRAIN_CAST_PAUSE_REMIND:	//训练暂停界面
		gui_training_cast_pause_btn_evt(Key_Value);
	break;

	case DISPLAY_SCREEN_TRAIN_CAST_PRESS_REMIND://长按ok键暂停训练
		
		gui_training_cast_remind_btn_evt(Key_Value);
		break;
	case DISPLAY_SCREEN_TRAIN_CAST:	//训练投屏
		gui_training_cast_btn_evt(Key_Value);
	break;

	case DISPLAY_SCREEN_SPORT_CAST_PAUSE_REMIND:
				
		 gui_sport_cast_pause_btn_evt(Key_Value);
		break;
	case DISPLAY_VICE_SCREEN_OUTDOOR_RUN:
	case DISPLAY_VICE_SCREEN_OUTDOOR_WALK:
	case DISPLAY_VICE_SCREEN_RIDE:
	case DISPLAY_VICE_SCREEN_CLIMB:
     gui_vice_sport_btn_evt(Key_Value);

	break;

   case DISPLAY_VICE_SCREEN_SPORT_DATA:

	 gui_vice_sport_detail_btn_evt(Key_Value);
   	break;

#endif

		default:
			break;
	}
}

