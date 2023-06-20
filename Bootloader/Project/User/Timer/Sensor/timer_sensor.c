#ifdef LAYER_APPLICATION 
#include "timer_sensor.h"
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
#include "gui_axis6.h"

#ifdef BOSH_OFFICIAL_SITE_API
#include "drv_bosh_official_api.h"
#endif
#ifdef BOSH_KEIL_API
#include "drv_bosh_keil_api.h"
#endif



#define MOUDLE_LOG_ENABLED 							0
#define MOUDLE_DEBUG_ENABLED   					0
#define MOUDLE_NAME                     "[TIMER_SENSOR]:"

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
	
	switch(g_gui_index)
	{
		case GUI_INDEX_AXIS6:{
			drv_bosh_official_api_read_acc();
			gui_axis6_sensor_data_paint();
		
		}break;
	
	}	

	

}

void timer_sensor_init(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"timer_sensor_init\n");
	
	m_timer = xTimerCreate("SENS",
												 ( TIMER_INTERVAL_MS / portTICK_PERIOD_MS ),
												 pdTRUE,
												 NULL,
												 callback);
	if (NULL == m_timer)
	{
			ERR_HANDLER(ERR_NO_MEM);
	}
}


void timer_sensor_uninit(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"timer_sensor_uninit\n");
	if (pdPASS != xTimerDelete(m_timer, OSTIMER_WAIT_FOR_QUEUE))
	{
			ERR_HANDLER(ERR_NO_MEM);
	}

}



void timer_sensor_start(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"timer_sensor_start\n");
	if (pdPASS != xTimerStart(m_timer, OSTIMER_WAIT_FOR_QUEUE))
	{
			ERR_HANDLER(ERR_NO_MEM);
	}	
}


void timer_sensor_stop(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"timer_sensor_stop\n");
	if (pdPASS != xTimerStop(m_timer, OSTIMER_WAIT_FOR_QUEUE))
	{
			ERR_HANDLER(ERR_NO_MEM);
	}
}







#endif




