#include "dev_notify.h"
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





#define MOUDLE_LOG_ENABLED 							0
#define MOUDLE_DEBUG_ENABLED   					0
#define MOUDLE_NAME                     "[DEV_NOTIFY]:"

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



void dev_notify_enable(_dev_notify *notify)
{
	DEBUG_PRINTF(MOUDLE_NAME"dev_notify_enable\n");

  if(notify->evt_mask & DEV_NOTIFY_FEATURE_MASK_MOTOR)
	{
		drv_motor_enable(&notify->cfg.motor_cfg);
	}

	
}
void dev_notify_disable(_dev_notify *notify)
{
	DEBUG_PRINTF(MOUDLE_NAME"dev_notify_disable\n");

  if(notify->evt_mask & DEV_NOTIFY_FEATURE_MASK_MOTOR)
	{
		drv_motor_disable();
	}

}


void dev_notify_init(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"dev_notify_init\n");
	drv_motor_init();
	

}









