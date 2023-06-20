#include "dev_ble.h"
#include "rtt_log.h"
#include "lib_error.h"
#include "SEGGER_RTT.h"


#include "am_mcu_apollo.h"
#include "drv_ble.h"
#include "drv_pc.h"
#include "task_ble.h"
#include "cmd_ble.h"

#include "cmd_pc.h"
#ifdef AM_FREERTOS 
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "portmacro.h"
#include "portable.h"
#include "event_groups.h"
#endif 

#include <string.h>
#include "com_apollo2.h"


#define MOUDLE_LOG_ENABLED 							0
#define MOUDLE_DEBUG_ENABLED   					0
#define MOUDLE_NAME                     "[DEV_BLE]:"

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

#define DEV_BLE_SYS_OFF_PIN            37


_dev_ble_info g_ble_info = {0};




	/**
	
	SYSTEM OFF PIN LOW : BLE ON
	SYSTEM OFF PIN HIGH : BLE OFF
	
	*/

void dev_ble_init(void)
{
	PIN_CLEAR(DEV_BLE_SYS_OFF_PIN);
	PIN_CONFIG(DEV_BLE_SYS_OFF_PIN,AM_HAL_GPIO_OUTPUT);
}


void dev_ble_system_on(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"dev_ble_system_on\n");
	if(PIN_READ_OUTPUT(DEV_BLE_SYS_OFF_PIN))
	{
		PIN_CLEAR(DEV_BLE_SYS_OFF_PIN);
		task_ble_init();
	}
}

void dev_ble_system_off(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"dev_ble_system_off\n");
	if(!PIN_READ_OUTPUT(DEV_BLE_SYS_OFF_PIN))
	{
		PIN_SET(DEV_BLE_SYS_OFF_PIN);
		task_ble_uninit();
	}
}



