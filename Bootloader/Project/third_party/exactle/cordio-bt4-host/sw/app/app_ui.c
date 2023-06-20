/*************************************************************************************************/
/*!
 *  \file   app_ui.c
 *
 *  \brief  Application framework user interface.
 *
 *          $Date: 2016-08-28 14:40:28 -0700 (Sun, 28 Aug 2016) $
 *          $Revision: 8609 $
 *
 *  Copyright (c) 2015 ARM, Ltd., all rights reserved.
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

#include <string.h>

#include "wsf_types.h"
#include "wsf_os.h"
#include "wsf_timer.h"
#include "wsf_trace.h"
#include "app_api.h"
#include "app_main.h"
#include "app_ui.h"
#include "board.h"
#include "platform.h"

/**************************************************************************************************
  Macros
**************************************************************************************************/

/*! \brief    Period for LED flash (connectable). */
#define APP_UI_GPIO_TIMER_PERIOD_CONN_MS     100

/*! \brief    Period for LED flash (non-connectable). */
#define APP_UI_GPIO_TIMER_PERIOD_NONCONN_MS  1000

/*! \brief    GPIO for LED flash. */
#define APP_UI_GPIO_NUM                      0

/* Button poll period in ms */
#define APP_UI_BTN_POLL_PERIOD        80

/* Number of poll period ticks for short, medium, and long presses */
#define APP_UI_BTN_TICKS_SHORT        1
#define APP_UI_BTN_TICKS_MED          24
#define APP_UI_BTN_TICKS_LONG         48
#define APP_UI_BTN_TICKS_XLONG        72

/* Number of ticks hardware delays button release edge */
#define APP_UI_BTN_HW_DELAY_TICKS     7

#define BUTTON_0_BITMASK              (1 << 0)

/**************************************************************************************************
  Global Variables
**************************************************************************************************/

#ifdef INIT_GPO
/*!< gpio timer */
static wsfTimer_t appUiGpioTimer;
/*!< gpio period */
static uint16_t appUiGpioPeriodMs;
#endif

/* Button press handler struct */
static struct
{
  appUiBtnCback_t     cback;
  wsfTimer_t          timer;
  uint32_t            btn;
  bool_t              pressed;
  uint8_t             tick;
} appUiBtn;


/*************************************************************************************************/
/*!
 *  \brief  Handler for GPIO messages.
 *
 *  \param  event   WSF event mask.
 *  \param  pMsg    WSF message.
 *
 *  \return None.
 */
/*************************************************************************************************/
#ifdef INIT_GPO
static void appUiGpioHandler(wsfEventMask_t event, wsfMsgHdr_t *pMsg)
{
  BoardGpoToggle(APP_UI_GPIO_NUM);

  WsfTimerStartMs(&appUiGpioTimer, appUiGpioPeriodMs);
}
#endif

/*************************************************************************************************/
/*!
 *  \brief  Initialize timer to flash LED.
 *
 *  \return None.
 */
/*************************************************************************************************/

void appUiGpioInit(void)
{
#ifdef INIT_GPO
  wsfHandlerId_t handlerId = WsfOsSetNextHandler(appUiGpioHandler);
  memset(&appUiGpioTimer, 0, sizeof(appUiGpioTimer));
  appUiGpioTimer.handlerId = handlerId;
#endif
}

/*************************************************************************************************/
/*!
 *  \fn     appUiSetGpioPeriod
 *
 *  \brief  Set period for GPIO toggle.
 *
 *  \param  periodMs  Period, in milliseconds.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void appUiSetGpioPeriod(uint16_t periodMs)
{
#ifdef INIT_GPO
  appUiGpioPeriodMs = periodMs;
  if (periodMs == 0)
  {
    WsfTimerStop(&appUiGpioTimer);
    BoardGpoClr(APP_UI_GPIO_NUM);
  }
  else
  {
    WsfTimerStartMs(&appUiGpioTimer, appUiGpioPeriodMs);
  }
#endif
}

/*************************************************************************************************/
/*!
 *  \fn     AppUiAction
 *
 *  \brief  Perform a user interface action based on the event value passed to the function.
 *
 *  \param  event   User interface event value.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppUiAction(uint8_t event)
{
  switch (event)
  {
    case APP_UI_PASSKEY_PROMPT:
      APP_TRACE_INFO0(">>> Passkey Needed <<<");
      break;
    
    case APP_UI_RESET_CMPL:
      APP_TRACE_INFO0(">>> Reset complete <<<");
      appUiSetGpioPeriod(APP_UI_GPIO_TIMER_PERIOD_NONCONN_MS);
      break;

    case APP_UI_CONN_OPEN:
      APP_TRACE_INFO0(">>> Conn open <<<");
      appUiSetGpioPeriod(0);
      break;

    case APP_UI_CONN_CLOSE:
      APP_TRACE_INFO0(">>> Conn close <<<");
      appUiSetGpioPeriod(APP_UI_GPIO_TIMER_PERIOD_CONN_MS);
      break;

    case APP_UI_ADV_START:
      APP_TRACE_INFO0(">>> Adv start [conn] <<<");
      appUiSetGpioPeriod(APP_UI_GPIO_TIMER_PERIOD_CONN_MS);
      break;

    case APP_UI_DISCOVERABLE:
      APP_TRACE_INFO0(">>> Adv start [nonconn] <<<");
      appUiSetGpioPeriod(APP_UI_GPIO_TIMER_PERIOD_NONCONN_MS);
      break;

    case APP_UI_ADV_STOP:
      APP_TRACE_INFO0(">>> Adv stop <<<");
      appUiSetGpioPeriod(APP_UI_GPIO_TIMER_PERIOD_NONCONN_MS);
      break;

    default:
      break;
  }
}

/*************************************************************************************************/
/*!
 *  \brief  Wakeup callback.
 *
 *  \return None.
 */
