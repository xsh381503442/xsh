#include "drv_apollo2.h"
#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "app_timer.h"
#include "nrf_peripherals.h"
#ifdef LAYER_APPLICATION 
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "portmacro.h"
#include "portable.h"
#include "event_groups.h"
#include "semphr.h"
#include "ble_advertising.h"
#include "ble_srv_nrf52.h"
#include "ble_conn_params.h"

#endif
#if defined (UART_PRESENT)
#include "nrf_uart.h"
#endif
#if defined (UARTE_PRESENT)
#include "nrf_uarte.h"
#endif
#include "nrf_drv_gpiote.h"
#include "nrf_delay.h"
#include "cmd_apollo2.h"


#define LOG_LEVEL_OFF 							0
#define LOG_LEVEL_ERROR 						1
#define LOG_LEVEL_WARNING 					2
#define LOG_LEVEL_INFO 							3
#define LOG_LEVEL_DEBUG 						4
#define NRF_LOG_MODULE_NAME drv_apollo2
#define NRF_LOG_LEVEL       LOG_LEVEL_INFO
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
NRF_LOG_MODULE_REGISTER();


#ifdef LAYER_BOOTLOADER  
#define DELAY_MS(X)                 nrf_delay_ms(X)
#endif

#ifdef LAYER_APPLICATION 
#define DELAY_MS(X)                 nrf_delay_ms(X)
//#define DELAY_MS(X)                 vTaskDelay(X)
//#define DELAY_MS(X)                   do{TickType_t xLastWakeTime;xLastWakeTime = xTaskGetTickCount ();vTaskDelayUntil( &xLastWakeTime, X / ( ( TickType_t ) 1000 / 3 ) );}while(0)
#endif


static drv_apollo2_callback m_callback;


#define DISCONNECT                0
#define CONNECTING                1
#define CONNECT                   2
#define DISCONNECTING             3
static volatile uint8_t m_state = DISCONNECT; 
static apollo2_user_callback m_user_cb = NULL;
#if defined (LAYER_APPLICATION) 
uint8_t cod_uart_normal_open = 0;
#endif


void ble_conn_params_set_conn_params(ble_adv_evt_t new_adv_params)
{

		extern ble_config_params_t     *g_ble_config;
		uint16_t watch_min_interval = g_ble_config->conn_params.min_conn_int;
		uint16_t watch_max_interval = g_ble_config->conn_params.max_conn_int;

		ble_gap_conn_params_t p_new_params;
		
		 switch (new_adv_params)
    	{
       		 case BLE_ADV_EVT_FAST:
			{
				p_new_params.max_conn_interval = MAX_CONN_INTERVAL_FAST;
				p_new_params.min_conn_interval = MIN_CONN_INTERVAL_FAST;
				p_new_params.slave_latency = SLAVE_LATENCY_FAST;
				p_new_params.conn_sup_timeout = CONN_SUP_TIMEOUT_FAST;
        	}
            break;
        	case BLE_ADV_EVT_SLOW:
			{
				p_new_params.max_conn_interval = MAX_CONN_INTERVAL_SLOW;
				p_new_params.min_conn_interval = MIN_CONN_INTERVAL_SLOW;
				p_new_params.slave_latency = SLAVE_LATENCY_SLOW;
				p_new_params.conn_sup_timeout = CONN_SUP_TIMEOUT_SLOW;
        	}
            break;
        default:
			return;
            //break;
         }
		
		if((watch_min_interval != p_new_params.min_conn_interval) && 
			 (watch_max_interval != p_new_params.max_conn_interval))
		{	
			uint16_t err_code;
			extern uint16_t g_conn_handle_peripheral;
			err_code = ble_conn_params_stop();
			APP_ERROR_CHECK(err_code);
			err_code = ble_conn_params_change_conn_params(g_conn_handle_peripheral, &p_new_params);
			
			if(err_code != NRF_SUCCESS)
			{
				NRF_LOG_ERROR("ble_conn_params_set_conn_params --> 0x%08X",err_code);
			}
		
		}
		
}


