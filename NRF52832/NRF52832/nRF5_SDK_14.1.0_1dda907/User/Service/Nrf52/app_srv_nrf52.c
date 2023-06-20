#include "app_srv_nrf52.h"
#include "ble_srv_nrf52.h"
#include "drv_ble_flash.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "cmd_nrf52_srv.h"
#include "dev_apollo2.h"
#include "cmd_gpregret.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_delay.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "peer_manager.h"
#include "drv_apollo2.h"
#include "cmd_apollo2.h"
#include "task_ble_conn.h"
#include "drv_ble_flash.h"
#include "nrf_ble_gatt.h"
#include "ble_db_discovery.h"
#include "app_srv_ancs.h"
#include "watch_config.h"


#define LOG_LEVEL_OFF 							0
#define LOG_LEVEL_ERROR 						1
#define LOG_LEVEL_WARNING 						2
#define LOG_LEVEL_INFO 							3
#define LOG_LEVEL_DEBUG 						4
#define NRF_LOG_MODULE_NAME app_srv_nrf52
#define NRF_LOG_LEVEL       					LOG_LEVEL_DEBUG
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
NRF_LOG_MODULE_REGISTER();


//#define DELAY_MS(X)                 nrf_delay_ms(X)
#define DELAY_MS(X)                 vTaskDelay(X)
//#define DELAY_MS(X)                   do{TickType_t xLastWakeTime;xLastWakeTime = xTaskGetTickCount ();vTaskDelayUntil( &xLastWakeTime, X / ( ( TickType_t ) 1000 / 3 ) );}while(0)



BLE_NRF52_DEF(m_nrf52);

extern ble_config_params_t     *g_ble_config;
extern uint16_t g_conn_handle_peripheral;
extern  nrf_ble_gatt_t m_gatt;


#define RESET_TYPE_JUMP_BOOTLOADER                  1
#define RESET_TYPE_RESET                            2
#define RESET_TYPE_BOND_REQUEST                     3
#define RESET_TYPE_MTU_REQUEST                      4

static uint8_t m_reset_type = 0;
extern uint16_t g_conn_handle_peripheral;
extern uint8_t g_ble_passkey[BLE_GAP_PASSKEY_LEN];
extern ble_gap_addr_t  g_peer_addr;
extern ble_advertising_t       m_advertising;
extern bool g_conn_params_update_from_mobile_app;//This flag will be reset to zero when connect and disconnect
extern nrf_ble_gatt_t m_gatt;
extern _drv_ble_flash_ancs_switch g_ancs_switch;
extern _android_bond g_android_bond_table[ANDROID_BOND_MAX];


typedef struct __attribute__ ((packed)) {

	uint8_t command;
	union __attribute__ ((packed)){
		uint32_t error;
		uint32_t bootloader_version;
	}_param;
	uint32_t application_version;
	uint32_t softdevice_version;
	uint32_t pcb_version;
}_app_srv_nrf52_version_code;


typedef struct __attribute__ ((packed))
{
	uint8_t command;
	union __attribute__ ((packed))
	{ 
		uint32_t val;
		struct __attribute__ ((packed))
		{		
			uint32_t call:1;       
			uint32_t wechat:1;       
			uint32_t qq:1;               
			uint32_t sms:1;        
		}bits;
	}u_ancs_switch;	
}_app_srv_nrf52_ancs_switch;
uint8_t g_android_mac[BLE_GAP_ADDR_LEN + 2];	

#define TIMER_INTERVAL            5000 
#define OSTIMER_WAIT_FOR_QUEUE              2 
static TimerHandle_t m_timer; 

bool g_mtu_update_from_mobile_app = false;

static void timeout_handler(TimerHandle_t xTimer);
static void timer_create(void);
static void timer_start(uint32_t timeout_msec);

static void cmd_nrf52_srv_device_name_read(ble_nrf52_evt_t *evt);	
static void cmd_nrf52_srv_adv_param_read(ble_nrf52_evt_t *evt);
static void cmd_nrf52_srv_adv_param_update(ble_nrf52_evt_t *evt);	
static void cmd_nrf52_srv_conn_param_read(ble_nrf52_evt_t *evt);	
static void cmd_nrf52_srv_conn_param_update(ble_nrf52_evt_t *evt);
static void cmd_nrf52_srv_version_code_read(void);
static void cmd_nrf52_srv_mtu_request(void);
//static void cmd_nrf52_srv_jump_bootloader(void);
//static void cmd_nrf52_srv_reset(void);
//static void cmd_nrf52_srv_bond_request(void);
static void cmd_nrf52_srv_bond_delete_request_android(ble_nrf52_evt_t *evt);
static void cmd_nrf52_srv_bond_delete_all_request_android(void);
static void cmd_nrf52_srv_bond_reqeust_android(ble_nrf52_evt_t *evt);
static void cmd_nrf52_srv_passkey_request_android(void);
static void cmd_nrf52_srv_passkey_confrim_android(void);
static void cmd_nrf52_srv_ancs_notify_disable(void);								
static void cmd_nrf52_srv_ancs_notify_enable(void);	


static void cmd_nrf52_srv_debond_ios(void);
static void cmd_nrf52_srv_debond_android(void);

static void cmd_nrf52_srv_ancs_switch_read(void);								
static void cmd_nrf52_srv_ancs_switch_write(ble_nrf52_evt_t *evt);

static void cmd_nrf52_srv_find_phone_response(ble_nrf52_evt_t *evt);

static void cmd_nrf52_srv_bond_write_direct(ble_nrf52_evt_t *evt);
static void cmd_nrf52_srv_find_watch(ble_nrf52_evt_t *evt);
static void cmd_nrf52_srv_get_watch_default_sport_mode(ble_nrf52_evt_t *evt);

#ifdef WATCH_HAS_HYTERA_FUNC
static void cmd_nrf52_srv_hytera_heartrate_collection_start(ble_nrf52_evt_t *evt);
static void cmd_nrf52_srv_hytera_heartrate_collection_stop(ble_nrf52_evt_t *evt);
static void cmd_nrf52_srv_hytera_sport_data_collection_start(ble_nrf52_evt_t *evt);
static void cmd_nrf52_srv_hytera_sport_data_collection_stop(ble_nrf52_evt_t *evt);
static void cmd_nrf52_srv_hytera_sport_data_read(ble_nrf52_evt_t *evt);
static void cmd_nrf52_srv_hytera_heartrate_data_read(ble_nrf52_evt_t *evt);

#endif


