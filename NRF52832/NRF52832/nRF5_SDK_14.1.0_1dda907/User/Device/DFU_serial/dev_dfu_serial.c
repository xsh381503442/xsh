#include "dev_dfu_serial.h"
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
#define NRF_LOG_MODULE_NAME dev_dfu_serial
#define NRF_LOG_LEVEL       LOG_LEVEL_DEBUG
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
NRF_LOG_MODULE_REGISTER();

#define DELAY_MS(X)                 nrf_delay_ms(X)

#define SCHED_EVENT_DATA_SIZE                256 
#define SCHED_QUEUE_SIZE                     16     


static uint32_t *m_buffer = NULL;

static app_timer_t    m_exception_timer    = {{0}};
const  app_timer_id_t m_exception_timer_id = &m_exception_timer;


static app_timer_t    m_wdt_timer    = {{0}};
const  app_timer_id_t m_wdt_timer_id = &m_wdt_timer;

#ifndef SERIAL_DFU_EXCEPTION_TIMEOUT_MS
#define SERIAL_DFU_EXCEPTION_TIMEOUT_MS (60000*2)
#endif
static void exception_timer_start(void);
static void exception_timer_stop(void);



#define FONT_SPI2_SS_PIN          19 



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


struct s_dev_dfu_bootsetting{
	uint32_t command;
	uint32_t offset;
	uint32_t size;
	uint32_t crc;
};

typedef struct s_dev_dfu_bootsetting _dev_dfu_bootsetting;


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

struct s_dev_dfu_prevalidate{
	uint32_t command;
	_prevalidate_error_mask error_mask;
};
typedef struct s_dev_dfu_prevalidate _dev_dfu_prevalidate;

#define POSTVALIDATE_BANK_UPDATE_ALGORITHM_CRC_ERROR       (1UL << 0)
#define POSTVALIDATE_BOOTSETTING_CRC_ERROR                 (1UL << 1)
#define POSTVALIDATE_BOOTLOADER_UPDATE_CRC_ERROR           (1UL << 2)
#define POSTVALIDATE_APPLICATION_UPDATE_CRC_ERROR          (1UL << 3)
#define POSTVALIDATE_SOFTDEVICE_UPDATE_CRC_ERROR           (1UL << 4)
typedef uint32_t _postvalidate_error_mask;
struct s_dev_dfu_postvalidate{
	uint32_t command;
	_postvalidate_error_mask error_mask;
};
typedef struct s_dev_dfu_postvalidate _dev_dfu_postvalidate;

#define BOOT_SETTING_BANK_COMMAND_DEFAULT                                0
#define BOOT_SETTING_BANK_COMMAND_CURRENT                                1
#define BOOT_SETTING_BANK_COMMAND_NEW                                    2


#define BLE_BANK_BOOTLOADER_DEFAULT_SIZE        0x10000
#define BLE_BANK_APPLICATION_DEFAULT_SIZE       0x38000
#define BLE_BANK_SOFTDEVICE_DEFAULT_SIZE        0x22000


static _ble_boot_setting m_ble_boot_setting = {0};



struct s_dev_apollo2_ble_info
{

	uint32_t command;
	uint32_t setting_version;
	uint32_t bootloader_version;
	uint32_t application_version;
	uint32_t softdevice_version;
	uint32_t pcb_version;
	uint32_t mac_address_f4b;
	uint32_t mac_address_l2b;


	
};
typedef struct s_dev_apollo2_ble_info _dev_apollo2_ble_info;

#define YEAR ((((__DATE__ [7] - '0') * 10 + (__DATE__ [8] - '0')) * 10  + (__DATE__ [9] - '0')) * 10 + (__DATE__ [10] - '0'))  
  
