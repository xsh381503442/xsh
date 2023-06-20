/**
 * Copyright (c) 2014 - 2017, Nordic Semiconductor ASA
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 * 
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 * 
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 * 
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 * 
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
/** @example examples/ble_peripheral/ble_app_buttonless_dfu
 *
 * @brief Secure DFU Buttonless Service Application main file.
 *
 * This file contains the source code for a sample application using the proprietary
 * Secure DFU Buttonless Service. This is a template application that can be modified
 * to your needs. To extend the functionality of this application, please find
 * locations where the comment "// YOUR_JOB:" is present and read the comments.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "nrf_dfu_svci.h"
#include "nrf_svci_async_function.h"
#include "nrf_svci_async_handler.h"

#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "nrf_sdh_freertos.h"
#include "app_timer.h"
#include "peer_manager.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_state.h"
#include "ble_dfu.h"
#include "nrf_ble_gatt.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
#include "fds.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_drv_clock.h"

#include "task_apollo2.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_power.h"

#ifdef BOARD_PCA10040
#include "bsp.h"
#include "boards.h"
#include "bsp_btn_ble.h"
#endif


#include "app_srv_apollo.h"
#include "drv_ble_flash.h"
#include "ble_srv_nrf52.h"
#include "app_srv_nrf52.h"
#include "drv_extFlash.h"
#include "dev_extFlash.h"


#include "drv_apollo2.h"
#include "dev_apollo2.h"
#include "nrf_delay.h"

#include "cmd_apollo2.h"
#include "cmd_gpregret.h"
#include "cmd_nrf52_srv.h"
#include "nrf_drv_gpiote.h"
#include "nrf_sdm.h"
#include "ble_srv_wechat.h"
#include "app_srv_wechat.h"
#include "task_ble_conn.h"


#include "ble_db_discovery.h"
#include "app_srv_ancs.h"
#include "nrf_ble_ancs_c.h"
#include "app_srv_acces.h"

#include "task_extFlash.h"
#include "task_ble_accessary.h"

#include "ble_srv_apollo.h"
#include "ble_srv_nrf52.h"
#include "ble_srv_wechat.h"


#include "nrf_ble_ancs_c.h"
#include "ble_hrs_c.h"
#include "ble_rscs_c.h"
#include "ble_cscs_c.h"
#include "ble_bas.h"
#include "app_srv_cod.h"
#include "drv_wdt.h"
#include "main.h"







#define APP_FEATURE_NOT_SUPPORTED       BLE_GATT_STATUS_ATTERR_APP_BEGIN + 2        /**< Reply when unsupported features are requested. */


//#define RSSI_ADV_DEVICE




#ifdef RSSI_ADV_DEVICE
#define DEVICE_NAME                     "JRSSI_AAAA"                         /**< Name of device. Will be included in the advertising data. */
#else

#ifdef PA806
#define DEVICE_NAME                    "TA806" 
#else
#define DEVICE_NAME                    "HY100"                         /**< Name of device. Will be included in the advertising data. */
#endif


#endif



#define TX_DBM_VALUE    4



#define MANUFACTURER_NAME               "YZL"                       /**< Manufacturer. Will be passed to Device Information Service. */
#define APP_ADV_INTERVAL_FAST                (100)                                         /**< The advertising interval (in units of 0.625 ms. This value corresponds to 187.5 ms). */
#define APP_ADV_TIMEOUT_FAST_IN_SECONDS      30                                         /**< The advertising timeout in units of seconds. */
#define APP_ADV_INTERVAL_SLOW                (1600)                                         /**< The advertising interval (in units of 0.625 ms. This value corresponds to 187.5 ms). */
#define APP_ADV_TIMEOUT_SLOW_IN_SECONDS      0                                         /**< The advertising timeout in units of seconds. */





#define APP_BLE_OBSERVER_PRIO           1                                           /**< Application's BLE observer priority. You shouldn't need to modify this value. */
#define APP_BLE_CONN_CFG_TAG            1                                           /**< A tag identifying the SoftDevice BLE configuration. */

//#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(15, UNIT_1_25_MS)            /**< Minimum acceptable connection interval (0.1 seconds). */
//#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(30, UNIT_1_25_MS)            /**< Maximum acceptable connection interval (0.2 second). */
//#define SLAVE_LATENCY                   0                                           /**< Slave latency. */
//#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)             /**< Connection supervisory timeout (4 seconds). */


/**

POWER PREQUIREMENT : 60 uA

SLOW CONNECTION PARAMETER
100-150-0-400 : 0x06,0x64,0x00,0x96,0x00,0x00,0x00,0x90,0x01

FAST CONNECTION PARAMTER
15-30-0-400 : 0x06,0x0c,0x00,0x18,0x00,0x00,0x00,0x90,0x01


*/



//#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000)                       /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
//#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000)                      /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
//#define MAX_CONN_PARAMS_UPDATE_COUNT    1                                           /**< Number of attempts before giving up the connection parameter negotiation. */

#define FIRST_CONN_PARAMS_UPDATE_DELAY      5000                                    /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY       30000                                   /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT        3                                       /**< Number of attempts before giving up the connection parameter negotiation. */



#define SEC_PARAM_BOND                  1                                           /**< Perform bonding. */
#define SEC_PARAM_MITM                  1                                           /**< Man In The Middle protection not required. */
#define SEC_PARAM_LESC                  0                                           /**< LE Secure Connections not enabled. */
#define SEC_PARAM_KEYPRESS              0                                           /**< Keypress notifications not enabled. */
#define SEC_PARAM_IO_CAPABILITIES       BLE_GAP_IO_CAPS_DISPLAY_ONLY                /**< No I/O capabilities. */
#define SEC_PARAM_OOB                   0                                           /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE          7                                           /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE          16                                          /**< Maximum encryption key size. */

#define DEAD_BEEF                       0xDEADBEEF                                  /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */
 

#ifdef LAYER_BOOTLOADER  
#define DELAY_MS(X)                 nrf_delay_ms(X)
#endif

#ifdef LAYER_APPLICATION 
//#define DELAY_MS(X)                 nrf_delay_ms(X)
#define DELAY_MS(X)                 vTaskDelay(X)
//#define DELAY_MS(X)                   do{TickType_t xLastWakeTime;xLastWakeTime = xTaskGetTickCount ();vTaskDelayUntil( &xLastWakeTime, X / ( ( TickType_t ) 1000 / 3 ) );}while(0)
#endif


 
#define SYS_OFF_PIN         18

NRF_BLE_GATT_DEF(m_gatt);                                                           /**< GATT module instance. */
BLE_ADVERTISING_DEF(m_advertising);                                                 /**< Advertising module instance. */

uint16_t g_conn_handle_peripheral = BLE_CONN_HANDLE_INVALID;                            /**< Handle of the current connection. */
static void advertising_start(void * p_erase_bonds);
uint8_t g_ble_passkey[BLE_GAP_PASSKEY_LEN];
ble_gap_addr_t  g_peer_addr;
bool g_conn_params_update_from_mobile_app = 0;//This flag will be reset to zero when connect and disconnect
pm_peer_id_t g_pm_peer_id;
extern _drv_ble_flash_ancs_switch g_ancs_switch;
uint8_t	g_adv_manuf_mac[BLE_GAP_ADDR_LEN];
#ifdef COD
#define COD_MANU_DATA_LEN  11
uint8_t g_scan_manuf_data[COD_MANU_DATA_LEN];
#endif
extern task_apollo2_ht_rssi_callback g_ht_rssi_callback;

extern ble_apollo_t m_apollo;
extern ble_nrf52_t m_nrf52;
extern ble_wechat_t m_wechat;
extern ble_bas_t m_bas;

extern ble_ancs_c_t m_ancs_c;
extern ble_hrs_c_t m_hrs_c;
extern ble_rscs_c_t m_rscs_c;
extern ble_cscs_c_t m_cscs_c;
extern ble_db_discovery_t m_db_disc[];
extern _accessory_running g_accessory_running;
extern _accessory_heartrate g_accessory_heartrate;
extern _accessory_cycling g_accessory_cycling;



#if NRF_LOG_ENABLED
static TaskHandle_t m_logger_thread;                                /**< Definition of Logger thread. */
#endif

#define HEYOU_CONFIG_DEFAULT                         \
{                                                     \
    .dev_name =                                       \
    {                                                 \
        .name = "DEVICE_NAME",                          \
        .len = 10,                                      \
    },                                                \
    .adv_params =                                     \
    {                                                 \
				.enable_fast   = true,                        \
        .interval_fast = APP_ADV_INTERVAL_FAST,       \
        .timeout_fast  = APP_ADV_TIMEOUT_FAST_IN_SECONDS,   \
				.enable_slow   = true,                        \
        .interval_slow = APP_ADV_INTERVAL_SLOW,       \
        .timeout_slow  = APP_ADV_TIMEOUT_SLOW_IN_SECONDS   \
		},                                                \
    .conn_params =                                    \
    {                                                 \
        .min_conn_int  = MIN_CONN_INTERVAL_FAST,           \
        .max_conn_int  = MAX_CONN_INTERVAL_FAST,           \
        .slave_latency = SLAVE_LATENCY_FAST,               \
        .sup_timeout   = CONN_SUP_TIMEOUT_FAST             \
    },                                                \
}

ble_config_params_t     g_ble_config_default_config = HEYOU_CONFIG_DEFAULT;
static ble_config_params_t     m_ble_config;
ble_config_params_t     *g_ble_config = &m_ble_config;


bool g_is_central_mode = false;



#define TIMER_INTERVAL            5000 
#define OSTIMER_WAIT_FOR_QUEUE              100 
static TimerHandle_t m_ble_conn_timer; 
static uint8_t m_ble_conn_timer_evt = 0;

//accessary conn state change event
static uint8_t m_ble_conn_timer_evt_heartrate = 0;
static uint8_t m_ble_conn_timer_evt_running = 0;
static uint8_t m_ble_conn_timer_evt_cycling = 0;

#define BLE_CONN_TIMEOUT_EVT_CONNECTTED          		 1
#define BLE_CONN_TIMEOUT_EVT_DISCONNECTTED       		 2

#define BLE_CONN_TIMEOUT_EVT_HEARTRATE_CONNECTTED        1
#define BLE_CONN_TIMEOUT_EVT_HEARTRATE_DISCONNECTTED     2

#define BLE_CONN_TIMEOUT_EVT_RUNNING_CONNECTTED          1
#define BLE_CONN_TIMEOUT_EVT_RUNNING_DISCONNECTTED       2

#define BLE_CONN_TIMEOUT_EVT_CYCLING_CONNECTTED          1
#define BLE_CONN_TIMEOUT_EVT_CYCLING_DISCONNECTTED       2





#define CONN_TIMER_TIMEOUT    			350 //防止测试反复快速开关苹果蓝牙
#define ACCESSARY_CONN_TIMER_TIMEOUT    350 //防止配件连接频繁连接断开导致通信失败

