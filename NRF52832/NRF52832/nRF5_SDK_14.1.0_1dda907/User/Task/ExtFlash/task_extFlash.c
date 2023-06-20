#include "task_extFlash.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "portmacro.h"
#include "portable.h"
#include "event_groups.h"
#include "semphr.h"

#include "dev_extFlash.h"
#include "drv_extFlash.h"
#include "nrf_gpio.h"
#include "am_bootloader.h"

#include "drv_apollo2.h"
#include "cmd_apollo2.h"
#include "cmd_apollo_srv.h"
#include "app_srv_apollo.h"
#ifdef COD 
#include "cod_ble_api.h"
#include "app_srv_cod.h"
#endif
#include "lib_quicklz.h"
#include <time.h>
#include <math.h>
#include "com_ringbuffer.h"

#define LOG_LEVEL_OFF 							0
#define LOG_LEVEL_ERROR 						1
#define LOG_LEVEL_WARNING 						2
#define LOG_LEVEL_INFO 							3
#define LOG_LEVEL_DEBUG 						4
#define NRF_LOG_MODULE_NAME 					task_extFlash
#define NRF_LOG_LEVEL       					LOG_LEVEL_INFO
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
NRF_LOG_MODULE_REGISTER();






//#define DELAY_MS(X)                 nrf_delay_ms(X)
#define DELAY_MS(X)                 vTaskDelay(X)
//#define DELAY_MS(X)                   do{TickType_t xLastWakeTime;xLastWakeTime = xTaskGetTickCount ();vTaskDelayUntil( &xLastWakeTime, X );}while(0)

static TaskHandle_t task_handle;
#ifdef COD 
extern cod_mtu_size cod_usr_ble_tx_mtu;
extern _all_day_steps all_day_steps[7];
extern uint16_t all_day_sync_data[5];
#endif
typedef struct{
	uint8_t	evt;
	uint32_t address;
	uint8_t *p_data;
	uint32_t length;
} _task_buffer;
static _task_buffer m_task_buffer = {0,NULL,0};


static void task_extflash_evt_hwt(void);
static void task_extflash_evt_read_raw(void);
static void task_extflash_evt_write_raw(void);
static void task_extflash_evt_erase_raw(void);
static void task_extflash_evt_ble_dfu_ble_postvalidate(void);
static void task_extflash_evt_ble_dfu_apollo_postvalidate(void);
static void task_extflash_evt_ble_dfu_apollo_prevalidate(void);
static void task_extflash_evt_ble_dfu_ble_prevalidate(void);

#ifdef COD
#define     R_EARTH 6371004      //
#define     PI 3.1415926
int32_t last_lon = 0;
int32_t last_lat  =0;
ActivityDataStr ActivityData;
static void task_extflash_evt_ota_erase_raw(void);
static void task_extflash_evt_ota_write_raw(void);
static void test_extFlash_evt_read_raw(void);
static void task_extFlash_evt_sport_data_frame_num(void);
static void task_extFlash_evt_get_sport_data(void);
static void task_extFlash_evt_all_day_step_data(uint16_t start_frame);
#define sport_data_buffer_len 512
static uint8_t sport_data_buffer[sport_data_buffer_len]; ////运动数据缓存
struct ring_buffer_header sport_data_rb; //运动数据环型缓存
SemaphoreHandle_t SportSemaphore = NULL;
uint8_t frame_id_flag = 0;//0 普通，1 10倍帧，2 最后帧
#endif




void task_extFlash_handle(uint32_t evt, uint32_t addr, uint8_t *p_data, uint32_t length)
{
	NRF_LOG_DEBUG("task_extFlash_handle");

	m_task_buffer.evt = evt;
	m_task_buffer.address = addr;
	m_task_buffer.p_data = p_data;
	m_task_buffer.length = length;

	BaseType_t xReturned = xTaskNotifyGive(task_handle);
	if (xReturned != pdPASS)
	{
			APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
	}	
}


static void task(void * pvParameter)
{
	NRF_LOG_DEBUG("task");
	(void)pvParameter;
//	test_extFlash_evt_read_raw();
		while (1)
		{
			
			(void) ulTaskNotifyTake(pdTRUE,/* Clear the notification value before exiting (equivalent to the binary semaphore). */
                                portMAX_DELAY); /* Block indefinitely (INCLUDE_vTaskSuspend has to be enabled).*/

			switch(m_task_buffer.evt)			
			{
				case TASK_EXTFLASH_EVT_HWT:{
					NRF_LOG_DEBUG("TASK_EXTFLASH_EVT_HWT");	
					task_extflash_evt_hwt();
				}break;									
				case TASK_EXTFLASH_EVT_READ_RAW:{
					NRF_LOG_DEBUG("TASK_EXTFLASH_EVT_READ_RAW");	
					task_extflash_evt_read_raw();
				}break;					
				case TASK_EXTFLASH_EVT_WRITE_RAW:{
					NRF_LOG_DEBUG("TASK_EXTFLASH_EVT_WRITE_RAW");	
					task_extflash_evt_write_raw();
				}break;					
				case TASK_EXTFLASH_EVT_ERASE_RAW:{
					NRF_LOG_DEBUG("TASK_EXTFLASH_EVT_ERASE_RAW");	
					task_extflash_evt_erase_raw();
				}break;			
				case TASK_EXTFLASH_EVT_BLE_DFU_BLE_POSTVALIDATE:{
					NRF_LOG_DEBUG("TASK_EXTFLASH_EVT_BLE_DFU_BLE_POSTVALIDATE");	
					task_extflash_evt_ble_dfu_ble_postvalidate();
				}break;						
				case TASK_EXTFLASH_EVT_BLE_DFU_APOLLO_POSTVALIDATE:{
					NRF_LOG_DEBUG("TASK_EXTFLASH_EVT_BLE_DFU_APOLLO_POSTVALIDATE");	
					task_extflash_evt_ble_dfu_apollo_postvalidate();
				}break;					
				case TASK_EXTFLASH_EVT_BLE_DFU_APOLLO_PREVALIDATE:{
					NRF_LOG_DEBUG("TASK_EXTFLASH_EVT_BLE_DFU_APOLLO_PREVALIDATE");	
					task_extflash_evt_ble_dfu_apollo_prevalidate();
				}break;					
				case TASK_EXTFLASH_EVT_BLE_DFU_BLE_PREVALIDATE:{
					NRF_LOG_DEBUG("TASK_EXTFLASH_EVT_BLE_DFU_BLE_PREVALIDATE");	
					task_extflash_evt_ble_dfu_ble_prevalidate();
				}break;	
				#ifdef COD
				case TASK_EXTFLASH_EVT_OTA_ERASE_RAW:{
					NRF_LOG_DEBUG("TASK_EXTFLASH_EVT_OTA_ERASE_RAW");
					task_extflash_evt_ota_erase_raw();
				}break;
				case TASK_EXTFLASH_EVT_OTA_WRITE_RAW:{
					NRF_LOG_DEBUG("TASK_EXTFLASH_EVT_OTA_WRITE_RAW");
					task_extflash_evt_ota_write_raw();
				}break;
				case TASK_EXTFLASH_EVT_COD_CMD_ON_GET_DATA_FRAME_NUM:{
						uint8_t msg_buf[1];
						memcpy((uint8_t *)msg_buf,(uint8_t *)m_task_buffer.p_data,1);
						cod_ble_data_type type = (cod_ble_data_type) msg_buf[0];
						NRF_LOG_DEBUG("TASK_EXTFLASH_EVT_COD_CMD_ON_GET_DATA_FRAME_NUM %d\r\n",type);
						if( type == DATA_TYPE_SPORT)
						{
							task_extFlash_evt_sport_data_frame_num();
						}
				}break;
				case TASK_EXTFLASH_EVT_COD_CMD_ON_GET_DATA:{
						uint8_t msg_buf[3];
						memcpy((uint8_t *)msg_buf,(uint8_t *)m_task_buffer.p_data,3);
						cod_ble_data_type type = (cod_ble_data_type) msg_buf[0];
						uint16_t start_frame =  msg_buf[1] | msg_buf[2]<< 8;
						NRF_LOG_INFO("TASK_EXTFLASH_EVT_COD_CMD_ON_GET_DATA %d,%d\r\n",type,start_frame); 
						if( type==DATA_TYPE_STEP)
						{
							task_extFlash_evt_all_day_step_data(start_frame);
						}
						else if (type==DATA_TYPE_SPORT )
						{
						#if  1
							if (start_frame == 0)
							{
								bool ret;
								ret = ring_buffer_init(&sport_data_rb, sport_data_buffer, sport_data_buffer_len);
								NRF_LOG_DEBUG("ring_buffer_init %d\r\n",ret);
								if (SportSemaphore == NULL)
								{
									SportSemaphore = xSemaphoreCreateBinary();  //创建二值信号量
								}
	
								task_extFlash_evt_get_sport_data();
							}
							//else
							//{
								//xSemaphoreGive(SportSemaphore);
							//}
						#endif
						}
				}break;
				#endif
				default:{
					
				}break;
			}			
    }
}

void task_extFlash_init(void * p_context)
{
  NRF_LOG_DEBUG("task_extFlash_init");

	BaseType_t xReturned = xTaskCreate(task,
																		 "EXTF",
																		 512+64,
																		 p_context,
																		 APP_IRQ_PRIORITY_LOW,
																		 &task_handle);

	if (xReturned != pdPASS)
	{
			NRF_LOG_ERROR("task not created.");
			APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
	}
}


