#ifndef _TASK_UART_H__
#define _TASK_UART_H__

#include <stdint.h>
#include "task_config.h"
#include "drv_heartrate.h"
#include "isr_uart.h"
#include "isr_gpio.h"

extern TaskHandle_t	TaskUartHandle;
extern SemaphoreHandle_t UartHeartrateSemaphore;
extern void CreateUartTask(void);
extern void CloseUartTask(void);
extern void TaskUart(void* pvParameter);

#endif
