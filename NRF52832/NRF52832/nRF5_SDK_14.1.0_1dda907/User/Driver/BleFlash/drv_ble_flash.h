#ifndef __DRV_BLE_FLASH_H__
#define __DRV_BLE_FLASH_H__

#include "fds.h"
#include "ble_srv_nrf52.h"
#include <stdint.h>

#define ANCS_MESSAGE_SWITCH_ON          0x1       
#define ANCS_MESSAGE_SWITCH_OFF         0x2  

typedef struct __attribute__ ((packed)) __attribute__((aligned(4))) 
{
	uint8_t call;
	uint8_t wechat;
	uint8_t qq;
	uint8_t sms;
#ifdef COD
	uint8_t company_wechat;
	uint8_t dingding;
	uint8_t reserved_3;
	uint8_t reserved_4;
#else
	uint8_t reserved_1;
	uint8_t reserved_2;
	uint8_t reserved_3;
	uint8_t reserved_4;
#endif
}_drv_ble_flash_ancs_switch;



#define ANDROID_BOND_MAX        20
typedef struct __attribute__((packed))
{
	uint8_t mac[BLE_GAP_ADDR_LEN];
}_android_bond;



uint32_t drv_ble_flash_config_store(ble_config_params_t * p_config);
ret_code_t drv_ble_flash_init(ble_config_params_t * p_default_config,
															ble_config_params_t       ** p_config);
ret_code_t drv_ble_flash_delete(void);


uint32_t drv_ble_flash_android_mac_write(uint8_t index, uint8_t *mac);
uint32_t drv_ble_flash_android_mac_delete_all(void);
uint32_t drv_ble_flash_android_mac_delete(uint8_t index);
uint32_t drv_ble_flash_android_mac_init(void);


uint32_t drv_ble_flash_ancs_switch_read(_drv_ble_flash_ancs_switch *ancs_switch);
uint32_t drv_ble_flash_ancs_switch_write(_drv_ble_flash_ancs_switch *ancs_switch);
uint32_t drv_ble_flash_ancs_switch_init(_drv_ble_flash_ancs_switch *ancs_switch);

#ifdef COD
uint32_t drv_ble_flash_cod_mac_read(ble_gap_addr_t * device_addr);
uint32_t drv_ble_flash_cod_mac_write(ble_gap_addr_t * device_addr);
uint32_t drv_ble_flash_cod_mac_init(ble_gap_addr_t * device_addr);
#endif


#endif //__DRV_BLE_FLASH_H__
