#include "task_ble_conn.h"
#include "ble_srv_common.h"
#include "task_ble_accessary.h"

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


#define LOG_LEVEL_OFF 							0
#define LOG_LEVEL_ERROR 						1
#define LOG_LEVEL_WARNING 					2
#define LOG_LEVEL_INFO 							3
#define LOG_LEVEL_DEBUG 						4
#define NRF_LOG_MODULE_NAME task_ble_accessary
#define NRF_LOG_LEVEL       LOG_LEVEL_DEBUG
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
NRF_LOG_MODULE_REGISTER();


//#define DELAY_MS(X)                 nrf_delay_ms(X)
#define DELAY_MS(X)                 vTaskDelay(X)
//#define DELAY_MS(X)                   do{TickType_t xLastWakeTime;xLastWakeTime = xTaskGetTickCount ();vTaskDelayUntil( &xLastWakeTime, X / ( ( TickType_t ) 1000 / 3 ) );}while(0)



struct s_queue_task{
	uint32_t	evt;
	uint32_t length;
	uint8_t	*p_data;
};
typedef struct s_queue_task _queue_task;
static QueueHandle_t QueueTask = NULL;
static _queue_task queue = {0,NULL,0};	
static TaskHandle_t task_handle;

static void task_ble_accessory_evt_scan_enable(void);	
static void task_ble_accessory_evt_scan_disable(void);

extern _accessory_heartrate g_accessory_heartrate;
extern _accessory_running g_accessory_running;
extern _accessory_cycling g_accessory_cycling;


_task_ble_accessory_data g_task_ble_accessory_data;
_task_ble_accessory_data g_task_ble_accessory_data_old;
static uint8_t accessory_data_send_count = 0;

#define TIMER_INTERVAL            			1500 
#define OSTIMER_WAIT_FOR_QUEUE              100
#define REPEAT_SEND_ACCESSORY_DATA_COUNT	10
static TimerHandle_t m_timer; 

static void accessory_data_send(bool data_valid)
{
	if (data_valid == false)
	{
		if(accessory_data_send_count<REPEAT_SEND_ACCESSORY_DATA_COUNT)
		{
			accessory_data_send_count++;
			memcpy((uint8_t *)&g_task_ble_accessory_data, (uint8_t *)&g_task_ble_accessory_data_old, sizeof(g_task_ble_accessory_data));
		}
	}
	else
	{
		accessory_data_send_count = 0;
		memcpy((uint8_t *)&g_task_ble_accessory_data_old,(uint8_t *)&g_task_ble_accessory_data,sizeof(g_task_ble_accessory_data));
	}
}

