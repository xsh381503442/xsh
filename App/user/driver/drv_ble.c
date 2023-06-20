#include "drv_ble.h"
#include "lib_error.h"
#include "SEGGER_RTT.h"
#include "am_mcu_apollo.h"
#include "am_util_delay.h"

#include "cmd_ble.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "portmacro.h"
#include "portable.h"
#include "event_groups.h"
#include "semphr.h"
 
 
#include <string.h>
#include "task_ble.h"

#define DRV_BLE_LOG_ENABLED       0
#if DEBUG_ENABLED == 1 && DRV_BLE_LOG_ENABLED == 1
	#define DRV_BLE_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define DRV_BLE_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define DRV_BLE_WRITESTRING(...)
	#define DRV_BLE_PRINTF(...)		        
#endif


//#define DELAY_US(X)                 am_util_delay_us(X)
//#define DELAY_MS(X)                 am_util_delay_ms(X)
#define DELAY_MS(X)                 vTaskDelay(X / portTICK_PERIOD_MS)
//#define DELAY_MS(X)                   do{TickType_t xLastWakeTime;xLastWakeTime = xTaskGetTickCount ();vTaskDelayUntil( &xLastWakeTime, X / portTICK_PERIOD_MS );}while(0)


extern SemaphoreHandle_t Apollo_UART1_Semaphore;
extern uint32_t real_time_log_flag;

static ble_callback m_ble_cb;

static ble_user_callback m_user_cb = NULL;


static void uart_disable(void);

#define BLE_UART_EXCEPTION_TIMER         1

#define EXCEPTION_TIMEOUT         3000

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



#define OSTIMER_WAIT_FOR_QUEUE           100
static TimerHandle_t m_timer; 
#define TIMER_INIT_INTERVAL_MS        1000
static bool m_timer_start = 0;

uint8_t cod_uart_normal_open =0;
extern uint8_t cod_sport_cast_flag;

#if BLE_UART_EXCEPTION_TIMER == 1
static void callback(TimerHandle_t xTimer)
{
	if(cod_uart_normal_open)
	{
		return;
	}
	DRV_BLE_PRINTF("[DRV_BLE]:timeout callback\n");
	//dev_ble_system_off();
	//DELAY_MS(500);
	//dev_ble_system_on();

	if (am_hal_gpio_input_bit_read(DRV_BLE_UART_CTS_PIN))
	{
		DRV_BLE_PRINTF("[DRV_BLE]:DRV_BLE_UART_CTS_PIN IS HIGN\n");
		if(am_hal_gpio_out_bit_read(DRV_BLE_UART_RTS_PIN)){
			DRV_BLE_PRINTF("[DRV_BLE]:DRV_BLE_UART_RTS_PIN IS HIGN\n");
			//am_hal_gpio_out_bit_clear(DRV_BLE_UART_RTS_PIN);
			drv_ble_uart_close();
		}
		else{
			DRV_BLE_PRINTF("[DRV_BLE]:DRV_BLE_UART_RTS_PIN IS LOW AND BLE OFF\n");
			dev_ble_system_off();
			DELAY_MS(500);
			dev_ble_system_on();
		}
	}
	else
	{
		DRV_BLE_PRINTF("[DRV_BLE]:DRV_BLE_UART_CTS_PIN is LOW\n");
		if(m_state == CONNECTING) {
			drv_ble_uart_close();
			m_state = DISCONNECT;
			DRV_BLE_PRINTF("[DRV_BLE]:m_state is connecting close uart\n");
		}
	}
	
//	drv_ble_uart_close();	
	
}
#endif



void drv_ble_timer_init(void)
{
#if BLE_UART_EXCEPTION_TIMER == 1	
	m_timer_start = 0;
	m_timer = xTimerCreate("DRVBLE",
												 ( TIMER_INIT_INTERVAL_MS / portTICK_PERIOD_MS ),
												 pdFALSE,
												 NULL,
												 callback);
	if (NULL == m_timer)
	{
		DRV_BLE_PRINTF("[DRV_BLE]:xTimerCreate fail\n");
	}
#endif
	
}