#define MONTH (__DATE__ [2] == 'n' ? 0  : __DATE__ [2] == 'b' ? 1  : __DATE__ [2] == 'r' ? (__DATE__ [0] == 'M' ? 2 : 3)  : __DATE__ [2] == 'y' ? 4  : __DATE__ [2] == 'n' ? 5  : __DATE__ [2] == 'l' ? 6 : __DATE__ [2] == 'g' ? 7 : __DATE__ [2] == 'p' ? 8 : __DATE__ [2] == 't' ? 9 : __DATE__ [2] == 'v' ? 10 : 11)  
  
#define DAY ((__DATE__ [4] == ' ' ? 0 : __DATE__ [4] - '0') * 10+ (__DATE__ [5] - '0'))  
  
#define DATE_AS_INT (((YEAR - 2000) * 12 + MONTH) * 31 + DAY)  

#define HOUR ((__TIME__ [0] == ' ' ? 0 : __TIME__ [0] - '0') * 10+ (__TIME__ [1] - '0')) 
#define MINUTE ((__TIME__ [3] == ' ' ? 0 : __TIME__ [3] - '0') * 10+ (__TIME__ [4] - '0'))
#define SECOND ((__TIME__ [6] == ' ' ? 0 : __TIME__ [6] - '0') * 10+ (__TIME__ [7] - '0'))

void dev_apollo2_ble_info_get(_dev_apollo2_ble_info *ble_info)
{
	NRF_LOG_DEBUG("dev_apollo2_ble_info_get");
	memcpy((void*)&s_dfu_settings, (uint8_t *)BOOTLOADER_SETTINGS_ADDRESS, sizeof(nrf_dfu_settings_t));
	ble_info->command = BLE_UART_RESPONSE_BOOTLOADER_BLE_INFO | BLE_UART_CCOMMAND_MASK_BOOTLOADER;
	ble_info->setting_version = s_dfu_settings.settings_version;
	ble_info->bootloader_version = s_dfu_settings.bootloader_version;
	ble_info->application_version = s_dfu_settings.app_version;
	ble_info->softdevice_version = SD_FWID_GET(MBR_SIZE); 
	ble_info->pcb_version = NRF_UICR->CUSTOMER[0];
	
	uint32_t ficr_0 = NRF_FICR->DEVICEADDR[0];
	uint32_t ficr_1 = NRF_FICR->DEVICEADDR[1];
	
	ble_info->mac_address_f4b = ficr_0;
	ble_info->mac_address_l2b = ficr_1;

	
	
//	uint8_t	g_adv_manuf_mac[BLE_GAP_ADDR_LEN];
//	ble_gap_addr_t device_addr;
//	uint32_t err_code = sd_ble_gap_addr_get(&device_addr);
//	APP_ERROR_CHECK(err_code);

//	uint8_t *d = device_addr.addr;
//	for ( uint8_t i = 6; i >0;)
//	{	
//		i--;
//		g_adv_manuf_mac[5-i]= d[i];
//	}		
//	ble_info->mac_address_f4b = g_adv_manuf_mac[2]<<24|g_adv_manuf_mac[3]<<16|g_adv_manuf_mac[4]<<8|g_adv_manuf_mac[5];	
//	ble_info->mac_address_l2b = g_adv_manuf_mac[0]<<8|g_adv_manuf_mac[1];


}

static void jump_bootloader(void)
{
	NRF_LOG_DEBUG("jump_bootloader");
//	uint32_t command[1] = {BLE_UART_RESPONSE_BOOTLOADER_JUMP_BOOTLOADER | BLE_UART_CCOMMAND_MASK_BOOTLOADER};
	
	
	
}

static void jump_application(void)
{
	NRF_LOG_DEBUG("jump_application");
//	uint32_t command[1] = {BLE_UART_RESPONSE_BOOTLOADER_JUMP_APPLICATION | BLE_UART_CCOMMAND_MASK_BOOTLOADER};
}
static void reset(void)
{
	NRF_LOG_DEBUG("reset");	
	uint32_t command[1] = {BLE_UART_RESPONSE_BOOTLOADER_RESET | BLE_UART_CCOMMAND_MASK_BOOTLOADER};
	drv_apollo2_uart_put((uint8_t *)&command, sizeof(command));
}

