#ifndef __LIB_BOOT_SETTING_H__
#define __LIB_BOOT_SETTING_H__


#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "am_mcu_apollo.h"



#define BOOTLOADER_START_DEFAULT_ADDRESS                      (0x00000000UL)
#define APPLICATION_START_DEFAULT_ADDRESS                     (0x00010000UL)
#define BOOTLOADER_DEFAULT_SIZE                               (APPLICATION_START_DEFAULT_ADDRESS - BOOTLOADER_START_DEFAULT_ADDRESS)
#define APP_DATA_ADDRESS                                      (0x000F8000UL)
#define APPLICATION_DEFAULT_SIZE                              (APP_DATA_ADDRESS - APPLICATION_START_DEFAULT_ADDRESS)
#define BOOT_SETTING_ADDRESS                                  (0x000FA000UL)
#define UPDATE_ALGORITHM_START_DEFAULT_ADDRESS                (0x000FC000UL)
#define UPDATE_ALGORITHM_DEFAULT_SIZE                         (APOLLO2_FLASH_SIZE - UPDATE_ALGORITHM_START_DEFAULT_ADDRESS)


#define BOOT_SETTING_ENTER_BOOT_REASON_DEFAULT                           0
#define BOOT_SETTING_ENTER_BOOT_REASON_HARDWARE_TEST                     1
#define BOOT_SETTING_ENTER_BOOT_REASON_DFU_PC                            2
#define BOOT_SETTING_ENTER_BOOT_REASON_DFU_BLE                           3
#define BOOT_SETTING_ENTER_BOOT_REASON_JUMP_APPLICATION                  4
#define BOOT_SETTING_ENTER_BOOT_REASON_JUMP_UPDATE_ALGORITHM             5

#define BOOT_SETTING_BANK_COMMAND_DEFAULT                                0
#define BOOT_SETTING_BANK_COMMAND_CURRENT                                1
#define BOOT_SETTING_BANK_COMMAND_NEW                                    2


typedef struct
{
		uint32_t                extFlash_address;
		uint32_t                intFlash_address;
    uint32_t                image_size;         
    uint32_t                image_crc;  
		uint32_t                version;   	
    uint32_t                command;          
} _image_bank;



typedef struct
{
    uint32_t            boot_setting_crc;   
		uint32_t            boot_setting_size;
		uint32_t            bootloader_version; 	//手表当前版本
    uint32_t            application_version;    	//手表当前版本
    uint32_t            update_version; 	//手表当前版本
		uint32_t            pcb_version; 	//手表当前版本

    _image_bank         bank_bootloader;             
    _image_bank         bank_application;             
		_image_bank         bank_update_algorithm; 

    uint32_t            enter_bootloader_reason;
	
		uint8_t model[5];
} _boot_setting;



typedef struct
{
		uint32_t                extFlash_address;
    uint32_t                image_size;         
    uint32_t                image_crc;  
		uint32_t                version;   	
    uint32_t                command;          
} _ble_image_bank;


typedef struct
{
    uint32_t            boot_setting_crc;   
		uint32_t            boot_setting_size;
		uint32_t            pcb_version; 

    _ble_image_bank         bank_bootloader;             
    _ble_image_bank         bank_application;             
		_ble_image_bank         bank_softdevice; 
} _ble_boot_setting;







void lib_boot_setting_write(_boot_setting *boot_setting);
void lib_boot_setting_read(_boot_setting *boot_setting);
void lib_boot_setting_init(void);
bool lib_boot_setting_crc_check_bootsetting(_boot_setting *boot_setting);
bool lib_boot_setting_crc_check_bootloader(_boot_setting *boot_setting);
bool lib_boot_setting_crc_check_application(_boot_setting *boot_setting);
bool lib_boot_setting_crc_check_updateAlgorithm(_boot_setting *boot_setting);


#endif //__LIB_BOOT_SETTING_H__
