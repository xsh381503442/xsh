#include "drv_pc.h"
#include "rtt_log.h"
#include "lib_error.h"
#include "com_apollo2.h"
#include "SEGGER_RTT.h"
#include "board.h"
#include "am_mcu_apollo.h"

#include <string.h>

#define MOUDLE_LOG_ENABLED 							0
#define MOUDLE_DEBUG_ENABLED   					0
#define MOUDLE_NAME                     "[DRV_PC]:"

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


static pc_callback m_pc_cb;




static void bus_enable(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"bus_enable\n");

	
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
	DEBUG_PRINTF(MOUDLE_NAME"bus_disable\n");
	am_hal_uart_int_clear(DRV_PC_UART_MOUDLE, 0xFFFFFFFF);
	am_hal_uart_int_disable(DRV_PC_UART_MOUDLE,0xFFFFFFFF);		
	am_hal_uart_disable(DRV_PC_UART_MOUDLE);
	am_hal_uart_clock_disable(DRV_PC_UART_MOUDLE);
	am_hal_uart_pwrctrl_disable(DRV_PC_UART_MOUDLE);
	am_hal_gpio_pin_config(DRV_PC_UART_TX_PIN, AM_HAL_PIN_DISABLE);
	am_hal_gpio_pin_config(DRV_PC_UART_RX_PIN, AM_HAL_PIN_DISABLE);
}

static void uart_cb(uint8_t *p_data)
{
	DEBUG_PRINTF(MOUDLE_NAME"uart_cb\n");
	DEBUG_HEXDUMP(p_data, DRV_PC_UART_RX_BUF_SIZE);
	m_pc_cb(DRV_PC_EVT_UART_DATA, p_data, DRV_PC_UART_RX_BUF_SIZE);	
}
void drv_pc_init(pc_callback pc_cb)
{
	DEBUG_PRINTF(MOUDLE_NAME"drv_pc_init\n");
	uart0_handler_register(uart_cb);	
	m_pc_cb = pc_cb;
	bus_enable();
}

void drv_pc_uninit(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"drv_pc_uninit\n");
	bus_disable();
}


void drv_pc_send_data(uint8_t *p_data, uint32_t length)
{
	LOG_PRINTF(MOUDLE_NAME"drv_pc_send_data\n");
	static uint8_t m_buf[DRV_PC_UART_TX_BUF_SIZE];
	//memset(m_buf,0,DRV_PC_UART_TX_BUF_SIZE);
	memcpy(m_buf,p_data,length);
	DEBUG_HEXDUMP(m_buf, length);
	for(uint32_t i=0;i<sizeof(m_buf);i++)
	{
		am_hal_uart_char_transmit_buffered(DRV_PC_UART_MOUDLE, m_buf[i]);	
		while ( am_hal_uart_flags_get(DRV_PC_UART_MOUDLE) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
	}
		
	
	LOG_PRINTF(MOUDLE_NAME"drv_pc_send_data finish\n");
}


