#include "dev_dfu_ble.h"
#include "drv_apollo2.h"
#include "nrf_power.h"
#include "nrf_soc.h"
#include "nrf_clock.h"
#include "nrf_drv_clock.h"

#include "nrf_bootloader_app_start.h"
#include "nrf_bootloader_info.h"
#include "nrf_dfu_settings.h"
#include "nrf_dfu_utils.h"
#include "nrf_dfu_req_handler.h"
#include "app_scheduler.h"
#include "app_timer.h"
#include "ble.h"
#include "dev_extFlash.h"
#include "drv_extFlash.h"
#include "cmd_gpregret.h"





#include <stddef.h>
//#include "boards.h"
#include "sdk_common.h"
#include "nrf_dfu_transport.h"
#include "nrf_dfu_req_handler.h"
#include "nrf_dfu_handling_error.h"
#include "nrf_dfu_mbr.h"
#include "nrf_bootloader_info.h"
#include "ble_conn_params.h"
#include "ble_hci.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_delay.h"
#include "app_timer.h"
#include "nrf_dfu_svci.h"
#include "nrf_dfu_settings.h"
#include "nrf_gpio.h"
#include "cmd_apollo2.h"
#include "drv_apollo2.h"
#include "am_bootloader.h"
#include <string.h>
#include "nrf_dfu_settings.h"
#include "nrf_dfu_mbr.h"
#include "nrf_bootloader_app_start.h"
#include "nrf_bootloader_info.h"
#include "crc32.h"
#include "nrf_drv_gpiote.h"

#include "nrf_pwr_mgmt.h"

#define LOG_LEVEL_OFF 							0
#define LOG_LEVEL_ERROR 						1
#define LOG_LEVEL_WARNING 					2
#define LOG_LEVEL_INFO 							3
#define LOG_LEVEL_DEBUG 						4
#define NRF_LOG_MODULE_NAME dev_dfu_ble
#define NRF_LOG_LEVEL       LOG_LEVEL_DEBUG
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
NRF_LOG_MODULE_REGISTER();


#define BOOT_SETTING_BANK_COMMAND_DEFAULT                                0
#define BOOT_SETTING_BANK_COMMAND_CURRENT                                1
#define BOOT_SETTING_BANK_COMMAND_NEW                                    2


#define SCHED_EVENT_DATA_SIZE                256 
#define SCHED_QUEUE_SIZE                     16  
#define EXT_FLASH_OAT_ADDRESS               0x700000
#define APPLICATION_ADDRESS_OFFSET          1024

static void read_ble_bootsetting_application_address(uint32_t *bootsetting_addr);

static void clock_init(void)
{
    ret_code_t err_code;
    NRF_LOG_DEBUG("Initializing the clock.");
	
//		if(!nrf_drv_clock_init_check())
		{
			err_code = nrf_drv_clock_init();
			APP_ERROR_CHECK(err_code);		
			nrf_drv_clock_lfclk_request(NULL);
//			while (!nrf_clock_lf_is_running()) {;;}
		}
   
}

static void timers_init(void)
{
    NRF_LOG_DEBUG("timers_init");


}


static void scheduler_init(void)
{
	NRF_LOG_DEBUG("scheduler_init");
  APP_SCHED_INIT(SCHED_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);    
}


static void wait_for_event()
{
    while (true)
    {
				NRF_WDT->RR[0] = WDT_RR_RR_Reload;
        app_sched_execute();
        if (!NRF_LOG_PROCESS())
        {
					// Wait for an event.
					__WFE();
					// Clear the internal event register.
					__SEV();
					__WFE();				
        }
    }    
}
static void device_init(void)
{
	NRF_LOG_INFO("device_init");
	nrf_gpio_cfg_default(EXTFLASH_SPI_SS_PIN);
	nrf_gpio_cfg_default(EXTFLASH_SPI_SCK_PIN);
	nrf_gpio_cfg_default(EXTFLASH_SPI_MOSI_PIN);
	nrf_gpio_cfg_default(EXTFLASH_SPI_MISO_PIN);	
	
	nrf_gpio_cfg_output(EXTFLASH_READY_PIN);
	nrf_gpio_pin_clear(EXTFLASH_READY_PIN);		
}

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
		drv_extFlash_pwr_up();
		dev_extFlash_read(currentAddr, buffer, bytesInBuffer);
		drv_extFlash_pwr_down();
		dev_extFlash_disable();	
		
		am_bootloader_partial_crc32(buffer, bytesInBuffer, &output_crc);
		currentAddr += bytesInBuffer;
		imageSizeWritten -= bytesInBuffer;
	}	
	NRF_LOG_DEBUG("input_crc --> 0x%08X",input_crc);
	NRF_LOG_DEBUG("output_crc --> 0x%08X",output_crc);
	if(input_crc == output_crc)
	{
		return true;
	}

	return false;
}
	

