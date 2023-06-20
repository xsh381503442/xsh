#include "task_setup.h"
#include "rtt_log.h"
#include "lib_error.h"
#include "SEGGER_RTT.h"
#include <string.h>

#include "am_mcu_apollo.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "portmacro.h"
#include "portable.h"
#include "event_groups.h"
#include "semphr.h"
#include "drv_pc.h"

#include "task_button.h"
#include "task_pc.h"
#include "dev_notify.h"
#include "lib_boot_setting.h"
#include "dev_dfu.h"
#include "lib_boot.h"
#include "task_ble.h"

#include "gui_boot.h"	
#include "gui_exception.h"


#include "task_charge.h"
#include "dev_bat.h"
#include "task_display.h"
#include "timer_boot.h"
#include "timer_exception.h"
#include "gui_back_door.h"
#include "dev_dfu_ble.h"
#include "drv_lcd.h"
#include "board.h"


#define MOUDLE_LOG_ENABLED 							0
#define MOUDLE_DEBUG_ENABLED   					0
#define MOUDLE_NAME                     "[TASK_SETUP]:"

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


static TaskHandle_t task_handle;
extern _dev_bat_event   g_bat_evt;
extern ScreenState_t   ScreenState;
extern QueueHandle_t 	DisplayQueue;

extern bool g_is_ble_dfu;

#define     BL_TIMER_NUMBER         3
#define     BL_TIMER_SEGMENT        AM_HAL_CTIMER_TIMERA
#define     BL_TIMER_INTERRUPT      AM_HAL_CTIMER_INT_TIMERA3
#define     BL_INTERVAL             1  //12K/1=12KHZ
#define     BL_PERIOD               10 //PWM周期1.2k

static uint8_t backlight_level = 2;         //默认背光等级

static void task(void *pvParameters)
{
	#ifdef   WATXCH_LCD_TRULY
     LCD_Poweron();
		//初始化LCD
	 LCD_Init();
	#endif
	DEBUG_PRINTF(MOUDLE_NAME"task\n");

	if(g_is_ble_dfu == true)
	{
		gui_boot_paint();
		LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
		dev_dfu_ble_start();
	}

	task_button_init();
	task_ble_init();
	task_charge_init();
	task_pc_init();
	task_display_init();
	
	
	
	
//	timer_boot_init();
//	timer_boot_start(2000);
	
	timer_exception_init();
	timer_exception_start(TIMER_EXCEPTION_TIMEOUT);	
#ifdef WATCH_COD_BSP

	//Enable Backlight
	am_hal_gpio_pin_config(BSP_APOLLO2_BACKLIGHT_PIN,AM_HAL_PIN_48_TCTA3);
	
	//A3,频率为4kHz
	am_hal_ctimer_config_single(BL_TIMER_NUMBER, BL_TIMER_SEGMENT,
                                (AM_HAL_CTIMER_FN_PWM_REPEAT |
                                 AM_HAL_CTIMER_XT_32_768KHZ |
                                 AM_HAL_CTIMER_PIN_ENABLE));
    am_hal_ctimer_period_set(BL_TIMER_NUMBER, BL_TIMER_SEGMENT, BL_PERIOD, backlight_level);
    am_hal_ctimer_start(BL_TIMER_NUMBER, BL_TIMER_SEGMENT);

#else
	am_hal_gpio_pin_config(BSP_APOLLO2_BACKLIGHT_PIN,AM_HAL_GPIO_OUTPUT);
	am_hal_gpio_out_bit_set(BSP_APOLLO2_BACKLIGHT_PIN);	
#endif
	
	if(ScreenState == DISPLAY_SCREEN_BOOTLOADER)
	{
		gui_boot_task_display();
	}
	else if(ScreenState == DISPLAY_SCREEN_EXCEPTION)
	{
		gui_exception_task_display();
	}
	else if(ScreenState == DISPLAY_SCREEN_BACK_DOOR)
	{
		gui_back_door_task_display();
	}
	
	
	
	vTaskSuspend(NULL);

	while (1);
}

void task_setup_init(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"task_setup_init\n");
	BaseType_t xResult;
	
	xResult = xTaskCreate(task, "SETUP", 256*2, 0, 5, &task_handle);

	if (xResult == pdFAIL)
	{
		ERR_HANDLER(ERR_FORBIDDEN);
	}

	vTaskStartScheduler();
}















