/*This progam mainly handle three events that includes timer: buzzer on/off, motor on/off 
and switching screens between previous screen state and current screen state*/

/*It includes the function of initialize/un-initialize, start, stop, and callback for the three
events listed above: buzzer, moter, and display, and one that combines buzzer & moter together*/

#include "time_notify.h"
#include "timer_config.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "portmacro.h"
#include "portable.h"
#include "event_groups.h"
#include "semphr.h"
#include "com_sport.h"

#include "task_display.h"
#include "drv_alarm.h"
#include "drv_motor.h"
#include "drv_buzzer.h"
#include "drv_light.h"
#include "com_data.h"
#include "lib_error.h"
#include "gui_system.h"


#define TIME_NOTIFY_LOG_ENABLED 0

#if DEBUG_ENABLED == 1 && TIME_NOTIFY_LOG_ENABLED == 1
	#define TIME_NOTIFY_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define TIME_NOTIFY_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define TIME_NOTIFY_WRITESTRING(...)
	#define TIME_NOTIFY_PRINTF(...)		        
#endif

extern SetValueStr SetValue;



#define OSTIMER_WAIT_FOR_QUEUE           100
static TimerHandle_t m_timer_display; 
static TimerHandle_t m_timer_motor; 
static TimerHandle_t m_timer_buzzer;
static TimerHandle_t m_timer_remind;

static TimerHandle_t m_timer_backlight; 
#define TIMER_INTERVAL_MS 5000



static struct {
	uint32_t timeout_msec;
	uint32_t repetition;
}m_display = {.timeout_msec = 1000, .repetition = 1};


#define TIME_NOTIFY_MOTOR_EVT_DISABLE       0
#define TIME_NOTIFY_MOTOR_EVT_ENABLE        1
static struct {
	uint32_t interval;
	uint32_t window;
	uint32_t repetition;
	uint8_t evt;
}m_motor = {.interval = 1000, .window = 500, .repetition = 5, .evt = TIME_NOTIFY_MOTOR_EVT_DISABLE};

#define TIME_NOTIFY_BUZZER_EVT_DISABLE       0
#define TIME_NOTIFY_BUZZER_EVT_ENABLE        1
static struct {
	uint32_t interval;
	uint32_t window;
	uint32_t repetition;
	uint8_t evt;
}m_buzzer = {.interval = 1000, .window = 500, .repetition = 5, .evt = TIME_NOTIFY_BUZZER_EVT_DISABLE};

#define TIME_NOTIFY_REMIND_EVT_DISABLE       0
#define TIME_NOTIFY_REMIND_EVT_ENABLE        1
static struct {
	uint32_t interval;
	uint32_t window;
    uint32_t motor_interval;
	uint32_t motor_window;
    uint32_t buzzer_interval;
	uint32_t buzzer_window;
	uint32_t repetition;
	uint8_t evt;
    bool is_isr;
}m_remind = {.interval = 1000, .window = 500,.motor_interval = 0, .motor_window = 0,.buzzer_interval = 0,.buzzer_window = 0,.repetition = 5, .evt = TIME_NOTIFY_BUZZER_EVT_DISABLE,.is_isr = false};



#define TIME_NOTIFY_BACKLIGHT_EVT_DISABLE       0
#define TIME_NOTIFY_BACKLIGHT_EVT_ENABLE        1
static struct {
	uint32_t timeout_msec;
	uint8_t evt;
}m_backlight = {.timeout_msec = 10000, .evt = TIME_NOTIFY_BACKLIGHT_EVT_DISABLE};

static uint8_t m_remind_motor_flag = 0;


static void callback_display(TimerHandle_t xTimer)
{
	TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:callback_display timeout m_display.repetition --> %d, ScreenState --> %d\n",m_display.repetition,ScreenState);
	if(m_display.repetition)
	{
		m_display.repetition--;
		if( xTimerChangePeriod( m_timer_display, m_display.timeout_msec / portTICK_PERIOD_MS, OSTIMER_WAIT_FOR_QUEUE ) == pdPASS )
		{
			TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:xTimerChangePeriod success\n");
			DISPLAY_MSG  msg = {0,0};
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
		}
		else
		{
			ERR_HANDLER(ERR_NO_MEM);
			TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:xTimerChangePeriod fail\n");
		}				
	}
	else{
		DISPLAY_MSG  msg = {0,0};
		ScreenState = ScreenStateSave;
		msg.cmd = MSG_DISPLAY_SCREEN;
		xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			
	}	
	
	
	
}

void timer_notify_display_init(void)
{

	m_timer_display = xTimerCreate("NOT_DISPLAY",
												 ( TIMER_INTERVAL_MS / portTICK_PERIOD_MS ),
												 pdFALSE,
												 NULL,
												 callback_display);
	if (NULL == m_timer_display)
	{
		ERR_HANDLER(ERR_NO_MEM);
		TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:xTimerCreate fail\n");
	}
}

