#include "ble_srv_nrf52.h"


#define LOG_LEVEL_OFF 							0
#define LOG_LEVEL_ERROR 						1
#define LOG_LEVEL_WARNING 					2
#define LOG_LEVEL_INFO 							3
#define LOG_LEVEL_DEBUG 						4
#define NRF_LOG_MODULE_NAME ble_srv_nrf52
#define NRF_LOG_LEVEL       LOG_LEVEL_OFF
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
NRF_LOG_MODULE_REGISTER();






#define BLE_UUID_NRF52_CONFIG_CHAR   0x0002                           
#define BLE_UUID_NRF52_CONTROL_CHAR   0x0003  

#define NRF52_BASE_UUID  {{0x00, 0x00, 0x00, 0x01, 0x32, 0x4B, 0x4E, 0x48, 0x49, 0x50, 0x00, 0x00, 0x00, 0x00, 0x68, 0x68}} /**< Used vendor specific UUID. */

static void on_connect(ble_nrf52_t * p_nrf52, ble_evt_t const * p_ble_evt)
{
    p_nrf52->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
		ble_nrf52_evt_t evt;
		evt.type = BLE_NRF52_EVT_CONNECTED;
		p_nrf52->evt_handler(&evt);	
}

static void on_disconnect(ble_nrf52_t * p_nrf52, ble_evt_t const * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_nrf52->conn_handle = BLE_CONN_HANDLE_INVALID;
		ble_nrf52_evt_t evt;
		evt.type = BLE_NRF52_EVT_DISCONNECTED;
		p_nrf52->evt_handler(&evt);		
}




static void on_write(ble_nrf52_t * p_nrf52, ble_evt_t const * p_ble_evt)
{
	ble_gatts_evt_write_t const * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
	ble_nrf52_evt_t   evt;
	
	if ((p_evt_write->handle == p_nrf52->config_handles.value_handle)
					 && (p_nrf52->evt_handler != NULL))
	{
			evt.params.data.p_data = p_evt_write->data;
			evt.params.data.length = p_evt_write->len;
			evt.type = BLE_NRF52_EVT_CONFIG_DATA;
			p_nrf52->evt_handler(&evt);
	}	
	else if ( (p_evt_write->handle == p_nrf52->config_handles.cccd_handle)
			&& (p_evt_write->len == 2))
	{
			if (ble_srv_is_notification_enabled(p_evt_write->data))
			{
					p_nrf52->is_config_notification_enabled = true;
					evt.type = BLE_NRF52_EVT_CONFIG_NOTIFY_ENABLE;
			}
			else
			{
					p_nrf52->is_config_notification_enabled = false;
					evt.type = BLE_NRF52_EVT_CONFIG_NOTIFY_DISABLE;
			}
			p_nrf52->evt_handler(&evt);
	}	
	else if ((p_evt_write->handle == p_nrf52->control_handles.value_handle)
					 && (p_nrf52->evt_handler != NULL))
	{
			evt.params.data.p_data = p_evt_write->data;
			evt.params.data.length = p_evt_write->len;
			evt.type = BLE_NRF52_EVT_CONTROL_DATA;
			p_nrf52->evt_handler(&evt);
	}	
	else if ( (p_evt_write->handle == p_nrf52->control_handles.cccd_handle)
			&& (p_evt_write->len == 2))
	{
			if (ble_srv_is_notification_enabled(p_evt_write->data))
			{
					p_nrf52->is_control_notification_enabled = true;
					evt.type = BLE_NRF52_EVT_CONTROL_NOTIFY_ENABLE;
			}
			else
			{
					p_nrf52->is_control_notification_enabled = false;
					evt.type = BLE_NRF52_EVT_CONTROL_NOTIFY_DISABLE;
			}
			p_nrf52->evt_handler(&evt);
	}	
}

static void on_hvc(ble_evt_t const * p_ble_evt)
{
	NRF_LOG_DEBUG("on_hvc");

}

