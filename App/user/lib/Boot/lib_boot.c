#include "lib_boot.h"

#include "am_mcu_apollo.h"

#include "am_util_delay.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "portmacro.h"
#include "portable.h"
#include "event_groups.h"

void lib_boot_goto_image(am_bootloader_image_t *image)
{
	static am_bootloader_image_t m_image;
	m_image.pui32LinkAddress                         = image->pui32LinkAddress;
	m_image.ui32NumBytes                             = 0;
	m_image.ui32CRC                                  = 0;
	m_image.ui32OverridePolarity                     = 0;
	m_image.pui32StackPointer                        = 0;
	m_image.pui32ResetVector                         = 0;
	m_image.bEncrypted                               = 0;

	am_bootloader_image_run(&m_image);
}


bool lib_boot_jump_to_bootloader(uint32_t reason)
{
	_boot_setting boot_setting;
	lib_boot_setting_read(&boot_setting);
	if(lib_boot_setting_crc_check_bootsetting(&boot_setting))	
	{
		
		if(lib_boot_setting_crc_check_bootloader(&boot_setting))	
		{
			boot_setting.enter_bootloader_reason = reason;
			boot_setting.boot_setting_crc = am_bootloader_fast_crc32(&boot_setting.boot_setting_size,boot_setting.boot_setting_size - 4);
			lib_boot_setting_write(&boot_setting);
			taskENTER_CRITICAL();
			am_hal_reset_por();
		}		
	}
	return false;
}



bool lib_boot_jump_to_update_algorithm(void)
{
	_boot_setting boot_setting;
	lib_boot_setting_read(&boot_setting);
	if(lib_boot_setting_crc_check_bootsetting(&boot_setting))	
	{
		if(lib_boot_setting_crc_check_updateAlgorithm(&boot_setting))	
		{
			boot_setting.enter_bootloader_reason = BOOT_SETTING_ENTER_BOOT_REASON_JUMP_UPDATE_ALGORITHM;
			boot_setting.boot_setting_crc = am_bootloader_fast_crc32(&boot_setting.boot_setting_size,boot_setting.boot_setting_size - 4);
			lib_boot_setting_write(&boot_setting); 	
			taskENTER_CRITICAL();		
			am_hal_reset_por();
		}		
	}
	return false;
}










