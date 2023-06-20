#ifndef BSP_CONFIG_H
#define	BSP_CONFIG_H

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

#include "lib_log.h"

#include "watch_config.h"

#define	BSP_LOG_ENABLED			0
#define	BSP_IOM_LOG_ENABLED		0
#define BSP_RTC_LOG_ENABLED		0
#define BSP_UART_LOG_ENABLED	1
#define BSP_TIMER_LOG_ENABLED 	1

#endif
