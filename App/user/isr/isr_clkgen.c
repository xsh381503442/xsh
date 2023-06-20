#include "isr_config.h"
#include "isr_clkgen.h"
#include "task_battery.h"
#include "task_display.h"
#include "drv_alarm.h"
#include "com_sport.h"
#include "task_sport.h"
#include "bsp_rtc.h"
#include "FreeRTOS.h"
#include "task.h"
#include "lib_error.h"


#if DEBUG_ENABLED == 1 && ISR_CLKGEN_LOG_ENABLED == 1
	#define ISR_CLKGEN_LOG_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define ISR_CLKGEN_LOG_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define ISR_CLKGEN_LOG_WRITESTRING(...)
	#define ISR_CLKGEN_LOG_PRINTF(...)		        
#endif

#ifdef DEBUG_ERR 
uint32_t heartlight = 0;
#endif

void am_clkgen_isr( void )
{
	uint32_t ui32Status;
    DISPLAY_MSG  msg = {0};	
	static BaseType_t xHigherPriorityTaskWoken = false;	
	ui32Status = am_hal_rtc_int_status_get(false);
	am_hal_rtc_int_clear(ui32Status);	
	
	if(am_hal_rtc_int_enable_get() == AM_HAL_RTC_INT_ALM)	
	{
		//drv_alarm_handler();
        if(Get_TaskSportSta() == true)
        {
#ifdef DEBUG_ERR  
              heartlight++;
              if (heartlight > 5)
              {
                  error_currenttask();
              }
#endif
              if(TaskSportHandle != NULL)
              {
                  //活动开始的情况下,每秒钟发送信号量到sporttask
                  xSemaphoreGiveFromISR(ActSemaphore, &xHigherPriorityTaskWoken);
              }
              am_hal_rtc_time_get(&RTC_time);

              if(RTC_time.ui32Second == 0)
              {
                  //在活动过程中，每分钟需要检测一次数据
                  msg.cmd = MSG_UPDATE_TIME;  
                  xQueueSendFromISR(DisplayQueue, &msg, &xHigherPriorityTaskWoken);
              }
         }

		else if(ScreenState != DISPLAY_SCREEN_LOGO)
		{
			xHigherPriorityTaskWoken = pdFALSE;		
			//更新时间
			msg.cmd = MSG_UPDATE_TIME;	
			xQueueSendFromISR(DisplayQueue, &msg, &xHigherPriorityTaskWoken);
				
		}	
        if( xHigherPriorityTaskWoken )
		{
			/* Actual macro used here is port specific. */
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}	
        
	
	}
}



