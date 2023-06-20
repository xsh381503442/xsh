#ifndef GUI_SPORT_CONFIG_H
#define GUI_SPORT_CONFIG_H

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

#define	GUI_SPORT_LOG_ENABLED					1
#define	GUI_RUN_LOG_ENABLED						1
#define	GUI_SPORT_CYCLING_LOG_ENABLED			0
#define	GUI_SPORT_INDOORRUN_LOG_ENABLED			0
#define	GUI_SPORT_WALK_LOG_ENABLED				0
#define	GUI_SPORT_MARATHON_LOG_ENABLED			0
#define GUI_SPORT_CROSSCOUNTRY_HIKE_LOG_ENABLED 0
#define	GUI_SPORT_TRIATHLON_LOG_ENABLED			0
#define GUI_TRAINPLAN_RUN_LOG_ENABLED      		0
#define GUI_SPORT_CROSSCOUNTRY_LOG_ENABLED 		1

#endif
