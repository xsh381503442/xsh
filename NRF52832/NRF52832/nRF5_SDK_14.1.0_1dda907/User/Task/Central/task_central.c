#include "task_central.h"


#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "portmacro.h"
#include "portable.h"
#include "event_groups.h"
#include "semphr.h"


#include "task_ble_conn.h"
#include "cmd_apollo2.h"

#define LOG_LEVEL_OFF 							0
#define LOG_LEVEL_ERROR 						1
#define LOG_LEVEL_WARNING 					2
#define LOG_LEVEL_INFO 							3
#define LOG_LEVEL_DEBUG 						4
#define NRF_LOG_MODULE_NAME task_central
#define NRF_LOG_LEVEL       LOG_LEVEL_DEBUG
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
NRF_LOG_MODULE_REGISTER();


//#define DELAY_MS(X)                 nrf_delay_ms(X)
#define DELAY_MS(X)                 vTaskDelay(X)
//#define DELAY_MS(X)                   do{TickType_t xLastWakeTime;xLastWakeTime = xTaskGetTickCount ();vTaskDelayUntil( &xLastWakeTime, X );}while(0)
#define UUID16_EXTRACT(DST, SRC) \
    do                           \
    {                            \
        (*(DST))   = (SRC)[1];   \
        (*(DST)) <<= 8;          \
        (*(DST))  |= (SRC)[0];   \
    } while (0)
		
static TaskHandle_t task_handle;
static QueueHandle_t QueueTaskBleConn = NULL;


struct s_queue_task_buffer{
	uint32_t	evt;
	uint32_t length;
	uint8_t	*p_data;
	
};
typedef struct s_queue_task_buffer _queue_task_buffer;
typedef struct
{
    uint8_t  * p_data;      /**< Pointer to data. */
    uint16_t   data_len;    /**< Length of data. */
} data_t;	
_central_controller g_central_controller;

void task_central_handle(uint32_t evt, uint8_t *p_data, uint32_t length)
{
	NRF_LOG_INFO("task_central_handle");

	_queue_task_buffer  queue = {0};
	queue.evt = evt;
	queue.p_data = p_data;
	queue.length = length;
	BaseType_t xReturned = xQueueSend(QueueTaskBleConn, &queue, portMAX_DELAY );	
	if (xReturned != pdPASS)
	{
			APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
	}	

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
static bool find_target_dev(ble_gap_evt_adv_report_t const * p_adv_report)
{
	ret_code_t err_code;
	ble_gap_addr_t p_addr 					= p_adv_report->peer_addr;
	int8_t rssi 										= p_adv_report->rssi;
	data_t adv_data;
	data_t dev_name;
	// Initialize advertisement report for parsing
	adv_data.p_data   = (uint8_t *)p_adv_report->data;
	adv_data.data_len = p_adv_report->dlen;	


	
	if (find_adv_uuid(p_adv_report, g_central_controller.param.uuid))
	{
		NRF_LOG_DEBUG("FOUND TARGET DEVICE");
		g_central_controller.scan_dev[0].rssi = rssi;
		memcpy(&g_central_controller.scan_dev[0].addr,&p_addr,sizeof(ble_gap_addr_t));
		NRF_LOG_INFO("MAC :");
		NRF_LOG_HEXDUMP_DEBUG(g_central_controller.scan_dev[0].addr.addr, 6);
		err_code = adv_report_parse(BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME, &adv_data, &dev_name);
		if (err_code == NRF_SUCCESS)
		{
			NRF_LOG_DEBUG("FOUND TARGET DEVICE NAME");
			memset(g_central_controller.scan_dev[0].name.name,0,sizeof(g_central_controller.scan_dev[0].name.name));
			memcpy(g_central_controller.scan_dev[0].name.name,dev_name.p_data,dev_name.data_len);
			NRF_LOG_HEXDUMP_DEBUG(g_central_controller.scan_dev[0].name.name, 10);
			g_central_controller.scan_dev[0].name.length = dev_name.data_len;
			NRF_LOG_DEBUG("FOUND TARGET DEVICE NAME LEN --> %d",g_central_controller.scan_dev[0].name.length);
		} 
		return true;
	}	
	return false;
}

static void task(void * pvParameter)
{
    NRF_LOG_DEBUG("task");
		(void)pvParameter;
		static _queue_task_buffer queue = {0,NULL,0};
		while (1)
		{
			if (xQueueReceive(QueueTaskBleConn, &queue, portMAX_DELAY) == pdTRUE)
			{
				switch(queue.evt&0x0000FF00)			
				{
	
					case PC_UART_COMMAND_HARDWARE_TEST_BLE_RSSI_READ:{
					
						if(find_target_dev((ble_gap_evt_adv_report_t *)queue.p_data))
						{
							task_ble_conn_handle(TASK_BLE_CONN_HWT_RSSI_READ, NULL, 0);	
						}
					
					}break;
					
					
					
					default:{
						
					}break;
				}			
			}

					
    }
}


void task_central_init(void * p_context)
{
  NRF_LOG_DEBUG("task_central_init");

	BaseType_t xReturned = xTaskCreate(task,
																		 "CENTRAL",
																		 256,
																		 p_context,
																		 APP_IRQ_PRIORITY_LOW,
																		 &task_handle);
	if (xReturned != pdPASS)
	{
			NRF_LOG_ERROR("task not created.");
			APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
	}

	QueueTaskBleConn = xQueueCreate( 10,sizeof( _queue_task_buffer ));
}






