//*****************************************************************************
//
//! @file rtos.c
//!
//! @brief Essential functions to make the RTOS run correctly.
//!
//! These functions are required by the RTOS for ticking, sleeping, and basic
//! error checking.
//
//*****************************************************************************

//*****************************************************************************
//
// Copyright (c) 2016, Ambiq Micro
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
// 
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// This is part of revision 1.0.6 of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "app_config.h"
#include "rtos.h"

#include "task_key.h"
#include "task_hardware.h"
#include "task_step.h"
#include "task_tool.h"
#include "task_display.h"
#include "task_battery.h"
#include "task_ble.h"
#include "task_sport.h"
#include "task_timer.h"
#include "time_notify.h"
#include "time_progress.h"
#include "drv_ble.h"
#include "drv_lcd.h"
#include "task_battery.h"
#include "timer_app.h"
#include "lib_error.h"
#include "drv_lsm6dsl.h"
#include "com_data.h"
#include "gui_tool_findphone.h"
#include "gui_dfu.h"
#include "gui_notify_warning.h"
#include "task_hrt.h"



//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
TaskHandle_t xSleepTask;
SemaphoreHandle_t Appolo_SPI_Semaphore = NULL;
SemaphoreHandle_t Appolo_I2C_Semaphore = NULL;
SemaphoreHandle_t Apollo_UART1_Semaphore = NULL;

extern SetValueStr SetValue;
uint8_t task_start_flag = 0 ;
//*****************************************************************************
//
// RTOS Tick events
//
//*****************************************************************************
//void
//rtos_tick(void)
//{
//    //
//    // If this was a timer a0 interrupt, perform the necessary functions
//    // for the tick ISR.
//    //
//    (void) portSET_INTERRUPT_MASK_FROM_ISR();
//    {
//        //
//        // Increment RTOS tick
//        //
//        if ( xTaskIncrementTick() != pdFALSE )
//        {
//            //
//            // A context switch is required.  Context switching is
//            // performed in the PendSV interrupt. Pend the PendSV
//            // interrupt.
//            //
//            portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET_BIT;
//        }
//    }
//    portCLEAR_INTERRUPT_MASK_FROM_ISR(0);
//}

//*****************************************************************************
//
// Configures and enables the timer interrupt used by FreeRTOS
//
//*****************************************************************************
//void vPortSetupTimerInterrupt(void)
//{
//    am_hal_ctimer_config_t sTimer0Config =
//    {
//        .ui32Link = 1,//Timer link bit置 1，配置为32位Timer
//		//Timer时钟改为外部32.768KHz
//        .ui32TimerAConfig = (AM_HAL_CTIMER_XT_32_768KHZ |
//                             AM_HAL_CTIMER_FN_REPEAT |						
//                             AM_HAL_CTIMER_INT_ENABLE),

//        .ui32TimerBConfig = (AM_HAL_CTIMER_XT_32_768KHZ |
//                             AM_HAL_CTIMER_FN_REPEAT |						
//                             AM_HAL_CTIMER_INT_ENABLE)
//    };

//	
//    //
//    // Configure the timer frequency and mode.
//    //
//    am_hal_ctimer_config(0, &sTimer0Config);

//    //
//    // Set the timeout interval
//    //
////    am_hal_ctimer_compare_set(0, AM_HAL_CTIMER_TIMERA, 0, 12);
//	am_hal_ctimer_period_set(0,AM_HAL_CTIMER_BOTH,32768/configTICK_RATE_HZ,0); //32768/100=327

//    //
//    // Enable the interrupt for timer A0
//    //
//    am_hal_ctimer_int_enable(AM_HAL_CTIMER_INT_TIMERA0);

//    //
//    // Enable the timer interrupt in the NVIC, making sure to use the
//    // appropriate priority level.
//    //
//    am_hal_interrupt_priority_set(AM_HAL_INTERRUPT_CTIMER, configKERNEL_INTERRUPT_PRIORITY);
//    am_hal_interrupt_enable(AM_HAL_INTERRUPT_CTIMER);

//    //
//    // Enable the timer.
//    //
//    am_hal_ctimer_start(0, AM_HAL_CTIMER_BOTH);
//}

//*****************************************************************************
//
// Sleep function called from FreeRTOS IDLE task.
// Do necessary application specific Power down operations here
// Return 0 if this function also incorporates the WFI, else return value same
// as idleTime
//
//*****************************************************************************
uint32_t am_freertos_sleep(uint32_t idleTime)
{
    am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
    return 0;
}

//*****************************************************************************
//
// Recovery function called from FreeRTOS IDLE task, after waking up from Sleep
// Do necessary 'wakeup' operations here, e.g. to power up/enable peripherals etc.
//
//*****************************************************************************
void am_freertos_wakeup(uint32_t idleTime)
{
    return;
}
extern TaskHandle_t	TaskKeyHandle;

