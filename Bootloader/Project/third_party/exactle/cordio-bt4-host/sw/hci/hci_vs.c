/*************************************************************************************************/
/*!
 *  \file   hci_vs.c
 *
 *  \brief  HCI vendor specific functions for Cordio BT4.
 *
 *          $Date: 2016-08-28 14:40:28 -0700 (Sun, 28 Aug 2016) $
 *          $Revision: 8609 $
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
#include "wsf_msg.h"
#include "wsf_trace.h"
#include "bda.h"
#include "bstream.h"
#include "hci_core.h"
#include "hci_api.h"
#include "hci_main.h"
#include "hci_cmd.h"
#include "hci_vs.h"

#include "llc_hci.h"
#include "cordio_ext_defs.h"
#include "cordio_cfg_defs.h"

/**************************************************************************************************
  Data Types
**************************************************************************************************/

/*! Configurations. */
enum
{
  HCI_VS_CFG_ACL_BUFS = 0,
  HCI_VS_CFG_VERSION,
  HCI_VS_CFG_DEV_FILT,
  HCI_VS_CFG_DEF_TX_PWR_LVL,
  HCI_VS_CFG_CE_JITTER,
  HCI_VS_CFG_MAX_CONN,
  HCI_VS_CFG_DTM_RX_SYNC,
  HCI_VS_CFG_BUFS,
  HCI_VS_CFG_MAX_SCAN_PERIOD,
  HCI_VS_CFG_MAX_ADV_REPORTS,
  HCI_VS_CFG_NUM
};

/**************************************************************************************************
  Macros
**************************************************************************************************/

/*! number of pools */
#define HCI_VS_NUM_POOLS      3

/**************************************************************************************************
  Global Variables
**************************************************************************************************/

/*! VS control block */
struct
{
  bdAddr_t addr;                        /*! BD_ADDR */
  bool_t   addrSet;                     /*! TRUE if BD_ADDR has been set */

  uint16_t cfgNext;                     /*! Next configuration */
  uint16_t cfgsSetMap;                  /*! Bitmap of set configurations */

  uint16_t cfgBufsNext;                 /*! Next buffers configuration */
  uint16_t cfgsBufsSetMap;              /*! Bitmap of set buffer configurations */

  /* ----- configurations ----- */

  uint8_t  numTxBufs;                   /*! Number of Tx buffers */
  uint8_t  numRxBufs;                   /*! Number of Rx buffers */
  uint16_t maxAclLen;                   /*! Maximum ACL length */
  uint16_t compId;                      /*! Company ID */
  uint16_t implRev;                     /*! Implementation revision */
  uint8_t  btVer;                       /*! Bluetooth version */
  uint8_t  whiteListSize;               /*! White list size */
  uint8_t  resolvingListSize;           /*! Resolving list size */
  int8_t   defTxPwrLvl;                 /*! Default Tx power level */
  uint8_t  ceJitterUsec;                /*! CE jitter in microseconds */
  uint8_t  maxConn;                     /*! Maximum number of connections */
  uint16_t dtmRxSyncMs;                 /*! DTM Rx synchronization window in milliseconds */
  uint8_t  numBufs[HCI_VS_NUM_POOLS];   /*! Number of buffers in pool */
  uint16_t bufSize[HCI_VS_NUM_POOLS];   /*! Size of buffers in pool */
  uint8_t  clkAccuracyPpm;              /*! Clock accuracy in PPM */
  uint16_t maxScanPeriodMs;             /*! Maximum scan period in milliseconds */
  uint8_t  maxAdvReports;               /*! Maximum number of pending advertising reports */
} hciVsCb;

/**************************************************************************************************
  Functions
**************************************************************************************************/

/*************************************************************************************************/
/*!
*  \fn     hciCoreReadMaxDataLen
*
*  \brief  Read maximum data length command.
*
*  \return None.
*/
/*************************************************************************************************/
static void hciCoreReadMaxDataLen(void)
{
  /* if LE Data Packet Length Extensions is supported by Controller and included */
  if ((hciCoreCb.leSupFeat & HCI_LE_SUP_FEAT_DATA_LEN_EXT) &&
      (hciLeSupFeatCfg & HCI_LE_SUP_FEAT_DATA_LEN_EXT))
  {
    /* send next command in sequence */
    HciLeReadMaxDataLen();
  }
  else
  {
    /* send next command in sequence */
    HciLeRandCmd();
  }
}

