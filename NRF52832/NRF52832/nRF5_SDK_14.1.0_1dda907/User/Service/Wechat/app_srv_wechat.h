#ifndef __APP_SRV_WECHAT_H__
#define __APP_SRV_WECHAT_H__




#include <stdint.h>
#ifndef COD 
#define 	WECHAT_STEP        0
#define 	WECHAT_DISTANCE    1
#define 	WECHAT_CALORY      2
typedef struct __attribute__((packed))
{
	uint8_t  flag;
	uint32_t step;
	uint32_t distance;
	uint32_t calory;
}_wechat_pedo;
typedef struct __attribute__((packed))
{
	uint8_t  flag;
	uint32_t step;
}_wechat_target;


void app_srv_wechat_init(void);
void app_srv_wechat_target_send(uint8_t *p_data, uint32_t length);
void app_srv_wechat_pedo_meas_send(uint8_t *p_data, uint32_t length);
#endif


#endif //__APP_SRV_WECHAT_H__
