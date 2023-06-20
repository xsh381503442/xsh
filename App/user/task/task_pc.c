#include "task_pc.h"
#include "task_config.h"
#include "drv_pc.h"
#include "am_mcu_apollo.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "portmacro.h"
#include "portable.h"
#include "event_groups.h"
#include "semphr.h"

#include "cmd_pc.h"
#include "cmd_ble.h"
#include "lib_error.h"
#include "drv_ble.h"
#include "lib_boot_setting.h"
#include "lib_boot.h"
#include "task_ble.h"
#include "task_display.h"
#include "gui_hwt.h"
#include "drv_extFlash.h"
#include "drv_buzzer.h"
#include "drv_lis3mdl.h"
#include "drv_lsm6dsl.h"
#include "drv_ms5837.h"
#include "drv_ublox.h"
#include "com_data.h"
#include "lib_app_data.h" 
#include "mode_power_on.h"
#include "com_data.h"
#include "drv_battery.h"
#include "mode_power_off.h"
#include "bsp.h"
#include "drv_light.h"
#include "drv_buzzer.h"
#include "drv_motor.h"
#include "algo_magn_cal.h"
#ifdef WATCH_CHIP_TEST
#include "si117x_functions.h"
#include "drv_hrt.h"
#endif
#if defined WATCH_IMU_CALIBRATION
#include "algo_imu_calibration.h"
#endif 
#include "com_dial.h"




#include "drv_gt24l24.h"


#if DEBUG_ENABLED == 1 && TASK_PC_LOG_ENABLED == 1
	#define TASK_PC_LOG_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define TASK_PC_LOG_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else       
	#define TASK_PC_LOG_WRITESTRING(...)
	#define TASK_PC_LOG_PRINTF(...)		        
#endif


//#define DELAY_MS(X)                 am_util_delay_ms(X)
#define DELAY_MS(X)                 vTaskDelay(X / portTICK_PERIOD_MS)
//#define DELAY_MS(X)                   do{TickType_t xLastWakeTime;xLastWakeTime = xTaskGetTickCount ();vTaskDelayUntil( &xLastWakeTime, X / portTICK_PERIOD_MS );}while(0)

extern uint8_t rtc_hw_flag;


static TaskHandle_t task_handle;
static EventGroupHandle_t evt_handle;
//static bool m_initialized = false;
bool m_initialized = false;

extern SetValueStr SetValue;
#if defined WATCH_IMU_CALIBRATION

extern uint8_t imu_cal_flag;
#endif
typedef struct{
	uint32_t *p_data;
	uint32_t length;
} _task_buffer;
static _task_buffer m_task_buffer = {NULL,0};


static void app_event_handle(uint32_t *p_data, uint32_t length);
static void hwt_event_handle(uint32_t *p_data, uint32_t length);

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
	TASK_PC_LOG_PRINTF("[TASK_PC]:task\n");
	uint32_t bitSet;
	while (1)
	{
		bitSet = xEventGroupWaitBits(evt_handle, 0xFF, pdTRUE,
												pdFALSE, portMAX_DELAY);
		switch(bitSet)
		{
			case DRV_PC_EVT_UART_DATA:{
				TASK_PC_LOG_PRINTF("[TASK_PC]:DRV_PC_EVT_UART_DATA\n");
				if(m_task_buffer.p_data[0] & ((1<<PC_UART_CCOMMAND_MASK_APPLICATION_POSITION)<<8))
				{
					app_event_handle(m_task_buffer.p_data, m_task_buffer.length);
				}
				else if(m_task_buffer.p_data[0] & ((1<<PC_UART_CCOMMAND_MASK_HARDWARE_TEST_POSITION)<<8))
				{
					hwt_event_handle(m_task_buffer.p_data, m_task_buffer.length);
				}				
				
				
				
			}break;
		
			
			default:{

				
			}break;
		}
		
		
	}
}


void task_pc_init(void)
{
	if(m_initialized)
	{
		return;
	}
	TASK_PC_LOG_PRINTF("[TASK_PC]:task_pc_init\n");	
	m_initialized = true;
	
	drv_pc_init(task_cb);

	evt_handle = xEventGroupCreate();

	if(evt_handle == NULL)
	{
		ERR_HANDLER(ERR_NO_MEM);
	}	
	BaseType_t xResult = xTaskCreate(task, "PC", TaskPC_StackDepth, NULL, TaskPC_Priority, &task_handle);
	if (xResult == pdFAIL)
	{
		ERR_HANDLER(ERR_NO_MEM);
	}	
}

void task_pc_uninit(void)
{
	if(!m_initialized)
	{
		return;
	}	
	TASK_PC_LOG_PRINTF("[TASK_PC]:task_pc_uninit\n");	
	m_initialized = false;
	#if defined WATCH_IMU_CALIBRATION
	while(imu_cal_flag == CAL_ING);
	#endif 
	drv_pc_uninit();
	vTaskDelete(task_handle);
	vEventGroupDelete(evt_handle);

}


static void pc_uart_ccommand_application_flash_read_raw(void);
static void pc_uart_ccommand_application_flash_write_raw(void);
static void pc_uart_ccommand_application_flash_erase_raw(void);
static void pc_uart_ccommand_application_get_watch_face_info(void);
static void pc_uart_ccommand_application_get_watch_face_info_2(void);
#ifdef COD 
 void pc_uart_ccommand_application_mac_read(void);
 void pc_uart_ccommand_application_mac_write(void);
#endif

static void app_event_handle(uint32_t *p_data, uint32_t length)
{

	switch(p_data[0]&0x0000FF00)
	{
		case PC_UART_CCOMMAND_APPLICATION_INFO_APOLLO2:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_CCOMMAND_APPLICATION_INFO_APOLLO2\n");	
			_boot_setting boot_setting;
			lib_boot_setting_read(&boot_setting);
			_info_apollo2 info_apollo2;
			info_apollo2.command = PC_UART_RESPONSE_APPLICATION_INFO_APOLLO2 | PC_UART_COMMAND_MASK_APPLICATION;
			info_apollo2.bootloader_version = boot_setting.bootloader_version;
			info_apollo2.application_version = boot_setting.application_version;
			info_apollo2.update_version = boot_setting.update_version;
			if (rtc_hw_flag == 1)
			{
				info_apollo2.pcb_version = boot_setting.pcb_version | 0x0100;
			}
			else
			{
			info_apollo2.pcb_version = boot_setting.pcb_version;
			}
			info_apollo2.chip_id_0 = AM_REG(MCUCTRL, CHIPID0);
			info_apollo2.chip_id_1 = AM_REG(MCUCTRL, CHIPID1);	
			memcpy(info_apollo2.model,boot_setting.model,sizeof(info_apollo2.model));
			info_apollo2.protocal_version = BLE_PROTOCAL_VERSION_CODE;
			drv_pc_send_data((uint8_t *)&info_apollo2, sizeof(_info_apollo2));	
		}break;
		case PC_UART_CCOMMAND_APPLICATION_JUMP_BOOTLOADER:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_CCOMMAND_APPLICATION_JUMP_BOOTLOADER\n");	
			uint32_t command[1] = {PC_UART_RESPONSE_APPLICATION_JUMP_BOOTLOADER | PC_UART_COMMAND_MASK_APPLICATION};
			drv_pc_send_data((uint8_t *)command, sizeof(command));
			drv_pc_uninit();
			DELAY_MS(10);
			mode_power_off_store_set_value();
			lib_app_data_erase();
//			lib_app_data_init();
			lib_boot_jump_to_bootloader(BOOT_SETTING_ENTER_BOOT_REASON_DFU_PC);
		}break;		
		
		case PC_UART_CCOMMAND_APPLICATION_RESET:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_CCOMMAND_APPLICATION_RESET\n");	
			uint32_t command[1] = {PC_UART_RESPONSE_APPLICATION_RESET | PC_UART_COMMAND_MASK_APPLICATION};
			drv_pc_send_data((uint8_t *)command, sizeof(command));
			taskENTER_CRITICAL();
			am_hal_reset_por();			
		}break;		

	case PC_UART_CCOMMAND_APPLICATION_FACTORY_RESET:{
		TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_CCOMMAND_APPLICATION_RESET\n");	
		uint32_t command[1] = {PC_UART_RESPONSE_APPLICATION_FACTORY_RESET | PC_UART_COMMAND_MASK_APPLICATION};
		drv_pc_send_data((uint8_t *)command, sizeof(command));
		DELAY_MS(100);
		Factory_Reset();
			
	}break; 	
		case PC_UART_CCOMMAND_APPLICATION_BLE_INFO:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_CCOMMAND_APPLICATION_BLE_INFO\n");	
			
			if(SetValue.SwBle == false)
			{

                if(am_hal_gpio_input_bit_read(DRV_BAT_USB_DETECT_PIN))
				{
					uint32_t command[2] = {PC_UART_RESPONSE_APPLICATION_BLE_INFO | PC_UART_COMMAND_MASK_APPLICATION,0xFFFFFFFE};
					drv_pc_send_data((uint8_t *)command, sizeof(command));
				}			
			}
			else{
			
				uint32_t command[1] = {PC_UART_CCOMMAND_APPLICATION_BLE_INFO | PC_UART_COMMAND_MASK_APPLICATION | PC_UART_CCOMMAND_MASK_BOOTLOADER};
				_uart_data_param param;
				param.user_callback = NULL;
				param.p_data = (uint8_t *)command;
				param.length = sizeof(command);
				uint8_t count = TASK_UART_DELAY_COUNT_OUT;
				while(drv_ble_send_data(&param) != ERR_SUCCESS && count--)
				{
					DELAY_MS(TASK_UART_DELAY);
				}				
			}
		}break;		
		case PC_UART_CCOMMAND_APPLICATION_BLE_JUMP_BOOTLOADER:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_CCOMMAND_APPLICATION_BLE_JUMP_BOOTLOADER\n");	
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
	

		case PC_UART_CCOMMAND_APPLICATION_FLASH_READ_RAW:{			
			pc_uart_ccommand_application_flash_read_raw();
		}break;
		case PC_UART_CCOMMAND_APPLICATION_FLASH_WRITE_RAW:{
			pc_uart_ccommand_application_flash_write_raw();
		}break;
		case PC_UART_CCOMMAND_APPLICATION_FLASH_ERASE_RAW:{
			pc_uart_ccommand_application_flash_erase_raw();
		}break;
		case PC_UART_CCOMMAND_APPLICATION_WATCH_FACE_INFO:{
			pc_uart_ccommand_application_get_watch_face_info();
		}break;
		case PC_UART_CCOMMAND_APPLICATION_WATCH_FACE_INFO2:{
			pc_uart_ccommand_application_get_watch_face_info_2();
		}break;
		#ifdef COD
		case PC_UART_CCOMMAND_APPLICATION_MAC_READ:{
			pc_uart_ccommand_application_mac_read();
		}break;
		case PC_UART_CCOMMAND_APPLICATION_MAC_WRITE:{
			pc_uart_ccommand_application_mac_write();
		}break;
		#endif
		default:{
			uint32_t command[1] = {PC_UART_CCOMMAND_APPLICATION_UNSPPORT | PC_UART_COMMAND_MASK_APPLICATION};
			drv_pc_send_data((uint8_t *)command, sizeof(command));
		}break;
	}


}



