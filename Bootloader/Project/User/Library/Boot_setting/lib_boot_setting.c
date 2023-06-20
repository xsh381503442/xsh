#include "lib_boot_setting.h"
#include "rtt_log.h"
#include "lib_error.h"
#include "com_apollo2.h"
#include "SEGGER_RTT.h"


#include "drv_intFlash.h"

#include "am_bootloader.h"


#include <string.h>

#define MOUDLE_LOG_ENABLED 							0
#define MOUDLE_DEBUG_ENABLED   					0
#define MOUDLE_NAME                     "[LIB_BOOT_SETTING]:"

#if MOUDLE_LOG_ENABLED == 1 && RTT_LOG_ENABLED == 1
#define LOG_PRINTF(...)                      SEGGER_RTT_printf(0, ##__VA_ARGS__) 
#define LOG_HEXDUMP(p_data, len)             rtt_log_hexdump(p_data, len) 
#else
#define LOG_PRINTF(...) 
#define LOG_HEXDUMP(p_data, len)             
#endif

#if MOUDLE_DEBUG_ENABLED == 1 && RTT_LOG_ENABLED == 1
#define DEBUG_PRINTF(...)                    SEGGER_RTT_printf(0, ##__VA_ARGS__)
#define DEBUG_HEXDUMP(p_data, len)           rtt_log_hexdump(p_data, len)
#else
#define DEBUG_PRINTF(...)   
#define DEBUG_HEXDUMP(p_data, len) 
#endif



uint8_t m_boot_setting_buffer[AM_HAL_FLASH_PAGE_SIZE]
		__attribute__((at(BOOT_SETTING_ADDRESS)))
		__attribute__((used));

void lib_boot_setting_read(_boot_setting *boot_setting)
{
	LOG_PRINTF(MOUDLE_NAME"lib_boot_setting_read\n");
	memcpy((void*)boot_setting, m_boot_setting_buffer, sizeof(_boot_setting));
	LOG_HEXDUMP((uint8_t *)boot_setting, sizeof(_boot_setting));	
}

void lib_boot_setting_write(_boot_setting *boot_setting)
{
	LOG_PRINTF(MOUDLE_NAME"lib_boot_setting_write\n");
	drv_intFlash_erase_page(BOOT_SETTING_ADDRESS);
	drv_intFlash_write_page(BOOT_SETTING_ADDRESS,(uint32_t *)boot_setting,sizeof(_boot_setting)/4);
}

void lib_boot_setting_init(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"lib_boot_setting_init\n");
	_boot_setting boot_setting;
	memset(&boot_setting,0,sizeof(_boot_setting));
	boot_setting.boot_setting_size = sizeof(_boot_setting);
	boot_setting.bootloader_version = 0;
	boot_setting.application_version = 0;
	boot_setting.update_version = 0;
	boot_setting.pcb_version = 0;
	boot_setting.bank_bootloader.extFlash_address = 0;
	boot_setting.bank_bootloader.intFlash_address = 0;
	boot_setting.bank_bootloader.image_size = 0;
	boot_setting.bank_bootloader.image_crc = 0;
	boot_setting.bank_bootloader.version = 0;
	boot_setting.bank_bootloader.command = 0;
	boot_setting.bank_application.extFlash_address = 0;
	boot_setting.bank_application.intFlash_address = 0;
	boot_setting.bank_application.image_size = 0;
	boot_setting.bank_application.image_crc = 0;
	boot_setting.bank_application.version = 0;
	boot_setting.bank_application.command = 0;
	boot_setting.bank_update_algorithm.extFlash_address = 0;
	boot_setting.bank_update_algorithm.intFlash_address = 0;
	boot_setting.bank_update_algorithm.image_size = 0;
	boot_setting.bank_update_algorithm.image_crc = 0;
	boot_setting.bank_update_algorithm.version = 0;
	boot_setting.bank_update_algorithm.command = 0;	
	boot_setting.enter_bootloader_reason = BOOT_SETTING_ENTER_BOOT_REASON_DEFAULT;
	boot_setting.boot_setting_crc = am_bootloader_fast_crc32(&boot_setting.boot_setting_size,boot_setting.boot_setting_size - 4);
	DEBUG_HEXDUMP((uint8_t *)&boot_setting, sizeof(_boot_setting));
	lib_boot_setting_write(&boot_setting);
}

