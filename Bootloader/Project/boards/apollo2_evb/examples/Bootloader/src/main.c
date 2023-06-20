#include "main.h"
#include "rtt_log.h"
#include "lib_error.h"
#include "SEGGER_RTT.h"


#include "am_mcu_apollo.h"
#include "am_util_delay.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "portmacro.h"
#include "portable.h"
#include "event_groups.h"
#include "semphr.h"


#include "lib_boot_setting.h"

#include "task_setup.h"
#include "drv_button.h"
#include "drv_extFlash.h"

#include "dev_dfu.h"
#include <string.h>

#include "lib_boot.h"

#include "com_apollo2.h"
#include "dev_lcd.h"
#include "drv_ble.h"
#include "dev_ble.h"
#include "drv_rtc.h"
#include "task_ble.h"
#ifdef LAYER_APPLICATION 		
#include "gui_util.h"
#include "dev_gps.h"
#include "dev_notify.h"
#include "dev_heartrate.h"
#include "dev_font.h"
#endif

#include "task_charge.h"
#include "dev_bat.h"
#include "drv_lcd.h"
#include "dev_extFlash.h"

#include "task_display.h"

#include "board.h"
#include "dev_dfu_ble.h"

#define MOUDLE_LOG_ENABLED 							1
#define MOUDLE_DEBUG_ENABLED   					1
//#define BACK_DOOR_FLAG					//开机进入后门
#ifdef LAYER_BOOTLOADER  
#define MOUDLE_NAME                     "[BOOT MAIN]:"
#endif
#ifdef LAYER_APPLICATION  
#define MOUDLE_NAME                     "[APP MAIN]:"
#endif
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


extern ScreenState_t   ScreenState;

bool g_is_ble_dfu = false;
ScreenState_exception g_screen_state_exception_type = SCREEN_STATE_EXCEPTION_TYPE_NONE;

