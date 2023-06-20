#include "ble_srv_wechat.h"
#include "ble_srv_common.h"



#define LOG_LEVEL_OFF 							0
#define LOG_LEVEL_ERROR 						1
#define LOG_LEVEL_WARNING 					2
#define LOG_LEVEL_INFO 							3
#define LOG_LEVEL_DEBUG 						4
#define NRF_LOG_MODULE_NAME ble_srv_wechat
#define NRF_LOG_LEVEL       LOG_LEVEL_OFF
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
NRF_LOG_MODULE_REGISTER();


extern uint8_t	g_adv_manuf_mac[BLE_GAP_ADDR_LEN];

static void on_connect(ble_wechat_t * p_wechat, ble_evt_t const * p_ble_evt)
{
	p_wechat->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
	
}
static void on_disconnect(ble_wechat_t * p_wechat, ble_evt_t const * p_ble_evt)
{
	UNUSED_PARAMETER(p_ble_evt);
	p_wechat->conn_handle = BLE_CONN_HANDLE_INVALID;
	ble_wechat_evt_t evt;
	evt.type = BLE_WECHAT_EVT_DISCONNECT;
	p_wechat->data_handler(&evt);
}


static void on_write(ble_wechat_t * p_wechat, ble_evt_t const * p_ble_evt)
{
    ble_gatts_evt_write_t const * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
    ble_wechat_evt_t evt;	
	
	
    evt.p_wechat = p_wechat;
    if (   (p_evt_write->handle == p_wechat->cpm_handles.cccd_handle)
        && (p_evt_write->len == 2))
    {
        if (ble_srv_is_notification_enabled(p_evt_write->data))
        {
            p_wechat->is_notification_enabled = true;
            evt.type = BLE_WECHAT_EVT_PEDO_MEAS_NOTIFY_ENABLE;
        }
        else
        {
            p_wechat->is_notification_enabled = false;
            evt.type = BLE_WECHAT_EVT_PEDO_MEAS_NOTIFY_DISABLE;
        }
        p_wechat->data_handler(&evt);
    }
    else if (   (p_evt_write->handle == p_wechat->target_handles.value_handle)
             && (p_wechat->data_handler != NULL))
    {
        evt.params.rx_data.p_data = p_evt_write->data;
        evt.params.rx_data.length = p_evt_write->len;
        evt.type = BLE_WECHAT_EVT_TARGET_SET;
        p_wechat->data_handler(&evt);
    }
    else if (   (p_evt_write->handle == p_wechat->target_handles.cccd_handle)
        && (p_evt_write->len == 2))
    {
        if (ble_srv_is_indication_enabled(p_evt_write->data))
        {
            p_wechat->is_indicate_enabled = true;
						evt.type = BLE_WECHAT_EVT_TARGET_INDICATE_ENABLE;
        }
        else
        {
            p_wechat->is_indicate_enabled = false;
						evt.type = BLE_WECHAT_EVT_TARGET_INDICATE_DISABLE;
        }
				p_wechat->data_handler(&evt);
    }			
    else
    {
        // Do Nothing. This event is not relevant for this service.
    }
}



static uint32_t current_pedometer_measurement_char_add(ble_wechat_t * p_wechat, const ble_wechat_init_t * p_wechat_init)
{
    /**@snippet [Adding proprietary characteristic to the SoftDevice] */
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

	
    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);

    cccd_md.vloc = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));
		char_md.char_props.read   = 1;
    char_md.char_props.notify = 1;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md;
    char_md.p_sccd_md         = NULL;

		BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_WECHAT_CURRENT_PEDOMETER_MEASUREMENT_CHARACTERISTICS);

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

    attr_md.vloc    = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 1;
    attr_md.wr_auth = 0;
    attr_md.vlen    = 1;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = 0;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = BLE_WECHAT_MAX_DATA_LEN;

    return sd_ble_gatts_characteristic_add(p_wechat->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_wechat->cpm_handles);
    /**@snippet [Adding proprietary characteristic to the SoftDevice] */
}