bool lib_boot_setting_crc_check_bootsetting(_boot_setting *boot_setting)
{
	LOG_PRINTF(MOUDLE_NAME"lib_boot_setting_crc_check_bootsetting\n");	
	uint32_t crc = am_bootloader_fast_crc32(&boot_setting->boot_setting_size,boot_setting->boot_setting_size - 4);
	LOG_PRINTF(MOUDLE_NAME"crc = 0x%08x\n",crc);	
	
	if(crc == boot_setting->boot_setting_crc)
	{
		LOG_PRINTF(MOUDLE_NAME"crc is valid\n");
		return true;
	}
	else
	{
		LOG_PRINTF(MOUDLE_NAME"crc is invalid\n");
		return false;
	}	
	
}


bool lib_boot_setting_crc_check_bootloader(_boot_setting *boot_setting)
{
	DEBUG_PRINTF(MOUDLE_NAME"lib_boot_setting_crc_check_bootloader\n");				
	DEBUG_PRINTF(MOUDLE_NAME"address --> 0x%08X\n",boot_setting->bank_bootloader.intFlash_address);
	DEBUG_PRINTF(MOUDLE_NAME"size --> %d\n",boot_setting->bank_bootloader.image_size);
	uint32_t crc = am_bootloader_fast_crc32((uint32_t *)boot_setting->bank_bootloader.intFlash_address,boot_setting->bank_bootloader.image_size);
	DEBUG_PRINTF(MOUDLE_NAME"crc = 0x%08x\n",crc);
	
	if(crc == boot_setting->bank_bootloader.image_crc)
	{
		DEBUG_PRINTF(MOUDLE_NAME"crc is valid\n");
		return true;
	}
	else
	{
		DEBUG_PRINTF(MOUDLE_NAME"crc is invalid\n");
		return false;
	}
}

bool lib_boot_setting_crc_check_application(_boot_setting *boot_setting)
{
	DEBUG_PRINTF(MOUDLE_NAME"lib_boot_setting_crc_check_application\n");				
	DEBUG_PRINTF(MOUDLE_NAME"address --> 0x%08X\n",boot_setting->bank_application.intFlash_address);
	DEBUG_PRINTF(MOUDLE_NAME"size --> %d\n",boot_setting->bank_application.image_size);
	uint32_t crc = am_bootloader_fast_crc32((uint32_t *)boot_setting->bank_application.intFlash_address,boot_setting->bank_application.image_size);
	DEBUG_PRINTF(MOUDLE_NAME"crc = 0x%08x\n",crc);
	
	if(crc == boot_setting->bank_application.image_crc)
	{
		DEBUG_PRINTF(MOUDLE_NAME"crc is valid\n");
		return true;
	}
	else
	{
		DEBUG_PRINTF(MOUDLE_NAME"crc is invalid\n");
		return false;
	}
}
bool lib_boot_setting_crc_check_updateAlgorithm(_boot_setting *boot_setting)
{
	DEBUG_PRINTF(MOUDLE_NAME"lib_boot_setting_crc_check_updateAlgorithm\n");				
	DEBUG_PRINTF(MOUDLE_NAME"address --> 0x%08X\n",boot_setting->bank_update_algorithm.intFlash_address);
	DEBUG_PRINTF(MOUDLE_NAME"size --> %d\n",boot_setting->bank_update_algorithm.image_size);	
	uint32_t crc = am_bootloader_fast_crc32((uint32_t *)boot_setting->bank_update_algorithm.intFlash_address,boot_setting->bank_update_algorithm.image_size);
	DEBUG_PRINTF(MOUDLE_NAME"crc = 0x%08x\n",crc);
	
	if(crc == boot_setting->bank_update_algorithm.image_crc)
	{
		DEBUG_PRINTF(MOUDLE_NAME"crc is valid\n");
		return true;
	}
	else
	{
		DEBUG_PRINTF(MOUDLE_NAME"crc is invalid\n");
		return false;
	}
}







