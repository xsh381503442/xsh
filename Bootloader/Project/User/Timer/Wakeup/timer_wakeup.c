#include "timer_wakeup.h"
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



#include "com_utils.h"
#include "drv_rtc.h"
#include "dev_bat.h"


#ifdef LAYER_BOOTLOADER  
#include "gui_boot.h"	
#endif


#define MOUDLE_LOG_ENABLED 							0
#define MOUDLE_DEBUG_ENABLED   					0
#define MOUDLE_NAME                     "[TIMER_WAKEUP]:"

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


#define OSTIMER_WAIT_FOR_QUEUE           100
static TimerHandle_t m_timer; 
#ifdef LAYER_BOOTLOADER  
#define TIMER_INTERVAL_MS 1000
#endif




static void callback(TimerHandle_t xTimer)
{
	UNUSED_PARAMETER(xTimer);
	DEBUG_PRINTF(MOUDLE_NAME"callback\n");
	
	dev_bat_adc_enable();

	#if MOUDLE_LOG_ENABLED == 1
	drv_rtc_time_debug();
	#endif	

	
}

void timer_wakeup_init(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"timer_wakeup_init\n");
	
	m_timer = xTimerCreate("WAKE",
												 ( TIMER_INTERVAL_MS / portTICK_PERIOD_MS ),
												 pdTRUE,
												 NULL,
												 callback);
	if (NULL == m_timer)
	{
			ERR_HANDLER(ERR_NO_MEM);
	}
	if (pdPASS != xTimerStart(m_timer, OSTIMER_WAIT_FOR_QUEUE))
	{
			ERR_HANDLER(ERR_NO_MEM);
	}	
}



void timer_wakeup_uninit(void)
{

	if (pdPASS != xTimerStop(m_timer, OSTIMER_WAIT_FOR_QUEUE))
	{
			ERR_HANDLER(ERR_NO_MEM);
	}

}












