#include "task_pc.h"
#include "rtt_log.h"
#include "lib_error.h"
#include "SEGGER_RTT.h"

#include "am_mcu_apollo.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "portmacro.h"
#include "portable.h"
#include "event_groups.h"
#include "semphr.h"

#include "com_apollo2.h"
#include "dev_dfu.h"
#include "lib_boot.h"
#include "lib_boot_setting.h"
#include <string.h>

#include "drv_pc.h"
#include "dev_hwt.h"
#include "cmd_pc.h"
#include "dev_app.h"
#include "dev_bat.h"

#define MOUDLE_LOG_ENABLED 							1
#define MOUDLE_DEBUG_ENABLED   					1
#define MOUDLE_NAME                     "[TASK_PC]:"

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

extern _dev_bat_event   g_bat_evt;
static TaskHandle_t task_handle;
static EventGroupHandle_t evt_handle;
static bool m_initialized = false;


#define TASK_DFU_DATA_LENGTH             256

typedef struct{
	uint32_t *p_data;
	uint32_t length;
} _task_buffer;
static _task_buffer m_task_buffer = {NULL,0};
static void task_cb(uint32_t evt, uint8_t *p_data, uint32_t length)
{
	BaseType_t xHigherPriorityTaskWoken, xResult;
	xHigherPriorityTaskWoken = pdFALSE;
	m_task_buffer.p_data = (uint32_t *)p_data;
	m_task_buffer.length = length;	
	xResult = xEventGroupSetBitsFromISR(evt_handle, (evt),
																			&xHigherPriorityTaskWoken);

	if (xResult == pdPASS)
	{
		 portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
//	else
//	{
//		ERR_HANDLER(ERR_FORBIDDEN);
//	}
}


static void task(void *pvParameters)
{
	DEBUG_PRINTF(MOUDLE_NAME"task\n");

	uint32_t bitSet;

	
	
	#ifdef LAYER_BOOTLOADER  
	if(g_bat_evt.cable_status == DEV_BAT_EVENT_USB_CONN_CHARGING)
	{
		_boot_setting boot_setting;
		lib_boot_setting_read(&boot_setting);
		_info_apollo2 info_apollo2;
		info_apollo2.command = PC_UART_RESPONSE_BOOTLOADER_INFO_APOLLO2 | PC_UART_CCOMMAND_MASK_BOOTLOADER;
		info_apollo2.bootloader_version = boot_setting.bootloader_version;
		info_apollo2.application_version = boot_setting.application_version;
		info_apollo2.update_version = boot_setting.update_version;
		info_apollo2.pcb_version = boot_setting.pcb_version;
		info_apollo2.chip_id_0 = AM_REG(MCUCTRL, CHIPID0);
		info_apollo2.chip_id_1 = AM_REG(MCUCTRL, CHIPID1);
		memcpy(info_apollo2.model,boot_setting.model,sizeof(info_apollo2.model));
		drv_pc_send_data((uint8_t *)&info_apollo2, sizeof(_info_apollo2));			
	}
	
	#endif	
	

	
	
	while (1)
	{
		bitSet = xEventGroupWaitBits(evt_handle, 0xFF, pdTRUE,
												pdFALSE, portMAX_DELAY);
		DEBUG_PRINTF(MOUDLE_NAME"bitSet : %04X\n",bitSet);
		

		switch(bitSet)
		{
			case DRV_PC_EVT_UART_DATA:{
				LOG_PRINTF(MOUDLE_NAME"DRV_PC_EVT_UART_DATA\n");

				#ifdef LAYER_BOOTLOADER  
				dev_dfu_event_handle(m_task_buffer.p_data, m_task_buffer.length);
				#endif
				#ifdef LAYER_APPLICATION 
				if(m_task_buffer.p_data[0] & ((1<<PC_UART_CCOMMAND_MASK_APPLICATION_POSITION)<<8))
				{
					dev_app_event_handle(m_task_buffer.p_data, m_task_buffer.length);
				}
//				else if(m_task_buffer.p_data[0] & ((1<<PC_UART_CCOMMAND_MASK_HARDWARE_TEST_POSITION)<<8))
//				{
//					dev_hwt_event_handle(m_task_buffer.p_data, m_task_buffer.length);
//				
//				}
				
				
				#endif					
			


				
				
				
			}break;
		
			
			default:{

				
			}break;
		}
		
		
	}
}

void task_pc_init(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"task_pc_init\n");
	
	if(m_initialized)
	{
		return;
	}
	
	m_initialized = true;
	

	drv_pc_init(task_cb);

	evt_handle = xEventGroupCreate();

	if(evt_handle == NULL)
	{
		DEBUG_PRINTF(MOUDLE_NAME"xEventGroupCreate failed\n");
		ERR_HANDLER(ERR_NULL);
	}
	
	BaseType_t xResult = xTaskCreate(task, "PC", 1024+512, 0, 4, &task_handle);
	if (xResult == pdFAIL)
	{
		ERR_HANDLER(ERR_FORBIDDEN);
	}	
}

void task_pc_uninit(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"task_pc_uninit\n");
	if(!m_initialized)
	{
		return;
	}	
	m_initialized = false;
	
	drv_pc_uninit();
	vTaskDelete(task_handle);
	vEventGroupDelete(evt_handle);

}



