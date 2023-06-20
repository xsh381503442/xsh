#include "drv_ble.h"
#include "rtt_log.h"
#include "lib_error.h"
#include "com_apollo2.h"
#include "SEGGER_RTT.h"

#include "am_mcu_apollo.h"
#include "drv_button.h"
#include "am_util_delay.h"

#include "cmd_ble.h"

 
#include <string.h>

#define MOUDLE_LOG_ENABLED 							1
#define MOUDLE_DEBUG_ENABLED   					0
#define MOUDLE_NAME                     "[DRV_BLE]:"

#if MOUDLE_LOG_ENABLED == 1 && RTT_LOG_ENABLED == 1
#define LOG_PRINTF(...)                      SEGGER_RTT_printf(0, ##__VA_ARGS__) 
#define LOG_HEXDUMP(p_data, len)             rtt_log_hexdump(p_data, len) 
#else
#define LOG_PRINTF(...) 
#define LOG_HEXDUMP(p_data, len)             
#endif

#if MOUDLE_DEBUG_ENABLED == 1 && RTT_LOG_ENABLED == 1
#define DEBUG_PRINTF(...)                    SEGGER_RTT_printf(0, ##__VA_ARGS__)
#define DEBUG_HEXDUMP(p_data, len)           rtt_log_hexdump(p_data, len)
#else
#define DEBUG_PRINTF(...)   
#define DEBUG_HEXDUMP(p_data, len) 
#endif





static ble_callback m_ble_cb;

static ble_user_callback m_user_cb = NULL;

#define DISCONNECT                0
#define CONNECTING                1
#define CONNECT                   2
#define DISCONNECTING             3
static volatile uint8_t m_state = DISCONNECT; 


static struct {
	uint8_t buffer[DRV_BLE_UART_TX_BUF_SIZE];
	uint32_t length;
}m_tx_buffer;
static struct {
	uint8_t buffer[DRV_BLE_UART_RX_BUF_SIZE/2];
	uint32_t length;
}m_rx_buffer;

static void uart_enable(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"uart_enable\n");
	am_hal_uart_config_t g_sUartConfig =
	{
			.ui32BaudRate = DRV_BLE_UART_BAUDRATE,
			.ui32DataBits = AM_HAL_UART_DATA_BITS_8,
			.bTwoStopBits = false,				
			.ui32Parity   = AM_HAL_UART_PARITY_NONE, 			
			.ui32FlowCtrl = AM_HAL_UART_FLOW_CTRL_NONE,    	
	};
	static uint8_t rx_buf[DRV_BLE_UART_TX_BUF_SIZE];
	static uint8_t tx_buf[DRV_BLE_UART_RX_BUF_SIZE];
	am_hal_uart_pwrctrl_enable(DRV_BLE_UART_MOUDLE);		
	am_hal_uart_clock_enable(DRV_BLE_UART_MOUDLE);
	am_hal_uart_disable(DRV_BLE_UART_MOUDLE);	
	am_hal_uart_config(DRV_BLE_UART_MOUDLE, &g_sUartConfig);
	am_hal_uart_fifo_config(DRV_BLE_UART_MOUDLE, AM_HAL_UART_RX_FIFO_1_2 | AM_HAL_UART_TX_FIFO_1_2);
	am_hal_uart_init_buffered(DRV_BLE_UART_MOUDLE, rx_buf, DRV_BLE_UART_RX_BUF_SIZE,tx_buf, DRV_BLE_UART_TX_BUF_SIZE);
	am_hal_uart_clock_enable(DRV_BLE_UART_MOUDLE);	
	am_hal_uart_enable(DRV_BLE_UART_MOUDLE);	
	am_hal_uart_int_clear(DRV_BLE_UART_MOUDLE, 0xFFFFFFFF);	
	am_hal_uart_int_enable(DRV_BLE_UART_MOUDLE, AM_HAL_UART_INT_RX_TMOUT|AM_HAL_UART_INT_RX);	
	PIN_CONFIG(DRV_BLE_UART_RX_PIN, AM_HAL_PIN_19_UART1RX);
	PIN_CONFIG(DRV_BLE_UART_TX_PIN, AM_HAL_PIN_18_UART1TX);
	am_hal_interrupt_enable(AM_HAL_INTERRUPT_UART + DRV_BLE_UART_MOUDLE);
	
//	char pcChar;
//	am_hal_uart_line_receive_polled(DRV_BLE_UART_MOUDLE,1,&pcChar);		
	//uart enable read rx to ensure rx is pull high
}
static void uart_disable(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"uart_disable\n");
	am_hal_uart_int_clear(DRV_BLE_UART_MOUDLE, 0xFFFFFFFF);	
	am_hal_uart_int_disable(DRV_BLE_UART_MOUDLE,0xFFFFFFFF);	
	am_hal_uart_disable(DRV_BLE_UART_MOUDLE);		
	am_hal_uart_clock_disable(DRV_BLE_UART_MOUDLE);
	am_hal_uart_pwrctrl_disable(DRV_BLE_UART_MOUDLE);
