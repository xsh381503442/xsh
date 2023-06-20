/*************************************************************************************************/
/*!
 *  \file   llc_init.h
 *
 *  \brief  Main Link Layer Controller (LLC) initialization.
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

#ifndef LLC_INIT_H
#define LLC_INIT_H

#include "wsf_types.h"

/**************************************************************************************************
  Data Types
**************************************************************************************************/

/*! \brief      Data provider. */
typedef bool_t (*LlcInitDataProvider_t)(uint32_t *pAddr, uint16_t len, uint8_t *pBuf);

/*************************************************************************************************/
/*!
 *  \brief  Set address and provider for firmware.
 *
 *  \param  addr            Address.
 *  \param  provider        Provider for firmware data.
 *
 *  \return None.
 */
/*************************************************************************************************/
void LlcInitSetFirmwareProvider(uint32_t addr, LlcInitDataProvider_t provider);

/*************************************************************************************************/
/*!
 *  \brief  Set address and provider for trim.
 *
 *  \param  addr            Address.
 *  \param  provider        Provider for trim data.
 *
 *  \return None.
 */
/*************************************************************************************************/
void LlcInitSetTrimProvider(uint32_t addr, LlcInitDataProvider_t provider);

/*************************************************************************************************/
/*!
 *  \brief  Set address and provider for patch.
 *
 *  \param  addr            Address.
 *  \param  provider        Provider for patch data.
 *
 *  \return None.
 */
/*************************************************************************************************/
void LlcInitSetPatchProvider(uint32_t addr, LlcInitDataProvider_t provider);

/*************************************************************************************************/
/*!
 *  \brief  Set transmit power.
 *
 *  \param  txPowerLevel    Transmit power level.
 *
 *  \return None.
 */
/*************************************************************************************************/
void LlcInitSetTxPower(int8_t txPowerLevel);

/*************************************************************************************************/
/*!
 *  \brief  Set timeout for LLC responses.
 *
 *  \param  timeout         Timeout.
 *
 *  \return None.
 */
/*************************************************************************************************/
void LlcInitSetTimeout(uint8_t timeout);

/*************************************************************************************************/
/*!
 *  \brief  Perform LLC initialization.
 *
 *  \return None.
 */
/*************************************************************************************************/
bool_t LlcInit(void);

#endif /* LLC_INIT_H */
