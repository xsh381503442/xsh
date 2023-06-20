#include "sched_button.h"
#include "rtt_log.h"
#include "lib_error.h"
#include "SEGGER_RTT.h"


#include "am_mcu_apollo.h"

#include "com_apollo2.h"
#include "lib_scheduler.h"

#include "am_bootloader.h"

#include "lib_boot.h"
#include "lib_boot_setting.h"

#define MOUDLE_LOG_ENABLED 							1
#define MOUDLE_DEBUG_ENABLED   					1
#define MOUDLE_NAME                     "[SCHED_BUTTON]:"

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








static uint8_t m_pin;
static bool m_polarity = 1;
static uint32_t count = 0;

#define BTN_RELEASE_TIMEOUT       10
#define BTN_LONG_PUSH_COUNT       40

#define BTN_TIMER_NUMBER           0
#define BTN_TIMER_SEGMENT          AM_HAL_CTIMER_TIMERA
#define BTN_TIMER_INTERRUPT        AM_HAL_CTIMER_INT_TIMERA0













#define BTN_PIN_PWR 35
#define BTN_PIN_PWR_POLARITY 0
#define BTN_PIN_BL 40
#define BTN_PIN_BL_POLARITY 0
#define BTN_PIN_UP 29
#define BTN_PIN_UP_POLARITY 0
#define BTN_PIN_OK 46
#define BTN_PIN_OK_POLARITY 0
#define BTN_PIN_DOWN 22
#define BTN_PIN_DOWN_POLARITY 0


static void sched_evt(void * p_event_data, uint16_t event_size)
{
	DEBUG_PRINTF(MOUDLE_NAME"sched_evt\n");

	uint32_t evt = *(uint32_t *)p_event_data;
	
		switch(evt)
		{		
			case (BTN_PIN_PWR):{
				DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_PWR release\n");

			}break;
			case (BTN_PIN_PWR<<1):{
				DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_PWR long push\n");
				
			}break;		
			case (BTN_PIN_BL):{
				DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_BL release\n");
					
			}break;
			case (BTN_PIN_BL<<1):{
				DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_BL long push\n");
		
			}break;									
			case (BTN_PIN_UP):{
				DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_UP release\n");
				lib_boot_jump_to_bootloader(BOOT_SETTING_ENTER_BOOT_REASON_JUMP_APPLICATION);		
			}break;	
			case (BTN_PIN_UP<<1):{
				DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_UP long push\n");

				
			}break;				
			case (BTN_PIN_OK):{
				DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_OK release\n");
					
			}break;
			case (BTN_PIN_OK<<1):{
				DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_OK long push\n");
				
			}break;		
			case (BTN_PIN_DOWN):{
				DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_DOWN release\n");
			}break;
			
			case (BTN_PIN_DOWN<<1):{
				DEBUG_PRINTF(MOUDLE_NAME"BTN_PIN_DOWN long push\n");
				
			
			}break;					
		
		}
}


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
	polarity = am_hal_gpio_input_bit_read(m_pin);
	uint32_t error;	
	uint32_t evt = m_pin;	
	
	if(polarity != m_polarity)
	{
		DEBUG_PRINTF(MOUDLE_NAME"release\n");
		count = 0;
		timer_stop();	
		evt = m_pin;	
		error = app_sched_event_put(&evt ,sizeof(evt),sched_evt);
		ERR_CHECK(error);	
	}
	else{
		count++;
		if(count > BTN_LONG_PUSH_COUNT)
		{
			DEBUG_PRINTF(MOUDLE_NAME"long push\n");
			count = 0;
			timer_stop();
			evt = m_pin<<1;
			error = app_sched_event_put(&evt ,sizeof(evt),sched_evt);
			ERR_CHECK(error);
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



void sched_button_init(void)
{

	DEBUG_PRINTF(MOUDLE_NAME"sched_button_init\n");
	
	timer_create();
	
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

	
}


