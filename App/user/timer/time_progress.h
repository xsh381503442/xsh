#ifndef __TIME_PROGRESS_H__
#define __TIME_PROGRESS_H__



#include <stdint.h>



void timer_progress_init(void);
void timer_progress_uninit(void);
void timer_progress_start(uint32_t timeout_msec);
void timer_progress_stop(void);








#endif //__TIME_PROGRESS_H__
