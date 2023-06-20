#ifndef __DRV_HRT_H__
#define __DRV_HRT_H__



#include <stdint.h>
#include <stdbool.h>

#define HRT_EVT_PPG_INTERRUPT               1
#define HRT_EVT_HRT_ENABLE                  2
#define HRT_EVT_HRT_DISABLE                 3
#define HRT_EVT_TIMER_INTERRUPT               4



typedef void (* drv_hrt_callback)(uint32_t evt, bool is_isr, uint8_t *p_data, uint32_t len);




void drv_hrt_restart(void);
void drv_hrt_suspend(void);

void drv_hrt_init(drv_hrt_callback callback);
void drv_hrt_disable(void);
void drv_hrt_enable(void);
void drv_hrt_5v_disable(void);
void drv_hrt_5v_enable(void);
void drv_hrt_3v3_enable(void);
void drv_hrt_3v3_disable(void);
void drv_hrt_int_handler(void);
uint32_t drv_hrt_all_auto_test(void);
void drv_hrt_irq_enable(void);
void drv_hrt_irq_disable(void);

void drv_hrt_logout(uint16_t testcounter);

#endif //__DRV_HRT_H__