static void timeout_handler(TimerHandle_t xTimer)
{
    UNUSED_PARAMETER(xTimer);
    NRF_LOG_DEBUG("timeout_handler");

	
	g_task_ble_accessory_data.command = BLE_UART_CCOMMAND_APPLICATION_ACCESSORY_DATA_UPLOAD | BLE_UART_CCOMMAND_MASK_APPLICATION;
	g_task_ble_accessory_data.hr_data_valid = g_accessory_heartrate.data_valid;	
	g_task_ble_accessory_data.hr_value = g_accessory_heartrate.hr_value;
	
	g_task_ble_accessory_data.run_data_valid = g_accessory_running.data_valid;	
	g_task_ble_accessory_data.run_is_inst_stride_len_present = g_accessory_running.is_inst_stride_len_present;
	g_task_ble_accessory_data.run_is_total_distance_present = g_accessory_running.is_total_distance_present;
	g_task_ble_accessory_data.run_is_running = g_accessory_running.is_running;
	g_task_ble_accessory_data.run_inst_speed = g_accessory_running.inst_speed;
	g_task_ble_accessory_data.run_inst_cadence = g_accessory_running.inst_cadence;
	g_task_ble_accessory_data.run_inst_stride_length = g_accessory_running.inst_stride_length;
	g_task_ble_accessory_data.run_total_distance = g_accessory_running.total_distance;	
	
	g_task_ble_accessory_data.cycle_data_valid = g_accessory_cycling.data_valid;	
	g_task_ble_accessory_data.cycle_is_wheel_revolution_data_present = g_accessory_cycling.is_wheel_revolution_data_present;
	g_task_ble_accessory_data.cycle_is_crank_revolution_data_present = g_accessory_cycling.is_crank_revolution_data_present;
	g_task_ble_accessory_data.cycle_cumulative_wheel_revolutions = g_accessory_cycling.cumulative_wheel_revolutions;
	g_task_ble_accessory_data.cycle_last_wheel_event_time = g_accessory_cycling.last_wheel_event_time;
	g_task_ble_accessory_data.cycle_cumulative_crank_revolutions = g_accessory_cycling.cumulative_crank_revolutions;
	g_task_ble_accessory_data.cycle_last_crank_event_time = g_accessory_cycling.last_crank_event_time;
	
	NRF_LOG_DEBUG("data.command --> 0x%08X",g_task_ble_accessory_data.command);
	NRF_LOG_DEBUG("data.hr_data_valid --> %d",g_task_ble_accessory_data.hr_data_valid);
	NRF_LOG_DEBUG("data.hr_value --> %d",g_task_ble_accessory_data.hr_value);
	
	NRF_LOG_DEBUG("data.run_data_valid --> %d",g_task_ble_accessory_data.run_data_valid);
	NRF_LOG_DEBUG("data.run_is_inst_stride_len_present --> %d",g_task_ble_accessory_data.run_is_inst_stride_len_present);
	NRF_LOG_DEBUG("data.run_is_total_distance_present --> %d",g_task_ble_accessory_data.run_is_total_distance_present);
	NRF_LOG_DEBUG("data.run_is_running --> %d",g_task_ble_accessory_data.run_is_running);
	NRF_LOG_DEBUG("data.run_inst_speed --> %d",g_task_ble_accessory_data.run_inst_speed);
	NRF_LOG_DEBUG("data.run_inst_cadence --> %d",g_task_ble_accessory_data.run_inst_cadence);
	NRF_LOG_DEBUG("data.run_inst_stride_length --> %d",g_task_ble_accessory_data.run_inst_stride_length);
	NRF_LOG_DEBUG("data.run_total_distance --> %d",g_task_ble_accessory_data.run_total_distance);
	
	NRF_LOG_DEBUG("data.cycle_data_valid --> %d",g_task_ble_accessory_data.cycle_data_valid);
	NRF_LOG_DEBUG("data.cycle_is_wheel_revolution_data_present --> %d",g_task_ble_accessory_data.cycle_is_wheel_revolution_data_present);
	NRF_LOG_DEBUG("data.cycle_is_crank_revolution_data_present --> %d",g_task_ble_accessory_data.cycle_is_crank_revolution_data_present);
	NRF_LOG_DEBUG("data.cycle_cumulative_wheel_revolutions --> %d",g_task_ble_accessory_data.cycle_cumulative_wheel_revolutions);
	NRF_LOG_DEBUG("data.cycle_last_wheel_event_time --> %d",g_task_ble_accessory_data.cycle_last_wheel_event_time);
	NRF_LOG_DEBUG("data.cycle_cumulative_crank_revolutions --> %d",g_task_ble_accessory_data.cycle_cumulative_crank_revolutions);
	NRF_LOG_DEBUG("data.cycle_last_crank_event_time --> %d",g_task_ble_accessory_data.cycle_last_crank_event_time);


	if(g_accessory_heartrate.info.conn_handle != BLE_CONN_HANDLE_INVALID)
	{
		accessory_data_send(g_accessory_heartrate.data_valid);		
	}
	else if(g_accessory_running.info.conn_handle != BLE_CONN_HANDLE_INVALID)
	{
		accessory_data_send(g_accessory_running.data_valid);
	}
	else if(g_accessory_cycling.info.conn_handle != BLE_CONN_HANDLE_INVALID)
	{
		accessory_data_send(g_accessory_cycling.data_valid);
	}
	
	g_accessory_heartrate.data_valid = false;
	g_accessory_running.data_valid = false;
	g_accessory_cycling.data_valid = false;


	task_ble_conn_handle(TASK_BLE_CONN_EVT_ACCESSORY_DATA_UPLOAD, NULL, 0);


}


