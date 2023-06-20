/*************************************************************************************************/
/*!
 *  \file   llc_init.c
 *
 *  \brief  Main Link Layer Controller (LLC) initialization.
 *
 *          $Date: 2016-08-05 07:27:17 -0700 (Fri, 05 Aug 2016) $
 *          $Revision: 8163 $
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

#include <string.h>

#include "wsf_types.h"
#include "wsf_assert.h"
#include "wsf_buf.h"
#include "wsf_cs.h"
#include "wsf_math.h"
#include "wsf_queue.h"
#include "wsf_trace.h"

#include "bstream.h"

#include "hci_defs.h"
#include "cordio_boot_defs.h"
#include "cordio_trim_defs.h"
#include "cordio_host_defs.h"
#include "cordio_tc2_defs.h"

#include "llc_init.h"
#include "llc_hci.h"

/**************************************************************************************************
  Data Types
**************************************************************************************************/

/*! \brief      Message information. */
typedef struct
{
  uint8_t     type;   /*!< Type. */
  uint16_t    len;    /*!< Length. */
  uint8_t     *pData; /*!< Data. */
} llcInitMsg_t;

/**************************************************************************************************
  Global Variables
**************************************************************************************************/

/*! \brief      Main control block. */
static struct
{
  uint32_t              fwAddr;             /*!< Firmware address. */
  LlcInitDataProvider_t fwProvider;         /*!< Firmware data provider. */
  uint32_t              trimAddr;           /*!< Trim address. */
  LlcInitDataProvider_t trimProvider;       /*!< Trim provider. */
  uint32_t              patchAddr;          /*!< Patch address. */
  LlcInitDataProvider_t patchProvider;      /*!< Patch provider. */
  bool_t                txPowerLevelSet;    /*!< TRUE if Tx power level set. */
  int8_t                txPowerLevel;       /*!< Tx power level. */
  uint32_t              timeout;            /*!< Timeout for LLC responses. */

  wsfQueue_t            rxQ;                /*!< HCI Rx queue. */
  volatile bool_t       txCompleteFlag;     /*!< Complete flag. */
} llcInitCb = { 0 };

/*************************************************************************************************/
/*!
 * \brief   Allocate a buffer.
 *
 * \param   len  Length of buffer
 *
** \return  Pointer to buffer or NULL if buffer could not be allocated
 */
/*************************************************************************************************/
static uint8_t *llcInitBufAlloc(uint16_t len)
{
  return WsfBufAlloc(len);
}

/*************************************************************************************************/
/*!
 * \brief   Free a buffer.
 *
 * \param   pData     Pointer to buffer
 */
/*************************************************************************************************/
static void llcInitBufFree(uint8_t *pData)
{
  WsfBufFree(pData);
}

/**************************************************************************************************
  HCI & LLCC callbacks
**************************************************************************************************/

/* HCI callback prototypes. */
static uint8_t *llcInitHciBufAlloc(uint16_t len);
static void llcInitHciBufFree(uint8_t *buf);
static void llcInitHciReadDone(uint8_t type, uint8_t *pData, uint16_t len);
static void llcInitHciWriteDone(uint8_t type, uint8_t *pData, int32_t err, void *pContext);

/*! \brief      HCI callbacks. */
static const LlcHciCbacks_t llcInitHciCbacks =
{
  llcInitHciBufAlloc,
  llcInitHciBufFree,
  llcInitHciReadDone,
  llcInitHciWriteDone
};

/*************************************************************************************************/
/*!
 * \brief   Allocate a buffer for a received message.
 *
 * This function is called from an interrupt context.
 *
 * \param   len  Length of buffer
 *
** \return  Pointer to buffer or NULL if buffer could not be allocated
 */
/*************************************************************************************************/
static uint8_t *llcInitHciBufAlloc(uint16_t len)
{
  uint8_t *pBuf;

  if ((pBuf = llcInitBufAlloc(len + 8)) != NULL)
  {
    /* Allow for queue, length, type. */
    pBuf += 8;
  }
  return (uint8_t *)pBuf;
}

/*************************************************************************************************/
/*!
 * \brief   Free a buffer previously allocated with BufAlloc().
 *
 * \param   pData     Pointer to buffer
 */
/*************************************************************************************************/
static void llcInitHciBufFree(uint8_t *pData)
{
  if (pData != NULL)
  {
    uint8_t *pBuf = pData;

    /* Recover header. */
    pBuf -= 8;
    llcInitBufFree(pBuf);
  }
}

