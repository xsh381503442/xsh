#ifndef TASK_CONFIG_H
#define	TASK_CONFIG_H

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
#include "task_gps.h"
#include "task_display.h"

#include "watch_config.h"

#define	TASK_KEY_LOG_ENABLED		0
#define TASK_GPS_LOG_ENABLED        0
#define TASK_DISPLAY_LOG_ENABLED    1
#define TASK_UART_LOG_ENABLED       1
#define TASK_TIMER_LOG_ENABLED      1
#define TASK_STEP_LOG_ENABLED       1
#define TASK_TOOL_LOG_ENABLED       1
#define TASK_BATTERY_LOG_ENABLED    0
#define TASK_PC_LOG_ENABLED         1
#define TASK_HDR_LOG_ENABLED        0
#define TASK_BLE_LOG_ENABLED        0
#define TASK_SPORT_LOG_ENABLED      1


#define TaskDisplay_Priority        5
#define TaskDisplay_StackDepth      1500
#define TaskKey_Priority            5
//#define TaskKey_StackDepth          300
#define TaskKey_StackDepth          1600
#define TaskUart_Priority           1
#define TaskUart_StackDepth         220
#define TaskGPSTime_Priority        5
#define TaskGPSTime_StackDepth      900
#define TaskTools_Priority          5
#define TaskTools_StackDepth        250*2
#define TaskBattery_Priority        5
#define TaskBattery_StackDepth      256
#define TaskPC_Priority             5

#if defined WATCH_IMU_CALIBRATION
#define TaskPC_StackDepth           256*8
#else
#define TaskPC_StackDepth           256 + 128
#endif

#define TaskHDR_Priority            5
#define TaskHDR_StackDepth          220
#define TaskBLE_Priority             5
#define TaskBLE_StackDepth           512+128
#define TaskTimer_Priority          5
#define TaskTimer_StackDepth        250
#define TaskSport_Priority          5
#define TaskSport_StackDepth        512
//#define TaskSport_StackDepth        2048
#define TaskStep_Priority          5
#define TaskStep_StackDepth        1024

//调试测试GPS
#define DEBUG_TEST_GPS              0
//调试spiflash
#define DEBUG_TEST_SPIFLASH         0
//调试心率
#define DEBUG_TEST_HEART            0
//调试gps/hdr/flash
#define DEBUG_TEST_GPS_HDR_FLASH    1
#endif
