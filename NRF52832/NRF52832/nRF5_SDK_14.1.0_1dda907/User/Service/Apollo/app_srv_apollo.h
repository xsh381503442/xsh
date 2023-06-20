#ifndef __APP_SRV_APOLLO_H__
#define __APP_SRV_APOLLO_H__




#include <stdint.h>
#include "sdk_config.h"

typedef struct __attribute__((packed))
{
	uint8_t cmd;
	uint32_t crc;
	uint32_t read_back;
	uint32_t address;
	uint32_t size;
	uint8_t data[NRF_SDH_BLE_GATT_MAX_MTU_SIZE - 3];
}_apollo_srv_flash_read_raw;

typedef struct __attribute__((packed))
{
	uint8_t cmd;
	uint32_t crc;
	uint32_t read_back;
	uint32_t address;
	uint32_t size;
}_apollo_srv_flash_write_raw;

typedef struct __attribute__((packed))
{
	uint8_t cmd;
	uint32_t read_back;
	uint32_t address;
	uint32_t size;
}_apollo_srv_flash_erase_raw;

typedef struct __attribute__((packed))
{
	uint8_t cmd;
	uint32_t address;
	uint32_t size;
	uint32_t crc;
}_apollo_srv_flash_ble_dfu_postvalidate;


typedef struct __attribute__((packed))
{
		uint32_t                extFlash_address;
    uint32_t                image_size;         
    uint32_t                image_crc;  
		uint32_t                version;   	
    uint32_t                command;          
}_ble_image_bank;

typedef struct __attribute__((packed))
{
    uint32_t            boot_setting_crc;   
		uint32_t            boot_setting_size;
		uint32_t            pcb_version; 

    _ble_image_bank         bank_bootloader;             
    _ble_image_bank         bank_application;             
		_ble_image_bank         bank_softdevice; 
}_ble_boot_setting;


typedef struct
{
		uint32_t                extFlash_address;
		uint32_t                intFlash_address;
    uint32_t                image_size;         
    uint32_t                image_crc;  
		uint32_t                version;   	
    uint32_t                command;          
} _apollo_image_bank;
typedef struct
{
    uint32_t            boot_setting_crc;   
		uint32_t            boot_setting_size;
		uint32_t            bootloader_version; 	
    uint32_t            application_version;        
    uint32_t            update_version; 
		uint32_t            pcb_version; 

    _apollo_image_bank         bank_bootloader;             
    _apollo_image_bank         bank_application;             
		_apollo_image_bank         bank_update_algorithm; 

    uint32_t            enter_bootloader_reason;

		uint8_t model[5];
} _apollo_boot_setting;


#define BOOT_SETTING_BANK_COMMAND_DEFAULT                                0
#define BOOT_SETTING_BANK_COMMAND_CURRENT                                1
#define BOOT_SETTING_BANK_COMMAND_NEW                                    2


void app_srv_apollo_init(void);
void app_srv_apollo_uart_send(uint8_t *p_data, uint32_t length);
void app_srv_apollo_flash_send(uint8_t *p_data, uint32_t length);
void app_srv_apollo_sensor_send(uint8_t *p_data, uint32_t length);



#endif //__APP_SRV_APOLLO_H__