void timer_notify_display_uninit(void)
{

	if (pdPASS != xTimerDelete(m_timer_display, OSTIMER_WAIT_FOR_QUEUE))
	{
		ERR_HANDLER(ERR_NO_MEM);
		TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:xTimerDelete fail\n");
	}

}


void timer_notify_display_start(uint32_t timeout_msec, uint32_t repetition, bool is_isr)
{
	timer_notify_display_stop(is_isr);
	
	m_display.timeout_msec = timeout_msec;
	m_display.repetition = repetition;
	
	if(is_isr)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		if( xTimerChangePeriodFromISR( m_timer_display,  timeout_msec / portTICK_PERIOD_MS ,&xHigherPriorityTaskWoken ) == pdPASS )
		{			
			m_display.repetition--;
		}
		else
		{
			ERR_HANDLER(ERR_NO_MEM);
		}
		if( xHigherPriorityTaskWoken != pdFALSE )
		{
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}	
	}
	else{
	
		if( xTimerChangePeriod( m_timer_display, timeout_msec / portTICK_PERIOD_MS, OSTIMER_WAIT_FOR_QUEUE ) == pdPASS )
		{
			m_display.repetition--;
		}
		else
		{
			ERR_HANDLER(ERR_NO_MEM);
			TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:xTimerChangePeriod fail\n");
		}
	}			
	
	
	
		
}


void timer_notify_display_stop(bool is_isr)
{
	if(is_isr)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		if( xTimerStopFromISR( m_timer_display, &xHigherPriorityTaskWoken ) != pdPASS )
		{
			ERR_HANDLER(ERR_NO_MEM);
			TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:xTimerStopFromISR fail\n");
		}
		if( xHigherPriorityTaskWoken != pdFALSE )
		{
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}	
	}
	else{
	
		if (pdPASS != xTimerStop(m_timer_display, OSTIMER_WAIT_FOR_QUEUE))
		{	
			ERR_HANDLER(ERR_NO_MEM);
			TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:xTimerStop fail\n");
		}
	}		
	

}



static void callback_motor(TimerHandle_t xTimer)
{
	TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:callback_motor timeout\n");
	if(m_motor.evt == TIME_NOTIFY_MOTOR_EVT_ENABLE)
	{	
	TIME_NOTIFY_PRINTF("[TIME_NOTIFY]>Callback Motor>m_motor.evt == TIME_NOTIFY_MOTOR_EVT_ENABLE\n");		
		if(m_motor.repetition)
		{
			m_motor.repetition--;
			if( xTimerChangePeriod( m_timer_motor, (m_motor.interval - m_motor.window) / portTICK_PERIOD_MS, OSTIMER_WAIT_FOR_QUEUE ) == pdPASS )
			{
				TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:TIME_NOTIFY_MOTOR_EVT_DISABLE\n");
				m_motor.evt = TIME_NOTIFY_MOTOR_EVT_DISABLE;
				drv_motor_disable();
				TIME_NOTIFY_PRINTF("[TIME_NOTIFY]>Callback Motor>xTimerChangePeriod=pdPASS>drv_motor_disable\n");
			}
			else
			{
				ERR_HANDLER(ERR_NO_MEM);
				TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:xTimerChangePeriod fail\n");
			}				
		}
		else{
			TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:TIME_NOTIFY_MOTOR_EVT_DISABLE\n");
				m_motor.evt = TIME_NOTIFY_MOTOR_EVT_DISABLE;
				drv_motor_disable();
				TIME_NOTIFY_PRINTF("[TIME_NOTIFY]>Callback Motor>xTimerChangePeriod=pdFAIL>drv_motor_disable\n");			
		}
	}
	else if(m_motor.evt == TIME_NOTIFY_MOTOR_EVT_DISABLE)
	{
		TIME_NOTIFY_PRINTF("[TIME_NOTIFY]>Callback Motor>m_motor.evt == TIME_NOTIFY_MOTOR_EVT_DISABLE\n");
		if(m_motor.repetition)
		{
			m_motor.repetition--;
			if( xTimerChangePeriod( m_timer_motor, m_motor.window / portTICK_PERIOD_MS, OSTIMER_WAIT_FOR_QUEUE ) == pdPASS )
			{
				TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:TIME_NOTIFY_MOTOR_EVT_ENABLE\n");
				m_motor.evt = TIME_NOTIFY_MOTOR_EVT_ENABLE;
				drv_motor_enable();

			}
			else
			{
				ERR_HANDLER(ERR_NO_MEM);
				TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:xTimerChangePeriod fail\n");
			}				
		}
		else{
			TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:TIME_NOTIFY_MOTOR_EVT_DISABLE\n");
				m_motor.evt = TIME_NOTIFY_MOTOR_EVT_DISABLE;
				drv_motor_disable();	
				TIME_NOTIFY_PRINTF("[TIME_NOTIFY]>Callback Motor>m_motor.evt == TIME_NOTIFY_MOTOR_EVT_DISABLE>m_motor.repetition=0\n");				
		}
	}
}

