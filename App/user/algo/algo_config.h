#ifndef __ALGO_CONFIG_H__
#define __ALGO_CONFIG_H__

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


#define ALGO_HDR_LOG_ENABLED     1
#define ALGO_TIME_CALIBRATION_LOG_ENABLED 1
#define ALGO_TRACKOPTIMIZING_LOG_ENABLED 1
#define ALGO_SPORT_CYCING_LOG_ENABLED 1
#define ALGO_HF_SWIMMING_MAIN_LOG_ENABLED 1
#endif