static struct {
	uint8_t buffer[UART_TX_BUF_SIZE];
	uint32_t length;
}m_tx_buffer;
static struct {
	uint8_t buffer[3][UART_RX_BUF_SIZE];
	uint32_t length;
}m_rx_buffer;

//static bool m_tx_finish_flag = false;


static void event_handle(app_uart_evt_t * p_event)
{
	static uint32_t i = 0;	
	static uint32_t index = 0;	
    switch (p_event->evt_type)
    {
        case APP_UART_DATA_READY:{
					
            UNUSED_VARIABLE(app_uart_get(&m_rx_buffer.buffer[i][index]));

						if(m_rx_buffer.buffer[i][0] == UART_PACKET_HEADER)
						{
							index++;
							if(index >= (UART_RX_BUF_SIZE))
							{
								m_rx_buffer.length = index;
								index = 0;
							
								if(m_user_cb != NULL)
								{
									m_user_cb(APP_UART_DATA_READY,m_rx_buffer.buffer[i],m_rx_buffer.length);
								}								
								m_callback(APP_UART_DATA_READY,m_rx_buffer.buffer[i],m_rx_buffer.length);
								i++;
								if (i >=3)
								{
									i =0;
								}
							}							
						}
					
	
					} break;
        case APP_UART_TX_EMPTY:{
            NRF_LOG_ERROR("APP_UART_TX_EMPTY");
//						m_tx_finish_flag = true;
						index = 0;
						//m_callback(APP_UART_TX_EMPTY,NULL,0);
				}break;
        case APP_UART_COMMUNICATION_ERROR:{
					NRF_LOG_ERROR("APP_UART_COMMUNICATION_ERROR");
						index = 0;					
//						m_callback(APP_UART_COMMUNICATION_ERROR,NULL,0);
//            APP_ERROR_HANDLER(p_event->data.error_communication);
				}break;

        case APP_UART_FIFO_ERROR:{
					NRF_LOG_ERROR("APP_UART_FIFO_ERROR");
						index = 0;
//						m_callback(APP_UART_FIFO_ERROR,NULL,0);
//            APP_ERROR_HANDLER(p_event->data.error_code);
				}break;

        default:
            break;
    }
}
static void uart_enable(void)
{
	NRF_LOG_DEBUG("uart_enable");
	
	uint32_t err_code;
	app_uart_comm_params_t const comm_params =
	{
			.rx_pin_no    = APOLLO2_RX_PIN_NUMBER,
			.tx_pin_no    = APOLLO2_TX_PIN_NUMBER,		
			.rts_pin_no   = NRF_UART_PSEL_DISCONNECTED,
			.cts_pin_no   = NRF_UART_PSEL_DISCONNECTED,			
			.flow_control = APP_UART_FLOW_CONTROL_DISABLED,
			.use_parity   = false,
			.baud_rate    = NRF_UART_BAUDRATE_115200
	};
	
	APP_UART_FIFO_INIT(&comm_params,
										 UART_RX_BUF_SIZE*2,
										 UART_TX_BUF_SIZE,
										 event_handle,
										 APP_IRQ_PRIORITY_HIGHEST,
										 err_code);
	APP_ERROR_CHECK(err_code);
	
//	err_code = app_uart_get(&m_rx_buffer.buffer[0]);
//	if (err_code != NRF_SUCCESS)
//	{
//			NRF_LOG_DEBUG("uart enable read rx to ensure rx is pull high");
//	}	
}
static void uart_disable(void)
{
	NRF_LOG_WARNING("uart_disable");
	uint32_t err_code;
	err_code = app_uart_close();
	APP_ERROR_CHECK(err_code);	
	
}

