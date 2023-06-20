#include "ble_srv_dfu.h"



#define BLE_UUID_JW_DFU_CONFIG_CHAR   0x0002                           
#define BLE_UUID_JW_DFU_CONTROL_CHAR   0x0003  
#define BLE_UUID_JW_DFU_FLASH_CHAR   0x0004  

#define JW_DFU_BASE_UUID  {{0x00, 0x00, 0x00, 0x01, 0xA9, 0x78, 0x99, 0x69, 0x99, 0x59, 0x00, 0x00, 0x00, 0x00, 0x68, 0x68}} /**< Used vendor specific UUID. */



static uint32_t config_char_add(ble_jw_dfu_t * p_jw_dfu, const ble_jw_dfu_init_t * p_jw_dfu_init)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.notify        = 1;
    char_md.char_props.write         = 1;
    char_md.p_char_user_desc         = NULL;
    char_md.p_char_pf                = NULL;
    char_md.p_user_desc_md           = NULL;
    char_md.p_cccd_md                = NULL;
    char_md.p_sccd_md                = NULL;

    ble_uuid.type = p_jw_dfu->uuid_type;
    ble_uuid.uuid = BLE_UUID_JW_DFU_CONFIG_CHAR;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

    attr_md.vloc    = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 0;
    attr_md.vlen    = 1;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = 1;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = BLE_JW_DFU_MAX_DATA_LEN;

    return sd_ble_gatts_characteristic_add(p_jw_dfu->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_jw_dfu->config_handles);
}
static uint32_t control_char_add(ble_jw_dfu_t * p_jw_dfu, const ble_jw_dfu_init_t * p_jw_dfu_init)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.notify        = 1;
    char_md.char_props.write         = 1;
    char_md.p_char_user_desc         = NULL;
    char_md.p_char_pf                = NULL;
    char_md.p_user_desc_md           = NULL;
    char_md.p_cccd_md                = NULL;
    char_md.p_sccd_md                = NULL;

    ble_uuid.type = p_jw_dfu->uuid_type;
    ble_uuid.uuid = BLE_UUID_JW_DFU_CONTROL_CHAR;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

    attr_md.vloc    = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 0;
    attr_md.vlen    = 1;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = 1;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = BLE_JW_DFU_MAX_DATA_LEN;

    return sd_ble_gatts_characteristic_add(p_jw_dfu->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_jw_dfu->control_handles);
}

uint32_t ble_jw_dfu_init(ble_jw_dfu_t * p_jw_dfu, const ble_jw_dfu_init_t * p_jw_dfu_init)
{
    uint32_t      err_code;
    ble_uuid_t    ble_uuid;
    ble_uuid128_t jw_dfu_base_uuid = JW_DFU_BASE_UUID;

    // Initialize the service structure.
    p_jw_dfu->conn_handle                  = BLE_CONN_HANDLE_INVALID;
    p_jw_dfu->evt_handler                  = p_jw_dfu_init->evt_handler;

    // Add a custom base UUID.
    err_code = sd_ble_uuid_vs_add(&jw_dfu_base_uuid, &p_jw_dfu->uuid_type);
    APP_ERROR_CHECK(err_code);

    ble_uuid.type = p_jw_dfu->uuid_type;
    ble_uuid.uuid = BLE_UUID_JW_DFU_SERVICE;

    // Add the service.
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &ble_uuid,
                                        &p_jw_dfu->service_handle);
    APP_ERROR_CHECK(err_code);


    err_code = config_char_add(p_jw_dfu, p_jw_dfu_init);
    APP_ERROR_CHECK(err_code);
    err_code = control_char_add(p_jw_dfu, p_jw_dfu_init);
    APP_ERROR_CHECK(err_code);

    return NRF_SUCCESS;
}



static void on_connect(ble_jw_dfu_t * p_jw_dfu, ble_evt_t const * p_ble_evt)
{
    p_jw_dfu->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
	
}

static void on_disconnect(ble_jw_dfu_t * p_jw_dfu, ble_evt_t const * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_jw_dfu->conn_handle = BLE_CONN_HANDLE_INVALID;
}




