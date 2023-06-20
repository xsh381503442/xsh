#include "dev_dfu_ble.h"
#include "rtt_log.h"
#include "lib_error.h"
#include "SEGGER_RTT.h"

#include "am_mcu_apollo.h"

#include "am_bootloader.h"

#include "lib_boot.h"

#include "cmd_pc.h"
#include <stdio.h>



#ifdef AM_FREERTOS 
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "portmacro.h"
#include "portable.h"
#include "event_groups.h"
#endif 



#ifdef LAYER_BOOTLOADER  
#include "gui_boot.h"
#include "drv_pc.h"
#include "drv_ble.h"
#include "com_apollo2.h"
#include "lib_boot_setting.h"
#include "drv_extFlash.h"
#include "dev_extFlash.h"
#include "dev_ble.h"
#include "timer_exception.h"
#endif 


#include "am_util_delay.h"
#include <string.h>
#include "drv_intFlash.h"
#include "am_util.h"


#define MOUDLE_LOG_ENABLED 							1
#define MOUDLE_DEBUG_ENABLED   					1
#define MOUDLE_NAME                     "[DEV_DFU_BLE]:"

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

#define DELAY_MS(X)                 am_util_delay_ms(X)
//#define DELAY_MS(X)                 vTaskDelay(X / portTICK_PERIOD_MS)
//#define DELAY_MS(X)                   do{TickType_t xLastWakeTime;xLastWakeTime = xTaskGetTickCount ();vTaskDelayUntil( &xLastWakeTime, X / portTICK_PERIOD_MS );}while(0)
#ifdef COD 
#define EXT_FLASH_OAT_ADDRESS               0x700000
#define APPLICATION_ADDRESS_OFFSET          1024

static void read_apollo_bootsetting_application_address(uint32_t *bootsetting_addr)
{
    DEBUG_PRINTF("[%s]",__func__);
	cod_ota_file_info_t cod_ota_file;
	memset((uint8_t *)&cod_ota_file,0,sizeof(cod_ota_file_info_t));	

	dev_extFlash_read(EXT_FLASH_OAT_ADDRESS + sizeof(cod_ota_file.command), (uint8_t *)&cod_ota_file.extra_info_len, sizeof(cod_ota_file.extra_info_len));
	*bootsetting_addr = EXT_FLASH_OAT_ADDRESS + sizeof(cod_ota_file.command) + sizeof(cod_ota_file.extra_info_len) + cod_ota_file.extra_info_len + sizeof(cod_ota_file.apollo.data_len);

	DEBUG_PRINTF("bootsetting_addr --> 0x%08X",*bootsetting_addr);
}
#endif
static bool check_crc_from_external_flash(uint32_t input_crc,uint32_t external_flash_address ,uint32_t size)
{
	uint32_t imageSizeWritten = size;
	uint32_t bytesInBuffer;
	uint32_t currentAddr = external_flash_address;
	uint32_t output_crc = 0;
	uint8_t buffer[512];
	while(imageSizeWritten)
	{
		bytesInBuffer = imageSizeWritten < sizeof(buffer) ? imageSizeWritten : sizeof(buffer);
		dev_extFlash_enable();
		dev_extFlash_read(currentAddr, buffer, bytesInBuffer);
		dev_extFlash_disable();	
		am_bootloader_partial_crc32(buffer, bytesInBuffer, &output_crc);
		currentAddr += bytesInBuffer;
		imageSizeWritten -= bytesInBuffer;
	}	
	if(input_crc == output_crc)
	{
		return true;
	}

	return false;
}

static _boot_setting m_flash_boot_setting;


