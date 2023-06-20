#ifndef __DRV_ALARM_H__
#define __DRV_ALARM_H__

#include "task_display.h"

extern ScreenState_t ScreenStateAlarm;
extern uint8_t g_drv_alarm_num;

void drv_alarm_handler(void);



#endif //__DRV_ALARM_H__