static void nrf52_evt_handler (ble_nrf52_evt_t *evt)
{
    switch (evt->type)
    {
			
        case BLE_NRF52_EVT_CONNECTED:{
					NRF_LOG_INFO("BLE_NRF52_EVT_CONNECTED");
					g_mtu_update_from_mobile_app = false;
				}break;			
        case BLE_NRF52_EVT_DISCONNECTED:{
					NRF_LOG_INFO("BLE_NRF52_EVT_DISCONNECTED");
					
				}break;				
			
        case BLE_NRF52_EVT_CONFIG_DATA:{
					NRF_LOG_DEBUG("BLE_NRF52_EVT_CONFIG_DATA");
					NRF_LOG_HEXDUMP_INFO(evt->params.data.p_data, 20);
					switch(evt->params.data.p_data[0])
					{
					  case CMD_NRF52_SRV_DEVICE_NAME_READ:{
							NRF_LOG_INFO("CMD_NRF52_SRV_DEVICE_NAME_READ");
							NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
							cmd_nrf52_srv_device_name_read(evt);
						}break;
					  case CMD_NRF52_SRV_DEVICE_NAME_UPDATE:{
							NRF_LOG_INFO("CMD_NRF52_SRV_DEVICE_NAME_UPDATE");
							NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
							task_ble_conn_handle(TASK_BLE_CONN_DEV_NAME_UPDATE, (uint8_t *)evt, evt->params.data.length);
						}break;					
					  case CMD_NRF52_SRV_ADV_PARAM_READ:{
							NRF_LOG_INFO("CMD_NRF52_SRV_ADV_PARAM_READ");
							NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
							cmd_nrf52_srv_adv_param_read(evt);
						}break;
					  case CMD_NRF52_SRV_ADV_PARAM_UPDATE:{
							NRF_LOG_INFO("CMD_NRF52_SRV_ADV_PARAM_UPDATE");
							NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
							cmd_nrf52_srv_adv_param_update(evt);
						}break;						
					  case CMD_NRF52_SRV_CONN_PARAM_READ:{
							NRF_LOG_INFO("CMD_NRF52_SRV_CONN_PARAM_READ");
							NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
							cmd_nrf52_srv_conn_param_read(evt);
						}break;
					  case CMD_NRF52_SRV_CONN_PARAM_UPDATE:{
							NRF_LOG_INFO("CMD_NRF52_SRV_CONN_PARAM_UPDATE");
							NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
							cmd_nrf52_srv_conn_param_update(evt);
						}break;					
					  case CMD_NRF52_SRV_VERSION_CODE_READ:{
							NRF_LOG_INFO("CMD_NRF52_SRV_VERSION_CODE_READ");
							NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
							cmd_nrf52_srv_version_code_read();
						}break;
					  case CMD_NRF52_SRV_VERSION_CODE_UPDATE:{
							NRF_LOG_INFO("CMD_NRF52_SRV_VERSION_CODE_UPDATE");
							/**
							@Note : This feature is not supported.
							*/
						}break;	
					  case CMD_NRF52_SRV_MTU_REQUEST:{
							NRF_LOG_INFO("CMD_NRF52_SRV_MTU_REQUEST");
							NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
							cmd_nrf52_srv_mtu_request();
						}break;	

					  case 0xf4:{
							NRF_LOG_INFO("test 1");
//							while(1);
							//pm_peers_delete();
//							APP_ERROR_HANDLER(1);
						}break;		
	

						
						default:{
							NRF_LOG_INFO("UNSPPORT COMMAND");
							uint8_t response = 0xFF;
							app_srv_nrf52_config_send((uint8_t *)&response, sizeof(response));
							/**<Unspported command need to response to phone APP>*/
						}break;
					}
				}break;
        case BLE_NRF52_EVT_CONFIG_NOTIFY_ENABLE:{
					NRF_LOG_INFO("BLE_NRF52_EVT_CONFIG_NOTIFY_ENABLE");
					ble_srv_common_char_notify_change(NRF52_SRV_CONFIG_CHAR_NOTIFY, true);
				}break;
        case BLE_NRF52_EVT_CONFIG_NOTIFY_DISABLE:{
					NRF_LOG_INFO("BLE_NRF52_EVT_CONFIG_NOTIFY_DISABLE");
					ble_srv_common_char_notify_change(NRF52_SRV_CONFIG_CHAR_NOTIFY, false);
				}break;				
	     case BLE_NRF52_EVT_CONTROL_DATA:{
					NRF_LOG_DEBUG("BLE_NRF52_EVT_CONTROL_DATA");
					NRF_LOG_INFO("evt->params.data.p_data[0] --> %d",evt->params.data.p_data[0]);

					switch(evt->params.data.p_data[0])
					{
					#if 0
					  case CMD_NRF52_SRV_JUMP_BOOTLOADER:{
							NRF_LOG_INFO("CMD_NRF52_SRV_JUMP_BOOTLOADER");
							NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
							cmd_nrf52_srv_jump_bootloader();
						}break;
					  case CMD_NRF52_SRV_RESET:{
							NRF_LOG_INFO("CMD_NRF52_SRV_RESET");
							NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
							cmd_nrf52_srv_reset();
						}break;	
					#endif
					  case CMD_NRF52_SRV_BOND_REQUEST:{
							NRF_LOG_INFO("CMD_NRF52_SRV_BOND_REQUEST");
							NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
							cmd_nrf52_srv_bond_request();
						}break;
					  case CMD_NRF52_SRV_BOND_DELETE_REQUEST_ANDROID:{
							NRF_LOG_INFO("CMD_NRF52_SRV_BOND_DELETE_REQUEST");
							NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
							cmd_nrf52_srv_bond_delete_request_android(evt);
						}break;				
					  case CMD_NRF52_SRV_BOND_DELETE_ALL_REQUEST_ANDROID:{
							NRF_LOG_INFO("CMD_NRF52_SRV_BOND_DELETE_ALL_REQUEST");
							NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
							cmd_nrf52_srv_bond_delete_all_request_android();
						}break;				
					  case CMD_NRF52_SRV_BOND_REQUEST_ANDROID:{
							NRF_LOG_INFO("CMD_NRF52_SRV_BOND_REQUEST_ANDROID");
							NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
							cmd_nrf52_srv_bond_reqeust_android(evt);
						}break;	
					  case CMD_NRF52_SRV_PASSKEY_REQUEST_ANDROID:{
							NRF_LOG_INFO("CMD_NRF52_SRV_PASSKEY_REQUEST");
							NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
							cmd_nrf52_srv_passkey_request_android();
						}break;	
					  case CMD_NRF52_SRV_PASSKEY_CONFIRM_ANDROID:{
							NRF_LOG_INFO("CMD_NRF52_SRV_PASSKEY_CONFIRM");		
							if(evt->params.data.p_data[1] != CMD_NRF52_SRV_SUCCUSS)
							{
								NRF_LOG_DEBUG("passkey failed");
								DELAY_MS(1000);
								/**<Do not check return error, because it will cause hang up in softdevice>*/
								sd_ble_gap_disconnect(g_conn_handle_peripheral, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
							}
							else
							{
								cmd_nrf52_srv_passkey_confrim_android();
							}
						}break;						
					  case CMD_NRF52_SRV_DISCONNECT:{
							NRF_LOG_INFO("CMD_NRF52_SRV_DISCONNECT");
							NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);

						}break;
					  case CMD_NRF52_SRV_ANCS_SWITCH_READ:{
							NRF_LOG_INFO("CMD_NRF52_SRV_ANCS_SWITCH_READ");
							NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
							cmd_nrf52_srv_ancs_switch_read();
						}break;		
					  case CMD_NRF52_SRV_ANCS_SWITCH_WRITE:{
							NRF_LOG_INFO("CMD_NRF52_SRV_ANCS_SWITCH_WRITE");
							NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
							cmd_nrf52_srv_ancs_switch_write(evt);
						}break;								
					  case CMD_NRF52_SRV_ANCS_NOTIFY_ENABLE:{
							NRF_LOG_INFO("CMD_NRF52_SRV_ANCS_NOTIFY_ENABLE");
							NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
							cmd_nrf52_srv_ancs_notify_enable();
						}break;		
					  case CMD_NRF52_SRV_ANCS_NOTIFY_DISABLE:{
							NRF_LOG_INFO("CMD_NRF52_SRV_ANCS_NOTIFY_DISABLE");
							NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
							cmd_nrf52_srv_ancs_notify_disable();
						}break;								
					  case CMD_NRF52_SRV_DEBOND_IOS:{
							NRF_LOG_INFO("CMD_NRF52_SRV_DEBOND_IOS");
							NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
							cmd_nrf52_srv_debond_ios();
						}break;		
					  case CMD_NRF52_SRV_DEBOND_ANDROID:{
							NRF_LOG_INFO("CMD_NRF52_SRV_DEBOND_ANDROID");
							NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
							cmd_nrf52_srv_debond_android();
						}break;								
					  case CMD_NRF52_SRV_FIND_PHONE_RESPONSE:{
							NRF_LOG_INFO("CMD_NRF52_SRV_FIND_PHONE_RESPONSE");
							NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
							cmd_nrf52_srv_find_phone_response(evt);
						}break;								
					  case CMD_NRF52_SRV_BOND_WRITE_DIRECT:{
							NRF_LOG_INFO("CMD_NRF52_SRV_BOND_WRITE_DIRECT");
							NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
							cmd_nrf52_srv_bond_write_direct(evt);
						}break;	
					  case CMD_NRF52_SRV_FIND_WATCH:{
							NRF_LOG_INFO("CMD_NRF52_SRV_FIND_WATCH");
							NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
							cmd_nrf52_srv_find_watch(evt);
						}break;	
						case CMD_NRF52_SRV_GET_WATCH_DEFAULT_SPORT_MODE:{
							NRF_LOG_INFO("CMD_NRF52_SRV_GET_WATCH_DEFAULT_SPORT_MODE");
							NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
							cmd_nrf52_srv_get_watch_default_sport_mode(evt);						
						}break;

					#ifdef WATCH_HAS_HYTERA_FUNC
						case CMD_NRF52_SRV_HYTERA_HEARTRATE_COLLECTION_START:{
							NRF_LOG_INFO("CMD_NRF52_SRV_HYTERA_HEARTRATE_COLLECTION_START");
							cmd_nrf52_srv_hytera_heartrate_collection_start(evt);						
						}break;
						case CMD_NRF52_SRV_HYTERA_HEARTRATE_COLLECTION_STOP:{
							NRF_LOG_INFO("CMD_NRF52_SRV_HYTERA_HEARTRATE_COLLECTION_STOP");
							cmd_nrf52_srv_hytera_heartrate_collection_stop(evt);						
						}break;
						case CMD_NRF52_SRV_HYTERA_SPORT_DATA_COLLECTION_START:{
							NRF_LOG_INFO("CMD_NRF52_SRV_HYTERA_SPORT_DATA_COLLECTION_START");
							cmd_nrf52_srv_hytera_sport_data_collection_start(evt);						
						}break;
						case CMD_NRF52_SRV_HYTERA_SPORT_DATA_COLLECTION_STOP:{
							NRF_LOG_INFO("CMD_NRF52_SRV_HYTERA_SPORT_DATA_COLLECTION_STOP");
							cmd_nrf52_srv_hytera_sport_data_collection_stop(evt);						
						}break;
						case CMD_NRF52_SRV_HYTERA_SPORT_DATA_READ:{
							NRF_LOG_INFO("CMD_NRF52_SRV_HYTERA_SPORT_DATA_READ");
							cmd_nrf52_srv_hytera_sport_data_read(evt);						
						}break;
						case CMD_NRF52_SRV_HYTERA_HEARTRATE_DATA_READ:{
							NRF_LOG_INFO("CMD_NRF52_SRV_HYTERA_HEARTRATE_DATA_READ");
							cmd_nrf52_srv_hytera_heartrate_data_read(evt);						
						}break;
					#endif
						default:{
							/**<Unspported command need to response to phone APP>*/
							NRF_LOG_INFO("UNSPPORT COMMAND");
							uint8_t response = 0xFF;
							app_srv_nrf52_control_send((uint8_t *)&response, sizeof(response));
						}break;
					}				 
				 
				 
				 
			}break;			
        case BLE_NRF52_EVT_CONTROL_NOTIFY_ENABLE:{
					NRF_LOG_INFO("BLE_NRF52_EVT_CONTROL_NOTIFY_ENABLE");
					ble_srv_common_char_notify_change(NRF52_SRV_CONTROL_CHAR_NOTIFY, true);
			}break;
        case BLE_NRF52_EVT_CONTROL_NOTIFY_DISABLE:{
					NRF_LOG_INFO("BLE_NRF52_EVT_CONTROL_NOTIFY_DISABLE");
					ble_srv_common_char_notify_change(NRF52_SRV_CONTROL_CHAR_NOTIFY, false);
				}break;			
			
				
        case BLE_NRF52_EVT_TX_RDY:{
					//NRF_LOG_DEBUG("BLE_NRF52_EVT_TX_RDY");
				}break;
        case BLE_NRF52_EVT_HVC:{
					NRF_LOG_DEBUG("BLE_NRF52_EVT_HVC");

				}break;
						
				default:
					break;
				
    }


}