static void timer_create(void)
{
	m_timer = xTimerCreate("ACCE",
                         TIMER_INTERVAL,
                         pdTRUE,
                         NULL,
                         timeout_handler);
	if (NULL == m_timer)
	{
			APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
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




extern ble_db_discovery_t m_db_disc[];
extern uint16_t g_conn_handle_peripheral;
extern ble_advertising_t m_advertising;
extern bool g_is_central_mode;

static void task_ble_accessory_evt_adv_report(void);	

_accessory_running g_accessory_running = {.info.uuid = BLE_UUID_RUNNING_SPEED_AND_CADENCE, .info.conn_handle = BLE_CONN_HANDLE_INVALID, .info.uuid_found = false, .info.name_found = false};
_accessory_heartrate g_accessory_heartrate = {.info.uuid = BLE_UUID_HEART_RATE_SERVICE, .info.conn_handle = BLE_CONN_HANDLE_INVALID, .info.uuid_found = false, .info.name_found = false};
_accessory_cycling g_accessory_cycling = {.info.uuid = BLE_UUID_CYCLING_SPEED_AND_CADENCE, .info.conn_handle = BLE_CONN_HANDLE_INVALID, .info.uuid_found = false, .info.name_found = false};



void task_ble_accessary_handle(uint32_t evt, uint8_t *p_data, uint32_t length)
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
    NRF_LOG_DEBUG("task");
    (void)pvParameter;	
    while (1)
    {
			if (xQueueReceive(QueueTask, &queue, portMAX_DELAY) == pdTRUE)
			{	
				switch(queue.evt)
				{
					case TASK_BLE_ACCESSORY_EVT_ADV_REPORT:{
						task_ble_accessory_evt_adv_report();
					}break;
					case TASK_BLE_ACCESSORY_EVT_SCAN:{
						task_ble_accessory_scan_start();
					}break;			
					case TASK_BLE_ACCESSORY_EVT_SCAN_ENABLE:{
						task_ble_accessory_evt_scan_enable();
					}break;			
					case TASK_BLE_ACCESSORY_EVT_SCAN_DISABLE:{
						NRF_LOG_DEBUG("TASK_BLE_ACCESSORY_EVT_SCAN_DISABLE");
						task_ble_accessory_evt_scan_disable();
					}break;						
					case TASK_BLE_ACCESSORY_EVT_HEARTRATE_SRV_DISC:{
						NRF_LOG_DEBUG("TASK_BLE_ACCESSORY_EVT_HEARTRATE_SRV_DISC");
						uint32_t error = ble_db_discovery_start(&m_db_disc[DISCONVER_ARRAY_HEARTRATE], g_accessory_heartrate.info.conn_handle);
//            if (error != NRF_ERROR_BUSY)
            {
                APP_ERROR_CHECK(error);
            }
					}break;							
					case TASK_BLE_ACCESSORY_EVT_RUNNING_SRV_DISC:{
						NRF_LOG_DEBUG("TASK_BLE_ACCESSORY_EVT_RUNNING_SRV_DISC");
						uint32_t error = ble_db_discovery_start(&m_db_disc[DISCONVER_ARRAY_RUNNING], g_accessory_running.info.conn_handle);
//            if (error != NRF_ERROR_BUSY)
            {
                APP_ERROR_CHECK(error);
            }							
					}break;							
					case TASK_BLE_ACCESSORY_EVT_CYCLING_SRV_DISC:{
						NRF_LOG_DEBUG("TASK_BLE_ACCESSORY_EVT_CYCLING_SRV_DISC");
						uint32_t error = ble_db_discovery_start(&m_db_disc[DISCONVER_ARRAY_CYCLING], g_accessory_cycling.info.conn_handle);
//            if (error != NRF_ERROR_BUSY)
            {
                APP_ERROR_CHECK(error);
            }							
					}break;	
					
					case TASK_BLE_ACCESSORY_EVT_DATA_TIMER_START:{
						NRF_LOG_DEBUG("TASK_BLE_ACCESSORY_EVT_DATA_TIMER_START");
						timer_start(1000);				
					}break;						
					case TASK_BLE_ACCESSORY_EVT_DATA_TIMER_STOP:{
						NRF_LOG_DEBUG("TASK_BLE_ACCESSORY_EVT_DATA_TIMER_STOP");
						timer_stop();				
					}break;						
					
				
					
					
					
					default:{
					}break;
				}
			}
    }
}

void task_ble_accessary_init(void * p_context)
{
  NRF_LOG_DEBUG("task_ble_accessary_init");
    
    
	BaseType_t xReturned = xTaskCreate(task,
																		 "ACES",
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

	timer_create();
}


void task_ble_accessory_scan_start(void)
{
	ret_code_t err_code;
	ble_gap_scan_params_t scan_params;
	scan_params.active = 1;
	scan_params.interval = 0xA0;
	scan_params.window = 0x50;
	scan_params.timeout = 0x0000;
	scan_params.use_whitelist = 0;
	
	
	(void) sd_ble_gap_scan_stop();

	err_code = sd_ble_gap_scan_start(&scan_params);
	// It is okay to ignore this error since we are stopping the scan anyway.
	if (err_code != NRF_ERROR_INVALID_STATE)
	{
			APP_ERROR_CHECK(err_code);
	}
	NRF_LOG_DEBUG("task_ble_accessory:sd_ble_gap_scan_start");
}


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


static bool find_adv_uuid(const ble_gap_evt_adv_report_t *p_adv_report, const uint16_t uuid_to_find)
{
    ret_code_t err_code;
    data_t     adv_data;
    data_t     type_data;

    // Initialize advertisement report for parsing.
    adv_data.p_data     = (uint8_t *)p_adv_report->data;
    adv_data.data_len   = p_adv_report->dlen;

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
            return true;
        }
    }
    return false;
}

