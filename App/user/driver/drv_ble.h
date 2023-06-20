#ifndef __DRV_BLE_H__
#define __DRV_BLE_H__





#include <stdint.h>
#include "watch_config.h"

#define DRV_BLE_UART_BAUDRATE                     115200
#define DRV_BLE_UART_TX_BUF_SIZE                  (256)
#define DRV_BLE_UART_RX_BUF_SIZE                  (256*2)
#define DRV_BLE_UART_MOUDLE                       1
#define DRV_BLE_UART_TX_PIN                       18
#define DRV_BLE_UART_RX_PIN                       19
#define DRV_BLE_UART_CTS_PIN                      29
#if defined( WATCH_MODEL_PLUS ) 
#define DRV_BLE_UART_RTS_PIN                      4
#else
#define DRV_BLE_UART_RTS_PIN                      30
#endif

#define DRV_BLE_POLARITY_CTS_PIN                 1


#define DRV_BLE_EVT_UART_DATA                    1
#define DRV_BLE_EVT_UART_CTS_INTERRUPT           2


#define TASK_UART_DELAY                     800
#define TASK_UART_DELAY_COUNT_OUT           3

typedef void (* ble_callback)(uint32_t evt, uint8_t *p_data, uint32_t length);

typedef void (* ble_user_callback)(uint32_t evt, uint8_t *p_data, uint32_t length);

typedef struct {

	uint8_t *p_data;
	uint32_t length;
	ble_user_callback user_callback;
} _uart_data_param;

void drv_ble_init(ble_callback ble_cb);
uint32_t drv_ble_send_data(_uart_data_param *param);
uint32_t drv_ble_uart_put(uint8_t *p_data, uint32_t length);
void drv_ble_uart_close(void);


void drv_ble_on_cts_low(void);
void drv_ble_on_cts_high(void);
void drv_ble_uart_close_on_ble_sys_off(void);

void drv_ble_timer_uninit(void);
void drv_ble_timer_init(void);
void drv_ble_timer_start(uint32_t timeout_msec);
void drv_ble_timer_stop(void);










#endif //__DRV_BLE_H__