//	PIN_CONFIG(DRV_BLE_UART_RX_PIN, AM_HAL_PIN_DISABLE);
//	PIN_CONFIG(DRV_BLE_UART_TX_PIN, AM_HAL_PIN_DISABLE);
	
	PIN_CONFIG(DRV_BLE_UART_RX_PIN, AM_HAL_GPIO_INPUT|AM_HAL_GPIO_PULLUP);
	PIN_CONFIG(DRV_BLE_UART_TX_PIN, AM_HAL_GPIO_INPUT|AM_HAL_GPIO_PULLUP);	
	
}


static void send(void)
{
	for(uint32_t i=0;i<(DRV_BLE_UART_TX_BUF_SIZE);i++)
	{
		am_hal_uart_char_transmit_buffered(DRV_BLE_UART_MOUDLE, m_tx_buffer.buffer[i]);	
		while ( am_hal_uart_flags_get(DRV_BLE_UART_MOUDLE) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
	}		
}

void drv_ble_on_cts_high(void)
{
	LOG_PRINTF(MOUDLE_NAME"drv_ble_on_cts_high\n");

	if(m_state == CONNECTING)
	{
		LOG_PRINTF(MOUDLE_NAME"m_state = CONNECTING\n");
		m_state = CONNECT;
		LOG_PRINTF(MOUDLE_NAME"m_state = CONNECT\n");
		send();
	}
	else if(m_state == DISCONNECT)
	{
		LOG_PRINTF(MOUDLE_NAME"m_state = DISCONNECT\n");
		m_state = CONNECT;
		LOG_PRINTF(MOUDLE_NAME"m_state = CONNECT\n");
		uart_enable();
		PIN_SET(DRV_BLE_UART_RTS_PIN);
		
	}
	else{
		LOG_PRINTF(MOUDLE_NAME"Exception interrupt\n");
		/**<Should never fall in here.>*/
	}	
	
}
void drv_ble_on_cts_low(void)
{
	LOG_PRINTF(MOUDLE_NAME"drv_ble_on_cts_low\n");
	if(m_state == CONNECT)
	{
		LOG_PRINTF(MOUDLE_NAME"m_state = CONNECT\n");
		uart_disable();
		PIN_CLEAR(DRV_BLE_UART_RTS_PIN);
		m_state = DISCONNECT;
		LOG_PRINTF(MOUDLE_NAME"m_state = DISCONNECT\n");
	}
	else if(m_state == DISCONNECTING)
	{
		LOG_PRINTF(MOUDLE_NAME"m_state = DISCONNECTING\n");
		m_state = DISCONNECT;
		LOG_PRINTF(MOUDLE_NAME"m_state = DISCONNECT\n");
	}
	else{
		LOG_PRINTF(MOUDLE_NAME"Exception interrupt\n");
		/**<Should never fall in here.>*/
	}
}



static void cts_handle(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"cts_handle\n");
	
	if (am_hal_gpio_input_bit_read(DRV_BLE_UART_CTS_PIN))
	{
		am_hal_gpio_int_polarity_bit_set(DRV_BLE_UART_CTS_PIN,AM_HAL_GPIO_FALLING);			
	}
	else
	{
		am_hal_gpio_int_polarity_bit_set(DRV_BLE_UART_CTS_PIN,AM_HAL_GPIO_RISING);
	}			
	
	m_ble_cb(DRV_BLE_EVT_UART_CTS_INTERRUPT,NULL,0);
	
	
}

void am_uart1_isr(void)
{
	uint32_t status;
	uint32_t rxSize, txSize;
	uint8_t mode = 0;
	status = am_hal_uart_int_status_get(DRV_BLE_UART_MOUDLE, false);
	am_hal_uart_int_clear(DRV_BLE_UART_MOUDLE, status);

	if (status & (AM_HAL_UART_INT_RX_TMOUT | AM_HAL_UART_INT_RX))
	{
			am_hal_uart_service_buffered_timeout_save(DRV_BLE_UART_MOUDLE, status);
	}

	if (status & (AM_HAL_UART_INT_RX_TMOUT))
	{
		DEBUG_PRINTF(MOUDLE_NAME"AM_HAL_UART_INT_RX_TMOUT\n");
		mode = 1;
	}	
	
	am_hal_uart_get_status_buffered(DRV_BLE_UART_MOUDLE, &rxSize, &txSize);
	
	if (rxSize >= (DRV_BLE_UART_RX_BUF_SIZE/2))
	{
		DEBUG_PRINTF(MOUDLE_NAME"DRV_BLE_UART_RX_BUF_SIZE\n");
		mode = 1;
	}	
	
	if(mode == 1)
	{
		m_rx_buffer.length = am_hal_uart_char_receive_buffered(DRV_BLE_UART_MOUDLE, (char *)m_rx_buffer.buffer, (DRV_BLE_UART_RX_BUF_SIZE/2));	

		if(m_rx_buffer.length >=(DRV_BLE_UART_RX_BUF_SIZE/2)  && m_rx_buffer.buffer[0] != UART_PACKET_HEADER)
		{
			uint32_t i = 0;
			for(i=0;i<m_rx_buffer.length;i++)
			{
				if(m_rx_buffer.buffer[i] == UART_PACKET_HEADER)
				{
					break;
				}
			}
			m_ble_cb(DRV_BLE_EVT_UART_DATA,m_rx_buffer.buffer+i,m_rx_buffer.length-i);
			if(m_user_cb != NULL)
			{
				m_user_cb(DRV_BLE_EVT_UART_DATA,m_rx_buffer.buffer+i,m_rx_buffer.length-i);
			}			
		}
		else 
		if(m_rx_buffer.buffer[0] == UART_PACKET_HEADER && m_rx_buffer.length >=(DRV_BLE_UART_RX_BUF_SIZE/2))
		{
			m_ble_cb(DRV_BLE_EVT_UART_DATA,m_rx_buffer.buffer,m_rx_buffer.length);
			if(m_user_cb != NULL)
			{
				m_user_cb(DRV_BLE_EVT_UART_DATA,m_rx_buffer.buffer,m_rx_buffer.length);
			}			
		}
	}	
}