void drv_ble_timer_uninit(void)
{
#if BLE_UART_EXCEPTION_TIMER == 1	
	if (pdPASS != xTimerDelete(m_timer, OSTIMER_WAIT_FOR_QUEUE))
	{
		DRV_BLE_PRINTF("[DRV_BLE]:xTimerDelete fail\n");
	}
#endif
}

void drv_ble_timer_start(uint32_t timeout_msec)
{
#if BLE_UART_EXCEPTION_TIMER == 1		
	if(m_timer_start)
	{
		return;
	}
	m_timer_start = 1;
	if( xTimerChangePeriod( m_timer, timeout_msec / portTICK_PERIOD_MS, OSTIMER_WAIT_FOR_QUEUE ) == pdPASS )
	{
		DRV_BLE_PRINTF("[DRV_BLE]:xTimerChangePeriod success\n");
	}
	else
	{
		DRV_BLE_PRINTF("[DRV_BLE]:xTimerChangePeriod fail\n");
	}	
#endif	
}


void drv_ble_timer_stop(void)
{
#if BLE_UART_EXCEPTION_TIMER == 1		
	if(!m_timer_start)
	{
		return;
	}
	m_timer_start = 0;	
	if (pdPASS != xTimerStop(m_timer, OSTIMER_WAIT_FOR_QUEUE))
	{
		DRV_BLE_PRINTF("[DRV_BLE]:xTimerStop fail\n");
	}
	DRV_BLE_PRINTF("[DRV_BLE]:drv_ble_timer_stop success\n");
#endif	
}



static void uart_enable(void)
{
	am_hal_uart_config_t g_sUartConfig =
	{
			.ui32BaudRate = DRV_BLE_UART_BAUDRATE,
			.ui32DataBits = AM_HAL_UART_DATA_BITS_8,
			.bTwoStopBits = false,				
			.ui32Parity   = AM_HAL_UART_PARITY_NONE, 			
			.ui32FlowCtrl = AM_HAL_UART_FLOW_CTRL_NONE,    	
	};
	static uint8_t rx_buf[DRV_BLE_UART_RX_BUF_SIZE];
	static uint8_t tx_buf[DRV_BLE_UART_TX_BUF_SIZE];
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
	am_hal_gpio_pin_config(DRV_BLE_UART_RX_PIN, AM_HAL_PIN_19_UART1RX);
	am_hal_gpio_pin_config(DRV_BLE_UART_TX_PIN, AM_HAL_PIN_18_UART1TX);
	am_hal_interrupt_enable(AM_HAL_INTERRUPT_UART + DRV_BLE_UART_MOUDLE);
	

//	for(uint32_t i=0;i<(0x4001D044-0x4001D000);i++)
//	{
//		if(*((uint32_t *)(0x4001D000+i*4)) != 0)
//		{
//			DRV_BLE_PRINTF("[DRV_BLE]:*(volatile uint32_t *)(0x%08X) = 0x%08X;\n",(0x4001D000+i*4),*((uint32_t *)(0x4001D000+i*4)));
//		}
//	}
}
static void uart_disable(void)
{
	am_hal_uart_int_clear(DRV_BLE_UART_MOUDLE, 0xFFFFFFFF);	
	am_hal_uart_int_disable(DRV_BLE_UART_MOUDLE,0xFFFFFFFF);	
	am_hal_uart_disable(DRV_BLE_UART_MOUDLE);		
	am_hal_uart_clock_disable(DRV_BLE_UART_MOUDLE);
	am_hal_uart_pwrctrl_disable(DRV_BLE_UART_MOUDLE);
	
	am_hal_gpio_pin_config(DRV_BLE_UART_RX_PIN, AM_HAL_GPIO_INPUT|AM_HAL_GPIO_PULLUP);
	am_hal_gpio_pin_config(DRV_BLE_UART_TX_PIN, AM_HAL_GPIO_INPUT|AM_HAL_GPIO_PULLUP);	
	
}

