#include "dev_hwt.h"
#include <stdlib.h>
#include "rtt_log.h"
#include "lib_error.h"
#include "SEGGER_RTT.h"


#include "am_mcu_apollo.h"
#include "drv_pc.h"

#include "dev_extFlash.h"
#include "com_apollo2.h"
#include "cmd_pc.h"
#include "lib_boot_setting.h"
#include "drv_extFlash.h"
#include "dev_extFlash.h"

#include <string.h>
#include "drv_intFlash.h"
#include "lib_boot.h"

#include "lib_boot_setting.h"
#ifdef AM_FREERTOS 
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "portmacro.h"
#include "portable.h"
#include "event_groups.h"
#endif 

#include "gui_util.h"
#include "gui_hwt.h"
#include "gui_notify.h"
#include "drv_ble.h"
#include "dev_extFlash.h"
#include "dev_font.h"
#include "task_ble.h"
#include "dev_ble.h"


#define MOUDLE_LOG_ENABLED 							1
#define MOUDLE_DEBUG_ENABLED   					1
#define MOUDLE_NAME                     "[DEV_HWT]:"

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




extern uint32_t g_gui_index;
extern uint32_t g_gui_index_save;
extern _gui_hwt g_gui_hwt;
static uint8_t *dev_extFlash_buffer_p = NULL;

extern _bluetooth_status g_bluetooth_status;

typedef struct {
	uint32_t command;
	int32_t x;
	int32_t y;
	int32_t z;
} _dev_hwt_motion;

#define HWT_BACKLIGHT_PIN             40
#define HWT_MOTOR_PIN                 17
#define HWT_BUZZER_PIN                32



