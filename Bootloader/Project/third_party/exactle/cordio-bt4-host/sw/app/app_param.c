/*************************************************************************************************/
/*!
 *  \file   app_param.c
 *
 *  \brief  Application framework parameter database for BT4 external flash.
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

#include <string.h>
#include "wsf_types.h"
#include "wsf_assert.h"
#include "bstream.h"
#include "app_param.h"
#include "board_flash.h"

#define BOARD_SPIFLASH_SETTINGS_LEN         (8u * 1024u)
#define BOARD_SPIFLASH_SETTINGS_OFFSET   (1012u * 1024u)

/**************************************************************************************************
  Macros
**************************************************************************************************/

/*! \brief    Length of a parameter header. */
#define APP_PARAM_HDR_LEN           5

/*! \brief    Maximum total length of parameters (including headers). */
#define APP_PARAM_MAX_LEN_TOTAL     128

/*! \brief    Maximum length of one parameter. */
#define APP_PARAM_MAX_LEN           (APP_PARAM_MAX_LEN_TOTAL - APP_PARAM_HDR_LEN)

/*! \brief    Length of a page of parameters. */
#define APP_PARAM_MAX_PAGE_LEN      255

/*! \brief    Invalid ID. */
#define APP_PARAM_INVALID_ID        0xFFFF

/**************************************************************************************************
  Data Types
**************************************************************************************************/

/*! \brief    Page numbers. */
enum
{
  APP_PARAM_PAGE_NUM_0 = 0,           /*!< 0th page. */
  APP_PARAM_PAGE_NUM_1 = 1            /*!< 1st page. */
};

/*! \brief    Valid markers. */
enum
{
  APP_PARAM_VALID_WRITTEN = 0xFF,     /*!< Parameter has been written. */
  APP_PARAM_VALID_CLEARED = 0x00      /*!< Parameter has been cleared. */
};

/**************************************************************************************************
  Global Variables
**************************************************************************************************/

/*! \brief    TRUE if parameter handling has been initialized. */
static bool_t appParamInit = FALSE;

/*! \brief    Active page number. */
static uint8_t appParamPageNum = APP_PARAM_PAGE_NUM_0;

/*************************************************************************************************/
/*!
 *  \brief  Erase flash page.
 *
 *  \param  pageNum     Page to erase.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void appParamFlashErase(bool_t pageNum)
{
  BoardFlashWake();
  BoardFlashEraseSector(BOARD_SPIFLASH_SETTINGS_OFFSET + (pageNum * BOARD_FLASH_ERASE_BLOCK_SIZE));
  BoardFlashSleep();
}

/*************************************************************************************************/
/*!
 *  \brief  Read from flash.
 *
 *  \param  addr        Address within page.
 *  \param  pBuf        Storage for read.
 *  \param  len         Read length.
 *  \param  pageNum     Page to read.
 *
 *  \return Number of bytes read.
 */
/*************************************************************************************************/
static uint16_t appParamFlashRead(uint32_t addr, uint8_t *pBuf, uint16_t len, uint8_t pageNum)
{
  uint32_t addrFlash = BOARD_SPIFLASH_SETTINGS_OFFSET + (pageNum * BOARD_FLASH_ERASE_BLOCK_SIZE);
  uint32_t addrFlashLimit = addrFlash + BOARD_FLASH_ERASE_BLOCK_SIZE - 1;
  uint32_t r;

  addrFlash += addr;
  if ((addrFlash > addrFlashLimit) || (len > addrFlashLimit - addrFlash + 1))
  {
    return 0;
  }

  BoardFlashWake();
  r = BoardFlashRead(addrFlash, pBuf, len);
  BoardFlashSleep();

  return r;
}

/*************************************************************************************************/
/*!
 *  \brief  Write to flash.
 *
 *  \param  addr        Address within page.
 *  \param  pBuf        Buffer to write.
 *  \param  len         Write length.
 *  \param  pageNum     Page to write.
 *
 *  \return Number of bytes written.
 */
