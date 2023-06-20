#include "drv_motor.h"
#include "rtt_log.h"
#include "lib_error.h"
#include "com_apollo2.h"
#include "SEGGER_RTT.h"

#include "am_mcu_apollo.h"


#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "portmacro.h"
#include "portable.h"
#include "event_groups.h"

#include <string.h>

#define MOUDLE_LOG_ENABLED 							0
#define MOUDLE_DEBUG_ENABLED   					0
#define MOUDLE_NAME                     "[DRV_MOTOR]:"

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

#define MOTOR_PIN 45                                                               
#define OSTIMER_WAIT_FOR_QUEUE           2


static struct {
	bool initialized;
	_drv_motor_cfg cfg;
	uint32_t period;
}m_instance = {.initialized = false};

static TimerHandle_t time_handle; 

static void motor_enable(void);
static void motor_disable(void);



static void timer_handle(TimerHandle_t xTimer)
{
	DEBUG_PRINTF(MOUDLE_NAME"timer_handle\n");
	
	if(m_instance.cfg.times == 0)
	{
		motor_disable();
		return;
	}
	
	if(m_instance.period == m_instance.cfg.window)
	{
		m_instance.period = m_instance.cfg.interval - m_instance.cfg.window;
	}
	else
	{
		m_instance.period = m_instance.cfg.window;
	}
	
	if( xTimerChangePeriod( time_handle, m_instance.period / portTICK_PERIOD_MS, 100 ) == pdFAIL )
	{
			// The command could not be sent, even after waiting for 100 ticks
			// to pass.  Take appropriate action here.
		DEBUG_PRINTF(MOUDLE_NAME"The command could not be sent, even after waiting for 100 ticks\n");
		DEBUG_PRINTF(MOUDLE_NAME"to pass.  Take appropriate action here.\n");
	}		
	
	PIN_TOGGLE(MOTOR_PIN);
	m_instance.cfg.times--;
}



static void motor_enable(void) 
{
	m_instance.period = m_instance.cfg.window;
	if( xTimerChangePeriod( time_handle, m_instance.period / portTICK_PERIOD_MS, 100 ) == pdFAIL )
	{
			// The command could not be sent, even after waiting for 100 ticks
			// to pass.  Take appropriate action here.
		DEBUG_PRINTF(MOUDLE_NAME"The command could not be sent, even after waiting for 100 ticks\n");
		DEBUG_PRINTF(MOUDLE_NAME"to pass.  Take appropriate action here.\n");
	}		

//	if (pdPASS != xTimerStart(time_handle, OSTIMER_WAIT_FOR_QUEUE))
//	{
//			ERR_HANDLER(ERR_NO_MEM);
//	}

	PIN_SET(MOTOR_PIN);
}
static void motor_disable(void) 
{
	if (pdPASS != xTimerStop(time_handle, OSTIMER_WAIT_FOR_QUEUE))
	{
			ERR_HANDLER(ERR_NO_MEM);
	}	

//	if(pdPASS != xTimerDelete(time_handle, OSTIMER_WAIT_FOR_QUEUE))
//	{
//			ERR_HANDLER(ERR_NO_MEM);
//	}	
	
	PIN_CLEAR(MOTOR_PIN);
}



void drv_motor_enable(_drv_motor_cfg *cfg)
{
	DEBUG_PRINTF(MOUDLE_NAME"drv_motor_config\n");
	/**<To ensure efficiency I do not check for run time config error>*/
	/**<Interval must be greater than window>*/
	m_instance.cfg.interval = cfg->interval;
	m_instance.cfg.window = cfg->window;
	m_instance.cfg.times = cfg->times * 2 - 1;
	motor_enable();		
}
void drv_motor_disable(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"drv_motor_disable\n");
	memset(&m_instance,0,sizeof(m_instance));
	motor_disable();	
}




void drv_motor_init(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"drv_motor_init\n");
	time_handle = xTimerCreate("MOTO",
														 ( DRV_MOTOR_DEFAULT_INTERVAL / portTICK_PERIOD_MS ),
														 pdFALSE,
														 NULL,
														 timer_handle);
	if (NULL == time_handle)
	{
			ERR_HANDLER(ERR_NO_MEM);
	}	
	memset(&m_instance,0,sizeof(m_instance));
	PIN_CONFIG(MOTOR_PIN,AM_HAL_GPIO_OUTPUT);
	PIN_CLEAR(MOTOR_PIN);		
	m_instance.initialized = true;
}






