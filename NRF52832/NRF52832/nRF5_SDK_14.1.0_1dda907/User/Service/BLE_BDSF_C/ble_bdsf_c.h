/**@file
 *
 * @author 	 MengXianyu
 * @date 		 0829/2018
 * @Version	 V0.1.0
 *
 * @defgroup ble_bdsf_c 与北斗收发机交互（领航项目） Service Client
 * @{
 * @ingroup  ble_sdk_srv
 * @brief    与北斗收发机交互 Service client.
 *
 * @details  这个模块用于做主与从（北斗收发机）进行信息交互与查询，包括：短报文、自动报位、信息查询交互等.
 *
 * @note     程序必须通过NRF_SDH_BLE_OBSERVER宏把这个模块注册为BLE事件观察者.
 *           例子:
 *          @code
 *              ble_bdsf_c_t instance;
 *              NRF_SDH_BLE_OBSERVER(anything, BLE_BDSF_C_BLE_OBSERVER_PRIO,
 *                                   ble_bdsf_c_on_ble_evt, &instance);
 *          @endcode
 */

#ifndef BLE_BDSF_C_H__
#define BLE_BDSF_C_H__

#include <stdint.h>
#include "ble.h"
#include "ble_db_discovery.h"
#include "nrf_sdh_ble.h"
#include "watch_config.h"
#ifdef __cplusplus
extern "C" {
#endif

/**@brief   Macro for defining a ble_bdsf_c instance.
 *
 * @param   _name   Name of the instance.
 * @hideinitializer
 */
#define BLE_BDSF_C_DEF(_name)                                                                        \
ble_bdsf_c_t _name;                                                                           \
NRF_SDH_BLE_OBSERVER(_name ## _obs,                                                                  \
                     BLE_BDSF_C_BLE_OBSERVER_PRIO,                                                   \
                     ble_bdsf_c_on_ble_evt, &_name)
	
#define BDSF_UUID_SERVICE     	0xFFF0
#define BDSF_UUID_READ_CHAR 	0xFFF4

#define BDSF_UUID_WRITE_CHAR	0xFFF4

#define OPCODE_LENGTH 1
#define HANDLE_LENGTH 2

/**@brief   Maximum length of data (in bytes) that can be transmitted to the peer by the BDSF service module. */
#if defined(NRF_SDH_BLE_GATT_MAX_MTU_SIZE) && (NRF_SDH_BLE_GATT_MAX_MTU_SIZE != 0)
    #define BLE_BDSF_MAX_DATA_LEN (NRF_SDH_BLE_GATT_MAX_MTU_SIZE - OPCODE_LENGTH - HANDLE_LENGTH)
#else
    #define BLE_BDSF_MAX_DATA_LEN (BLE_GATT_MTU_SIZE_DEFAULT - OPCODE_LENGTH - HANDLE_LENGTH)
    #warning NRF_SDH_BLE_GATT_MAX_MTU_SIZE is not defined.
#endif
	
/**
 * @defgroup bdsf_c_enums Enumerations
 * @{
 */
	
/**@brief BDSF Client event type. */
typedef enum
{
		BLE_BDSF_C_EVT_DISCOVERY_COMPLETE,   
    BLE_BDSF_C_EVT_MESSAGE_GET_EVT,           
    BLE_BDSF_C_EVT_DISCONNECTED          
} ble_bdsf_c_evt_type_t;

/** @} */


/**
 * @defgroup bdsf_c_structs Structures
 * @{
 */

/**@brief Structure containing the handles related to the BeiDou RT Machine Service found on the peer. */
typedef struct
{
    uint16_t bdsf_cccd_handle;  /**< Handle of the CCCD of the BeiDou RT Machine message characteristic. */
    uint16_t bdsf_read_handle;       /**< Handle of the BeiDou RT Machine read characteristic as provided by the SoftDevice. */
		uint16_t bdsf_write_handle;
} ble_bdsf_c_handles_t;

/**@brief BeiDou RT Machine Event structure. */
typedef struct
{
    ble_bdsf_c_evt_type_t evt_type;    
    uint16_t              conn_handle; 
    uint16_t              max_data_len;
    uint8_t             * p_data;
    uint8_t               data_len;
    ble_bdsf_c_handles_t  handles;    
} ble_bdsf_c_evt_t;

/** @} */

/**
 * @defgroup bdsf_c_types Types
 * @{
 */

// Forward declaration of the ble_bas_t type.
typedef struct ble_bdsf_c_s ble_bdsf_c_t;

/**@brief   Event handler type.
 *
 * @details This is the type of the event handler that should be provided by the application
 *          of this module in order to receive events.
 */
typedef void (* ble_bdsf_c_evt_handler_t) (ble_bdsf_c_t * p_ble_bdsf_c, ble_bdsf_c_evt_t * p_evt);

/** @} */

/**
 * @addtogroup bdsf_c_structs
 * @{
 */
 
/**@brief BeiDou RT Machine Client structure.
 */
struct ble_bdsf_c_s
{
    uint8_t                  uuid_type;      /**< UUID type. */
    uint16_t                 conn_handle;    /**< Handle of the current connection. Set with @ref ble_nus_c_handles_assign when connected. */
    ble_bdsf_c_handles_t     handles;        /**< Handles on the connected peer device needed to interact with it. */
    ble_bdsf_c_evt_handler_t evt_handler;    /**< Application event handler to be called when there is an event related to the NUS. */
};

/**@brief BeiDou RT Machine Client initialization structure.
 */
typedef struct
{
    ble_bdsf_c_evt_handler_t evt_handler;  /**< Event handler to be called by the BeiDou Machine Client module whenever there is an event related to the BeiDou Machine Service. */
} ble_bdsf_c_init_t;

/** @} */	

/**
 * @defgroup hrs_c_functions Functions
 * @{
 */

//Function for initializing the BeiDou R/T Machine client module.
extern uint32_t ble_bdsf_c_init(ble_bdsf_c_t * p_ble_bdsf_c, ble_bdsf_c_init_t * p_ble_bdsf_c_init);

//Function for handling BLE events from the SoftDevice.
extern void ble_bdsf_c_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context);

//Function for requesting the peer to start sending notification of BeiDou machine messages.
extern uint32_t ble_bdsf_c_message_notif_enable(ble_bdsf_c_t * p_ble_bdsf_c);

//Function for handling events from the database discovery module.
extern void ble_bdsf_on_db_disc_evt(ble_bdsf_c_t * p_ble_bdsf_c, ble_db_discovery_evt_t * p_evt);

//Function for assigning a handles to a this instance of bdsf_c.
extern uint32_t ble_bdsf_c_handles_assign(ble_bdsf_c_t *    p_ble_bdsf_c,
                                  uint16_t         conn_handle,
                                  const ble_bdsf_c_handles_t * p_peer_bdsf_handles);

//Function for sending a string to the server.
extern uint32_t ble_bdsf_c_string_send(ble_bdsf_c_t * p_ble_bdsf_c, uint8_t * p_string, uint16_t length);


//BeiDou R/T machine controller handler
//void bdsf_c_evt_handler(ble_bdsf_c_t * p_bdsf_c, ble_bdsf_c_evt_t * p_bdsf_c_evt);



/** @} */ // End tag for Function group.
	
#ifdef __cplusplus
}
#endif

#endif	// BLE_BDSF_C_H__

/** @} */	// End tag for the file.

