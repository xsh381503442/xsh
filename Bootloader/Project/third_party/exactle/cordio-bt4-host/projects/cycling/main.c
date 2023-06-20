/*************************************************************************************************/
/*!
 *  \file   main.c
 *
 *  \brief  Main file for cycling application.
 *
 *          $Date: 2016-08-05 07:27:17 -0700 (Fri, 05 Aug 2016) $
 *          $Revision: 8163 $
 *
 *  Copyright (c) 2016 ARM, Ltd., all rights reserved.
 *  ARM confidential and proprietary.
 *
 *  IMPORTANT.  Your use of this file is governed by a Software License Agreement
 *  ("Agreement") that must be accepted in order to download or otherwise receive a
 *  copy of this file.  You may not use or copy this file for any purpose other than
 *  as described in the Agreement.  If you do not agree to all of the terms of the
 *  Agreement do not use this file and delete all copies in your possession or control;
 *  if you do not have a copy of the Agreement, you must contact ARM, Ltd. prior
 *  to any use, copying or further distribution of this software.
 */
/*************************************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "cordio_sdk_version.h"

#include "wsf_types.h"
#include "wsf_assert.h"
#include "wsf_buf.h"
#include "wsf_math.h"
#include "wsf_msg.h"
#include "wsf_os.h"
#include "wsf_trace.h"
#include "bstream.h"

#include "board.h"
#include "main_time.h"
#include "app_param.h"

#include "platform.h"

#include "cordio_tc2.h"

#include "app_terminal.h"
#include "cycling_api.h"

#include "hci_handler.h"
#include "dm_handler.h"
#include "l2c_handler.h"
#include "att_handler.h"
#include "smp_handler.h"
#include "l2c_api.h"
#include "att_api.h"
#include "smp_api.h"
#include "app_api.h"
#include "hci_core.h"
#include "svc_dis.h"
#include "svc_core.h"
#include "sec_api.h"

/**************************************************************************************************
  Macros
**************************************************************************************************/

/*! \brief      Number of WSF buffer pools. */
#define WSF_BUF_POOLS           4

/*! \brief      Buffer size. */
#define ACL_BUF_SIZE            256

/*! \brief      Total number of buffers. */
#define NUM_BUFS                8

/*! \brief      Total size in bytes for buffer memory. */
#define BUF_MEM_SIZE            (1024 * 4)

/**************************************************************************************************
  Global Variables
**************************************************************************************************/

/*! \brief      Free memory for pool buffers (align to word boundary). */
static uint32_t         mainBufMem[BUF_MEM_SIZE/sizeof(uint32_t)];

/*! \brief      Default pool descriptor. */
static wsfBufPoolDesc_t mainPoolDesc[WSF_BUF_POOLS] =
{
  { 16,   8 },
  { 32,   8 },
  { 64,   8 },
  { ACL_BUF_SIZE, NUM_BUFS }
};

/**************************************************************************************************
  External Functions
**************************************************************************************************/

/*< Initialize timer to flash LED. */
extern void appUiGpioInit(void);

/*< Initialize wakeup. */
extern void appUiWakeupInit(void);

/**************************************************************************************************
  Stack
**************************************************************************************************/

/*************************************************************************************************/
/*!
 *  \brief  Initialize the platform.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void mainPlatformInit(void)
{
  WsfTimerInit();
  WsfBufInit(sizeof(mainBufMem), (uint8_t*)mainBufMem, WSF_BUF_POOLS, mainPoolDesc);

  PlatformUartRxAsyncStart(AppTerminalRx);
  AppTerminalRegisterUartTxFunc(PlatformUartTx);
}

/*************************************************************************************************/
/*!
 *  \brief  Initialize the device information.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void mainDeviceInfoInit(void)
{
  /* TODO make device info settable at run-time */
}

/*************************************************************************************************/
/*!
 *  \fn     mainStackInit
 *
 *  \brief  Initialize stack.
 *
 *  \param  None.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void mainStackInit(void)
{
  wsfHandlerId_t handlerId;

  SecInit();
  SecAesInit();
  SecCmacInit();
  SecEccInit();

  handlerId = WsfOsSetNextHandler(HciHandler);
  HciHandlerInit(handlerId);

  handlerId = WsfOsSetNextHandler(DmHandler);
  DmDevVsInit(0);
  DmAdvInit();
  DmConnInit();
  DmConnSlaveInit();
  DmSecInit();
  DmSecLescInit();
  DmPrivInit();
  DmHandlerInit(handlerId);

  handlerId = WsfOsSetNextHandler(L2cSlaveHandler);
  L2cSlaveHandlerInit(handlerId);
  L2cInit();
  L2cSlaveInit();

  handlerId = WsfOsSetNextHandler(AttHandler);
  AttHandlerInit(handlerId);
  AttsInit();
  AttsIndInit();

  handlerId = WsfOsSetNextHandler(SmpHandler);
  SmpHandlerInit(handlerId);
  SmprInit();
  SmprScInit();
  HciSetMaxRxAclLen(100);

  handlerId = WsfOsSetNextHandler(AppHandler);
  AppHandlerInit(handlerId);

  handlerId = WsfOsSetNextHandler(AppTerminalHandler);
  AppTerminalInit(handlerId);

  handlerId = WsfOsSetNextHandler(CyclingHandler);
  CyclingHandlerInit(handlerId);
}

/*************************************************************************************************/
/*!
 *  \brief  Start demo.
 *
 *  \param  status      TRUE if startup succeeded.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void mainDemoStart(bool_t status)
{
  /* Only start demo upon success. */
  if (status)
  {
    /* Initialize the stack. */
    mainStackInit();

    /* Start Cycling application. */
    CyclingStart();

    /* Set device information service data. */
    mainDeviceInfoInit();
  }
}

/*************************************************************************************************/
/*!
 *  \brief  Entry point for demo software.
 *
 *  \param  None.
 *
 *  \return None.
 */
/*************************************************************************************************/
int main()
{
  /* ---------- init ---------- */

  WsfTraceEnable(TRUE);
  BoardHwInit();
  AppParamInit();

  mainPlatformInit();
  mainTimeInit();

  mainTimeAllowDeepSleep(CONFIG_ALLOW_DEEP_SLEEP);

  /* Initialize GPIO output. */
  appUiGpioInit();

  /* Initialize wakeup input. */
  appUiWakeupInit();

  /* Initialize LLC. */
  bool_t status = CordioTc2LlcInit();

  /* ---------- application ---------- */

  mainTimeUpdate();
  mainDemoStart(status);

  bool_t tracePending = FALSE;

  while (TRUE)
  {
    mainTimeUpdate();

    wsfOsDispatcher();

#if WSF_TOKEN_ENABLED == TRUE
    tracePending = WsfTokenService();
#endif

    if (!tracePending)
    {
      mainTimeSleep();
    }
  }
}
