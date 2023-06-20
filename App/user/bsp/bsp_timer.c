#include "bsp_timer.h"
#include "bsp_config.h"
#include "bsp.h"
#include "task_timer.h"
#include "algo_time_calibration.h"

#if DEBUG_ENABLED == 1 && BSP_TIMER_LOG_ENABLED == 1
	#define BSP_TIMER_LOG_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define BSP_TIMER_LOG_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define BSP_TIMER_LOG_WRITESTRING(...)
	#define BSP_TIMER_LOG_PRINTF(...)		        
#endif


void SetupTimerB0( uint32_t	Sec)
{
//	am_hal_ctimer_config_t sTimer1Config =
//	{
//			.ui32Link = 0,//
//			.ui32TimerAConfig = 0,

//			.ui32TimerBConfig = (AM_HAL_CTIMER_XT_256HZ |								
//													 AM_HAL_CTIMER_FN_ONCE |						
//													 AM_HAL_CTIMER_INT_ENABLE)
//	};
	am_hal_ctimer_stop(0,AM_HAL_CTIMER_TIMERB);
	am_hal_ctimer_clear(0, AM_HAL_CTIMER_TIMERB);
	
	//! Configure the timer frequency and mode.
	//am_hal_ctimer_config(0, &sTimer1Config);
	am_hal_ctimer_config_single(0, AM_HAL_CTIMER_TIMERB,AM_HAL_CTIMER_XT_256HZ |								
													 AM_HAL_CTIMER_FN_ONCE |						
													 AM_HAL_CTIMER_INT_ENABLE);
	am_hal_ctimer_period_set(0,AM_HAL_CTIMER_TIMERB,Sec*256,0); //1秒定时设置

	//! Enable the interrupt for timer B0
	am_hal_ctimer_int_enable(AM_HAL_CTIMER_INT_TIMERB0);

//	am_hal_interrupt_priority_set(AM_HAL_INTERRUPT_CTIMER, configKERNEL_INTERRUPT_PRIORITY);
//	am_hal_interrupt_enable(AM_HAL_INTERRUPT_CTIMER);

	am_hal_ctimer_int_register(AM_HAL_CTIMER_INT_TIMERB0, CTimerB0TickHandler);
	
	//! Enable the timer.
	am_hal_ctimer_start(0, AM_HAL_CTIMER_TIMERB);
	BSP_TIMER_LOG_PRINTF("[bsp_timer]:SetupTimerB0\n");
}

void StopTimerB0( void )
{
	am_hal_ctimer_stop(0,AM_HAL_CTIMER_TIMERB);
	BSP_TIMER_LOG_PRINTF("[bsp_timer]:StopTimerB0\n");
}

void ResetTimerB0( uint32_t	Sec )
{
	am_hal_ctimer_stop(0,AM_HAL_CTIMER_TIMERB);
	am_hal_ctimer_clear(0, AM_HAL_CTIMER_TIMERB);
	am_hal_ctimer_period_set(0,AM_HAL_CTIMER_TIMERB,Sec*256,0);
	am_hal_ctimer_start(0, AM_HAL_CTIMER_TIMERB);
	BSP_TIMER_LOG_PRINTF("[bsp_timer]:ResetTimerB0\n");
}

//500毫秒定时设置刷屏 用以显示精密时间补偿界面
void SetupTimerB2( uint32_t	Sec)
{
//	am_hal_ctimer_config_t sTimer1Config =
//	{
//			.ui32Link = 0,//
//			.ui32TimerAConfig = 0,

//			.ui32TimerBConfig = (AM_HAL_CTIMER_XT_256HZ |								
//													 AM_HAL_CTIMER_FN_ONCE |						
//													 AM_HAL_CTIMER_INT_ENABLE)
//	};
	am_hal_ctimer_stop(2,AM_HAL_CTIMER_TIMERB);
	am_hal_ctimer_clear(2, AM_HAL_CTIMER_TIMERB);
	
	//! Configure the timer frequency and mode.
	am_hal_ctimer_config_single(2, AM_HAL_CTIMER_TIMERB,AM_HAL_CTIMER_XT_256HZ |								
													 AM_HAL_CTIMER_FN_ONCE |						
													 AM_HAL_CTIMER_INT_ENABLE);
	am_hal_ctimer_period_set(2,AM_HAL_CTIMER_TIMERB,(Sec*256/5),0); //200毫秒定时设置

	//! Enable the interrupt for timer B2
	am_hal_ctimer_int_enable(AM_HAL_CTIMER_INT_TIMERB2);

//	am_hal_interrupt_priority_set(AM_HAL_INTERRUPT_CTIMER, configKERNEL_INTERRUPT_PRIORITY);
//	am_hal_interrupt_enable(AM_HAL_INTERRUPT_CTIMER);

	am_hal_ctimer_int_register(AM_HAL_CTIMER_INT_TIMERB2, CTimerB2TickHandler);
	
	//! Enable the timer.
	am_hal_ctimer_start(2, AM_HAL_CTIMER_TIMERB);
	BSP_TIMER_LOG_PRINTF("[bsp_timer]:SetupTimerB2\n");
}

void StopTimerB2( void )
{
	am_hal_ctimer_stop(2,AM_HAL_CTIMER_TIMERB);
	BSP_TIMER_LOG_PRINTF("[bsp_timer]:StopTimerB2\n");
}

void ResetTimerB2( uint32_t	Sec )
{
	am_hal_ctimer_stop(2,AM_HAL_CTIMER_TIMERB);
	am_hal_ctimer_clear(2, AM_HAL_CTIMER_TIMERB);
	am_hal_ctimer_period_set(2,AM_HAL_CTIMER_TIMERB,Sec*256/5,0);
	am_hal_ctimer_start(2, AM_HAL_CTIMER_TIMERB);
	//BSP_TIMER_LOG_PRINTF("[bsp_timer]:ResetTimerB2\n");
}
