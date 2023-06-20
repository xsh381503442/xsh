#include "drv_button.h"
#include "rtt_log.h"
#include "lib_error.h"
#include "SEGGER_RTT.h"

#include "am_mcu_apollo.h"


#include "com_apollo2.h"


#define MOUDLE_LOG_ENABLED 							0
#define MOUDLE_DEBUG_ENABLED   					0
#define MOUDLE_NAME                     "[DRV_BUTTON]:"

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



static struct {
	bool initialized;
	button_callback callback;

}m_instance = {.initialized = false, .callback = NULL};


static uint8_t m_pin;
static bool m_polarity = 1;
static uint32_t count = 0;

#define BTN_RELEASE_TIMEOUT       10
#define BTN_LONG_PUSH_COUNT       40
#ifdef WATCH_COD_BSP
#define BTN_TIMER_NUMBER           2
#define BTN_TIMER_SEGMENT          AM_HAL_CTIMER_TIMERA
#define BTN_TIMER_INTERRUPT        AM_HAL_CTIMER_INT_TIMERA2
#else
#define BTN_TIMER_NUMBER           3
#define BTN_TIMER_SEGMENT          AM_HAL_CTIMER_TIMERA
#define BTN_TIMER_INTERRUPT        AM_HAL_CTIMER_INT_TIMERA3
#endif






static void timer_start(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"timer_start\n");
	am_hal_ctimer_clear(BTN_TIMER_NUMBER, BTN_TIMER_SEGMENT);
	am_hal_ctimer_period_set(BTN_TIMER_NUMBER,BTN_TIMER_SEGMENT,BTN_RELEASE_TIMEOUT,0);	
	am_hal_ctimer_start(BTN_TIMER_NUMBER, BTN_TIMER_SEGMENT);
}

static void timer_stop(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"timer_stop\n");
	am_hal_ctimer_stop(BTN_TIMER_NUMBER, BTN_TIMER_SEGMENT);
}


static void timer_handle(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"timer_handle\n");
	
	bool polarity;	
	polarity = PIN_READ_INPUT(m_pin);
	
	if(polarity != m_polarity)
	{
		LOG_PRINTF(MOUDLE_NAME"release button --> %d\n",m_pin);
		count = 0;
		timer_stop();	
		m_instance.callback(m_pin);
	}
	else{
		count++;
		if(count > BTN_LONG_PUSH_COUNT)
		{
			LOG_PRINTF(MOUDLE_NAME"long push button --> %d\n",m_pin);
			count = 0;
			timer_stop();	
			m_instance.callback(m_pin<<1);
		}
	}	
}


static void btn_hdl_pwr(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"btn_hdl_pwr\n");
	m_pin = BTN_PIN_PWR;
	count = 0;
	timer_start();
}
static void btn_hdl_bl(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"btn_hdl_pwr\n");
	m_pin = BTN_PIN_BL;
	count = 0;
	timer_start();
}
static void btn_hdl_up(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"btn_hdl_pwr\n");
	m_pin = BTN_PIN_UP;
	count = 0;
	timer_start();
}
static void btn_hdl_ok(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"btn_hdl_pwr\n");
	m_pin = BTN_PIN_OK;
	count = 0;
	timer_start();
}
static void btn_hdl_down(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"btn_hdl_pwr\n");
	m_pin = BTN_PIN_DOWN;
	count = 0;
	timer_start();
}


static void timer_create(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"timer_create\n");
	am_hal_ctimer_config_single(BTN_TIMER_NUMBER, BTN_TIMER_SEGMENT,
															(AM_HAL_CTIMER_INT_ENABLE |
															 AM_HAL_CTIMER_LFRC_512HZ |
															 AM_HAL_CTIMER_FN_REPEAT));

	am_hal_ctimer_int_register(BTN_TIMER_INTERRUPT, timer_handle);
	am_hal_ctimer_int_enable(BTN_TIMER_INTERRUPT);

}



void drv_button_init(button_callback task_cb)
{

	DEBUG_PRINTF(MOUDLE_NAME"drv_button_init\n");
	
	timer_create();
	
	m_instance.callback = task_cb;
	
	am_hal_gpio_int_register(BTN_PIN_PWR, btn_hdl_pwr);
	am_hal_gpio_int_register(BTN_PIN_BL, btn_hdl_bl);
	am_hal_gpio_int_register(BTN_PIN_UP, btn_hdl_up);
	am_hal_gpio_int_register(BTN_PIN_OK, btn_hdl_ok);
	am_hal_gpio_int_register(BTN_PIN_DOWN, btn_hdl_down);

	com_apollo2_gpio_int_init(BTN_PIN_PWR, BTN_PIN_PWR_POLARITY);
	com_apollo2_gpio_int_init(BTN_PIN_BL, BTN_PIN_BL_POLARITY);
	com_apollo2_gpio_int_init(BTN_PIN_UP, BTN_PIN_UP_POLARITY);
	com_apollo2_gpio_int_init(BTN_PIN_OK, BTN_PIN_OK_POLARITY);
	com_apollo2_gpio_int_init(BTN_PIN_DOWN, BTN_PIN_DOWN_POLARITY);
		
	

	
	m_instance.initialized = true;
	
}





