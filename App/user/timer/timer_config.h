#ifndef __TIMER_CONGIF_H__
#define __TIMER_CONGIF_H__

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

#define TIMER_APP_LOG_ENABLED   0

#if defined WATCH_HAS_SOS
	#define	TIMER_SOS_LOG_ENABLED	0
#endif

#endif
