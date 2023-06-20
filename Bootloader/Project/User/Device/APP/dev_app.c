#include "dev_app.h"
#include "rtt_log.h"
#include "lib_error.h"
#include "SEGGER_RTT.h"


#include "am_mcu_apollo.h"
#include "drv_pc.h"

#include "dev_extFlash.h"
#include "com_apollo2.h"
#include "lib_boot_setting.h"
#include "drv_extFlash.h"
#include "dev_extFlash.h"



#include "lib_boot.h"
#include "lib_boot_setting.h"
#include "cmd_pc.h"
#include "drv_ble.h"
#ifdef AM_FREERTOS 
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "portmacro.h"
#include "portable.h"
#include "event_groups.h"
#endif 

#include "am_util.h"

#define MOUDLE_LOG_ENABLED 							1
#define MOUDLE_DEBUG_ENABLED   					1
#define MOUDLE_NAME                     "[DEV_APP]:"

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


//#define DELAY_MS(X)                 am_util_delay_ms(X)
#define DELAY_MS(X)                 vTaskDelay(X / portTICK_PERIOD_MS)
//#define DELAY_MS(X)                   do{TickType_t xLastWakeTime;xLastWakeTime = xTaskGetTickCount ();vTaskDelayUntil( &xLastWakeTime, X / portTICK_PERIOD_MS );}while(0)



void dev_app_event_handle(uint32_t *p_data, uint32_t length)
{
	LOG_PRINTF(MOUDLE_NAME"dev_app_event_handle\n");

	switch(p_data[0]&0x0000FF00)
	{
		case PC_UART_CCOMMAND_APPLICATION_INFO_APOLLO2:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_CCOMMAND_APPLICATION_INFO_APOLLO2\n");
			_boot_setting boot_setting;
			lib_boot_setting_read(&boot_setting);
			_info_apollo2 info_apollo2;
			info_apollo2.command = PC_UART_RESPONSE_APPLICATION_INFO_APOLLO2 | PC_UART_COMMAND_MASK_APPLICATION;
			info_apollo2.bootloader_version = boot_setting.bootloader_version;
			info_apollo2.application_version = boot_setting.application_version;
			info_apollo2.update_version = boot_setting.update_version;
			info_apollo2.pcb_version = boot_setting.pcb_version;
			info_apollo2.chip_id_0 = AM_REG(MCUCTRL, CHIPID0);
			info_apollo2.chip_id_1 = AM_REG(MCUCTRL, CHIPID1);		
			LOG_PRINTF(MOUDLE_NAME"chip_id_0 --> 0x%08X\n",info_apollo2.chip_id_0);
			LOG_PRINTF(MOUDLE_NAME"chip_id_1 --> 0x%08X\n",info_apollo2.chip_id_1);			
			drv_pc_send_data((uint8_t *)&info_apollo2, sizeof(_info_apollo2));	
		}break;
		case PC_UART_CCOMMAND_APPLICATION_JUMP_BOOTLOADER:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_CCOMMAND_APPLICATION_JUMP_BOOTLOADER\n");
			uint32_t command[1] = {PC_UART_RESPONSE_APPLICATION_JUMP_BOOTLOADER | PC_UART_COMMAND_MASK_APPLICATION};
			drv_pc_send_data((uint8_t *)command, sizeof(command));
			drv_pc_uninit();
			DELAY_MS(10);
			lib_boot_jump_to_bootloader(BOOT_SETTING_ENTER_BOOT_REASON_DFU_PC);
			
		}break;		
		case PC_UART_CCOMMAND_APPLICATION_JUMP_APPLICATION:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_CCOMMAND_APPLICATION_JUMP_APPLICATION\n");
			uint32_t command[1] = {PC_UART_RESPONSE_APPLICATION_JUMP_APPLICATION | PC_UART_COMMAND_MASK_APPLICATION};
			drv_pc_send_data((uint8_t *)command, sizeof(command));
			lib_boot_jump_to_bootloader(BOOT_SETTING_ENTER_BOOT_REASON_JUMP_APPLICATION);
			
		}break;		
		case PC_UART_CCOMMAND_APPLICATION_JUMP_UPDATE_ALGORITHM:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_CCOMMAND_APPLICATION_JUMP_UPDATE_ALGORITHM\n");
			uint32_t command[1] = {PC_UART_RESPONSE_APPLICATION_JUMP_UPDATE_ALGORITHM | PC_UART_COMMAND_MASK_APPLICATION};
			drv_pc_send_data((uint8_t *)command, sizeof(command));
			lib_boot_jump_to_update_algorithm();
			
		}break;		
		case PC_UART_CCOMMAND_APPLICATION_RESET:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_CCOMMAND_APPLICATION_RESET\n");
			uint32_t command[1] = {PC_UART_RESPONSE_APPLICATION_RESET | PC_UART_COMMAND_MASK_APPLICATION};
			drv_pc_send_data((uint8_t *)command, sizeof(command));
			#ifdef AM_FREERTOS 	
				taskENTER_CRITICAL();
			#endif 
			am_hal_reset_por();				
			
		}break;			
		
		case PC_UART_CCOMMAND_APPLICATION_BLE_INFO:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_CCOMMAND_APPLICATION_BLE_INFO\n");
			uint32_t command[1] = {PC_UART_CCOMMAND_APPLICATION_BLE_INFO | PC_UART_COMMAND_MASK_APPLICATION};
			_uart_data_param param;
			param.user_callback = NULL;
			param.p_data = (uint8_t *)command;
			param.length = sizeof(command);
			uint8_t count = TASK_UART_DELAY_COUNT_OUT;
			while(drv_ble_send_data(&param) != ERR_SUCCESS && count--)
			{
				DELAY_MS(TASK_UART_DELAY);
			}	
	
		}break;		
		case PC_UART_CCOMMAND_APPLICATION_BLE_JUMP_BOOTLOADER:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_CCOMMAND_APPLICATION_BLE_JUMP_BOOTLOADER\n");
			uint32_t command[1] = {PC_UART_CCOMMAND_APPLICATION_BLE_JUMP_BOOTLOADER | PC_UART_COMMAND_MASK_APPLICATION};
			_uart_data_param param;
			param.user_callback = NULL;
			param.p_data = (uint8_t *)command;
			param.length = sizeof(command);
			uint8_t count = TASK_UART_DELAY_COUNT_OUT;
			while(drv_ble_send_data(&param) != ERR_SUCCESS && count--)
			{
				DELAY_MS(TASK_UART_DELAY);
			}	
	
		}break;	
		case PC_UART_CCOMMAND_APPLICATION_BLE_JUMP_APPLICATION:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_CCOMMAND_APPLICATION_BLE_JUMP_APPLICATION\n");
		}break;	
		case PC_UART_CCOMMAND_APPLICATION_BLE_RESET:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_CCOMMAND_APPLICATION_BLE_RESET\n");

		}break;	

		
		default:{
			
			uint32_t command[1] = {PC_UART_CCOMMAND_APPLICATION_UNSPPORT | PC_UART_COMMAND_MASK_APPLICATION};
			drv_pc_send_data((uint8_t *)command, sizeof(command));
			
		}break;
	}


}



