/*************************************************************************************************/
static uint16_t appParamFlashWrite(uint32_t addr, const uint8_t *pBuf, uint16_t len, uint8_t pageNum)
{
  uint32_t addrFlash = BOARD_SPIFLASH_SETTINGS_OFFSET + (pageNum * BOARD_FLASH_ERASE_BLOCK_SIZE);
  uint32_t addrFlashLimit = addrFlash + BOARD_FLASH_ERASE_BLOCK_SIZE - 1;
  uint16_t lenRemain = len;
  uint16_t lenWrite;

  addrFlash += addr;
  if ((addrFlash > addrFlashLimit) || (len > addrFlashLimit - addrFlash + 1))
  {
    return 0;
  }

  BoardFlashWake();
  while (lenRemain > 0u)
  {
    /* Length cannot span two pages, by either being too long or crossing a page boundary. */
    lenWrite = (lenRemain < BOARD_FLASH_WRITE_BLOCK_SIZE) ? lenRemain : BOARD_FLASH_WRITE_BLOCK_SIZE;

    if (lenWrite > BOARD_FLASH_WRITE_BLOCK_SIZE - (addrFlash % BOARD_FLASH_WRITE_BLOCK_SIZE))
    {
      lenWrite = BOARD_FLASH_WRITE_BLOCK_SIZE - (addrFlash % BOARD_FLASH_WRITE_BLOCK_SIZE);
    }
    if (BoardFlashWrite(addrFlash, pBuf, lenWrite) < lenWrite)
    {
      break;
    }

    lenRemain -= lenWrite;
    pBuf      += lenWrite;
    addrFlash += lenWrite;
  }
  BoardFlashSleep();

  return len - lenRemain;
}

/*************************************************************************************************/
/*!
 *  \fn     AppParamInit()
 *
 *  \brief  Initialize the parameter database.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppParamInit(void)
{
  uint8_t page0[APP_PARAM_HDR_LEN];
  uint8_t page1[APP_PARAM_HDR_LEN];

  appParamPageNum = APP_PARAM_PAGE_NUM_0;   /* default */
  appParamFlashRead(0u, page0, APP_PARAM_HDR_LEN, APP_PARAM_PAGE_NUM_0);
  appParamFlashRead(0u, page1, APP_PARAM_HDR_LEN, APP_PARAM_PAGE_NUM_1);

  if (page0[0] == 0xFF)
  {
    if (page1[0] != 0xFF)
    {
      appParamPageNum = APP_PARAM_PAGE_NUM_1;
    }
  }
  else
  {
    if (page1[0] != 0xFF)
    {
      /*
       * Both pages should not be occupied; clear one to prevent later problems.  This may occur
       * if a defragment operation is interrupted.
       */
      appParamFlashErase(APP_PARAM_PAGE_NUM_1);
    }
  }

  appParamInit = TRUE;
}

/*************************************************************************************************/
/*!
 *  \fn     AppParamClear()
 *
 *  \brief  Clear the parameter database.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppParamClear(void)
{
  if (!appParamInit)
  {
    return;
  }

  appParamFlashErase(APP_PARAM_PAGE_NUM_0);
  appParamFlashErase(APP_PARAM_PAGE_NUM_1);
  appParamPageNum = APP_PARAM_PAGE_NUM_0;
}

/*************************************************************************************************/
/*!
 *  \brief  Defragment the parameter database.
 *
 *  \return Offset of next write within active page.
 */
