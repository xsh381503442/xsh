#include "sdk_common.h"
//#include "watch_config.h"
//#ifdef WATCH_HAS_LINGHANG_FUNC
#include "ble_bdsf_c.h"
#include "ble_db_discovery.h"
#include "ble_types.h"
#include "ble_srv_common.h"
#include "ble_gattc.h"
#include "task_ble_bdsf.h"

#define NRF_LOG_MODULE_NAME ble_bdsf_c
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

extern const char *ins_recv_arr[];
extern uint8_t MAX_INS_RECV_NUM;


char task_ble_bdsf_buf_t[256] = {0};
uint8_t index_t = 0;

void ble_bdsf_on_db_disc_evt(ble_bdsf_c_t * p_ble_bdsf_c, ble_db_discovery_evt_t * p_evt)
{
    ble_bdsf_c_evt_t bdsf_c_evt;
    memset(&bdsf_c_evt,0,sizeof(ble_bdsf_c_evt_t));

    ble_gatt_db_char_t * p_chars = p_evt->params.discovered_db.charateristics;

    // Check if the BDSF was discovered.
    if (    (p_evt->evt_type == BLE_DB_DISCOVERY_COMPLETE)
        &&  (p_evt->params.discovered_db.srv_uuid.uuid == BDSF_UUID_SERVICE)
        &&  (p_evt->params.discovered_db.srv_uuid.type == p_ble_bdsf_c->uuid_type))
    {
        for (uint32_t i = 0; i < p_evt->params.discovered_db.char_count; i++)
        {
            switch (p_chars[i].characteristic.uuid.uuid)
            {
                case BDSF_UUID_WRITE_CHAR:                  
										bdsf_c_evt.handles.bdsf_write_handle = p_chars[i].characteristic.handle_value;
										NRF_LOG_INFO("BDSF_UUID_WRITE_CHAR_HANDLE: 0x%x\n", bdsf_c_evt.handles.bdsf_write_handle);
			#if defined JW902_OLD 
                    break;

                case BDSF_UUID_READ_CHAR:
			#endif
										bdsf_c_evt.handles.bdsf_read_handle = p_chars[i].characteristic.handle_value;
										bdsf_c_evt.handles.bdsf_cccd_handle = p_chars[i].cccd_handle;
										NRF_LOG_INFO("BDSF_UUID_READ_CHAR_HANDLE: 0x%x\n", bdsf_c_evt.handles.bdsf_read_handle);
										NRF_LOG_INFO("BDSF_UUID_READ_CHAR_CCCD_HANDLE: 0x%x\n", bdsf_c_evt.handles.bdsf_cccd_handle);
                    break;

                default:
                    break;
            }
        }
        if (p_ble_bdsf_c->evt_handler != NULL)
        {
            bdsf_c_evt.conn_handle = p_evt->conn_handle;
            bdsf_c_evt.evt_type    = BLE_BDSF_C_EVT_DISCOVERY_COMPLETE;
            p_ble_bdsf_c->evt_handler(p_ble_bdsf_c, &bdsf_c_evt);
        }
    }
}


