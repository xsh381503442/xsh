#include "dev_dfu.h"
#include "rtt_log.h"
#include "lib_error.h"
#include "SEGGER_RTT.h"

#include "am_mcu_apollo.h"

#include "am_bootloader.h"

#include "lib_boot.h"

#include "cmd_pc.h"
#include <stdio.h>

#ifdef LAYER_UPDATEALGORITHM  
#include "lib_scheduler.h"
#include "dev_extFlash.h"
#include "drv_pc.h"
#endif

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


#define MOUDLE_LOG_ENABLED 							0
#define MOUDLE_DEBUG_ENABLED   					0
#define MOUDLE_NAME                     "[DEV_DFU]:"

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



//#define DELAY_MS(X)                 am_util_delay_ms(X)
#define DELAY_MS(X)                 vTaskDelay(X / portTICK_PERIOD_MS)
//#define DELAY_MS(X)                   do{TickType_t xLastWakeTime;xLastWakeTime = xTaskGetTickCount ();vTaskDelayUntil( &xLastWakeTime, X / portTICK_PERIOD_MS );}while(0)


#ifdef LAYER_BOOTLOADER  
_dev_dfu_progress g_dev_dfu_progress;
#endif 




static void dev_dfu_flash_memcpy(uint32_t dest_addr,uint32_t src_addr,uint32_t size)
{
	LOG_PRINTF(MOUDLE_NAME"dev_dfu_flash_memcpy\n");
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
		
		LOG_PRINTF(MOUDLE_NAME"src_addr_t --> 0x%08x\n",src_addr_t);
		LOG_PRINTF(MOUDLE_NAME"dest_addr_t --> 0x%08x\n",(uint32_t)p_dest_addr_t);
		LOG_PRINTF(MOUDLE_NAME"size_buf --> 0x%08x\n",size_buf);
		
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





#ifndef LAYER_UPDATEALGORITHM  



typedef struct {
	uint32_t command;
	uint32_t offset;
	uint32_t size;
	uint32_t crc;
}_dev_dfu_bootsetting;

typedef struct {
	uint32_t command;
	uint32_t offset;
	uint32_t size;
	uint32_t crc;
}_dev_dfu_image_data;


#define PREVALIDATE_ERROR_MASK_DEFAULT                (0)
#define PREVALIDATE_ERROR_MASK_BOOT_SETTING_CRC       (1UL << 0)
#define PREVALIDATE_ERROR_MASK_PCB_VERSION            (1UL << 1)
#define PREVALIDATE_ERROR_MASK_BANK_BOOT_EXT_ADDRESS  (1UL << 2)
#define PREVALIDATE_ERROR_MASK_BANK_BOOT_INT_ADDRESS  (1UL << 3)
#define PREVALIDATE_ERROR_MASK_BANK_BOOT_SIZE         (1UL << 4)
#define PREVALIDATE_ERROR_MASK_BANK_BOOT_CRC          (1UL << 5)
#define PREVALIDATE_ERROR_MASK_BANK_BOOT_VERSION      (1UL << 6)
#define PREVALIDATE_ERROR_MASK_BANK_APP_EXT_ADDRESS   (1UL << 7)
#define PREVALIDATE_ERROR_MASK_BANK_APP_INT_ADDRESS   (1UL << 8)
#define PREVALIDATE_ERROR_MASK_BANK_APP_SIZE          (1UL << 9)
#define PREVALIDATE_ERROR_MASK_BANK_APP_CRC           (1UL << 10)
#define PREVALIDATE_ERROR_MASK_BANK_APP_VERSION       (1UL << 11)
#define PREVALIDATE_ERROR_MASK_BANK_UPDATE_EXT_ADDRESS  (1UL << 12)
#define PREVALIDATE_ERROR_MASK_BANK_UPDATE_INT_ADDRESS  (1UL << 13)
#define PREVALIDATE_ERROR_MASK_BANK_UPDATE_SIZE         (1UL << 14)
#define PREVALIDATE_ERROR_MASK_BANK_UPDATE_CRC          (1UL << 15)
#define PREVALIDATE_ERROR_MASK_BANK_UPDATE_VERSION      (1UL << 16)
#define PREVALIDATE_ERROR_MASK_ENTER_BOOTLOADER_REASON  (1UL << 17)

#define PREVALIDATE_ERROR_MASK_BANK_SOFTDEVICE_EXT_ADDRESS  (1UL << 12)
#define PREVALIDATE_ERROR_MASK_BANK_SOFTDEVICE_SIZE         (1UL << 14)
#define PREVALIDATE_ERROR_MASK_BANK_SOFTDEVICE_CRC          (1UL << 15)
#define PREVALIDATE_ERROR_MASK_BANK_SOFTDEVICE_VERSION      (1UL << 16)


typedef uint32_t _prevalidate_error_mask;

typedef struct {
	uint32_t command;
	_prevalidate_error_mask error_mask;
}_dev_dfu_prevalidate;


#define IMAGE_WRITE_ERROR_MASK_IMAGE_CRC            (1UL << 0)
typedef uint32_t _image_write_error_mask;
typedef struct {
	uint32_t command;
	_image_write_error_mask error_mask;
}_dev_dfu_image_write;

static _boot_setting m_boot_setting;
static _ble_boot_setting m_ble_boot_setting;



#define POSTVALIDATE_BANK_UPDATE_ALGORITHM_CRC_ERROR                (1UL << 0)
#define POSTVALIDATE_BOOTSETTING_CRC_ERROR                          (1UL << 1)
#define POSTVALIDATE_BANK_UPDATE_ALGORITHM_CRC_ERROR_EXTFLASH       (1UL << 2)
#define POSTVALIDATE_BANK_BOOTLOADER_CRC_ERROR_EXTFLASH             (1UL << 3)
#define POSTVALIDATE_BANK_APPLICATION_CRC_ERROR_EXTFLASH            (1UL << 4)
#define POSTVALIDATE_BANK_SOFTDEVICE_CRC_ERROR_EXTFLASH             (1UL << 5)


typedef uint32_t _postvalidate_error_mask;


typedef struct {
	uint32_t command;
	_postvalidate_error_mask error_mask;
}_dev_dfu_postvalidate;

typedef struct {
	
	uint32_t extFlash_address;
	uint32_t image_total_size;
	uint32_t image_written_size;
	uint32_t data_length;
	uint8_t buffer[4096];
	
}_dev_dfu_static_buffer;

static _dev_dfu_static_buffer m_buffer;


static void bootsetting(uint32_t *p_data, uint32_t length)
{
	LOG_PRINTF(MOUDLE_NAME"bootsetting\n");
	
	memset(&m_ble_boot_setting,0,sizeof(m_ble_boot_setting));
	memset(&m_buffer,0,sizeof(uint32_t)*4);
//	memset(&g_dev_dfu_progress,0,sizeof(g_dev_dfu_progress));	
	
	uint8_t *pp_data = (uint8_t *)p_data + sizeof(_dev_dfu_bootsetting);
	_dev_dfu_bootsetting *bootsetting = (_dev_dfu_bootsetting *)p_data;
	LOG_PRINTF(MOUDLE_NAME"offset : 0x%d , size : 0x%08x , crc : 0x%08x\n",bootsetting->offset,bootsetting->size,bootsetting->crc);
	LOG_HEXDUMP((uint8_t *)p_data,bootsetting->size+sizeof(_dev_dfu_bootsetting));
	DEBUG_HEXDUMP(pp_data,bootsetting->size);
	m_boot_setting = *(_boot_setting *)pp_data;
	
	LOG_PRINTF(MOUDLE_NAME"boot_setting_crc --> 0x%08x\n",m_boot_setting.boot_setting_crc);
	LOG_PRINTF(MOUDLE_NAME"boot_setting_size --> %d\n",m_boot_setting.boot_setting_size);
	LOG_PRINTF(MOUDLE_NAME"bootloader_version --> 0x%08x\n",m_boot_setting.bootloader_version);
	LOG_PRINTF(MOUDLE_NAME"application_version --> 0x%08x\n",m_boot_setting.application_version);
	LOG_PRINTF(MOUDLE_NAME"pcb_version --> 0x%08x\n",m_boot_setting.pcb_version);
	LOG_PRINTF(MOUDLE_NAME"bank_bootloader extFlash_address --> 0x%08x\n",m_boot_setting.bank_bootloader.extFlash_address);
	LOG_PRINTF(MOUDLE_NAME"bank_bootloader intFlash_address --> 0x%08x\n",m_boot_setting.bank_bootloader.intFlash_address);
	LOG_PRINTF(MOUDLE_NAME"bank_bootloader image_size --> %d\n",m_boot_setting.bank_bootloader.image_size);
	LOG_PRINTF(MOUDLE_NAME"bank_bootloader image_crc --> 0x%08x\n",m_boot_setting.bank_bootloader.image_crc);
	LOG_PRINTF(MOUDLE_NAME"bank_bootloader version --> 0x%08x\n",m_boot_setting.bank_bootloader.version);
	LOG_PRINTF(MOUDLE_NAME"bank_bootloader command --> 0x%08x\n",m_boot_setting.bank_bootloader.command);
	LOG_PRINTF(MOUDLE_NAME"bank_application extFlash_address --> 0x%08x\n",m_boot_setting.bank_application.extFlash_address);
	LOG_PRINTF(MOUDLE_NAME"bank_application intFlash_address --> 0x%08x\n",m_boot_setting.bank_application.intFlash_address);
	LOG_PRINTF(MOUDLE_NAME"bank_application image_size --> %d\n",m_boot_setting.bank_application.image_size);
	LOG_PRINTF(MOUDLE_NAME"bank_application image_crc --> 0x%08x\n",m_boot_setting.bank_application.image_crc);
	LOG_PRINTF(MOUDLE_NAME"bank_application version --> 0x%08x\n",m_boot_setting.bank_application.version);
	LOG_PRINTF(MOUDLE_NAME"bank_application command --> 0x%08x\n",m_boot_setting.bank_application.command);
	LOG_PRINTF(MOUDLE_NAME"bank_update_algorithm extFlash_address --> 0x%08x\n",m_boot_setting.bank_update_algorithm.extFlash_address);
	LOG_PRINTF(MOUDLE_NAME"bank_update_algorithm intFlash_address --> 0x%08x\n",m_boot_setting.bank_update_algorithm.intFlash_address);
	LOG_PRINTF(MOUDLE_NAME"bank_update_algorithm image_size --> %d\n",m_boot_setting.bank_update_algorithm.image_size);
	LOG_PRINTF(MOUDLE_NAME"bank_update_algorithm image_crc --> 0x%08x\n",m_boot_setting.bank_update_algorithm.image_crc);
	LOG_PRINTF(MOUDLE_NAME"bank_update_algorithm version --> 0x%08x\n",m_boot_setting.bank_update_algorithm.version);
	LOG_PRINTF(MOUDLE_NAME"bank_update_algorithm command --> 0x%08x\n",m_boot_setting.bank_update_algorithm.command);
	LOG_PRINTF(MOUDLE_NAME"enter_bootloader_reason --> %d\n",m_boot_setting.enter_bootloader_reason);
	
	
	uint32_t crc = am_bootloader_fast_crc32(&m_boot_setting,bootsetting->size);
	LOG_PRINTF(MOUDLE_NAME"calculate crc --> 0x%08x\n",crc);
	
	
	_dev_dfu_bootsetting bootsetting_response;
	bootsetting_response.command = PC_UART_RESPONSE_BOOTLOADER_BOOT_SETTING | PC_UART_CCOMMAND_MASK_BOOTLOADER;
	bootsetting_response.offset = bootsetting->offset;
	bootsetting_response.size = bootsetting->size;
	bootsetting_response.crc = crc;
	drv_pc_send_data((uint8_t *)&bootsetting_response, sizeof(_dev_dfu_bootsetting));
	
}

static void prevalidate(void)
{
	LOG_PRINTF(MOUDLE_NAME"prevalidate\n");
	_dev_dfu_prevalidate prevalidate;
	_prevalidate_error_mask error_mask = 0;;
	_boot_setting boot_setting;
	lib_boot_setting_read(&boot_setting);
	
	prevalidate.command = PC_UART_RESPONSE_BOOTLOADER_PREVALIDATE | PC_UART_CCOMMAND_MASK_BOOTLOADER;

	
	
	if(m_boot_setting.pcb_version < boot_setting.pcb_version)
	{
		error_mask |= PREVALIDATE_ERROR_MASK_PCB_VERSION;
	}
	if(m_boot_setting.bank_bootloader.command == BOOT_SETTING_BANK_COMMAND_NEW)
	{
		if(m_boot_setting.bank_bootloader.extFlash_address > W25Q128_MAX_SIZE)
		{
			error_mask |= PREVALIDATE_ERROR_MASK_BANK_BOOT_EXT_ADDRESS;
		}		
		if(m_boot_setting.bank_bootloader.intFlash_address != 0)
		{
			error_mask |= PREVALIDATE_ERROR_MASK_BANK_BOOT_INT_ADDRESS;
		}		
		if(m_boot_setting.bank_bootloader.image_size > BOOTLOADER_DEFAULT_SIZE)
		{
			error_mask |= PREVALIDATE_ERROR_MASK_BANK_BOOT_SIZE;
		}	
		if(m_boot_setting.bank_bootloader.version < boot_setting.bootloader_version)
		{
			error_mask |= PREVALIDATE_ERROR_MASK_BANK_BOOT_VERSION;
		}
	}
	if(m_boot_setting.bank_application.command == BOOT_SETTING_BANK_COMMAND_NEW)
	{
		if(m_boot_setting.bank_application.extFlash_address > W25Q128_MAX_SIZE)
		{
			error_mask |= PREVALIDATE_ERROR_MASK_BANK_APP_EXT_ADDRESS;
		}		
		if(m_boot_setting.bank_application.intFlash_address == 0)
		{
			error_mask |= PREVALIDATE_ERROR_MASK_BANK_APP_INT_ADDRESS;
		}		
		if(m_boot_setting.bank_application.image_size > APPLICATION_DEFAULT_SIZE)
		{
			error_mask |= PREVALIDATE_ERROR_MASK_BANK_APP_SIZE;
		}	
		if(m_boot_setting.bank_application.version < boot_setting.application_version)
		{
			error_mask |= PREVALIDATE_ERROR_MASK_BANK_APP_VERSION;
		}
	}	
	if(m_boot_setting.bank_update_algorithm.command == BOOT_SETTING_BANK_COMMAND_NEW)
	{
		if(m_boot_setting.bank_update_algorithm.extFlash_address > W25Q128_MAX_SIZE)
		{
			error_mask |= PREVALIDATE_ERROR_MASK_BANK_UPDATE_EXT_ADDRESS;
		}		
		if(m_boot_setting.bank_update_algorithm.intFlash_address != UPDATE_ALGORITHM_START_DEFAULT_ADDRESS)
		{
			error_mask |= PREVALIDATE_ERROR_MASK_BANK_UPDATE_INT_ADDRESS;
		}		
		if(m_boot_setting.bank_update_algorithm.image_size > UPDATE_ALGORITHM_DEFAULT_SIZE)
		{
			error_mask |= PREVALIDATE_ERROR_MASK_BANK_UPDATE_SIZE;
		}	
		if(m_boot_setting.bank_update_algorithm.version < boot_setting.update_version)
		{
			error_mask |= PREVALIDATE_ERROR_MASK_BANK_UPDATE_VERSION;
		}
	}	
	if(m_boot_setting.enter_bootloader_reason != BOOT_SETTING_BANK_COMMAND_DEFAULT)
	{
		error_mask |= PREVALIDATE_ERROR_MASK_ENTER_BOOTLOADER_REASON;
	}
	prevalidate.error_mask = error_mask;
	LOG_HEXDUMP((uint8_t *)&prevalidate,sizeof(_dev_dfu_prevalidate));	
	
	uint32_t size = 0;
	uint32_t address = 0;
	if(m_boot_setting.bank_bootloader.command == BOOT_SETTING_BANK_COMMAND_NEW)
	{
		size += m_boot_setting.bank_bootloader.image_size;
		address = m_boot_setting.bank_bootloader.extFlash_address;
	}
	if(m_boot_setting.bank_application.command == BOOT_SETTING_BANK_COMMAND_NEW)
	{
		size += m_boot_setting.bank_application.image_size;
		if(m_boot_setting.bank_bootloader.command != BOOT_SETTING_BANK_COMMAND_NEW)
		{
			address = m_boot_setting.bank_application.extFlash_address;
		}
	}	
	if(m_boot_setting.bank_update_algorithm.command == BOOT_SETTING_BANK_COMMAND_NEW)
	{
		size += m_boot_setting.bank_update_algorithm.image_size;
		if(m_boot_setting.bank_bootloader.command != BOOT_SETTING_BANK_COMMAND_NEW &&
			 m_boot_setting.bank_application.command != BOOT_SETTING_BANK_COMMAND_NEW)
		{
			address = m_boot_setting.bank_update_algorithm.extFlash_address;
		}		
	}		

	
	if(error_mask == PREVALIDATE_ERROR_MASK_DEFAULT)
	{
		m_buffer.image_total_size = size;
		m_buffer.extFlash_address = address;
		dev_extFlash_enable();
		dev_extFlash_erase(address,size);	
		dev_extFlash_disable();
	}

	
	
#if MOUDLE_DEBUG_ENABLED == 1 && RTT_LOG_ENABLED == 1
	uint8_t p_data[1024];
	dev_extFlash_enable();
	dev_extFlash_read(address, p_data, sizeof(p_data));
	dev_extFlash_disable();			
	
	DEBUG_HEXDUMP(p_data, sizeof(p_data));
#endif	
	
	drv_pc_send_data((uint8_t *)&prevalidate, sizeof(_dev_dfu_prevalidate));
	
}
static void image_data(uint32_t *p_data, uint32_t length)
{
	LOG_PRINTF(MOUDLE_NAME"image_data\n");
	DEBUG_HEXDUMP((uint8_t *)p_data,length);	
	uint8_t *pp_data = (uint8_t *)p_data + sizeof(_dev_dfu_image_data);
	_dev_dfu_image_data *image_data = (_dev_dfu_image_data *)p_data;
	
	LOG_PRINTF(MOUDLE_NAME"offset : 0x%08x , size : 0x%08x , crc : 0x%08x\n",image_data->offset,image_data->size,image_data->crc);
	
	DEBUG_HEXDUMP(pp_data,image_data->size);	
	uint32_t crc = am_bootloader_fast_crc32(pp_data,image_data->size);
	LOG_PRINTF(MOUDLE_NAME"calculate crc --> 0x%08x\n",crc);
	
	_dev_dfu_image_data response_image_data;
	response_image_data.command = PC_UART_RESPONSE_BOOTLOADER_IMAGE_DATA | PC_UART_CCOMMAND_MASK_BOOTLOADER;
	response_image_data.offset = image_data->offset;
	response_image_data.size = image_data->size;
	response_image_data.crc = crc;
	
	LOG_HEXDUMP((uint8_t *)&response_image_data,sizeof(_dev_dfu_image_data));	
	
	if(m_buffer.data_length <= sizeof(m_buffer.buffer))
	{
		memcpy(m_buffer.buffer+m_buffer.data_length,pp_data,image_data->size);
		m_buffer.data_length += image_data->size;	
		m_buffer.image_written_size += image_data->size;
	}
	else
	{
		ERR_HANDLER(ERR_INVALID_LENGTH);
	}
	
	drv_pc_send_data((uint8_t *)&response_image_data, sizeof(_dev_dfu_image_data));	
	
}

static void iamge_write(void)
{
	LOG_PRINTF(MOUDLE_NAME"iamge_write\n");
	DEBUG_HEXDUMP(m_buffer.buffer,m_buffer.data_length);
	uint32_t buf_crc = am_bootloader_fast_crc32(m_buffer.buffer,m_buffer.data_length);
	LOG_PRINTF(MOUDLE_NAME"buf_crc --> 0x%08x\n",buf_crc);
	LOG_PRINTF(MOUDLE_NAME"address : 0x%08x , length : 0x%08x\n",m_buffer.extFlash_address,m_buffer.data_length);
	dev_extFlash_enable();
	dev_extFlash_write(m_buffer.extFlash_address, m_buffer.buffer, m_buffer.data_length);
	dev_extFlash_read(m_buffer.extFlash_address, m_buffer.buffer, m_buffer.data_length);
	dev_extFlash_disable();			
	uint32_t flash_crc = am_bootloader_fast_crc32(m_buffer.buffer,m_buffer.data_length);
	LOG_PRINTF(MOUDLE_NAME"flash_crc --> 0x%08x\n",flash_crc);
	m_buffer.extFlash_address += m_buffer.data_length;
	m_buffer.data_length = 0;
	
	_dev_dfu_image_write image_write;
	image_write.command = PC_UART_RESPONSE_BOOTLOADER_IMAGE_WRITE | PC_UART_CCOMMAND_MASK_BOOTLOADER;
	image_write.error_mask = 0;
	if(buf_crc != flash_crc)
	{
		image_write.error_mask |= IMAGE_WRITE_ERROR_MASK_IMAGE_CRC;
	}

	drv_pc_send_data((uint8_t *)&image_write, sizeof(_dev_dfu_image_write));
}
static uint8_t m_buffer_2048[2048];
static bool check_crc_from_external_flash(uint32_t input_crc,uint32_t external_flash_address ,uint32_t size)
{
	uint32_t imageSizeWritten = size;
	uint32_t bytesInBuffer;
	uint32_t currentAddr = external_flash_address;
	uint32_t output_crc = 0;
	
	while(imageSizeWritten)
	{
		bytesInBuffer = imageSizeWritten < 2048 ? imageSizeWritten : 2048;
		dev_extFlash_enable();
		dev_extFlash_read(currentAddr, m_buffer_2048, bytesInBuffer);
		dev_extFlash_disable();	
		am_bootloader_partial_crc32(m_buffer_2048, bytesInBuffer, &output_crc);
		currentAddr += bytesInBuffer;
		imageSizeWritten -= bytesInBuffer;
	}	
	if(input_crc == output_crc)
	{
		return true;
	}

	return false;
}


static void postvalidate(void)
{
	LOG_PRINTF(MOUDLE_NAME"postvalidate\n");
	_dev_dfu_postvalidate postvalidate;
	postvalidate.command = PC_UART_RESPONSE_BOOTLOADER_POSTVALIDATE | PC_UART_CCOMMAND_MASK_BOOTLOADER;
	postvalidate.error_mask = 0;
	
	_boot_setting boot_setting;
	lib_boot_setting_read(&boot_setting);	
	m_boot_setting.bootloader_version = boot_setting.bootloader_version;
	m_boot_setting.application_version = boot_setting.application_version;
	m_boot_setting.update_version = boot_setting.update_version;
//	m_boot_setting.pcb_version = boot_setting.pcb_version;
	m_boot_setting.bank_update_algorithm.intFlash_address = boot_setting.bank_update_algorithm.intFlash_address;
	m_boot_setting.enter_bootloader_reason = BOOT_SETTING_ENTER_BOOT_REASON_JUMP_UPDATE_ALGORITHM;

	
		uint32_t src_addr = 0;
		uint32_t dest_addr = 0;
		uint32_t size = 0;
		uint32_t image_crc = 0;	
	
	
	if(m_boot_setting.bank_update_algorithm.command == BOOT_SETTING_BANK_COMMAND_NEW)
	{
		LOG_PRINTF(MOUDLE_NAME"BOOT_SETTING_BANK_COMMAND_NEW\n");
		src_addr = m_boot_setting.bank_update_algorithm.extFlash_address;
		dest_addr = m_boot_setting.bank_update_algorithm.intFlash_address;
		size = m_boot_setting.bank_update_algorithm.image_size;
		image_crc = m_boot_setting.bank_update_algorithm.image_crc;
		//读出外部FLASH中IMAGE算CRC与CONFIG文件中的CRC对比，错误返回错误值，不进行拷贝
		LOG_PRINTF(MOUDLE_NAME"src_addr = 0x%08x\n",src_addr);
		LOG_PRINTF(MOUDLE_NAME"dest_addr = 0x%08x\n",dest_addr);
		LOG_PRINTF(MOUDLE_NAME"size = 0x%08x\n",size);
		
		if(check_crc_from_external_flash(image_crc,src_addr,size))
		{
			dev_dfu_flash_memcpy(dest_addr,src_addr,size);
			uint32_t crc = am_bootloader_fast_crc32((uint32_t *)dest_addr,size);
			LOG_PRINTF(MOUDLE_NAME"calculate crc = 0x%08x\n",crc);
			LOG_PRINTF(MOUDLE_NAME"dfu crc = 0x%08x\n",m_boot_setting.bank_update_algorithm.image_crc);
			if(crc != m_boot_setting.bank_update_algorithm.image_crc)
			{
				postvalidate.error_mask |= POSTVALIDATE_BANK_UPDATE_ALGORITHM_CRC_ERROR;
				goto POST_VALIDATE_EXIT;
			}
			m_boot_setting.update_version = m_boot_setting.bank_update_algorithm.version;
			m_boot_setting.bank_update_algorithm.command = BOOT_SETTING_BANK_COMMAND_DEFAULT;		
		}
		else{
			//外部FLASH存的IMAGE有问题
			postvalidate.error_mask |= POSTVALIDATE_BANK_UPDATE_ALGORITHM_CRC_ERROR_EXTFLASH;
			goto POST_VALIDATE_EXIT;		
		}
	}
	else if(m_boot_setting.bank_update_algorithm.command == BOOT_SETTING_BANK_COMMAND_DEFAULT)
	{
		memcpy(&m_boot_setting.bank_update_algorithm,&boot_setting.bank_update_algorithm,sizeof(_image_bank));
	}
	

	if(m_boot_setting.bank_bootloader.command == BOOT_SETTING_BANK_COMMAND_DEFAULT &&
		 m_boot_setting.bank_application.command == BOOT_SETTING_BANK_COMMAND_DEFAULT 
	  )//BOOTLOADER和APPLICATION两个软件都不升级
	{
		memcpy(&m_boot_setting.bank_application,&boot_setting.bank_application,sizeof(_image_bank));
		memcpy(&m_boot_setting.bank_bootloader,&boot_setting.bank_bootloader,sizeof(_image_bank));
		m_boot_setting.enter_bootloader_reason = BOOT_SETTING_ENTER_BOOT_REASON_JUMP_APPLICATION;
	}
	else if(m_boot_setting.bank_application.command == BOOT_SETTING_BANK_COMMAND_DEFAULT)
	{//APPLICATION软件不升级
		memcpy(&m_boot_setting.bank_application,&boot_setting.bank_application,sizeof(_image_bank));
	}
	else if(m_boot_setting.bank_bootloader.command == BOOT_SETTING_BANK_COMMAND_DEFAULT)
	{//BOOTLOADER软件不升级
		memcpy(&m_boot_setting.bank_bootloader,&boot_setting.bank_bootloader,sizeof(_image_bank));
	}
	//其他情况将PC发来的CONFIG文件拷入BOOT SETTING区域，在UPDATE ALGORITHM里面判断哪种升级
	
	if(m_boot_setting.bank_bootloader.command == BOOT_SETTING_BANK_COMMAND_NEW)
	{
		//读出外部FLASH中IMAGE算CRC与CONFIG文件中的CRC对比，错误返回错误值，不进行拷贝
		LOG_PRINTF(MOUDLE_NAME"update bootloader\n");
		src_addr = m_boot_setting.bank_bootloader.extFlash_address;
		size = m_boot_setting.bank_bootloader.image_size;
		image_crc = m_boot_setting.bank_bootloader.image_crc;		
		
		if(!check_crc_from_external_flash(image_crc,src_addr,size))
		{
			//外部FLASH存的IMAGE有问题
			postvalidate.error_mask |= POSTVALIDATE_BANK_BOOTLOADER_CRC_ERROR_EXTFLASH;
			goto POST_VALIDATE_EXIT;				
		}
	}	
	if(m_boot_setting.bank_application.command == BOOT_SETTING_BANK_COMMAND_NEW)
	{
		//读出外部FLASH中IMAGE算CRC与CONFIG文件中的CRC对比，错误返回错误值，不进行拷贝
		LOG_PRINTF(MOUDLE_NAME"update application\n");
		src_addr = m_boot_setting.bank_application.extFlash_address;
		size = m_boot_setting.bank_application.image_size;
		image_crc = m_boot_setting.bank_application.image_crc;		
		
		if(!check_crc_from_external_flash(image_crc,src_addr,size))
		{
			//外部FLASH存的IMAGE有问题
			postvalidate.error_mask |= POSTVALIDATE_BANK_APPLICATION_CRC_ERROR_EXTFLASH;
			goto POST_VALIDATE_EXIT;				
		}
		
	}	
	
	m_boot_setting.boot_setting_crc = am_bootloader_fast_crc32(&m_boot_setting.boot_setting_size,m_boot_setting.boot_setting_size-4);
	lib_boot_setting_write(&m_boot_setting);
	lib_boot_setting_read(&boot_setting);
	
	
	LOG_HEXDUMP((uint8_t *)&boot_setting, sizeof(_boot_setting));	
	if(!lib_boot_setting_crc_check_bootsetting(&boot_setting))	
	{
		postvalidate.error_mask |= POSTVALIDATE_BOOTSETTING_CRC_ERROR;
	}		
	
	POST_VALIDATE_EXIT:
	
	LOG_HEXDUMP((uint8_t *)&postvalidate, sizeof(_dev_dfu_postvalidate));
	drv_pc_send_data((uint8_t *)&postvalidate, sizeof(_dev_dfu_postvalidate));	
	
}


#define BLE_BANK_BOOTLOADER_DEFAULT_SIZE        0x10000
#define BLE_BANK_APPLICATION_DEFAULT_SIZE       0x49000
#define BLE_BANK_SOFTDEVICE_DEFAULT_SIZE        0x22000

extern _dev_ble_info g_ble_info;




static void ble_bootsetting(uint32_t *p_data, uint32_t length)
{
	LOG_PRINTF(MOUDLE_NAME"ble_bootsetting\n");
	uint8_t *pp_data = (uint8_t *)p_data + sizeof(_dev_dfu_bootsetting);
	m_ble_boot_setting = *(_ble_boot_setting *)pp_data;
	
	LOG_PRINTF(MOUDLE_NAME"ble boot_setting_crc --> 0x%08x\n",m_ble_boot_setting.boot_setting_crc);
	LOG_PRINTF(MOUDLE_NAME"ble boot_setting_size --> %d\n",m_ble_boot_setting.boot_setting_size);
	LOG_PRINTF(MOUDLE_NAME"ble pcb_version --> 0x%08x\n",m_ble_boot_setting.pcb_version);
	LOG_PRINTF(MOUDLE_NAME"ble bank_bootloader extFlash_address --> 0x%08x\n",m_ble_boot_setting.bank_bootloader.extFlash_address);
	LOG_PRINTF(MOUDLE_NAME"ble bank_bootloader image_size --> %d\n",m_ble_boot_setting.bank_bootloader.image_size);
	LOG_PRINTF(MOUDLE_NAME"ble bank_bootloader image_crc --> 0x%08x\n",m_ble_boot_setting.bank_bootloader.image_crc);
	LOG_PRINTF(MOUDLE_NAME"ble bank_bootloader version --> 0x%08x\n",m_ble_boot_setting.bank_bootloader.version);
	LOG_PRINTF(MOUDLE_NAME"ble bank_bootloader command --> 0x%08x\n",m_ble_boot_setting.bank_bootloader.command);
	LOG_PRINTF(MOUDLE_NAME"ble bank_application extFlash_address --> 0x%08x\n",m_ble_boot_setting.bank_application.extFlash_address);
	LOG_PRINTF(MOUDLE_NAME"ble bank_application image_size --> %d\n",m_ble_boot_setting.bank_application.image_size);
	LOG_PRINTF(MOUDLE_NAME"ble bank_application image_crc --> 0x%08x\n",m_ble_boot_setting.bank_application.image_crc);
	LOG_PRINTF(MOUDLE_NAME"ble bank_application version --> 0x%08x\n",m_ble_boot_setting.bank_application.version);
	LOG_PRINTF(MOUDLE_NAME"ble bank_application command --> 0x%08x\n",m_ble_boot_setting.bank_application.command);
	LOG_PRINTF(MOUDLE_NAME"ble bank_softdevice extFlash_address --> 0x%08x\n",m_ble_boot_setting.bank_softdevice.extFlash_address);
	LOG_PRINTF(MOUDLE_NAME"ble bank_softdevice image_size --> %d\n",m_ble_boot_setting.bank_softdevice.image_size);
	LOG_PRINTF(MOUDLE_NAME"ble bank_softdevice image_crc --> 0x%08x\n",m_ble_boot_setting.bank_softdevice.image_crc);
	LOG_PRINTF(MOUDLE_NAME"ble bank_softdevice version --> 0x%08x\n",m_ble_boot_setting.bank_softdevice.version);
	LOG_PRINTF(MOUDLE_NAME"ble bank_softdevice command --> 0x%08x\n",m_ble_boot_setting.bank_softdevice.command);



//	g_dev_dfu_progress.status = DEV_DFU_PROGRESS_STATUS_UPDATE_NRF52832;		
	memset(&m_buffer,0,sizeof(uint32_t)*4);	
	_uart_data_param param;
	param.user_callback = NULL;
	param.p_data = (uint8_t *)p_data;
	param.length = length;
	uint8_t count = TASK_UART_DELAY_COUNT_OUT;
	while(drv_ble_send_data(&param) != ERR_SUCCESS && count--)
	{
		DELAY_MS(TASK_UART_DELAY);
	}		


}


static void ble_prevalidate(uint32_t *p_data, uint32_t length)
{
	LOG_PRINTF(MOUDLE_NAME"ble_prevalidate\n");

	
	uint32_t size = 0;
	uint32_t address = 0;
	if(m_ble_boot_setting.bank_bootloader.command == BOOT_SETTING_BANK_COMMAND_NEW)
	{
		size += m_ble_boot_setting.bank_bootloader.image_size;
		address = m_ble_boot_setting.bank_bootloader.extFlash_address;
	}
	if(m_ble_boot_setting.bank_application.command == BOOT_SETTING_BANK_COMMAND_NEW)
	{
		size += m_ble_boot_setting.bank_application.image_size;
		if(m_ble_boot_setting.bank_bootloader.command != BOOT_SETTING_BANK_COMMAND_NEW)
		{
			address = m_ble_boot_setting.bank_application.extFlash_address;
		}
	}	
	if(m_ble_boot_setting.bank_softdevice.command == BOOT_SETTING_BANK_COMMAND_NEW)
	{
		size += m_ble_boot_setting.bank_softdevice.image_size;
	}		

	LOG_PRINTF(MOUDLE_NAME"address : 0x%08x , size : 0x%08x\n",address,size);


	m_buffer.image_total_size = size;
	m_buffer.extFlash_address = address;
	dev_extFlash_enable();
	dev_extFlash_erase(address,size);	
	dev_extFlash_disable();	
	
	_uart_data_param param;
	param.user_callback = NULL;
	param.p_data = (uint8_t *)p_data;
	param.length = length;
	uint8_t count = TASK_UART_DELAY_COUNT_OUT;
	while(drv_ble_send_data(&param) != ERR_SUCCESS && count--)
	{
		DELAY_MS(TASK_UART_DELAY);
	}	
}
static void ble_image_data(uint32_t *p_data, uint32_t length)
{
	LOG_PRINTF(MOUDLE_NAME"ble_image_data\n");
	DEBUG_HEXDUMP((uint8_t *)p_data,length);	
	uint8_t *pp_data = (uint8_t *)p_data + sizeof(_dev_dfu_image_data);
	_dev_dfu_image_data *image_data = (_dev_dfu_image_data *)p_data;
	
	LOG_PRINTF(MOUDLE_NAME"offset : 0x%08x , size : 0x%08x , crc : 0x%08x\n",image_data->offset,image_data->size,image_data->crc);
	
	DEBUG_HEXDUMP(pp_data,image_data->size);	
	uint32_t crc = am_bootloader_fast_crc32(pp_data,image_data->size);
	LOG_PRINTF(MOUDLE_NAME"calculate crc --> 0x%08x\n",crc);
	
	_dev_dfu_image_data response_image_data;
	response_image_data.command = PC_UART_RESPONSE_BOOTLOADER_BLE_IMAGE_DATA | PC_UART_CCOMMAND_MASK_BOOTLOADER;
	response_image_data.offset = image_data->offset;
	response_image_data.size = image_data->size;
	response_image_data.crc = crc;
	
	LOG_HEXDUMP((uint8_t *)&response_image_data,sizeof(_dev_dfu_image_data));	
	
	if(m_buffer.data_length <= sizeof(m_buffer.buffer))
	{
		memcpy(m_buffer.buffer+m_buffer.data_length,pp_data,image_data->size);
		m_buffer.data_length += image_data->size;	
		m_buffer.image_written_size += image_data->size;
	}
	else
	{
		ERR_HANDLER(ERR_INVALID_LENGTH);
	}
	drv_pc_send_data((uint8_t *)&response_image_data, sizeof(_dev_dfu_image_data));	
	
}

static void ble_iamge_write(void)
{
	LOG_PRINTF(MOUDLE_NAME"ble_iamge_write\n");
	DEBUG_HEXDUMP(m_buffer.buffer,m_buffer.data_length);
	uint32_t buf_crc = am_bootloader_fast_crc32(m_buffer.buffer,m_buffer.data_length);
	LOG_PRINTF(MOUDLE_NAME"buf_crc --> 0x%08x\n",buf_crc);
	LOG_PRINTF(MOUDLE_NAME"address : 0x%08x , length : 0x%08x\n",m_buffer.extFlash_address,m_buffer.data_length);
	dev_extFlash_enable();
	dev_extFlash_write(m_buffer.extFlash_address, m_buffer.buffer, m_buffer.data_length);
	dev_extFlash_read(m_buffer.extFlash_address, m_buffer.buffer, m_buffer.data_length);
	dev_extFlash_disable();		
	uint32_t flash_crc = am_bootloader_fast_crc32(m_buffer.buffer,m_buffer.data_length);
	LOG_PRINTF(MOUDLE_NAME"flash_crc --> 0x%08x\n",flash_crc);
	m_buffer.extFlash_address += m_buffer.data_length;
	m_buffer.data_length = 0;
	
	_dev_dfu_image_write image_write;
	image_write.command = PC_UART_RESPONSE_BOOTLOADER_BLE_IMAGE_WRITE | PC_UART_CCOMMAND_MASK_BOOTLOADER;
	image_write.error_mask = 0;
	if(buf_crc != flash_crc)
	{
		image_write.error_mask |= IMAGE_WRITE_ERROR_MASK_IMAGE_CRC;
	}
	LOG_HEXDUMP((uint8_t *)&image_write,sizeof(_dev_dfu_image_write));
	drv_pc_send_data((uint8_t *)&image_write, sizeof(_dev_dfu_image_write));
}
static uint32_t current_command = 0;
static void exception_response(void)
{
	uint32_t command = PC_UART_CCOMMAND_BOOTLOADER_UNSPPORT | PC_UART_CCOMMAND_MASK_BOOTLOADER;
	drv_pc_send_data((uint8_t *)&command, sizeof(command));	
}	

static void ble_postvalidate(uint32_t *p_data, uint32_t length)
{
	_dev_dfu_postvalidate postvalidate_resp;
	postvalidate_resp.command = PC_UART_RESPONSE_BOOTLOADER_BLE_POSTVALIDATE | PC_UART_CCOMMAND_MASK_BOOTLOADER;
															
	uint32_t src_addr = 0;
//	uint32_t dest_addr = 0;
	uint32_t size = 0;
	uint32_t image_crc = 0;	
	
	
	if(m_ble_boot_setting.bank_bootloader.command == BOOT_SETTING_BANK_COMMAND_NEW)
	{
		LOG_PRINTF(MOUDLE_NAME"BOOT_SETTING_BANK_COMMAND_NEW\n");
		src_addr = m_ble_boot_setting.bank_bootloader.extFlash_address;
		size = m_ble_boot_setting.bank_bootloader.image_size;
		image_crc = m_ble_boot_setting.bank_bootloader.image_crc;
		//读出外部FLASH中IMAGE算CRC与CONFIG文件中的CRC对比，错误返回错误值，不进行拷贝
		LOG_PRINTF(MOUDLE_NAME"src_addr = 0x%08x\n",src_addr);
		LOG_PRINTF(MOUDLE_NAME"dest_addr = 0x%08x\n",dest_addr);
		LOG_PRINTF(MOUDLE_NAME"size = 0x%08x\n",size);
		
		if(check_crc_from_external_flash(image_crc,src_addr,size))
		{
		}
		else{
			//外部FLASH存的IMAGE有问题
			postvalidate_resp.error_mask = POSTVALIDATE_BANK_BOOTLOADER_CRC_ERROR_EXTFLASH;
			drv_pc_send_data((uint8_t *)&postvalidate_resp, sizeof(_dev_dfu_postvalidate));	
			return;		
		}
	}		
	
	
	if(m_ble_boot_setting.bank_application.command == BOOT_SETTING_BANK_COMMAND_NEW)
	{
		LOG_PRINTF(MOUDLE_NAME"BOOT_SETTING_BANK_COMMAND_NEW\n");
		src_addr = m_ble_boot_setting.bank_application.extFlash_address;
		size = m_ble_boot_setting.bank_application.image_size;
		image_crc = m_ble_boot_setting.bank_application.image_crc;
		//读出外部FLASH中IMAGE算CRC与CONFIG文件中的CRC对比，错误返回错误值，不进行拷贝
		LOG_PRINTF(MOUDLE_NAME"src_addr = 0x%08x\n",src_addr);
		LOG_PRINTF(MOUDLE_NAME"dest_addr = 0x%08x\n",dest_addr);
		LOG_PRINTF(MOUDLE_NAME"size = 0x%08x\n",size);
		if(check_crc_from_external_flash(image_crc,src_addr,size))
		{
		}
		else{
			//外部FLASH存的IMAGE有问题
			postvalidate_resp.error_mask = POSTVALIDATE_BANK_APPLICATION_CRC_ERROR_EXTFLASH;
			drv_pc_send_data((uint8_t *)&postvalidate_resp, sizeof(_dev_dfu_postvalidate));	
			return;		
		}		
	}	
	
	if(m_ble_boot_setting.bank_softdevice.command == BOOT_SETTING_BANK_COMMAND_NEW)
	{
		LOG_PRINTF(MOUDLE_NAME"BOOT_SETTING_BANK_COMMAND_NEW\n");
		src_addr = m_ble_boot_setting.bank_softdevice.extFlash_address;
		size = m_ble_boot_setting.bank_softdevice.image_size;
		image_crc = m_ble_boot_setting.bank_softdevice.image_crc;
		//读出外部FLASH中IMAGE算CRC与CONFIG文件中的CRC对比，错误返回错误值，不进行拷贝
		LOG_PRINTF(MOUDLE_NAME"src_addr = 0x%08x\n",src_addr);
		LOG_PRINTF(MOUDLE_NAME"dest_addr = 0x%08x\n",dest_addr);
		LOG_PRINTF(MOUDLE_NAME"size = 0x%08x\n",size);
		if(check_crc_from_external_flash(image_crc,src_addr,size))
		{
		}
		else{
			//外部FLASH存的IMAGE有问题
			postvalidate_resp.error_mask = POSTVALIDATE_BANK_SOFTDEVICE_CRC_ERROR_EXTFLASH;
			drv_pc_send_data((uint8_t *)&postvalidate_resp, sizeof(_dev_dfu_postvalidate));	
			return;		
		}		
	}		
	
	_uart_data_param param;
	param.user_callback = NULL;
	param.p_data = (uint8_t *)p_data;
	param.length = length;
	uint8_t count = TASK_UART_DELAY_COUNT_OUT;
	while(drv_ble_send_data(&param) != ERR_SUCCESS && count--)
	{
		DELAY_MS(TASK_UART_DELAY);
	}	

}
static void ble_reset_N_activate(uint32_t *p_data, uint32_t length)
{
	_uart_data_param param;
	param.user_callback = NULL;
	param.p_data = (uint8_t *)p_data;
	param.length = length;
	uint8_t count = TASK_UART_DELAY_COUNT_OUT;
	while(drv_ble_send_data(&param) != ERR_SUCCESS && count--)
	{
		DELAY_MS(TASK_UART_DELAY);
	}	
	am_hal_gpio_out_bit_set(DEV_BLE_SYS_OFF_PIN);
	am_hal_gpio_pin_config(DEV_BLE_SYS_OFF_PIN,AM_HAL_GPIO_OUTPUT);	
}

void dev_dfu_event_handle(uint32_t *p_data, uint32_t length)
{
	LOG_PRINTF(MOUDLE_NAME"dev_dfu_event_handle\n");
	DEBUG_HEXDUMP((uint8_t *)p_data,length);	

	switch(p_data[0]&0x0000FF00)
	{
		case PC_UART_CCOMMAND_BOOTLOADER_INFO_APOLLO2:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_CCOMMAND_BOOTLOADER_INFO_APOLLO2\n");
			
			timer_exception_stop();
			
			_boot_setting boot_setting;
			lib_boot_setting_read(&boot_setting);
			_info_apollo2 info_apollo2;
			info_apollo2.command = PC_UART_RESPONSE_BOOTLOADER_INFO_APOLLO2 | PC_UART_CCOMMAND_MASK_BOOTLOADER;
			info_apollo2.bootloader_version = boot_setting.bootloader_version;
			info_apollo2.application_version = boot_setting.application_version;
			info_apollo2.update_version = boot_setting.update_version;
			info_apollo2.pcb_version = boot_setting.pcb_version;
			info_apollo2.chip_id_0 = AM_REG(MCUCTRL, CHIPID0);
			info_apollo2.chip_id_1 = AM_REG(MCUCTRL, CHIPID1);	
			memcpy(info_apollo2.model,boot_setting.model,sizeof(info_apollo2.model));
			drv_pc_send_data((uint8_t *)&info_apollo2, sizeof(_info_apollo2));	
			

			timer_exception_start(TIMER_EXCEPTION_TIMEOUT);	
			
		}break;	
		case PC_UART_CCOMMAND_BOOTLOADER_JUMP_BOOTLOADER:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_CCOMMAND_BOOTLOADER_JUMP_BOOTLOADER\n");
			uint32_t command[1] = {PC_UART_RESPONSE_BOOTLOADER_JUMP_BOOTLOADER | PC_UART_CCOMMAND_MASK_BOOTLOADER};
			drv_pc_send_data((uint8_t *)command, sizeof(command));

			
		}break;		
		case PC_UART_CCOMMAND_BOOTLOADER_JUMP_APPLICATION:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_CCOMMAND_BOOTLOADER_JUMP_APPLICATION\n");
			uint32_t command[1] = {PC_UART_RESPONSE_BOOTLOADER_JUMP_APPLICATION | PC_UART_CCOMMAND_MASK_BOOTLOADER};
			drv_pc_send_data((uint8_t *)command, sizeof(command));
			drv_pc_uninit();
			DELAY_MS(500);
			lib_boot_jump_to_bootloader(BOOT_SETTING_ENTER_BOOT_REASON_JUMP_APPLICATION);
			
		}break;		
		case PC_UART_CCOMMAND_BOOTLOADER_JUMP_UPDATE_ALGORITHM:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_CCOMMAND_BOOTLOADER_JUMP_UPDATE_ALGORITHM\n");
			uint32_t command[1] = {PC_UART_RESPONSE_BOOTLOADER_JUMP_UPDATE_ALGORITHM | PC_UART_CCOMMAND_MASK_BOOTLOADER};
			drv_pc_send_data((uint8_t *)command, sizeof(command));
			lib_boot_jump_to_update_algorithm();
			
			
		}break;		
		case PC_UART_CCOMMAND_BOOTLOADER_RESET:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_CCOMMAND_BOOTLOADER_RESET\n");	
			
			uint32_t command[1] = {PC_UART_RESPONSE_BOOTLOADER_RESET | PC_UART_CCOMMAND_MASK_BOOTLOADER};
			drv_pc_send_data((uint8_t *)command, sizeof(command));
			#ifdef AM_FREERTOS 	
				taskENTER_CRITICAL();
			#endif 
			am_hal_reset_por();			
			
		}break;			
		case PC_UART_CCOMMAND_BOOTLOADER_BOOT_SETTING:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_CCOMMAND_BOOTLOADER_BOOT_SETTING\n");
			
			timer_exception_stop();
			