static void dev_dfu_flash_memcpy(uint32_t dest_addr,uint32_t src_addr,uint32_t size)
{
	static uint32_t buf[AM_HAL_FLASH_PAGE_SIZE/4];
	uint32_t error = ERR_SUCCESS;
	uint32_t src_addr_t = src_addr;
	uint32_t *p_dest_addr_t = (uint32_t *)dest_addr;
	uint32_t size_buf;
	uint32_t size_remain = size;
	uint8_t *p8_buf = (uint8_t *) buf;


	while(size_remain)
	{
		size_buf = size_remain < AM_HAL_FLASH_PAGE_SIZE ? size_remain : AM_HAL_FLASH_PAGE_SIZE;

		dev_extFlash_enable();
		dev_extFlash_read(src_addr_t, p8_buf, size_buf);
		dev_extFlash_disable();			
		src_addr_t += size_buf;

#ifdef AM_FREERTOS 	
	taskENTER_CRITICAL();
#endif 
		error = am_hal_flash_page_erase(AM_HAL_FLASH_PROGRAM_KEY,                                             
														AM_HAL_FLASH_ADDR2INST((uint32_t)p_dest_addr_t),
														AM_HAL_FLASH_ADDR2PAGE((uint32_t)p_dest_addr_t));
		ERR_CHECK(error);		
	  error = am_hal_flash_program_main(AM_HAL_FLASH_PROGRAM_KEY,
																						buf,
																						p_dest_addr_t,
																						size_buf/4);	
		ERR_CHECK(error);
#ifdef AM_FREERTOS	
	taskEXIT_CRITICAL();	
#endif		
		p_dest_addr_t += (size_buf / 4);
		size_remain -= size_buf;
	}
}


static uint32_t copy_image(void)
{
	LOG_PRINTF(MOUDLE_NAME"copy_image\n");
	
	uint32_t error = ERR_SUCCESS;
	uint32_t bank_addr_src = 0;
	uint32_t bank_addr_dest = 0;
	uint32_t bank_size = 0;
	uint32_t bank_crc = 0;
	
	if(m_flash_boot_setting.bank_update_algorithm.command == BOOT_SETTING_BANK_COMMAND_NEW)
	{
		LOG_PRINTF(MOUDLE_NAME"bank_update_algorithm BOOT_SETTING_BANK_COMMAND_NEW\n");
		LOG_PRINTF(MOUDLE_NAME"bank_update_algorithm version --> 0x%08X\n",m_flash_boot_setting.bank_update_algorithm.version);
		LOG_PRINTF(MOUDLE_NAME"bank_update_algorithm image_crc --> 0x%08X\n",m_flash_boot_setting.bank_update_algorithm.image_crc);
		LOG_PRINTF(MOUDLE_NAME"bank_update_algorithm image_size --> 0x%08X\n",m_flash_boot_setting.bank_update_algorithm.image_size);
		LOG_PRINTF(MOUDLE_NAME"bank_update_algorithm extFlash_address --> 0x%08X\n",m_flash_boot_setting.bank_update_algorithm.extFlash_address);
		LOG_PRINTF(MOUDLE_NAME"bank_update_algorithm intFlash_address --> 0x%08X\n",m_flash_boot_setting.bank_update_algorithm.intFlash_address);		
		
		bank_addr_src = m_flash_boot_setting.bank_update_algorithm.extFlash_address;
		bank_addr_dest = m_flash_boot_setting.bank_update_algorithm.intFlash_address;
		bank_size = m_flash_boot_setting.bank_update_algorithm.image_size;
		bank_crc = m_flash_boot_setting.bank_update_algorithm.image_crc;		
		
		dev_dfu_flash_memcpy(bank_addr_dest,bank_addr_src,bank_size);
		uint32_t crc_dest = am_bootloader_fast_crc32((uint32_t *)bank_addr_dest,bank_size);
		LOG_PRINTF(MOUDLE_NAME"crc_dest = 0x%08x\n",crc_dest);
		if(crc_dest != bank_crc)
		{
			return ERR_CRC;
		}
		m_flash_boot_setting.update_version = m_flash_boot_setting.bank_update_algorithm.version;
		m_flash_boot_setting.bank_update_algorithm.command = BOOT_SETTING_BANK_COMMAND_DEFAULT;				
	}	
	
	
	return error;
}

