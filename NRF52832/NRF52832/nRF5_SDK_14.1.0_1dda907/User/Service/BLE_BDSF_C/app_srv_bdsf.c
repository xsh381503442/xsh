#include "app_srv_bdsf.h"
//#ifdef WATCH_HAS_LINGHANG_FUNC
#include "ble_bdsf_c.h"

#include "sdk_errors.h"
#include "task_apollo2.h"
#include "task_ble_bdsf.h"
#include "cmd_apollo2.h"
#include "task_ble_conn.h"

#define LOG_LEVEL_OFF 							0
#define LOG_LEVEL_ERROR 						1
#define LOG_LEVEL_WARNING 					2
#define LOG_LEVEL_INFO 							3
#define LOG_LEVEL_DEBUG 						4
#define NRF_LOG_MODULE_NAME app_srv_bdsf
#define NRF_LOG_LEVEL       LOG_LEVEL_INFO
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
NRF_LOG_MODULE_REGISTER();


#define SCAN_INTERVAL                   0x00A0                              /**< Determines scan interval in units of 0.625 millisecond. */
#define SCAN_WINDOW                     0x0050                              /**< Determines scan window in units of 0.625 millisecond. */
#define SCAN_TIMEOUT                    0x0000                              /**< Timout when scanning. 0x0000 disables timeout. */

#define MIN_CONNECTION_INTERVAL         MSEC_TO_UNITS(7.5, UNIT_1_25_MS)    /**< Determines minimum connection interval in milliseconds. */
#define MAX_CONNECTION_INTERVAL         MSEC_TO_UNITS(30, UNIT_1_25_MS)     /**< Determines maximum connection interval in milliseconds. */
#define SLAVE_LATENCY                   0                                   /**< Determines slave latency in terms of connection events. */
#define SUPERVISION_TIMEOUT             MSEC_TO_UNITS(4000, UNIT_10_MS)     /**< Determines supervision time-out in units of 10 milliseconds. */

#define APP_BLE_CONN_CFG_TAG            1                                   /**< A tag identifying the SoftDevice BLE configuration. */


BLE_BDSF_C_DEF(m_ble_bdsf_c);				//±±¶·ÊÕ·¢


static char const m_target_periph_name[] = "Max_Testing";;     /**< Name of the device we try to connect to. This name is searched in the scan report data*/

/**@brief Parameters used when scanning. */
static ble_gap_scan_params_t const m_scan_params =
{
    .active   = 1,
    .interval = SCAN_INTERVAL,
    .window   = SCAN_WINDOW,
    .timeout  = SCAN_TIMEOUT,
    #if (NRF_SD_BLE_API_VERSION <= 2)
        .selective   = 0,
        .p_whitelist = NULL,
    #endif
    #if (NRF_SD_BLE_API_VERSION >= 3)
        .use_whitelist = 0,
    #endif
};

/**@brief Connection parameters requested for connection. */
static ble_gap_conn_params_t const m_connection_param =
{
    (uint16_t)MIN_CONNECTION_INTERVAL,
    (uint16_t)MAX_CONNECTION_INTERVAL,
    (uint16_t)SLAVE_LATENCY,
    (uint16_t)SUPERVISION_TIMEOUT
};

static void bdsf_c_evt_handler(ble_bdsf_c_t * p_ble_bdsf_c, ble_bdsf_c_evt_t * p_ble_bdsf_evt)
{
    ret_code_t err_code;

    switch (p_ble_bdsf_evt->evt_type)
    {
        case BLE_BDSF_C_EVT_DISCOVERY_COMPLETE:
            NRF_LOG_INFO("Discovery complete.");
            err_code = ble_bdsf_c_handles_assign(p_ble_bdsf_c, p_ble_bdsf_evt->conn_handle, &p_ble_bdsf_evt->handles);
            APP_ERROR_CHECK(err_code);
						
						NRF_LOG_INFO("BeiDou r/t machine service discovered on conn_handle 0x%x.", p_ble_bdsf_evt->conn_handle);
						
            err_code = ble_bdsf_c_message_notif_enable(p_ble_bdsf_c);
            APP_ERROR_CHECK(err_code);
						
            NRF_LOG_INFO("Connected to device with BeiDou r/t machine Service.");
						task_ble_conn_handle(TASK_BLE_CONN_EVT_CONNECT_BDSF, NULL, 0);	 
            break;

        case BLE_BDSF_C_EVT_MESSAGE_GET_EVT:
//            NRF_LOG_INFO("bdsf message geted!\n");//ble_nus_chars_received_uart_print(p_ble_nus_evt->p_data, p_ble_nus_evt->data_len);
            break;
				
//				case BLE_BDSF_C_EVT_MESSAGE_SEND_EVT:
//						//send to peer.
//						NRF_LOG_INFO("bdsf message sended!\n");
//						break;

        case BLE_BDSF_C_EVT_DISCONNECTED:
					NRF_LOG_INFO("BLE_BDSF_C_EVT_DISCONNECTED");
					NRF_LOG_INFO("Disconnected.");
					ret_code_t err_code;
					SENDING_DEV_NAME = 1;
					(void) sd_ble_gap_scan_stop();

					err_code = sd_ble_gap_scan_start(&m_scan_params);
					APP_ERROR_CHECK(err_code);

//            scan_start();
						//task_ble_bdsf_handle(TASK_BLE_BDSF_EVT_SCAN_AGAIN, NULL, 0);
            break;
    }
}



