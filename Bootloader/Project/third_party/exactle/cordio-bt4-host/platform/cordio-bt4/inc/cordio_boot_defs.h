/*************************************************************************************************/
/*!
 *  \file   cordio_boot_defs.h
 *
 *  \brief  Cordio boot command defines.
 *
 *          $Date: 2016-03-20 21:31:03 -0400 (Sun, 20 Mar 2016) $
 *          $Revision: 6412 $
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

#ifndef CORDIO_BOOT_DEFS_H
#define CORDIO_BOOT_DEFS_H

/**************************************************************************************************
  Macros
**************************************************************************************************/

/* Cordio boot vendor specific OCF range is 0x000-0x00F. */
#define CORDIO_BOOT_OCF_BASE                0x000
#define CORDIO_BOOT_OCF_END                 0x00F

#define CORDIO_BOOT_OPCODE_FW_LOAD          HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_BOOT_OCF_BASE + 0x01)   /*!< Firmware load command. */
#define CORDIO_BOOT_OPCODE_FW_DATA          HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_BOOT_OCF_BASE + 0x02)   /*!< Firmware data command. */
#define CORDIO_BOOT_OPCODE_TRIM_LOAD        HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_BOOT_OCF_BASE + 0x03)   /*!< Trim load command.  */
#define CORDIO_BOOT_OPCODE_TRIM_DATA        HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_BOOT_OCF_BASE + 0x04)   /*!< Trim data command. */
#define CORDIO_BOOT_OPCODE_TEST_EXEC        HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_BOOT_OCF_BASE + 0x05)   /*!< Execute test. */
#define CORDIO_BOOT_OPCODE_PATCH_RESERVE    HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_BOOT_OCF_BASE + 0x06)   /*!< Reserve area for patch. */
#define CORDIO_BOOT_OPCODE_ENA_STATUS_GPO   HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_BOOT_OCF_BASE + 0x07)   /*!< Enable status GPOs. */
#define CORDIO_BOOT_OPCODE_ENA_ISR_GPO      HCI_OPCODE(HCI_OGF_VENDOR_SPEC, CORDIO_BOOT_OCF_BASE + 0x08)   /*!< Enable ISR GPOs. */

#define CORDIO_BOOT_LEN_FW_LOAD             18
#define CORDIO_BOOT_LEN_FW_DATA(dataLen)    (dataLen)
#define CORDIO_BOOT_LEN_TRIM_LOAD           4
#define CORDIO_BOOT_LEN_TRIM_DATA(dataLen)  (dataLen)
#define CORDIO_BOOT_LEN_TEST_EXEC(paramLen) (paramLen)
#define CORDIO_BOOT_LEN_PATCH_RESERVE       4
#define CORDIO_BOOT_DATA_LEN_MAX            252
#define CORDIO_BOOT_LEN_EN_STATUS_GPO       0
#define CORDIO_BOOT_LEN_VERSION_STRING      40

/* Cordio boot vendor-specific event code range is 0xFE80-0xFE8F. */
#define CORDIO_BOOT_EVT_BASE                0xFE80
#define CORDIO_BOOT_EVT_END                 0xFE8F

#define CORDIO_BOOT_EVT_STARTUP             (CORDIO_BOOT_EVT_BASE + 0x00)   /*!< Startup event. */
#define CORDIO_BOOT_LEN_EVT_STARTUP         60                              /*!< Startup event parameters length. */
#define CORDIO_BOOT_LEN_VS_EVT              2                               /*!< VS event parameters length. */

/*! \brief    Initial value for CRC calculation. */
#define CORDIO_BOOT_CRC_INIT_VAL            0xFFFFFFFF

/*! \brief    FW load command flags. */
enum
{
  CORDIO_BOOT_FW_LOAD_FLAG_ENCRYPTED         = (1 << 0),  /*!< Image is encrypted. */
  CORDIO_BOOT_FW_LOAD_FLAG_PATCH             = (1 << 1)   /*!< Image is a patch. */
};