static void on_hvx(ble_bdsf_c_t * p_ble_bdsf_c, ble_evt_t const * p_ble_evt)
{
		static uint8_t data_array[256] = {0};
		NRF_LOG_INFO("[ble_bdsf_c]:on_hvx");
    // HVX can only occur from client sending.
    if (   (p_ble_bdsf_c->handles.bdsf_read_handle != BLE_GATT_HANDLE_INVALID)
        && (p_ble_evt->evt.gattc_evt.params.hvx.handle == p_ble_bdsf_c->handles.bdsf_read_handle)
        && (p_ble_bdsf_c->evt_handler != NULL))
    {
    	NRF_LOG_INFO("[ble_bdsf_c]:client sending");
        ble_bdsf_c_evt_t ble_bdsf_c_evt = {0};

        ble_bdsf_c_evt.evt_type = BLE_BDSF_C_EVT_MESSAGE_GET_EVT;
        ble_bdsf_c_evt.p_data   = (uint8_t *)p_ble_evt->evt.gattc_evt.params.hvx.data;
        ble_bdsf_c_evt.data_len = p_ble_evt->evt.gattc_evt.params.hvx.len;

        p_ble_bdsf_c->evt_handler(p_ble_bdsf_c, &ble_bdsf_c_evt);
				
		memcpy(data_array, ble_bdsf_c_evt.p_data, ble_bdsf_c_evt.data_len);
        NRF_LOG_INFO("recieve data legth:%d",ble_bdsf_c_evt.data_len);
				
	if(strchr((char *)&data_array[0], '$') != NULL)	//ͷ
	{
		index_t = 0;
		memset(&task_ble_bdsf_buf_t[0], 0, 256);
		memcpy(&task_ble_bdsf_buf_t[0], ble_bdsf_c_evt.p_data, ble_bdsf_c_evt.data_len);
		index_t += ble_bdsf_c_evt.data_len;
        NRF_LOG_INFO("[ble_bdsf_c]:Head");
	}
	else
	{
		if(strchr((char *)&data_array[0], '\n') == NULL)		
		{
			memcpy(&task_ble_bdsf_buf_t[index_t], ble_bdsf_c_evt.p_data, ble_bdsf_c_evt.data_len);
			index_t += ble_bdsf_c_evt.data_len;
            NRF_LOG_INFO("[ble_bdsf_c]:END");
		}
		else		//β
		{
			memcpy(&task_ble_bdsf_buf_t[index_t], ble_bdsf_c_evt.p_data, ble_bdsf_c_evt.data_len);
			index_t += ble_bdsf_c_evt.data_len;
					
			NRF_LOG_INFO("Command on_hvx: %s", task_ble_bdsf_buf_t);	
			task_ble_bdsf_handle(TASK_BLE_BDSF_EVT_DATA_NOTIFY_SEND, (uint8_t *)&task_ble_bdsf_buf_t[0], index_t);
		}			
	}
    
    if ((strchr((char *)&data_array[0], '$') != NULL) && (strchr((char *)&data_array[0], '\n') != NULL))
    {
        memset(&task_ble_bdsf_buf_t[0], 0, 256);
		memcpy(&task_ble_bdsf_buf_t[0], ble_bdsf_c_evt.p_data, ble_bdsf_c_evt.data_len);
        NRF_LOG_INFO("Command1 on_hvx: %s", task_ble_bdsf_buf_t);	
		task_ble_bdsf_handle(TASK_BLE_BDSF_EVT_DATA_NOTIFY_SEND, (uint8_t *)&task_ble_bdsf_buf_t[0], ble_bdsf_c_evt.data_len);        
    }
    
				
//        NRF_LOG_DEBUG("Client sending data.");
//				memcpy(data_array, ble_bdsf_c_evt.p_data, ble_bdsf_c_evt.data_len);
//				NRF_LOG_INFO("Client sending data[%d]: recvive --> %s\n",ble_bdsf_c_evt.data_len, ble_bdsf_c_evt.p_data);
//				task_ble_bdsf_handle(TASK_BLE_BDSF_EVT_DATA_NOTIFY_SEND, ble_bdsf_c_evt.p_data, ble_bdsf_c_evt.data_len);
//				ble_bdsf_c_string_send(p_ble_bdsf_c, data_array, 30);
    }
}

uint32_t ble_bdsf_c_init(ble_bdsf_c_t * p_ble_bdsf_c, ble_bdsf_c_init_t * p_ble_bdsf_c_init)
{
    ble_uuid_t    bdsf_uuid;

    VERIFY_PARAM_NOT_NULL(p_ble_bdsf_c);
    VERIFY_PARAM_NOT_NULL(p_ble_bdsf_c_init);

		p_ble_bdsf_c->uuid_type = BLE_UUID_TYPE_BLE;
    bdsf_uuid.type = p_ble_bdsf_c->uuid_type;
    bdsf_uuid.uuid = BDSF_UUID_SERVICE;

    p_ble_bdsf_c->conn_handle           = BLE_CONN_HANDLE_INVALID;
    p_ble_bdsf_c->evt_handler           = p_ble_bdsf_c_init->evt_handler;
    p_ble_bdsf_c->handles.bdsf_read_handle = BLE_GATT_HANDLE_INVALID;
    p_ble_bdsf_c->handles.bdsf_write_handle = BLE_GATT_HANDLE_INVALID;

    return ble_db_discovery_evt_register(&bdsf_uuid);
}

static void on_write_rsp(ble_bdsf_c_t * p_ble_bdsf_c, const ble_evt_t * p_ble_evt)
{
    // Check if the event if on the link for this instance
    if (p_ble_bdsf_c->conn_handle != p_ble_evt->evt.gattc_evt.conn_handle)
    {
        return;
    }
    // Check if there is any message to be sent across to the peer and send it.
    sd_ble_gattc_read(p_ble_bdsf_c->conn_handle,
                                         p_ble_bdsf_c->handles.bdsf_read_handle,
                                         0);
}


