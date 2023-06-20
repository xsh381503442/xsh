/*************************************************************************************************/
/*!
 *  \file   drv_spiflash_sst.c
 *
 *  \brief  SPI/SST flash driver.
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

#include "wsf_types.h"
#include "wsf_trace.h"
#include "wsf_math.h"

#include "SMD_TC2.h"

#include "board_flash.h"
#include "platform.h"

/**************************************************************************************************
  Macros
**************************************************************************************************/

/*! \brief    Number of times to retry operation. */
#define BOARD_FLASH_MAX_RETRIES             10

/*! \brief    Flash ID byte. */
#define BOARD_FLASH_ID_WF080B               0x86

/*! \brief    Flash enable. */
#define BOARD_FLASH_ENABLE                  GPIO_GPIO1

/**************************************************************************************************
  Data Types
**************************************************************************************************/

/*! \brief        Instructions. */
enum
{
 BOARD_FLASH_INSTR_WRSR  = 0x01,
 BOARD_FLASH_INSTR_WRITE = 0x02,
 BOARD_FLASH_INSTR_READ  = 0x03,
 BOARD_FLASH_INSTR_WRDI  = 0x04,
 BOARD_FLASH_INSTR_RDSR  = 0x05,
 BOARD_FLASH_INSTR_WREN  = 0x06,
 BOARD_FLASH_INSTR_SE    = 0x20,  /*!< 4KB sector erase. */
 BOARD_FLASH_INSTR_BE    = 0xD8,  /*!< 64KB block erase. */
 BOARD_FLASH_INSTR_CE    = 0x60,  /*!< Entire chip erase. */
 BOARD_FLASH_INSTR_RDID  = 0xAB,
 BOARD_FLASH_INSTR_DPD   = 0xB9,
};

/*! \brief        Status register bits. */
enum
{
  BOARD_FLASH_SR_WIP = (1 << 0),
  BOARD_FLASH_SR_WEL = (1 << 1),
  BOARD_FLASH_SR_BP0 = (1 << 2),
  BOARD_FLASH_SR_BP1 = (1 << 3)
};

/*************************************************************************************************/
/*!
 *  \brief  Perform SPI transfer.
 *
 *  \param  datum     Datum to write.
 *
 *  \return Datum read.
 */
/*************************************************************************************************/
static uint8_t boardFlashSpiTr(uint8_t datum)
{
  uint32_t timeout;

  timeout = 0xFFFF;
  while (!(ARM_SSP0->SR & SSP_SR_TFE) && (timeout-- > 0)) { ; }
  if (timeout == 0)
  {
    APP_TRACE_INFO0("flash timeout waiting to write");
  }
  ARM_SSP0->DR = datum;

  timeout = 0xFFFF;
  while (!(ARM_SSP0->SR & SSP_SR_RNE) && (timeout-- > 0)) { ; }
  if (timeout == 0)
  {
    APP_TRACE_INFO0("flash timeout waiting to read");
  }
  return ARM_SSP0->DR;
}

/*************************************************************************************************/
/*!
 *  \brief  Enable or disable CS.
 *
 *  \param  ena       TRUE to select flash.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void boardFlashSpiCs(bool_t ena)
{
  if (!ena) /* active low */
  {
    ARM_SSP0->CR1 = 0;
    GPIOA->DATAOUT |= GPIO_SSP_FSS;
  }
  else
  {
    ARM_SSP0->CR1 = SSP_CR1_SSE;
    GPIOA->DATAOUT &= ~GPIO_SSP_FSS;
  }

  PlatformDelayUsec(1);
}

/*************************************************************************************************/
/*!
 *  \brief  Enable or disable flash.
 *
 *  \param  ena       TRUE to enable flash.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void boardFlashEna(bool_t ena)
{
  GPIOA->OUTENSET = BOARD_FLASH_ENABLE;
  if (ena)
  {
    GPIOA->DATAOUT |= BOARD_FLASH_ENABLE;
  }
  else
  {
    GPIOA->DATAOUT &= ~BOARD_FLASH_ENABLE;
  }

  PlatformDelayUsec(500); /* tPU-READ/tPU-WRITE. 500us minimum. */
}

/*************************************************************************************************/
/*!
 *  \brief  Check flash ID.
 *
 *  \return TRUE if flash ID verified.
 */