struct s_ble_image_bank
{
		uint32_t                extFlash_address;
    uint32_t                image_size;         
    uint32_t                image_crc;  
		uint32_t                version;   	
    uint32_t                command;          
};
typedef struct s_ble_image_bank _ble_image_bank;
struct s_ble_boot_setting
{
    uint32_t            boot_setting_crc;   
		uint32_t            boot_setting_size;
		uint32_t            pcb_version; 

    _ble_image_bank         bank_bootloader;             
    _ble_image_bank         bank_application;             
		_ble_image_bank         bank_softdevice; 
};
typedef struct s_ble_boot_setting _ble_boot_setting;
static _ble_boot_setting m_ble_boot_setting = {0};

static uint32_t bootsetting_read(void)
{
	#ifdef COD 
    uint32_t application_address = 0x0;
	uint32_t bootsetting_address = 0x0; 
	#else
	uint32_t bootsetting_address = 0x800000-0x1000;
	#endif
	uint32_t bootsetting_size = sizeof(_ble_boot_setting);	

	dev_extFlash_enable();
	drv_extFlash_pwr_up();
	#ifdef COD 
    read_ble_bootsetting_application_address(&bootsetting_address); 
	#endif
	dev_extFlash_read(bootsetting_address, (uint8_t *)&m_ble_boot_setting, bootsetting_size);
	drv_extFlash_pwr_down();
	dev_extFlash_disable();	
	
	
	NRF_LOG_HEXDUMP_DEBUG((uint8_t *)&m_ble_boot_setting, sizeof(_ble_boot_setting)) ;	
	
	NRF_LOG_DEBUG("ble boot_setting_crc --> 0x%08x",m_ble_boot_setting.boot_setting_crc);
	NRF_LOG_DEBUG("ble boot_setting_size --> %d",m_ble_boot_setting.boot_setting_size);
	NRF_LOG_DEBUG("ble pcb_version --> 0x%08x",m_ble_boot_setting.pcb_version);
	NRF_LOG_DEBUG("ble bank_bootloader extFlash_address --> 0x%08x",m_ble_boot_setting.bank_bootloader.extFlash_address);
	NRF_LOG_DEBUG("ble bank_bootloader image_size --> %d",m_ble_boot_setting.bank_bootloader.image_size);
	NRF_LOG_DEBUG("ble bank_bootloader image_crc --> 0x%08x",m_ble_boot_setting.bank_bootloader.image_crc);
	NRF_LOG_DEBUG("ble bank_bootloader version --> 0x%08x",m_ble_boot_setting.bank_bootloader.version);
	NRF_LOG_DEBUG("ble bank_bootloader command --> 0x%08x",m_ble_boot_setting.bank_bootloader.command);
	NRF_LOG_DEBUG("ble bank_application extFlash_address --> 0x%08x",m_ble_boot_setting.bank_application.extFlash_address);
	NRF_LOG_DEBUG("ble bank_application image_size --> %d",m_ble_boot_setting.bank_application.image_size);
	NRF_LOG_DEBUG("ble bank_application image_crc --> 0x%08x",m_ble_boot_setting.bank_application.image_crc);
	NRF_LOG_DEBUG("ble bank_application version --> 0x%08x",m_ble_boot_setting.bank_application.version);
	NRF_LOG_DEBUG("ble bank_application command --> 0x%08x",m_ble_boot_setting.bank_application.command);
	NRF_LOG_DEBUG("ble bank_softdevice extFlash_address --> 0x%08x",m_ble_boot_setting.bank_softdevice.extFlash_address);
	NRF_LOG_DEBUG("ble bank_softdevice image_size --> %d",m_ble_boot_setting.bank_softdevice.image_size);
	NRF_LOG_DEBUG("ble bank_softdevice image_crc --> 0x%08x",m_ble_boot_setting.bank_softdevice.image_crc);
	NRF_LOG_DEBUG("ble bank_softdevice version --> 0x%08x",m_ble_boot_setting.bank_softdevice.version);
	NRF_LOG_DEBUG("ble bank_softdevice command --> 0x%08x",m_ble_boot_setting.bank_softdevice.command);	
	
	
	
	
	uint32_t crc_dest = am_bootloader_fast_crc32((uint8_t*)(&m_ble_boot_setting.boot_setting_crc)+sizeof(m_ble_boot_setting.boot_setting_crc),sizeof(_ble_boot_setting) - sizeof(m_ble_boot_setting.boot_setting_crc));
	
	
	if(crc_dest != m_ble_boot_setting.boot_setting_crc)
	{
		NRF_LOG_INFO("BOOTLOADER SETTING CRC MISMATCH");
		NRF_LOG_INFO("crc_dest --> 0x%08x",crc_dest);
		NRF_LOG_INFO("ble boot_setting_crc --> 0x%08x",m_ble_boot_setting.boot_setting_crc);
		return NRF_ERROR_INTERNAL;
	}
	#ifdef COD 
	//modify ble bank_application extFlash_address
    m_ble_boot_setting.bank_application.extFlash_address = bootsetting_address + APPLICATION_ADDRESS_OFFSET;
	#endif
    
	return NRF_SUCCESS;
}
static uint32_t dfu_update_application(uint32_t src_addr, uint32_t src_size, uint32_t src_crc)
{
	if(m_ble_boot_setting.bank_application.command != BOOT_SETTING_BANK_COMMAND_NEW)
	{
		return NRF_SUCCESS;
	}
	
	NRF_LOG_INFO("dfu_update_application start...");	
	
    uint32_t const image_size  = src_size;
    uint32_t const split_size  = CODE_PAGE_SIZE; // Must be page aligned

    uint32_t ret_val     = NRF_SUCCESS;
    uint32_t target_addr = MAIN_APPLICATION_START_ADDR;
    uint32_t length_left = image_size;
    uint32_t cur_len;
    uint32_t crc;

		uint8_t transfer_buffer[CODE_PAGE_SIZE];
	

    // Copy the application down safely
    do
    {
        cur_len = (length_left > split_size) ? split_size : length_left;

        // Erase the target page
        ret_val = nrf_dfu_flash_erase(target_addr, split_size / CODE_PAGE_SIZE, NULL);
        if (ret_val != NRF_SUCCESS)
        {
            return ret_val;
        }
				
				dev_extFlash_enable();
				drv_extFlash_pwr_up();
				dev_extFlash_read(src_addr, transfer_buffer, cur_len);
				drv_extFlash_pwr_down();
				dev_extFlash_disable();	
				
        // Flash one page
        ret_val = nrf_dfu_flash_store(target_addr, (uint32_t*)transfer_buffer, cur_len, NULL);
        if (ret_val != NRF_SUCCESS)
        {
            return ret_val;
        }

        target_addr += cur_len;
        src_addr += cur_len;

        length_left -= cur_len;
    }
    while (length_left > 0);

    // Check the CRC of the copied data. Enable if so.
    crc = am_bootloader_fast_crc32((uint8_t *)MAIN_APPLICATION_START_ADDR,image_size);

    if (crc == src_crc)
    {
        NRF_LOG_INFO("dfu_update_application valid");
    }
    else
    {
        NRF_LOG_ERROR("CRC computation failed for copied app: "
                      "src crc: 0x%08x, res crc: 0x%08x",
                      src_crc,
                      crc);
				return NRF_ERROR_INTERNAL;
    }


		s_dfu_settings.app_version = m_ble_boot_setting.bank_application.version;
		s_dfu_settings.bank_0.bank_code = NRF_DFU_BANK_VALID_APP;
		s_dfu_settings.bank_0.image_size = m_ble_boot_setting.bank_application.image_size;
		s_dfu_settings.bank_0.image_crc = crc32_compute((uint8_t*) CODE_REGION_1_START,
																		 s_dfu_settings.bank_0.image_size,
																		 NULL);

    return ret_val;
}