static bool m_erase_bonds = 0;


static void ble_conn_timeout_handler(TimerHandle_t xTimer)
{
    UNUSED_PARAMETER(xTimer);
    NRF_LOG_DEBUG("ble_conn_timeout_handler");

	switch(m_ble_conn_timer_evt)
	{
		case BLE_CONN_TIMEOUT_EVT_CONNECTTED:{
			task_ble_conn_handle(TASK_BLE_CONN_EVT_CONNECT, NULL, 0);		
		}break;
		case BLE_CONN_TIMEOUT_EVT_DISCONNECTTED:{
			task_ble_conn_handle(TASK_BLE_CONN_EVT_DISCONNECT, NULL, 0);
		}break;		
		default:
			break;
	}
	m_ble_conn_timer_evt = 0;
	
//配件连接事件
	switch(m_ble_conn_timer_evt_heartrate)
	{
		case BLE_CONN_TIMEOUT_EVT_HEARTRATE_CONNECTTED:{
			task_ble_conn_handle(TASK_BLE_CONN_EVT_CONNECT_HEARTRATE, NULL, 0);		
		}break;
		case BLE_CONN_TIMEOUT_EVT_HEARTRATE_DISCONNECTTED:{
			task_ble_conn_handle(TASK_BLE_CONN_EVT_DISCONNECT_HEARTRATE, NULL, 0);
		}break;	
		default:
			break;
	}
	m_ble_conn_timer_evt_heartrate = 0;

	switch(m_ble_conn_timer_evt_running)
	{
		case BLE_CONN_TIMEOUT_EVT_RUNNING_CONNECTTED:{
			task_ble_conn_handle(TASK_BLE_CONN_EVT_CONNECT_RUNNING, NULL, 0);		
		}break;
		case BLE_CONN_TIMEOUT_EVT_RUNNING_DISCONNECTTED:{
			task_ble_conn_handle(TASK_BLE_CONN_EVT_DISCONNECT_RUNNING, NULL, 0);
		}break;	
		default:
			break;
	}
	m_ble_conn_timer_evt_running = 0;

	switch(m_ble_conn_timer_evt_cycling)
	{
		case BLE_CONN_TIMEOUT_EVT_CYCLING_CONNECTTED:{
			task_ble_conn_handle(TASK_BLE_CONN_EVT_CONNECT_CYCLING, NULL, 0);		
		}break;
		case BLE_CONN_TIMEOUT_EVT_CYCLING_DISCONNECTTED:{
			task_ble_conn_handle(TASK_BLE_CONN_EVT_DISCONNECT_CYCLING, NULL, 0);
		}break;	
		default:
			break;
	}
	m_ble_conn_timer_evt_cycling = 0;
}


static void ble_conn_timer_create(void)
{
	m_ble_conn_timer = xTimerCreate("BLEC",
                         TIMER_INTERVAL,
                         pdFALSE,
                         NULL,
                         ble_conn_timeout_handler);
	if (NULL == m_ble_conn_timer)
	{
			APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
	}	
}
static void ble_conn_timer_start(uint32_t timeout_msec)
{
	if( xTimerChangePeriod( m_ble_conn_timer, timeout_msec , OSTIMER_WAIT_FOR_QUEUE ) == pdPASS )
	{
		NRF_LOG_DEBUG("The command was successfully sent.");
	}
	else
	{
		NRF_LOG_DEBUG("The command could not be sent, even after waiting for 100 ticks");
		NRF_LOG_DEBUG("to pass.  Take appropriate action here.");
	}		
	
}
static void ble_conn_timer_stop(void)
{
	if (pdPASS != xTimerStop(m_ble_conn_timer, OSTIMER_WAIT_FOR_QUEUE))
	{
			APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
	}	
}




/**@brief Handler for shutdown preparation.
 *
 * @details During shutdown procedures, this function will be called at a 1 second interval
 *          untill the function returns true. When the function returns true, it means that the
 *          app is ready to reset to DFU mode.
 *
 * @param[in]   event   Power manager event.
 *
 * @retval  True if shutdown is allowed by this power manager handler, otherwise false.
 */
static bool app_shutdown_handler(nrf_pwr_mgmt_evt_t event)
{
    switch (event)
    {
        case NRF_PWR_MGMT_EVT_PREPARE_DFU:
            NRF_LOG_INFO("Power management wants to reset to DFU mode.");
            // YOUR_JOB: Get ready to reset into DFU mode
            //
            // If you aren't finished with any ongoing tasks, return "false" to
            // signal to the system that reset is impossible at this stage.
            //
            // Here is an example using a variable to delay resetting the device.
            //
            // if (!m_ready_for_reset)
            // {
            //      return false;
            // }
            // else
            //{
            //
            //    // Device ready to enter
            //    uint32_t err_code;
            //    err_code = sd_softdevice_disable();
            //    APP_ERROR_CHECK(err_code);
            //    err_code = app_timer_stop_all();
            //    APP_ERROR_CHECK(err_code);
            //}
            break;

        default:
            // YOUR_JOB: Implement any of the other events available from the power management module:
            //      -NRF_PWR_MGMT_EVT_PREPARE_SYSOFF
            //      -NRF_PWR_MGMT_EVT_PREPARE_WAKEUP
            //      -NRF_PWR_MGMT_EVT_PREPARE_RESET
            return true;
    }

    NRF_LOG_INFO("Power management allowed to reset to DFU mode.");
    return true;
}

//lint -esym(528, m_app_shutdown_handler)
/**@brief Register application shutdown handler with priority 0.
 */
NRF_PWR_MGMT_HANDLER_REGISTER(app_shutdown_handler, 0);




// YOUR_JOB: Use UUIDs for service(s) used in your application.


#ifdef RSSI_ADV_DEVICE
 ble_uuid_t m_adv_uuids[] = {{0xFEFE, BLE_UUID_TYPE_BLE}};
#else
#ifdef COD
ble_uuid_t m_adv_uuids[BLE_UUID_ADV_COUNT] = {{0x0AF0, BLE_UUID_TYPE_BLE}};
#else
 ble_uuid_t m_adv_uuids[BLE_UUID_ADV_COUNT] = {{BLE_UUID_PI_COMPANY, BLE_UUID_TYPE_BLE},
																	 {BLE_UUID_HN_BRAND, BLE_UUID_TYPE_BLE},
																	 {BLE_UUID_HY100_SERIES, BLE_UUID_TYPE_BLE},
																		{BLE_UUID_WECHAT_SERVICE, BLE_UUID_TYPE_BLE}};	
#endif
#endif	    

// YOUR_JOB: Update this code if you want to do anything given a DFU event (optional).
/**@brief Function for handling dfu events from the Buttonless Secure DFU service
 *
 * @param[in]   event   Event from the Buttonless Secure DFU service.
 */
static void ble_dfu_evt_handler(ble_dfu_buttonless_evt_type_t event)
{
    switch (event)
    {
        case BLE_DFU_EVT_BOOTLOADER_ENTER_PREPARE:
            NRF_LOG_DEBUG("Device is preparing to enter bootloader mode.");
            // YOUR_JOB: Disconnect all bonded devices that currently are connected.
            //           This is required to receive a service changed indication
            //           on bootup after a successful (or aborted) Device Firmware Update.
            break;

        case BLE_DFU_EVT_BOOTLOADER_ENTER:
            // YOUR_JOB: Write app-specific unwritten data to FLASH, control finalization of this
            //           by delaying reset by reporting false in app_shutdown_handler
            NRF_LOG_DEBUG("Device will enter bootloader mode.");
            break;

        case BLE_DFU_EVT_BOOTLOADER_ENTER_FAILED:
            NRF_LOG_ERROR("Request to enter bootloader mode failed asynchroneously.");
            // YOUR_JOB: Take corrective measures to resolve the issue
            //           like calling APP_ERROR_CHECK to reset the device.
            break;

        case BLE_DFU_EVT_RESPONSE_SEND_ERROR:
            NRF_LOG_ERROR("Request to send a response to client failed.");
            // YOUR_JOB: Take corrective measures to resolve the issue
            //           like calling APP_ERROR_CHECK to reset the device.
            APP_ERROR_CHECK(false);
            break;

        default:
            NRF_LOG_ERROR("Unknown event from ble_dfu_buttonless.");
            break;
    }
}


/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num   Line number of the failing ASSERT call.
 * @param[in] file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

/**@brief Function for handling Peer Manager events.
 *
 * @param[in] p_evt  Peer Manager event.
 */
