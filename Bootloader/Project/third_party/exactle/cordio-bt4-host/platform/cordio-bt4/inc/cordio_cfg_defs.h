/*************************************************************************************************/
/*!
 *  \file   cordio_cfg_defs.h
 *
 *  \brief  Cordio configuration command defines.
 *
 *          $Date: 2015-12-20 21:12:02 -0500 (Sun, 20 Dec 2015) $
 *          $Revision: 4935 $
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

#ifndef CORDIO_CFG_DEFS_H
#define CORDIO_CFG_DEFS_H

/**************************************************************************************************
  Macros
**************************************************************************************************/

/* Cordio test vendor specific OCF range is 0x000-0x00F. */
#define CORDIO_CFG_OCF_BASE                   0x380
#define CORDIO_CFG_OCF_END                    0x38F

#define CORDIO_CFG_OPCODE_VS_GET_STATUS             HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_CFG_OCF_BASE + 0x00) /*!< Get status of bootloader. */
#define CORDIO_CFG_OPCODE_VS_SET_ACL_BUFS           HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_CFG_OCF_BASE + 0x01) /*!< Set ACL Buffers opcode. */
#define CORDIO_CFG_OPCODE_VS_SET_VERSION            HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_CFG_OCF_BASE + 0x02) /*!< Set Version opcode. */
#define CORDIO_CFG_OPCODE_VS_SET_DEV_FILT           HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_CFG_OCF_BASE + 0x03) /*!< Set Device Filters opcode. */
#define CORDIO_CFG_OPCODE_VS_SET_DEF_TX_PWR_LVL     HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_CFG_OCF_BASE + 0x04) /*!< Set Default Tx Power Level opcode. */
#define CORDIO_CFG_OPCODE_VS_SET_CE_JITTER          HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_CFG_OCF_BASE + 0x05) /*!< Set CE Jitter opcode. */
#define CORDIO_CFG_OPCODE_VS_SET_MAX_CONN           HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_CFG_OCF_BASE + 0x06) /*!< Set Maximum Number Connections opcode. */
#define CORDIO_CFG_OPCODE_VS_SET_DTM_RX_SYNC        HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_CFG_OCF_BASE + 0x07) /*!< Set DTM Rx Sync Window opcode. */
#define CORDIO_CFG_OPCODE_VS_SET_BUFS               HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_CFG_OCF_BASE + 0x08) /*!< Set Buffers opcode. */
#define CORDIO_CFG_OPCODE_VS_SET_CLK_ACCURACY       HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_CFG_OCF_BASE + 0x09) /*!< Set Clock Accuracy opcode. */
#define CORDIO_CFG_OPCODE_VS_SET_MAX_SCAN_PERIOD    HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_CFG_OCF_BASE + 0x0A) /*!< Set Max Scan Period opcode. */
#define CORDIO_CFG_OPCODE_VS_SET_MAX_ADV_REPORT     HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_CFG_OCF_BASE + 0x0B) /*!< Set Max Number of Pending Advertising Reports opcode. */
#define CORDIO_CFG_OPCODE_VS_SET_SCH_SETUP_DELAY    HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_CFG_OCF_BASE + 0x0C) /*!< Set Scheduler Setup Delay opcode. */

#endif /* CORDIO_CFG_DEFS_H */
