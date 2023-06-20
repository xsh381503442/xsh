#ifndef __APP_SRV_NRF52_H__
#define __APP_SRV_NRF52_H__



#include <stdint.h>
#include "ble_srv_common.h"
#include "ble_srv_nrf52.h"
struct s_ble_passkey
{
	uint32_t command;
	uint8_t passkey[BLE_GAP_PASSKEY_LEN];

};
typedef struct s_ble_passkey _ble_passkey;

typedef struct __attribute__ ((packed)){
	uint8_t command;
	union __attribute__ ((packed)){
		uint8_t error;
		ble_config_dev_name_t dev;
	}param;
}_app_srv_nrf52_dev_name;

typedef struct __attribute__ ((packed)){
	uint8_t command;
	union __attribute__ ((packed)){
		uint8_t error;
		ble_config_adv_params_t    adv;
	}param;
}_app_srv_nrf52_adv_param;



typedef struct __attribute__ ((packed)){
    uint8_t command;
    union __attribute__ ((packed)){
		uint8_t error;
		ble_config_conn_params_t    conn_param;
	}param;
}_app_srv_nrf52_conn_param;


typedef struct __attribute__ ((packed))
{
	uint8_t command;
	uint8_t mac[BLE_GAP_ADDR_LEN];	
}_bond_request_android;


uint32_t app_srv_nrf52_init(void);
void app_srv_nrf52_config_send(uint8_t *p_data, uint32_t length);
uint32_t app_srv_nrf52_control_send(uint8_t *p_data, uint32_t length);
void cmd_nrf52_srv_bond_request(void);





#endif //__APP_SRV_NRF52_H__
