#include "drv_intFlash.h"


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






void drv_intFlash_erase_page(uint32_t address)
{	
	taskENTER_CRITICAL();
	am_hal_flash_page_erase(AM_HAL_FLASH_PROGRAM_KEY,                                             
																	AM_HAL_FLASH_ADDR2INST(address),
																	AM_HAL_FLASH_ADDR2PAGE(address));
	taskEXIT_CRITICAL();		
}


void drv_intFlash_write_page(uint32_t address,uint32_t *p_data,uint32_t word_cnt)
{
	uint32_t *p_addr = (uint32_t *)address;	
	taskENTER_CRITICAL();
	am_hal_flash_program_main(AM_HAL_FLASH_PROGRAM_KEY,
																						p_data,
																						p_addr,
																						word_cnt);	
	taskEXIT_CRITICAL();	
}

void drv_intFlash_read(uint32_t address,uint32_t *p_data,uint32_t size)
{
	uint32_t *p_addr = (uint32_t *)address;	
	memcpy((uint8_t *)p_data,(uint8_t *)p_addr,size);
}