static uint32_t dfu_update_bootloader(uint32_t src_addr, uint32_t src_size, uint32_t src_crc)
{
	
	if(m_ble_boot_setting.bank_bootloader.command != BOOT_SETTING_BANK_COMMAND_NEW)
	{
		return NRF_SUCCESS;
	}	
	NRF_LOG_INFO("dfu_update_bootloader start...");	
	
	NRF_LOG_INFO("src_addr: 0x%08x, src_size: 0x%08x, src_crc: 0x%08x", src_addr, src_size, src_crc);
    uint32_t const image_size  = src_size;
    uint32_t const split_size  = CODE_PAGE_SIZE; // Must be page aligned

    uint32_t ret_val     = NRF_SUCCESS;
    uint32_t target_addr = BOOTLOADER_UPDATE_BUFFER_START_ADDR;
    uint32_t length_left = image_size;
    uint32_t cur_len;
    uint32_t crc;
		uint8_t transfer_buffer[CODE_PAGE_SIZE];
/*
1. Copy bootloader image from ExtFlash to Nrf52 IntFlash(0x5B000).
2. Call nrf_dfu_mbr_copy_bl function to copy Nrf52 IntFlash(0x5B000) data to Nrf52 IntFlash(0x6E000).
*/	
    do
    {
        cur_len = (length_left > split_size) ? split_size : length_left;

        // Erase the target page
        ret_val = nrf_dfu_flash_erase(target_addr, split_size / CODE_PAGE_SIZE, NULL);
        if (ret_val != NRF_SUCCESS)
        {
            return ret_val;
        }
				dev_extFlash_enable();
				drv_extFlash_pwr_up();
				dev_extFlash_read(src_addr, transfer_buffer, cur_len);
				drv_extFlash_pwr_down();
				dev_extFlash_disable();	
				
				
        // Flash one page
        ret_val = nrf_dfu_flash_store(target_addr, (uint32_t*)transfer_buffer, cur_len, NULL);
        if (ret_val != NRF_SUCCESS)
        {
            return ret_val;
        }

        target_addr += cur_len;
        src_addr += cur_len;

        length_left -= cur_len;
    }
    while (length_left > 0);	
	
    // Check the CRC of the copied data. Enable if so.
    crc = am_bootloader_fast_crc32((uint8_t *)BOOTLOADER_UPDATE_BUFFER_START_ADDR,image_size);

    if (crc == src_crc)
    {
        NRF_LOG_INFO("dfu_update_bootloader valid");
    }
    else
    {
        NRF_LOG_ERROR("CRC computation failed for copied app: "
                      "src crc: 0x%08x, res crc: 0x%08x",
                      src_crc,
                      crc);
				ret_val = NRF_ERROR_INTERNAL;
    }	
		
    return ret_val;
}


