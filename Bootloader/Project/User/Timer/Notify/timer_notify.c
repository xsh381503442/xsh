#ifdef LAYER_APPLICATION 
#include "timer_notify.h"
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


#include "gui_util.h"
#include "gui_home.h"
#include "gui_menu.h"
#include "gui_ble.h"
#include "gui_axis6.h"
#include "gui_mag.h"
#include "gui_gps.h"
#include "gui_heartrate.h"
#include "gui_notify.h"




#define MOUDLE_LOG_ENABLED 							0
#define MOUDLE_DEBUG_ENABLED   					0
#define MOUDLE_NAME                     "[TIMER_NOTIFY]:"

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





#define OSTIMER_WAIT_FOR_QUEUE           2
static TimerHandle_t m_timer; 
#define TIMER_INTERVAL_MS 5000

#ifdef LAYER_APPLICATION 
extern uint32_t g_gui_index;
extern uint32_t g_gui_index_save;
#endif


static void callback(TimerHandle_t xTimer)
{
	UNUSED_PARAMETER(xTimer);
	DEBUG_PRINTF(MOUDLE_NAME"callback\n");
	
#ifdef LAYER_APPLICATION 
	DEBUG_PRINTF(MOUDLE_NAME"g_gui_index --> %d\n",g_gui_index);
	DEBUG_PRINTF(MOUDLE_NAME"g_gui_index_save --> %d\n",g_gui_index_save);	
	g_gui_index = g_gui_index_save;
	g_gui_index_save = g_gui_index;	
	gui_notify_rollback_paint(g_gui_index);
#endif

	
}

void timer_notify_init(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"timer_notify_init\n");
	
	m_timer = xTimerCreate("NOTI",
												 ( TIMER_INTERVAL_MS / portTICK_PERIOD_MS ),
												 pdFALSE,
												 NULL,
												 callback);
	if (NULL == m_timer)
	{
			ERR_HANDLER(ERR_NO_MEM);
	}
}



void timer_notify_uninit(void)
{

	if (pdPASS != xTimerDelete(m_timer, OSTIMER_WAIT_FOR_QUEUE))
	{
			ERR_HANDLER(ERR_NO_MEM);
	}

}

void timer_notify_start(uint32_t timeout_msec)
{
	if( xTimerChangePeriod( m_timer, timeout_msec / portTICK_PERIOD_MS, 100 ) == pdPASS )
	{
		DEBUG_PRINTF(MOUDLE_NAME"The command was successfully sent.\n");
	}
	else
	{
		DEBUG_PRINTF(MOUDLE_NAME"The command could not be sent, even after waiting for 100 ticks\n");
		DEBUG_PRINTF(MOUDLE_NAME"to pass.  Take appropriate action here.\n");
	}		
}


void timer_notify_stop(void)
{
	if (pdPASS != xTimerStop(m_timer, OSTIMER_WAIT_FOR_QUEUE))
	{
			ERR_HANDLER(ERR_NO_MEM);
	}
}







#endif




