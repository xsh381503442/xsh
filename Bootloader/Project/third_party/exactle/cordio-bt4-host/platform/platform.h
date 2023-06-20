/*************************************************************************************************/
/*!
 *  \file   platform.h
 *
 *  \brief  Platform services.
 *
 *          $Date: 2016-03-29 14:55:12 -0700 (Tue, 29 Mar 2016) $
 *          $Revision: 6524 $
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

#ifndef PLATFORM_H
#define PLATFORM_H

#include "wsf_types.h"

/**************************************************************************************************
  Data Types
**************************************************************************************************/

/*! \brief    Platform UART Rx callback. */
typedef void (*platformUartRxCback_t)(uint8_t rxByte);

/*! \brief    Platform UART Tx complete callback. */
typedef void (*platformUartTxCmplCback_t)(const uint8_t *pBuf);

/*! \brief    Platform sleep or wake callback. */
typedef void (*platformSleepCback_t)(void);

/*! \brief    Wakeup input callback. */
typedef void (*platformWakeupCback_t)(void);

/*************************************************************************************************/
/*!
 *  \brief  Initialize the platform GPIO.
 *
 *  \return None.
 */
/*************************************************************************************************/
void PlatformGpioInit(void);

/*************************************************************************************************/
/*!
 *  \brief  Initialize the platform UART.
 *
 *  \return None.
 */
/*************************************************************************************************/
void PlatformUartInit(void);

/*************************************************************************************************/
/*!
 *  \brief  Start receiving data asynchronously on UART.
 *
 *  \brief  cback   Callback to invoke when byte is received.
 *
 *  \return None.
 */
/*************************************************************************************************/
void PlatformUartRxAsyncStart(platformUartRxCback_t cback);

/*************************************************************************************************/
/*!
 *  \brief  Transmit buffer on platform UART asynchronously.
 *
 *  \param  pBuf    Buffer to transmit.
 *  \param  len     Length of buffer in octets.
 *  \param  cback   Callback to invoke when transmission is complete.
 *
 *  \return None.
 */
/*************************************************************************************************/
void PlatformUartTxAsync(const uint8_t *pBuf, uint32_t len, platformUartTxCmplCback_t cback);

/*************************************************************************************************/
/*!
 *  \brief  Transmit buffer on platform UART.
 *
 *  \param  pBuf    Buffer to transmit.
 *  \param  len     Length of buffer in octets.
 *
 *  \return None.
 */
/*************************************************************************************************/
void PlatformUartTx(const uint8_t *pBuf, uint32_t len);

/*************************************************************************************************/
/*!
 *  \brief  Increment counter to keep MCU from entering deep sleep.
 *
 *  \return None.
 */
/*************************************************************************************************/
void PlatformWakeLockInc(void);

/*************************************************************************************************/
/*!
 *  \brief  Increment counter to keep MCU from entering deep sleep.
 *
 *  \return None.
 */
/*************************************************************************************************/
void PlatformWakeLockDec(void);

/*************************************************************************************************/
/*!
 *  \brief  Get status of counter to keep MCU from entering deep sleep.
 *
 *  \return TRUE if MCU prevented from deep sleep.
 */
/*************************************************************************************************/
bool_t PlatformGetWakeLockStatus(void);

/*************************************************************************************************/
/*!
 *  \brief  Initialize sleep handling.
 *
 *  \param  sleepCback        Sleep callback.
 *  \param  wakeCback         Wake callback.
 *
 *  \return None.
 */
/*************************************************************************************************/
void PlatformInitSleep(platformSleepCback_t sleepCback, platformSleepCback_t wakeCback);

/*************************************************************************************************/
/*!
 *  \brief  Sleep or deep sleep for a number of ticks.
 *
 *  \param  ticks     Number of ticks to sleep.
 *  \param  deep      TRUE to enable deep sleep.
 *
 *  \return None.
 */
/*************************************************************************************************/
void PlatformSleep(uint32_t ticks, bool_t deep);

/*************************************************************************************************/
/*!
 *  \brief  Read value of sleep timer.
 *
 *  \return Value of sleep timer.
 */
/*************************************************************************************************/
uint32_t PlatformReadSleepTimer(void);

/*************************************************************************************************/
/*!
 *  \brief  Enable wakeup input.
 *
 *  \param  wakeupCback       Wakeup input callback.
 *
 *  \return None.
 */
/*************************************************************************************************/
void PlatformEnableWakeup(platformWakeupCback_t wakeupCback);

/*************************************************************************************************/
/*!
 *  \brief  Delay for a certain time, in microseconds.
 *
 *  \param  delayUsec  Microseconds to delay.
 *
 *  \return None.
 */
/*************************************************************************************************/
void PlatformDelayUsec(uint32_t delayUsec);

/*************************************************************************************************/
/*!
 *  \brief  Reset after a delay.
 *
 *  \param  delayMs           Delay, in milliseconds.
 *
 *  \return None.
 */
/*************************************************************************************************/
void PlatformReset(uint16_t delayMs);

#endif /* PLATFORM_H */