typedef struct __attribute__((packed))
{
	uint32_t cmd;
	uint32_t crc;
	uint32_t read_back;
	uint32_t address;
	uint32_t size;
	uint8_t data[256];
}_apollo_srv_flash_read_raw;

typedef struct __attribute__((packed))
{
	uint32_t cmd;
	uint32_t crc;
	uint32_t read_back;
	uint32_t address;
	uint32_t size;
}_apollo_srv_flash_write_raw;

typedef struct __attribute__((packed))
{
	uint32_t cmd;
	uint32_t read_back;
	uint32_t address;
	uint32_t size;
}_apollo_srv_flash_erase_raw;



void pc_uart_ccommand_application_flash_read_raw(void)
{
	_apollo_srv_flash_read_raw *cmd = (_apollo_srv_flash_read_raw *)(m_task_buffer.p_data);	
	TASK_PC_LOG_PRINTF("cmd->crc --> 0x%08X",cmd->crc);
	TASK_PC_LOG_PRINTF("cmd->read_back --> 0x%08X",cmd->read_back);	
	TASK_PC_LOG_PRINTF("cmd->address --> 0x%08X",cmd->address);
	TASK_PC_LOG_PRINTF("cmd->size --> 0x%08X",cmd->size);	
	
	_apollo_srv_flash_read_raw response;
	uint32_t crc_src = 0;			
	dev_extFlash_enable();	
	dev_extFlash_read(cmd->address, response.data, cmd->size);			
	dev_extFlash_disable();	
	crc_src = am_bootloader_fast_crc32(response.data,cmd->size);
	TASK_PC_LOG_PRINTF("crc_src --> 0x%08X",crc_src);		
	
	response.cmd = PC_UART_RESPONSE_APPLICATION_FLASH_READ_RAW | PC_UART_COMMAND_MASK_APPLICATION;
	response.crc = crc_src;
	response.read_back = cmd->read_back;
	response.address = cmd->address;
	response.size = cmd->size;
	
	drv_pc_send_data((uint8_t *)&response, 
	sizeof(response.cmd)+sizeof(response.crc)+sizeof(response.read_back)+sizeof(response.address)+sizeof(response.size)+response.size);	
}



static void pc_uart_ccommand_application_flash_write_raw(void)
{
	_apollo_srv_flash_write_raw *cmd = (_apollo_srv_flash_write_raw *)(m_task_buffer.p_data);	
	TASK_PC_LOG_PRINTF("cmd->read_back --> 0x%08X",cmd->read_back);	
	TASK_PC_LOG_PRINTF("cmd->crc --> 0x%08X",cmd->crc);
	TASK_PC_LOG_PRINTF("cmd->address --> 0x%08X",cmd->address);
	TASK_PC_LOG_PRINTF("cmd->size --> 0x%08X",cmd->size);	
	
	uint32_t response[3];
	response[0] = PC_UART_RESPONSE_APPLICATION_FLASH_WRITE_RAW | PC_UART_COMMAND_MASK_APPLICATION;
	response[1] = cmd->read_back;
	uint32_t crc_src = am_bootloader_fast_crc32((uint8_t *)m_task_buffer.p_data + 20,cmd->size);
	TASK_PC_LOG_PRINTF("crc_src --> 0x%08X",crc_src);
//	if(cmd->crc != crc_src)
//	{
//		response[2] = 0x01;
//	}
//	else
	{
		uint8_t buffer[256]; 
		dev_extFlash_enable();
		dev_extFlash_write(cmd->address, (uint8_t *)m_task_buffer.p_data + 20, cmd->size);                
		dev_extFlash_read(cmd->address, buffer, cmd->size);			
		dev_extFlash_disable();	
		crc_src = am_bootloader_fast_crc32(buffer,cmd->size);
		TASK_PC_LOG_PRINTF("crc_flash --> 0x%08X",crc_src);
		if(crc_src != cmd->crc)
		{
			response[2] = 0x02;
		}
		else{
			response[2] = 0x00;
		}       
	}
	drv_pc_send_data((uint8_t *)&response, sizeof(response));
}
static void pc_uart_ccommand_application_flash_erase_raw(void)
{
	_apollo_srv_flash_erase_raw *cmd = (_apollo_srv_flash_erase_raw *)(m_task_buffer.p_data);
	TASK_PC_LOG_PRINTF("cmd->read_back --> 0x%08X",cmd->read_back);
	TASK_PC_LOG_PRINTF("cmd->address --> 0x%08X",cmd->address);
	TASK_PC_LOG_PRINTF("cmd->size --> 0x%08X",cmd->size);	
	
	dev_extFlash_enable();		
	dev_extFlash_erase(cmd->address,cmd->size);
	dev_extFlash_disable();		
	
	uint32_t response[3];	
	response[0] = PC_UART_RESPONSE_APPLICATION_FLASH_ERASE_RAW | PC_UART_COMMAND_MASK_APPLICATION;
	response[1] = cmd->read_back;
	response[2] = 0x00;
	drv_pc_send_data((uint8_t *)&response, sizeof(response));	
}

#ifdef COD 
 void pc_uart_ccommand_application_mac_read(void)
{
	TASK_PC_LOG_PRINTF("%s\r\n",__func__);	
	uint32_t command[1] = {BLE_UART_RESPONSE_APPLICATION_MAC_READ|BLE_UART_CCOMMAND_MASK_APPLICATION};
	_uart_data_param param;
	param.user_callback = NULL;
	param.p_data = (uint8_t *)command;
	param.length = sizeof(command);
	uint8_t count = TASK_UART_DELAY_COUNT_OUT;
	while(drv_ble_send_data(&param) != ERR_SUCCESS && count--)
	{
		DELAY_MS(TASK_UART_DELAY);
	}	
}	
	
 void pc_uart_ccommand_application_mac_write(void)
{
	//0C-FE-5D-0B-FE-B2  ~ 0C-FE-5D-0B-FE-BA  cod test
	_ble_mac *rec_mac = (_ble_mac *)(m_task_buffer.p_data);
	_ble_mac  ble_mac;
	ble_mac.cmd = BLE_UART_RESPONSE_APPLICATION_MAC_WRITE|BLE_UART_CCOMMAND_MASK_APPLICATION;
	memcpy(&ble_mac.mac,&rec_mac->mac,6);
	//ble_mac.mac[0] = 0x0C;
	//ble_mac.mac[1] = 0xFE;
	//ble_mac.mac[2] = 0x5D;
	//ble_mac.mac[3] = 0x0B;
	//ble_mac.mac[4] = 0xFE;
	//ble_mac.mac[5] = 0xB2;
	TASK_PC_LOG_PRINTF("[TASK_PC]: ble_mac %x:%x:%x:%x:%x:%x\r\n",ble_mac.mac[0],ble_mac.mac[1],ble_mac.mac[2],ble_mac.mac[3],ble_mac.mac[4],ble_mac.mac[5]);
	_uart_data_param param;
	param.user_callback = NULL;
	param.p_data = (uint8_t *)&ble_mac;
	param.length = sizeof(ble_mac);
	uint8_t count = TASK_UART_DELAY_COUNT_OUT;
	while(drv_ble_send_data(&param) != ERR_SUCCESS && count--)
	{
		DELAY_MS(TASK_UART_DELAY);
	}
		
}


