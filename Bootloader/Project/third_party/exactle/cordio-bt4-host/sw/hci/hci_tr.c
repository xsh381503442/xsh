/*************************************************************************************************/
/*!
 *  \file   hci_tr.c
 *
 *  \brief  HCI transport module for BT4.
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

#include "wsf_types.h"
#include "wsf_trace.h"
#include "bstream.h"
#include "hci_api.h"
#include "hci_core.h"
#include "hci_tr.h"

#include "llc_hci.h"

/*************************************************************************************************/
/*!
 *  \fn     hciTrSendAclData
 *
 *  \brief  Send a complete HCI ACL packet to the transport.
 *
 *  \param  pContext Connection context.
 *  \param  pData    WSF msg buffer containing an ACL packet.
 *
 *  \return None.
 */
/*************************************************************************************************/
void hciTrSendAclData(void *pContext, uint8_t *pData)
{
  uint16_t   len;

  /* get 16-bit length */
  BYTES_TO_UINT16(len, &pData[2]);
  len += HCI_ACL_HDR_LEN;

  /* transmit ACL header and data */
  LlcHciWrite(HCI_ACL_TYPE, pData, len, pContext);
}

/*************************************************************************************************/
/*!
 *  \fn     hciTrSendCmd
 *
 *  \brief  Send a complete HCI command to the transport.
 *
 *  \param  pData    WSF msg buffer containing an HCI command.
 *
 *  \return None.
 */
/*************************************************************************************************/
void hciTrSendCmd(uint8_t *pData)
{
  uint8_t   len;

  /* get length */
  len = pData[2] + HCI_CMD_HDR_LEN;

  /* dump event for protocol analysis */
  HCI_PDUMP_CMD(len, pData);

  /* transmit ACL header and data */
  LlcHciWrite(HCI_CMD_TYPE, pData, len, NULL);
}
