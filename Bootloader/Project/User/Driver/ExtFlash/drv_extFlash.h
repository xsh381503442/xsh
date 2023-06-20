#ifndef _DRV_EXTFALSH_H__
#define _DRV_EXTFALSH_H__


#include <stdint.h>

//#define DRV_EXTFLASH_MOULDE                  0
#define DRV_EXTFLASH_SPEED                   4000000
//#define DRV_EXTFLASH_PIN_MOSI                7
//#define DRV_EXTFLASH_PIN_MISO                6
//#define DRV_EXTFLASH_PIN_CLK                 5
//#define DRV_EXTFLASH_PIN_CS                  45
//#define DRV_EXTFLASH_CHIP_SELECT             3
//#define EXTFLASH_READY_PIN                   49


#define W25Q128_REG_ID                       (0x9F)   
#define W25Q128_REG_ID_VAL                   (0x55)   
#define W25Q128_REG_PAGE_WRITE      				 (0x02)  /* Write to Memory instruction */
#define W25Q128_REG_WRSR       							 (0x01)  /* Write Status Register instruction */
#define W25Q128_REG_WREN       							 (0x06)  /* Write enable instruction */
#define W25Q128_REG_READ       							 (0x03)  /* Read from Memory instruction */
#define W25Q128_REG_RDSR       							 (0x05)  /* Read Status Register instruction  */
#define W25Q128_REG_SECTOR_ERASE         		 (0x20)  /* Sector Erase instruction */
#define W25Q128_REG_CHIP_ERASE         			 (0xC7)  /* Bulk Erase instruction */        
#define W25Q128_REG_BLOCK_32K_ERASE					 (0x52)
#define W25Q128_REG_BLOCK_64K_ERASE					 (0xD8)
#define W25Q128_REG_POWER_DOWN       				 (0xB9)
#define W25Q128_REG_RELEASE_POWER_DOWN       (0xAB)

#define W25Q128_WIP_FLAG   									 (0x01)  /* Write In Progress (WIP) flag */
#define W25Q128_DUMMY_BYTE 									 (0xA5)
#define W25Q128_SET        									 (1)


#define W25Q128_PAGE_SIZE    0x100/**< FLASH_PAGE_SIZE >*/ 

#define W25Q128_MAX_SIZE        0x800000

#define W25Q128_SECTOR_SIZE     0x1000
#define W25Q128_32K_SIZE        0x8000
#define W25Q128_64K_SIZE        0x10000

void drv_extFlash_sector_erase(uint32_t addr);
void drv_extFlash_32K_erase(uint32_t addr);
void drv_extFlash_64K_erase(uint32_t addr);
void drv_extFlash_chip_erase(void);
void drv_extFlash_page_write( uint32_t addr, uint8_t * p_data, uint16_t len);
void drv_extFlash_read(uint32_t addr, uint8_t* p_data, uint16_t len);

uint32_t drv_extFlash_ID_read(void);
void drv_extFlash_enable(void);
void drv_extFlash_disable(void);
void drv_extFlash_pwr_down(void);
void drv_extFlash_pwr_up(void);




#endif //_DRV_EXTFALSH_H__