uint32_t app_srv_nrf52_init(void)
{
    ble_nrf52_init_t params;
    uint32_t err_code;

    params.evt_handler = nrf52_evt_handler;
    err_code = ble_nrf52_init(&m_nrf52, &params);
    APP_ERROR_CHECK(err_code);

	
		timer_create();
	
    return NRF_SUCCESS;
}



static void timeout_handler(TimerHandle_t xTimer)
{
	uint32_t err_code;
    UNUSED_PARAMETER(xTimer);
    NRF_LOG_DEBUG("timeout_handler");
//	if (pdPASS != xTimerStop(m_timer, OSTIMER_WAIT_FOR_QUEUE))
//	{
//			APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
//	}	


	
	if(m_reset_type == RESET_TYPE_JUMP_BOOTLOADER)
	{
		err_code = sd_power_gpregret_clr(0, 0xffffffff);
		APP_ERROR_CHECK(err_code);
		err_code = sd_power_gpregret_set(0, CMD_GPREGRET_BOOTLOADER_BLE_DFU);
		APP_ERROR_CHECK(err_code);								
		nrf_pwr_mgmt_shutdown(NRF_PWR_MGMT_SHUTDOWN_GOTO_DFU);	
	}
	else if(m_reset_type == RESET_TYPE_RESET)
	{
		err_code = sd_power_gpregret_clr(0, 0xffffffff);
		APP_ERROR_CHECK(err_code);
		err_code = sd_power_gpregret_set(0, CMD_GPREGRET_BOOTLOADER_BLE_RESET);
		APP_ERROR_CHECK(err_code);								
		nrf_pwr_mgmt_shutdown(NRF_PWR_MGMT_SHUTDOWN_RESET);		
	}
	else if(m_reset_type == RESET_TYPE_BOND_REQUEST)
	{
		pm_conn_sec_status_t conn_sec_status;
		err_code = pm_conn_sec_status_get(g_conn_handle_peripheral, &conn_sec_status);
		NRF_LOG_INFO("conn_sec_status.bonded --> %d",conn_sec_status.bonded);
		NRF_LOG_INFO("conn_sec_status.encrypted --> %d",conn_sec_status.encrypted);
		NRF_LOG_INFO("conn_sec_status.connected --> %d",conn_sec_status.connected);
		NRF_LOG_INFO("conn_sec_status.mitm_protected --> %d",conn_sec_status.mitm_protected);
		
		if(conn_sec_status.connected)
		{
		
			if(!conn_sec_status.encrypted || !conn_sec_status.mitm_protected)
//			if(!conn_sec_status.encrypted)
			{
				err_code = pm_conn_secure(g_conn_handle_peripheral,false);
				NRF_LOG_INFO("pm_conn_secure err_code --> 0x%08X",err_code);
				if(err_code == NRF_ERROR_INVALID_STATE)
				{
					NRF_LOG_INFO("pm_conn_secure --> NRF_ERROR_INVALID_STATE");
				}
				else{
					APP_ERROR_CHECK(err_code);	
				}
					
			}
			else
			{
//				uint8_t response[2] = {CMD_NRF52_SRV_BOND_REQUEST_RESPONSE,CMD_NRF52_SRV_SUCCUSS};
//				app_srv_nrf52_control_send(response, sizeof(response));			
				extern ble_db_discovery_t m_db_disc[];
				// Discover peer's services.
				err_code  = ble_db_discovery_start(&m_db_disc[DISCONVER_ARRAY_ANCS], g_conn_handle_peripheral);
				if (err_code != NRF_ERROR_BUSY)
				{
						APP_ERROR_CHECK(err_code);
				}			
			}		
		}

	}if(m_reset_type == RESET_TYPE_MTU_REQUEST)
	{		
		g_mtu_update_from_mobile_app = true;
		uint32_t err_code = sd_ble_gattc_exchange_mtu_request(g_conn_handle_peripheral, 247);
		APP_ERROR_CHECK(err_code);
        #if !defined WATCH_HAS_HYTERA_FUNC && !defined BLE_ALWAYS_DISCONNECT_DEBUG
		ble_gap_data_length_params_t const dlp =
		{
				.max_rx_octets  = 247,
				.max_tx_octets  = 247,
				.max_rx_time_us = BLE_GAP_DATA_LENGTH_AUTO,
				.max_tx_time_us = BLE_GAP_DATA_LENGTH_AUTO,
		};
		err_code = sd_ble_gap_data_length_update(g_conn_handle_peripheral, &dlp, NULL);		
		APP_ERROR_CHECK(err_code);	
        #endif
	
	}
	

	
}
static void timer_create(void)
{
	m_timer = xTimerCreate("RST",
																 TIMER_INTERVAL,
																 pdFALSE,
																 NULL,
																 timeout_handler);
	if (NULL == m_timer)
	{
			APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
	}	
}
static void timer_start(uint32_t timeout_msec)
{
	if( xTimerChangePeriod( m_timer, timeout_msec , OSTIMER_WAIT_FOR_QUEUE ) == pdPASS )
	{
		NRF_LOG_DEBUG("The command was successfully sent.");
	}
	else
	{
		NRF_LOG_DEBUG("The command could not be sent, even after waiting for 100 ticks");
		NRF_LOG_DEBUG("to pass.  Take appropriate action here.");
	}		
	
}