/*************************************************************************************************/
/*!
*  \fn     hciCoreReadResolvingListSize
*
*  \brief  Read resolving list command.
*
*  \return None.
*/
/*************************************************************************************************/
static void hciCoreReadResolvingListSize(void)
{
  /* if LL Privacy is supported by Controller and included */
  if ((hciCoreCb.leSupFeat & HCI_LE_SUP_FEAT_PRIVACY) &&
      (hciLeSupFeatCfg & HCI_LE_SUP_FEAT_PRIVACY))
  {
    /* send next command in sequence */
    HciLeReadResolvingListSize();
  }
  else
  {
    hciCoreCb.resListSize = 0;

    /* send next command in sequence */
    hciCoreReadMaxDataLen();
  }
}

/*************************************************************************************************/
/*!
 *  \fn     hciVsSendNextCfg
 *
 *  \brief  Send the next configuration.
 *
 *  \return TRUE if a configuration was sent.
 */
/*************************************************************************************************/
static bool_t hciVsSendNextCfg(void)
{
  while (hciVsCb.cfgNext < HCI_VS_CFG_NUM)
  {
    /* set configuration if configuration set */
    if (hciVsCb.cfgsSetMap & (1 << hciVsCb.cfgNext))
    {
      switch (hciVsCb.cfgNext)
      {
        case HCI_VS_CFG_ACL_BUFS:
          HciVsSetAclBufsCmd(hciVsCb.numTxBufs, hciVsCb.numRxBufs, hciVsCb.maxAclLen);
          break;
        case HCI_VS_CFG_VERSION:
          HciVsSetVersionCmd(hciVsCb.compId, hciVsCb.implRev, hciVsCb.btVer);
          break;
        case HCI_VS_CFG_DEV_FILT:
          HciVsSetDevFiltCmd(hciVsCb.whiteListSize, hciVsCb.resolvingListSize);
          break;
        case HCI_VS_CFG_DEF_TX_PWR_LVL:
          HciVsSetDefTxPwrLvlCmd(hciVsCb.defTxPwrLvl);
          break;
        case HCI_VS_CFG_CE_JITTER:
          HciVsSetCeJitterCmd(hciVsCb.ceJitterUsec);
          break;
        case HCI_VS_CFG_MAX_CONN:
          HciVsSetMaxConnCmd(hciVsCb.maxConn);
          break;
        case HCI_VS_CFG_DTM_RX_SYNC:
          HciVsSetDtmRxSyncCmd(hciVsCb.dtmRxSyncMs);
          break;
        case HCI_VS_CFG_BUFS:
          while (hciVsCb.cfgBufsNext < HCI_VS_NUM_POOLS)
          {
            /* set buffers if buffer set */
            if (hciVsCb.cfgsBufsSetMap & (1 << hciVsCb.cfgBufsNext))
            {
              HciVsSetBufsCmd(hciVsCb.cfgBufsNext, hciVsCb.numBufs[hciVsCb.cfgBufsNext], hciVsCb.bufSize[hciVsCb.cfgBufsNext]);

              /* configure next pool or next configuration on next try */
              hciVsCb.cfgsBufsSetMap &= ~(1 << hciVsCb.cfgBufsNext);
              if (hciVsCb.cfgsBufsSetMap != 0)
              {
                return TRUE;
              }
            }

            /* next pool */
            hciVsCb.cfgBufsNext++;
          }
          break;
        case HCI_VS_CFG_MAX_SCAN_PERIOD:
          HciVsSetMaxScanPeriodCmd(hciVsCb.maxScanPeriodMs);
          break;
        case HCI_VS_CFG_MAX_ADV_REPORTS:
          HciVsSetMaxAdvReportsCmd(hciVsCb.maxAdvReports);
          break;
        default:
          return FALSE;
      }

      /* next configuration */
      hciVsCb.cfgNext++;
      return TRUE;
    }

    /* next configuration */
    hciVsCb.cfgNext++;
  }

  /* no more configurations */
  return FALSE;
}

/*************************************************************************************************/
/*!
 *  \fn     hciCoreResetStart
 *
 *  \brief  Start the HCI reset sequence.
 *
 *  \return None.
 */
/*************************************************************************************************/
void hciCoreResetStart(void)
{
  /* send a VS configuration before HCI Reset */
  if (!hciVsSendNextCfg())
  {
    /* or send an HCI Reset command to start the sequence */
    HciResetCmd();
  }
}

/*************************************************************************************************/
/*!
 *  \fn     hciCoreResetSequence
 *
 *  \brief  Implement the HCI reset sequence.
 *
 *  \param  pMsg    HCI event message from previous command in the sequence.
 *
 *  \return None.
 */
