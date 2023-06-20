/*************************************************************************************************/
/*!
 *  \file   hci_vs.h
 *
 *  \brief  HCI vendor specific functionality for Cordio BT4.
 *
 *          $Date: 2016-08-28 14:40:28 -0700 (Sun, 28 Aug 2016) $
 *          $Revision: 8609 $
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
#ifndef HCI_VS_H
#define HCI_VS_H

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************************
  Function Declarations
**************************************************************************************************/

/*! Configuration interface. */
void HciVsCfgBdAddr(bdAddr_t addr);
void HciVsCfgAclBufs(uint8_t numTxBufs, uint8_t numRxBufs, uint16_t maxAclLen);
void HciVsCfgVersion(uint16_t compId, uint16_t implRev, uint8_t btVer);
void HciVsCfgDevFilt(uint8_t whiteListSize, uint8_t resolvingListSize);
void HciVsCfgDefTxPwrLvl(int8_t defTxPwrLvl);
void HciVsCfgCeJitter(uint8_t ceJitterUsec);
void HciVsCfgMaxConn(uint8_t maxConn);
void HciVsCfgDtmRxSync(uint16_t dtmRxSyncMs);
void HciVsCfgBufs(uint8_t poolIdx, uint8_t numBufs, uint16_t bufSize);
void HciVsCfgMaxScanPeriod(uint16_t maxScanPeriodMs);
void HciVsCfgMaxAdvReports(uint8_t maxAdvReports);

/*! Command interface. */
void HciVsSetBdAddrCmd(bdAddr_t addr);
void HciVsSetAclBufsCmd(uint8_t numTxBufs, uint8_t numRxBufs, uint16_t maxAclLen);
void HciVsSetVersionCmd(uint16_t compId, uint16_t implRev, uint8_t btVer);
void HciVsSetDevFiltCmd(uint8_t whiteListSize, uint8_t resolvingListSize);
void HciVsSetDefTxPwrLvlCmd(int8_t defTxPwrLvl);
void HciVsSetCeJitterCmd(uint8_t ceJitter);
void HciVsSetMaxConnCmd(uint8_t maxConn);
void HciVsSetDtmRxSyncCmd(uint16_t dtmRxSyncMs);
void HciVsSetBufsCmd(uint8_t poolIdx, uint8_t numBufs, uint16_t bufSize);
void HciVsSetMaxScanPeriodCmd(uint16_t maxScanPeriodMs);
void HciVsSetMaxAdvReportsCmd(uint8_t maxAdvReports);

#ifdef __cplusplus
};
#endif

#endif /* HCI_VS_H */
