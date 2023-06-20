#ifndef __DRV_PC_H__
#define __DRV_PC_H__



#include <stdint.h>
#include "watch_config.h"

#define DRV_PC_UART_BAUDRATE                     460800//115200//460800//921600
#define DRV_PC_UART_TX_BUF_SIZE                  (256)
#define DRV_PC_UART_RX_BUF_SIZE                  (256)
#define DRV_PC_UART_MOUDLE                       0

#if defined( WATCH_MODEL_PLUS ) 
#define DRV_PC_UART_TX_PIN                       30
#define DRV_PC_UART_RX_PIN                       31
#define	DRV_PV_UART_TX_CONFIG					AM_HAL_PIN_30_UART0TX
#define	DRV_PV_UART_RX_CONFIG					AM_HAL_PIN_31_UART0RX
#else
#define DRV_PC_UART_TX_PIN                       22
#define DRV_PC_UART_RX_PIN                       23

#define	DRV_PV_UART_TX_CONFIG					AM_HAL_PIN_22_UART0TX
#define	DRV_PV_UART_RX_CONFIG					AM_HAL_PIN_23_UART0RX
#endif


#define DRV_PC_EVT_UART_DATA           1



typedef void (* pc_callback)(uint32_t evt, uint8_t *p_data, uint32_t length);

void drv_pc_init(pc_callback pc_cb);
void drv_pc_uninit(void);
void drv_pc_send_data(uint8_t *p_data, uint32_t length);

void drv_pc_send_data_buf(uint8_t *p_data, uint32_t length);











#endif //__DRV_PC_H__
