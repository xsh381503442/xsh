/*************************************************************************************************/
/*!
 *  \file   cordio_ext_defs.h
 *
 *  \brief  Cordio extended command defines.
 *
 *          $Date: 2016-03-08 09:40:32 -0500 (Tue, 08 Mar 2016) $
 *          $Revision: 6226 $
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

#ifndef CORDIO_EXT_DEFS_H
#define CORDIO_EXT_DEFS_H

/**************************************************************************************************
  Macros
**************************************************************************************************/

/* Cordio test vendor specific OCF range is 0x000-0x00F. */
#define CORDIO_EXT_OCF_BASE                   0x3E0
#define CORDIO_EXT_OCF_END                    0x3FF

#define CORDIO_EXT_OPCODE_VS_SET_SCAN_CH_MAP      HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_EXT_OCF_BASE + 0x00)  /*!< Set Scan Channel Map opcode. */
#define CORDIO_EXT_OPCODE_VS_SET_EVENT_MASK       HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_EXT_OCF_BASE + 0x01)  /*!< Set Vendor Specific Event Mask opcode. */
#define CORDIO_EXT_OPCODE_VS_ENA_ACL_SINK         HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_EXT_OCF_BASE + 0x03)  /*!< Enable ACL Packet Sink opcode. */
#define CORDIO_EXT_OPCODE_VS_GENERATE_ACL         HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_EXT_OCF_BASE + 0x04)  /*!< Generate ACL Packets opcode. */
#define CORDIO_EXT_OPCODE_VS_ENA_AUTO_GEN_ACL     HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_EXT_OCF_BASE + 0x05)  /*!< Enable Auto Generate ACL Packets opcode. */
#define CORDIO_EXT_OPCODE_VS_SET_TX_TEST_ERR_PATT HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_EXT_OCF_BASE + 0x06)  /*!< Set Tx Test Error Pattern opcode. */
#define CORDIO_EXT_OPCODE_VS_SET_CONN_OP_FLAGS    HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_EXT_OCF_BASE + 0x07)  /*!< Set Connection Operation Flags opcode. */
#define CORDIO_EXT_OPCODE_VS_SET_P256_PRIV_KEY    HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_EXT_OCF_BASE + 0x08)  /*!< Set P-256 Private Key opcode. */
#define CORDIO_EXT_OPCODE_VS_GET_ACL_TEST_REPORT  HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_EXT_OCF_BASE + 0x09)  /*!< Get ACL Test Report opcode. */
#define CORDIO_EXT_OPCODE_VS_SET_BD_ADDR          HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_EXT_OCF_BASE + 0x10)  /*!< Set BD Address opcode. */
#define CORDIO_EXT_OPCODE_VS_GET_RAND_ADDR        HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_EXT_OCF_BASE + 0x11)  /*!< Get Random Address opcode. */
#define CORDIO_EXT_OPCODE_VS_SET_LOCAL_FEAT       HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_EXT_OCF_BASE + 0x12)  /*!< Set Local Feature opcode. */
#define CORDIO_EXT_OPCODE_VS_SET_OP_FLAGS         HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_EXT_OCF_BASE + 0x13)  /*!< Set operational flags opcode. */
#define CORDIO_EXT_OPCODE_VS_GET_PDU_FILT_STATS   HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_EXT_OCF_BASE + 0x14)  /*!< Get PDU FIlter Statistics opcode. */
#define CORDIO_EXT_OPCODE_VS_SET_ADV_TX_PWR       HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_EXT_OCF_BASE + 0x15)  /*!< Set Advertising Tx Power opcode. */
#define CORDIO_EXT_OPCODE_VS_SET_CONN_TX_PWR      HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_EXT_OCF_BASE + 0x16)  /*!< Set Connection Tx Power opcode. */
#define CORDIO_EXT_OPCODE_VS_SET_CHAN_MAP         HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_EXT_OCF_BASE + 0x18)  /*!< Set Channel Map opcode. */
#define CORDIO_EXT_OPCDOE_VS_GET_SYS_STATS        HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_EXT_OCF_BASE + 0x1A)  /*!< Get System Statistics opcode. */
#define CORDIO_EXT_OPCDOE_VS_GET_ADV_STATS        HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_EXT_OCF_BASE + 0x1B)  /*!< Get Advertising Statistics opcode. */
#define CORDIO_EXT_OPCDOE_VS_GET_SCAN_STATS       HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_EXT_OCF_BASE + 0x1C)  /*!< Get Scan Statistics opcode. */
#define CORDIO_EXT_OPCDOE_VS_GET_CONN_STATS       HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_EXT_OCF_BASE + 0x1D)  /*!< Get Connection Statistics opcode. */
#define CORDIO_EXT_OPCDOE_VS_GET_TEST_STATS       HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_EXT_OCF_BASE + 0x1E)  /*!< Get Test Statistics opcode. */
#define CORDIO_EXT_OPCDOE_VS_GET_POOL_STATS       HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_EXT_OCF_BASE + 0x1F)  /*!< Get Pool Statistics opcode. */