void timer_notify_motor_init(void)
{

	m_timer_motor = xTimerCreate("NOT_MOTOR",
												 ( TIMER_INTERVAL_MS / portTICK_PERIOD_MS ),
												 pdFALSE,
												 NULL,
												 callback_motor);
	if (NULL == m_timer_motor)
	{
		ERR_HANDLER(ERR_NO_MEM);
		TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:xTimerCreate fail\n");
	}
}



void timer_notify_motor_uninit(void)
{

	if (pdPASS != xTimerDelete(m_timer_motor, OSTIMER_WAIT_FOR_QUEUE))
	{
		ERR_HANDLER(ERR_NO_MEM);
		TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:xTimerDelete fail\n");
	}

}
/*函数说明:马达震动开始
备注:勿扰模式下type有多种定义 
1.正常待机模式下跟随系统设置控制、受勿扰模式控制
2.运动功能模式下跟随系统设置控制、不受勿扰模式控制
3.闹钟模式下不跟随系统设置控制、不受勿扰模式控制
4.特殊功能下不跟随系统控制、不受勿扰模式控制(如SOS、寻找腕表等)
5.系统时间到点后的通知提醒/完成类提醒等，跟随系统设置控制、受勿扰模式控制，如训练计划、消息通知、目标达成、基础心率、低电量
6.直接响应、不跟随系统设置控制、不受勿扰模式控制(一般用于测试或开关机)
7.静音模式
8.来电提醒模式不跟随系统设置控制,受勿扰模式控制
*/
void timer_notify_motor_start(uint32_t interval, uint32_t window, uint32_t repetition, bool is_isr, uint8_t type)
{
	bool is_need_motor = false;
	
	timer_notify_motor_stop(is_isr);
	
	if(system_silent_mode_get() == 1)
	{//勿扰模式打开且在勿扰时间内
		switch(type)
		{
			case NOTIFY_MODE_SPORTS:
			case NOTIFY_MODE_TOOLS:
			{
				#ifndef COD
					if(SetValue.SwVibration == 1)
					{//震动
						is_need_motor = true;
					}
					else
					{
						is_need_motor = false;
					}
				#else
				is_need_motor = false;
				#endif
			}
				break;
			case NOTIFY_MODE_ALARM:
			case NOTIFY_MODE_SPECIFY_FUNC:
			case NOTIFY_MODE_DIRECT_RESPONSE:
			{
				is_need_motor = true;
			}
				break;
			case NOTIFY_MODE_NORMAL:
			case NOTIFY_MODE_SYSTEM_TIME_REACH:
			case NOTIFY_MODE_MUTE:
			case NOTIFY_MODE_PHONE_CALL:
			{
				is_need_motor = false;
			}
				break;
			default:
				break;
		}
	}
	else
	{
		switch(type)
		{
		#ifdef COD 
			case NOTIFY_MODE_NORMAL:
			case NOTIFY_MODE_SYSTEM_TIME_REACH:
			{

				if(SetValue.SwVibration == 1)
				{//震动
					is_need_motor = true;
				}
				else
				{
					is_need_motor = false;
				}	
			}
			
				break;
			case NOTIFY_MODE_SPORTS:
			case NOTIFY_MODE_TOOLS:
			{
				is_need_motor = true;
			}
			    break;
		#else
			case NOTIFY_MODE_NORMAL:
			case NOTIFY_MODE_SYSTEM_TIME_REACH:
			case NOTIFY_MODE_SPORTS:
			case NOTIFY_MODE_TOOLS:
			{

				if(SetValue.SwVibration == 1)
				{//震动
					is_need_motor = true;
				}
				else
				{
					is_need_motor = false;
				}	
			}
			
				break;
		#endif
			case NOTIFY_MODE_ALARM:
			case NOTIFY_MODE_SPECIFY_FUNC:
			case NOTIFY_MODE_DIRECT_RESPONSE:
			case NOTIFY_MODE_PHONE_CALL:
			{
				is_need_motor = true;
			}
				break;
			case NOTIFY_MODE_MUTE:
			{
				is_need_motor = false;
			}
				break;
			default:
				break;
		}
	}

	if(is_need_motor)
	{
		m_motor.interval = interval;
		m_motor.window = window;
		if(repetition>1)
		{
			m_motor.repetition = repetition*2;
		}else{
			m_motor.repetition = 1;
		}	
		
		if(is_isr)
		{
			BaseType_t xHigherPriorityTaskWoken = pdFALSE;
			if( xTimerChangePeriodFromISR( m_timer_motor,  window / portTICK_PERIOD_MS ,&xHigherPriorityTaskWoken ) == pdPASS )
			{
				m_motor.repetition--;
				m_motor.evt = TIME_NOTIFY_MOTOR_EVT_ENABLE;
				drv_motor_enable();			
			}
			else
			{
				ERR_HANDLER(ERR_NO_MEM);
			}		
			if( xHigherPriorityTaskWoken != pdFALSE )
			{
				portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
			}	
		}
		else{
		
			if( xTimerChangePeriod( m_timer_motor, window / portTICK_PERIOD_MS, OSTIMER_WAIT_FOR_QUEUE ) == pdPASS )
			{
				TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:TIME_NOTIFY_MOTOR_EVT_ENABLE\n");
				m_motor.repetition--;
				m_motor.evt = TIME_NOTIFY_MOTOR_EVT_ENABLE;
				drv_motor_enable();
				TIME_NOTIFY_PRINTF("[TIME_NOTIFY]>Motor Start>drv_motor_enable\n");
			}
			else
			{
				ERR_HANDLER(ERR_NO_MEM);
				TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:xTimerChangePeriod fail\n");
			}	
		}
	}
}


