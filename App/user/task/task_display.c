#include "task_config.h"
#include "task_display.h"

#include "drv_lcd.h"
#include "drv_extFlash.h"
#include "drv_heartrate.h"
#include "drv_battery.h"
#include "task_hardware.h"
#include "task_tool.h"

#include "gui_gps_test.h"
#include "gui.h"
#include "gui_dial.h"
#include "gui_home.h"
#include "gui_tool.h"
#include "gui_tool_compass.h"
#include "gui_tool_calendar.h"
#include "gui_tool_ambient.h"
#include "gui_tool_gps.h"
#include "gui_tool_stopwatch.h"
#include "gui_tool_countdown.h"
#include "gui_hwt.h"
#include "gui_menu.h"
#include "gui_theme.h"
#include "gui_time.h"
#include "gui_sports_record.h"
#include "gui_heartrate.h"
#include "gui_sport.h"
#include "gui_notify_alarm.h"
#include "gui_run.h"
#include "gui_swimming.h"
#include "gui_notify_ble.h"
#include "gui_time_calibration_test.h"
#include "gui_step.h"
#include "gui_customized_tools.h"
#include "gui_customized_sports.h"
#include "gui_accessory.h"
#include "gui_system.h"
#include "gui_watch_info.h"
#include "gui_notify_base_hdr.h"
#include "gui_VO2max.h"
#include "gui_notify_low_bat.h"
#include "gui_notify_bat.h"
#include "gui_trainplan.h"
#include "gui_motionsrecord.h"
#include "gui_notification.h"
#include "gui_calories.h"
#include "gui_lactate_threshold.h"
#include "gui_recovery.h"
#include "gui_sport_cycling.h"
#include "gui_sport_indoorrun.h"
#include "gui_sport_walk.h"
#include "gui_sport_marathon.h"
#include "gui_notify_app_msg.h"
#include "gui_climbing.h"
#include "gui_sport_crosscountry_hike.h"
#include "gui_sport_triathlon.h"
#include "gui_crosscountry.h"
#include "gui_notify_train.h"
#include "gui_notify_sport.h"


#include "task_hrt.h"
#include "task_timer.h"
#include "task_gps.h"
#include "task_sport.h"
#include "bsp_timer.h"
#include "bsp_rtc.h"

#include "algo_time_calibration.h"
#include "algo_hdr.h"
#include "algo_sport.h"
#include "algo_trackoptimizing.h"

#include "time_notify.h"
#include "timer_app.h"

#include "drv_lsm6dsl.h"

#include "gui_tool_findphone.h"
#include "gui_notify_warning.h"
#include "gui_post_watch_info.h"
#include "gui_dfu.h"
#include "mode_power_off.h"
#include "task_ble.h"


#if DEBUG_ENABLED == 1 && TASK_DISPLAY_LOG_ENABLED == 1
	#define TASK_DISPLAY_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define TASK_DISPLAY_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define TASK_DISPLAY_WRITESTRING(...)
	#define TASK_DISPLAY_PRINTF(...)		        
#endif

TaskHandle_t	TaskDisplayHandle = NULL;
QueueHandle_t 	DisplayQueue;
ScreenState_t   ScreenState;
ScreenState_t   ScreenStateSave;
//#if defined WATCH_SIM_SPORT
ScreenState_t   ScreenSportSave;	//运动界面暂存
//#endif

volatile ScreenState_t Menu_Index;	//子索引


static float distance_VO2 = 0,distance_lactate = 0,mile_minute_VO2 = 0;
#ifdef COD 
extern cod_ble_sport_cast cod_user_sport_cast;
#endif
extern uint8_t send_pause_flag;
extern uint8_t send_stop_flag;
extern uint8_t send_resume_flag; 
extern uint8_t send_vice_pause_flag;
extern uint8_t send_vice_stop_flag;
extern uint8_t send_vice_resume_flag; 
extern uint8_t gui_sport_index;
extern uint8_t gui_sport_type;
extern float m_pres;	//气压
extern float m_alt;		//高度
extern float m_temp;	//温度

void CreatDisplayTask(void)
{
	
	if( TaskDisplayHandle == NULL )
	{
	  xTaskCreate(TaskDisplay, "Task Display",TaskDisplay_StackDepth, 0, TaskDisplay_Priority, &TaskDisplayHandle); 
		DisplayQueue = xQueueCreate( 5,sizeof( DISPLAY_MSG ));
	}
	
}


