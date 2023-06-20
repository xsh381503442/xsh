#include "main.h"
#include "rtt_log.h"
#include "lib_error.h"
#include "SEGGER_RTT.h"
#include <string.h>

#include "am_mcu_apollo.h"
#include "com_apollo2.h"
#include "lib_scheduler.h"
#include "sched_button.h"
#include "sched_pc.h"
#include "lib_boot.h"
#include "dev_dfu.h"
#include "drv_pc.h"
#include "drv_extFlash.h"
#include "cmd_pc.h"

#include "board.h"
#include "dev_extFlash.h"

#define MOUDLE_LOG_ENABLED 							1
#define MOUDLE_DEBUG_ENABLED   					1
#define MOUDLE_NAME                     "[MAIN]:"

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

#define SCHED_MAX_EVENT_DATA_SIZE  (128)/**<uncertain about schedule size, so abitrarily set to 512>*/
#define SCHED_QUEUE_SIZE                 20                                             /**< Maximum number of events in the scheduler queue. */
#define configKERNEL_INTERRUPT_PRIORITY         (0x7 << 5)
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    (0x4 << 5)



static void log_init(void)
{
	#if RTT_LOG_ENABLED == 1
	SEGGER_RTT_Init();
	#endif
}
static void system_init(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"system_init\n");
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
	am_hal_pwrctrl_low_power_init();
	am_hal_pwrctrl_memory_enable(AM_HAL_PWRCTRL_MEMEN_FLASH1M);
	am_hal_pwrctrl_memory_enable(AM_HAL_PWRCTRL_MEMEN_SRAM256K);	
}
static void timer_init(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"timer_init\n");
}

static void scheduler_init(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"scheduler_init\n");
  APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
	
	
//	sched_button_init();
//	sched_pc_init();
}


static void device_init(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"device_init\n");
		//将SWDIO和SWCLK内部上拉
	PIN_CONFIG(BSP_APOLLO2_SWDIO, ((AM_HAL_GPIO_FUNC(0) | AM_HAL_PIN_DIR_INPUT)|AM_HAL_GPIO_PULLUP));
	PIN_CONFIG(BSP_APOLLO2_SWCLK, ((AM_HAL_GPIO_FUNC(0) | AM_HAL_PIN_DIR_INPUT)|AM_HAL_GPIO_PULLUP));	
	
	
	PIN_CONFIG(BSP_APOLLO2_EXTFLASH_CLK, AM_HAL_GPIO_INPUT|AM_HAL_GPIO_PULLUP);
	PIN_CONFIG(BSP_APOLLO2_EXTFLASH_MISO, AM_HAL_GPIO_INPUT|AM_HAL_GPIO_PULLUP);
	PIN_CONFIG(BSP_APOLLO2_EXTFLASH_MOSI, AM_HAL_GPIO_INPUT|AM_HAL_GPIO_PULLUP);
	PIN_CONFIG(BSP_APOLLO2_EXTFLASH_CS,AM_HAL_GPIO_INPUT | AM_HAL_GPIO_PULLUP);
	PIN_CONFIG(BSP_APOLLO2_FONT_SPI2_SS_PIN,AM_HAL_GPIO_INPUT | AM_HAL_GPIO_PULLUP);
	
	dev_extFlash_exit_deep_sleep();
}


static void priority_init(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"priority_init\n");
//	am_hal_interrupt_priority_set(AM_HAL_INTERRUPT_GPIO, configMAX_SYSCALL_INTERRUPT_PRIORITY);
//	am_hal_interrupt_priority_set(AM_HAL_INTERRUPT_CTIMER, configKERNEL_INTERRUPT_PRIORITY);
//	am_hal_interrupt_priority_set(AM_HAL_INTERRUPT_UART,configKERNEL_INTERRUPT_PRIORITY);
}



static void system_interrupt(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"system_interrupt\n");
//	am_hal_interrupt_enable(AM_HAL_INTERRUPT_CTIMER);	
//	am_hal_interrupt_enable(AM_HAL_INTERRUPT_GPIO);	
//	am_hal_interrupt_master_enable();	
}


static void lcd_gpio_init(void)
{
#ifndef WATCH_COD_BSP
	PIN_CONFIG(BSP_APOLLO2_GPIO_LCD_VDD1,AM_HAL_GPIO_OUTPUT);
	PIN_CONFIG(BSP_APOLLO2_GPIO_LCD_VDD2,AM_HAL_GPIO_OUTPUT);	

	PIN_CLEAR(BSP_APOLLO2_GPIO_LCD_VDD1);
	PIN_CLEAR(BSP_APOLLO2_GPIO_LCD_VDD2);
#endif
}

int main(void)
{
	log_init();
	DEBUG_PRINTF("\n\n"MOUDLE_NAME"update algorithm start...\n");
	
	am_hal_gpio_out_bit_set(DEV_BLE_SYS_OFF_PIN);
	am_hal_gpio_pin_config(DEV_BLE_SYS_OFF_PIN,AM_HAL_GPIO_OUTPUT);	//蓝牙先关，再开，防止升级后蓝牙在UPDATE和BOOTLOADER中错误打开
	
	
	lcd_gpio_init();
	
	_boot_setting boot_setting;
	lib_boot_setting_read(&boot_setting);
	if(lib_boot_setting_crc_check_bootsetting(&boot_setting))
	{
		if(boot_setting.bank_bootloader.command == BOOT_SETTING_BANK_COMMAND_NEW)
		{
			DEBUG_PRINTF(MOUDLE_NAME"bank_bootloader BOOT_SETTING_BANK_COMMAND_NEW\n");	
		}	
		if(boot_setting.bank_application.command == BOOT_SETTING_BANK_COMMAND_NEW)
		{
			DEBUG_PRINTF(MOUDLE_NAME"bank_application BOOT_SETTING_BANK_COMMAND_NEW\n");
		}	
	}		
	
	
	
	system_init();
	timer_init();
	device_init();
	scheduler_init();	
	device_init();
	priority_init();
	system_interrupt();	
		
	
	
	
	
//	_info_apollo2 info_apollo2;
//	info_apollo2.command = PC_UART_RESPONSE_UPDATE_ALGORITHM_INFO_APOLLO2 | PC_UART_COMMAND_MASK_UPDATE_ALGORITHM;
//	info_apollo2.bootloader_version = boot_setting.bootloader_version;
//	info_apollo2.application_version = boot_setting.application_version;
//	info_apollo2.update_version = boot_setting.update_version;
//	info_apollo2.pcb_version = boot_setting.pcb_version;
//	drv_pc_send_data((uint8_t *)&info_apollo2, sizeof(_info_apollo2));	
	
	
	
	
	
	
	if(dev_dfu_update_algorithm_start())
	{
		lib_boot_jump_to_bootloader(BOOT_SETTING_ENTER_BOOT_REASON_JUMP_APPLICATION);
	}
	
	
		
	
	
	while (1)
	{
		
		app_sched_execute();
		am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
	}

}