void timer_notify_motor_stop(bool is_isr)
{
	TIME_NOTIFY_PRINTF("[TIME_NOTIFY]>Motor Stop>drv_motor_disable\n");
//	drv_motor_disable();
	
	if(is_isr)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		if( xTimerStopFromISR( m_timer_motor, &xHigherPriorityTaskWoken ) != pdPASS )
		{
			ERR_HANDLER(ERR_NO_MEM);
			TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:xTimerStopFromISR fail\n");
		}
		if( xHigherPriorityTaskWoken != pdFALSE )
		{
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}	
	}
	else
	{
	
		if (pdPASS != xTimerStop(m_timer_motor, OSTIMER_WAIT_FOR_QUEUE))
		{	
			ERR_HANDLER(ERR_NO_MEM);
			TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:xTimerStop fail\n");
		}	
	}
	drv_motor_disable();

}



static void callback_buzzer(TimerHandle_t xTimer)
{
	TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:callback_buzzer timeout\n");
	if(m_buzzer.evt == TIME_NOTIFY_BUZZER_EVT_ENABLE)
	{		
		if(m_buzzer.repetition)
		{
			m_buzzer.repetition--;
			if( xTimerChangePeriod( m_timer_buzzer, (m_buzzer.interval - m_buzzer.window) / portTICK_PERIOD_MS, OSTIMER_WAIT_FOR_QUEUE ) == pdPASS )
			{
				TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:TIME_NOTIFY_BUZZER_EVT_DISABLE\n");
				m_buzzer.evt = TIME_NOTIFY_BUZZER_EVT_DISABLE;
				drv_buzzer_disable();
			}
			else
			{
				ERR_HANDLER(ERR_NO_MEM);
				TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:xTimerChangePeriod fail\n");
			}				
		}
		else{
				TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:TIME_NOTIFY_BUZZER_EVT_DISABLE\n");
				m_buzzer.evt = TIME_NOTIFY_BUZZER_EVT_DISABLE;
				drv_buzzer_disable();		
		}
	}
	else if(m_buzzer.evt == TIME_NOTIFY_MOTOR_EVT_DISABLE)
	{
		if(m_buzzer.repetition)
		{
			m_buzzer.repetition--;
			if( xTimerChangePeriod( m_timer_buzzer, m_buzzer.window / portTICK_PERIOD_MS, OSTIMER_WAIT_FOR_QUEUE ) == pdPASS )
			{
				TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:TIME_NOTIFY_BUZZER_EVT_ENABLE\n");
				m_buzzer.evt = TIME_NOTIFY_BUZZER_EVT_ENABLE;
				drv_buzzer_enable();
			}
			else
			{
				ERR_HANDLER(ERR_NO_MEM);
				TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:xTimerChangePeriod fail\n");
			}				
		}
		else{
			TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:TIME_NOTIFY_BUZZER_EVT_DISABLE\n");
			m_buzzer.evt = TIME_NOTIFY_BUZZER_EVT_DISABLE;
			drv_buzzer_disable();		
		}
	}
}

void timer_notify_buzzer_init(void)
{

	m_timer_buzzer = xTimerCreate("NOT_BUZZER",
												 ( TIMER_INTERVAL_MS / portTICK_PERIOD_MS ),
												 pdFALSE,
												 NULL,
												 callback_buzzer);
	if (NULL == m_timer_buzzer)
	{
		ERR_HANDLER(ERR_NO_MEM);
		TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:xTimerCreate fail\n");
	}

}



void timer_notify_buzzer_uninit(void)
{

	if (pdPASS != xTimerDelete(m_timer_buzzer, OSTIMER_WAIT_FOR_QUEUE))
	{
		ERR_HANDLER(ERR_NO_MEM);
		TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:xTimerDelete fail\n");
	}

}

//按键音
void timer_notify_keytone_start(void)
{
	if(m_timer_buzzer == NULL)
	{
		return;
	}
	
	if(xTimerIsTimerActive(m_timer_buzzer) != pdTRUE)
	{
		//如果蜂鸣器定时器空闲时，响蜂鸣器
		timer_notify_buzzer_start(100,100,1,false,NOTIFY_MODE_NORMAL);
	}
}

