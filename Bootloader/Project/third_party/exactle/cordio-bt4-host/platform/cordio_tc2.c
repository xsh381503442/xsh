/*************************************************************************************************/
/*!
 *  \file   cordio_tc2.c
 *
 *  \brief  Cordio TC2.
 *
 *          $Date: 2016-08-11 04:04:48 -0700 (Thu, 11 Aug 2016) $
 *          $Revision: 8285 $
 *
 *  Copyright (c) 2016 ARM, Ltd., all rights reserved.
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
#include "wsf_math.h"

#include "bstream.h"

#include "cordio_tc2.h"
#include "cordio_tc2_defs.h"
#include "cordio_trim_defs.h"

#include "board_flash.h"

#include "app_param.h"

#include "llc.h"
#include "llc_init.h"

/**************************************************************************************************
  Macros
**************************************************************************************************/

/*! \brief      Length of NV data. */
#define CORDIO_TC2_NVDATA_LEN     (4 * 1024)

/*! \brief      Offset of NV data. */
#define CORDIO_TC2_NVDATA_OFFSET  (1020 * 1024)

/*! \brief      Limit of NV data. */
#define CORDIO_TC2_NVDATA_LIMIT   (CORDIO_TC2_NVDATA_OFFSET + CORDIO_TC2_NVDATA_LEN - 1)

/*************************************************************************************************/
/*!
 *  \brief  Find BT4 object.
 *
 *  \param  pAddr       Storage for object address.
 *
 *  \return TRUE if object found.
 */
/*************************************************************************************************/
static bool_t cordioTc2FindBt4Object(uint32_t *pAddr, uint8_t type)
{
  CordioTc2ImgHdr_t hdr;
  uint32_t addr = 0;

  while (addr < BOARD_FLASH_SIZE)
  {
    BoardFlashWake();
    BoardFlashRead(addr, (uint8_t *)&hdr, sizeof(hdr));
    BoardFlashSleep();

    if (hdr.cookie != CORDIO_TC2_HEADER_COOKIE)
    {
      return FALSE;
    }
    if (hdr.type == type)
    {
      *pAddr = addr;
      return TRUE;
    }
    addr += sizeof(hdr);
    addr += hdr.dataLen & CORDIO_TC2_DATA_LEN_MASK;
  }

  return FALSE;
}

/*************************************************************************************************/
/*!
 *  \brief  Find BT4 firmware.
 *
 *  \param  pAddr       Storage for firmware address.
 *
 *  \return TRUE if firmware found.
 */
/*************************************************************************************************/
bool_t CordioTc2FindBt4Firmware(uint32_t *pAddr)
{
  return cordioTc2FindBt4Object(pAddr, CORDIO_TC2_IMG_TYPE_BT4_FW);
}

/*************************************************************************************************/
/*!
 *  \brief  Find BT4 patch.
 *
 *  \param  pAddr       Storage for patch address.
 *
 *  \return TRUE if patch found.
 */
/*************************************************************************************************/
bool_t CordioTc2FindBt4Patch(uint32_t *pAddr)
{
  return cordioTc2FindBt4Object(pAddr, CORDIO_TC2_IMG_TYPE_BT4_PATCH);
}

/*************************************************************************************************/
/*!
 *  \brief  Get size of NV data.
 *
 *  \return Size of NV data.
 */
/*************************************************************************************************/
uint32_t CordioTc2NvGetSize(void)
{
    return CORDIO_TC2_NVDATA_LEN;
}

/*************************************************************************************************/
/*!
 *  \brief  Read from NV data.
 *
 *  \param  addr      Address to read.
 *  \param  pData     Storage for data.
 *  \param  len       Length of data.
 *
 *  \return Number of bytes read.
 */
/*************************************************************************************************/
uint32_t CordioTc2NvRead(uint32_t addr, uint8_t *pData, uint32_t len)
{
  uint32_t  addrFlash;

  /* Check address and clamp length to limit. */
  addrFlash = addr + CORDIO_TC2_NVDATA_OFFSET;
  if (addrFlash > CORDIO_TC2_NVDATA_LIMIT)
  {
    return 0;
  }
  len = WSF_MIN(len, CORDIO_TC2_NVDATA_LIMIT - addrFlash + 1);

  /* Read from SPI flash. */
  BoardFlashWake();
  len = BoardFlashRead(addrFlash, pData, len);
  BoardFlashSleep();

  return len;
}

/*************************************************************************************************/
/*!
 *  \brief  Write to NV data.
 *
 *  \param  addr      Address to write.
 *  \param  pData     Data.
 *  \param  len       Length of data.
 *
 *  \return Number of bytes written.
 */