static void find_ble_dev(ble_gap_evt_adv_report_t const * p_adv_report, _accessory_info *dev_info)
{
//	ret_code_t err_code;
	ble_gap_addr_t p_addr 					= p_adv_report->peer_addr;
//	data_t adv_data;
//	data_t dev_name;

	// Initialize advertisement report for parsing
//	adv_data.p_data   = (uint8_t *)p_adv_report->data;
//	adv_data.data_len = p_adv_report->dlen;	

	if(!dev_info->uuid_found)
	{
		if (find_adv_uuid(p_adv_report, dev_info->uuid))
		{
			memcpy(&dev_info->addr,&p_addr,sizeof(ble_gap_addr_t));
			dev_info->uuid_found = true;
		}		
	}


//	if(!dev_info->name_found && dev_info->uuid_found)
//	{	
//		if(!memcmp(p_addr.addr,dev_info->addr.addr,BLE_GAP_ADDR_LEN))
//		{
//			err_code = adv_report_parse(BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME, &adv_data, &dev_name);
//			if (err_code == NRF_SUCCESS)
//			{
//				memset(dev_info->name,0,sizeof(dev_info->name));
//				memcpy(dev_info->name,dev_name.p_data,dev_name.data_len);
//				dev_info->name_found = true;
//			}
//			else
//			{
//				// Look for the short local name if it was not found as complete
//				err_code = adv_report_parse(BLE_GAP_AD_TYPE_SHORT_LOCAL_NAME, &adv_data, &dev_name);
//				if (err_code == NRF_SUCCESS)
//				{
//					memset(dev_info->name,0,sizeof(dev_info->name));
//					memcpy(dev_info->name,dev_name.p_data,dev_name.data_len);
//					dev_info->name_found = true;
//				}
//			}			
//		}
//	}	
}

static void accessory_connect(ble_gap_addr_t const *p_peer_addr)
{
		ret_code_t            err_code;
		(void) sd_ble_gap_scan_stop();
		ble_gap_scan_params_t const scan_param =
		{
			.active   = 1,
			.interval = 0x00A0,
			.window   = 0x0050,
			.timeout  = 0x0190,
			.use_whitelist = 0,
		};

		ble_gap_conn_params_t const connection_param =
		{
			MSEC_TO_UNITS(20, UNIT_1_25_MS),
			MSEC_TO_UNITS(200, UNIT_1_25_MS),
			0,
			MSEC_TO_UNITS(6000, UNIT_10_MS),
		};	

		err_code = sd_ble_gap_connect(p_peer_addr,
																	&scan_param,
																	&connection_param,
																	1);
		if (err_code != NRF_SUCCESS)
		{
				NRF_LOG_ERROR("Connection Request Failed, reason %d.", err_code);
		}	
}