static void bootsetting(void)
{
	NRF_LOG_DEBUG("bootsetting");
	uint8_t *pp_data = (uint8_t *)m_buffer + sizeof(_dev_dfu_bootsetting);
	_dev_dfu_bootsetting *bootsetting = (_dev_dfu_bootsetting *)m_buffer;
	NRF_LOG_DEBUG("offset : 0x%d , size : 0x%08x , crc : 0x%08x",bootsetting->offset,bootsetting->size,bootsetting->crc);
	
	
	m_ble_boot_setting = *(_ble_boot_setting *)pp_data;
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

	uint32_t crc = 0;
	crc = am_bootloader_fast_crc32(&m_ble_boot_setting,bootsetting->size);
	NRF_LOG_DEBUG("calculate crc --> 0x%08x",crc);
	
	
	_dev_dfu_bootsetting bootsetting_response;
	bootsetting_response.command = BLE_UART_RESPONSE_BOOTLOADER_BOOTSETTING | BLE_UART_CCOMMAND_MASK_BOOTLOADER;
	bootsetting_response.offset = bootsetting->offset;
	bootsetting_response.size = bootsetting->size;
	bootsetting_response.crc = crc;
	
	NRF_LOG_HEXDUMP_DEBUG((uint8_t *)&bootsetting_response, sizeof(_dev_dfu_bootsetting));
	
	drv_apollo2_uart_put((uint8_t *)&bootsetting_response, sizeof(_dev_dfu_bootsetting));

	
}
static void prevalidate(void)
{
	NRF_LOG_DEBUG("prevalidate");
	_dev_dfu_prevalidate prevalidate;
	_prevalidate_error_mask error_mask = PREVALIDATE_ERROR_MASK_DEFAULT;
	
	prevalidate.command = BLE_UART_RESPONSE_BOOTLOADER_PREVALIDATE | BLE_UART_CCOMMAND_MASK_BOOTLOADER;
	

//	if(m_ble_boot_setting.pcb_version < g_ble_info.pcb_version)
//	{
//		LOG_PRINTF(MOUDLE_NAME"PREVALIDATE_ERROR_MASK_PCB_VERSION\n");
//		error_mask |= PREVALIDATE_ERROR_MASK_PCB_VERSION;
//	}
	if(m_ble_boot_setting.bank_bootloader.command == BOOT_SETTING_BANK_COMMAND_NEW)
	{
		if(m_ble_boot_setting.bank_bootloader.extFlash_address > EXTFLASH_MAX_SIZE)
		{
			NRF_LOG_DEBUG("PREVALIDATE_ERROR_MASK_BANK_BOOT_EXT_ADDRESS");
			error_mask |= PREVALIDATE_ERROR_MASK_BANK_BOOT_EXT_ADDRESS;
		}			
		if(m_ble_boot_setting.bank_bootloader.image_size > BLE_BANK_BOOTLOADER_DEFAULT_SIZE)
		{
			NRF_LOG_DEBUG("ble bank_bootloader image_size --> %d",m_ble_boot_setting.bank_bootloader.image_size);
			NRF_LOG_DEBUG("BLE_BANK_BOOTLOADER_DEFAULT_SIZE --> %d",BLE_BANK_BOOTLOADER_DEFAULT_SIZE);
			NRF_LOG_DEBUG("PREVALIDATE_ERROR_MASK_BANK_BOOT_SIZE");
			error_mask |= PREVALIDATE_ERROR_MASK_BANK_BOOT_SIZE;
		}
		if(m_ble_boot_setting.bank_bootloader.version < s_dfu_settings.bootloader_version)
		{
			NRF_LOG_DEBUG("PREVALIDATE_ERROR_MASK_BANK_BOOT_VERSION");
			error_mask |= PREVALIDATE_ERROR_MASK_BANK_BOOT_VERSION;
		}		
	}
	if(m_ble_boot_setting.bank_application.command == BOOT_SETTING_BANK_COMMAND_NEW)
	{
		if(m_ble_boot_setting.bank_application.extFlash_address > EXTFLASH_MAX_SIZE)
		{
			NRF_LOG_DEBUG("PREVALIDATE_ERROR_MASK_BANK_APP_EXT_ADDRESS");
			error_mask |= PREVALIDATE_ERROR_MASK_BANK_APP_EXT_ADDRESS;
		}	
		if(m_ble_boot_setting.bank_application.image_size > BLE_BANK_APPLICATION_DEFAULT_SIZE)
		{
			NRF_LOG_DEBUG("PREVALIDATE_ERROR_MASK_BANK_APP_SIZE");
			error_mask |= PREVALIDATE_ERROR_MASK_BANK_APP_SIZE;
		}	
		if(m_ble_boot_setting.bank_application.version < s_dfu_settings.app_version)
		{
			NRF_LOG_DEBUG("PREVALIDATE_ERROR_MASK_BANK_APP_VERSION");
			error_mask |= PREVALIDATE_ERROR_MASK_BANK_APP_VERSION;
		}
	}	
	if(m_ble_boot_setting.bank_softdevice.command == BOOT_SETTING_BANK_COMMAND_NEW)
	{
		if(m_ble_boot_setting.bank_softdevice.extFlash_address > EXTFLASH_MAX_SIZE)
		{
			NRF_LOG_DEBUG("PREVALIDATE_ERROR_MASK_BANK_SOFTDEVICE_EXT_ADDRESS");
			error_mask |= PREVALIDATE_ERROR_MASK_BANK_SOFTDEVICE_EXT_ADDRESS;
		}	
		if(m_ble_boot_setting.bank_softdevice.image_size > BLE_BANK_SOFTDEVICE_DEFAULT_SIZE)
		{
			NRF_LOG_DEBUG("PREVALIDATE_ERROR_MASK_BANK_SOFTDEVICE_SIZE");
			error_mask |= PREVALIDATE_ERROR_MASK_BANK_SOFTDEVICE_SIZE;
		}	
		if(m_ble_boot_setting.bank_softdevice.version < SD_FWID_GET(MBR_SIZE))
		{
			NRF_LOG_DEBUG("PREVALIDATE_ERROR_MASK_BANK_SOFTDEVICE_VERSION");
			error_mask |= PREVALIDATE_ERROR_MASK_BANK_SOFTDEVICE_VERSION;
		}
	}	

	prevalidate.error_mask = error_mask;
	NRF_LOG_HEXDUMP_DEBUG((uint8_t *)&prevalidate, sizeof(_dev_dfu_prevalidate));

	drv_apollo2_uart_put((uint8_t *)&prevalidate, sizeof(_dev_dfu_prevalidate));

}

