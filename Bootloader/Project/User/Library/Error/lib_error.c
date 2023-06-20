#include "lib_error.h"
#include "rtt_log.h"
#include "lib_error.h"
#include "com_apollo2.h"
#include "SEGGER_RTT.h"
#ifndef LAYER_UPDATEALGORITHM  
#include "lib_display.h"
#include "drv_lcd.h"
#endif
#include "am_mcu_apollo.h"

#include <stdio.h>
#include <string.h>

#ifdef LAYER_BOOTLOADER   
#ifdef AM_FREERTOS 
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "portmacro.h"
#include "portable.h"
#include "event_groups.h"
#include "semphr.h"
#endif
#endif

#define MOUDLE_LOG_ENABLED 							0
#define MOUDLE_DEBUG_ENABLED   					0
#define MOUDLE_NAME                     "[LIB_ERROR]:"

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


__weak void error_save_and_stop(uint32_t info)
{
    /* static error variables - in order to prevent removal by optimizers */
    static volatile struct
    {
        ret_code_t      err_code;
        uint32_t        line_num;
        const uint8_t * p_file_name;
    } m_error_data = {0};

    // The following variable helps Keil keep the call stack visible, in addition, it can be set to
    // 0 in the debugger to continue executing code after the error check.
    volatile bool loop = true;

		m_error_data.err_code     = ((error_info_t *)(info))->err_code;
		m_error_data.line_num     = (unsigned int) ((error_info_t *)(info))->line_num;
		m_error_data.p_file_name  = ((error_info_t *)(info))->p_file_name;

    // If printing is disrupted, remove the irq calls, or set the loop variable to 0 in the debugger.
    __disable_irq();
		
    while (loop);

    __enable_irq();
}



static void error_fault_handler(uint32_t info)
{
	DEBUG_PRINTF(MOUDLE_NAME"Hit error_fault_handler\n");
	if (((error_info_t *)(info))->p_file_name)
	{
		DEBUG_PRINTF("error --> " LOG_FLOAT_MARKER "\r\n", LOG_FLOAT((int)((error_info_t *)(info))->err_code));	
		DEBUG_PRINTF(MOUDLE_NAME"Err code --> 0x%08X\r\n",((error_info_t *)(info))->err_code);
		DEBUG_PRINTF(MOUDLE_NAME"Line Number: %u\r\n", (unsigned int) ((error_info_t *)(info))->line_num);
		DEBUG_PRINTF(MOUDLE_NAME"File Name:   %s\r\n", ((error_info_t *)(info))->p_file_name);
	}

#ifdef DEBUG_ENABLED
    DEBUG_PRINTF(MOUDLE_NAME"DEBUG\r\n");
    error_save_and_stop(info);
#else
	
#ifdef AM_FREERTOS 	
	taskENTER_CRITICAL();
#endif 	
	
		am_hal_reset_por();
#endif // DEBUG
}


void error_handler(ret_code_t error_code, uint32_t line_num, const uint8_t * p_file_name)
{
    error_info_t error_info =
    {
        .line_num    = line_num,
        .p_file_name = p_file_name,
        .err_code    = error_code,
    };
    error_fault_handler((uint32_t)(&error_info));
}