/*************************************************************************************************/
/*!
 * \brief   Handle read completion.
 *
 * This function is called from an interrupt context.
 *
 * \param   type    Packet type
 * \param   pData   Packet data, which was allocated with BufAlloc().  The caller must free
 *                  this buffer
 * \param   len     Length of packet data, in bytes
 *
 * \return  None.
 */
/*************************************************************************************************/
static void llcInitHciReadDone(uint8_t type, uint8_t *pData, uint16_t len)
{
  uint8_t *pMsg = pData;

  pMsg   -= 4;
  pMsg[0] = len & 0xFF;
  pMsg[1] = len >> 8;
  pMsg[3] = type; /* Immediately before data. */

  pMsg   -= 4;
  WsfQueueEnq(&llcInitCb.rxQ, pMsg);
}

/*************************************************************************************************/
/*!
 * \brief   Handle write completion.
 *
 * This function is called from an interrupt context.
 *
 * \param   type      Packet type.
 * \param   pData     Pointer to buffer that held written data, which was passed to
 *                   `LlcHciWrite()`
 * \param   err       Indicates success (0) or error (one of the LLC_HCI_ERROR_#### codes)
 * \param   pContext  Context pointer passed to LlcHciWrite()
 *
 * \return  None.
 */
/*************************************************************************************************/
static void llcInitHciWriteDone(uint8_t type, uint8_t *pData, int32_t err, void *pContext)
{
  llcInitCb.txCompleteFlag = TRUE;
}

/**************************************************************************************************
  BT4 Start Up
**************************************************************************************************/

/*************************************************************************************************/
/*!
 *  \brief  Wait for Rx message.
 *
 *  \param  pMsg    Storage for message.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void llcInitWaitForRxMessage(llcInitMsg_t *pMsg)
{
  uint8_t *pData;

#if (WSF_TOKEN_ENABLED == TRUE)
  /* Service tokens before waiting. */
  WsfTokenService();
#endif

  while (WsfQueueEmpty(&llcInitCb.rxQ))
  {
    ;
  }

  pData = WsfQueueDeq(&llcInitCb.rxQ);
  BYTES_TO_UINT16(pMsg->len,  pData + 4);
  pMsg->type  = pData[7];
  pMsg->pData = pData + 8;
}

/*************************************************************************************************/
/*!
 *  \brief  Tx message.
 *
 *  \param  pMsg    Message.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void llcInitTxMessage(llcInitMsg_t *pMsg)
{
  /* Write message. */
  LlcHciWrite(pMsg->type, pMsg->pData, pMsg->len, NULL);

#if (WSF_TOKEN_ENABLED == TRUE)
  /* Service tokens before waiting. */
  WsfTokenService();
#endif

  /* Wait for Tx complete. */
  while (!llcInitCb.txCompleteFlag)
  {
    ;
  }
  llcInitCb.txCompleteFlag = FALSE;
}

/*************************************************************************************************/
/*!
 *  \brief  Tx command.
 *
 *  \param  pMsg          Message.
 *  \param  waitComplete  TRUE to wait for command complete event.
 *  \param  vsCommand     TRUE if this is a VS command.
 *
 *  \return None.
 */
/*************************************************************************************************/
static bool_t llcInitTxCommand(llcInitMsg_t *pMsg, bool_t waitComplete, bool_t vsCommand)
{
  llcInitMsg_t msg;
  uint8_t   *pData;
  uint8_t   evtCode, status, extStatus;

  /* Transmit message. */
  llcInitTxMessage(pMsg);
  if (!waitComplete)
  {
    return TRUE;
  }

  /* Wait for event response. */
  llcInitWaitForRxMessage(&msg);

  if (msg.type != HCI_EVT_TYPE)
  {
    APP_TRACE_INFO2("    Rx message of type %02X != %02X", msg.type, HCI_EVT_TYPE);
    goto Fail;
  }

  pData = msg.pData;
  BSTREAM_TO_UINT8(evtCode,  pData);
  pData += 4; /* paramLen, numHciCommandPackets, commandOpcode */
  if (evtCode != HCI_CMD_CMPL_EVT)
  {
    APP_TRACE_INFO2("    Rx event code %02X != %02X", evtCode, HCI_CMD_CMPL_EVT);
    goto Fail;
  }

  BSTREAM_TO_UINT8(status, pData);
  if (vsCommand)
  {
    BSTREAM_TO_UINT8(extStatus, pData);
    if ((status != HCI_SUCCESS) || (extStatus != CORDIO_EXT_STATUS_SUCCESS))
    {
      APP_TRACE_INFO2("    Rx status %02X/%02X != 00/00", status, extStatus);
      goto Fail;
    }
  }
  else
  {
    if (status != HCI_SUCCESS)
    {
      APP_TRACE_INFO1("    Rx status %02X != 00", status);
      goto Fail;
    }
  }

  llcInitHciBufFree(msg.pData);
  return TRUE;

Fail:
  llcInitHciBufFree(msg.pData);
  return FALSE;
}