static void pm_evt_handler(pm_evt_t const * p_evt)
{
    ret_code_t err_code;

    switch (p_evt->evt_id)
    {
        case PM_EVT_BONDED_PEER_CONNECTED:
        {
					NRF_LOG_DEBUG("PM_EVT_BONDED_PEER_CONNECTED");
            NRF_LOG_DEBUG("Connected to a previously bonded device.");
        } break;

        case PM_EVT_CONN_SEC_SUCCEEDED:
        {
					NRF_LOG_DEBUG("PM_EVT_CONN_SEC_SUCCEEDED");
            NRF_LOG_DEBUG("Connection secured: role: %d, conn_handle: 0x%x, procedure: %d.",
                         ble_conn_state_role(p_evt->conn_handle),
                         p_evt->conn_handle,
                         p_evt->params.conn_sec_succeeded.procedure);
					
						g_pm_peer_id = p_evt->peer_id; 
					
						task_ble_conn_handle(TASK_BLE_CONN_EVT_BOND, NULL, 0);
					
					
					
        } break;

        case PM_EVT_CONN_SEC_FAILED:
        {
					NRF_LOG_DEBUG("PM_EVT_CONN_SEC_FAILED");
					
						err_code = sd_ble_gap_disconnect(g_conn_handle_peripheral, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
						APP_ERROR_CHECK(err_code);				
					
//					err_code = pm_peers_delete();
//					APP_ERROR_CHECK(err_code);
            /* Often, when securing fails, it shouldn't be restarted, for security reasons.
             * Other times, it can be restarted directly.
             * Sometimes it can be restarted, but only after changing some Security Parameters.
             * Sometimes, it cannot be restarted until the link is disconnected and reconnected.
             * Sometimes it is impossible, to secure the link, or the peer device does not support it.
             * How to handle this error is highly application dependent. */
        } break;

        case PM_EVT_CONN_SEC_CONFIG_REQ:
        {
					NRF_LOG_DEBUG("PM_EVT_CONN_SEC_CONFIG_REQ");
            // Reject pairing request from an already bonded peer.
            pm_conn_sec_config_t conn_sec_config = {.allow_repairing = true};
            pm_conn_sec_config_reply(p_evt->conn_handle, &conn_sec_config);
        } break;

        case PM_EVT_STORAGE_FULL:
        {
					NRF_LOG_DEBUG("PM_EVT_STORAGE_FULL");
            // Run garbage collection on the flash.
            err_code = fds_gc();
						APP_ERROR_CHECK(err_code);
//            if (err_code == FDS_ERR_BUSY || err_code == FDS_ERR_NO_SPACE_IN_QUEUES)
//            {
//							// Retry.
//							NRF_LOG_DEBUG("Retry");
//							err_code = fds_gc();
//							if (err_code == FDS_ERR_BUSY || err_code == FDS_ERR_NO_SPACE_IN_QUEUES)
//							{
//								// Retry.
//								NRF_LOG_DEBUG("Retry");
//								err_code = fds_gc();
//									
//							}
//							else
//							{
//									APP_ERROR_CHECK(err_code);
//							}							
//                
//            }
//            else
//            {
//                APP_ERROR_CHECK(err_code);
//            }
        } break;

        case PM_EVT_PEERS_DELETE_SUCCEEDED:
        {
					NRF_LOG_DEBUG("PM_EVT_PEERS_DELETE_SUCCEEDED");
//					err_code = sd_ble_gap_disconnect(g_conn_handle_peripheral,
//																					 BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
//					APP_ERROR_CHECK(err_code);					
					advertising_start(false);
        } break;

        case PM_EVT_LOCAL_DB_CACHE_APPLY_FAILED:
        {
					NRF_LOG_DEBUG("PM_EVT_LOCAL_DB_CACHE_APPLY_FAILED");
            // The local database has likely changed, send service changed indications.
            pm_local_database_has_changed();
        } break;

        case PM_EVT_PEER_DATA_UPDATE_FAILED:
        {
					NRF_LOG_DEBUG("PM_EVT_PEER_DATA_UPDATE_FAILED");
            // Assert.
            APP_ERROR_CHECK(p_evt->params.peer_data_update_failed.error);
        } break;

        case PM_EVT_PEER_DELETE_FAILED:
        {
					NRF_LOG_DEBUG("PM_EVT_PEER_DELETE_FAILED");
					
            // Assert.
            APP_ERROR_CHECK(p_evt->params.peer_delete_failed.error);
        } break;

        case PM_EVT_PEERS_DELETE_FAILED:
        {
					NRF_LOG_DEBUG("PM_EVT_PEERS_DELETE_FAILED");
            // Assert.
            APP_ERROR_CHECK(p_evt->params.peers_delete_failed_evt.error);
        } break;

        case PM_EVT_ERROR_UNEXPECTED:
        {
					NRF_LOG_DEBUG("PM_EVT_ERROR_UNEXPECTED");
            // Assert.
            APP_ERROR_CHECK(p_evt->params.error_unexpected.error);
        } break;

        case PM_EVT_CONN_SEC_START:{}break;
        case PM_EVT_PEER_DATA_UPDATE_SUCCEEDED:{}break;
        case PM_EVT_PEER_DELETE_SUCCEEDED:{
					NRF_LOG_DEBUG("PM_EVT_PEER_DELETE_SUCCEEDED");

					
					
				}break;
        case PM_EVT_LOCAL_DB_CACHE_APPLIED:{}break;
        case PM_EVT_SERVICE_CHANGED_IND_SENT:{}break;
        case PM_EVT_SERVICE_CHANGED_IND_CONFIRMED:{}break;
        default:
            break;
    }
}





/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module. This creates and starts application timers.
 */
static void timers_init(void)
{
	
    // Initialize timer module.
  //  uint32_t err_code = app_timer_init();
  //  APP_ERROR_CHECK(err_code);

		ble_conn_timer_create();
		
	
	
    // Create timers.

    /* YOUR_JOB: Create any timers to be used by the application.
                 Below is an example of how to create a timer.
                 For every new timer needed, increase the value of the macro APP_TIMER_MAX_TIMERS by
                 one.
       uint32_t err_code;
       err_code = app_timer_create(&m_app_timer_id, APP_TIMER_MODE_REPEATED, timer_timeout_handler);
       APP_ERROR_CHECK(err_code); */
}


/**@brief Function for the GAP initialization.
 *
 * @details This function sets up all the necessary GAP (Generic Access Profile) parameters of the
 *          device including the device name, appearance, and the preferred connection parameters.
 */
static void gap_params_init(void)
{
	NRF_LOG_DEBUG("gap_params_init");
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
	
#ifdef RSSI_ADV_DEVICE
    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *)DEVICE_NAME,
                                          sizeof(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);
#else
    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                           g_ble_config->dev_name.name,
                                          g_ble_config->dev_name.len);
    APP_ERROR_CHECK(err_code);
#endif			


    /* YOUR_JOB: Use an appearance value matching the application's use case.
       err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_);
       APP_ERROR_CHECK(err_code); */

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));
//    gap_conn_params.min_conn_interval = g_ble_config->conn_params.min_conn_int;
//    gap_conn_params.max_conn_interval = g_ble_config->conn_params.max_conn_int;
//    gap_conn_params.slave_latency     = g_ble_config->conn_params.slave_latency;
//    gap_conn_params.conn_sup_timeout  = g_ble_config->conn_params.sup_timeout;

    g_ble_config->conn_params.min_conn_int  = MIN_CONN_INTERVAL_SLOW;
    g_ble_config->conn_params.max_conn_int  = MAX_CONN_INTERVAL_SLOW;
    g_ble_config->conn_params.slave_latency = SLAVE_LATENCY_SLOW;
    g_ble_config->conn_params.sup_timeout   = CONN_SUP_TIMEOUT_SLOW;

    gap_conn_params.min_conn_interval   = MIN_CONN_INTERVAL_SLOW;
    gap_conn_params.max_conn_interval   = MAX_CONN_INTERVAL_SLOW;
    gap_conn_params.slave_latency       = SLAVE_LATENCY_SLOW;
    gap_conn_params.conn_sup_timeout    = CONN_SUP_TIMEOUT_SLOW;
		
		
//    g_ble_config->conn_params.min_conn_int  = MSEC_TO_UNITS(20, UNIT_1_25_MS);
//    g_ble_config->conn_params.max_conn_int  = MSEC_TO_UNITS(75, UNIT_1_25_MS);
//    g_ble_config->conn_params.slave_latency = 0;
//    g_ble_config->conn_params.sup_timeout   = MSEC_TO_UNITS(4000, UNIT_10_MS);

//    gap_conn_params.min_conn_interval   = MSEC_TO_UNITS(20, UNIT_1_25_MS);
//    gap_conn_params.max_conn_interval   = MSEC_TO_UNITS(75, UNIT_1_25_MS);
//    gap_conn_params.slave_latency       = 0;
//    gap_conn_params.conn_sup_timeout    = MSEC_TO_UNITS(4000, UNIT_10_MS);		
		
		
    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}

static void dfu_service_init(void)
{
    uint32_t err_code;
    ble_dfu_buttonless_init_t dfus_init =
    {
        .evt_handler = ble_dfu_evt_handler
    };

    // Initialize the async SVCI interface to bootloader.
    err_code = ble_dfu_buttonless_async_svci_init();
    APP_ERROR_CHECK(err_code);


    err_code = ble_dfu_buttonless_init(&dfus_init);
    APP_ERROR_CHECK(err_code);

}





/**@brief Function for initializing services that will be used by the application.
 */
static void services_init(void)
{
	
	dfu_service_init();
	app_srv_apollo_init();
	app_srv_nrf52_init();
	#ifndef COD 
	app_srv_wechat_init();
	#endif
	app_srv_ancs_init();
	app_srv_acces_c_init();
	#ifdef COD 
	app_srv_cod_init();
	#endif
}



/**@brief Function for handling the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module which
 *          are passed to the application.
 *          @note All this function does is to disconnect. This could have been done by simply
 *                setting the disconnect_on_fail config parameter, but instead we use the event
 *                handler mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */


static void on_conn_params_evt(ble_conn_params_evt_t * p_evt, ble_gap_conn_params_t *new_conn_params)
{
		NRF_LOG_DEBUG("on_conn_params_evt");


	
		if(g_conn_params_update_from_mobile_app)
		{
			g_conn_params_update_from_mobile_app = 0;
			NRF_LOG_DEBUG("g_conn_params_update_from_mobile_app");
			//Only response to mobile app when conn params update request is from mobile app
			//So system conn params update will be ignored.
			if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
			{
				NRF_LOG_DEBUG("BLE_CONN_PARAMS_EVT_FAILED");
			
				_app_srv_nrf52_conn_param response;
				response.command = CMD_NRF52_SRV_CONN_PARAM_UPDATE_RESPONSE;
				response.param.error = CMD_NRF52_SRV_ERROR_INVALID_PARAMS;
				app_srv_nrf52_config_send((uint8_t *)&response, 2);				
				
			}
			else if(p_evt->evt_type == BLE_CONN_PARAMS_EVT_SUCCEEDED)
			{
				NRF_LOG_DEBUG("BLE_CONN_PARAMS_EVT_SUCCEEDED");
				NRF_LOG_DEBUG("min_conn_interval --> %d",new_conn_params->min_conn_interval);
				NRF_LOG_DEBUG("max_conn_interval --> %d",new_conn_params->max_conn_interval);
				NRF_LOG_DEBUG("slave_latency --> %d",new_conn_params->slave_latency);
				NRF_LOG_DEBUG("conn_sup_timeout --> %d",new_conn_params->conn_sup_timeout);	
				
        g_ble_config->conn_params.max_conn_int = new_conn_params->max_conn_interval;
        g_ble_config->conn_params.min_conn_int = new_conn_params->min_conn_interval;
        g_ble_config->conn_params.slave_latency = new_conn_params->slave_latency;
        g_ble_config->conn_params.sup_timeout = new_conn_params->conn_sup_timeout;
				
				_app_srv_nrf52_conn_param response;
				response.command = CMD_NRF52_SRV_CONN_PARAM_UPDATE_RESPONSE;
				response.param.error = CMD_NRF52_SRV_SUCCUSS;
				app_srv_nrf52_config_send((uint8_t *)&response, 2);	
				
			}		
		}
		else
		{
			if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
			{
				NRF_LOG_DEBUG("BLE_CONN_PARAMS_EVT_FAILED");
			}
			else if(p_evt->evt_type == BLE_CONN_PARAMS_EVT_SUCCEEDED)
			{
				NRF_LOG_DEBUG("22 BLE_CONN_PARAMS_EVT_SUCCEEDED");
				NRF_LOG_DEBUG("min_conn_interval --> %d",new_conn_params->min_conn_interval);
				NRF_LOG_DEBUG("max_conn_interval --> %d",new_conn_params->max_conn_interval);
				NRF_LOG_DEBUG("slave_latency --> %d",new_conn_params->slave_latency);
				NRF_LOG_DEBUG("conn_sup_timeout --> %d",new_conn_params->conn_sup_timeout);				
        g_ble_config->conn_params.max_conn_int = new_conn_params->max_conn_interval;
        g_ble_config->conn_params.min_conn_int = new_conn_params->min_conn_interval;
        g_ble_config->conn_params.slave_latency = new_conn_params->slave_latency;
        g_ble_config->conn_params.sup_timeout = new_conn_params->conn_sup_timeout;				
				
				
				
				
				
			}		

			
		}
}