/*! \brief    Cordio extended status values. */
enum
{
  CORDIO_EXT_STATUS_SUCCESS                  = 0x00,      /*!< No error. */
  CORDIO_EXT_STATUS_ERR_CRC_FAILED           = 0x01,      /*!< CRC check failed. */
  CORDIO_EXT_STATUS_ERR_IMG_TOO_LARGE        = 0x02,      /*!< Firmware image is too large. */
  CORDIO_EXT_STATUS_ERR_BAD_OFFSET           = 0x03,      /*!< Firmware offset parameter value is unexpected. */
  CORDIO_EXT_STATUS_ERR_TRIM_BAD_LENGTH      = 0x04,      /*!< Trim length parameter does not match type. */
  CORDIO_EXT_STATUS_ERR_TRIM_UNKNOWN_TYPE    = 0x05,      /*!< Trim type parameter is unrecognized. */
  CORDIO_EXT_STATUS_ERR_TRIM_MEMORY_ALLOC    = 0x06,      /*!< Memory could not be allocated for trim load operation. */
  CORDIO_EXT_STATUS_ERR_TRIM_WRONG_VOLTAGE   = 0x07,      /*!< Trim data does not apply to present voltage mode. */
  CORDIO_EXT_STATUS_ERR_CLOCK_32M_XTAL       = 0x09,      /*!< 32-MHz xtal did not start up. */
  CORDIO_EXT_STATUS_ERR_CLOCK_32K_XTAL       = 0x0A,      /*!< 32-kHz xtal did not start up. */
  CORDIO_EXT_STATUS_ERR_CLOCK_32M_RC         = 0x0B,      /*!< 32-MHz RC did not start up. */
  CORDIO_EXT_STATUS_ERR_CLOCK_32K_RC         = 0x0C,      /*!< 32-kHz RC did not start up. */
  CORDIO_EXT_STATUS_ERR_HW_MISMATCHED        = 0x0D,      /*!< Firmware image is for different product or version. */
  CORDIO_EXT_STATUS_ERR_NO_EMULATION_RAM     = 0x0F,      /*!< No ROM emulation RAM present. */
  CORDIO_EXT_STATUS_ERR_TEST_FAILED          = 0x10,      /*!< Test failed. */
  CORDIO_EXT_STATUS_ERR_TRIM_PRESENT         = 0x11,      /*!< Trim has already been supplied. */
};

/*! \brief    Cordio reset reasons. */
enum
{
  CORDIO_RESET_REASON_UNKNOWN                = 0x00,      /*!< Unknown reason. */
  CORDIO_RESET_REASON_POWER_ON_RESET         = 0x01,      /*!< Power-on reset. */
  CORDIO_RESET_REASON_BROWNOUT               = 0x02,      /*!< Brownout. */
  CORDIO_RESET_REASON_POWER_INTERRUPTION     = 0x03,      /*!< Power interrupted. */
  CORDIO_RESET_REASON_WATCHDOG               = 0x04,      /*!< Watchdog timer expired.  */
  CORDIO_RESET_REASON_SOFTWARE               = 0x05,      /*!< Software requested reset. */
  CORDIO_RESET_REASON_LOCKUP                 = 0x06       /*!< Lockup.  */
};

/*! \brief  Cordio startup event flags. */
enum
{
  CORDIO_BOOT_STARTUP_EVT_FLAG_DETECTED_3V   = (1 << 0),  /*!< Detected 3V mode. */
  CORDIO_BOOT_STARTUP_EVT_FLAG_NEED_TRIM     = (1 << 1),  /*!< Trim load needed to operate. */
  CORDIO_BOOT_STARTUP_EVT_FLAG_NEED_FIRMWARE = (1 << 2),  /*!< Firmware needed to operate. */
  CORDIO_BOOT_STARTUP_EVT_FLAG_ROM_EMULATION = (1 << 4)   /*!< ROM emulation RAM is present. */
};

/*! \brief  Cordio startup event versions. */
enum
{
  CORDIO_BOOT_STARTUP_EVT_VERSION_0          = 0x00       /*!< Version 0. */
};

#endif /* CORDIO_BOOT_DEFS_H */
