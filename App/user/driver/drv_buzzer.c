#include "drv_config.h"
#include "drv_buzzer.h"

#include "bsp.h"

#if DEBUG_ENABLED == 1 && DRV_BUZZER_LOG_ENABLED == 1
	#define DRV_BUZZER_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define DRV_BUZZER_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define DRV_BUZZER_WRITESTRING(...)
	#define DRV_BUZZER_PRINTF(...)		        
#endif

void drv_buzzer_enable(void)
{

	am_hal_gpio_pin_config(BSP_GPIO_BUZZER_PWR,BSP_GPIO_CFG_BUZZER_PWR);
	
	//B3,频率为4kHz
    am_hal_ctimer_config_single(BSP_BUZZER_CTIMER_NUM, BSP_BUZZER_CTIMER_SEGMENT,
                                (AM_HAL_CTIMER_FN_PWM_REPEAT |
                                 AM_HAL_CTIMER_XT_32_768KHZ |
                                 AM_HAL_CTIMER_PIN_ENABLE));
    am_hal_ctimer_period_set(BSP_BUZZER_CTIMER_NUM, BSP_BUZZER_CTIMER_SEGMENT, 6, 3);
    am_hal_ctimer_start(BSP_BUZZER_CTIMER_NUM, BSP_BUZZER_CTIMER_SEGMENT);

}

void drv_buzzer_disable(void)
{
	//关闭定时器
	am_hal_ctimer_stop(BSP_BUZZER_CTIMER_NUM, BSP_BUZZER_CTIMER_SEGMENT);
	
	am_hal_gpio_pin_config(BSP_GPIO_BUZZER_PWR,AM_HAL_GPIO_OUTPUT);
	am_hal_gpio_out_bit_clear(BSP_GPIO_BUZZER_PWR);
}