/*************************************************************************************************/
#ifdef INIT_WAKEUP
static void appUiWakeupCback(void)
{
  WsfSetEvent(appHandlerId, APP_BTN_DOWN_EVT);
}
#endif

/*************************************************************************************************/
/*!
 *  \brief  Initialize wakeup.
 *
 *  \return None.
 */
/*************************************************************************************************/
void appUiWakeupInit(void)
{
#ifdef INIT_WAKEUP
  PlatformEnableWakeup(appUiWakeupCback);
#endif
}

/*************************************************************************************************/
/*!
 *  \fn     AppUiDisplayPasskey
 *
 *  \brief  Display a passkey.
 *
 *  \param  passkey   Passkey to display.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppUiDisplayPasskey(uint32_t passkey)
{
  APP_TRACE_INFO1(">>> Passkey: %d <<<", passkey);
}

/*************************************************************************************************/
/*!
*  \fn     AppUiDisplayConfirmValue
*
*  \brief  Display a confirmation value.
*
*  \param  confirm    Confirm value to display.
*
*  \return None.
*/
/*************************************************************************************************/
void AppUiDisplayConfirmValue(uint32_t confirm)
{
  APP_TRACE_INFO1(">>> Confirm Value: %d <<<", confirm);
}

/*************************************************************************************************/
/*!
 *  \fn     AppUiDisplayRssi
 *
 *  \brief  Display an RSSI value.
 *
 *  \param  rssi   Rssi value to display.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppUiDisplayRssi(int8_t rssi)
{
  APP_TRACE_INFO1(">>> RSSI: %d dBm<<<", rssi);
}

/*************************************************************************************************/
/*!
 *  \fn     appUiTimerExpired
 *
 *  \brief  Handle a UI timer expiration event.
 *
 *  \param  pMsg    Pointer to message.
 *
 *  \return None.
 */
/*************************************************************************************************/
void appUiTimerExpired(wsfMsgHdr_t *pMsg)
{

}

/*************************************************************************************************/
/*!
 *  \fn     AppUiBtnPoll
 *
 *  \brief  Perform button press polling.  This function is called to handle WSF
 *          message APP_BTN_POLL_IND.
 *
 *  \return None.
 */
/*************************************************************************************************/
void appUiBtnPoll(void)
{
  uint8_t btn = APP_UI_BTN_NONE;
  uint32_t btnBitmap = BoardReadBtns();

  /* if button still pressed */
  if (appUiBtn.btn & btnBitmap)
  {
    /* restart timer */
    appUiBtn.timer.msg.event = APP_BTN_POLL_IND;
    WsfTimerStartMs(&appUiBtn.timer, APP_UI_BTN_POLL_PERIOD);

    /* increment tick count */
    appUiBtn.tick++;
  }
  /* else button released */
  else
  {
    appUiBtn.pressed = FALSE;

    /* Decrement ticks for hardware delay */ 
    if (appUiBtn.tick > APP_UI_BTN_HW_DELAY_TICKS)
      appUiBtn.tick -= APP_UI_BTN_HW_DELAY_TICKS;
    
    /* determine button code */
    if (appUiBtn.tick >= APP_UI_BTN_TICKS_SHORT)
    {
      btn = (appUiBtn.btn & BUTTON_0_BITMASK) ? APP_UI_BTN_1_SHORT : APP_UI_BTN_2_SHORT;

      if (appUiBtn.tick >= APP_UI_BTN_TICKS_MED)
      {
        btn++;
      }
      if (appUiBtn.tick >= APP_UI_BTN_TICKS_LONG)
      {
        btn++;
      }
      if (appUiBtn.tick >= APP_UI_BTN_TICKS_XLONG)
      {
        btn++;
      }
    }
  }

  /* execute callback */
  if (btn != APP_UI_BTN_NONE)
  {
    (*appUiBtn.cback)(btn);
  }
}

/*************************************************************************************************/
/*!
 *  \fn     AppUiBtnRegister
 *
 *  \brief  Register a callback function to receive button presses.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppUiBtnRegister(appUiBtnCback_t cback)
{
  appUiBtn.cback = cback;
}

/*************************************************************************************************/
/*!
 *  \fn     AppUiBtnPressed
 *
 *  \brief  Handle a hardware button press.  This function is called to handle WSF
 *          event APP_BTN_DOWN_EVT.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppUiBtnPressed(void)
{ 
  /* if button already pressed ignore new press */
  if (appUiBtn.pressed)
  {
    return;
  }

  /* read buttons */
  appUiBtn.btn = BoardReadBtns();

  /* initialize */
  appUiBtn.pressed = TRUE;
  appUiBtn.tick = 0;

  /* start button poll timer */
  appUiBtn.timer.handlerId = appHandlerId;
  appUiBtn.timer.msg.event = APP_BTN_POLL_IND;
  WsfTimerStartMs(&appUiBtn.timer, APP_UI_BTN_POLL_PERIOD);
}

/*************************************************************************************************/
/*!
 *
 *  \fn     AppUiSoundPlay
 *
 *  \brief  Play a sound.
 *
 *  \param  pSound   Pointer to sound tone/duration array.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppUiSoundPlay(const appUiSound_t *pSound)
{

}

/*************************************************************************************************/
/*!
 *  \fn     AppUiSoundStop
 *
 *  \brief  Stop the sound that is currently playing.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppUiSoundStop(void)
{

}

/*************************************************************************************************/
/*!
 *  \fn     AppUiBtnTest
 *
 *  \brief  Button test function-- for test purposes only.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppUiBtnTest(uint8_t btn)
{
  (*appUiBtn.cback)(btn);
}