/*************************************************************************************************/
static bool_t boardFlashCheckId(void)
{
  uint8_t val;

  /*
   * The SST25VF family expects address of 00,00,01 to get the device ID.
   * The SST25WF family doesn't care about the address
   */
  boardFlashSpiCs(TRUE);
  boardFlashSpiTr(BOARD_FLASH_INSTR_RDID);  /* RDID */
  boardFlashSpiTr(0x00);                    /* Dummy write 1 */
  boardFlashSpiTr(0x00);                    /* Dummy write 2 */
  boardFlashSpiTr(0x01);                    /* Dummy write 3 */
  val = boardFlashSpiTr(0x00);              /* Read device id */
  boardFlashSpiCs(FALSE);

  return (val == BOARD_FLASH_ID_WF080B);
}

/*************************************************************************************************/
/*!
 *  \brief  Enable write and erase.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void boardFlashWriteEnable(void)
{
  boardFlashSpiCs(TRUE);
  boardFlashSpiTr(BOARD_FLASH_INSTR_WREN); /* WREN */
  boardFlashSpiCs(FALSE);
}

/*************************************************************************************************/
/*!
 *  \brief  Read status register.
 *
 *  \return Status register value.
 */
/*************************************************************************************************/
static uint8_t boardFlashReadStatus(void)
{
  uint8_t val;

  boardFlashSpiCs(TRUE);
  boardFlashSpiTr(BOARD_FLASH_INSTR_RDSR); /* RDSR */
  val = boardFlashSpiTr(0xaa);              /* Dummy write */
  boardFlashSpiCs(FALSE);

  return val;
}

/*************************************************************************************************/
/*!
 *  \brief  Wait while flash is busy.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void boardFlashWaitWhileBusy(void)
{
  uint32_t retries = BOARD_FLASH_MAX_RETRIES;

  while (retries-- > 0)
  {
    if ((boardFlashReadStatus() & BOARD_FLASH_SR_WIP) == 0)
    {
      break;
    }

    PlatformDelayUsec(1000);
  }

  if (retries == 0)
  {
    APP_TRACE_INFO0("flash wait ready timed out");
  }
}

/*************************************************************************************************/
/*!
 *  \brief  Initialize flash.
 *
 *  \return TRUE if flash was initialized & detected.
 */
/*************************************************************************************************/
bool_t BoardFlashInit(void)
{
  /* Select GPIO as AFs. */
  GPIOA->ALTFUNCSET = GPIO_SSP_TXD | GPIO_SSP_CLK | GPIO_SSP_RXD;
  GPIOA->ALTFUNCCLR = GPIO_SSP_FSS;

  /* Set up SPI. */
  ARM_SSP0->CR0  = SSP_CR0_SCR_SELECT(1) | SSP_CR0_FRF_Moto | SSP_CR0_DSS_SELECT(8);
  ARM_SSP0->CPSR = SSP_CSPR_CVPSDVR(2); /* SST25WF080B can run up to 30MHz.  Use max divisor */

  /* Set up GPIO. */
  GPIOA->INTENCLR = GPIO_SSP_FSS | GPIO_SSP_TXD | GPIO_SSP_CLK | GPIO_SSP_RXD;
  GPIOA->OUTENSET = GPIO_SSP_FSS | GPIO_SSP_TXD | GPIO_SSP_CLK;
  GPIOA->OUTENCLR = GPIO_SSP_RXD;

  /* Disable flash & deselect. */
  boardFlashEna(FALSE);
  boardFlashSpiCs(FALSE);

  /* Attempt to detect flash. */
  boardFlashEna(TRUE);
  return boardFlashCheckId();
}

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
uint32_t BoardFlashRead(uint32_t addr, uint8_t *pData, uint32_t len)
{
  uint32_t lenRemain = len;

  boardFlashSpiCs(TRUE);

  boardFlashSpiTr(BOARD_FLASH_INSTR_READ); /* READ */
  boardFlashSpiTr((addr >> 16) & 0xFF);     /* MSB addr */
  boardFlashSpiTr((addr >>  8) & 0xFF);
  boardFlashSpiTr((addr >>  0) & 0xFF);     /* LSB addr */

  while (lenRemain > 0)
  {
    const uint8_t lenFifo = 8;
    const uint8_t lenRead = WSF_MIN(lenRemain, lenFifo);
    uint8_t i;

    while (ARM_SSP0->SR & SSP_SR_BSY) { ; }

    for (i = 0; i < lenRead; i++)
    {
      ARM_SSP0->DR = 0; /* Dummy write */
    }

    for (i = 0 ; i < lenRead; i++)
    {
      while (!(ARM_SSP0->SR & SSP_SR_RNE)) { ; }
      *pData++ = ARM_SSP0->DR;
    }

    lenRemain -= lenRead;
  }

  boardFlashSpiCs(FALSE);

  return len;
}

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
uint32_t BoardFlashWrite(uint32_t addr, const uint8_t *pData, uint32_t len)
{
  uint32_t i;

  /* Check length and address. */
  if (len == 0u)
  {
    return 0;
  }
  if (addr >= BOARD_FLASH_SIZE)
  {
    return 0;
  }

  /* Make sure write does not cross a page boundary. */
  if (len > BOARD_FLASH_WRITE_BLOCK_SIZE - (addr % BOARD_FLASH_WRITE_BLOCK_SIZE))
  {
    len = BOARD_FLASH_WRITE_BLOCK_SIZE - (addr % BOARD_FLASH_WRITE_BLOCK_SIZE);
  }

  boardFlashWriteEnable();

  boardFlashSpiCs(TRUE);
  boardFlashSpiTr(BOARD_FLASH_INSTR_WRITE); /* WRITE */
  boardFlashSpiTr((addr >> 16) & 0xFF);     /* MSB addr */
  boardFlashSpiTr((addr >>  8) & 0xFF);
  boardFlashSpiTr((addr >>  0) & 0xFF);     /* LSB addr */

  for (i = 0u; i < len; i++, pData++)
  {
    boardFlashSpiTr(*pData);
  }

  boardFlashSpiCs(FALSE);

  /* Wait for write to complete. */
  boardFlashWaitWhileBusy();

  return len;
}