/*************************************************************************************************/
/*!
 *  \brief  Wait for startup event.
 *
 *  \return TRUE if startup event received.
 */
/*************************************************************************************************/
static bool_t llcInitWaitForStartupEvent(void)
{
  llcInitMsg_t msg;
  uint8_t   *pData;
  uint8_t   evtCode;
  uint16_t  vsEvtCode;
  uint8_t   status, extStatus, version, resetReason;
  uint16_t  flags, maxSize;
  uint32_t  hwId, hwRev, swRev;

  llcInitWaitForRxMessage(&msg);

  APP_TRACE_INFO0("llcInitWaitForStartupEvent");
  if (msg.type != HCI_EVT_TYPE)
  {
    APP_TRACE_INFO2("  Rx message of type %02X != %02X", msg.type, HCI_EVT_TYPE);
    goto Fail;
  }

  pData = msg.pData;
  BSTREAM_TO_UINT8(evtCode,  pData);
  pData += 1; /* paramLen */
  if (evtCode != HCI_VENDOR_SPEC_EVT)
  {
    APP_TRACE_INFO2("  Rx event code %02X != %02X", evtCode, HCI_VENDOR_SPEC_EVT);
    goto Fail;
  }

  BSTREAM_TO_UINT16(vsEvtCode, pData);
  if (vsEvtCode != CORDIO_BOOT_EVT_STARTUP)
  {
    APP_TRACE_INFO2("  Rx vs event code %04X != %04X", vsEvtCode, CORDIO_BOOT_EVT_STARTUP);
    goto Fail;
  }

  BSTREAM_TO_UINT8(status,    pData);
  BSTREAM_TO_UINT8(extStatus, pData);
  if ((status != HCI_SUCCESS) || (extStatus != CORDIO_EXT_STATUS_SUCCESS))
  {
    APP_TRACE_INFO2("  Rx status %02X/%02X != 00/00", status, extStatus);
    goto Fail;
  }

  BSTREAM_TO_UINT8(version, pData);
  if (version != CORDIO_BOOT_STARTUP_EVT_VERSION_0)
  {
    APP_TRACE_INFO2("  Rx version %02X != %02X", version, CORDIO_BOOT_STARTUP_EVT_VERSION_0);
    goto Fail;
  }

  BSTREAM_TO_UINT8 (resetReason, pData);
  BSTREAM_TO_UINT16(flags,       pData);
  BSTREAM_TO_UINT16(maxSize,     pData);
  BSTREAM_TO_UINT32(hwId,        pData);
  BSTREAM_TO_UINT32(hwRev,       pData);
  BSTREAM_TO_UINT32(swRev,       pData);
  APP_TRACE_INFO0("  Cordio BT4 startup event");
  APP_TRACE_INFO1("  -- resetReason %02X", resetReason);
  APP_TRACE_INFO1("  -- flags       %04X", flags);
  APP_TRACE_INFO1("  -- maxSize     %u",   4 * maxSize);
  APP_TRACE_INFO1("  -- hwId        %08X", hwId);
  APP_TRACE_INFO1("  -- hwRev       %08X", hwRev);
  APP_TRACE_INFO1("  -- swRev       %08X", swRev);
  APP_TRACE_INFO1("  -- swVerStr    %s",   pData);
  llcInitHciBufFree(msg.pData);
  return TRUE;

Fail:
  llcInitHciBufFree(msg.pData);
  return FALSE;
}

/*************************************************************************************************/
/*!
 *  \brief  Upload new firmware.
 *
 *  \param  addr      Data address.
 *  \param  provider  Data provider.
 *
 *  \return TRUE if firmware upload completed.
 */
