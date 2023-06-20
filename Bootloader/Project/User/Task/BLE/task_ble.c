#include "task_ble.h"
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
#include <string.h>

#include "drv_ble.h"
#include "drv_pc.h"
#include "dev_ble.h"
#include "cmd_ble.h"
#include "lib_boot_setting.h"
#include "cmd_pc.h"
#ifdef LAYER_APPLICATION
#include "gui_ble.h"
#include "gui_util.h"
#include "timer_notify.h"
#include "gui_notify.h"
#include "drv_extFlash.h"
#include "gui_hwt.h"
#endif
#include "lib_boot.h"
#include "dev_bat.h"
#include "am_util.h"
#include "com_utils.h"


#include "drv_button.h"
#include "task_display.h"

#define MOUDLE_LOG_ENABLED 							1
#define MOUDLE_DEBUG_ENABLED   					1
#define MOUDLE_NAME                     "[TASK_BLE]:"

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

#define DELAY_US(X)                 am_util_delay_us(X)
#define DELAY_MS(X)                 am_util_delay_ms(X)
//#define DELAY_MS(X)                 vTaskDelay(X / portTICK_PERIOD_MS)
//#define DELAY_MS(X)                   do{TickType_t xLastWakeTime;xLastWakeTime = xTaskGetTickCount ();vTaskDelayUntil( &xLastWakeTime, X / portTICK_PERIOD_MS );}while(0)


static TaskHandle_t task_handle;
static EventGroupHandle_t evt_handle;
static bool m_initialized = false;

extern _bluetooth_status g_bluetooth_status;

#ifdef LAYER_APPLICATION
#define JUMP_BOOTLOADER               1

static uint8_t m_event = 0;
#endif

typedef struct{
	uint32_t *p_data;
	uint32_t length;
} _task_buffer;
static _task_buffer m_task_buffer = {NULL,0};
extern _dev_ble_info g_ble_info;
#ifdef LAYER_APPLICATION 
extern uint32_t g_gui_index;
extern uint32_t g_gui_index_save;
#endif
_bluetooth_status g_bluetooth_status = {true, BLE_DISCONNECT};
extern _dev_bat_event   g_bat_evt;
extern ScreenState_t   ScreenState;
extern QueueHandle_t 	DisplayQueue;

_ble_passkey g_ble_passkey;