/*函数说明:蜂鸣器声音开始
备注:勿扰模式下type有多种定义
1.正常待机模式下跟随系统设置控制、受勿扰模式控制
2.运动功能模式下跟随系统设置控制、不受勿扰模式控制
3.闹钟模式下不跟随系统设置控制、不受勿扰模式控制
4.特殊功能下不跟随系统控制、不受勿扰模式控制(如SOS、寻找腕表等)
5.系统时间到点后的通知提醒/完成类提醒等，跟随系统设置控制、受勿扰模式控制，如训练计划、消息通知、目标达成、基础心率、低电量
6.直接响应、不跟随系统设置控制、不受勿扰模式控制(一般用于测试或开关机)
7.静音模式
*/
void timer_notify_buzzer_start(uint32_t interval, uint32_t window, uint32_t repetition, bool is_isr, uint8_t type)
{
	bool is_need_buzzer = false;
	timer_notify_buzzer_stop(is_isr);

	if(system_silent_mode_get() == 1)
	{//勿扰模式打开且在勿扰时间内
		switch(type)
		{
			case NOTIFY_MODE_SPORTS:
			case NOTIFY_MODE_TOOLS:
			{
				#ifndef COD
				if(SetValue.SwBuzzer == 1)
				{//震动
					is_need_buzzer = true;
				}
				else
				{
					is_need_buzzer = false;
				}
				#else
				is_need_buzzer = false;
				#endif
			}
				break;
			case NOTIFY_MODE_ALARM:
			case NOTIFY_MODE_SPECIFY_FUNC:
			case NOTIFY_MODE_DIRECT_RESPONSE:
			{
				is_need_buzzer = true;
			}
				break;
			case NOTIFY_MODE_NORMAL:
			case NOTIFY_MODE_SYSTEM_TIME_REACH:
			case NOTIFY_MODE_MUTE:
			case NOTIFY_MODE_PHONE_CALL:
			{
				is_need_buzzer = false;
			}
				break;
			default:
				break;
		}
	}
	else
	{
		switch(type)
		{
		#ifdef COD 
			case NOTIFY_MODE_NORMAL:
			case NOTIFY_MODE_SYSTEM_TIME_REACH:
			{
				if(SetValue.SwBuzzer == 1)
				{//震动
					is_need_buzzer = true;
				}
				else
				{
					is_need_buzzer = false;
				}
			}
				break;
			case NOTIFY_MODE_SPORTS:
			case NOTIFY_MODE_TOOLS:
			{
				is_need_buzzer = true;
			}
			break;
		#else
			case NOTIFY_MODE_NORMAL:
			case NOTIFY_MODE_SYSTEM_TIME_REACH:
			case NOTIFY_MODE_SPORTS:
			case NOTIFY_MODE_TOOLS:
			{
				if(SetValue.SwBuzzer == 1)
				{//震动
					is_need_buzzer = true;
				}
				else
				{
					is_need_buzzer = false;
				}
			}
				break;
		#endif
			case NOTIFY_MODE_ALARM:
			case NOTIFY_MODE_SPECIFY_FUNC:
			case NOTIFY_MODE_DIRECT_RESPONSE:
			case NOTIFY_MODE_PHONE_CALL:
			{
				is_need_buzzer = true;
			}
				break;
			case NOTIFY_MODE_MUTE:
			{
				is_need_buzzer = false;
			}
				break;
			default:
				break;
		}
	}	
	if(is_need_buzzer)
	{
		m_buzzer.interval = interval;
		m_buzzer.window = window;
		if(repetition>1)
		{
			m_buzzer.repetition = repetition*2;
		}else{
			m_buzzer.repetition = 1;
		}
		
		if(is_isr)
		{
			BaseType_t xHigherPriorityTaskWoken = pdFALSE;
			if( xTimerChangePeriodFromISR( m_timer_buzzer,  window / portTICK_PERIOD_MS ,&xHigherPriorityTaskWoken ) == pdPASS )
			{
				m_buzzer.repetition--;
				m_buzzer.evt = TIME_NOTIFY_BUZZER_EVT_ENABLE;
				drv_buzzer_enable();		
			}		
			else
			{
				ERR_HANDLER(ERR_NO_MEM);
			}
			if( xHigherPriorityTaskWoken != pdFALSE )
			{
				portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
			}	
		}
		else{
		
			if( xTimerChangePeriod( m_timer_buzzer, window / portTICK_PERIOD_MS, OSTIMER_WAIT_FOR_QUEUE ) == pdPASS )
			{
				TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:TIME_NOTIFY_BUZZER_EVT_ENABLE\n");
				m_buzzer.repetition--;
				m_buzzer.evt = TIME_NOTIFY_BUZZER_EVT_ENABLE;
				drv_buzzer_enable();		
			}
			else
			{
				ERR_HANDLER(ERR_NO_MEM);
				TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:xTimerChangePeriod fail\n");
			}	
		}		
	}
}


