#ifndef __BLE_SRV_APOLLO_H__
#define __BLE_SRV_APOLLO_H__

#include <stdint.h>
#include <stdbool.h>
#include "sdk_config.h"
#include "ble.h"
#include "ble_srv_common.h"
#include "nrf_sdh_ble.h"

#ifndef BLE_APOLLO_OBSERVER_PRIO
#define BLE_APOLLO_OBSERVER_PRIO 2
#endif

#define BLE_APOLLO_DEF(_name)                                                                          \
ble_apollo_t _name;                                                                             \
NRF_SDH_BLE_OBSERVER(_name ## _obs,                                                                 \
                     BLE_APOLLO_OBSERVER_PRIO,                                                     \
                     ble_apollo_on_ble_evt, &_name)

#define BLE_UUID_APOLLO_SERVICE 0x0001                      


#define OPCODE_LENGTH 1
#define HANDLE_LENGTH 2

/**@brief   Maximum length of data (in bytes) that can be transmitted to the peer by the Nordic UART service module. */
#if defined(NRF_SDH_BLE_GATT_MAX_MTU_SIZE) && (NRF_SDH_BLE_GATT_MAX_MTU_SIZE != 0)
    #define BLE_APOLLO_MAX_DATA_LEN (NRF_SDH_BLE_GATT_MAX_MTU_SIZE - OPCODE_LENGTH - HANDLE_LENGTH)
#else
    #define BLE_APOLLO_MAX_DATA_LEN (BLE_GATT_MTU_SIZE_DEFAULT - OPCODE_LENGTH - HANDLE_LENGTH)
    #warning NRF_SDH_BLE_GATT_MAX_MTU_SIZE is not defined.
#endif



typedef enum
{
    BLE_EVT_APOLLO_UART_RX_DATA,           
    BLE_EVT_APOLLO_TX_RDY,            
    BLE_EVT_APOLLO_UART_NOTIFY_ENABLE,      
    BLE_EVT_APOLLO_UART_NOTIFY_DISABLE,   
	BLE_EVT_APOLLO_CONNECTED,
	BLE_EVT_APOLLO_DISCONNECTED,
    BLE_EVT_APOLLO_FLASH_DATA,
	BLE_EVT_APOLLO_FLASH_NOTIFY_ENABLE,
	BLE_EVT_APOLLO_FLASH_NOTIFY_DISABLE,
	BLE_EVT_APOLLO_SENSOR_DATA,
	BLE_EVT_APOLLO_SENSOR_NOTIFY_ENABLE,
	BLE_EVT_APOLLO_SENSOR_NOTIFY_DISABLE,	
} ble_apollo_evt_type_t;

typedef struct ble_apollo_s ble_apollo_t;

typedef struct
{
    uint8_t const * p_data;           
    uint16_t        length;          
} ble_apollo_evt_data_t;

typedef struct
{
    ble_apollo_evt_type_t type;           
    ble_apollo_t * p_apollo;                 
    union
    {
        ble_apollo_evt_data_t data; 
    } params;
} ble_apollo_evt_t;

typedef void (*ble_apollo_data_handler_t) (ble_apollo_evt_t * p_evt);

typedef struct
{
    ble_apollo_data_handler_t data_handler; 
} ble_apollo_init_t;

struct ble_apollo_s
{
    uint8_t                     uuid_type;               
    uint16_t                    service_handle;               
    ble_gatts_char_handles_t    uart_handles;
	ble_gatts_char_handles_t    flash_handles;	
	ble_gatts_char_handles_t    sensor_handles;		
    uint16_t                    conn_handle;             
    bool                        is_uart_notification_enabled; 
	bool                        is_flash_notification_enabled; 
	bool                        is_sensor_notification_enabled; 
    ble_apollo_data_handler_t   data_handler;            
};

uint32_t ble_apollo_init(ble_apollo_t * p_apollo, ble_apollo_init_t const * p_apollo_init);
void ble_apollo_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context);
uint32_t ble_apollo_uart_string_send(ble_apollo_t * p_apollo, uint8_t * p_string, uint16_t * p_length);
uint32_t ble_apollo_flash_string_send(ble_apollo_t * p_apollo, uint8_t * p_string, uint16_t * p_length);
uint32_t ble_apollo_sensor_string_send(ble_apollo_t * p_apollo, uint8_t * p_string, uint16_t * p_length);





#endif //__BLE_SRV_APOLLO_H__
