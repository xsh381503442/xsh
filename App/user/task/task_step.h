#ifndef _TASK_STEP_H__
#define _TASK_STEP_H__

#include <stdint.h>
#include "app_config.h"
#include "algo_swimming.h"
#include "task_config.h"



#if DEBUG_ENABLED == 1 && TASK_STEP_LOG_ENABLED == 1
	#define TASK_STEP_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define TASK_STEP_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define TASK_STEP_WRITESTRING(...)
	#define TASK_STEP_PRINTF(...)		        
#endif

extern uint8_t rtc_hw_flag;

extern TaskHandle_t	TaskStepHandle;
extern uint8_t		move_int;
extern void TaskStep(void* pvParameter);
extern void CreateStepTask(void);
extern void CloseStepTask(void);

extern uint32_t get_filt_stepcount(void);
extern void set_filt_stepcount(uint32_t steps,uint32_t sport_steps);

#endif