void timer_notify_buzzer_stop(bool is_isr)
{
//	drv_buzzer_disable();	
	
	if(is_isr)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		if( xTimerStopFromISR( m_timer_buzzer, &xHigherPriorityTaskWoken ) != pdPASS )
		{
			ERR_HANDLER(ERR_NO_MEM);
			TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:xTimerStopFromISR fail\n");
		}
		if( xHigherPriorityTaskWoken != pdFALSE )
		{
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}	
	}
	else
	{
	
		if (pdPASS != xTimerStop(m_timer_buzzer, OSTIMER_WAIT_FOR_QUEUE))
		{	
			ERR_HANDLER(ERR_NO_MEM);
			TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:xTimerStop fail\n");
		}
	}	
	drv_buzzer_disable();	
}

static void callback_remind(TimerHandle_t xTimer)
{
	TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:callback_motor timeout\n");
	if(m_remind.evt == TIME_NOTIFY_REMIND_EVT_ENABLE)
	{		
		if(m_remind.repetition)
		{
			m_remind.repetition--;
			if( xTimerChangePeriod( m_timer_remind, (m_remind.interval - m_remind.window) / portTICK_PERIOD_MS, OSTIMER_WAIT_FOR_QUEUE ) == pdPASS )
			{
				TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:TIME_NOTIFY_MOTOR_EVT_DISABLE\n");
				m_remind.evt = TIME_NOTIFY_REMIND_EVT_DISABLE;
                timer_notify_remind_motor_stop();
				
				if(m_remind.repetition == 0)
				{
					//提醒结束，退出闹钟界面
					if(ScreenState == DISPLAY_SCREEN_NOTIFY_ALARM)
					{
						ScreenState = ScreenStateAlarm;
						
						DISPLAY_MSG  msg = {0,0};
						msg.cmd = MSG_DISPLAY_SCREEN;
						xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
					}
					else if(ScreenState >= DISPLAY_SCREEN_NOTIFY)
					{
						ScreenStateSave = ScreenStateAlarm;
					}
				}
			}
			else
			{
				ERR_HANDLER(ERR_NO_MEM);
				TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:xTimerChangePeriod fail\n");
			}				
		}
		else{
			TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:TIME_NOTIFY_MOTOR_EVT_DISABLE\n");
				m_remind.evt = TIME_NOTIFY_REMIND_EVT_DISABLE;
                timer_notify_remind_motor_stop();
		}
	}
	else if(m_remind.evt == TIME_NOTIFY_REMIND_EVT_DISABLE)
	{
		if(m_remind.repetition)
		{
			m_remind.repetition--;
			if( xTimerChangePeriod( m_timer_remind, m_remind.window / portTICK_PERIOD_MS, OSTIMER_WAIT_FOR_QUEUE ) == pdPASS )
			{
				TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:TIME_NOTIFY_MOTOR_EVT_ENABLE\n");
				m_remind.evt = TIME_NOTIFY_REMIND_EVT_ENABLE;
				m_remind_motor_flag = 0;
			    if (m_remind.motor_interval != 0)
                {
                    timer_notify_motor_start(m_remind.motor_interval,m_remind.motor_window,(m_remind.window/m_remind.motor_interval),m_remind.is_isr,NOTIFY_MODE_ALARM);
                } 
                if (m_remind.buzzer_interval != 0)
                {
                    timer_notify_buzzer_start(m_remind.buzzer_interval,m_remind.buzzer_window,(m_remind.window/m_remind.buzzer_interval),m_remind.is_isr,NOTIFY_MODE_ALARM);
                }
				
				if((ScreenState != DISPLAY_SCREEN_NOTIFY_ALARM)
					&& (ScreenState != DISPLAY_SCREEN_NOTIFY_BAT_CHG) 
					&& time_notify_is_ok())
				{
					//显示闹钟界面
					if(ScreenState < DISPLAY_SCREEN_NOTIFY)
					{
						ScreenStateAlarm = ScreenState;
					}
					else
					{
						if(ScreenStateSave != DISPLAY_SCREEN_NOTIFY_ALARM)
						{
							ScreenStateAlarm = ScreenStateSave;
						}
					}
					ScreenState = DISPLAY_SCREEN_NOTIFY_ALARM;
					DISPLAY_MSG  msg = {0,0};
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				}
			}
			else
			{
				ERR_HANDLER(ERR_NO_MEM);
				TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:xTimerChangePeriod fail\n");
			}				
		}
		else{
			TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:TIME_NOTIFY_MOTOR_EVT_DISABLE\n");
				m_remind.evt = TIME_NOTIFY_REMIND_EVT_DISABLE;
				timer_notify_remind_motor_stop();		
		}
	}
}




void timer_notify_remind_init(void)
{

	m_timer_remind = xTimerCreate("NOT_REMIND",
												 ( TIMER_INTERVAL_MS / portTICK_PERIOD_MS ),
												 pdFALSE,
												 NULL,
												 callback_remind);
	if (NULL == m_timer_remind)
	{
		ERR_HANDLER(ERR_NO_MEM);
		TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:xTimerCreate fail\n");
	}

}

