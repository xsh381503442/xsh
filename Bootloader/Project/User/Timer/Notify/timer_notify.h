#ifndef __TIMER_NOTIFY_H__
#define __TIMER_NOTIFY_H__


#include <stdint.h>



void timer_notify_init(void);
void timer_notify_uninit(void);
void timer_notify_start(uint32_t timeout_msec);
void timer_notify_stop(void);




#endif //__TIMER_NOTIFY_H__
