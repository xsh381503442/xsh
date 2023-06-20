#ifndef _TASK_TOOL_H__
#define _TASK_TOOL_H__

#include "task_display.h"

extern TaskHandle_t TaskToolHandle;

extern float tool_compass_angle_get(void);
extern void tool_ambient_value_get(float *pPres,float *pAlt,float *pTemp);
extern ScreenState_t tool_task_init(ScreenState_t ScreenSta);
extern void tool_task_start(ScreenState_t ScreenSta);
extern void tool_task_close(void);

extern void CreateToolTask(void);
extern void TaskTools(void* pvParameter);

#endif


