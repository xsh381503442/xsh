#include "dev_font.h"
#include "rtt_log.h"
#include "lib_error.h"
#include "SEGGER_RTT.h"

#include "am_mcu_apollo.h"
#ifdef AM_FREERTOS 
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "portmacro.h"
#include "portable.h"
#include "event_groups.h"
#endif

#include "com_apollo2.h"
#include "drv_font.h"
#include "am_util.h"

#include "HFMA2Ylib.h"


#define MOUDLE_LOG_ENABLED 							1
#define MOUDLE_DEBUG_ENABLED   					1
#define MOUDLE_NAME                     "[DEV_FONT]:"

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

#define DELAY_US(X)                 am_util_delay_us(X)
#define DELAY_MS(X)                 am_util_delay_ms(X)
//#define DELAY_MS(X)                 vTaskDelay(X / portTICK_PERIOD_MS)
//#define DELAY_MS(X)                   do{TickType_t xLastWakeTime;xLastWakeTime = xTaskGetTickCount ();vTaskDelayUntil( &xLastWakeTime, X / portTICK_PERIOD_MS );}while(0)







uint32_t dev_font_self_test(void)
{
	
	LOG_PRINTF(MOUDLE_NAME"dev_font_self_test\n");

	uint8_t i_buf[5] = {0x3E,0x51,0x49,0x45,0x3E};
	uint8_t o_buf[5] = {0};
	

	drv_font_enable();
	#ifdef LAYER_APPLICATION 
	drv_font_pwr_up();
	#endif		
	ASCII_GetData('0',ASCII_5X7,o_buf);
	#ifdef LAYER_APPLICATION 
	drv_font_pwr_down();
	#endif	
	drv_font_disable();
	
	DEBUG_HEXDUMP(o_buf, sizeof(o_buf));
	
	if(memcmp(o_buf,i_buf,sizeof(o_buf)) != 0)
	{	
		ERR_HANDLER(ERR_NOT_FOUND);
	}
	
	return ERR_SUCCESS;
	
}





void dev_font_enter_deep_sleep(void)
{
	drv_font_enable();
	drv_font_pwr_down();
	drv_font_disable();
}

void dev_font_exit_deep_sleep(void)
{
	drv_font_enable();
	drv_font_pwr_up();
	drv_font_disable();
}




