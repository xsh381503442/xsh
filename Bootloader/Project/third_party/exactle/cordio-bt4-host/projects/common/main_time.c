/*************************************************************************************************/
/*!
 *  \file   main_time.c
 *
 *  \brief  Tickless timer implementation.
 *
 *          $Date: 2016-07-22 14:16:10 -0700 (Fri, 22 Jul 2016) $
 *          $Revision: 7978 $
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

#include "wsf_types.h"
#include "wsf_timer.h"

#include "main_time.h"

#include "SMD_TC2.h"
#include "platform.h"

/**************************************************************************************************
  Macros
**************************************************************************************************/

/*! \brief  WSF timer ticks per second. */
#define WSF_TICKS_PER_SEC       (1000 / WSF_MS_PER_TICK)

/*! \brief  Clock frequency of the RTC timer used. */
/* RTC ticks per second (with prescaler) */
#define RTC_TICKS_PER_SEC       (32768)

/*! \brief  Calculate number of elapsed WSF timer ticks. */
#define WSF_RTC_TICKS_TO_WSF(x) (((x) * WSF_TICKS_PER_SEC) / RTC_TICKS_PER_SEC)

/*! \brief  Convert WSF ticks to RTC ticks, with round. */
#define WSF_WSF_TICKS_TO_RTC(x) ((((x) * RTC_TICKS_PER_SEC) + \
                                  (WSF_TICKS_PER_SEC >> 1)) / WSF_TICKS_PER_SEC)

/*! \brief  Max value of RTC. */
#define MAX_RTC_COUNTER_VAL     (0xFFFFFFFF)

/*! \brief  Computing the difference between two RTC counter values. */
#if MAX_RTC_COUNTER_VAL == 0xFFFFFFFF
#define RTC_ELAPSED_TICKS(x)     ((x) - mainRtcLastTicks + mainRtcRemainder)
#else
#define RTC_ELAPSED_TICKS(x)    ((MAX_RTC_COUNTER_VAL + 1 + (x) - mainRtcLastTicks \
                                  + mainRtcRemainder) & MAX_RTC_COUNTER_VAL)
#endif

/**************************************************************************************************
  Variables
**************************************************************************************************/

/*! \brief  Last RTC value read. */
static uint32_t mainRtcLastTicks = 0;

/*! \brief  Remainder value. */
static uint32_t mainRtcRemainder = 0;

/*! \brief  Toggle sleep or deep sleep.  */
static bool_t   mainTimeDeepSleep;

/*************************************************************************************************/
/*!
 *  \brief  Tickless timer initialization routine.
 *
 *  \return None.
 */
/*************************************************************************************************/
void mainTimeInit(void)
{
  mainRtcLastTicks = PlatformReadSleepTimer();
  mainRtcRemainder = 0;
  mainTimeDeepSleep = TRUE;
}

/*************************************************************************************************/
/*!
 *  \brief  Function for checking if there is an active timer and if there is enough time to
 *          go to sleep and going to sleep.
 *
 *  \return None.
 */
/*************************************************************************************************/
void mainTimeSleep(void)
{
  wsfTimerTicks_t nextExpiration;
  bool_t          timerRunning;

  nextExpiration = WsfTimerNextExpiration(&timerRunning);

  if (timerRunning && (nextExpiration > 0))
  {
    uint32_t awake = WSF_WSF_TICKS_TO_RTC((uint32_t)nextExpiration);
    uint32_t rtcCurrentTicks = PlatformReadSleepTimer();
    uint32_t elapsed = RTC_ELAPSED_TICKS(rtcCurrentTicks);

    /* if we have time to sleep before timer expiration */
    if (awake > elapsed)
    {
      uint32_t sleepVal = (awake - elapsed) & MAX_RTC_COUNTER_VAL;

      /* enter sleep */
      __disable_irq();
      if (wsfOsReadyToSleep())
      {
        PlatformSleep(sleepVal, mainTimeDeepSleep);
      }
      __enable_irq();
    }
  }
  else
  {
    /* enter sleep */
    __disable_irq();
    if (wsfOsReadyToSleep())
    {
      PlatformSleep(0, mainTimeDeepSleep);
    }
    __enable_irq();
  }
}

/*************************************************************************************************/
/*!
 *  \brief  Function for updating WSF timer based on elapsed RTC ticks.
 *
 *  \return None.
 */
/*************************************************************************************************/
void mainTimeUpdate(void)
{
  uint32_t        elapsed;
  wsfTimerTicks_t wsfElapsed;

  /* get current RTC tick count */
  uint32_t rtcCurrentTicks = PlatformReadSleepTimer();
  if (rtcCurrentTicks != mainRtcLastTicks)
  {
    elapsed = RTC_ELAPSED_TICKS(rtcCurrentTicks);
    wsfElapsed = (wsfTimerTicks_t) WSF_RTC_TICKS_TO_WSF(elapsed);
    if (wsfElapsed)
    {
      /* update last ticks and remainder */
      mainRtcLastTicks = rtcCurrentTicks;
      mainRtcRemainder = ((elapsed * WSF_TICKS_PER_SEC) % RTC_TICKS_PER_SEC) / WSF_TICKS_PER_SEC;

      /* update wsf timers */
      WsfTimerUpdate(wsfElapsed);
    }
  }
}

/*************************************************************************************************/
/*!
 *  \brief  Allow the MCU to enter deep sleep.
 *
 *  \param  allow   TRUE if the MCU may enter deep sleep.
 *
 *  \return None.
 */
/*************************************************************************************************/
void mainTimeAllowDeepSleep(bool_t allow)
{
  mainTimeDeepSleep = allow;
}
