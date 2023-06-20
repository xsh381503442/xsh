/*应用层FreeRtos级定时器*/
#include "timer_app.h"
#include "time_notify.h"
#include "app_config.h"
#include "timer_config.h"

#include "task_display.h"
#include "task_key.h"
#include "task_hrt.h"
#include "task_gps.h"
#include "task_sport.h"
#include "task_tool.h"
#include "task_ble.h"

#include "drv_heartrate.h"
#include "drv_lsm6dsl.h"
#include "drv_ms5837.h"
#include "drv_lcd.h"

#include "algo_hdr.h"
#include "algo_trackoptimizing.h"
#include "algo_time_calibration.h"
#include "com_data.h"
#include "com_sport.h"

#include "gui_home.h"
#include "gui_sport.h"
#include "gui_tool.h"
#include "mode_power_off.h"
#include "task_ble.h"




#if DEBUG_ENABLED == 1 && TIMER_APP_LOG_ENABLED == 1
	#define TIMER_APP_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define TIMER_APP_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define TIMER_APP_WRITESTRING(...)
	#define TIMER_APP_PRINTF(...)		        
#endif

static TimerHandle_t m_xTimers_COUNTDOWN = NULL;//3秒倒计时
static TimerHandle_t m_xTimers_HDR = NULL;//心率
static TimerHandle_t m_xTimers_VO2max = NULL;//最大摄氧量12分钟倒计时
static TimerHandle_t m_xTimers_VO2max_Countdown = NULL;
static TimerHandle_t m_xTimers_LOCK = NULL;//定时锁屏
static TimerHandle_t m_xTimers_UNLOCK_HINT = NULL;//锁屏3秒自动返回
static TimerHandle_t m_xTimers_Step_Calibration = NULL;//步长校准 1s刷新
static TimerHandle_t m_xTimers_Tool = NULL;//工具界面 2S超时

#if defined WATCH_SIM_SPORT || defined WATCH_COM_SPORT
static TimerHandle_t m_xTimers_Pause = NULL;//暂停提示


static uint32_t m_timeout = 0;

uint32_t vice_timeout = 0, inter_timeout = 0;
extern PauseTimeStr train_pausetime;

extern ScreenState_t save_vice_detail_index;

#endif
#if defined COD
#include "drv_ble.h"
#include "cmd_ble.h"
#include "lib_error.h"
#include "com_ringbuffer.h"

static TimerHandle_t m_xTimers_rt_cod = NULL;

static TimerHandle_t m_xTimers_cod_sensor = NULL;

static TimerHandle_t m_xTimers_vice_Pause = NULL;//暂停提示

static TimerHandle_t m_xTimers_timeout_back = NULL;//超时退出

extern cod_ble_sport_cast cod_user_sport_cast;


extern struct ring_buffer_header cod_sensor_acc_rb; //存储咕咚六轴数据环型队列

extern struct ring_buffer_header cod_sensor_gyro_rb; //存储咕咚六轴数据环型队列

#endif

#ifdef WATCH_COD_BSP
static TimerHandle_t m_xTimers_lcd = NULL;
#endif

#ifdef WATCH_LCD_ESD
static TimerHandle_t m_xTimers_read_lcd = NULL;
#endif
#ifdef WATCH_LCD_LPM
static TimerHandle_t m_xTimers_lcd_lpm = NULL;
#endif
#define DELAY_MS(X)                 vTaskDelay(X / portTICK_PERIOD_MS)



#if defined(WATCH_PRECISION_TIME_JUMP_TEST)
static TimerHandle_t m_xTimers_Precision_Time_Jump_Test = NULL;//精密授时界面时间跳变测试 3s
#endif


#if defined(WATCH_NOT_LEAP_SECOND_SET_TIME)
static TimerHandle_t m_xTimers_Detect_Leap_Second = NULL;//开启GPS后用于检测闰秒是否有效
static uint32_t s_detect_leap_second_count = 0;
void timer_app_detect_leap_second_stop(bool is_isr);
#endif

static TimerHandle_t m_xTimers_sport_record_detail = NULL;//运动详情界面2分钟超时
#if defined(WATCH_AUTO_BACK_HOME)
static TimerHandle_t m_xTimers_auto_back_home = NULL;//10分钟无任何操作（无按键操作）返回待机界面
#endif

extern ScreenState_t ScreenStateAlarm;
static uint8_t counter = 2;
static uint32_t countdown_counter = 0;
Countdown_time_t time_countdown;
uint32_t total_hdr_value = 0,total_hdr_count = 0,total_hdr_VO2max_value = 0,total_hdr_VO2max_count = 0;//为了测算心率在一定时长内，平均心率值
uint8_t Distinguish_VO2_Lactate = 0;//为了区分测量界面是最大摄氧量界面还是乳酸阈界面进入的
uint8_t Average_Hdr_Value = 0;
uint32_t mile_minute_lactate = 0;
extern uint32_t g_VO2max_Step;//最大摄氧量步数;
uint32_t g_VO2maxLactate_SpeedTotal = 0,g_VO2maxLactate_Count = 0;
//最大摄氧量乳酸阈或者步长校准界面  第一次定到位前步数及有效步长及gps定到位后与没有gps信号之间的步差
uint32_t Step_VO2CalInit = 0,Step_VO2maxOrCalibration = 0,StepDiff_VO2maxOrCalibration = 0;
bool CalibrationStep_GPSFlag = false; //校准步长界面第一次定到位标志
uint32_t calibration_step = 0,calibration_mile = 0;
/*在一定时长内，平均心率值获取*/
uint8_t get_average_time_hdr(void)
{
	uint8_t value = 0;
	
	switch(Distinguish_VO2_Lactate)
	{
		case DISTINGUISH_VO2:
		{
			if(total_hdr_VO2max_count != 0)
			{
				value = (int)(total_hdr_VO2max_value / total_hdr_VO2max_count);
			}
			else
			{
				value = 0;
			}
		}
			break;
		case DISTINGUISH_LACTATE:
		{
			if(total_hdr_count != 0)
			{
				value = (int)(total_hdr_value / total_hdr_count);
			}
			else
			{
				value = 0;
			}
		}
			break;
		default:
			value = 0;
			break;
	}

	return value;
}