static uint32_t dfu_update_application(uint32_t src_addr, uint32_t src_size, uint32_t src_crc)
{
	if(m_ble_boot_setting.bank_application.command != BOOT_SETTING_BANK_COMMAND_NEW)
	{
		return NRF_SUCCESS;
	}
	
	NRF_LOG_DEBUG("dfu_update_application start...");	
	
    uint32_t const image_size  = src_size;
    uint32_t const split_size  = CODE_PAGE_SIZE; // Must be page aligned

    uint32_t ret_val     = NRF_SUCCESS;
    uint32_t target_addr = MAIN_APPLICATION_START_ADDR;
    uint32_t length_left = image_size;
    uint32_t cur_len;
    uint32_t crc;

		uint8_t transfer_buffer[CODE_PAGE_SIZE];
	
    NRF_LOG_DEBUG("Enter dfu_update_application");

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
				dev_extFlash_read(src_addr, transfer_buffer, cur_len);
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
        NRF_LOG_DEBUG("Setting app as valid");
    }
    else
    {
        NRF_LOG_ERROR("CRC computation failed for copied app: "
                      "src crc: 0x%08x, res crc: 0x%08x",
                      src_crc,
                      crc);
				return POSTVALIDATE_APPLICATION_UPDATE_CRC_ERROR;
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
	NRF_LOG_DEBUG("dfu_update_bootloader start...");	
	
	NRF_LOG_DEBUG("src_addr: 0x%08x, src_size: 0x%08x, src_crc: 0x%08x", src_addr, src_size, src_crc);
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
				dev_extFlash_read(src_addr, transfer_buffer, cur_len);
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
        NRF_LOG_DEBUG("Setting app as valid");
    }
    else
    {
        NRF_LOG_ERROR("CRC computation failed for copied app: "
                      "src crc: 0x%08x, res crc: 0x%08x",
                      src_crc,
                      crc);
				ret_val = POSTVALIDATE_BOOTLOADER_UPDATE_CRC_ERROR;
    }	
		
    return ret_val;
}