void timer_notify_remind_uninit(void)
{

	if (pdPASS != xTimerDelete(m_timer_remind, OSTIMER_WAIT_FOR_QUEUE))
	{
		ERR_HANDLER(ERR_NO_MEM);
		TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:xTimerDelete fail\n");
	}

}

//重新振动和蜂鸣器
void timer_notify_remind_motor_restart(void)
{
	if(m_remind_motor_flag != 0)
	{
		//闹钟已振动结束
		return;
	}
	
	if (m_remind.motor_interval != 0)
	{
		timer_notify_motor_start(m_remind.motor_interval,m_remind.motor_window,(m_remind.window/m_remind.motor_interval),false,NOTIFY_MODE_ALARM);
	}
	
	if (m_remind.buzzer_interval != 0)
	{
		timer_notify_buzzer_start(m_remind.buzzer_interval,m_remind.buzzer_window,(m_remind.window/m_remind.buzzer_interval),false,NOTIFY_MODE_ALARM);
	}
}

//停止振动马达和蜂鸣器
void timer_notify_remind_motor_stop(void)
{
	m_remind_motor_flag = 1;
	
	if(ScreenState == DISPLAY_SCREEN_NOTIFY_ALARM)
	{
		if (m_remind.motor_interval != 0)
		{
			timer_notify_motor_stop(m_remind.is_isr);
		}
		
		if (m_remind.buzzer_interval != 0)
		{
			timer_notify_buzzer_stop(m_remind.is_isr);
		}
	}
}

void timer_notify_remind_start(uint32_t interval, uint32_t window, uint32_t motor_interval, uint32_t motor_window, uint32_t buzzer_interval, uint32_t buzzer_window, uint32_t repetition, bool is_isr)
{
	m_remind_motor_flag = 0;
	
	timer_notify_remind_stop(is_isr);
	m_remind.interval = interval;
	m_remind.window = window;
  m_remind.motor_interval = motor_interval;
	m_remind.motor_window = motor_window;
  m_remind.buzzer_interval = buzzer_interval;
	m_remind.buzzer_window = buzzer_window;
  m_remind.is_isr = is_isr; 
	if(repetition>1)
	{
		m_remind.repetition = repetition*2;
	}else{
		m_remind.repetition = 1;
	}
	
	if(is_isr)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		if( xTimerChangePeriodFromISR( m_timer_remind,  window / portTICK_PERIOD_MS ,&xHigherPriorityTaskWoken ) == pdPASS )
		{
			m_remind.repetition--;
			m_remind.evt = TIME_NOTIFY_REMIND_EVT_ENABLE;
			if (m_remind.motor_interval != 0)
            {
                timer_notify_motor_start(motor_interval,motor_window,(window/motor_interval),is_isr,NOTIFY_MODE_ALARM);
            } 
      if (m_remind.buzzer_interval != 0)
            {
                timer_notify_buzzer_start(buzzer_interval,buzzer_window,(window/buzzer_interval),is_isr,NOTIFY_MODE_ALARM);
            }
		}		
		else
		{
			ERR_HANDLER(ERR_NO_MEM);
		}
		if( xHigherPriorityTaskWoken != pdFALSE )
		{
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}	
	}
	else{
	
		if( xTimerChangePeriod( m_timer_remind, window / portTICK_PERIOD_MS, OSTIMER_WAIT_FOR_QUEUE ) == pdPASS )
		{
			TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:TIME_NOTIFY_BUZZER_EVT_ENABLE\n");
			m_remind.repetition--;
			m_remind.evt = TIME_NOTIFY_REMIND_EVT_ENABLE;
			if (m_remind.motor_interval != 0)
            {
                timer_notify_motor_start(motor_interval,motor_window,(window/motor_interval),is_isr,NOTIFY_MODE_ALARM);
            } 
            if (m_remind.buzzer_interval != 0)
            {
                timer_notify_buzzer_start(buzzer_interval,buzzer_window,(window/buzzer_interval),is_isr,NOTIFY_MODE_ALARM);
            }		
		}
		else
		{
			ERR_HANDLER(ERR_NO_MEM);
			TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:xTimerChangePeriod fail\n");
		}	
	}		
	
		
}

void timer_notify_remind_stop(bool is_isr)
{
    timer_notify_motor_stop(is_isr);
 
    timer_notify_buzzer_stop(is_isr);

	if(is_isr)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		if( xTimerStopFromISR( m_timer_remind, &xHigherPriorityTaskWoken ) != pdPASS )
		{
			ERR_HANDLER(ERR_NO_MEM);
			TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:xTimerStopFromISR fail\n");
		}
		if( xHigherPriorityTaskWoken != pdFALSE )
		{
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}	
	}
	else{
	
		if (pdPASS != xTimerStop(m_timer_remind, OSTIMER_WAIT_FOR_QUEUE))
		{	
			ERR_HANDLER(ERR_NO_MEM);
			TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:xTimerStop fail\n");
		}	
	}
	

}