void app_srv_nrf52_config_send(uint8_t *p_data, uint32_t length)
{
	NRF_LOG_DEBUG("app_srv_nrf52_config_send");
	
	uint32_t bytes_sent = 0;
	uint16_t packet_length = 0;	
	uint32_t err_code;

	while(bytes_sent < length)
	{
		packet_length = (length - bytes_sent) > BLE_NRF52_MAX_DATA_LEN ? BLE_NRF52_MAX_DATA_LEN : length - bytes_sent;
		err_code = ble_srv_nrf52_config_string_send(&m_nrf52, p_data + bytes_sent, &packet_length);	
		
		/**
		 * @retval ::NRF_ERROR_BUSY For @ref BLE_GATT_HVX_INDICATION Procedure already in progress. Wait for a @ref BLE_GATTS_EVT_HVC event and retry.		
		 * @retval ::NRF_ERROR_RESOURCES Too many notifications queued.
		 *                               Wait for a @ref BLE_GATTS_EVT_HVN_TX_COMPLETE event and retry.		
		*/
		if(err_code == NRF_ERROR_BUSY) 
		{
			//Wait for a @ref BLE_GATTS_EVT_HVC event and retry
		}
		else if(err_code == NRF_ERROR_RESOURCES) 
		{
			//Wait for a @ref BLE_GATTS_EVT_HVN_TX_COMPLETE event and retry
		}
		else if(err_code == NRF_SUCCESS) 
		{
			bytes_sent += packet_length;
		}
		else //if ((err_code == NRF_ERROR_INVALID_STATE)||(err_code == NRF_ERROR_INVALID_PARAM))
		{
			/**<Connection handle invalid or Char notification is not enabled>*/
			break;
		}		
		
	}		
	NRF_LOG_DEBUG("app_srv_nrf52_config_send finish");
}