static void task_extflash_evt_hwt(void)
{
	uint32_t crc_src = 0, crc_dest = 0;
	crc_src = am_bootloader_fast_crc32(m_task_buffer.p_data,m_task_buffer.length);					
	dev_extFlash_enable();
	dev_extFlash_erase(m_task_buffer.address, m_task_buffer.length);		
	dev_extFlash_write(m_task_buffer.address, m_task_buffer.p_data, m_task_buffer.length);	
	dev_extFlash_read(m_task_buffer.address, m_task_buffer.p_data, m_task_buffer.length);			
	dev_extFlash_disable();	
	crc_dest = am_bootloader_fast_crc32(m_task_buffer.p_data,m_task_buffer.length);
	NRF_LOG_DEBUG("crc_src --> 0x%08X",crc_src);	
	NRF_LOG_DEBUG("crc_dest --> 0x%08X",crc_dest);	
	uint32_t response[2] = {PC_UART_COMMAND_HARDWARE_TEST_BLE_EXTFLASH_READ_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};						
	if(crc_src == crc_dest)
	{
		NRF_LOG_DEBUG("TASK_EXTFLASH_EVT_HWT success");
		response[1] = NRF_SUCCESS;
	}
	else{
		NRF_LOG_DEBUG("TASK_EXTFLASH_EVT_HWT fail");
		response[1] = 1;
	}
	_uart_data_param param;
	param.user_callback = NULL;
	param.p_data = (uint8_t *)response;
	param.length = sizeof(response);	
	uint8_t count = TASK_UART_DELAY_COUNT_OUT;
	while(drv_ble_send_data(&param) != NRF_SUCCESS && count--)
	{
		DELAY_MS(TASK_UART_DELAY);
	}	
}
static void task_extflash_evt_read_raw(void)
{
	_apollo_srv_flash_read_raw *cmd = (_apollo_srv_flash_read_raw *)(m_task_buffer.p_data);	
	NRF_LOG_DEBUG("cmd->crc --> 0x%08X",cmd->crc);
	NRF_LOG_DEBUG("cmd->read_back --> 0x%08X",cmd->read_back);	
	NRF_LOG_DEBUG("cmd->address --> 0x%08X",cmd->address);
	NRF_LOG_DEBUG("cmd->size --> 0x%08X",cmd->size);	

	
	
	_apollo_srv_flash_read_raw response;
	uint32_t crc_src = 0;			
	dev_extFlash_enable();	
	dev_extFlash_read(cmd->address, response.data, cmd->size);			
	dev_extFlash_disable();	
	crc_src = am_bootloader_fast_crc32(response.data,cmd->size);
	NRF_LOG_DEBUG("crc_src --> 0x%08X",crc_src);		
	
	response.cmd = CMD_APOLLO_SRV_FLASH_READ_RAW_RESPONSE;
	response.crc = crc_src;
	response.read_back = cmd->read_back;
	response.address = cmd->address;
	response.size = cmd->size;

	NRF_LOG_HEXDUMP_DEBUG(response.data+cmd->size-64, 64);
	
	
	
	app_srv_apollo_flash_send((uint8_t *)&response, 
	sizeof(response.cmd)+sizeof(response.crc)+sizeof(response.read_back)+sizeof(response.address)+sizeof(response.size)+response.size);
	
	
	
}

static void task_extflash_evt_write_raw(void)
{
	_apollo_srv_flash_write_raw *cmd = (_apollo_srv_flash_write_raw *)(m_task_buffer.p_data);	
	NRF_LOG_DEBUG("cmd->read_back --> 0x%08X",cmd->read_back);	
	NRF_LOG_DEBUG("cmd->crc --> 0x%08X",cmd->crc);
	NRF_LOG_DEBUG("cmd->address --> 0x%08X",cmd->address);
	NRF_LOG_DEBUG("cmd->size --> 0x%08X",cmd->size);	
	

	
	uint8_t response[6];
	response[0] = CMD_APOLLO_SRV_FLASH_WRITE_RAW_RESPONSE;
	response[1] = (uint8_t)(cmd->read_back>>0);
	response[2] = (uint8_t)(cmd->read_back>>8);
	response[3] = (uint8_t)(cmd->read_back>>16);
	response[4] = (uint8_t)(cmd->read_back>>24);
	uint32_t crc_src = am_bootloader_fast_crc32(m_task_buffer.p_data+17,cmd->size);
	NRF_LOG_DEBUG("crc_src --> 0x%08X",crc_src);
	if(cmd->crc != crc_src)
	{
		response[5] = 0x01;
	}
	else
	{
		uint8_t buffer[256];		
		dev_extFlash_enable();		
//		if(cmd->address == 0x00629000)
//		{		
//			dev_extFlash_read(cmd->address, buffer, 64);	
//			NRF_LOG_HEXDUMP_DEBUG(buffer,64);				
//		}		
		dev_extFlash_write(cmd->address, m_task_buffer.p_data+17, cmd->size);	
		dev_extFlash_read(cmd->address, buffer, cmd->size);			
		dev_extFlash_disable();	
		crc_src = am_bootloader_fast_crc32(buffer,cmd->size);
		NRF_LOG_DEBUG("crc_flash --> 0x%08X",crc_src);
		if(crc_src != cmd->crc)
		{
			response[5] = 0x02;
		}
		else{
			response[5] = 0x00;
		}
	}
	app_srv_apollo_flash_send((uint8_t *)&response, sizeof(response));	
}
static void task_extflash_evt_erase_raw(void)
{
	_apollo_srv_flash_erase_raw *cmd = (_apollo_srv_flash_erase_raw *)(m_task_buffer.p_data);	 
	NRF_LOG_DEBUG("cmd->read_back --> 0x%08X",cmd->read_back);
	NRF_LOG_DEBUG("cmd->address --> 0x%08X",cmd->address);
	NRF_LOG_DEBUG("cmd->size --> 0x%08X",cmd->size);	
	
	dev_extFlash_enable();		
	dev_extFlash_erase(cmd->address,cmd->size);
//	if(cmd->address == 0x00629000)
//	{
//		uint8_t buffer[64];			
//		dev_extFlash_read(cmd->address, buffer, 64);	
//		NRF_LOG_HEXDUMP_DEBUG(buffer,64);				
//	}
	dev_extFlash_disable();		
	
	
	
	uint8_t response[6];
	response[0] = CMD_APOLLO_SRV_FLASH_ERASE_RAW_RESPONSE;
	response[1] = (uint8_t)(cmd->read_back>>0);
	response[2] = (uint8_t)(cmd->read_back>>8);
	response[3] = (uint8_t)(cmd->read_back>>16);
	response[4] = (uint8_t)(cmd->read_back>>24);
	response[5] = 0x00;
	app_srv_apollo_flash_send((uint8_t *)&response, sizeof(response));	
	
}
#define POSTVALIDATE_BANK_UPDATE_ALGORITHM_CRC_ERROR                (1UL << 0)
#define POSTVALIDATE_BOOTSETTING_CRC_ERROR                          (1UL << 1)
#define POSTVALIDATE_BANK_UPDATE_ALGORITHM_CRC_ERROR_EXTFLASH       (1UL << 2)
#define POSTVALIDATE_BANK_BOOTLOADER_CRC_ERROR_EXTFLASH             (1UL << 3)
#define POSTVALIDATE_BANK_APPLICATION_CRC_ERROR_EXTFLASH            (1UL << 4)
#define POSTVALIDATE_BANK_SOFTDEVICE_CRC_ERROR_EXTFLASH             (1UL << 5)
#define POSTVALIDATE_DFU_BIN_FILE_CRC_ERROR                         (1UL << 6)

extern _ble_boot_setting g_ble_boot_setting;