#endif
typedef struct __attribute__ ((packed))
{
	uint32_t cmd;
	
	uint16_t watch_face_0_index;     //手表当前表盘
	uint32_t watch_face_0_address;
	uint32_t watch_face_0_version;	
	uint32_t watch_face_0_valid;	
	uint8_t  watch_face_0_name[20];
	
	uint16_t watch_face_1_index;    //手表FLASH中第一个表盘
	uint32_t watch_face_1_address;
	uint32_t watch_face_1_version;
	uint32_t watch_face_1_valid;
	uint8_t  watch_face_1_name[20];
	
	uint16_t watch_face_2_index;
	uint32_t watch_face_2_address;
	uint32_t watch_face_2_version;
	uint32_t watch_face_2_valid;
	uint8_t  watch_face_2_name[20];

	uint16_t watch_face_3_index;
	uint32_t watch_face_3_address;
	uint32_t watch_face_3_version;	
	uint32_t watch_face_3_valid;
	uint8_t  watch_face_3_name[20];
	
	uint16_t watch_face_4_index;
	uint32_t watch_face_4_address;
	uint32_t watch_face_4_version;	
	uint32_t watch_face_4_valid;
	uint8_t  watch_face_4_name[20];
	
} _watch_face_info;

static void pc_uart_ccommand_application_get_watch_face_info(void)
{
	_watch_face_info info = {0};

	info.cmd = PC_UART_RESPONSE_APPLICATION_WATCH_FACE_INFO | PC_UART_COMMAND_MASK_APPLICATION;
	
	uint8_t buffer[34] = {0};
/*
第一个表盘	
*/		
	dev_extFlash_enable();		
	dev_extFlash_read(CUSTOMTHEME_START_ADDRESS, (uint8_t *)buffer, sizeof(buffer));	
	dev_extFlash_read(CUSTOMTHEME_START_ADDRESS+34, (uint8_t *)info.watch_face_1_name, sizeof(info.watch_face_1_name));	
	dev_extFlash_read(CUSTOMTHEME_START_ADDRESS+(200*1024-4), (uint8_t *)&info.watch_face_1_valid, 4);	
	dev_extFlash_disable();				
	info.watch_face_1_index = *(uint16_t *)((uint8_t *)buffer+32);
	info.watch_face_1_address = CUSTOMTHEME_START_ADDRESS;
	info.watch_face_1_version = *(uint32_t *)((uint8_t *)buffer+12);

	
	TASK_PC_LOG_PRINTF("[TASK_BLE]:watch_face_1_index --> 0x%04X\n",info.watch_face_1_index);
	TASK_PC_LOG_PRINTF("[TASK_BLE]:watch_face_1_address --> 0x%08X\n",info.watch_face_1_address);
	TASK_PC_LOG_PRINTF("[TASK_BLE]:watch_face_1_valid --> 0x%08X\n",info.watch_face_1_valid);
	
	
/*
第二个表盘	
*/			
	dev_extFlash_enable();		
	dev_extFlash_read(CUSTOMTHEME_START_ADDRESS+(200*1024), (uint8_t *)buffer, sizeof(buffer));	
	dev_extFlash_read(CUSTOMTHEME_START_ADDRESS+(200*1024)+34, (uint8_t *)info.watch_face_2_name, sizeof(info.watch_face_2_name));		
	dev_extFlash_read(CUSTOMTHEME_START_ADDRESS+(200*1024)+(200*1024-4), (uint8_t *)&info.watch_face_2_valid, 4);
	dev_extFlash_disable();				
	info.watch_face_2_index = *(uint16_t *)((uint8_t *)buffer+32);
	info.watch_face_2_address = CUSTOMTHEME_START_ADDRESS+(200*1024);
	info.watch_face_2_version = *(uint32_t *)((uint8_t *)buffer+12);	
	
	TASK_PC_LOG_PRINTF("[TASK_BLE]:watch_face_2_index --> 0x%04X\n",info.watch_face_2_index);
	TASK_PC_LOG_PRINTF("[TASK_BLE]:watch_face_2_address --> 0x%08X\n",info.watch_face_2_address);
	TASK_PC_LOG_PRINTF("[TASK_BLE]:watch_face_2_valid --> 0x%08X\n",info.watch_face_2_valid);	
	
/*
第三个表盘	
*/			
	dev_extFlash_enable();		
	dev_extFlash_read(CUSTOMTHEME_START_ADDRESS+(200*1024*2), (uint8_t *)buffer, sizeof(buffer));		
	dev_extFlash_read(CUSTOMTHEME_START_ADDRESS+(200*1024*2)+34, (uint8_t *)info.watch_face_3_name, sizeof(info.watch_face_3_name));	
	dev_extFlash_read(CUSTOMTHEME_START_ADDRESS+(200*1024*2)+(200*1024-4), (uint8_t *)&info.watch_face_3_valid, 4);			
	dev_extFlash_disable();				
	info.watch_face_3_index = *(uint16_t *)((uint8_t *)buffer+32);
	info.watch_face_3_address = CUSTOMTHEME_START_ADDRESS+(200*1024*2);
	info.watch_face_3_version = *(uint32_t *)((uint8_t *)buffer+12);	

	TASK_PC_LOG_PRINTF("[TASK_BLE]:watch_face_3_index --> 0x%04X\n",info.watch_face_3_index);
	TASK_PC_LOG_PRINTF("[TASK_BLE]:watch_face_3_address --> 0x%08X\n",info.watch_face_3_address);
	TASK_PC_LOG_PRINTF("[TASK_BLE]:watch_face_3_valid --> 0x%08X\n",info.watch_face_3_valid);

/*
第四个表盘	
*/			
	dev_extFlash_enable();		
	dev_extFlash_read(CUSTOMTHEME_START_ADDRESS+(200*1024*3), (uint8_t *)buffer, sizeof(buffer));		
	dev_extFlash_read(CUSTOMTHEME_START_ADDRESS+(200*1024*3)+34, (uint8_t *)info.watch_face_4_name, sizeof(info.watch_face_4_name));		
	dev_extFlash_read(CUSTOMTHEME_START_ADDRESS+(200*1024*3)+(200*1024-4), (uint8_t *)&info.watch_face_4_valid, 4);			
	dev_extFlash_disable();	
	#ifdef COD //第四个表盘为咕咚下载表盘,默认返回有效,防止下载工具写错
	info.watch_face_4_index = 0x10000;
	info.watch_face_4_address = CUSTOMTHEME_START_ADDRESS+(200*1024*3);
	info.watch_face_4_version = *(uint32_t *)((uint8_t *)buffer+12);
	info.watch_face_4_valid = 0xFEFFFFFF;
	#else
	info.watch_face_4_index = *(uint16_t *)((uint8_t *)buffer+32);
	info.watch_face_4_address = CUSTOMTHEME_START_ADDRESS+(200*1024*3);
	info.watch_face_4_version = *(uint32_t *)((uint8_t *)buffer+12);
	#endif
	
	TASK_PC_LOG_PRINTF("[TASK_BLE]:watch_face_4_index --> 0x%04X\n",info.watch_face_4_index);
	TASK_PC_LOG_PRINTF("[TASK_BLE]:watch_face_4_address --> 0x%08X\n",info.watch_face_4_address);
	TASK_PC_LOG_PRINTF("[TASK_BLE]:watch_face_4_valid --> 0x%08X\n",info.watch_face_4_valid);	
	
	
	/*
	当前表盘	
	*/	

	info.watch_face_0_index = DialData.Number;
	info.watch_face_0_address = 0xFFFFFFFF;
	info.watch_face_0_version = 0xFFFFFFFF;	
	info.watch_face_0_valid = 0xFFFFFFFF;	
	memset(info.watch_face_0_name,0xFF,sizeof(info.watch_face_0_name));
		
	TASK_PC_LOG_PRINTF("[TASK_BLE]:watch_face_0_index --> 0x%04X\n",info.watch_face_0_index);

	
	drv_pc_send_data((uint8_t *)&info, sizeof(_watch_face_info));	
}

