#include "lib_boot.h"
#include "rtt_log.h"
#include "lib_error.h"
#include "com_apollo2.h"
#include "SEGGER_RTT.h"

#include "am_mcu_apollo.h"

#include "com_utils.h"

#include <string.h>

#ifdef AM_FREERTOS 
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "portmacro.h"
#include "portable.h"
#include "event_groups.h"
#endif 
#include "am_util_delay.h"

#define MOUDLE_LOG_ENABLED 							0
#define MOUDLE_DEBUG_ENABLED   					0
#define MOUDLE_NAME                     "[LIB_BOOT]:"

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



#define DELAY_MS(X)                 am_util_delay_ms(X)
//#define DELAY_MS(X)                 vTaskDelay(X / portTICK_PERIOD_MS)
//#define DELAY_MS(X)                   do{TickType_t xLastWakeTime;xLastWakeTime = xTaskGetTickCount ();vTaskDelayUntil( &xLastWakeTime, X / portTICK_PERIOD_MS );}while(0)

void lib_boot_goto_image(am_bootloader_image_t *image)
{
	DEBUG_PRINTF(MOUDLE_NAME"lib_boot_goto_image\n");
	static am_bootloader_image_t m_image;
	m_image.pui32LinkAddress                         = image->pui32LinkAddress;
	m_image.ui32NumBytes                             = 0;
	m_image.ui32CRC                                  = 0;
	m_image.ui32OverridePolarity                     = 0;
	m_image.pui32StackPointer                        = 0;
	m_image.pui32ResetVector                         = 0;
	m_image.bEncrypted                               = 0;

	am_bootloader_image_run(&m_image);
}


bool lib_boot_jump_to_bootloader(uint32_t reason)
{
	DEBUG_PRINTF(MOUDLE_NAME"lib_boot_jump_to_bootloader\n");
	_boot_setting boot_setting;
	lib_boot_setting_read(&boot_setting);
	if(lib_boot_setting_crc_check_bootsetting(&boot_setting))	
	{
		
		if(lib_boot_setting_crc_check_bootloader(&boot_setting))	
		{
			boot_setting.enter_bootloader_reason = reason;
			boot_setting.boot_setting_crc = am_bootloader_fast_crc32(&boot_setting.boot_setting_size,boot_setting.boot_setting_size - 4);
			DEBUG_PRINTF(MOUDLE_NAME"crc = 0x%08x\n",boot_setting.boot_setting_crc);	
			lib_boot_setting_write(&boot_setting);
			#ifdef AM_FREERTOS 	
				taskENTER_CRITICAL();
			#endif 			
			am_hal_reset_por();
		}		
	}
	return false;
}



bool lib_boot_jump_to_update_algorithm(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"lib_boot_jump_to_update_algorithm\n");
	_boot_setting boot_setting;
	lib_boot_setting_read(&boot_setting);
	if(lib_boot_setting_crc_check_bootsetting(&boot_setting))	
	{
		if(lib_boot_setting_crc_check_updateAlgorithm(&boot_setting))	
		{
			boot_setting.enter_bootloader_reason = BOOT_SETTING_ENTER_BOOT_REASON_JUMP_UPDATE_ALGORITHM;
			boot_setting.boot_setting_crc = am_bootloader_fast_crc32(&boot_setting.boot_setting_size,boot_setting.boot_setting_size - 4);
			DEBUG_PRINTF(MOUDLE_NAME"crc = 0x%08x\n",boot_setting.boot_setting_crc);	
			lib_boot_setting_write(&boot_setting);
			#ifdef AM_FREERTOS 	
				taskENTER_CRITICAL();
			#endif 				
			am_hal_reset_por();
		}		
	}
	return false;
}