static void task_extflash_evt_ble_dfu_ble_postvalidate(void)
{
	_apollo_srv_flash_ble_dfu_postvalidate *cmd = (_apollo_srv_flash_ble_dfu_postvalidate *)(m_task_buffer.p_data);	 
	NRF_LOG_INFO("cmd->address --> 0x%08X",cmd->address);
	NRF_LOG_INFO("cmd->size --> 0x%08X",cmd->size);		
	NRF_LOG_INFO("cmd->crc --> 0x%08X",cmd->crc);
	
	uint8_t response[5];
	response[0] = CMD_APOLLO_SRV_FLASH_DFU_BLE_POSTVALIDATE_RESPONSE;
	uint32_t error = 0;
	if(am_bootloader_check_crc_from_external_flash(cmd->crc,cmd->address,cmd->size))
	{
		NRF_LOG_INFO("image crc check is correct");
		
//		uint32_t bank_addr = 0;
//		uint32_t bank_size = 0;
//		uint32_t bank_crc = 0;	
		
//		if(g_ble_boot_setting.bank_bootloader.command == BOOT_SETTING_BANK_COMMAND_NEW)
//		{
//			NRF_LOG_INFO("bootloader image is new");
//			bank_addr = g_ble_boot_setting.bank_bootloader.extFlash_address;
//			bank_size = g_ble_boot_setting.bank_bootloader.image_size;
//			bank_crc = g_ble_boot_setting.bank_bootloader.image_crc;
//			if(am_bootloader_check_crc_from_external_flash(bank_crc,bank_addr,bank_size))
//			{
//				NRF_LOG_INFO("bootloader image crc is correct");
//				NRF_LOG_INFO("g_ble_boot_setting.bank_bootloader.image_crc --> 0x%08X",g_ble_boot_setting.bank_bootloader.image_crc);
//			}
//			else
//			{
//				NRF_LOG_INFO("bootloader image crc is wrong");
//				error |= POSTVALIDATE_BANK_BOOTLOADER_CRC_ERROR_EXTFLASH;
//			}
//		}
//		if(g_ble_boot_setting.bank_application.command == BOOT_SETTING_BANK_COMMAND_NEW)
//		{
//			NRF_LOG_INFO("application image is new");
//			bank_addr = g_ble_boot_setting.bank_application.extFlash_address;
//			bank_size = g_ble_boot_setting.bank_application.image_size;
//			bank_crc = g_ble_boot_setting.bank_application.image_crc;
//			if(am_bootloader_check_crc_from_external_flash(bank_crc,bank_addr,bank_size))
//			{
//				NRF_LOG_INFO("application image crc is correct");
//				NRF_LOG_INFO("g_ble_boot_setting.bank_application.image_crc --> 0x%08X",g_ble_boot_setting.bank_application.image_crc);
//			}
//			else
//			{
//				NRF_LOG_INFO("application image crc is wrong");
//				error |= POSTVALIDATE_BANK_APPLICATION_CRC_ERROR_EXTFLASH;
//			}			
//		}
//		if(g_ble_boot_setting.bank_softdevice.command == BOOT_SETTING_BANK_COMMAND_NEW)
//		{
//			NRF_LOG_INFO("softdevice image is new");
//			bank_addr = g_ble_boot_setting.bank_softdevice.extFlash_address;
//			bank_size = g_ble_boot_setting.bank_softdevice.image_size;
//			bank_crc = g_ble_boot_setting.bank_softdevice.image_crc;
//			if(am_bootloader_check_crc_from_external_flash(bank_crc,bank_addr,bank_size))
//			{
//				NRF_LOG_INFO("softdevice image crc is correct");
//				NRF_LOG_INFO("g_ble_boot_setting.bank_softdevice.image_crc --> 0x%08X",g_ble_boot_setting.bank_softdevice.image_crc);
//			}
//			else
//			{
//				NRF_LOG_INFO("softdevice image crc is wrong");
//				error |= POSTVALIDATE_BANK_SOFTDEVICE_CRC_ERROR_EXTFLASH;
//			}					
//		}
		
	}
	else
	{
		NRF_LOG_INFO("image crc check is wrong");
		//外部FLASH存的IMAGE有问题
		error |= POSTVALIDATE_DFU_BIN_FILE_CRC_ERROR;	
	}
	
	
	if(error == 0)
	{
		uint32_t bootsetting_address = 0x800000-0x1000;
		uint32_t bootsetting_size = sizeof(_ble_boot_setting);
		_ble_boot_setting buffer = {0};
		
		dev_extFlash_enable();		
		dev_extFlash_erase(bootsetting_address,bootsetting_size);	
		dev_extFlash_write(bootsetting_address, (uint8_t *)&g_ble_boot_setting, bootsetting_size);	
		dev_extFlash_read(bootsetting_address, (uint8_t *)&buffer, bootsetting_size);			
		dev_extFlash_disable();			
		NRF_LOG_INFO("boot_setting_crc --> 0x%08x",g_ble_boot_setting.boot_setting_crc);
		uint32_t boot_setting_crc_dest = am_bootloader_fast_crc32((uint8_t*)(&buffer.boot_setting_crc)+sizeof(buffer.boot_setting_crc),sizeof(_ble_boot_setting) - sizeof(buffer.boot_setting_crc));
		NRF_LOG_INFO("boot_setting_crc_dest --> 0x%08x",boot_setting_crc_dest);			
		uint32_t crc_src = am_bootloader_fast_crc32((uint8_t*)&g_ble_boot_setting,sizeof(_ble_boot_setting));
		NRF_LOG_INFO("crc_src --> 0x%08x",crc_src);	
		uint32_t crc_dest = am_bootloader_fast_crc32((uint8_t*)&buffer,sizeof(_ble_boot_setting));
		NRF_LOG_INFO("crc_dest --> 0x%08x",crc_dest);	
		if(crc_src == crc_dest)
		{
			NRF_LOG_INFO("bootsetting flash crc check is correct");
		}
		else
		{
			NRF_LOG_INFO("bootsetting flash crc check is wrong");
			error |= POSTVALIDATE_BOOTSETTING_CRC_ERROR;	
		}
	}
	
	response[1] = (uint8_t)(error>>0);
	response[2] = (uint8_t)(error>>8);
	response[3] = (uint8_t)(error>>16);
	response[4] = (uint8_t)(error>>24);	
	app_srv_apollo_flash_send((uint8_t *)&response, sizeof(response));
}
extern _apollo_boot_setting g_apollo_boot_setting;
static void task_extflash_evt_ble_dfu_apollo_postvalidate(void)
{
	_apollo_srv_flash_ble_dfu_postvalidate *cmd = (_apollo_srv_flash_ble_dfu_postvalidate *)(m_task_buffer.p_data);	 
	NRF_LOG_INFO("cmd->address --> 0x%08X",cmd->address);
	NRF_LOG_INFO("cmd->size --> 0x%08X",cmd->size);		
	NRF_LOG_INFO("cmd->crc --> 0x%08X",cmd->crc);
	
	uint8_t response[5];
	response[0] = CMD_APOLLO_SRV_FLASH_DFU_BLE_DFU_APOLLO_POSTVALIDATE_RESPONSE;
	uint32_t error = 0;	
	

	if(am_bootloader_check_crc_from_external_flash(cmd->crc,cmd->address,cmd->size))
	{
		NRF_LOG_INFO("image crc check is correct");
//		uint32_t bank_addr = 0;
//		uint32_t bank_size = 0;
//		uint32_t bank_crc = 0;	
		
//		if(g_apollo_boot_setting.bank_bootloader.command == BOOT_SETTING_BANK_COMMAND_NEW)
//		{
//			NRF_LOG_INFO("bootloader image is new");
//			bank_addr = g_apollo_boot_setting.bank_bootloader.extFlash_address;
//			bank_size = g_apollo_boot_setting.bank_bootloader.image_size;
//			bank_crc = g_apollo_boot_setting.bank_bootloader.image_crc;
//			if(am_bootloader_check_crc_from_external_flash(bank_crc,bank_addr,bank_size))
//			{
//				NRF_LOG_INFO("bootloader image crc is correct");
//			}
//			else
//			{
//				NRF_LOG_INFO("bootloader image crc is wrong");
//				error |= POSTVALIDATE_BANK_BOOTLOADER_CRC_ERROR_EXTFLASH;
//			}
//		}
//		if(g_apollo_boot_setting.bank_application.command == BOOT_SETTING_BANK_COMMAND_NEW)
//		{
//			NRF_LOG_INFO("application image is new");
//			bank_addr = g_apollo_boot_setting.bank_application.extFlash_address;
//			bank_size = g_apollo_boot_setting.bank_application.image_size;
//			bank_crc = g_apollo_boot_setting.bank_application.image_crc;
//			if(am_bootloader_check_crc_from_external_flash(bank_crc,bank_addr,bank_size))
//			{
//				NRF_LOG_INFO("application image crc is correct");
//			}
//			else
//			{
//				NRF_LOG_INFO("application image crc is wrong");
//				error |= POSTVALIDATE_BANK_APPLICATION_CRC_ERROR_EXTFLASH;
//			}			
//		}
//		if(g_apollo_boot_setting.bank_update_algorithm.command == BOOT_SETTING_BANK_COMMAND_NEW)
//		{
//			NRF_LOG_INFO("update_algorithm image is new");
//			bank_addr = g_apollo_boot_setting.bank_update_algorithm.extFlash_address;
//			bank_size = g_apollo_boot_setting.bank_update_algorithm.image_size;
//			bank_crc = g_apollo_boot_setting.bank_update_algorithm.image_crc;
//			if(am_bootloader_check_crc_from_external_flash(bank_crc,bank_addr,bank_size))
//			{
//				NRF_LOG_INFO("update_algorithm image crc is correct");
//			}
//			else
//			{
//				NRF_LOG_INFO("update_algorithm image crc is wrong");
//				error |= POSTVALIDATE_BANK_UPDATE_ALGORITHM_CRC_ERROR_EXTFLASH;
//			}					
//		}		
	}
	else
	{
		error |= POSTVALIDATE_BOOTSETTING_CRC_ERROR;
	}		
	
	
	if(error == 0)
	{
		uint32_t bootsetting_address = 0x800000-0x1000;
		uint32_t bootsetting_size = sizeof(_apollo_boot_setting);
		_apollo_boot_setting buffer = {0};
		
		dev_extFlash_enable();		
		dev_extFlash_erase(bootsetting_address,bootsetting_size);	
		dev_extFlash_write(bootsetting_address, (uint8_t *)&g_apollo_boot_setting, bootsetting_size);	
		dev_extFlash_read(bootsetting_address, (uint8_t *)&buffer, bootsetting_size);			
		dev_extFlash_disable();			
		NRF_LOG_INFO("boot_setting_crc --> 0x%08x",g_apollo_boot_setting.boot_setting_crc);
		uint32_t boot_setting_crc_dest = am_bootloader_fast_crc32((uint8_t*)(&buffer.boot_setting_crc)+sizeof(buffer.boot_setting_crc),sizeof(_apollo_boot_setting) - sizeof(buffer.boot_setting_crc));
		NRF_LOG_INFO("boot_setting_crc_dest --> 0x%08x",boot_setting_crc_dest);			
//		uint32_t crc_src = am_bootloader_fast_crc32((uint8_t*)&g_apollo_boot_setting,sizeof(_apollo_boot_setting));
//		NRF_LOG_INFO("crc_src --> 0x%08x",crc_src);	
//		uint32_t crc_dest = am_bootloader_fast_crc32((uint8_t*)&buffer,sizeof(_apollo_boot_setting));
//		NRF_LOG_INFO("crc_dest --> 0x%08x",crc_dest);	
		if(g_apollo_boot_setting.boot_setting_crc == boot_setting_crc_dest)
		{
			NRF_LOG_INFO("bootsetting flash crc check is correct");
		}
		else
		{
			NRF_LOG_INFO("bootsetting flash crc check is wrong");
			error |= POSTVALIDATE_BOOTSETTING_CRC_ERROR;	
		}		
	}	
	
	response[1] = (uint8_t)(error>>0);
	response[2] = (uint8_t)(error>>8);
	response[3] = (uint8_t)(error>>16);
	response[4] = (uint8_t)(error>>24);	
	app_srv_apollo_flash_send((uint8_t *)&response, sizeof(response));	

}

