#ifndef __DRV_INTFLASH_H__
#define __DRV_INTFLASH_H__




#include <stdint.h>



void drv_intFlash_read(uint32_t address,uint32_t *p_data,uint32_t size);
void drv_intFlash_write_page(uint32_t address,uint32_t *p_data,uint32_t word_cnt);
void drv_intFlash_erase_page(uint32_t address);


#endif //__DRV_INTFLASH_H__
