#ifndef COM_CONFIG_H
#define COM_CONFIG_H

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

#define	COM_DATA_LOG_ENABLED 	0
#define COM_SPORT_LOG_ENABLED   0
#define	COM_DIAL_LOG_ENABLED	0

#endif