static void task_extflash_evt_ble_dfu_apollo_prevalidate(void)
{
	uint8_t response[5];
	response[0] = CMD_APOLLO_SRV_FLASH_DFU_BLE_DFU_APOLLO_PREVALIDATE_RESPONSE;
	uint32_t error = 0;	
	
	uint32_t bootsetting_address = 0x800000-0x1000;
	uint32_t bootsetting_size = sizeof(_apollo_boot_setting);
	_apollo_boot_setting buffer = {0};
	
	dev_extFlash_enable();		
	dev_extFlash_erase(bootsetting_address,bootsetting_size);	
	dev_extFlash_write(bootsetting_address, (uint8_t *)&g_apollo_boot_setting, bootsetting_size);	
	dev_extFlash_read(bootsetting_address, (uint8_t *)&buffer, bootsetting_size);			
	dev_extFlash_disable();			
	NRF_LOG_INFO("boot_setting_crc --> 0x%08x",g_apollo_boot_setting.boot_setting_crc);
	uint32_t boot_setting_crc_dest = am_bootloader_fast_crc32((uint8_t*)(&buffer.boot_setting_crc)+sizeof(buffer.boot_setting_crc),sizeof(_apollo_boot_setting) - sizeof(buffer.boot_setting_crc));
	NRF_LOG_INFO("boot_setting_crc_dest --> 0x%08x",boot_setting_crc_dest);			
//		uint32_t crc_src = am_bootloader_fast_crc32((uint8_t*)&g_apollo_boot_setting,sizeof(_apollo_boot_setting));
//		NRF_LOG_INFO("crc_src --> 0x%08x",crc_src);	
//		uint32_t crc_dest = am_bootloader_fast_crc32((uint8_t*)&buffer,sizeof(_apollo_boot_setting));
//		NRF_LOG_INFO("crc_dest --> 0x%08x",crc_dest);	
	if(g_apollo_boot_setting.boot_setting_crc == boot_setting_crc_dest)
	{
		NRF_LOG_INFO("bootsetting flash crc check is correct");
	}
	else
	{
		NRF_LOG_INFO("bootsetting flash crc check is wrong");
		error |= POSTVALIDATE_BOOTSETTING_CRC_ERROR;	
	}		
	response[1] = (uint8_t)(error>>0);
	response[2] = (uint8_t)(error>>8);
	response[3] = (uint8_t)(error>>16);
	response[4] = (uint8_t)(error>>24);	
	app_srv_apollo_flash_send((uint8_t *)&response, sizeof(response));	
}
static void task_extflash_evt_ble_dfu_ble_prevalidate(void)
{
	uint8_t response[5];
	response[0] = CMD_APOLLO_SRV_FLASH_DFU_BLE_PREVALIDATE_RESPONSE;
	uint32_t error = 0;

	
	uint32_t bootsetting_address = 0x800000-0x1000;
	uint32_t bootsetting_size = sizeof(_ble_boot_setting);
	_ble_boot_setting buffer = {0};
	
	dev_extFlash_enable();		
	dev_extFlash_erase(bootsetting_address,bootsetting_size);	
	dev_extFlash_write(bootsetting_address, (uint8_t *)&g_ble_boot_setting, bootsetting_size);	
	dev_extFlash_read(bootsetting_address, (uint8_t *)&buffer, bootsetting_size);			
	dev_extFlash_disable();			
	NRF_LOG_INFO("boot_setting_crc --> 0x%08x",g_ble_boot_setting.boot_setting_crc);
	uint32_t boot_setting_crc_dest = am_bootloader_fast_crc32((uint8_t*)(&buffer.boot_setting_crc)+sizeof(buffer.boot_setting_crc),sizeof(_ble_boot_setting) - sizeof(buffer.boot_setting_crc));
	NRF_LOG_INFO("boot_setting_crc_dest --> 0x%08x",boot_setting_crc_dest);			
	uint32_t crc_src = am_bootloader_fast_crc32((uint8_t*)&g_ble_boot_setting,sizeof(_ble_boot_setting));
	NRF_LOG_INFO("crc_src --> 0x%08x",crc_src);	
	uint32_t crc_dest = am_bootloader_fast_crc32((uint8_t*)&buffer,sizeof(_ble_boot_setting));
	NRF_LOG_INFO("crc_dest --> 0x%08x",crc_dest);	
	if(crc_src == crc_dest)
	{
		NRF_LOG_INFO("bootsetting flash crc check is correct");
	}
	else
	{
		NRF_LOG_INFO("bootsetting flash crc check is wrong");
		error |= POSTVALIDATE_BOOTSETTING_CRC_ERROR;	
	}
	
	response[1] = (uint8_t)(error>>0);
	response[2] = (uint8_t)(error>>8);
	response[3] = (uint8_t)(error>>16);
	response[4] = (uint8_t)(error>>24);	
	app_srv_apollo_flash_send((uint8_t *)&response, sizeof(response));	
	
}
#ifdef COD
static void task_extflash_evt_ota_erase_raw(void)
{
	NRF_LOG_INFO("task_extflash_evt_ota_erase_raw");
    dev_extFlash_enable();
	dev_extFlash_erase(m_task_buffer.address, m_task_buffer.length);
    dev_extFlash_disable();	
}

static void task_extflash_evt_ota_write_raw(void)
{
	NRF_LOG_INFO("task_extflash_evt_ota_write_raw. write_address=0x%x,write_length=%d",m_task_buffer.address,m_task_buffer.length);
	dev_extFlash_enable();
	dev_extFlash_write(m_task_buffer.address, m_task_buffer.p_data, m_task_buffer.length);
    dev_extFlash_disable();	
}

//DEBUG CODE, YOU CAN DELETE WHEN RELEASE
static void test_extFlash_evt_read_raw(void)
{
	NRF_LOG_INFO("test_extFlash_evt_read_raw");
	uint32_t address = 0x700000;
	uint16_t length = 200;
	uint8_t buffer[200]={0};

	for(uint32_t i=0; i<1; i++)
	{
		dev_extFlash_enable();
		dev_extFlash_read(address, (uint8_t *)&buffer, length);
		dev_extFlash_disable();	
		address += length;
		NRF_LOG_HEXDUMP_INFO(&buffer, length);
        //DELAY_MS(100);
	}
}

/*根据时间 年月日时分秒 计算出秒数*/
unsigned int GetTimeSecond(rtc_time_t start)
{
	struct tm time_c = {0};
	time_t t1;
	
	time_c.tm_year = 2000 - 1900 + start.Year;
	time_c.tm_mon  = start.Month - 1;
	time_c.tm_mday = start.Day;
	time_c.tm_hour = start.Hour;
	time_c.tm_min  = start.Minute;
	time_c.tm_sec  = start.Second;

	t1 = mktime(&time_c);

	return t1;
}