static uint32_t read_char_add(ble_wechat_t * p_wechat, const ble_wechat_init_t * p_wechat_init)
{
    /**@snippet [Adding proprietary characteristic to the SoftDevice] */
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

	
    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);

    cccd_md.vloc = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));
		char_md.char_props.read   = 1;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md;
    char_md.p_sccd_md         = NULL;

		BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_WECHAT_READ_CHARACTERISTICS);

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
		attr_char_value.init_len     = sizeof(g_adv_manuf_mac);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = BLE_WECHAT_MAX_DATA_LEN;
		attr_char_value.p_value   = g_adv_manuf_mac;

    return sd_ble_gatts_characteristic_add(p_wechat->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_wechat->read_handles);
    /**@snippet [Adding proprietary characteristic to the SoftDevice] */
}
static uint32_t target_char_add(ble_wechat_t * p_wechat, const ble_wechat_init_t * p_wechat_init)
{
    /**@snippet [Adding proprietary characteristic to the SoftDevice] */
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);

    cccd_md.vloc = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));
		char_md.char_props.read    = 1;
		char_md.char_props.write   = 1;
		char_md.char_props.indicate   = 1;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md;
    char_md.p_sccd_md         = NULL;

		BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_WECHAT_TARGET_CHARACTERISTICS);

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
		attr_char_value.init_len  = sizeof(uint8_t);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = BLE_WECHAT_MAX_DATA_LEN;

    return sd_ble_gatts_characteristic_add(p_wechat->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_wechat->target_handles);
    /**@snippet [Adding proprietary characteristic to the SoftDevice] */
}
uint32_t ble_wechat_init(ble_wechat_t * p_wechat, ble_wechat_init_t const * p_wechat_init)
{
    uint32_t      err_code;
    ble_uuid_t    ble_uuid;

    // Initialize the service structure.
    p_wechat->conn_handle             = BLE_CONN_HANDLE_INVALID;
    p_wechat->data_handler            = p_wechat_init->data_handler;
    p_wechat->is_notification_enabled = false;

    // Add service
    BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_WECHAT_SERVICE);

    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &ble_uuid,
                                        &p_wechat->service_handle);	
	
	
    APP_ERROR_CHECK(err_code);
		
    err_code = current_pedometer_measurement_char_add(p_wechat, p_wechat_init);
    APP_ERROR_CHECK(err_code);		
		
    err_code = read_char_add(p_wechat, p_wechat_init);
    APP_ERROR_CHECK(err_code);		
		
    err_code = target_char_add(p_wechat, p_wechat_init);
    APP_ERROR_CHECK(err_code);			
		
    return NRF_SUCCESS;
}