/*************************************************************************************************/
void hciCoreResetSequence(uint8_t *pMsg)
{
  uint16_t       opcode;
  wsfMsgHdr_t    hdr;
  static uint8_t randCnt;

  /* if event is a command complete event */
  if (*pMsg == HCI_CMD_CMPL_EVT)
  {
    /* parse parameters */
    pMsg += HCI_EVT_HDR_LEN;
    pMsg++;                   /* skip num packets */
    BSTREAM_TO_UINT16(opcode, pMsg);
    pMsg++;                   /* skip status */

    /* decode opcode */
    switch (opcode)
    {
      case CORDIO_CFG_OPCODE_VS_SET_ACL_BUFS:
      case CORDIO_CFG_OPCODE_VS_SET_VERSION:
      case CORDIO_CFG_OPCODE_VS_SET_DEV_FILT:
      case CORDIO_CFG_OPCODE_VS_SET_DEF_TX_PWR_LVL:
      case CORDIO_CFG_OPCODE_VS_SET_CE_JITTER:
      case CORDIO_CFG_OPCODE_VS_SET_MAX_CONN:
      case CORDIO_CFG_OPCODE_VS_SET_DTM_RX_SYNC:
      case CORDIO_CFG_OPCODE_VS_SET_BUFS:
      case CORDIO_CFG_OPCODE_VS_SET_MAX_SCAN_PERIOD:
      case CORDIO_CFG_OPCODE_VS_SET_MAX_ADV_REPORT:
        /* continue with next VS configuration */
        if (!hciVsSendNextCfg())
        {
          /* or send an HCI Reset command to start the sequence */
          HciResetCmd();
        }
        break;

      case HCI_OPCODE_RESET:
        /* initialize rand command count */
        randCnt = 0;

        /* send next command in sequence */
        HciLeSetEventMaskCmd((uint8_t *) hciLeEventMask);
        break;

      case HCI_OPCODE_LE_SET_EVENT_MASK:
        /* send next command in sequence */
        HciSetEventMaskPage2Cmd((uint8_t *)hciEventMaskPage2);
        break;

      case HCI_OPCODE_SET_EVENT_MASK_PAGE2:
      case CORDIO_EXT_OPCODE_VS_SET_BD_ADDR:
        /* send next command in sequence */
        if (hciVsCb.addrSet)
        {
          HciVsSetBdAddrCmd(hciVsCb.addr);
          hciVsCb.addrSet = FALSE;
        }
        else
        {
          HciReadBdAddrCmd();
        }
        break;

      case HCI_OPCODE_READ_BD_ADDR:
        /* parse and store event parameters */
        BdaCpy(hciCoreCb.bdAddr, pMsg);

        /* send next command in sequence */
        HciLeReadBufSizeCmd();
        break;

      case HCI_OPCODE_LE_READ_BUF_SIZE:
        /* parse and store event parameters */
        BSTREAM_TO_UINT16(hciCoreCb.bufSize, pMsg);
        BSTREAM_TO_UINT8(hciCoreCb.numBufs, pMsg);

        /* initialize ACL buffer accounting */
        hciCoreCb.availBufs = hciCoreCb.numBufs;

        /* send next command in sequence */
        HciLeReadSupStatesCmd();
        break;

      case HCI_OPCODE_LE_READ_SUP_STATES:
        /* parse and store event parameters */
        memcpy(hciCoreCb.leStates, pMsg, HCI_LE_STATES_LEN);

        /* send next command in sequence */
        HciLeReadWhiteListSizeCmd();
        break;

      case HCI_OPCODE_LE_READ_WHITE_LIST_SIZE:
        /* parse and store event parameters */
        BSTREAM_TO_UINT8(hciCoreCb.whiteListSize, pMsg);

        /* send next command in sequence */
        HciLeReadLocalSupFeatCmd();
        break;

      case HCI_OPCODE_LE_READ_LOCAL_SUP_FEAT:
        /* parse and store event parameters */
        BSTREAM_TO_UINT8(hciCoreCb.leSupFeat, pMsg);

        /* send next command in sequence */
        hciCoreReadResolvingListSize();
        break;

      case HCI_OPCODE_LE_READ_RES_LIST_SIZE:
        /* parse and store event parameters */
        BSTREAM_TO_UINT8(hciCoreCb.resListSize, pMsg);

        /* send next command in sequence */
        hciCoreReadMaxDataLen();
        break;

      case HCI_OPCODE_LE_READ_MAX_DATA_LEN:
        {
          uint16_t maxTxOctets;
          uint16_t maxTxTime;

          BSTREAM_TO_UINT16(maxTxOctets, pMsg);
          BSTREAM_TO_UINT16(maxTxTime, pMsg);

          /* use Controller's maximum supported payload octets and packet duration times
           * for transmission as Host's suggested values for maximum transmission number
           * of payload octets and maximum packet transmission time for new connections.
           */
          HciLeWriteDefDataLen(maxTxOctets, maxTxTime);
        }
        break;

      case HCI_OPCODE_LE_WRITE_DEF_DATA_LEN:
        /* send next command in sequence */
        HciLeRandCmd();
        break;

      case HCI_OPCODE_LE_RAND:
        /* check if need to send second rand command */
        if (randCnt < (HCI_RESET_RAND_CNT-1))
        {
          randCnt++;
          HciLeRandCmd();
        }
        else
        {
          /* last command in sequence; set resetting state and call callback */
          hciCb.resetting = FALSE;
          hdr.param = 0;
          hdr.event = HCI_RESET_SEQ_CMPL_CBACK_EVT;
          (*hciCb.evtCback)((hciEvt_t *) &hdr);
        }
        break;

      default:
        break;
    }
  }
}