//运动summary数据格式转换
void cod_sport_summary_conver(cod_sport_summary *summary, ActivityDataStr *ActivityData)
{
	summary->id = ActivityData->cod_id;

	if (ActivityData->Activity_Type == ACT_RUN || ActivityData->Activity_Type == ACT_CROSSCOUNTRY)
	{
		summary->sport_type = SPORT_TYPE_OUTDOOR_RUN;
	}
	else if (ActivityData->Activity_Type == ACT_HIKING)
	{
		summary->sport_type = SPORT_TYPE_OUTDOOR_WALK;
	}
	else if (ActivityData->Activity_Type == ACT_CLIMB)
	{
		summary->sport_type = SPORT_TYPE_CLIMB;
	}
	else if (ActivityData->Activity_Type == ACT_CYCLING)
	{
		summary->sport_type = SPORT_TYPE_RIDE;
	}

	summary->distance = ActivityData->ActivityDetails.CommonDetail.Distance *0.01;

	summary->total_time = GetTimeSecond(ActivityData->Act_Stop_Time) - GetTimeSecond(ActivityData->Act_Start_Time);

	summary->sport_time = ActivityData->ActTime;
   // NRF_LOG_INFO("%s distance %d,%d,%d\r\n",__func__,(summary->distance*100),summary->total_time,summary->sport_time);
	summary->start_time =(uint64_t)(GetTimeSecond(ActivityData->Act_Start_Time)-28800)*1000;

	summary->end_time = (uint64_t)(GetTimeSecond(ActivityData->Act_Stop_Time)-28800)*1000;
	//NRF_LOG_INFO("start_time %llx%llx,data %d,%d,%d,%d\r\n",summary->start_time>>32,summary->start_time,ActivityData->Act_Start_Time.Day,ActivityData->Act_Start_Time.Hour,ActivityData->Act_Start_Time.Minute,ActivityData->Act_Start_Time.Second);
	//NRF_LOG_INFO("end_time %llx%llx,data %d,%d,%d,%d\r\n",summary->end_time>>32,summary->end_time,ActivityData->Act_Stop_Time.Day,ActivityData->Act_Stop_Time.Hour,ActivityData->Act_Stop_Time.Minute,ActivityData->Act_Stop_Time.Second);
	if (ActivityData->Activity_Type == ACT_CYCLING)
	{
		summary->speed_max = ActivityData->ActivityDetails.CommonDetail.OptimumSpeed/36.f;
		summary->speed_avg = ActivityData->ActivityDetails.CommonDetail.Speed/36.f;
		summary->climb_distance = ActivityData->ActivityDetails.CommonDetail.HeartRateZone[1]/100.f;
	}
	else if (ActivityData->Activity_Type == ACT_CLIMB)
	{
		summary->speed_max = ActivityData->ActivityDetails.CommonDetail.OptimumSpeed/36.f; //需要新加
		summary->speed_avg = ActivityData->ActivityDetails.CommonDetail.Distance/100.f/ActivityData->ActTime;
		summary->total_step = ActivityData->ActivityDetails.CommonDetail.Steps;
		
	}
	else
	{	
		summary->total_step = ActivityData->ActivityDetails.CommonDetail.Steps;
		
		if (ActivityData->ActivityDetails.CommonDetail.OptimumSpeed == 0)
		{
			summary->speed_max = 0;
		}
		else
		{
			summary->speed_max = 1000.f/ActivityData->ActivityDetails.CommonDetail.OptimumSpeed;
		}
		if (ActivityData->ActivityDetails.CommonDetail.Speed == 0)
		{
			summary->speed_avg = 0;
		}
		else
		{
			summary->speed_avg = 1000.f/ActivityData->ActivityDetails.CommonDetail.Speed;
		}
	}
		
	
	summary->heart_avg = ActivityData->ActivityDetails.CommonDetail.AvgHeart;
	
	summary->climb_attitude = ActivityData->ActivityDetails.CommonDetail.RiseHeight;
	
	summary->calorie = ActivityData->ActivityDetails.CommonDetail.Calorie*0.001;
	//NRF_LOG_INFO("%scalorie %d,%d,%d,%d\r\n",__func__,(summary->calorie *100),(summary->speed_max*100),(summary->speed_avg*100),ActivityData->ActivityDetails.CommonDetail.Speed);
	summary->half_marathon_time = ActivityData->half_marathon_time;
	
	summary->full_marathon_time = ActivityData->full_marathon_time;

	summary->version_h = (ActivityData->cod_version >> 8)&0xFF;

	summary->version_l =  ActivityData->cod_version &0xFF;

	summary->agps_update_time = (uint64_t)(ActivityData->agps_update_time-28800)*1000;
	//NRF_LOG_INFO("%s version_h %d,%d,agps_update_time %llx%llx\r\n",__func__,summary->version_h,summary->version_l,summary->agps_update_time >>32,summary->agps_update_time);
}

//游泳summary数据格式转换
void cod_swim_summary_conver(cod_swim_summary *summary, ActivityDataStr *ActivityData)
{
	summary->id = ActivityData->cod_id;

	summary->swim_type = SWIM_TYPE_INDOOR;
	
	summary->distance = ActivityData->ActivityDetails.CommonDetail.Distance *0.01;

	summary->total_time = GetTimeSecond(ActivityData->Act_Stop_Time) - GetTimeSecond(ActivityData->Act_Start_Time);

	summary->sport_time = ActivityData->ActTime;
	
	summary->start_time = (uint64_t)(GetTimeSecond(ActivityData->Act_Start_Time)-28800)*1000;

	summary->end_time = (uint64_t)(GetTimeSecond(ActivityData->Act_Stop_Time)-28800)*1000;

	summary->calorie = ActivityData->ActivityDetails.CommonDetail.Calorie*0.001;
	
	if (ActivityData->ActivityDetails.SwimmingDetail.Stroke == Freestyle)
	{
		summary->stroke = SWIM_STROKE_FRONT_CRAWL;
	}
	else if (ActivityData->ActivityDetails.SwimmingDetail.Stroke == Backstroke)
	{
		summary->stroke = SWIM_STROKE_BACK_CRAWL;
	}
	else if (ActivityData->ActivityDetails.SwimmingDetail.Stroke == Breaststroke)
	{
		summary->stroke = SWIM_STROKE_BREASTSTROKE;
	}
	else if (ActivityData->ActivityDetails.SwimmingDetail.Stroke == ButterflyStroke)
	{
		summary->stroke = SWIM_STROKE_BUTTERFLY;
	}
	else if (ActivityData->ActivityDetails.SwimmingDetail.Stroke == Medley)
	{
		summary->stroke = SWIM_STROKE_MEDLEY;
	}
	else 
	{
		summary->stroke = SWIM_STROKE_OTHER;
	}

	summary->laps = ActivityData->ActivityDetails.SwimmingDetail.laps;
	
	summary->pool_length = ActivityData->ActivityDetails.SwimmingDetail.pool_length;
	
	summary->total_strokes = ActivityData->ActivityDetails.SwimmingDetail.Strokes;
	
	summary->stroke_freq = ActivityData->ActivityDetails.SwimmingDetail.Frequency;

	summary->pace_avg = ActivityData->ActivityDetails.SwimmingDetail.Speed;

	summary->swolf_avg = ActivityData->ActivityDetails.SwimmingDetail.Swolf/10;
	//NRF_LOG_INFO("cod_swim_summary_conver %d,%d,%d,%d,%d,%d\r\n",summary->laps,summary->pool_length,summary->total_strokes,summary->stroke_freq,summary->pace_avg,summary->swolf_avg);
}

 //运动轨迹中精度5位，统一用5位处理
 float GpsDistance5Bit(int32_t	LonA, int32_t  LatA, int32_t  LonB, int32_t  LatB)
 {
	 float distance = 0;
	 float	lona = LonA / 100000.f;
	 float	lata = LatA / 100000.f;
	 float	lonb = LonB / 100000.f;
	 float	latb = LatB / 100000.f;
	 
	 if ((lona == lonb) && (lata == latb))
	 {
		 distance = 0;
	 }
	 else
	 {
		 double  Temp_C;
		 Temp_C = acos(cos((double)lata  * PI / 180) * cos((double)latb * PI / 180) * cos((double)(lona - lonb)  * PI / 180)
			 + sin((double)lata  * PI / 180) * sin((double)latb  * PI / 180));
		 distance = (float)R_EARTH *Temp_C;  //??m 
	 }
 
	 return  distance;
}

//gps点格式转换  
void cod_gps_point_conver(cod_gps_point *cod_gps, GpspointStr *gps_point,uint32_t distance)
{
	cod_gps->time =(uint64_t)(gps_point->timestamp - 28800)*1000;
	cod_gps->latitude = gps_point->Lat /100000.f;
	cod_gps->longitude= gps_point->Lon/100000.f;
	cod_gps->altitude = gps_point->altitude;
	cod_gps->accuracy = gps_point->accuracy;
	if (last_lat != 0)
	{
		cod_gps->distance2pre_point = distance/100.f;//GpsDistance5Bit(gps_point->Lon,gps_point->Lat,last_lon,last_lat);
	}
	else
	{
		cod_gps->distance2pre_point = 0;
	}
	last_lon = gps_point->Lon;
	last_lat = gps_point->Lat;

	cod_gps->point_type = (cod_gps_point_type)gps_point->point_type;
}
//gps数据获取及转换
void cod_sport_data_conv_process_gps(void)
{
	uint32_t n,i;
	GpspointStr gps_point;
	cod_gps_point cod_gps;
	uint32_t startaddress,endaddress,start_dis,distance;
	if (frame_id_flag == 2)
	{
		NRF_LOG_INFO("%s \r\n",__func__);
		return;
	}
	startaddress = ActivityData.GPS_Start_Address;
	endaddress = ActivityData.GPS_Stop_Address;
	start_dis = ActivityData.DistancePoint_Start_Address;
	//endaddress = ActivityData.GPS_Stop_Address;
	last_lon = 0;
	last_lat = 0;
	
    if (endaddress >= startaddress)//是否超出结束地址
    {
	
		n = (endaddress - startaddress)/16;
       
    }
    else
    {
		n = (GPS_DATA_STOP_ADDRESS +1 - startaddress + endaddress - GPS_DATA_START_ADDRESS)/16;
    }
	
	//NRF_LOG_INFO("%s %d,%d\r\n",__func__,n,sizeof(GpspointStr));

	for (i = 0;i < n;i++)
    { 
	   if(startaddress + 16*i < GPS_DATA_STOP_ADDRESS)
       {
           dev_extFlash_enable();
           dev_extFlash_read(startaddress +16*i,(uint8_t*)&gps_point,16 );         
           dev_extFlash_disable();
       }
       else
       {
       
           dev_extFlash_enable();
           dev_extFlash_read(GPS_DATA_START_ADDRESS +16*i -(GPS_DATA_STOP_ADDRESS +1 - startaddress) ,(uint8_t*)&gps_point,16 );
           dev_extFlash_disable();
        
       }

	    if(start_dis + 4*i < DISTANCEPOINT_STOP_ADDRESS)
       {
           dev_extFlash_enable();
           dev_extFlash_read(start_dis +4*i,(uint8_t*)&distance,4 );         
           dev_extFlash_disable();
       }
       else
       {
       
           dev_extFlash_enable();
           dev_extFlash_read(DISTANCEPOINT_START_ADDRESS +4*i -(DISTANCEPOINT_STOP_ADDRESS +1 - start_dis) ,(uint8_t*)&distance,4 );
           dev_extFlash_disable();
        
       } 
	   
		cod_gps_point_conver(&cod_gps,&gps_point,distance);
		//NRF_LOG_INFO("cod_gps %d,%d,%d \r\n",cod_gps.time,cod_gps.point_type,cod_gps.accuracy,(cod_gps.distance2pre_point *100));
		//NRF_LOG_INFO("lat %d lon%d, alt %d\r\n",(cod_gps.latitude *100000),(cod_gps.longitude*100000),(uint16_t)cod_gps.altitude);

		cod_sport_data_conv_input_gps(&sport_converter,n,&cod_gps,1);
		if (frame_id_flag == 2)
		{
			NRF_LOG_INFO("%s \r\n",__func__);
			return;
		}
	   
	 }
}

