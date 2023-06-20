#include "main.h"
#include "app_config.h"
#include "bsp.h"
#include "rtos.h"

#include "drv_config.h"
#include "drv_extFlash.h"
#include "drv_gt24l24.h"
#include "drv_key.h"
#include "drv_motor.h"

#include "task_display.h"
#include "task_config.h"
#include "task_ble.h"

#include "mode_power_on.h"

#include "com_data.h"
#include "com_dial.h"
#include "drv_battery.h"

#include "gui_time.h"




#if DEBUG_ENABLED == 1 && MAIN_LOG_ENABLED == 1
	#define MAIN_LOG_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	
	#define MAIN_LOG_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else       
	#define MAIN_LOG_WRITESTRING(...)
	#define MAIN_LOG_PRINTF(...)		        
#endif
extern SetValueStr SetValue;
static void rtt_init(void)
{
#if DEBUG_ENABLED == 1
	SEGGER_RTT_Init();
#endif
	
	MAIN_LOG_PRINTF("[main]:##### rtt init #####\r\n");
}

static void reset_handle(bool *power_on)
{
	uint32_t ui32ResetStatus = am_hal_reset_status_get();
	am_hal_reset_status_clear();

	if (ui32ResetStatus & AM_HAL_RESET_STAT_POWER_CYCLE)
	{
		
	}	
	if (ui32ResetStatus & AM_HAL_RESET_STAT_WDT)
	{
		*power_on = true;
		return;
	}
	if (ui32ResetStatus & AM_HAL_RESET_STAT_DEBUG)
	{
		
	}	
	if (ui32ResetStatus & AM_HAL_RESET_STAT_POI)
	{
		*power_on = true;
		return;		
	}	
	if (ui32ResetStatus & AM_HAL_RESET_STAT_SOFTWARE)
	{
		*power_on = true;
		return;
	}	
	if (ui32ResetStatus & AM_HAL_RESET_STAT_BOD)
	{
	}
	if (ui32ResetStatus & AM_HAL_RESET_STAT_EXTERNAL)
	{
	}	

	
	am_hal_gpio_pin_config(KEY_BACK,AM_HAL_GPIO_INPUT);
	if(am_hal_gpio_input_bit_read(KEY_BACK))
	{
		*power_on = true;
		return;
	}
	
	am_hal_gpio_pin_config(DRV_BAT_USB_DETECT_PIN,AM_HAL_GPIO_INPUT);
   
    if(am_hal_gpio_input_bit_read(DRV_BAT_USB_DETECT_PIN))
	{
		*power_on = true;
		return;
	}	

}

static void system_init(void)
{
	am_hal_clkgen_sysclk_select(AM_HAL_CLKGEN_SYSCLK_48MHZ);
	am_hal_pwrctrl_low_power_init();	
	am_hal_pwrctrl_bucks_init();
//	am_hal_pwrctrl_bucks_disable();
	am_hal_cachectrl_enable(&am_hal_cachectrl_defaults);
	am_hal_pwrctrl_memory_enable(AM_HAL_PWRCTRL_MEMEN_FLASH1M);
	am_hal_pwrctrl_memory_enable(AM_HAL_PWRCTRL_MEMEN_SRAM256K);	
	am_hal_rtc_alarm_interval_set(AM_HAL_RTC_ALM_RPT_MIN);
	am_hal_rtc_int_clear(AM_HAL_RTC_INT_ALM);
	am_hal_rtc_int_enable(AM_HAL_RTC_INT_ALM);
	AM_BFW(CLKGEN,OCTRL,FOS,0);	
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

static void system_interrupt(void)
{
	am_hal_interrupt_enable(AM_HAL_INTERRUPT_CTIMER);
	am_hal_interrupt_enable(AM_HAL_INTERRUPT_GPIO);
	am_hal_interrupt_enable(AM_HAL_INTERRUPT_ADC);
	am_hal_interrupt_enable(AM_HAL_INTERRUPT_CLKGEN);
//	am_hal_interrupt_enable(AM_HAL_INTERRUPT_WATCHDOG);
	am_hal_interrupt_master_enable();	
}

int main(void)
{
	bool power_on = false;
#ifndef WATCH_HAS_NO_BLE 	
	am_hal_gpio_out_bit_set(DEV_BLE_SYS_OFF_PIN);
	am_hal_gpio_pin_config(DEV_BLE_SYS_OFF_PIN,AM_HAL_GPIO_OUTPUT);	//蓝牙先关，再开，防止升级后蓝牙在UPDATE和BOOTLOADER中错误打开
#endif	
	
	rtt_init();
	reset_handle(&power_on);

	system_init();
	priority_init();
	system_interrupt();	
	semaphore_create();
	
	bsp_init();	
	/* CLK输出*/
	//am_hal_gpio_pin_config(BSP_GPIO_IOM4_I2C_SCL,AM_HAL_PIN_39_CLKOUT); 
	//am_hal_clkgen_clkout_enable(AM_HAL_CLKGEN_CLKOUT_CKSEL_XT);
	


	if(power_on)
	{	
		drv_motor_enable();		
	}
	dev_extFlash_enter_deep_sleep();
	com_data_init(power_on);
	SetValue.SwBle = BLE_DISCONNECT;
	com_dial_init();
		
	if(power_on)
	{
		ScreenState = DISPLAY_SCREEN_HOME;	
		drv_motor_disable();
	}
	else
	{
		ScreenState = DISPLAY_SCREEN_LOGO;
	}

	run_tasks();
	
	while (1)
	{
		
	}
}