/*************************************************************************************************/
uint32_t CordioTc2NvWrite(uint32_t addr, const uint8_t *pData, uint32_t len)
{
  uint32_t addrFlash;
  uint32_t lenRemain;

  /* Check address and clamp length to limit. */
  addrFlash = addr + CORDIO_TC2_NVDATA_OFFSET;
  if (addrFlash > CORDIO_TC2_NVDATA_LIMIT)
  {
    return 0;
  }
  len = WSF_MIN(len, CORDIO_TC2_NVDATA_LIMIT - addrFlash + 1);

  /* Wake SPI flash. */
  BoardFlashWake();

  /* Write to SPI flash.*/
  lenRemain = len;
  while (lenRemain > 0)
  {
    /* Length cannot span two pages, by either being too long or crossing a page boundary. */
    uint32_t lenWrite = WSF_MIN(WSF_MIN(lenRemain, BOARD_FLASH_WRITE_BLOCK_SIZE),
                                BOARD_FLASH_WRITE_BLOCK_SIZE - (addr % BOARD_FLASH_WRITE_BLOCK_SIZE));

    lenWrite   = BoardFlashWrite(addrFlash, pData, lenWrite);
    lenRemain -= lenWrite;
    pData     += lenWrite;
    addrFlash += lenWrite;

    /* Stop if write failed. */
    if (lenWrite == 0)
    {
      break;
    }
  }

  /* Send SPI flash back to sleep. */
  BoardFlashSleep();

  return len - lenRemain;
}

/*************************************************************************************************/
/*!
 *  \brief  Erase NV data.
 *
 *  \param  addr      Address to erase.
 *
 *  \return None.
 */
/*************************************************************************************************/
void CordioTc2NvErase(void)
{
  uint32_t addrFlash;

  /* Wake SPI flash. */
  BoardFlashWake();

  addrFlash = CORDIO_TC2_NVDATA_OFFSET;
  while (addrFlash < CORDIO_TC2_NVDATA_LIMIT)
  {
    BoardFlashEraseSector(addrFlash);
    addrFlash += BOARD_FLASH_ERASE_BLOCK_SIZE;
  }

  /* Send SPI flash back to sleep. */
  BoardFlashSleep();
}

/*************************************************************************************************/
/*!
 *  \brief  Check for trim in NV data.
 *
 *  \return TRUE if NV data contains trim.
 */
/*************************************************************************************************/
bool_t CordioTc2NvCheckForTrim(void)
{
  CordioTc2ImgHdr_t hdr;

  CordioTc2NvRead(0, (uint8_t *)&hdr, sizeof(hdr));
  if (hdr.cookie != CORDIO_TC2_HEADER_COOKIE)
  {
    return FALSE;
  }
  if (hdr.type == CORDIO_TC2_IMG_TYPE_BT4_TRIM)
  {
    return TRUE;
  }
  return FALSE;
}

/*************************************************************************************************/
/*!
 *  \brief  Find transmit power options.
 *
 *  \param  maxOpts     Maximum number of options.
 *  \param  pBuf        Storage for (Voltage_Mode, Tx_Power_Level) pairs.
 *
 *  \return Number of options.
 */
/*************************************************************************************************/
uint8_t CordioTc2FindTxPowerOpts(uint8_t maxOpts, uint8_t *pBuf)
{
  uint8_t numOpts = 0;
  CordioTc2ImgHdr_t hdr;
  CordioTc2TrimHdr_t trimHdr;
  uint32_t  addr = 0;
  uint32_t  addrMax;

  CordioTc2NvRead(addr, (uint8_t *)&hdr, CORDIO_TC2_HEADER_LEN);
  if (hdr.cookie == CORDIO_TC2_HEADER_COOKIE)
  {
    addr   += CORDIO_TC2_HEADER_LEN;
    addrMax = addr + (hdr.dataLen & CORDIO_TC2_DATA_LEN_MASK);

    while ((addrMax - addr >= sizeof(trimHdr)) && (numOpts < maxOpts))
    {
      CordioTc2NvRead(addr, (uint8_t *)&trimHdr, sizeof(trimHdr));
      addr += sizeof(trimHdr);
      if (addrMax - addr < trimHdr.len)
      {
        break;
      }
      addr += trimHdr.len;

      if (trimHdr.type == CORDIO_TRIM_TYPE_TX_1V)
      {
        numOpts++;
        UINT8_TO_BSTREAM(pBuf, 1); /* voltage mode */
        UINT8_TO_BSTREAM(pBuf, trimHdr.param);
      }
      else if (trimHdr.type == CORDIO_TRIM_TYPE_TX_3V)
      {
        numOpts++;
        UINT8_TO_BSTREAM(pBuf, 3); /* voltage mode */
        UINT8_TO_BSTREAM(pBuf, trimHdr.param);
      }
    }
  }
  return numOpts;
}

/*************************************************************************************************/
/*!
 *  \brief  Set transmit power.
 *
 *  \param  voltageMode     Voltage mode (1 or 3).
 *  \param  txPowerLevel    Transmit power level.
 *
 *  \return None.
 */
/*************************************************************************************************/
void CordioTc2SetTxPower(uint8_t voltageMode, int8_t txPowerLevel)
{
  uint16_t paramId;

  paramId = (voltageMode == 1) ? CORDIO_TC2_PARAM_TX_POWER_LEVEL_1V : CORDIO_TC2_PARAM_TX_POWER_LEVEL_3V;
  AppParamWrite(paramId, sizeof(txPowerLevel), (uint8_t *)&txPowerLevel);
}