static uint32_t check_image(void)
{
	
	LOG_PRINTF(MOUDLE_NAME"check_image\n");
	
	uint32_t error = ERR_SUCCESS;
	uint32_t bank_addr = 0;
	uint32_t bank_size = 0;
	uint32_t bank_crc = 0;		
	
	
	if(m_flash_boot_setting.bank_bootloader.command == BOOT_SETTING_BANK_COMMAND_NEW)
	{
		LOG_PRINTF(MOUDLE_NAME"bank_bootloader BOOT_SETTING_BANK_COMMAND_NEW\n");
		LOG_PRINTF(MOUDLE_NAME"bank_bootloader version --> 0x%08X\n",m_flash_boot_setting.bank_bootloader.version);
		LOG_PRINTF(MOUDLE_NAME"bank_bootloader image_crc --> 0x%08X\n",m_flash_boot_setting.bank_bootloader.image_crc);
		LOG_PRINTF(MOUDLE_NAME"bank_bootloader image_size --> 0x%08X\n",m_flash_boot_setting.bank_bootloader.image_size);
		LOG_PRINTF(MOUDLE_NAME"bank_bootloader extFlash_address --> 0x%08X\n",m_flash_boot_setting.bank_bootloader.extFlash_address);
		LOG_PRINTF(MOUDLE_NAME"bank_bootloader intFlash_address --> 0x%08X\n",m_flash_boot_setting.bank_bootloader.intFlash_address);
		
		bank_addr = m_flash_boot_setting.bank_bootloader.extFlash_address;
		bank_size = m_flash_boot_setting.bank_bootloader.image_size;
		bank_crc = m_flash_boot_setting.bank_bootloader.image_crc;		
		if(check_crc_from_external_flash(bank_crc,bank_addr,bank_size))
		{
			LOG_PRINTF(MOUDLE_NAME"bank_bootloader crc is correct\n");
		}
		else
		{
			LOG_PRINTF(MOUDLE_NAME"bank_bootloader crc is wrong\n");
			return ERR_CRC;
		}
		
		
	}
	if(m_flash_boot_setting.bank_application.command == BOOT_SETTING_BANK_COMMAND_NEW)
	{
		LOG_PRINTF(MOUDLE_NAME"bank_application BOOT_SETTING_BANK_COMMAND_NEW\n");
		LOG_PRINTF(MOUDLE_NAME"bank_application version --> 0x%08X\n",m_flash_boot_setting.bank_application.version);
		LOG_PRINTF(MOUDLE_NAME"bank_application image_crc --> 0x%08X\n",m_flash_boot_setting.bank_application.image_crc);
		LOG_PRINTF(MOUDLE_NAME"bank_application image_size --> 0x%08X\n",m_flash_boot_setting.bank_application.image_size);
		LOG_PRINTF(MOUDLE_NAME"bank_application extFlash_address --> 0x%08X\n",m_flash_boot_setting.bank_application.extFlash_address);
		LOG_PRINTF(MOUDLE_NAME"bank_application intFlash_address --> 0x%08X\n",m_flash_boot_setting.bank_application.intFlash_address);		
		
		bank_addr = m_flash_boot_setting.bank_application.extFlash_address;
		bank_size = m_flash_boot_setting.bank_application.image_size;
		bank_crc = m_flash_boot_setting.bank_application.image_crc;		
		if(check_crc_from_external_flash(bank_crc,bank_addr,bank_size))
		{
			LOG_PRINTF(MOUDLE_NAME"bank_application crc is correct\n");
		}
		else
		{
			LOG_PRINTF(MOUDLE_NAME"bank_application crc is wrong\n");
			return ERR_CRC;
		}		
		
		
	}
	if(m_flash_boot_setting.bank_update_algorithm.command == BOOT_SETTING_BANK_COMMAND_NEW)
	{
		LOG_PRINTF(MOUDLE_NAME"bank_update_algorithm BOOT_SETTING_BANK_COMMAND_NEW\n");
		LOG_PRINTF(MOUDLE_NAME"bank_update_algorithm version --> 0x%08X\n",m_flash_boot_setting.bank_update_algorithm.version);
		LOG_PRINTF(MOUDLE_NAME"bank_update_algorithm image_crc --> 0x%08X\n",m_flash_boot_setting.bank_update_algorithm.image_crc);
		LOG_PRINTF(MOUDLE_NAME"bank_update_algorithm image_size --> 0x%08X\n",m_flash_boot_setting.bank_update_algorithm.image_size);
		LOG_PRINTF(MOUDLE_NAME"bank_update_algorithm extFlash_address --> 0x%08X\n",m_flash_boot_setting.bank_update_algorithm.extFlash_address);
		LOG_PRINTF(MOUDLE_NAME"bank_update_algorithm intFlash_address --> 0x%08X\n",m_flash_boot_setting.bank_update_algorithm.intFlash_address);		
		
		bank_addr = m_flash_boot_setting.bank_update_algorithm.extFlash_address;
		bank_size = m_flash_boot_setting.bank_update_algorithm.image_size;
		bank_crc = m_flash_boot_setting.bank_update_algorithm.image_crc;		
		if(check_crc_from_external_flash(bank_crc,bank_addr,bank_size))
		{
			LOG_PRINTF(MOUDLE_NAME"bank_update_algorithm crc is correct\n");
		}
		else
		{
			LOG_PRINTF(MOUDLE_NAME"bank_update_algorithm crc is wrong\n");
			return ERR_CRC;
		}		
	}	

	return error;
}