/**@brief Function for handling a Connection Parameters error.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
//    APP_ERROR_HANDLER(nrf_error);

		uint32_t               err_code;
		err_code = sd_ble_gap_disconnect(g_conn_handle_peripheral, BLE_HCI_LOCAL_HOST_TERMINATED_CONNECTION);
		APP_ERROR_CHECK(err_code);

}


/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
extern ble_nrf52_t m_nrf52;
	
	
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;//m_nrf52.config_handles.cccd_handle;//BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}








/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{

    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST:
						NRF_LOG_DEBUG("BLE_ADV_EVT_FAST %d,%d", g_ble_config->conn_params.min_conn_int, g_ble_config->conn_params.max_conn_int);
            break;
        case BLE_ADV_EVT_SLOW:
						NRF_LOG_DEBUG("BLE_ADV_EVT_SLOW %d,%d",g_ble_config->conn_params.min_conn_int, g_ble_config->conn_params.max_conn_int);
            break;
        case BLE_ADV_EVT_IDLE:
						NRF_LOG_DEBUG("BLE_ADV_EVT_IDLE");
            break;

        default:
            break;
    }
}
/**@brief   Function for handling BLE events from central applications.
 *
 * @details This function parses scanning reports and initiates a connection to peripherals when a
 *          target UUID is found. It updates the status of LEDs used to report central applications
 *          activity.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 */
//static void on_ble_central_evt(ble_evt_t const * p_ble_evt)
//{
//    ret_code_t            err_code;
//    ble_gap_evt_t const * p_gap_evt = &p_ble_evt->evt.gap_evt;

//    switch (p_ble_evt->header.evt_id)
//    {
//        // Upon connection, check which peripheral has connected (HR or RSC), initiate DB
//        // discovery, update LEDs status and resume scanning if necessary.
//        case BLE_GAP_EVT_CONNECTED:
//        {
//            NRF_LOG_INFO("Central connected");
//						bool start_scan = false;
//						
//						if(!memcmp(p_ble_evt->evt.gap_evt.params.connected.peer_addr.addr,g_accessory_running.info.addr.addr,BLE_GAP_ADDR_LEN))
//						{
//							NRF_LOG_DEBUG("g_accessory_running connect");
//							g_accessory_running.info.conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
//							task_ble_conn_handle(TASK_BLE_CONN_EVT_CONNECT_RUNNING, NULL, 0);	  
//							task_ble_accessary_handle(TASK_BLE_ACCESSORY_EVT_RUNNING_SRV_DISC, NULL, 0);	
//							start_scan = true;
//						}
//						if(!memcmp(p_ble_evt->evt.gap_evt.params.connected.peer_addr.addr,g_accessory_heartrate.info.addr.addr,BLE_GAP_ADDR_LEN))
//						{
//							NRF_LOG_DEBUG("g_accessory_heartrate connect");
//							g_accessory_heartrate.info.conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
//							task_ble_conn_handle(TASK_BLE_CONN_EVT_CONNECT_HEARTRATE, NULL, 0);	   
//							task_ble_accessary_handle(TASK_BLE_ACCESSORY_EVT_HEARTRATE_SRV_DISC, NULL, 0);	
//							start_scan = true;
//						}						
//						if(!memcmp(p_ble_evt->evt.gap_evt.params.connected.peer_addr.addr,g_accessory_cycling.info.addr.addr,BLE_GAP_ADDR_LEN))
//						{
//							NRF_LOG_DEBUG("g_accessory_cycling connect");
//							g_accessory_cycling.info.conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
//							task_ble_conn_handle(TASK_BLE_CONN_EVT_CONNECT_CYCLING, NULL, 0);	 	  
//							task_ble_accessary_handle(TASK_BLE_ACCESSORY_EVT_CYCLING_SRV_DISC, NULL, 0);		
//							start_scan = true;
//						}								
//						
//						if(start_scan)
//						{
//							uint8_t conn_count = ble_conn_state_n_centrals();
//							NRF_LOG_DEBUG("conn_count --> %d",conn_count);
//							if(conn_count < NRF_SDH_BLE_CENTRAL_LINK_COUNT)
//							{
//								task_ble_accessary_handle(TASK_BLE_ACCESSORY_EVT_SCAN, NULL, 0);
//							}
//						}            
//        } break; // BLE_GAP_EVT_CONNECTED

//        // Upon disconnection, reset the connection handle of the peer which disconnected,
//        // update the LEDs status and start scanning again.
//        case BLE_GAP_EVT_DISCONNECTED:
//        {
//					bool start_scan = false;
//					if(p_ble_evt->evt.gap_evt.conn_handle == g_accessory_running.info.conn_handle)
//					{
//						NRF_LOG_DEBUG("g_accessory_running");
//						task_ble_conn_handle(TASK_BLE_CONN_EVT_DISCONNECT_RUNNING, NULL, 0);	 
//						g_accessory_running.info.uuid_found = false;
//						g_accessory_running.info.conn_handle = BLE_CONN_HANDLE_INVALID;
//						start_scan = true;
//					}
//					else if(p_ble_evt->evt.gap_evt.conn_handle == g_accessory_heartrate.info.conn_handle)
//					{
//						NRF_LOG_DEBUG("g_accessory_heartrate");
//						task_ble_conn_handle(TASK_BLE_CONN_EVT_DISCONNECT_HEARTRATE, NULL, 0);	
//						g_accessory_heartrate.info.uuid_found = false;
//						g_accessory_heartrate.info.conn_handle = BLE_CONN_HANDLE_INVALID;
//						start_scan = true;
//					}
//					else if(p_ble_evt->evt.gap_evt.conn_handle == g_accessory_cycling.info.conn_handle)
//					{
//						NRF_LOG_DEBUG("g_accessory_cycling");
//						task_ble_conn_handle(TASK_BLE_CONN_EVT_DISCONNECT_CYCLING, NULL, 0);
//						g_accessory_cycling.info.uuid_found = false;
//						g_accessory_cycling.info.conn_handle = BLE_CONN_HANDLE_INVALID;
//						start_scan = true;
//					}					
//					
//					if(start_scan)
//					{
//						uint8_t conn_count = ble_conn_state_n_centrals();
//						NRF_LOG_DEBUG("conn_count --> %d",conn_count);
//						if(conn_count < NRF_SDH_BLE_CENTRAL_LINK_COUNT)
//						{
//							if(conn_count == 0)
//							{
//								task_ble_accessary_handle(TASK_BLE_ACCESSORY_EVT_DATA_TIMER_STOP, NULL, 0);
//							}
//							task_ble_accessary_handle(TASK_BLE_ACCESSORY_EVT_SCAN, NULL, 0);
//						}
//					} 
//        } break; // BLE_GAP_EVT_DISCONNECTED

//        case BLE_GAP_EVT_ADV_REPORT:
//        {
//					
//					static ble_gap_evt_adv_report_t m_adv_report;
//					memcpy(&m_adv_report,&p_ble_evt->evt.gap_evt.params.adv_report,sizeof(ble_gap_evt_adv_report_t));

//					if(g_ht_rssi_callback != NULL)
//					{
//						//贴片厂测试蓝牙天线
//						NRF_LOG_DEBUG("hardware test");
//						g_ht_rssi_callback(&m_adv_report);
//					}
//				  else
//					{
//						//配件			
//						task_ble_accessary_handle(TASK_BLE_ACCESSORY_EVT_ADV_REPORT, (uint8_t *)&m_adv_report, sizeof(ble_gap_evt_adv_report_t));
//					}
//        } break; // BLE_GAP_ADV_REPORT

//        case BLE_GAP_EVT_TIMEOUT:
//        {
//            // We have not specified a timeout for scanning, so only connection attemps can timeout.
//            if (p_gap_evt->params.timeout.src == BLE_GAP_TIMEOUT_SRC_CONN)
//            {
//                NRF_LOG_INFO("Connection Request timed out.");
//            }
//        } break;

//        case BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST:
//        {
//            // Accept parameters requested by peer.
//            err_code = sd_ble_gap_conn_param_update(p_gap_evt->conn_handle,
//                                        &p_gap_evt->params.conn_param_update_request.conn_params);
//            APP_ERROR_CHECK(err_code);
//        } break;



//        case BLE_GATTC_EVT_TIMEOUT:
//            // Disconnect on GATT Client timeout event.
//            NRF_LOG_DEBUG("GATT Client Timeout.");
//            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
//                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
//            APP_ERROR_CHECK(err_code);
//            break;

//        case BLE_GATTS_EVT_TIMEOUT:
//            // Disconnect on GATT Server timeout event.
//            NRF_LOG_DEBUG("GATT Server Timeout.");
//            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
//                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
//            APP_ERROR_CHECK(err_code);
//            break;

//        default:
//            // No implementation needed.
//            break;
//    }
//}


/**@brief   Function for handling BLE events from peripheral applications.
 * @details Updates the status LEDs used to report the activity of the peripheral applications.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 */
//static void on_ble_peripheral_evt(ble_evt_t const * p_ble_evt)
//{
//    ret_code_t err_code;

//    switch (p_ble_evt->header.evt_id)
//    {
//        case BLE_GAP_EVT_CONNECTED:{
//            NRF_LOG_INFO("Peripheral connected");
//						g_conn_params_update_from_mobile_app = 0;
//						NRF_LOG_HEXDUMP_DEBUG(p_ble_evt->evt.gap_evt.params.connected.peer_addr.addr, BLE_GAP_ADDR_LEN);
//						memcpy(g_peer_addr.addr, p_ble_evt->evt.gap_evt.params.connected.peer_addr.addr, BLE_GAP_ADDR_LEN);				
//						g_conn_handle_peripheral = p_ble_evt->evt.gap_evt.conn_handle;
//						task_ble_conn_handle(TASK_BLE_CONN_EVT_CONNECT, NULL, 0);	            
//				} break;

//        case BLE_GAP_EVT_DISCONNECTED:{
//            NRF_LOG_INFO("Peripheral disconnected reason --> 0x%04X",p_ble_evt->evt.gap_evt.params.disconnected.reason);					
//					if(p_ble_evt->evt.gap_evt.conn_handle == g_conn_handle_peripheral)
//					{
//						NRF_LOG_DEBUG("g_conn_handle_peripheral");
//						g_conn_handle_peripheral = BLE_CONN_HANDLE_INVALID;
//						task_ble_conn_handle(TASK_BLE_CONN_EVT_DISCONNECT, NULL, 0);
//					}            
//				}break;

//        case BLE_GATTC_EVT_TIMEOUT:{
//            // Disconnect on GATT Client timeout event.
//            NRF_LOG_DEBUG("GATT Client Timeout.");
//            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
//                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
//            APP_ERROR_CHECK(err_code);
//				}break;