/*************************************************************************************************/
static uint16_t appParamDefragment(void)
{
  uint8_t  paramsCopy[APP_PARAM_MAX_LEN_TOTAL];
  uint8_t  *pParams = paramsCopy;
  uint32_t paramsLen = 0;
  uint32_t offset;

  for (offset = 0u; offset < APP_PARAM_MAX_PAGE_LEN - APP_PARAM_HDR_LEN; )
  {
    uint8_t  hdr[APP_PARAM_HDR_LEN];
    uint16_t idRead;
    uint16_t valueLenRead;
    uint8_t  valid;
    uint8_t  *pHdr = &hdr[0];

    /* Read the setting header (ID, length, and valid bytes). */
    appParamFlashRead(offset, pHdr, APP_PARAM_HDR_LEN, appParamPageNum);
    offset += APP_PARAM_HDR_LEN;

    /* The data area is initialized to 0xFF; the parameter was not found. */
    BSTREAM_TO_UINT16(idRead, pHdr);
    if (idRead == APP_PARAM_INVALID_ID)
    {
      break;
    }

    /* Check for validity. */
    BSTREAM_TO_UINT16(valueLenRead, pHdr);
    BSTREAM_TO_UINT8 (valid,        pHdr);
    if (valid != APP_PARAM_VALID_CLEARED)
    {
      if (paramsLen + APP_PARAM_HDR_LEN + valueLenRead > APP_PARAM_MAX_LEN_TOTAL)
      {
        /* TODO this is an overrun error; prevent it */
        paramsLen += APP_PARAM_HDR_LEN + valueLenRead;
      }
      else
      {
        UINT16_TO_BSTREAM(pParams, idRead);
        UINT16_TO_BSTREAM(pParams, valueLenRead);
        UINT8_TO_BSTREAM (pParams, APP_PARAM_VALID_WRITTEN);
        paramsLen += APP_PARAM_HDR_LEN;

        appParamFlashRead(offset, pParams, valueLenRead, appParamPageNum);
        pParams   += valueLenRead;
        paramsLen += valueLenRead;
      }
    }

    /* Move past the parameter. */
    offset += valueLenRead;
  }

  /* In case of overrun, return current limit. */
  if (paramsLen > APP_PARAM_MAX_LEN_TOTAL)
  {
    return offset;
  }

  /* Switch to new page. */
  appParamPageNum = (appParamPageNum == APP_PARAM_PAGE_NUM_0) ? APP_PARAM_PAGE_NUM_1 : APP_PARAM_PAGE_NUM_0;

  /* Write settings to new page. */
  appParamFlashWrite(0, paramsCopy, paramsLen, appParamPageNum);

  /* An interruption between write and erase may result in both pages occupied upon init. */

  /* Erase old page. */
  appParamFlashErase((appParamPageNum == APP_PARAM_PAGE_NUM_0) ? APP_PARAM_PAGE_NUM_1 : APP_PARAM_PAGE_NUM_0);

  return paramsLen;
}

/*************************************************************************************************/
/*!
 *  \fn     AppParamWrite
 *
 *  \brief  Write parameter value.
 *
 *  \param  id          Identifier.
 *  \param  valueLen    Value length in bytes.
 *  \param  pValue      Value data.
 *
 *  \return Number of bytes written.
 */