//			g_dev_dfu_progress.status = DEV_DFU_PROGRESS_STATUS_UPDATE_APOLLO2;
			/*
				单独升级APOLLO时，需要把蓝牙关掉，不然APP交互会影响升级
			*/
			am_hal_gpio_out_bit_set(DEV_BLE_SYS_OFF_PIN);
			am_hal_gpio_pin_config(DEV_BLE_SYS_OFF_PIN,AM_HAL_GPIO_OUTPUT);				
			bootsetting(p_data,length);
			timer_exception_start(TIMER_EXCEPTION_TIMEOUT);	
			
		}break;
		case PC_UART_CCOMMAND_BOOTLOADER_PREVALIDATE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_CCOMMAND_BOOTLOADER_PREVALIDATE\n");		
			timer_exception_stop();
			prevalidate();
			timer_exception_start(TIMER_EXCEPTION_TIMEOUT);	
		}break;	
		case PC_UART_CCOMMAND_BOOTLOADER_IMAGE_DATA:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_CCOMMAND_BOOTLOADER_IMAGE_DATA\n");
			timer_exception_stop();
//			g_dev_dfu_progress.percentage = (m_buffer.image_written_size*100)/m_buffer.image_total_size;//不用显示进度条
			image_data(p_data,length);
			timer_exception_start(TIMER_EXCEPTION_TIMEOUT);	
		}break;
		case PC_UART_CCOMMAND_BOOTLOADER_IMAGE_WRITE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_CCOMMAND_BOOTLOADER_IMAGE_WRITE\n");
			if(current_command == PC_UART_CCOMMAND_BOOTLOADER_POSTVALIDATE ||
				 current_command == PC_UART_CCOMMAND_BOOTLOADER_RESET_N_ACTIVATE
				)
			{
				timer_exception_stop();			
				exception_response();
				timer_exception_start(TIMER_EXCEPTION_TIMEOUT);						
			}
			else
			{
				timer_exception_stop();
				iamge_write();
				gui_boot_task_display();
				timer_exception_start(TIMER_EXCEPTION_TIMEOUT);			
			}			
		}break;		
		case PC_UART_CCOMMAND_BOOTLOADER_POSTVALIDATE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_CCOMMAND_BOOTLOADER_POSTVALIDATE\n");	
			current_command = PC_UART_CCOMMAND_BOOTLOADER_POSTVALIDATE;
			timer_exception_stop();
			postvalidate();
			timer_exception_start(TIMER_EXCEPTION_TIMEOUT);
		}break;
		case PC_UART_CCOMMAND_BOOTLOADER_RESET_N_ACTIVATE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_CCOMMAND_BOOTLOADER_RESET_N_ACTIVATE\n");
			current_command = PC_UART_CCOMMAND_BOOTLOADER_RESET_N_ACTIVATE;
			timer_exception_stop();
			uint32_t command[1] = {PC_UART_RESPONSE_BOOTLOADER_RESET_N_ACTIVATE | PC_UART_CCOMMAND_MASK_BOOTLOADER};
			drv_pc_send_data((uint8_t *)command, sizeof(command));
			drv_pc_uninit();
			DELAY_MS(500);
			#ifdef AM_FREERTOS 	
				taskENTER_CRITICAL();
			#endif 
			am_hal_reset_por();
			

			timer_exception_start(TIMER_EXCEPTION_TIMEOUT);
		}break;	
		case PC_UART_CCOMMAND_BOOTLOADER_BLE_INFO:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_CCOMMAND_BOOTLOADER_BLE_INFO\n");
			
			timer_exception_stop();
			uint32_t command[1] = {PC_UART_CCOMMAND_BOOTLOADER_BLE_INFO | PC_UART_CCOMMAND_MASK_BOOTLOADER | PC_UART_COMMAND_MASK_APPLICATION};
			_uart_data_param param;
			param.user_callback = NULL;
			param.p_data = (uint8_t *)command;
			param.length = sizeof(command);
			uint8_t count = TASK_UART_DELAY_COUNT_OUT;
			while(drv_ble_send_data(&param) != ERR_SUCCESS && count--)
			{
				DELAY_MS(TASK_UART_DELAY);
			}				
			timer_exception_start(TIMER_EXCEPTION_TIMEOUT);
		}break;		
		case PC_UART_CCOMMAND_BOOTLOADER_BLE_JUMP_BOOTLOADER:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_CCOMMAND_BOOTLOADER_BLE_JUMP_BOOTLOADER\n");
			timer_exception_stop();
			uint32_t command[1] = {PC_UART_CCOMMAND_BOOTLOADER_BLE_JUMP_BOOTLOADER | PC_UART_CCOMMAND_MASK_BOOTLOADER};
			_uart_data_param param;
			param.user_callback = NULL;
			param.p_data = (uint8_t *)command;
			param.length = sizeof(command);
			uint8_t count = TASK_UART_DELAY_COUNT_OUT;
			while(drv_ble_send_data(&param) != ERR_SUCCESS && count--)
			{
				DELAY_MS(TASK_UART_DELAY);
			}	
			timer_exception_start(TIMER_EXCEPTION_TIMEOUT);
		}break;	
		case PC_UART_CCOMMAND_BOOTLOADER_BLE_JUMP_APPLICATION:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_CCOMMAND_BOOTLOADER_BLE_JUMP_APPLICATION\n");
			timer_exception_stop();
			uint32_t command[1] = {PC_UART_CCOMMAND_BOOTLOADER_BLE_JUMP_APPLICATION | PC_UART_CCOMMAND_MASK_BOOTLOADER};
			_uart_data_param param;
			param.user_callback = NULL;
			param.p_data = (uint8_t *)command;
			param.length = sizeof(command);
			uint8_t count = TASK_UART_DELAY_COUNT_OUT;
			while(drv_ble_send_data(&param) != ERR_SUCCESS && count--)
			{
				DELAY_MS(TASK_UART_DELAY);
			}	
			

			timer_exception_start(TIMER_EXCEPTION_TIMEOUT);	
			
		}break;	
		case PC_UART_CCOMMAND_BOOTLOADER_BLE_RESET:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_CCOMMAND_BOOTLOADER_BLE_RESET\n");
			timer_exception_stop();
			uint32_t command[1] = {PC_UART_CCOMMAND_BOOTLOADER_BLE_RESET | PC_UART_CCOMMAND_MASK_BOOTLOADER};
			_uart_data_param param;
			param.user_callback = NULL;
			param.p_data = (uint8_t *)command;
			param.length = sizeof(command);
			uint8_t count = TASK_UART_DELAY_COUNT_OUT;
			while(drv_ble_send_data(&param) != ERR_SUCCESS && count--)
			{
				DELAY_MS(TASK_UART_DELAY);
			}	
			timer_exception_start(TIMER_EXCEPTION_TIMEOUT);	
			
		}break;	
		case PC_UART_CCOMMAND_BOOTLOADER_BLE_BOOTSETTING:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_CCOMMAND_BOOTLOADER_BLE_BOOTSETTING\n");
			timer_exception_stop();
			ble_bootsetting(p_data, length);
			timer_exception_start(TIMER_EXCEPTION_TIMEOUT);	
		}break;	
		case PC_UART_CCOMMAND_BOOTLOADER_BLE_PREVALIDATE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_CCOMMAND_BOOTLOADER_BLE_PREVALIDATE\n");
			timer_exception_stop();					
			ble_prevalidate(p_data, length);
			timer_exception_start(TIMER_EXCEPTION_TIMEOUT);	
		}break;	
		case PC_UART_CCOMMAND_BOOTLOADER_BLE_IMAGE_DATA:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_CCOMMAND_BOOTLOADER_BLE_IMAGE_DATA\n");
			timer_exception_stop();	
