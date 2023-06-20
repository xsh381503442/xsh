#include "task_ble_conn.h"
#include "ble_srv_common.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "portmacro.h"
#include "portable.h"
#include "event_groups.h"
#include "semphr.h"

#include "drv_apollo2.h"
#include "cmd_apollo2.h"
#include "nrf_delay.h"
#include "app_srv_nrf52.h"
#include "cmd_nrf52_srv.h"
#include "app_srv_nrf52.h"
#include "ble_db_discovery.h"
#include "nrf_sdh_ble.h"
#include "app_srv_apollo.h"
#include "ble_conn_params.h"
#include "drv_ble_flash.h"
#include "ble_srv_nrf52.h"
#include "nrf_ble_ancs_c.h"
#include "ble_advertising.h"
#include "app_srv_ancs.h"
#include "am_bootloader.h"
#include "task_ble_accessary.h"

#include "drv_apollo2.h"
#include "dev_apollo2.h"
#include "cmd_apollo2.h"
#ifdef COD 
#include "cod_ble_api.h"
#include "app_srv_cod.h"
#endif
#include "peer_manager.h"


#define LOG_LEVEL_OFF 							0
#define LOG_LEVEL_ERROR 						1
#define LOG_LEVEL_WARNING 						2
#define LOG_LEVEL_INFO 							3
#define LOG_LEVEL_DEBUG 						4
#define NRF_LOG_MODULE_NAME 					task_ble_conn
#define NRF_LOG_LEVEL       					LOG_LEVEL_INFO
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
NRF_LOG_MODULE_REGISTER();


//#define DELAY_MS(X)                 nrf_delay_ms(X)
#define DELAY_MS(X)                 vTaskDelay(X)
//#define DELAY_MS(X)                   do{TickType_t xLastWakeTime;xLastWakeTime = xTaskGetTickCount ();vTaskDelayUntil( &xLastWakeTime, X );}while(0)

extern uint16_t g_conn_handle_peripheral;
//BLE_DB_DISCOVERY_DEF(m_db_disc);



BLE_DB_DISCOVERY_ARRAY_DEF(m_db_disc, DISCOVERY_ARRAY_SIZE);  /**< Database discovery module instances. */



static TaskHandle_t task_handle;
extern ble_gap_addr_t  g_peer_addr;
extern ble_config_params_t     *g_ble_config;
extern ble_ancs_c_t m_ancs_c;;
extern ble_ancs_c_evt_notif_t g_notification_latest;
extern ble_ancs_c_attr_t      g_notif_attr_latest;
extern ble_ancs_c_attr_t      g_notif_attr_app_id_latest;
extern _task_ble_accessory_data g_task_ble_accessory_data;

#define USE_QUEUE	

#ifdef COD
extern cod_ble_dev_param usr_dev_param;
extern cod_mtu_size cod_usr_ble_tx_mtu;
#endif
uint8_t task_ble_conn_evt_connect_flag = 0;
#ifdef USE_QUEUE
static QueueHandle_t QueueTask = NULL;
struct s_queue_task{
	uint32_t	evt;
	uint32_t length;
	uint8_t	*p_data;
	
};
typedef struct s_queue_task _queue_task;
#endif

#ifdef USE_NOTIFY
typedef struct{
	uint8_t	evt;
	uint8_t *p_data;
	uint32_t length;
} _task_buffer;
static _task_buffer m_task_buffer = {0,NULL,0};
#endif

#ifdef USE_QUEUE	
static _queue_task queue = {0,NULL,0};
#endif


typedef struct __attribute__ ((packed))
{
	uint32_t command;
	uint8_t name[BLE_CONFIG_DEVICE_NAME_LEN_MAX+1];
	uint8_t len;	
} _task_ble_conn_dev_name_update;

typedef struct __attribute__ ((packed))
{
	uint32_t command;
	uint8_t addr[BLE_GAP_ADDR_LEN];	
} _task_ble_conn_evt;
typedef struct __attribute__ ((packed))
{
	uint32_t command;
	uint8_t disconnect_reason;

} _task_ble_conn_evt_disconnect;