//extern uint16_t log_times;
//extern uint16_t send_times;
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
	DRV_BLE_PRINTF("[DRV_BLE]:drv_ble_on_cts_high\n");
	if(m_state == CONNECTING)
	{	
#if BLE_UART_EXCEPTION_TIMER == 1				
		drv_ble_timer_stop();
		drv_ble_timer_start(EXCEPTION_TIMEOUT);	
#endif			
		DRV_BLE_PRINTF("[DRV_BLE]:m_state = CONNECTING\n");
		m_state = CONNECT;
		DRV_BLE_PRINTF("[DRV_BLE]:m_state = CONNECT\n");
		send();
	}
	else if(m_state == DISCONNECT)
	{
#if BLE_UART_EXCEPTION_TIMER == 1				
		drv_ble_timer_stop();
		drv_ble_timer_start(EXCEPTION_TIMEOUT);	
#endif		
		DRV_BLE_PRINTF("[DRV_BLE]:m_state = DISCONNECT\n");
		m_state = CONNECT;
		DRV_BLE_PRINTF("[DRV_BLE]:m_state = CONNECT\n");
		uart_enable();
		am_hal_gpio_out_bit_set(DRV_BLE_UART_RTS_PIN);
	}
	else{
		DRV_BLE_PRINTF("[DRV_BLE]:Exception interrupt\n");
		/**<Should never fall in here.>*/

	}	
	
}
void drv_ble_on_cts_low(void)
{
	DRV_BLE_PRINTF("[DRV_BLE]:drv_ble_on_cts_low\n");
	if(m_state == CONNECT)
	{
#if BLE_UART_EXCEPTION_TIMER == 1			
		drv_ble_timer_stop();
#endif			
		DRV_BLE_PRINTF("[DRV_BLE]:m_state = CONNECT\n");
		cod_uart_normal_open = 0;
		uart_disable();
		am_hal_gpio_out_bit_set(DRV_BLE_UART_RTS_PIN);
		am_hal_gpio_out_bit_clear(DRV_BLE_UART_RTS_PIN);
		m_state = DISCONNECT;
		DRV_BLE_PRINTF("[DRV_BLE]:m_state = DISCONNECT\n");
		
	}
	else if(m_state == DISCONNECT)
	{
#if BLE_UART_EXCEPTION_TIMER == 1				
		drv_ble_timer_stop();
#endif		
		DRV_BLE_PRINTF("[DRV_BLE]:m_state = DISCONNECT\n");
	}

}



