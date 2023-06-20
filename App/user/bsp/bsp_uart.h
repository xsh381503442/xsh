#ifndef _BSP_UART_H__
#define _BSP_UART_H__

#include <stdint.h>
#include "bsp_config.h"

extern void bsp_uart_init(uint32_t ui32Module,am_hal_uart_config_t uartConfig,
	                 uint8_t *uartRxBuffer, uint8_t *uartTxBuffer, uint32_t UART_BUFFER_SIZE);
extern void bsp_uart_enable(uint32_t ui32Module);
extern void bsp_uart_disable(uint32_t ui32Module);

#endif