//        case BLE_GATTS_EVT_TIMEOUT:{
//            // Disconnect on GATT Server timeout event.
//            NRF_LOG_DEBUG("GATT Server Timeout.");
//            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
//                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
//            APP_ERROR_CHECK(err_code);
//				}break;

//        case BLE_EVT_USER_MEM_REQUEST:{
//            err_code = sd_ble_user_mem_reply(p_ble_evt->evt.gap_evt.conn_handle, NULL);
//            APP_ERROR_CHECK(err_code);
//        } break; // BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST
//        case BLE_GAP_EVT_AUTH_STATUS:{
//					NRF_LOG_DEBUG("BLE_GAP_EVT_AUTH_STATUS");
//					if(p_ble_evt->evt.gap_evt.conn_handle == g_conn_handle_peripheral)
//					{
//						if(p_ble_evt->evt.gap_evt.params.auth_status.auth_status != BLE_GAP_SEC_STATUS_SUCCESS)
//						{
//							if(p_ble_evt->evt.gap_evt.params.auth_status.auth_status == BLE_GAP_SEC_STATUS_CONFIRM_VALUE)
//							{
//								
//							}
//							NRF_LOG_DEBUG("auth_status not success error --> 0x%08X",p_ble_evt->evt.gap_evt.params.auth_status.auth_status);
//							/**<Do not check return error, because it will cause hang up in softdevice>*/
//							err_code = sd_ble_gap_disconnect(g_conn_handle_peripheral, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
//							APP_ERROR_CHECK(err_code);
//						}					
//					
//					}

//				}break;	
//        case BLE_GAP_EVT_PASSKEY_DISPLAY:{
//					NRF_LOG_DEBUG("BLE_GAP_EVT_PASSKEY_DISPLAY");
//					if(p_ble_evt->evt.gap_evt.conn_handle == g_conn_handle_peripheral)
//					{
//						memcpy(g_ble_passkey,p_ble_evt->evt.gap_evt.params.passkey_display.passkey,BLE_GAP_PASSKEY_LEN);
//						task_ble_conn_handle(TASK_BLE_CONN_EVT_PASSKEY, g_ble_passkey, BLE_GAP_PASSKEY_LEN);					
//					}

//				}break;






//				
//        default:
//            // No implementation needed.
//            break;
//    }
//}
/**@brief Function for checking if a bluetooth stack event is an advertising timeout.
 *
 * @param[in] p_ble_evt Bluetooth stack event.
 */
//static bool ble_evt_is_advertising_timeout(ble_evt_t const * p_ble_evt)
//{
//  return (   (p_ble_evt->header.evt_id == BLE_GAP_EVT_TIMEOUT)
//          && (p_ble_evt->evt.gap_evt.params.timeout.src == BLE_GAP_TIMEOUT_SRC_ADVERTISING));
//}


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




/**@brief Function for handling BLE events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
	uint16_t conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
	uint16_t role        = ble_conn_state_role(conn_handle);
	ble_gap_evt_t const * p_gap_evt = &p_ble_evt->evt.gap_evt;
	const ble_gap_addr_t  * const peer_addr = &p_gap_evt->params.adv_report.peer_addr;
	
	
	// Based on the role this device plays in the connection, dispatch to the right handler.
//	if (role == BLE_GAP_ROLE_PERIPH || ble_evt_is_advertising_timeout(p_ble_evt))
//	{
//		on_ble_peripheral_evt(p_ble_evt);
//	}
//	else if ((role == BLE_GAP_ROLE_CENTRAL) || (p_ble_evt->header.evt_id == BLE_GAP_EVT_ADV_REPORT))
//	{
//		on_ble_central_evt(p_ble_evt);
//	}	
//	

	
	
    uint32_t err_code = NRF_SUCCESS;
//		static uint8_t disconnect_reason = 0;
    switch (p_ble_evt->header.evt_id)
    {
			
        case BLE_GAP_EVT_ADV_REPORT:{
					//NRF_LOG_DEBUG("================BLE_GAP_EVT_ADV_REPORT==================");
					static ble_gap_evt_adv_report_t m_adv_report;
					memcpy(&m_adv_report,&p_ble_evt->evt.gap_evt.params.adv_report,sizeof(ble_gap_evt_adv_report_t));

					if(g_ht_rssi_callback != NULL)
					{
						//贴片厂测试蓝牙天线
						NRF_LOG_DEBUG("hardware test");
						g_ht_rssi_callback(&m_adv_report);
					}
				  else
					{
						
						//配件			
						task_ble_accessary_handle(TASK_BLE_ACCESSORY_EVT_ADV_REPORT, (uint8_t *)&m_adv_report, sizeof(ble_gap_evt_adv_report_t));
						
					}
        } break; // BLE_GAP_EVT_ADV_REPORT						
			
			
			
        case BLE_GAP_EVT_CONN_PARAM_UPDATE:{
					NRF_LOG_DEBUG("BLE_GAP_EVT_CONN_PARAM_UPDATE");
					
				}break;					
			
        case BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST:{
//            ble_gap_conn_params_t conn_params;
            NRF_LOG_DEBUG("BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST");
            //update connection parameters for accessary.
            if(BLE_GAP_ROLE_CENTRAL == ble_conn_state_role(p_ble_evt->evt.gap_evt.conn_handle))
            {
//                conn_params.min_conn_interval = p_ble_evt->evt.gap_evt.params.conn_param_update_request.conn_params.min_conn_interval;
//                conn_params.max_conn_interval = p_ble_evt->evt.gap_evt.params.conn_param_update_request.conn_params.max_conn_interval;
//                conn_params.slave_latency = p_ble_evt->evt.gap_evt.params.conn_param_update_request.conn_params.slave_latency;
//                conn_params.conn_sup_timeout = p_ble_evt->evt.gap_evt.params.conn_param_update_request.conn_params.conn_sup_timeout;
              ble_gap_evt_t const * p_gap_evt = &p_ble_evt->evt.gap_evt;  
							err_code = sd_ble_gap_conn_param_update(p_ble_evt->evt.gap_evt.conn_handle,
																												&p_gap_evt->params.conn_param_update_request.conn_params);
							APP_ERROR_CHECK(err_code);
            }
        }break;				
			
        case BLE_GAP_EVT_AUTH_STATUS:{
					NRF_LOG_DEBUG("BLE_GAP_EVT_AUTH_STATUS");
					if(p_ble_evt->evt.gap_evt.params.auth_status.auth_status != BLE_GAP_SEC_STATUS_SUCCESS)
					{
//						if(p_ble_evt->evt.gap_evt.params.auth_status.auth_status == BLE_GAP_SEC_STATUS_CONFIRM_VALUE)
//						{
//							disconnect_reason = TASK_BLE_CONN_DISCONNECT_REASON_WRONG_PASSKEY;
//						}
						NRF_LOG_DEBUG("auth_status not success error --> 0x%08X",p_ble_evt->evt.gap_evt.params.auth_status.auth_status);
						if(p_ble_evt->evt.gap_evt.params.auth_status.auth_status == BLE_GAP_SEC_STATUS_UNSPECIFIED)
						{
							task_ble_conn_handle(TASK_BLE_CONN_EVT_PAIRING_FAILED, NULL, 0);
						}


						/**<Do not check return error, because it will cause hang up in softdevice>*/
