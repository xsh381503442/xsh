#include "com_apollo2.h"
#include "rtt_log.h"
#include "lib_error.h"
#include "SEGGER_RTT.h"

#include "am_mcu_apollo.h"

#include "cmd_pc.h"
#include "drv_pc.h"

#ifndef LAYER_UPDATEALGORITHM 
#include "drv_ble.h"
#endif


#define MOUDLE_LOG_ENABLED 							0
#define MOUDLE_DEBUG_ENABLED   					0
#define MOUDLE_NAME                     "[COM_APOLLO2]:"

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


static uart0_callback m_uart0_cb;

void am_uart_isr(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"am_uart0_isr\n");
	uint32_t status;
	uint32_t rxSize, txSize;
	uint8_t mode = 0;

	status = am_hal_uart_int_status_get(DRV_PC_UART_MOUDLE, false);
	am_hal_uart_int_clear(DRV_PC_UART_MOUDLE, status);

	
	if (status & (AM_HAL_UART_INT_RX_TMOUT | AM_HAL_UART_INT_TX | AM_HAL_UART_INT_RX))
	{
			am_hal_uart_service_buffered_timeout_save(DRV_PC_UART_MOUDLE, status);
	}

	if (status & (AM_HAL_UART_INT_RX_TMOUT))
	{
		mode = 1;
	}

	am_hal_uart_get_status_buffered(DRV_PC_UART_MOUDLE, &rxSize, &txSize);

//	if (status & (AM_HAL_UART_INT_RX))
//	{
//	}

//	if (status & (AM_HAL_UART_INT_TXCMP))
//	{
//	}

//	am_hal_uart_get_status_buffered(DRV_PC_UART_MOUDLE, &rxSize, &txSize);

	if (rxSize >= DRV_PC_UART_RX_BUF_SIZE)
	{
		mode = 1;
	}
	
	if(mode == 1)
	{
		static uint8_t data[DRV_PC_UART_RX_BUF_SIZE];
		
		uint32_t length = am_hal_uart_char_receive_buffered(DRV_PC_UART_MOUDLE, (char *)data, DRV_PC_UART_RX_BUF_SIZE);
		
		if(data[0] == UART_PACKET_HEADER)
		{
			DEBUG_HEXDUMP(data, length);	
			m_uart0_cb(data);		
		
		}
		

	}	

}
void uart0_handler_register(uart0_callback uart0_cb)
{
	m_uart0_cb = uart0_cb;
}



void am_gpio_isr(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"am_gpio_isr\n");
	uint64_t ui64Status;
	ui64Status = am_hal_gpio_int_status_get(false);
	am_hal_gpio_int_clear(ui64Status);
	am_hal_gpio_int_service(ui64Status);
}
void am_ctimer_isr(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"am_ctimer_isr\n");
	uint32_t ui32Status;
	ui32Status = am_hal_ctimer_int_status_get(false);
	am_hal_ctimer_int_clear(ui32Status);
	am_hal_ctimer_int_service(ui32Status);
}













void com_apollo2_gpio_int_init(uint32_t pin, uint32_t polarity)
{
	DEBUG_PRINTF(MOUDLE_NAME"com_apollo2_gpio_int_init\n");
	am_hal_gpio_pin_config(pin,AM_HAL_GPIO_INPUT);
	am_hal_gpio_int_polarity_bit_set(pin,polarity);
	am_hal_gpio_int_clear(AM_HAL_GPIO_BIT(pin));
	am_hal_gpio_int_enable(AM_HAL_GPIO_BIT(pin));	

}
void com_apollo2_gpio_int_uninit(uint32_t pin)
{
	DEBUG_PRINTF(MOUDLE_NAME"com_apollo2_gpio_int_uninit\n");
	am_hal_gpio_int_clear(AM_HAL_GPIO_BIT(pin));
	am_hal_gpio_int_disable(AM_HAL_GPIO_BIT(pin));
	am_hal_gpio_pin_config(pin,AM_HAL_PIN_DISABLE);
}







