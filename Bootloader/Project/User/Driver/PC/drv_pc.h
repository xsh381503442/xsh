#ifndef __DRV_PC_H__
#define __DRV_PC_H__

#include <stdint.h>
#include "board.h"



#define DRV_PC_UART_BAUDRATE                     460800//115200//460800//921600
#define DRV_PC_UART_TX_BUF_SIZE                  (256)
#define DRV_PC_UART_RX_BUF_SIZE                  (256)
#define DRV_PC_UART_MOUDLE                       BSP_APOLLO2_DRV_PC_UART_MOUDLE
#define DRV_PC_UART_TX_PIN                       BSP_APOLLO2_DRV_PC_UART_TX_PIN
#define DRV_PC_UART_RX_PIN                       BSP_APOLLO2_DRV_PC_UART_RX_PIN

//#define DRV_PC_UART_MOUDLE                       0
//#define DRV_PC_UART_TX_PIN                       22
//#define DRV_PC_UART_RX_PIN                       23



#define DRV_PC_EVT_UART_DATA           1


typedef void (* pc_callback)(uint32_t evt, uint8_t *p_data, uint32_t length);

void drv_pc_init(pc_callback pc_cb);
void drv_pc_uninit(void);
void drv_pc_send_data(uint8_t *p_data, uint32_t length);


#endif //__DRV_PC_H__