static uint32_t dfu_update_softdevice(uint32_t src_addr, uint32_t src_size, uint32_t src_crc)
{
	if(m_ble_boot_setting.bank_softdevice.command != BOOT_SETTING_BANK_COMMAND_NEW)
	{
		return NRF_SUCCESS;
	}	
	
	NRF_LOG_DEBUG("dfu_update_softdevice start...");
	
    uint32_t const image_size  = src_size;
    uint32_t const split_size  = CODE_PAGE_SIZE; // Must be page aligned	
    uint32_t ret_val     = NRF_SUCCESS;
    uint32_t target_addr = SOFTDEVICE_REGION_START;
    uint32_t length_left = image_size;
    uint32_t cur_len;
    uint32_t crc;
    NRF_LOG_DEBUG("Enter dfu_update_softdevice");
		uint8_t transfer_buffer[CODE_PAGE_SIZE];
		uint32_t softdevice_size = MAIN_APPLICATION_START_ADDR - SOFTDEVICE_REGION_START;
		// Erase the target page
		ret_val = nrf_dfu_flash_erase(target_addr, softdevice_size / CODE_PAGE_SIZE, NULL);
		APP_ERROR_CHECK(ret_val);		
	
    do
    {
        cur_len = (length_left > split_size) ? split_size : length_left;
				dev_extFlash_enable();
				dev_extFlash_read(src_addr, transfer_buffer, cur_len);
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
        NRF_LOG_DEBUG("Setting app as valid");
    }
    else
    {
        NRF_LOG_ERROR("CRC computation failed for copied app: "
                      "src crc: 0x%08x, res crc: 0x%08x",
                      src_crc,
                      crc);
				return POSTVALIDATE_SOFTDEVICE_UPDATE_CRC_ERROR;
    }		


    return ret_val;
}
static void postvalidate(void)
{
	NRF_LOG_DEBUG("postvalidate");
	
/*
	Memory Layout:
	Bootloader start address : 0x6E000, size : 0x10000.
	App Data start address : 0x6B000, size : 0x3000(DFU_APP_DATA_RESERVED).
	Bootloader update buffer start address : 0x5B000, size : 0x10000.
	Application start address : 0x23000, size : 0x38000.
*/	

	_dev_dfu_postvalidate postvalidate_resp;
	postvalidate_resp.command = BLE_UART_RESPONSE_BOOTLOADER_POSTVALIDATE | BLE_UART_CCOMMAND_MASK_BOOTLOADER;	

	postvalidate_resp.error_mask = dfu_update_application(m_ble_boot_setting.bank_application.extFlash_address, 
																												m_ble_boot_setting.bank_application.image_size, 
																												m_ble_boot_setting.bank_application.image_crc);
	if(postvalidate_resp.error_mask != NRF_SUCCESS)
	{
		NRF_LOG_DEBUG("bank_application POSTVALIDATE_APPLICATION_UPDATE_CRC_ERROR");		
		goto EXIT;		
	}

	postvalidate_resp.error_mask |= dfu_update_softdevice(m_ble_boot_setting.bank_softdevice.extFlash_address, 
																												m_ble_boot_setting.bank_softdevice.image_size, 
																												m_ble_boot_setting.bank_softdevice.image_crc);
	if(postvalidate_resp.error_mask != NRF_SUCCESS)
	{
		NRF_LOG_DEBUG("bank_application POSTVALIDATE_APPLICATION_UPDATE_CRC_ERROR");		
		goto EXIT;		
	}
	
	postvalidate_resp.error_mask |= dfu_update_bootloader(m_ble_boot_setting.bank_bootloader.extFlash_address, 
																												m_ble_boot_setting.bank_bootloader.image_size, 
																												m_ble_boot_setting.bank_bootloader.image_crc);
	
	
	EXIT:
	drv_apollo2_uart_put((uint8_t *)&postvalidate_resp, sizeof(_dev_dfu_postvalidate));
	
}

