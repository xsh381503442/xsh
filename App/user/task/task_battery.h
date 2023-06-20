#ifndef __TASK_BATTERY_H__
#define __TASK_BATTERY_H__

#include <stdint.h>

#include "drv_battery.h"

#define SHUTDOWN_VAL 3400
void task_battery_init(void);

void timer_battery_init(void);
void timer_battery_uninit(void);
extern void timer_battery_start(uint32_t timeout_msec);
void timer_battery_stop(void);



#endif //__TASK_BATTERY_H__