uint32_t app_srv_nrf52_control_send(uint8_t *p_data, uint32_t length)
{
	NRF_LOG_DEBUG("app_srv_nrf52_control_send");
	uint32_t bytes_sent = 0;
	uint16_t packet_length = 0;	
	uint32_t err_code;

	while(bytes_sent < length)
	{
		packet_length = (length - bytes_sent) > BLE_NRF52_MAX_DATA_LEN ? BLE_NRF52_MAX_DATA_LEN : length - bytes_sent;
		err_code = ble_srv_nrf52_control_string_send(&m_nrf52, p_data + bytes_sent, &packet_length);	
		
		/**
		 * @retval ::NRF_ERROR_BUSY For @ref BLE_GATT_HVX_INDICATION Procedure already in progress. Wait for a @ref BLE_GATTS_EVT_HVC event and retry.		
		 * @retval ::NRF_ERROR_RESOURCES Too many notifications queued.
		 *                               Wait for a @ref BLE_GATTS_EVT_HVN_TX_COMPLETE event and retry.		
		*/
		if(err_code == NRF_ERROR_BUSY) 
		{
			//Wait for a @ref BLE_GATTS_EVT_HVC event and retry
		}
		else if(err_code == NRF_ERROR_RESOURCES) 
		{
			//Wait for a @ref BLE_GATTS_EVT_HVN_TX_COMPLETE event and retry
		}
		else if(err_code == NRF_SUCCESS) 
		{
			bytes_sent += packet_length;
		}
		else //if ((err_code == NRF_ERROR_INVALID_STATE)||(err_code == NRF_ERROR_INVALID_PARAM))
		{
			/**<Connection handle invalid or Char notification is not enabled>*/
			break;
		}		
	}		
	NRF_LOG_DEBUG("app_srv_nrf52_control_send finish");
	return err_code;
}


						
static void cmd_nrf52_srv_device_name_read(ble_nrf52_evt_t *evt)
{
	_app_srv_nrf52_dev_name response;
	response.command = CMD_NRF52_SRV_DEVICE_NAME_READ_RESPONSE;
	memcpy(response.param.dev.name,g_ble_config->dev_name.name,g_ble_config->dev_name.len);
	response.param.dev.len = g_ble_config->dev_name.len;
	app_srv_nrf52_config_send((uint8_t *)&response, sizeof(response));
}	

static void cmd_nrf52_srv_adv_param_read(ble_nrf52_evt_t *evt)
{
	_app_srv_nrf52_adv_param response;
	response.command = CMD_NRF52_SRV_ADV_PARAM_READ_RESPONSE;
	memcpy((uint8_t *)&response.param.adv,(uint8_t *)&g_ble_config->adv_params,sizeof(ble_config_adv_params_t));	
	app_srv_nrf52_config_send((uint8_t *)&response, sizeof(response));	
}	
static void cmd_nrf52_srv_adv_param_update(ble_nrf52_evt_t *evt)
{
	_app_srv_nrf52_adv_param response;
	_app_srv_nrf52_adv_param *new_adv_param = (_app_srv_nrf52_adv_param *)evt->params.data.p_data;
	response.command = CMD_NRF52_SRV_ADV_PARAM_UPDATE_RESPONSE;
	if(new_adv_param->param.adv.enable_fast > 1 || new_adv_param->param.adv.enable_slow > 1 ||
			new_adv_param->param.adv.interval_fast < 0x0020 || new_adv_param->param.adv.interval_slow > 0x4000 )
	{
			response.param.error = CMD_NRF52_SRV_ERROR_OUT_OF_BOUND;
	}
	else if(new_adv_param->param.adv.interval_fast > new_adv_param->param.adv.interval_slow)
	{
			response.param.error = CMD_NRF52_SRV_ERROR_INVALID_PARAMS;
	}
	else{	
			memcpy((uint8_t *)&g_ble_config->adv_params,(uint8_t *)&new_adv_param->param.adv,sizeof(ble_config_adv_params_t));
			
			//Settings will take effect after disconnection.
			m_advertising.adv_modes_config.ble_adv_fast_enabled = new_adv_param->param.adv.enable_fast;
			m_advertising.adv_modes_config.ble_adv_fast_interval = new_adv_param->param.adv.interval_fast;
			m_advertising.adv_modes_config.ble_adv_fast_timeout = new_adv_param->param.adv.timeout_fast;
			m_advertising.adv_modes_config.ble_adv_slow_enabled = new_adv_param->param.adv.enable_slow;
			m_advertising.adv_modes_config.ble_adv_slow_interval = new_adv_param->param.adv.interval_slow;
			m_advertising.adv_modes_config.ble_adv_slow_timeout = new_adv_param->param.adv.timeout_slow;		
			response.param.error = CMD_NRF52_SRV_SUCCUSS;
	}
	app_srv_nrf52_config_send((uint8_t *)&response, 2);
}	

static void cmd_nrf52_srv_conn_param_read(ble_nrf52_evt_t *evt)
{
	_app_srv_nrf52_conn_param response;
	response.command = CMD_NRF52_SRV_CONN_PARAM_READ_RESPONSE;
	memcpy((uint8_t*)&response.param.conn_param, &g_ble_config->conn_params,sizeof(ble_config_conn_params_t));
	app_srv_nrf52_config_send((uint8_t *)&response, sizeof(response));
}	

