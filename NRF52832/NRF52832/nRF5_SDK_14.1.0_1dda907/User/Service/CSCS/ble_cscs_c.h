#ifndef __BLE_CSCS_C_H__
#define __BLE_CSCS_C_H__


#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_db_discovery.h"
#include "nrf_sdh_ble.h"

#define BLE_CSCS_C_BLE_OBSERVER_PRIO 2
#define BLE_CSCS_C_DEF(_name)                                                                       \
ble_cscs_c_t _name;                                                                          \
NRF_SDH_BLE_OBSERVER(_name ## _obs,                                                                 \
                     BLE_CSCS_C_BLE_OBSERVER_PRIO,                                                  \
                     ble_cscs_c_on_ble_evt, &_name)

#define BLE_CSCS_WHEEL_REVOLUTION_DATA_PRESENT     0x00  /**< Wheel Revolution Data Present	. */
#define BLE_CSCS_CRANK_REVOLUTION_DATA_PRESENT     0x01  /**< Crank Revolution Data Present . */
typedef struct
{
    uint16_t csc_cccd_handle;                /**< Handle of the CCCD of the Running Speed and Cadence characteristic. */
    uint16_t csc_handle;                     /**< Handle of the Cycling Speed and Cadence characteristic as provided by the SoftDevice. */
} ble_cscs_c_db_t;

/**@brief   CSCS Client event type. */
typedef enum
{
	BLE_CSCS_C_EVT_DISCOVERY_COMPLETE = 1, 
	BLE_CSCS_C_EVT_CSC_NOTIFICATION,
} ble_cscs_c_evt_type_t;

/**@brief   Structure containing the Cycling Speed and Cadence measurement received from the peer. */
typedef struct
{
	bool     is_wheel_revolution_data_present; 
	bool     is_crank_revolution_data_present; 
	uint32_t cumulative_wheel_revolutions;
	uint16_t last_wheel_event_time;
	uint16_t cumulative_crank_revolutions;
	uint16_t last_crank_event_time;	
} ble_csc_t;

/**@brief   Cycling Speed and Cadence Event structure. */
typedef struct
{
    ble_cscs_c_evt_type_t evt_type;  /**< Type of the event. */
    uint16_t  conn_handle;           /**< Connection handle on which the rscs_c event  occured.*/
    union
    {
        ble_cscs_c_db_t cscs_db;           /**< Cycling Speed and Cadence Service related handles found on the peer device. This will be filled if the evt_type is @ref BLE_RSCS_C_EVT_DISCOVERY_COMPLETE.*/
        ble_csc_t       csc;               /**< Cycling Speed and Cadence measurement received. This will be filled if the evt_type is @ref BLE_RSCS_C_EVT_RSC_NOTIFICATION. */
    } params;
} ble_cscs_c_evt_t;

// Forward declaration of the ble_cscs_c_t type.
typedef struct ble_cscs_c_s ble_cscs_c_t;

/**@brief   Event handler type.
 *
 * @details This is the type of the event handler that should be provided by the application
 *          of this module in order to receive events.
 */
typedef void (* ble_cscs_c_evt_handler_t) (ble_cscs_c_t * p_ble_cscs_c, ble_cscs_c_evt_t * p_evt);

/**@brief   Cycling Speed and Cadence client structure. */
struct ble_cscs_c_s
{
    uint16_t                 conn_handle;      /**< Connection handle as provided by the SoftDevice. */
    ble_cscs_c_db_t          peer_db;          /**< Handles related to CSCS on the peer*/
    ble_cscs_c_evt_handler_t evt_handler;      /**< Application event handler to be called when there is an event related to the Running Speed and Cadence service. */
};

/**@brief   Cycling Speed and Cadence client initialization structure. */
typedef struct
{
    ble_cscs_c_evt_handler_t evt_handler;  /**< Event handler to be called by the Running Speed and Cadence Client module whenever there is an event related to the Running Speed and Cadence Service. */
} ble_cscs_c_init_t;






uint32_t ble_cscs_c_init(ble_cscs_c_t * p_ble_rscs_c, ble_cscs_c_init_t * p_ble_cscs_c_init);


void ble_cscs_c_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context);


uint32_t ble_cscs_c_csc_notif_enable(ble_cscs_c_t * p_ble_cscs_c);

void ble_cscs_on_db_disc_evt(ble_cscs_c_t * p_ble_cscs_c, ble_db_discovery_evt_t const * p_evt);

uint32_t ble_cscs_c_handles_assign(ble_cscs_c_t    * p_ble_cscs_c,
                                   uint16_t          conn_handle,
                                   ble_cscs_c_db_t * p_peer_handles);




#endif //__BLE_CSCS_C_H__
