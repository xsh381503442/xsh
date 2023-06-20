#ifndef DRV_LIGHT_H
#define	DRV_LIGHT_H

#include <stdint.h>
#define     BL_TIMER_NUMBER         3
#define     BL_TIMER_SEGMENT        AM_HAL_CTIMER_TIMERA
#define     BL_TIMER_INTERRUPT      AM_HAL_CTIMER_INT_TIMERA3
#define     BL_INTERVAL             1  //12K/1=12KHZ
#define     BL_PERIOD               20 //PWMÖÜÆÚ1.2k

extern void drv_light_enable(void);
extern void drv_light_disable(void);
#endif


