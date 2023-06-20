#ifndef GUI_HOME_CONFIG_H
#define GUI_HOME_CONFIG_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "am_mcu_apollo.h"
#include "am_util.h"
#include "SEGGER_RTT.h"

#include "FreeRTOS.h"
#include "task.h"
#include "portmacro.h"
#include "portable.h"
#include "semphr.h"
#include "event_groups.h"

#include "drv_lcd.h"
#include "drv_key.h"
#include "drv_battery.h"

#include "task_display.h"
#include "task_key.h"

#include "font_config.h"
#include "com_data.h"
#include "algo_hdr.h"
#include "timer_app.h"

#include "watch_config.h"

#define	GUI_HOME_LOG_ENABLED 		   0
#define	GUI_CLOCKLINE_LOG_ENABLE	0
#define GUI_TOOL_LOG_ENABLED        0
#define GUI_HEARTRATE_LOG_ENABLED   1
#define GUI_STEP_LOG_ENABLED        1
#define GUI_MOTION_RECORD_LOG_ENABLED 1
#define GUI_NOTIFICATION_LOG_ENABLED 1
#define GUI_TRAINPLAN_LOG_ENABLED    1
#define GUI_CALORIES_LOG_ENABLED    0
#endif