//						err_code = sd_ble_gap_disconnect(g_conn_handle_peripheral, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
//						NRF_LOG_DEBUG("sd_ble_gap_disconnect --> 0x%08X",err_code);
//						APP_ERROR_CHECK(err_code);
					}
				}break;			
			
			
        case BLE_GAP_EVT_PASSKEY_DISPLAY:{
					NRF_LOG_DEBUG("BLE_GAP_EVT_PASSKEY_DISPLAY");
					memcpy(g_ble_passkey,p_ble_evt->evt.gap_evt.params.passkey_display.passkey,BLE_GAP_PASSKEY_LEN);
					NRF_LOG_HEXDUMP_INFO(g_ble_passkey, BLE_GAP_PASSKEY_LEN) ;
					task_ble_conn_handle(TASK_BLE_CONN_EVT_PASSKEY,  g_ble_passkey, BLE_GAP_PASSKEY_LEN);
				
					
					
				}break;			
        case BLE_GAP_EVT_DISCONNECTED:{
					NRF_LOG_DEBUG("BLE_GAP_EVT_DISCONNECTED");
					NRF_LOG_DEBUG("REASON --> 0x%02X",p_ble_evt->evt.gap_evt.params.disconnected.reason);//BLE_HCI_STATUS_CODES

					bool start_scan = false;
					if(p_ble_evt->evt.gap_evt.conn_handle == g_conn_handle_peripheral)
					{
						NRF_LOG_DEBUG("g_conn_handle_peripheral");
						g_conn_handle_peripheral = BLE_CONN_HANDLE_INVALID;
						
//						if(disconnect_reason == TASK_BLE_CONN_DISCONNECT_REASON_WRONG_PASSKEY)
//						{
//							disconnect_reason = 0;
//							uint8_t reason = TASK_BLE_CONN_DISCONNECT_REASON_WRONG_PASSKEY;
//							task_ble_conn_handle(TASK_BLE_CONN_EVT_DISCONNECT, &reason, 1);							
//						}
//						else
						{
							
							ble_conn_timer_stop();
							m_ble_conn_timer_evt = BLE_CONN_TIMEOUT_EVT_DISCONNECTTED;
							ble_conn_timer_start(CONN_TIMER_TIMEOUT);
							
						}	
					}
					else if(p_ble_evt->evt.gap_evt.conn_handle == g_accessory_running.info.conn_handle)
					{
						NRF_LOG_DEBUG("g_accessory_running");
//						task_ble_conn_handle(TASK_BLE_CONN_EVT_DISCONNECT_RUNNING, NULL, 0);	
						g_accessory_running.info.uuid_found = false;
						g_accessory_running.info.conn_handle = BLE_CONN_HANDLE_INVALID;
						start_scan = true;

						ble_conn_timer_stop();
						m_ble_conn_timer_evt_running = BLE_CONN_TIMEOUT_EVT_RUNNING_DISCONNECTTED;
						ble_conn_timer_start(ACCESSARY_CONN_TIMER_TIMEOUT);
					}
					else if(p_ble_evt->evt.gap_evt.conn_handle == g_accessory_heartrate.info.conn_handle)
					{
						NRF_LOG_DEBUG("g_accessory_heartrate");
//						task_ble_conn_handle(TASK_BLE_CONN_EVT_DISCONNECT_HEARTRATE, NULL, 0);	
						g_accessory_heartrate.info.uuid_found = false;
						g_accessory_heartrate.info.conn_handle = BLE_CONN_HANDLE_INVALID;
						start_scan = true;

						ble_conn_timer_stop();
						m_ble_conn_timer_evt_heartrate = BLE_CONN_TIMEOUT_EVT_HEARTRATE_DISCONNECTTED;
						ble_conn_timer_start(ACCESSARY_CONN_TIMER_TIMEOUT);
					}
					else if(p_ble_evt->evt.gap_evt.conn_handle == g_accessory_cycling.info.conn_handle)
					{
						NRF_LOG_DEBUG("g_accessory_cycling");
//						task_ble_conn_handle(TASK_BLE_CONN_EVT_DISCONNECT_CYCLING, NULL, 0);
						g_accessory_cycling.info.uuid_found = false;
						g_accessory_cycling.info.conn_handle = BLE_CONN_HANDLE_INVALID;
						start_scan = true;

						ble_conn_timer_stop();
						m_ble_conn_timer_evt_cycling = BLE_CONN_TIMEOUT_EVT_CYCLING_DISCONNECTTED;
						ble_conn_timer_start(ACCESSARY_CONN_TIMER_TIMEOUT);
					}

					
					if(start_scan)
					{
						uint8_t conn_count = ble_conn_state_n_centrals();
						NRF_LOG_DEBUG("conn_count --> %d",conn_count);
						if(conn_count < NRF_SDH_BLE_CENTRAL_LINK_COUNT)
						{

							task_ble_accessary_handle(TASK_BLE_ACCESSORY_EVT_SCAN, NULL, 0);

						}
						
						if(conn_count == 0)
						{
							task_ble_accessary_handle(TASK_BLE_ACCESSORY_EVT_DATA_TIMER_STOP, NULL, 0);
						}
						
					}
				}break;

				case BLE_GAP_EVT_DATA_LENGTH_UPDATE_REQUEST:{
					NRF_LOG_DEBUG("BLE_GAP_EVT_DATA_LENGTH_UPDATE_REQUEST");
	
				
				}break;
				
				
        case BLE_GAP_EVT_CONNECTED:{
					NRF_LOG_DEBUG("BLE_GAP_EVT_CONNECTED");
					
					if (p_ble_evt->evt.gap_evt.params.connected.role == BLE_GAP_ROLE_PERIPH)
					{

						
						
						
						g_conn_params_update_from_mobile_app = 0;
						NRF_LOG_HEXDUMP_DEBUG(p_ble_evt->evt.gap_evt.params.connected.peer_addr.addr, BLE_GAP_ADDR_LEN);
						memcpy(g_peer_addr.addr, p_ble_evt->evt.gap_evt.params.connected.peer_addr.addr, BLE_GAP_ADDR_LEN);				
						g_conn_handle_peripheral = p_ble_evt->evt.gap_evt.conn_handle;
						
						ble_conn_timer_stop();
						m_ble_conn_timer_evt = BLE_CONN_TIMEOUT_EVT_CONNECTTED;
						ble_conn_timer_start(CONN_TIMER_TIMEOUT);						 
						
						
					}
					else{
						NRF_LOG_DEBUG("BLE_GAP_ROLE_CENTRAL");
						bool start_scan = false;
						
						if(!memcmp(p_ble_evt->evt.gap_evt.params.connected.peer_addr.addr,g_accessory_running.info.addr.addr,BLE_GAP_ADDR_LEN))
						{
							NRF_LOG_DEBUG("g_accessory_running connect");
							g_accessory_running.info.conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
//							task_ble_conn_handle(TASK_BLE_CONN_EVT_CONNECT_RUNNING, NULL, 0);	 
							task_ble_accessary_handle(TASK_BLE_ACCESSORY_EVT_RUNNING_SRV_DISC, NULL, 0);	
							start_scan = true;

							ble_conn_timer_stop();
							m_ble_conn_timer_evt_running = BLE_CONN_TIMEOUT_EVT_RUNNING_CONNECTTED;
							ble_conn_timer_start(ACCESSARY_CONN_TIMER_TIMEOUT);
						}
						if(!memcmp(p_ble_evt->evt.gap_evt.params.connected.peer_addr.addr,g_accessory_heartrate.info.addr.addr,BLE_GAP_ADDR_LEN))
						{
							NRF_LOG_DEBUG("g_accessory_heartrate connect");
							g_accessory_heartrate.info.conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
//							task_ble_conn_handle(TASK_BLE_CONN_EVT_CONNECT_HEARTRATE, NULL, 0);	  
							task_ble_accessary_handle(TASK_BLE_ACCESSORY_EVT_HEARTRATE_SRV_DISC, NULL, 0);	
							start_scan = true;

							ble_conn_timer_stop();
							m_ble_conn_timer_evt_heartrate = BLE_CONN_TIMEOUT_EVT_HEARTRATE_CONNECTTED;
							ble_conn_timer_start(ACCESSARY_CONN_TIMER_TIMEOUT);
						}						
						if(!memcmp(p_ble_evt->evt.gap_evt.params.connected.peer_addr.addr,g_accessory_cycling.info.addr.addr,BLE_GAP_ADDR_LEN))
						{
							NRF_LOG_DEBUG("g_accessory_cycling connect");
							g_accessory_cycling.info.conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
//							task_ble_conn_handle(TASK_BLE_CONN_EVT_CONNECT_CYCLING, NULL, 0);	 	 
							task_ble_accessary_handle(TASK_BLE_ACCESSORY_EVT_CYCLING_SRV_DISC, NULL, 0);	
							start_scan = true;

							ble_conn_timer_stop();
							m_ble_conn_timer_evt_cycling = BLE_CONN_TIMEOUT_EVT_CYCLING_CONNECTTED;
							ble_conn_timer_start(ACCESSARY_CONN_TIMER_TIMEOUT);
						}
					
						
						if(start_scan)
						{
							uint8_t conn_count = ble_conn_state_n_centrals();
							NRF_LOG_DEBUG("conn_count --> %d",conn_count);
							if(conn_count < NRF_SDH_BLE_CENTRAL_LINK_COUNT)
							{
								task_ble_accessary_handle(TASK_BLE_ACCESSORY_EVT_SCAN, NULL, 0);
							}
						}

					}
				}break;

#ifndef S140
        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
        {
            NRF_LOG_DEBUG("PHY update request.");
            ble_gap_phys_t const phys =
            {
                .rx_phys = BLE_GAP_PHY_AUTO,
                .tx_phys = BLE_GAP_PHY_AUTO,
            };
            err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
            APP_ERROR_CHECK(err_code);
        } break;
#endif

        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect on GATT Client timeout event.
            NRF_LOG_DEBUG("GATT Client Timeout.");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
            NRF_LOG_DEBUG("GATT Server Timeout.");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_EVT_USER_MEM_REQUEST:
            err_code = sd_ble_user_mem_reply(p_ble_evt->evt.gattc_evt.conn_handle, NULL);
            APP_ERROR_CHECK(err_code);
            break;
        
#if !defined WATCH_HAS_HYTERA_FUNC && !defined BLE_ALWAYS_DISCONNECT_DEBUG
        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
		NRF_LOG_DEBUG("BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST");
        {
            ble_gatts_evt_rw_authorize_request_t  req;
            ble_gatts_rw_authorize_reply_params_t auth_reply;

            req = p_ble_evt->evt.gatts_evt.params.authorize_request;

            if (req.type != BLE_GATTS_AUTHORIZE_TYPE_INVALID)
            {
                if ((req.request.write.op == BLE_GATTS_OP_PREP_WRITE_REQ)     ||
                    (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_NOW) ||
                    (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_CANCEL))
                {
                    if (req.type == BLE_GATTS_AUTHORIZE_TYPE_WRITE)
                    {
                        auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
                    }
                    else
                    {
                        auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_READ;
                    }
                    auth_reply.params.write.gatt_status = APP_FEATURE_NOT_SUPPORTED;//BLE_GATT_STATUS_ATTERR_WRITE_NOT_PERMITTED
                    err_code = sd_ble_gatts_rw_authorize_reply(p_ble_evt->evt.gatts_evt.conn_handle,
                                                               &auth_reply);
                    APP_ERROR_CHECK(err_code);
                }
            }
        } break; // BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST
#endif
        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
	
//		ble_opt_t      ble_opt;
//		memset(&ble_opt,0,sizeof(ble_opt_t));
//		ble_opt.gap_opt.compat_mode_1.enable = true;
//		err_code = sd_ble_opt_set(BLE_GAP_OPT_COMPAT_MODE_1, &ble_opt);
//		APP_ERROR_CHECK(err_code);



	
}


/**@brief Function for the Peer Manager initialization.
 */
static void peer_manager_init()
{
    ble_gap_sec_params_t sec_param;
    ret_code_t           err_code;

    err_code = pm_init();
    APP_ERROR_CHECK(err_code);

    memset(&sec_param, 0, sizeof(ble_gap_sec_params_t));

    // Security parameters to be used for all security procedures.
    sec_param.bond           = SEC_PARAM_BOND;
    sec_param.mitm           = SEC_PARAM_MITM;
    sec_param.lesc           = SEC_PARAM_LESC;
    sec_param.keypress       = SEC_PARAM_KEYPRESS;
    sec_param.io_caps        = SEC_PARAM_IO_CAPABILITIES;
    sec_param.oob            = SEC_PARAM_OOB;
    sec_param.min_key_size   = SEC_PARAM_MIN_KEY_SIZE;
    sec_param.max_key_size   = SEC_PARAM_MAX_KEY_SIZE;
    sec_param.kdist_own.enc  = 1;
    sec_param.kdist_own.id   = 1;
    sec_param.kdist_peer.enc = 1;
    sec_param.kdist_peer.id  = 1;

    err_code = pm_sec_params_set(&sec_param);
    APP_ERROR_CHECK(err_code);

    err_code = pm_register(pm_evt_handler);
    APP_ERROR_CHECK(err_code);
}


/** @brief Clear bonding information from persistent storage.
 */
static void delete_bonds(void)
{
    ret_code_t err_code;

    NRF_LOG_DEBUG("Erase bonds!");

    err_code = pm_peers_delete();
    APP_ERROR_CHECK(err_code);
}



/**@brief Function for initializing the Advertising functionality.
 */