//计步数据获取及转换
void cod_sport_data_conv_porcess_step(void)
{
	
	uint64_t sport_start_time;
	cod_step_min step_min;
	uint32_t n,i,step_start,step_end,dis_start;
	uint32_t step,distance,last_step,last_distance,time;
	if (frame_id_flag == 2)
	{
		NRF_LOG_INFO("%s \r\n",__func__);
		return;
	}
	sport_start_time = GetTimeSecond(ActivityData.Act_Start_Time) -28800;
	step_start = ActivityData.Steps_Start_Address;
	step_end = ActivityData.Steps_Stop_Address;
	dis_start =ActivityData.Distance_Start_Address;
	//dis_end = ActivityData.Distance_Stop_Address;
	last_step = 0;
	last_distance = 0;
	 if (step_end >= step_start)//是否超出结束地址
	 {
	  
		  n = (step_end - step_start)/8;
		 
	 }
	 else
	 {
		  n = (STEP_DATA_STOP_ADDRESS +1 - step_start + step_end - STEP_DATA_START_ADDRESS)/8;
	 }
	 NRF_LOG_DEBUG("%s %d\r\n",__func__,n);

	for (i = 0;i < n;i++)
    { 
    	//时间戳
    	if(i== 0)
    	{
    		step_min.start_time = sport_start_time*1000;
    	}
		else
		{

			if(step_start + 8*(i-1) < STEP_DATA_STOP_ADDRESS)
       		{
           		dev_extFlash_enable();
           		dev_extFlash_read(step_start +8*(i-1)+4,(uint8_t*)&time,4 );         
          		dev_extFlash_disable();
       		}
       		else
       		{
       
           		dev_extFlash_enable();
           		dev_extFlash_read(STEP_DATA_START_ADDRESS +8*(i-1) +4 -(STEP_DATA_STOP_ADDRESS +1 - step_start) ,(uint8_t*)time,4 );
          	    dev_extFlash_disable();
      		}
			step_min.start_time = (uint64_t)(sport_start_time + time)*1000;
	
		}
	
		
		//计步
	   if(step_start + 8*i < STEP_DATA_STOP_ADDRESS)
       {
           dev_extFlash_enable();
           dev_extFlash_read(step_start +8*i,(uint8_t*)&step,4 );         
           dev_extFlash_disable();
       }
       else
       {
       
           dev_extFlash_enable();
           dev_extFlash_read(STEP_DATA_START_ADDRESS +8*i -(STEP_DATA_STOP_ADDRESS +1 - step_start) ,(uint8_t*)step,4 );
           dev_extFlash_disable();
        
       }
	   
	   step_min.step = step -last_step;
	   last_step = step;
        //距离
	   if(dis_start + 4*i < DISTANCE_DATA_STOP_ADDRESS)
       {
           dev_extFlash_enable();
           dev_extFlash_read(dis_start +4*i,(uint8_t*)&distance,4 );         
           dev_extFlash_disable();
       }
       else
       {
       
           dev_extFlash_enable();
           dev_extFlash_read(DISTANCE_DATA_START_ADDRESS +4*i -(DISTANCE_DATA_STOP_ADDRESS +1 - dis_start) ,(uint8_t*)&distance,4 );
           dev_extFlash_disable();
        
       }

	   step_min.distance = (distance -last_distance)*0.01;
	   last_distance =distance;
	  //NRF_LOG_INFO("%s step_mindistance %d,%d,%d\r\n",__func__,(uint32_t)step_min.distance,distance,(uint32_t)(step_min.distance*100));
	  // NRF_LOG_INFO("step sport_start_time %d,%llx%llx,\r\n",(sport_start_time + (i+1)*20),step_min.start_time>>32,step_min.start_time);
	   cod_sport_data_conv_input_step(&sport_converter,n,&step_min,1);
	  	if (frame_id_flag == 2)
		{
			NRF_LOG_INFO("%s \r\n",__func__);
			return;
		}
	
	}
}
//心率数据获取及转换
void cod_sport_data_conv_process_heart(void)
{
	uint64_t sport_start_time;
	cod_heart_rate heart;
	uint32_t n,i,heart_start,heart_end,time_second; 
	if (frame_id_flag == 2)
	{
		NRF_LOG_INFO("%s \r\n",__func__);
		return;
	}
	sport_start_time = GetTimeSecond(ActivityData.Act_Start_Time) -28800 ;
	heart_start = ActivityData.HeartRate_Start_Address;
	heart_end = ActivityData.HeartRate_Stop_Address;
	//NRF_LOG_INFO("sport_start_time %d,%d,%d,%d,%d\r\n",sport_start_time,ActivityData.Act_Start_Time.Day,ActivityData.Act_Start_Time.Hour,ActivityData.Act_Start_Time.Minute,ActivityData.Act_Start_Time.Second);

	 if (heart_end >= heart_start)//是否超出结束地址
	 {
	  
		  n = (heart_end - heart_start)/8;
		 
	 }
	 else
	 {
		  n = (HEARTRATE_DATA_STOP_ADDRESS +1 - heart_start + heart_end - HEARTRATE_DATA_START_ADDRESS)/8;
	 }

	
	
	for (i = 0;i < n;i++)
    { 
    		
	   if(heart_start + 8*i < HEARTRATE_DATA_STOP_ADDRESS)
       {
           dev_extFlash_enable();
           dev_extFlash_read(heart_start +8*i,(uint8_t*)&heart.heart_rate,1); 
		   dev_extFlash_read(heart_start +8*i+1,(uint8_t*)&time_second,4); 
           dev_extFlash_disable();
       }
       else
       {
       
           dev_extFlash_enable();
           dev_extFlash_read(HEARTRATE_DATA_START_ADDRESS +8*i -(HEARTRATE_DATA_STOP_ADDRESS +1 - heart_start) ,(uint8_t*)&heart.heart_rate,1 );
		   dev_extFlash_read(HEARTRATE_DATA_START_ADDRESS +8*i+1 -(HEARTRATE_DATA_STOP_ADDRESS +1 - heart_start) ,(uint8_t*)&time_second,4 );

		   dev_extFlash_disable();
        
       }
	   heart.time =(uint64_t) (sport_start_time + time_second)*1000;

	   cod_sport_data_conv_input_heart(&sport_converter,n,&heart,1);
	  if (frame_id_flag == 2)
	  {
	  	 NRF_LOG_INFO("%s \r\n",__func__);
		 return;
	  }
	}
}

//公里牌数据获取及转换
void cod_sport_data_conv_process_kilometer_record(void)
{
	int32_t store_data[3];
	uint32_t n,i;
	uint32_t startaddress,endaddress;
	if (frame_id_flag == 2)
	{
		NRF_LOG_INFO("%s \r\n",__func__);
		return;
	}
	cod_kilometer_record kr_list;
	uint32_t last_time;
	startaddress = ActivityData.Pressure_Start_Address;
	endaddress = ActivityData.Pressure_Stop_Address;

	if (endaddress >= startaddress)//是否超出结束地址
    {
	
		n = (endaddress - startaddress)/16;
       
    }
    else
    {
		n = (PRESSURE_DATA_STOP_ADDRESS +1 - startaddress + endaddress - PRESSURE_DATA_START_ADDRESS)/16;
    }

	NRF_LOG_DEBUG("%s %d\r\n",__func__,n);
	last_time = 0;
	for (i = 0;i < n;i++)
    { 
	   if(startaddress + 16*i < PRESSURE_DATA_STOP_ADDRESS)
       {
           dev_extFlash_enable();
           dev_extFlash_read(startaddress +16*i,(uint8_t*)store_data,12);         
           dev_extFlash_disable();
       }
       else
       {
       
           dev_extFlash_enable();
           dev_extFlash_read(PRESSURE_DATA_START_ADDRESS +16*i -(PRESSURE_DATA_STOP_ADDRESS +1 - startaddress) ,(uint8_t*)store_data,12 );
           dev_extFlash_disable();
        
       }
		kr_list.total_time = store_data[0] - last_time ;
		last_time = store_data[0];
		kr_list.longitude  = store_data[1]/100000.f;
		kr_list.latitude   = store_data[2]/100000.f;
		NRF_LOG_INFO("%s %d,%d,%d\r\n",__func__,kr_list.total_time,(kr_list.longitude*100000),(kr_list.latitude*100000));
	   cod_sport_data_conv_input_kilometer_record(&sport_converter,n,&kr_list,1);
	   	if (frame_id_flag == 2)
	    {
	    	NRF_LOG_INFO("%s \r\n",__func__);
			return;
	    }
	 }

	
}