/**@brief BeiDou machine client initialization.
 */
void bdsf_c_init(void)
{
    ret_code_t       err_code;
    ble_bdsf_c_init_t bdsf_c_init_obj;

    bdsf_c_init_obj.evt_handler = bdsf_c_evt_handler;

    err_code = ble_bdsf_c_init(&m_ble_bdsf_c, &bdsf_c_init_obj);
    APP_ERROR_CHECK(err_code);
}





/**
 * @brief Parses advertisement data, providing length and location of the field in case
 *        matching data is found.
 *
 * @param[in]  type       Type of data to be looked for in advertisement data.
 * @param[in]  p_advdata  Advertisement report length and pointer to report.
 * @param[out] p_typedata If data type requested is found in the data report, type data length and
 *                        pointer to data will be populated here.
 *
 * @retval NRF_SUCCESS if the data type is found in the report.
 * @retval NRF_ERROR_NOT_FOUND if the data type could not be found.
 */
static uint32_t adv_report_parse(uint8_t type, data_t * p_advdata, data_t * p_typedata)
{
    uint32_t  index = 0;
    uint8_t * p_data;

    p_data = p_advdata->p_data;

    while (index < p_advdata->data_len)
    {
        uint8_t field_length = p_data[index];
        uint8_t field_type   = p_data[index + 1];

        if (field_type == type)
        {
            p_typedata->p_data   = &p_data[index + 2];
            p_typedata->data_len = field_length - 1;
            return NRF_SUCCESS;
        }
        index += field_length + 1;
    }
    return NRF_ERROR_NOT_FOUND;
}


/**@brief Function for searching a UUID in the advertisement packets.
 *
 * @details Use this function to parse received advertising data and to find a given
 * UUID in them.
 *
 * @param[in]   p_adv_report   advertising data to parse.
 * @param[in]   uuid_to_find   UUIID to search.
 * @return   true if the given UUID was found, false otherwise.
 */
uint8_t find_adv_uuid(const ble_gap_evt_adv_report_t *p_adv_report, const uint16_t uuid_to_find)
{
    ret_code_t err_code;
    data_t     adv_data;
    data_t     type_data;

    // Initialize advertisement report for parsing.
    adv_data.p_data     = (uint8_t *)p_adv_report->data;
    adv_data.data_len   = p_adv_report->dlen;
		
//		NRF_LOG_INFO("length --> %d\n", adv_data.p_data[0]);
		NRF_LOG_INFO("type   ### %d\n", adv_data.p_data[1]);
#if 0
    err_code = adv_report_parse(BLE_GAP_AD_TYPE_16BIT_SERVICE_UUID_MORE_AVAILABLE,
                                &adv_data,
                                &type_data);

    if (err_code != NRF_SUCCESS)
    {
        // Look for the services in 'complete' if it was not found in 'more available'.
        err_code = adv_report_parse(BLE_GAP_AD_TYPE_16BIT_SERVICE_UUID_COMPLETE,
                                    &adv_data,
                                    &type_data);

        if (err_code != NRF_SUCCESS)
        {
            // If we can't parse the data, then exit.
            return false;
        }
    }

    // Verify if any UUID match the given UUID.
    for (uint32_t u_index = 0; u_index < (type_data.data_len / sizeof(uint16_t)); u_index++)
    {
        uint16_t extracted_uuid;

        UUID16_EXTRACT(&extracted_uuid, &type_data.p_data[u_index * sizeof(uint16_t)]);

        if (extracted_uuid == uuid_to_find)
        {		
						NRF_LOG_INFO("UUID match!!! 0x%x, p_adv_report->dlen is %d...\n", extracted_uuid, p_adv_report->dlen);
            return true;
        }
    }
#endif
    return false;
}