void advertising_init(void)
{
    uint32_t               err_code;
    ble_advertising_init_t init;
		ble_gap_addr_t device_addr;
		
	  ble_advdata_manuf_data_t        manuf_data;
	#ifdef COD
		err_code = drv_ble_flash_cod_mac_read(&device_addr);
		APP_ERROR_CHECK(err_code);
	#endif
		err_code = sd_ble_gap_addr_get(&device_addr);
		APP_ERROR_CHECK(err_code);


		uint8_t *d = device_addr.addr;
		for ( uint8_t i = 6; i >0;)
		{	
			i--;
			g_adv_manuf_mac[5-i]= d[i];
		}		
	
	
    memset(&init, 0, sizeof(init));

    init.advdata.name_type               = BLE_ADVDATA_FULL_NAME;
    init.advdata.include_appearance      = false;
		
#ifdef RSSI_ADV_DEVICE
		init.advdata.flags                   = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
#else
		init.advdata.flags                   = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
#endif			
		

    init.srdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    init.srdata.uuids_complete.p_uuids  = m_adv_uuids;
    

#ifdef COD	
	memset(g_scan_manuf_data, 0, sizeof(g_scan_manuf_data));
	//g_scan_manuf_data[0] = COD_01_ID;
	for ( uint8_t i = 0; i< 6;i++)
	{	
		g_scan_manuf_data[i+5]= g_adv_manuf_mac[i];
	}
	manuf_data.company_identifier     = COMPANY_IDENTIFIER;
    manuf_data.data.size              = sizeof(g_scan_manuf_data);
    manuf_data.data.p_data            = g_scan_manuf_data;
    init.srdata.p_manuf_specific_data = &manuf_data;	
#else				
    manuf_data.company_identifier     = COMPANY_IDENTIFIER;
    manuf_data.data.size              = sizeof(g_adv_manuf_mac);
    manuf_data.data.p_data            = g_adv_manuf_mac;
    init.srdata.p_manuf_specific_data = &manuf_data;			
#endif		


#ifdef RSSI_ADV_DEVICE
    init.config.ble_adv_fast_enabled  = true;
    init.config.ble_adv_fast_interval = APP_ADV_INTERVAL_FAST;
    init.config.ble_adv_fast_timeout  = 0;
#else
    init.config.ble_adv_fast_enabled  = g_ble_config->adv_params.enable_fast;
    init.config.ble_adv_fast_interval = g_ble_config->adv_params.interval_fast;
    init.config.ble_adv_fast_timeout  = g_ble_config->adv_params.timeout_fast;
    init.config.ble_adv_slow_enabled  = g_ble_config->adv_params.enable_slow;
    init.config.ble_adv_slow_interval = g_ble_config->adv_params.interval_slow;
    init.config.ble_adv_slow_timeout  = g_ble_config->adv_params.timeout_slow;
#endif	



	

#ifdef RSSI_ADV_DEVICE
		int8_t tx_power_level = 4;
		init.advdata.p_tx_power_level = &tx_power_level;
		init.srdata.p_tx_power_level = &tx_power_level;
#else
		int8_t tx_power_level = TX_DBM_VALUE;
		init.advdata.p_tx_power_level = &tx_power_level;
		init.srdata.p_tx_power_level = &tx_power_level;
#endif			

	
    init.evt_handler = on_adv_evt;


    err_code = ble_advertising_init(&m_advertising, &init);
    APP_ERROR_CHECK(err_code);

    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}





/**@brief Function for the Power manager.
 */
static void log_init(void)
{
    uint32_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

/**@brief GATT module event handler.
 */
extern bool g_mtu_update_from_mobile_app;
static void gatt_evt_handler(nrf_ble_gatt_t * p_gatt, nrf_ble_gatt_evt_t const * p_evt)
{
    switch (p_evt->evt_id)
    {
        case NRF_BLE_GATT_EVT_ATT_MTU_UPDATED:
        {
					NRF_LOG_INFO("NRF_BLE_GATT_EVT_ATT_MTU_UPDATED");
            NRF_LOG_INFO("GATT ATT MTU on connection 0x%x changed to %d.",
                         p_evt->conn_handle,
                         p_evt->params.att_mtu_effective);
					
        } break;

        case NRF_BLE_GATT_EVT_DATA_LENGTH_UPDATED:
        {
					NRF_LOG_INFO("NRF_BLE_GATT_EVT_DATA_LENGTH_UPDATED");
            NRF_LOG_INFO("Data length for connection 0x%x updated to %d.",
                         p_evt->conn_handle,
                         p_evt->params.data_length);
					if(g_mtu_update_from_mobile_app)
					{
						
						uint8_t response[3] = {CMD_NRF52_SRV_MTU_REQUEST_RESPONSE};
						uint16_t mtu = nrf_ble_gatt_eff_mtu_get(&m_gatt, g_conn_handle_peripheral) - 3;	
						response[1] = mtu>>8;
						response[2] = mtu;
						app_srv_nrf52_config_send((uint8_t *)response, sizeof(response));						
					
					}
			
					
        } break;

        default:
            break;
    }
}
/**@brief   Function for initializing the GATT module.
 * @details The GATT module handles ATT_MTU and Data Length update procedures automatically.
 */
static void gatt_init(void)
{
    ret_code_t err_code = nrf_ble_gatt_init(&m_gatt, gatt_evt_handler);
    APP_ERROR_CHECK(err_code);
	
}


/**@brief Function for starting advertising. */
static void advertising_start(void * p_erase_bonds)
{

    if (m_erase_bonds)
    {
        delete_bonds();
        // Advertising is started by PM_EVT_PEERS_DELETE_SUCCEEDED event.
    }
    else
    {
        ret_code_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
        APP_ERROR_CHECK(err_code);
    }
}

void ble_adv_start(void)
{
     ret_code_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
     APP_ERROR_CHECK(err_code);
}


#if NRF_LOG_ENABLED
/**@brief Thread for handling the logger.
 *
 * @details This thread is responsible for processing log entries if logs are deferred.
 *          Thread flushes all log entries and suspends. It is resumed by idle task hook.
 *
 * @param[in]   arg   Pointer used for passing some arbitrary information (context) from the
 *                    osThreadCreate() call to the thread.
 */
static void logger_thread(void * arg)
{
    UNUSED_PARAMETER(arg);

    while (1)
    {
        NRF_LOG_FLUSH();

        vTaskSuspend(NULL); // Suspend myself
    }
}
#endif //NRF_LOG_ENABLED
/**@brief A function which is hooked to idle task.
 * @note Idle hook must be enabled in FreeRTOS configuration (configUSE_IDLE_HOOK).
 */
void vApplicationIdleHook( void )
{
	
#if NRF_LOG_ENABLED	
   vTaskResume(m_logger_thread);  
#endif //NRF_LOG_ENABLED	
	
	
}
void vApplicationMallocFailedHook(void)
{
	NRF_LOG_ERROR("vApplicationMallocFailedHook");
	APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
	
    while (1);
}

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
{
	(void) pcTaskName;
	(void) pxTask;

	NRF_LOG_ERROR(" read_m_state%d,xPortGetFreeHeapSize -- > %d",read_m_state(),xPortGetFreeHeapSize());
	APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
	while (1);
}
#ifdef BOARD_PCA10040
#include "drv_apollo2.h"
/**@brief Function for handling events from the BSP module.
 *
 * @param[in]   event   Event generated by button press.
 */
void bsp_event_handler(bsp_event_t event)
{

    switch (event)
    {
        case BSP_EVENT_KEY_0:{
            NRF_LOG_DEBUG("BSP_EVENT_KEY_0");
					
					
				}break;

        case BSP_EVENT_KEY_1:{
            NRF_LOG_DEBUG("BSP_EVENT_KEY_1");
				}break;
				
        case BSP_EVENT_KEY_2:{
            NRF_LOG_DEBUG("BSP_EVENT_KEY_2");
				}break;

        case BSP_EVENT_KEY_3:{
            NRF_LOG_DEBUG("BSP_EVENT_KEY_3");
				}break;				

        default:
            break;
    }
}
/**@brief Function for initializing buttons and leds.
 *
 * @param[out] p_erase_bonds  Will be true if the clear bonding button was pressed to wake the application up.
 */
static void buttons_leds_init(bool * p_erase_bonds)
{

    uint32_t err_code = bsp_init(BSP_INIT_BUTTONS, bsp_event_handler);
    APP_ERROR_CHECK(err_code);

}
#endif		
/**@brief Function for initializing the clock.
 */
static void clock_init(void)
{
    ret_code_t err_code = nrf_drv_clock_init();
    APP_ERROR_CHECK(err_code);
}


static void ble_flash_config(void)
{
//	NRF_LOG_DEBUG("ble_flash_config");
	uint32_t err_code;
	uint32_t mac = NRF_FICR->DEVICEADDR[0];
	uint8_t mac_5 = (mac<<16)>>24;
	uint8_t mac_6 = (mac<<24)>>24;
	memset((char *)g_ble_config_default_config.dev_name.name,0,g_ble_config_default_config.dev_name.len);
#ifdef COD
	ble_gap_addr_t device_addr;
	sprintf((char *)g_ble_config_default_config.dev_name.name,"%s","COD_WATCH");
#else
	#ifdef PA806
	sprintf((char *)g_ble_config_default_config.dev_name.name,"TA806_%02X%02X",mac_5,mac_6);
	#else
	sprintf((char *)g_ble_config_default_config.dev_name.name,"HY100_%02X%02X",mac_5,mac_6);
	#endif
#endif



	g_ble_config_default_config.dev_name.len = 10;
	err_code = drv_ble_flash_init(&g_ble_config_default_config, &g_ble_config);
	APP_ERROR_CHECK(err_code);
	NRF_LOG_DEBUG("g_ble_config->dev_name.len --> %d,%d-%d",g_ble_config->dev_name.len, g_ble_config->conn_params.max_conn_int, g_ble_config->conn_params.min_conn_int);
	NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
	
	
	g_ancs_switch.call = ANCS_MESSAGE_SWITCH_ON;
	g_ancs_switch.wechat = ANCS_MESSAGE_SWITCH_ON;
	g_ancs_switch.qq = ANCS_MESSAGE_SWITCH_ON;
	g_ancs_switch.sms = ANCS_MESSAGE_SWITCH_ON;
	#ifdef COD
	g_ancs_switch.company_wechat = ANCS_MESSAGE_SWITCH_ON;
	g_ancs_switch.dingding = ANCS_MESSAGE_SWITCH_ON;
	#endif
	err_code = drv_ble_flash_ancs_switch_init(&g_ancs_switch);	
	APP_ERROR_CHECK(err_code);
	
//	NRF_LOG_DEBUG("g_ancs_switch.call --> %d",g_ancs_switch.call);
//	NRF_LOG_DEBUG("g_ancs_switch.wechat --> %d",g_ancs_switch.wechat);
//	NRF_LOG_DEBUG("g_ancs_switch.qq --> %d",g_ancs_switch.qq);
//	NRF_LOG_DEBUG("g_ancs_switch.sms --> %d",g_ancs_switch.sms);
	#ifdef COD 
	device_addr.addr_id_peer = 0;
	device_addr.addr_type = BLE_GAP_ADDR_TYPE_RANDOM_STATIC;
	device_addr.addr[0] = NRF_FICR->DEVICEADDR[0];
	device_addr.addr[1] = NRF_FICR->DEVICEADDR[0] >> 8 ;
	device_addr.addr[2] = NRF_FICR->DEVICEADDR[0] >> 16;
	device_addr.addr[3] = NRF_FICR->DEVICEADDR[0] >> 24;
	device_addr.addr[4] = NRF_FICR->DEVICEADDR[1] ;
	device_addr.addr[5] = (NRF_FICR->DEVICEADDR[1] >> 8)|0xD0;
	NRF_LOG_DEBUG("ble_flash_config device_addr.addr %x:%x:%x:%x:%x:%x",device_addr.addr[0],device_addr.addr[1],device_addr.addr[2],device_addr.addr[3],device_addr.addr[4],device_addr.addr[5]);
	//NRF_LOG_DEBUG("ble_flash_config DEVICEADDR[0] %x:%x:%x:%x",(uint8_t) NRF_FICR->DEVICEADDR[0], (uint8_t) (NRF_FICR->DEVICEADDR[0]>>8), (uint8_t) (NRF_FICR->DEVICEADDR[0] >> 16), (uint8_t) (NRF_FICR->DEVICEADDR[0]>>24));
	//NRF_LOG_DEBUG("ble_flash_config DEVICEADDR[0] %x:%x:%x:%x",(uint8_t) NRF_FICR->DEVICEADDR[1], (uint8_t) (NRF_FICR->DEVICEADDR[1]>>8), (uint8_t) (NRF_FICR->DEVICEADDR[1] >> 16), (uint8_t) (NRF_FICR->DEVICEADDR[1]>>24));
	err_code = drv_ble_flash_cod_mac_init(&device_addr);
	APP_ERROR_CHECK(err_code);
	#else
	err_code = drv_ble_flash_android_mac_init();
	APP_ERROR_CHECK(err_code);
	#endif

	

	
}
static void sys_off_pin_high(void)
{
	

	#if NRF_LOG_ENABLED == 1
		NRF_LOG_DEBUG("sys_off_pin_high : SYSTEM OFF PIN HIGH : BLE OFF");
		NRF_LOG_FLUSH();
		nrf_delay_ms(400);
	#endif

	if(xTaskGetSchedulerState() == taskSCHEDULER_RUNNING)
	{
//		taskENTER_CRITICAL();
		taskENTER_CRITICAL_FROM_ISR();
	}
	uint32_t err_code;
	uint8_t softdevice_enabled;
	err_code = sd_softdevice_is_enabled(&softdevice_enabled);
	APP_ERROR_CHECK(err_code);
	if(softdevice_enabled)
	{
		err_code = sd_softdevice_disable();
		APP_ERROR_CHECK(err_code);	
	}
	if (nrf_drv_gpiote_is_init())
	{
			nrf_drv_gpiote_uninit();	
	}	
  for(uint32_t i=0;i<32;i++)		
	{
		if(i!=21)
		{
			nrf_gpio_cfg_default(i);
		}
	}
	nrf_power_gpregret_set(CMD_GPREGRET_BOOTLOADER_PIN_RESET);
	nrf_gpio_cfg_sense_input(SYS_OFF_PIN,NRF_GPIO_PIN_NOPULL,NRF_GPIO_PIN_SENSE_LOW);	
	nrf_power_system_off();

	
	
//	nrf_pwr_mgmt_shutdown(NRF_PWR_MGMT_SHUTDOWN_GOTO_SYSOFF);	
}
static void sys_off_pin_low(void)
{
	NRF_LOG_DEBUG("sys_off_pin_high : SYSTEM OFF PIN LOW : BLE ON");


	
	
	
}
static void sys_off_pin_gpiote_event_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	NRF_LOG_DEBUG("sys_off_pin_gpiote_event_handler");
	if (nrf_drv_gpiote_in_is_set(SYS_OFF_PIN))
	{
			sys_off_pin_high();
	}
	else
	{
			sys_off_pin_low();
	}	
}