/*************************************************************************************************/
/*!
 *  \fn     hciCoreVsCmdCmplRcvd
 *
 *  \brief  Perform internal HCI processing of vendor specific command complete events.
 *
 *  \param  opcode  HCI command opcode.
 *  \param  pMsg    Pointer to input HCI event parameter byte stream.
 *  \param  len     Parameter byte stream length.
 *
 *  \return HCI callback event code or zero.
 */
/*************************************************************************************************/
uint8_t hciCoreVsCmdCmplRcvd(uint16_t opcode, uint8_t *pMsg, uint8_t len)
{
  return HCI_VENDOR_SPEC_CMD_CMPL_CBACK_EVT;
}

/*************************************************************************************************/
/*!
 *  \fn     hciCoreVsEvtRcvd
 *
 *  \brief  Perform internal HCI processing of vendor specific HCI events.
 *
 *  \param  p       Pointer to input HCI event parameter byte stream.
 *  \param  len     Parameter byte stream length.
 *
 *  \return HCI callback event code or zero.
 */
/*************************************************************************************************/
uint8_t hciCoreVsEvtRcvd(uint8_t *p, uint8_t len)
{
  return HCI_VENDOR_SPEC_EVT;
}

/*************************************************************************************************/
/*!
 *  \fn     hciCoreHwErrorRcvd
 *
 *  \brief  Perform internal HCI processing of hardware error event.
 *
 *  \param  p       Pointer to input HCI event parameter byte stream.
 *
 *  \return HCI callback event code or zero.
 */
/*************************************************************************************************/
uint8_t hciCoreHwErrorRcvd(uint8_t *p)
{
  return 0;
}

/*************************************************************************************************/
/*!
 *  \brief    Allocate a buffer for a received message.
 *
 *  This function is called from an interrupt context.
 *
 *  \param    len     Length of buffer
 *
**  \return   Pointer to buffer or NULL if buffer could not be allocated
 */
/*************************************************************************************************/
static uint8_t *hciVsBufAlloc(uint16_t len)
{
  return (uint8_t *)WsfMsgAlloc(len);
}

/*************************************************************************************************/
/*!
 *  \brief    Free a buffer previously allocated with `BufAlloc()`.
 *
 *  \param    pData     Pointer to buffer
 */
/*************************************************************************************************/
static void hciVsBufFree(uint8_t *pData)
{
  WsfMsgFree(pData);
}

/*************************************************************************************************/
/*!
 *  \brief    Handle read completion.
 *
 *  This function is called from an interrupt context.
 *
 *  \param    type    Packet type
 *  \param    pData   Packet data, which was allocated with BufAlloc().  The caller must free
 *                    this buffer
 *  \param    len     Length of packet data, in bytes
 *
 *  \return   None.
 */
/*************************************************************************************************/
static void hciVsReadDone(uint8_t type, uint8_t *pData, uint16_t len)
{
  switch (type)
  {
    /* Route standard types to stack. */
    case HCI_CMD_TYPE:
    case HCI_ACL_TYPE:
    case HCI_EVT_TYPE:
    {
      hciCoreRecv(type, pData);
      break;
    }

    /* We should not get anything else. */
    default:
    {
      WsfMsgFree(pData);
      break;
    }
  }
}

