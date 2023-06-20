#include "app_srv_apollo.h"
#include "ble_srv_apollo.h"
#include "drv_apollo2.h"
#include "nrf_delay.h"
#include "task_ble_conn.h"
#include "task_extFlash.h"
#include "task_apollo2.h"
#include "ble_srv_nrf52.h"
#include "ble_conn_params.h"
#include "ble_advertising.h"
#ifdef FREERTOS  
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "portmacro.h"
#include "portable.h"
#include "event_groups.h"
#include "semphr.h"
#endif


#include "nrf_ble_gatt.h"
#include "cmd_apollo2.h"
#include "cmd_apollo_srv.h"

#include "drv_apollo2.h"
#include "dev_apollo2.h"
#include "cmd_apollo2.h"

#include "am_bootloader.h"

#include "cmd_gpregret.h"
#include "nrf_pwr_mgmt.h"
#include "drv_wdt.h"

#define LOG_LEVEL_OFF 							0
#define LOG_LEVEL_ERROR 						1
#define LOG_LEVEL_WARNING                       2
#define LOG_LEVEL_INFO 							3
#define LOG_LEVEL_DEBUG 						4
#define NRF_LOG_MODULE_NAME app_srv_apollo
#define NRF_LOG_LEVEL       LOG_LEVEL_DEBUG
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
NRF_LOG_MODULE_REGISTER();

BLE_APOLLO_DEF(m_apollo);

#ifdef LAYER_BOOTLOADER  
#define DELAY_MS(X)                 nrf_delay_ms(X)
#endif

#ifdef LAYER_APPLICATION 
//#define DELAY_MS(X)                 nrf_delay_ms(X)
#define DELAY_MS(X)                 vTaskDelay(X)
//#define DELAY_MS(X)                   do{TickType_t xLastWakeTime;xLastWakeTime = xTaskGetTickCount ();vTaskDelayUntil( &xLastWakeTime, X / ( ( TickType_t ) 1000 / 3 ) );}while(0)
#endif


extern uint16_t g_conn_handle_peripheral;
extern nrf_ble_gatt_t m_gatt;
extern uint8_t cod_uart_normal_open;
extern void ble_conn_params_set_conn_params(ble_adv_evt_t new_adv_params);


static void timeout_handler(TimerHandle_t xTimer);
static void timer_create(void);
static void timer_start(uint32_t timeout_msec);


static void cmd_apollo_srv_flash_read_raw(ble_apollo_evt_data_t *data);
static void cmd_apollo_srv_flash_write_raw(ble_apollo_evt_data_t *data);
static void cmd_apollo_srv_flash_erase_raw(ble_apollo_evt_data_t *data);
static void cmd_apollo_srv_flash_config_astro_sync_request(ble_apollo_evt_data_t *data);
static void cmd_apollo_srv_flash_config_main_day_data_request(ble_apollo_evt_data_t *data);
static void cmd_apollo_srv_flash_config_activity_address_request(ble_apollo_evt_data_t *data);
static void cmd_apollo_srv_flash_config_activity_address_boundary_request(ble_apollo_evt_data_t *data);
static void cmd_apollo_srv_flash_config_shared_track_address(ble_apollo_evt_data_t *data);	
static void cmd_apollo_srv_flash_config_shared_track_header_write(ble_apollo_evt_data_t *data);	
static void cmd_apollo_srv_flash_config_train_plan_request_uuid(ble_apollo_evt_data_t *data);			
static void cmd_apollo_srv_flash_config_train_plan_read_address_request(ble_apollo_evt_data_t *data);
static void cmd_apollo_srv_flash_config_train_plan_write_address_request(ble_apollo_evt_data_t *data);
static void cmd_apollo_srv_flash_config_train_plan_result_request_uuid(ble_apollo_evt_data_t *data);
static void cmd_apollo_srv_flash_config_train_plan_result_read_address_req(ble_apollo_evt_data_t *data);
static void cmd_apollo_srv_flash_config_train_plan_result_write_address_request(ble_apollo_evt_data_t *data);
static void cmd_apollo_srv_flash_config_train_plan_notify_switch(ble_apollo_evt_data_t *data);
static void cmd_apollo_srv_flash_config_train_plan_erase_address_request(ble_apollo_evt_data_t *data);			
static void cmd_apollo_srv_flash_config_train_plan_result_erase_address_request(ble_apollo_evt_data_t *data);	

static void cmd_apollo_srv_flash_dfu_start(ble_apollo_evt_data_t *data);	
static void cmd_apollo_srv_flash_dfu_ble_bootsetting(ble_apollo_evt_data_t *data);	
static void cmd_apollo_srv_flash_dfu_ble_prevalidate(ble_apollo_evt_data_t *data);	
static void cmd_apollo_srv_flash_dfu_ble_postvalidate(ble_apollo_evt_data_t *data);	
static void cmd_apollo_srv_flash_dfu_ble_reset_N_activate(ble_apollo_evt_data_t *data);	
static void cmd_apollo_srv_flash_dfu_ble_dfu_finish(ble_apollo_evt_data_t *data);	
static void cmd_apollo_srv_flash_dfu_ble_dfu_apollo_bootsetting(ble_apollo_evt_data_t *data);	
static void cmd_apollo_srv_flash_dfu_ble_dfu_apollo_prevalidate(ble_apollo_evt_data_t *data);	
static void cmd_apollo_srv_flash_dfu_ble_dfu_apollo_postvalidate(ble_apollo_evt_data_t *data);
static void cmd_apollo_srv_flash_dfu_ble_dfu_apollo_reset_n_activate(ble_apollo_evt_data_t *data);

static void cmd_apollo_srv_flash_dfu_ble_watch_face_info(ble_apollo_evt_data_t *data);
static void cmd_apollo_srv_flash_dfu_ble_watch_face_info_2(ble_apollo_evt_data_t *data);
static void cmd_apollo_srv_flash_dfu_ble_watch_face_finish(ble_apollo_evt_data_t *data);
static void cmd_apollo_srv_flash_dfu_ble_watch_face_activate(ble_apollo_evt_data_t *data);

static void apollo_sensor_data_upload_command(ble_apollo_evt_data_t *data);

static bool m_uart_packet_mtu_flag = false;


static struct {
	uint8_t buffer[UART_TX_BUF_SIZE];
	uint32_t length;
} m_uart_rx;
static uint8_t m_uart_rx_data_index = 0;





#define SRV_APOLLO_RESET_TYPE_JUMP_BOOTLOADER                  1


#define TIMER_INTERVAL            5000 
#define OSTIMER_WAIT_FOR_QUEUE              2 
static TimerHandle_t m_timer; 
static uint8_t m_reset_type = 0;

extern ble_config_params_t     *g_ble_config;


static void timeout_handler(TimerHandle_t xTimer)
{
	uint32_t err_code;
    UNUSED_PARAMETER(xTimer);
    NRF_LOG_DEBUG("timeout_handler");
	if (pdPASS != xTimerStop(m_timer, OSTIMER_WAIT_FOR_QUEUE))
	{
			APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
	}	
	
	if(m_reset_type == SRV_APOLLO_RESET_TYPE_JUMP_BOOTLOADER)
	{
		NRF_LOG_DEBUG("jump bootloader start sd_power_gpregret_clr");
		err_code = sd_power_gpregret_clr(0, 0xffffffff);
		APP_ERROR_CHECK(err_code);
		err_code = sd_power_gpregret_set(0, CMD_GPREGRET_BOOTLOADER_BLE_DFU);
		APP_ERROR_CHECK(err_code);		
		drv_wdt_feed();	
		nrf_pwr_mgmt_shutdown(NRF_PWR_MGMT_SHUTDOWN_GOTO_DFU);
		NRF_LOG_DEBUG("jump bootloader end sd_power_gpregret_set");
	}	
}