static uint32_t dfu_update_softdevice(uint32_t src_addr, uint32_t src_size, uint32_t src_crc)
{
	if(m_ble_boot_setting.bank_softdevice.command != BOOT_SETTING_BANK_COMMAND_NEW)
	{
		return NRF_SUCCESS;
	}	
	
	NRF_LOG_INFO("dfu_update_softdevice start...");
	
    uint32_t const image_size  = src_size;
    uint32_t const split_size  = CODE_PAGE_SIZE; // Must be page aligned	
    uint32_t ret_val     = NRF_SUCCESS;
    uint32_t target_addr = SOFTDEVICE_REGION_START;
    uint32_t length_left = image_size;
    uint32_t cur_len;
    uint32_t crc;
    NRF_LOG_INFO("Enter dfu_update_softdevice");
		uint8_t transfer_buffer[CODE_PAGE_SIZE];
		uint32_t softdevice_size = MAIN_APPLICATION_START_ADDR - SOFTDEVICE_REGION_START;
		// Erase the target page
		ret_val = nrf_dfu_flash_erase(target_addr, softdevice_size / CODE_PAGE_SIZE, NULL);
		APP_ERROR_CHECK(ret_val);		
	
    do
    {
        cur_len = (length_left > split_size) ? split_size : length_left;
			
				dev_extFlash_enable();
				drv_extFlash_pwr_up();
				dev_extFlash_read(src_addr, transfer_buffer, cur_len);
				drv_extFlash_pwr_down();
				dev_extFlash_disable();				
				
        // Copy a chunk of the SD. Size in words
        ret_val = nrf_dfu_mbr_copy_sd((uint32_t*)target_addr, (uint32_t*)transfer_buffer, split_size);
        if (ret_val != NRF_SUCCESS)
        {
            NRF_LOG_ERROR("Failed to copy SD: target: 0x%08x, src: 0x%08x, len: 0x%08x", target_addr, src_addr, split_size);
            return ret_val;
        }
        // Validate copy. Size in words
        ret_val = nrf_dfu_mbr_compare((uint32_t*)target_addr, (uint32_t*)transfer_buffer, split_size);
        if (ret_val != NRF_SUCCESS)
        {
            NRF_LOG_ERROR("Failed to Compare SD: target: 0x%08x, src: 0x%08x, len: 0x%08x", target_addr, src_addr, split_size);
            return ret_val;
        }				

        target_addr += cur_len;
        src_addr += cur_len;

        length_left -= cur_len;
    }
    while (length_left > 0);

    // Check the CRC of the copied data. Enable if so.
    crc = am_bootloader_fast_crc32((uint8_t *)SOFTDEVICE_REGION_START,image_size);

    if (crc == src_crc)
    {
        NRF_LOG_INFO("dfu_update_softdevice valid");
    }
    else
    {
        NRF_LOG_ERROR("CRC computation failed for copied app: "
                      "src crc: 0x%08x, res crc: 0x%08x",
                      src_crc,
                      crc);
				return NRF_ERROR_INTERNAL;
    }		


    return ret_val;
}
static uint32_t image_copy(void)
{
	uint32_t error = 0;
	uint32_t bank_addr = 0;
	uint32_t bank_size = 0;
	uint32_t bank_crc = 0;		
	
	if(m_ble_boot_setting.bank_bootloader.command == BOOT_SETTING_BANK_COMMAND_NEW)
	{
		if(m_ble_boot_setting.bank_bootloader.version < s_dfu_settings.bootloader_version)
		{
			NRF_LOG_DEBUG("BOOTLOADER VERSION IS TOO LOW");
			return NRF_ERROR_INTERNAL;
		}			
		bank_addr = m_ble_boot_setting.bank_bootloader.extFlash_address;
		bank_size = m_ble_boot_setting.bank_bootloader.image_size;
		bank_crc = m_ble_boot_setting.bank_bootloader.image_crc;		
		NRF_LOG_INFO("m_ble_boot_setting.bank_bootloader.image_crc --> 0x%08X",m_ble_boot_setting.bank_bootloader.image_crc);
		if(check_crc_from_external_flash(bank_crc,bank_addr,bank_size))
		{
			NRF_LOG_INFO("bootloader image crc is correct");
			error = dfu_update_bootloader(m_ble_boot_setting.bank_bootloader.extFlash_address, 
																		m_ble_boot_setting.bank_bootloader.image_size, 
																		m_ble_boot_setting.bank_bootloader.image_crc);
			if(error != NRF_SUCCESS)
			{
				return error;
			}			
		}
		else
		{
			NRF_LOG_INFO("bootloader image crc is wrong");
			return NRF_ERROR_INTERNAL;
		}		
		
	}
	if(m_ble_boot_setting.bank_application.command == BOOT_SETTING_BANK_COMMAND_NEW)
	{
		if(m_ble_boot_setting.bank_application.version < s_dfu_settings.app_version)
		{
			NRF_LOG_DEBUG("APPLICATION VERSION IS TOO LOW");
			return NRF_ERROR_INTERNAL;
		}			
		bank_addr = m_ble_boot_setting.bank_application.extFlash_address;
		bank_size = m_ble_boot_setting.bank_application.image_size;
		bank_crc = m_ble_boot_setting.bank_application.image_crc;
		NRF_LOG_INFO("m_ble_boot_setting.bank_application.image_crc --> 0x%08X",m_ble_boot_setting.bank_application.image_crc);		
		if(check_crc_from_external_flash(bank_crc,bank_addr,bank_size))
		{
			NRF_LOG_INFO("application image crc is correct");
			error = dfu_update_application(m_ble_boot_setting.bank_application.extFlash_address, 
																		 m_ble_boot_setting.bank_application.image_size, 
																		 m_ble_boot_setting.bank_application.image_crc);
			if(error != NRF_SUCCESS)
			{
				return error;
			}
		}
		else
		{
			NRF_LOG_INFO("application image crc is wrong");
			return NRF_ERROR_INTERNAL;
		}				
	}
	if(m_ble_boot_setting.bank_softdevice.command == BOOT_SETTING_BANK_COMMAND_NEW)
	{
		if(m_ble_boot_setting.bank_softdevice.version < SD_FWID_GET(MBR_SIZE))
		{
			NRF_LOG_INFO("SOFTDEVICE VERSION IS TOO LOW");
			return NRF_ERROR_INTERNAL;
		}		
		bank_addr = m_ble_boot_setting.bank_softdevice.extFlash_address;
		bank_size = m_ble_boot_setting.bank_softdevice.image_size;
		bank_crc = m_ble_boot_setting.bank_softdevice.image_crc;		
		NRF_LOG_INFO("m_ble_boot_setting.bank_softdevice.image_crc --> 0x%08X",m_ble_boot_setting.bank_softdevice.image_crc);
		if(check_crc_from_external_flash(bank_crc,bank_addr,bank_size))
		{
			NRF_LOG_INFO("softdevice image crc is correct");
			error = dfu_update_softdevice(m_ble_boot_setting.bank_softdevice.extFlash_address, 
																		m_ble_boot_setting.bank_softdevice.image_size, 
																		m_ble_boot_setting.bank_softdevice.image_crc);		
			if(error != NRF_SUCCESS)
			{
				return error;
			}			
		}
		else
		{
			NRF_LOG_INFO("softdevice image crc is wrong");
			return NRF_ERROR_INTERNAL;
		}			
	}
	
	return NRF_SUCCESS;
	
}