/*************************************************************************************************/
/*!
 *  \brief    Handle write completion.
 *
 *  This function is called from an interrupt context.
 *
 *  \param    type      Packet type.
 *  \param    pData     Written data.
 *  \param    err       0 upon success.
 *  \param    pContext  Context pointer.
 *
 *  \return   None.
 */
/*************************************************************************************************/
static void hciVsWriteDone(uint8_t type, uint8_t *pData, int32_t err, void *pContext)
{
  switch (type)
  {
    case HCI_CMD_TYPE:
    {
      WsfMsgFree(pData);
      break;
    }

    case HCI_ACL_TYPE:
    {
      hciCoreTxAclComplete(pContext, pData);
      break;
    }
  }
}

/*************************************************************************************************/
/*!
 *  \fn     HciVsInit
 *
 *  \brief  Vendor-specific controller initialization function.
 *
 *  \param  param    Vendor-specific parameter.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciVsInit(uint8_t param)
{
  static const LlcHciCbacks_t cbacks =
  {
    hciVsBufAlloc,
    hciVsBufFree,
    hciVsReadDone,
    hciVsWriteDone
  };

  memset(&hciVsCb, 0, sizeof(hciVsCb));

  LlcHciSetCbacks(&cbacks);
  LlcHciTakeUp();
}

/*************************************************************************************************/
/*!
 *  \fn     HciVsCfgBdAddr
 *
 *  \brief  Configure BD_ADDR.
 *
 *  \param  addr      BD_ADDR.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciVsCfgBdAddr(bdAddr_t addr)
{
  memcpy(hciVsCb.addr, addr, BDA_ADDR_LEN);
  hciVsCb.addrSet = TRUE;
}

/*************************************************************************************************/
/*!
 *  \fn     HciVsCfgAclBufs
 *
 *  \brief  Configure ACL buffers.
 *
 *  \param  numTxBufs   Number of transmit buffers.
 *  \param  numRxBufs   Number of receive buffers.
 *  \param  maxAclSize  Maximum ACL buffer size.
 *
 *  \return Status error code.
 */
/*************************************************************************************************/
void HciVsCfgAclBufs(uint8_t numTxBufs, uint8_t numRxBufs, uint16_t maxAclLen)
{
  hciVsCb.numTxBufs = numTxBufs;
  hciVsCb.numRxBufs = numRxBufs;
  hciVsCb.maxAclLen = maxAclLen;
  hciVsCb.cfgsSetMap |= 1 << HCI_VS_CFG_ACL_BUFS;
}

/*************************************************************************************************/
/*!
 *  \fn     HciVsCfgVersion
 *
 *  \brief  Configure version.
 *
 *  \param  compId    Company ID.
 *  \param  implRev   Implementation revision number.
 *  \param  btVer     Core specification implementation level.
 *
 *  \return Status error code.
 */
/*************************************************************************************************/
void HciVsCfgVersion(uint16_t compId, uint16_t implRev, uint8_t btVer)
{
  hciVsCb.compId  = compId;
  hciVsCb.implRev = implRev;
  hciVsCb.btVer   = btVer;
  hciVsCb.cfgsSetMap |= 1 << HCI_VS_CFG_VERSION;
}

/*************************************************************************************************/
/*!
 *  \fn     HciVsCfgDevFilt
 *
 *  \brief  Configure device filters.
 *
 *  \param  whiteListSize       Number of white list entries.
 *  \param  resolvingListSize   Number of resolving list entries.
 *
 *  \return Status error code.
 */
/*************************************************************************************************/
void HciVsCfgDevFilt(uint8_t whiteListSize, uint8_t resolvingListSize)
{
  hciVsCb.whiteListSize     = whiteListSize;
  hciVsCb.resolvingListSize = resolvingListSize;
  hciVsCb.cfgsSetMap |= 1 << HCI_VS_CFG_DEV_FILT;
}

/*************************************************************************************************/
/*!
 *  \fn     HciVsCfgDefTxPwrLvl
 *
 *  \brief  Configure default transmit power level.
 *
 *  \param  defTxPwrLvl         Default transmit power level.
 *
 *  \return Status error code.
 */
/*************************************************************************************************/
void HciVsCfgDefTxPwrLvl(int8_t defTxPwrLvl)
{
  hciVsCb.defTxPwrLvl = defTxPwrLvl;
  hciVsCb.cfgsSetMap |= 1 << HCI_VS_CFG_DEF_TX_PWR_LVL;
}

/*************************************************************************************************/
/*!
 *  \fn     HciVsCfgCeJitter
 *
 *  \brief  Configure CE jitter on a slave in microseconds.
 *
 *  \param  ceJitterUsec        CE jitter in microseconds.
 *
 *  \return Status error code.
 */