static void reset_n_activate(void)
{
	uint32_t ret_val = NRF_SUCCESS;
	uint32_t command[1] = {BLE_UART_RESPONSE_BOOTLOADER_RESET_N_ACTIVATE | BLE_UART_CCOMMAND_MASK_BOOTLOADER};
	drv_apollo2_uart_put((uint8_t *)&command, sizeof(command));	
	DELAY_MS(1000);

	
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
			
			s_dfu_settings.app_version = 0;
			s_dfu_settings.bootloader_version = 0;
			ret_val = nrf_dfu_settings_write(NULL);	
			APP_ERROR_CHECK(ret_val);				
			nrf_power_gpregret_set(CMD_GPREGRET_BOOTLOADER_EXCEPTION_RESET);
			nrf_delay_ms(100);
			NVIC_SystemReset();
			
		}
			
	}

	
	ret_val = nrf_dfu_settings_write(NULL);	
	APP_ERROR_CHECK(ret_val);	
	
	nrf_delay_ms(100);
	NVIC_SystemReset();


}

static void sched_evt(void * p_event_data, uint16_t event_size)
{
	NRF_LOG_DEBUG("sched_evt");
	uint32_t event = *(uint32_t *)p_event_data; 

	if(event == APP_UART_DATA_READY)
	{
		NRF_LOG_HEXDUMP_DEBUG((uint8_t *)m_buffer,64);
		switch(m_buffer[0]&0x0000FF00)
		{
			case BLE_UART_CCOMMAND_BOOTLOADER_BLE_INFO:{
				NRF_LOG_DEBUG("BLE_UART_CCOMMAND_BOOTLOADER_BLE_INFO");
				
				
				_dev_apollo2_ble_info ble_info;
				dev_apollo2_ble_info_get(&ble_info);
				drv_apollo2_uart_put((uint8_t *)&ble_info, sizeof(_dev_apollo2_ble_info));

				
			}break;
			case BLE_UART_CCOMMAND_BOOTLOADER_JUMP_BOOTLOADER:{
				NRF_LOG_DEBUG("BLE_UART_CCOMMAND_BOOTLOADER_JUMP_BOOTLOADER");
				jump_bootloader();
				drv_apollo2_uart_close();	
			}break;		
			case BLE_UART_CCOMMAND_BOOTLOADER_JUMP_APPLICATION:{
				NRF_LOG_DEBUG("BLE_UART_CCOMMAND_BOOTLOADER_JUMP_APPLICATION");
				jump_application();
				drv_apollo2_uart_close();	
			}break;	
			case BLE_UART_CCOMMAND_BOOTLOADER_RESET:{
				NRF_LOG_DEBUG("BLE_UART_CCOMMAND_BOOTLOADER_RESET");
				reset();
				nrf_delay_ms(1000);
				NVIC_SystemReset();
				NRF_LOG_DEBUG("BLE_UART_CCOMMAND_BOOTLOADER_RESET");
			}break;				
			case BLE_UART_CCOMMAND_BOOTLOADER_BOOTSETTING:{
				NRF_LOG_DEBUG("BLE_UART_CCOMMAND_BOOTLOADER_BOOTSETTING");
				bootsetting();
			}break;			
			case BLE_UART_CCOMMAND_BOOTLOADER_PREVALIDATE:{
				NRF_LOG_DEBUG("BLE_UART_CCOMMAND_BOOTLOADER_PREVALIDATE");
				prevalidate();
			}break;					
			case BLE_UART_CCOMMAND_BOOTLOADER_IMAGE_DATA:{
				NRF_LOG_DEBUG("BLE_UART_CCOMMAND_BOOTLOADER_IMAGE_DATA");
			}break;					
			case BLE_UART_CCOMMAND_BOOTLOADER_IMAGE_WRITE:{
				NRF_LOG_DEBUG("BLE_UART_CCOMMAND_BOOTLOADER_IMAGE_WRITE");
			}break;		
			case BLE_UART_CCOMMAND_BOOTLOADER_POSTVALIDATE:{
				NRF_LOG_DEBUG("BLE_UART_CCOMMAND_BOOTLOADER_POSTVALIDATE");
				postvalidate();
			}break;		
			case BLE_UART_CCOMMAND_BOOTLOADER_RESET_N_ACTIVATE:{
				NRF_LOG_DEBUG("BLE_UART_CCOMMAND_BOOTLOADER_RESET_N_ACTIVATE");
				reset_n_activate();
			}break;				

			default:{
				NRF_LOG_DEBUG("COMMAND BLE_UART_CCOMMAND_BOOTLOADER_UNSPPORT");
				drv_apollo2_uart_close();	
			}break;			
		}	
	
	}
	else if(event == APP_UART_CTS_INTERRUPT)
	{
		if (nrf_drv_gpiote_in_is_set(APOLLO2_CTS_PIN_NUMBER))
		{
				drv_apollo2_on_cts_high();
		}
		else
		{
				drv_apollo2_on_cts_low();
		}		
	}

}