/*************************************************************************************************/
/*!
 *  \brief  Erase flash sector.
 *
 *  \param  addr      Address to erase.
 *
 *  \return None.
 */
/*************************************************************************************************/
void BoardFlashEraseSector(uint32_t addr)
{
  boardFlashWriteEnable();

  boardFlashSpiCs(TRUE);
  boardFlashSpiTr(BOARD_FLASH_INSTR_SE);  /* ERASE SECTOR */
  boardFlashSpiTr((addr >> 16) & 0xff);   /* MSB addr */
  boardFlashSpiTr((addr >>  8) & 0xff);
  boardFlashSpiTr((addr >>  0) & 0xff);   /* LSB addr */
  boardFlashSpiCs(FALSE);

  /* Wait for erase to complete. */
  boardFlashWaitWhileBusy();
}

/*************************************************************************************************/
/*!
 *  \brief  Erase flash chip.
 *
 *  \param  addr      Address to erase.
 *
 *  \return None.
 */
/*************************************************************************************************/
void BoardFlashEraseChip(void)
{
  boardFlashWriteEnable();

  boardFlashSpiCs(TRUE);
  boardFlashSpiTr(BOARD_FLASH_INSTR_CE); /* ERASE CHIP */
  boardFlashSpiCs(FALSE);

  /* Wait for erase to complete. */
  boardFlashWaitWhileBusy();
}

/*************************************************************************************************/
/*!
 *  \brief  Put flash into deep sleep mode.
 *
 *  \return None.
 */
/*************************************************************************************************/
void BoardFlashSleep(void)
{
  boardFlashSpiCs(TRUE);
  boardFlashSpiTr(BOARD_FLASH_INSTR_DPD); /* DPD */
  boardFlashSpiCs(FALSE);
  boardFlashEna(FALSE);
}

/*************************************************************************************************/
/*!
 *  \brief  Remove flash from deep sleep mode.
 *
 *  \return None.
 */
/*************************************************************************************************/
void BoardFlashWake(void)
{
  uint32_t retries = BOARD_FLASH_MAX_RETRIES;

  boardFlashEna(TRUE);
  while (retries-- > 0)
  {
    if (boardFlashCheckId())
    {
      break;
    }

    /* Ensure flash is awake. */
    PlatformDelayUsec(500);
  }

  if (retries == 0)
  {
    APP_TRACE_INFO0("flash wake failed");
  }
}
