#ifndef DRV_KEY_H
#define	DRV_KEY_H

#include <stdint.h>
#include "watch_config.h"


#if defined( WATCH_MODEL_PLUS ) 
#define		KEY_LIGHT   	                 47
#define		KEY_DOWN			             22
#define		KEY_UP				             28
#define		KEY_BACK			             24
#define		KEY_OFFSET			             24
#define		KEY_MASK			             ((uint64_t)(0x800015400000))
#elif defined (WATCH_COD_BSP)
#define		KEY_LIGHT   	                 36
#define		KEY_DOWN			             47
#define		KEY_UP				             28
#define		KEY_BACK			             24
#define		KEY_OFFSET			             24
#define		KEY_MASK			             ((uint64_t)(0x801015000000))
#else
#define		KEY_LIGHT   	                 39
#define		KEY_DOWN			             47
#define		KEY_UP				             28
#define		KEY_BACK			             24
#define		KEY_OFFSET			             24
#define		KEY_MASK			             ((uint64_t)(0x808015000000))
#endif

#define		KEY_OK				             26

#define		KEY_NONE			             0

#define		KEY_LONG			             0x80
#define		KEY_POWER			            (KEY_BACK<<1)
#define		KEY_LONG_UP		        (KEY_UP<<1)
#define		KEY_LONG_DOWN	        (KEY_DOWN<<1)
#define		KEY_UP_DOWN			          ((KEY_UP<<1) + (KEY_DOWN<<1))
#define   KEY_LOCK                   (KEY_LIGHT<<1)
#define   KEY_LONG_OK                (KEY_OK<<1)
#define		READ_KEY_VALUE 		        ((uint32_t)((am_hal_gpio_input_read() & KEY_MASK) >> KEY_OFFSET))
#ifdef WATCH_COD_BSP
#define		KEY_UNLOCK_BACK_UP				((KEY_OK<<1) + (KEY_DOWN<<1))
#else
#define		KEY_UNLOCK_BACK_UP				((KEY_UP<<1) + (KEY_BACK<<1))
#endif

//wrist tilt parameters
#define DRV_LSM6DSL_WRIST_TILT_THS              45                      //arcsin(ths/64)
#define DRV_LSM6DSL_WRIST_TILT_POS_LAT          6                       // lat * 40ms
#define DRV_LSM6DSL_WRIST_TILT_NEG_LAT          2                       // lat * 40ms
//#define DRV_LSM6DSL_WRIST_TILT_MASK             0x08                    //z axes positive
#define DRV_LSM6DSL_WRIST_TILT_Z_ACC_THS        0x2d45                    //




typedef void (* button_callback)(uint32_t evt);


extern void drv_key_init(button_callback task_cb);



#endif
