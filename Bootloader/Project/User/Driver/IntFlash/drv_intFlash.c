#include "drv_intFlash.h"
#include "rtt_log.h"
#include "lib_error.h"
#include "com_apollo2.h"
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
#include <string.h>

#define MOUDLE_LOG_ENABLED 							0
#define MOUDLE_DEBUG_ENABLED   					0
#define MOUDLE_NAME                     "[DRV_INTFLASH]:"

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






void drv_intFlash_erase_page(uint32_t address)
{	
	uint32_t error = ERR_SUCCESS;
#ifdef AM_FREERTOS 	
	taskENTER_CRITICAL();
#endif 	
	error = am_hal_flash_page_erase(AM_HAL_FLASH_PROGRAM_KEY,                                             
																	AM_HAL_FLASH_ADDR2INST(address),
																	AM_HAL_FLASH_ADDR2PAGE(address));
#ifdef AM_FREERTOS	
	taskEXIT_CRITICAL();	
#endif 	
	ERR_CHECK(error);
}


void drv_intFlash_write_page(uint32_t address,uint32_t *p_data,uint32_t word_cnt)
{
	DEBUG_PRINTF(MOUDLE_NAME"drv_intFlash_write_page\n");
	uint32_t error = ERR_SUCCESS;
	uint32_t *p_addr = (uint32_t *)address;
#ifdef AM_FREERTOS 	
	taskENTER_CRITICAL();
#endif 
	error = am_hal_flash_program_main(AM_HAL_FLASH_PROGRAM_KEY,
																						p_data,
																						p_addr,
																						word_cnt);	
#ifdef AM_FREERTOS	
	taskEXIT_CRITICAL();	
#endif 	
	ERR_CHECK(error);
}

void drv_intFlash_read(uint32_t address,uint32_t *p_data,uint32_t size)
{
	DEBUG_PRINTF(MOUDLE_NAME"drv_intFlash_read\n");
	uint32_t *p_addr = (uint32_t *)address;	
	memcpy((uint8_t *)p_data,(uint8_t *)p_addr,size);
}