static uint32_t write_bootsetting(void)
{
	
	uint32_t ret_val = NRF_SUCCESS;
	
	if(m_ble_boot_setting.bank_bootloader.command == BOOT_SETTING_BANK_COMMAND_NEW)
	{		
		/**<nrf_dfu_settings_write will write new bootsetting crc to bootsetting region>*/		
		NRF_LOG_DEBUG("reset_n_activate bootloader copy start...");	
		s_dfu_settings.bootloader_version = m_ble_boot_setting.bank_bootloader.version;
		ret_val = nrf_dfu_settings_write(NULL);	//nrf_dfu_mbr_copy_bl函数不会返回，直接复位芯片
		APP_ERROR_CHECK(ret_val);		
/** @brief Function for copying the bootloader using an MBR command.
 *
 * @param[in] p_src         Source address of the bootloader data to copy.
 * @param[in] len           Length of the data to copy in bytes.
 *
 * @return  This function will return only if the command request could not be run.
 *          See @ref sd_mbr_command_copy_bl_t for possible return values.
 */		
		ret_val = nrf_dfu_mbr_copy_bl((uint32_t*)BOOTLOADER_UPDATE_BUFFER_START_ADDR, m_ble_boot_setting.bank_bootloader.image_size);
		
		if (ret_val == NRF_SUCCESS)
		{
			
		}	
		else{
			NRF_LOG_INFO("nrf_dfu_mbr_copy_bl error --> 0x%08X",ret_val);
			return ret_val;
		}
			
	}	
	
	ret_val = nrf_dfu_settings_write(NULL);	
	APP_ERROR_CHECK(ret_val);	
	
	return ret_val;
	
}

