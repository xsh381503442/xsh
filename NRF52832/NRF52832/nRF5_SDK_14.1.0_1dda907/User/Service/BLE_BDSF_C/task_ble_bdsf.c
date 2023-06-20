#include "task_ble_bdsf.h"

#include "task_ble_conn.h"
#include "ble_srv_common.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "portmacro.h"
#include "portable.h"
#include "event_groups.h"
#include "semphr.h"

#include "task_apollo2.h"
#include "cmd_apollo2.h"
#include "nrf_delay.h"
#include "app_srv_nrf52.h"
#include "cmd_nrf52_srv.h"
#include "app_srv_nrf52.h"
#include "ble_db_discovery.h"
#include "nrf_sdh_ble.h"
#include "app_srv_apollo.h"
#include "drv_ble_flash.h"
#include "ble_conn_state.h"
#include "app_srv_acces.h"
#include "ble_hci.h"
#include "ble_advertising.h"
#include "app_srv_ancs.h"
#include "drv_apollo2.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "ble_bdsf_c.h"

//NRF_LOG_MODULE_REGISTER();

#define MIN_CONNECTION_INTERVAL         MSEC_TO_UNITS(7.5, UNIT_1_25_MS)    /**< Determines minimum connection interval in milliseconds. */
#define MAX_CONNECTION_INTERVAL         MSEC_TO_UNITS(30, UNIT_1_25_MS)     /**< Determines maximum connection interval in milliseconds. */
#define SLAVE_LATENCY                   0                                   /**< Determines slave latency in terms of connection events. */
#define SUPERVISION_TIMEOUT             MSEC_TO_UNITS(4000, UNIT_10_MS)     /**< Determines supervision time-out in units of 10 milliseconds. */


#define BLE_MAX_PACK_LEN		16

extern uint16_t g_conn_handle_peripheral;
extern bool g_is_central_mode;
extern ble_advertising_t m_advertising;
extern ble_bdsf_c_t m_ble_bdsf_c;
extern ble_db_discovery_t m_db_disc[];
extern char task_ble_bdsf_buf_t[];
extern uint8_t index_t;

char task_ble_bdsf_buf_g[256] = {0};
uint8_t tash_ble_bdsf_buf_index = 0;

struct s_queue_task{
	uint32_t	evt;
	uint32_t length;
	uint8_t	*p_data;
};
typedef struct s_queue_task _queue_task;
static QueueHandle_t QueueTask = NULL;
static _queue_task queue = {0,NULL,0};	
static TaskHandle_t task_handle;


const char *ins_recv_arr[] = {
			"$BDTXR",		//设备自检//短报文
			"$BDZDX",		//设备信息
			"$GNRMC",		//RNSS_RMC
			"$BDRMC"
//			"$GNGGA",		//RNSS_GGA
//			"$BDGGA",
//			"$BDDWR",		//RDSS
//			"$BDHMX",		//救援中心号码
//			"$BDVER",		//版本
//			"$BDBCC",		//策略查询
//			"$BDBCL",		//报位策略更改
//			"$BDZCS",		//报位总次数
//			"$BDDGH",		//低功耗反馈
//			"$BDFKI"		//反馈信息
};

uint8_t MAX_INS_RECV_NUM = sizeof(ins_recv_arr) / sizeof(ins_recv_arr[0]);

uint8_t state_g = 1;

static void task_ble_bdsf_evt_scan_enable(void);
static void task_ble_bdsf_evt_handle_device_name(void);

uint8_t VALID_DEVICE_LIST = 0;
ble_bdsf_name_peer_addr DEVICE_ADV_LIST[MAX_DEV_NUM] = {0};			
ble_bdsf_name_peer_addr DEV_S = {0};	//保存上一次连接过的设备


#define TIMER_INTERVAL            6 * 1000 		//10m
#define OSTIMER_WAIT_FOR_QUEUE              100 
static TimerHandle_t m_timer = NULL; 