/*最大摄氧量和乳酸阈  距离算法初始化及步数保存*/
void InitDistanceAlgo_VO2maxLactate(void)
{
	g_VO2max_Step = drv_lsm6dsl_get_stepcount();
	//算法初始化
	PhysicalParamCalculateInit();
	g_VO2maxLactate_SpeedTotal = 0;
	g_VO2maxLactate_Count = 0;
	Step_VO2CalInit = 0;
	Step_VO2maxOrCalibration = 0;
	StepDiff_VO2maxOrCalibration = 0;
	CalibrationStep_GPSFlag = false;
	calibration_step = 0;
	calibration_mile = 0;
}
/*最大摄氧量和乳酸阈 关闭GPS 距离算法初始化及步数保存*/
void UninitVO2maxMeasing(void)
{
#if defined WATCH_COM_SHORTCUNT_UP
	if(Distinguish_VO2_Lactate == DISTINGUISH_LACTATE)
	{
		Average_Hdr_Value = get_average_time_hdr();
		
		if(g_VO2maxLactate_Count != 0)
		{
			mile_minute_lactate = g_VO2maxLactate_SpeedTotal / g_VO2maxLactate_Count;
		}
		else
		{
			mile_minute_lactate = 0;
		}
		if(countdown_counter > (COUNTDOWN_COUNTER_MS_30_MINUTE - TIMER_EARLY_END_LACTATE))
		{//测量20分钟以上 才保存值,否则置0
			Average_Hdr_Value = 0;
			mile_minute_lactate = 0;
			SetValue.IsMeasuredLthreshold = 1;
		}
		else
		{
			if(DistanceResult() != 0 && Average_Hdr_Value != 0 && mile_minute_lactate != 0)
			{//移动一短距离 有效
				SetValue.IsMeasuredLthreshold = 2;
			}
			else
			{//无移动时候
				SetValue.IsMeasuredLthreshold = 1;
			}
		}
		if(SetValue.IsMeasuredLthreshold == 1)
		{
			Average_Hdr_Value = 0;
			mile_minute_lactate = 0;
		}
		else if(SetValue.IsMeasuredLthreshold == 2)
		{
			//数组有效时才存
			DayMainData.LTBmpMeasuring = Average_Hdr_Value;
			SetValue.AppGet.LTBmpMeasuring = DayMainData.LTBmpMeasuring;
			DayMainData.LTSpeedMeasuring = mile_minute_lactate;
			SetValue.AppGet.LTSpeedMeasuring = DayMainData.LTSpeedMeasuring;
		}
	}
	else if(Distinguish_VO2_Lactate == DISTINGUISH_VO2)
	{
		uint32_t m_VO2max_value = 0;
		m_VO2max_value = CalculateVO2max(DistanceResult()/100.f,SetValue.AppSet.Sex);
		if(countdown_counter > (COUNTDOWN_COUNTER_MS_12_MINUTE - TIMER_EARLY_END_VO2MAX))
		{//测量10分钟以上 才保存值,否则置0
			SetValue.IsMeasuredVO2max = 1;
		}
		else
		{
			if(DistanceResult() != 0 && m_VO2max_value != 0)
			{//移动一短距离
				SetValue.IsMeasuredVO2max = 2;
			}
			else
			{//无移动时候
				SetValue.IsMeasuredVO2max = 1;
			}
		}
		if(SetValue.IsMeasuredVO2max == 1)
		{
			m_VO2max_value = 0;
		}
		else if(SetValue.IsMeasuredVO2max == 2)
		{
			DayMainData.VO2MaxMeasuring = m_VO2max_value;
			SetValue.AppGet.VO2MaxMeasuring = DayMainData.VO2MaxMeasuring;
		}
	}
#else
	if(Distinguish_VO2_Lactate == DISTINGUISH_LACTATE)
	{
		Average_Hdr_Value = get_average_time_hdr();
		
		if(g_VO2maxLactate_Count != 0)
		{
			mile_minute_lactate = g_VO2maxLactate_SpeedTotal / g_VO2maxLactate_Count;
		}
		else
		{
			mile_minute_lactate = 0;
		}
		if(countdown_counter > (COUNTDOWN_COUNTER_MS_30_MINUTE - TIMER_EARLY_END_LACTATE))
		{//测量20分钟以上 才保存值,否则置0
			Average_Hdr_Value = 0;
			mile_minute_lactate = 0;
		}
		DayMainData.LTBmpMeasuring = Average_Hdr_Value;
		SetValue.AppGet.LTBmpMeasuring = DayMainData.LTBmpMeasuring;
		DayMainData.LTSpeedMeasuring = mile_minute_lactate;
		SetValue.AppGet.LTSpeedMeasuring = DayMainData.LTSpeedMeasuring;
	}
	else if(Distinguish_VO2_Lactate == DISTINGUISH_VO2)
	{
		DayMainData.VO2MaxMeasuring = CalculateVO2max(DistanceResult()/100.f,SetValue.AppSet.Sex);
		if(countdown_counter > (COUNTDOWN_COUNTER_MS_12_MINUTE - TIMER_EARLY_END_VO2MAX))
		{//测量10分钟以上 才保存值,否则置0
			DayMainData.VO2MaxMeasuring = 0;
		}
	}
#endif
	countdown_counter = 0;
	Step_VO2CalInit = 0;
	Step_VO2maxOrCalibration = 0;
	StepDiff_VO2maxOrCalibration = 0;
	CalibrationStep_GPSFlag = false;
	calibration_step = 0;
	calibration_mile = 0;
	timer_app_VO2max_Countdown_stop(false);
	timer_app_VO2max_stop(false);
	task_hrt_close();
	CloseGPSTask();
}
/*暂停定时器 开始*/
void timer_app_pasue_start(void)
{
	if (NULL == m_xTimers_Pause)
	{
		TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_Tool fail\n");
	}
	else
	{
		m_timeout = 0;
		 if(xTimerStart(m_xTimers_Pause, TIMER_TICKS_TO_WAIT) != pdPASS)
		 {
			 TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_Tool success\n");
		 }
	}
}
/*暂停定时器 停止*/
void timer_app_pasue_stop(bool is_isr)
{
	if(is_isr)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		if( xTimerStopFromISR( m_xTimers_Pause, &xHigherPriorityTaskWoken ) != pdPASS )
		{
			TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_Tool xTimerStopFromISR fail\n");
		}
		if( xHigherPriorityTaskWoken != pdFALSE )
		{
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}	
	}
	else{
		if (pdPASS != xTimerStop(m_xTimers_Pause, TIMER_TICKS_TO_WAIT))
		{	
			TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_Tool Stop fail\n");
		}
	}		
}


static void vTimerCallback_pasue(TimerHandle_t pxTimer)
{
	DISPLAY_MSG  msg = {0};
			
	m_timeout++;
	if (m_timeout ==1)
	{
		ScreenState = DISPLAY_SCREEN_REMIND_PAUSE;
		msg.cmd = MSG_DISPLAY_SCREEN;
		xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
	}
    else if (m_timeout > 8)
	{
		m_timeout = 0;
		timer_app_pasue_stop(false);
		timer_notify_display_stop(false);
		timer_notify_motor_stop(false);
		timer_notify_buzzer_stop(false);
		ScreenState = Save_Detail_Index;
		msg.cmd = MSG_DISPLAY_SCREEN;
		xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
	
	}
			
}

#if defined COD
/*互屏暂停定时器 开始*/
void timer_app_vice_pasue_start(uint32_t num,uint8_t flag)
{
	if (NULL == m_xTimers_vice_Pause)
	{
		TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_Tool fail\n");
	}
	else
	{
		m_timeout = 0;
		if(flag==0)
			{
		     vice_timeout = 0;
			}
		else if(flag==1)
			{
           vice_timeout = num;
           inter_timeout = 0;

		    }
		else if(flag==2)
			{
              
			  vice_timeout = num;
              inter_timeout = 1;
		    }
			
		 if(xTimerStart(m_xTimers_vice_Pause, TIMER_TICKS_TO_WAIT) != pdPASS)
		 {
			 TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_Tool success\n");
		 }
	}
}
/*暂停定时器 停止*/
void timer_app_vice_pasue_stop(bool is_isr)
{
	if(is_isr)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		if( xTimerStopFromISR( m_xTimers_vice_Pause, &xHigherPriorityTaskWoken ) != pdPASS )
		{
			TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_Tool xTimerStopFromISR fail\n");
		}
		if( xHigherPriorityTaskWoken != pdFALSE )
		{
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}	
	}
	else{
		if (pdPASS != xTimerStop(m_xTimers_vice_Pause, TIMER_TICKS_TO_WAIT))
		{	
			TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_Tool Stop fail\n");
		}
	}		
}


static void vTimerCallback_vice_pasue(TimerHandle_t pxTimer)
{
   int i;
	DISPLAY_MSG  msg = {0};
	m_timeout++;
	if(vice_timeout==0)
	{
	 	if (m_timeout ==1)
	 	{
	 		ScreenState = DISPLAY_SCREEN_REMIND_VICE_PAUSE_CONTINUE;
	 		msg.cmd = MSG_DISPLAY_SCREEN;
	 		xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
	 	}
	 	
	     else if (m_timeout >=3)
	 	{
	 		m_timeout = 0;
	 		timer_app_vice_pasue_stop(false);
	 		//timer_notify_display_stop(false);
	 		//timer_notify_motor_stop(false);
	 		//timer_notify_buzzer_stop(false);
	 		ScreenState = save_vice_detail_index;
	 		msg.cmd = MSG_DISPLAY_SCREEN;
	 		xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
	 	
	 	}
	}
	else
		{
         if(inter_timeout==0)
         	{
	         if (m_timeout >=vice_timeout)
	         	{
	            m_timeout=0;
	               timer_notify_motor_start(1000,500,3,false,NOTIFY_MODE_NORMAL);
			       timer_notify_buzzer_start(1000,500,3,false,NOTIFY_MODE_NORMAL);
				   if(cod_user_sport_cast.sport_status==COD_SPORT_STATUS_STOP)
				   	{
				   	    
						timer_app_vice_pasue_stop(false);
						
						set_train_pause_time(); 
                     	ScreenState = DISPLAY_SCREEN_HOME;
				 		msg.cmd = MSG_DISPLAY_SCREEN;
				 		xQueueSend(DisplayQueue, &msg, portMAX_DELAY);

				    }
				   else;

			    }
		 	
         	}
		 else;  //不管有没有5分钟振动提醒都计时
		 	
             
              train_pausetime.Pause_Stop_Time.Second++;
			  
			  gui_swich_msg();
                if( train_pausetime.Pause_Stop_Time.Second >= 60 )
			    {	
				    train_pausetime.Pause_Stop_Time.Second = 0;				
				    train_pausetime.Pause_Stop_Time.Minute++; 
				    if( train_pausetime.Pause_Stop_Time.Minute >= 60 )
				    {
					    train_pausetime.Pause_Stop_Time.Minute = 0;
					    train_pausetime.Pause_Stop_Time.Hour++;
				    }   
			    }

	    }
			TIMER_APP_PRINTF("m_timeout:%d;h:%d;m:%d;s:%d\r\n",m_timeout,train_pausetime.Pause_Stop_Time.Hour,train_pausetime.Pause_Stop_Time.Minute,train_pausetime.Pause_Stop_Time.Second);
}



