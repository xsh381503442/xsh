#ifdef BOSH_OFFICIAL_SITE_API
#ifndef __DRV_BOSH_OFFICIAL_API_H__
#define __DRV_BOSH_OFFICIAL_API_H__



#include "bmi160_official_api.h"

typedef struct {

	struct bmi160_sensor_data accel;
	struct bmi160_sensor_data gyro;

} _bmi160_data;




void drv_bosh_official_api_init(void);
void drv_bosh_official_api_uninit(void);
void drv_bosh_official_api_read_acc(void);





#endif //__DRV_BOSH_OFFICIAL_API_H__
#endif //BOSH_OFFICIAL_SITE_API



