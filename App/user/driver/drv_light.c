#include "drv_config.h"
#include "drv_light.h"

#include "bsp.h"
#include "drv_lcd.h"
#include "timer_app.h"

#if DEBUG_ENABLED == 1 && DRV_LIGHT_LOG_ENABLED == 1
	#define DRV_LIGHT_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define DRV_LIGHT_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define DRV_LIGHT_WRITESTRING(...)
	#define DRV_LIGHT_PRINTF(...)		        
#endif

#define     BL_TIMER_NUMBER         3
#define     BL_TIMER_SEGMENT        AM_HAL_CTIMER_TIMERA
#define     BL_TIMER_INTERRUPT      AM_HAL_CTIMER_INT_TIMERA3
#define     BL_INTERVAL             1  //12K/1=12KHZ
#define     BL_PERIOD               20 //PWM周期1.2k

uint8_t backlight_level = 1;         //默认背光等级

extern uint8_t truly_lcd_flag;


uint8_t light_enable = 0;
extern SemaphoreHandle_t Appolo_SPI_Semaphore;
#if defined WATCH_SIM_LIGHTS_ON
extern uint8_t backlight_always_on;  //背光常亮标志
#endif

void drv_light_enable(void)
{
#ifdef WATCH_COD_BSP
#ifdef WATXCH_LCD_TRULY
		if (truly_lcd_flag)
		{
			backlight_level = 1;
			wait_lcd_light_te_high();
			drv_lcd_enable();
			drv_lcd_switch_lpm_to_hpm();
			light_enable = 1;
			drv_lcd_disable();
			wait_lcd_light_te_high();
				//Enable Backlight
			am_hal_gpio_pin_config(BSP_GPIO_BL_PWR,AM_HAL_PIN_48_TCTA3);
		
			//A3,频率为4kHz
			am_hal_ctimer_config_single(BL_TIMER_NUMBER, BL_TIMER_SEGMENT,
									(AM_HAL_CTIMER_FN_PWM_REPEAT |
									 AM_HAL_CTIMER_XT_32_768KHZ |
									 AM_HAL_CTIMER_PIN_ENABLE));
			am_hal_ctimer_period_set(BL_TIMER_NUMBER, BL_TIMER_SEGMENT, BL_PERIOD, backlight_level);
			am_hal_ctimer_start(BL_TIMER_NUMBER, BL_TIMER_SEGMENT);
		}
		else
		{

		
				//Enable Backlight
			am_hal_gpio_pin_config(BSP_GPIO_BL_PWR,AM_HAL_PIN_48_TCTA3);
		
			//A3,频率为4kHz
			am_hal_ctimer_config_single(BL_TIMER_NUMBER, BL_TIMER_SEGMENT,
									(AM_HAL_CTIMER_FN_PWM_REPEAT |
									 AM_HAL_CTIMER_XT_32_768KHZ |
									 AM_HAL_CTIMER_PIN_ENABLE));
			am_hal_ctimer_period_set(BL_TIMER_NUMBER, BL_TIMER_SEGMENT, BL_PERIOD, backlight_level);
			am_hal_ctimer_start(BL_TIMER_NUMBER, BL_TIMER_SEGMENT);
			drv_lcd_enable();
			drv_lcd_switch_lpm_to_hpm();
			light_enable = 1;
			drv_lcd_disable();
		}
	
		
#else

		//Enable Backlight
		am_hal_gpio_pin_config(BSP_GPIO_BL_PWR,AM_HAL_PIN_48_TCTA3);
		
		//A3,频率为4kHz
		am_hal_ctimer_config_single(BL_TIMER_NUMBER, BL_TIMER_SEGMENT,
									(AM_HAL_CTIMER_FN_PWM_REPEAT |
									 AM_HAL_CTIMER_XT_32_768KHZ |
									 AM_HAL_CTIMER_PIN_ENABLE));
		am_hal_ctimer_period_set(BL_TIMER_NUMBER, BL_TIMER_SEGMENT, BL_PERIOD, backlight_level);
		am_hal_ctimer_start(BL_TIMER_NUMBER, BL_TIMER_SEGMENT);
	
		drv_lcd_enable();
		drv_lcd_switch_lpm_to_hpm();
		light_enable = 1;
		drv_lcd_disable();
#endif
#else
	am_hal_gpio_pin_config(BSP_GPIO_BL_PWR,AM_HAL_GPIO_OUTPUT);
	am_hal_gpio_out_bit_set(BSP_GPIO_BL_PWR);
#endif
	

}

void drv_light_disable(void)
{
#if defined WATCH_SIM_LIGHTS_ON
	   backlight_always_on = 0;
#endif
#ifdef WATCH_COD_BSP
#ifdef WATXCH_LCD_TRULY
		//Dsiable Backlight
		//关闭定时器
		am_hal_ctimer_stop(BL_TIMER_NUMBER, BL_TIMER_SEGMENT);
		
		am_hal_gpio_pin_config(BSP_GPIO_BL_PWR,AM_HAL_GPIO_OUTPUT);
		am_hal_gpio_out_bit_clear(BSP_GPIO_BL_PWR);
		xSemaphoreTake( Appolo_SPI_Semaphore, portMAX_DELAY );
		light_enable = 0;
		xSemaphoreGive( Appolo_SPI_Semaphore ); 
		timer_lcd_start();
		
#else

		//Dsiable Backlight
		//关闭定时器
		am_hal_ctimer_stop(BL_TIMER_NUMBER, BL_TIMER_SEGMENT);
		
		am_hal_gpio_pin_config(BSP_GPIO_BL_PWR,AM_HAL_GPIO_OUTPUT);
		am_hal_gpio_out_bit_clear(BSP_GPIO_BL_PWR);
		xSemaphoreTake( Appolo_SPI_Semaphore, portMAX_DELAY );
		light_enable = 0;
		xSemaphoreGive( Appolo_SPI_Semaphore ); 
		timer_lcd_start();
		//
#endif
#else
am_hal_gpio_pin_config(BSP_GPIO_BL_PWR,AM_HAL_GPIO_OUTPUT);
am_hal_gpio_out_bit_clear(BSP_GPIO_BL_PWR);
#endif
}