static void on_write(ble_jw_dfu_t * p_jw_dfu, ble_evt_t const * p_ble_evt)
{
	ble_gatts_evt_write_t const * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
	ble_jw_dfu_evt_t   evt;
	
	if ((p_evt_write->handle == p_jw_dfu->config_handles.value_handle)
					 && (p_jw_dfu->evt_handler != NULL))
	{
			evt.params.data.p_data = p_evt_write->data;
			evt.params.data.length = p_evt_write->len;
			evt.type = BLE_JW_DFU_EVT_CONFIG_DATA;
			p_jw_dfu->evt_handler(&evt);
	}	
	else if ( (p_evt_write->handle == p_jw_dfu->config_handles.cccd_handle)
			&& (p_evt_write->len == 2))
	{
			if (ble_srv_is_notification_enabled(p_evt_write->data))
			{
					p_jw_dfu->is_config_notification_enabled = true;
					evt.type = BLE_JW_DFU_EVT_CONFIG_NOTIFY_ENABLE;
			}
			else
			{
					p_jw_dfu->is_config_notification_enabled = false;
					evt.type = BLE_JW_DFU_EVT_CONFIG_NOTIFY_DISABLE;
			}
			p_jw_dfu->evt_handler(&evt);
	}	
	else if ((p_evt_write->handle == p_jw_dfu->control_handles.value_handle)
					 && (p_jw_dfu->evt_handler != NULL))
	{
			evt.params.data.p_data = p_evt_write->data;
			evt.params.data.length = p_evt_write->len;
			evt.type = BLE_JW_DFU_EVT_CONTROL_DATA;
			p_jw_dfu->evt_handler(&evt);
	}	
	else if ( (p_evt_write->handle == p_jw_dfu->control_handles.cccd_handle)
			&& (p_evt_write->len == 2))
	{
			if (ble_srv_is_notification_enabled(p_evt_write->data))
			{
					p_jw_dfu->is_control_notification_enabled = true;
					evt.type = BLE_JW_DFU_EVT_CONTROL_NOTIFY_ENABLE;
			}
			else
			{
					p_jw_dfu->is_control_notification_enabled = false;
					evt.type = BLE_JW_DFU_EVT_CONTROL_NOTIFY_DISABLE;
			}
			p_jw_dfu->evt_handler(&evt);
	}	
	else if ((p_evt_write->handle == p_jw_dfu->flash_handles.value_handle)
					 && (p_jw_dfu->evt_handler != NULL))
	{
			evt.params.data.p_data = p_evt_write->data;
			evt.params.data.length = p_evt_write->len;
			evt.type = BLE_JW_DFU_EVT_FLASH_DATA;
			p_jw_dfu->evt_handler(&evt);
	}	
	else if ( (p_evt_write->handle == p_jw_dfu->flash_handles.cccd_handle)
			&& (p_evt_write->len == 2))
	{
			if (ble_srv_is_notification_enabled(p_evt_write->data))
			{
					p_jw_dfu->is_flash_notification_enabled = true;
					evt.type = BLE_JW_DFU_EVT_FLASH_NOTIFY_ENABLE;
			}
			else
			{
					p_jw_dfu->is_flash_notification_enabled = false;
					evt.type = BLE_JW_DFU_EVT_FLASH_NOTIFY_DISABLE;
			}
			p_jw_dfu->evt_handler(&evt);
	}		
	
	
	
}

static void on_hvc(ble_evt_t const * p_ble_evt)
{
	

}


void ble_jw_dfu_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context)
{
    if ((p_context == NULL) || (p_ble_evt == NULL))
    {
        return;
    }
		ble_jw_dfu_t * p_jw_dfu = (ble_jw_dfu_t *)p_context;
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_jw_dfu, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_jw_dfu, p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
            on_write(p_jw_dfu, p_ble_evt);
            break;
        case BLE_GATTS_EVT_HVC:{
					on_hvc(p_ble_evt);				
				
				}break;				
				
				
        case BLE_GATTS_EVT_HVN_TX_COMPLETE:
        {
            //notify with empty data that some tx was completed.
            ble_jw_dfu_evt_t evt = {
                    .type = BLE_JW_DFU_EVT_TX_RDY,
                    .p_jw_dfu = p_jw_dfu
            };
            p_jw_dfu->evt_handler(&evt);
            break;
        }
        default:
            // No implementation needed.
            break;
    }

}











