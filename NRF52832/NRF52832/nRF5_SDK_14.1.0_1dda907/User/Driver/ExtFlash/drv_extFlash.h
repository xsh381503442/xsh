#ifndef __DRV_EXTFLASH_H__
#define __DRV_EXTFLASH_H__



#include <stdint.h>



#define EXTFLASH_SPI_SCK_PIN         2                       
#define EXTFLASH_SPI_MOSI_PIN        4       
#define EXTFLASH_SPI_MISO_PIN        3        
#define EXTFLASH_SPI_SS_PIN          11 

#define EXTFLASH_READY_PIN           20

#define EXTFLASH_MAX_SIZE        0x800000

void drv_extFlash_enable(void);
void drv_extFlash_disable(void);

uint32_t drv_extFlash_readID(void);
void drv_extFlash_read(uint32_t addr, uint8_t* p_data, uint16_t len);
void drv_extFlash_page_write( uint32_t addr, uint8_t * p_data, uint16_t len);
void drv_extFlash_chip_erase(void);
void drv_extFlash_64K_erase(uint32_t addr);
void drv_extFlash_32K_erase(uint32_t addr);
void drv_extFlash_sector_erase(uint32_t addr);
void drv_extFlash_pwr_down(void);
void drv_extFlash_pwr_up(void);








#endif //__DRV_EXTFLASH_H__
