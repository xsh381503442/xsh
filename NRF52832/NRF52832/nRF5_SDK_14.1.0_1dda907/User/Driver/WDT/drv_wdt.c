#include "drv_wdt.h"
#include "nrf_drv_wdt.h"


#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "portmacro.h"
#include "portable.h"
#include "event_groups.h"
#include "semphr.h"


#include "nrf_nvmc.h"
#include "nrf_power.h"
#include "drv_apollo2.h"



#define LOG_LEVEL_OFF 							0
#define LOG_LEVEL_ERROR 						1
#define LOG_LEVEL_WARNING 					2
#define LOG_LEVEL_INFO 							3
#define LOG_LEVEL_DEBUG 						4
#define NRF_LOG_MODULE_NAME drv_wdt
#define NRF_LOG_LEVEL       LOG_LEVEL_DEBUG
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
NRF_LOG_MODULE_REGISTER();

static nrf_drv_wdt_channel_id m_channel_id = NRF_WDT_RR0;


#define TIMER_INTERVAL            60000  //60000 
#define OSTIMER_WAIT_FOR_QUEUE              100 
static TimerHandle_t m_timer; 

#define TIMER_EVT_STORE_BLE_CONFIG_FILE         1


void drv_wdt_feed(void)
{
	nrf_drv_wdt_channel_feed(m_channel_id);
}

static void timeout_handler(TimerHandle_t xTimer)
{
	UNUSED_PARAMETER(xTimer);
	NRF_LOG_DEBUG("timeout_handler");
	#ifdef COD 
	NRF_LOG_DEBUG("m_state -- > %d,%d,%d\r\n",read_m_state(),cod_get_malloc_count(),xPortGetFreeHeapSize());
	#endif
	nrf_drv_wdt_channel_feed(m_channel_id);
}


static void timer_create(void)
{
	m_timer = xTimerCreate("WDT",
                         TIMER_INTERVAL,
                         pdTRUE,
                         NULL,
                         timeout_handler);
	if (NULL == m_timer)
	{
			APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
	}	
}
static void timer_start(void)
{
	
	if (pdPASS != xTimerStart(m_timer, OSTIMER_WAIT_FOR_QUEUE))
	{
			APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
	}

}






static void wdt_event_handler(void)
{
  //NOTE: The max amount of time we can spend in WDT interrupt is two cycles of 32768[Hz] clock - after that, reset occurs

	NRF_LOG_DEBUG("wdt_event_handler");
}


void drv_wdt_init(void)
{
	uint32_t err_code = NRF_SUCCESS;
	nrf_drv_wdt_config_t config =     {                                                                
        .behaviour          = NRF_WDT_BEHAVIOUR_RUN_SLEEP_HALT,             
        .reload_value       = 70000, /* 327679 3 minutes (32768/6*60*3-1) */            
        .interrupt_priority = APP_IRQ_PRIORITY_HIGHEST,                  
    };
	
	err_code = nrf_drv_wdt_init(&config, wdt_event_handler);
	APP_ERROR_CHECK(err_code);
	err_code = nrf_drv_wdt_channel_alloc(&m_channel_id);
	APP_ERROR_CHECK(err_code);
	nrf_drv_wdt_enable();	
	nrf_drv_wdt_channel_feed(m_channel_id);	
	timer_create();
	timer_start();	
		


		
		
		
}







