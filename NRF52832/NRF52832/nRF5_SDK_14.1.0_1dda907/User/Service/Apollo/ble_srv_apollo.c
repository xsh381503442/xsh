#include "ble_srv_apollo.h"






#define LOG_LEVEL_OFF 							0
#define LOG_LEVEL_ERROR 						1
#define LOG_LEVEL_WARNING 					2
#define LOG_LEVEL_INFO 							3
#define LOG_LEVEL_DEBUG 						4
#define NRF_LOG_MODULE_NAME ble_srv_apollo
#define NRF_LOG_LEVEL       LOG_LEVEL_OFF
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
NRF_LOG_MODULE_REGISTER();


                
#define BLE_UUID_APOLLO_UART_CHARACTERISTIC          0x0002  
#define BLE_UUID_APOLLO_FLASH_CHARACTERISTIC         0x0003  
#define BLE_UUID_APOLLO_SENSOR_CHARACTERISTIC        0x0004 

       
#define BLE_UART_MAX_APOLLO_UART_CHAR_LEN             BLE_APOLLO_MAX_DATA_LEN   

#define APOLLO_BASE_UUID {{0x00, 0x00, 0x00, 0x02, 0x32, 0x4B, 0x4E, 0x48, 0x49, 0x50, 0x00, 0x00, 0x00, 0x00, 0x68, 0x68}}  


static void on_connect(ble_apollo_t * p_apollo, ble_evt_t const * p_ble_evt)
{
    p_apollo->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;

	
	
		ble_apollo_evt_t evt;
		evt.type = BLE_EVT_APOLLO_CONNECTED;
		p_apollo->data_handler(&evt);
}

static void on_disconnect(ble_apollo_t * p_apollo, ble_evt_t const * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_apollo->conn_handle = BLE_CONN_HANDLE_INVALID;
		ble_apollo_evt_t evt;
		evt.type = BLE_EVT_APOLLO_DISCONNECTED;
		p_apollo->data_handler(&evt);	
}

static void on_write(ble_apollo_t * p_apollo, ble_evt_t const * p_ble_evt)
{
    ble_gatts_evt_write_t const * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
    ble_apollo_evt_t evt;
    evt.p_apollo = p_apollo;
    if (   (p_evt_write->handle == p_apollo->uart_handles.cccd_handle)
        && (p_evt_write->len == 2))
    {
        if (ble_srv_is_notification_enabled(p_evt_write->data))
        {
            p_apollo->is_uart_notification_enabled = true;
            evt.type = BLE_EVT_APOLLO_UART_NOTIFY_ENABLE;
        }
        else
        {
            p_apollo->is_uart_notification_enabled = false;
            evt.type = BLE_EVT_APOLLO_UART_NOTIFY_DISABLE;
        }
        p_apollo->data_handler(&evt);
    }
    else if (   (p_evt_write->handle == p_apollo->uart_handles.value_handle)
             && (p_apollo->data_handler != NULL))
    {
        evt.params.data.p_data = p_evt_write->data;
        evt.params.data.length = p_evt_write->len;
        evt.type = BLE_EVT_APOLLO_UART_RX_DATA;
        p_apollo->data_handler(&evt);
    }
		
	else if (   (p_evt_write->handle == p_apollo->flash_handles.cccd_handle)
        && (p_evt_write->len == 2))
    {
        if (ble_srv_is_notification_enabled(p_evt_write->data))
        {
            p_apollo->is_flash_notification_enabled = true;
            evt.type = BLE_EVT_APOLLO_FLASH_NOTIFY_ENABLE;
        }
        else
        {
            p_apollo->is_flash_notification_enabled = false;
            evt.type = BLE_EVT_APOLLO_FLASH_NOTIFY_DISABLE;
        }
        p_apollo->data_handler(&evt);
    }
    else if (   (p_evt_write->handle == p_apollo->flash_handles.value_handle)
             && (p_apollo->data_handler != NULL))
    {
        evt.params.data.p_data = p_evt_write->data;
        evt.params.data.length = p_evt_write->len;
        evt.type = BLE_EVT_APOLLO_FLASH_DATA;
        p_apollo->data_handler(&evt);
    }				
	else if (   (p_evt_write->handle == p_apollo->sensor_handles.cccd_handle)
        && (p_evt_write->len == 2))
    {
        if (ble_srv_is_notification_enabled(p_evt_write->data))
        {
            p_apollo->is_sensor_notification_enabled = true;
            evt.type = BLE_EVT_APOLLO_SENSOR_NOTIFY_ENABLE;
        }
        else
        {
            p_apollo->is_sensor_notification_enabled = false;
            evt.type = BLE_EVT_APOLLO_SENSOR_NOTIFY_DISABLE;
        }
        p_apollo->data_handler(&evt);
    }
    else if (   (p_evt_write->handle == p_apollo->sensor_handles.value_handle)
             && (p_apollo->data_handler != NULL))
    {
        evt.params.data.p_data = p_evt_write->data;
        evt.params.data.length = p_evt_write->len;
        evt.type = BLE_EVT_APOLLO_SENSOR_DATA;
        p_apollo->data_handler(&evt);
    }		
    else
    {
        // Do Nothing. This event is not relevant for this service.
    }
}

