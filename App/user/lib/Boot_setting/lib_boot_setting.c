#include "lib_boot_setting.h"


#include "drv_intFlash.h"

#include "am_bootloader.h"


#include <string.h>




//uint8_t m_boot_setting_buffer[AM_HAL_FLASH_PAGE_SIZE]
//		__attribute__((at(BOOT_SETTING_ADDRESS)))
//		__attribute__((used));


void lib_boot_setting_read(_boot_setting *boot_setting)
{
//	memcpy((void*)boot_setting, m_boot_setting_buffer, sizeof(_boot_setting));	
	
	memcpy((void*)boot_setting, (uint8_t *)BOOT_SETTING_ADDRESS, sizeof(_boot_setting));
}

void lib_boot_setting_write(_boot_setting *boot_setting)
{
	drv_intFlash_erase_page(BOOT_SETTING_ADDRESS);
	drv_intFlash_write_page(BOOT_SETTING_ADDRESS,(uint32_t *)boot_setting,sizeof(_boot_setting)/4);
}

void lib_boot_setting_init(void)
{
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

	lib_boot_setting_write(&boot_setting);
}

bool lib_boot_setting_crc_check_bootsetting(_boot_setting *boot_setting)
{

	uint32_t crc = am_bootloader_fast_crc32(&boot_setting->boot_setting_size,boot_setting->boot_setting_size - 4);

	if(crc == boot_setting->boot_setting_crc)
	{
		return true;
	}
	else
	{
		return false;
	}	
	
}


bool lib_boot_setting_crc_check_bootloader(_boot_setting *boot_setting)
{
	uint32_t crc = am_bootloader_fast_crc32((uint32_t *)boot_setting->bank_bootloader.intFlash_address,boot_setting->bank_bootloader.image_size);
	
	if(crc == boot_setting->bank_bootloader.image_crc)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool lib_boot_setting_crc_check_application(_boot_setting *boot_setting)
{			
	uint32_t crc = am_bootloader_fast_crc32((uint32_t *)boot_setting->bank_application.intFlash_address,boot_setting->bank_application.image_size);
	
	if(crc == boot_setting->bank_application.image_crc)
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool lib_boot_setting_crc_check_updateAlgorithm(_boot_setting *boot_setting)
{
	uint32_t crc = am_bootloader_fast_crc32((uint32_t *)boot_setting->bank_update_algorithm.intFlash_address,boot_setting->bank_update_algorithm.image_size);
	
	if(crc == boot_setting->bank_update_algorithm.image_crc)
	{
		return true;
	}
	else
	{
		return false;
	}
}







