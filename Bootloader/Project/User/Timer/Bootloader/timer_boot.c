#include "timer_boot.h"
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



#include "task_display.h"	


#include "gui_boot.h"	



#define MOUDLE_LOG_ENABLED 							1
#define MOUDLE_DEBUG_ENABLED   					1
#define MOUDLE_NAME                     "[TIMER_BOOT]:"

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
#define TIMER_INTERVAL_MS 1000

extern ScreenState_t   ScreenState;
extern QueueHandle_t 	DisplayQueue;
static bool m_timer_start = 0;


static void callback(TimerHandle_t xTimer)
{

	DEBUG_PRINTF(MOUDLE_NAME"callback\n");
	
	DISPLAY_MSG  msg = {0,0};
	ScreenState = DISPLAY_SCREEN_BOOTLOADER;
	msg.cmd = MSG_DISPLAY_SCREEN;
	xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			

	
}

void timer_boot_init(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"timer_boot_init\n");
	m_timer_start = 0;
	m_timer = xTimerCreate("BOOT",
												 ( TIMER_INTERVAL_MS / portTICK_PERIOD_MS ),
												 pdTRUE,
												 NULL,
												 callback);
	if (NULL == m_timer)
	{
			ERR_HANDLER(ERR_NO_MEM);
	}

}



void timer_boot_uninit(void)
{
	if (pdPASS != xTimerDelete(m_timer, OSTIMER_WAIT_FOR_QUEUE))
	{
		ERR_HANDLER(ERR_NO_MEM);
	}


}

void timer_boot_start(uint32_t timeout_msec)
{
	if(m_timer_start)
	{
		return;
	}
	m_timer_start = 1;
	if( xTimerChangePeriod( m_timer, timeout_msec / portTICK_PERIOD_MS, OSTIMER_WAIT_FOR_QUEUE ) == pdPASS )
	{
		DEBUG_PRINTF(MOUDLE_NAME"xTimerChangePeriod success\n");
	}
	else
	{
		ERR_HANDLER(ERR_NO_MEM);
		DEBUG_PRINTF(MOUDLE_NAME"xTimerChangePeriod fail\n");
	}		
}

void timer_boot_stop(void)
{
	if(!m_timer_start)
	{
		return;
	}
	m_timer_start = 0;	
	if (pdPASS != xTimerStop(m_timer, OSTIMER_WAIT_FOR_QUEUE))
	{
		ERR_HANDLER(ERR_NO_MEM);
		DEBUG_PRINTF(MOUDLE_NAME"xTimerStop fail\n");
	}
}