void TaskDisplay(void* pvParameter)
{
	static DISPLAY_MSG msg = {MSG_DEFAULT,0};
	float pres,alt,temp;
	GPSMode gps_data = {0};
	
	#ifdef WATXCH_LCD_TRULY
	//打开屏电源
	LCD_Poweron();
	//初始化LCD
	LCD_Init();
	#endif
	
	while(1)
	{
		if (xQueueReceive(DisplayQueue, &msg, portMAX_DELAY) == pdTRUE)
    {
#if defined(WATCH_AUTO_BACK_HOME)
			if(is_auto_back_home(ScreenState) == true)
			{
				//TASK_DISPLAY_PRINTF("[task_display]:<---is_auto_back_home--ScreenState=%d->\r\n",ScreenState);
				timer_app_auto_back_home_start();
			}
#endif
			switch( msg.cmd ) 
			{					
				case MSG_DISPLAY_SCREEN:
					//刷新整屏界面,用于更改界面
					DisplayScreen(msg.value);
					LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
					break;
				case MSG_UPDATE_TIME:
					am_hal_rtc_time_get(&RTC_time);

					if(ScreenState == DISPLAY_SCREEN_HOME)
					{

						Send_Timer_Cmd(CMD_HOME_STEP);
						
						//Send_Timer_Cmd(CMD_AMBIENT_DATA);
						gui_dial_paint();
						LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);

						//更新主界面时间
//						Send_Timer_Cmd(CMD_HOME_STEP);
//						gui_dial_paint();
//						LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
					}
					else if((ScreenState == DISPLAY_SCREEN_THEME_APP_MENU)
						|| (ScreenState == DISPLAY_SCREEN_THEME_APP_CUSTOM))
					{
						//更新预览主题界面时间
						gui_theme_app_menu_paint();
						LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
					}
					else if((ScreenState == DISPLAY_SCREEN_HEARTRATE) || (ScreenState == DISPLAY_SCREEN_GPS) 
								|| (ScreenState == DISPLAY_SCREEN_CALENDAR) || (ScreenState == DISPLAY_SCREEN_STOPWATCH)
								|| (ScreenState == DISPLAY_SCREEN_AMBIENT))
					{
						//更新其他界面的底部栏时间
					}


					if( ( RTC_time.ui32Minute % 3) == 0 )
					{
			
						//每3分钟读取一次高度，每6分钟读取一次气压
						Send_Timer_Cmd(CMD_AMBIENT_DATA);
					}
				
			
					//基础心率提醒时间到
					if((SetValue.SwBasedMonitorHdr == true) && (RTC_time.ui32Minute == SetValue.DefBasedMonitorHdrMinute) &&
										(RTC_time.ui32Hour == SetValue.DefBasedMonitorHdrHour) && (SetValue.IsAllowBaseHDR == 1))
					{
						if((ScreenState != DISPLAY_SCREEN_LOGO)
							&& (ScreenState != DISPLAY_SCREEN_NOTIFY_BAT_CHG)
							&& time_notify_is_ok())
						{
							if(ScreenState < DISPLAY_SCREEN_NOTIFY)
							{
								ScreenStateSave = ScreenState;
							}
							timer_notify_motor_start(1000,500,3,false,NOTIFY_MODE_SYSTEM_TIME_REACH);
							timer_notify_buzzer_start(1000,500,3,false,NOTIFY_MODE_SYSTEM_TIME_REACH);
							
							ScreenState = DISPLAY_SCREEN_NOTIFY_TIME_IS_MONITOR_HDR;
							gui_monitor_hdr_time_paint();
							LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
						}
					}

					if (ScreenState == DISPLAY_SCREEN_NOTIFY_TIME_IS_MONITOR_HDR)
					{

						if ((RTC_time.ui32Second == 0) && (RTC_time.ui32Minute == 0)&& (RTC_time.ui32Hour == 12))
						{
							timer_notify_motor_stop(false);
							timer_notify_buzzer_stop(false);	
			
							DISPLAY_MSG  msg = {0,0};
							mode_close();
							ScreenState = DISPLAY_SCREEN_HOME;
							msg.cmd = MSG_DISPLAY_SCREEN;
							xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	

						}

					}

					
			

			
					//每天0点读取训练计划以提醒用
					if( RTC_time.ui32Minute == 0 ) //整点
					{
						if( RTC_time.ui32Hour == 0 )  //0点
						{
							SetValue.TrainPlanHintFlag = 0;//训练计划处于非运动模式非超提醒时间后不需要提醒
							if(get_trainplan_valid_flag())
							{//有训练计划同步
								Get_TrainPlanDayRecord();
							}
						}
					}
#ifdef COD 
					if( ( RTC_time.ui32Minute % 10) == 0 )
					{
						Store_all_day_steps();
					}
#endif
					DayMainData.CommonEnergy = Get_CommonEnergy();//每分钟计算一次基础卡路里
					if(SetValue.RecoverTime > 0)
					{//恢复时间 恢复
						SetValue.RecoverTime --;
					}
               	    //整点存日常数据 0点清日常数据
					StoreDayMainData();


					//不在充电界面下,每分钟检测一次电量
					drv_bat_adc_enable();
					Send_Timer_Cmd(CMD_MINUTE_DATA);


			

				
					break;
				case MSG_START_TOOL:

					//显示工具选择菜单
					//if(ScreenState >= DISPLAY_SCREEN_GPS && ScreenState <= DISPLAY_SCREEN_FINDPHONE )
					//{
					   if (ScreenState != DISPLAY_SCREEN_COUNTDOWN)
					   {
					   
						tool_task_start(ScreenState);
					    }
						DisplayScreen(msg.value);
						LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
					//}
					break;
				case MSG_UPDATE_COMPASS_VALUE:
					if(ScreenState == DISPLAY_SCREEN_COMPASS)
					{
						//在GPS定位界面先更新经纬度信息
						gui_tool_compass_paint(tool_compass_angle_get());
						LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
					}
					break;
				 case MSG_UPDATE_COMPASS_CALIBRATION:
					if(ScreenState == DISPLAY_SCREEN_COMPASS_CALIBRATION)
					{
						gui_tool_compass_cali_paint(msg.value);
						LCD_DisplaySomeLine(0,240);
					}
					break;
				 case MSG_UPDATE_AMBIENT_VALUE:
					if(ScreenState == DISPLAY_SCREEN_AMBIENT)
					{
						tool_ambient_value_get(&pres, &alt, &temp);
						gui_tool_ambient_value(pres,alt,temp);
						LCD_DisplaySomeLine(68,140);
					}
					break;
				 case MSG_UPDATE_GPS_VALUE:
					if(ScreenState == DISPLAY_SCREEN_GPS)
					{
						gps_value_get(&gps_data);
						gui_tool_gps_paint(&gps_data);
						
					   LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
					}
					break;
				 case MSG_UPDATE_HDR_VALUE:
				 	 TASK_DISPLAY_PRINTF(" in update_hdr_value\r\n");
					 //心率界面更新心率值
					 if(ScreenState == DISPLAY_SCREEN_HEARTRATE)
					 {
						 	am_hal_rtc_time_get(&RTC_time);
						 	//5~12点之间显示稳定心率界面且允许此功能
							if( RTC_time.ui32Hour >= 5 && RTC_time.ui32Hour < 12 && (SetValue.IsAllowBaseHDR == 1))
							{
								 if(g_HeartNum < HEARTBUF_SIZE-1)
								 {//监测时间数小于HEARTBUF_SIZE，显示波峰图，否则显示稳定心率界面
									gui_heartrate_paint(drv_getHdrValue());
									
									LCD_DisplaySomeLine(0,240);
								 }
							   else
								 {
									stableHdr = cal_stable_heartrate();
									task_hrt_close();//关闭心率
									ScreenState = DISPLAY_SCREEN_HEARTRATE_STABLE;
									gui_heartrate_stable_paint(stableHdr);//稳定心率
									LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
									//测完基础心率后提醒
									timer_notify_motor_start(300,300,1,false,NOTIFY_MODE_TOOLS);
			        		       timer_notify_buzzer_start(300,300,1,false,NOTIFY_MODE_TOOLS);	
									
								 }
							}
							else
							{
								gui_heartrate_paint(drv_getHdrValue());
								LCD_DisplaySomeLine(0,240);
							}
							
					 }
					 else if(ScreenState == DISPLAY_SCREEN_POST_HDR)
					 {
							DisplayPostHdr(drv_getHdrValue());
							LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
					 }
					 else if ( ScreenState == DISPLAY_SCREEN_POST_HDR_SNR)
					 {
							DisplayPostHdr_SNR(0);
							LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
					 }
					 
					 break;
				case MSG_UPDATE_STEP_VALUE:
					if(ScreenState == DISPLAY_SCREEN_STEP)
					{
						gui_step_paint();
						LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
					}
					break;				
				case MSG_UPDATE_STOPWATCH_TIME:
					if(ScreenState == DISPLAY_SCREEN_STOPWATCH)
					{
						uint8_t line = gui_tool_stopwatch_time(LCD_WHITE);
						gui_tool_stopwatch_button();
						LCD_DisplaySomeLine(line,Font_Number_56.height);
					}
					break;
				case MSG_UPDATE_STOPWATCH_RECORD:
					if(ScreenState == DISPLAY_SCREEN_STOPWATCH)
					{
						//gui_tool_stopwatch_button();
						//LCD_DisplaySomeLine(62,10);
						
						//gui_tool_stopwatch_record();
						//LCD_DisplaySomeLine(130,88);
                         gui_tool_stopwatch_paint();
						
						LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
					}
					break;
				case MSG_UPDATE_COUNTDOWN:
					{
						if(ScreenState == DISPLAY_SCREEN_CUTDOWN)
						{
							gui_VO2max_countdown_paint(msg.value);
							LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
						}
					}
					break;
				case MSG_UPDATE_VO2MAX_VALUE:
					{
						if(ScreenState == DISPLAY_SCREEN_VO2MAX_MEASURING)
						{
                          
						 // set_gps_flicker(1);
							//待定 目前用距离时间计算配速
							if(msg.value == DISTINGUISH_VO2)
							{
								distance_VO2 = DistanceResult();

								mile_minute_VO2 = SpeedResult();
								gui_VO2max_mesuring_paint(distance_VO2,time_countdown,drv_getHdrValue(),mile_minute_VO2);
								LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
							}
							else if(msg.value == DISTINGUISH_LACTATE)
							{
								distance_lactate = DistanceResult();

								if(SpeedResult() > 0)
								{
									g_VO2maxLactate_SpeedTotal += SpeedResult();
									g_VO2maxLactate_Count ++;
								}
								//gui_VO2max_mesuring_paint(distance_lactate,time_countdown,drv_getHdrValue(),SpeedResult());
								
								gui_lactate_mesuring_paint(distance_lactate,time_countdown,drv_getHdrValue(),SpeedResult());
								LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
							}
						}
					}
					break;
				case MSG_UPDATE_STEP_CALIBRETION:
					{
						if(ScreenState == DISPLAY_SCREEN_STEP_CALIBRATION_SHOW)
						{
							calibration_step = Step_VO2maxOrCalibration;
							calibration_mile = DistanceResult();//m
							if(calibration_step != 0)
							{
								stepStride = DistanceResult()/calibration_step;//cm
							}
							else
							{
								stepStride = 0;
							}
								
							gui_step_calibration_show_paint(calibration_mile,calibration_step,stepStride);
							LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
						}
					}
					break;
			/*	case MSG_UPDATE_VO2MAX_LACTATE_THRESHOLD:
				{//仅仅更新一次 最大摄氧量或乳酸阈测量界面
					Countdown_time_t time_countdown_t;
					get_countdown_time(msg.value,&time_countdown_t);
					gui_VO2max_mesuring_paint(0,time_countdown_t,0,0);
				}
				break;*/
				case MSG_UPDATE_PACE_TRAINPLAN_SPORT:
				{
					gui_trainplan_pace_up_down_paint(msg.value);
					LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
				}
					break;
				case MSG_UPDATE_INTERVAL_TRAINPLAN_SPORT:
				{
					gui_trainplan_pace_inter_paint(msg.value);
					LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
				}
					break;
				case MSG_UPDATE_TRAINPLAN_SW_HINT:
				{//暂时未使用
					if(ScreenState == DISPLAY_SCREEN_NOTIFY_SW_HINT)
					{
						gui_trainplan_sw_hint_paint(msg.value);
						LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
					}
				}
					break;
#if defined(HARDWARE_TEST)
         case MSG_UPDATE_TEST_STEP:
					if(ScreenState == DISPLAY_SCREEN_TEST_STEP)
					{
						//更新自检下的STEP
						DisplayTestSTEP();
						LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
					}
          break;
         case MSG_UPDATE_TEST_COMPASS:
					if(ScreenState == DISPLAY_SCREEN_TEST_COMPASS)
					{
						//更新自检下的COMPASS
						DisplayTestCOMPASS(msg.value);
						LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
					}
          break;
				case MSG_UPDATE_TEST_GPS:
					if(ScreenState == DISPLAY_SCREEN_TEST_GPS)
					{
						//更新自检下的GPS
						gps_value_get(&gps_data);
						DisplayTestGPS(&gps_data);
						LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
					}
					else if(ScreenState == DISPLAY_SCREEN_TEST_GPS_DETAILS)
					{
						gps_value_get(&gps_data);
						DisplayTestGPSDetail(&gps_data);
						LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
					}
          break;
				case MSG_UPDATE_TEST_HEARTRATE:
					if( ScreenState == DISPLAY_SCREEN_TEST_HDR )
					{
						//更新自检下的心率
						DisplayTestHdr(msg.value);//获取心率值 版本
						LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
					}
					break;
				case MSG_UPDATE_TEST_FLASH:
					if(ScreenState == DISPLAY_SCREEN_TEST_FLASH)
					{
						//更新自检下的FLASH
						if( msg.value ==  0x176001)//SPIFLASH_ID
						{
							DisplayTestFlash(true);
						}
						else
						{
							DisplayTestFlash(false);
						}
						LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
					}
					break;
				case MSG_UPDATE_TEST_PRESSURE:
					if(ScreenState == DISPLAY_SCREEN_TEST_PRESSURE)
					{
						//更新自检下的气压
						DisplayTestPressure();
						LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
					}
          break;
				case MSG_UPDATE_TEST_MOTOR:
					if(ScreenState == DISPLAY_SCREEN_TEST_MOTOR)
					{
						//更新自检下的马达
						DisplayTestMotor();
						LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
					}
           break;
			  case MSG_UPDATE_TEST_COMPASS_CALIBRATION:
					if(ScreenState == DISPLAY_SCREEN_TEST_COMPASS_CALIBRATION)
					{
						//更新自检下的COMPASS校准
						DisplayTestCompassCalibration();
						LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
					}
					break;
#endif	
				case MSG_UPDATE_ICON_CALIBRTIONSTEP:
				{//图标更新
					if(ScreenState == DISPLAY_SCREEN_STEP_WEEK_STEPS)
					{
						gui_step_week_steps_paint();
						LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
					}
					else if(ScreenState == DISPLAY_SCREEN_STEP_WEEK_MILEAGE)
					{
						gui_step_week_mileage_paint();
						LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
					}
				}
					break;
				case MSG_UPDATE_TIME_CALIBRATION:
					if(ScreenState == DISPLAY_SCREEN_TIME_CALIBRATION_REALTIME)
					{
						gui_time_calibration_realtime_test_paint();
						LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
					}
				break;
				default:
					break;
			}
		}
	}
}