#ifdef LAYER_BOOTLOADER  
static void boot(void)
{
	_boot_setting boot_setting;
	lib_boot_setting_read(&boot_setting);
	if(lib_boot_setting_crc_check_bootsetting(&boot_setting))
	{
		
		am_bootloader_image_t image;
		
		if(boot_setting.enter_bootloader_reason == BOOT_SETTING_ENTER_BOOT_REASON_JUMP_APPLICATION)
		{
			DEBUG_PRINTF(MOUDLE_NAME"BOOT_SETTING_ENTER_BOOT_REASON_JUMP_APPLICATION\n");
			
			if(boot_setting.bank_application.intFlash_address >= APPLICATION_START_DEFAULT_ADDRESS && 
				 lib_boot_setting_crc_check_application(&boot_setting))
			{
				image.pui32LinkAddress = (uint32_t *)boot_setting.bank_application.intFlash_address;
				DEBUG_PRINTF(MOUDLE_NAME"address --> %08X\n",boot_setting.bank_application.intFlash_address);
				DEBUG_PRINTF(MOUDLE_NAME"size --> %d\n",boot_setting.bank_application.image_size);
				lib_boot_goto_image(&image);					
			}
			else
			{
				if(boot_setting.bank_application.intFlash_address >= APPLICATION_START_DEFAULT_ADDRESS)
				{
					g_screen_state_exception_type = SCREEN_STATE_EXCEPTION_TYPE_APPLICATION_START_ADDRESS_ERROR;
				}
				else{
				
					g_screen_state_exception_type = SCREEN_STATE_EXCEPTION_TYPE_APPLICATION_CRC_ERROR;
				}
				
				ScreenState = DISPLAY_SCREEN_EXCEPTION;	
				lib_boot_setting_init();
				//Application must handle either address is too small that overrides the bootloader region ,
				//or application image that is corrupte due to its crc mismatch.
				DEBUG_PRINTF(MOUDLE_NAME"APPLICATION CRC MISMATCH\r\n");
				DEBUG_PRINTF(MOUDLE_NAME"Application must handle either address is too small that overrides the bootloader region");
				DEBUG_PRINTF(MOUDLE_NAME"or application image that is corrupte due to its crc mismatch.\r\n");
			}

		}	
		else if(boot_setting.enter_bootloader_reason == BOOT_SETTING_ENTER_BOOT_REASON_JUMP_UPDATE_ALGORITHM)
		{
			DEBUG_PRINTF(MOUDLE_NAME"BOOT_SETTING_ENTER_BOOT_REASON_JUMP_UPDATE_ALGORITHM\n");
			
			if(boot_setting.bank_update_algorithm.intFlash_address >= UPDATE_ALGORITHM_START_DEFAULT_ADDRESS && 
				 lib_boot_setting_crc_check_updateAlgorithm(&boot_setting))
			{
				image.pui32LinkAddress = (uint32_t *)boot_setting.bank_update_algorithm.intFlash_address;
				DEBUG_PRINTF(MOUDLE_NAME"address --> %08X\n",boot_setting.bank_update_algorithm.intFlash_address);
				DEBUG_PRINTF(MOUDLE_NAME"size --> %d\n",boot_setting.bank_update_algorithm.image_size);
				lib_boot_goto_image(&image);					
			}
			else
			{
				if(boot_setting.bank_update_algorithm.intFlash_address >= UPDATE_ALGORITHM_START_DEFAULT_ADDRESS)
				{
					g_screen_state_exception_type = SCREEN_STATE_EXCEPTION_TYPE_UPDATE_ALGORITHM_START_ADDRESS_ERROR;
				}
				else{
				
					g_screen_state_exception_type = SCREEN_STATE_EXCEPTION_TYPE_UPDATE_ALGORITHM_CRC_ERROR;
				}
								
				ScreenState = DISPLAY_SCREEN_EXCEPTION;	
				lib_boot_setting_init();
				//Application must handle either address is too small that overrides the bootloader region ,
				//or application image that is corrupte due to its crc mismatch.
				DEBUG_PRINTF(MOUDLE_NAME"UPDATE ALGORITHM CRC MISMATCH\r\n");
				DEBUG_PRINTF(MOUDLE_NAME"Application must handle either address is too small that overrides the bootloader region");
				DEBUG_PRINTF(MOUDLE_NAME"or application image that is corrupte due to its crc mismatch.\r\n");				
			}			
			
//			image.pui32LinkAddress = (uint32_t *)boot_setting.bank_update_algorithm.intFlash_address;
//			DEBUG_PRINTF(MOUDLE_NAME"address --> 0x%08X\n",boot_setting.bank_update_algorithm.intFlash_address);
//			DEBUG_PRINTF(MOUDLE_NAME"size --> %d\n",boot_setting.bank_update_algorithm.image_size);
//			lib_boot_goto_image(&image);
		}	
		else if(boot_setting.enter_bootloader_reason == BOOT_SETTING_ENTER_BOOT_REASON_DFU_PC)
		{
			DEBUG_PRINTF(MOUDLE_NAME"BOOT_SETTING_ENTER_BOOT_REASON_DFU_PC\n");
			ScreenState = DISPLAY_SCREEN_BOOTLOADER;
		}	
		else if(boot_setting.enter_bootloader_reason == BOOT_SETTING_ENTER_BOOT_REASON_DFU_BLE)
		{
			DEBUG_PRINTF(MOUDLE_NAME"BOOT_SETTING_ENTER_BOOT_REASON_DFU_BLE\n");
			PIN_SET(DEV_BLE_SYS_OFF_PIN);
			PIN_CONFIG(DEV_BLE_SYS_OFF_PIN,AM_HAL_GPIO_OUTPUT);	
			g_is_ble_dfu = true;
			
		}		
	

		boot_setting.enter_bootloader_reason = BOOT_SETTING_ENTER_BOOT_REASON_JUMP_APPLICATION;
		boot_setting.boot_setting_crc = am_bootloader_fast_crc32(&boot_setting.boot_setting_size,boot_setting.boot_setting_size-4);
		lib_boot_setting_write(&boot_setting);		
		
	}
	else
	{	
		g_screen_state_exception_type = SCREEN_STATE_EXCEPTION_TYPE_BOOTSETTING_CRC_ERROR;
		ScreenState = DISPLAY_SCREEN_EXCEPTION;
		lib_boot_setting_init();
	}
}

#endif

static void log_init(void)
{
	#if RTT_LOG_ENABLED == 1
	SEGGER_RTT_Init();
	#endif
}

