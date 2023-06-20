#include "drv_pc.h"
#include "drv_config.h"
#include "isr_uart.h"

#include "am_mcu_apollo.h"

#include <string.h>
#if DEBUG_ENABLED == 1 && DRV_PC_LOG_ENABLED == 1
	#define DRV_PC_LOG_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define DRV_PC_LOG_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else       
	#define DRV_PC_LOG_WRITESTRING(...)
	#define DRV_PC_LOG_PRINTF(...)		        
#endif

static pc_callback m_pc_cb;




static void bus_enable(void)
{
	am_hal_uart_config_t g_sUartConfig =
	{
			.ui32BaudRate = DRV_PC_UART_BAUDRATE,
			.ui32DataBits = AM_HAL_UART_DATA_BITS_8,
			.bTwoStopBits = false,
			.ui32Parity   = AM_HAL_UART_PARITY_NONE,
			.ui32FlowCtrl = AM_HAL_UART_FLOW_CTRL_NONE,
	};
	static uint8_t rx_buf[DRV_PC_UART_TX_BUF_SIZE];
	static uint8_t tx_buf[DRV_PC_UART_RX_BUF_SIZE];
	am_hal_uart_pwrctrl_enable(DRV_PC_UART_MOUDLE);
	am_hal_uart_clock_enable(DRV_PC_UART_MOUDLE);
	am_hal_uart_disable(DRV_PC_UART_MOUDLE);
	am_hal_uart_config(DRV_PC_UART_MOUDLE, &g_sUartConfig);
	am_hal_uart_fifo_config(DRV_PC_UART_MOUDLE, AM_HAL_UART_TX_FIFO_1_2 | AM_HAL_UART_RX_FIFO_1_2);
	am_hal_uart_init_buffered(DRV_PC_UART_MOUDLE, rx_buf, DRV_PC_UART_RX_BUF_SIZE,
														tx_buf, DRV_PC_UART_TX_BUF_SIZE);		
	am_hal_uart_clock_enable(DRV_PC_UART_MOUDLE);
	am_hal_uart_enable(DRV_PC_UART_MOUDLE);
	am_hal_uart_int_clear(DRV_PC_UART_MOUDLE, 0xFFFFFFFF);
	am_hal_uart_int_enable(DRV_PC_UART_MOUDLE, AM_HAL_UART_INT_RX_TMOUT|AM_HAL_UART_INT_RX|AM_HAL_UART_INT_TXCMP);
	am_hal_gpio_pin_config(DRV_PC_UART_TX_PIN, DRV_PV_UART_TX_CONFIG);
	am_hal_gpio_pin_config(DRV_PC_UART_RX_PIN, DRV_PV_UART_RX_CONFIG);
	am_hal_interrupt_enable(AM_HAL_INTERRUPT_UART + DRV_PC_UART_MOUDLE);
}
static void bus_disable(void)
{
	am_hal_uart_int_clear(DRV_PC_UART_MOUDLE, 0xFFFFFFFF);
	am_hal_uart_int_disable(DRV_PC_UART_MOUDLE,0xFFFFFFFF);		
	am_hal_uart_disable(DRV_PC_UART_MOUDLE);
	am_hal_uart_clock_disable(DRV_PC_UART_MOUDLE);
	am_hal_uart_pwrctrl_disable(DRV_PC_UART_MOUDLE);
	#if defined( WATCH_MODEL_PLUS ) 
	am_hal_gpio_pin_config(DRV_PC_UART_TX_PIN, AM_HAL_PIN_INPUT);   //This pin in 910 is Motion interrupt pin
	#else
	am_hal_gpio_pin_config(DRV_PC_UART_TX_PIN, AM_HAL_PIN_DISABLE);
	#endif
	am_hal_gpio_pin_config(DRV_PC_UART_RX_PIN, AM_HAL_PIN_DISABLE);
}

static void uart_cb(uint8_t *p_data)
{
	DRV_PC_LOG_PRINTF("[DRV_PC]:uart_cb\n");
	m_pc_cb(DRV_PC_EVT_UART_DATA, p_data, DRV_PC_UART_RX_BUF_SIZE);	
}
void drv_pc_init(pc_callback pc_cb)
{
	DRV_PC_LOG_PRINTF("[DRV_PC]:drv_pc_init\n");
	uart0_handler_register(uart_cb,true);	
	m_pc_cb = pc_cb;
	bus_enable();
}

void drv_pc_uninit(void)
{
	DRV_PC_LOG_PRINTF("[DRV_PC]:drv_pc_uninit\n");
	bus_disable();
	uart0_handler_unregister();
}
void drv_pc_send_data(uint8_t *p_data, uint32_t length)
{
	uint8_t m_buf[DRV_PC_UART_TX_BUF_SIZE];
	memcpy(m_buf,p_data,length);
	for(uint32_t i=0;i<sizeof(m_buf);i++)
	{
		am_hal_uart_char_transmit_buffered(DRV_PC_UART_MOUDLE, m_buf[i]);	
		while ( am_hal_uart_flags_get(DRV_PC_UART_MOUDLE) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
	}
}


void drv_pc_send_data_buf(uint8_t *p_data, uint32_t length)
	{	
	static uint8_t m_buf[DRV_PC_UART_TX_BUF_SIZE];
	memcpy(m_buf,p_data,length);	

	for(uint32_t i=0;i<length;i++)	
		{		
		am_hal_uart_char_transmit_buffered(DRV_PC_UART_MOUDLE, m_buf[i]);	
		while ( am_hal_uart_flags_get(DRV_PC_UART_MOUDLE) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
		}
	}