//读取泳姿点数据及转换
void cod_swim_data_conv_porcess_stroke(void)
{
	store_swim_stroke_point store_data;
	uint32_t n,i;
	uint32_t startaddress,endaddress;
	if (frame_id_flag == 2)
	{
		NRF_LOG_INFO("%s \r\n",__func__);
		return;
	}

	cod_swim_stroke_point stroke_list = {0};
	startaddress = ActivityData.Pressure_Start_Address;
	endaddress = ActivityData.Pressure_Stop_Address;

	if (endaddress >= startaddress)//是否超出结束地址
    {
	
		n = (endaddress - startaddress)/16;
       
    }
    else
    {
		n = (PRESSURE_DATA_STOP_ADDRESS +1 - startaddress + endaddress - PRESSURE_DATA_START_ADDRESS)/16;
    }

    
	
	for (i = 0;i < n;i++)
    { 
	   if(startaddress + 16*i < PRESSURE_DATA_STOP_ADDRESS)
       {
           dev_extFlash_enable();
           dev_extFlash_read(startaddress +16*i,(uint8_t*)&store_data,sizeof(store_swim_stroke_point));         
           dev_extFlash_disable();
       }
       else
       {
       
           dev_extFlash_enable();
           dev_extFlash_read(PRESSURE_DATA_START_ADDRESS +16*i -(PRESSURE_DATA_STOP_ADDRESS +1 - startaddress) ,(uint8_t*)&store_data,sizeof(store_swim_stroke_point));
           dev_extFlash_disable();
        
       }
		stroke_list.time = (uint64_t)(store_data.time -28800)*1000;
		stroke_list.lap_id = store_data.lap_id;

		if (store_data.stroke== Freestyle)
		{
			stroke_list.stroke = SWIM_STROKE_FRONT_CRAWL;
		}
		else if (store_data.stroke == Backstroke)
		{
			stroke_list.stroke = SWIM_STROKE_BACK_CRAWL;
		}
		else if (store_data.stroke == Breaststroke)
		{
			stroke_list.stroke = SWIM_STROKE_BREASTSTROKE;
		}
		else if (store_data.stroke == ButterflyStroke)
		{
			stroke_list.stroke = SWIM_STROKE_BUTTERFLY;
		}
		else 
		{
			stroke_list.stroke = SWIM_STROKE_OTHER;
		}
		NRF_LOG_INFO("%s %d,%llx%llx,%d,%d\r\n",__func__,sizeof(store_swim_stroke_point),stroke_list.time>>32,stroke_list.time,stroke_list.stroke,stroke_list.lap_id);
		cod_swim_data_conv_input_stroke(&swim_converter,n,&stroke_list,1);
		if (frame_id_flag == 2)
		{
			NRF_LOG_INFO("%s \r\n",__func__);
			return;
	    }
  }

	
}

//运动数据转换处理
void cod_sport_data_conv_process(void)
{
	cod_sport_summary summary;
	memset(&summary,0,sizeof(summary));
	if (frame_id_flag == 2)
	{
		NRF_LOG_INFO("%s \r\n",__func__);
		return;
	}
	cod_sport_data_conv_init_by_outer_buffer(&sport_converter,sport_buffer,sizeof(sport_buffer),&sport_callback);
	cod_sport_data_conv_start(&sport_converter);

	cod_sport_data_conv_process_gps();

	cod_sport_data_conv_porcess_step();

	cod_sport_data_conv_process_heart();

	cod_sport_data_conv_process_kilometer_record();

	cod_sport_summary_conver(&summary,&ActivityData);
	cod_sport_data_conv_finish(&sport_converter,&summary);
	//NRF_LOG_INFO("End %s\r\n",__func__);
}

//游泳数据转换处理
void cod_swim_data_conv_process(void)
{
	cod_swim_summary summary;
	memset(&summary,0,sizeof(summary));
	if (frame_id_flag == 2)
	{
		NRF_LOG_INFO("%s \r\n",__func__);
		return;
	}
	cod_swim_data_conv_init_by_outer_buffer(&swim_converter,sport_buffer,sizeof(sport_buffer),&sport_callback);
	cod_swim_data_conv_start(&swim_converter);
	
	cod_swim_data_conv_porcess_stroke();

	cod_swim_summary_conver(&summary,&ActivityData);
	
	cod_swim_data_conv_finish(&swim_converter,&summary);

	//NRF_LOG_INFO("End %s\r\n",__func__);
}

//运动数据发送处理
void cod_sport_data_copy_send(uint8_t* buffer, uint32_t output_size)
{
	
	uint16_t frame_len = cod_usr_ble_tx_mtu -8;//减数据头8
	bool last_frame = false;
	uint8_t read_buffer[frame_len];
	uint16_t len;
	uint32_t ret;
	if (frame_id_flag == 2)
	{
		NRF_LOG_INFO("%s \r\n",__func__);
		return;
	}
	//frame_id_flag = 0;//0 普通，1 10倍帧，2 最后帧
	ret = ring_buffer_write(&sport_data_rb,buffer,output_size);
	//NRF_LOG_INFO("%s ring_buffer_write %d,ret %d\r\n",__func__,output_size,ret);
	if (cur_frame_id == (total_frame_nums-1))
	{
		len = ring_buffer_data_left(&sport_data_rb);
		
		if ((len % frame_len) ==(total_size %frame_len))
		{
			NRF_LOG_INFO("%s last_frame_id len %d,total_size %d\r\n",__func__,len,total_size);
			ring_buffer_read(&sport_data_rb,read_buffer,len);
			last_frame = true;
			cod_cmd_get_data_resp(DATA_TYPE_SPORT,cur_frame_id,last_frame,read_buffer,len);
			frame_id_flag = 2;
		}
	}
    else if (ring_buffer_data_left(&sport_data_rb) >= frame_len)
	{
		ring_buffer_read(&sport_data_rb,read_buffer,frame_len);
		if ((cur_frame_id+1) % 10 ==0)
		{
			last_frame = true;
			frame_id_flag =1;
			if (cur_frame_id == (total_frame_nums-1))
			{
				frame_id_flag =2;
			}
		}
		//NRF_LOG_INFO("%s cur_frame_id %d \r\n",__func__,cur_frame_id);
		cod_cmd_get_data_resp(DATA_TYPE_SPORT,cur_frame_id,last_frame,read_buffer,frame_len);
		DELAY_MS(1);
		cur_frame_id++;

		if (cur_frame_id == (total_frame_nums-1))
		{
			len = ring_buffer_data_left(&sport_data_rb);
			if ((len % frame_len) ==(total_size %frame_len))
			{
				NRF_LOG_INFO("%s last_frame_id len %d,total_size %d\r\n",__func__,len,total_size);
				ring_buffer_read(&sport_data_rb,read_buffer,len);
				last_frame = true;
				cod_cmd_get_data_resp(DATA_TYPE_SPORT,cur_frame_id,last_frame,read_buffer,len);
				frame_id_flag = 2;
			}
		}

		if (frame_id_flag == 1)
		{
			NRF_LOG_INFO("%s wait \r\n",__func__);
			xSemaphoreTake(SportSemaphore,portMAX_DELAY);
			NRF_LOG_INFO("%s wake\r\n",__func__);
		}
	}
	
#if 0
	bool last_frame = false;
	if (send_buffer_len+output_size > MTU_155)
	{
		memcpy(send_buffer+send_buffer_len,buffer,MTU_155 -send_buffer_len);
		send_buffer_len = MTU_155;
		memcpy(remain_buffer+remain_buffer_len,buffer + (MTU_155 -send_buffer_len),(output_size + send_buffer_len - MTU_155));
		remain_buffer_len += output_size + send_buffer_len - MTU_155;
	}
	else
	{
		memcpy(send_buffer+send_buffer_len,buffer,output_size);
		send_buffer_len +=output_size;	
	}

	if (cur_frame_id == (total_frame_nums-1))
	{
		last_frame = true;
		cod_cmd_get_data_resp(DATA_TYPE_SPORT,cur_frame_id,last_frame,send_buffer,send_buffer_len);
	}
	else if(send_buffer_len == MTU_155)
	{
		if (cur_frame_id % 10 ==0)
		{
			last_frame = true;
		}
		cod_cmd_get_data_resp(DATA_TYPE_SPORT,cur_frame_id,last_frame,send_buffer,MTU_155);
		cur_frame_id++;
		memcpy(send_buffer,remain_buffer,remain_buffer_len);
		send_buffer_len = remain_buffer_len;
		remain_buffer_len = 0;
		
	}
#endif
}
//* 转换后的运动summary数据获取及发送
void cod_get_sport_summary_data_process(void)
{
	
	cod_sport_summary summary;
	uint8_t buffer[sport_summary_size];
	uint32_t output_size;
	memset(&summary,0,sizeof(summary));
	if (frame_id_flag == 2)
	{
		NRF_LOG_INFO("%s \r\n",__func__);
		return;
	}
	cod_sport_summary_conver(&summary,&ActivityData);
	output_size = sport_summary_size;
	//NRF_LOG_INFO("%s cur_total_size %d\r\n",__func__,cur_total_size[cur_sport_id]);
	cod_get_sport_summary_data(&summary,cur_total_size[cur_sport_id],buffer,output_size);

	cod_sport_data_copy_send(buffer,output_size);

}			
//* 转换后的游泳summary数据获取及发送
void cod_get_swim_summary_data_process(void)
{
	
	cod_swim_summary summary;
	uint8_t buffer[swim_summary_size];
	uint32_t output_size;
	memset(&summary,0,sizeof(summary));
	if (frame_id_flag == 2)
	{
		NRF_LOG_INFO("%s \r\n",__func__);
		return;
	}
	cod_swim_summary_conver(&summary,&ActivityData);
	output_size = swim_summary_size;
	//NRF_LOG_INFO("%s %d,%d\n",__func__,total_size,output_size);
	cod_get_swim_summary_data(&summary,cur_total_size[cur_sport_id],buffer,output_size);
	cod_sport_data_copy_send(buffer,output_size);

}