static void system_init(void)
{
	am_hal_clkgen_sysclk_select(AM_HAL_CLKGEN_SYSCLK_48MHZ);
//	am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_ADC|
//																AM_HAL_PWRCTRL_IOM0|
//																AM_HAL_PWRCTRL_IOM1|
//																AM_HAL_PWRCTRL_IOM2|
//																AM_HAL_PWRCTRL_IOM3|
//																AM_HAL_PWRCTRL_IOM4|
//																AM_HAL_PWRCTRL_IOM5|
//																AM_HAL_PWRCTRL_IOS|
//																AM_HAL_PWRCTRL_PDM|
//																AM_HAL_PWRCTRL_UART0|
//																AM_HAL_PWRCTRL_UART1);
//	am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_ADC|
//																AM_HAL_PWRCTRL_IOM0|
//																AM_HAL_PWRCTRL_IOM1|
//																AM_HAL_PWRCTRL_IOM2|
//																AM_HAL_PWRCTRL_IOM3|
//																AM_HAL_PWRCTRL_IOM4|
//																AM_HAL_PWRCTRL_IOM5|
//																AM_HAL_PWRCTRL_IOS|
//																AM_HAL_PWRCTRL_PDM|
//																AM_HAL_PWRCTRL_UART0|
//																AM_HAL_PWRCTRL_UART1);


//	am_hal_pwrctrl_low_power_init();	
	am_hal_pwrctrl_bucks_init();
	am_hal_pwrctrl_bucks_enable();
//	am_hal_cachectrl_enable(&am_hal_cachectrl_defaults);
	am_hal_pwrctrl_memory_enable(AM_HAL_PWRCTRL_MEMEN_FLASH1M);
	am_hal_pwrctrl_memory_enable(AM_HAL_PWRCTRL_MEMEN_SRAM256K);
	am_hal_rtc_osc_select(AM_HAL_RTC_OSC_XT);
	am_hal_clkgen_osc_start(AM_HAL_CLKGEN_OSC_XT);
	am_hal_rtc_osc_enable();	
}


static void priority_init(void)
{
	am_hal_interrupt_priority_set(AM_HAL_INTERRUPT_GPIO, configMAX_SYSCALL_INTERRUPT_PRIORITY);
	am_hal_interrupt_priority_set(AM_HAL_INTERRUPT_UART1,configMAX_SYSCALL_INTERRUPT_PRIORITY);
	am_hal_interrupt_priority_set(AM_HAL_INTERRUPT_UART,configMAX_SYSCALL_INTERRUPT_PRIORITY);
	am_hal_interrupt_priority_set(AM_HAL_INTERRUPT_ADC, configMAX_SYSCALL_INTERRUPT_PRIORITY);
	am_hal_interrupt_priority_set(AM_HAL_INTERRUPT_CTIMER, configMAX_SYSCALL_INTERRUPT_PRIORITY);
	am_hal_interrupt_priority_set(AM_HAL_INTERRUPT_CLKGEN, configMAX_SYSCALL_INTERRUPT_PRIORITY);
}



static void device_init(void)
{
		//将SWDIO和SWCLK内部上拉
	PIN_CONFIG(BSP_APOLLO2_SWDIO, ((AM_HAL_GPIO_FUNC(0) | AM_HAL_PIN_DIR_INPUT)|AM_HAL_GPIO_PULLUP));
	PIN_CONFIG(BSP_APOLLO2_SWCLK, ((AM_HAL_GPIO_FUNC(0) | AM_HAL_PIN_DIR_INPUT)|AM_HAL_GPIO_PULLUP));		

	PIN_CONFIG(BSP_APOLLO2_EXTFLASH_CLK, AM_HAL_GPIO_INPUT|AM_HAL_GPIO_PULLUP);
	PIN_CONFIG(BSP_APOLLO2_EXTFLASH_MISO, AM_HAL_GPIO_INPUT|AM_HAL_GPIO_PULLUP);
	PIN_CONFIG(BSP_APOLLO2_EXTFLASH_MOSI, AM_HAL_GPIO_INPUT|AM_HAL_GPIO_PULLUP);
	PIN_CONFIG(BSP_APOLLO2_EXTFLASH_CS,AM_HAL_GPIO_INPUT | AM_HAL_GPIO_PULLUP);
	PIN_CONFIG(BSP_APOLLO2_FONT_SPI2_SS_PIN,AM_HAL_GPIO_INPUT | AM_HAL_GPIO_PULLUP);
		
	

	#ifdef LAYER_APPLICATION 
	PIN_CONFIG(EXTFLASH_READY_PIN,AM_HAL_PIN_INPUT | AM_HAL_GPIO_PULLUP);
	
	//心率 ENABLE脚拉低
	am_hal_gpio_pin_config(46,AM_HAL_PIN_OUTPUT);
	am_hal_gpio_out_bit_clear(46);//如果悬空32K晶振会有电流串到主控，关闭心率后需要将该脚拉低	
	
	#endif
	
	dev_ble_init();
	dev_lcd_init();
	drv_rtc_init(false);
	
	#ifdef LAYER_APPLICATION 
	dev_extFlash_enter_deep_sleep();
	dev_font_enter_deep_sleep();
	#endif	

	#ifdef LAYER_BOOTLOADER
	dev_extFlash_exit_deep_sleep();
	#endif		
	
	
	

	
}