/*! \brief        Local feature bits. */
enum
{
  CORDIO_EXT_FEAT_ENCRYPTION                     = (1 << 0), /*!< Encryption supported */
  CORDIO_EXT_FEAT_CONN_PARAM_REQ_PROC            = (1 << 1), /*!< Connection Parameters Request Procedure supported */
  CORDIO_EXT_FEAT_EXT_REJECT_IND                 = (1 << 2), /*!< Extended Reject Indication supported */
  CORDIO_EXT_FEAT_SLV_INIT_FEAT_EXCH             = (1 << 3), /*!< Slave-Initiated Features Exchange supported */
  CORDIO_EXT_FEAT_LE_PING                        = (1 << 4), /*!< LE Ping supported */
  CORDIO_EXT_FEAT_DATA_LEN_EXT                   = (1 << 5), /*!< Data Length Extension supported */
  CORDIO_EXT_FEAT_PRIVACY                        = (1 << 6), /*!< LL Privacy supported */
  CORDIO_EXT_FEAT_EXT_SCAN_FILT_POLICY           = (1 << 7)  /*!< Extended Scan Filter Policy supported */
};

/*! \brief        Operational mode flags. */
enum
{
  CORDIO_EXT_OP_MODE_FLAG_ENA_LLCP_STARTUP       = (1 << 0),  /*!< Send LLCP startup messages. */
  CORDIO_EXT_OP_MODE_FLAG_SLV_REQ_IMMED_ACK      = (1 << 1),  /*!< MD bit set if data transmitted. */
  CORDIO_EXT_OP_MODE_FLAG_BYPASS_CE_GUARD        = (1 << 2),  /*!< Bypass end of CE guard. */
  CORDIO_EXT_OP_MODE_FLAG_MST_RETX_AFTER_RX_NACK = (1 << 3),  /*!< Master retransmits after receiving NACK. */
  CORDIO_EXT_OP_MODE_FLAG_MST_IGNORE_CP_RSP      = (1 << 4),  /*!< Master ignores LL_CONNECTION_PARAM_RSP. */
  CORDIO_EXT_OP_MODE_FLAG_MST_UNCOND_CP_RSP      = (1 << 5),  /*!< Master unconditionally accepts LL_CONNECTION_PARAM_RSP. */
                                                              /*!<   (CORDIO_EXT_OP_MODE_FLAG_MST_IGNORE_CP_RSP must be cleared). */
  CORDIO_EXT_OP_MODE_FLAG_ENA_LEN_LLCP_STARTUP   = (1 << 6)   /*!< Perform data length update LLCP at connection establishment. */
};

/*! \brief        Event mask bits. */
enum
{
  CORDIO_EXT_EVT_MASK_SCAN_REPORT_EVT            = (1 << 0),  /*!< Scan report event. */
  CORDIO_EXT_EVT_MASK_DIAG_TRACE_EVT             = (1 << 1)   /*!< Diagnostic trace event. */
};

#endif /* CORDIO_EXT_DEFS_H */
