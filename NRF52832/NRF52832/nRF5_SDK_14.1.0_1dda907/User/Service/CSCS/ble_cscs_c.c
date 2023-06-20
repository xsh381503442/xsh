#include "ble_cscs_c.h"


#include "sdk_common.h"
#include "ble_db_discovery.h"
#include "ble_types.h"
#include "ble_srv_common.h"
#include "ble_gattc.h"




#define LOG_LEVEL_OFF 							0
#define LOG_LEVEL_ERROR 						1
#define LOG_LEVEL_WARNING 					2
#define LOG_LEVEL_INFO 							3
#define LOG_LEVEL_DEBUG 						4
#define NRF_LOG_MODULE_NAME ble_cscs_c
#define NRF_LOG_LEVEL       LOG_LEVEL_OFF
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
NRF_LOG_MODULE_REGISTER();



#define TX_BUFFER_MASK         0x07                  /**< TX Buffer mask, must be a mask of continuous zeroes, followed by continuous sequence of ones: 000...111. */
#define TX_BUFFER_SIZE         (TX_BUFFER_MASK + 1)  /**< Size of send buffer, which is 1 higher than the mask. */

#define WRITE_MESSAGE_LENGTH   BLE_CCCD_VALUE_LEN    /**< Length of the write message for CCCD. */

typedef enum
{
    READ_REQ,  /**< Type identifying that this tx_message is a read request. */
    WRITE_REQ  /**< Type identifying that this tx_message is a write request. */
} tx_request_t;

/**@brief Structure for writing a message to the peer, i.e. CCCD.
 */
typedef struct
{
    uint8_t                  gattc_value[WRITE_MESSAGE_LENGTH];  /**< The message to write. */
    ble_gattc_write_params_t gattc_params;                       /**< GATTC parameters for this message. */
} write_params_t;

/**@brief Structure for holding data to be transmitted to the connected central.
 */
typedef struct
{
    uint16_t     conn_handle;  /**< Connection handle to be used when transmitting this message. */
    tx_request_t type;         /**< Type of this message, i.e. read or write message. */
    union
    {
        uint16_t       read_handle;  /**< Read request message. */
        write_params_t write_req;    /**< Write request message. */
    } req;
} tx_message_t;


static tx_message_t   m_tx_buffer[TX_BUFFER_SIZE];  /**< Transmit buffer for messages to be transmitted to the central. */
static uint32_t       m_tx_insert_index = 0;        /**< Current index in the transmit buffer where the next message should be inserted. */
static uint32_t       m_tx_index = 0;               /**< Current index in the transmit buffer from where the next message to be transmitted resides. */


static void tx_buffer_process(void)
{
    if (m_tx_index != m_tx_insert_index)
    {
        uint32_t err_code;

        if (m_tx_buffer[m_tx_index].type == READ_REQ)
        {
            err_code = sd_ble_gattc_read(m_tx_buffer[m_tx_index].conn_handle,
                                         m_tx_buffer[m_tx_index].req.read_handle,
                                         0);
        }
        else
        {
            err_code = sd_ble_gattc_write(m_tx_buffer[m_tx_index].conn_handle,
                                          &m_tx_buffer[m_tx_index].req.write_req.gattc_params);
					
					
        }
        if (err_code == NRF_SUCCESS)
        {
            NRF_LOG_DEBUG("SD Read/Write API returns Success.");
            m_tx_index++;
            m_tx_index &= TX_BUFFER_MASK;
        }
        else
        {
            NRF_LOG_DEBUG("SD Read/Write API returns error. This message sending will be "
                          "attempted again..");
        }
    }
}


static void on_write_rsp(ble_cscs_c_t * p_ble_cscs_c, const ble_evt_t * p_ble_evt)
{
    // Check if the event if on the link for this instance
    if (p_ble_cscs_c->conn_handle != p_ble_evt->evt.gattc_evt.conn_handle)
    {
        return;
    }
    // Check if there is any message to be sent across to the peer and send it.
    tx_buffer_process();
}


