#include "app_srv_dfu.h"
#include "ble_srv_dfu.h"



#define LOG_LEVEL_OFF 							0
#define LOG_LEVEL_ERROR 						1
#define LOG_LEVEL_WARNING 					2
#define LOG_LEVEL_INFO 							3
#define LOG_LEVEL_DEBUG 						4
#define NRF_LOG_MODULE_NAME app_srv_dfu
#define NRF_LOG_LEVEL       LOG_LEVEL_DEBUG
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
NRF_LOG_MODULE_REGISTER();


BLE_JW_DFU_DEF(m_jw_dfu);


static void jw_dfu_evt_handler (ble_jw_dfu_evt_t *evt)
{
    switch (evt->type)
    {
        case BLE_JW_DFU_EVT_CONFIG_DATA:{
					NRF_LOG_DEBUG("BLE_JW_DFU_EVT_CONFIG_DATA");
					
				}break;
        case BLE_JW_DFU_EVT_CONFIG_NOTIFY_ENABLE:{
					NRF_LOG_DEBUG("BLE_JW_DFU_EVT_CONFIG_NOTIFY_ENABLE");

				}break;
        case BLE_JW_DFU_EVT_CONFIG_NOTIFY_DISABLE:{
					NRF_LOG_DEBUG("BLE_JW_DFU_EVT_CONFIG_NOTIFY_DISABLE");
  
				}break;				
	     case BLE_JW_DFU_EVT_CONTROL_DATA:{
					NRF_LOG_DEBUG("BLE_JW_DFU_EVT_CONTROL_DATA");
				 
				}break;			
        case BLE_JW_DFU_EVT_CONTROL_NOTIFY_ENABLE:{
					NRF_LOG_DEBUG("BLE_JW_DFU_EVT_CONTROL_NOTIFY_ENABLE");

				}break;
        case BLE_JW_DFU_EVT_CONTROL_NOTIFY_DISABLE:{
					NRF_LOG_DEBUG("BLE_JW_DFU_EVT_CONTROL_NOTIFY_DISABLE");
  
				}break;			


	     case BLE_JW_DFU_EVT_FLASH_DATA:{
					NRF_LOG_DEBUG("BLE_JW_DFU_EVT_FLASH_DATA");
				 
				}break;			
        case BLE_JW_DFU_EVT_FLASH_NOTIFY_ENABLE:{
					NRF_LOG_DEBUG("BLE_JW_DFU_EVT_FLASH_NOTIFY_ENABLE");

				}break;
        case BLE_JW_DFU_EVT_FLASH_NOTIFY_DISABLE:{
					NRF_LOG_DEBUG("BLE_JW_DFU_EVT_FLASH_NOTIFY_DISABLE");
  
				}break;	

				
        case BLE_JW_DFU_EVT_TX_RDY:{
					NRF_LOG_DEBUG("BLE_NRF52_EVT_TX_RDY");
				}break;
        case BLE_JW_DFU_EVT_HVC:{
					NRF_LOG_DEBUG("BLE_NRF52_EVT_HVC");

				}break;
						
				default:
					break;
				
    }


}


uint32_t app_srv_dfu_init(void)
{
    ble_jw_dfu_init_t params;
    uint32_t err_code;

    params.evt_handler = jw_dfu_evt_handler;
    err_code = ble_jw_dfu_init(&m_jw_dfu, &params);
    APP_ERROR_CHECK(err_code);


    return NRF_SUCCESS;
}



