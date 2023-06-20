#ifndef DRV_CONFIG_H
#define	DRV_CONFIG_H

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


#define	DRV_LCD_LOG_ENABLED			0
#define	DRV_KEY_LOG_ENABLED			1
#define	DRV_GT24L24_LOG_ENABLED		0
#define	DRV_MOTOR_LOG_ENABLED		0
#define	DRV_BUZZER_LOG_ENABLED		0
#define DRV_UBLOX_LOG_ENABLED  		1
#define DRV_FLASH_LOG_ENABLED  		0
#define	DRV_MS5837_LOG_ENABLED		0
#define DRV_NDOF_LOG_ENABLED        1
#define DRV_HEARTRATE_LOG_ENABLED   0
#define DRV_LIGHT_LOG_ENABLED       1
#define DRV_PC_LOG_ENABLED       1




#endif