//			g_dev_dfu_progress.percentage = (m_buffer.image_written_size*100)/m_buffer.image_total_size;//不用显示进度条
			ble_image_data(p_data,length);
			timer_exception_start(TIMER_EXCEPTION_TIMEOUT);	
		}break;	
		case PC_UART_CCOMMAND_BOOTLOADER_BLE_IMAGE_WRITE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_CCOMMAND_BOOTLOADER_BLE_IMAGE_WRITE\n");
			if(current_command == PC_UART_CCOMMAND_BOOTLOADER_BLE_POSTVALIDATE ||
				 current_command == PC_UART_CCOMMAND_BOOTLOADER_BLE_RESET_N_ACTIVATE
				)
			{
				timer_exception_stop();			
				exception_response();
				timer_exception_start(TIMER_EXCEPTION_TIMEOUT);						
			}
			else
			{
				timer_exception_stop();			
				ble_iamge_write();
				gui_boot_task_display();
				timer_exception_start(TIMER_EXCEPTION_TIMEOUT);				
			}
		}break;	
		case PC_UART_CCOMMAND_BOOTLOADER_BLE_POSTVALIDATE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_CCOMMAND_BOOTLOADER_BLE_POSTVALIDATE\n");
			current_command = PC_UART_CCOMMAND_BOOTLOADER_BLE_POSTVALIDATE;
			timer_exception_stop();	
			ble_postvalidate(p_data, length);
			timer_exception_start(TIMER_EXCEPTION_TIMEOUT);
		}break;	
		case PC_UART_CCOMMAND_BOOTLOADER_BLE_RESET_N_ACTIVATE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_CCOMMAND_BOOTLOADER_BLE_RESET_N_ACTIVATE\n");
			current_command = PC_UART_CCOMMAND_BOOTLOADER_BLE_RESET_N_ACTIVATE;
			timer_exception_stop();		
			ble_reset_N_activate(p_data, length);	
			timer_exception_start(TIMER_EXCEPTION_TIMEOUT);	
				
		}break;	
	
		default:{
			
			uint32_t command[1] = {PC_UART_CCOMMAND_BOOTLOADER_UNSPPORT | PC_UART_CCOMMAND_MASK_BOOTLOADER};
			drv_pc_send_data((uint8_t *)command, sizeof(command));
			
		}break;
	}



}





