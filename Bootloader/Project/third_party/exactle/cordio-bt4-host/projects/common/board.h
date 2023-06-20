/*************************************************************************************************/
/*!
 *  \file   board.h
 *
 *  \brief  Board services.
 *
 *          $Date: 2016-07-22 14:16:10 -0700 (Fri, 22 Jul 2016) $
 *          $Revision: 7978 $
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

#ifndef BOARD_H
#define BOARD_H

#include "wsf_types.h"

/**************************************************************************************************
  Data Types
**************************************************************************************************/

/*! \brief      Application mode. */
enum
{
  BOARD_MODE_DTM    = 0,
  BOARD_MODE_BEACON = 1,
  BOARD_MODE_SENSOR = 2,
};

/*! \brief      Mux mode settings. */
enum
{
  BOARD_MUXMODE_NORMAL = 0,
  BOARD_MUXMODE_DIGITAL_TEST = 1,
};

/**************************************************************************************************
  Functions
**************************************************************************************************/

void BoardHwInit(void);
#ifdef INIT_I2C
void BoardI2cDevicesInit(void);
#endif
uint8_t BoardGetMode(void);
void BoardSetMuxMode(uint8_t mode);

#ifdef INIT_GPO
void BoardGpoSet(uint8_t num);
void BoardGpoClr(uint8_t num);
void BoardGpoToggle(uint8_t num);
#endif

uint32_t BoardReadBtns(void);

#endif /* BOARD_H */
