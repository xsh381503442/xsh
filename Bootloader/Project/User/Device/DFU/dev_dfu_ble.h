#ifndef __DEV_DFU_BLE_H__
#define __DEV_DFU_BLE_H__
#include <stdint.h>
#include "YZL_BSP.h"

#ifdef COD 
typedef struct __attribute__((packed))
{
	uint32_t data_len;
	uint8_t  *data;
	uint32_t crc;
}cod_ota_data_info_t;

typedef struct __attribute__((packed)){
	uint32_t command;
	uint8_t  extra_info_len;
	uint8_t *extra_info_data;
	cod_ota_data_info_t apollo;
	uint16_t ble_id;
	cod_ota_data_info_t ble;
}cod_ota_file_info_t;
#endif





void dev_dfu_ble_start(void);







#endif //__DEV_DFU_BLE_H__