/*************************************************************************************************/
static bool_t llcInitFirmwareUpload(uint32_t addr, LlcInitDataProvider_t provider)
{
  CordioTc2ImgHdr_t hdr;
  uint32_t  dataRemain;
  uint32_t  dataCompare;
  uint16_t  flags = 0;
  llcInitMsg_t msg;
  uint8_t   *pBuf;
  uint8_t   *pCmd;

  APP_TRACE_INFO0("llcInitFirmwareUpload");

  /* Allocate buffer for firmware upload. */
  if ((pBuf = llcInitBufAlloc(256)) == NULL)
  {
    APP_TRACE_INFO0("  buffer allocation failed");
    WSF_ASSERT(FALSE);
    return FALSE;
  }
  msg.type  = HCI_CMD_TYPE;
  msg.pData = pBuf;

  /* ---------- parse header ---------- */
  provider(&addr, sizeof(hdr), (uint8_t *)&hdr);
  APP_TRACE_INFO0("  Cordio BT4 firmware image");
  APP_TRACE_INFO1("  -- type    %02X", hdr.type);
  APP_TRACE_INFO2("  -- rev     %u.%u", hdr.revMajor, hdr.revMinor);
  APP_TRACE_INFO1("  -- dataLen %u", hdr.dataLen & CORDIO_TC2_DATA_LEN_MASK);
  APP_TRACE_INFO1("     -- enc? %s", ((hdr.dataLen & CORDIO_TC2_DATA_LEN_FLAG_ENCRYPTED) != 0) ? "yes" : "no");

  /* ---------- start fw load ---------- */

  if (hdr.type == CORDIO_TC2_IMG_TYPE_BT4_PATCH)
  {
    flags |= CORDIO_BOOT_FW_LOAD_FLAG_PATCH;
  }
  if ((hdr.dataLen & CORDIO_TC2_DATA_LEN_FLAG_ENCRYPTED) != 0)
  {
    flags |= CORDIO_BOOT_FW_LOAD_FLAG_ENCRYPTED;
  }
  pCmd = pBuf;
  UINT16_TO_BSTREAM(pCmd, CORDIO_BOOT_OPCODE_FW_LOAD);
  UINT8_TO_BSTREAM (pCmd, CORDIO_BOOT_LEN_FW_LOAD);
  UINT16_TO_BSTREAM(pCmd, flags);
  UINT32_TO_BSTREAM(pCmd, hdr.dataLen & CORDIO_TC2_DATA_LEN_MASK);
  UINT32_TO_BSTREAM(pCmd, hdr.offset);
  UINT32_TO_BSTREAM(pCmd, hdr.crc);
  UINT32_TO_BSTREAM(pCmd, hdr.entry);
  msg.len = HCI_CMD_HDR_LEN + CORDIO_BOOT_LEN_FW_LOAD;
  if (!llcInitTxCommand(&msg, TRUE, TRUE))
  {
    APP_TRACE_INFO0("  Tx FW_LOAD failed");
    goto Fail;
  }
  APP_TRACE_INFO0("  Tx FW_LOAD cmd");

  /* ---------- send fw data ---------- */

  hdr.dataLen &= CORDIO_TC2_DATA_LEN_MASK;
  dataRemain   = hdr.dataLen;
  dataCompare  = hdr.dataLen / 16;
  while (dataRemain > 0)
  {
    uint8_t txLen = WSF_MIN(dataRemain, CORDIO_BOOT_DATA_LEN_MAX);

    pCmd = pBuf;
    UINT16_TO_BSTREAM(pCmd, CORDIO_BOOT_OPCODE_FW_DATA);
    UINT8_TO_BSTREAM (pCmd, CORDIO_BOOT_LEN_FW_DATA(txLen));
    provider(&addr, txLen, pCmd);
    msg.len = HCI_CMD_HDR_LEN + CORDIO_BOOT_LEN_FW_DATA(txLen);
    if (!llcInitTxCommand(&msg, TRUE, TRUE))
    {
      APP_TRACE_INFO0("  Tx FW_DATA failed");
      goto Fail;
    }
    if (hdr.dataLen - dataRemain > dataCompare)
    {
      APP_TRACE_INFO2("  Tx FW_DATA cmd [%u/%u]", hdr.dataLen - dataRemain, hdr.dataLen);
      dataCompare += hdr.dataLen / 16;
    }
    dataRemain -= txLen;
  }
  APP_TRACE_INFO0("  Tx FW_DATA done");
  llcInitBufFree(pBuf);
  return TRUE;

Fail:
  llcInitBufFree(pBuf);
  return FALSE;
}