void dev_hwt_event_handle(uint32_t *p_data, uint32_t length)
{
	LOG_PRINTF(MOUDLE_NAME"dev_hwt_event_handle\n");
	DEBUG_HEXDUMP((uint8_t *)p_data,16);	
	static short x=-999,y=20,z=500;
	static uint16_t factor = 10;
	switch(p_data[0]&0x0000FF00)
	{
		case PC_UART_COMMAND_HARDWARE_TEST_MODE_ENABLE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_MODE_ENABLE\n");
			memset((uint8_t *)&g_gui_hwt,0,sizeof(g_gui_hwt));
			g_gui_index_save = g_gui_index;
			g_gui_index = GUI_INDEX_HWT;
			gui_hwt_paint();			
			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_MODE_ENABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));		
		}break;
		case PC_UART_COMMAND_HARDWARE_TEST_MODE_DISABLE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_MODE_DISABLE\n");
			g_gui_index = g_gui_index_save;
			g_gui_index_save = g_gui_index;	
			gui_notify_rollback_paint(g_gui_index);		
			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_MODE_DISABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));		
		}break;		
		case PC_UART_COMMAND_HARDWARE_TEST_BUTTON_PWR_ENABLE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_BUTTON_PWR_ENABLE\n");
			g_gui_hwt.button_pwr.enable = true;
			g_gui_hwt.button_pwr.press_cnt = 0;
			gui_hwt_paint();
			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_BUTTON_PWR_ENABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));		
		}break;				
		case PC_UART_COMMAND_HARDWARE_TEST_BUTTON_PWR_PRESS_RESPONSE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_BUTTON_PWR_PRESS_RESPONSE\n");	
		}break;			
		case PC_UART_COMMAND_HARDWARE_TEST_BUTTON_PWR_DISABLE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_BUTTON_PWR_DISABLE\n");
			g_gui_hwt.button_pwr.enable = false;
			g_gui_hwt.button_pwr.press_cnt = 0;
			gui_hwt_paint();
			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_BUTTON_PWR_DISABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));			
		}break;			
		case PC_UART_COMMAND_HARDWARE_TEST_BUTTON_BL_ENABLE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_BUTTON_BL_ENABLE\n");
			g_gui_hwt.button_bl.enable = true;
			g_gui_hwt.button_bl.press_cnt = 0;
			gui_hwt_paint();
			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_BUTTON_BL_ENABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));		
		}break;				
		case PC_UART_COMMAND_HARDWARE_TEST_BUTTON_BL_PRESS_RESPONSE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_BUTTON_BL_PRESS_RESPONSE\n");	
		}break;			
		case PC_UART_COMMAND_HARDWARE_TEST_BUTTON_BL_DISABLE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_BUTTON_BL_DISABLE\n");
			g_gui_hwt.button_bl.enable = false;
			g_gui_hwt.button_bl.press_cnt = 0;
			gui_hwt_paint();
			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_BUTTON_BL_DISABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));			
		}break;	
		case PC_UART_COMMAND_HARDWARE_TEST_BUTTON_UP_ENABLE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_BUTTON_UP_ENABLE\n");
			g_gui_hwt.button_up.enable = true;
			g_gui_hwt.button_up.press_cnt = 0;
			gui_hwt_paint();
			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_BUTTON_UP_ENABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));		
		}break;				
		case PC_UART_COMMAND_HARDWARE_TEST_BUTTON_UP_PRESS_RESPONSE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_BUTTON_UP_PRESS_RESPONSE\n");	
		}break;			
		case PC_UART_COMMAND_HARDWARE_TEST_BUTTON_UP_DISABLE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_BUTTON_UP_DISABLE\n");
			g_gui_hwt.button_up.enable = false;
			g_gui_hwt.button_up.press_cnt = 0;
			gui_hwt_paint();
			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_BUTTON_UP_DISABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));			
		}break;			
		case PC_UART_COMMAND_HARDWARE_TEST_BUTTON_OK_ENABLE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_BUTTON_OK_ENABLE\n");
			g_gui_hwt.button_ok.enable = true;
			g_gui_hwt.button_ok.press_cnt = 0;
			gui_hwt_paint();
			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_BUTTON_OK_ENABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));		
		}break;				
		case PC_UART_COMMAND_HARDWARE_TEST_BUTTON_OK_PRESS_RESPONSE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_BUTTON_OK_PRESS_RESPONSE\n");	
		}break;			
		case PC_UART_COMMAND_HARDWARE_TEST_BUTTON_OK_DISABLE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_BUTTON_OK_DISABLE\n");
			g_gui_hwt.button_ok.enable = false;
			g_gui_hwt.button_ok.press_cnt = 0;
			gui_hwt_paint();
			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_BUTTON_OK_DISABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));			
		}break;				
		case PC_UART_COMMAND_HARDWARE_TEST_BUTTON_DWN_ENABLE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_BUTTON_DWN_ENABLE\n");
			g_gui_hwt.button_dwn.enable = true;
			g_gui_hwt.button_dwn.press_cnt = 0;
			gui_hwt_paint();
			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_BUTTON_DWN_ENABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));		
		}break;				
		case PC_UART_COMMAND_HARDWARE_TEST_BUTTON_DWN_PRESS_RESPONSE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_BUTTON_DWN_PRESS_RESPONSE\n");	
		}break;			
		case PC_UART_COMMAND_HARDWARE_TEST_BUTTON_DWN_DISABLE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_BUTTON_DWN_DISABLE\n");
			g_gui_hwt.button_dwn.enable = false;
			g_gui_hwt.button_dwn.press_cnt = 0;
			gui_hwt_paint();
			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_BUTTON_DWN_DISABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));			
		}break;			
		case PC_UART_COMMAND_HARDWARE_TEST_LCD_ENABLE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_LCD_ENABLE\n");
			g_gui_hwt.lcd_enable = true;
			gui_hwt_paint();			
			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_LCD_ENABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));			
		}break;		
		case PC_UART_COMMAND_HARDWARE_TEST_LCD_DISABLE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_LCD_DISABLE\n");
			g_gui_hwt.lcd_enable = false;
			gui_hwt_paint();					
			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_LCD_DISABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));			
		}break;			
		case PC_UART_COMMAND_HARDWARE_TEST_BACKLIGHT_ENABLE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_BACKLIGHT_ENABLE\n");
			PIN_CONFIG(HWT_BACKLIGHT_PIN, AM_HAL_GPIO_OUTPUT);
			PIN_SET(HWT_BACKLIGHT_PIN);
			g_gui_hwt.backlight_enable = true;
			gui_hwt_paint();			
			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_BACKLIGHT_ENABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));			
		}break;		
		case PC_UART_COMMAND_HARDWARE_TEST_BACKLIGHT_DISABLE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_BACKLIGHT_DISABLE\n");
			PIN_CLEAR(HWT_BACKLIGHT_PIN);		
			PIN_CONFIG(HWT_BACKLIGHT_PIN, AM_HAL_GPIO_DISABLE);
			g_gui_hwt.backlight_enable = false;
			gui_hwt_paint();					
			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_BACKLIGHT_DISABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));			
		}break;			
		case PC_UART_COMMAND_HARDWARE_TEST_MOTOR_ENABLE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_MOTOR_ENABLE\n");
			PIN_CONFIG(HWT_MOTOR_PIN, AM_HAL_GPIO_OUTPUT);
			PIN_SET(HWT_MOTOR_PIN);			
			g_gui_hwt.motor_enable = true;
			gui_hwt_paint();			
			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_MOTOR_ENABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));			
		}break;		
		case PC_UART_COMMAND_HARDWARE_TEST_MOTOR_DISABLE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_MOTOR_DISABLE\n");
			PIN_CLEAR(HWT_MOTOR_PIN);		
			PIN_CONFIG(HWT_MOTOR_PIN, AM_HAL_GPIO_DISABLE);			
			g_gui_hwt.motor_enable = false;
			gui_hwt_paint();					
			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_MOTOR_DISABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));			
		}break;	
		case PC_UART_COMMAND_HARDWARE_TEST_BUZZER_ENABLE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_BUZZER_ENABLE\n");
			PIN_CONFIG(HWT_BUZZER_PIN, AM_HAL_GPIO_OUTPUT);
			PIN_SET(HWT_BUZZER_PIN);				
			g_gui_hwt.buzzer_enable = true;
			gui_hwt_paint();			
			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_BUZZER_ENABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));			
		}break;		
		case PC_UART_COMMAND_HARDWARE_TEST_BUZZER_DISABLE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_BUZZER_DISABLE\n");
			PIN_CLEAR(HWT_BUZZER_PIN);		
			PIN_CONFIG(HWT_BUZZER_PIN, AM_HAL_GPIO_DISABLE);					
			g_gui_hwt.buzzer_enable = false;
			gui_hwt_paint();					
			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_BUZZER_DISABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));			
		}break;				
		case PC_UART_COMMAND_HARDWARE_TEST_EXTFLASH_ENABLE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_EXTFLASH_ENABLE\n");
			g_gui_hwt.extFlash.enable = true;
			gui_hwt_paint();					
			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_EXTFLASH_ENABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));			
		}break;		
		case PC_UART_COMMAND_HARDWARE_TEST_EXTFLASH_SECTOR:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_EXTFLASH_SECTOR\n");
			uint32_t address = p_data[1];
			uint32_t size = p_data[2];
			LOG_PRINTF(MOUDLE_NAME"address --> 0x08X\n",address);		
			LOG_PRINTF(MOUDLE_NAME"size --> %d\n",size);
			dev_extFlash_buffer_p = pvPortMalloc(W25Q128_PAGE_SIZE);
			memset(dev_extFlash_buffer_p,0x5A,W25Q128_PAGE_SIZE);
			uint32_t crc_src = am_bootloader_fast_crc32(dev_extFlash_buffer_p,W25Q128_PAGE_SIZE);
			LOG_PRINTF(MOUDLE_NAME"crc_src --> 0x%08x\n",crc_src);	
			dev_extFlash_enable();			
			dev_extFlash_erase(address, size);
			dev_extFlash_write(address, dev_extFlash_buffer_p, size);
			dev_extFlash_read(address, dev_extFlash_buffer_p, size);			
			dev_extFlash_disable();	
			uint32_t crc_dest = am_bootloader_fast_crc32(dev_extFlash_buffer_p,W25Q128_PAGE_SIZE);
			vPortFree(dev_extFlash_buffer_p);
			LOG_PRINTF(MOUDLE_NAME"crc_dest --> 0x%08x\n",crc_dest);			
			uint32_t command[2] = {PC_UART_COMMAND_HARDWARE_TEST_EXTFLASH_SECTOR_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			if(crc_src == crc_dest)
			{
				LOG_PRINTF(MOUDLE_NAME"SECTOR HWT SUCCESS\n");	
				command[1] = ERR_SUCCESS;
			}else{
				LOG_PRINTF(MOUDLE_NAME"SECTOR HWT FAIL\n");
				command[1] = ERR_CRC;
			}
			g_gui_hwt.extFlash.type = HWT_EXTFLASH_TYPE_SECTOR;
			g_gui_hwt.extFlash.address = address;
			gui_hwt_paint();		
			drv_pc_send_data((uint8_t *)command, sizeof(command));			
		}break;				
		case PC_UART_COMMAND_HARDWARE_TEST_EXTFLASH_BLOCK32:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_EXTFLASH_BLOCK32\n");			
			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_EXTFLASH_BLOCK32_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));			
		}break;				
		case PC_UART_COMMAND_HARDWARE_TEST_EXTFLASH_BLOCK64:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_EXTFLASH_BLOCK64\n");			
			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_EXTFLASH_BLOCK64_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));			
		}break;				
		case PC_UART_COMMAND_HARDWARE_TEST_EXTFLASH_ERASE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_EXTFLASH_ERASE\n");			
			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_EXTFLASH_ERASE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));			
		}break;		
		case PC_UART_COMMAND_HARDWARE_TEST_EXTFLASH_READ:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_EXTFLASH_READ\n");			
			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_EXTFLASH_READ_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));			
		}break;				
		case PC_UART_COMMAND_HARDWARE_TEST_EXTFLASH_WRITE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_EXTFLASH_WRITE\n");			
			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_EXTFLASH_WRITE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));			
		}break;					
		case PC_UART_COMMAND_HARDWARE_TEST_EXTFLASH_DISABLE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_EXTFLASH_DISABLE\n");
			dev_extFlash_enter_deep_sleep();			
			g_gui_hwt.extFlash.enable = false;
			gui_hwt_paint();					
			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_EXTFLASH_DISABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));			
		}break;				
		case PC_UART_COMMAND_HARDWARE_TEST_FONT_ENABLE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_FONT_ENABLE\n");
			g_gui_hwt.font_enable = true;
			gui_hwt_paint();					
			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_FONT_ENABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));			
		}break;		
		case PC_UART_COMMAND_HARDWARE_TEST_FONT_ERASE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_FONT_ERASE\n");	
			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_FONT_ERASE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));			
		}break;		
		case PC_UART_COMMAND_HARDWARE_TEST_FONT_READ:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_FONT_READ\n");			
			uint32_t command[2] = {PC_UART_COMMAND_HARDWARE_TEST_FONT_READ_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			command[1] = dev_font_self_test();
			drv_pc_send_data((uint8_t *)command, sizeof(command));			
		}break;				
		case PC_UART_COMMAND_HARDWARE_TEST_FONT_WRITE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_FONT_WRITE\n");			
			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_FONT_WRITE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));			
		}break;					
		case PC_UART_COMMAND_HARDWARE_TEST_FONT_DISABLE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_FONT_DISABLE\n");		
			g_gui_hwt.font_enable = false;
			gui_hwt_paint();					
			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_FONT_DISABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));			
		}break;		
		case PC_UART_COMMAND_HARDWARE_TEST_GSENSOR_ENABLE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_GSENSOR_ENABLE\n");	
			g_gui_hwt.g_sensor.enable = true;
			g_gui_hwt.g_sensor.int1 = false;
			g_gui_hwt.g_sensor.int2 = false;
			g_gui_hwt.g_sensor.x = 0;
			g_gui_hwt.g_sensor.y = 0;
			g_gui_hwt.g_sensor.z = 0;					
			gui_hwt_paint();
			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_GSENSOR_ENABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));			
		}break;		
		case PC_UART_COMMAND_HARDWARE_TEST_GSENSOR_READ:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_GSENSOR_READ\n");	
			x += factor;
			y += factor;
			z += factor;
			_dev_hwt_motion motion;
			motion.command = PC_UART_COMMAND_HARDWARE_TEST_GSENSOR_READ_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST;
			motion.x = x;
			motion.y = y;
			motion.z = z;
			g_gui_hwt.g_sensor.x = motion.x;
			g_gui_hwt.g_sensor.y = motion.y;
			g_gui_hwt.g_sensor.z = motion.z;					
			gui_hwt_paint();			
			LOG_PRINTF(MOUDLE_NAME"x : %d, y : %d, z : %d\n",motion.x,motion.y,motion.z);
			drv_pc_send_data((uint8_t *)&motion, sizeof(motion));			
		}break;			
		case PC_UART_COMMAND_HARDWARE_TEST_GSENSOR_INT1_RESPONSE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_GSENSOR_INT1_RESPONSE\n");	


		}break;			
		case PC_UART_COMMAND_HARDWARE_TEST_GSENSOR_INT2_RESPONSE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_GSENSOR_INT2_RESPONSE\n");	


		}break;		
		case PC_UART_COMMAND_HARDWARE_TEST_GSENSOR_DISABLE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_GSENSOR_DISABLE\n");
			x=0;y=0;z=0;			
			g_gui_hwt.g_sensor.enable = false;
			g_gui_hwt.g_sensor.int1 = false;
			g_gui_hwt.g_sensor.int2 = false;
			g_gui_hwt.g_sensor.x = 0;
			g_gui_hwt.g_sensor.y = 0;
			g_gui_hwt.g_sensor.z = 0;				
			gui_hwt_paint();
			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_GSENSOR_DISABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));		
		}break;		
		case PC_UART_COMMAND_HARDWARE_TEST_GYRO_ENABLE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_GYRO_ENABLE\n");	
			g_gui_hwt.gyro.enable = true;
			g_gui_hwt.gyro.int1 = false;
			g_gui_hwt.gyro.int2 = false;
			g_gui_hwt.gyro.x = 0;
			g_gui_hwt.gyro.y = 0;
			g_gui_hwt.gyro.z = 0;				
			gui_hwt_paint();
			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_GYRO_ENABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));		
		}break;
		case PC_UART_COMMAND_HARDWARE_TEST_GYRO_READ:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_GYRO_READ\n");	
			x += factor;
			y += factor;
			z += factor;
			_dev_hwt_motion motion;
			motion.command = PC_UART_COMMAND_HARDWARE_TEST_GYRO_READ_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST;
			motion.x = x;
			motion.y = y;
			motion.z = z;
			g_gui_hwt.gyro.x = motion.x;
			g_gui_hwt.gyro.y = motion.y;
			g_gui_hwt.gyro.z = motion.z;					
			gui_hwt_paint();			
			LOG_PRINTF(MOUDLE_NAME"x : %d, y : %d, z : %d\n",motion.x,motion.y,motion.z);
			drv_pc_send_data((uint8_t *)&motion, sizeof(motion));					
		}break;
		case PC_UART_COMMAND_HARDWARE_TEST_GYRO_DISABLE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_GYRO_DISABLE\n");	
			x=0;y=0;z=0;
			g_gui_hwt.gyro.enable = false;
			g_gui_hwt.gyro.int1 = false;
			g_gui_hwt.gyro.int2 = false;
			g_gui_hwt.gyro.x = 0;
			g_gui_hwt.gyro.y = 0;
			g_gui_hwt.gyro.z = 0;				
			gui_hwt_paint();
			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_GYRO_DISABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));		
		}break;		
		case PC_UART_COMMAND_HARDWARE_TEST_COMPASS_ENABLE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_COMPASS_ENABLE\n");	
			g_gui_hwt.compass.enable = true;
			g_gui_hwt.compass.int1 = false;
			g_gui_hwt.compass.int2 = false;
			g_gui_hwt.compass.x = 0;
			g_gui_hwt.compass.y = 0;
			g_gui_hwt.compass.z = 0;
			gui_hwt_paint();
			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_COMPASS_ENABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));		
		}break;
		case PC_UART_COMMAND_HARDWARE_TEST_COMPASS_READ:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_COMPASS_READ\n");	
			x += factor;
			y += factor;
			z += factor;
			_dev_hwt_motion motion;
			motion.command = PC_UART_COMMAND_HARDWARE_TEST_COMPASS_READ_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST;
			motion.x = x;
			motion.y = y;
			motion.z = z;
			g_gui_hwt.compass.x = motion.x;
			g_gui_hwt.compass.y = motion.y;
			g_gui_hwt.compass.z = motion.z;					
			gui_hwt_paint();			
			LOG_PRINTF(MOUDLE_NAME"x : %d, y : %d, z : %d\n",motion.x,motion.y,motion.z);
			drv_pc_send_data((uint8_t *)&motion, sizeof(motion));			
		}break;
		case PC_UART_COMMAND_HARDWARE_TEST_COMPASS_DISABLE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_COMPASS_DISABLE\n");	
			x=0;y=0;z=0;
			g_gui_hwt.compass.enable = false;
			g_gui_hwt.compass.int1 = false;
			g_gui_hwt.compass.int2 = false;
			g_gui_hwt.compass.x = 0;
			g_gui_hwt.compass.y = 0;
			g_gui_hwt.compass.z = 0;			
			gui_hwt_paint();
			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_COMPASS_DISABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));		
		}break;	
		case PC_UART_COMMAND_HARDWARE_TEST_HR_ENABLE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_HR_ENABLE\n");	
			g_gui_hwt.hr.enable = true;
			g_gui_hwt.hr.value = 0;
			gui_hwt_paint();
			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_HR_ENABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));		
		}break;
		case PC_UART_COMMAND_HARDWARE_TEST_HR_READ:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_HR_READ\n");	

			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_HR_READ_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));		
		}break;
		case PC_UART_COMMAND_HARDWARE_TEST_HR_DISABLE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_HR_DISABLE\n");	
			g_gui_hwt.hr.enable = false;
			g_gui_hwt.hr.value = 0;
			gui_hwt_paint();
			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_HR_DISABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));		
		}break;	
		case PC_UART_COMMAND_HARDWARE_TEST_GPS_ENABLE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_GPS_ENABLE\n");	
			g_gui_hwt.gps_enable = true;
			gui_hwt_paint();
			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_GPS_ENABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));		
		}break;
		case PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_HOT_START:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_HOT_START\n");	

			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_HOT_START_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));		
		}break;
		case PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_WARM_START:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_WARM_START\n");	

			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_WARM_START_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));		
		}break;
		case PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_COLD_START:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_COLD_START\n");	

			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_COLD_START_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));		
		}break;
		case PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_GPS_MODE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_GPS_MODE\n");	

			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_GPS_MODE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));		
		}break;
		case PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_BD_MODE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_BD_MODE\n");	

			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_BD_MODE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));		
		}break;
		case PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_GPS_BD_MODE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_GPS_BD_MODE\n");	

			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_GPS_BD_MODE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));		
		}break;
		case PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_NMEA_GGA:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_NMEA_GGA\n");	

			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_NMEA_GGA_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));		
		}break;
		case PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_NMEA_GSA:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_NMEA_GSA\n");	

			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_NMEA_GSA_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));		
		}break;
		case PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_NMEA_GSV:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_NMEA_GSV\n");	

			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_NMEA_GSV_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));		
		}break;
		case PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_NMEA_RMC:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_NMEA_RMC\n");	

			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_NMEA_RMC_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));		
		}break;
		case PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_NMEA_VTG:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_NMEA_VTG\n");	

			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_NMEA_VTG_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));		
		}break;
		case PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_NMEA_GLL:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_NMEA_GLL\n");	

			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_NMEA_GLL_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));		
		}break;
		case PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_NMEA_ZDA:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_NMEA_ZDA\n");	

			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_GPS_CONFIG_NMEA_ZDA_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));		
		}break;		
		case PC_UART_COMMAND_HARDWARE_TEST_GPS_READ:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_GPS_READ\n");	

			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_GPS_READ_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));		
		}break;		
		case PC_UART_COMMAND_HARDWARE_TEST_GPS_DISABLE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_GPS_DISABLE\n");	
			g_gui_hwt.gps_enable = false;
			gui_hwt_paint();
			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_GPS_DISABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));		
		}break;		
		case PC_UART_COMMAND_HARDWARE_TEST_BLE_ENABLE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_BLE_ENABLE\n");	
			if(!g_bluetooth_status.is_on)
			{
				dev_ble_system_on();
			}			
			g_gui_hwt.ble.enable = true;
			g_gui_hwt.ble.read_info = false;
			g_gui_hwt.ble.read_extFlash = false;			
			gui_hwt_paint();
			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_BLE_ENABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));		
		}break;		
		case PC_UART_COMMAND_HARDWARE_TEST_BLE_INFO_READ:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_BLE_INFO_READ\n");	
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
	
		}break;		
		case PC_UART_COMMAND_HARDWARE_TEST_BLE_EXTFLASH_READ:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_BLE_EXTFLASH_READ\n");	
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
	
		}break;			
		case PC_UART_COMMAND_HARDWARE_TEST_BLE_DISABLE:{
			LOG_PRINTF(MOUDLE_NAME"PC_UART_COMMAND_HARDWARE_TEST_BLE_DISABLE\n");	
			if(!g_bluetooth_status.is_on)
			{
				dev_ble_system_off();
			}				
			g_gui_hwt.ble.enable = false;
			g_gui_hwt.ble.read_info = false;
			g_gui_hwt.ble.read_extFlash = false;
			gui_hwt_paint();
			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_BLE_DISABLE_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));		
		}break;			
		
		
		
		
		
		default:{
			uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_UNSPPORT | PC_UART_COMMAND_MASK_HARDWARE_TEST};
			drv_pc_send_data((uint8_t *)command, sizeof(command));
		}break;
	}		
		


}





