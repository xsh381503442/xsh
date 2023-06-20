#include "bsp_uart.h"
#include "bsp_config.h"
#include "bsp.h"
#if DEBUG_ENABLED == 1 && BSP_UART_LOG_ENABLED == 1
	#define BSP_UART_LOG_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define BSP_UART_LOG_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define BSP_UART_LOG_WRITESTRING(...)
	#define BSP_UART_LOG_PRINTF(...)		        
#endif
/****
//! @brief Initialize the UART
****/
void bsp_uart_init(uint32_t ui32Module,am_hal_uart_config_t uartConfig,
	                 uint8_t *uartRxBuffer, uint8_t *uartTxBuffer, uint32_t UART_BUFFER_SIZE)
{
	BSP_UART_LOG_PRINTF("[bsp_uart]: bsp_uart_init  %d\r\n",ui32Module);
	
	switch(ui32Module)
  {
     case 0://uart0 心率
				am_hal_gpio_pin_config(BSP_GPIO_UART0_TX,BSP_GPIO_CFG_UART0_TX);
				am_hal_gpio_pin_config(BSP_GPIO_UART0_RX,BSP_GPIO_CFG_UART0_RX);
        break;
		 case 1://uart1 蓝牙
			 	am_hal_gpio_pin_config(BSP_GPIO_UART1_TX,BSP_GPIO_CFG_UART1_TX);
				am_hal_gpio_pin_config(BSP_GPIO_UART1_RX,BSP_GPIO_CFG_UART1_RX);
			 break;
		 default:
			 BSP_UART_LOG_PRINTF("[bsp_uart]: bsp_uart_init ui32Modeule=%d is Error!\n",ui32Module);
			 break;
	}
  // Power on the selected UART
  am_hal_uart_pwrctrl_enable(ui32Module);

  // Start the UART interface, apply the desired configuration settings, and enable the FIFOs.
  am_hal_uart_clock_enable(ui32Module);
	
  // Disable the UART before configuring it.
  am_hal_uart_disable(ui32Module);

  // Configure the UART.
  am_hal_uart_config(ui32Module, &uartConfig);

  // Configure the UART FIFO.
  am_hal_uart_fifo_config(ui32Module, AM_HAL_UART_TX_FIFO_1_2 | AM_HAL_UART_RX_FIFO_1_2);

  // Initialize the UART queues.
  am_hal_uart_init_buffered(ui32Module, uartRxBuffer, UART_BUFFER_SIZE,
                              uartTxBuffer, UART_BUFFER_SIZE);
	am_hal_uart_int_clear(ui32Module,0xFFFFFFFF);
	am_hal_uart_int_enable(ui32Module, AM_HAL_UART_INT_RX_TMOUT|AM_HAL_UART_INT_RX|AM_HAL_UART_INT_TXCMP);
	am_hal_interrupt_priority_set(AM_HAL_INTERRUPT_UART,AM_HAL_INTERRUPT_PRIORITY(5));
	am_hal_interrupt_enable(AM_HAL_INTERRUPT_UART);	
	am_hal_uart_enable(0);							  
//	am_hal_uart_power_off_save(ui32Module);
}
/****
//! @brief Enable the UART
****/
void bsp_uart_enable(uint32_t ui32Module)
{
	// Enable the UART clock.
//	am_hal_uart_clock_enable(ui32Module);
//	am_hal_uart_power_on_restore(ui32Module);
	am_hal_uart_int_enable(ui32Module, AM_HAL_UART_INT_RX_TMOUT|AM_HAL_UART_INT_RX);
	// Enable the UART.
	am_hal_uart_enable(ui32Module);
	

	// Enable the UART pins.
	switch(ui32Module)
	{
	 case 0://uart0 心率
			am_hal_gpio_pin_config(BSP_GPIO_UART0_TX,BSP_GPIO_CFG_UART0_TX);
			am_hal_gpio_pin_config(BSP_GPIO_UART0_RX,BSP_GPIO_CFG_UART0_RX);
			break;
	 case 1://uart1 蓝牙
			am_hal_gpio_pin_config(BSP_GPIO_UART1_TX,BSP_GPIO_CFG_UART1_TX);
			am_hal_gpio_pin_config(BSP_GPIO_UART1_RX,BSP_GPIO_CFG_UART1_RX);
		 break;
	 default:
		 BSP_UART_LOG_PRINTF("[bsp_uart]: bsp_uart_enable ui32Modeule=%d is Error!\n",ui32Module);
		 break;
	}

	am_hal_interrupt_enable(AM_HAL_INTERRUPT_UART + ui32Module);
}

/****
//! @brief Disable the UART
****/
void bsp_uart_disable(uint32_t ui32Module)
{
	// Clear all interrupts before sleeping as having a pending UART interrupt burns power.
	am_hal_uart_int_clear(ui32Module, 0xFFFFFFFF);
	am_hal_uart_int_disable(ui32Module,0xFFFFFFFF);	
	// Disable the UART.
	am_hal_uart_disable(ui32Module);

//	am_hal_uart_power_off_save(ui32Module);
	// Disable the UART clock.
	am_hal_uart_clock_disable(ui32Module);
	am_hal_uart_pwrctrl_disable(ui32Module);
	// Disable the UART pins.
	switch(ui32Module)
	{
	 case 0://uart0 心率
			am_hal_gpio_pin_config(BSP_GPIO_UART0_TX,AM_HAL_PIN_DISABLE);
			am_hal_gpio_pin_config(BSP_GPIO_UART0_RX,AM_HAL_PIN_DISABLE);
			break;
	 case 1://uart1 蓝牙
			am_hal_gpio_pin_config(BSP_GPIO_UART1_TX,AM_HAL_GPIO_INPUT|AM_HAL_GPIO_PULLUP);
			am_hal_gpio_pin_config(BSP_GPIO_UART1_RX,AM_HAL_GPIO_INPUT|AM_HAL_GPIO_PULLUP);
		 break;
	 default:
		 BSP_UART_LOG_PRINTF("[bsp_uart]: bsp_uart_disable ui32Modeule=%d is Error!\n",ui32Module);
		 break;
	}
}



