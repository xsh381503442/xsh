/*************************************************************************************************/
/*!
 *  \file   llc.h
 *
 *  \brief  Main Link Layer Controller (LLC) services.
 *
 *          $Date: 2016-07-14 19:18:04 -0700 (Thu, 14 Jul 2016) $
 *          $Revision: 7832 $
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

#ifndef LLC_H
#define LLC_H

#include "wsf_types.h"

/**************************************************************************************************
  Data Types
**************************************************************************************************/

/*! \brief    LLC status bits. */
enum
{
  LLC_STATUS_BATTERY_DET3V  = (1 << 0),     /*!< 3V supply detected (otherwise 1V supply detected). */
  LLC_STATUS_BATTERY_STATUS = (1 << 1),     /*!< Battery status. */
  LLC_STATUS_V1V_STATUS     = (1 << 2),     /*!< V1V status. */
  LLC_STATUS_ACTIVE_32KXTAL = (1 << 3),     /*!< 32K XTAL active. */
  LLC_STATUS_ACTIVE_32MXTAL = (1 << 4)      /*!< 32M XTAL active. */
};

/*************************************************************************************************/
/*!
 *  \brief  Request the 32-KHz XTAL from LLC.
 *
 *  \param  ena     TRUE if request should be enabled.
 *
 *  \return None.
 */
/*************************************************************************************************/
void LlcRequest32KXtal(bool_t ena);

/*************************************************************************************************/
/*!
 *  \brief  Get status of 32-KHz XTAL request to LLC.
 *
 *  \return TRUE if 32-KHz XTAL is being requested.
 */
/*************************************************************************************************/
bool_t LlcGet32KXtalRequest(void);

/*************************************************************************************************/
/*!
 *  \brief  Request the 32-MHz XTAL from LLC.
 *
 *  \param  ena     TRUE if request should be enabled.
 *
 *  \return None.
 */
/*************************************************************************************************/
void LlcRequest32MXtal(bool_t ena);

/*************************************************************************************************/
/*!
 *  \brief  Get status of 32-MHz XTAL request to LLC.
 *
 *  \return TRUE if 32-MHz XTAL is being requested.
 */
/*************************************************************************************************/
bool_t LlcGet32MXtalRequest(void);

/*************************************************************************************************/
/*!
 *  \brief  Request the switching regulator from LLC.
 *
 *  \param  ena     TRUE if request should be enabled.
 *
 *  \return None.
 */
/*************************************************************************************************/
void LlcRequestSwitchingRegulator(bool_t ena);

/*************************************************************************************************/
/*!
 *  \brief  Get status of switching regulator request to LLC.
 *
 *  \return TRUE if switching regulator is being requested.
 */
/*************************************************************************************************/
bool_t LlcGetSwitchingRegulatorRequest(void);

/*************************************************************************************************/
/*!
 *  \brief  Request the battery status from LLC.
 *
 *  \param  ena     TRUE if request should be enabled.
 *
 *  \return None.
 */
/*************************************************************************************************/
void LlcRequestBatteryStatus(bool_t ena);

/*************************************************************************************************/
/*!
 *  \brief  Get status of battery status request to LLC.
 *
 *  \return TRUE if battery status is being requested.
 */
/*************************************************************************************************/
bool_t LlcGetBatteryStatusRequest(void);

/*************************************************************************************************/
/*!
 *  \brief  Get LLC status flags.
 *
 *  \return Status flags.
 */
/*************************************************************************************************/
uint16_t LlcGetStatusFlags(void);

/*************************************************************************************************/
/*!
 *  \brief  Apply or remove reset to LLC.
 *
 *  \param  on      TRUE if reset should be applied.
 *
 *  \return None.
 */
/*************************************************************************************************/
void LlcReset(bool_t on);

/*************************************************************************************************/
/*!
 *  \brief  Disable LLC interrupts.
 *
 *  \return None.
 */
/*************************************************************************************************/
void LlcDisableInterrupts(void);

/*************************************************************************************************/
/*!
 *  \brief  Enable LLC interrupts.
 *
 *  \return None.
 */
/*************************************************************************************************/
void LlcEnableInterrupts(void);

#endif /* LLC_H */