static uint32_t dfu_process(void)
{
	NRF_LOG_INFO("dfu_process");

	uint32_t error = NRF_SUCCESS;
	
	error = bootsetting_read();
	if(error != NRF_SUCCESS)
	{
		NRF_LOG_INFO("bootsetting_read error");
		return NRF_ERROR_INTERNAL;
	}	

	error = image_copy();
	if(error != NRF_SUCCESS)
	{
		NRF_LOG_INFO("image_copy error");
		return NRF_ERROR_INTERNAL;
	}		
	
	error = write_bootsetting();
	if(error != NRF_SUCCESS)
	{
		NRF_LOG_INFO("write_bootsetting error");
		return NRF_ERROR_INTERNAL;
	}			
	NRF_LOG_INFO("dfu_process success");
	return NRF_SUCCESS;
}

void dev_dfu_ble_init(void)
{
	uint32_t error = NRF_SUCCESS;
	NRF_LOG_INFO("dev_dfu_ble_init");
	NRF_WDT->RR[0] = WDT_RR_RR_Reload;
	nrf_dfu_settings_init(false);
	
	NRF_LOG_HEXDUMP_DEBUG((uint8_t *)&s_dfu_settings, sizeof(nrf_dfu_settings_t)) ;	
	
	NRF_LOG_DEBUG("s_dfu_settings.settings_version --> 0x%08X",s_dfu_settings.settings_version);
	NRF_LOG_DEBUG("s_dfu_settings.bootloader_version --> 0x%08X",s_dfu_settings.bootloader_version);
	NRF_LOG_DEBUG("s_dfu_settings.app_version --> 0x%08X",s_dfu_settings.app_version);
	NRF_LOG_DEBUG("softdevice_version --> 0x%08X",SD_FWID_GET(MBR_SIZE));	
	
	
	clock_init();
	timers_init();	
	device_init();
	scheduler_init();
	
	error = dfu_process();
	NRF_LOG_DEBUG("dfu_progress return value --> 0x%08X",error);	
	//wait_for_event();			
	
	if(error != NRF_SUCCESS)
	{
		//nrf_power_gpregret_set(CMD_GPREGRET_BOOTLOADER_EXCEPTION_RESET);
		nrf_power_gpregret_set(CMD_GPREGRET_BOOTLOADER_BLE_DFU);
		nrf_delay_ms(100);
		NVIC_SystemReset();
	}		
	#ifdef COD 
    nrf_power_gpregret_set(CMD_GPREGRET_BOOTLOADER_BLE_DFU_FINISH);  //ble ota finish xiaoyang-20191212 
	#endif
	nrf_gpio_cfg_input(EXTFLASH_READY_PIN, NRF_GPIO_PIN_PULLUP);	
	
	NVIC_SystemReset();
}
#ifdef COD 
//???????
uint32_t reversebytes_uint32t(uint32_t value){
    return (value & 0x000000FFU) << 24 | (value & 0x0000FF00U) << 8 | 
        (value & 0x00FF0000U) >> 8 | (value & 0xFF000000U) >> 24; 
} 