/*************************************************************************************************/
uint16_t AppParamWrite(uint16_t id, uint16_t valueLen, const uint8_t *pValue)
{
  uint8_t  valueBuf[APP_PARAM_MAX_LEN_TOTAL];
  uint8_t  *pValueBuf = valueBuf;
  uint32_t paramsLen = 0;
  uint32_t offset;

  if (!appParamInit)
  {
    return 0;
  }

  /* An invalid ID cannot be written. */
  if (id == APP_PARAM_INVALID_ID)
  {
    return 0;
  }

  /* Validate setting length. */
  if (valueLen > APP_PARAM_MAX_LEN - APP_PARAM_HDR_LEN)
  {
    return 0;
  }

  /* Buffer for value write. */
  UINT16_TO_BSTREAM(pValueBuf, id);
  UINT16_TO_BSTREAM(pValueBuf, valueLen);
  UINT8_TO_BSTREAM (pValueBuf, APP_PARAM_VALID_WRITTEN);
  memcpy(pValueBuf, pValue, valueLen);

  /* Search for location at which to write. */
  for (offset = 0u; offset < APP_PARAM_MAX_PAGE_LEN - APP_PARAM_HDR_LEN; )
  {
    uint8_t  hdr[APP_PARAM_HDR_LEN];
    uint16_t idRead;
    uint16_t valueLenRead;
    uint8_t  valid;
    uint8_t  *pHdr = &hdr[0];

    /* Read the setting header (ID, length, and valid bytes). */
    appParamFlashRead(offset, pHdr, APP_PARAM_HDR_LEN, appParamPageNum);

    /* The data area is initialized to 0xFF; the parameter was not found. */
    BSTREAM_TO_UINT16(idRead, pHdr);
    if (idRead == APP_PARAM_INVALID_ID)
    {
      break;
    }
    offset += APP_PARAM_HDR_LEN;

    /* Check for validity. */
    BSTREAM_TO_UINT16(valueLenRead, pHdr);
    BSTREAM_TO_UINT8 (valid,        pHdr);
    if (valid != APP_PARAM_VALID_CLEARED)
    {
      if (idRead == id)
      {
        /* Clear this copy of the setting. */
        valid = APP_PARAM_VALID_CLEARED;
        appParamFlashWrite(offset - 1, &valid, 1, appParamPageNum);

        /* TODO clear copy of setting after writing in new location */
      }
      else
      {
        /* Accumulate parameters length to prevent an overrun error. */
        paramsLen += APP_PARAM_HDR_LEN + valueLenRead;
      }
    }
    /* Move past the setting. */
    offset += valueLenRead;
  }

  /* Check for an overrun error. */
  if (paramsLen + APP_PARAM_HDR_LEN + valueLen > APP_PARAM_MAX_LEN_TOTAL)
  {
    return 0;
  }

  /* Defragment page if setting will not fit. */
  if (offset + APP_PARAM_HDR_LEN + valueLen >= APP_PARAM_MAX_PAGE_LEN)
  {
    offset = appParamDefragment();
  }

  /* Write parameter at end of defragmented page. */
  appParamFlashWrite(offset, valueBuf, valueLen + APP_PARAM_HDR_LEN, appParamPageNum);
  return valueLen;
}

/*************************************************************************************************/
/*!
 *  \fn     AppParamRead
 *
 *  \brief  Read parameter value.
 *
 *  \param  id          Identifier.
 *  \param  valueLen    Maximum value length in bytes.
 *  \param  pValue      Storage value data.
 *
 *  \return Number of bytes read.
 */
/*************************************************************************************************/
uint16_t AppParamRead(uint16_t id, uint16_t valueLen, uint8_t *pValue)
{
  uint8_t  hdr[APP_PARAM_HDR_LEN];
  uint32_t offset;

  if (!appParamInit)
  {
    return 0;
  }

  /* An invalid ID cannot be sought. */
  if (id == APP_PARAM_INVALID_ID)
  {
    return 0;
  }

  for (offset = 0u; offset < APP_PARAM_MAX_PAGE_LEN; )
  {
    uint16_t idRead;
    uint16_t valueLenRead;
    uint8_t  valid;
    uint8_t  *pHdr = &hdr[0];

    /* Read the setting header (ID, length, and valid bytes). */
    appParamFlashRead(offset, pHdr, APP_PARAM_HDR_LEN, appParamPageNum);
    offset += APP_PARAM_HDR_LEN;

    /* The data area is initialized to 0xFF; the parameter was not found. */
    BSTREAM_TO_UINT16(idRead, pHdr);
    if (idRead == APP_PARAM_INVALID_ID)
    {
      return 0;
    }

    /* Check for matching parameter. */
    BSTREAM_TO_UINT16(valueLenRead, pHdr);
    BSTREAM_TO_UINT8 (valid,        pHdr);
    if ((idRead == id) && (valid != APP_PARAM_VALID_CLEARED))
    {
      valueLenRead = (valueLen < valueLenRead) ? valueLen : valueLenRead;
      appParamFlashRead(offset, pValue, valueLenRead, appParamPageNum);
      return valueLenRead;
    }

    /* Move past the parameter. */
    offset += valueLenRead;
  }

  return 0;
}
