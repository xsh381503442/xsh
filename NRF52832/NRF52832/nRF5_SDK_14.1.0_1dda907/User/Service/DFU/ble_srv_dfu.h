#ifndef __BLE_SRV_DFU_H__
#define __BLE_SRV_DFU_H__

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "sdk_config.h"
#include "ble_srv_common.h"
#include "nrf_sdh_ble.h"




#ifndef BLE_JW_DFU_OBSERVER_PRIO
#define BLE_JW_DFU_OBSERVER_PRIO 2
#endif



#define BLE_JW_DFU_DEF(_name)                                                                          \
ble_jw_dfu_t _name;                                                                             \
NRF_SDH_BLE_OBSERVER(_name ## _obs,                                                                    \
                     BLE_JW_DFU_OBSERVER_PRIO,                                                     \
                     ble_jw_dfu_on_ble_evt, &_name)

#define BLE_UUID_JW_DFU_SERVICE 0x0001                      
#define BLE_JW_DFU_MAX_DATA_LEN (23 - 3)  


typedef enum
{
		BLE_JW_DFU_EVT_TX_RDY,
		BLE_JW_DFU_EVT_HVC,
		BLE_JW_DFU_EVT_CONFIG_DATA,
		BLE_JW_DFU_EVT_CONFIG_NOTIFY_ENABLE,
		BLE_JW_DFU_EVT_CONFIG_NOTIFY_DISABLE,
    BLE_JW_DFU_EVT_CONTROL_DATA,
		BLE_JW_DFU_EVT_CONTROL_NOTIFY_ENABLE,
		BLE_JW_DFU_EVT_CONTROL_NOTIFY_DISABLE,
    BLE_JW_DFU_EVT_FLASH_DATA,
		BLE_JW_DFU_EVT_FLASH_NOTIFY_ENABLE,
		BLE_JW_DFU_EVT_FLASH_NOTIFY_DISABLE,	
}ble_jw_dfu_evt_type_t;




typedef struct ble_jw_dfu_s     ble_jw_dfu_t;

typedef struct
{
    uint8_t const * p_data;           
    uint16_t        length;          
} ble_jw_dfu_evt_data_t;
typedef struct
{
    ble_jw_dfu_evt_type_t type;           
    ble_jw_dfu_t * p_jw_dfu;                 
    union
    {
        ble_jw_dfu_evt_data_t data; 
    } params;
} ble_jw_dfu_evt_t;

typedef void (*ble_jw_dfu_evt_handler_t) (ble_jw_dfu_evt_t * p_evt);

typedef struct
{

    ble_jw_dfu_evt_handler_t   evt_handler; /**< Event handler to be called for handling received data. */
} ble_jw_dfu_init_t;


struct ble_jw_dfu_s
{
    uint8_t                     uuid_type;                    
    uint16_t                    service_handle;           
    ble_gatts_char_handles_t    config_handles;   
		ble_gatts_char_handles_t    control_handles;  
		ble_gatts_char_handles_t    flash_handles; 
    uint16_t                    conn_handle;                
    bool                        is_config_notification_enabled;
		bool                        is_control_notification_enabled;
		bool                        is_flash_notification_enabled;
		ble_jw_dfu_evt_handler_t    evt_handler;     
};


uint32_t ble_jw_dfu_init(ble_jw_dfu_t * p_jw_dfu, const ble_jw_dfu_init_t * p_jw_dfu_init);
void ble_jw_dfu_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context);





#endif //__BLE_SRV_DFU_H__