/*************************************************************************************************/
/*!
 *  \brief  Upload new trim.
 *
 *  \param  addr      Data address.
 *  \param  provider  Data provider.
 *
 *  \return TRUE if trim upload completed.
 */
/*************************************************************************************************/
static bool_t llcInitTrimUpload(uint32_t addr, LlcInitDataProvider_t provider)
{
  CordioTc2ImgHdr_t hdr;
  uint32_t  dataLen;
  uint32_t  dataRemain;
  llcInitMsg_t msg;
  uint8_t   *pBuf;
  uint8_t   *pCmd;

  APP_TRACE_INFO0("llcInitTrimUpload");

  /* Allocate buffer for trim upload. */
  if ((pBuf = llcInitBufAlloc(256)) == NULL)
  {
    APP_TRACE_INFO0("  buffer allocation failed");
    WSF_ASSERT(FALSE);
    return FALSE;
  }
  msg.type  = HCI_CMD_TYPE;
  msg.pData = pBuf;

  /* ---------- parse header ---------- */

  provider(&addr, sizeof(hdr), (uint8_t *)&hdr);

  APP_TRACE_INFO0("  Cordio BT4 trim image");
  if (hdr.cookie != CORDIO_TC2_HEADER_COOKIE)
  {
    APP_TRACE_INFO0("    trim header is invalid");
    goto Fail;
  }
  dataLen = hdr.dataLen;
  APP_TRACE_INFO1("  -- dataLen %u", dataLen & CORDIO_TC2_DATA_LEN_MASK);

  /* ---------- upload trim ---------- */

  dataLen   &= CORDIO_TC2_DATA_LEN_MASK;
  dataRemain = dataLen;
  while (dataRemain > sizeof(uint16_t) * 2)
  {
    /* ---------- parse header ---------- */

    CordioTc2TrimHdr_t trimHdr;

    provider(&addr, sizeof(trimHdr), (uint8_t *)&trimHdr);
    dataRemain -= sizeof(trimHdr);
    APP_TRACE_INFO3("  Cordio BT4 trim type %04X, len %u, param %u", trimHdr.type, trimHdr.len, trimHdr.param);
    if (trimHdr.len > dataRemain)
    {
      APP_TRACE_INFO0("    trim is too long");
      goto Fail;
    }
    dataRemain -= trimHdr.len;

    /* ---------- check tx power level ---------- */

    if (llcInitCb.txPowerLevelSet)
    {
      if ((trimHdr.type == CORDIO_TRIM_TYPE_TX_1V) || (trimHdr.type == CORDIO_TRIM_TYPE_TX_3V))
      {
        if ((int8_t)trimHdr.param != llcInitCb.txPowerLevel)
        {
          APP_TRACE_INFO2("    Ignoring trim for other txPowerLevel [%d != %d]", (int8_t)trimHdr.param, llcInitCb.txPowerLevel);
          addr += trimHdr.len;
          continue;
        }
      }
    }

    /* ---------- start trim load ---------- */

    pCmd = pBuf;
    UINT16_TO_BSTREAM(pCmd, CORDIO_BOOT_OPCODE_TRIM_LOAD);
    UINT8_TO_BSTREAM (pCmd, CORDIO_BOOT_LEN_TRIM_LOAD);
    UINT16_TO_BSTREAM(pCmd, trimHdr.type);
    UINT16_TO_BSTREAM(pCmd, trimHdr.len);
    msg.len = HCI_CMD_HDR_LEN + CORDIO_BOOT_LEN_TRIM_LOAD;
    if (!llcInitTxCommand(&msg, TRUE, TRUE))
    {
      APP_TRACE_INFO0("    Tx TRIM_LOAD failed [continuing]");
      addr += trimHdr.len;
      continue;
    }
    APP_TRACE_INFO0("    Tx TRIM_LOAD done");

    /* ---------- send trim data ---------- */

    while (trimHdr.len > 0)
    {
      uint16_t trimLenTemp = WSF_MIN(trimHdr.len, CORDIO_BOOT_DATA_LEN_MAX);

      pCmd = pBuf;
      UINT16_TO_BSTREAM(pCmd, CORDIO_BOOT_OPCODE_TRIM_DATA);
      UINT8_TO_BSTREAM (pCmd, CORDIO_BOOT_LEN_TRIM_DATA(trimLenTemp));
      provider(&addr, trimLenTemp, pCmd);
      trimHdr.len -= trimLenTemp;
      msg.len = HCI_CMD_HDR_LEN + CORDIO_BOOT_LEN_TRIM_DATA(trimLenTemp);
      if (!llcInitTxCommand(&msg, TRUE, TRUE))
      {
        APP_TRACE_INFO0("    Tx TRIM_DATA failed");
        goto Fail;
      }
      APP_TRACE_INFO0("    Tx TRIM_DATA done");
    }
  }
  llcInitBufFree(pBuf);
  return TRUE;

Fail:
  llcInitBufFree(pBuf);
  return FALSE;
}