static void pc_uart_ccommand_application_get_watch_face_info_2(void)
{
	_watch_face_info info = {0};

	info.cmd = PC_UART_RESPONSE_APPLICATION_WATCH_FACE_INFO_2 | PC_UART_COMMAND_MASK_APPLICATION;
	
	uint8_t buffer[34] = {0};	
	
/*
空表盘	
*/			
	info.watch_face_0_index = 0xFFFF;
	info.watch_face_0_address = 0xFFFFFFFF;
	info.watch_face_0_version = 0xFFFFFFFF;	
	info.watch_face_0_version = 0xFFFFFFFF;	
	memset(info.watch_face_0_name,0xFF,sizeof(info.watch_face_0_name));

/*
空表盘	
*/				
	info.watch_face_1_index = 0xFFFF;
	info.watch_face_1_address = 0xFFFFFFFF;
	info.watch_face_1_version = 0xFFFFFFFF;	
	info.watch_face_1_version = 0xFFFFFFFF;	
	memset(info.watch_face_1_name,0xFF,sizeof(info.watch_face_1_name));	
	
/*
第五个表盘	
*/		
	dev_extFlash_enable();		
	dev_extFlash_read(CUSTOMTHEME_START_ADDRESS+(200*1024*4), (uint8_t *)buffer, sizeof(buffer));		
	dev_extFlash_read(CUSTOMTHEME_START_ADDRESS+(200*1024*4)+34, (uint8_t *)info.watch_face_2_name, sizeof(info.watch_face_2_name));		
	dev_extFlash_read(CUSTOMTHEME_START_ADDRESS+(200*1024*4)+(200*1024-4), (uint8_t *)&info.watch_face_2_valid, 4);			
	dev_extFlash_disable();			
	info.watch_face_2_index = *(uint16_t *)((uint8_t *)buffer+32);
	info.watch_face_2_address = CUSTOMTHEME_START_ADDRESS+(200*1024*4);
	info.watch_face_2_version = *(uint32_t *)((uint8_t *)buffer+12);	
		
	
/*
第六个表盘	
*/		
	dev_extFlash_enable();		
	dev_extFlash_read(CUSTOMTHEME_START_ADDRESS+(200*1024*5), (uint8_t *)buffer, sizeof(buffer));		
	dev_extFlash_read(CUSTOMTHEME_START_ADDRESS+(200*1024*5)+34, (uint8_t *)info.watch_face_3_name, sizeof(info.watch_face_3_name));		
	dev_extFlash_read(CUSTOMTHEME_START_ADDRESS+(200*1024*5)+(200*1024-4), (uint8_t *)&info.watch_face_3_valid, 4);			
	dev_extFlash_disable();			
	info.watch_face_3_index = *(uint16_t *)((uint8_t *)buffer+32);
	info.watch_face_3_address = CUSTOMTHEME_START_ADDRESS+(200*1024*5);
	info.watch_face_3_version = *(uint32_t *)((uint8_t *)buffer+12);	
	
	
/*
第七个表盘	
*/			
	dev_extFlash_enable();		
	dev_extFlash_read(CUSTOMTHEME_START_ADDRESS+(200*1024*6), (uint8_t *)buffer, sizeof(buffer));		
	dev_extFlash_read(CUSTOMTHEME_START_ADDRESS+(200*1024*6)+34, (uint8_t *)info.watch_face_4_name, sizeof(info.watch_face_4_name));		
	dev_extFlash_read(CUSTOMTHEME_START_ADDRESS+(200*1024*6)+(200*1024-4), (uint8_t *)&info.watch_face_4_valid, 4);			
	dev_extFlash_disable();			
	info.watch_face_4_index = *(uint16_t *)((uint8_t *)buffer+32);
	info.watch_face_4_address = CUSTOMTHEME_START_ADDRESS+(200*1024*6);
	info.watch_face_4_version = *(uint32_t *)((uint8_t *)buffer+12);	
	
	drv_pc_send_data((uint8_t *)&info, sizeof(_watch_face_info));
}



static uint8_t *dev_extFlash_buffer_p = NULL;
extern _gui_hwt g_gui_hwt;


typedef struct {
	uint32_t command;
	int32_t x;
	int32_t y;
	int32_t z;
} _dev_hwt_motion;


typedef struct {
	uint32_t command;
	char pressure[8];
	char altitude[8];
	char temperature[8];
} _dev_hwt_pressure;


typedef struct {
	uint32_t command;
	uint8_t data[DRV_PC_UART_TX_BUF_SIZE-4];
}_hwt_ublox_data;			


static void pc_uart_command_hardware_test_mode_enable(void);
static void pc_uart_command_hardware_test_mode_disable(void);			
static void pc_uart_command_hardware_test_button_pwr_enable(void);	
static void pc_uart_command_hardware_test_button_pwr_disable(void);
static void pc_uart_command_hardware_test_button_bl_enable(void);	
static void pc_uart_command_hardware_test_button_bl_disable(void);
static void pc_uart_command_hardware_test_button_up_enable(void);	
static void pc_uart_command_hardware_test_button_up_disable(void);
static void pc_uart_command_hardware_test_button_ok_enable(void) ;	
static void pc_uart_command_hardware_test_button_ok_disable(void);	
static void pc_uart_command_hardware_test_button_dwn_enable(void);
static void pc_uart_command_hwardware_test_button_dwn_disable(void);
static void pc_uart_command_hardware_test_lcd_enable(void);	
static void pc_uart_command_hwardware_test_lcd_disable(void);
static void pc_uart_command_hardware_test_backlight_enable(void);
static void pc_uart_command_hardware_test_backlight_disable(void);
static void pc_uart_command_hardware_test_motor_enable(void);
static void pc_uart_command_hardware_test_motor_disable(void);
static void pc_uart_command_hardware_test_buzzer_enable(void);	
static void pc_uart_command_hardware_test_buzzer_disable(void);	
static void pc_uart_command_hardware_test_extflash_enable(void)	;	
static void extFlash_sector(uint32_t address,uint32_t size);
static void pressure_read(void);
static void pc_uart_command_hardware_test_extflash_disable(void);	
static void pc_uart_command_hardware_test_font_enable(void);
static void pc_uart_command_hardware_test_font_read(void);	
static void pc_uart_command_hardware_test_font_disable(void);
static void pc_uart_command_hardware_test_gsensor_enable(void);
static void pc_uart_command_hardware_test_gsensor_read(void);
static void pc_uart_command_hardware_test_gsensor_disable(void);
static void pc_uart_command_hardware_test_gyro_enable(void);	
static void pc_uart_command_hardware_test_gyro_read(void);
static void pc_uart_command_hardware_test_gyro_disable(void);
static void pc_uart_command_hardware_test_compass_enable(void);
static void pc_uart_command_hardware_test_compass_read(void);		
static void pc_uart_command_hardware_test_compass_disable(void);
static void pc_uart_command_hardware_test_gps_enable(void);	
static void pc_uart_command_hardware_test_gps_config_hot_start(void);		
static void pc_uart_command_hardware_test_gps_config_warm_start(void);
static void pc_uart_command_hardware_test_gps_config_cold_start(void);
static void pc_uart_command_hardware_test_gps_config_gps_mode(void);
static void pc_uart_command_hardware_test_gps_config_bd_mode(void);	
static void pc_uart_command_hardware_test_gps_config_gps_bd_mode(void);	
static void pc_uart_command_hardware_test_gps_config_nmea_gga(void);
static void pc_uart_command_hardware_test_gps_config_nmea_gsa(void);	
static void pc_uart_command_hardware_test_gps_config_nmea_gsv(void);
static void pc_uart_command_hardware_test_gps_config_nmea_rmc(void)	;
static void pc_uart_command_hardware_test_gps_config_nmea_vtg(void);
static void pc_uart_command_hardware_test_gps_config_nmea_gll(void);
static void pc_uart_command_hardware_test_gps_config_nmea_zda(void);	
static void pc_uart_command_hardware_test_gps_read(void);	
static void pc_uart_command_hardware_test_gps_disable(void);
#ifndef WATCH_HAS_NO_BLE 
static void pc_uart_command_hardware_test_ble_enable(void);	
static void pc_uart_command_hardware_test_ble_info_read(void);
static void pc_uart_command_hardware_test_ble_rssi_read(void);	
static void pc_uart_command_hardware_test_ble_rssi_read_stop(void);
static void pc_uart_command_hardware_test_ble_extflash_read(void);
static void pc_uart_command_hardware_test_ble_disable(void);
#endif
static void pc_uart_command_hardware_test_pressure_enable(void);
static void pc_uart_command_hardware_test_pressure_disable(void);

#ifdef WATCH_CHIP_TEST
static void pc_uart_command_hardware_test_sixaxis_id_read(void);
static void pc_uart_command_hardware_test_compass_id_read(void);
static void pc_uart_command_hardware_test_flash_id_read(void);
static void pc_uart_command_hardware_test_heart_id_read(void);
#endif