static void cts_handle(void)
{
	DRV_BLE_PRINTF("[DRV_BLE]:cts_handle\n");
	if (am_hal_gpio_input_bit_read(DRV_BLE_UART_CTS_PIN))
	{
		DRV_BLE_PRINTF("[DRV_BLE]:AM_HAL_GPIO_FALLING\n");
		am_hal_gpio_int_polarity_bit_set(DRV_BLE_UART_CTS_PIN,AM_HAL_GPIO_FALLING);			
	}
	else
	{
		DRV_BLE_PRINTF("[DRV_BLE]:AM_HAL_GPIO_RISING\n");
		am_hal_gpio_int_polarity_bit_set(DRV_BLE_UART_CTS_PIN,AM_HAL_GPIO_RISING);
		//¹Ø¼ü
		if(m_state == DISCONNECTING)
		{	
			m_state = DISCONNECT;
		}
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
		DRV_BLE_PRINTF("[DRV_BLE]:AM_HAL_UART_INT_RX_TMOUT\r\n");
		mode = 1;
	}	
	
	am_hal_uart_get_status_buffered(DRV_BLE_UART_MOUDLE, &rxSize, &txSize);
	
	if (rxSize >= (DRV_BLE_UART_RX_BUF_SIZE/2))
	{
		DRV_BLE_PRINTF("[DRV_BLE]:DRV_BLE_UART_RX_BUF_SIZE/2\r\n");
		mode = 1;
	}	
	
	if(mode == 1)
	{
		m_rx_buffer.length = am_hal_uart_char_receive_buffered(DRV_BLE_UART_MOUDLE, (char *)m_rx_buffer.buffer, (DRV_BLE_UART_RX_BUF_SIZE/2));	

		if(m_rx_buffer.length >=(DRV_BLE_UART_RX_BUF_SIZE/2))
		{
			if(m_rx_buffer.buffer[0] == UART_PACKET_HEADER)
			{
				m_ble_cb(DRV_BLE_EVT_UART_DATA,m_rx_buffer.buffer,m_rx_buffer.length);
				if(m_user_cb != NULL)
				{
					m_user_cb(DRV_BLE_EVT_UART_DATA,m_rx_buffer.buffer,m_rx_buffer.length);
				}			
			}
			else{
				uint32_t i = 0;
				for(i=0;i<m_rx_buffer.length;i++)
				{
					if(m_rx_buffer.buffer[i] == UART_PACKET_HEADER)
					{
						break;
					}
				}
				if(i < 12)
				{
					DRV_BLE_PRINTF("[DRV_BLE]:m_rx_buffer.buffer[0] != UART_PACKET_HEADER\n");
					m_ble_cb(DRV_BLE_EVT_UART_DATA,m_rx_buffer.buffer+i,m_rx_buffer.length-i);
					if(m_user_cb != NULL)
					{
						m_user_cb(DRV_BLE_EVT_UART_DATA,m_rx_buffer.buffer+i,m_rx_buffer.length-i);
					}						
				}
				else{
					drv_ble_uart_close();
				
				}
			}
		}
		else
		{
			DRV_BLE_PRINTF("[DRV_BLE]:m_rx_buffer.length < (DRV_BLE_UART_RX_BUF_SIZE/2)\r\n");
			if ((m_rx_buffer.buffer[0] == UART_PACKET_HEADER)&&((m_rx_buffer.buffer[1] == 0x31)||(m_rx_buffer.buffer[1] == 0x32))
				&&(m_rx_buffer.buffer[2] == 0x02)&&(m_rx_buffer.buffer[3] == 0x00))
				{
					DRV_BLE_PRINTF("[DRV_BLE]:CONNECT OR DISCONNECT COMMAMD\r\n");
					m_ble_cb(DRV_BLE_EVT_UART_DATA,m_rx_buffer.buffer,m_rx_buffer.length);
					if(m_user_cb != NULL)
					{
						m_user_cb(DRV_BLE_EVT_UART_DATA,m_rx_buffer.buffer,m_rx_buffer.length);
					}	
				}			
		}
		
//		if(m_rx_buffer.length >=(DRV_BLE_UART_RX_BUF_SIZE/2)  && m_rx_buffer.buffer[0] != UART_PACKET_HEADER)
//		{
//			uint32_t i = 0;
//			for(i=0;i<m_rx_buffer.length;i++)
//			{
//				if(m_rx_buffer.buffer[i] == UART_PACKET_HEADER)
//				{
//					break;
//				}
//			}
//			
//			DRV_BLE_PRINTF("[DRV_BLE]:m_rx_buffer.buffer[0] != UART_PACKET_HEADER\n");
//			m_ble_cb(DRV_BLE_EVT_UART_DATA,m_rx_buffer.buffer+i,m_rx_buffer.length-i);
//			if(m_user_cb != NULL)
//			{
//				m_user_cb(DRV_BLE_EVT_UART_DATA,m_rx_buffer.buffer+i,m_rx_buffer.length-i);
//			}			
//		}

//		if(m_rx_buffer.buffer[0] == UART_PACKET_HEADER && m_rx_buffer.length >=(DRV_BLE_UART_RX_BUF_SIZE/2))
//		{
//			
//			DRV_BLE_PRINTF("[DRV_BLE]:m_rx_buffer.buffer[0] == UART_PACKET_HEADER\n");
//			m_ble_cb(DRV_BLE_EVT_UART_DATA,m_rx_buffer.buffer,m_rx_buffer.length);
//			if(m_user_cb != NULL)
//			{
//				m_user_cb(DRV_BLE_EVT_UART_DATA,m_rx_buffer.buffer,m_rx_buffer.length);
//			}			
//		}
	}	
}


void drv_ble_init(ble_callback ble_cb)
{
	if(ble_cb != NULL)
	{
		m_ble_cb = ble_cb;	
	}
	am_hal_gpio_int_register(DRV_BLE_UART_CTS_PIN, cts_handle);
	am_hal_gpio_pin_config(DRV_BLE_UART_CTS_PIN,AM_HAL_GPIO_INPUT);
	am_hal_gpio_int_polarity_bit_set(DRV_BLE_UART_CTS_PIN,DRV_BLE_POLARITY_CTS_PIN);
	am_hal_gpio_int_clear(AM_HAL_GPIO_BIT(DRV_BLE_UART_CTS_PIN));
	am_hal_gpio_int_enable(AM_HAL_GPIO_BIT(DRV_BLE_UART_CTS_PIN));	
	am_hal_gpio_pin_config(DRV_BLE_UART_RTS_PIN, AM_HAL_PIN_OUTPUT);
	am_hal_gpio_out_bit_clear(DRV_BLE_UART_RTS_PIN);
}