/*************************************************************************************************/
void HciVsCfgCeJitter(uint8_t ceJitterUsec)
{
  hciVsCb.ceJitterUsec = ceJitterUsec;
  hciVsCb.cfgsSetMap |= 1 << HCI_VS_CFG_CE_JITTER;
}

/*************************************************************************************************/
/*!
 *  \fn     HciVsCfgMaxConn
 *
 *  \brief  Configure maximum number of connections.
 *
 *  \param  maxConn             Maximum number of connections.
 *
 *  \return Status error code.
 */
/*************************************************************************************************/
void HciVsCfgMaxConn(uint8_t maxConn)
{
  hciVsCb.maxConn = maxConn;
  hciVsCb.cfgsSetMap |= 1 << HCI_VS_CFG_MAX_CONN;
}

/*************************************************************************************************/
/*!
 *  \fn     HciVsCfgDtmRxSync
 *
 *  \brief  Configure DTM Rx synchronization window in milliseconds.
 *
 *  \param  dtmRxSyncMs         DTM Rx synchronization window in milliseconds.
 *
 *  \return Status error code.
 */
/*************************************************************************************************/
void HciVsCfgDtmRxSync(uint16_t dtmRxSyncMs)
{
  hciVsCb.dtmRxSyncMs = dtmRxSyncMs;
  hciVsCb.cfgsSetMap |= 1 << HCI_VS_CFG_DTM_RX_SYNC;
}

/*************************************************************************************************/
/*!
 *  \fn     HciVsCfgBufs
 *
 *  \brief  Configure buffer pool.
 *
 *  \param  poolIdx             Pool index.
 *  \param  numBufs             Number of buffers.
 *  \param  bufSize             Buffer size.
 *
 *  \return Status error code.
 */
/*************************************************************************************************/
void HciVsCfgBufs(uint8_t poolIdx, uint8_t numBufs, uint16_t bufSize)
{
  if (poolIdx < HCI_VS_NUM_POOLS)
  {
    hciVsCb.bufSize[poolIdx] = bufSize;
    hciVsCb.numBufs[poolIdx] = numBufs;
    hciVsCb.cfgsSetMap     |= 1 << HCI_VS_CFG_BUFS;
    hciVsCb.cfgsBufsSetMap |= 1 << poolIdx;
  }
}

/*************************************************************************************************/
/*!
 *  \fn     HciVsCfgMaxScanPeriod
 *
 *  \brief  Configure maximum scan period in milliseconds.
 *
 *  \param  maxScanPeriodMs         Maximum scan period in milliseconds.
 *
 *  \return Status error code.
 */
/*************************************************************************************************/
void HciVsCfgMaxScanPeriod(uint16_t maxScanPeriodMs)
{
  hciVsCb.maxScanPeriodMs = maxScanPeriodMs;
  hciVsCb.cfgsSetMap |= 1 << HCI_VS_CFG_MAX_SCAN_PERIOD;
}

/*************************************************************************************************/
/*!
 *  \fn     HciVsCfgMaxAdvReports
 *
 *  \brief  Configure maximum number of pending advertising reports.
 *
 *  \param  maxAdvReports           Maximum number of pending advertising reports.
 *
 *  \return Status error code.
 */
/*************************************************************************************************/
void HciVsCfgMaxAdvReports(uint8_t maxAdvReports)
{
  hciVsCb.maxAdvReports = maxAdvReports;
  hciVsCb.cfgsSetMap |= 1 << HCI_VS_CFG_MAX_ADV_REPORTS;
}

/*************************************************************************************************/
/*!
*  \fn     HciVsSetBdAddrCmd
*
*  \brief  HCI VS set BD_ADDR command.
*
*  \param addr        BD_ADDR.
*
*  \return None.
*/
/*************************************************************************************************/
void HciVsSetBdAddrCmd(bdAddr_t addr)
{
  uint8_t *pBuf;
  uint8_t *p;

  if ((pBuf = hciCmdAlloc(CORDIO_EXT_OPCODE_VS_SET_BD_ADDR, BDA_ADDR_LEN)) != NULL)
  {
    p = pBuf + HCI_CMD_HDR_LEN;
    memcpy(p, addr, BDA_ADDR_LEN);
    hciCmdSend(pBuf);
  }
}