static void hwt_event_handle(uint32_t *p_data, uint32_t length)
{
	switch(p_data[0]&0x0000FF00)
	{
		case PC_UART_COMMAND_HARDWARE_TEST_MODE_ENABLE:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_MODE_ENABLE\n");	
			pc_uart_command_hardware_test_mode_enable();
		}break;
		case PC_UART_COMMAND_HARDWARE_TEST_MODE_DISABLE:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_MODE_DISABLE\n");	
			pc_uart_command_hardware_test_mode_disable();
		}break;		
		case PC_UART_COMMAND_HARDWARE_TEST_BUTTON_PWR_ENABLE:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_BUTTON_PWR_ENABLE\n");	
			pc_uart_command_hardware_test_button_pwr_enable();
		}break;						
		case PC_UART_COMMAND_HARDWARE_TEST_BUTTON_PWR_DISABLE:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_BUTTON_PWR_DISABLE\n");	
			pc_uart_command_hardware_test_button_pwr_disable();
		}break;			
		case PC_UART_COMMAND_HARDWARE_TEST_BUTTON_BL_ENABLE:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_BUTTON_BL_ENABLE\n");
			pc_uart_command_hardware_test_button_bl_enable();
		}break;					
		case PC_UART_COMMAND_HARDWARE_TEST_BUTTON_BL_DISABLE:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_BUTTON_BL_DISABLE\n");
			pc_uart_command_hardware_test_button_bl_disable();
		}break;	
		case PC_UART_COMMAND_HARDWARE_TEST_BUTTON_UP_ENABLE:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_BUTTON_UP_ENABLE\n");
			pc_uart_command_hardware_test_button_up_enable();
		}break;					
		case PC_UART_COMMAND_HARDWARE_TEST_BUTTON_UP_DISABLE:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_BUTTON_UP_DISABLE\n");
			pc_uart_command_hardware_test_button_up_disable();
		}break;			
		case PC_UART_COMMAND_HARDWARE_TEST_BUTTON_OK_ENABLE:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_BUTTON_OK_ENABLE\n");
			pc_uart_command_hardware_test_button_ok_enable();
		}break;					
		case PC_UART_COMMAND_HARDWARE_TEST_BUTTON_OK_DISABLE:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_BUTTON_OK_DISABLE\n");
			pc_uart_command_hardware_test_button_ok_disable();
		}break;				
		case PC_UART_COMMAND_HARDWARE_TEST_BUTTON_DWN_ENABLE:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_BUTTON_DWN_ENABLE\n");
			pc_uart_command_hardware_test_button_dwn_enable();
		}break;					
		case PC_UART_COMMAND_HARDWARE_TEST_BUTTON_DWN_DISABLE:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_BUTTON_DWN_DISABLE\n");
			pc_uart_command_hwardware_test_button_dwn_disable();
		}break;			
		case PC_UART_COMMAND_HARDWARE_TEST_LCD_ENABLE:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_LCD_ENABLE\n");
			pc_uart_command_hardware_test_lcd_enable();
		}break;		
		case PC_UART_COMMAND_HARDWARE_TEST_LCD_DISABLE:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_LCD_DISABLE\n");
			pc_uart_command_hwardware_test_lcd_disable();
		}break;			
		case PC_UART_COMMAND_HARDWARE_TEST_BACKLIGHT_ENABLE:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_BACKLIGHT_ENABLE\n");
			pc_uart_command_hardware_test_backlight_enable();
		}break;		
		case PC_UART_COMMAND_HARDWARE_TEST_BACKLIGHT_DISABLE:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_BACKLIGHT_DISABLE\n");
			pc_uart_command_hardware_test_backlight_disable();
		}break;			
		case PC_UART_COMMAND_HARDWARE_TEST_MOTOR_ENABLE:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_MOTOR_ENABLE\n");
			pc_uart_command_hardware_test_motor_enable();
		}break;		
		case PC_UART_COMMAND_HARDWARE_TEST_MOTOR_DISABLE:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_MOTOR_DISABLE\n");
			pc_uart_command_hardware_test_motor_disable();
		}break;	
		case PC_UART_COMMAND_HARDWARE_TEST_BUZZER_ENABLE:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_BUZZER_ENABLE\n");
			pc_uart_command_hardware_test_buzzer_enable();
		}break;		
		case PC_UART_COMMAND_HARDWARE_TEST_BUZZER_DISABLE:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_BUZZER_DISABLE\n");
			pc_uart_command_hardware_test_buzzer_disable();
		}break;				
		case PC_UART_COMMAND_HARDWARE_TEST_EXTFLASH_ENABLE:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_EXTFLASH_ENABLE\n");
			pc_uart_command_hardware_test_extflash_enable();
		}break;		
		case PC_UART_COMMAND_HARDWARE_TEST_EXTFLASH_SECTOR:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_EXTFLASH_SECTOR\n");
			extFlash_sector(p_data[1],p_data[2]);		
		}break;				
		case PC_UART_COMMAND_HARDWARE_TEST_EXTFLASH_DISABLE:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_EXTFLASH_DISABLE\n");
			pc_uart_command_hardware_test_extflash_disable();
		}break;				
		case PC_UART_COMMAND_HARDWARE_TEST_FONT_ENABLE:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_FONT_ENABLE\n");
			pc_uart_command_hardware_test_font_enable();
		}break;		
		case PC_UART_COMMAND_HARDWARE_TEST_FONT_READ:{	
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_FONT_READ\n");
			pc_uart_command_hardware_test_font_read();
		}break;					
		case PC_UART_COMMAND_HARDWARE_TEST_FONT_DISABLE:{	
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_FONT_DISABLE\n");
			pc_uart_command_hardware_test_font_disable();
		}break;		
		case PC_UART_COMMAND_HARDWARE_TEST_GSENSOR_ENABLE:{	
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_GSENSOR_ENABLE\n");
			pc_uart_command_hardware_test_gsensor_enable();
		}break;		
		case PC_UART_COMMAND_HARDWARE_TEST_GSENSOR_READ:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_GSENSOR_READ\n");
			pc_uart_command_hardware_test_gsensor_read();
		}break;			
		case PC_UART_COMMAND_HARDWARE_TEST_GSENSOR_DISABLE:{		
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_GSENSOR_DISABLE\n");
			pc_uart_command_hardware_test_gsensor_disable();
		}break;		
		case PC_UART_COMMAND_HARDWARE_TEST_GYRO_ENABLE:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_GYRO_ENABLE\n");
			pc_uart_command_hardware_test_gyro_enable();
		}break;
		case PC_UART_COMMAND_HARDWARE_TEST_GYRO_READ:{		
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_GYRO_READ\n");
			pc_uart_command_hardware_test_gyro_read();
		}break;
		case PC_UART_COMMAND_HARDWARE_TEST_GYRO_DISABLE:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_GYRO_DISABLE\n");
			pc_uart_command_hardware_test_gyro_disable();
		}break;		
		case PC_UART_COMMAND_HARDWARE_TEST_COMPASS_ENABLE:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_COMPASS_ENABLE\n");
			pc_uart_command_hardware_test_compass_enable();
		}break;
		case PC_UART_COMMAND_HARDWARE_TEST_COMPASS_READ:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_COMPASS_READ\n");
			pc_uart_command_hardware_test_compass_read();
		}break;
		case PC_UART_COMMAND_HARDWARE_TEST_COMPASS_DISABLE:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_COMPASS_DISABLE\n");
			pc_uart_command_hardware_test_compass_disable();
		}break;	
		case PC_UART_COMMAND_HARDWARE_TEST_GPS_ENABLE:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_GPS_ENABLE\n");
			pc_uart_command_hardware_test_gps_enable();
		}break;
		case PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_HOT_START:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_HOT_START\n");
			pc_uart_command_hardware_test_gps_config_hot_start();
		}break;
		case PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_WARM_START:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_WARM_START\n");
			pc_uart_command_hardware_test_gps_config_warm_start();
		}break;
		case PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_COLD_START:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_COLD_START\n");
			pc_uart_command_hardware_test_gps_config_cold_start();
		}break;
		case PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_GPS_MODE:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_GPS_MODE\n");
			pc_uart_command_hardware_test_gps_config_gps_mode();
		}break;
		case PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_BD_MODE:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_BD_MODE\n");
			pc_uart_command_hardware_test_gps_config_bd_mode();
		}break;
		case PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_GPS_BD_MODE:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_GPS_BD_MODE\n");
			pc_uart_command_hardware_test_gps_config_gps_bd_mode();
		}break;
		case PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_NMEA_GGA:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_NMEA_GGA\n");
			pc_uart_command_hardware_test_gps_config_nmea_gga();
		}break;
		case PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_NMEA_GSA:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_NMEA_GSA\n");
			pc_uart_command_hardware_test_gps_config_nmea_gsa();
		}break;
		case PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_NMEA_GSV:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_NMEA_GSV\n");	
			pc_uart_command_hardware_test_gps_config_nmea_gsv();
		}break;
		case PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_NMEA_RMC:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_NMEA_RMC\n");
			pc_uart_command_hardware_test_gps_config_nmea_rmc();
		}break;
		case PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_NMEA_VTG:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_NMEA_VTG\n");
			pc_uart_command_hardware_test_gps_config_nmea_vtg();
		}break;
		case PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_NMEA_GLL:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_NMEA_GLL\n");
			pc_uart_command_hardware_test_gps_config_nmea_gll();
		}break;
		case PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_NMEA_ZDA:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_NMEA_ZDA\n");
			pc_uart_command_hardware_test_gps_config_nmea_zda();
		}break;		
		case PC_UART_COMMAND_HARDWARE_TEST_GPS_READ:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_GPS_READ\n");
			pc_uart_command_hardware_test_gps_read();
		}break;		
		case PC_UART_COMMAND_HARDWARE_TEST_GPS_DISABLE:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_GPS_DISABLE\n");
			pc_uart_command_hardware_test_gps_disable();
		}break;	
        #if defined WATCH_IMU_CALIBRATION
		 case PC_UART_COMMAND_HARDWARE_TEST_ACC_CAL://加速度校准
	 	   {
			acc_calibration_entrance();
		   
		   }break;	
		  #endif
        #if defined WATCH_IMU_CALIBRATION 	
      case PC_UART_COMMAND_HARDWARE_TEST_ACC_CAL_PARAMETER_GET://获取加速度计和陀螺仪校准参数,确认校准
	  	  {
		    	 	
		     pass_back_parameter();
					
				
		  }break;	
         #endif

	    #if defined WATCH_IMU_CALIBRATION
      case PC_UART_COMMAND_HARDWARE_TEST_WATCH_RESET://手表复位
	 	{
				
		   taskENTER_CRITICAL();
	       am_hal_reset_por();
		  
		}
	      break;	
		  #endif
		  #if defined WATCH_IMU_CALIBRATION
		 case PC_UART_COMMAND_HARDWARE_TEST_RESET_IMU_PARAMETER://重置校准参数，把手表恢复到未校准的状态
	 	{
		  	
		   parameter_reset(0);
			
		}
	      break;	
       #endif
	   
       #if defined WATCH_IMU_CALIBRATION
       case PC_UART_COMMAND_HARDWARE_TEST_GYRO_CAL://陀螺仪校准
	 	{	
		 gyro_calibration_entrance();
		}
	      break;	
	   #endif

		
