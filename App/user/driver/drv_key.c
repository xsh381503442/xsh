/************************************************************
  Copyright (C), 2016  L.T.F Comm. E&T Co., Ltd.
  FileName: Key.c
  Author:  曾德忠      Version :  1.0     Date:  2016.03.31
  Description:  按键处理函数    
  Version:  1.0
  Function List:   
    1. -------
  History:        
  <author>  <time>    <version >      <desc>
  曾德忠  2016/03/31     1.0     build this moudle  
***********************************************************/
#include "drv_config.h"
#include "drv_key.h"

#include "bsp.h"
#include "drv_lcd.h"
#include "gui_run.h"
#include "com_data.h"
#include "timers.h"
#include "time_notify.h"
#include "drv_lsm6dsl.h"

#if DEBUG_ENABLED == 1 && DRV_KEY_LOG_ENABLED == 1
	#define DRV_KEY_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define DRV_KEY_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define DRV_KEY_WRITESTRING(...)
	#define DRV_KEY_PRINTF(...)		        
#endif

static button_callback m_callback;

static uint8_t m_pin;
static uint32_t count = 0;

//static uint8_t xMaxExpiryCountBeforeStopping = 10;

extern wrist_tilt_cb_t wrist_tilt_cb;
extern TimerHandle_t wrist_tilt_timer;
extern SetValueStr SetValue;

#define BTN_RELEASE_TIMEOUT       10 // 10/512 约20ms
#define BTN_LONG_PUSH_COUNT       50	//长按超时次数
#define BTN_BACK_LONG_PUSH_COUNT  100	//back键长按超时次数

#if defined WATCH_UNLOCK_BACK_UP
#define BTN_NLOCK_BACK_UP_COUNT		5	//back+up解锁时间
#endif
#ifdef WATCH_COD_BSP
#define BTN_TIMER_NUMBER           2
#define BTN_TIMER_SEGMENT          AM_HAL_CTIMER_TIMERA
#define BTN_TIMER_INTERRUPT        AM_HAL_CTIMER_INT_TIMERA2
#else
#define BTN_TIMER_NUMBER           3
#define BTN_TIMER_SEGMENT          AM_HAL_CTIMER_TIMERA
#define BTN_TIMER_INTERRUPT        AM_HAL_CTIMER_INT_TIMERA3
#endif

//#define WRIST_TILT_RELEASE_TIMEOUT        500

//#define WRIST_TILT_TIMER_NUMBER           2
//#define WRIST_TILT_TIMER_SEGMENT          AM_HAL_CTIMER_TIMERA
//#define WRIST_TILT_TIMER_INTERRUPT        AM_HAL_CTIMER_INT_TIMERA2

static void timer_start(void)
{
	am_hal_ctimer_clear(BTN_TIMER_NUMBER, BTN_TIMER_SEGMENT);
	am_hal_ctimer_period_set(BTN_TIMER_NUMBER,BTN_TIMER_SEGMENT,BTN_RELEASE_TIMEOUT,0);	
	am_hal_ctimer_start(BTN_TIMER_NUMBER, BTN_TIMER_SEGMENT);
}

static void timer_stop(void)
{
	am_hal_ctimer_stop(BTN_TIMER_NUMBER, BTN_TIMER_SEGMENT);
}


static void timer_handle(void)
{	
	bool polarity;	
	polarity = am_hal_gpio_input_bit_read(m_pin);
	uint32_t long_conut = 0;
	if(polarity != 1)
	{
		count = 0;
		timer_stop();	
		m_callback(m_pin);
	}
	else{ 
		count++;

		if (m_pin == KEY_BACK)
		{
			long_conut = BTN_BACK_LONG_PUSH_COUNT;
		}
		else
		{
			long_conut = BTN_LONG_PUSH_COUNT;
		}
		
		if(count > long_conut)
		{
			count = 0;
			timer_stop();	
			m_callback(m_pin<<1);
		}
#if defined WATCH_UNLOCK_BACK_UP
		else if(count > BTN_NLOCK_BACK_UP_COUNT)
		{
			if(SetValue.AutoLockScreenFlag == 1 && 
				(ScreenState == DISPLAY_SCREEN_HOME || ScreenState == DISPLAY_SCREEN_LARGE_LOCK	))
			{
				uint64_t key_value = am_hal_gpio_input_read();
				#ifdef WATCH_COD_BSP
				if((key_value & KEY_MASK) == (AM_HAL_GPIO_BIT(KEY_OK) | AM_HAL_GPIO_BIT(KEY_DOWN)))
				#else
				if((key_value & KEY_MASK) == (AM_HAL_GPIO_BIT(KEY_UP) | AM_HAL_GPIO_BIT(KEY_BACK)))
				#endif
	
				{
					//back+up键解锁
					count = 0;
					timer_stop();
					m_callback(KEY_UNLOCK_BACK_UP);
				}
			}
		}
#endif		
	}	
}