#endif

#ifdef LAYER_UPDATEALGORITHM  



bool dev_dfu_update_algorithm_start(void)
{
	
	_boot_setting boot_setting;
	lib_boot_setting_read(&boot_setting);

	
	if(boot_setting.bank_bootloader.command != BOOT_SETTING_BANK_COMMAND_NEW &&
		 boot_setting.bank_application.command != BOOT_SETTING_BANK_COMMAND_NEW &&
		 boot_setting.bank_update_algorithm.command != BOOT_SETTING_BANK_COMMAND_NEW
		)
	{
		return false;
	}
	
	if(boot_setting.bank_bootloader.command == BOOT_SETTING_BANK_COMMAND_NEW)
	{
		LOG_PRINTF(MOUDLE_NAME"bank_bootloader BOOT_SETTING_BANK_COMMAND_NEW\n");	
		
		uint32_t src_addr = boot_setting.bank_bootloader.extFlash_address;
		uint32_t dest_addr = boot_setting.bank_bootloader.intFlash_address;
		uint32_t size = boot_setting.bank_bootloader.image_size;
		LOG_PRINTF(MOUDLE_NAME"src_addr = 0x%08x\n",src_addr);
		LOG_PRINTF(MOUDLE_NAME"dest_addr = 0x%08x\n",dest_addr);
		LOG_PRINTF(MOUDLE_NAME"size = 0x%08x\n",size);
		dev_dfu_flash_memcpy(dest_addr,src_addr,size);
		uint32_t crc = am_bootloader_fast_crc32((uint32_t *)dest_addr,size);
		LOG_PRINTF(MOUDLE_NAME"calculate crc = 0x%08x\n",crc);
		LOG_PRINTF(MOUDLE_NAME"dfu crc = 0x%08x\n",boot_setting.bank_bootloader.image_crc);
		if(crc != boot_setting.bank_bootloader.image_crc)
		{
			LOG_PRINTF(MOUDLE_NAME"CRC MISMATCH\n");	
//			uint32_t command[1] = {PC_UART_RESPONSE_UPDATE_ALGORITHM_BOOTLOADER_CRC_ERROR | PC_UART_COMMAND_MASK_UPDATE_ALGORITHM};
//			drv_pc_send_data((uint8_t *)command, sizeof(command));			
			return false;
		}			
		
		boot_setting.bootloader_version = boot_setting.bank_bootloader.version;
		boot_setting.bank_bootloader.command = BOOT_SETTING_BANK_COMMAND_DEFAULT;
//		uint32_t command[1] = {PC_UART_RESPONSE_UPDATE_ALGORITHM_BOOTLOADER_SUCCESS | PC_UART_COMMAND_MASK_UPDATE_ALGORITHM};
//		drv_pc_send_data((uint8_t *)command, sizeof(command));		
	}	

	
	if(boot_setting.bank_application.command == BOOT_SETTING_BANK_COMMAND_NEW)
	{
		LOG_PRINTF(MOUDLE_NAME"bank_application BOOT_SETTING_BANK_COMMAND_NEW\n");
		uint32_t src_addr = boot_setting.bank_application.extFlash_address;
		uint32_t dest_addr = boot_setting.bank_application.intFlash_address;
		uint32_t size = boot_setting.bank_application.image_size;
		LOG_PRINTF(MOUDLE_NAME"src_addr = 0x%08x\n",src_addr);
		LOG_PRINTF(MOUDLE_NAME"dest_addr = 0x%08x\n",dest_addr);
		LOG_PRINTF(MOUDLE_NAME"size = 0x%08x\n",size);
		dev_dfu_flash_memcpy(dest_addr,src_addr,size);
		uint32_t crc = am_bootloader_fast_crc32((uint32_t *)dest_addr,size);
		LOG_PRINTF(MOUDLE_NAME"calculate crc = 0x%08x\n",crc);
		LOG_PRINTF(MOUDLE_NAME"dfu crc = 0x%08x\n",boot_setting.bank_application.image_crc);
		if(crc != boot_setting.bank_application.image_crc)
		{
			LOG_PRINTF(MOUDLE_NAME"CRC MISMATCH\n");	
//			uint32_t command[1] = {PC_UART_RESPONSE_UPDATE_ALGORITHM_APPLICATION_CRC_ERROR | PC_UART_COMMAND_MASK_UPDATE_ALGORITHM};
//			drv_pc_send_data((uint8_t *)command, sizeof(command));			
			return false;
		}			
		boot_setting.application_version = boot_setting.bank_application.version;
		boot_setting.bank_application.command = BOOT_SETTING_BANK_COMMAND_DEFAULT;
//		uint32_t command[1] = {PC_UART_RESPONSE_UPDATE_ALGORITHM_APPLICATION_SUCCESS | PC_UART_COMMAND_MASK_UPDATE_ALGORITHM};
//		drv_pc_send_data((uint8_t *)command, sizeof(command));			
		
	}	
	


	boot_setting.boot_setting_crc = am_bootloader_fast_crc32(&boot_setting.boot_setting_size,boot_setting.boot_setting_size-4);
	lib_boot_setting_write(&boot_setting);
	
	
	return true;
}


#endif














