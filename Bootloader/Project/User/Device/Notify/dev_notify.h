#ifndef __DEV_NOTIFY_H__
#define __DEV_NOTIFY_H__




#include <stdint.h>
#include "drv_motor.h"


typedef enum {

	DEV_NOTIFY_FEATURE_MOTOR,

	
}_dev_notify_feature;

typedef uint32_t _dev_notify_feature_mask;

#define DEV_NOTIFY_FEATURE_MASK_MOTOR                (1UL << DEV_NOTIFY_FEATURE_MOTOR)



typedef struct {

	_drv_motor_cfg          motor_cfg;

}_dev_notify_cfg;


typedef struct {

	_dev_notify_feature_mask                    evt_mask;
	_dev_notify_cfg                             cfg;

}_dev_notify;






void dev_notify_init(void);
void dev_notify_enable(_dev_notify *notify);
void dev_notify_disable(_dev_notify *notify);


#endif //__DEV_NOTIFY_H__
