#ifndef __DEV_EXTFLASH_H__
#define __DEV_EXTFLASH_H__



#include <stdint.h>




void dev_extFlash_IDverify(void);
void dev_extFlash_erase(uint32_t addr, uint32_t size);
void dev_extFlash_read(uint32_t addr, uint8_t* p_data, uint16_t len);
void dev_extFlash_write(uint32_t addr, uint8_t* p_data, uint16_t len);
void dev_extFlash_enter_deep_sleep(void);
void dev_extFlash_exit_deep_sleep(void);
void dev_extFlash_enable(void);
void dev_extFlash_disable(void);



#endif //__DEV_EXTFLASH_H__
