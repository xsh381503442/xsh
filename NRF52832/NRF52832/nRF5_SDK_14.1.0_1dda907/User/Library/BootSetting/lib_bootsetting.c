#include "lib_bootsetting.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"


extern uint8_t m_dfu_settings_buffer[CODE_PAGE_SIZE];



void lib_bootsetting_read(nrf_dfu_settings_t *boot_setting)
{
	NRF_LOG_INFO("lib_bootsetting_read");
	ret_code_t rc = nrf_dfu_flash_init(false);
	if (rc != NRF_SUCCESS)
	{
			NRF_LOG_ERROR("nrf_dfu_flash_init() failed with error: %x", rc);
			APP_ERROR_HANDLER(rc);
	}

	// Copy the DFU settings out of flash and into a buffer in RAM.
	memcpy((void*)&boot_setting, m_dfu_settings_buffer, sizeof(nrf_dfu_settings_t));

	NRF_LOG_HEXDUMP_INFO((uint8_t *)boot_setting, sizeof(nrf_dfu_settings_t));	
}












