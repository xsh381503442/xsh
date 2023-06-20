#include "sched_pc.h"
#include "rtt_log.h"
#include "lib_error.h"
#include "SEGGER_RTT.h"


#include "am_mcu_apollo.h"

#include "com_apollo2.h"
#include "lib_scheduler.h"

#include "am_bootloader.h"

#include "lib_boot.h"
#include "lib_boot_setting.h"
#include "drv_pc.h"
#include "dev_dfu.h"
#include "cmd_pc.h"


#define MOUDLE_LOG_ENABLED 							1
#define MOUDLE_DEBUG_ENABLED   					1
#define MOUDLE_NAME                     "[SCHED_PC]:"

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







static void event_handle(uint32_t *p_data, uint32_t length)
{
	LOG_PRINTF(MOUDLE_NAME"event_handle\n");
	
	
	switch(((p_data[0]<<24)>>24))
	{
	
		case PC_UART_CCOMMAND_UPDATE_ALGORITHM_INFO_APOLLO2:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_CCOMMAND_UPDATE_ALGORITHM_INFO_APOLLO2\n");
			_boot_setting boot_setting;
			lib_boot_setting_read(&boot_setting);
			_info_apollo2 info_apollo2;
			info_apollo2.command = PC_UART_RESPONSE_UPDATE_ALGORITHM_INFO_APOLLO2 | PC_UART_COMMAND_MASK_UPDATE_ALGORITHM;
			info_apollo2.bootloader_version = boot_setting.bootloader_version;
			info_apollo2.application_version = boot_setting.application_version;
			info_apollo2.update_version = boot_setting.update_version;
			info_apollo2.pcb_version = boot_setting.pcb_version;
			drv_pc_send_data((uint8_t *)&info_apollo2, sizeof(_info_apollo2));	
		}break;
		case PC_UART_CCOMMAND_UPDATE_ALGORITHM_JUMP_BOOTLOADER:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_CCOMMAND_UPDATE_ALGORITHM_JUMP_BOOTLOADER\n");
			uint32_t command[1] = {PC_UART_RESPONSE_UPDATE_ALGORITHM_JUMP_BOOTLOADER | PC_UART_COMMAND_MASK_UPDATE_ALGORITHM};
			drv_pc_send_data((uint8_t *)command, sizeof(command));
			lib_boot_jump_to_bootloader(BOOT_SETTING_ENTER_BOOT_REASON_DFU_PC);
			
		}break;		
		case PC_UART_CCOMMAND_UPDATE_ALGORITHM_JUMP_APPLICATION:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_CCOMMAND_UPDATE_ALGORITHM_JUMP_APPLICATION\n");
			uint32_t command[1] = {PC_UART_RESPONSE_UPDATE_ALGORITHM_JUMP_APPLICATION | PC_UART_COMMAND_MASK_UPDATE_ALGORITHM};
			drv_pc_send_data((uint8_t *)command, sizeof(command));
			lib_boot_jump_to_bootloader(BOOT_SETTING_ENTER_BOOT_REASON_JUMP_APPLICATION);
			
		}break;		
		case PC_UART_CCOMMAND_UPDATE_ALGORITHM_JUMP_UPDATE_ALGORITHM:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_CCOMMAND_UPDATE_ALGORITHM_JUMP_UPDATE_ALGORITHM\n");
			uint32_t command[1] = {PC_UART_RESPONSE_UPDATE_ALGORITHM_JUMP_UPDATE_ALGORITHM | PC_UART_COMMAND_MASK_UPDATE_ALGORITHM};
			drv_pc_send_data((uint8_t *)command, sizeof(command));
			lib_boot_jump_to_update_algorithm();
			
			
		}break;		
		case PC_UART_CCOMMAND_UPDATE_ALGORITHM_JUMP_RESET:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_CCOMMAND_UPDATE_ALGORITHM_JUMP_RESET\n");
			uint32_t command[1] = {PC_UART_RESPONSE_UPDATE_ALGORITHM_JUMP_RESET | PC_UART_COMMAND_MASK_UPDATE_ALGORITHM};
			drv_pc_send_data((uint8_t *)command, sizeof(command));
			am_hal_reset_por();		
			
		}break;			
		
		default:{
			
			uint32_t command[1] = {PC_UART_CCOMMAND_UPDATE_ALGORITHM_UNSPPORT};
			drv_pc_send_data((uint8_t *)command, sizeof(command));
			
		}break;	
	
	
	}		
	
	
	
	
	
}

static void sched_evt(uint32_t evt, uint8_t *p_data, uint32_t length)
{
	
	DEBUG_HEXDUMP(p_data,length);


	switch(evt)
	{
		case DRV_PC_EVT_UART_DATA:{
			DEBUG_PRINTF(MOUDLE_NAME"DRV_PC_EVT_UART_DATA\n");
			
			uint32_t *pp_data = (uint32_t *)p_data;
			
			if(pp_data[0] & PC_UART_COMMAND_MASK_UPDATE_ALGORITHM)
			{
				DEBUG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_MASK_UPDATE_ALGORITHM\n");
				event_handle(pp_data, length);
			}

			
		}break;
	
		default:
			break;
	}
	
}




void sched_pc_init(void)
{

	DEBUG_PRINTF(MOUDLE_NAME"sched_pc_init\n");
	
	drv_pc_init(sched_evt);
	
}