/*************************************************************************************************/
/*!
 *  \fn     HciVsSetAclBufsCmd
 *
 *  \brief  HCI VS set ACL buffers command.
 *
 *  \param  numTxBufs   Number of transmit buffers.
 *  \param  numRxBufs   Number of receive buffers.
 *  \param  maxAclSize  Maximum ACL buffer size.
 *
 *  \return Status error code.
 */
/*************************************************************************************************/
void HciVsSetAclBufsCmd(uint8_t numTxBufs, uint8_t numRxBufs, uint16_t maxAclLen)
{
  uint8_t *pBuf;
  uint8_t *p;

  if ((pBuf = hciCmdAlloc(CORDIO_CFG_OPCODE_VS_SET_ACL_BUFS, (sizeof(uint8_t) * 2) + sizeof(uint16_t))) != NULL)
  {
    p = pBuf + HCI_CMD_HDR_LEN;
    UINT8_TO_BSTREAM (p, numTxBufs);
    UINT8_TO_BSTREAM (p, numRxBufs);
    UINT16_TO_BSTREAM(p, maxAclLen);
    hciCmdSend(pBuf);
  }
}

/*************************************************************************************************/
/*!
 *  \fn     HciVsSetVersionCmd
 *
 *  \brief  HCI VS set version command.
 *
 *  \param  compId    Company ID.
 *  \param  implRev   Implementation revision number.
 *  \param  btVer     Core specification implementation level.
 *
 *  \return Status error code.
 */
/*************************************************************************************************/
void HciVsSetVersionCmd(uint16_t compId, uint16_t implRev, uint8_t btVer)
{
  uint8_t *pBuf;
  uint8_t *p;

  if ((pBuf = hciCmdAlloc(CORDIO_CFG_OPCODE_VS_SET_VERSION, (sizeof(uint16_t) * 2) + sizeof(uint8_t))) != NULL)
  {
    p = pBuf + HCI_CMD_HDR_LEN;
    UINT16_TO_BSTREAM(p, compId);
    UINT16_TO_BSTREAM(p, implRev);
    UINT8_TO_BSTREAM (p, btVer);
    hciCmdSend(pBuf);
  }
}

/*************************************************************************************************/
/*!
 *  \fn     HciVsSetDevFiltCmd
 *
 *  \brief  HCI VS set device filters command.
 *
 *  \param  whiteListSize       Number of white list entries.
 *  \param  resolvingListSize   Number of resolving list entries.
 *
 *  \return Status error code.
 */
/*************************************************************************************************/
void HciVsSetDevFiltCmd(uint8_t whiteListSize, uint8_t resolvingListSize)
{
  uint8_t *pBuf;
  uint8_t *p;

  if ((pBuf = hciCmdAlloc(CORDIO_CFG_OPCODE_VS_SET_DEV_FILT, sizeof(uint8_t) * 2)) != NULL)
  {
    p = pBuf + HCI_CMD_HDR_LEN;
    UINT8_TO_BSTREAM(p, whiteListSize);
    UINT8_TO_BSTREAM(p, resolvingListSize);
    hciCmdSend(pBuf);
  }
}

/*************************************************************************************************/
/*!
 *  \fn     HciVsSetDefTxPwrLvlCmd
 *
 *  \brief  HCI VS set default transmit power level command.
 *
 *  \param  defTxPwrLvl         Default transmit power level.
 *
 *  \return Status error code.
 */
/*************************************************************************************************/
void HciVsSetDefTxPwrLvlCmd(int8_t defTxPwrLvl)
{
  uint8_t *pBuf;
  uint8_t *p;

  if ((pBuf = hciCmdAlloc(CORDIO_CFG_OPCODE_VS_SET_DEF_TX_PWR_LVL, sizeof(int8_t))) != NULL)
  {
    p = pBuf + HCI_CMD_HDR_LEN;
    UINT8_TO_BSTREAM(p, defTxPwrLvl);
    hciCmdSend(pBuf);
  }
}

/*************************************************************************************************/
/*!
 *  \fn     HciVsSetCeJitterCmd
 *
 *  \brief  HCI VS set CE jitter command.
 *
 *  \param  ceJitterUsec        CE jitter in microseconds.
 *
 *  \return Status error code.
 */
/*************************************************************************************************/
void HciVsSetCeJitterCmd(uint8_t ceJitterUsec)
{
  uint8_t *pBuf;
  uint8_t *p;

  if ((pBuf = hciCmdAlloc(CORDIO_CFG_OPCODE_VS_SET_CE_JITTER, sizeof(uint8_t))) != NULL)
  {
    p = pBuf + HCI_CMD_HDR_LEN;
    UINT8_TO_BSTREAM(p, ceJitterUsec);
    hciCmdSend(pBuf);
  }
}

