/*************************************************************************************************/
/*!
 *  \file   board_flash.h
 *
 *  \brief  Flash driver.
 *
 *          $Date: 2016-03-29 14:55:12 -0700 (Tue, 29 Mar 2016) $
 *          $Revision: 6524 $
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

#ifndef BOARD_FLASH_H
#define BOARD_FLASH_H

#include "wsf_types.h"

/**************************************************************************************************
  Macros
**************************************************************************************************/

#define BOARD_FLASH_SIZE                1048576
#define BOARD_FLASH_WRITE_BLOCK_SIZE        256
#define BOARD_FLASH_ERASE_BLOCK_SIZE       4096

/*************************************************************************************************/
/*!
 *  \brief  Initialize flash.
 *
 *  \return TRUE if flash was initialized & detected.
 */
/*************************************************************************************************/
bool_t BoardFlashInit(void);

/*************************************************************************************************/
/*!
 *  \brief  Read from flash.
 *
 *  \param  addr      Address to read.
 *  \param  pData     Storage for data.
 *  \param  len       Length of data.
 *
 *  \return Number of bytes read.
 */
/*************************************************************************************************/
uint32_t BoardFlashRead(uint32_t addr, uint8_t *pData, uint32_t len);

/*************************************************************************************************/
/*!
 *  \brief  Write to flash.
 *
 *  \param  addr      Address to write.
 *  \param  pData     Data.
 *  \param  len       Length of data.
 *
 *  \return Number of bytes written.
 */
/*************************************************************************************************/
uint32_t BoardFlashWrite(uint32_t addr, const uint8_t *pData, uint32_t len);

/*************************************************************************************************/
/*!
 *  \brief  Erase flash sector.
 *
 *  \param  addr      Address to erase.
 *
 *  \return None.
 */
/*************************************************************************************************/
void BoardFlashEraseSector(uint32_t addr);

/*************************************************************************************************/
/*!
 *  \brief  Erase flash chip.
 *
 *  \param  addr      Address to erase.
 *
 *  \return None.
 */
/*************************************************************************************************/
void BoardFlashEraseChip(void);

/*************************************************************************************************/
/*!
 *  \brief  Put flash into deep sleep mode.
 *
 *  \return None.
 */
/*************************************************************************************************/
void BoardFlashSleep(void);

/*************************************************************************************************/
/*!
 *  \brief  Remove flash from deep sleep mode.
 *
 *  \return None.
 */
/*************************************************************************************************/
void BoardFlashWake(void);

#endif /* BOARD_FLASH_H */