static void on_hvx(ble_cscs_c_t * p_ble_cscs_c, const ble_evt_t * p_ble_evt)
{
    const ble_gattc_evt_hvx_t * p_notif = &p_ble_evt->evt.gattc_evt.params.hvx;

    // Check if the event if on the link for this instance
    if (p_ble_cscs_c->conn_handle != p_ble_evt->evt.gattc_evt.conn_handle)
    {
        return;
    }

    // Check if this is a Cycling Speed and Cadence notification.
    if (p_ble_evt->evt.gattc_evt.params.hvx.handle == p_ble_cscs_c->peer_db.csc_handle)
    {
        uint32_t         index = 0;
        ble_cscs_c_evt_t ble_cscs_c_evt;
        ble_cscs_c_evt.evt_type    = BLE_CSCS_C_EVT_CSC_NOTIFICATION;
        ble_cscs_c_evt.conn_handle = p_ble_evt->evt.gattc_evt.conn_handle;

        // Flags field
        ble_cscs_c_evt.params.csc.is_wheel_revolution_data_present = p_notif->data[index] >> BLE_CSCS_WHEEL_REVOLUTION_DATA_PRESENT    & 0x01;
        ble_cscs_c_evt.params.csc.is_crank_revolution_data_present = p_notif->data[index] >> BLE_CSCS_CRANK_REVOLUTION_DATA_PRESENT    & 0x01;
        index++;			

        if (ble_cscs_c_evt.params.csc.is_wheel_revolution_data_present == true)
        {
					ble_cscs_c_evt.params.csc.cumulative_wheel_revolutions = uint32_decode(&p_notif->data[index]);
					index += sizeof(uint32_t);
					
					ble_cscs_c_evt.params.csc.last_wheel_event_time = uint16_decode(&p_notif->data[index]);
					index += sizeof(uint16_t);					
        }			
        if (ble_cscs_c_evt.params.csc.is_crank_revolution_data_present == true)
        {
					ble_cscs_c_evt.params.csc.cumulative_crank_revolutions = uint16_decode(&p_notif->data[index]);
					index += sizeof(uint16_t);
					
					ble_cscs_c_evt.params.csc.last_crank_event_time = uint16_decode(&p_notif->data[index]);
					index += sizeof(uint16_t);					
        }				
			
        p_ble_cscs_c->evt_handler(p_ble_cscs_c, &ble_cscs_c_evt);
    }
}

void ble_cscs_on_db_disc_evt(ble_cscs_c_t * p_ble_cscs_c, const ble_db_discovery_evt_t * p_evt)
{
    if (p_evt->evt_type == BLE_DB_DISCOVERY_COMPLETE &&
        p_evt->params.discovered_db.srv_uuid.uuid == BLE_UUID_CYCLING_SPEED_AND_CADENCE &&
        p_evt->params.discovered_db.srv_uuid.type == BLE_UUID_TYPE_BLE)
    {
				NRF_LOG_DEBUG("CSCS Service discovery success at peer. ");
			
        ble_cscs_c_evt_t evt;
        evt.conn_handle = p_evt->conn_handle;

        // Find the CCCD Handle of the Cycling Speed and Cadence characteristic.
        for (uint32_t i = 0; i < p_evt->params.discovered_db.char_count; i++)
        {
            if (p_evt->params.discovered_db.charateristics[i].characteristic.uuid.uuid ==
                BLE_UUID_CSC_MEASUREMENT_CHAR)
            {
                // Found Cycling Speed and Cadence characteristic. Store CCCD handle and break.
							NRF_LOG_DEBUG("Found Cycling Speed and Cadence characteristic. Store CCCD handle and break.");
                evt.params.cscs_db.csc_cccd_handle =
                    p_evt->params.discovered_db.charateristics[i].cccd_handle;
                evt.params.cscs_db.csc_handle      =
                    p_evt->params.discovered_db.charateristics[i].characteristic.handle_value;
                break;
            }
        }			
				
				
				NRF_LOG_DEBUG("Cycling Speed and Cadence Service discovered at peer.");
				
        //If the instance has been assigned prior to db_discovery, assign the db_handles
        if (p_ble_cscs_c->conn_handle != BLE_CONN_HANDLE_INVALID)
        {
            if ((p_ble_cscs_c->peer_db.csc_cccd_handle == BLE_GATT_HANDLE_INVALID)&&
                (p_ble_cscs_c->peer_db.csc_handle == BLE_GATT_HANDLE_INVALID))
            {
                p_ble_cscs_c->peer_db = evt.params.cscs_db;
            }
        }			

        evt.evt_type = BLE_CSCS_C_EVT_DISCOVERY_COMPLETE;

        p_ble_cscs_c->evt_handler(p_ble_cscs_c, &evt);				
				
				
    }
    else
    {
        NRF_LOG_DEBUG("CSCS Service discovery failure at peer. ");
			
			NRF_LOG_DEBUG("p_evt->evt_type --> %02X",p_evt->evt_type);
			NRF_LOG_DEBUG("p_evt->params.discovered_db.srv_uuid.uuid --> %04X",p_evt->params.discovered_db.srv_uuid.uuid);
			NRF_LOG_DEBUG("p_evt->params.discovered_db.srv_uuid.type --> %02X",p_evt->params.discovered_db.srv_uuid.type);
			
    }			
}


