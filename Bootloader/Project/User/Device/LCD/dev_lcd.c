#include "dev_lcd.h"
#include "drv_lcd.h"
#include "rtt_log.h"
#include "lib_error.h"
#include "com_apollo2.h"
#include "SEGGER_RTT.h"

#include "am_mcu_apollo.h"

#include "lib_display.h"
#include <string.h>


#define MOUDLE_LOG_ENABLED 							0
#define MOUDLE_DEBUG_ENABLED   					0
#define MOUDLE_NAME                     "[DEV_LCD]:"

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











void dev_lcd_init(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"dev_lcd_init\n");	
#ifdef WATCH_COD_BSP
	#ifndef   WATXCH_LCD_TRULY
      LCD_Poweron();
		//初始化LCD
	 LCD_Init();
	#endif
#else


	//初始化LCD
	LCD_Init();
	//打开屏电源
	LCD_Poweron();
	//将屏刷黑
	LCD_SetBackgroundColor(LCD_BLACK);
	LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
	//显示
	LCD_DisplayOn();	
	
#endif	
	
}






