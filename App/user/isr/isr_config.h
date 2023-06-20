#ifndef ISR_CONFIG_H
#define	ISR_CONFIG_H

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

#define	ISR_GPIO_LOG_ENABLED		0
#define	ISR_CTIMER_LOG_ENABLED		0
#define	ISR_IOM_LOG_ENABLED			0
#define ISR_UART_LOG_ENABLED		1
#define	ISR_CLKGEN_LOG_ENABLED		0

#endif
