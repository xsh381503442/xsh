#ifndef __DEV_BLE_H__
#define __DEV_BLE_H__


#include <stdint.h>

#define BLE_CONFIG_DEVICE_NAME_LEN_MAX 10
typedef struct __attribute__ ((packed))
{
    uint8_t name[BLE_CONFIG_DEVICE_NAME_LEN_MAX+1];
    uint8_t len;
}ble_config_dev_name_t;

typedef struct __attribute__ ((packed)){

	uint32_t command;
	uint32_t setting_version;
	uint32_t bootloader_version;
	uint32_t application_version;
	uint32_t softdevice_version;
	uint32_t pcb_version;
	uint32_t mac_address_f4b;
	uint32_t mac_address_l2b;
	ble_config_dev_name_t      dev_name;
	
	uint32_t build_date;
	uint32_t build_time;

	
} _dev_ble_info;


void dev_ble_init(void);
void dev_ble_system_on(void);
void dev_ble_system_off(void);





#endif //__DEV_BLE_TO_PC_H__