/*咕咚全天定时器 开始*/
void timer_rt_cod_start(void)
{
	if (NULL == m_xTimers_rt_cod)
	{
		TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_Tool fail\n");
	}
	else
	{
		 if(xTimerStart(m_xTimers_rt_cod, TIMER_TICKS_TO_WAIT) != pdPASS)
		 {
			 TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_Tool success\n");
		 }
	}
}
/*咕咚全天定时器 停止*/
void timer_rt_cod_stop(bool is_isr)
{
	if(is_isr)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		if( xTimerStopFromISR( m_xTimers_rt_cod, &xHigherPriorityTaskWoken ) != pdPASS )
		{
			TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_Tool xTimerStopFromISR fail\n");
		}
		if( xHigherPriorityTaskWoken != pdFALSE )
		{
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}	
	}
	else{
		if (pdPASS != xTimerStop(m_xTimers_rt_cod, TIMER_TICKS_TO_WAIT))
		{	
			TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_Tool Stop fail\n");
		}
	}		
}


static void vTimerCallback_cod(TimerHandle_t pxTimer)
{
	if (SetValue.rt_today_summary){
		uint32_t command[4] = {BLE_UART_RESPONSE_APPLICATION_RT_TODAY | BLE_UART_CCOMMAND_MASK_APPLICATION, 0,0,0};
		command[1] = Get_TotalStep();
		command[2] = Get_TotalDistance()/100;//米
		command[3] = Get_TotalEnergy(); //卡
	
		_uart_data_param param;	
		param.user_callback = NULL;
		param.p_data = (uint8_t *)command;
		param.length = sizeof(command);
		uint8_t count = TASK_UART_DELAY_COUNT_OUT;
 
		while(drv_ble_send_data(&param) != ERR_SUCCESS && count--)
		{
			DELAY_MS(TASK_UART_DELAY);
		}	
   	}			
}


/*sensor_start定时器 开始*/
void timer_cod_sensor_start(void)
{
	if (NULL == m_xTimers_cod_sensor)
	{
		TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_Tool fail\n");
	}
	else
	{
		 if(xTimerReset(m_xTimers_cod_sensor, TIMER_TICKS_TO_WAIT) != pdPASS)
		 {
			 TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_Tool success\n");
		 }
	}
}
/*sensor_start定时器 停止*/
void timer_cod_sensor_stop(bool is_isr)
{
	if(is_isr)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		if( xTimerStopFromISR( m_xTimers_cod_sensor, &xHigherPriorityTaskWoken ) != pdPASS )
		{
			TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_Tool xTimerStopFromISR fail\n");
		}
		if( xHigherPriorityTaskWoken != pdFALSE )
		{
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}	
	}
	else{
		if (pdPASS != xTimerStop(m_xTimers_cod_sensor, TIMER_TICKS_TO_WAIT))
		{	
			TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_Tool Stop fail\n");
		}
	}		
}

static void vTimerCallback_cod_sensor(TimerHandle_t pxTimer)
{
	uint16_t data_len;
	uint8_t heart;
	uint8_t cod_acc_data[36];
	uint8_t cod_gyro_data[36];
	
	if (cod_user_sensor_req.sensor_start == 0)
	{
		return;
	}
	if (cod_user_sensor_req.six_freq  == 100)
	{
		data_len = 6*6;//100hz时 60ms 6组数据，100ms10组数据
	}
	else
	{
		data_len =3*6;//50hz时 60ms 3组数据,100ms 5组数据
	}

	if (ring_buffer_data_left(&cod_sensor_acc_rb) >= data_len || ring_buffer_data_left(&cod_sensor_gyro_rb) >= data_len)
	{
		
		if  ((cod_user_sensor_req.acc_flag) && (cod_user_sensor_req.gyro_flag))
		{
			if (ring_buffer_data_left(&cod_sensor_acc_rb) >= data_len && ring_buffer_data_left(&cod_sensor_gyro_rb) >= data_len)
			{
				ring_buffer_read(&cod_sensor_acc_rb,cod_acc_data,data_len);
				ring_buffer_read(&cod_sensor_gyro_rb,cod_gyro_data,data_len);
				data_len += 2;//包含2字节的类型、长度,
				if (cod_user_sensor_req.heart_flag)
				{
				
					heart = drv_getHdrValue();
					ble_cod_sensor_data_trains(cod_acc_data,data_len,cod_gyro_data,data_len,&heart,3);
				}
				else
				{
	
					ble_cod_sensor_data_trains(cod_acc_data,data_len,cod_gyro_data,data_len,NULL,0);
				}
			}
		}
		else
		{
			 if (cod_user_sensor_req.acc_flag)
		  	  {
		  	  		ring_buffer_read(&cod_sensor_acc_rb,cod_acc_data,data_len);
					data_len += 2;//包含2字节的类型、长度
					
			  		if (cod_user_sensor_req.heart_flag)
					{
						
					    heart = drv_getHdrValue();
						ble_cod_sensor_data_trains(cod_acc_data,data_len,NULL,0,&heart,3);
					}
					else
					{
					
						ble_cod_sensor_data_trains(cod_acc_data,data_len,NULL,0,NULL,0);
					}
		  	  }
			  if (cod_user_sensor_req.gyro_flag)
		  	  {
		  	  		ring_buffer_read(&cod_sensor_gyro_rb,cod_gyro_data,data_len);
					data_len += 2;//包含2字节的类型、长度
					
			  		if (cod_user_sensor_req.heart_flag)
					{	
						heart = drv_getHdrValue();
						ble_cod_sensor_data_trains(NULL,0,cod_gyro_data,data_len,&heart,3);
					}
					else
					{
						ble_cod_sensor_data_trains(NULL,0,cod_gyro_data,data_len,NULL,0);
					}
		  	  }
		}
	}
}


/*timeout_back定时器 开始*/
void timer_timeout_back_start(void)
{
	if (NULL == m_xTimers_timeout_back)
	{
		TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_Tool fail\n");
	}
	else
	{
		 if(xTimerReset(m_xTimers_timeout_back, TIMER_TICKS_TO_WAIT) != pdPASS)
		 {
			 TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_Tool success\n");
		 }
	}
}
/*timeout_back定时器 停止*/
void timer_timeout_back_stop(bool is_isr)
{
	if(is_isr)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		if( xTimerStopFromISR( m_xTimers_timeout_back, &xHigherPriorityTaskWoken ) != pdPASS )
		{
			TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_Tool xTimerStopFromISR fail\n");
		}
		if( xHigherPriorityTaskWoken != pdFALSE )
		{
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}	
	}
	else{
		if (pdPASS != xTimerStop(m_xTimers_timeout_back, TIMER_TICKS_TO_WAIT))
		{	
			TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_Tool Stop fail\n");
		}
	}		
}


static void vTimerCallback_timeout_back(TimerHandle_t pxTimer)
{
	DISPLAY_MSG  msg = {0};
	extern uint32_t location_timer_count ;
	extern uint32_t progress_pre_timer;
	if (IsSportReady(ScreenState) == true ||ScreenState == DISPLAY_SCREEN_SPORT )
	{

			//退出运动选择关闭gps,心率
			CloseGPSTask();	
			task_hrt_close();
			
			progress_pre_timer = 0;
			location_timer_count = 0;
			ScreenState = DISPLAY_SCREEN_HOME;
			//返回待机界面
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
	}

}



#endif

#ifdef WATCH_LCD_LPM
void timer_lcd_lpm_start(void)
{
	if (NULL == m_xTimers_lcd_lpm)
	{
		TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_Tool fail\n");
	}
	else
	{
		 if(xTimerReset(m_xTimers_lcd_lpm, TIMER_TICKS_TO_WAIT) != pdPASS)
		 {
			 TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_Tool success\n");
		 }
	}
}

extern uint8_t mp_flag;
extern uint8_t light_enable;
static void vTimerCallback_lcd_lpm(TimerHandle_t pxTimer)
{
	if (ScreenState != DISPLAY_SCREEN_LOGO && mp_flag == 1 &&  light_enable == 0)
	{
		drv_lcd_enable();
		drv_lcd_switch_hpm_to_lpm();
		drv_lcd_disable();
		timer_lcd_lpm_start();
	}
	
}
#endif