static void send(void)
{
//	uint32_t err_code;
	for (uint32_t i = 0; i < (UART_TX_BUF_SIZE); i++)
	{
		app_uart_put(m_tx_buffer.buffer[i]);
//		do
//		{
//				err_code = app_uart_put(m_tx_buffer.buffer[i]);
//				if ((err_code != NRF_SUCCESS) && (err_code != NRF_ERROR_BUSY))
//				{
//						APP_ERROR_CHECK(err_code);
//				}
//		} while (err_code == NRF_ERROR_BUSY);		
		
	}	
}
void drv_apollo2_on_cts_high(void)
{
	NRF_LOG_INFO("drv_apollo2_on_cts_high");
	NRF_LOG_INFO("m_state= %d",m_state);

	if(m_state == CONNECTING)
	{
		NRF_LOG_INFO("m_state = CONNECTING");
		m_state = CONNECT;
		NRF_LOG_INFO("m_state = CONNECT");
		send();
	}
	else if(m_state == DISCONNECT)
	{
		NRF_LOG_INFO("m_state = DISCONNECT");
		m_state = CONNECT;
		NRF_LOG_INFO("m_state = CONNECT");
		uart_enable();	
		nrf_gpio_pin_set(APOLLO2_RTS_PIN_NUMBER);	
		nrf_gpio_pin_clear(APOLLO2_RTS_PIN_NUMBER);
		nrf_gpio_pin_set(APOLLO2_RTS_PIN_NUMBER);			
	

	}
	else{
		NRF_LOG_DEBUG("Exception interrupt");
		NRF_LOG_INFO("m_state= %d",m_state);
		/**<Should never fall in here.>*/
		//异常让APOLLO复位蓝牙
//		nrf_gpio_pin_set(APOLLO2_RTS_PIN_NUMBER);	
//		nrf_gpio_pin_clear(APOLLO2_RTS_PIN_NUMBER);
//		nrf_gpio_pin_set(APOLLO2_RTS_PIN_NUMBER);	
//		while(1);
	}	
	
}

void drv_apollo2_on_cts_low(void)
{
	NRF_LOG_INFO("drv_apollo2_on_cts_low");
	if(m_state == CONNECT)
	{	
	#if defined (LAYER_APPLICATION) 
		if (cod_uart_normal_open == 1)
		{
			ble_conn_params_set_conn_params(BLE_ADV_EVT_SLOW);
		}	
		cod_uart_normal_open = 0;
	#endif
		NRF_LOG_INFO("m_state = CONNECT");
		uart_disable();
		nrf_gpio_pin_clear(APOLLO2_RTS_PIN_NUMBER);
		nrf_gpio_pin_set(APOLLO2_RTS_PIN_NUMBER);
		nrf_gpio_pin_clear(APOLLO2_RTS_PIN_NUMBER);

		
		m_state = DISCONNECT;
		NRF_LOG_WARNING("m_state = DISCONNECT");
	}
	else if(m_state == DISCONNECTING)
	{
		NRF_LOG_INFO("m_state = DISCONNECTING");
		m_state = DISCONNECT;
		NRF_LOG_INFO("m_state = DISCONNECT");
	}
	else{
		NRF_LOG_DEBUG("Exception interrupt");
		/**<Should never fall in here.>*/
//		nrf_gpio_pin_set(APOLLO2_RTS_PIN_NUMBER);	
//		nrf_gpio_pin_clear(APOLLO2_RTS_PIN_NUMBER);
//		nrf_gpio_pin_set(APOLLO2_RTS_PIN_NUMBER);	
//		while(1);

		NRF_LOG_INFO("m_state= %d",m_state);
		if(m_state == CONNECTING) 
		{
			m_state = CONNECT;
		}
	}
}
static void gpiote_event_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	m_callback(APP_UART_CTS_INTERRUPT,NULL,0);
}



void drv_apollo2_init(drv_apollo2_callback callback)
{
	NRF_LOG_DEBUG("drv_apollo2_init");
//	memset(&m_tx_buffer,0,sizeof(m_tx_buffer));
//	memset(&m_rx_buffer,0,sizeof(m_rx_buffer));
	
	m_callback = callback;
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
	config.sense = NRF_GPIOTE_POLARITY_TOGGLE;  	
	err_code = nrf_drv_gpiote_in_init(APOLLO2_CTS_PIN_NUMBER, &config, gpiote_event_handler);
	APP_ERROR_CHECK(err_code);	
	nrf_drv_gpiote_in_event_enable(APOLLO2_CTS_PIN_NUMBER, true);
	nrf_gpio_cfg_output(APOLLO2_RTS_PIN_NUMBER);	
	nrf_gpio_pin_clear(APOLLO2_RTS_PIN_NUMBER);
	

}