static void cmd_nrf52_srv_conn_param_update(ble_nrf52_evt_t *evt)
{
	_app_srv_nrf52_conn_param response;
	uint16_t err_code;
	_app_srv_nrf52_conn_param *conn_param = (_app_srv_nrf52_conn_param *)evt->params.data.p_data;
	

	response.command = CMD_NRF52_SRV_CONN_PARAM_UPDATE_RESPONSE;
	
	uint16_t watch_min_interval = g_ble_config->conn_params.min_conn_int;
	uint16_t watch_max_interval = g_ble_config->conn_params.max_conn_int;
	uint16_t app_min_interval = conn_param->param.conn_param.min_conn_int;
	uint16_t app_max_interval = conn_param->param.conn_param.max_conn_int;
	
	NRF_LOG_DEBUG("watch_min_interval --> %d",watch_min_interval);
	NRF_LOG_DEBUG("watch_max_interval --> %d",watch_max_interval);
	NRF_LOG_DEBUG("app_min_interval --> %d",app_min_interval);
	NRF_LOG_DEBUG("app_max_interval --> %d",app_max_interval);
	
	
	
	
	
	
	if((app_min_interval>(watch_min_interval/2)) && 
		 (app_min_interval<(watch_min_interval*2)) && 
		 (app_max_interval>(watch_max_interval/2)) && 
		 (app_max_interval<(watch_max_interval*2))
	  )
	{
				_app_srv_nrf52_conn_param response;
				response.command = CMD_NRF52_SRV_CONN_PARAM_UPDATE_RESPONSE;
				response.param.error = CMD_NRF52_SRV_SUCCUSS;
				app_srv_nrf52_config_send((uint8_t *)&response, 2);			
	}
	else{
//		if(evt->params.data.length != (sizeof(response.command)+sizeof(response.param.conn_param)))
//		{
//			response.param.error = CMD_NRF52_SRV_ERROR_INVALID_PARAMS;
//		}
//		else if (conn_param->param.conn_param.max_conn_int <= conn_param->param.conn_param.min_conn_int ||
//				conn_param->param.conn_param.slave_latency > 4 ||
//				conn_param->param.conn_param.sup_timeout > 400)
//		{
//			NRF_LOG_DEBUG("CMD_NRF52_SRV_ERROR_INVALID_PARAMS");
//			response.param.error = CMD_NRF52_SRV_ERROR_CONN_PARAM_NOT_ACCEPT;
//		}
//		else
		{
			ble_gap_conn_params_t p_new_params;
			p_new_params.max_conn_interval = conn_param->param.conn_param.max_conn_int;
			p_new_params.min_conn_interval = conn_param->param.conn_param.min_conn_int;
			p_new_params.slave_latency = conn_param->param.conn_param.slave_latency;
			p_new_params.conn_sup_timeout = conn_param->param.conn_param.sup_timeout;

			err_code = ble_conn_params_stop();
			APP_ERROR_CHECK(err_code);
			err_code = ble_conn_params_change_conn_params(g_conn_handle_peripheral, &p_new_params);
			
			if(err_code != NRF_SUCCESS)
			{
				NRF_LOG_ERROR("ble_conn_params_change_conn_params err_code --> 0x%08X",err_code);
				response.param.error = err_code | CMD_NRF52_SRV_ERROR_INTERVAL_MASK;
			}
			else{
				response.param.error = CMD_NRF52_SRV_SUCCUSS;
				g_conn_params_update_from_mobile_app = true;//Wait for on_conn_params_evt is called
			}		
		
		}
		if(response.param.error != CMD_NRF52_SRV_SUCCUSS)
		{
			app_srv_nrf52_config_send((uint8_t *)&response, 2);
		}	
		
	}
	
	

	
}
static void cmd_nrf52_srv_version_code_read(void)						
{
	_dev_apollo2_ble_info ble_info;
	dev_apollo2_ble_info_get(&ble_info);			
	_app_srv_nrf52_version_code version_code = 
	{
		 .command = CMD_NRF52_SRV_VERSION_CODE_READ_RESPONSE,
		 ._param = {ble_info.bootloader_version},
		 .application_version = ble_info.application_version,
		 .softdevice_version = ble_info.softdevice_version,
		 .pcb_version = ble_info.pcb_version,	
	};
	app_srv_nrf52_config_send((uint8_t *)&version_code, sizeof(version_code));
}			

static void cmd_nrf52_srv_mtu_request(void)
{
//	uint8_t response[3] = {CMD_NRF52_SRV_MTU_REQUEST_RESPONSE};
//	uint16_t mtu = nrf_ble_gatt_eff_mtu_get(&m_gatt, g_conn_handle_peripheral) - 3;
//	response[1] = mtu>>8;
//	response[2] = mtu;
//	app_srv_nrf52_config_send((uint8_t *)response, sizeof(response));
	
	if(g_mtu_update_from_mobile_app == false)
	{
		m_reset_type = RESET_TYPE_MTU_REQUEST;							
		timer_start(1000);	
	}else{
	
		uint8_t response[3] = {CMD_NRF52_SRV_MTU_REQUEST_RESPONSE};
		uint16_t mtu = nrf_ble_gatt_eff_mtu_get(&m_gatt, g_conn_handle_peripheral) - 3;
		response[1] = mtu>>8;
		response[2] = mtu;
		app_srv_nrf52_config_send((uint8_t *)response, sizeof(response));		
	
	}
	
	
	
	
}	
#if 0
static void cmd_nrf52_srv_jump_bootloader(void)
{
	uint8_t response = CMD_NRF52_SRV_JUMP_BOOTLOADER_RESPONSE;
	app_srv_nrf52_control_send(&response, sizeof(response));
	m_reset_type = RESET_TYPE_JUMP_BOOTLOADER;							
	timer_start(5000);
}	

static void cmd_nrf52_srv_reset(void)
{
	uint8_t response = CMD_NRF52_SRV_RESET_RESPONSE;
	app_srv_nrf52_control_send(&response, sizeof(response));
	m_reset_type = RESET_TYPE_RESET;
	timer_start(5000);
}	
#endif
void cmd_nrf52_srv_bond_request(void)
{
	m_reset_type = RESET_TYPE_BOND_REQUEST;
	timer_start(3000);
}	

static void cmd_nrf52_srv_bond_delete_request_android(ble_nrf52_evt_t *evt)
{
	_bond_request_android *bond_request_android = (_bond_request_android *)evt->params.data.p_data;
	NRF_LOG_HEXDUMP_DEBUG(bond_request_android->mac,BLE_GAP_ADDR_LEN);	
//	memcpy(g_android_mac,bond_request_android->mac,BLE_GAP_ADDR_LEN);	
	uint32_t error;
//	uint8_t i=0;
//	for(i=0;i<ANDROID_BOND_MAX;i++)
//	{
//		if(!memcmp(g_android_bond_table[i].mac,g_android_mac,BLE_GAP_ADDR_LEN))
//		{
//			break;
//		}			
//	}		
//	NRF_LOG_DEBUG("mac address index --> %d",i);
	
	uint8_t i=0;
	for(i=0;i<ANDROID_BOND_MAX;i++)
	{
		memset(g_android_bond_table[i].mac,0xFF,BLE_GAP_ADDR_LEN);	
	}		
	memset(g_android_mac,0xFF,sizeof(g_android_mac));	
	error = drv_ble_flash_android_mac_delete_all();
	APP_ERROR_CHECK(error);	
	
	
	uint8_t response[2] = {CMD_NRF52_SRV_BOND_DELETE_REQUEST_ANDROID_RESPONSE,CMD_NRF52_SRV_SUCCUSS};
	app_srv_nrf52_control_send(response, sizeof(response));	
}	

static void cmd_nrf52_srv_bond_delete_all_request_android(void)
{
	uint32_t error;
	uint8_t i=0;
	for(i=0;i<ANDROID_BOND_MAX;i++)
	{
		memset(g_android_bond_table[i].mac,0xFF,BLE_GAP_ADDR_LEN);	
	}		
	memset(g_android_mac,0xFF,sizeof(g_android_mac));		
	error = drv_ble_flash_android_mac_delete_all();
	APP_ERROR_CHECK(error);
	uint8_t response[2] = {CMD_NRF52_SRV_BOND_DELETE_ALL_REQUEST_ANDROID_RESPONSE,CMD_NRF52_SRV_SUCCUSS};
	app_srv_nrf52_control_send(response, sizeof(response));	
}	