void timer_notify_rollback(ScreenState_t nowscreen, ScreenState_t destscreen)
{
	//从通知返回原界面
	if(ScreenState == nowscreen)
	{
		DISPLAY_MSG  msg = {0,0};
		ScreenState = destscreen;
		msg.cmd = MSG_DISPLAY_SCREEN;
		xQueueSend(DisplayQueue, &msg, portMAX_DELAY);		
	}
	else if(ScreenState == DISPLAY_SCREEN_NOTIFY_ALARM)
	{
		//闹钟界面
		ScreenStateAlarm = destscreen;
	}
	else if(ScreenState >= DISPLAY_SCREEN_NOTIFY)
	{
		//通知界面
		if(ScreenStateSave == DISPLAY_SCREEN_NOTIFY_ALARM)
		{
			ScreenStateAlarm = destscreen;
		}
		else
		{
			ScreenStateSave = destscreen;
		}
	}	
}

static void callback_backlight(TimerHandle_t xTimer)
{
	TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:callback_backlight timeout\n");
	drv_light_disable();
	m_backlight.evt = TIME_NOTIFY_BACKLIGHT_EVT_DISABLE;	
}

void timer_notify_backlight_init(void)
{

	m_timer_backlight = xTimerCreate("NOT_BLIGHT",
												 ( TIMER_INTERVAL_MS / portTICK_PERIOD_MS ),
												 pdFALSE,
												 NULL,
												 callback_backlight);
	if (NULL == m_timer_backlight)
	{
		ERR_HANDLER(ERR_NO_MEM);
		TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:xTimerCreate fail\n");
	}
}




void timer_notify_backlight_uninit(void)
{

	if (pdPASS != xTimerDelete(m_timer_backlight, OSTIMER_WAIT_FOR_QUEUE))
	{
		ERR_HANDLER(ERR_NO_MEM);
		TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:xTimerDelete fail\n");
	}

}


void timer_notify_backlight_start(uint32_t timeout_msec, bool is_isr)
{
	//timer_notify_backlight_stop(is_isr);


	if(is_isr)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		if( xTimerChangePeriodFromISR( m_timer_backlight,  timeout_msec / portTICK_PERIOD_MS ,&xHigherPriorityTaskWoken ) == pdPASS )
		{
			drv_light_enable();	
			m_backlight.evt = TIME_NOTIFY_BACKLIGHT_EVT_ENABLE;			
		}	
		else
		{
			ERR_HANDLER(ERR_NO_MEM);
			TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:xTimerChangePeriod fail\n");
		}				
		if( xHigherPriorityTaskWoken != pdFALSE )
		{
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}	
	}
	else{
	
		if( xTimerChangePeriod( m_timer_backlight, timeout_msec / portTICK_PERIOD_MS, OSTIMER_WAIT_FOR_QUEUE ) == pdPASS )
		{
			drv_light_enable();	
			m_backlight.evt = TIME_NOTIFY_BACKLIGHT_EVT_ENABLE;					
		}
		else
		{
			ERR_HANDLER(ERR_NO_MEM);
			TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:xTimerChangePeriod fail\n");
		}	
	}		
	
		
}

void timer_notify_backlight_stop(bool is_isr)
{
	drv_light_disable();	
	m_backlight.evt = TIME_NOTIFY_BACKLIGHT_EVT_DISABLE;	
	if(is_isr)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		if( xTimerStopFromISR( m_timer_backlight, &xHigherPriorityTaskWoken ) != pdPASS )
		{
			ERR_HANDLER(ERR_NO_MEM);
			TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:xTimerStopFromISR fail\n");
		}
		if( xHigherPriorityTaskWoken != pdFALSE )
		{
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}	
	}
	else{
	
		if (pdPASS != xTimerStop(m_timer_backlight, OSTIMER_WAIT_FOR_QUEUE))
		{	
			ERR_HANDLER(ERR_NO_MEM);
			TIME_NOTIFY_PRINTF("[TIME_NOTIFY]:xTimerStop fail\n");
		}
	}	
}


bool timer_notify_backlight_is_enable(void)
{
	if(m_backlight.evt == TIME_NOTIFY_BACKLIGHT_EVT_ENABLE)
	{
		return 1;
	}
	else{
		return 0;
	}
}

//判断能否弹出通知界面
uint8_t time_notify_is_ok(void)
{
	uint8_t enable = 1;
	
	
	if (ScreenState == DISPLAY_SCREEN_NOTIFY_FIND_WATCH) 
	{
		//寻找腕表界面时不弹通知界面
		enable = 0;
	}
#ifdef COD 
	if (ScreenState == DISPLAY_SCREEN_NOTIFY_ON_BIND_DEVICE)
	{
		//绑定确认界面时不弹通知界面
		enable = 0;
	}
#endif
	return enable;
}