static void timer_create(void)
{
	am_hal_ctimer_config_single(BTN_TIMER_NUMBER, BTN_TIMER_SEGMENT,
															(AM_HAL_CTIMER_INT_ENABLE |
															 AM_HAL_CTIMER_LFRC_512HZ |
															 AM_HAL_CTIMER_FN_REPEAT));

	am_hal_ctimer_int_register(BTN_TIMER_INTERRUPT, timer_handle);
	am_hal_ctimer_int_enable(BTN_TIMER_INTERRUPT);

}

static void btn_hdl_pwr(void)
{
	m_pin = KEY_BACK;
	count = 0;
	timer_start();
}
static void btn_hdl_bl(void)
{
	m_pin = KEY_LIGHT;
	count = 0;
	timer_start();
}
static void btn_hdl_up(void)
{
	m_pin = KEY_UP;
	count = 0;
	timer_start();
}
static void btn_hdl_ok(void)
{
	m_pin = KEY_OK;
	count = 0;
	timer_start();
}
static void btn_hdl_down(void)
{
	m_pin = KEY_DOWN;
	count = 0;
	timer_start();
}
#if !defined WATCH_RAISE_BRIGHT_SCREEN_ALGO
static void btn_hdl_awt(void)
{
    BaseType_t  xHigherPriorityTaskWoken= pdFALSE;
    DRV_KEY_PRINTF("AWT handler");
    if(system_raiselight_mode_get() == 1){
        if(wrist_tilt_cb.is_verify_timer_started != 1){
            wrist_tilt_cb.is_verify_timer_started = 1;
        }
        else{
            //reset timer and positive count
       //     wrist_tilt_cb.positive_count = 0;
            wrist_tilt_cb.lExpireCounters = 0;
        }
        xTimerChangePeriodFromISR(wrist_tilt_timer, 110/portTICK_PERIOD_MS, &xHigherPriorityTaskWoken);
    }
}
#endif
static void com_apollo2_gpio_int_init(uint32_t pin, uint32_t polarity)
{
	am_hal_gpio_pin_config(pin,AM_HAL_GPIO_INPUT);
	am_hal_gpio_int_polarity_bit_set(pin,polarity);
	am_hal_gpio_int_clear(AM_HAL_GPIO_BIT(pin));
	am_hal_gpio_int_enable(AM_HAL_GPIO_BIT(pin));	

}

void drv_key_init(button_callback task_cb)
{
	timer_create();

	m_callback = task_cb;
    
	am_hal_gpio_int_register(KEY_BACK, btn_hdl_pwr);
	am_hal_gpio_int_register(KEY_LIGHT, btn_hdl_bl);
	am_hal_gpio_int_register(KEY_UP, btn_hdl_up);
	am_hal_gpio_int_register(KEY_OK, btn_hdl_ok);
	am_hal_gpio_int_register(KEY_DOWN, btn_hdl_down);


    com_apollo2_gpio_int_init(KEY_BACK, 0);

	com_apollo2_gpio_int_init(KEY_LIGHT, 0);
	com_apollo2_gpio_int_init(KEY_UP, 0);
	com_apollo2_gpio_int_init(KEY_OK, 0);
	com_apollo2_gpio_int_init(KEY_DOWN, 0);

#if !defined WATCH_RAISE_BRIGHT_SCREEN_ALGO
    timer_wrist_tilt_init();
	am_hal_gpio_int_register(BSP_GPIO_MOTION_INT1, btn_hdl_awt);
	com_apollo2_gpio_int_init(BSP_GPIO_MOTION_INT1, 0);
#endif


}