static void task_cb(uint32_t evt, uint8_t *p_data, uint32_t length)
{
	LOG_PRINTF(MOUDLE_NAME"task_cb\n");
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

	while (1)
	{
		bitSet = xEventGroupWaitBits(evt_handle, 0xFF, pdTRUE,
												pdFALSE, portMAX_DELAY);
		DEBUG_PRINTF(MOUDLE_NAME"bitSet : %04X\n",bitSet);
			switch(bitSet)
			{
				case DRV_BLE_EVT_UART_DATA:{
					LOG_PRINTF(MOUDLE_NAME"DRV_BLE_EVT_UART_DATA\n");
					
					LOG_HEXDUMP((uint8_t *)m_task_buffer.p_data, 64) ;  
					
				
						switch(m_task_buffer.p_data[0]&0x0000FF00)
						{
							case BLE_UART_RESPONSE_APPLICATION_BLE_INFO:{
								LOG_PRINTF(MOUDLE_NAME"BLE_UART_RESPONSE_APPLICATION_BLE_INFO\n");
								
								_dev_ble_info *info = (_dev_ble_info *)m_task_buffer.p_data;
								g_ble_info.bootloader_version = info->bootloader_version;
								g_ble_info.application_version = info->application_version;
								g_ble_info.setting_version = info->setting_version;
								g_ble_info.softdevice_version = info->softdevice_version;
								g_ble_info.pcb_version = info->pcb_version;
								g_ble_info.mac_address_f4b = info->mac_address_f4b;
								g_ble_info.mac_address_l2b = info->mac_address_l2b;
								uint8_t buffer[256];
								memcpy(buffer,(uint8_t *)m_task_buffer.p_data, sizeof(buffer));							
								drv_ble_uart_close();
								/**<Check if the USB dectect pin is active, the ble info should forward to pc>*/
								if(g_bat_evt.cable_status == DEV_BAT_EVENT_USB_CONN_CHARGING)
								{
									drv_pc_send_data((uint8_t *)buffer, sizeof(buffer));
								}
								
								#ifdef LAYER_APPLICATION 
								if(g_gui_index == GUI_INDEX_BLE)
								{
									gui_ble_paint();		
								}
								#endif
								
							}break;
							case BLE_UART_CCOMMAND_APPLICATION_INFO_APOLLO2:{
								LOG_PRINTF(MOUDLE_NAME"BLE_UART_CCOMMAND_APPLICATION_INFO_APOLLO2\n");
								_boot_setting boot_setting;
								lib_boot_setting_read(&boot_setting);
								_info_apollo2 info_apollo2;
								info_apollo2.command = BLE_UART_RESPONSE_APPLICATION_INFO_APOLLO2 | BLE_UART_CCOMMAND_MASK_APPLICATION;
								info_apollo2.bootloader_version = boot_setting.bootloader_version;
								info_apollo2.application_version = boot_setting.application_version;
								info_apollo2.update_version = boot_setting.update_version;
								info_apollo2.pcb_version = boot_setting.pcb_version;
								info_apollo2.chip_id_0 = AM_REG(MCUCTRL, CHIPID0);
								info_apollo2.chip_id_1 = AM_REG(MCUCTRL, CHIPID1);
								drv_ble_uart_put((uint8_t *)&info_apollo2, sizeof(_info_apollo2));								
								
							}break;	
							case BLE_UART_CCOMMAND_APPLICATION_BLE_CONNECT:{
								LOG_PRINTF(MOUDLE_NAME"BLE_UART_CCOMMAND_APPLICATION_BLE_CONNECT\n");
								drv_ble_uart_close();
								g_bluetooth_status.is_conn = BLE_CONNECT;
								#ifdef LAYER_APPLICATION 
								if(g_gui_index < GUI_INDEX_NOTIFY_CONNECT)
								{
									g_gui_index_save = g_gui_index;		
								}else{
									timer_notify_stop();
								}
									g_gui_index = GUI_INDEX_NOTIFY_CONNECT;
									timer_notify_start(5000);
									gui_notify_connect_paint();
								#endif
							}break;		
							case BLE_UART_CCOMMAND_APPLICATION_BLE_DISCONNECT:{
								LOG_PRINTF(MOUDLE_NAME"BLE_UART_CCOMMAND_APPLICATION_BLE_DISCONNECT\n");
								drv_ble_uart_close();
								g_bluetooth_status.is_conn = BLE_DISCONNECT;
								#ifdef LAYER_APPLICATION 
								if(g_gui_index < GUI_INDEX_NOTIFY_CONNECT)
								{
									g_gui_index_save = g_gui_index;		
								}else{
									timer_notify_stop();
								}	
								g_gui_index = GUI_INDEX_NOTIFY_DISCONNECT;															
								timer_notify_start(5000);
								gui_notify_disconnect_paint();
								#endif								
								
							}break;		
							case BLE_UART_CCOMMAND_APPLICATION_BLE_BOND:{
								LOG_PRINTF(MOUDLE_NAME"BLE_UART_CCOMMAND_APPLICATION_BLE_BOND\n");
								drv_ble_uart_close();
								g_bluetooth_status.is_conn = BLE_BONDED;
								#ifdef LAYER_APPLICATION 
								if(g_gui_index < GUI_INDEX_NOTIFY_CONNECT)
								{
									g_gui_index_save = g_gui_index;		
								}else{
									timer_notify_stop();
								}	
								g_gui_index = GUI_INDEX_NOTIFY_BOND;																	
								timer_notify_start(5000);
								gui_notify_bond_paint();
								#endif								
							}break;	
							case BLE_UART_CCOMMAND_APPLICATION_BLE_PASSKEY:{
								LOG_PRINTF(MOUDLE_NAME"BLE_UART_CCOMMAND_APPLICATION_BLE_PASSKEY\n");
								memcpy((uint8_t *)&g_ble_passkey,(uint8_t *)m_task_buffer.p_data,sizeof(_ble_passkey));
								drv_ble_uart_close();
								
								
								#ifdef LAYER_APPLICATION 
								if(g_gui_index < GUI_INDEX_NOTIFY_CONNECT)
								{
									g_gui_index_save = g_gui_index;		
								}else{
									timer_notify_stop();
								}	
								g_gui_index = GUI_INDEX_NOTIFY_PASSKEY;								
								timer_notify_start(10000);
								gui_notify_passkey_paint();
								#endif								
							}break;	
							case BLE_UART_RESPONSE_APPLICATION_JUMP_BOOTLOADER:{
								LOG_PRINTF(MOUDLE_NAME"BLE_UART_RESPONSE_APPLICATION_JUMP_BOOTLOADER\n");
								uint8_t buffer[256];
								memcpy(buffer,(uint8_t *)m_task_buffer.p_data, sizeof(buffer));							
								drv_ble_uart_close();
								g_ble_info.build_date = *((uint32_t *)buffer+1);
								g_ble_info.build_time = *((uint32_t *)buffer+2);
								if(ScreenState == DISPLAY_SCREEN_BACK_DOOR)
								{
									ScreenState = DISPLAY_SCREEN_POST_WATCH_INFO;
									DISPLAY_MSG  msg = {0,0};
									msg.cmd = MSG_DISPLAY_SCREEN;
									xQueueSend(DisplayQueue, &msg, portMAX_DELAY);								
								
								}
								/**<Check if the USB dectect pin is active, the ble info should forward to pc>*/
								if(g_bat_evt.cable_status == DEV_BAT_EVENT_USB_CONN_CHARGING)
								{
									drv_pc_send_data((uint8_t *)buffer, sizeof(buffer));
								}
								
							}break;	
							case BLE_UART_RESPONSE_BOOTLOADER_RESET:{
								LOG_PRINTF(MOUDLE_NAME"BLE_UART_RESPONSE_BOOTLOADER_RESET\n");
								uint8_t buffer[256];
								memcpy(buffer,(uint8_t *)m_task_buffer.p_data, sizeof(buffer));							
								drv_ble_uart_close();
								/**<Check if the USB dectect pin is active, the ble info should forward to pc>*/
								if(g_bat_evt.cable_status == DEV_BAT_EVENT_USB_CONN_CHARGING)
								{
									drv_pc_send_data((uint8_t *)buffer, sizeof(buffer));
								}
								
							}break;								
							case BLE_UART_RESPONSE_BOOTLOADER_BOOTSETTING:{
								LOG_PRINTF(MOUDLE_NAME"BLE_UART_RESPONSE_BOOTLOADER_BOOTSETTING\n");
								uint8_t buffer[256];
								memcpy(buffer,(uint8_t *)m_task_buffer.p_data, sizeof(buffer));							
								drv_ble_uart_close();
								/**<Check if the USB dectect pin is active, the ble info should forward to pc>*/
								if(g_bat_evt.cable_status == DEV_BAT_EVENT_USB_CONN_CHARGING)
								{
									drv_pc_send_data((uint8_t *)buffer, sizeof(buffer));
								}
								
							}break;							
							case BLE_UART_RESPONSE_BOOTLOADER_PREVALIDATE:{
								LOG_PRINTF(MOUDLE_NAME"BLE_UART_RESPONSE_BOOTLOADER_PREVALIDATE\n");
								uint8_t buffer[256];
								memcpy(buffer,(uint8_t *)m_task_buffer.p_data, sizeof(buffer));							
								drv_ble_uart_close();
								/**<Check if the USB dectect pin is active, the ble info should forward to pc>*/
								if(g_bat_evt.cable_status == DEV_BAT_EVENT_USB_CONN_CHARGING)
								{
									drv_pc_send_data((uint8_t *)buffer, sizeof(buffer));
								}
								
							}break;										
							case BLE_UART_RESPONSE_BOOTLOADER_POSTVALIDATE:{
								LOG_PRINTF(MOUDLE_NAME"BLE_UART_RESPONSE_BOOTLOADER_POSTVALIDATE\n");
								uint8_t buffer[256];
								memcpy(buffer,(uint8_t *)m_task_buffer.p_data, sizeof(buffer));							
								drv_ble_uart_close();
								/**<Check if the USB dectect pin is active, the ble info should forward to pc>*/
								if(g_bat_evt.cable_status == DEV_BAT_EVENT_USB_CONN_CHARGING)
								{
									drv_pc_send_data((uint8_t *)buffer, sizeof(buffer));
								}								
								
							}break;			
							case BLE_UART_RESPONSE_BOOTLOADER_RESET_N_ACTIVATE:{
								LOG_PRINTF(MOUDLE_NAME"BLE_UART_RESPONSE_BOOTLOADER_RESET_N_ACTIVATE\n");
								uint8_t buffer[256];
								memcpy(buffer,(uint8_t *)m_task_buffer.p_data, sizeof(buffer));							
								drv_ble_uart_close();
								/**<Check if the USB dectect pin is active, the ble info should forward to pc>*/
								if(g_bat_evt.cable_status == DEV_BAT_EVENT_USB_CONN_CHARGING)
								{
									drv_pc_send_data((uint8_t *)buffer, sizeof(buffer));
								}	
								
							}break;					

							case PC_UART_CCOMMAND_APPLICATION_JUMP_BOOTLOADER:{
								LOG_PRINTF(MOUDLE_NAME"PC_UART_CCOMMAND_APPLICATION_JUMP_BOOTLOADER\n");
								uint32_t command[1] = {PC_UART_RESPONSE_APPLICATION_JUMP_BOOTLOADER | PC_UART_COMMAND_MASK_APPLICATION};
								drv_ble_uart_put((uint8_t *)&command, sizeof(command));	

								#ifdef LAYER_APPLICATION
								m_event = JUMP_BOOTLOADER;								
								#endif
								
							}break;
							#ifdef LAYER_APPLICATION 
							case PC_UART_COMMAND_HARDWARE_TEST_BLE_INFO_READ_RESPONSE:{
								LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_BLE_INFO_READ_RESPONSE\n");
								extern _gui_hwt g_gui_hwt;
								memcpy((uint8_t *)&g_ble_info,(uint8_t *)m_task_buffer.p_data,sizeof(_dev_ble_info));
								if(g_bat_evt.cable_status == DEV_BAT_EVENT_USB_CONN_CHARGING)
								{
									drv_pc_send_data((uint8_t *)m_task_buffer.p_data, m_task_buffer.length);
								}
								if(g_gui_index == GUI_INDEX_HWT)
								{
									g_gui_hwt.ble.read_info = true;
									gui_hwt_paint();		
								}
								
							}break;
							case PC_UART_COMMAND_HARDWARE_TEST_BLE_EXTFLASH_READ_RESPONSE:{
								LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_BLE_EXTFLASH_READ_RESPONSE\n");
								drv_ble_uart_close();
								if(g_bat_evt.cable_status == DEV_BAT_EVENT_USB_CONN_CHARGING)
								{
									drv_pc_send_data((uint8_t *)m_task_buffer.p_data, m_task_buffer.length);
								}
							}break;							
							
							
							
							#endif
							
							
							case BLE_UART_CCOMMAND_APPLICATION_UNSPPORT:{
								LOG_PRINTF(MOUDLE_NAME"BLE_UART_CCOMMAND_APPLICATION_UNSPPORT\n");
								drv_ble_uart_close();
								if(g_bat_evt.cable_status == DEV_BAT_EVENT_USB_CONN_CHARGING)
								{
									drv_pc_send_data((uint8_t *)m_task_buffer.p_data, m_task_buffer.length);
								}
							}break;									
							
							default:
		
								break;
						}

				}break;				
				case DRV_BLE_EVT_UART_CTS_INTERRUPT:{
					LOG_PRINTF(MOUDLE_NAME"DRV_BLE_EVT_UART_CTS_INTERRUPT\n");
					//DELAY_US(10);
//					vTaskDelay(10);
					if (PIN_READ_INPUT(DRV_BLE_UART_CTS_PIN))
					{
							drv_ble_on_cts_high();				
					}
					else
					{
							drv_ble_on_cts_low();
						#ifdef LAYER_APPLICATION
						switch(m_event)
						{
							case JUMP_BOOTLOADER:{
								LOG_PRINTF(MOUDLE_NAME"JUMP_BOOTLOADER\n");
								lib_boot_jump_to_bootloader(BOOT_SETTING_ENTER_BOOT_REASON_DFU_BLE);
							}break;
						
						}
						#endif
					}						
				}break;				
			
				
				
				default:
					break;
			}		
	
	}
}

void task_ble_init(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"task_ble_init\n");
	if(m_initialized)
	{
		return;
	}
	
	m_initialized = true;	

#if !defined JW902
	drv_ble_init(task_cb);
#else
	jw902_drv_ble_init(task_cb);
#endif

	g_bluetooth_status.is_on = true;
	g_bluetooth_status.is_conn = BLE_DISCONNECT;
	
	evt_handle = xEventGroupCreate();
	if(evt_handle == NULL)
	{
		DEBUG_PRINTF(MOUDLE_NAME"xEventGroupCreate failed\n");
		ERR_HANDLER(ERR_NULL);
	}
	BaseType_t xResult = xTaskCreate(task, "BLE", 1024, 0, 5, &task_handle);
	if (xResult == pdFAIL)
	{
		ERR_HANDLER(ERR_FORBIDDEN);
	}
}






void task_ble_uninit(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"task_ble_uninit\n");
	if(!m_initialized)
	{
		return;
	}	
	m_initialized = false;	

	drv_ble_uart_close_on_ble_sys_off();

	g_bluetooth_status.is_on = false;
	g_bluetooth_status.is_conn = BLE_DISCONNECT;
	
	
	vTaskDelete(task_handle);
	vEventGroupDelete(evt_handle);

}