/*************************************************************************************************/
/*!
 *  \fn     HciVsSetMaxConnCmd
 *
 *  \brief  HCI VS set maximum number of connections command.
 *
 *  \param  maxConn             Maximum number of connections.
 *
 *  \return Status error code.
 */
/*************************************************************************************************/
void HciVsSetMaxConnCmd(uint8_t maxConn)
{
  uint8_t *pBuf;
  uint8_t *p;

  if ((pBuf = hciCmdAlloc(CORDIO_CFG_OPCODE_VS_SET_MAX_CONN, sizeof(uint8_t))) != NULL)
  {
    p = pBuf + HCI_CMD_HDR_LEN;
    UINT8_TO_BSTREAM(p, maxConn);
    hciCmdSend(pBuf);
  }
}

/*************************************************************************************************/
/*!
 *  \fn     HciVsSetDtmRxSyncCmd
 *
 *  \brief  HCI VS set DTM synchronization window command.
 *
 *  \param  dtmRxSyncMs         DTM Rx synchronization window in milliseconds.
 *
 *  \return Status error code.
 */
/*************************************************************************************************/
void HciVsSetDtmRxSyncCmd(uint16_t dtmRxSyncMs)
{
  uint8_t *pBuf;
  uint8_t *p;

  if ((pBuf = hciCmdAlloc(CORDIO_CFG_OPCODE_VS_SET_DTM_RX_SYNC, sizeof(uint16_t))) != NULL)
  {
    p = pBuf + HCI_CMD_HDR_LEN;
    UINT16_TO_BSTREAM(p, dtmRxSyncMs);
    hciCmdSend(pBuf);
  }
}

/*************************************************************************************************/
/*!
 *  \fn     HciVsSetBufsCmd
 *
 *  \brief  HCI VS set buffer pool command.
 *
 *  \param  poolIdx             Pool index.
 *  \param  numBufs             Number of buffers.
 *  \param  bufSize             Buffer size.
 *
 *  \return Status error code.
 */
/*************************************************************************************************/
void HciVsSetBufsCmd(uint8_t poolIdx, uint8_t numBufs, uint16_t bufSize)
{
  uint8_t *pBuf;
  uint8_t *p;

  if ((pBuf = hciCmdAlloc(CORDIO_CFG_OPCODE_VS_SET_BUFS, (sizeof(uint8_t) * 2) + sizeof(uint16_t))) != NULL)
  {
    p = pBuf + HCI_CMD_HDR_LEN;
    UINT8_TO_BSTREAM (p, poolIdx);
    UINT8_TO_BSTREAM (p, numBufs);
    UINT16_TO_BSTREAM(p, bufSize);
    hciCmdSend(pBuf);
  }
}

/*************************************************************************************************/
/*!
 *  \fn     HciVsSetMaxScanPeriodCmd
 *
 *  \brief  HCI VS set maximum scan period command.
 *
 *  \param  maxScanPeriodMs         Maximum scan period in milliseconds.
 *
 *  \return Status error code.
 */
/*************************************************************************************************/
void HciVsSetMaxScanPeriodCmd(uint16_t maxScanPeriodMs)
{
  uint8_t *pBuf;
  uint8_t *p;

  if ((pBuf = hciCmdAlloc(CORDIO_CFG_OPCODE_VS_SET_MAX_SCAN_PERIOD, sizeof(uint16_t))) != NULL)
  {
    p = pBuf + HCI_CMD_HDR_LEN;
    UINT16_TO_BSTREAM(p, maxScanPeriodMs);
    hciCmdSend(pBuf);
  }
}

/*************************************************************************************************/
/*!
 *  \fn     HciVsSetMaxAdvReportsCmd
 *
 *  \brief  HCI VS set maximum number of pending advertising reports command.
 *
 *  \param  maxAdvReports           Maximum number of pending advertising reports.
 *
 *  \return Status error code.
 */
/*************************************************************************************************/
void HciVsSetMaxAdvReportsCmd(uint8_t maxAdvReports)
{
  uint8_t *pBuf;
  uint8_t *p;

  if ((pBuf = hciCmdAlloc(CORDIO_CFG_OPCODE_VS_SET_MAX_ADV_REPORT, sizeof(uint8_t))) != NULL)
  {
    p = pBuf + HCI_CMD_HDR_LEN;
    UINT8_TO_BSTREAM(p, maxAdvReports);
    hciCmdSend(pBuf);
  }
}