#ifdef WATCH_COD_BSP
/*lcd定时器 开始*/
void timer_lcd_start(void)
{
	if (NULL == m_xTimers_lcd)
	{
		TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_Tool fail\n");
	}
	else
	{
		 if(xTimerReset(m_xTimers_lcd, TIMER_TICKS_TO_WAIT) != pdPASS)
		 {
			 TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_Tool success\n");
		 }
	}
}
/*lcd定时器 停止*/
void timer_lcd_stop(bool is_isr)
{
	if(is_isr)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		if( xTimerStopFromISR( m_xTimers_lcd, &xHigherPriorityTaskWoken ) != pdPASS )
		{
			TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_Tool xTimerStopFromISR fail\n");
		}
		if( xHigherPriorityTaskWoken != pdFALSE )
		{
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}	
	}
	else{
		if (pdPASS != xTimerStop(m_xTimers_lcd, TIMER_TICKS_TO_WAIT))
		{	
			TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_Tool Stop fail\n");
		}
	}		
}

static void vTimerCallback_lcd(TimerHandle_t pxTimer)
{

	if (ScreenState != DISPLAY_SCREEN_LOGO)
	{
		drv_lcd_enable();
		drv_lcd_switch_hpm_to_lpm();
		drv_lcd_disable();
		#ifdef WATCH_LCD_LPM
		timer_lcd_lpm_start();
		#endif
	}
}
#endif



#ifdef WATCH_LCD_ESD
void timer_read_lcd_start(void)
{
	if (NULL == m_xTimers_read_lcd)
	{
		TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_Tool fail\n");
	}
	else
	{
		 if(xTimerStart(m_xTimers_read_lcd, TIMER_TICKS_TO_WAIT) != pdPASS)
		 {
			 TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_Tool success\n");
		 }
	}
}


static void vTimerCallback_esd(TimerHandle_t pxTimer)
{
	    uint8_t ret;
		if(ScreenState == DISPLAY_SCREEN_HOME)
		{
			drv_lcd_read_enable();
			ret = drv_lcd_status_read(0x0A);
			drv_lcd_disable();
		
			if ((ret != 0x9c)&&(ret !=0xDC))
			{
				lcd_esd_init();
			}
		}
	
}
#endif