typedef struct __attribute__((packed))
{
	uint32_t command;
	uint8_t mac[6];
}_ble_accessory_conn_status;
extern _accessory_running g_accessory_running;
extern _accessory_heartrate g_accessory_heartrate;
extern _accessory_cycling g_accessory_cycling;


static void task_ble_conn_evt_connect(void);
static void task_ble_conn_evt_disconnect(void);
static void task_ble_conn_param_update(void);
static void task_ble_conn_evt_bond(void);
static void task_ble_conn_evt_ancs_disc_complete(void);
static void task_ble_conn_evt_bond_android(void);
static void task_ble_conn_evt_passkey(void);
static void task_ble_conn_evt_ble_uart(void);
static void task_ble_conn_hwt_rssi_read(void);
static void task_ble_conn_dev_name_update(void);
static void task_ble_conn_ancs_attr_request(void);
static void task_ble_conn_ancs_attr_id_app_identify(void);
static void task_ble_conn_ancs_message_send(void);
static void task_ble_conn_wechat_sport_data_request(void);	
static void task_ble_conn_evt_connect_heartrate(void);
static void task_ble_conn_evt_connect_running(void);	
static void task_ble_conn_evt_connect_cycling(void);	
static void task_ble_conn_evt_disconnect_heartrate(void);	
static void task_ble_conn_evt_disconnect_running(void);	
static void task_ble_conn_evt_disconnect_cycling(void);	
static void task_ble_conn_evt_accessory_data_upload(void);
static void task_ble_conn_evt_pairing_failed(void);
#ifdef COD
static void task_ble_conn_evt_cod_cmd_device_mac_resp(void);
static void task_ble_conn_cod_message_send(void);
#endif



#define TIMER_INTERVAL            5000 
#define OSTIMER_WAIT_FOR_QUEUE              100 
static TimerHandle_t m_timer; 

#define TIMER_EVT_STORE_BLE_CONFIG_FILE         1
static uint8_t m_timer_evt = 0;


static void timeout_handler(TimerHandle_t xTimer)
{
    UNUSED_PARAMETER(xTimer);
    NRF_LOG_DEBUG("timeout_handler");
	if (pdPASS != xTimerStop(m_timer, OSTIMER_WAIT_FOR_QUEUE))
	{
			APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
	}	

	if(m_timer_evt == TIMER_EVT_STORE_BLE_CONFIG_FILE)
	{
			/**<Store the name to flash and send to apollo>*/
			uint32_t err_code = drv_ble_flash_config_store(g_ble_config);
			APP_ERROR_CHECK(err_code);		
	}	
}