static uint32_t uart_char_add(ble_apollo_t * p_apollo, const ble_apollo_init_t * p_apollo_init)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.write         = 1;
		char_md.char_props.notify        = 1;
    char_md.p_char_user_desc         = NULL;
    char_md.p_char_pf                = NULL;
    char_md.p_user_desc_md           = NULL;
    char_md.p_cccd_md                = NULL;
    char_md.p_sccd_md                = NULL;

    ble_uuid.type = p_apollo->uuid_type;
    ble_uuid.uuid = BLE_UUID_APOLLO_UART_CHARACTERISTIC;

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
    attr_char_value.max_len   = NRF_SDH_BLE_GATT_MAX_MTU_SIZE;

    return sd_ble_gatts_characteristic_add(p_apollo->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_apollo->uart_handles);
}
static uint32_t flash_char_add(ble_apollo_t * p_apollo, const ble_apollo_init_t * p_apollo_init)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.write         = 1;
		char_md.char_props.notify        = 1;
    char_md.p_char_user_desc         = NULL;
    char_md.p_char_pf                = NULL;
    char_md.p_user_desc_md           = NULL;
    char_md.p_cccd_md                = NULL;
    char_md.p_sccd_md                = NULL;

    ble_uuid.type = p_apollo->uuid_type;
    ble_uuid.uuid = BLE_UUID_APOLLO_FLASH_CHARACTERISTIC;

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
    attr_char_value.max_len   = NRF_SDH_BLE_GATT_MAX_MTU_SIZE;

    return sd_ble_gatts_characteristic_add(p_apollo->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_apollo->flash_handles);
}

static uint32_t sensor_char_add(ble_apollo_t * p_apollo, const ble_apollo_init_t * p_apollo_init)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.write         = 1;
		char_md.char_props.notify        = 1;
    char_md.p_char_user_desc         = NULL;
    char_md.p_char_pf                = NULL;
    char_md.p_user_desc_md           = NULL;
    char_md.p_cccd_md                = NULL;
    char_md.p_sccd_md                = NULL;

    ble_uuid.type = p_apollo->uuid_type;
    ble_uuid.uuid = BLE_UUID_APOLLO_SENSOR_CHARACTERISTIC;

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
    attr_char_value.max_len   = NRF_SDH_BLE_GATT_MAX_MTU_SIZE;

    return sd_ble_gatts_characteristic_add(p_apollo->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_apollo->sensor_handles);
}


void ble_apollo_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context)
{
    if ((p_context == NULL) || (p_ble_evt == NULL))
    {
        return;
    }

    ble_apollo_t * p_apollo = (ble_apollo_t *)p_context;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
					NRF_LOG_DEBUG("BLE_GAP_EVT_CONNECTED");
            on_connect(p_apollo, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
					NRF_LOG_DEBUG("BLE_GAP_EVT_DISCONNECTED");
            on_disconnect(p_apollo, p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
					NRF_LOG_DEBUG("BLE_GATTS_EVT_WRITE");
            on_write(p_apollo, p_ble_evt);
            break;

        case BLE_GATTS_EVT_HVN_TX_COMPLETE:
        {
					NRF_LOG_DEBUG("BLE_GATTS_EVT_HVN_TX_COMPLETE");
            //notify with empty data that some tx was completed.
            ble_apollo_evt_t evt = {
                    .type = BLE_EVT_APOLLO_TX_RDY,
                    .p_apollo = p_apollo
            };
            p_apollo->data_handler(&evt);
            break;
        }
        default:
            // No implementation needed.
            break;
    }
}

uint32_t ble_apollo_init(ble_apollo_t * p_apollo, ble_apollo_init_t const * p_apollo_init)
{
    uint32_t      err_code;
    ble_uuid_t    ble_uuid;
    ble_uuid128_t apollo_base_uuid = APOLLO_BASE_UUID;

    VERIFY_PARAM_NOT_NULL(p_apollo);
    VERIFY_PARAM_NOT_NULL(p_apollo_init);

    // Initialize the service structure.
    p_apollo->conn_handle                  = BLE_CONN_HANDLE_INVALID;
    p_apollo->data_handler                 = p_apollo_init->data_handler;
    p_apollo->is_uart_notification_enabled = false;

    /**@snippet [Adding proprietary Service to the SoftDevice] */
    // Add a custom base UUID.
    err_code = sd_ble_uuid_vs_add(&apollo_base_uuid, &p_apollo->uuid_type);
    VERIFY_SUCCESS(err_code);

    ble_uuid.type = p_apollo->uuid_type;
    ble_uuid.uuid = BLE_UUID_APOLLO_SERVICE;

    // Add the service.
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &ble_uuid,
                                        &p_apollo->service_handle);
    /**@snippet [Adding proprietary Service to the SoftDevice] */
    VERIFY_SUCCESS(err_code);

    // Add the RX Characteristic.
    err_code = uart_char_add(p_apollo, p_apollo_init);
    VERIFY_SUCCESS(err_code);
    err_code = flash_char_add(p_apollo, p_apollo_init);
    VERIFY_SUCCESS(err_code);
	err_code = sensor_char_add(p_apollo, p_apollo_init);
    VERIFY_SUCCESS(err_code);

    return NRF_SUCCESS;
}


