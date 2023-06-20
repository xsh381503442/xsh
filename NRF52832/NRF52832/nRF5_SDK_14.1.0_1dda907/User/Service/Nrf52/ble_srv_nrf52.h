#ifndef __BLE_SRV_NRF52_H__
#define __BLE_SRV_NRF52_H__



#include <stdint.h>
#include <stdbool.h>
#include "sdk_config.h"
#include "ble_srv_common.h"
#include "nrf_sdh_ble.h"



#define MIN_CONN_INTERVAL_FAST               MSEC_TO_UNITS(15, UNIT_1_25_MS)            /**< Minimum acceptable connection interval (0.1 seconds). */
#define MAX_CONN_INTERVAL_FAST               MSEC_TO_UNITS(30, UNIT_1_25_MS)            /**< Maximum acceptable connection interval (0.2 second). */
#define SLAVE_LATENCY_FAST                   0                                           /**< Slave latency. */
#define CONN_SUP_TIMEOUT_FAST                MSEC_TO_UNITS(4000, UNIT_10_MS)             /**< Connection supervisory timeout (4 seconds). */

#define MIN_CONN_INTERVAL_SLOW               MSEC_TO_UNITS(100, UNIT_1_25_MS)            /**< Minimum acceptable connection interval (0.1 seconds). */
#define MAX_CONN_INTERVAL_SLOW               MSEC_TO_UNITS(150, UNIT_1_25_MS)            /**< Maximum acceptable connection interval (0.2 second). */
#define SLAVE_LATENCY_SLOW                   0                                           /**< Slave latency. */
#define CONN_SUP_TIMEOUT_SLOW                MSEC_TO_UNITS(6000, UNIT_10_MS)             /**< Connection supervisory timeout (4 seconds). */



#ifndef BLE_NRF52_OBSERVER_PRIO
#define BLE_NRF52_OBSERVER_PRIO 2
#endif



#define BLE_NRF52_DEF(_name)                                                                          \
ble_nrf52_t _name;                                                                             \
NRF_SDH_BLE_OBSERVER(_name ## _obs,                                                                    \
                     BLE_NRF52_OBSERVER_PRIO,                                                     \
                     ble_nrf52_on_ble_evt, &_name)


#define BLE_UUID_NRF52_SERVICE 0x0001                      
#define BLE_NRF52_MAX_DATA_LEN (NRF_SDH_BLE_GATT_MAX_MTU_DEFAULT_SIZE - 3)  





#define BLE_CONFIG_DEVICE_NAME_LEN_MAX 10




typedef struct __attribute__ ((packed))
{
    uint8_t name[BLE_CONFIG_DEVICE_NAME_LEN_MAX+1];
    uint8_t len;
}ble_config_dev_name_t;


typedef struct __attribute__ ((packed))
{
		uint8_t  enable_fast;
    uint16_t interval_fast;
    uint8_t  timeout_fast;
		uint8_t  enable_slow;
    uint16_t interval_slow;
    uint8_t  timeout_slow;	
}ble_config_adv_params_t;


typedef struct __attribute__ ((packed))
{
    uint16_t min_conn_int;
    uint16_t max_conn_int;
    uint16_t slave_latency;
    uint16_t sup_timeout;
}ble_config_conn_params_t;

typedef struct __attribute__ ((packed)) __attribute__((aligned(4))) 
{
    ble_config_dev_name_t      dev_name;
    ble_config_adv_params_t    adv_params;
    ble_config_conn_params_t   conn_params;

}ble_config_params_t;


#define CONFIG_ADV_PARAMS_INTERVAL_MIN 32UL
#define CONFIG_ADV_PARAMS_INTERVAL_MAX 8000UL
#define CONFIG_ADV_PARAMS_TIMEOUT_MIN  0UL
#define CONFIG_ADV_PARAMS_TIMEOUT_MAX  180UL


typedef enum
{
    BLE_NRF52_EVT_CONFIG_DATA,
		BLE_NRF52_EVT_TX_RDY,
		BLE_NRF52_EVT_HVC,
		BLE_NRF52_EVT_CONFIG_NOTIFY_ENABLE,
		BLE_NRF52_EVT_CONFIG_NOTIFY_DISABLE,
    BLE_NRF52_EVT_CONTROL_DATA,
		BLE_NRF52_EVT_CONTROL_NOTIFY_ENABLE,
		BLE_NRF52_EVT_CONTROL_NOTIFY_DISABLE,
		BLE_NRF52_EVT_CONNECTED,
		BLE_NRF52_EVT_DISCONNECTED,	
}ble_nrf52_evt_type_t;




typedef struct ble_nrf52_s     ble_nrf52_t;

typedef struct
{
    uint8_t const * p_data;           
    uint16_t        length;          
} ble_nrf52_evt_data_t;
typedef struct
{
    ble_nrf52_evt_type_t type;           
    ble_nrf52_t * p_nrf52;                 
    union
    {
        ble_nrf52_evt_data_t data; 
    } params;
} ble_nrf52_evt_t;

typedef void (*ble_nrf52_evt_handler_t) (ble_nrf52_evt_t * p_evt);

typedef struct
{

    ble_nrf52_evt_handler_t   evt_handler; /**< Event handler to be called for handling received data. */
} ble_nrf52_init_t;


struct ble_nrf52_s
{
    uint8_t                     uuid_type;                    
    uint16_t                    service_handle;           
    ble_gatts_char_handles_t    config_handles;   
		ble_gatts_char_handles_t    control_handles;   
    uint16_t                    conn_handle;                
    bool                        is_config_notification_enabled;
		bool                        is_control_notification_enabled;
		ble_nrf52_evt_handler_t    evt_handler;     
};



uint32_t ble_nrf52_init(ble_nrf52_t * p_nrf52, const ble_nrf52_init_t * p_nrf52_init);
void ble_nrf52_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context);




uint32_t ble_srv_nrf52_config_string_send(ble_nrf52_t * p_nrf52, uint8_t * p_string, uint16_t * p_length);
uint32_t ble_srv_nrf52_control_string_send(ble_nrf52_t * p_nrf52, uint8_t * p_string, uint16_t * p_length);




#endif //__BLE_SRV_NRF52_H__