uint32_t ble_cscs_c_init(ble_cscs_c_t * p_ble_cscs_c, ble_cscs_c_init_t * p_ble_cscs_c_init)
{
    VERIFY_PARAM_NOT_NULL(p_ble_cscs_c);
    VERIFY_PARAM_NOT_NULL(p_ble_cscs_c_init);

    ble_uuid_t rscs_uuid;

    rscs_uuid.type = BLE_UUID_TYPE_BLE;
    rscs_uuid.uuid = BLE_UUID_CYCLING_SPEED_AND_CADENCE;

    p_ble_cscs_c->evt_handler             = p_ble_cscs_c_init->evt_handler;
    p_ble_cscs_c->conn_handle             = BLE_CONN_HANDLE_INVALID;
    p_ble_cscs_c->peer_db.csc_cccd_handle = BLE_GATT_HANDLE_INVALID;
    p_ble_cscs_c->peer_db.csc_handle      = BLE_GATT_HANDLE_INVALID;

    return ble_db_discovery_evt_register(&rscs_uuid);
}

uint32_t ble_cscs_c_handles_assign(ble_cscs_c_t *    p_ble_cscs_c,
                                   uint16_t         conn_handle,
                                   ble_cscs_c_db_t * p_peer_handles)
{
    VERIFY_PARAM_NOT_NULL(p_ble_cscs_c);
    p_ble_cscs_c->conn_handle = conn_handle;
    if (p_peer_handles != NULL)
    {
        p_ble_cscs_c->peer_db = *p_peer_handles;
    }

    return NRF_SUCCESS;
}


static void on_disconnected(ble_cscs_c_t * p_ble_cscs_c, const ble_evt_t * p_ble_evt)
{
    if (p_ble_cscs_c->conn_handle == p_ble_evt->evt.gap_evt.conn_handle)
    {
        p_ble_cscs_c->conn_handle             = BLE_CONN_HANDLE_INVALID;
        p_ble_cscs_c->peer_db.csc_cccd_handle = BLE_GATT_HANDLE_INVALID;
        p_ble_cscs_c->peer_db.csc_handle      = BLE_GATT_HANDLE_INVALID;
    }
}

void ble_cscs_c_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context)
{
    if ((p_context == NULL) || (p_ble_evt == NULL))
    {
        return;
    }

    ble_cscs_c_t * p_ble_cscs_c = (ble_cscs_c_t *)p_context;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GATTC_EVT_HVX:
					NRF_LOG_DEBUG("BLE_GATTC_EVT_HVX");
            on_hvx(p_ble_cscs_c, p_ble_evt);
            break;

        case BLE_GATTC_EVT_WRITE_RSP:
					NRF_LOG_DEBUG("BLE_GATTC_EVT_WRITE_RSP");
            on_write_rsp(p_ble_cscs_c, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
					NRF_LOG_DEBUG("BLE_GAP_EVT_DISCONNECTED");
            on_disconnected(p_ble_cscs_c, p_ble_evt);
            break;

        default:
            break;
    }
}


static uint32_t cccd_configure(uint16_t conn_handle, uint16_t handle_cccd, bool enable)
{
    NRF_LOG_DEBUG("Configuring CCCD. CCCD Handle = %d, Connection Handle = %d",
                  handle_cccd, conn_handle);

    tx_message_t * p_msg;
    uint16_t       cccd_val = enable ? BLE_GATT_HVX_NOTIFICATION : 0;

    p_msg              = &m_tx_buffer[m_tx_insert_index++];
    m_tx_insert_index &= TX_BUFFER_MASK;

    p_msg->req.write_req.gattc_params.handle   = handle_cccd;
    p_msg->req.write_req.gattc_params.len      = WRITE_MESSAGE_LENGTH;
    p_msg->req.write_req.gattc_params.p_value  = p_msg->req.write_req.gattc_value;
    p_msg->req.write_req.gattc_params.offset   = 0;
    p_msg->req.write_req.gattc_params.write_op = BLE_GATT_OP_WRITE_REQ;
    p_msg->req.write_req.gattc_value[0]        = LSB_16(cccd_val);
    p_msg->req.write_req.gattc_value[1]        = MSB_16(cccd_val);
    p_msg->conn_handle                         = conn_handle;
    p_msg->type                                = WRITE_REQ;

    tx_buffer_process();
    return NRF_SUCCESS;
}

uint32_t ble_cscs_c_csc_notif_enable(ble_cscs_c_t * p_ble_cscs_c)
{
    VERIFY_PARAM_NOT_NULL(p_ble_cscs_c);

    if (p_ble_cscs_c->conn_handle == BLE_CONN_HANDLE_INVALID)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    return cccd_configure(p_ble_cscs_c->conn_handle, p_ble_cscs_c->peer_db.csc_cccd_handle, true);
}




