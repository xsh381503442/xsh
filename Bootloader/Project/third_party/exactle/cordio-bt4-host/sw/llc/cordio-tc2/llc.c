/*************************************************************************************************/
/*!
 *  \file   llc.c
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

#include "SMD_TC2.h"

#include "wsf_types.h"
#include "llc.h"

/*************************************************************************************************/
/*!
 *  \brief  Request the 32-KHz XTAL from LLC.
 *
 *  \param  ena     TRUE if request should be enabled.
 *
 *  \return None.
 */
/*************************************************************************************************/
void LlcRequest32KXtal(bool_t ena)
{
  if (ena)
  {
    /* Request but do not wait for ready. */
    POWERCON->TIMER_32K_CONTROL |= POWERCON_TIMER_32K_CONTROL_XTAL_REQUEST;
  }
  else
  {
    POWERCON->TIMER_32K_CONTROL &= ~POWERCON_TIMER_32K_CONTROL_XTAL_REQUEST;
  }
}

/*************************************************************************************************/
/*!
 *  \brief  Get status of 32-KHz XTAL request to LLC.
 *
 *  \return TRUE if 32-KHz XTAL is being requested.
 */
/*************************************************************************************************/
bool_t LlcGet32KXtalRequest(void)
{
  return ((POWERCON->TIMER_32K_CONTROL & POWERCON_TIMER_32K_CONTROL_XTAL_REQUEST) == 0) ? FALSE : TRUE;
}

/*************************************************************************************************/
/*!
 *  \brief  Request the 32-MHz XTAL from LLC.
 *
 *  \param  ena     TRUE if request should be enabled.
 *
 *  \return None.
 */
/*************************************************************************************************/
void LlcRequest32MXtal(bool_t ena)
{
  if (ena)
  {
    /* Request but do not wait for ready. */
    POWERCON->LLCCTRL |= POWERCON_LLCCTRL_32M_XTAL_REQUEST;
  }
  else
  {
    POWERCON->LLCCTRL &= ~POWERCON_LLCCTRL_32M_XTAL_REQUEST;
  }
}

/*************************************************************************************************/
/*!
 *  \brief  Get status of 32-MHz XTAL request to LLC.
 *
 *  \return TRUE if 32-MHz XTAL is being requested.
 */
/*************************************************************************************************/
bool_t LlcGet32MXtalRequest(void)
{
  return ((POWERCON->LLCCTRL & POWERCON_LLCCTRL_32M_XTAL_REQUEST) == 0) ? FALSE : TRUE;
}

/*************************************************************************************************/
/*!
 *  \brief  Request the switching regulator from LLC.
 *
 *  \param  ena     TRUE if request should be enabled.
 *
 *  \return None.
 */
/*************************************************************************************************/
void LlcRequestSwitchingRegulator(bool_t ena)
{
  if (ena)
  {
    POWERCON->TIMER_32K_CONTROL |= POWERCON_TIMER_32K_CONTROL_SWITCHING_REGULATOR_REQUEST;
  }
  else
  {
    POWERCON->TIMER_32K_CONTROL &= ~POWERCON_TIMER_32K_CONTROL_SWITCHING_REGULATOR_REQUEST;
  }
}

/*************************************************************************************************/
/*!
 *  \brief  Get status of switching regulator request to LLC.
 *
 *  \return TRUE if switching regulator is being requested.
 */
/*************************************************************************************************/
bool_t LlcGetSwitchingRegulatorRequest(void)
{
  return ((POWERCON->TIMER_32K_CONTROL & POWERCON_TIMER_32K_CONTROL_SWITCHING_REGULATOR_REQUEST) == 0) ? FALSE : TRUE;
}

/*************************************************************************************************/
/*!
 *  \brief  Request the battery status from LLC.
 *
 *  \param  ena     TRUE if request should be enabled.
 *
 *  \return None.
 */
/*************************************************************************************************/
void LlcRequestBatteryStatus(bool_t ena)
{
  if (ena)
  {
    POWERCON->LLCCTRL |= POWERCON_LLCCTRL_BATTERY_STATUS_REQUEST;
  }
  else
  {
    POWERCON->LLCCTRL &= ~POWERCON_LLCCTRL_BATTERY_STATUS_REQUEST;
  }
}

/*************************************************************************************************/
/*!
 *  \brief  Get status of battery status request to LLC.
 *
 *  \return TRUE if battery status is being requested.
 */
/*************************************************************************************************/
bool_t LlcGetBatteryStatusRequest(void)
{
  return ((POWERCON->LLCCTRL & POWERCON_LLCCTRL_BATTERY_STATUS_REQUEST) == 0) ? FALSE : TRUE;
}

/*************************************************************************************************/
/*!
 *  \brief  Get LLC status flags.
 *
 *  \return Status flags.
 */
/*************************************************************************************************/
uint16_t LlcGetStatusFlags(void)
{
  return (POWERCON->LLCSTATUS & 0x1F);    /* bit definition coincides */
}

/*************************************************************************************************/
/*!
 *  \brief  Apply or remove reset to LLC.
 *
 *  \param  on      TRUE if reset should be applied.
 *
 *  \return None.
 */
/*************************************************************************************************/
void LlcReset(bool_t on)
{
  if (on)
  {
    POWERCON->LLCCTRL &= ~POWERCON_LLCCTRL_RESET;
  }
  else
  {
    POWERCON->LLCCTRL |= POWERCON_LLCCTRL_RESET;
  }
}

/*************************************************************************************************/
/*!
 *  \brief  Disable LLC interrupts.
 *
 *  \return None.
 */
/*************************************************************************************************/
void LlcDisableInterrupts(void)
{
  NVIC_DisableIRQ(LLCC_RXCMD_VALID_IRQn);
  NVIC_DisableIRQ(LLCC_RXEVT_VALID_IRQn);
  NVIC_DisableIRQ(LLCC_TXDMAL_DONE_IRQn);
  NVIC_DisableIRQ(LLCC_RXDMAL_DONE_IRQn);
}

/*************************************************************************************************/
/*!
 *  \brief  Enable LLC interrupts.
 *
 *  \return None.
 */
/*************************************************************************************************/
void LlcEnableInterrupts(void)
{
  NVIC_SetPriority(LLCC_RXCMD_VALID_IRQn, 2);
  NVIC_ClearPendingIRQ(LLCC_RXCMD_VALID_IRQn);
  NVIC_EnableIRQ(LLCC_RXCMD_VALID_IRQn);
  NVIC_SetPriority(LLCC_RXEVT_VALID_IRQn, 2);
  NVIC_ClearPendingIRQ(LLCC_RXEVT_VALID_IRQn);
  NVIC_EnableIRQ(LLCC_RXEVT_VALID_IRQn);
  NVIC_SetPriority(LLCC_TXDMAL_DONE_IRQn, 2);
  NVIC_ClearPendingIRQ(LLCC_TXDMAL_DONE_IRQn);
  NVIC_EnableIRQ(LLCC_TXDMAL_DONE_IRQn);
  NVIC_SetPriority(LLCC_RXDMAL_DONE_IRQn, 2);
  NVIC_ClearPendingIRQ(LLCC_RXDMAL_DONE_IRQn);
  NVIC_EnableIRQ(LLCC_RXDMAL_DONE_IRQn);
}
