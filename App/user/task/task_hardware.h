#ifndef _TASK_HARDWARE_H__
#define _TASK_HARDWARE_H__

#include <stdint.h>
#include "app_config.h"
#include "task_display.h"


extern void DisplayTestSTEP(void);
extern void DisplayTestCOMPASS(uint16_t angle);
extern void DisplayTestPressure(void);
extern void DisplayTestMotor(void);
extern void MsgHardware(ScreenState_t ScreenSta,uint32_t Key_Value);
extern void TaskHardware( void* pvParameter);
extern TaskHandle_t TaskHardHandle;
extern void HardewareTaskCreate( void);
extern void DisplayTestCompassCalibration(void);
#endif