static void event_handle(uint32_t evt, uint8_t *p_data, uint32_t length)
{
	NRF_LOG_DEBUG("event_handle");
	ret_code_t err_code;
	uint32_t event;
	if(evt == APP_UART_DATA_READY || evt == APP_UART_TIMEOUT)
	{
		NRF_LOG_DEBUG("APP_UART_DATA_READY || APP_UART_TIMEOUT");
		m_buffer = (uint32_t *)p_data;
		event = APP_UART_DATA_READY;
		err_code = app_sched_event_put(&event ,sizeof(uint32_t),sched_evt);
		APP_ERROR_CHECK(err_code);	
	
	}
	else if(evt == APP_UART_TX_EMPTY)
	{
		NRF_LOG_DEBUG("APP_UART_TX_EMPTY");
	}
	else if(evt == APP_UART_COMMUNICATION_ERROR)
	{
		NRF_LOG_DEBUG("APP_UART_COMMUNICATION_ERROR");
	}
	else if(evt == APP_UART_FIFO_ERROR)
	{
		NRF_LOG_DEBUG("APP_UART_FIFO_ERROR");
	}	
	else if(evt == APP_UART_CTS_INTERRUPT)
	{
		
		NRF_LOG_DEBUG("APP_UART_CTS_INTERRUPT");
		
		event = APP_UART_CTS_INTERRUPT;
		err_code = app_sched_event_put(&event ,sizeof(uint32_t),sched_evt);
		APP_ERROR_CHECK(err_code);			
		
		

		
	}		
}







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


