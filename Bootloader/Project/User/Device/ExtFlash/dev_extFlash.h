#ifndef _DEV_EXTFLASH_H__
#define _DEV_EXTFLASH_H__


#include <stdint.h>
#include <stdbool.h>

#define DEV_EXTFLASH_ERASE_TYPE_SECTOR                 1
#define DEV_EXTFLASH_ERASE_TYPE_32K                 	 2
#define DEV_EXTFLASH_ERASE_TYPE_64K                    3
#define DEV_EXTFLASH_ERASE_TYPE_CHIP                   4




void dev_extFlash_write(uint32_t addr, uint8_t* p_data, uint16_t len);
void dev_extFlash_read(uint32_t addr, uint8_t* p_data, uint16_t len);
void dev_extFlash_erase(uint32_t addr, uint32_t size);
bool dev_extFlash_IDverify(uint8_t times);
void dev_extFlash_enter_deep_sleep(void);
void dev_extFlash_exit_deep_sleep(void);
void dev_extFlash_disable(void);
void dev_extFlash_enable(void);


#endif //_DEV_EXTFLASH_H__