static void read_ble_bootsetting_application_address(uint32_t *bootsetting_addr)
{
    cod_ota_file_info_t cod_ota_file;
	uint32_t apollo_length_address = 0;
    memset((uint8_t *)&cod_ota_file,0,sizeof(cod_ota_file_info_t));	

	dev_extFlash_read(EXT_FLASH_OAT_ADDRESS + sizeof(cod_ota_file.command), (uint8_t *)&cod_ota_file.extra_info_len, sizeof(cod_ota_file.extra_info_len));
	NRF_LOG_DEBUG("cod_ota_file.extra_info_len=0x%x",cod_ota_file.extra_info_len);
	
	apollo_length_address = EXT_FLASH_OAT_ADDRESS + sizeof(cod_ota_file.command) + sizeof(cod_ota_file.extra_info_len) + cod_ota_file.extra_info_len; 	
	NRF_LOG_DEBUG("apollo_length_address=0x%x",apollo_length_address);
	dev_extFlash_read(apollo_length_address, (uint8_t *)&cod_ota_file.apollo.data_len, sizeof(cod_ota_file.apollo.data_len));

	cod_ota_file.apollo.data_len = reversebytes_uint32t(cod_ota_file.apollo.data_len);
	NRF_LOG_DEBUG("cod_ota_file.apollo.data_len0x%x",cod_ota_file.apollo.data_len);  
    
	*bootsetting_addr = apollo_length_address + sizeof(cod_ota_file.apollo.data_len) + cod_ota_file.apollo.data_len + sizeof(cod_ota_file.apollo.crc) + sizeof(cod_ota_file.ble_id) + sizeof(cod_ota_file.ble.data_len);
                        
    NRF_LOG_DEBUG("bootsetting_addr --> 0x%08X",*bootsetting_addr);
}
#endif


