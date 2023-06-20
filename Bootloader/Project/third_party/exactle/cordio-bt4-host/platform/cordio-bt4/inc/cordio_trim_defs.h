/*************************************************************************************************/
/*!
 *  \file   cordio_trim_defs.h
 *
 *  \brief  Cordio trim definitions.
 *
 *          $Date: 2016-08-28 14:40:28 -0700 (Sun, 28 Aug 2016) $
 *          $Revision: 8609 $
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

#ifndef CORDIO_TRIM_DEFS_H
#define CORDIO_TRIM_DEFS_H

/**************************************************************************************************
  Macros
**************************************************************************************************/

/*! \brief    Cordio trim data set types.  */
enum
{
  CORDIO_TRIM_TYPE_IC    = 0x0A,    /*!< IC-level trim and calibration data. */
  CORDIO_TRIM_TYPE_BOARD = 0x0B,    /*!< Board-level trim, calibration, and configuration data. */
  CORDIO_TRIM_TYPE_TX_1V = 0x14,    /*!< Transmit trim (1v mode). */
  CORDIO_TRIM_TYPE_TX_3V = 0x15     /*!< Transmit trim (3v mode). */
};

/*! \brief    Size of IC trim data set. */
#define CORDIO_TRIM_SIZE_IC       32

/*! \brief    Size of board trim data set. */
#define CORDIO_TRIM_SIZE_BOARD    32

/*! \brief    Minimum size of transmit trim data set. */
#define CORDIO_TRIM_SIZE_MIN_TX   112

/*! \brief    Maximum size of transmit trim data set. */
#define CORDIO_TRIM_SIZE_MAX_TX   268

#endif /* CORDIO_TRIM_DEFS_H */
