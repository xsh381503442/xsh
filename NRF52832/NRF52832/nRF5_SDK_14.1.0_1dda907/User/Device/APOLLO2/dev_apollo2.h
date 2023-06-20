#ifndef __DEV_APOLLO2_H__
#define __DEV_APOLLO2_H__



#include "ble_srv_nrf52.h"
#include <stdint.h>
#include <stdbool.h>




typedef struct __attribute__ ((packed))
{

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
	
} _dev_apollo2_ble_info;


void dev_apollo2_ble_info_get(_dev_apollo2_ble_info *ble_info);
void dev_apollo2_ble_info_set(_dev_apollo2_ble_info *ble_info);


#endif //__DEV_APOLLO2_H__
