/*************************************************************************************************/
/*!
 *  \file   cordio_tc2.h
 *
 *  \brief  Cordio TC2.
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

#ifndef CORDIO_TC2_H
#define CORDIO_TC2_H

#include "wsf_types.h"

/**************************************************************************************************
  Data Types
**************************************************************************************************/

/*! \brief  Parameters. */
enum
{
  CORDIO_TC2_PARAM_TX_POWER_LEVEL_1V = 100,
  CORDIO_TC2_PARAM_TX_POWER_LEVEL_3V = 101
};

/*************************************************************************************************/
/*!
 *  \brief  Find BT4 firmware.
 *
 *  \param  pAddr       Storage for firmware address.
 *
 *  \return TRUE if firmware found.
 */
/*************************************************************************************************/
bool_t CordioTc2FindBt4Firmware(uint32_t *pAddr);

/*************************************************************************************************/
/*!
 *  \brief  Find BT4 patch.
 *
 *  \param  pAddr       Storage for patch address.
 *
 *  \return TRUE if patch found.
 */
/*************************************************************************************************/
bool_t CordioTc2FindBt4Patch(uint32_t *pAddr);

/*************************************************************************************************/
/*!
 *  \brief  Get size of NV data.
 *
 *  \return Size of NV data.
 */
/*************************************************************************************************/
uint32_t CordioTc2NvGetSize(void);

/*************************************************************************************************/
/*!
 *  \brief  Read from NV data.
 *
 *  \param  addr      Address to read.
 *  \param  pData     Storage for data.
 *  \param  len       Length of data.
 *
 *  \return Number of bytes read.
 */
/*************************************************************************************************/
uint32_t CordioTc2NvRead(uint32_t addr, uint8_t *pData, uint32_t len);

/*************************************************************************************************/
/*!
 *  \brief  Write to NV data.
 *
 *  \param  addr      Address to write.
 *  \param  pData     Data.
 *  \param  len       Length of data.
 *
 *  \return Number of bytes written.
 */
/*************************************************************************************************/
uint32_t CordioTc2NvWrite(uint32_t addr, const uint8_t *pData, uint32_t len);

/*************************************************************************************************/
/*!
 *  \brief  Erase NV data.
 *
 *  \param  addr      Address to erase.
 *
 *  \return None.
 */
/*************************************************************************************************/
void CordioTc2NvErase(void);

/*************************************************************************************************/
/*!
 *  \brief  Check for trim in NV data.
 *
 *  \return TRUE if NV data contains trim.
 */
/*************************************************************************************************/
bool_t CordioTc2NvCheckForTrim(void);

/*************************************************************************************************/
/*!
 *  \brief  Find transmit power options.
 *
 *  \param  maxOpts     Maximum number of options.
 *  \param  pBuf        Storage for (Voltage_Mode, Tx_Power_Level) pairs.
 *
 *  \return Number of options.
 */
/*************************************************************************************************/
uint8_t CordioTc2FindTxPowerOpts(uint8_t maxOpts, uint8_t *pBuf);

/*************************************************************************************************/
/*!
 *  \brief  Set transmit power.
 *
 *  \param  voltageMode     Voltage mode (1 or 3).
 *  \param  txPowerLevel    Transmit power level.
 *
 *  \return None.
 */
/*************************************************************************************************/
void CordioTc2SetTxPower(uint8_t voltageMode, int8_t txPowerLevel);

/*************************************************************************************************/
/*!
 *  \brief  Get transmit power.
 *
 *  \param  voltageMode     Voltage mode (1 or 3).
 *  \param  pTxPoweLevel    Storage for transmit power level.
 *
 *  \return None.
 */
/*************************************************************************************************/
bool_t CordioTc2GetTxPower(uint8_t voltageMode, int8_t *pTxPowerLevel);

/*************************************************************************************************/
/*!
 *  \brief  Determine whether power option exists for voltage mode..
 *
 *  \param  voltageMode     Voltage mode (1 or 3).
 *  \param  txPowerLevel    Transmit power level.
 *
 *  \return TRUE if option exists.
 */
/*************************************************************************************************/
bool_t CordioTc2TxPowerOptExists(uint8_t voltageMode, int8_t txPowerLevel);

/*************************************************************************************************/
/*!
 *  \brief  Perform LLC initialization.
 *
 *  \return TRUE if LLC initialization succeeds.
 */
/*************************************************************************************************/
bool_t CordioTc2LlcInit(void);

#endif /* CORDIO_TC2_H */
