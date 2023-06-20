/*************************************************************************************************/
/*!
 *  \file   cordio_host_defs.h
 *
 *  \brief  Cordio host command defines.
 *
 *          $Date: 2015-11-01 11:26:53 -0500 (Sun, 01 Nov 2015) $
 *          $Revision: 4357 $
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

#ifndef CORDIO_HOST_DEFS_H
#define CORDIO_HOST_DEFS_H

/**************************************************************************************************
  Macros
**************************************************************************************************/

/* Cordio host vendor specific OCF range is 0x000-0x00F. */
#define CORDIO_HOST_OCF_BASE                      0x200
#define CORDIO_HOST_OCF_END                       0x22F

#define CORDIO_HOST_OPCODE_NV_READ                HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_HOST_OCF_BASE + 0x06)  /*!< Non-Volatile Memory Read command. */
#define CORDIO_HOST_OPCODE_NV_WRITE               HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_HOST_OCF_BASE + 0x07)  /*!< Non-Volatile Memory Write command. */
#define CORDIO_HOST_OPCODE_PMU_REQ                HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_HOST_OCF_BASE + 0x0A)  /*!< PMU Request command.  */
#define CORDIO_HOST_OPCODE_SET_MUX_MODE           HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_HOST_OCF_BASE + 0x11)  /*!< Set Pin Mux Mode command. */
#define CORDIO_HOST_OPCODE_HOST_SLEEP             HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_HOST_OCF_BASE + 0x12)  /*!< Host Sleep command. */
#define CORDIO_HOST_OPCODE_HOST_RESET             HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_HOST_OCF_BASE + 0x13)  /*!< Host Reset command. */
#define CORDIO_HOST_OPCODE_NV_ERASE               HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_HOST_OCF_BASE + 0x17)  /*!< Non-Volatile Memory Erase command. */
#define CORDIO_HOST_OPCODE_ENA_ACL_SINK           HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_HOST_OCF_BASE + 0x18)  /*!< ACL Data Sink command. */
#define CORDIO_HOST_OPCODE_TEST_LOAD              HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_HOST_OCF_BASE + 0x19)  /*!< Load and Execute Test command. */
#define CORDIO_HOST_OPCODE_GENERATE_ACL           HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_HOST_OCF_BASE + 0x1A)  /*!< Generate ACL Data command. */
#define CORDIO_HOST_OPCODE_ENA_AUTO_GENERATE_ACL  HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_HOST_OCF_BASE + 0x1B)  /*!< Auto-generate ACL Data command. */
#define CORDIO_HOST_OPCODE_GET_ACL_TEST_REPORT    HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_HOST_OCF_BASE + 0x1C)  /*!< Get ACL Test Report command. */
#define CORDIO_HOST_OPCODE_START_SLEEP_TIMER      HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_HOST_OCF_BASE + 0x1E)  /*!< Start Sleep Timer command. */
#define CORDIO_HOST_OPCODE_ENA_ADV_SINK           HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_HOST_OCF_BASE + 0x20)  /*!< Advertisement Sink command. */
#define CORDIO_HOST_OPCODE_GET_SCAN_TEST_REPORT   HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_HOST_OCF_BASE + 0x21)  /*!< Get Scan Test Report command. */
#define CORDIO_HOST_OPCODE_GET_TX_TRIM_OPTS       HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_HOST_OCF_BASE + 0x22)  /*!< Get Transmit Trim Options command. */
#define CORDIO_HOST_OPCODE_SET_TX_TRIM            HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_HOST_OCF_BASE + 0x23)  /*!< Set Transmit Trim command. */
#define CORDIO_HOST_OPCODE_SIGNAL_RESPONSE        HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_HOST_OCF_BASE + 0x24)  /*!< Tester Signal Response command. */
#define CORDIO_HOST_OPCODE_SIGNAL_BOOL_RESPONSE   HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_HOST_OCF_BASE + 0x25)  /*!< Tester Signal Boolean Response command. */

/* Cordio host vendor-specific event code range is 0xFE10-0xFE1F. */
#define CORDIO_HOST_EVT_BASE                      0xFE10
#define CORDIO_HOST_EVT_END                       0xFE1F