#ifndef WATCH_HAS_NO_BLE 	
		case PC_UART_COMMAND_HARDWARE_TEST_BLE_ENABLE:{	
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_BLE_ENABLE\n");
			pc_uart_command_hardware_test_ble_enable();
		}break;		
		case PC_UART_COMMAND_HARDWARE_TEST_BLE_INFO_READ:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_BLE_INFO_READ\n");
			pc_uart_command_hardware_test_ble_info_read();
		}break;		
		case PC_UART_COMMAND_HARDWARE_TEST_BLE_RSSI_READ:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_BLE_RSSI_READ\n");
			pc_uart_command_hardware_test_ble_rssi_read();
		}break;	
		case PC_UART_COMMAND_HARDWARE_TEST_BLE_RSSI_READ_STOP:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_BLE_RSSI_READ_STOP\n");
			pc_uart_command_hardware_test_ble_rssi_read_stop();
		}break;			
		case PC_UART_COMMAND_HARDWARE_TEST_BLE_EXTFLASH_READ:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_BLE_EXTFLASH_READ\n");
			pc_uart_command_hardware_test_ble_extflash_read();
		}break;			
		case PC_UART_COMMAND_HARDWARE_TEST_BLE_DISABLE:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_BLE_DISABLE\n");
			pc_uart_command_hardware_test_ble_disable();
		}break;	
#endif		
		case PC_UART_COMMAND_HARDWARE_TEST_PRESSURE_ENABLE:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_PRESSURE_ENABLE\n");
			pc_uart_command_hardware_test_pressure_enable();
		}break;		
		case PC_UART_COMMAND_HARDWARE_TEST_PRESSURE_READ:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_PRESSURE_READ\n");
			pressure_read();	
		}break;		
		case PC_UART_COMMAND_HARDWARE_TEST_PRESSURE_DISABLE:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_PRESSURE_DISABLE\n");
			pc_uart_command_hardware_test_pressure_disable();
		}break;	
#ifdef WATCH_CHIP_TEST
		case PC_UART_COMMAND_HARDWARE_TEST_SIXAXIS_ID_READ:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_PRESSURE_ENABLE\n");
			pc_uart_command_hardware_test_sixaxis_id_read();
		}break;	
		case PC_UART_COMMAND_HARDWARE_TEST_COMPASS_ID_READ:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_PRESSURE_ENABLE\n");
			pc_uart_command_hardware_test_compass_id_read();
		}break;
		case PC_UART_COMMAND_HARDWARE_TEST_FLASH_ID_READ:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_PRESSURE_ENABLE\n");
			pc_uart_command_hardware_test_flash_id_read();
		}break;	
		case PC_UART_COMMAND_HARDWARE_TEST_HEART_ID_READ:{
			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_PRESSURE_ENABLE\n");
			pc_uart_command_hardware_test_heart_id_read();
		}break;	
#endif	
//		case PC_UART_COMMAND_HARDWARE_TEST_IMU_CALIBRATION_START:{
//			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_IMU_CALIBRATION_START\n");
//			acc_calibration_entrance();
//		}break;
//		case PC_UART_COMMAND_HARDWARE_TEST_IMU_CALIBRATION_END:{
//			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_IMU_CALIBRATION_END\n");
//			
//		}break;
//		case PC_UART_COMMAND_HARDWARE_TEST_IMU_CALIBRATION_READ_PARAM:{
//			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_IMU_CALIBRATION_READ_PARAM\n");
//			
//		}break;
//		case PC_UART_COMMAND_HARDWARE_TEST_IMU_CALIBRATION_RESTART:{
//			TASK_PC_LOG_PRINTF("[TASK_PC]:PC_UART_COMMAND_HARDWARE_TEST_IMU_CALIBRATION_RESTART\n");
//			
//		}break;
			
		default:{
		}break;
	}		
		


}