static void cmd_nrf52_srv_bond_reqeust_android(ble_nrf52_evt_t *evt)
{
	_bond_request_android *bond_request_android = (_bond_request_android *)evt->params.data.p_data;
	NRF_LOG_HEXDUMP_DEBUG(bond_request_android->mac,BLE_GAP_ADDR_LEN);	
	memcpy(g_android_mac,bond_request_android->mac,BLE_GAP_ADDR_LEN);
	uint8_t response[2] = {CMD_NRF52_SRV_BOND_REQUEST_ANDROID_RESPONSE,CMD_NRF52_SRV_ERROR_ANDROID_MAC_NOT_EXIT};
	uint8_t empty_space[BLE_GAP_ADDR_LEN] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
	uint8_t i=0;
	for(i=0;i<ANDROID_BOND_MAX;i++)
	{
		NRF_LOG_HEXDUMP_INFO(g_android_bond_table[i].mac,BLE_GAP_ADDR_LEN);	
		if(!memcmp(g_android_bond_table[i].mac,empty_space,BLE_GAP_ADDR_LEN))
		{
			break;
		}
	}	
	
	if(i == ANDROID_BOND_MAX)
	{
		response[1] = CMD_NRF52_SRV_ERROR_ANDROID_MAC_TABLE_FULL;
	}
	else
	{
		for(i=0;i<ANDROID_BOND_MAX;i++)
		{
			if(!memcmp(g_android_bond_table[i].mac,g_android_mac,BLE_GAP_ADDR_LEN))
			{
				response[1] = CMD_NRF52_SRV_SUCCUSS;
				break;
			}			
		}	
	}


	app_srv_nrf52_control_send(response, sizeof(response));	
	
//	if(response[1] == CMD_NRF52_SRV_SUCCUSS)
//	{
//		task_ble_conn_handle(TASK_BLE_CONN_EVT_BOND_ANDROID, NULL, 0);
//	}
	
	
}	


static void cmd_nrf52_srv_bond_write_direct(ble_nrf52_evt_t *evt)
{
	_bond_request_android *bond_request_android = (_bond_request_android *)evt->params.data.p_data;
	memcpy(g_android_mac,bond_request_android->mac,BLE_GAP_ADDR_LEN);
	uint32_t error = NRF_SUCCESS;
	error = drv_ble_flash_android_mac_write(0, g_android_mac);
//	APP_ERROR_CHECK(error);	
	uint8_t response[2] = {CMD_NRF52_SRV_BOND_WRITE_DIRECT_RESPONSE,0};
	response[1] = error;
	memcpy(g_android_bond_table[0].mac,g_android_mac,BLE_GAP_ADDR_LEN);
	app_srv_nrf52_control_send(response, sizeof(response));		
}


static void cmd_nrf52_srv_passkey_request_android(void)							
{
	/**<Only android APP will send this request>*/
	/**<Calculate a 6 digits random number and send that to APOLLO2 and back to android>*/
	uint32_t error = NRF_SUCCESS;
	uint8_t p_pool_capacity,p_bytes_available;
	uint8_t p_buff[6];
	error = sd_rand_application_pool_capacity_get(&p_pool_capacity);
	APP_ERROR_CHECK(error);
	NRF_LOG_DEBUG("p_pool_capacity --> %d",p_pool_capacity);
	error = sd_rand_application_bytes_available_get(&p_bytes_available);
	APP_ERROR_CHECK(error);
	NRF_LOG_DEBUG("p_bytes_available --> %d",p_bytes_available);
	error = sd_rand_application_vector_get(p_buff, sizeof(p_buff));
	APP_ERROR_CHECK(error);
	NRF_LOG_HEXDUMP_INFO(p_buff, sizeof(p_buff));
	for(uint8_t i=0;i<sizeof(p_buff);i++)
	{
		p_buff[i] |= '0';
		p_buff[i] &= '9';
	}
	NRF_LOG_HEXDUMP_INFO(p_buff, sizeof(p_buff));
	memcpy(g_ble_passkey,p_buff,BLE_GAP_PASSKEY_LEN);
	uint8_t response[7] = {CMD_NRF52_SRV_PASSKEY_REQUEST_ANDROID_RESPONSE};
	memcpy(response+1,p_buff,BLE_GAP_PASSKEY_LEN);
	app_srv_nrf52_control_send(response, sizeof(response));							
	task_ble_conn_handle(TASK_BLE_CONN_EVT_PASSKEY, g_ble_passkey, BLE_GAP_PASSKEY_LEN);
}

static void cmd_nrf52_srv_passkey_confrim_android(void)							
{
	uint32_t error = NRF_SUCCESS;
	uint8_t empty_space[BLE_GAP_ADDR_LEN] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
	uint8_t i=0;
	for(i=0;i<ANDROID_BOND_MAX;i++)
	{
		if(!memcmp(g_android_bond_table[i].mac,empty_space,BLE_GAP_ADDR_LEN))
		{
			break;
		}
	}
	NRF_LOG_DEBUG("bond table index --> %d",i);
	error = drv_ble_flash_android_mac_write(i, g_android_mac);
//	APP_ERROR_CHECK(error);	
	uint8_t response[2] = {CMD_NRF52_SRV_PASSKEY_CONFIRM_ANDROID_RESPONSE,0};
	response[1] = error;
	memcpy(g_android_bond_table[i].mac,g_android_mac,BLE_GAP_ADDR_LEN);
	app_srv_nrf52_control_send(response, sizeof(response));									
	/**<Inform apollo2 that ble is bond>*/
//	task_ble_conn_handle(TASK_BLE_CONN_EVT_BOND_ANDROID, NULL, 0);
}


static void cmd_nrf52_srv_ancs_notify_enable(void)
{
	app_srv_ancs_notify_enable();
}	
static void cmd_nrf52_srv_ancs_notify_disable(void)
{
	app_srv_ancs_notify_disable();
}

static void cmd_nrf52_srv_debond_ios(void)	
{


}
static void cmd_nrf52_srv_debond_android(void)	
{

}	


