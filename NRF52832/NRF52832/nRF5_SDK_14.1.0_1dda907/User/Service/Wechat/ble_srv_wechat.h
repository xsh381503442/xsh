#ifndef __BLE_SRV_WECHAT_H__
#define __BLE_SRV_WECHAT_H__




#include <stdint.h>
#include <stdbool.h>
#include "sdk_config.h"
#include "ble_srv_common.h"
#include "nrf_sdh_ble.h"



#ifndef BLE_WECHAT_OBSERVER_PRIO
#define BLE_WECHAT_OBSERVER_PRIO 2
#endif

#define BLE_WECHAT_DEF(_name)                                                                          \
ble_wechat_t _name;                                                                             \
NRF_SDH_BLE_OBSERVER(_name ## _obs,                                                                    \
                     BLE_WECHAT_OBSERVER_PRIO,                                                     \
                     ble_wechat_on_ble_evt, &_name)
                     
#define BLE_WECHAT_MAX_DATA_LEN (NRF_SDH_BLE_GATT_MAX_MTU_DEFAULT_SIZE - 3)  



#define BLE_UUID_WECHAT_SERVICE							 															0xFEE7
#define BLE_UUID_WECHAT_WRITE_CHARACTERISTICS 														0xFEC7
#define BLE_UUID_WECHAT_INDICATE_CHARACTERISTICS 													0xFEC8
#define BLE_UUID_WECHAT_READ_CHARACTERISTICS 															0xFEC9
#define BLE_UUID_WECHAT_CURRENT_PEDOMETER_MEASUREMENT_CHARACTERISTICS 		0xFEA1
#define BLE_UUID_WECHAT_TARGET_CHARACTERISTICS 														0xFEA2




typedef struct ble_wechat_s ble_wechat_t;


typedef enum
{
    BLE_WECHAT_EVT_RX_DATA,          
    BLE_WECHAT_EVT_PEDO_MEAS_NOTIFY_ENABLE,     
    BLE_WECHAT_EVT_PEDO_MEAS_NOTIFY_DISABLE,  
    BLE_WECHAT_EVT_TARGET_INDICATE_ENABLE,     
    BLE_WECHAT_EVT_TARGET_INDICATE_DISABLE,
		BLE_WECHAT_EVT_TARGET_SET,
		BLE_WECHAT_EVT_DISCONNECT,
		BLE_WECHAT_EVT_READ_DATA,
} ble_wechat_evt_type_t;
typedef struct ble_wechat_s ble_wechat_t;
typedef struct
{
    uint8_t const * p_data;           
    uint16_t        length;           
} ble_wechat_evt_rx_data_t;

typedef struct
{
    ble_wechat_evt_type_t type;           
    ble_wechat_t * p_wechat;                 
    union
    {
        ble_wechat_evt_rx_data_t rx_data; 
    } params;
} ble_wechat_evt_t;
typedef void (*ble_wechat_data_handler_t) (ble_wechat_evt_t * p_evt);
typedef struct
{
    ble_wechat_data_handler_t data_handler; 
} ble_wechat_init_t;
struct ble_wechat_s
{             
    uint16_t                 service_handle;          
    ble_gatts_char_handles_t cpm_handles;              
    ble_gatts_char_handles_t target_handles; 
		ble_gatts_char_handles_t read_handles;	
    uint16_t                 conn_handle;             
    bool                     is_notification_enabled; 
		bool                     is_indicate_enabled; 
    ble_wechat_data_handler_t   data_handler;            
};

uint32_t ble_wechat_init(ble_wechat_t * p_wechat, ble_wechat_init_t const * p_wechat_init);
void ble_wechat_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context);
uint32_t ble_wechat_pedo_meas_send(ble_wechat_t * p_wechat, uint8_t * p_string, uint16_t * p_length);
uint32_t ble_wechat_target_send(ble_wechat_t * p_wechat, uint8_t * p_string, uint16_t * p_length);

#endif //__BLE_SRV_WECHAT_H__
