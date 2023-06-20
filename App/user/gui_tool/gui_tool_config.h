#ifndef GUI_TOOL_CONFIG_H
#define GUI_TOOL_CONFIG_H

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

#include "watch_config.h"

#define	GUI_TOOL_CALENDAR_LOG_ENABLED	0
#define	GUI_TOOL_AMBIENT_LOG_ENABLED	0
#define	GUI_TOOL_GPS_LOG_ENABLED		0
#define	GUI_TOOL_COMPASS_LOG_ENABLED	0
#define	GUI_TOOL_STOPWATCH_ENABLED		0
#define GUI_TOOL_STOPWATCH_MENU_ENABLED	0
#define GUI_TOOL_COUNTDOWN_LOG_ENABLED	0

#endif
