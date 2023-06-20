#ifndef __DRV_MOTOR_H__
#define __DRV_MOTOR_H__

#include <stdint.h>
#include <stdbool.h>


#define DRV_MOTOR_DEFAULT_INTERVAL           1000                  
#define DRV_MOTOR_DEFAULT_WINDOW             500
#define DRV_MOTOR_DEFAULT_TIMES              5



typedef struct {
	uint32_t interval;
	uint32_t window;
	uint32_t times;
}_drv_motor_cfg;





void drv_motor_init(void);
void drv_motor_disable(void);
void drv_motor_enable(_drv_motor_cfg *cfg);




#endif //__DRV_MOTOR_H__

