/**
*Module:TASK HeartRate
*Detail:handle receive-data
*Version: V 0.0.1
*Time: 2018/01/02
**/
#include "task_hdr.h"
#include "task_config.h"

#include "drv_heartrate.h"

#include "algo_hdr.h"
#include "bsp_rtc.h"

#include "gui_heartrate.h"
#include "timer_app.h"



#if DEBUG_ENABLED == 1 && TASK_HDR_LOG_ENABLED == 1
	#define TASK_HDR_LOG_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define TASK_HDR_LOG_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else       
	#define TASK_HDR_LOG_WRITESTRING(...)
	#define TASK_HDR_LOG_PRINTF(...)		        
#endif
#if !defined( WATCH_MODEL_PLUS ) 
static TaskHandle_t task_hdr_handle;
#endif
static bool m_initialized = false;
SemaphoreHandle_t UartHDRSemaphore = NULL;
static uint8_t hdrStatus = 0;
typedef struct{
	uint8_t *p_data;
	uint8_t length;
} _task_hdr_buffer;
static _task_hdr_buffer m_task_buffer = {NULL,0};

static void task_cb(uint32_t evt, uint8_t *p_data, uint32_t length)
{
	TASK_HDR_LOG_PRINTF("[TASK_HDR]:task_cb\n");	
	
	BaseType_t xHigherPriorityTaskWoken = pdFALSE ;
	m_task_buffer.p_data = (uint8_t *)p_data;
	m_task_buffer.length = length;	

	xSemaphoreGiveFromISR( UartHDRSemaphore, &xHigherPriorityTaskWoken );
}
#if !defined( WATCH_MODEL_PLUS ) 
static void task_hdr(void *pvParameters)
{
	TASK_HDR_LOG_PRINTF("[TASK_HDR]:task_hdr!!!\n");

	while (1)
	{

		if(hdrStatus == 0)
		{
			vTaskSuspend(NULL);
		}

		if( xSemaphoreTake( UartHDRSemaphore, portMAX_DELAY ) == pdTRUE )
		{
			drv_receive_handler(m_task_buffer.p_data, m_task_buffer.length);
		}
	}
}
#endif
/*开心率*/
void task_hdr_init(void)
{
	if(m_initialized)
	{
		return;
	}
	TASK_HDR_LOG_PRINTF("[TASK_HDR]:task_hdr_init\n");	
	m_initialized = true;
	hdrStatus = 1;
#if !defined( WATCH_MODEL_PLUS ) 
	if(task_hdr_handle == NULL)
	{
		UartHDRSemaphore = xSemaphoreCreateBinary();  //创建二值信号量
	  xTaskCreate(task_hdr, "Task HDR", TaskHDR_StackDepth, NULL, TaskHDR_Priority, &task_hdr_handle);
	}
	else
	{
		vTaskResume(task_hdr_handle);
	}
#endif
	drv_hdr_init(task_cb);
	timer_app_hdr_start();
  //开始监测采集心率,采集数清零
	g_HeartNum = 0;

}
/*关心率*/
void task_hdr_uninit(void)
{
	if(!m_initialized)
	{
		return;
	}
	TASK_HDR_LOG_PRINTF("[TASK_HDR]:task_hdr_uninit\n");
	m_initialized = false;
	drv_hdr_uninit();
	timer_app_hdr_stop(false);
	hdrStatus = 0;
	for(int i = 0; i < HEARTBUF_SIZE; i++)
	{
		g_HeartBuf[i] = 0;
	}
//	vTaskDelete(task_hdr_handle);
//	task_hdr_handle = NULL;
}