static uint32_t check_bootsetting(void)
{
	uint32_t error = ERR_SUCCESS;

	LOG_PRINTF(MOUDLE_NAME"check_bootsetting\n");
#ifdef COD 
	uint32_t bootsetting_address = 0x0; 
	_boot_setting boot_setting;
	lib_boot_setting_read(&boot_setting);		
	
	LOG_PRINTF(MOUDLE_NAME"boot_setting.application_version --> 0x%08X\n",boot_setting.application_version);
	LOG_PRINTF(MOUDLE_NAME"boot_setting.bootloader_version --> 0x%08X\n",boot_setting.bootloader_version);
	LOG_PRINTF(MOUDLE_NAME"boot_setting.update_version --> 0x%08X\n",boot_setting.update_version);
	LOG_PRINTF(MOUDLE_NAME"boot_setting.pcb_version --> 0x%08X\n",boot_setting.pcb_version);
	
	
	dev_extFlash_enable();
    read_apollo_bootsetting_application_address(&bootsetting_address);
	dev_extFlash_read(bootsetting_address, (uint8_t *)&m_flash_boot_setting, sizeof(_boot_setting));    
	dev_extFlash_disable();		
	
	LOG_PRINTF(MOUDLE_NAME"flash_boot_setting.boot_setting_crc --> 0x%08X\n",m_flash_boot_setting.boot_setting_crc);
	
	uint32_t crc_dest = am_bootloader_fast_crc32((uint8_t*)(&m_flash_boot_setting.boot_setting_crc)+sizeof(m_flash_boot_setting.boot_setting_crc),sizeof(_boot_setting) - sizeof(m_flash_boot_setting.boot_setting_crc));
	LOG_PRINTF(MOUDLE_NAME"crc_dest --> 0x%08x\n",crc_dest);
	
	
	if(crc_dest != m_flash_boot_setting.boot_setting_crc)
	{
		return ERR_CRC;
	}
	//modify apollo bank_application extFlash_address
    m_flash_boot_setting.bank_application.extFlash_address = bootsetting_address + APPLICATION_ADDRESS_OFFSET;
#else		
	_boot_setting boot_setting;
	lib_boot_setting_read(&boot_setting);		
	
	LOG_PRINTF(MOUDLE_NAME"boot_setting.application_version --> 0x%08X\n",boot_setting.application_version);
	LOG_PRINTF(MOUDLE_NAME"boot_setting.bootloader_version --> 0x%08X\n",boot_setting.bootloader_version);
	LOG_PRINTF(MOUDLE_NAME"boot_setting.update_version --> 0x%08X\n",boot_setting.update_version);
	LOG_PRINTF(MOUDLE_NAME"boot_setting.pcb_version --> 0x%08X\n",boot_setting.pcb_version);
	
	
	dev_extFlash_enable();
	dev_extFlash_read(0x800000-0x1000, (uint8_t *)&m_flash_boot_setting, sizeof(_boot_setting));
	dev_extFlash_disable();		
	
	LOG_PRINTF(MOUDLE_NAME"flash_boot_setting.boot_setting_crc --> 0x%08X\n",m_flash_boot_setting.boot_setting_crc);
	
	uint32_t crc_dest = am_bootloader_fast_crc32((uint8_t*)(&m_flash_boot_setting.boot_setting_crc)+sizeof(m_flash_boot_setting.boot_setting_crc),sizeof(_boot_setting) - sizeof(m_flash_boot_setting.boot_setting_crc));
	LOG_PRINTF(MOUDLE_NAME"crc_dest --> 0x%08x\n",crc_dest);
	
	
	if(crc_dest != m_flash_boot_setting.boot_setting_crc)
	{
		return ERR_CRC;
	}
	
#endif
	
	return error;
}


