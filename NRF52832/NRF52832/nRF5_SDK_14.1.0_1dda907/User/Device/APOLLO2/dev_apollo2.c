#include "dev_apollo2.h"
#include "cmd_apollo2.h"
#include "nrf_dfu_types.h"
#include "drv_apollo2.h"
#include "nrf_fstorage.h"
#include "nrf_fstorage_sd.h"
#include "nrf_fstorage_nvmc.h"
#include "crc32.h"


#define LOG_LEVEL_OFF 							0
#define LOG_LEVEL_ERROR 						1
#define LOG_LEVEL_WARNING 					2
#define LOG_LEVEL_INFO 							3
#define LOG_LEVEL_DEBUG 						4
#define NRF_LOG_MODULE_NAME dev_apollo2
#define NRF_LOG_LEVEL       LOG_LEVEL_OFF
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
NRF_LOG_MODULE_REGISTER();



extern uint8_t	g_adv_manuf_mac[BLE_GAP_ADDR_LEN];


#define DFU_SETTINGS_INIT_COMMAND_OFFSET        offsetof(nrf_dfu_settings_t, init_command)          //<! Offset in the settings struct where the InitCommand is located.


static void fstorage_evt_handler(nrf_fstorage_evt_t * p_evt);


NRF_FSTORAGE_DEF(nrf_fstorage_t dev_apoollo2_fstorage) =
{
    .evt_handler    = fstorage_evt_handler,
    .start_addr     = MBR_SIZE,
    .end_addr       = BOOTLOADER_SETTINGS_ADDRESS + CODE_PAGE_SIZE
};


extern ble_config_params_t     *g_ble_config;
static uint32_t volatile m_flash_operations_pending;

static void fstorage_evt_handler(nrf_fstorage_evt_t * p_evt)
{
    if (m_flash_operations_pending > 0)
    {
        m_flash_operations_pending--;
    }

    if (p_evt->result == NRF_SUCCESS)
    {
        NRF_LOG_DEBUG("Flash %s success: addr=%p, pending %d",
                      (p_evt->id == NRF_FSTORAGE_EVT_WRITE_RESULT) ? "write" : "erase",
                      p_evt->addr, m_flash_operations_pending);
    }
    else
    {
        NRF_LOG_DEBUG("Flash %s failed (0x%x): addr=%p, len=0x%x bytes, pending %d",
                      (p_evt->id == NRF_FSTORAGE_EVT_WRITE_RESULT) ? "write" : "erase",
                      p_evt->result, p_evt->addr, p_evt->len, m_flash_operations_pending);
    }

    switch (p_evt->id)
    {
        case NRF_FSTORAGE_EVT_WRITE_RESULT:
        {
            NRF_LOG_INFO("--> Event received: wrote %d bytes at address 0x%x.",
                         p_evt->len, p_evt->addr);
        } break;

        case NRF_FSTORAGE_EVT_ERASE_RESULT:
        {
            NRF_LOG_INFO("--> Event received: erased %d page from address 0x%x.",
                         p_evt->len, p_evt->addr);
        } break;

        default:
            break;
    }
}
#define YEAR ((((__DATE__ [7] - '0') * 10 + (__DATE__ [8] - '0')) * 10  + (__DATE__ [9] - '0')) * 10 + (__DATE__ [10] - '0'))  
  
#define MONTH (__DATE__ [2] == 'n' ? (__DATE__ [1] == 'a' ? 0 : 5) : __DATE__ [2] == 'b' ? 1  : __DATE__ [2] == 'r' ? (__DATE__ [0] == 'M' ? 2 : 3)  : __DATE__ [2] == 'y' ? 4  : __DATE__ [2] == 'n' ? 5  : __DATE__ [2] == 'l' ? 6 : __DATE__ [2] == 'g' ? 7 : __DATE__ [2] == 'p' ? 8 : __DATE__ [2] == 't' ? 9 : __DATE__ [2] == 'v' ? 10 : 11)  
  
#define DAY ((__DATE__ [4] == ' ' ? 0 : __DATE__ [4] - '0') * 10+ (__DATE__ [5] - '0'))  
  
#define DATE_AS_INT (((YEAR - 2000) * 12 + MONTH) * 31 + DAY)  

#define HOUR ((__TIME__ [0] == ' ' ? 0 : __TIME__ [0] - '0') * 10+ (__TIME__ [1] - '0')) 
#define MINUTE ((__TIME__ [3] == ' ' ? 0 : __TIME__ [3] - '0') * 10+ (__TIME__ [4] - '0'))
#define SECOND ((__TIME__ [6] == ' ' ? 0 : __TIME__ [6] - '0') * 10+ (__TIME__ [7] - '0'))