static void timer_create(void)
{
	m_timer = xTimerCreate("DFUR",
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
static uint8_t m_dfu_command_state = 0;

static void apollo_data_handler(ble_apollo_evt_t * p_evt)
{

	switch(p_evt->type)
	{
	
		case BLE_EVT_APOLLO_UART_RX_DATA:{
			NRF_LOG_DEBUG("BLE_EVT_APOLLO_UART_RX_DATA");
//			extern nrf_ble_gatt_t m_gatt;
			uint16_t mtu = nrf_ble_gatt_eff_mtu_get(&m_gatt, g_conn_handle_peripheral);			
//			
			NRF_LOG_INFO("p_evt->params.data.length --> %d",p_evt->params.data.length);
			NRF_LOG_HEXDUMP_INFO(p_evt->params.data.p_data, 32);

			/*
			if(!m_uart_packet_mtu_flag && p_evt->params.data.p_data[0] == UART_PACKET_HEADER)
			{
				m_uart_packet_mtu_flag = true;
				m_uart_rx_data_index = p_evt->params.data.length;
				memcpy(m_uart_rx.buffer,(uint8_t *)p_evt->params.data.p_data,p_evt->params.data.length);						
				
			}else{
				m_uart_packet_mtu_flag = false;
				
				if((p_evt->params.data.length + m_uart_rx_data_index) == 256)
				{
					uint16_t len = ((m_uart_rx_data_index+p_evt->params.data.length)>256) ? (256-m_uart_rx_data_index) : p_evt->params.data.length;
					memcpy(m_uart_rx.buffer+m_uart_rx_data_index,(uint8_t *)p_evt->params.data.p_data,len);
					m_uart_rx.length = 256;
					m_uart_rx_data_index = 0;
					task_ble_conn_handle(TASK_BLE_CONN_EVT_BLE_UART, m_uart_rx.buffer, m_uart_rx.length);						
				}
			}
			*/

			
			if((!m_uart_packet_mtu_flag || (p_evt->params.data.length == mtu)) && p_evt->params.data.p_data[0] == UART_PACKET_HEADER)
			{
				m_uart_packet_mtu_flag = true;
				m_uart_rx_data_index = p_evt->params.data.length;
				memcpy(m_uart_rx.buffer,(uint8_t *)p_evt->params.data.p_data,p_evt->params.data.length);						
				
			}else{
				m_uart_packet_mtu_flag = false;
				
				if((p_evt->params.data.length + m_uart_rx_data_index) == 256)
				{
					uint16_t len = ((m_uart_rx_data_index+p_evt->params.data.length)>256) ? (256-m_uart_rx_data_index) : p_evt->params.data.length;
					memcpy(m_uart_rx.buffer+m_uart_rx_data_index,(uint8_t *)p_evt->params.data.p_data,len);
					m_uart_rx.length = 256;
					m_uart_rx_data_index = 0;
					task_ble_conn_handle(TASK_BLE_CONN_EVT_BLE_UART, m_uart_rx.buffer, m_uart_rx.length);						
				}
			}
			
			
			

#if 0			
			if(p_evt->params.data.p_data[0] == UART_PACKET_HEADER)
			{
				NRF_LOG_DEBUG("UART_PACKET_HEADER");
				NRF_LOG_DEBUG("p_evt->params.data.length --> %d",p_evt->params.data.length);
				
				if(p_evt->params.data.length == (mtu - 3))
				{
					m_uart_rx_data_index = p_evt->params.data.length;
					memcpy(m_uart_rx.buffer,(uint8_t *)p_evt->params.data.p_data,p_evt->params.data.length);				
				}
			}
		 else if(m_uart_rx_data_index == (mtu - 3))
		 {
				NRF_LOG_DEBUG("p_evt->params.data.length --> %d",p_evt->params.data.length);				
				memcpy(m_uart_rx.buffer+m_uart_rx_data_index,(uint8_t *)p_evt->params.data.p_data,p_evt->params.data.length);
				m_uart_rx.length = m_uart_rx_data_index + p_evt->params.data.length;
				m_uart_rx_data_index = 0;
				task_ble_conn_handle(TASK_BLE_CONN_EVT_BLE_UART, m_uart_rx.buffer, m_uart_rx.length);			
		 }
#endif
		 
		 
#if 0
				memcpy(m_uart_rx.buffer,(uint8_t *)p_evt->params.data.p_data,p_evt->params.data.length);	
				m_uart_rx_data_index = p_evt->params.data.length;
				task_ble_conn_handle(TASK_BLE_CONN_EVT_BLE_UART, m_uart_rx.buffer, p_evt->params.data.length);
#endif				
		}break;
		case BLE_EVT_APOLLO_TX_RDY:{
			//NRF_LOG_DEBUG("BLE_EVT_APOLLO_TX_RDY");
		
		}break;
		case BLE_EVT_APOLLO_UART_NOTIFY_ENABLE:{
			NRF_LOG_INFO("BLE_EVT_APOLLO_UART_NOTIFY_ENABLE");
			ble_srv_common_char_notify_change(APOLLO_SRV_UART_CHAR_NOTIFY, true);
		}break;
		case BLE_EVT_APOLLO_UART_NOTIFY_DISABLE:{
			NRF_LOG_INFO("BLE_EVT_APOLLO_UART_NOTIFY_DISABLE");
			ble_srv_common_char_notify_change(APOLLO_SRV_UART_CHAR_NOTIFY, false);
		}break;
		case BLE_EVT_APOLLO_CONNECTED:{
			NRF_LOG_DEBUG("BLE_EVT_APOLLO_CONNECTED");
			m_dfu_command_state = 0;
			m_uart_rx_data_index = 0;
			m_uart_packet_mtu_flag = false;
			memset(m_uart_rx.buffer,0,sizeof(m_uart_rx.buffer));
			NRF_LOG_HEXDUMP_INFO(m_uart_rx.buffer, 32);
			
		}break;	
		case BLE_EVT_APOLLO_DISCONNECTED:{
			NRF_LOG_DEBUG("BLE_EVT_APOLLO_DISCONNECTED");
			m_uart_rx_data_index = 0;
			m_uart_packet_mtu_flag = false;
			memset(m_uart_rx.buffer,0,sizeof(m_uart_rx.buffer));
			NRF_LOG_HEXDUMP_INFO(m_uart_rx.buffer, 32);
		}break;	
		
		
		
		case BLE_EVT_APOLLO_FLASH_DATA:{
			NRF_LOG_DEBUG("BLE_EVT_APOLLO_FLASH_DATA");

			switch(p_evt->params.data.p_data[0])
			{
				case CMD_APOLLO_SRV_FLASH_READ_RAW:{
					NRF_LOG_DEBUG("CMD_APOLLO_SRV_FLASH_READ_RAW");
					NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
					cmd_apollo_srv_flash_read_raw(&p_evt->params.data);
				}break;
				case CMD_APOLLO_SRV_FLASH_WRITE_RAW:{
					NRF_LOG_DEBUG("CMD_APOLLO_SRV_FLASH_WRITE_RAW");
					NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
					if( m_dfu_command_state != CMD_APOLLO_SRV_FLASH_DFU_BLE_POSTVALIDATE &&
							m_dfu_command_state != CMD_APOLLO_SRV_FLASH_DFU_BLE_RESET_N_ACTIVATE &&
							m_dfu_command_state != CMD_APOLLO_SRV_FLASH_DFU_BLE_DFU_APOLLO_POSTVALIDATE &&
							m_dfu_command_state != CMD_APOLLO_SRV_FLASH_DFU_BLE_DFU_APOLLO_RESET_N_ACTIVATE 
						)
					{
						cmd_apollo_srv_flash_write_raw(&p_evt->params.data);
					}
					
				}break;
				case CMD_APOLLO_SRV_FLASH_ERASE_RAW:{
					NRF_LOG_DEBUG("CMD_APOLLO_SRV_FLASH_ERASE_RAW");
					NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
					if( m_dfu_command_state != CMD_APOLLO_SRV_FLASH_DFU_BLE_POSTVALIDATE &&
							m_dfu_command_state != CMD_APOLLO_SRV_FLASH_DFU_BLE_RESET_N_ACTIVATE &&
							m_dfu_command_state != CMD_APOLLO_SRV_FLASH_DFU_BLE_DFU_APOLLO_POSTVALIDATE &&
							m_dfu_command_state != CMD_APOLLO_SRV_FLASH_DFU_BLE_DFU_APOLLO_RESET_N_ACTIVATE 
						)
					{
						cmd_apollo_srv_flash_erase_raw(&p_evt->params.data);
					}					
				}break;	
				case CMD_APOLLO_SRV_FLASH_CONFIG_ASTRO_SYNC_RRQUEST:{
					NRF_LOG_INFO("CMD_APOLLO_SRV_FLASH_CONFIG_ASTRO_SYNC_RRQUEST");
					NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
					cmd_apollo_srv_flash_config_astro_sync_request(&p_evt->params.data);
				}break;	
				case CMD_APOLLO_SRV_FLASH_CONFIG_MAIN_DAY_DATA_RRQUEST:{
					NRF_LOG_INFO("CMD_APOLLO_SRV_FLASH_CONFIG_MAIN_DAY_DATA_RRQUEST");
					NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
					cmd_apollo_srv_flash_config_main_day_data_request(&p_evt->params.data);
				}break;	
				case CMD_APOLLO_SRV_FLASH_CONFIG_ACTIVITY_ADDRESS_RRQUEST:{
					NRF_LOG_INFO("CMD_APOLLO_SRV_FLASH_CONFIG_ACTIVITY_ADDRESS_RRQUEST");
					NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
					cmd_apollo_srv_flash_config_activity_address_request(&p_evt->params.data);
				}break;	
				case CMD_APOLLO_SRV_FLASH_CONFIG_ACTIVITY_ADDRESS_BOUNDARY_RRQUEST:{
					NRF_LOG_INFO("CMD_APOLLO_SRV_FLASH_CONFIG_ACTIVITY_ADDRESS_BOUNDARY_RRQUEST");
					NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
					cmd_apollo_srv_flash_config_activity_address_boundary_request(&p_evt->params.data);
				}break;		
				case CMD_APOLLO_SRV_FLASH_CONFIG_SHARED_TRACK_ADDRESS_REQUEST:{
					NRF_LOG_INFO("CMD_APOLLO_SRV_FLASH_CONFIG_SHARED_TRACK_ADDRESS_REQUEST");
					NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
					cmd_apollo_srv_flash_config_shared_track_address(&p_evt->params.data);
				}break;	
				case CMD_APOLLO_SRV_FLASH_CONFIG_SHARED_TRACK_HEADER_WRITE:{
					NRF_LOG_INFO("CMD_APOLLO_SRV_FLASH_CONFIG_SHARED_TRACK_HEADER_WRITE");
					NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
					cmd_apollo_srv_flash_config_shared_track_header_write(&p_evt->params.data);
				}break;				
				case CMD_APOLLO_SRV_FLASH_CONFIG_TRAIN_PLAN_REQUEST_UUID:{
					NRF_LOG_INFO("CMD_APOLLO_SRV_FLASH_CONFIG_TRAIN_PLAN_REQUEST_UUID");
					NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
					cmd_apollo_srv_flash_config_train_plan_request_uuid(&p_evt->params.data);
				}break;			
				case CMD_APOLLO_SRV_FLASH_CONFIG_TRAIN_PLAN_READ_ADDRESS_REQUEST:{
					NRF_LOG_INFO("CMD_APOLLO_SRV_FLASH_CONFIG_TRAIN_PLAN_READ_ADDRESS_REQUEST");
					NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
					cmd_apollo_srv_flash_config_train_plan_read_address_request(&p_evt->params.data);
				}break;								
				case CMD_APOLLO_SRV_FLASH_CONFIG_TRAIN_PLAN_WRITE_ADDRESS_REQUEST:{
					NRF_LOG_INFO("CMD_APOLLO_SRV_FLASH_CONFIG_TRAIN_PLAN_WRITE_ADDRESS_REQUEST");
					NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
					cmd_apollo_srv_flash_config_train_plan_write_address_request(&p_evt->params.data);
				}break;		
				case CMD_APOLLO_SRV_FLASH_CONFIG_TRAIN_PLAN_ERASE_ADDRESS_REQUEST:{
					NRF_LOG_INFO("CMD_APOLLO_SRV_FLASH_CONFIG_TRAIN_PLAN_ERASE_ADDRESS_REQUEST");
					NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
					cmd_apollo_srv_flash_config_train_plan_erase_address_request(&p_evt->params.data);
				}break;					
				case CMD_APOLLO_SRV_FLASH_CONFIG_TRAIN_PLAN_RESULT_REQUEST_UUID:{
					NRF_LOG_INFO("CMD_APOLLO_SRV_FLASH_CONFIG_TRAIN_PLAN_RESULT_REQUEST_UUID");
					NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
					cmd_apollo_srv_flash_config_train_plan_result_request_uuid(&p_evt->params.data);
				}break;			
				case CMD_APOLLO_SRV_FLASH_CONFIG_TRAIN_PLAN_RESULT_READ_ADDRESS_REQUEST:{
					NRF_LOG_INFO("CMD_APOLLO_SRV_FLASH_CONFIG_TRAIN_PLAN_RESULT_READ_ADDRESS_REQUEST");
					NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
					cmd_apollo_srv_flash_config_train_plan_result_read_address_req(&p_evt->params.data);
				}break;								
				case CMD_APOLLO_SRV_FLASH_CONFIG_TRAIN_PLAN_RESULT_WRITE_ADDRESS_REQUEST:{
					NRF_LOG_INFO("CMD_APOLLO_SRV_FLASH_CONFIG_TRAIN_PLAN_RESULT_WRITE_ADDRESS_REQUEST");
					NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
					cmd_apollo_srv_flash_config_train_plan_result_write_address_request(&p_evt->params.data);
				}break;	
				case CMD_APOLLO_SRV_FLASH_CONFIG_TRAIN_PLAN_RESULT_ERASE_ADDRESS_REQUEST:{
					NRF_LOG_INFO("CMD_APOLLO_SRV_FLASH_CONFIG_TRAIN_PLAN_RESULT_ERASE_ADDRESS_REQUEST");
					NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
					cmd_apollo_srv_flash_config_train_plan_result_erase_address_request(&p_evt->params.data);
				}break;				
				
				case CMD_APOLLO_SRV_FLASH_CONFIG_TRAIN_PLAN_NOTIFY_SWITCH:{
					NRF_LOG_INFO("CMD_APOLLO_SRV_FLASH_CONFIG_TRAIN_PLAN_NOTIFY_SWITCH");
					NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
					cmd_apollo_srv_flash_config_train_plan_notify_switch(&p_evt->params.data);
				}break;	
				
				case CMD_APOLLO_SRV_FLASH_DFU_START:{
					NRF_LOG_INFO("CMD_APOLLO_SRV_FLASH_DFU_START");
					NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
					cmd_apollo_srv_flash_dfu_start(&p_evt->params.data);
				}break;	
				case CMD_APOLLO_SRV_FLASH_DFU_BLE_BOOTSETTING:{
					NRF_LOG_INFO("CMD_APOLLO_SRV_FLASH_DFU_BLE_BOOTSETTING");
					NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
					cmd_apollo_srv_flash_dfu_ble_bootsetting(&p_evt->params.data);
				}break;		
				case CMD_APOLLO_SRV_FLASH_DFU_BLE_PREVALIDATE:{
					NRF_LOG_INFO("CMD_APOLLO_SRV_FLASH_DFU_BLE_PREVALIDATE");
					NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
					cmd_apollo_srv_flash_dfu_ble_prevalidate(&p_evt->params.data);
				}break;	
				case CMD_APOLLO_SRV_FLASH_DFU_BLE_POSTVALIDATE:{
					NRF_LOG_INFO("CMD_APOLLO_SRV_FLASH_DFU_BLE_POSTVALIDATE");
					NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
					if(m_dfu_command_state != CMD_APOLLO_SRV_FLASH_DFU_BLE_POSTVALIDATE)
					{
						m_dfu_command_state = CMD_APOLLO_SRV_FLASH_DFU_BLE_POSTVALIDATE;
						cmd_apollo_srv_flash_dfu_ble_postvalidate(&p_evt->params.data);					
					}					
				}break;					
				case CMD_APOLLO_SRV_FLASH_DFU_BLE_RESET_N_ACTIVATE:{
					NRF_LOG_INFO("CMD_APOLLO_SRV_FLASH_DFU_BLE_RESET_N_ACTIVATE");
					NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
					if(m_dfu_command_state != CMD_APOLLO_SRV_FLASH_DFU_BLE_RESET_N_ACTIVATE)
					{
						m_dfu_command_state = CMD_APOLLO_SRV_FLASH_DFU_BLE_RESET_N_ACTIVATE;
						cmd_apollo_srv_flash_dfu_ble_reset_N_activate(&p_evt->params.data);					
					}								
				}break;					
				case CMD_APOLLO_SRV_FLASH_DFU_BLE_DFU_FINISH:{
					NRF_LOG_INFO("CMD_APOLLO_SRV_FLASH_DFU_BLE_DFU_FINISH");
					NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
					cmd_apollo_srv_flash_dfu_ble_dfu_finish(&p_evt->params.data);
				}break;						
				case CMD_APOLLO_SRV_FLASH_DFU_BLE_DFU_APOLLO_BOOTSETTING:{
					NRF_LOG_INFO("CMD_APOLLO_SRV_FLASH_DFU_BLE_DFU_APOLLO_BOOTSETTING");
					NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
					m_dfu_command_state = CMD_APOLLO_SRV_FLASH_DFU_BLE_DFU_APOLLO_BOOTSETTING;
					cmd_apollo_srv_flash_dfu_ble_dfu_apollo_bootsetting(&p_evt->params.data);
				}break;					
				case CMD_APOLLO_SRV_FLASH_DFU_BLE_DFU_APOLLO_PREVALIDATE:{
					NRF_LOG_INFO("CMD_APOLLO_SRV_FLASH_DFU_BLE_DFU_APOLLO_PREVALIDATE");
					NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
					m_dfu_command_state = CMD_APOLLO_SRV_FLASH_DFU_BLE_DFU_APOLLO_PREVALIDATE;
					cmd_apollo_srv_flash_dfu_ble_dfu_apollo_prevalidate(&p_evt->params.data);
				}break;		
				case CMD_APOLLO_SRV_FLASH_DFU_BLE_DFU_APOLLO_POSTVALIDATE:{
					NRF_LOG_INFO("CMD_APOLLO_SRV_FLASH_DFU_BLE_DFU_APOLLO_POSTVALIDATE");
					NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
					if(m_dfu_command_state != CMD_APOLLO_SRV_FLASH_DFU_BLE_DFU_APOLLO_POSTVALIDATE)
					{
						m_dfu_command_state = CMD_APOLLO_SRV_FLASH_DFU_BLE_DFU_APOLLO_POSTVALIDATE;
						cmd_apollo_srv_flash_dfu_ble_dfu_apollo_postvalidate(&p_evt->params.data);					
					}
				}break;	
				case CMD_APOLLO_SRV_FLASH_DFU_BLE_DFU_APOLLO_RESET_N_ACTIVATE:{
					NRF_LOG_INFO("CMD_APOLLO_SRV_FLASH_DFU_BLE_DFU_APOLLO_RESET_N_ACTIVATE");
					NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
					if(m_dfu_command_state != CMD_APOLLO_SRV_FLASH_DFU_BLE_DFU_APOLLO_RESET_N_ACTIVATE)
					{
						m_dfu_command_state = CMD_APOLLO_SRV_FLASH_DFU_BLE_DFU_APOLLO_RESET_N_ACTIVATE;
						cmd_apollo_srv_flash_dfu_ble_dfu_apollo_reset_n_activate(&p_evt->params.data);					
					}

				}break;	
				
				case CMD_APOLLO_SRV_FLASH_DFU_BLE_WATCH_FACE_INFO:{
					NRF_LOG_INFO("CMD_APOLLO_SRV_FLASH_DFU_BLE_WATCH_FACE_INFO");
					NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
					cmd_apollo_srv_flash_dfu_ble_watch_face_info(&p_evt->params.data);	
					NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);					
				}break;	
				case CMD_APOLLO_SRV_FLASH_DFU_BLE_WATCH_FACE_INFO_2:{
					NRF_LOG_INFO("CMD_APOLLO_SRV_FLASH_DFU_BLE_WATCH_FACE_INFO_2");
					NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
					cmd_apollo_srv_flash_dfu_ble_watch_face_info_2(&p_evt->params.data);	
					NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);						
				}break;					
				case CMD_APOLLO_SRV_FLASH_DFU_BLE_WATCH_FACE_FINISH:{
					NRF_LOG_INFO("CMD_APOLLO_SRV_FLASH_DFU_BLE_WATCH_FACE_FINISH");
					NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
					cmd_apollo_srv_flash_dfu_ble_watch_face_finish(&p_evt->params.data);		
				}break;		
				case CMD_APOLLO_SRV_FLASH_DFU_BLE_WATCH_FACE_ACTIVATE:{
					NRF_LOG_INFO("CMD_APOLLO_SRV_FLASH_DFU_BLE_WATCH_FACE_ACTIVATE");
					NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
					cmd_apollo_srv_flash_dfu_ble_watch_face_activate(&p_evt->params.data);		
				}break;						
			
				
				default:
				{
					NRF_LOG_INFO("UNSPPORT COMMAND");
					uint8_t response = 0xFF;
					app_srv_apollo_flash_send((uint8_t *)&response, sizeof(response));				
				}break;
				
				
			}
			
			
			
			
		}break;		
		case BLE_EVT_APOLLO_FLASH_NOTIFY_ENABLE:{
			NRF_LOG_INFO("BLE_EVT_APOLLO_FLASH_NOTIFY_ENABLE");
			ble_srv_common_char_notify_change(APOLLO_SRV_FLASH_CHAR_NOTIFY, true);
		}break;	
		case BLE_EVT_APOLLO_FLASH_NOTIFY_DISABLE:{
			NRF_LOG_INFO("BLE_EVT_APOLLO_FLASH_NOTIFY_DISABLE");
			ble_srv_common_char_notify_change(APOLLO_SRV_FLASH_CHAR_NOTIFY, false);
		}break;
		case BLE_EVT_APOLLO_SENSOR_DATA:{
			switch(p_evt->params.data.p_data[0])
			{
				case APOLLO_SENSOR_DATA_UPLOAD_COMMAND:{
					NRF_LOG_INFO("CMD_APOLLO_SENSOR_DATA_UPLOAD");
					apollo_sensor_data_upload_command(&p_evt->params.data);
				}break;	
				
				default:
				{
					NRF_LOG_INFO("UNSPPORT COMMAND");
					uint8_t response = 0xFF;
					app_srv_apollo_sensor_send((uint8_t *)&response, sizeof(response));				
				}break;
				
				
			}
			
			
		}break;
		case BLE_EVT_APOLLO_SENSOR_NOTIFY_ENABLE:{
			NRF_LOG_INFO("BLE_EVT_APOLLO_SENSOR_NOTIFY_ENABLE");
			ble_srv_common_char_notify_change(APOLLO_SRV_SENSOR_CHAR_NOTIFY, true);
		}break;	
		case BLE_EVT_APOLLO_SENSOR_NOTIFY_DISABLE:{
			NRF_LOG_INFO("BLE_EVT_APOLLO_SENSOR_NOTIFY_DISABLE");
			ble_srv_common_char_notify_change(APOLLO_SRV_SENSOR_CHAR_NOTIFY, false);
		}break;
		default:
		{

		}
			break;
	}
	
	


}