static void system_interrupt(void)
{
	am_hal_interrupt_enable(AM_HAL_INTERRUPT_CTIMER);		
	am_hal_interrupt_enable(AM_HAL_INTERRUPT_GPIO);	
	am_hal_interrupt_enable(AM_HAL_INTERRUPT_ADC);
//	am_hal_interrupt_enable(AM_HAL_INTERRUPT_CLKGEN);
//	am_hal_interrupt_enable(AM_HAL_INTERRUPT_WATCHDOG);
	am_hal_interrupt_master_enable();	
}



static void reset_handler(void)
{
	PIN_CONFIG(TASK_CHARGE_USB_DETECT_PIN,AM_HAL_GPIO_INPUT);
	if(PIN_READ_INPUT(TASK_CHARGE_USB_DETECT_PIN))
	{
		DEBUG_PRINTF(MOUDLE_NAME"DEV_BAT_EVENT_USB_CONN_CHARGING\n");
		g_bat_evt.cable_status = DEV_BAT_EVENT_USB_CONN_CHARGING;
		g_bat_evt.usb_detect_pin_polarity = USB_DETECT_PIN_DISCONNECT_POLARITY;
	}
	else{
		DEBUG_PRINTF(MOUDLE_NAME"DEV_BAT_EVENT_USB_DISCONN\n");
		g_bat_evt.cable_status = DEV_BAT_EVENT_USB_DISCONN;
		g_bat_evt.usb_detect_pin_polarity = USB_DETECT_PIN_CONNECT_POLARITY;
	}
	
}

int main(void)
{
	#ifdef LAYER_BOOTLOADER  
	DEBUG_PRINTF("\n\n"MOUDLE_NAME"bootloader start...   2\n");
	log_init();
#if defined BACK_DOOR_FLAG //开机进入后门
	PIN_CONFIG(BTN_PIN_UP,AM_HAL_GPIO_INPUT);
	if(PIN_READ_INPUT(BTN_PIN_UP))
	{
		/**<Special back door(recovery) mode for entering bootloader directly>*/
		_boot_setting boot_setting;
		lib_boot_setting_read(&boot_setting);		
		boot_setting.enter_bootloader_reason = BOOT_SETTING_ENTER_BOOT_REASON_JUMP_APPLICATION;
		boot_setting.boot_setting_crc = am_bootloader_fast_crc32(&boot_setting.boot_setting_size,boot_setting.boot_setting_size-4);
		lib_boot_setting_write(&boot_setting);
		ScreenState = DISPLAY_SCREEN_BACK_DOOR;
	}else{

		boot();	
	}
#else
		boot();	
#endif 


//	{
//		am_bootloader_image_t image;
//		image.pui32LinkAddress = (uint32_t *)APPLICATION_START_DEFAULT_ADDRESS;
//		lib_boot_goto_image(&image);	
//	}
	
	#endif
	#ifdef LAYER_APPLICATION 
	log_init();
	DEBUG_PRINTF("\n\n"MOUDLE_NAME"application start...   2\n");
	_boot_setting boot_setting;
	lib_boot_setting_read(&boot_setting);	
	#endif	
	

	reset_handler();
	system_init();
	device_init();
	priority_init();
	system_interrupt();	
	
	
	

	
	task_setup_init();
	
	while (1)
	{
		ERR_HANDLER(ERR_FORBIDDEN);
	}

}
