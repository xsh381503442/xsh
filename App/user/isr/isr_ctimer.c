#include "isr_config.h"
#include "isr_ctimer.h"
#include "isr_gpio.h"

#if DEBUG_ENABLED == 1 && ISR_CTIMER_LOG_ENABLED == 1
	#define ISR_CTIMER_LOG_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define ISR_CTIMER_LOG_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define ISR_CTIMER_LOG_WRITESTRING(...)
	#define ISR_CTIMER_LOG_PRINTF(...)		        
#endif

//*****************************************************************************
//
// Interrupt handler for the CTIMER module.
//
//*****************************************************************************
void
am_ctimer_isr(void)
{
	uint32_t ui32Status;

	//! Check the timer interrupt status.
	ui32Status = am_hal_ctimer_int_status_get(false);
	
	am_hal_ctimer_int_clear(ui32Status);

	//! Run handlers for the various possible timer events.
	am_hal_ctimer_int_service(ui32Status);
}