#define CORDIO_HOST_EVT_TEST_COMPLETE             (CORDIO_HOST_EVT_BASE + 0x00)   /*!< Test Complete event. */
#define CORDIO_HOST_LEN_EVT_TEST_COMPLETE(len)    (2 + (len))
#define CORDIO_HOST_EVT_SLEEP_TIMER_EXPIRED       (CORDIO_HOST_EVT_BASE + 0x01)   /*!< Sleep Timer Expired event. */
#define CORDIO_HOST_LEN_EVT_SLEEP_TIMER_EXPIRED   1
#define CORDIO_HOST_LEN_VS_EVT                    2
#define CORDIO_HOST_EVT_SIGNAL_REQUEST            (CORDIO_HOST_EVT_BASE + 0x02)   /*!< Tester Signal Request event.  */
#define CORDIO_HOST_EVT_SIGNAL_BOOL_REQUEST       (CORDIO_HOST_EVT_BASE + 0x03)   /*!< Tester Signal Boolean Request event.  */

/*! \brief    PMU request defines. */
enum
{
  CORDIO_HOST_PMU_REQ_SWITCHING_REGULATOR = (1 << 2),   /*!< Request switching regulator. */
  CORDIO_HOST_PMU_REQ_32M_XTAL            = (1 << 4)    /*!< Request 32M XTAL. */
};

/*! \brief    Test defines. */
enum
{
  CORDIO_HOST_TEST_PMU_TRIM       = 0,                  /*!< PMU Trim. */
  CORDIO_HOST_TEST_CLOCKS_TEST    = 1,                  /*!< Clocks Test. */
  CORDIO_HOST_TEST_VCO_ALT        = 2,                  /*!< VCO Tune (Alternate). */
  CORDIO_HOST_TEST_RXFE1          = 3,                  /*!< RXFE LNA Trim, Image Trim, IQ Cal, DCOC. */
  CORDIO_HOST_TEST_TX_TRIM_ALT    = 4,                  /*!< Tx Trim (3V or 1V). */
  CORDIO_HOST_TEST_CURRENT        = 5,                  /*!< Block Currents. */
  CORDIO_HOST_TEST_VOLTAGES       = 6,                  /*!< Block Voltages. */
  CORDIO_HOST_TEST_SLEEP          = 7,                  /*!< Sleep (1V and 3V). */
  CORDIO_HOST_PASS_PMU_PARAMS     = 8,	                /*!< pass in PMU trim values for 1V. */
  CORDIO_HOST_TEST_TX_TRIM1       = 9,                  /*!< Tx Trim Part 1 (3V or 1V). */
  CORDIO_HOST_TEST_TX_TRIM2       = 10,                 /*!< Tx Trim Part 2 (3V or 1V). */
  CORDIO_HOST_TEST_TX_FINE_TRIM   = 11,                 /*!< Tx Fine trim to follow Alternate Part 2. */
  CORDIO_HOST_TEST_CLOCK_CAPS     = 12,                 /*!< Clock Caps Test. */
  CORDIO_HOST_TEST_CLOCK_CAL      = 13,                 /*!< Internal Xtal Clock Cal  */
  CORDIO_HOST_TEST_BATTERY_COMPS  = 14,                 /*!< Trim Battery Comparators */
  CORDIO_HOST_TEST_RXFE2          = 15,                 /*!< RXFE AGC Test, Mod Trim */
  CORDIO_HOST_TEST_32MRC          = 16,                 /*!< Test tuning of 32M RC */
  CORDIO_HOST_TEST_PMU_TRIM_ALT   = 17,                 /*!< PMU Trim (Alternate). */
  CORDIO_HOST_TEST_VCO            = 18,                 /*!< VCO Tune. */
  CORDIO_HOST_TEST_NUM            = 19
};

#define CORDIO_HOST_MAX_TX_TRIM_OPTS  8                 /*!< Maximum transmit trim options. */

#endif /* CORDIO_HOST_DEFS_H */