void app_srv_apollo_init(void)
{
	NRF_LOG_DEBUG("app_srv_apollo_init");
    uint32_t       err_code;
    ble_apollo_init_t apollo_init;

    memset(&apollo_init, 0, sizeof(apollo_init));

    apollo_init.data_handler = apollo_data_handler;

    err_code = ble_apollo_init(&m_apollo, &apollo_init);
    APP_ERROR_CHECK(err_code);
	
	timer_create();
	
}



void app_srv_apollo_uart_send(uint8_t *p_data, uint32_t length)
{
	NRF_LOG_DEBUG("app_srv_apollo_uart_send 0 %x",p_data[0]);

#ifndef COD 
	uint16_t mtu = nrf_ble_gatt_eff_mtu_get(&m_gatt, g_conn_handle_peripheral) - 3;	
	uint32_t bytes_sent = 0;
	uint16_t packet_length = 0;	
	uint32_t err_code;
	NRF_LOG_DEBUG("app_srv_apollo_uart_send");
	NRF_LOG_DEBUG("mtu --> %d",mtu);

	while(bytes_sent < length)
	{
		packet_length = (length - bytes_sent) > mtu ? mtu : length - bytes_sent;
		
		NRF_LOG_DEBUG("packet_length --> %d",packet_length);
		
		err_code = ble_apollo_uart_string_send(&m_apollo, p_data + bytes_sent, &packet_length);	
		NRF_LOG_DEBUG("err_code --> 0x%08X",err_code);
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
#endif
}



void app_srv_apollo_flash_send(uint8_t *p_data, uint32_t length)
{
	NRF_LOG_DEBUG("app_srv_apollo_flash_send 0 %x",p_data[0]);

#ifndef COD 

	uint16_t mtu = nrf_ble_gatt_eff_mtu_get(&m_gatt, g_conn_handle_peripheral) - 3;	
	uint32_t bytes_sent = 0;
	uint16_t packet_length = 0;	
	uint32_t err_code;
	NRF_LOG_DEBUG("app_srv_apollo_flash_send");
	NRF_LOG_DEBUG("mtu --> %d",mtu);
	while(bytes_sent < length)
	{
		packet_length = (length - bytes_sent) > mtu ? mtu : length - bytes_sent;
		
		NRF_LOG_DEBUG("packet_length --> %d",packet_length);	
		err_code = ble_apollo_flash_string_send(&m_apollo, p_data + bytes_sent, &packet_length);	
		NRF_LOG_DEBUG("err_code --> 0x%08X",err_code);
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
	
#endif
}

void app_srv_apollo_sensor_send(uint8_t *p_data, uint32_t length)
{
	NRF_LOG_DEBUG("app_srv_apollo_sensor_send 0 %x",p_data[0]);

#ifndef COD 

	uint16_t mtu = nrf_ble_gatt_eff_mtu_get(&m_gatt, g_conn_handle_peripheral) - 3;	
	uint32_t bytes_sent = 0;
	uint16_t packet_length = 0;	
	uint32_t err_code;
	NRF_LOG_DEBUG("app_srv_apollo_sensor_send");
	NRF_LOG_DEBUG("mtu --> %d",mtu);
	while(bytes_sent < length)
	{
		packet_length = (length - bytes_sent) > mtu ? mtu : length - bytes_sent;
		
		NRF_LOG_DEBUG("packet_length --> %d",packet_length);	
		err_code = ble_apollo_sensor_string_send(&m_apollo, p_data + bytes_sent, &packet_length);	
		NRF_LOG_DEBUG("err_code --> 0x%08X",err_code);
		/**
		 * @retval ::NRF_ERROR_BUSY For @ref BLE_GATT_HVX_INDICATION Procedure already in progress. Wait for a @ref BLE_GATTS_EVT_HVC event and retry.		
		 * @retval ::NRF_ERROR_RESOURCES Too many notifications queued.
		 *                               Wait for a @ref BLE_GATTS_EVT_HVN_TX_COMPLETE event and retry.		
		*/
		if(err_code == NRF_ERROR_BUSY) 
		{
			//Wait for a @ref BLE_GATTS_EVT_HVC event and retry
			DELAY_MS(1);
		}
		else if(err_code == NRF_ERROR_RESOURCES) 
		{
			//Wait for a @ref BLE_GATTS_EVT_HVN_TX_COMPLETE event and retry
			DELAY_MS(1);
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
	
#endif
}



static void cmd_apollo_srv_flash_read_raw(ble_apollo_evt_data_t *data)
{
	task_extFlash_handle(TASK_EXTFLASH_EVT_READ_RAW, 0, (uint8_t *)(data->p_data), data->length);
}

static void cmd_apollo_srv_flash_write_raw(ble_apollo_evt_data_t *data)
{
	task_extFlash_handle(TASK_EXTFLASH_EVT_WRITE_RAW, 0, (uint8_t *)(data->p_data), data->length);
}

static void cmd_apollo_srv_flash_erase_raw(ble_apollo_evt_data_t *data)
{
	task_extFlash_handle(TASK_EXTFLASH_EVT_ERASE_RAW, 0, (uint8_t *)(data->p_data), data->length);
}

static void cmd_apollo_srv_flash_config_astro_sync_request(ble_apollo_evt_data_t *data)
{
	uint32_t command[128] = {BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_ASTRO_SYNC_REQUEST | BLE_UART_CCOMMAND_MASK_APPLICATION};
	memcpy((uint8_t *)(command+1),data->p_data+1,sizeof(command)-1);		
	task_ble_conn_handle(TASK_BLE_CONN_EVT_BLE_UART, (uint8_t *)(&command), sizeof(command));
}

static void cmd_apollo_srv_flash_config_main_day_data_request(ble_apollo_evt_data_t *data)
{
	uint32_t command[128] = {BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_MAIN_DAY_DATA_REQUEST | BLE_UART_CCOMMAND_MASK_APPLICATION};
	memcpy((uint8_t *)(command+1),data->p_data+1,sizeof(command)-1);	
	task_ble_conn_handle(TASK_BLE_CONN_EVT_BLE_UART, (uint8_t *)(&command), sizeof(command));	
}

static void cmd_apollo_srv_flash_config_activity_address_request(ble_apollo_evt_data_t *data)
{
	uint32_t command[128] = {BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_ACTIVITY_ADDRESS_REQUEST | BLE_UART_CCOMMAND_MASK_APPLICATION};
	memcpy((uint8_t *)(command+1),data->p_data+1,sizeof(command)-1);		
	task_ble_conn_handle(TASK_BLE_CONN_EVT_BLE_UART, (uint8_t *)(&command), sizeof(command));		
}

static void cmd_apollo_srv_flash_config_activity_address_boundary_request(ble_apollo_evt_data_t *data)
{
	uint32_t command[128] = {BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_ACTIVITY_ADDRESS_BOUNDARY_REQUEST | BLE_UART_CCOMMAND_MASK_APPLICATION};
	memcpy((uint8_t *)(command+1),data->p_data+1,sizeof(command)-1);	
	task_ble_conn_handle(TASK_BLE_CONN_EVT_BLE_UART, (uint8_t *)(&command), sizeof(command));

}
static void cmd_apollo_srv_flash_config_shared_track_address(ble_apollo_evt_data_t *data)
{
	uint32_t command[128] = {BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_SHARD_TRACK_ADDRESS_REQUEST | BLE_UART_CCOMMAND_MASK_APPLICATION};
	memcpy((uint8_t *)(command+1),data->p_data+1,sizeof(command)-1);		
	task_ble_conn_handle(TASK_BLE_CONN_EVT_BLE_UART, (uint8_t *)(&command), sizeof(command));
}
static void cmd_apollo_srv_flash_config_shared_track_header_write(ble_apollo_evt_data_t *data)
{
	uint32_t command[128] = {BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_SHARD_TRACK_HEADER_WRITE | BLE_UART_CCOMMAND_MASK_APPLICATION};
	memcpy((uint8_t *)(command+1),data->p_data+1,sizeof(command)-1);		
	task_ble_conn_handle(TASK_BLE_CONN_EVT_BLE_UART, (uint8_t *)(&command), sizeof(command));
}

static void cmd_apollo_srv_flash_config_train_plan_request_uuid(ble_apollo_evt_data_t *data)
{
	uint32_t command[128] = {BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_REQUEST_UUID | BLE_UART_CCOMMAND_MASK_APPLICATION};
	memcpy((uint8_t *)(command+1),data->p_data+1,sizeof(command)-1);		
	task_ble_conn_handle(TASK_BLE_CONN_EVT_BLE_UART, (uint8_t *)(&command), sizeof(command));
}	
static void cmd_apollo_srv_flash_config_train_plan_read_address_request(ble_apollo_evt_data_t *data)
{
	uint32_t command[128] = {BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_READ_ADDRESS_REQUEST | BLE_UART_CCOMMAND_MASK_APPLICATION};
	memcpy((uint8_t *)(command+1),data->p_data+1,sizeof(command)-1);		
	task_ble_conn_handle(TASK_BLE_CONN_EVT_BLE_UART, (uint8_t *)(&command), sizeof(command));
}	
static void cmd_apollo_srv_flash_config_train_plan_write_address_request(ble_apollo_evt_data_t *data)
{
	uint32_t command[128] = {BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_WRITE_ADDRESS_REQUEST | BLE_UART_CCOMMAND_MASK_APPLICATION};
	memcpy((uint8_t *)(command+1),data->p_data+1,sizeof(command)-1);		
	task_ble_conn_handle(TASK_BLE_CONN_EVT_BLE_UART, (uint8_t *)(&command), sizeof(command));
}	
static void cmd_apollo_srv_flash_config_train_plan_result_request_uuid(ble_apollo_evt_data_t *data)
{
	uint32_t command[128] = {BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_RESULT_REQUEST_UUID | BLE_UART_CCOMMAND_MASK_APPLICATION};
	memcpy((uint8_t *)(command+1),data->p_data+1,sizeof(command)-1);		
	task_ble_conn_handle(TASK_BLE_CONN_EVT_BLE_UART, (uint8_t *)(&command), sizeof(command));
}	
static void cmd_apollo_srv_flash_config_train_plan_result_read_address_req(ble_apollo_evt_data_t *data)
{
	uint32_t command[128] = {BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_RESULT_READ_ADDRESS_REQUEST | BLE_UART_CCOMMAND_MASK_APPLICATION};
	memcpy((uint8_t *)(command+1),data->p_data+1,sizeof(command)-1);		
	task_ble_conn_handle(TASK_BLE_CONN_EVT_BLE_UART, (uint8_t *)(&command), sizeof(command));
}	
static void cmd_apollo_srv_flash_config_train_plan_result_write_address_request(ble_apollo_evt_data_t *data)
{
	uint32_t command[128] = {BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_RESULT_WRITE_ADDRESS_REQUEST | BLE_UART_CCOMMAND_MASK_APPLICATION};
	memcpy((uint8_t *)(command+1),data->p_data+1,sizeof(command)-1);		
	task_ble_conn_handle(TASK_BLE_CONN_EVT_BLE_UART, (uint8_t *)(&command), sizeof(command));
}	
static void cmd_apollo_srv_flash_config_train_plan_notify_switch(ble_apollo_evt_data_t *data)
{
	uint32_t command[128] = {BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_TRAIN_NOTIFY_SWITCH | BLE_UART_CCOMMAND_MASK_APPLICATION};
	memcpy((uint8_t *)(command+1),data->p_data+1,sizeof(command)-1);		
	task_ble_conn_handle(TASK_BLE_CONN_EVT_BLE_UART, (uint8_t *)(&command), sizeof(command));
}	
static void cmd_apollo_srv_flash_config_train_plan_erase_address_request(ble_apollo_evt_data_t *data)
{
	uint32_t command[128] = {BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_ERASE_ADDRESS_REQUEST | BLE_UART_CCOMMAND_MASK_APPLICATION};
	memcpy((uint8_t *)(command+1),data->p_data+1,sizeof(command)-1);		
	task_ble_conn_handle(TASK_BLE_CONN_EVT_BLE_UART, (uint8_t *)(&command), sizeof(command));
}
static void cmd_apollo_srv_flash_config_train_plan_result_erase_address_request(ble_apollo_evt_data_t *data)
{
	uint32_t command[128] = {BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_RESULT_ERASE_ADDRESS_REQUEST | BLE_UART_CCOMMAND_MASK_APPLICATION};
	memcpy((uint8_t *)(command+1),data->p_data+1,sizeof(command)-1);		
	task_ble_conn_handle(TASK_BLE_CONN_EVT_BLE_UART, (uint8_t *)(&command), sizeof(command));
}



static void cmd_apollo_srv_flash_dfu_start(ble_apollo_evt_data_t *data)
{
	uint32_t command[180] = {BLE_UART_CCOMMAND_APP_BLE_DFU_START | BLE_UART_CCOMMAND_MASK_APP};
	memcpy((uint8_t *)(command+1),data->p_data+1,sizeof(command)-1);		
	task_ble_conn_handle(TASK_BLE_CONN_EVT_BLE_UART, (uint8_t *)(&command), sizeof(command));


}

_ble_boot_setting g_ble_boot_setting;

static void cmd_apollo_srv_flash_dfu_ble_bootsetting(ble_apollo_evt_data_t *data)
{
	NRF_LOG_HEXDUMP_INFO(data->p_data, data->length);
	
	uint32_t crc_src = *(uint32_t *)((uint8_t *)data->p_data + 1);
	NRF_LOG_INFO("crc32 --> 0x%08X",crc_src);
	
	g_ble_boot_setting = *(_ble_boot_setting *)((uint8_t *)data->p_data + 1 + 4); 
	
	
	NRF_LOG_INFO("ble boot_setting_crc --> 0x%08x",g_ble_boot_setting.boot_setting_crc);
	NRF_LOG_INFO("ble boot_setting_size --> %d",g_ble_boot_setting.boot_setting_size);
	NRF_LOG_INFO("ble pcb_version --> 0x%08x",g_ble_boot_setting.pcb_version);
	NRF_LOG_INFO("ble bank_bootloader extFlash_address --> 0x%08x",g_ble_boot_setting.bank_bootloader.extFlash_address);
	NRF_LOG_INFO("ble bank_bootloader image_size --> %d",g_ble_boot_setting.bank_bootloader.image_size);
	NRF_LOG_INFO("ble bank_bootloader image_crc --> 0x%08x",g_ble_boot_setting.bank_bootloader.image_crc);
	NRF_LOG_INFO("ble bank_bootloader version --> 0x%08x",g_ble_boot_setting.bank_bootloader.version);
	NRF_LOG_INFO("ble bank_bootloader command --> 0x%08x",g_ble_boot_setting.bank_bootloader.command);
	NRF_LOG_INFO("ble bank_application extFlash_address --> 0x%08x",g_ble_boot_setting.bank_application.extFlash_address);
	NRF_LOG_INFO("ble bank_application image_size --> %d",g_ble_boot_setting.bank_application.image_size);
	NRF_LOG_INFO("ble bank_application image_crc --> 0x%08x",g_ble_boot_setting.bank_application.image_crc);
	NRF_LOG_INFO("ble bank_application version --> 0x%08x",g_ble_boot_setting.bank_application.version);
	NRF_LOG_INFO("ble bank_application command --> 0x%08x",g_ble_boot_setting.bank_application.command);
	NRF_LOG_INFO("ble bank_softdevice extFlash_address --> 0x%08x",g_ble_boot_setting.bank_softdevice.extFlash_address);
	NRF_LOG_INFO("ble bank_softdevice image_size --> %d",g_ble_boot_setting.bank_softdevice.image_size);
	NRF_LOG_INFO("ble bank_softdevice image_crc --> 0x%08x",g_ble_boot_setting.bank_softdevice.image_crc);
	NRF_LOG_INFO("ble bank_softdevice version --> 0x%08x",g_ble_boot_setting.bank_softdevice.version);
	NRF_LOG_INFO("ble bank_softdevice command --> 0x%08x",g_ble_boot_setting.bank_softdevice.command);


	uint32_t crc_dest = am_bootloader_fast_crc32(&g_ble_boot_setting,sizeof(_ble_boot_setting));
	NRF_LOG_INFO("crc_dest --> 0x%08x",crc_dest);	
	
	uint8_t command[180] = {CMD_APOLLO_SRV_FLASH_DFU_BLE_BOOTSETTING_RESPONSE};
	
	if(crc_src == crc_dest)
	{
		command[1] = 0x00;
	}
	else
	{
		command[1] = 0x01;//传输数据CRC错误
	}
	app_srv_apollo_flash_send(command, sizeof(command));	
}
#define PREVALIDATE_ERROR_MASK_DEFAULT                (0)
#define PREVALIDATE_ERROR_MASK_BOOT_SETTING_CRC       (1UL << 0)
#define PREVALIDATE_ERROR_MASK_PCB_VERSION            (1UL << 1)
#define PREVALIDATE_ERROR_MASK_BANK_BOOT_EXT_ADDRESS  (1UL << 2)
#define PREVALIDATE_ERROR_MASK_BANK_BOOT_INT_ADDRESS  (1UL << 3)
#define PREVALIDATE_ERROR_MASK_BANK_BOOT_SIZE         (1UL << 4)
#define PREVALIDATE_ERROR_MASK_BANK_BOOT_CRC          (1UL << 5)
#define PREVALIDATE_ERROR_MASK_BANK_BOOT_VERSION      (1UL << 6)
#define PREVALIDATE_ERROR_MASK_BANK_APP_EXT_ADDRESS   (1UL << 7)
#define PREVALIDATE_ERROR_MASK_BANK_APP_INT_ADDRESS   (1UL << 8)
#define PREVALIDATE_ERROR_MASK_BANK_APP_SIZE          (1UL << 9)
#define PREVALIDATE_ERROR_MASK_BANK_APP_CRC           (1UL << 10)
#define PREVALIDATE_ERROR_MASK_BANK_APP_VERSION       (1UL << 11)
#define PREVALIDATE_ERROR_MASK_BANK_UPDATE_EXT_ADDRESS  (1UL << 12)
#define PREVALIDATE_ERROR_MASK_BANK_UPDATE_INT_ADDRESS  (1UL << 13)
#define PREVALIDATE_ERROR_MASK_BANK_UPDATE_SIZE         (1UL << 14)
#define PREVALIDATE_ERROR_MASK_BANK_UPDATE_CRC          (1UL << 15)
#define PREVALIDATE_ERROR_MASK_BANK_UPDATE_VERSION      (1UL << 16)
#define PREVALIDATE_ERROR_MASK_ENTER_BOOTLOADER_REASON  (1UL << 17)

#define PREVALIDATE_ERROR_MASK_BANK_SOFTDEVICE_EXT_ADDRESS  (1UL << 12)
#define PREVALIDATE_ERROR_MASK_BANK_SOFTDEVICE_SIZE         (1UL << 14)
#define PREVALIDATE_ERROR_MASK_BANK_SOFTDEVICE_CRC          (1UL << 15)
#define PREVALIDATE_ERROR_MASK_BANK_SOFTDEVICE_VERSION      (1UL << 16)
typedef uint32_t _prevalidate_error_mask;





#define BLE_BANK_BOOTLOADER_DEFAULT_SIZE        0x10000
#define BLE_BANK_APPLICATION_DEFAULT_SIZE       0x38000
#define BLE_BANK_SOFTDEVICE_DEFAULT_SIZE        0x22000

#define EXTFLASH_MAX_SIZE        0x800000

typedef struct __attribute__((packed)){
	uint8_t command;
	_prevalidate_error_mask error_mask;
}_dev_dfu_prevalidate;
#include "nrf_dfu_types.h"
static void cmd_apollo_srv_flash_dfu_ble_prevalidate(ble_apollo_evt_data_t *data)
{
	_dev_dfu_prevalidate prevalidate;
	prevalidate.command = CMD_APOLLO_SRV_FLASH_DFU_BLE_PREVALIDATE_RESPONSE;
	_prevalidate_error_mask error_mask = PREVALIDATE_ERROR_MASK_DEFAULT;
	NRF_LOG_INFO("boot_setting_crc --> 0x%08x",g_ble_boot_setting.boot_setting_crc);
	uint32_t crc_dest = am_bootloader_fast_crc32((uint8_t*)(&g_ble_boot_setting.boot_setting_crc)+sizeof(g_ble_boot_setting.boot_setting_crc),sizeof(_ble_boot_setting) - sizeof(g_ble_boot_setting.boot_setting_crc));
	NRF_LOG_INFO("crc_dest --> 0x%08x",crc_dest);		
	
	if(g_ble_boot_setting.boot_setting_crc == crc_dest)
	{
		nrf_dfu_settings_t s_dfu_settings;	
		memcpy((void*)&s_dfu_settings, (uint8_t *)BOOTLOADER_SETTINGS_ADDRESS, sizeof(nrf_dfu_settings_t));			
		if(g_ble_boot_setting.bank_bootloader.command == BOOT_SETTING_BANK_COMMAND_NEW)
		{
			if(g_ble_boot_setting.bank_bootloader.extFlash_address > EXTFLASH_MAX_SIZE)
			{
				NRF_LOG_INFO("PREVALIDATE_ERROR_MASK_BANK_BOOT_EXT_ADDRESS");
				error_mask |= PREVALIDATE_ERROR_MASK_BANK_BOOT_EXT_ADDRESS;
			}			
			if(g_ble_boot_setting.bank_bootloader.image_size > BLE_BANK_BOOTLOADER_DEFAULT_SIZE)
			{
				NRF_LOG_INFO("ble bank_bootloader image_size --> %d",g_ble_boot_setting.bank_bootloader.image_size);
				NRF_LOG_INFO("BLE_BANK_BOOTLOADER_DEFAULT_SIZE --> %d",BLE_BANK_BOOTLOADER_DEFAULT_SIZE);
				NRF_LOG_INFO("PREVALIDATE_ERROR_MASK_BANK_BOOT_SIZE");
				error_mask |= PREVALIDATE_ERROR_MASK_BANK_BOOT_SIZE;
			}
			if(g_ble_boot_setting.bank_bootloader.version < s_dfu_settings.bootloader_version)
			{
				NRF_LOG_INFO("PREVALIDATE_ERROR_MASK_BANK_BOOT_VERSION");
				error_mask |= PREVALIDATE_ERROR_MASK_BANK_BOOT_VERSION;
			}		
		}
		if(g_ble_boot_setting.bank_application.command == BOOT_SETTING_BANK_COMMAND_NEW)
		{
			if(g_ble_boot_setting.bank_application.extFlash_address > EXTFLASH_MAX_SIZE)
			{
				NRF_LOG_INFO("PREVALIDATE_ERROR_MASK_BANK_APP_EXT_ADDRESS");
				error_mask |= PREVALIDATE_ERROR_MASK_BANK_APP_EXT_ADDRESS;
			}	
			if(g_ble_boot_setting.bank_application.image_size > BLE_BANK_APPLICATION_DEFAULT_SIZE)
			{
				NRF_LOG_INFO("PREVALIDATE_ERROR_MASK_BANK_APP_SIZE");
				error_mask |= PREVALIDATE_ERROR_MASK_BANK_APP_SIZE;
			}	
			if(g_ble_boot_setting.bank_application.version < s_dfu_settings.app_version)
			{
				NRF_LOG_INFO("PREVALIDATE_ERROR_MASK_BANK_APP_VERSION");
				error_mask |= PREVALIDATE_ERROR_MASK_BANK_APP_VERSION;
			}
		}	
		if(g_ble_boot_setting.bank_softdevice.command == BOOT_SETTING_BANK_COMMAND_NEW)
		{
			if(g_ble_boot_setting.bank_softdevice.extFlash_address > EXTFLASH_MAX_SIZE)
			{
				NRF_LOG_INFO("PREVALIDATE_ERROR_MASK_BANK_SOFTDEVICE_EXT_ADDRESS");
				error_mask |= PREVALIDATE_ERROR_MASK_BANK_SOFTDEVICE_EXT_ADDRESS;
			}	
			if(g_ble_boot_setting.bank_softdevice.image_size > BLE_BANK_SOFTDEVICE_DEFAULT_SIZE)
			{
				NRF_LOG_INFO("PREVALIDATE_ERROR_MASK_BANK_SOFTDEVICE_SIZE");
				error_mask |= PREVALIDATE_ERROR_MASK_BANK_SOFTDEVICE_SIZE;
			}	
			if(g_ble_boot_setting.bank_softdevice.version < SD_FWID_GET(MBR_SIZE))
			{
				NRF_LOG_INFO("PREVALIDATE_ERROR_MASK_BANK_SOFTDEVICE_VERSION");
				error_mask |= PREVALIDATE_ERROR_MASK_BANK_SOFTDEVICE_VERSION;
			}
		}				
	}
	else
	{
		error_mask |= PREVALIDATE_ERROR_MASK_BOOT_SETTING_CRC;//数据CRC错误
	}
	prevalidate.error_mask = error_mask;
	
	if(prevalidate.error_mask == PREVALIDATE_ERROR_MASK_DEFAULT)
	{
		task_extFlash_handle(TASK_EXTFLASH_EVT_BLE_DFU_BLE_PREVALIDATE, 0, NULL, 0);
	}else{
		app_srv_apollo_flash_send((uint8_t *)&prevalidate, sizeof(_dev_dfu_prevalidate));	
	}	
}

static void cmd_apollo_srv_flash_dfu_ble_postvalidate(ble_apollo_evt_data_t *data)
{
	task_extFlash_handle(TASK_EXTFLASH_EVT_BLE_DFU_BLE_POSTVALIDATE, 0, (uint8_t *)(data->p_data), data->length);
}



static void cmd_apollo_srv_flash_dfu_ble_reset_N_activate(ble_apollo_evt_data_t *data)
{
	uint8_t response = CMD_APOLLO_SRV_FLASH_DFU_BLE_RESET_N_ACTIVATE_RESPONSE;
	app_srv_apollo_flash_send(&response, sizeof(response));
	m_reset_type = SRV_APOLLO_RESET_TYPE_JUMP_BOOTLOADER;	
	NRF_LOG_DEBUG("SRV_APOLLO_RESET_TYPE_JUMP_BOOTLOADER");
	timer_start(500);	
}
static void cmd_apollo_srv_flash_dfu_ble_dfu_finish(ble_apollo_evt_data_t *data)
{
	uint32_t command[128] = {BLE_UART_CCOMMAND_APP_BLE_DFU_FINISH | BLE_UART_CCOMMAND_MASK_APP};
	memcpy((uint8_t *)(command+1),data->p_data+1,sizeof(command)-1);		
	task_ble_conn_handle(TASK_BLE_CONN_EVT_BLE_UART, (uint8_t *)(&command), sizeof(command));
}

_apollo_boot_setting g_apollo_boot_setting;
static void cmd_apollo_srv_flash_dfu_ble_dfu_apollo_bootsetting(ble_apollo_evt_data_t *data)
{
	uint32_t crc_src = *(uint32_t *)((uint8_t *)data->p_data+1);
	g_apollo_boot_setting = *(_apollo_boot_setting *)((uint8_t *)data->p_data+5);	
	
#if 0	
	NRF_LOG_INFO("boot_setting_crc --> 0x%08x",g_apollo_boot_setting.boot_setting_crc);
	NRF_LOG_INFO("boot_setting_size --> %d",g_apollo_boot_setting.boot_setting_size);
	NRF_LOG_INFO("bootloader_version --> 0x%08x",g_apollo_boot_setting.bootloader_version);
	NRF_LOG_INFO("application_version --> 0x%08x",g_apollo_boot_setting.application_version);
	NRF_LOG_INFO("pcb_version --> 0x%08x",g_apollo_boot_setting.pcb_version);
	NRF_LOG_INFO("bank_bootloader extFlash_address --> 0x%08x",g_apollo_boot_setting.bank_bootloader.extFlash_address);
	NRF_LOG_INFO("bank_bootloader intFlash_address --> 0x%08x",g_apollo_boot_setting.bank_bootloader.intFlash_address);
	NRF_LOG_INFO("bank_bootloader image_size --> %d",g_apollo_boot_setting.bank_bootloader.image_size);
	NRF_LOG_INFO("bank_bootloader image_crc --> 0x%08x",g_apollo_boot_setting.bank_bootloader.image_crc);
	NRF_LOG_INFO("bank_bootloader version --> 0x%08x",g_apollo_boot_setting.bank_bootloader.version);
	NRF_LOG_INFO("bank_bootloader command --> 0x%08x",g_apollo_boot_setting.bank_bootloader.command);
	NRF_LOG_INFO("bank_application extFlash_address --> 0x%08x",g_apollo_boot_setting.bank_application.extFlash_address);
	NRF_LOG_INFO("bank_application intFlash_address --> 0x%08x",g_apollo_boot_setting.bank_application.intFlash_address);
	NRF_LOG_INFO("bank_application image_size --> %d",g_apollo_boot_setting.bank_application.image_size);
	NRF_LOG_INFO("bank_application image_crc --> 0x%08x",g_apollo_boot_setting.bank_application.image_crc);
	NRF_LOG_INFO("bank_application version --> 0x%08x",g_apollo_boot_setting.bank_application.version);
	NRF_LOG_INFO("bank_application command --> 0x%08x",g_apollo_boot_setting.bank_application.command);
	NRF_LOG_INFO("bank_update_algorithm extFlash_address --> 0x%08x",g_apollo_boot_setting.bank_update_algorithm.extFlash_address);
	NRF_LOG_INFO("bank_update_algorithm intFlash_address --> 0x%08x",g_apollo_boot_setting.bank_update_algorithm.intFlash_address);
	NRF_LOG_INFO("bank_update_algorithm image_size --> %d",g_apollo_boot_setting.bank_update_algorithm.image_size);
	NRF_LOG_INFO("bank_update_algorithm image_crc --> 0x%08x",g_apollo_boot_setting.bank_update_algorithm.image_crc);
	NRF_LOG_INFO("bank_update_algorithm version --> 0x%08x",g_apollo_boot_setting.bank_update_algorithm.version);
	NRF_LOG_INFO("bank_update_algorithm command --> 0x%08x",g_apollo_boot_setting.bank_update_algorithm.command);
	NRF_LOG_INFO("enter_bootloader_reason --> %d",g_apollo_boot_setting.enter_bootloader_reason);				
#endif	
	
	uint32_t crc_dest = am_bootloader_fast_crc32(&g_apollo_boot_setting,sizeof(_apollo_boot_setting));
	NRF_LOG_INFO("crc_dest --> 0x%08x",crc_dest);	
	NRF_LOG_INFO("crc_src --> 0x%08x",crc_src);		
	
	uint8_t command[2] = {CMD_APOLLO_SRV_FLASH_DFU_BLE_DFU_APOLLO_BOOTSETTING_RESPONSE};
	if(crc_dest == crc_src)
	{
		command[1] = 0;
	}
	else
	{
		command[1] = 1;//BOOTSETTING CRC 错误
	}	
	
	app_srv_apollo_flash_send((uint8_t *)command, sizeof(command));	
	
}

#define APOLLO2_FLASH_SIZE       0x100000

#define APOLLO2_BOOTLOADER_START_DEFAULT_ADDRESS                      (0x00000000UL)
#define APOLLO2_APPLICATION_START_DEFAULT_ADDRESS                     (0x00010000UL)
#define APOLLO2_BOOTLOADER_DEFAULT_SIZE                               (APOLLO2_APPLICATION_START_DEFAULT_ADDRESS - APOLLO2_BOOTLOADER_START_DEFAULT_ADDRESS)
#define APOLLO2_APP_DATA_ADDRESS                                      (0x000F8000UL)
#define APOLLO2_APPLICATION_DEFAULT_SIZE                              (APOLLO2_APP_DATA_ADDRESS - APOLLO2_APPLICATION_START_DEFAULT_ADDRESS)
#define APOLLO2_BOOT_SETTING_ADDRESS                                  (0x000FA000UL)
#define APOLLO2_UPDATE_ALGORITHM_START_DEFAULT_ADDRESS                (0x000FC000UL)
#define APOLLO2_UPDATE_ALGORITHM_DEFAULT_SIZE                         (APOLLO2_FLASH_SIZE - APOLLO2_UPDATE_ALGORITHM_START_DEFAULT_ADDRESS)
extern _info_apollo2 g_info_apollo2;
static void cmd_apollo_srv_flash_dfu_ble_dfu_apollo_prevalidate(ble_apollo_evt_data_t *data)
{
	_dev_dfu_prevalidate prevalidate;
	prevalidate.command = CMD_APOLLO_SRV_FLASH_DFU_BLE_DFU_APOLLO_PREVALIDATE_RESPONSE;
	_prevalidate_error_mask error_mask = PREVALIDATE_ERROR_MASK_DEFAULT;
	NRF_LOG_INFO("boot_setting_crc --> 0x%08x",g_apollo_boot_setting.boot_setting_crc);
	uint32_t crc_dest = am_bootloader_fast_crc32((uint8_t*)(&g_apollo_boot_setting.boot_setting_crc)+sizeof(g_apollo_boot_setting.boot_setting_crc),sizeof(_apollo_boot_setting) - sizeof(g_apollo_boot_setting.boot_setting_crc));
	NRF_LOG_INFO("crc_dest --> 0x%08x",crc_dest);		
	
	if(g_apollo_boot_setting.boot_setting_crc == crc_dest)
	{
		
		if(g_apollo_boot_setting.bank_bootloader.command == BOOT_SETTING_BANK_COMMAND_NEW)
		{
			if(g_apollo_boot_setting.bank_bootloader.extFlash_address > EXTFLASH_MAX_SIZE)
			{
				error_mask |= PREVALIDATE_ERROR_MASK_BANK_BOOT_EXT_ADDRESS;
			}			
			if(g_apollo_boot_setting.bank_bootloader.intFlash_address != 0)
			{
				error_mask |= PREVALIDATE_ERROR_MASK_BANK_BOOT_INT_ADDRESS;
			}		
			if(g_apollo_boot_setting.bank_bootloader.image_size > APOLLO2_BOOTLOADER_DEFAULT_SIZE)
			{
				error_mask |= PREVALIDATE_ERROR_MASK_BANK_BOOT_SIZE;
			}	
			if(g_apollo_boot_setting.bank_bootloader.version < g_info_apollo2.bootloader_version)
			{
				error_mask |= PREVALIDATE_ERROR_MASK_BANK_BOOT_VERSION;
			}
		}
		if(g_apollo_boot_setting.bank_application.command == BOOT_SETTING_BANK_COMMAND_NEW)
		{
			if(g_apollo_boot_setting.bank_application.extFlash_address > EXTFLASH_MAX_SIZE)
			{
				error_mask |= PREVALIDATE_ERROR_MASK_BANK_APP_EXT_ADDRESS;
			}		
			if(g_apollo_boot_setting.bank_application.intFlash_address == 0)
			{
				error_mask |= PREVALIDATE_ERROR_MASK_BANK_APP_INT_ADDRESS;
			}		
			if(g_apollo_boot_setting.bank_application.image_size > APOLLO2_APPLICATION_DEFAULT_SIZE)
			{
				error_mask |= PREVALIDATE_ERROR_MASK_BANK_APP_SIZE;
			}	
			if(g_apollo_boot_setting.bank_application.version < g_info_apollo2.application_version)
			{
				error_mask |= PREVALIDATE_ERROR_MASK_BANK_APP_VERSION;
			}
		}	
		if(g_apollo_boot_setting.bank_update_algorithm.command == BOOT_SETTING_BANK_COMMAND_NEW)
		{
			if(g_apollo_boot_setting.bank_update_algorithm.extFlash_address > EXTFLASH_MAX_SIZE)
			{
				error_mask |= PREVALIDATE_ERROR_MASK_BANK_UPDATE_EXT_ADDRESS;
			}		
			if(g_apollo_boot_setting.bank_update_algorithm.intFlash_address != APOLLO2_UPDATE_ALGORITHM_START_DEFAULT_ADDRESS)
			{
				error_mask |= PREVALIDATE_ERROR_MASK_BANK_UPDATE_INT_ADDRESS;
			}		
			if(g_apollo_boot_setting.bank_update_algorithm.image_size > APOLLO2_UPDATE_ALGORITHM_DEFAULT_SIZE)
			{
				error_mask |= PREVALIDATE_ERROR_MASK_BANK_UPDATE_SIZE;
			}	
			if(g_apollo_boot_setting.bank_update_algorithm.version < g_info_apollo2.update_version)
			{
				error_mask |= PREVALIDATE_ERROR_MASK_BANK_UPDATE_VERSION;
			}
		}	
		if(g_apollo_boot_setting.enter_bootloader_reason != BOOT_SETTING_BANK_COMMAND_DEFAULT)
		{
			error_mask |= PREVALIDATE_ERROR_MASK_ENTER_BOOTLOADER_REASON;
		}		
	}
	else
	{
		error_mask |= PREVALIDATE_ERROR_MASK_BOOT_SETTING_CRC;//数据CRC错误
	}
	prevalidate.error_mask = error_mask;
	if(prevalidate.error_mask == PREVALIDATE_ERROR_MASK_DEFAULT)
	{
		task_extFlash_handle(TASK_EXTFLASH_EVT_BLE_DFU_APOLLO_PREVALIDATE, 0, NULL, 0);
	}else{
		app_srv_apollo_flash_send((uint8_t *)&prevalidate, sizeof(_dev_dfu_prevalidate));
	}
		
}
static void cmd_apollo_srv_flash_dfu_ble_dfu_apollo_postvalidate(ble_apollo_evt_data_t *data)
{
	_apollo_srv_flash_ble_dfu_postvalidate *cmd = (_apollo_srv_flash_ble_dfu_postvalidate *)(data->p_data);	 
	NRF_LOG_INFO("cmd->address --> 0x%08X",cmd->address);
	NRF_LOG_INFO("cmd->size --> 0x%08X",cmd->size);		
	NRF_LOG_INFO("cmd->crc --> 0x%08X",cmd->crc);
	
	uint8_t response[5];
	response[0] = CMD_APOLLO_SRV_FLASH_DFU_BLE_DFU_APOLLO_POSTVALIDATE_RESPONSE;
	uint32_t error = 0;	
	response[1] = (uint8_t)(error>>0);
	response[2] = (uint8_t)(error>>8);
	response[3] = (uint8_t)(error>>16);
	response[4] = (uint8_t)(error>>24);	
	app_srv_apollo_flash_send((uint8_t *)&response, sizeof(response));

	
	//task_extFlash_handle(TASK_EXTFLASH_EVT_BLE_DFU_APOLLO_POSTVALIDATE, 0, (uint8_t *)(data->p_data), data->length);
}
static void cmd_apollo_srv_flash_dfu_ble_dfu_apollo_reset_n_activate(ble_apollo_evt_data_t *data)
{
	uint32_t command[128] = {BLE_UART_CCOMMAND_APP_BLE_DFU_APOLLO_RESET_N_ACTIVATE | BLE_UART_CCOMMAND_MASK_APP};
	memcpy((uint8_t *)(command+1),data->p_data+1,sizeof(command)-1);		
	task_ble_conn_handle(TASK_BLE_CONN_EVT_BLE_UART, (uint8_t *)(&command), sizeof(command));
}

static void cmd_apollo_srv_flash_dfu_ble_watch_face_info(ble_apollo_evt_data_t *data)
{
	uint32_t command[128] = {BLE_UART_CCOMMAND_APPLICATION_WATCH_FACE_INFO | BLE_UART_CCOMMAND_MASK_APPLICATION};
	memcpy((uint8_t *)(command+1),data->p_data+1,sizeof(command)-1);		
	task_ble_conn_handle(TASK_BLE_CONN_EVT_BLE_UART, (uint8_t *)(&command), sizeof(command));
}
static void cmd_apollo_srv_flash_dfu_ble_watch_face_info_2(ble_apollo_evt_data_t *data)
{
	uint32_t command[128] = {BLE_UART_CCOMMAND_APPLICATION_WATCH_FACE_INFO_2 | BLE_UART_CCOMMAND_MASK_APPLICATION};
	memcpy((uint8_t *)(command+1),data->p_data+1,sizeof(command)-1);		
	task_ble_conn_handle(TASK_BLE_CONN_EVT_BLE_UART, (uint8_t *)(&command), sizeof(command));
}
static void cmd_apollo_srv_flash_dfu_ble_watch_face_finish(ble_apollo_evt_data_t *data)
{
	uint32_t command[128] = {BLE_UART_CCOMMAND_APPLICATION_WATCH_FACE_FINISH | BLE_UART_CCOMMAND_MASK_APPLICATION};
	memcpy((uint8_t *)(command+1),data->p_data+1,sizeof(command)-1);		
	task_ble_conn_handle(TASK_BLE_CONN_EVT_BLE_UART, (uint8_t *)(&command), sizeof(command));
}
static void cmd_apollo_srv_flash_dfu_ble_watch_face_activate(ble_apollo_evt_data_t *data)
{
	uint32_t command[128] = {BLE_UART_CCOMMAND_APPLICATION_WATCH_FACE_ACTIVATE | BLE_UART_CCOMMAND_MASK_APPLICATION};
	memcpy((uint8_t *)(command+1),data->p_data+1,sizeof(command)-1);		
	task_ble_conn_handle(TASK_BLE_CONN_EVT_BLE_UART, (uint8_t *)(&command), sizeof(command));
}



static void apollo_sensor_data_upload_command(ble_apollo_evt_data_t *data)
{
	uint8_t start =0 ;
	uint32_t command[128] = {BLE_UART_CCOMAND_APPLICATION_SENSOR_DATA_UPLOAD | BLE_UART_CCOMMAND_MASK_APPLICATION};
	memcpy((uint8_t *)(command+1),data->p_data+1,sizeof(command)-1);		
	task_ble_conn_handle(TASK_BLE_CONN_EVT_BLE_UART, (uint8_t *)(&command), sizeof(command));

	start = data->p_data[2];
	
	NRF_LOG_INFO("capollo_sensor_data_upload_command,%x,%x,%x", data->p_data[0], data->p_data[1] ,data->p_data[2]);
	DELAY_MS(10);
	
	if (start)
	{
		ble_conn_params_set_conn_params(BLE_ADV_EVT_FAST);
	}
	else
	{
		ble_conn_params_set_conn_params(BLE_ADV_EVT_SLOW);
	}
}