bool g_app_error_reset_flag = false;
bool g_app_other_reset_flag =false;

bool g_ble_dfu_finish_flag = false;
static void reset_handler()
{
	uint32_t gpregret = nrf_power_gpregret_get();
	nrf_power_gpregret_set(0);



	/**
	
	SYSTEM OFF PIN LOW : BLE ON
	SYSTEM OFF PIN HIGH : BLE OFF
	
	*/
	
	
	
	nrf_gpio_cfg_sense_input(SYS_OFF_PIN,NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_SENSE_LOW);
	if(nrf_gpio_pin_read(SYS_OFF_PIN))
	{		
		nrf_power_system_off();
		while(1);
	}
	nrf_gpio_cfg_input(EXTFLASH_READY_PIN, NRF_GPIO_PIN_PULLUP);
	uint32_t err_code;
	if (!nrf_drv_gpiote_is_init())
	{
			err_code = nrf_drv_gpiote_init();
			APP_ERROR_CHECK(err_code);
	}
	nrf_drv_gpiote_in_config_t config;
	config.is_watcher = false;                    
	config.hi_accuracy = false;                  
	config.pull = NRF_GPIO_PIN_NOPULL;
	config.sense = NRF_GPIOTE_POLARITY_LOTOHI;
	err_code = nrf_drv_gpiote_in_init(SYS_OFF_PIN, &config, sys_off_pin_gpiote_event_handler);
	APP_ERROR_CHECK(err_code);	
	nrf_drv_gpiote_in_event_enable(SYS_OFF_PIN, true);	
	
	if(CMD_GPREGRET_APPLICATION_APP_ERROR_RESET == gpregret)
	{
		NRF_LOG_DEBUG("CMD_GPREGRET_APPLICATION_APP_ERROR_RESET");
		g_app_error_reset_flag = true;
	}

	else if(CMD_GPREGRET_BOOTLOADER_BLE_DFU_FINISH == gpregret)
	{
		NRF_LOG_DEBUG("CMD_GPREGRET_BOOTLOADER_BLE_DFU_FINISH");
		g_ble_dfu_finish_flag = true;
	}
	else if (gpregret == 0 )
	{
		g_app_other_reset_flag = true;
		
	}

	

}


static void flash_cs_gpio_config_init(void)
{
	nrf_gpio_cfg_default(EXTFLASH_SPI_SS_PIN);
	nrf_gpio_cfg_default(EXTFLASH_SPI_SCK_PIN);
	nrf_gpio_cfg_default(EXTFLASH_SPI_MOSI_PIN);
	nrf_gpio_cfg_default(EXTFLASH_SPI_MISO_PIN);
	
	
	
}
/**@brief Function for handling Database Discovery events.
 *
 * @details This function is a callback function to handle events from the database discovery module.
 *          Depending on the UUIDs that are discovered, this function should forward the events
 *          to their respective service instances.
 *
 * @param[in] p_event  Pointer to the database discovery event.
 */

static void db_disc_handler(ble_db_discovery_evt_t * p_evt)
{
//	NRF_LOG_DEBUG("db_disc_handler uuid --> 0x%04X",p_evt->params.discovered_db.srv_uuid.uuid);
	
	if(g_is_central_mode)
	{
		ble_hrs_on_db_disc_evt(&m_hrs_c, p_evt);
		ble_rscs_on_db_disc_evt(&m_rscs_c, p_evt);
		ble_cscs_on_db_disc_evt(&m_cscs_c, p_evt);
	
	}
	else{
		
		if(p_evt->params.discovered_db.srv_uuid.uuid == ANCS_UUID_SERVICE
			)
		{
			app_srv_ancs_ble_ancs_c_on_db_disc_evt(p_evt);
		}
		
	
	}
	
//	if(p_evt->params.discovered_db.srv_uuid.uuid == ANCS_UUID_SERVICE && p_evt->conn_handle == g_conn_handle_peripheral)	
//	{
//		NRF_LOG_DEBUG("ANCS_UUID_SERVICE");
//		app_srv_ancs_ble_ancs_c_on_db_disc_evt(p_evt);
//	}
//	else if(p_evt->params.discovered_db.srv_uuid.uuid == BLE_UUID_HEART_RATE_SERVICE && p_evt->conn_handle == g_accessory_heartrate.info.conn_handle)
//	{
//		NRF_LOG_DEBUG("BLE_UUID_HEART_RATE_SERVICE");
//		ble_hrs_on_db_disc_evt(&m_hrs_c, p_evt);
//	}
//	else if(p_evt->params.discovered_db.srv_uuid.uuid == BLE_UUID_RUNNING_SPEED_AND_CADENCE && p_evt->conn_handle == g_accessory_running.info.conn_handle)
//	{
//		NRF_LOG_DEBUG("BLE_UUID_RUNNING_SPEED_AND_CADENCE");
//		ble_rscs_on_db_disc_evt(&m_rscs_c, p_evt);
//	}	
//	else if(p_evt->params.discovered_db.srv_uuid.uuid == BLE_UUID_CYCLING_SPEED_AND_CADENCE && p_evt->conn_handle == g_accessory_cycling.info.conn_handle)
//	{
//		NRF_LOG_DEBUG("BLE_UUID_CYCLING_SPEED_AND_CADENCE");
//		ble_cscs_on_db_disc_evt(&m_cscs_c, p_evt);
//	}		
	
	
	
	
}
/**@brief Function for initializing the database discovery module.
 */
static void db_discovery_init(void)
{
    uint32_t error = ble_db_discovery_init(db_disc_handler);
    APP_ERROR_CHECK(error);
}



/**@brief Function for application main entry.
 */
int main(void)
{
    
		uint32_t error = 0;


//		nrf_power_system_off();
	
		
		reset_handler();
		
		
		clock_init();

    // Do not start any interrupt that uses system functions before system initialisation.
    // The best solution is to start the OS before any other initalisation.
		log_init();

#if NRF_LOG_ENABLED
    // Start execution.
    if (pdPASS != xTaskCreate(logger_thread, "LOGGER", 256, NULL, 1, &m_logger_thread))
    {
        APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
    }
#endif	
	
		/**<This will affect NVIC_SystemReset and sd_power_system_off>*/
    // Activate deep sleep mode.
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;


#ifdef BOARD_PCA10040
		buttons_leds_init(&erase_bonds);
#endif		

		ble_flash_config();		
		ble_stack_init();		
		error = sd_power_dcdc_mode_set(NRF_POWER_DCDC_ENABLE);
		APP_ERROR_CHECK(error);
		timers_init();
		gap_params_init();
    gatt_init();
		db_discovery_init();
		advertising_init();	
    services_init();
    conn_params_init();
		peer_manager_init();
		
	  
#ifdef RSSI_ADV_DEVICE
		error = sd_ble_gap_tx_power_set(4);
		APP_ERROR_CHECK(error);
#else
		error = sd_ble_gap_tx_power_set(TX_DBM_VALUE);
		APP_ERROR_CHECK(error);
#endif		

		
    NRF_LOG_INFO("Application started");



		flash_cs_gpio_config_init();		


#ifdef RSSI_ADV_DEVICE

#else

	
		task_apollo2_init(NULL);
		task_ble_conn_init(NULL);
		task_extFlash_init(NULL);
		task_ble_accessary_init(NULL);
		//cod_sdk_init();
	
#endif	
		NRF_LOG_DEBUG("m_erase_bonds --> %d",m_erase_bonds);
    // Create a FreeRTOS task for the BLE stack.
    // The task will run advertising_start() before entering its loop.
    nrf_sdh_freertos_init(advertising_start, NULL);
		drv_wdt_init();

    // Start FreeRTOS scheduler.
    vTaskStartScheduler();

    while (true)
    {
        APP_ERROR_HANDLER(NRF_ERROR_FORBIDDEN);
    }
}

/**
 * @}
 */