//*****************************************************************************
//
// FreeRTOS debugging functions.
//
//*****************************************************************************
void
vApplicationMallocFailedHook(void)
{
 #if 0
		LCD_SetBackgroundColor(LCD_BLACK);
		SetWord_t word = {0};
		word.x_axis = 40;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_16_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_BLACK;
		word.kerning = 0;		
	
		char p_char[23];	
		memset(p_char,0,sizeof(p_char));
		sprintf(p_char,"free :%d",xPortGetFreeHeapSize()); 	
		LCD_SetErrCode(p_char,&word);	
		  
	
			
		word.x_axis += 32;
		memset(p_char,0,sizeof(p_char));
		sprintf(p_char,"step %d",uxTaskGetStackHighWaterMark(TaskStepHandle));
		LCD_SetErrCode(p_char,&word);

		word.x_axis += 32;
		memset(p_char,0,sizeof(p_char));
		sprintf(p_char,"sport %d",uxTaskGetStackHighWaterMark(TaskSportHandle));
		LCD_SetErrCode(p_char,&word);

	    word.x_axis += 32;
		memset(p_char,0,sizeof(p_char));
		sprintf(p_char,"key %d",uxTaskGetStackHighWaterMark(TaskKeyHandle));
		LCD_SetErrCode(p_char,&word);
		
		LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
		vTaskSuspend(TaskSportHandle);
#else
	ERR_HANDLER(ERR_NO_MEM);
#endif
    //
    // Called if a call to pvPortMalloc() fails because there is insufficient
    // free memory available in the FreeRTOS heap.  pvPortMalloc() is called
    // internally by FreeRTOS API functions that create tasks, queues, software
    // timers, and semaphores.  The size of the FreeRTOS heap is set by the
    // configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h.
    //
    while (1);
}

void
vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
{
    (void) pcTaskName;
    (void) pxTask;
	
	SPI_No_Semaphore = 1;

	

	if (Get_TaskSportSta())
	{
		SaveSport(1);
	}
	mode_power_off_store_set_value();
#if ERR_PRINT == 1 
	LCD_SetBackgroundColor(LCD_BLACK);
	SetWord_t word = {0};
	word.x_axis = 40;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	word.kerning = 0;		

	char p_char[23];	
	memset(p_char,0,sizeof(p_char));
	sprintf(p_char,"error :%d",ERR_NO_MEM);		
	LCD_SetErrCode(p_char,&word);	
      

        
	word.x_axis += 32;
	memset(p_char,0,sizeof(p_char));
	sprintf(p_char,"%s",pcTaskName);
	LCD_SetErrCode(p_char,&word);
	LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
#else
	//taskENTER_CRITICAL();
	am_hal_reset_por();
#endif		

    //ERR_HANDLER(ERR_NO_MEM);
    //
    // Run time stack overflow checking is performed if
    // configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
    // function is called if a stack overflow is detected.
    //
    while (1);
}

//*****************************************************************************
//
// Sets the "idle" action.
//
//*****************************************************************************
void
sleep_task(void *pvParameters)
{
    while(1)
    {
        //
        // Enter deep-sleep.
        //
        am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
    }
}

void semaphore_create(void)
{
	//SPI二值信号量，用于多SPI通信互斥机制
	vSemaphoreCreateBinary( Appolo_SPI_Semaphore );
	
	//I2C二值信号量，用于多I2C通信互斥机制
	vSemaphoreCreateBinary( Appolo_I2C_Semaphore );

    //UART二值信号量，用于多UART通信互斥机制
	vSemaphoreCreateBinary( Apollo_UART1_Semaphore );
}

//*****************************************************************************
//
// Initializes all tasks
//
//*****************************************************************************
void run_tasks(void)
{
	RTOS_LOG_PRINTF("[rtos]:##### run tasks #####\r\n");
	
	CreateKeyTask();
	CreatDisplayTask();
	CreateStepTask();

	task_hrt_init();
	CreateToolTask();
#if defined(HARDWARE_TEST)
	xTaskCreate(TaskHardware, "Task Hardware",350, 0, 5, &TaskHardHandle);
#endif
	
	
	timer_app_init();
	
  CreatTimerTask();
#ifndef WATCH_HAS_NO_BLE 

	if(SetValue.SwBle)
	{
		task_ble_init();
	}
#endif
	timer_notify_display_init();
	timer_notify_motor_init();
	timer_notify_buzzer_init();
	timer_notify_backlight_init();
	timer_progress_init();
#if !defined WATCH_RAISE_BRIGHT_SCREEN_ALGO
  timer_wrist_tilt_init();
#endif	

	timer_notify_remind_init();
 
	drv_ble_timer_init();


	gui_tool_findphone_timer_init();
	gui_notify_findwatch_timer_init();
	gui_dfu_timer_init();

	task_battery_init();


	dev_ble_init(SetValue.SwBle);
#ifdef WATCH_LCD_LPM
	timer_lcd_lpm_start();
#endif
#ifdef WATCH_LCD_ESD
	timer_read_lcd_start();
#endif
    //
    // Start the scheduler.
    //
    task_start_flag = 1;
    vTaskStartScheduler();
	
}

