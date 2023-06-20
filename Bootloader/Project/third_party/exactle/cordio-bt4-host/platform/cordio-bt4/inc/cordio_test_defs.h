/*************************************************************************************************/
/*!
 *  \file   cordio_test_defs.h
 *
 *  \brief  Cordio test command defines.
 *
 *          $Date: 2016-02-23 19:17:27 -0500 (Tue, 23 Feb 2016) $
 *          $Revision: 5966 $
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

#ifndef CORDIO_TEST_DEFS_H
#define CORDIO_TEST_DEFS_H

/**************************************************************************************************
  Macros
**************************************************************************************************/

/* Cordio test vendor specific OCF range is 0x000-0x00F. */
#define CORDIO_TEST_OCF_BASE                  0x200
#define CORDIO_TEST_OCF_END                   0x22F

#define CORDIO_TEST_OPCODE_WRITE_REG          HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_TEST_OCF_BASE + 0x00)  /*!< Write Radio Register command. */
#define CORDIO_TEST_OPCODE_READ_REG           HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_TEST_OCF_BASE + 0x01)  /*!< Read Radio Register command. */
#define CORDIO_TEST_OPCODE_SET_POWER_MGMT     HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_TEST_OCF_BASE + 0x02)  /*!< Set Power Management Mode command. */
#define CORDIO_TEST_OPCODE_GET_BASEBAND_STATS HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_TEST_OCF_BASE + 0x08)  /*!< Get Baseband Statistics command. */
#define CORDIO_TEST_OPCODE_KEEP_RADIO_REGS    HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_TEST_OCF_BASE + 0x09)  /*!< Keep Radio Registers command. */
#define CORDIO_TEST_OPCODE_CONTINUOUS_RX      HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_TEST_OCF_BASE + 0x0E)  /*!< Continuous Receive command. */
#define CORDIO_TEST_OPCODE_CONTINUOUS_TX      HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_TEST_OCF_BASE + 0x0F)  /*!< Continuous Transmit command. */
#define CORDIO_TEST_OPCODE_GET_VERSION        HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_TEST_OCF_BASE + 0x10)  /*!< Get Version Informaiton command. */
#define CORDIO_TEST_OPCODE_GET_LAST_RSSI      HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_TEST_OCF_BASE + 0x14)  /*!< Get Last RSSI command. */
#define CORDIO_TEST_OPCODE_EXEC_PATCH         HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_TEST_OCF_BASE + 0x18)  /*!< Execute Patched Function command. */
#define CORDIO_TEST_OPCODE_SET_OP_FLAGS       HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_TEST_OCF_BASE + 0x1D)  /*!< Set Operational Flags command. */

/*! /brief    Length of version string. */
#define CORDIO_TEST_VERSION_STRING_LEN        40
#define CORDIO_TEST_GET_VERSION_RSP_LEN      (CORDIO_TEST_VERSION_STRING_LEN + 4 * sizeof(uint8_t))

/*! /brief    Power management modes. */
enum
{
  CORDIO_TEST_POWER_MGMT_OFF       = 0,             /*!< No power management. */
  CORDIO_TEST_POWER_MGMT_CLOCK     = 1,             /*!< Disable clocks when not needed. */
  CORDIO_TEST_POWER_MGMT_SLEEP     = 2,             /*!< Enter sleep. */
  CORDIO_TEST_POWER_MGMT_DEEPSLEEP = 3              /*!< Enter deep sleep. */
};

/*! /brief      Operational flags. */
enum
{
  CORDIO_TEST_OP_FLAG_RX_AUTO_RESTART       = (1 << 0),   /* Auto-restart Rx after CRC failure on an advertising PDU. */
  CORDIO_TEST_OP_FLAG_ENA_SWITCHER_IN_SLEEP = (1 << 1),   /* Enable switcher during deep sleep. */
  CORDIO_TEST_OP_FLAG_DIS_32K_TICK_COMP     = (1 << 2)    /* Disable compensation for 32kHz timer. */
};

#endif /* CORDIO_TEST_DEFS_H */
