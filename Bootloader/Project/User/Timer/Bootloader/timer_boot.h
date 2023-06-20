#ifndef __TIMER_BOOT_H__
#define __TIMER_BOOT_H__



#include <stdint.h>




void timer_boot_init(void);
void timer_boot_uninit(void);
void timer_boot_stop(void);
void timer_boot_start(uint32_t timeout_msec);




#endif //__TIMER_BOOT_H__
