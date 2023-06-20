#include "task_config.h"
#include "task_uart.h"

#if DEBUG_ENABLED == 1 && TASK_UART_LOG_ENABLED == 1
	#define TASK_UART_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define TASK_UART_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define TASK_UART_WRITESTRING(...)
	#define TASK_UART_PRINTF(...)		        
#endif
TaskHandle_t	TaskUartHandle = NULL;
SemaphoreHandle_t UartHeartrateSemaphore = NULL;

void CreateUartTask(void)
{
	if( TaskUartHandle == NULL )
	{
	   TASK_UART_PRINTF("[task_uart]:CreateUartTask\n");
		 UartHeartrateSemaphore = xSemaphoreCreateBinary();  //创建二值信号量
	   xTaskCreate(TaskUart,"Task Uart",TaskUart_StackDepth,0,TaskUart_Priority,&TaskUartHandle);
	}
}
void CloseUartTask(void)
{
	vTaskDelete(TaskUartHandle);
	TaskUartHandle = NULL;
}
void TaskUart(void* pvParameter)
{
	(void)pvParameter;
	while(1)
	{
		if( xSemaphoreTake( UartHeartrateSemaphore, portMAX_DELAY ) == pdTRUE )
		{
			TASK_UART_PRINTF("[task_uart]:TaskUart--\n");
			#if !defined WATCH_COD_BSP
			if(GetHdrFlag())
			{
				SetHdrFlag(false);
				drv_receivePacket();
			}
			#endif
		}
	}
}