static void timer_create(void)
{
	m_timer = xTimerCreate("CONN",
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



void task_ble_conn_handle(uint32_t evt, uint8_t *p_data, uint32_t length)
{
	//NRF_LOG_INFO("task_ble_conn_handle");
#ifdef USE_NOTIFY	
	m_task_buffer.evt = evt;
	m_task_buffer.p_data = p_data;
	m_task_buffer.length = length;
//	BaseType_t yield_req = pdFALSE;
//	vTaskNotifyGiveFromISR(task_handle, &yield_req);
//	/* Switch the task if required. */
//	portYIELD_FROM_ISR(yield_req);
	BaseType_t xReturned = xTaskNotifyGive(task_handle);
	if (xReturned != pdPASS)
	{
			APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
	}	
#endif	
#ifdef USE_QUEUE	
	_queue_task  queue = {0};
	queue.evt = evt;
	queue.p_data = p_data;
	queue.length = length;
	BaseType_t xReturned = xQueueSend(QueueTask, &queue, portMAX_DELAY );	
	if (xReturned != pdPASS)
	{
			APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
	}	
#endif	
}


static void task(void * pvParameter)
{
    NRF_LOG_DEBUG("task");
		(void)pvParameter;
		#ifdef COD 
	   cod_sdk_init();
	   #endif
		while (1)
		{
			#ifdef USE_NOTIFY				
			(void) ulTaskNotifyTake(pdTRUE,/* Clear the notification value before exiting (equivalent to the binary semaphore). */
                                portMAX_DELAY); /* Block indefinitely (INCLUDE_vTaskSuspend has to be enabled).*/
			#endif	
			#ifdef USE_QUEUE	
			if (xQueueReceive(QueueTask, &queue, portMAX_DELAY) == pdTRUE)
			{
			#endif	
			#ifdef USE_NOTIFY		
				switch(m_task_buffer.evt)
			#endif	
			#ifdef USE_QUEUE	
				switch(queue.evt)
			#endif			
				{
					case TASK_BLE_CONN_EVT_CONNECT:{
						NRF_LOG_INFO("TASK_BLE_CONN_EVT_CONNECT");
						if (task_ble_conn_evt_connect_flag == 0)
						{
							task_ble_conn_evt_connect();
						}
					}break;
					case TASK_BLE_CONN_EVT_DISCONNECT:{
						NRF_LOG_INFO("TASK_BLE_CONN_EVT_DISCONNECT");
						if (task_ble_conn_evt_connect_flag == 1)
						{
							task_ble_conn_evt_disconnect();	
						}
					}break;
					case TASK_BLE_CONN_PARAM_UPDATE:{
						NRF_LOG_INFO("TASK_BLE_CONN_PARAM_UPDATE");
						task_ble_conn_param_update();
					}break;					
					case TASK_BLE_CONN_EVT_BOND:{
						NRF_LOG_INFO("TASK_BLE_CONN_EVT_BOND");
						task_ble_conn_evt_bond();
					}break;
					case TASK_BLE_CONN_EVT_ANCS_DISC_COMPLETE:{
						NRF_LOG_INFO("TASK_BLE_CONN_EVT_ANCS_DISC_COMPLETE");
						task_ble_conn_evt_ancs_disc_complete();
					}break;				
					case TASK_BLE_CONN_EVT_BOND_ANDROID:{
						NRF_LOG_INFO("TASK_BLE_CONN_EVT_BOND_ANDROID");	
						task_ble_conn_evt_bond_android();
					}break;					
					case TASK_BLE_CONN_EVT_PASSKEY:{
						NRF_LOG_INFO("TASK_BLE_CONN_EVT_PASSKEY");
						task_ble_conn_evt_passkey();
					}break;					
					case TASK_BLE_CONN_EVT_BLE_UART:{
						NRF_LOG_INFO("TASK_BLE_CONN_EVT_BLE_UART");
						//NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
						task_ble_conn_evt_ble_uart();		
						//NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
					}break;		
					case TASK_BLE_CONN_HWT_RSSI_READ:{
						task_ble_conn_hwt_rssi_read();
					}break;
					case TASK_BLE_CONN_DEV_NAME_UPDATE:{
						NRF_LOG_INFO("TASK_BLE_CONN_DEV_NAME_UPDATE");
						task_ble_conn_dev_name_update();
					}break;					
					case TASK_BLE_CONN_ANCS_ATTR_REQUEST:{
						NRF_LOG_INFO("TASK_BLE_CONN_ANCS_ATTR_REQUEST");
						task_ble_conn_ancs_attr_request();
					}break;	
					case TASK_BLE_CONN_ANCS_ATTR_ID_APP_IDENTIFY:{
						NRF_LOG_INFO("TASK_BLE_CONN_ANCS_ATTR_ID_APP_IDENTIFY");
						task_ble_conn_ancs_attr_id_app_identify();
					}break;	
					case TASK_BLE_CONN_ANCS_MESSAGE_SEND:{
						NRF_LOG_INFO("TASK_BLE_CONN_ANCS_MESSAGE_SEND");
						task_ble_conn_ancs_message_send();
					}break;	
					case TASK_BLE_CONN_WECHAT_SPORT_DATA_REQUEST:{
						NRF_LOG_INFO("TASK_BLE_CONN_WECHAT_SPORT_DATA_REQUEST");
						task_ble_conn_wechat_sport_data_request();
					}break;						
					case TASK_BLE_CONN_EVT_CONNECT_HEARTRATE:{
						NRF_LOG_INFO("TASK_BLE_CONN_EVT_CONNECT_HEARTRATE");
						task_ble_conn_evt_connect_heartrate();
					}break;							
					case TASK_BLE_CONN_EVT_CONNECT_RUNNING:{
						NRF_LOG_INFO("TASK_BLE_CONN_EVT_CONNECT_RUNNING");
						task_ble_conn_evt_connect_running();
					}break;	
					case TASK_BLE_CONN_EVT_CONNECT_CYCLING:{
						NRF_LOG_INFO("TASK_BLE_CONN_EVT_CONNECT_CYCLING");
						task_ble_conn_evt_connect_cycling();
					}break;	
			
					case TASK_BLE_CONN_EVT_DISCONNECT_HEARTRATE:{
						NRF_LOG_INFO("TASK_BLE_CONN_EVT_DISCONNECT_HEARTRATE");
						task_ble_conn_evt_disconnect_heartrate();
					}break;							
					case TASK_BLE_CONN_EVT_DISCONNECT_RUNNING:{
						NRF_LOG_INFO("TASK_BLE_CONN_EVT_DISCONNECT_RUNNING");
						task_ble_conn_evt_disconnect_running();
					}break;	
					case TASK_BLE_CONN_EVT_DISCONNECT_CYCLING:{
						NRF_LOG_INFO("TASK_BLE_CONN_EVT_DISCONNECT_CYCLING");
						task_ble_conn_evt_disconnect_cycling();
					}break;	
					case TASK_BLE_CONN_EVT_ACCESSORY_DATA_UPLOAD:{
						NRF_LOG_INFO("TASK_BLE_CONN_EVT_ACCESSORY_DATA_UPLOAD");
						task_ble_conn_evt_accessory_data_upload();
					}break;
					case TASK_BLE_CONN_EVT_PAIRING_FAILED:{
						NRF_LOG_INFO("TASK_BLE_CONN_EVT_PAIRING_FAILED");
						task_ble_conn_evt_pairing_failed();
					}break;
					default:{
						
					}break;
				}			
			#ifdef USE_QUEUE
			}
			#endif
					
    }
}


void task_ble_conn_init(void * p_context)
{
  NRF_LOG_DEBUG("task_ble_conn_init");

	BaseType_t xReturned = xTaskCreate(task,
																		 "CONN",
																		 256+128,
																		 p_context,
																		 APP_IRQ_PRIORITY_LOW,
																		 &task_handle);
	if (xReturned != pdPASS)
	{
			NRF_LOG_ERROR("task not created.");
			APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
	}
	#ifdef USE_QUEUE
	QueueTask = xQueueCreate( 10,sizeof( _queue_task ));
	#endif
	
	timer_create();
	
}



static void task_ble_conn_evt_connect(void)
{
	NRF_LOG_INFO("task_ble_conn_evt_connect");
	NRF_LOG_HEXDUMP_INFO(g_peer_addr.addr, sizeof(g_peer_addr.addr));
	uint8_t ret = 0;
	_task_ble_conn_evt task_ble_conn_evt;
	task_ble_conn_evt.command = BLE_UART_CCOMMAND_APPLICATION_BLE_CONNECT | BLE_UART_CCOMMAND_MASK_APPLICATION;
	memcpy(task_ble_conn_evt.addr, g_peer_addr.addr, 6);	
	
	
	
	
	_uart_data_param param;
	param.user_callback = NULL;
	param.p_data = (uint8_t *)&task_ble_conn_evt;
	param.length = sizeof(task_ble_conn_evt);	
	ret = drv_apollo2_send(&param);
	if (ret != 0)
	{
		task_ble_conn_evt_connect_flag = 1;
	}
	
//	uint8_t count = TASK_UART_DELAY_COUNT_OUT;
//	while(drv_ble_send_data(&param) != NRF_SUCCESS && count--)
//	{
//		DELAY_MS(TASK_UART_DELAY);
//	}			


}	



static void task_ble_conn_evt_disconnect(void)
{
	NRF_LOG_INFO("task_ble_conn_evt_disconnect");
	uint8_t ret = 0;
	_task_ble_conn_evt_disconnect task_ble_conn_evt_disconnect;
	task_ble_conn_evt_disconnect.command = BLE_UART_CCOMMAND_APPLICATION_BLE_DISCONNECT | BLE_UART_CCOMMAND_MASK_APPLICATION;
//	task_ble_conn_evt_disconnect.disconnect_reason = queue.p_data[0];
	task_ble_conn_evt_disconnect.disconnect_reason = 0;
	_uart_data_param param;
	param.user_callback = NULL;
	param.p_data = (uint8_t *)&task_ble_conn_evt_disconnect;
	param.length = sizeof(task_ble_conn_evt_disconnect);			
	ret = drv_apollo2_send(&param);
	if (ret != 0)
	{
		task_ble_conn_evt_connect_flag = 0;
	}
	
	
//	uint8_t count = TASK_UART_DELAY_COUNT_OUT;
//	while(drv_ble_send_data(&param) != NRF_SUCCESS && count--)
//	{
//		DELAY_MS(TASK_UART_DELAY);
//	}		

}	
static void task_ble_conn_param_update(void)
{
	
}	
static void task_ble_conn_evt_bond(void)						
{
	uint32_t error;
	// Discover peer's services.
	error  = ble_db_discovery_start(&m_db_disc[DISCONVER_ARRAY_ANCS], g_conn_handle_peripheral);
	if (error != NRF_ERROR_BUSY || error != NRF_ERROR_INVALID_STATE)
	{
			APP_ERROR_CHECK(error);
	}						
	/**Inform phone APP that connection is secured*/
//	uint8_t response[2] = {CMD_NRF52_SRV_BOND_REQUEST_RESPONSE,CMD_NRF52_SRV_SUCCUSS};
//	app_srv_nrf52_control_send(response, sizeof(response));	
	/**Inform APOLLO2 that connection is secured*/
//	uint32_t command[1] = {BLE_UART_CCOMMAND_APPLICATION_BLE_BOND | BLE_UART_CCOMMAND_MASK_APPLICATION};
//	_uart_data_param param;
//	param.user_callback = NULL;
//	param.p_data = (uint8_t *)command;
//	param.length = sizeof(command);	
//	uint8_t count = TASK_UART_DELAY_COUNT_OUT;
//	while(drv_ble_send_data(&param) != NRF_SUCCESS && count--)
//	{
//		DELAY_MS(TASK_UART_DELAY);
//	}	
}

static void task_ble_conn_evt_ancs_disc_complete(void)
{

//	uint32_t command[1] = {BLE_UART_CCOMMAND_APPLICATION_BLE_BOND | BLE_UART_CCOMMAND_MASK_APPLICATION};
//	_uart_data_param param;
//	param.user_callback = NULL;
//	param.p_data = (uint8_t *)command;
//	param.length = sizeof(command);	
//	uint8_t count = TASK_UART_DELAY_COUNT_OUT;
//	while(drv_ble_send_data(&param) != NRF_SUCCESS && count--)
//	{
//		DELAY_MS(TASK_UART_DELAY);
//	}	
}

static void task_ble_conn_evt_bond_android(void)						
{
	/**Inform APOLLO2 that connection is secured*/
//	uint32_t command[1] = {BLE_UART_CCOMMAND_APPLICATION_BLE_BOND | BLE_UART_CCOMMAND_MASK_APPLICATION};
//	_uart_data_param param;
//	param.user_callback = NULL;
//	param.p_data = (uint8_t *)command;
//	param.length = sizeof(command);	
//	uint8_t count = TASK_UART_DELAY_COUNT_OUT;
//	while(drv_ble_send_data(&param) != NRF_SUCCESS && count--)
//	{
//		DELAY_MS(TASK_UART_DELAY);
//	}		
}
static void task_ble_conn_evt_passkey(void)						
{
	_ble_passkey ble_passkey;
	ble_passkey.command = BLE_UART_CCOMMAND_APPLICATION_BLE_PASSKEY | BLE_UART_CCOMMAND_MASK_APPLICATION;
	#ifdef USE_NOTIFY	
	memcpy(ble_passkey.passkey,m_task_buffer.p_data,BLE_GAP_PASSKEY_LEN);	
	#endif
	#ifdef USE_QUEUE	
	memcpy(ble_passkey.passkey,queue.p_data,BLE_GAP_PASSKEY_LEN);
	#endif
	_uart_data_param param;
	param.user_callback = NULL;
	param.p_data = (uint8_t *)&ble_passkey;
	param.length = sizeof(ble_passkey);	
	drv_apollo2_send(&param);
	
	
//	uint8_t count = TASK_UART_DELAY_COUNT_OUT;
//	while(drv_ble_send_data(&param) != NRF_SUCCESS && count--)
//	{
//		DELAY_MS(TASK_UART_DELAY);
//	}	
	
}
static void task_ble_conn_evt_ble_uart(void)						
{
	_uart_data_param param;
	param.user_callback = NULL;
	#ifdef USE_NOTIFY
	param.p_data = m_task_buffer.p_data;
	param.length = m_task_buffer.length;	
	#endif
	#ifdef USE_QUEUE
	param.p_data = queue.p_data;
	param.length = queue.length;
	#endif	
	
	uint8_t count = TASK_UART_DELAY_COUNT_OUT;
	while(drv_ble_send_data(&param) != NRF_SUCCESS && count--)
	{
		DELAY_MS(TASK_UART_DELAY);
	}

	if (param.p_data[1] == (BLE_UART_CCOMAND_APPLICATION_SENSOR_DATA_UPLOAD >> 8))
	{
		uint8_t start = param.p_data[5];
		
		extern uint8_t cod_uart_normal_open;
		if (start)
		{
		cod_uart_normal_open = 1;		
		}
		else
		{
		cod_uart_normal_open =2;
		}
	}

}

static void task_ble_conn_hwt_rssi_read(void)
{
	_uart_data_param param;
	param.user_callback = NULL;
	param.p_data = (uint8_t *)queue.p_data;
	param.length = queue.length;	
	uint8_t count = TASK_UART_DELAY_COUNT_OUT;
	while(drv_ble_send_data(&param) != NRF_SUCCESS && count--)
	{
		DELAY_MS(TASK_UART_DELAY);
	}	

}	

static void task_ble_conn_dev_name_update(void)						
{
	_app_srv_nrf52_dev_name dev_name_response;
	ble_nrf52_evt_t *evt = (ble_nrf52_evt_t *)queue.p_data;
	_app_srv_nrf52_dev_name *new_dev_name = (_app_srv_nrf52_dev_name *)evt->params.data.p_data;

	dev_name_response.command = CMD_NRF52_SRV_DEVICE_NAME_UPDATE_RESPONSE;
	
	if(evt->params.data.length > BLE_CONFIG_DEVICE_NAME_LEN_MAX + 3)
	{
		dev_name_response.param.error = CMD_NRF52_SRV_ERROR_OUT_OF_BOUND;
	}
	else{
		/**<Copy new device name to global ram>*/
		memset((uint8_t *)&g_ble_config->dev_name.name,0,BLE_CONFIG_DEVICE_NAME_LEN_MAX+1);
		memcpy((uint8_t *)&g_ble_config->dev_name.name, (uint8_t *)&new_dev_name->param.dev.name, BLE_CONFIG_DEVICE_NAME_LEN_MAX+1);	
		/**<device name length is the data length minus the command length>*/
		g_ble_config->dev_name.len = new_dev_name->param.dev.len;  
		/**<Update new device name in softdevice>*/
		ble_advertising_dev_name_update(g_ble_config->dev_name.name, g_ble_config->dev_name.len);
		dev_name_response.param.error = CMD_NRF52_SRV_SUCCUSS;
		
	}		
	app_srv_nrf52_config_send((uint8_t *)&dev_name_response, 2);		
	if(dev_name_response.param.error == CMD_NRF52_SRV_SUCCUSS)
	{
		NRF_LOG_HEXDUMP_DEBUG(evt->params.data.p_data, evt->params.data.length);
		_app_srv_nrf52_dev_name *dev_name = (_app_srv_nrf52_dev_name *)(evt->params.data.p_data);
		_task_ble_conn_dev_name_update dev_name_update;
		dev_name_update.command = BLE_UART_CCOMMAND_APPLICATION_DEV_NAME_UPDATE | BLE_UART_CCOMMAND_MASK_APPLICATION;
		memcpy((uint8_t *)&dev_name_update.name,dev_name->param.dev.name,BLE_CONFIG_DEVICE_NAME_LEN_MAX+1);
		dev_name_update.len = dev_name->param.dev.len;
		_uart_data_param param;
		param.user_callback = NULL;
		param.p_data = (uint8_t *)&dev_name_update;
		param.length = sizeof(_task_ble_conn_dev_name_update);	
		uint8_t count = TASK_UART_DELAY_COUNT_OUT;
		while(drv_ble_send_data(&param) != NRF_SUCCESS && count--)
		{
			DELAY_MS(TASK_UART_DELAY);
		}	
		m_timer_evt = TIMER_EVT_STORE_BLE_CONFIG_FILE;
		timer_start(1500);
	}
}

				
static void task_ble_conn_ancs_attr_request(void)
{
	uint32_t ret = nrf_ble_ancs_c_request_attrs(&m_ancs_c, &g_notification_latest);
	APP_ERROR_CHECK(ret);	
}	

				
static void task_ble_conn_ancs_attr_id_app_identify(void)
{
	if (g_notif_attr_app_id_latest.attr_id == BLE_ANCS_NOTIF_ATTR_ID_APP_IDENTIFIER
			&& g_notif_attr_app_id_latest.attr_len != 0)
	{
			NRF_LOG_INFO("Request for %s: ", (uint32_t)g_notif_attr_app_id_latest.p_attr_data);
			uint32_t ret = nrf_ble_ancs_c_app_attr_request(&m_ancs_c,
																						g_notif_attr_app_id_latest.p_attr_data,
																						g_notif_attr_app_id_latest.attr_len);
			APP_ERROR_CHECK(ret);
	}
}	

static void task_ble_conn_ancs_message_send(void)					
{
	_app_srv_ancs_msg *app_srv_ancs_msg = (_app_srv_ancs_msg *)queue.p_data;
	app_srv_ancs_msg->command = BLE_UART_CCOMMAND_APPLICATION_APP_MESSAGE_IOS | BLE_UART_CCOMMAND_MASK_APPLICATION;
	app_srv_ancs_msg->crc = am_bootloader_fast_crc32((uint8_t*)(&app_srv_ancs_msg->crc)+sizeof(app_srv_ancs_msg->crc),  
														sizeof(_app_srv_ancs_msg)-sizeof(app_srv_ancs_msg->crc)-sizeof(app_srv_ancs_msg->command));		
	NRF_LOG_HEXDUMP_DEBUG(app_srv_ancs_msg, sizeof(_app_srv_ancs_msg));
	_uart_data_param param;
	param.user_callback = NULL;
	param.p_data = (uint8_t *)app_srv_ancs_msg;
	param.length = sizeof(_app_srv_ancs_msg);	
	uint8_t count = TASK_UART_DELAY_COUNT_OUT;
	while(drv_ble_send_data(&param) != NRF_SUCCESS && count--)
	{
		DELAY_MS(TASK_UART_DELAY);
	}		
}


static void task_ble_conn_wechat_sport_data_request(void)					
{
	uint32_t command[1] = {BLE_UART_CCOMMAND_APPLICATION_WECHAT_SPORT_DATA_REQUEST | BLE_UART_CCOMMAND_MASK_APPLICATION}; 
	_uart_data_param param;
	param.user_callback = NULL;
	param.p_data = (uint8_t *)command;
	param.length = sizeof(command);	
	uint8_t count = TASK_UART_DELAY_COUNT_OUT;
	while(drv_ble_send_data(&param) != NRF_SUCCESS && count--)
	{
		DELAY_MS(TASK_UART_DELAY);
	}	
	
}


static void task_ble_conn_evt_connect_heartrate(void)
{
	_ble_accessory_conn_status status;
	status.command = BLE_UART_CCOMMAND_APPLICATION_ACCESSORY_HEARTRATE_CONNECT | BLE_UART_CCOMMAND_MASK_APPLICATION; 
	memcpy(status.mac,g_accessory_heartrate.info.addr.addr,BLE_GAP_ADDR_LEN);
	_uart_data_param param;
	param.user_callback = NULL;
	param.p_data = (uint8_t *)&status;
	param.length = sizeof(status);	
	uint8_t count = TASK_UART_DELAY_COUNT_OUT;
	while(drv_ble_send_data(&param) != NRF_SUCCESS && count--)
	{
		DELAY_MS(TASK_UART_DELAY);
	}	
}	
static void task_ble_conn_evt_connect_running(void)
{
	_ble_accessory_conn_status status;
	status.command = BLE_UART_CCOMMAND_APPLICATION_ACCESSORY_RUNNING_CONNECT | BLE_UART_CCOMMAND_MASK_APPLICATION; 
	memcpy(status.mac,g_accessory_running.info.addr.addr,BLE_GAP_ADDR_LEN);
	_uart_data_param param;
	param.user_callback = NULL;
	param.p_data = (uint8_t *)&status;
	param.length = sizeof(status);	
	uint8_t count = TASK_UART_DELAY_COUNT_OUT;
	while(drv_ble_send_data(&param) != NRF_SUCCESS && count--)
	{
		DELAY_MS(TASK_UART_DELAY);
	}		
	
}	
static void task_ble_conn_evt_connect_cycling(void)
{
	_ble_accessory_conn_status status;
	status.command = BLE_UART_CCOMMAND_APPLICATION_ACCESSORY_CYCLING_CONNECT | BLE_UART_CCOMMAND_MASK_APPLICATION; 
	memcpy(status.mac,g_accessory_cycling.info.addr.addr,BLE_GAP_ADDR_LEN);
	_uart_data_param param;
	param.user_callback = NULL;
	param.p_data = (uint8_t *)&status;
	param.length = sizeof(status);	
	uint8_t count = TASK_UART_DELAY_COUNT_OUT;
	while(drv_ble_send_data(&param) != NRF_SUCCESS && count--)
	{
		DELAY_MS(TASK_UART_DELAY);
	}		
	
}
static void task_ble_conn_evt_disconnect_heartrate(void)
{
	uint32_t command[1] = {BLE_UART_CCOMMAND_APPLICATION_ACCESSORY_HEARTRATE_DISCONNECT | BLE_UART_CCOMMAND_MASK_APPLICATION}; 
	_uart_data_param param;
	param.user_callback = NULL;
	param.p_data = (uint8_t *)command;
	param.length = sizeof(command);	
	uint8_t count = TASK_UART_DELAY_COUNT_OUT;
	while(drv_ble_send_data(&param) != NRF_SUCCESS && count--)
	{
		DELAY_MS(TASK_UART_DELAY);
	}	
}	
static void task_ble_conn_evt_disconnect_running(void)
{
	uint32_t command[1] = {BLE_UART_CCOMMAND_APPLICATION_ACCESSORY_RUNNING_DISCONNECT | BLE_UART_CCOMMAND_MASK_APPLICATION}; 
	_uart_data_param param;
	param.user_callback = NULL;
	param.p_data = (uint8_t *)command;
	param.length = sizeof(command);	
	uint8_t count = TASK_UART_DELAY_COUNT_OUT;
	while(drv_ble_send_data(&param) != NRF_SUCCESS && count--)
	{
		DELAY_MS(TASK_UART_DELAY);
	}	
}
static void task_ble_conn_evt_disconnect_cycling(void)
{
	uint32_t command[1] = {BLE_UART_CCOMMAND_APPLICATION_ACCESSORY_CYCLING_DISCONNECT | BLE_UART_CCOMMAND_MASK_APPLICATION}; 
	_uart_data_param param;
	param.user_callback = NULL;
	param.p_data = (uint8_t *)command;
	param.length = sizeof(command);	
	uint8_t count = TASK_UART_DELAY_COUNT_OUT;
	while(drv_ble_send_data(&param) != NRF_SUCCESS && count--)
	{
		DELAY_MS(TASK_UART_DELAY);
	}	
}			

static void task_ble_conn_evt_accessory_data_upload(void)
{
	_uart_data_param param;
	param.user_callback = NULL;
	param.p_data = (uint8_t *)&g_task_ble_accessory_data;
	param.length = sizeof(g_task_ble_accessory_data);	
	uint8_t count = TASK_UART_DELAY_COUNT_OUT;
	while(drv_ble_send_data(&param) != NRF_SUCCESS && count--)
	{
		DELAY_MS(TASK_UART_DELAY);
	}	
}	

static void task_ble_conn_evt_pairing_failed(void)
{
	uint8_t response[2] = {CMD_NRF52_SRV_BOND_REQUEST_RESPONSE,CMD_NRF52_SRV_ERROR_CONN_PARAM_NOT_ACCEPT};
	app_srv_nrf52_control_send(response, sizeof(response));		
	NRF_LOG_INFO("task_ble_conn_evt_pairing_failed\n");
}