/**@brief Function for searching a given name in the advertisement packets.
 *
 * @details Use this function to parse received advertising data and to find a given
 * name in them either as 'complete_local_name' or as 'short_local_name'.
 *
 * @param[in]   p_adv_report   advertising data to parse.
 * @param[in]   name_to_find   name to search.
 * @return   true if the given name was found, false otherwise.
 */
uint8_t find_adv_name(ble_gap_evt_adv_report_t const * p_adv_report)
{
    ret_code_t err_code;
    data_t     adv_data;
    data_t     dev_name;
		NRF_LOG_INFO("IN find_adv_name()..\n");
    // Initialize advertisement report for parsing
    adv_data.p_data   = (uint8_t *)p_adv_report->data;
    adv_data.data_len = p_adv_report->dlen;
	
//		NRF_LOG_INFO("find_adv_name(): p_adv_report->dlen is %d\n", p_adv_report->dlen);

    //search for advertising names
    err_code = adv_report_parse(BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME, &adv_data, &dev_name);
    if (err_code == NRF_SUCCESS)
    {
        NRF_LOG_INFO("(1)dev_name.p_data is %s...\n", dev_name.p_data);
        return true;
    }
    else
    {
        // Look for the short local name if it was not found as complete
        err_code = adv_report_parse(BLE_GAP_AD_TYPE_SHORT_LOCAL_NAME, &adv_data, &dev_name);
        if (err_code != NRF_SUCCESS)
        {
            return false;
        }
        else
        {
						NRF_LOG_INFO("(2)dev_name.p_data is %s...\n", dev_name.p_data);
            return true;
        }
    }
    return false;
}



/**@brief Function for handling the advertising report BLE event.
 *
 * @param[in] p_ble_evt  Bluetooth stack event.
 */
void on_adv_report(const ble_evt_t * const p_ble_evt)
{
    ret_code_t err_code;
    data_t     adv_data;
    data_t     dev_name;
    bool       do_connect = false;

    // For readibility.
    ble_gap_evt_t  const * p_gap_evt  = &p_ble_evt->evt.gap_evt;
    ble_gap_addr_t const * peer_addr  = &p_gap_evt->params.adv_report.peer_addr;

    // Initialize advertisement report for parsing
    adv_data.p_data   = (uint8_t *)p_gap_evt->params.adv_report.data;
    adv_data.data_len = p_gap_evt->params.adv_report.dlen;
		

    // Search for advertising names.
    bool name_found = false;
    err_code = adv_report_parse(BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME, &adv_data, &dev_name);
		
		//NRF_LOG_INFO("dev_name : %s\n", dev_name.p_data);

    if (err_code != NRF_SUCCESS)
    {
        // Look for the short local name if it was not found as complete.
        err_code = adv_report_parse(BLE_GAP_AD_TYPE_SHORT_LOCAL_NAME, &adv_data, &dev_name);
        if (err_code != NRF_SUCCESS)
        {
            // If we can't parse the data, then exit
            return;
        }
        else
        {
            name_found = true;
        }
    }
    else
    {
        name_found = true;
    }

    if (name_found)
    {
        if (strlen(m_target_periph_name) != 0)
        {
            if (memcmp(m_target_periph_name, dev_name.p_data, 11/*dev_name.data_len*/ )== 0)
            {
//								task_ble_bdsf_handle(TASK_BLE_BDSF_EVT_NAME_MATCH, (uint8_t *)m_target_periph_name, 11);
							NRF_LOG_INFO("p_data is : %s, dev_name.data_len is : %d.\n", dev_name.p_data, dev_name.data_len);
							//for(int i = 0; i < 11; i++)
							//	NRF_LOG_INFO("%c", dev_name.p_data[i]);
							//NRF_LOG_INFO("\n");
							//NRF_LOG_HEXDUMP_DEBUG(dev_name.p_data, 12);
							//uint32_t command[256] = {BLE_UART_CCOMMAND_BLE_DEVICE_NAME | BLE_UART_CCOMMAND_MASK_BDSF};
							//memcpy((uint8_t *)(command+1),(uint8_t *)m_target_periph_name,12);		
							//task_ble_conn_handle(TASK_BLE_CONN_EVT_BLE_UART, (uint8_t *)(&command), sizeof(command));							
							//
                //do_connect = true;
            }
        }
    }

    if (do_connect)
    {
        // Initiate connection.
        err_code = sd_ble_gap_connect(peer_addr,
                                      &m_scan_params,
                                      &m_connection_param,
                                      APP_BLE_CONN_CFG_TAG);
        APP_ERROR_CHECK(err_code);
    }
}

//#endif