void ble_bdsf_c_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context)
{
    ble_bdsf_c_t * p_ble_bdsf_c = (ble_bdsf_c_t *)p_context;

    if ((p_ble_bdsf_c == NULL) || (p_ble_evt == NULL))
    {
        return;
    }

    if ( (p_ble_bdsf_c->conn_handle != BLE_CONN_HANDLE_INVALID)
       &&(p_ble_bdsf_c->conn_handle != p_ble_evt->evt.gap_evt.conn_handle)
       )
    {
        return;
    }

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GATTC_EVT_HVX:
						//NRF_LOG_INFO("BLE_GATTC_EVT_HVX in ble_bdsf_c_on_ble_evt()......\n");
            on_hvx(p_ble_bdsf_c, p_ble_evt);
            break;
				case BLE_GATTC_EVT_WRITE_RSP:
						//NRF_LOG_INFO("BLE_GATTC_EVT_WRITE_RSP in ble_bdsf_c_on_ble_evt()......\n");
            //on_write_rsp(p_ble_bdsf_c, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            if (p_ble_evt->evt.gap_evt.conn_handle == p_ble_bdsf_c->conn_handle
                    && p_ble_bdsf_c->evt_handler != NULL)
            {
                ble_bdsf_c_evt_t bdsf_c_evt;

                bdsf_c_evt.evt_type = BLE_BDSF_C_EVT_DISCONNECTED;

                p_ble_bdsf_c->conn_handle = BLE_CONN_HANDLE_INVALID;
                p_ble_bdsf_c->evt_handler(p_ble_bdsf_c, &bdsf_c_evt);
            }
            break;

        default:
            // No implementation needed.
            break;
    }
}

/**@brief Function for creating a message for writing to the CCCD. */
static uint32_t cccd_configure(uint16_t conn_handle, uint16_t cccd_handle, bool enable)
{
    uint8_t buf[BLE_CCCD_VALUE_LEN];

    buf[0] = enable ? BLE_GATT_HVX_NOTIFICATION : 0;
    buf[1] = 0;

    ble_gattc_write_params_t const write_params =
    {
        .write_op = BLE_GATT_OP_WRITE_REQ,
        .flags    = BLE_GATT_EXEC_WRITE_FLAG_PREPARED_WRITE,
        .handle   = cccd_handle,
        .offset   = 0,
        .len      = sizeof(buf),
        .p_value  = buf
    };

    return sd_ble_gattc_write(conn_handle, &write_params);
}

uint32_t ble_bdsf_c_message_notif_enable(ble_bdsf_c_t * p_ble_bdsf_c)
{
    VERIFY_PARAM_NOT_NULL(p_ble_bdsf_c);

    if ( (p_ble_bdsf_c->conn_handle == BLE_CONN_HANDLE_INVALID)
       ||(p_ble_bdsf_c->handles.bdsf_read_handle == BLE_GATT_HANDLE_INVALID)
       )
    {
        return NRF_ERROR_INVALID_STATE;
    }
    return cccd_configure(p_ble_bdsf_c->conn_handle,p_ble_bdsf_c->handles.bdsf_cccd_handle, true);
}

uint32_t ble_bdsf_c_string_send(ble_bdsf_c_t * p_ble_bdsf_c, uint8_t * p_string, uint16_t length)
{
    VERIFY_PARAM_NOT_NULL(p_ble_bdsf_c);

    if (length > BLE_BDSF_MAX_DATA_LEN)
    {
				NRF_LOG_WARNING("BLE_BDSF_MAX_DATA_LEN(%d):Content too long.", BLE_BDSF_MAX_DATA_LEN);
        return NRF_ERROR_INVALID_PARAM;
    }
    if (p_ble_bdsf_c->conn_handle == BLE_CONN_HANDLE_INVALID)
    {
        NRF_LOG_WARNING("Connection handle invalid.");
        return NRF_ERROR_INVALID_STATE;
    }

    ble_gattc_write_params_t const write_params =
    {
        .write_op = BLE_GATT_OP_WRITE_CMD,
        .flags    = BLE_GATT_EXEC_WRITE_FLAG_PREPARED_WRITE,
        .handle   = p_ble_bdsf_c->handles.bdsf_write_handle,
        .offset   = 0,
        .len      = length,
        .p_value  = p_string
    };
		
		//NRF_LOG_INFO("p_ble_bdsf_c->handles.bdsf_write_handle is 0x%x...\n", p_ble_bdsf_c->handles.bdsf_write_handle);
		//NRF_LOG_INFO("p_string is %s\n", p_string);

    return sd_ble_gattc_write(p_ble_bdsf_c->conn_handle, &write_params);
}

uint32_t ble_bdsf_c_handles_assign(ble_bdsf_c_t               * p_ble_bdsf,
                                  uint16_t                    conn_handle,
                                  ble_bdsf_c_handles_t const * p_peer_handles)
{
    VERIFY_PARAM_NOT_NULL(p_ble_bdsf);

    p_ble_bdsf->conn_handle = conn_handle;
    if (p_peer_handles != NULL)
    {
        p_ble_bdsf->handles.bdsf_cccd_handle = p_peer_handles->bdsf_cccd_handle;
        p_ble_bdsf->handles.bdsf_read_handle      = p_peer_handles->bdsf_read_handle;
        p_ble_bdsf->handles.bdsf_write_handle      = p_peer_handles->bdsf_write_handle;
    }
    return NRF_SUCCESS;
}

//#endif


