#include "drv_config.h"
#include "drv_motor.h"
#include "task_step.h"

#include "bsp.h"

uint8_t motorflag = 0;

#if DEBUG_ENABLED == 1 && DRV_MOTOR_LOG_ENABLED == 1
	#define DRV_MOTOR_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define DRV_MOTOR_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define DRV_MOTOR_WRITESTRING(...)
	#define DRV_MOTOR_PRINTF(...)		        
#endif

void drv_motor_enable(void)
{

	am_hal_gpio_pin_config(BSP_GPIO_MOTOR_PWR,AM_HAL_GPIO_OUTPUT);
	am_hal_gpio_out_bit_set(BSP_GPIO_MOTOR_PWR);

	motorflag = 1;

}

void drv_motor_disable(void)
{

	//am_hal_gpio_pin_config(BSP_GPIO_MOTOR_PWR,AM_HAL_GPIO_OUTPUT);
	//am_hal_gpio_out_bit_clear(BSP_GPIO_MOTOR_PWR);
    am_hal_gpio_pin_config(BSP_GPIO_MOTOR_PWR, AM_HAL_PIN_DISABLE);
	motorflag = 0;
#if defined (WATCH_STEP_ALGO)
    move_int = 1;
#else
	move_int = 1;
#endif

}

void drv_motor_toggle(void)
{

	am_hal_gpio_out_bit_toggle(BSP_GPIO_MOTOR_PWR);

}