uint8_t SENDING_DEV_NAME = 1;

static void ble_searching_timer_delete(void);
static void task_ble_bdsf_evt_handle_device_name_send(void);
static void task_ble_bdsf_evt_scan_disable(void);
static void task_ble_bdsf_evt_handle_notify_data_send(void);
static void task_ble_bdsf_evt_handle_write_data(void);
static uint8_t task_ble_bdsf_evt_handle_notify_data_to_ins(char *buffer, uint8_t *index);
static void task_ble_bdsf_evt_handle_reconnect_last_device(void);

static void timeout_handler(TimerHandle_t xTimer)
{
    NRF_LOG_DEBUG("timeout_handler\n");
		
		SENDING_DEV_NAME = 0;
		
		task_ble_bdsf_handle(TASK_BLE_BDSF_EVT_DEVICE_NAME_SEND, NULL, 0);
		
		ble_searching_timer_delete();
}


static void timer_create(void)
{
	if (m_timer == NULL)
	{
		m_timer = xTimerCreate("BDSF",
													 TIMER_INTERVAL,
													 pdTRUE,
													 NULL,
													 timeout_handler);
		if (NULL == m_timer)
		{
				APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
		}	
	}
	else
	{
		if(xTimerStart(m_timer, 100) != pdPASS)
		{
			NRF_LOG_DEBUG("[TIMER_BDSF]:BDSFTimer success\n");
		}
	}
}