static void vTimerCallback(TimerHandle_t pxTimer)
{
	DISPLAY_MSG  msg = {0};
	uint32_t ulTimerID = (uint32_t)pvTimerGetTimerID(pxTimer);

	switch(ulTimerID)
	{
		case TIMER_ID_COUNTDOWN:
			{//3秒倒计时
				TIMER_APP_PRINTF("[TIMER_APP]:vTimerCallback count=%d\n",counter);
				if(counter > 0)
				{
					ScreenState = DISPLAY_SCREEN_CUTDOWN;
					msg.cmd = MSG_UPDATE_COUNTDOWN;
					msg.value = counter;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
					counter --;
				}
				else if(counter == 0)
				{
					timer_app_countdown_stop(false);
					
					timer_notify_motor_start(1000,500,1,false,NOTIFY_MODE_TOOLS);
					timer_notify_buzzer_start(1000,500,1,false,NOTIFY_MODE_TOOLS);
					task_hrt_start();
#if defined WATCH_COM_SHORTCUNT_UP
					//switch(get_current_monitor_screen())

					switch(get_current_longtool_screen())
#else
					switch(Get_Curr_Home_Index())
#endif
					{
						case DISPLAY_SCREEN_VO2MAX:
						{
							Distinguish_VO2_Lactate = DISTINGUISH_VO2;
							ScreenState = DISPLAY_SCREEN_VO2MAX_MEASURING;
							//msg.cmd = MSG_UPDATE_VO2MAX_LACTATE_THRESHOLD;
							msg.value = COUNTDOWN_COUNTER_MS_12_MINUTE;
							//xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
							timer_app_VO2max_Countdown_start(COUNTDOWN_COUNTER_MS_12_MINUTE);//倒计时 12分钟
							timer_app_VO2max_start();
							InitDistanceAlgo_VO2maxLactate();
						}
							break;
						case DISPLAY_SCREEN_LACTATE_THRESHOLD:
						{
							Distinguish_VO2_Lactate = DISTINGUISH_LACTATE;
							ScreenState = DISPLAY_SCREEN_VO2MAX_MEASURING;
							//msg.cmd = MSG_UPDATE_VO2MAX_LACTATE_THRESHOLD;
							msg.value = COUNTDOWN_COUNTER_MS_30_MINUTE;
							//xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
							timer_app_VO2max_Countdown_start(COUNTDOWN_COUNTER_MS_30_MINUTE);//倒计时 30分钟
							timer_app_VO2max_start();//定时界面刷新用
							InitDistanceAlgo_VO2maxLactate();
						}
							break;
						default:
							break;
					}
				}
			}
			break;
		case TIMER_ID_HDR:
			{//心率定时器
				//drv_sendHdrHandler();//发送命令
				save_heartrate(drv_getHdrValue());//保存心率值进缓存


        if(ScreenState == DISPLAY_SCREEN_HEARTRATE || ScreenState == DISPLAY_SCREEN_POST_HDR || ScreenState == DISPLAY_SCREEN_POST_HDR_SNR)
        {
					msg.cmd = MSG_UPDATE_HDR_VALUE;
					msg.value = drv_getHdrValue();
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				
        }
				else if(ScreenState == DISPLAY_SCREEN_VO2MAX_MEASURING)
				{
					if(drv_getHdrValue() > 0)
					{
						if(Distinguish_VO2_Lactate == DISTINGUISH_LACTATE)
						{
							total_hdr_count++;
							total_hdr_value += drv_getHdrValue();
						}
						else if(Distinguish_VO2_Lactate == DISTINGUISH_VO2)
						{
							total_hdr_VO2max_count++;
							total_hdr_VO2max_value += drv_getHdrValue();
						}
					}
				}
			}
		#ifdef COD 
			if (SetValue.rt_heart){
					TIMER_APP_PRINTF("[TIMER_APP]:BLE_UART_RESPONSE_APPLICATION_RT_HEART\r\n");
					uint32_t command[2] = {BLE_UART_RESPONSE_APPLICATION_RT_HEART | BLE_UART_CCOMMAND_MASK_APPLICATION};
					command[1] =  drv_getHdrValue();
					
					_uart_data_param param; 
					param.user_callback = NULL;
					param.p_data = (uint8_t *)command;
					param.length = sizeof(command);	
					uint8_t count = TASK_UART_DELAY_COUNT_OUT;
					
					while(drv_ble_send_data(&param) != ERR_SUCCESS && count--)
					{
						DELAY_MS(150);
					}
			}
			
			if (cod_user_sensor_req.sensor_open ==1)
			{
				if ((cod_user_sensor_req.acc_flag ==0)&&(cod_user_sensor_req.gyro_flag ==0) &&(cod_user_sensor_req.heart_flag ==1))
				{
					uint8_t heart_tmp = drv_getHdrValue();
					ble_cod_sensor_data_trains(NULL,0,NULL,0,&heart_tmp,3);
				}
			}

			
		#endif
			break;
		case TIMER_ID_VO2MAX:
			{//界面刷新用
				if(ScreenState == DISPLAY_SCREEN_VO2MAX_MEASURING)
				{
				
				set_gps_flicker(1);
					if(countdown_counter > 0)
					{
						get_countdown_time(countdown_counter,&time_countdown);
						ScreenState = DISPLAY_SCREEN_VO2MAX_MEASURING;
						msg.cmd = MSG_UPDATE_VO2MAX_VALUE;
						msg.value = Distinguish_VO2_Lactate;
						xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
					}
				}
			}
			break;
		case TIMER_ID_LOCK:
			{//锁屏定时器
				if( ScreenState == DISPLAY_SCREEN_HOME )
				{
					SetValue.AutoLockScreenFlag = 1;
					ScreenState = DISPLAY_SCREEN_HOME;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				}
			}
			break;
		case TIMER_ID_UNLOCK_HINT:
			{//解锁错误提醒3s定时
				if( ScreenState == DISPLAY_SCREEN_LARGE_LOCK )
				{//大锁界面3秒自动返回待机界面
					ScreenState = DISPLAY_SCREEN_HOME;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				}
			}
			break;
		case TIMER_ID_VO2MAX_COUNTDOWN:
			{//12分钟或者15分钟等定时倒计时,时间到显示时间完成界面
				if(ScreenState == DISPLAY_SCREEN_VO2MAX_MEASURING)
				{
					countdown_counter -= TIMER_INTERVAL_MS_VO2MAX_COUNTDOWN;
					if(countdown_counter == 0)
					{
						UninitVO2maxMeasing();
						if((ScreenState != DISPLAY_SCREEN_LOGO)
							&& (ScreenState != DISPLAY_SCREEN_NOTIFY_BAT_CHG)
							&& time_notify_is_ok())
						{
							if(ScreenState < DISPLAY_SCREEN_NOTIFY)
							{
#if defined WATCH_COM_SHORTCUNT_UP
								//ScreenStateSave = get_current_monitor_screen();
                               ScreenStateSave = get_current_longtool_screen();

#else
								ScreenStateSave = Get_Curr_Home_Index();
#endif
							}
							timer_notify_display_start(5000,1,false);
							timer_notify_motor_start(1000,500,1,false,NOTIFY_MODE_TOOLS);
							timer_notify_buzzer_start(1000,500,1,false,NOTIFY_MODE_TOOLS);
							ScreenState = DISPLAY_SCREEN_TIME_DONE;
							msg.cmd = MSG_DISPLAY_SCREEN;
							xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
						}
					}
				}
			}
			break;
		case TIMER_ID_STEP_CLIBRATION:
			{
				if(ScreenState == DISPLAY_SCREEN_STEP_WEEK_STEPS
					 || ScreenState == DISPLAY_SCREEN_STEP_WEEK_MILEAGE)
				{
					msg.cmd = MSG_UPDATE_ICON_CALIBRTIONSTEP;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				}
			}
			break;
		case TIMER_ID_TOOL:
			//初始化心率
			task_hrt_start();
			timer_app_tool_stop(false);
			break;
#if defined(WATCH_PRECISION_TIME_JUMP_TEST)
		case TIMER_ID_PRECISION_TIME_JUMP_TEST:
			open_difftime_rtc_gps_time();
			break;
#endif
		
#if defined(WATCH_NOT_LEAP_SECOND_SET_TIME)
		case TIMER_ID_DETECT_LEAP_SECOND:
			//闰秒检测
			s_detect_leap_second_count ++;

			if(get_is_nav_time_gps_valid() == true || s_detect_leap_second_count >= DETECT_LEAP_SECOND_MAX_TIME)
			{//检测到闰秒或者 检测时间超过13分钟 都关闭检测
				s_detect_leap_second_count = 0;
				timer_app_detect_leap_second_stop(false);
				if (get_is_nav_time_gps_valid() == true )
				{
					s_is_open_nav_time_gps = TIME_NAV_GPS_VALID;
				}
				TIMER_APP_PRINTF("[TIMER_APP]: TIMER_ID_DETECT_LEAP_SECOND get leap second success!!!!!!!!!!\n");
			}
			else
			{
				{
					if(s_detect_leap_second_count % 30 == 1)//开始检测
					{
						TIMER_APP_PRINTF("[TIMER_APP]: TIMER_ID_DETECT_LEAP_SECOND start detect!!!!!!!!!!\n");
						drv_ublox_nav_timegps(true);
					}
					else if(s_detect_leap_second_count % 30 == 4)//关闭检测 保证开启和检测之间只有几秒
					{
						TIMER_APP_PRINTF("[TIMER_APP]: TIMER_ID_DETECT_LEAP_SECOND stop detect!!!!!!!!!!\n");
						drv_ublox_nav_timegps(false);
					}
				}
			}
			break;
#endif
		case TIMER_ID_SPORT_RECORD_DETAIL://120s运动总结详情界面超时处理 无任何按键操作,则自动返回待机界面
		{
			if(ScreenState != DISPLAY_SCREEN_HOME && is_sport_record_detail_screen(ScreenState) == true)
			{//返回到home界面
				TIMER_APP_PRINTF("[TIMER_APP]: TIMER_ID_SPORT_RECORD_DETAIL return DISPLAY_SCREEN_HOME!!!!!!!!!!\n");
				timer_notify_motor_start(1000,500,1,false,NOTIFY_MODE_NORMAL);
				timer_notify_buzzer_start(1000,500,1,false,NOTIFY_MODE_NORMAL);
				ScreenState = DISPLAY_SCREEN_HOME;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
			else if(is_sport_record_detail_screen(ScreenStateSave))
			{//通知界面 如果之前是运动详情界面且时间达成后需把之前的界面索引直接替换成待机主界面
				if(ScreenState > DISPLAY_SCREEN_NOTIFY)
				{
					ScreenStateSave = DISPLAY_SCREEN_HOME;
				}
				
				TIMER_APP_PRINTF("[TIMER_APP]: TIMER_ID_SPORT_RECORD_DETAIL ScreenStateSave = DISPLAY_SCREEN_HOME*******\n");
			}
			else if(is_sport_record_detail_screen(ScreenStateAlarm))
			{//闹钟界面 如果之前是运动详情界面且时间达成后需把之前的界面索引直接替换成待机主界面
				if(ScreenState == DISPLAY_SCREEN_NOTIFY_ALARM)
				{
					ScreenStateAlarm = DISPLAY_SCREEN_HOME;
				}
				TIMER_APP_PRINTF("[TIMER_APP]: TIMER_ID_SPORT_RECORD_DETAIL ScreenStateAlarm = DISPLAY_SCREEN_HOME*******\n");
			}
			else
			{
				TIMER_APP_PRINTF("[TIMER_APP]: TIMER_ID_SPORT_RECORD_DETAIL stop*******\n");
			}
		}
			break;
#if defined(WATCH_AUTO_BACK_HOME)
		case TIMER_ID_AUTO_BACK_HOME:
		{//10分钟无任何操作（无按键操作）返回待机界面
			if(is_auto_back_home(ScreenState) == true)
			{
				timer_app_auto_back_home_stop(false);
				
				am_hal_rtc_time_get(&RTC_time);
				AutoBackScreenStr m_auto_back_home;
				
				m_auto_back_home.status = true;
				m_auto_back_home.screen_before_index = ScreenState;
				m_auto_back_home.before_time = RTC_time;
				set_m_AutoBackScreen(m_auto_back_home);
				
				mode_close();
				ScreenState = DISPLAY_SCREEN_HOME;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
		}
		break;
#endif
		default:
			break;
	}
}

void timer_app_init(void)
{
	m_xTimers_COUNTDOWN = xTimerCreate("COUNTDOWN",
												            ( TIMER_INTERVAL_MS_1000 / portTICK_PERIOD_MS ),
												            pdTRUE,
												            (void*)TIMER_ID_COUNTDOWN,
												            vTimerCallback);
	m_xTimers_HDR = xTimerCreate("HDR",
												            ( TIMER_INTERVAL_MS_1000 / portTICK_PERIOD_MS ),
												            pdTRUE,
												            (void*)TIMER_ID_HDR,
												            vTimerCallback);
	m_xTimers_VO2max = xTimerCreate("VO2MAX",
												            ( TIMER_INTERVAL_MS_VO2MAX / portTICK_PERIOD_MS ),
												            pdTRUE,
												            (void*)TIMER_ID_VO2MAX,
												            vTimerCallback);	
	m_xTimers_VO2max_Countdown = xTimerCreate("VO2MAXCOUNTDOWN",
												            ( TIMER_INTERVAL_MS_VO2MAX_COUNTDOWN / portTICK_PERIOD_MS ),
												            pdTRUE,
												            (void*)TIMER_ID_VO2MAX_COUNTDOWN,
												            vTimerCallback);																			
	m_xTimers_LOCK = xTimerCreate("LOCK",
												            ( TIMER_INTERVAL_MS_LOCK / portTICK_PERIOD_MS ),
												            pdFALSE,
												            (void*)TIMER_ID_LOCK,
												            vTimerCallback);		
	m_xTimers_UNLOCK_HINT = xTimerCreate("UNLOCK HINT",
												            ( TIMER_INTERVAL_MS_UNLOCK_HINT / portTICK_PERIOD_MS ),
												            pdFALSE,
												            (void*)TIMER_ID_UNLOCK_HINT,
												            vTimerCallback);
	m_xTimers_Step_Calibration = xTimerCreate("Step",
												            ( TIMER_INTERVAL_MS_1000 / portTICK_PERIOD_MS ),
												            pdTRUE,
												            (void*)TIMER_ID_STEP_CLIBRATION,
												            vTimerCallback);
	m_xTimers_Tool = xTimerCreate("Tool",
												            ( TIMER_INTERVAL_MS_TOOL / portTICK_PERIOD_MS ),
												            pdTRUE,
												            (void*)TIMER_ID_TOOL,
										            vTimerCallback);

#if defined WATCH_SIM_SPORT || defined WATCH_COM_SPORT
	m_xTimers_Pause = xTimerCreate("Pause",
												            ( 200 / portTICK_PERIOD_MS ),
												            pdTRUE,
												            (void*)TIMER_ID_PAUSE,
												            vTimerCallback_pasue);



#endif
#if defined COD
	m_xTimers_vice_Pause = xTimerCreate("vice_Pause",
												            ( TIMER_INTERVAL_MS_1000 / portTICK_PERIOD_MS ),
												            pdTRUE,
												            (void*)TIMER_ID_PAUSE,
												            vTimerCallback_vice_pasue);										
	m_xTimers_rt_cod = xTimerCreate("RT_COD",
												            ( 5000 / portTICK_PERIOD_MS ),
												            pdTRUE,
												            NULL,
												            vTimerCallback_cod);

	m_xTimers_cod_sensor = xTimerCreate("COD_SENSOR",
												            ( 10 / portTICK_PERIOD_MS ),
												            pdTRUE,
												            NULL,
												            vTimerCallback_cod_sensor);
	m_xTimers_timeout_back = xTimerCreate("timeout_back",
												            ( 15*60*1000 / portTICK_PERIOD_MS ),
												            pdFALSE,
												            NULL,
												            vTimerCallback_timeout_back);
#endif
#ifdef WATCH_COD_BSP

	m_xTimers_lcd = xTimerCreate("lcd",
													( 1200 / portTICK_PERIOD_MS ),
													pdFALSE,
													NULL,
													vTimerCallback_lcd);
#endif

#ifdef WATCH_LCD_LPM
m_xTimers_lcd_lpm = xTimerCreate("lcd_lpm",
													( 10000 / portTICK_PERIOD_MS ),
													pdTRUE,
													NULL,
													vTimerCallback_lcd_lpm);
#endif


#ifdef WATCH_LCD_ESD
m_xTimers_read_lcd = xTimerCreate("esd_lcd",
													( 8000 / portTICK_PERIOD_MS ),
													pdTRUE,
													NULL,
													vTimerCallback_esd);
#endif


#if defined(WATCH_PRECISION_TIME_JUMP_TEST)
	m_xTimers_Precision_Time_Jump_Test = xTimerCreate("PRECISION_TIME_JUMP",
												            ( TIMER_INTERVAL_MS_PRECISION_TIME_JUMP_TEST / portTICK_PERIOD_MS ),
												            pdFALSE,
												            (void*)TIMER_ID_PRECISION_TIME_JUMP_TEST,
												            vTimerCallback);//单次
#endif												

#if defined(WATCH_NOT_LEAP_SECOND_SET_TIME)
	m_xTimers_Detect_Leap_Second = xTimerCreate("DETECT LEAP SECOND",
												            ( TIMER_DETECT_LEAP_SECOND / portTICK_PERIOD_MS ),
												            pdTRUE,
												            (void*)TIMER_ID_DETECT_LEAP_SECOND,
												            vTimerCallback);
#endif																		
	m_xTimers_sport_record_detail = xTimerCreate("SPORT RECORD DETAIL",
												            ( TIMER_SPORT_RECORD_DETAIL / portTICK_PERIOD_MS ),
												            pdFALSE,
												            (void*)TIMER_ID_SPORT_RECORD_DETAIL,
												            vTimerCallback);					
#if defined(WATCH_AUTO_BACK_HOME)
	m_xTimers_auto_back_home = xTimerCreate("AUTO BACK HOME",
												            ( TIMER_AUTO_BACK_HOME / portTICK_PERIOD_MS ),
												            pdFALSE,
												            (void*)TIMER_ID_AUTO_BACK_HOME,
												            vTimerCallback);		
#endif

	if (NULL == m_xTimers_COUNTDOWN)
	{
		TIMER_APP_PRINTF("[TIMER_APP]: m_xTimers_COUNTDOWN Create fail\n");
	}
	if (NULL == m_xTimers_HDR)
	{
		TIMER_APP_PRINTF("[TIMER_APP]: m_xTimers_HDR Create fail\n");
	}
	if (NULL == m_xTimers_VO2max)
	{
		TIMER_APP_PRINTF("[TIMER_APP]: m_xTimers_VO2max Create fail\n");
	}
	if (NULL == m_xTimers_VO2max_Countdown)
	{
		TIMER_APP_PRINTF("[TIMER_APP]: m_xTimers_VO2max_Countdown Create fail\n");
	}
	if (NULL == m_xTimers_LOCK)
	{
		TIMER_APP_PRINTF("[TIMER_APP]: m_xTimers_LOCK Create fail\n");
	}
	if (NULL == m_xTimers_UNLOCK_HINT)
	{
		TIMER_APP_PRINTF("[TIMER_APP]: m_xTimers_UNLOCK_HINT Create fail\n");
	}
	if (NULL == m_xTimers_Step_Calibration)
	{
		TIMER_APP_PRINTF("[TIMER_APP]: m_xTimers_Step_Calibration Create fail\n");
	}
#if defined WATCH_SIM_SPORT || defined WATCH_COM_SPORT
	if (NULL == m_xTimers_Pause)
	{
		TIMER_APP_PRINTF("[TIMER_APP]: m_xTimers_Pasue Create fail\n");
	}

#endif

#if defined(WATCH_PRECISION_TIME_JUMP_TEST)
	if (NULL == m_xTimers_Precision_Time_Jump_Test)
	{
		TIMER_APP_PRINTF("[TIMER_APP]: m_xTimers_Precision_Time_Jump_Test Create fail\n");
	}
#endif
#if defined(WATCH_NOT_LEAP_SECOND_SET_TIME)
	if (NULL == m_xTimers_Detect_Leap_Second)
	{
		TIMER_APP_PRINTF("[TIMER_APP]: m_xTimers_Detect_Leap_Second Create fail\n");
	}
#endif
	if (NULL == m_xTimers_sport_record_detail)
	{
		TIMER_APP_PRINTF("[TIMER_APP]: m_xTimers_sport_record_detail Create fail\n");
	}
#if defined(WATCH_AUTO_BACK_HOME)
	if (NULL == m_xTimers_auto_back_home)
	{
		TIMER_APP_PRINTF("[TIMER_APP]: m_xTimers_auto_back_home Create fail\n");
	}
#endif
#if defined COD
		if (NULL == m_xTimers_vice_Pause)
	{
		TIMER_APP_PRINTF("[TIMER_APP]: m_xTimers_vice_Pasue Create fail\n");
	}

	if (NULL == m_xTimers_rt_cod) //失败后重新创建一次
	{
		DELAY_MS(1);
		m_xTimers_rt_cod = xTimerCreate("RT_COD",
												            ( 5000 / portTICK_PERIOD_MS ),
												            pdTRUE,
												            NULL,
												            vTimerCallback_cod);
	}

#endif
#ifdef WATCH_LCD_LPM
if (NULL == m_xTimers_lcd_lpm) //失败后重新创建一次
{
	DELAY_MS(1);
	m_xTimers_lcd_lpm = xTimerCreate("lcd_lpm",
													( 10000 / portTICK_PERIOD_MS ),
													pdTRUE,
													NULL,
													vTimerCallback_lcd_lpm);
}
#endif

#ifdef WATCH_COD_BSP
if (NULL == m_xTimers_lcd) //失败后重新创建一次
{
	DELAY_MS(1);
	m_xTimers_lcd = xTimerCreate("lcd",
													( 1200 / portTICK_PERIOD_MS ),
													pdTRUE,
													NULL,
													vTimerCallback_lcd);
}

#endif

}
void timer_app_uninit(void)
{
	if (pdPASS != xTimerDelete(m_xTimers_COUNTDOWN, TIMER_TICKS_TO_WAIT))
	{
		TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_COUNTDOWN Delete fail\n");
	}
	if (pdPASS != xTimerDelete(m_xTimers_HDR, TIMER_TICKS_TO_WAIT))
	{
		TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_HDR Delete fail\n");
	}
	if (pdPASS != xTimerDelete(m_xTimers_VO2max, TIMER_TICKS_TO_WAIT))
	{
		TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_VO2max Delete fail\n");
	}
	if (pdPASS != xTimerDelete(m_xTimers_VO2max_Countdown, TIMER_TICKS_TO_WAIT))
	{
		TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_VO2max_Countdown Delete fail\n");
	}
	if (pdPASS != xTimerDelete(m_xTimers_LOCK, TIMER_TICKS_TO_WAIT))
	{
		TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_LOCK Delete fail\n");
	}
	if (pdPASS != xTimerDelete(m_xTimers_UNLOCK_HINT, TIMER_TICKS_TO_WAIT))
	{
		TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_UNLOCK_HINT Delete fail\n");
	}
	if (pdPASS != xTimerDelete(m_xTimers_Step_Calibration, TIMER_TICKS_TO_WAIT))
	{
		TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_Step_Calibration Delete fail\n");
	}

#if defined WATCH_SIM_SPORT || defined WATCH_COM_SPORT
	if (pdPASS != xTimerDelete(m_xTimers_Pause,TIMER_TICKS_TO_WAIT))
	{
		TIMER_APP_PRINTF("[TIMER_APP]: m_xTimers_Pasue Delete fail\n");
	}
   

#endif

#if defined(WATCH_PRECISION_TIME_JUMP_TEST)
	if (pdPASS != xTimerDelete(m_xTimers_Precision_Time_Jump_Test, TIMER_TICKS_TO_WAIT))
	{
		TIMER_APP_PRINTF("[TIMER_APP]: m_xTimers_Precision_Time_Jump_Test Delete fail\n");
	}
#endif
#if defined(WATCH_NOT_LEAP_SECOND_SET_TIME)
	if (pdPASS != xTimerDelete(m_xTimers_Detect_Leap_Second, TIMER_TICKS_TO_WAIT))
	{
		TIMER_APP_PRINTF("[TIMER_APP]: m_xTimers_Detect_Leap_Second Delete fail\n");
	}
#endif
	if (pdPASS != xTimerDelete(m_xTimers_sport_record_detail, TIMER_TICKS_TO_WAIT))
	{
		TIMER_APP_PRINTF("[TIMER_APP]: m_xTimers_sport_record_detail Delete fail\n");
	}
#if defined(WATCH_AUTO_BACK_HOME)
	if (pdPASS != xTimerDelete(m_xTimers_auto_back_home, TIMER_TICKS_TO_WAIT))
	{
		TIMER_APP_PRINTF("[TIMER_APP]: m_xTimers_auto_back_home Delete fail\n");
	}
#endif

#ifdef COD 
   if (pdPASS != xTimerDelete(m_xTimers_vice_Pause,TIMER_TICKS_TO_WAIT))
	   {
		   TIMER_APP_PRINTF("[TIMER_APP]: m_xTimers_vice_Pasue Delete fail\n");
	   }
    if (pdPASS != xTimerDelete(m_xTimers_rt_cod, TIMER_TICKS_TO_WAIT))
    {
    	TIMER_APP_PRINTF("[TIMER_APP]: m_xTimers_auto_back_home Delete fail\n");
    }

	if (pdPASS != xTimerDelete(m_xTimers_cod_sensor, TIMER_TICKS_TO_WAIT))
	 {
		 TIMER_APP_PRINTF("[TIMER_APP]: m_xTimers_auto_back_home Delete fail\n");
	 }
	if (pdPASS != xTimerDelete(m_xTimers_timeout_back, TIMER_TICKS_TO_WAIT))
	 {
		 TIMER_APP_PRINTF("[TIMER_APP]: m_xTimers_auto_back_home Delete fail\n");
	 }
#endif
#ifdef WATCH_COD_BSP
if (pdPASS != xTimerDelete(m_xTimers_lcd, TIMER_TICKS_TO_WAIT))
{
	TIMER_APP_PRINTF("[TIMER_APP]: m_xTimers_auto_back_home Delete fail\n");
}
#endif

#ifdef WATCH_LCD_LPM
 if (pdPASS != xTimerDelete(m_xTimers_lcd_lpm, TIMER_TICKS_TO_WAIT))
 {
	 TIMER_APP_PRINTF("[TIMER_APP]: m_xTimers_auto_back_home Delete fail\n");
 }
#endif


#ifdef WATCH_LCD_ESD
 if (pdPASS != xTimerDelete(m_xTimers_read_lcd, TIMER_TICKS_TO_WAIT))
 {
	 TIMER_APP_PRINTF("[TIMER_APP]: m_xTimers_auto_back_home Delete fail\n");
 }
#endif




}
/*倒计时定时器 开始*/
void timer_app_countdown_start(void)
{
	if (NULL == m_xTimers_COUNTDOWN)
	{
		TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_COUNTDOWN Start fail\n");
	}
	else
	{
		 if(xTimerStart(m_xTimers_COUNTDOWN, TIMER_TICKS_TO_WAIT) != pdPASS)
		 {
			 counter = 2;
			 TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_COUNTDOWN Start success\n");
		 }
	}
}
/*倒计时定时器 停止*/
void timer_app_countdown_stop(bool is_isr)
{
	counter = 2;
	if(is_isr)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		if( xTimerStopFromISR( m_xTimers_COUNTDOWN, &xHigherPriorityTaskWoken ) != pdPASS )
		{
			TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_COUNTDOWN xTimerStopFromISR fail\n");
		}
		if( xHigherPriorityTaskWoken != pdFALSE )
		{
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}	
	}
	else{
		if (pdPASS != xTimerStop(m_xTimers_COUNTDOWN, TIMER_TICKS_TO_WAIT))
		{	
			TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_COUNTDOWN Stop fail\n");
		}
	}		
}
/*心率定时器 开始*/
void timer_app_hdr_start(void)
{
	if (NULL == m_xTimers_HDR)
	{
		TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_HDR fail\n");
	}
	else
	{
		 if(xTimerStart(m_xTimers_HDR, TIMER_TICKS_TO_WAIT) != pdPASS)
		 {
			 TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_HDR success\n");
		 }
	}
}
/*心率定时器 停止*/
void timer_app_hdr_stop(bool is_isr)
{
	if(is_isr)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		if( xTimerStopFromISR( m_xTimers_HDR, &xHigherPriorityTaskWoken ) != pdPASS )
		{
			TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_HDR xTimerStopFromISR fail\n");
		}
		if( xHigherPriorityTaskWoken != pdFALSE )
		{
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}	
	}
	else{
		if (pdPASS != xTimerStop(m_xTimers_HDR, TIMER_TICKS_TO_WAIT))
		{	
			TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_HDR Stop fail\n");
		}
	}		
}
/*最大摄氧量界面刷新定时器 开始*/
void timer_app_VO2max_start(void)
{
	if (NULL == m_xTimers_VO2max)
	{
		TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_VO2max fail\n");
	}
	else
	{
		 if(xTimerStart(m_xTimers_VO2max, TIMER_TICKS_TO_WAIT) != pdPASS)
		 {
			 TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_VO2max success\n");
		 }
	}
}
/*最大摄氧量界面刷新定时器 停止*/
void timer_app_VO2max_stop(bool is_isr)
{
	if(is_isr)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		if( xTimerStopFromISR( m_xTimers_VO2max, &xHigherPriorityTaskWoken ) != pdPASS )
		{
			TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_VO2max xTimerStopFromISR fail\n");
		}
		if( xHigherPriorityTaskWoken != pdFALSE )
		{
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}	
	}
	else{
		if (pdPASS != xTimerStop(m_xTimers_VO2max, TIMER_TICKS_TO_WAIT))
		{	
			TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_VO2max Stop fail\n");
		}
	}		
}
/*最大摄氧量定时器 开始*/
void timer_app_VO2max_Countdown_start(uint32_t time)
{
	countdown_counter = time;
	if (NULL == m_xTimers_VO2max_Countdown)
	{
		TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_VO2max_Countdown fail\n");
	}
	else
	{
		 if(xTimerStart(m_xTimers_VO2max_Countdown, TIMER_TICKS_TO_WAIT) != pdPASS)
		 {
			 TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_VO2max_Countdown success\n");
		 }
	}
}
/*最大摄氧量定时器 停止*/
void timer_app_VO2max_Countdown_stop(bool is_isr)
{
	if(is_isr)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		if( xTimerStopFromISR( m_xTimers_VO2max_Countdown, &xHigherPriorityTaskWoken ) != pdPASS )
		{
			TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_VO2max_Countdown xTimerStopFromISR fail\n");
		}
		if( xHigherPriorityTaskWoken != pdFALSE )
		{
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}	
	}
	else{
		if (pdPASS != xTimerStop(m_xTimers_VO2max_Countdown, TIMER_TICKS_TO_WAIT))
		{	
			TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_VO2max_Countdown Stop fail\n");
		}
	}		
}
/*锁屏定时器 开始*/
void timer_app_LOCK_start(void)
{
	if (NULL == m_xTimers_LOCK)
	{
		TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_LOCK fail\n");
	}
	else
	{
		 if(xTimerStart(m_xTimers_LOCK, TIMER_TICKS_TO_WAIT) != pdPASS)
		 {
			 TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_LOCK success\n");
		 }
	}
}
/*锁屏定时器 停止*/
void timer_app_LOCK_stop(bool is_isr)
{
	if(is_isr)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		if( xTimerStopFromISR( m_xTimers_LOCK, &xHigherPriorityTaskWoken ) != pdPASS )
		{
			TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_LOCK xTimerStopFromISR fail\n");
		}
		if( xHigherPriorityTaskWoken != pdFALSE )
		{
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}	
	}
	else{
		if (pdPASS != xTimerStop(m_xTimers_LOCK, TIMER_TICKS_TO_WAIT))
		{	
			TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_LOCK Stop fail\n");
		}
	}		
}
/*解锁失败自动返回定时器 开始*/
void timer_app_UNLOCK_HINT_start(void)
{
	if (NULL == m_xTimers_UNLOCK_HINT)
	{
		TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_UNLOCK_HINT fail\n");
	}
	else
	{
		 if(xTimerStart(m_xTimers_UNLOCK_HINT, TIMER_TICKS_TO_WAIT) != pdPASS)
		 {
			 TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_UNLOCK_HINT success\n");
		 }
	}
}
/*解锁失败自动返回定时器 停止*/
void timer_app_UNLOCK_HINT_stop(bool is_isr)
{
	if(is_isr)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		if( xTimerStopFromISR( m_xTimers_UNLOCK_HINT, &xHigherPriorityTaskWoken ) != pdPASS )
		{
			TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_UNLOCK_HINT xTimerStopFromISR fail\n");
		}
		if( xHigherPriorityTaskWoken != pdFALSE )
		{
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}	
	}
	else{
		if (pdPASS != xTimerStop(m_xTimers_UNLOCK_HINT, TIMER_TICKS_TO_WAIT))
		{	
			TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_UNLOCK_HINT Stop fail\n");
		}
	}		
}
/*步长校准界面刷新定时器 开始*/
void timer_app_step_calibration_start(void)
{
	if (NULL == m_xTimers_Step_Calibration)
	{
		TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_Step_Calibration fail\n");
	}
	else
	{
		 if(xTimerStart(m_xTimers_Step_Calibration, TIMER_TICKS_TO_WAIT) != pdPASS)
		 {
			 TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_Step_Calibration success\n");
		 }
	}
}
/*步长校准界面刷新定时器 停止*/
void timer_app_step_calibration_stop(bool is_isr)
{
	if(is_isr)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		if( xTimerStopFromISR( m_xTimers_Step_Calibration, &xHigherPriorityTaskWoken ) != pdPASS )
		{
			TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_Step_Calibration xTimerStopFromISR fail\n");
		}
		if( xHigherPriorityTaskWoken != pdFALSE )
		{
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}	
	}
	else{
		if (pdPASS != xTimerStop(m_xTimers_Step_Calibration, TIMER_TICKS_TO_WAIT))
		{	
			TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_Step_Calibration Stop fail\n");
		}
	}		
}

