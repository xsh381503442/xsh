#ifndef __DRV_APOLLO2_H__
#define __DRV_APOLLO2_H__




#include <stdint.h>

#include "app_uart.h"
#define UART_TX_BUF_SIZE                256                                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE                256                                         /**< UART RX buffer size. */


#define APP_UART_TIMEOUT        (APP_UART_DATA+1)
#define APP_UART_CTS_INTERRUPT  (APP_UART_DATA+2)

#define APOLLO2_RX_PIN_NUMBER  5
#define APOLLO2_TX_PIN_NUMBER  6			
#define APOLLO2_CTS_PIN_NUMBER 7
#define APOLLO2_RTS_PIN_NUMBER 8  
#define HWFC           false


#define TASK_UART_DELAY                     800
#define TASK_UART_DELAY_COUNT_OUT           3



typedef void (* drv_apollo2_callback)(uint32_t evt, uint8_t *p_data, uint32_t length);
typedef void (* apollo2_user_callback)(uint32_t evt, uint8_t *p_data, uint32_t length);

typedef struct {

	uint8_t *p_data;
	uint32_t length;
	apollo2_user_callback user_callback;
} _uart_data_param;
void drv_apollo2_init(drv_apollo2_callback callback);
uint32_t drv_ble_send_data(_uart_data_param *param);
uint32_t drv_apollo2_uart_put(uint8_t *p_data, uint32_t length);
void drv_apollo2_uart_close(void);
uint8_t read_m_state(void);


void drv_apollo2_on_cts_high(void);
void drv_apollo2_on_cts_low(void);
uint8_t drv_apollo2_send(_uart_data_param *param);

#endif //__DRV_APOLLO2_H__
