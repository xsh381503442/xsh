#ifndef MODE_CONFIG_H
#define MODE_CONFIG_H

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

#define	MODE_POWER_ON_LOG_ENABLED 	1
#define	MODE_POWER_OFF_LOG_ENABLED 	1

#if defined WATCH_HAS_SOS
	#define MODE_SOS_LOG_ENABLED	0
#endif

#endif