/*工具界面2S超时定时器 开始*/
void timer_app_tool_start(void)
{
	if (NULL == m_xTimers_Tool)
	{
		TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_Tool fail\n");
	}
	else
	{
		 if(xTimerStart(m_xTimers_Tool, TIMER_TICKS_TO_WAIT) != pdPASS)
		 {
			 TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_Tool success\n");
		 }
	}
}
/*工具界面2S超时定时器 停止*/
void timer_app_tool_stop(bool is_isr)
{
	if(is_isr)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		if( xTimerStopFromISR( m_xTimers_Tool, &xHigherPriorityTaskWoken ) != pdPASS )
		{
			TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_Tool xTimerStopFromISR fail\n");
		}
		if( xHigherPriorityTaskWoken != pdFALSE )
		{
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}	
	}
	else{
		if (pdPASS != xTimerStop(m_xTimers_Tool, TIMER_TICKS_TO_WAIT))
		{	
			TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_Tool Stop fail\n");
		}
	}		
}


#if defined(WATCH_PRECISION_TIME_JUMP_TEST)
/*精密授时时间跳变测试定时器 开始*/
void timer_app_precision_time_jump_test_start(void)
{
	if (NULL == m_xTimers_Precision_Time_Jump_Test)
	{
		TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_Precision_Time_Jump_Test fail\n");
	}
	else
	{
		 if(xTimerStart(m_xTimers_Precision_Time_Jump_Test, TIMER_TICKS_TO_WAIT) != pdPASS)
		 {
			 TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_Precision_Time_Jump_Test success\n");
		 }
	}
}
/*精密授时时间跳变测试定时器 停止*/
void timer_app_precision_time_jump_test_stop(bool is_isr)
{
	if(is_isr)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		if( xTimerStopFromISR( m_xTimers_Precision_Time_Jump_Test, &xHigherPriorityTaskWoken ) != pdPASS )
		{
			TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_Tool m_xTimers_Precision_Time_Jump_Test fail\n");
		}
		if( xHigherPriorityTaskWoken != pdFALSE )
		{
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}	
	}
	else{
		if (pdPASS != xTimerStop(m_xTimers_Precision_Time_Jump_Test, TIMER_TICKS_TO_WAIT))
		{	
			TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_Precision_Time_Jump_Test Stop fail\n");
		}
	}		
}
#endif