/*************************************************************************************************/
/*!
 *  \brief  Set address and provider for firmware.
 *
 *  \param  addr            Address.
 *  \param  provider        Provider for firmware data.
 *
 *  \return None.
 */
/*************************************************************************************************/
void LlcInitSetFirmwareProvider(uint32_t addr, LlcInitDataProvider_t provider)
{
  llcInitCb.fwAddr     = addr;
  llcInitCb.fwProvider = provider;
}

/*************************************************************************************************/
/*!
 *  \brief  Set address and provider for trim.
 *
 *  \param  addr            Address.
 *  \param  provider        Provider for trim data.
 *
 *  \return None.
 */
/*************************************************************************************************/
void LlcInitSetTrimProvider(uint32_t addr, LlcInitDataProvider_t provider)
{
  llcInitCb.trimAddr     = addr;
  llcInitCb.trimProvider = provider;
}

/*************************************************************************************************/
/*!
 *  \brief  Set address and provider for patch.
 *
 *  \param  addr            Address.
 *  \param  provider        Provider for patch data.
 *
 *  \return None.
 */
/*************************************************************************************************/
void LlcInitSetPatchProvider(uint32_t addr, LlcInitDataProvider_t provider)
{
  llcInitCb.patchAddr     = addr;
  llcInitCb.patchProvider = provider;
}

/*************************************************************************************************/
/*!
 *  \brief  Set transmit power.
 *
 *  \param  txPowerLevel    Transmit power level.
 *
 *  \return None.
 */
/*************************************************************************************************/
void LlcInitSetTxPower(int8_t txPowerLevel)
{
  llcInitCb.txPowerLevel    = txPowerLevel;
  llcInitCb.txPowerLevelSet = TRUE;
}

/*************************************************************************************************/
/*!
 *  \brief  Set timeout for LLC responses.
 *
 *  \param  timeout         Timeout.
 *
 *  \return None.
 */
/*************************************************************************************************/
void LlcInitSetTimeout(uint8_t timeout)
{
  llcInitCb.timeout = 0;
}

/*************************************************************************************************/
/*!
 *  \brief  Perform LLC initialization.
 *
 *  \return None.
 */
/*************************************************************************************************/
bool_t LlcInit(void)
{
  LlcHciInit();
  LlcHciSetCbacks(&llcInitHciCbacks);
  LlcHciTakeUp();

  /* ---------- start up BT4 ---------- */

  /* ROM bootloader startup. */
  if (!llcInitWaitForStartupEvent())
  {
    goto Fail;
  }

  /* FW upload. */
  if (llcInitCb.fwProvider != NULL)
  {
    if (!llcInitFirmwareUpload(llcInitCb.fwAddr, llcInitCb.fwProvider) ||
        !llcInitWaitForStartupEvent())
    {
      goto Fail;
    }
  }
  else
  {
    APP_TRACE_INFO0("skipping firmware upload");
  }

  /* Trim upload. */
  if (llcInitCb.trimProvider != NULL)
  {
    if (!llcInitTrimUpload(llcInitCb.trimAddr, llcInitCb.trimProvider))
    {
      goto Fail;
    }
  }
  else
  {
    APP_TRACE_INFO0("skipping trim upload");
  }

  /* Patch upload. */
  if (llcInitCb.patchProvider != NULL)
  {
    if (!llcInitFirmwareUpload(llcInitCb.patchAddr, llcInitCb.patchProvider))
    {
      goto Fail;
    }
  }
  else
  {
    APP_TRACE_INFO0("skipping patch upload");
  }

  LlcHciTakeDown();

  APP_TRACE_INFO0("startup complete");
  return TRUE;

Fail:
  LlcHciTakeDown();

  APP_TRACE_INFO0("startup failed");
  return FALSE;
}