void ble_nrf52_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context)
{
    if ((p_context == NULL) || (p_ble_evt == NULL))
    {
        return;
    }
		ble_nrf52_t * p_nrf52 = (ble_nrf52_t *)p_context;
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
					NRF_LOG_DEBUG("BLE_GAP_EVT_CONNECTED");
            on_connect(p_nrf52, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
					NRF_LOG_DEBUG("BLE_GAP_EVT_DISCONNECTED");
            on_disconnect(p_nrf52, p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
					NRF_LOG_DEBUG("BLE_GATTS_EVT_WRITE");
            on_write(p_nrf52, p_ble_evt);
            break;
        case BLE_GATTS_EVT_HVC:{
					NRF_LOG_DEBUG("BLE_GATTS_EVT_HVC");
					on_hvc(p_ble_evt);				
				
				}break;				
				
				
        case BLE_GATTS_EVT_HVN_TX_COMPLETE:
        {
					NRF_LOG_DEBUG("BLE_GATTS_EVT_HVN_TX_COMPLETE");
            //notify with empty data that some tx was completed.
            ble_nrf52_evt_t evt = {
                    .type = BLE_NRF52_EVT_TX_RDY,
                    .p_nrf52 = p_nrf52
            };
            p_nrf52->evt_handler(&evt);
            break;
        }
        default:
            // No implementation needed.
            break;
    }
}


static uint32_t config_char_add(ble_nrf52_t * p_nrf52, const ble_nrf52_init_t * p_nrf52_init)
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

    ble_uuid.type = p_nrf52->uuid_type;
    ble_uuid.uuid = BLE_UUID_NRF52_CONFIG_CHAR;

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
    attr_char_value.max_len   = BLE_NRF52_MAX_DATA_LEN;

    return sd_ble_gatts_characteristic_add(p_nrf52->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_nrf52->config_handles);
}
static uint32_t control_char_add(ble_nrf52_t * p_nrf52, const ble_nrf52_init_t * p_nrf52_init)
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

    ble_uuid.type = p_nrf52->uuid_type;
    ble_uuid.uuid = BLE_UUID_NRF52_CONTROL_CHAR;

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
    attr_char_value.max_len   = BLE_NRF52_MAX_DATA_LEN;

    return sd_ble_gatts_characteristic_add(p_nrf52->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_nrf52->control_handles);
}
uint32_t ble_nrf52_init(ble_nrf52_t * p_nrf52, const ble_nrf52_init_t * p_nrf52_init)
{
    uint32_t      err_code;
    ble_uuid_t    ble_uuid;
    ble_uuid128_t nrf52_base_uuid = NRF52_BASE_UUID;

    VERIFY_PARAM_NOT_NULL(p_nrf52);
    VERIFY_PARAM_NOT_NULL(p_nrf52_init);

    // Initialize the service structure.
    p_nrf52->conn_handle                  = BLE_CONN_HANDLE_INVALID;
    p_nrf52->evt_handler                  = p_nrf52_init->evt_handler;

    // Add a custom base UUID.
    err_code = sd_ble_uuid_vs_add(&nrf52_base_uuid, &p_nrf52->uuid_type);
    APP_ERROR_CHECK(err_code);

    ble_uuid.type = p_nrf52->uuid_type;
    ble_uuid.uuid = BLE_UUID_NRF52_SERVICE;

    // Add the service.
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &ble_uuid,
                                        &p_nrf52->service_handle);
    APP_ERROR_CHECK(err_code);


    err_code = config_char_add(p_nrf52, p_nrf52_init);
    APP_ERROR_CHECK(err_code);
    err_code = control_char_add(p_nrf52, p_nrf52_init);
    APP_ERROR_CHECK(err_code);

    return NRF_SUCCESS;
}



uint32_t ble_srv_nrf52_config_string_send(ble_nrf52_t * p_nrf52, uint8_t * p_string, uint16_t * p_length)
{
    ble_gatts_hvx_params_t hvx_params;

    VERIFY_PARAM_NOT_NULL(p_nrf52);

    if ((p_nrf52->conn_handle == BLE_CONN_HANDLE_INVALID) || (!p_nrf52->is_config_notification_enabled))
    {
			NRF_LOG_DEBUG("NRF_ERROR_INVALID_STATE");
        return NRF_ERROR_INVALID_STATE;
    }

    if (*p_length > BLE_NRF52_MAX_DATA_LEN)
    {
			NRF_LOG_DEBUG("NRF_ERROR_INVALID_PARAM");
			NRF_LOG_DEBUG("p_length --> %d",*p_length);
			NRF_LOG_DEBUG("BLE_NRF52_MAX_DATA_LEN --> %d",BLE_NRF52_MAX_DATA_LEN);
        return NRF_ERROR_INVALID_PARAM;
    }

    memset(&hvx_params, 0, sizeof(hvx_params));

    hvx_params.handle = p_nrf52->config_handles.value_handle;
    hvx_params.p_data = p_string;
    hvx_params.p_len  = p_length;
    hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;

    return sd_ble_gatts_hvx(p_nrf52->conn_handle, &hvx_params);
}

uint32_t ble_srv_nrf52_control_string_send(ble_nrf52_t * p_nrf52, uint8_t * p_string, uint16_t * p_length)
{
    ble_gatts_hvx_params_t hvx_params;

    VERIFY_PARAM_NOT_NULL(p_nrf52);

    if ((p_nrf52->conn_handle == BLE_CONN_HANDLE_INVALID) || (!p_nrf52->is_control_notification_enabled))
    {
			NRF_LOG_DEBUG("NRF_ERROR_INVALID_STATE");
        return NRF_ERROR_INVALID_STATE;
    }

    if (*p_length > BLE_NRF52_MAX_DATA_LEN)
    {
			NRF_LOG_DEBUG("NRF_ERROR_INVALID_PARAM");
			NRF_LOG_DEBUG("p_length --> %d",*p_length);
			NRF_LOG_DEBUG("BLE_NRF52_MAX_DATA_LEN --> %d",BLE_NRF52_MAX_DATA_LEN);
        return NRF_ERROR_INVALID_PARAM;
    }

    memset(&hvx_params, 0, sizeof(hvx_params));

    hvx_params.handle = p_nrf52->control_handles.value_handle;
    hvx_params.p_data = p_string;
    hvx_params.p_len  = p_length;
    hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;

    return sd_ble_gatts_hvx(p_nrf52->conn_handle, &hvx_params);
}




