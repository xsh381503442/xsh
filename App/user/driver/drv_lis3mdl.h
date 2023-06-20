#ifndef DRV_LIS3MDL_H
#define	DRV_LIS3MDL_H

#include <stdint.h>
#include "Compass/akm_oss_wrapper.h"
#include <stdbool.h>

#define		LIS3MDL_ADDR	0x1E

#define     BSP_LIS3MDL_IOM			3


#define lis3mdl_sample(n)                                          \
    union                                                                     \
    {                                                                         \
       int16_t words[((3 * n) + 1) >> 1];                                   \
        int16_t samples[3 * n];                                               \
    }

extern bool drv_lis3mdl_id(void);
//extern void drv_lis3mdl_init(void);
extern bool drv_lis3mdl_data(int16_t *psdata);
extern void drv_lis3mdl_standby(void);
extern void drv_lis3mdl_reset(void);
void lis3mdl_reg_write(uint8_t reg,uint8_t value);
 //static void drv_lis3mdl_bubble_get(float pitch, float roll);

extern void drv_lis3mdl_enable(void);
extern void drv_lis3mdl_disable(void);

//extern void drv_lis3mdl_cail_init(void);
//extern uint8_t drv_lis3mdl_cail_status_get(void);
	
#endif