void dev_apollo2_ble_info_get(_dev_apollo2_ble_info *ble_info)
{
	NRF_LOG_DEBUG("dev_apollo2_ble_info_get");
	nrf_dfu_settings_t s_dfu_settings;	
	memcpy((void*)&s_dfu_settings, (uint8_t *)BOOTLOADER_SETTINGS_ADDRESS, sizeof(nrf_dfu_settings_t));
	ble_info->command = BLE_UART_RESPONSE_APPLICATION_BLE_INFO | BLE_UART_CCOMMAND_MASK_APPLICATION;
	ble_info->setting_version = s_dfu_settings.settings_version;
	ble_info->bootloader_version = s_dfu_settings.bootloader_version;
	ble_info->application_version = s_dfu_settings.app_version;
	ble_info->softdevice_version = SD_FWID_GET(MBR_SIZE); 
	ble_info->pcb_version = NRF_UICR->CUSTOMER[0];
//	ble_info->mac_address_f4b = NRF_FICR->DEVICEADDR[0];
//	ble_info->mac_address_l2b = NRF_FICR->DEVICEADDR[1];
	ble_info->mac_address_f4b = g_adv_manuf_mac[2]<<24|g_adv_manuf_mac[3]<<16|g_adv_manuf_mac[4]<<8|g_adv_manuf_mac[5];	
	ble_info->mac_address_l2b = g_adv_manuf_mac[0]<<8|g_adv_manuf_mac[1];	
	NRF_LOG_DEBUG("mac_address_f4b --> 0x%08X",ble_info->mac_address_f4b);
	NRF_LOG_DEBUG("mac_address_l2b --> 0x%08X",ble_info->mac_address_l2b);
	memcpy((uint8_t *)&ble_info->dev_name,(uint8_t *)&g_ble_config->dev_name,sizeof(ble_config_dev_name_t));
	
	
	ble_info->build_date = (YEAR<<16) | (MONTH<<8) | (DAY<<0);
	ble_info->build_time = (HOUR<<16) | (MINUTE<<8) | (SECOND<<0);
	
}



/*
Note:
      This function can only be called when softdevice is not enabled.

Example:
			_dev_apollo2_ble_info ble_info;
			dev_apollo2_ble_info_get(&ble_info);
			ble_info.bootloader_version = 0x01020304;
			dev_apollo2_ble_info_set(&ble_info);
*/
void dev_apollo2_ble_info_set(_dev_apollo2_ble_info *ble_info)
{
	NRF_LOG_DEBUG("dev_apollo2_ble_info_set");

	
	nrf_fstorage_api_t * p_api_impl;
	
#ifdef SOFTDEVICE_PRESENT
    NRF_LOG_INFO("SoftDevice is present.");
    NRF_LOG_INFO("Initializing nrf_fstorage_sd implementation...");
    /* Initialize an fstorage instance using the nrf_fstorage_sd backend.
     * nrf_fstorage_sd uses the SoftDevice to write to flash. This implementation can safely be
     * used whenever there is a SoftDevice, regardless of its status (enabled/disabled). */
    p_api_impl = &nrf_fstorage_sd;
#else
    NRF_LOG_INFO("SoftDevice not present.");
    NRF_LOG_INFO("Initializing nrf_fstorage_nvmc implementation...");
    /* Initialize an fstorage instance using the nrf_fstorage_nvmc backend.
     * nrf_fstorage_nvmc uses the NVMC peripheral. This implementation can be used when the
     * SoftDevice is disabled or not present.
     *
     * Using this implementation when the SoftDevice is enabled results in a hardfault. */
    p_api_impl = &nrf_fstorage_nvmc;
#endif	
	
	uint32_t err_code;
	err_code = nrf_fstorage_init(&dev_apoollo2_fstorage, p_api_impl, NULL);
	APP_ERROR_CHECK(err_code);	
	
	nrf_dfu_settings_t s_dfu_settings;	
	memcpy((void*)&s_dfu_settings, (uint8_t *)BOOTLOADER_SETTINGS_ADDRESS, sizeof(nrf_dfu_settings_t));
	s_dfu_settings.settings_version = ble_info->setting_version;
	s_dfu_settings.bootloader_version = ble_info->bootloader_version;
	s_dfu_settings.app_version = ble_info->application_version;		
	s_dfu_settings.crc = crc32_compute((uint8_t*)&s_dfu_settings + 4, DFU_SETTINGS_INIT_COMMAND_OFFSET  - 4, NULL);
	
	m_flash_operations_pending++;
	uint32_t             page_addr = BOOTLOADER_SETTINGS_ADDRESS;
	uint32_t             num_pages = 1;
	NRF_LOG_DEBUG("nrf_fstorage_erase(addr=0x%p, len=%d pages), queue usage: %d",
								page_addr, num_pages, m_flash_operations_pending);	
	err_code = nrf_fstorage_erase(&dev_apoollo2_fstorage, page_addr, num_pages, NULL);
	APP_ERROR_CHECK(err_code);	
	static nrf_dfu_settings_t temp_dfu_settings;
	memcpy(&temp_dfu_settings, &s_dfu_settings, sizeof(nrf_dfu_settings_t));
	uint32_t             dest = BOOTLOADER_SETTINGS_ADDRESS;
	uint32_t             len = sizeof(nrf_dfu_settings_t);	
	m_flash_operations_pending++;
	NRF_LOG_DEBUG("nrf_fstorage_write(addr=%p, len=0x%x bytes), queue usage: %d",
								dest, len, m_flash_operations_pending);
	err_code = nrf_fstorage_write(&dev_apoollo2_fstorage, dest, &temp_dfu_settings, len, NULL);
	APP_ERROR_CHECK(err_code);
	
	
	
}