static void timer_start(uint32_t timeout_msec)
{
	
	 if ( xTimerIsTimerActive( m_timer )) 
	 {
		 return;
	 }
	
	
	
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
static void timer_stop(void)
{
	 if ( !xTimerIsTimerActive( m_timer )) 
	 {
			 // xTimer is active, do something.
		 return;
	 }	
	if (pdPASS != xTimerStop(m_timer, OSTIMER_WAIT_FOR_QUEUE))
	{
			APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
	}		
}


static void ble_searching_timer_delete(void)
{
	if(m_timer != NULL)
	{
		xTimerDelete(m_timer, 100);
		m_timer = NULL;
	}
}



void task_ble_bdsf_handle(uint32_t evt, uint8_t *p_data, uint32_t length)
{
	_queue_task  queue = {0};
	queue.evt = evt;
	queue.p_data = p_data;
	queue.length = length;
	BaseType_t xReturned = xQueueSend(QueueTask, &queue, portMAX_DELAY );	
	if (xReturned != pdPASS)
	{
			APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
	}	
}


static void task(void * pvParameter)
{
    NRF_LOG_DEBUG("task bdsf");
    (void)pvParameter;	
    while (1)
    {
			if (xQueueReceive(QueueTask, &queue, portMAX_DELAY) == pdTRUE)
			{	
				switch(queue.evt)
				{
					case TASK_BLE_BDSF_EVT_SCAN_ENABLE:{
						VALID_DEVICE_LIST = 0;
						memset(DEVICE_ADV_LIST, 0, sizeof(ble_bdsf_name_peer_addr) * MAX_DEV_NUM);
						task_ble_bdsf_evt_scan_enable();
						timer_create();
						timer_start(TIMER_INTERVAL);
					}break;
					case TASK_BLE_BDSF_EVT_SCAN_AGAIN:{
						SENDING_DEV_NAME = 1;
						task_ble_bdsf_evt_scan_enable();
					}break;
					case TASK_BLE_BDSF_EVT_RECONNECT_LAST_DEVICE:{
						task_ble_bdsf_evt_handle_reconnect_last_device();
					}break;
					case TASK_BLE_BDSF_EVT_NAME_MATCH:{
//						ble_bdsf_dev_name tmp;
//						tmp.command = BLE_UART_RESPONSE_BLE_DEVICE_NAME | BLE_UART_CCOMMAND_MASK_BDSF;
//						tmp.dev_name = queue.p_data;
//						//memcpy(tmp.dev_name, queue.p_data, queue.length);
//						tmp.len = queue.length + 1 + 4 + 1;
//						drv_apollo2_uart_put((uint8_t *)&tmp, sizeof(tmp));
												
					}break;
					case TASK_BLE_BDSF_EVT_DEVICE_NAME_HANDLE:{
						task_ble_bdsf_evt_handle_device_name();
					}break;
					case TASK_BLE_BDSF_EVT_DEVICE_NAME_SEND:{
						task_ble_bdsf_evt_handle_device_name_send();
					}break;
					case TASK_BLE_BDSF_EVT_SCAN_DISABLE:{
						task_ble_bdsf_evt_scan_disable();
					}break;
					case TASK_BLE_BDSF_EVT_BD_SRV_DISC:{
						NRF_LOG_DEBUG("TASK_BLE_BDSF_EVT_BD_SRV_DISC");
						uint32_t error = ble_db_discovery_start(&m_db_disc[DISCONVER_ARRAY_BDSF], m_ble_bdsf_c.conn_handle);
            if (error != NRF_ERROR_BUSY)
            {
                APP_ERROR_CHECK(error);
            }
					}break;
					case TASK_BLE_BDSF_EVT_DATA_NOTIFY_SEND:{
//						NRF_LOG_DEBUG("TASK_BLE_BDSF_EVT_DATA_NOTIFY_SEND");
//						task_ble_bdsf_evt_handle_notify_data_send();
						task_ble_bdsf_evt_handle_notify_data_to_ins(task_ble_bdsf_buf_g, &tash_ble_bdsf_buf_index);
					}break;
					case TASK_BLE_BDSF_EVT_WRITE_COMMAND:{
						NRF_LOG_DEBUG("TASK_BLE_BDSF_EVT_WRITE_COMMAND");
						//
						task_ble_bdsf_evt_handle_write_data();
					}break;
					default:{
						
					}break;
				}
			}
    }
}


void task_ble_bdsf_init(void * p_context)
{
  NRF_LOG_DEBUG("task_ble_bdsf_init");
    
    
	BaseType_t xReturned = xTaskCreate(task,
																		 "BDSF",
																		 256,
																		 p_context,
																		 APP_IRQ_PRIORITY_LOW,
																		 &task_handle);
	if (xReturned != pdPASS)
	{
			NRF_LOG_ERROR("task not created.");
			APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
	}

	QueueTask = xQueueCreate( 10,sizeof( _queue_task ));

}

void task_ble_bdsf_scan_start(void)
{
	ret_code_t err_code;
	ble_gap_scan_params_t scan_params;
	scan_params.active = 1;
	scan_params.interval = 0x2000;//5秒
	scan_params.window = 0x1000;//10毫秒
	scan_params.timeout = 0x0000;
	scan_params.use_whitelist = 0;
	
	
	(void) sd_ble_gap_scan_stop();

	err_code = sd_ble_gap_scan_start(&scan_params);
	// It is okay to ignore this error since we are stopping the scan anyway.
	if (err_code != NRF_ERROR_INVALID_STATE)
	{
			APP_ERROR_CHECK(err_code);
	}
}


static void task_ble_bdsf_evt_scan_enable(void)						
{
//	uint32_t err_code;
//	ble_advertising_auto_reset();
//	if(g_conn_handle_peripheral != BLE_CONN_HANDLE_INVALID)
//	{
//		err_code = sd_ble_gap_disconnect(g_conn_handle_peripheral,
//																		 BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
//		APP_ERROR_CHECK(err_code);	
//		g_conn_handle_peripheral = BLE_CONN_HANDLE_INVALID;
//	} 	
	task_ble_bdsf_scan_start();
	g_is_central_mode = true;
}

static uint32_t adv_report_parse(uint8_t type, uint8_array_t * p_advdata, uint8_array_t * p_typedata)
{
    uint32_t  index = 0;
    uint8_t * p_data;

    p_data = p_advdata->p_data;

    while (index < p_advdata->size)
    {
        uint8_t field_length = p_data[index];
        uint8_t field_type   = p_data[index + 1];

        if (field_type == type)
        {
            p_typedata->p_data   = &p_data[index + 2];
            p_typedata->size = field_length - 1;
            return NRF_SUCCESS;
        }
        index += field_length + 1;
    }
    return NRF_ERROR_NOT_FOUND;
}

static void task_ble_bdsf_evt_handle_device_name(void)
{
	int i;
//	uint8_array_t adv_data;
//  uint8_array_t dev_name;
	if(VALID_DEVICE_LIST == MAX_DEV_NUM)
	{

		for(int i = 0; i < MAX_DEV_NUM; i++)
			NRF_LOG_INFO("DEVICE_ADV_LIST[%d]->dev_name is %s...\n",i, DEVICE_ADV_LIST[i].dev_name);
		return;
	}
	
	NRF_LOG_INFO("VALID_DEVICE_LIST is %d\n", VALID_DEVICE_LIST);
	ble_bdsf_name_peer_addr *adv_info = (ble_bdsf_name_peer_addr *)queue.p_data;
		
//	adv_data.p_data = adv_info->data;
//	adv_data.size = adv_info->dlen;
//	NRF_LOG_INFO("adv_data.size is %d\n", adv_data.size);

	
	if(VALID_DEVICE_LIST < MAX_DEV_NUM)
	{	

		//NRF_LOG_INFO("dev_name is %s\n", dev_name.p_data);
		for(i = 0; i < MAX_DEV_NUM; i++)
		{
			if((memcmp(&(adv_info->peer_addr), &(DEVICE_ADV_LIST[i].peer_addr), sizeof(ble_gap_addr_t))== 0) &&
					(memcmp(&(adv_info->dev_name), &(DEVICE_ADV_LIST[i].dev_name), 7) == 0))
			{
				NRF_LOG_INFO("Alreaday have.\n");
				break;
			}
		}
		
		if(i == MAX_DEV_NUM)
		{	
			memcpy(&(DEVICE_ADV_LIST[VALID_DEVICE_LIST++]), adv_info, sizeof(ble_bdsf_name_peer_addr));
		}

	}
	
	for(int i=0;i<VALID_DEVICE_LIST-1;i++)
	{
    for(int j=VALID_DEVICE_LIST;j>i;j--)
		{
       if(DEVICE_ADV_LIST[j].rssi > DEVICE_ADV_LIST[j-1].rssi)
			 {
					ble_bdsf_name_peer_addr tmp = DEVICE_ADV_LIST[j];
          DEVICE_ADV_LIST[j] = DEVICE_ADV_LIST[j-1];
          DEVICE_ADV_LIST[j-1] = tmp;
       }
    }
  }		
}

static void task_ble_bdsf_evt_handle_device_name_send(void)
{
	uint8_t num;
	
	num = (VALID_DEVICE_LIST <= 3) ? VALID_DEVICE_LIST : 3;
	
	uint32_t command[64] = {BLE_UART_CCOMMAND_BLE_DEVICE_NAME | BLE_UART_CCOMMAND_MASK_BDSF};
	command[1] = num;
	for(int i = 0; i < num; i++)
		memcpy(((uint8_t *)(command + 2) + sizeof(ble_bdsf_name_peer_addr) * i), &(DEVICE_ADV_LIST[i]), sizeof(ble_bdsf_name_peer_addr));	

	_uart_data_param param;
	param.user_callback = NULL;
	param.p_data = (uint8_t *)&command[0];
	param.length = sizeof(command);	
	drv_apollo2_send(&param);		
//	task_ble_conn_handle(TASK_BLE_CONN_EVT_BLE_UART, (uint8_t *)(&command), sizeof(command));	
	
	NRF_LOG_INFO("After send ble_dev_info...\n");
}


static void task_ble_bdsf_evt_scan_disable(void)
{
	uint32_t err_code;
	timer_stop();
	(void) sd_ble_gap_scan_stop();
	if(m_ble_bdsf_c.conn_handle != BLE_CONN_HANDLE_INVALID)
	{
		err_code = sd_ble_gap_disconnect(m_ble_bdsf_c.conn_handle,
																		 BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
		APP_ERROR_CHECK(err_code);	
		m_ble_bdsf_c.conn_handle = BLE_CONN_HANDLE_INVALID;						
	}
	ble_advertising_auto_set();
	if(g_conn_handle_peripheral == BLE_CONN_HANDLE_INVALID)
	{
		err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
		if(err_code != NRF_ERROR_INVALID_STATE)
		{
			APP_ERROR_CHECK(err_code);
		}				
	}
	
	g_is_central_mode = false;
}	

static uint8_t task_ble_bdsf_evt_handle_notify_data_to_ins(char *buffer, uint8_t *index)
{
	#if 0	//与nordic 通信调试接口
	char buffer_t[256] = {0};
	memcpy(buffer_t, (char *)queue.p_data, queue.length);
	uint32_t command[64] = {BLE_UART_CCOMMAND_BLE_NOTIFY_DATA | BLE_UART_CCOMMAND_MASK_BDSF};

	memset(&task_ble_bdsf_buf_g[0], 0, 256);
	memcpy(&task_ble_bdsf_buf_g[0], buffer_t, queue.length);
	memcpy((uint8_t *)(&command[1]), task_ble_bdsf_buf_g, 220);
	_uart_data_param param;
	param.user_callback = NULL;
	param.p_data = (uint8_t *)&command[0];
	param.length = sizeof(command);	
	NRF_LOG_INFO("Command receive : %s", (char *)(&command[1]));
	drv_apollo2_send(&param);
	#else
//	char buffer_t[256] = {0};
//	memcpy(buffer_t, (char *)queue.p_data, queue.length);
	
//	if(strchr(buffer_t, '$') != NULL)	//头
//	{
//		*index = 0;
//		memset(&task_ble_bdsf_buf_g[0], 0, 256);
//		memcpy(&task_ble_bdsf_buf_g[0], queue.p_data, queue.length);
//		*index += queue.length;
//	}
//	else
//	{
//		if(strchr(buffer_t, '\n') == NULL)		
//		{
//			memcpy(&task_ble_bdsf_buf_g[*index], queue.p_data, queue.length);
//			*index += queue.length;
//		}
//		else		//尾
//		{
//			memcpy(&task_ble_bdsf_buf_g[*index], queue.p_data, queue.length);
//			*index += queue.length;
						
			for(int i = 0; i < MAX_INS_RECV_NUM; i++)
			{
				if((strncmp(ins_recv_arr[i], (char *)&queue.p_data[0], 6) == 0) && strchr((char *)&queue.p_data[0], '*') != NULL)
				{
					uint32_t command[64] = {BLE_UART_CCOMMAND_BLE_NOTIFY_DATA | BLE_UART_CCOMMAND_MASK_BDSF};

					memcpy((uint8_t *)(&command[1]), queue.p_data, queue.length);
					index_t = 0;
					memset(task_ble_bdsf_buf_t, 0, 256);
					_uart_data_param param;
					param.user_callback = NULL;
					param.p_data = (uint8_t *)&command[0];
					param.length = sizeof(command);	
					
					drv_apollo2_send(&param);

					NRF_LOG_INFO("Command task_ble: %s", (char *)(&command[1]));
					
					break;
				}
			}
			
//		}			
//	}
	#endif
	
	return 0;
}

//static void task_ble_bdsf_evt_handle_notify_data_send(void)
//{	
//	uint32_t command[64] = {BLE_UART_CCOMMAND_BLE_NOTIFY_DATA | BLE_UART_CCOMMAND_MASK_BDSF};
//	
//	memcpy((uint8_t *)(&command[1]), queue.p_data, queue.length);

//	_uart_data_param param;
//	param.user_callback = NULL;
//	param.p_data = (uint8_t *)&command[0];
//	param.length = sizeof(command);	
////	drv_apollo2_send(&param);		
///*//	task_ble_conn_handle(TASK_BLE_CONN_EVT_BLE_UART, (uint8_t *)(&command), sizeof(command));	*/
//	NRF_LOG_INFO("Command is %s", (char *)(&command[1]));
////	NRF_LOG_INFO("After send notify_data_send...\n");
//}


static void task_ble_bdsf_evt_handle_write_data(void)
{
	uint8_t command[240] = {0};
	uint8_t command_t[24] = {0};
	uint16_t i = 0;
	
//	memset(command, 0, 240);
	memcpy(command, &queue.p_data[4], queue.length);
	uint8_t len_t = strlen((char *)&command[0]);
	NRF_LOG_INFO("len_t:%d,Command send : %s\n",len_t , command);
	
	if(len_t < BLE_MAX_PACK_LEN)
	{
		ble_bdsf_c_string_send(&m_ble_bdsf_c, command, len_t);
	}
	else
	{
		for(i = 0; i < (len_t / BLE_MAX_PACK_LEN); i++)
		{
			memset((uint8_t *)&command_t[0], 0, sizeof(command_t));
			memcpy((uint8_t *)&command_t[0], &command[i * BLE_MAX_PACK_LEN], BLE_MAX_PACK_LEN);
//			NRF_LOG_ERROR("%d.command_t[%d]:%c",i, k, command_t[k]);
			ble_bdsf_c_string_send(&m_ble_bdsf_c, command_t, BLE_MAX_PACK_LEN);
			nrf_delay_ms(100);
//			vTaskDelay(200);
		}
		
		memset((uint8_t *)&command_t[0], 0, sizeof(command_t));
		memcpy((uint8_t *)&command_t[0], &command[i * BLE_MAX_PACK_LEN], strlen((char *)&command[i * BLE_MAX_PACK_LEN]));
//		NRF_LOG_ERROR("%d.command_t[%d]:%c", i, j, command_t[j]);
		ble_bdsf_c_string_send(&m_ble_bdsf_c, command_t, BLE_MAX_PACK_LEN);
	}
	
	
//	NRF_LOG_INFO("task_ble_bdsf_evt_handle_write_data after send string ...\n");
}

static void task_ble_bdsf_evt_handle_reconnect_last_device(void)
{
	//连接
	uint32_t err_code;
	ble_gap_addr_t peer_addr;	
	memcpy((uint8_t*)(&peer_addr),(uint8_t*)(&DEV_S.peer_addr), sizeof(ble_gap_addr_t));	
		
	(void) sd_ble_gap_scan_stop();
	
	ble_gap_scan_params_t const scan_param =
	{
		.active   = 1,
		.interval = 0x00A0,
		.window   = 0x0050,
		.timeout  = 0x0000,
		.use_whitelist = 0,
	};

	ble_gap_conn_params_t const connection_param =
	{
		(uint16_t)MIN_CONNECTION_INTERVAL,
		(uint16_t)MAX_CONNECTION_INTERVAL,
		(uint16_t)SLAVE_LATENCY,
		(uint16_t)SUPERVISION_TIMEOUT
	};	

	err_code = sd_ble_gap_connect((const ble_gap_addr_t *)&peer_addr,
																&scan_param,
																&connection_param,
																1);
	if (err_code != NRF_SUCCESS)
	{	
			NRF_LOG_ERROR("Connection Request Failed, reason %d.", err_code);
	}	
	
	NRF_LOG_INFO("task_ble_bdsf_evt_handle_reconnect_last_device()-->SENDING_DEV_NAME:%d ...\n", SENDING_DEV_NAME);
}