/*************************************************************************************************/
/*!
 *  \brief  Get transmit power.
 *
 *  \param  voltageMode     Voltage mode (1 or 3).
 *  \param  pTxPoweLevel    Storage for transmit power level.
 *
 *  \return None.
 */
/*************************************************************************************************/
bool_t CordioTc2GetTxPower(uint8_t voltageMode, int8_t *pTxPowerLevel)
{
  uint16_t paramId;

  paramId = (voltageMode == 1) ? CORDIO_TC2_PARAM_TX_POWER_LEVEL_1V : CORDIO_TC2_PARAM_TX_POWER_LEVEL_3V;
  return (AppParamRead(paramId, sizeof(*pTxPowerLevel), (uint8_t *)pTxPowerLevel) == sizeof(*pTxPowerLevel));
}

/*************************************************************************************************/
/*!
 *  \brief  Determine whether power option exists for voltage mode..
 *
 *  \param  voltageMode     Voltage mode (1 or 3).
 *  \param  txPowerLevel    Transmit power level.
 *
 *  \return TRUE if option exists.
 */
/*************************************************************************************************/
bool_t CordioTc2TxPowerOptExists(uint8_t voltageMode, int8_t txPowerLevel)
{
  CordioTc2ImgHdr_t hdr;
  CordioTc2TrimHdr_t trimHdr;
  uint32_t  addr = 0;
  uint32_t  addrMax;

  CordioTc2NvRead(addr, (uint8_t *)&hdr, CORDIO_TC2_HEADER_LEN);
  if (hdr.cookie == CORDIO_TC2_HEADER_COOKIE)
  {
    addr   += CORDIO_TC2_HEADER_LEN;
    addrMax = addr + (hdr.dataLen & CORDIO_TC2_DATA_LEN_MASK);

    while (addrMax - addr >= sizeof(trimHdr))
    {
      CordioTc2NvRead(addr, (uint8_t *)&trimHdr, sizeof(trimHdr));
      addr += sizeof(trimHdr);
      if (addrMax - addr < trimHdr.len)
      {
        break;
      }
      addr += trimHdr.len;

      if (trimHdr.param == txPowerLevel)
      {
        if ((trimHdr.type == CORDIO_TRIM_TYPE_TX_1V) && (voltageMode == 1) && (trimHdr.param == txPowerLevel))
        {
          return TRUE;
        }
        if ((trimHdr.type == CORDIO_TRIM_TYPE_TX_3V) && (voltageMode == 3) && (trimHdr.param == txPowerLevel))
        {
          return TRUE;
        }
      }
    }
  }
  return FALSE;
}

/*************************************************************************************************/
/*!
 *  \brief  NVDATA data provider.
 *
 *  \param  pAddr       Current address to read and storage for next address.
 *  \param  len         Length of data, in bytes.
 *  \param  pBuf        Storage for data.
 *
 *  \return TRUE if data read.
 */
/*************************************************************************************************/
static bool_t cordioTc2FlashProvider(uint32_t *pAddr, uint16_t len, uint8_t *pBuf)
{
  BoardFlashWake();
  BoardFlashRead(*pAddr, pBuf, len);
  BoardFlashSleep();
  *pAddr += len;
  return TRUE;
}

/*************************************************************************************************/
/*!
 *  \brief  NV data data provider.
 *
 *  \param  pAddr       Current address to read and storage for next address.
 *  \param  len         Length of data, in bytes.
 *  \param  pBuf        Storage for data.
 *
 *  \return TRUE if data read.
 */
/*************************************************************************************************/
static bool_t cordioTc2NvDataProvider(uint32_t *pAddr, uint16_t len, uint8_t *pBuf)
{
  if (CordioTc2NvRead(*pAddr, pBuf, len) < len)
  {
    return FALSE;
  }
  *pAddr += len;
  return TRUE;
}

/*************************************************************************************************/
/*!
 *  \brief  Perform LLC initialization.
 *
 *  \return TRUE if LLC initialization succeeds.
 */
/*************************************************************************************************/
bool_t CordioTc2LlcInit(void)
{
  uint32_t fwAddr = 0;
  uint32_t patchAddr = 0;
  uint8_t  voltageMode;
  int8_t   txPowerLevel;

  if (CordioTc2FindBt4Firmware(&fwAddr))
  {
    LlcInitSetFirmwareProvider(fwAddr, cordioTc2FlashProvider);
  }
  if (CordioTc2NvCheckForTrim())
  {
    LlcInitSetTrimProvider(0, cordioTc2NvDataProvider);
  }
  if (CordioTc2FindBt4Patch(&patchAddr))
  {
    LlcInitSetPatchProvider(patchAddr, cordioTc2FlashProvider);
  }

  LlcRequestBatteryStatus(TRUE);
  voltageMode = ((LlcGetStatusFlags() & LLC_STATUS_BATTERY_DET3V) == 0) ? 1 : 3;
  if (CordioTc2GetTxPower(voltageMode, &txPowerLevel))
  {
    if (CordioTc2TxPowerOptExists(voltageMode, txPowerLevel))
    {
      LlcInitSetTxPower(txPowerLevel);
    }
  }

  return LlcInit();
}
