#ifndef __TIMER_EXCEPTION_H__
#define __TIMER_EXCEPTION_H__



#include <stdint.h>

#define TIMER_EXCEPTION_TIMEOUT            20000

void timer_exception_init(void);
void timer_exception_uninit(void);
void timer_exception_stop(void);
void timer_exception_start(uint32_t timeout_msec);




#endif //__TIMER_EXCEPTION_H__