static void pc_uart_command_hardware_test_mode_enable(void)			
{
	memset((uint8_t *)&g_gui_hwt,0,sizeof(g_gui_hwt));
#ifdef WATCH_CHIP_TEST
	g_gui_hwt.hardware_test_enable = true;
#endif
	DISPLAY_MSG  msg = {0,0};
	ScreenState = DISPLAY_SCREEN_PC_HWT;
	msg.cmd = MSG_DISPLAY_SCREEN;
	xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_MODE_ENABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	drv_pc_send_data((uint8_t *)command, sizeof(command));
}
static void pc_uart_command_hardware_test_mode_disable(void)			
{

	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_MODE_DISABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	drv_pc_send_data((uint8_t *)command, sizeof(command));	
#ifdef WATCH_CHIP_TEST	
	g_gui_hwt.hardware_test_enable = false;
#endif
	
	DISPLAY_MSG  msg = {0,0};
	ScreenState = DISPLAY_SCREEN_HOME;
	msg.cmd = MSG_DISPLAY_SCREEN;
	xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
	
//	DELAY_MS(1000);
//	mode_button_pon();
}
static void pc_uart_command_hardware_test_button_pwr_enable(void)			
{
	g_gui_hwt.button_pwr.enable = true;
	g_gui_hwt.button_pwr.press_cnt = 0;
	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_BUTTON_PWR_ENABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	drv_pc_send_data((uint8_t *)command, sizeof(command));
}
static void pc_uart_command_hardware_test_button_pwr_disable(void)			
{
	g_gui_hwt.button_pwr.enable = false;
	g_gui_hwt.button_pwr.press_cnt = 0;
	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_BUTTON_PWR_DISABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	drv_pc_send_data((uint8_t *)command, sizeof(command));
}
static void pc_uart_command_hardware_test_button_bl_enable(void)			
{
	g_gui_hwt.button_bl.enable = true;
	g_gui_hwt.button_bl.press_cnt = 0;
	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_BUTTON_BL_ENABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	drv_pc_send_data((uint8_t *)command, sizeof(command));
}
static void pc_uart_command_hardware_test_button_bl_disable(void)			
{
	g_gui_hwt.button_bl.enable = false;
	g_gui_hwt.button_bl.press_cnt = 0;
	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_BUTTON_BL_DISABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	drv_pc_send_data((uint8_t *)command, sizeof(command));	
}
static void pc_uart_command_hardware_test_button_up_enable(void)			
{
	g_gui_hwt.button_up.enable = true;
	g_gui_hwt.button_up.press_cnt = 0;
	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_BUTTON_UP_ENABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	drv_pc_send_data((uint8_t *)command, sizeof(command));
}
static void pc_uart_command_hardware_test_button_up_disable(void)			
{
	g_gui_hwt.button_up.enable = false;
	g_gui_hwt.button_up.press_cnt = 0;
	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_BUTTON_UP_DISABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	drv_pc_send_data((uint8_t *)command, sizeof(command));
}
static void pc_uart_command_hardware_test_button_ok_enable(void) 			
{
	g_gui_hwt.button_ok.enable = true;
	g_gui_hwt.button_ok.press_cnt = 0;
	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_BUTTON_OK_ENABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	drv_pc_send_data((uint8_t *)command, sizeof(command));	
}
static void pc_uart_command_hardware_test_button_ok_disable(void)			
{
	g_gui_hwt.button_ok.enable = false;
	g_gui_hwt.button_ok.press_cnt = 0;
	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_BUTTON_OK_DISABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	drv_pc_send_data((uint8_t *)command, sizeof(command));
}
static void pc_uart_command_hardware_test_button_dwn_enable(void)			
{
	g_gui_hwt.button_dwn.enable = true;
	g_gui_hwt.button_dwn.press_cnt = 0;
	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_BUTTON_DWN_ENABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	drv_pc_send_data((uint8_t *)command, sizeof(command));
}
static void pc_uart_command_hwardware_test_button_dwn_disable(void)			
{
	g_gui_hwt.button_dwn.enable = false;
	g_gui_hwt.button_dwn.press_cnt = 0;
	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_BUTTON_DWN_DISABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	drv_pc_send_data((uint8_t *)command, sizeof(command));
}
static void pc_uart_command_hardware_test_lcd_enable(void)			
{
	g_gui_hwt.lcd_enable = true;
	gui_hwt_paint();			
	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_LCD_ENABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	drv_pc_send_data((uint8_t *)command, sizeof(command));	
}
static void pc_uart_command_hwardware_test_lcd_disable(void)
{
	g_gui_hwt.lcd_enable = false;				
	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_LCD_DISABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	drv_pc_send_data((uint8_t *)command, sizeof(command));	
}	
static void pc_uart_command_hardware_test_backlight_enable(void)
{
	drv_light_enable();
	g_gui_hwt.backlight_enable = true;			
	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_BACKLIGHT_ENABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	drv_pc_send_data((uint8_t *)command, sizeof(command));	
}	
static void pc_uart_command_hardware_test_backlight_disable(void)			
{
	drv_light_disable();
	g_gui_hwt.backlight_enable = false;					
	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_BACKLIGHT_DISABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	drv_pc_send_data((uint8_t *)command, sizeof(command));
}
static void pc_uart_command_hardware_test_motor_enable(void)			
{
	drv_motor_enable();		
	g_gui_hwt.motor_enable = true;			
	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_MOTOR_ENABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	drv_pc_send_data((uint8_t *)command, sizeof(command));	
}
static void pc_uart_command_hardware_test_motor_disable(void)			
{
	drv_motor_disable();		
	g_gui_hwt.motor_enable = false;				
	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_MOTOR_DISABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	drv_pc_send_data((uint8_t *)command, sizeof(command));
}
static void pc_uart_command_hardware_test_buzzer_enable(void)			
{
	drv_buzzer_enable();			
	g_gui_hwt.buzzer_enable = true;	
	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_BUZZER_ENABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	drv_pc_send_data((uint8_t *)command, sizeof(command));
}
static void pc_uart_command_hardware_test_buzzer_disable(void)			
{
	drv_buzzer_disable();					
	g_gui_hwt.buzzer_enable = false;				
	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_BUZZER_DISABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	drv_pc_send_data((uint8_t *)command, sizeof(command));
}
static void pc_uart_command_hardware_test_extflash_enable(void)			
{
	g_gui_hwt.extFlash.enable = true;	
	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_EXTFLASH_ENABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	drv_pc_send_data((uint8_t *)command, sizeof(command));	
}
static void extFlash_sector(uint32_t address,uint32_t size)
{
	dev_extFlash_buffer_p = pvPortMalloc(W25Q128_PAGE_SIZE);
	memset(dev_extFlash_buffer_p,0x5A,W25Q128_PAGE_SIZE);
	uint32_t crc_src = am_bootloader_fast_crc32(dev_extFlash_buffer_p,W25Q128_PAGE_SIZE);	
	dev_extFlash_enable();			
	dev_extFlash_erase(address, size);
	dev_extFlash_write(address, dev_extFlash_buffer_p, size);
	dev_extFlash_read(address, dev_extFlash_buffer_p, size);			
	dev_extFlash_disable();	
	uint32_t crc_dest = am_bootloader_fast_crc32(dev_extFlash_buffer_p,W25Q128_PAGE_SIZE);
	vPortFree(dev_extFlash_buffer_p);	
	uint32_t command[2] = {PC_UART_COMMAND_HARDWARE_TEST_EXTFLASH_SECTOR_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	if(crc_src == crc_dest)
	{
		command[1] = ERR_SUCCESS;
	}else{
		command[1] = ERR_CRC;
	}	
	drv_pc_send_data((uint8_t *)command, sizeof(command));	
}
static void pressure_read(void)
{
	float pressure,altitude,temperature;
	drv_ms5837_data_get(&pressure, &altitude, &temperature);
	//字符串个格式
	_dev_hwt_pressure dev_hwt_pressure;
	dev_hwt_pressure.command = PC_UART_COMMAND_HARDWARE_TEST_PRESSURE_READ_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST;
	sprintf(dev_hwt_pressure.pressure,"%5.3f",pressure);
	sprintf(dev_hwt_pressure.altitude,"%08d",(int)(altitude));
	sprintf(dev_hwt_pressure.temperature,"%5.3f",temperature);	
	drv_pc_send_data((uint8_t *)&dev_hwt_pressure, sizeof(_dev_hwt_pressure));	
}
static void pc_uart_command_hardware_test_extflash_disable(void)			
{
	g_gui_hwt.extFlash.enable = false;				
	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_EXTFLASH_DISABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	drv_pc_send_data((uint8_t *)command, sizeof(command));
}
static void pc_uart_command_hardware_test_font_enable(void)			
{
	g_gui_hwt.font_enable = true;				
	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_FONT_ENABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	drv_pc_send_data((uint8_t *)command, sizeof(command));
}
static void pc_uart_command_hardware_test_font_read(void)			
{
	uint32_t command[2] = {PC_UART_COMMAND_HARDWARE_TEST_FONT_READ_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
    command[1] = drv_gt24l24_self_test();
	TASK_PC_LOG_PRINTF("[TASK_PC]:command[1] --> %d\n",command[1]);	
	drv_pc_send_data((uint8_t *)command, sizeof(command));	
}
static void pc_uart_command_hardware_test_font_disable(void)			
{
	g_gui_hwt.font_enable = false;					
	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_FONT_DISABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	drv_pc_send_data((uint8_t *)command, sizeof(command));
}
static void pc_uart_command_hardware_test_gsensor_enable(void)			
{
	g_gui_hwt.g_sensor.enable = true;	
	drv_lsm6dsl_acc_init();
	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_GSENSOR_ENABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	drv_pc_send_data((uint8_t *)command, sizeof(command));	
}
static void pc_uart_command_hardware_test_gsensor_read(void)			
{
	_dev_hwt_motion motion;
	motion.command = PC_UART_COMMAND_HARDWARE_TEST_GSENSOR_READ_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST;
	int16_t pData[3];
	if(drv_lsm6dsl_acc_axes_raw(pData) == COMPONENT_OK)
	{
		motion.x = pData[0];
		motion.y = pData[1];
		motion.z = pData[2];						
	}			
	drv_pc_send_data((uint8_t *)&motion, sizeof(motion));
}
static void pc_uart_command_hardware_test_gsensor_disable(void)
{
	g_gui_hwt.g_sensor.enable = false;	
	drv_lsm6dsl_acc_powerdown();
	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_GSENSOR_DISABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	drv_pc_send_data((uint8_t *)command, sizeof(command));
}	
static void pc_uart_command_hardware_test_gyro_enable(void)			
{
	g_gui_hwt.gyro.enable = true;		
	drv_lsm6dsl_gyro_init();
	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_GYRO_ENABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	drv_pc_send_data((uint8_t *)command, sizeof(command));
}
static void pc_uart_command_hardware_test_gyro_read(void)			
{
	_dev_hwt_motion motion;
	motion.command = PC_UART_COMMAND_HARDWARE_TEST_GYRO_READ_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST;
	int16_t pData[3];
	if(drv_lsm6dsl_gyro_axes_raw(pData) == COMPONENT_OK)
	{
		motion.x = pData[0];
		motion.y = pData[1];
		motion.z = pData[2];						
	}				
	drv_pc_send_data((uint8_t *)&motion, sizeof(motion));	
}
static void pc_uart_command_hardware_test_gyro_disable(void)			
{
	g_gui_hwt.gyro.enable = false;
	drv_lsm6dsl_gyro_powerdown();
	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_GYRO_DISABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	drv_pc_send_data((uint8_t *)command, sizeof(command));
}
static void pc_uart_command_hardware_test_compass_enable(void)			
{
	g_gui_hwt.compass.enable = true;
	drv_lis3mdl_reset();
	DELAY_MS(30);					
	//drv_lis3mdl_init();
	magn_dev_para_init();
	magn_angle_and_cal(0);
	DELAY_MS(30);		
	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_COMPASS_ENABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	drv_pc_send_data((uint8_t *)command, sizeof(command));	
}
static void pc_uart_command_hardware_test_compass_read(void)			
{
	_dev_hwt_motion motion;
	motion.command = PC_UART_COMMAND_HARDWARE_TEST_COMPASS_READ_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST;
	int16 psdata[3];
	drv_lis3mdl_enable();
	if(drv_lis3mdl_data(psdata))
	{
		motion.x = psdata[0];
		motion.y = psdata[1];
		motion.z = psdata[2];			
	}
	drv_lis3mdl_disable();	
	drv_pc_send_data((uint8_t *)&motion, sizeof(motion));	
}
static void pc_uart_command_hardware_test_compass_disable(void)			
{
	drv_lis3mdl_standby();
	
	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_COMPASS_DISABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	drv_pc_send_data((uint8_t *)command, sizeof(command));	
	
	
	
	
}
static void pc_uart_command_hardware_test_gps_enable(void)			
{
	g_gui_hwt.gps_enable = true;
	drv_ublox_poweron();
	DELAY_MS(100);
	drv_ublox_reset_rtc();
	DELAY_MS(100);
	drv_ublox_GGA(false);
//	DELAY_MS(1000);
	drv_ublox_GSV(false);
//	DELAY_MS(1000);
	drv_ublox_GLL(false);
//	DELAY_MS(1000);
	drv_ublox_GSA(false);
//	DELAY_MS(1000);
	drv_ublox_VTG(false);
//	DELAY_MS(1000);
	drv_ublox_RMC(true);
//	DELAY_MS(500);
	drv_ublox_GNSS(UBLOX_GN);		
	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_GPS_ENABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	drv_pc_send_data((uint8_t *)command, sizeof(command));	
}
static void pc_uart_command_hardware_test_gps_config_hot_start(void)			
{
	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_HOT_START_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	drv_pc_send_data((uint8_t *)command, sizeof(command));	
}
static void pc_uart_command_hardware_test_gps_config_warm_start(void)			
{
	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_WARM_START_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	drv_pc_send_data((uint8_t *)command, sizeof(command));
}
static void pc_uart_command_hardware_test_gps_config_cold_start(void)			
{
	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_COLD_START_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	drv_pc_send_data((uint8_t *)command, sizeof(command));
}
static void pc_uart_command_hardware_test_gps_config_gps_mode(void)			
{
//	drv_ublox_GNSS(UBLOX_GP);	
//	DELAY_MS(500);
	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_GPS_MODE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	drv_pc_send_data((uint8_t *)command, sizeof(command));	
}
static void pc_uart_command_hardware_test_gps_config_bd_mode(void)			
{
//	drv_ublox_GNSS(UBLOX_GB);	
//	DELAY_MS(500);				
	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_BD_MODE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	drv_pc_send_data((uint8_t *)command, sizeof(command));	
}
static void pc_uart_command_hardware_test_gps_config_gps_bd_mode(void)			
{
//	drv_ublox_GNSS(UBLOX_GN);			
//	DELAY_MS(500);			
	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_GPS_BD_MODE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	drv_pc_send_data((uint8_t *)command, sizeof(command));
}
static void pc_uart_command_hardware_test_gps_config_nmea_gga(void)			
{
	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_NMEA_GGA_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	drv_pc_send_data((uint8_t *)command, sizeof(command));
}
static void pc_uart_command_hardware_test_gps_config_nmea_gsa(void)			
{
	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_NMEA_GSA_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	drv_pc_send_data((uint8_t *)command, sizeof(command));
}
static void pc_uart_command_hardware_test_gps_config_nmea_gsv(void)			
{
//	drv_ublox_GSV(true);	
//	DELAY_MS(500);				
	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_NMEA_GSV_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	drv_pc_send_data((uint8_t *)command, sizeof(command));	
}
static void pc_uart_command_hardware_test_gps_config_nmea_rmc(void)			
{
//	drv_ublox_RMC(true);
//	DELAY_MS(500);				
	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_NMEA_RMC_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	drv_pc_send_data((uint8_t *)command, sizeof(command));
}
static void pc_uart_command_hardware_test_gps_config_nmea_vtg(void)			
{
	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_NMEA_VTG_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	drv_pc_send_data((uint8_t *)command, sizeof(command));
}
static void pc_uart_command_hardware_test_gps_config_nmea_gll(void)			
{
	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_NMEA_GLL_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	drv_pc_send_data((uint8_t *)command, sizeof(command));
}
static void pc_uart_command_hardware_test_gps_config_nmea_zda(void)			
{
	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_NMEA_ZDA_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	drv_pc_send_data((uint8_t *)command, sizeof(command));
}
static void pc_uart_command_hardware_test_gps_read(void)			
{
	_hwt_ublox_data ublox_data;
	ublox_data.command = PC_UART_COMMAND_HARDWARE_TEST_GPS_READ_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST;
	drv_ublox_enable();	
	for(uint32_t i=0;i<sizeof(ublox_data.data);i++)
	{
		ublox_data.data[i] = drv_ublox_read();
		TASK_PC_LOG_PRINTF("%c",ublox_data.data[i]);
	}
	TASK_PC_LOG_PRINTF("\n");
	drv_ublox_disable();
	drv_pc_send_data((uint8_t *)&ublox_data, sizeof(ublox_data));	
}
static void pc_uart_command_hardware_test_gps_disable(void)			
{
	g_gui_hwt.gps_enable = false;
	drv_ublox_poweroff();
	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_GPS_DISABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	drv_pc_send_data((uint8_t *)command, sizeof(command));
}
#if !defined WATCH_HAS_NO_BLE 
static void pc_uart_command_hardware_test_ble_enable(void)
{

	if(SetValue.SwBle != false)
	{
		dev_ble_system_on();
	}	
	
	g_gui_hwt.ble.enable = true;
	g_gui_hwt.ble.read_info = false;
	g_gui_hwt.ble.read_extFlash = false;			
	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_BLE_ENABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	drv_pc_send_data((uint8_t *)command, sizeof(command));
}	
static void pc_uart_command_hardware_test_ble_info_read(void)			
{
	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_BLE_INFO_READ | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	_uart_data_param param;
	param.user_callback = NULL;
	param.p_data = (uint8_t *)command;
	param.length = sizeof(command);
	uint8_t count = TASK_UART_DELAY_COUNT_OUT;
	while(drv_ble_send_data(&param) != ERR_SUCCESS && count--)
	{
		DELAY_MS(TASK_UART_DELAY);
	}	
}
static void pc_uart_command_hardware_test_ble_rssi_read(void)			
{
	uint32_t command[5] = {PC_UART_COMMAND_HARDWARE_TEST_BLE_RSSI_READ | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	command[1] = 0x02A0;//SCAN_INTERVAL
	command[2] = 0x0010;//SCAN_WINDOW
	command[3] = 0x0000;//SCAN_TIMEOUT
	command[4] = 0xFEFE;//UUID
	_uart_data_param param;
	param.user_callback = NULL;
	param.p_data = (uint8_t *)command;
	param.length = sizeof(command);
	uint8_t count = TASK_UART_DELAY_COUNT_OUT;
	while(drv_ble_send_data(&param) != ERR_SUCCESS && count--)
	{
		DELAY_MS(TASK_UART_DELAY);
	}	
}
static void pc_uart_command_hardware_test_ble_rssi_read_stop(void)			
{
	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_BLE_RSSI_READ_STOP | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	_uart_data_param param;
	param.user_callback = NULL;
	param.p_data = (uint8_t *)command;
	param.length = sizeof(command);
	uint8_t count = TASK_UART_DELAY_COUNT_OUT;
	while(drv_ble_send_data(&param) != ERR_SUCCESS && count--)
	{
		DELAY_MS(TASK_UART_DELAY);
	}	
}
static void pc_uart_command_hardware_test_ble_extflash_read(void)			
{
	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_BLE_EXTFLASH_READ | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	_uart_data_param param;
	param.user_callback = NULL;
	param.p_data = (uint8_t *)command;
	param.length = sizeof(command);
	uint8_t count = TASK_UART_DELAY_COUNT_OUT;
	while(drv_ble_send_data(&param) != ERR_SUCCESS && count--)
	{
		DELAY_MS(TASK_UART_DELAY);
	}	
}
static void pc_uart_command_hardware_test_ble_disable(void)			
{
	if(SetValue.SwBle != false)
	{
		dev_ble_system_off();
	}
	
	g_gui_hwt.ble.enable = false;
	g_gui_hwt.ble.read_info = false;
	g_gui_hwt.ble.read_extFlash = false;
	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_BLE_DISABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	drv_pc_send_data((uint8_t *)command, sizeof(command));	
}
#endif
static void pc_uart_command_hardware_test_pressure_enable(void)			
{
	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_PRESSURE_ENABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	drv_pc_send_data((uint8_t *)command, sizeof(command));
}
static void pc_uart_command_hardware_test_pressure_disable(void)		
{
	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_PRESSURE_DISABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	drv_pc_send_data((uint8_t *)command, sizeof(command));
}

#ifdef WATCH_CHIP_TEST
static void pc_uart_command_hardware_test_sixaxis_id_read(void)			
{
	uint32_t command[2] = {PC_UART_COMMAND_HARDWARE_TEST_SIXAXIS_ID_READ_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	command[1] = drv_lsm6dsl_id(); ;
	drv_pc_send_data((uint8_t *)command, sizeof(command));
}

static void pc_uart_command_hardware_test_compass_id_read(void)			
{
	uint32_t command[2] = {PC_UART_COMMAND_HARDWARE_TEST_COMPASS_ID_READ_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	command[1] =drv_lis3mdl_id();
	drv_pc_send_data((uint8_t *)command, sizeof(command));
}

static void pc_uart_command_hardware_test_flash_id_read(void)			
{
	uint32_t command[2] = {PC_UART_COMMAND_HARDWARE_TEST_FLASH_ID_READ_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	command[1] =dev_extFlash_IDverify(1);
	drv_pc_send_data((uint8_t *)command, sizeof(command));
}

static void pc_uart_command_hardware_test_heart_id_read(void)			
{
	uint8_t herat_id;
	uint32_t command[2] = {PC_UART_COMMAND_HARDWARE_TEST_HEART_ID_READ_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};

	drv_hrt_3v3_enable();
	am_util_delay_ms(50);	
	drv_i2c_hw1_enable();
	herat_id= Si117xReadFromRegister(REG_PART_ID);
	drv_i2c_hw1_disable();		
	drv_hrt_3v3_disable();

	if (herat_id==0x81)
	{
		command[1] = 1;
	}
	else
	{
		command[1] = 0;
	}
	drv_pc_send_data((uint8_t *)command, sizeof(command));
}

#endif