static void timer_handle(void * p_context)
{
	UNUSED_PARAMETER(p_context);

	NRF_LOG_INFO("DFU reset due to inactivity timeout.");

	NVIC_SystemReset();
}
static void exception_timer_start(void)
{
//	ret_code_t err_code;
//	err_code = app_timer_start(m_exception_timer_id,
//														APP_TIMER_TICKS(SERIAL_DFU_EXCEPTION_TIMEOUT_MS),
//														NULL);	
//	APP_ERROR_CHECK(err_code);
}

static void exception_timer_stop(void)
{
//	ret_code_t err_code;
//	err_code = app_timer_stop(m_exception_timer_id);	
//	APP_ERROR_CHECK(err_code);
}

static void wdt_timer_handle(void * p_context)
{
	UNUSED_PARAMETER(p_context);

	NRF_LOG_INFO("wdt_timer_handle");

	NRF_WDT->RR[0] = WDT_RR_RR_Reload;
}


static void timers_init(void)
{
	NRF_LOG_INFO("timers_init");
	ret_code_t err_code;
	err_code = app_timer_init();
	APP_ERROR_CHECK(err_code);
	
	
	

//	// Start a single shot timer that will reset the DFU on timeout.
//	err_code = app_timer_create(&m_exception_timer_id,
//															APP_TIMER_MODE_SINGLE_SHOT,
//															timer_handle);
//	APP_ERROR_CHECK(err_code);	
	
	
	// Start a repeat timer to feed watchdog because application start a 70seconds wdt.
	err_code = app_timer_create(&m_wdt_timer_id,
															APP_TIMER_MODE_REPEATED,
															wdt_timer_handle);
	APP_ERROR_CHECK(err_code);		
	err_code = app_timer_start(m_wdt_timer_id,
														APP_TIMER_TICKS(60000),
														NULL);	
	APP_ERROR_CHECK(err_code);	
}





static void scheduler_init(void)
{
	NRF_LOG_INFO("scheduler_init");
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
	nrf_gpio_cfg_default(EXTFLASH_SPI_SS_PIN);
	nrf_gpio_cfg_default(EXTFLASH_SPI_SCK_PIN);
	nrf_gpio_cfg_default(EXTFLASH_SPI_MOSI_PIN);
	nrf_gpio_cfg_default(EXTFLASH_SPI_MISO_PIN);	
}

void dev_dfu_serial_init(uint8_t gpregret_val)
{
	NRF_LOG_INFO("dev_dfu_serial_init");


	nrf_dfu_settings_init(false);
	
	NRF_LOG_HEXDUMP_INFO((uint8_t *)&s_dfu_settings, sizeof(nrf_dfu_settings_t)) ;
	
	
	clock_init();
	timers_init();
	device_init();
	scheduler_init();

	drv_apollo2_init(event_handle);


	//如果是CMD_GPREGRET_BOOTLOADER_EXCEPTION_RESET异常进入BOOTLOADER，就不向APOLLO发进入BOOTLOADER命令了
	if(gpregret_val == CMD_GPREGRET_BOOTLOADER_PC_DFU)
	{
		uint32_t command[3] = {BLE_UART_RESPONSE_APPLICATION_JUMP_BOOTLOADER | BLE_UART_CCOMMAND_MASK_BOOTLOADER};
		command[1] = (YEAR<<16) | (MONTH<<8) | (DAY);
		command[2] = (HOUR<<16) | (MINUTE<<8) | (SECOND);
		_uart_data_param param;
		param.user_callback = NULL;
		param.p_data = (uint8_t *)command;
		param.length = sizeof(command);	
		uint8_t count = TASK_UART_DELAY_COUNT_OUT;
		while(drv_ble_send_data(&param) != NRF_SUCCESS && count--)
		{
			DELAY_MS(TASK_UART_DELAY);
		}			
	
	}

	

	exception_timer_start();
	
	wait_for_event();			


}