void drv_ble_init(ble_callback ble_cb)
{
	DEBUG_PRINTF(MOUDLE_NAME"drv_ble_init\n");
	if(ble_cb != NULL)
	{
		m_ble_cb = ble_cb;	
	}
	am_hal_gpio_int_register(DRV_BLE_UART_CTS_PIN, cts_handle);
	com_apollo2_gpio_int_init(DRV_BLE_UART_CTS_PIN, DRV_BLE_POLARITY_CTS_PIN);	
	PIN_CONFIG(DRV_BLE_UART_RTS_PIN, AM_HAL_PIN_OUTPUT);
	PIN_CLEAR(DRV_BLE_UART_RTS_PIN);
}


uint32_t drv_ble_send_data(_uart_data_param *param)
{
#if defined JW902
	m_tx_buffer.length = param->length;
	memcpy(m_tx_buffer.buffer,param->p_data,param->length);	
	send();	
#else
	LOG_PRINTF(MOUDLE_NAME"drv_ble_send_data\n");
	
	if(m_state != DISCONNECT || PIN_READ_INPUT(DRV_BLE_UART_CTS_PIN))
	{
		return ERR_BUSY;
	}
	m_state = CONNECTING;
	m_user_cb = NULL;
	if(param->user_callback != NULL)
	{
		m_user_cb = param->user_callback;
	}
	m_tx_buffer.length = param->length;
	memcpy(m_tx_buffer.buffer,param->p_data,param->length);
	uart_enable();
	PIN_SET(DRV_BLE_UART_RTS_PIN);
#endif
	return ERR_SUCCESS;
}

uint32_t drv_ble_uart_put(uint8_t *p_data, uint32_t length)
{
#if defined JW902
	memcpy(m_tx_buffer.buffer,p_data,length);
	send();
#else
	LOG_PRINTF(MOUDLE_NAME"drv_ble_uart_put\n");
	
	if(m_state != CONNECT)
	{
		DEBUG_PRINTF(MOUDLE_NAME"ERR_INVALID_STATE\n");
		return ERR_INVALID_STATE;
	}

	memcpy(m_tx_buffer.buffer,p_data,length);
	send();
	DEBUG_PRINTF(MOUDLE_NAME"drv_ble_uart_put finish\n");
#endif
	return ERR_SUCCESS;
}



void drv_ble_uart_close(void)
{
#if defined JW902

#else
	LOG_PRINTF(MOUDLE_NAME"drv_ble_uart_close\n");
	uart_disable();
	m_state = DISCONNECTING;
	LOG_PRINTF(MOUDLE_NAME"m_state = DISCONNECTING\n");	
	PIN_CLEAR(DRV_BLE_UART_RTS_PIN);
#endif
}

void drv_ble_uart_close_on_ble_sys_off(void)
{
	LOG_PRINTF(MOUDLE_NAME"drv_ble_uart_close_on_ble_sys_off\n");
	uart_disable();
	m_state = DISCONNECT;
	LOG_PRINTF(MOUDLE_NAME"m_state = DISCONNECT\n");	
	com_apollo2_gpio_int_uninit(DRV_BLE_UART_CTS_PIN);
	PIN_CONFIG(DRV_BLE_UART_CTS_PIN,AM_HAL_GPIO_DISABLE);
	PIN_CONFIG(DRV_BLE_UART_RTS_PIN,AM_HAL_GPIO_DISABLE);
	

}


#ifdef JW902
void jw902_drv_ble_init(ble_callback ble_cb)
{
	if(ble_cb != NULL)
	{
		m_ble_cb = ble_cb;	
	}
	uart_enable();
}
#endif



