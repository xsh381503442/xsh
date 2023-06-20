#include "timer_exception.h"
#include "rtt_log.h"
#include "lib_error.h"
#include "SEGGER_RTT.h"

#include "am_mcu_apollo.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "portmacro.h"
#include "portable.h"
#include "event_groups.h"
#include "semphr.h"

#include "drv_pc.h"


#define MOUDLE_LOG_ENABLED 							1
#define MOUDLE_DEBUG_ENABLED   					1
#define MOUDLE_NAME                     "[TIMER_EXCEPTION]:"

#if MOUDLE_LOG_ENABLED == 1 && RTT_LOG_ENABLED == 1
#define LOG_PRINTF(...)                      SEGGER_RTT_printf(0, ##__VA_ARGS__) 
#define LOG_HEXDUMP(p_data, len)             rtt_log_hexdump(p_data, len) 
#else
#define LOG_PRINTF(...) 
#define LOG_HEXDUMP(p_data, len)             
#endif

#if MOUDLE_DEBUG_ENABLED == 1 && RTT_LOG_ENABLED == 1
#define DEBUG_PRINTF(...)                    SEGGER_RTT_printf(0, ##__VA_ARGS__)
#define DEBUG_HEXDUMP(p_data, len)           rtt_log_hexdump(p_data, len)
#else
#define DEBUG_PRINTF(...)   
#define DEBUG_HEXDUMP(p_data, len) 
#endif


//#define DELAY_MS(X)                 am_util_delay_ms(X)
#define DELAY_MS(X)                 vTaskDelay(X / portTICK_PERIOD_MS)
//#define DELAY_MS(X)                   do{TickType_t xLastWakeTime;xLastWakeTime = xTaskGetTickCount ();vTaskDelayUntil( &xLastWakeTime, X / portTICK_PERIOD_MS );}while(0)


#define OSTIMER_WAIT_FOR_QUEUE           100
static TimerHandle_t m_timer; 
#define TIMER_INTERVAL_MS 1000



static void callback(TimerHandle_t xTimer)
{

	DEBUG_PRINTF(MOUDLE_NAME"callback\n");

	drv_pc_uninit();
	DELAY_MS(500);
	#ifdef AM_FREERTOS 	
		taskENTER_CRITICAL();
	#endif 
	am_hal_reset_por();	
}

void timer_exception_init(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"timer_exception_init\n");
	
//	m_timer = xTimerCreate("BOOT",
//												 ( TIMER_INTERVAL_MS / portTICK_PERIOD_MS ),
//												 pdFALSE,
//												 NULL,
//												 callback);
//	if (NULL == m_timer)
//	{
//			ERR_HANDLER(ERR_NO_MEM);
//	}

}



void timer_exception_uninit(void)
{
//	if (pdPASS != xTimerDelete(m_timer, OSTIMER_WAIT_FOR_QUEUE))
//	{
//		ERR_HANDLER(ERR_NO_MEM);
//	}


}

void timer_exception_start(uint32_t timeout_msec)
{

//	if( xTimerChangePeriod( m_timer, timeout_msec / portTICK_PERIOD_MS, OSTIMER_WAIT_FOR_QUEUE ) == pdPASS )
//	{
//		DEBUG_PRINTF(MOUDLE_NAME"xTimerChangePeriod success\n");
//	}
//	else
//	{
//		ERR_HANDLER(ERR_NO_MEM);
//		DEBUG_PRINTF(MOUDLE_NAME"xTimerChangePeriod fail\n");
//	}		
}

void timer_exception_stop(void)
{
//	if (pdPASS != xTimerStop(m_timer, OSTIMER_WAIT_FOR_QUEUE))
//	{
//		ERR_HANDLER(ERR_NO_MEM);
//		DEBUG_PRINTF(MOUDLE_NAME"xTimerStop fail\n");
//	}
}