static void cmd_nrf52_srv_ancs_switch_read(void)
{
#ifndef COD 
	_app_srv_nrf52_ancs_switch app_srv_nrf52_ancs_switch;
	app_srv_nrf52_ancs_switch.command = CMD_NRF52_SRV_ANCS_SWITCH_READ_RESPONSE;
	app_srv_nrf52_ancs_switch.u_ancs_switch.val = 0x00000000;	
	if(g_ancs_switch.call == ANCS_MESSAGE_SWITCH_ON)
	{
		app_srv_nrf52_ancs_switch.u_ancs_switch.bits.call = 1;
	}
	if(g_ancs_switch.wechat == ANCS_MESSAGE_SWITCH_ON)
	{
		app_srv_nrf52_ancs_switch.u_ancs_switch.bits.wechat = 1;
	}
	if(g_ancs_switch.qq == ANCS_MESSAGE_SWITCH_ON)
	{
		app_srv_nrf52_ancs_switch.u_ancs_switch.bits.qq = 1;
	}
	if(g_ancs_switch.sms == ANCS_MESSAGE_SWITCH_ON)
	{
		app_srv_nrf52_ancs_switch.u_ancs_switch.bits.sms = 1;
	}		
	app_srv_nrf52_control_send((uint8_t *)&app_srv_nrf52_ancs_switch, sizeof(app_srv_nrf52_ancs_switch));
#endif
	
}
static void cmd_nrf52_srv_ancs_switch_write(ble_nrf52_evt_t *evt)
{
#ifndef COD 
	_app_srv_nrf52_ancs_switch *app_srv_nrf52_ancs_switch = (_app_srv_nrf52_ancs_switch *)evt->params.data.p_data;
	uint32_t error = NRF_SUCCESS;
	if(app_srv_nrf52_ancs_switch->u_ancs_switch.bits.call)
	{
		g_ancs_switch.call = ANCS_MESSAGE_SWITCH_ON;
	}
	else
	{
		g_ancs_switch.call = ANCS_MESSAGE_SWITCH_OFF;
	}
	if(app_srv_nrf52_ancs_switch->u_ancs_switch.bits.wechat)
	{
		g_ancs_switch.wechat = ANCS_MESSAGE_SWITCH_ON;
	}
	else
	{
		g_ancs_switch.wechat = ANCS_MESSAGE_SWITCH_OFF;
	}	
	if(app_srv_nrf52_ancs_switch->u_ancs_switch.bits.qq)
	{
		g_ancs_switch.qq = ANCS_MESSAGE_SWITCH_ON;
	}
	else
	{
		g_ancs_switch.qq = ANCS_MESSAGE_SWITCH_OFF;
	}		
	if(app_srv_nrf52_ancs_switch->u_ancs_switch.bits.sms)
	{
		g_ancs_switch.sms = ANCS_MESSAGE_SWITCH_ON;
	}
	else
	{
		g_ancs_switch.sms = ANCS_MESSAGE_SWITCH_OFF;
	}			
	
	error = drv_ble_flash_ancs_switch_write(&g_ancs_switch);
	APP_ERROR_CHECK(error);	
	
	uint8_t response[2] = {CMD_NRF52_SRV_ANCS_SWITCH_WRITE_RESPONSE,CMD_NRF52_SRV_SUCCUSS};
	app_srv_nrf52_control_send((uint8_t *)response, sizeof(response));
#endif
}	


static void cmd_nrf52_srv_find_phone_response(ble_nrf52_evt_t *evt)
{
	uint32_t command[5] = {BLE_UART_RESPONSE_APPLICATION_FIND_PHONE | BLE_UART_CCOMMAND_MASK_APPLICATION};
	memcpy((uint8_t *)(command+1),evt->params.data.p_data+1,20);	
	task_ble_conn_handle(TASK_BLE_CONN_EVT_BLE_UART, (uint8_t *)(&command), sizeof(command));
}

static void cmd_nrf52_srv_find_watch(ble_nrf52_evt_t *evt)
{
	uint32_t command[5] = {BLE_UART_CCOMMAND_APPLICATION_FIND_WATCH | BLE_UART_CCOMMAND_MASK_APPLICATION};
	memcpy((uint8_t *)(command+1),evt->params.data.p_data+1,20);	
	task_ble_conn_handle(TASK_BLE_CONN_EVT_BLE_UART, (uint8_t *)(&command), sizeof(command));
}

static void cmd_nrf52_srv_get_watch_default_sport_mode(ble_nrf52_evt_t *evt)
{
    uint32_t command[5] = {BLE_UART_CCOMMAND_APPLICATION_GET_WATCH_DEFAULT_SPORT_MODE | BLE_UART_CCOMMAND_MASK_APPLICATION};
	memcpy((uint8_t *)(command+1),evt->params.data.p_data+1,20);	
	task_ble_conn_handle(TASK_BLE_CONN_EVT_BLE_UART, (uint8_t *)(&command), sizeof(command));
}

#ifdef WATCH_HAS_HYTERA_FUNC
static void cmd_nrf52_srv_hytera_heartrate_collection_start(ble_nrf52_evt_t *evt)
{
	uint32_t command[3] = {BLE_UART_CCOMMAND_APPLICATION_HYTERA_HEARTRATE_COLLECTION_START | BLE_UART_CCOMMAND_MASK_APPLICATION};
	memcpy((uint8_t *)(command+1),evt->params.data.p_data+1,8);
	task_ble_conn_handle(TASK_BLE_CONN_EVT_BLE_UART, (uint8_t *)(&command), sizeof(command));
}

static void cmd_nrf52_srv_hytera_heartrate_collection_stop(ble_nrf52_evt_t *evt)
{
	uint32_t command[1] = {BLE_UART_CCOMMAND_APPLICATION_HYTERA_HEARTRATE_COLLECTION_STOP | BLE_UART_CCOMMAND_MASK_APPLICATION};
	task_ble_conn_handle(TASK_BLE_CONN_EVT_BLE_UART, (uint8_t *)(&command), sizeof(command));
}

static void cmd_nrf52_srv_hytera_sport_data_collection_start(ble_nrf52_evt_t *evt)
{
	uint32_t command[1] = {BLE_UART_CCOMMAND_APPLICATION_HYTERA_SPORT_DATA_COLLECTION_START | BLE_UART_CCOMMAND_MASK_APPLICATION};
	task_ble_conn_handle(TASK_BLE_CONN_EVT_BLE_UART, (uint8_t *)(&command), sizeof(command));
}

static void cmd_nrf52_srv_hytera_sport_data_collection_stop(ble_nrf52_evt_t *evt)
{
	uint32_t command[1] = {BLE_UART_CCOMMAND_APPLICATION_HYTERA_SPORT_DATA_COLLECTION_STOP | BLE_UART_CCOMMAND_MASK_APPLICATION};
	task_ble_conn_handle(TASK_BLE_CONN_EVT_BLE_UART, (uint8_t *)(&command), sizeof(command));
}

static void cmd_nrf52_srv_hytera_sport_data_read(ble_nrf52_evt_t *evt)
{
	uint32_t command[1] = {BLE_UART_CCOMMAND_APPLICATION_HYTERA_SPORT_DATA_READ | BLE_UART_CCOMMAND_MASK_APPLICATION};
	task_ble_conn_handle(TASK_BLE_CONN_EVT_BLE_UART, (uint8_t *)(&command), sizeof(command));
}

static void cmd_nrf52_srv_hytera_heartrate_data_read(ble_nrf52_evt_t *evt)
{
	uint32_t command[1] = {BLE_UART_CCOMMAND_APPLICATION_HYTERA_HEARTRATE_DATA_READ | BLE_UART_CCOMMAND_MASK_APPLICATION};
	task_ble_conn_handle(TASK_BLE_CONN_EVT_BLE_UART, (uint8_t *)(&command), sizeof(command));
}


#endif