static uint32_t write_bootsetting(void)
{
	LOG_PRINTF(MOUDLE_NAME"write_bootsetting\n");
	
	uint32_t error = ERR_SUCCESS;

	_boot_setting boot_setting;
	lib_boot_setting_read(&boot_setting);		
	
	m_flash_boot_setting.bootloader_version = boot_setting.bootloader_version;
	m_flash_boot_setting.application_version = boot_setting.application_version;
	m_flash_boot_setting.update_version = boot_setting.update_version;
//	m_boot_setting.pcb_version = boot_setting.pcb_version;
	m_flash_boot_setting.bank_update_algorithm.intFlash_address = boot_setting.bank_update_algorithm.intFlash_address;
	m_flash_boot_setting.enter_bootloader_reason = BOOT_SETTING_ENTER_BOOT_REASON_JUMP_UPDATE_ALGORITHM;	
	
	
	if(m_flash_boot_setting.bank_update_algorithm.command == BOOT_SETTING_BANK_COMMAND_DEFAULT)
	{
		memcpy(&m_flash_boot_setting.bank_update_algorithm,&boot_setting.bank_update_algorithm,sizeof(_image_bank));
	}
	

	if(m_flash_boot_setting.bank_bootloader.command == BOOT_SETTING_BANK_COMMAND_DEFAULT &&
		 m_flash_boot_setting.bank_application.command == BOOT_SETTING_BANK_COMMAND_DEFAULT 
	  )//BOOTLOADER和APPLICATION两个软件都不升级
	{
		memcpy(&m_flash_boot_setting.bank_application,&boot_setting.bank_application,sizeof(_image_bank));
		memcpy(&m_flash_boot_setting.bank_bootloader,&boot_setting.bank_bootloader,sizeof(_image_bank));
		m_flash_boot_setting.enter_bootloader_reason = BOOT_SETTING_ENTER_BOOT_REASON_JUMP_APPLICATION;
	}
	else if(m_flash_boot_setting.bank_application.command == BOOT_SETTING_BANK_COMMAND_DEFAULT)
	{//APPLICATION软件不升级
		memcpy(&m_flash_boot_setting.bank_application,&boot_setting.bank_application,sizeof(_image_bank));
	}
	else if(m_flash_boot_setting.bank_bootloader.command == BOOT_SETTING_BANK_COMMAND_DEFAULT)
	{//BOOTLOADER软件不升级
		memcpy(&m_flash_boot_setting.bank_bootloader,&boot_setting.bank_bootloader,sizeof(_image_bank));
	}	
	
	
	LOG_PRINTF(MOUDLE_NAME"boot_setting_size --> %d\n",m_flash_boot_setting.boot_setting_size);

	m_flash_boot_setting.boot_setting_crc = am_bootloader_fast_crc32(&m_flash_boot_setting.boot_setting_size,m_flash_boot_setting.boot_setting_size-4);
	lib_boot_setting_write(&m_flash_boot_setting);
	lib_boot_setting_read(&boot_setting);

	LOG_HEXDUMP((uint8_t *)&boot_setting, sizeof(_boot_setting));	
	if(!lib_boot_setting_crc_check_bootsetting(&boot_setting))	
	{
		return ERR_CRC;
	}			
	
	LOG_PRINTF(MOUDLE_NAME"write_bootsetting success\n");

	
	return error;
}


void dev_dfu_ble_start(void)
{
	LOG_PRINTF(MOUDLE_NAME"dev_dfu_ble_start\n");

	uint32_t error = ERR_SUCCESS;
	

	
	
	error = check_bootsetting();
	ERR_CHECK(error);
	
	error = check_image();
	ERR_CHECK(error);	
	
	error = copy_image();
	ERR_CHECK(error);	
	
	error = write_bootsetting();
	ERR_CHECK(error);		
	
	

	DELAY_MS(500);
#ifdef AM_FREERTOS 	
	taskENTER_CRITICAL();
#endif 		
	am_hal_reset_por();	
	
	
	while(1);
	
	
}