uint32_t drv_ble_send_data(_uart_data_param *param)
{
	NRF_LOG_INFO("drv_ble_send_data,%x,%x",param->p_data[1],param->p_data[4]);
#if defined (LAYER_APPLICATION) 
	if (cod_uart_normal_open)
	{
		if (cod_uart_normal_open == 1)
		{
			memset(m_tx_buffer.buffer,0,UART_TX_BUF_SIZE);
			m_tx_buffer.length = param->length;
			memcpy(m_tx_buffer.buffer,param->p_data,param->length);
			send();
			return NRF_SUCCESS;
		}
		else
		{
			return NRF_ERROR_BUSY;
		}
	}
#endif
	if(m_state != DISCONNECT || nrf_drv_gpiote_in_is_set(APOLLO2_CTS_PIN_NUMBER))
	{
		NRF_LOG_WARNING("NRF_ERROR_BUSY");
		NRF_LOG_WARNING("m_state = %d",m_state);
		if(nrf_drv_gpiote_in_is_set(APOLLO2_CTS_PIN_NUMBER)){
				NRF_LOG_DEBUG("nrf_drv_gpiote_in_is_set");
		}
		
		return NRF_ERROR_BUSY;
	}
	m_state = CONNECTING;
	NRF_LOG_DEBUG("m_state = CONNECTING");
	m_user_cb = NULL;
	if(param->user_callback != NULL)
	{
		m_user_cb = param->user_callback;
	}
	if(param->user_callback != NULL)
	{
		m_user_cb = param->user_callback;
	}
	memset(m_tx_buffer.buffer,0,UART_TX_BUF_SIZE);
	m_tx_buffer.length = param->length;
	memcpy(m_tx_buffer.buffer,param->p_data,param->length);
	
	uart_enable();
	nrf_gpio_pin_set(APOLLO2_RTS_PIN_NUMBER);	
	nrf_gpio_pin_clear(APOLLO2_RTS_PIN_NUMBER);
	nrf_gpio_pin_set(APOLLO2_RTS_PIN_NUMBER);
	
	return NRF_SUCCESS;
}

uint32_t drv_apollo2_uart_put(uint8_t *p_data, uint32_t length)
{
	NRF_LOG_INFO("drv_ble_uart_put");
#if defined (LAYER_APPLICATION) 

	if(cod_uart_normal_open == 0)
#endif
	{
		if(m_state != CONNECT)
		{
			NRF_LOG_DEBUG("NRF_ERROR_INVALID_STATE");
			return NRF_ERROR_INVALID_STATE;
		}
	}
	memset(m_tx_buffer.buffer,0,UART_TX_BUF_SIZE);
	memcpy(m_tx_buffer.buffer,p_data,length);
	
	send();
//	m_tx_finish_flag = false;
//	while(!m_tx_finish_flag)
//	{
//		DELAY_MS(30);
//	}
	NRF_LOG_DEBUG("drv_ble_uart_put finish");


	return NRF_SUCCESS;
}


void drv_apollo2_uart_close(void)
{
	//NRF_LOG_ERROR("drv_apollo2_uart_close %d",cod_uart_normal_open);
#if defined (LAYER_APPLICATION) 
	if (cod_uart_normal_open)
	{
		return ;
	}
	#endif

	uart_disable();
	m_state = DISCONNECTING;
	NRF_LOG_INFO("m_state = DISCONNECTING");	
	nrf_gpio_pin_clear(APOLLO2_RTS_PIN_NUMBER);
	nrf_gpio_pin_set(APOLLO2_RTS_PIN_NUMBER);	
	nrf_gpio_pin_clear(APOLLO2_RTS_PIN_NUMBER);
		
//	vTaskDelay(200);//等待APOLLO关闭串口，因为是异步操作


}

uint8_t drv_apollo2_send(_uart_data_param *param)
{
	uint8_t count = TASK_UART_DELAY_COUNT_OUT;
	while(count && drv_ble_send_data(param) != NRF_SUCCESS )
	{
		count--;
		DELAY_MS(TASK_UART_DELAY);
	}	
	return count;
}


uint8_t read_m_state(void)
{
	return m_state;
}




