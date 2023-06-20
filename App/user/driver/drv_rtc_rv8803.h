#ifndef  DRV_RTC_RV8803_H
#define  DRV_RTC_RV8803_H
#include "drv_config.h"
#include "watch_config.h"
#include "bsp_rtc.h"
#include "bsp_iom.h"

#define RTC_RV8803_ADD  0x32

#define		BSP_RTC_RV8803_IOM					3	//RTC i2c3
#define     BSP_GPIO_RTC_INT                    2


extern uint32_t drv_rtc_rv8803_time_no_ms_get(am_hal_rtc_time_t *pTime);
extern uint32_t drv_rtc_rv8803_time_has_ms_get(am_hal_rtc_time_t *pTime);

extern void drv_rtc_rv8803_time_set(am_hal_rtc_time_t *pTime);

extern void drv_rtv_periodic_time_int_set(uint32_t ui32RepeatInterval);
#ifdef WATCH_RV8803_CALIBRATION
extern void rv8803_cailbration_apollo_rtc(void);
#endif


#endif