//获取运动数据帧数
static void task_extFlash_evt_sport_data_frame_num(void)
{
	uint32_t num ;
	total_size = 0;
	last_total_size = 0;
	get_data_flag = 0;
	cur_sport_id = 0;
	frame_id_flag = 0;
	uint16_t frame_len = cod_usr_ble_tx_mtu -8;//减数据头8
	for(uint8_t day =0;day < 7;day++)
	{
		for(uint8_t num = 0;num <DAY_ACTIVITY_MAX_NUM;num++)
		{
			dev_extFlash_enable();
			dev_extFlash_read( DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*day+ACTIVITY_DATA_START_ADDRESS + 
																ACTIVITY_DAY_DATA_SIZE*num,(uint8_t*)(&ActivityData),ACTIVITY_DAY_DATA_SIZE);
			dev_extFlash_disable();
		
		if (ActivityData.Act_Stop_Time.Day >= 1 && ActivityData.Act_Stop_Time.Day <= 31) //有效数据
		{
			NRF_LOG_INFO(" %s Day %x\r\n",__func__,ActivityData.Act_Stop_Time.Day );
			NRF_LOG_INFO(" %s Type %x\r\n",__func__,ActivityData.Activity_Type );
			if((ActivityData.Activity_Type == ACT_RUN) ||(ActivityData.Activity_Type == ACT_CROSSCOUNTRY)||(ActivityData.Activity_Type == ACT_HIKING)
				||(ActivityData.Activity_Type == ACT_CLIMB)||(ActivityData.Activity_Type == ACT_CYCLING))
			{
				cod_sport_data_conv_process();
				cur_total_size[cur_sport_id] =  total_size - last_total_size;
				last_total_size = total_size;
				cur_sport_id++;
			}
			else if (ActivityData.Activity_Type == ACT_SWIMMING)
			{
				cod_swim_data_conv_process();
				cur_total_size[cur_sport_id] =  total_size - last_total_size;
				last_total_size = total_size;
				cur_sport_id++;
			}
		}

		}
	}
	
	 num = total_size;
	 if ( (num %frame_len) == 0)
	 {
		num = num /frame_len;
	 }
	 else
	 {
		num = (num /frame_len) +1;
	 }
	total_frame_nums = num;
	NRF_LOG_INFO("%s num %d,total_size%d\r\n",__func__,num,total_size);
	cod_cmd_get_data_frame_num_resp(DATA_TYPE_SPORT,num,true);
}

//获取运动数据
static void task_extFlash_evt_get_sport_data(void)
{
	
	cur_frame_id = 0;
	cur_sport_id = 0;
	frame_id_flag = 0;
	#if 0
	send_buffer_len= 0;	
	remain_buffer_len = 0;
	#endif
	get_data_flag =1;
	for(uint8_t day =0;day < 7;day++)
	{
		for(uint8_t num = 0;num <DAY_ACTIVITY_MAX_NUM;num++)
		{
			dev_extFlash_enable();
			dev_extFlash_read( DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*day+ACTIVITY_DATA_START_ADDRESS + 
																ACTIVITY_DAY_DATA_SIZE*num,(uint8_t*)(&ActivityData),ACTIVITY_DAY_DATA_SIZE);
			dev_extFlash_disable();
		
		if (ActivityData.Act_Stop_Time.Day >= 1 && ActivityData.Act_Stop_Time.Day <= 31) //有效数据
		{
			if((ActivityData.Activity_Type == ACT_RUN) ||(ActivityData.Activity_Type == ACT_CROSSCOUNTRY)||(ActivityData.Activity_Type == ACT_HIKING)
				||(ActivityData.Activity_Type == ACT_CLIMB)||(ActivityData.Activity_Type == ACT_CYCLING))
			{
				cod_get_sport_summary_data_process();
				cod_sport_data_conv_process();
				cur_sport_id++;
				if (frame_id_flag == 2)
				{
					NRF_LOG_INFO("%s \r\n",__func__);
					return;
				}
			}
			else if (ActivityData.Activity_Type == ACT_SWIMMING)
			{
				cod_get_swim_summary_data_process();
				cod_swim_data_conv_process();
				cur_sport_id++;
			    if (frame_id_flag == 2)
				{
					NRF_LOG_INFO("%s \r\n",__func__);
					return;
				}
			}
		}

		}
	}

}

//获取全天计步数据
static void task_extFlash_evt_all_day_step_data(uint16_t start_frame)
{
	//NRF_LOG_DEBUG("%s %d\r\n",__func__,start_frame);
	uint8_t data_buf[150]; 
	uint32_t offset[8];//七天的全天计步对应偏移
	uint32_t off_start,off_end;
	uint8_t day_start,day_end,end_flag;
	uint16_t length,last_length;
	uint16_t frame_len = cod_usr_ble_tx_mtu -8;//减数据头8
	end_flag = 0;
	uint32_t tmp_offset =0;
	for (uint8_t i = 0;i < 7;i++) //
	{
		tmp_offset += all_day_steps[i].size;
		offset[i] = tmp_offset;
		//DELAY_MS(1);
		NRF_LOG_DEBUG("offset %d, %d\r\n",offset[i],all_day_steps[i].size);
	}
	offset[7] = offset[6] + 10;							
	for (uint8_t j= 0;j < 10;j++)
	{
		off_start = 0;
		off_end = 0;
		day_start = 0;
		day_end = 0;
		memset(data_buf,0,sizeof(data_buf));
		
		for (uint8_t i = 0;i < 8;i++)//开始偏移地址
		{
			if ((start_frame + j ) *frame_len < offset[i])
			{	
				if(i != 0)
				{
					off_start = (start_frame + j) *frame_len - offset[i-1];								
				}
				else
				{
					off_start = (start_frame + j) *frame_len;
				}
				day_start = i;
				break;
			}
		}
		for (uint8_t i = 0;i < 8;i++)//结束偏移地址
		{
			if ((start_frame + j+1) *frame_len < offset[i])
			{
				if (i != 0)
				{
					off_end = (start_frame + j+1) *frame_len - offset[i-1];
				}
				else
				{
					off_end= (start_frame + j+1) *frame_len;
				}
				day_end = i;						
				break;
			}
			else
			{
				if(i != 0)
				{
					off_end = offset[i]- offset[i-1];
					if (i==7)
					{
						end_flag = 1;
					}
				}
				else
				{
					off_end = offset[0];
				}
				day_end = i;
			}
		}
		
		last_length = 0;
		NRF_LOG_DEBUG("off_start %d,%d,day_start %d,%d\r\n",off_start,off_end,day_start,day_end);
		#if 1
			for(uint8_t i = day_start;i <= day_end;i++ )//读取数据
			{
				if (i == day_start )
				{
					if (i == 7)
					{
						length = 10 - off_start;
						memcpy((uint8_t *)&data_buf+last_length,(uint8_t *)&all_day_sync_data +off_start,length);
						last_length += length;
						//NRF_LOG_DEBUG(" 7 day_start %d,%d\r\n",length,last_length);
					}
					else
					{
						if (day_start==day_end)
						{
							length = off_end - off_start;
						}
						else
						{
							length = all_day_steps[i].size - off_start;
						}
						dev_extFlash_enable();
						dev_extFlash_read(all_day_steps[i].addr + off_start, (uint8_t *)&data_buf+last_length, length);
						dev_extFlash_disable(); 
						last_length += length;
						//NRF_LOG_DEBUG(" day_start %d,%d\r\n",length,last_length);
					}
					
				}
				else if (i == day_end)
				{
					if (i == 7)
					{
						length = off_end;
						memcpy((uint8_t *)&data_buf+last_length,(uint8_t *)&all_day_sync_data,length);
						last_length += length;
						//NRF_LOG_DEBUG(" 7day_end %d,%d\r\n",length,last_length);
					}
					else
					{
						if (day_start != day_end)
						{
							length = off_end;
						}
						else
						{
							length = 0;
						}
						dev_extFlash_enable();
						dev_extFlash_read(all_day_steps[i].addr, (uint8_t *)&data_buf+last_length, length);
						dev_extFlash_disable();
						last_length += length;
						//NRF_LOG_DEBUG(" day_end %d,%d\r\n",length,last_length)
					}
				}
				else
				{
					length = all_day_steps[i].size;
					dev_extFlash_enable();
					dev_extFlash_read(all_day_steps[i].addr, (uint8_t *)&data_buf+last_length, length); 
					dev_extFlash_disable(); 
					last_length += length;
					//NRF_LOG_DEBUG(" day_min %d,%d\r\n",length,last_length)
				}
			}
			#endif
			if (j == 9 || end_flag == 1) //发送数据
			{
				cod_cmd_get_data_resp(DATA_TYPE_STEP,start_frame + j, true,data_buf,last_length);
				DELAY_MS(1);
				//NRF_LOG_DEBUG(" last id:%d, len:%d,data:%x%x\r\n",start_frame + j,last_length,data_buf[0],data_buf[1]);
				break;
			}
			else
			{
				cod_cmd_get_data_resp(DATA_TYPE_STEP,start_frame + j, false,data_buf,last_length);
				DELAY_MS(1);
				//NRF_LOG_DEBUG(" no_last id:%d, len:%d,data:%x%x\r\n",start_frame + j,last_length,data_buf[0],data_buf[1]);
			}
																	
	}	
}

#endif

