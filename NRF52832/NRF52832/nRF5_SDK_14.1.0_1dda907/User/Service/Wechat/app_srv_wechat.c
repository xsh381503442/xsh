#include "app_srv_wechat.h"
#include "ble_srv_wechat.h"


#include "nrf_delay.h"
#include "FreeRTOS.h"
#include "timers.h"

#include "task_ble_conn.h"
#ifndef COD 
#define LOG_LEVEL_OFF 							0
#define LOG_LEVEL_ERROR 						1
#define LOG_LEVEL_WARNING 					2
#define LOG_LEVEL_INFO 							3
#define LOG_LEVEL_DEBUG 						4
#define NRF_LOG_MODULE_NAME app_srv_wechat
#define NRF_LOG_LEVEL       LOG_LEVEL_DEBUG
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
NRF_LOG_MODULE_REGISTER();


#define DELAY_MS(X)                 nrf_delay_ms(X)
//#define DELAY_MS(X)                 vTaskDelay(X)
//#define DELAY_MS(X)                   do{TickType_t xLastWakeTime;xLastWakeTime = xTaskGetTickCount ();vTaskDelayUntil( &xLastWakeTime, X / ( ( TickType_t ) 1000 / 3 ) );}while(0)







BLE_WECHAT_DEF(m_wechat);

_wechat_pedo wechat_pedo = {.step = 15000};
_wechat_target wechat_target = {.step = 0};


#define TIMER_INTERVAL            500 
#define OSTIMER_WAIT_FOR_QUEUE              2 
static TimerHandle_t m_timer; 

static void timeout_handler(TimerHandle_t xTimer)
{
	UNUSED_PARAMETER(xTimer);
	NRF_LOG_DEBUG("timeout_handler");
	task_ble_conn_handle(TASK_BLE_CONN_WECHAT_SPORT_DATA_REQUEST, NULL, 0);	
}


static void timer_create(void)
{
	m_timer = xTimerCreate("WECT",
																 TIMER_INTERVAL,
																 pdFALSE,
																 NULL,
																 timeout_handler);
	if (NULL == m_timer)
	{
			APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
	}	
}
static void timer_start(void)
{
	if (pdPASS != xTimerStart(m_timer, OSTIMER_WAIT_FOR_QUEUE))
	{
			APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
	}
}

static void timer_stop(void)
{
	if (pdPASS != xTimerStop(m_timer, OSTIMER_WAIT_FOR_QUEUE))
	{
			APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
	}
}

static void wechat_evt_handler(ble_wechat_evt_t * p_evt)
{

	switch(p_evt->type)
	{
		case BLE_WECHAT_EVT_PEDO_MEAS_NOTIFY_ENABLE:{
			NRF_LOG_DEBUG("BLE_WECHAT_EVT_PEDO_MEAS_NOTIFY_ENABLE");
			//timer_start();
		}break;
		case BLE_WECHAT_EVT_PEDO_MEAS_NOTIFY_DISABLE:{
			NRF_LOG_DEBUG("BLE_WECHAT_EVT_PEDO_MEAS_NOTIFY_DISABLE");
			timer_stop();
		}break;		
		case BLE_WECHAT_EVT_TARGET_INDICATE_ENABLE:{
			NRF_LOG_DEBUG("BLE_WECHAT_EVT_TARGET_INDICATE_ENABLE");
			wechat_target.flag = (1<<WECHAT_STEP);
			uint8_t data[4];
			data[0] = wechat_target.flag;
			data[1] = (uint8_t)(wechat_target.step>>0);
			data[2] = (uint8_t)(wechat_target.step>>8);
			data[3] = (uint8_t)(wechat_target.step>>16);	
			app_srv_wechat_target_send(data,sizeof(data));
		}break;		
		case BLE_WECHAT_EVT_TARGET_INDICATE_DISABLE:{
			NRF_LOG_DEBUG("BLE_WECHAT_EVT_TARGET_INDICATE_DISABLE");
		
			
		}break;		
		case BLE_WECHAT_EVT_RX_DATA:{
			NRF_LOG_DEBUG("BLE_WECHAT_EVT_RX_DATA");
		
		}break;
		case BLE_WECHAT_EVT_TARGET_SET:{
			NRF_LOG_DEBUG("BLE_WECHAT_EVT_TARGET_SET");

		}break;
		
		case BLE_WECHAT_EVT_DISCONNECT:{
			NRF_LOG_DEBUG("BLE_WECHAT_EVT_DISCONNECT");
			timer_stop();
		}break;	

		case BLE_WECHAT_EVT_READ_DATA:{
			NRF_LOG_DEBUG("BLE_WECHAT_EVT_READ_DATA");
			timer_start();
		}
		
		
		default:
			break;
	}
}



void app_srv_wechat_init(void)
{
	NRF_LOG_DEBUG("app_srv_wechat_init");
	uint32_t       err_code;
	ble_wechat_init_t wechat_init;
	memset(&wechat_init, 0, sizeof(wechat_init));
	wechat_init.data_handler = wechat_evt_handler;
	err_code = ble_wechat_init(&m_wechat, &wechat_init);
	APP_ERROR_CHECK(err_code);	

	timer_create();
	
}
void app_srv_wechat_pedo_meas_send(uint8_t *p_data, uint32_t length)
{
	NRF_LOG_DEBUG("app_srv_wechat_pedo_meas_send");
	uint32_t bytes_sent = 0;
	uint16_t packet_length = 0;	
	uint32_t err_code;

	while(bytes_sent < length)
	{
		packet_length = (length - bytes_sent) > BLE_WECHAT_MAX_DATA_LEN ? BLE_WECHAT_MAX_DATA_LEN : length - bytes_sent;
		err_code = ble_wechat_pedo_meas_send(&m_wechat, p_data + bytes_sent, &packet_length);	
		
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
	NRF_LOG_DEBUG("app_srv_wechat_pedo_meas_send finish");
}



void app_srv_wechat_target_send(uint8_t *p_data, uint32_t length)
{
	NRF_LOG_DEBUG("app_srv_wechat_send");
	uint32_t bytes_sent = 0;
	uint16_t packet_length = 0;	
	uint32_t err_code;

	while(bytes_sent < length)
	{
		packet_length = (length - bytes_sent) > BLE_WECHAT_MAX_DATA_LEN ? BLE_WECHAT_MAX_DATA_LEN : length - bytes_sent;
		err_code = ble_wechat_target_send(&m_wechat, p_data + bytes_sent, &packet_length);	
		
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
	NRF_LOG_DEBUG("app_srv_wechat_send finish");
}
#endif