#if defined(WATCH_NOT_LEAP_SECOND_SET_TIME)
/*闰秒检测定时器 开始*/
void timer_app_detect_leap_second_start(void)
{
	s_detect_leap_second_count = 0;
	if (NULL == m_xTimers_Detect_Leap_Second)
	{
		TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_Detect_Leap_Second fail\n");
	}
	else
	{
		 if(xTimerStart(m_xTimers_Detect_Leap_Second, TIMER_TICKS_TO_WAIT) != pdPASS)
		 {
			 TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_Detect_Leap_Second success\n");
		 }
	}
}
/*闰秒检测定时器 停止*/
void timer_app_detect_leap_second_stop(bool is_isr)
{
	if(is_isr)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		if( xTimerStopFromISR( m_xTimers_Detect_Leap_Second, &xHigherPriorityTaskWoken ) != pdPASS )
		{
			TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_Tool m_xTimers_Detect_Leap_Second fail\n");
		}
		if( xHigherPriorityTaskWoken != pdFALSE )
		{
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}	
	}
	else{
		if (pdPASS != xTimerStop(m_xTimers_Detect_Leap_Second, TIMER_TICKS_TO_WAIT))
		{	
			TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_Detect_Leap_Second Stop fail\n");
		}
	}		
}
#endif
/*运动详情总结界面超时定时器 开始*/
void timer_app_sport_record_detail_start(void)
{
	TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_Tool timer_app_sport_record_detail_start ----------\n");
	if (NULL == m_xTimers_sport_record_detail)
	{
		TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_sport_record_detail fail\n");
	}
	else
	{
		 if(xTimerStart(m_xTimers_sport_record_detail, TIMER_TICKS_TO_WAIT) != pdPASS)
		 {
			 TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_sport_record_detail success\n");
		 }
	}
}
/*运动详情总结界面超时定时器 停止*/
void timer_app_sport_record_detail_stop(bool is_isr)
{
	TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_Tool timer_app_sport_record_detail_stop ++++++++++++\n");
	if(is_isr)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		if( xTimerStopFromISR( m_xTimers_sport_record_detail, &xHigherPriorityTaskWoken ) != pdPASS )
		{
			TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_Tool m_xTimers_sport_record_detail fail\n");
		}
		if( xHigherPriorityTaskWoken != pdFALSE )
		{
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}	
	}
	else{
		if (pdPASS != xTimerStop(m_xTimers_sport_record_detail, TIMER_TICKS_TO_WAIT))
		{	
			TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_sport_record_detail Stop fail\n");
		}
	}		
}
#if defined(WATCH_AUTO_BACK_HOME)
/*10分钟无任何操作（无按键操作）返回待机界面定时器 开始*/
void timer_app_auto_back_home_start(void)
{
//	TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_Tool timer_app_auto_back_home_start ----------\n");
	if (NULL == m_xTimers_auto_back_home)
	{
		TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_auto_back_home fail\n");
	}
	else
	{
		 if(xTimerStart(m_xTimers_auto_back_home, TIMER_TICKS_TO_WAIT) != pdPASS)
		 {
			 TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_auto_back_home success\n");
		 }
	}
}
/*10分钟无任何操作（无按键操作）返回待机界面定时器 停止*/
void timer_app_auto_back_home_stop(bool is_isr)
{
//	TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_Tool timer_app_auto_back_home_stop ++++++++++++\n");
	if(is_isr)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		if( xTimerStopFromISR( m_xTimers_auto_back_home, &xHigherPriorityTaskWoken ) != pdPASS )
		{
			TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_Tool m_xTimers_auto_back_home fail\n");
		}
		if( xHigherPriorityTaskWoken != pdFALSE )
		{
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}	
	}
	else{
		if (pdPASS != xTimerStop(m_xTimers_auto_back_home, TIMER_TICKS_TO_WAIT))
		{	
			TIMER_APP_PRINTF("[TIMER_APP]:m_xTimers_auto_back_home Stop fail\n");
		}
	}		
}
#endif