void ble_wechat_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context)
{
    if ((p_context == NULL) || (p_ble_evt == NULL))
    {
        return;
    }
		
		ble_wechat_t * p_wechat = (ble_wechat_t *)p_context;
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
						NRF_LOG_DEBUG("BLE_GAP_EVT_CONNECTED");
						on_connect(p_wechat, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
						NRF_LOG_DEBUG("BLE_GAP_EVT_DISCONNECTED");
						on_disconnect(p_wechat, p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
						NRF_LOG_DEBUG("BLE_GATTS_EVT_WRITE");
						on_write(p_wechat, p_ble_evt);
            break;
        		
        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:{
                        NRF_LOG_DEBUG("BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST");
                        ble_wechat_evt_t evt = {
                            .type = BLE_WECHAT_EVT_READ_DATA,
                            .p_wechat = p_wechat
                        };
                        p_wechat->data_handler(&evt);
            }break;			


				
        case BLE_GATTS_EVT_HVN_TX_COMPLETE:{
                        NRF_LOG_DEBUG("BLE_GATTS_EVT_HVN_TX_COMPLETE");
                        //notify with empty data that some tx was completed.
                        ble_wechat_evt_t evt = {
                            .type = BLE_WECHAT_EVT_RX_DATA,
                            .p_wechat = p_wechat
                        };
                        p_wechat->data_handler(&evt);
            }break;
        
        default:
            // No implementation needed.
            break;
    }
}

uint32_t ble_wechat_pedo_meas_send(ble_wechat_t * p_wechat, uint8_t * p_string, uint16_t * p_length)
{
	#if 0
    ble_gatts_hvx_params_t hvx_params;
		VERIFY_PARAM_NOT_NULL(p_wechat);
    if ((p_wechat->conn_handle == BLE_CONN_HANDLE_INVALID) || (!p_wechat->is_notification_enabled))
    {
			NRF_LOG_DEBUG("NRF_ERROR_INVALID_STATE");
        return NRF_ERROR_INVALID_STATE;
    }

    if (*p_length > BLE_WECHAT_MAX_DATA_LEN)
    {
			NRF_LOG_DEBUG("NRF_ERROR_INVALID_PARAM");
			NRF_LOG_DEBUG("p_length --> %d",*p_length);
			NRF_LOG_DEBUG("BLE_NRF52_MAX_DATA_LEN --> %d",BLE_WECHAT_MAX_DATA_LEN);			
        return NRF_ERROR_INVALID_PARAM;
    }
		
    memset(&hvx_params, 0, sizeof(hvx_params));

    hvx_params.handle = p_wechat->cpm_handles.value_handle;
    hvx_params.p_data = p_string;
    hvx_params.p_len  = p_length;
    hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;

    return sd_ble_gatts_hvx(p_wechat->conn_handle, &hvx_params);
	#endif
	ble_gatts_rw_authorize_reply_params_t rw_authorize_reply_params;
	VERIFY_PARAM_NOT_NULL(p_wechat);
    if (p_wechat->conn_handle == BLE_CONN_HANDLE_INVALID)
    {
		NRF_LOG_DEBUG("NRF_ERROR_INVALID_STATE");
		return NRF_ERROR_INVALID_STATE;
    }

    if (*p_length > BLE_WECHAT_MAX_DATA_LEN)
    {
		NRF_LOG_DEBUG("NRF_ERROR_INVALID_PARAM");
		NRF_LOG_DEBUG("p_length --> %d",*p_length);
		NRF_LOG_DEBUG("BLE_NRF52_MAX_DATA_LEN --> %d",BLE_WECHAT_MAX_DATA_LEN);			
		return NRF_ERROR_INVALID_PARAM;
    }
		
    memset(&rw_authorize_reply_params, 0, sizeof(rw_authorize_reply_params));
	rw_authorize_reply_params.type = BLE_GATTS_AUTHORIZE_TYPE_READ;
	rw_authorize_reply_params.params.read.gatt_status = BLE_GATT_STATUS_SUCCESS;
	rw_authorize_reply_params.params.read.update = 1;
	rw_authorize_reply_params.params.read.offset = 0;
	rw_authorize_reply_params.params.read.len = *p_length;
	rw_authorize_reply_params.params.read.p_data = p_string;
	return sd_ble_gatts_rw_authorize_reply(p_wechat->conn_handle, &rw_authorize_reply_params);
	
}
uint32_t ble_wechat_target_send(ble_wechat_t * p_wechat, uint8_t * p_string, uint16_t * p_length)
{
    ble_gatts_hvx_params_t hvx_params;

    VERIFY_PARAM_NOT_NULL(p_wechat);

    if ((p_wechat->conn_handle == BLE_CONN_HANDLE_INVALID) || (!p_wechat->is_indicate_enabled))
    {
			NRF_LOG_DEBUG("NRF_ERROR_INVALID_STATE");
        return NRF_ERROR_INVALID_STATE;
    }

    if (*p_length > BLE_WECHAT_MAX_DATA_LEN)
    {
			NRF_LOG_DEBUG("NRF_ERROR_INVALID_PARAM");
			NRF_LOG_DEBUG("p_length --> %d",*p_length);
			NRF_LOG_DEBUG("BLE_NRF52_MAX_DATA_LEN --> %d",BLE_WECHAT_MAX_DATA_LEN);
        return NRF_ERROR_INVALID_PARAM;
    }

    memset(&hvx_params, 0, sizeof(hvx_params));

    hvx_params.handle = p_wechat->target_handles.value_handle;
    hvx_params.p_data = p_string;
    hvx_params.p_len  = p_length;
    hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;

    return sd_ble_gatts_hvx(p_wechat->conn_handle, &hvx_params);		
}