uint32_t drv_ble_send_data(_uart_data_param *param)
{
	if(cod_uart_normal_open)
	{	
		xSemaphoreTake(Apollo_UART1_Semaphore, portMAX_DELAY );
		if (real_time_log_flag && cod_sport_cast_flag)
		{
			vTaskDelay(10/portTICK_PERIOD_MS);
		}
		m_tx_buffer.length = param->length;
	    memcpy(m_tx_buffer.buffer,param->p_data,param->length);
		send();
		if (real_time_log_flag && cod_sport_cast_flag)
		{
			vTaskDelay(10/portTICK_PERIOD_MS);
		}
		xSemaphoreGive(Apollo_UART1_Semaphore );	
	    return ERR_SUCCESS;
	}

	if(m_state != DISCONNECT || am_hal_gpio_input_bit_read(DRV_BLE_UART_CTS_PIN))
	{
		DRV_BLE_PRINTF("[DRV_BLE]:m_state = %d\n",m_state);
		DRV_BLE_PRINTF("[DRV_BLE]:drv_ble_send_data err busy\n");
		return ERR_BUSY;
	}		
	DRV_BLE_PRINTF("[DRV_BLE]:m_state = CONNECTING\n");
	m_state = CONNECTING;
	m_user_cb = NULL;
	if(param->user_callback != NULL)
	{
		m_user_cb = param->user_callback;
	}
	m_tx_buffer.length = param->length;
	memcpy(m_tx_buffer.buffer,param->p_data,param->length);
	uart_enable();
	am_hal_gpio_out_bit_set(DRV_BLE_UART_RTS_PIN);
	
	drv_ble_timer_stop();
	drv_ble_timer_start(EXCEPTION_TIMEOUT);
	DRV_BLE_PRINTF("[DRV_BLE]:drv_ble_send_data timer start\n");
	return ERR_SUCCESS;
}

uint32_t drv_ble_uart_put(uint8_t *p_data, uint32_t length)
{

	if(cod_uart_normal_open == 0)
	{
		if(m_state != CONNECT)
		{
			return ERR_INVALID_STATE;
		}
	}
	
	xSemaphoreTake(Apollo_UART1_Semaphore, portMAX_DELAY );
	memcpy(m_tx_buffer.buffer,p_data,length);
	send();
	if (real_time_log_flag && cod_sport_cast_flag)
	{
		vTaskDelay(10/portTICK_PERIOD_MS);
	}
	xSemaphoreGive( Apollo_UART1_Semaphore );

	return ERR_SUCCESS;
}



void drv_ble_uart_close(void)
{
	if(cod_uart_normal_open)
	{
		return;
	}
	uart_disable();
	m_state = DISCONNECTING;
	DRV_BLE_PRINTF("[DRV_BLE]:m_state = DISCONNECTING\n");
	am_hal_gpio_out_bit_set(DRV_BLE_UART_RTS_PIN);
	am_hal_gpio_out_bit_clear(DRV_BLE_UART_RTS_PIN);
	
	
	
	

	
}

void drv_ble_uart_close_on_ble_sys_off(void)
{
	uart_disable();
	m_state = DISCONNECT;	
	DRV_BLE_PRINTF("[DRV_BLE]:m_state = DISCONNECT\n");
	am_hal_gpio_int_clear(AM_HAL_GPIO_BIT(DRV_BLE_UART_CTS_PIN));
	am_hal_gpio_int_disable(AM_HAL_GPIO_BIT(DRV_BLE_UART_CTS_PIN));
	am_hal_gpio_pin_config(DRV_BLE_UART_CTS_PIN,AM_HAL_PIN_DISABLE);	
	am_hal_gpio_pin_config(DRV_BLE_UART_RTS_PIN,AM_HAL_GPIO_DISABLE);
	

}