static void task_ble_accessory_evt_adv_report(void)
{
	ble_gap_evt_adv_report_t *report = (ble_gap_evt_adv_report_t *)queue.p_data;	
	
	if(g_accessory_running.info.conn_handle == BLE_CONN_HANDLE_INVALID)
	{
		find_ble_dev(report, &g_accessory_running.info);
	}			
	if(g_accessory_heartrate.info.conn_handle == BLE_CONN_HANDLE_INVALID)
	{
		find_ble_dev(report, &g_accessory_heartrate.info);
	}		
	if(g_accessory_cycling.info.conn_handle == BLE_CONN_HANDLE_INVALID)
	{
		find_ble_dev(report, &g_accessory_cycling.info);
	}			
	
	if(g_accessory_running.info.uuid_found && g_accessory_running.info.conn_handle == BLE_CONN_HANDLE_INVALID)
	{
		NRF_LOG_INFO("=============g_accessory_running=================");
		NRF_LOG_INFO("device addr --> %02X:%02X:%02X:%02X:%02X:%02X",g_accessory_running.info.addr.addr[0],
																																	g_accessory_running.info.addr.addr[1],
																																	g_accessory_running.info.addr.addr[2],
																																	g_accessory_running.info.addr.addr[3],
																																	g_accessory_running.info.addr.addr[4],
																																	g_accessory_running.info.addr.addr[5]);					
		accessory_connect(&g_accessory_running.info.addr);
	}	
	if(g_accessory_heartrate.info.uuid_found && g_accessory_heartrate.info.conn_handle == BLE_CONN_HANDLE_INVALID)
	{
		NRF_LOG_INFO("=============g_accessory_heartrate=================");
		NRF_LOG_INFO("device addr --> %02X:%02X:%02X:%02X:%02X:%02X",g_accessory_heartrate.info.addr.addr[0],
																																	g_accessory_heartrate.info.addr.addr[1],
																																	g_accessory_heartrate.info.addr.addr[2],
																																	g_accessory_heartrate.info.addr.addr[3],
																																	g_accessory_heartrate.info.addr.addr[4],
																																	g_accessory_heartrate.info.addr.addr[5]);					
		accessory_connect(&g_accessory_heartrate.info.addr);
	}		
	if(g_accessory_cycling.info.uuid_found && g_accessory_cycling.info.conn_handle == BLE_CONN_HANDLE_INVALID)
	{
		NRF_LOG_INFO("=============g_accessory_cycling=================");
		NRF_LOG_INFO("device addr --> %02X:%02X:%02X:%02X:%02X:%02X",g_accessory_cycling.info.addr.addr[0],
																																	g_accessory_cycling.info.addr.addr[1],
																																	g_accessory_cycling.info.addr.addr[2],
																																	g_accessory_cycling.info.addr.addr[3],
																																	g_accessory_cycling.info.addr.addr[4],
																																	g_accessory_cycling.info.addr.addr[5]);					
		accessory_connect(&g_accessory_cycling.info.addr);
	}			
}	



static void task_ble_accessory_evt_scan_enable(void)						
{
	uint32_t err_code;
	ble_advertising_auto_reset();
	NRF_LOG_DEBUG("task_ble_accessory:g_conn_handle_peripheral=%d",g_conn_handle_peripheral);
	if(g_conn_handle_peripheral != BLE_CONN_HANDLE_INVALID)
	{
		err_code = sd_ble_gap_disconnect(g_conn_handle_peripheral,
																		 BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
		APP_ERROR_CHECK(err_code);	
		g_conn_handle_peripheral = BLE_CONN_HANDLE_INVALID;
		NRF_LOG_DEBUG("task_ble_accessory:sd_ble_gap_disconnect");
	}
//	err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_IDLE);//�رչ㲥
//	if(err_code != NRF_ERROR_INVALID_STATE)
//	{
//		APP_ERROR_CHECK(err_code);
//	}

	task_ble_accessory_scan_start();
	g_is_central_mode = true;

	
}


static void task_ble_accessory_evt_scan_disable(void)
{
	uint32_t err_code;
	timer_stop();
	(void) sd_ble_gap_scan_stop();
	if(g_accessory_heartrate.info.conn_handle != BLE_CONN_HANDLE_INVALID)
	{
		err_code = sd_ble_gap_disconnect(g_accessory_heartrate.info.conn_handle,
																		 BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
		APP_ERROR_CHECK(err_code);	
		g_accessory_heartrate.info.conn_handle = BLE_CONN_HANDLE_INVALID;						
	}
	if(g_accessory_running.info.conn_handle != BLE_CONN_HANDLE_INVALID)
	{
		err_code = sd_ble_gap_disconnect(g_accessory_running.info.conn_handle,
																		 BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
		APP_ERROR_CHECK(err_code);	
		g_accessory_running.info.conn_handle = BLE_CONN_HANDLE_INVALID;						
	}
	if(g_accessory_cycling.info.conn_handle != BLE_CONN_HANDLE_INVALID)
	{
		err_code = sd_ble_gap_disconnect(g_accessory_cycling.info.conn_handle,
																		 BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
		APP_ERROR_CHECK(err_code);	
		g_accessory_cycling.info.conn_handle = BLE_CONN_HANDLE_INVALID;						
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