uint32_t ble_apollo_uart_string_send(ble_apollo_t * p_apollo, uint8_t * p_string, uint16_t * p_length)
{
    ble_gatts_hvx_params_t hvx_params;

    VERIFY_PARAM_NOT_NULL(p_apollo);

    if ((p_apollo->conn_handle == BLE_CONN_HANDLE_INVALID) || (!p_apollo->is_uart_notification_enabled))
    {
			NRF_LOG_DEBUG("NRF_ERROR_INVALID_STATE");
        return NRF_ERROR_INVALID_STATE;
    }

    if (*p_length > NRF_SDH_BLE_GATT_MAX_MTU_SIZE)
    {
			NRF_LOG_DEBUG("NRF_ERROR_INVALID_PARAM");
			NRF_LOG_DEBUG("p_length --> %d",*p_length);
			NRF_LOG_DEBUG("BLE_UART_MAX_APOLLO_UART_CHAR_LEN --> %d",BLE_UART_MAX_APOLLO_UART_CHAR_LEN);
        return NRF_ERROR_INVALID_PARAM;
    }

    memset(&hvx_params, 0, sizeof(hvx_params));

    hvx_params.handle = p_apollo->uart_handles.value_handle;
    hvx_params.p_data = p_string;
    hvx_params.p_len  = p_length;
    hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;

    return sd_ble_gatts_hvx(p_apollo->conn_handle, &hvx_params);
}

uint32_t ble_apollo_flash_string_send(ble_apollo_t * p_apollo, uint8_t * p_string, uint16_t * p_length)
{
    ble_gatts_hvx_params_t hvx_params;

    VERIFY_PARAM_NOT_NULL(p_apollo);

    if ((p_apollo->conn_handle == BLE_CONN_HANDLE_INVALID) || (!p_apollo->is_flash_notification_enabled))
    {
			NRF_LOG_DEBUG("NRF_ERROR_INVALID_STATE");
        return NRF_ERROR_INVALID_STATE;
    }

    if (*p_length > NRF_SDH_BLE_GATT_MAX_MTU_SIZE)
    {
			NRF_LOG_DEBUG("NRF_ERROR_INVALID_PARAM");
			NRF_LOG_DEBUG("p_length --> %d",*p_length);
			NRF_LOG_DEBUG("BLE_UART_MAX_APOLLO_UART_CHAR_LEN --> %d",BLE_UART_MAX_APOLLO_UART_CHAR_LEN);
        return NRF_ERROR_INVALID_PARAM;
    }

    memset(&hvx_params, 0, sizeof(hvx_params));

    hvx_params.handle = p_apollo->flash_handles.value_handle;
    hvx_params.p_data = p_string;
    hvx_params.p_len  = p_length;
    hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;

    return sd_ble_gatts_hvx(p_apollo->conn_handle, &hvx_params);
}

uint32_t ble_apollo_sensor_string_send(ble_apollo_t * p_apollo, uint8_t * p_string, uint16_t * p_length)
{
    ble_gatts_hvx_params_t hvx_params;

    VERIFY_PARAM_NOT_NULL(p_apollo);

    if ((p_apollo->conn_handle == BLE_CONN_HANDLE_INVALID) || (!p_apollo->is_sensor_notification_enabled))
    {
			NRF_LOG_DEBUG("NRF_ERROR_INVALID_STATE");
        return NRF_ERROR_INVALID_STATE;
    }

    if (*p_length > NRF_SDH_BLE_GATT_MAX_MTU_SIZE)
    {
			NRF_LOG_DEBUG("NRF_ERROR_INVALID_PARAM");
			NRF_LOG_DEBUG("p_length --> %d",*p_length);
			NRF_LOG_DEBUG("BLE_UART_MAX_APOLLO_UART_CHAR_LEN --> %d",BLE_UART_MAX_APOLLO_UART_CHAR_LEN);
        return NRF_ERROR_INVALID_PARAM;
    }

    memset(&hvx_params, 0, sizeof(hvx_params));

    hvx_params.handle = p_apollo->sensor_handles.value_handle;
    hvx_params.p_data = p_string;
    hvx_params.p_len  = p_length;
    hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;

    return sd_ble_gatts_hvx(p_apollo->conn_handle, &hvx_params);
}









