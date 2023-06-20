#include "isr_config.h"
#include "isr_gpio.h"

#include "task_key.h"
#include "bsp.h"
#include "task_step.h"
#include "rtos.h"

#if DEBUG_ENABLED == 1 && ISR_GPIO_LOG_ENABLED == 1
	#define ISR_GPIO_LOG_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define ISR_GPIO_LOG_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define ISR_GPIO_LOG_WRITESTRING(...)
	#define ISR_GPIO_LOG_PRINTF(...)		        
#endif

static bool g_heart_pin_flag = false;//心率 D_AVAIL脚 显示心率有数据
/*设置心率串口中断标志*/
void SetHdrPinFlag(bool isEnable)
{
	g_heart_pin_flag = isEnable;
}
/*获取心率串口中断标志*/
bool GetHdrPinFlag(void)
{
	return g_heart_pin_flag;
}
//*****************************************************************************
//
// Interrupt handler for the GPIO pins.
//
//*****************************************************************************
void am_gpio_isr(void)
{	
	uint64_t ui64Status;
	ui64Status = am_hal_gpio_int_status_get(false);
	am_hal_gpio_int_clear(ui64Status);
	am_hal_gpio_int_service(ui64Status);	
}