void DisplayScreen(uint32_t index)
{
	float pres,alt,temp;
	GPSMode gps_data = {0};
	
	switch(ScreenState)
	{
		case DISPLAY_SCREEN_POST_GPS://GPS测试
			//g_Second = 0;
			gps_value_get(&gps_data);
			DisplayTestGPS(&gps_data);
			break;
		case DISPLAY_SCREEN_POST_GPS_DETAILS:
			//g_Second = 0;
			gps_value_get(&gps_data);
			DisplayTestGPSDetail(&gps_data);
			break;
		case DISPLAY_SCREEN_HOME:
			Send_Timer_Cmd(CMD_HOME_STEP);
			drv_ms5837_data_get(&m_pres,&m_alt,&m_temp);
			gui_dial_paint();

			break;
		case DISPLAY_SCREEN_WEEK_AVERAGE_HDR:
			gui_heartrate_average_based_paint();
			break;
		case DISPLAY_SCREEN_STEP:
			gui_step_paint();
			break;
		case DISPLAY_SCREEN_VO2MAX:
			gui_VO2max_value_paint(DayMainData.VO2MaxMeasuring);//最大摄氧量
			break;
		case DISPLAY_SCREEN_MOTIONS_RECORD:
			gui_motionsrecord_last_paint();
			break;
		case DISPLAY_SCREEN_NOTIFICATION:  
             gui_notification_paint();
           
			break;
		case DISPLAY_SCREEN_CALORIES:
			gui_calories_sports_foundation_paint();
			break;
		case DISPLAY_SCREEN_LACTATE_THRESHOLD:
			gui_lactate_threshold_value_paint(DayMainData.LTBmpMeasuring,DayMainData.LTSpeedMeasuring);
			break;
		case DISPLAY_SCREEN_TRAINPLAN_MARATHON:
			gui_trainplan_target_paint();
			break;
		case DISPLAY_SCREEN_RECOVERY:
			gui_recovery_time_paint();
			break;
		
		case DISPLAY_SCREEN_HEARTRATE_STABLE:
			gui_heartrate_stable_paint(stableHdr);//稳定心率
			break;
		case DISPLAY_SCREEN_CUSTOM_ADD_SUB_MAIN:
			//gui_custom_add_sub_main_paint();
			break;
		case DISPLAY_SCREEN_CUSTOM_ADD_SUB:
			//gui_custom_add_sub_paint();
			break;
		case DISPLAY_SCREEN_LARGE_LOCK:
			gui_home_lock_hint_paint();
			if(SetValue.AutoLockScreenFlag != 0)
			{
				//锁屏界面显示3S
				timer_app_UNLOCK_HINT_start();
			}
			else
			{
				//解锁界面显示1S
				ScreenStateSave = DISPLAY_SCREEN_HOME;
				timer_notify_display_start(1200,1,false);
			}
			break;
		case DISPLAY_SCREEN_NOTIFY_TIME_IS_MONITOR_HDR:
			gui_monitor_hdr_time_paint();
			break;
		case DISPLAY_SCREEN_TIME_IS_MONITOR_HDR_EXPLAIN:
			gui_monitor_hdr_time_explain_paint();
			break;
		
		case DISPLAY_SCREEN_HEARTRATE_HINT:
			gui_heartrate_hint_measuring_paint();
			break;
    case DISPLAY_SCREEN_HEARTRATE:
			gui_heartrate_paint(drv_getHdrValue());
			break;
		case DISPLAY_SCREEN_STEP_WEEK_STEPS:
			gui_step_week_steps_paint();
			break;
		case DISPLAY_SCREEN_STEP_WEEK_MILEAGE:
			gui_step_week_mileage_paint();
			break;
		case DISPLAY_SCREEN_STEP_CALIBRATION_HINT:
			gui_step_calibration_hint_paint();
			break;
		case DISPLAY_SCREEN_STEP_CALIBRATION_SHOW:
			gui_step_calibration_show_paint(0,0,0);
			break;
		case DISPLAY_SCREEN_STEP_CALIBRATION_CONFIRM_SAVE:
		{
			gui_step_calibration_confirm_save_paint(calibration_mile,calibration_step,stepStride);
		}
			break;
		case DISPLAY_SCREEN_VO2MAX_HINT:
			gui_VO2max_hint_mesuring_paint();
			break;
		case DISPLAY_SCREEN_CUTDOWN:
			gui_VO2max_countdown_paint(3);
			break;
		
		case DISPLAY_SCREEN_CALORIES_WEEK_SPORTS:
			gui_calories_week_sports_paint();
			break;
		case DISPLAY_SCREEN_LACTATE_THRESHOLD_HINT:
			gui_lactate_threshold_hint_paint();
			break;
		case DISPLAY_SCREEN_TIME_DONE:
			gui_time_done_hint_paint();
			break;
		case DISPLAY_SCREEN_TOOL:
			gui_tool_select_paint(tool_menu_index_record());
			break;
		case DISPLAY_SCREEN_GPS:
			
			gps_value_get(&gps_data);
			gui_tool_gps_paint(&gps_data);
			break;
		case DISPLAY_SCREEN_GPS_DETAIL:
			gui_gps_detail_paint();
			break;
		case DISPLAY_SCREEN_GPS_TRAIL_REMIND:
			gui_gps_trail_remind_paint();
			break;
		case DISPLAY_SCREEN_PROJECTION_ZONING:
			gui_projection_paint();
			break;
		case DISPLAY_SCREEN_CLOUD_NAVIGATION:
			gui_cloud_navigation_paint();
			break;
		case DISPLAY_SCREEN_MOTION_TRAIL:
			gui_motion_trail_paint();
			break;
		case DISPLAY_SCREEN_MOTION_TRAIL_DETAIL:
			gui_motion_trail_detail_paint();
			break;
		case DISPLAY_SCREEN_VIEW_MOTION_TRAIL:
			gui_view_motion_trail_paint();
			break;
		case DISPLAY_SCREEN_NAVIGATION_OPS:
			gui_navigation_item_paint();
			break;
		case DISPLAY_SCREEN_CLOUD_NAVIGATION_TRAIL:
			gui_navigation_trail_paint();
			break;
		case DISPLAY_SCREEN_CLOUD_NAVIGATION_DEL:
			gui_navigation_del_paint();
			break;
		case DISPLAY_SCREEN_TRACK_LOAD:
			{
				gui_track__trail_load_paint();
			}
			break;
		case DISPLAY_SCREEN_COMPASS:
			gui_tool_compass_paint(tool_compass_angle_get());
			break;
		case DISPLAY_SCREEN_COMPASS_CALIBRATION:
			gui_tool_compass_cali_paint(0);
			break;
		case DISPLAY_SCREEN_CALENDAR:
			gui_tool_calendar_paint();
			break;
		case DISPLAY_SCREEN_WEATHER_INFO:  
			gui_tool_calendar_weather_paint();
			break;				
		case DISPLAY_SCREEN_AMBIENT:
			tool_ambient_value_get(&pres, &alt, &temp);
			gui_tool_ambient_paint(pres,alt,temp);
			break;
		
		case DISPLAY_SCREEN_COUNTDOWN:	//倒计时
		gui_tool_countdown_menu_paint();
			break;
		case DISPLAY_SCREEN_COUNTDOWN_SETTING:
			gui_tool_countdown_setting_paint();
			break;
		case DISPLAY_SCREEN_COUNTDOWN_MENU:
			gui_tool_countdown_menu_paint();
			break;
		case DISPLAY_SCREEN_COUNTDOWN_TIME:
			gui_tool_countdown_time_paint();
			break;
		case DISPLAY_SCREEN_COUNTDOWN_EXIT:
			gui_tool_countdown_exit_paint();
			break;
		case DISPLAY_SCREEN_COUNTDOWN_FINISH:
			gui_tool_countdown_finish_paint();
			break;
		case DISPLAY_SCREEN_FINDPHONE:
			gui_tool_findphone_paint();
			break;

		case DISPLAY_SCREEN_AMBIENT_MENU:
			gui_tool_ambient_menu_paint();
			break;
		case DISPLAY_SCREEN_AMBIENT_PRESSURE:
			tool_ambient_value_get(&pres, &alt, &temp);
			gui_tool_ambient_pressure_paint(pres);
			break;
		case DISPLAY_SCREEN_AMBIENT_ALTITUDE:
			tool_ambient_value_get(&pres, &alt, &temp);
			gui_tool_ambient_altitude_paint(alt);
			break;
		case DISPLAY_SCREEN_AMBIENT_ALTITUDE_CALIBRATION:
			gui_tool_ambient_altitude_calibration_paint();
			break;
		case DISPLAY_SCREEN_STOPWATCH://秒表
			gui_tool_stopwatch_paint();
			break;
		case DISPLAY_SCREEN_STOPWATCH_MENU:
			gui_tool_stopwatch_menu_paint();
			break;
		case DISPLAY_SCREEN_STOPWATCH_RECORD:
			gui_tool_stopwatch_record_paint();
			break;
		case DISPLAY_SCREEN_MENU:
			gui_menu_paint();
			break;
		case DISPLAY_SCREEN_THEME:       
			gui_theme_paint();
			break;
		
		case DISPLAY_SCREEN_THEME_PREVIEW:       
			gui_theme_preview_paint();
			break;
		case DISPLAY_SCREEN_THEME_APP_MENU:
		case DISPLAY_SCREEN_THEME_APP_CUSTOM:
			gui_theme_app_menu_paint();
			break;
		case DISPLAY_SCREEN_THEME_APP_SUCCESS:       
			gui_theme_app_success_paint();
			break;
		case DISPLAY_SCREEN_THEME_INVALID:
			gui_theme_invalid_paint();
			break;
		case DISPLAY_SCREEN_TIME:       
			gui_time_paint();
			break;
		case DISPLAY_SCREEN_UTC_GET:{
			
				gui_time_utc_get_paint();	
		 }break;		
			 
		case DISPLAY_SCREEN_ALARM:{
				gui_time_alarm_paint();	
		 }break;				 
		case DISPLAY_SCREEN_ALARM_SET:
				gui_time_alarm_set_paint();	
		 break;			 
		
		case DISPLAY_SCREEN_ALARM_SET_TIME:
				gui_time_alarm_set_time_paint();	
		 break;		
		
		case DISPLAY_SCREEN_ALARM_SET_SOUND_N_VIBERATE:
				gui_time_alarm_set_sound_N_viberate_paint();	
		 break;		
		case DISPLAY_SCREEN_ALARM_SET_REPITION:
				gui_time_alarm_set_repition_paint();	
		 break;				
		case DISPLAY_SCREEN_SPORTS_RECORD:
				gui_sports_record_paint();		
		 break;		
		
		case DISPLAY_SCREEN_SPORTS_RECORD_DETAIL:
				gui_sports_record_detail_paint();	
		 break;				
		case DISPLAY_SCREEN_SPORTS_RECORD_DELETE:
				gui_sports_record_delete_paint();	
		 break;				
		case DISPLAY_SCREEN_NOTIFY_SPORTS_RECORD_DELETE:
				gui_sports_record_delete_notify_paint();	
		 break;

		case DISPLAY_SCREEN_CUSTOMIZED_TOOLS:
				gui_customized_tools_paint();	
		 break;	

		case DISPLAY_SCREEN_CUSTOM_MON_HINT:
		case DISPLAY_SCREEN_CUSTOM_TOOL_HINT:
		case DISPLAY_SCREEN_CUSTOM_SPORT_HINT:
            gui_customized_tools_hint_paint();
		
		break;
#ifdef COD 
		case DISPLAY_VICE_SCREEN_SPORT_HINT:
           	 gui_vice_screen_hint_paint();
			break;

	case DISPLAY_VICE_SCREEN_OUTDOOR_RUN:
	case DISPLAY_VICE_SCREEN_OUTDOOR_WALK:
	case DISPLAY_VICE_SCREEN_RIDE:
    case DISPLAY_VICE_SCREEN_CLIMB:
     	
		   gui_vice_screen_data_paint(VICE_SCREEN_DATA_SHOW);
	
		break;
   
  case DISPLAY_VICE_SCREEN_SPORT_DATA://运动投屏结束数据详情界面

	gui_vice_sport_detail_paint(cod_user_sport_cast.sport_type);


	break;
#endif
		
		case DISPLAY_SCREEN_CUSTOMIZED_SPORTS:
				gui_customized_sports_paint();	
		 break;

		case DISPLAY_SCREEN_ACCESSORY:
				gui_accessory_paint();	
		 break;	

		case DISPLAY_SCREEN_ACCESSORY_HEARTRATE:
				gui_accessory_heartrate_paint();	
		 break;			
		case DISPLAY_SCREEN_ACCESSORY_RUNNING:
				gui_accessory_running_paint();	
		 break;	
		case DISPLAY_SCREEN_ACCESSORY_CYCLING_WHEEL:
				gui_accessory_cycling_wheel_paint();	
		 break;			
		case DISPLAY_SCREEN_ACCESSORY_CYCLING_CADENCE:
				gui_accessory_cycling_cadence_paint();	
		 break;			
		
		
		
		case DISPLAY_SCREEN_SYSTEM:
				gui_system_paint();	
		 break;	
		case DISPLAY_SCREEN_SYSTEM_SILENT:
			gui_system_silent_paint();
			break;
		case DISPLAY_SCREEN_SYSTEM_SILENT_TIME:
			gui_system_silent_time_paint();
			break;
		case DISPLAY_SCREEN_SYSTEM_AUTOLIGHT:
			gui_system_autolight_paint();
			break;
		case DISPLAY_SCREEN_SYSTEM_AUTOLIGHT_TIME:
			gui_system_autolight_time_paint();
			break;

		case DISPLAY_SCREEN_SYSTEM_BASE_HEARTRATE:
				gui_system_base_heartrate_paint();
		 break;				
		case DISPLAY_SCREEN_SYSTEM_LANGUAGE:
				gui_system_language_paint();	
		 break;
		case DISPLAY_SCREEN_SYSTEM_BACKLIGHT:
				gui_system_backlight_paint();
		 break;			
		case DISPLAY_SCREEN_SYSTEM_SOUND:
				gui_system_sound_paint();	
		 break;		
		case DISPLAY_SCREEN_WATCH_INFO:
				gui_watch_info_paint();	
		 break;		
		case DISPLAY_SCREEN_NOTIFY_BLE_CONNECT:
				gui_notify_ble_connect_paint();	
		 break;		
		case DISPLAY_SCREEN_NOTIFY_BLE_DISCONNECT:
				gui_notify_ble_disconnect_paint();
		 break;			
		case DISPLAY_SCREEN_NOTIFY_BLE_BOND:
				gui_notify_ble_bond_paint();
		 break;			
		case DISPLAY_SCREEN_NOTIFY_BLE_PASSKEY:
				gui_notify_ble_passkey_paint();
		 break;	

		case DISPLAY_SCREEN_NOTIFY_ALARM:
			//振动马达和蜂鸣器
			timer_notify_remind_motor_restart();
				gui_notify_alarm_paint();
		 break;	
		case DISPLAY_SCREEN_NOTIFY_LOW_BAT:
				gui_monitor_bat_low_paint();
		 break;
		case DISPLAY_SCREEN_NOTIFY_BAT_CHG:
				gui_monitor_bat_chg_paint();
		 break;		
        case DISPLAY_SCREEN_NOTIFY_APP_MSG:
            gui_notify_app_msg_paint();
        break;			
        case DISPLAY_SCREEN_APP_MSG_INFO:
            gui_notification_info_paint();
        break;
		case DISPLAY_SCREEN_PC_HWT:
			gui_hwt_paint();
			break;					
		case DISPLAY_SCREEN_TIME_CALIBRATION_REALTIME:
			gui_time_calibration_realtime_test_paint();
			break;
		case DISPLAY_SCREEN_SPORT:
			gui_sport_select_paint(sports_menu_index_record());
			break;	
	

    case DISPLAY_SCREEN_RUN:
		gui_sport_run_paint();
		break;
	case DISPLAY_SCREEN_RUN_PAUSE:
		gui_sport_run_pause_paint();
		break;
	case DISPLAY_SCREEN_RUN_CANCEL:
		gui_sport_run_cancel_paint();
		break;
	case DISPLAY_SCREEN_RUN_DETAIL:
		gui_sport_run_detail_paint();
		break;
	case DISPLAY_SCREEN_REMIND_PAUSE:
		gui_sport_pause_and_data_paint(gui_sport_type,gui_sport_index);
			
			//gui_sport_pause_hint_paint();
			break;
	#ifdef COD 
     case DISPLAY_SCREEN_REMIND_VICE_PAUSE_CONTINUE://运动投屏暂停或继续提醒
			
			gui_vice_screen_data_paint(VICE_SCREEN_DATA_SHOW);
			
			gui_vice_pause_hint_paint();
			break;
		case	DISPLAY_SCREEN_SPORT_CAST_PAUSE_REMIND://运动投屏暂停界面

         
          
		  gui_sport_cast_pause_paint();
		
			break;
#endif

			/*新增运动准备界面*/
			case DISPLAY_SCREEN_SPORT_READY:
				 				
			gui_sport_ready_paint();
			break;
		//预览界面
		case DISPLAY_SCREEN_SPORT_DATA_DISPLAY_PREVIEW:
			gui_sport_data_display_preview_paint(SPORT_DATA_PREVIEW);
			break;
   //四栏显示界面
			case DISPLAY_SCREEN_SPORT_DATA_DISPLAY:
			gui_sport_data_bar_display_paint();
			break;
			//数据显示内部设置界面
		case DISPLAY_SCREEN_SPORT_DATA_DISPLAY_SET:
			gui_sport_data_display_set_paint();
			break;
		case DISPLAY_SCREEN_SPORT_REMIND_SET:
			gui_sport_remind_set_paint();
			break;
		case DISPLAY_SCREEN_SPORT_REMIND_SET_HAERT:
			gui_sport_remind_set_heart_paint();
			break;
		case DISPLAY_SCREEN_SPORT_REMIND_SET_PACE:
			gui_sport_remind_set_pace_paint();
			break;
		case DISPLAY_SCREEN_SPORT_REMIND_SET_DISTANCE:
			gui_sport_remind_set_distance_paint();
			break;
		case DISPLAY_SCREEN_SPORT_REMIND_SET_AUTOCIRCLE:
			gui_sport_remind_set_autocircle_paint();
			break;
		case DISPLAY_SCREEN_SPORT_REMIND_SET_COALKCAL:
			gui_sport_remind_set_goalkcal_paint();
			break;
		case DISPLAY_SCREEN_SPORT_REMIND_SET_TIME:
			gui_sport_remind_set_time_paint();
			break;
		case DISPLAY_SCREEN_SPORT_REMIND_SET_SPEED:
			gui_sport_remind_set_speed_paint();
			break;
		case DISPLAY_SCREEN_SPORT_REMIND_SET_SWIMPOOL:
			gui_sport_remind_set_swimpool_paint();
			break;
		case DISPLAY_SCREEN_SPORT_REMIND_SET_GOALCIRCLE:
			gui_sport_remind_set_goalcircle_paint();
			break;
		case DISPLAY_SCREEN_SPORT_REMIND_SET_INDOORLENG:
			gui_sport_remind_set_indoorleng_paint();
			break;	
			
       /*高度提醒设置显示界面*/
		case DISPLAY_SCREEN_SPORT_REMIND_SET_ALTITUDE:
            
		gui_sport_remind_set_altitude_paint();
		break;
        
        case DISPLAY_SCREEN_SPORT_CANCEL:
            gui_sport_cancel_paint();
            break;
        case DISPLAY_SCREEN_SPORT_FEEL:
            gui_sport_feel_paint();
            break;

		case DISPLAY_SCREEN_SPORT_HRRECROVY:
            gui_sport_heart_recovery_paint();
            break;
        case DISPLAY_SCREEN_SPORT_SAVING:
            gui_sport_saving_paint((uint8_t)index);
            break;
        case DISPLAY_SCREEN_SPORT_SAVED:
            gui_sport_saved_paint();
            break;

				//训练计划 跑步
				case DISPLAY_SCREEN_TRAINPLAN_HOME_RUN_HINT:
				case DISPLAY_SCREEN_TRAINPLAN_SPORT_RUN_HINT:
					gui_trainplan_today_type_content_paint();
					break;
				
				case DISPLAY_SCREEN_TRAINPLAN_IS_COMPLETE:
					gui_trainplan_iscomplete_paint();
					break;
				case DISPLAY_SCREEN_NOTIFY_TRAINING_DAY://训练计划日提醒
					gui_trainplan_today_hint_paint();
					break;
				case DISPLAY_SCREEN_NOTIFY_TRAINING_COMPLETE://训练计划目标达成提醒
					gui_trainplan_target_hint_paint();
					break;
		case DISPLAY_SCREEN_CLIMBING:
			gui_sport_climbing_paint();
			break;
		case DISPLAY_SCREEN_CLIMBING_PAUSE:
			gui_sport_climbing_pause_paint();
			break;
		case DISPLAY_SCREEN_CLIMBING_CANCEL:
			gui_sport_climbing_cancel_paint();
			break;
		case DISPLAY_SCREEN_CLIMBING_DETAIL:
			gui_sport_climbing_detail_paint();
			break;
//Jason_V1.00:Addin Swimming Interface Display Cases in the following section:		
			case DISPLAY_SCREEN_SWIMMING:
			gui_sport_swim_paint();
			break;
		case DISPLAY_SCREEN_SWIM_PAUSE:
			gui_sport_swim_pause_paint();
			break;
		case DISPLAY_SCREEN_SWIM_CANCEL:
			gui_sport_swim_cancel_paint();
			break;
		case DISPLAY_SCREEN_SWIM_DETAIL:
			gui_sport_swim_detail_paint();
			break;
	

	
//Jason_V1.10: Addin CrossCountryRunning Interface Display Cases in the following section:	
	case DISPLAY_SCREEN_CROSSCOUNTRY:
		gui_sport_crosscountry_paint();
	break;
	case DISPLAY_SCREEN_CROSSCOUNTRY_PAUSE:
		gui_sport_crosscountry_pause_paint();
	break;
	case DISPLAY_SCREEN_CROSSCOUNTRY_CANCEL:
		gui_sport_crosscountry_cancel_paint();
	break;
	case DISPLAY_SCREEN_CROSSCOUNTRY_DETAIL:
		gui_sport_crosscountry_detail_paint();
	break;
//Jason_V1.30: Addin NotifyMenu Interface Display Cases in the following section:	
	
	case DISPLAY_SCREEN_NOTIFY_HEARTRATE:
		//if((index&0xF1000000) == 0xF1000000)
		//{
			gui_notify_heartrate_warning_paint(index&0xFFFF);
		//}
		break;
	case DISPLAY_SCREEN_NOTIFY_CALORY:
    gui_notify_train_calory_paint();
    break;
	case DISPLAY_SCREEN_NOTIFY_STEP:
    gui_notify_train_step_paint();
    break;	


	
  case DISPLAY_SCREEN_NOTIFY_GOALKCAL:
    gui_notify_sport_calory_paint((uint32_t)index);
    break;
	case DISPLAY_SCREEN_NOTIFY_GOALCIRCLE:     
    gui_notify_sport_circle_paint((uint32_t)index);
    break;
  case DISPLAY_SCREEN_NOTIFY_GOALTIME:
    gui_notify_sport_time_paint((uint32_t)index);
    break;
  case DISPLAY_SCREEN_NOTIFY_GOALDISTANCE:
    gui_notify_sport_distance_paint((uint32_t)index);
    break;
	case DISPLAY_SCREEN_NOTIFY_CIRCLEDISTANCE:
	gui_notify_sport_atuocircle_paint();
	break;
	case DISPLAY_SCREEN_NOTIFY_ALTITUDE:
    gui_notify_sport_height_paint((uint32_t)index);
    break;
  case DISPLAY_SCREEN_NOTIFY_PACE:
    gui_notify_sport_pace_paint((uint32_t)index);
    break;
	case DISPLAY_SCREEN_NOTIFY_SPEED:
    gui_notify_sport_speed_paint((uint32_t)index);
    break;
//Jason's code end in here
	
		case DISPLAY_SCREEN_POST_HDR:
			DisplayPostHdr(0);
			break;
		case DISPLAY_SCREEN_POST_HDR_SNR:
			DisplayPostHdr_SNR(0);
			break;
		case DISPLAY_SCREEN_POST_WATCH_INFO:
			gui_post_watch_info_paint();
			break;		
		
		
		case DISPLAY_SCREEN_POST_SETTINGS:
			DisplayPostSettings();
			break;
		case DISPLAY_SCREEN_CYCLING:	//骑行界面
			gui_sport_cycling_paint();
			break;
		case DISPLAY_SCREEN_CYCLING_PAUSE:
			gui_sport_cycling_pause_paint();
			break;
		case DISPLAY_SCREEN_CYCLING_CANCEL:
			gui_sport_cycling_cancel_paint();
			break;
		case DISPLAY_SCREEN_CYCLING_DETAIL:
			gui_sport_cycling_detail_paint();
			break;
		case DISPLAY_SCREEN_INDOORRUN:	//室内跑界面
			gui_sport_indoorrun_paint();
			break;
		case DISPLAY_SCREEN_INDOORRUN_PAUSE:
			gui_sport_indoorrun_pause_paint();
			break;
		case DISPLAY_SCREEN_INDOORRUN_CANCEL:
			gui_sport_indoorrun_cancel_paint();
			break;
		case DISPLAY_SCREEN_INDOORRUN_DETAIL:
			gui_sport_indoorrun_detail_paint();
			break;

		case DISPLAY_SCREEN_WALK:		//健走界面
			gui_sport_walk_paint();
			break;
		case DISPLAY_SCREEN_WALK_PAUSE:
			gui_sport_walk_pause_paint();
			break;
		case DISPLAY_SCREEN_WALK_CANCEL:
			gui_sport_walk_cancel_paint();
			break;
		case DISPLAY_SCREEN_WALK_DETAIL:
			gui_sport_walk_detail_paint();
			break;
		case DISPLAY_SCREEN_MARATHON:	//马拉松界面
			gui_sport_marathon_paint();
			break;
		case DISPLAY_SCREEN_MARATHON_PAUSE:
			gui_sport_marathon_pause_paint();
			break;
		case DISPLAY_SCREEN_MARATHON_CANCEL:
			gui_sport_marathon_cancel_paint();
			break;
		case DISPLAY_SCREEN_MARATHON_DETAIL:
			gui_sport_marathon_detail_paint();
			break;
		case DISPLAY_SCREEN_CROSSCOUNTRY_HIKE:	//徒步越野
			
			gui_sport_hike_paint();
			
			break;
		case DISPLAY_SCREEN_CROSSCOUNTRY_HIKE_PAUSE:
			gui_sport_hike_pause_paint();
			break;
		case DISPLAY_SCREEN_CROSSCOUNTRY_HIKE_CANCEL:
			gui_sport_hike_cancel_paint();
			break;
		case DISPLAY_SCREEN_CROSSCOUNTRY_HIKE_DETAIL:
			gui_sport_hike_detail_paint();
			break;
	
		case DISPLAY_SCREEN_TRIATHLON:	//铁人三项
			gui_sport_triathlon_ready_paint();
			break;
		case DISPLAY_SCREEN_TRIATHLON_SWIMMING:
			gui_sport_triathlon_swimming_paint();
			break;
		case DISPLAY_SCREEN_TRIATHLON_CYCLING:
			gui_sport_triathlon_cycling_paint();
			break;
		case DISPLAY_SCREEN_TRIATHLON_RUNNING:
			gui_sport_triathlon_running_paint();
			break;
		case DISPLAY_SCREEN_TRIATHLON_FIRST_CHANGE:
			gui_sport_triathlon_first_change_paint();
			break;
		case DISPLAY_SCREEN_TRIATHLON_CYCLING_READY:
			gui_sport_triathlon_cycling_ready_paint();
			break;
		case DISPLAY_SCREEN_TRIATHLON_SECOND_CHANGE:
			gui_sport_triathlon_second_change_paint();
			break;
		case DISPLAY_SCREEN_TRIATHLON_RUNNING_READY:
			gui_sport_triathlon_running_ready_paint();
			break;
		case DISPLAY_SCREEN_TRIATHLON_FINISH:
			gui_sport_triathlon_finish_paint((uint8_t)index);
			break;
		case DISPLAY_SCREEN_TRIATHLON_PAUSE:
			gui_sport_triathlon_pause_paint();
			break;
		case DISPLAY_SCREEN_TRIATHLON_CANCEL:
			gui_sport_triathlon_cancel_paint();
			break;
		case DISPLAY_SCREEN_TRIATHLON_DETAIL:
			gui_sport_triathlon_detail_paint();
			break;
		
		case DISPLAY_SCREEN_NOTIFY_WARNING:       
				gui_notify_warning_paint(index);
				break;			
		case DISPLAY_SCREEN_SPORTS_RECORD_TRACK_DETAIL:
			gui_sports_track_paint(LCD_BLACK);
			break;
		case DISPLAY_SCREEN_NOTIFY_VO2MAX_LACTATE_ERALY_END:
			gui_VO2max_lactate_eraly_end_hint_paint();
			break;

		
		
		case DISPLAY_SCREEN_BLE_DFU:
				gui_dfu_paint();	
		 break;				
		

		case DISPLAY_SCREEN_TRAINPLAN_RUN_SELECT:
			gui_trainplan_run_select_paint();
			break;
		case DISPLAY_SCREEN_TEST_CALIBRATION_VIEW_STATUS:
			gui_time_calibration_status_test_paint();
			break;
		case DISPLAY_SCREEN_END_UNFINISH_SPORT:
#if  defined WATCH_COM_SHORTCUNT_UP
			gui_end_unfinished_sport_hint_paint(index);
#else
			gui_end_unfinished_sport_hint_paint();
#endif
			break;
		case DISPLAY_SCREEN_TEST_REAL_DIFF_TIME:
			break;
		case DISPLAY_SCREEN_NOTIFY_FIND_WATCH:
			gui_notify_find_watch_piant();
			break;
#if defined(WATCH_AUTO_BACK_HOME)
		case DISPLAY_SCREEN_POST_AUTO_BACK_HOME:
			gui_post_auto_back_home_paint();
			break;
#endif
#ifdef COD 
		case DISPLAY_SCREEN_UPDATE_BP:
			gui_update_cod_dial_paint();
			break;
		case DISPLAY_SCREEN_NOTIFY_ON_BIND_DEVICE://
		{
			gui_notify_on_bind_device_paint();
		}
		break;
		case DISPLAY_SCREEN_NOTIFY_BIND_STATUS://
		{
			gui_notify_bind_status_paint((cod_bind_status)index);
		}
		break;
         case  DISPLAY_SCREEN_NOTIFY_LOCATION_OK:

			
			gui_notify_locatin_ok_paint();
		 	break;
		case DISPLAY_SCREEN_TRAIN_CAST_START_REMIND:	//即将进入训练提醒
		gui_training_cast_start_remind_paint();
		break;
		case DISPLAY_SCREEN_TRAIN_CAST_PRESS_REMIND:	//训练暂停按键提醒
		gui_training_cast_press_remind_paint();
		break;
		case DISPLAY_SCREEN_TRAIN_CAST_END_REMIND:	//训练结束提醒
		gui_training_cast_end_remind_paint();
		break;
		case DISPLAY_SCREEN_TRAIN_CAST_PAUSE_REMIND:	//训练暂停界面
		gui_training_cast_pause_paint();

		break;
		case DISPLAY_SCREEN_TRAIN_CAST://训练投屏
		gui_training_cast_paint();
		
		break;
#endif
		default:
			break;
	}
    
}


