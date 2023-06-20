/*************************************************************************************************/
/*!
 *  \file   app_slave.c
 *
 *  \brief  Application framework module for slave.
 *
 *          $Date: 2016-08-22 17:32:42 -0700 (Mon, 22 Aug 2016) $
 *          $Revision: 8489 $
 *
 *  Copyright (c) 2016 ARM, Ltd., all rights reserved.
 *  ARM confidential and proprietary.
 *
 *  IMPORTANT.  Your use of this file is governed by a Software License Agreement
 *  ("Agreement") that must be accepted in order to download or otherwise receive a
 *  copy of this file.  You may not use or copy this file for any purpose other than
 *  as described in the Agreement.  If you do not agree to all of the terms of the
 *  Agreement do not use this file and delete all copies in your possession or control;
 *  if you do not have a copy of the Agreement, you must contact Wicentric, Inc. prior
 *  to any use, copying or further distribution of this software.
 */
/*************************************************************************************************/

#include <string.h>
#include "wsf_types.h"
#include "wsf_msg.h"
#include "wsf_timer.h"
#include "wsf_assert.h"
#include "calc128.h"
#include "dm_api.h"
#include "att_api.h"
#include "svc_core.h"
#include "app_api.h"
#include "app_main.h"

/**************************************************************************************************
  Macros
**************************************************************************************************/

/* Convert data storage location to discoverable/connectable mode */
#define APP_LOC_2_MODE(loc)       ((loc) / 2)

/* Convert data storage location to DM advertising data location (adv or scan data) */
#define APP_LOC_2_DM_LOC(loc)     ((loc) & 1)

/* Convert mode advertising data location */
#define APP_MODE_2_ADV_LOC(mode)  (((mode) * 2) + DM_DATA_LOC_ADV)

/* Convert mode scan data location */
#define APP_MODE_2_SCAN_LOC(mode) (((mode) * 2) + DM_DATA_LOC_SCAN)

/**************************************************************************************************
  Golbal Variables
**************************************************************************************************/

/* Slave control block */
appSlaveCb_t appSlaveCb;

/**************************************************************************************************
  Local Functions
**************************************************************************************************/

static void appSlaveProcConnOpen(dmEvt_t *pMsg, appConnCb_t *pCb);
static void appSlaveProcConnClose(dmEvt_t *pMsg, appConnCb_t *pCb);

/*************************************************************************************************/
/*!
 *  \fn     appSlaveAdvModeInit
 *
 *  \brief  Initialize advertising mode.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void appSlaveAdvModeInit(void)
{
  /* initialize advertising callbacks */
  appSlaveCb.advStopCback = NULL;
  appSlaveCb.advRestartCback = NULL;
}

/*************************************************************************************************/
/*!
 *  \fn     appSlaveConnectableAdv
 *
 *  \brief  Check if connectable advertising is enabled for a given advertising set.
 *
 *  \param  numSets      Number of advertising sets.
 *  \param  pAdvHandles  Advertising handles array.
 *
 *  \return TRUE if connectable advertising enabled. FALSE, otherwise.
 */
/*************************************************************************************************/
static bool_t appSlaveConnectableAdv(uint8_t numSets, uint8_t *pAdvHandles)
{
  uint8_t i, j;

  /* for each advertising set in advHandle */
  for (i = 0; i < numSets; i++)
  {
    /* for each advertising set */
    for (j = 0; j < DM_NUM_ADV_SETS; j++)
    {
      /* if connectable advertising */
      if ((pAdvHandles[i] == j)                            &&
          ((appSlaveCb.advType[j] == DM_ADV_CONN_UNDIRECT) ||
           (appSlaveCb.advType[j] == DM_EXT_ADV_CONN_UNDIRECT)))
      {
        return TRUE;
      }
    }
  }

  return FALSE;
}

/*************************************************************************************************/
/*!
*  \fn     appSlaveForceAdvDataSync
*
*  \brief  Force update of advertising and scan response data.
*
*  \param  advHandle Advertising handle.
*  \param  mode      Discoverable/connectable mode.
*
*  \return None.
*/
/*************************************************************************************************/
static void appSlaveForceAdvDataSync(uint8_t advHandle, uint8_t mode)
{
  uint8_t advLoc;
  uint8_t scanLoc;

  /* get advertising/scan data location based on mode */
  advLoc = APP_MODE_2_ADV_LOC(mode);
  scanLoc = APP_MODE_2_SCAN_LOC(mode);

  /* force update of advertising data */
  appSlaveCb.advDataOffset[advHandle][advLoc] = 0;
  appSlaveCb.advDataOffset[advHandle][scanLoc] = 0;
  appSlaveCb.advDataSynced[advHandle] = FALSE;
}

/*************************************************************************************************/
/*!
*  \fn     appSetAdvScanDataFrag
*
*  \brief  Set advertising and scan response data fragment.
*
*  \param  mode      Discoverable/connectable mode.
*
*  \return None.
*/
/*************************************************************************************************/
static void appSetAdvScanDataFrag(uint8_t advHandle, uint8_t location)
{
  uint8_t  op;
  uint16_t fragLen;
  uint16_t remainLen;
  uint8_t  *pAdvData;

  /* get data pointer and remaining data length */
  pAdvData = appSlaveCb.pAdvData[advHandle][location];
  remainLen = appSlaveCb.advDataLen[advHandle][location] - appSlaveCb.advDataOffset[advHandle][location];

  /* if remaing data length > max adv data length supported by Controller */
  if (remainLen > appSlaveCb.maxAdvDataLen[advHandle])
  {
    remainLen = appSlaveCb.maxAdvDataLen[advHandle];
  }

  /* while there remains data to be sent */
  while (remainLen > 0)
  {
    /* if remaing data length > max length of extended advertising data (per set adv data command) */
    if (remainLen > HCI_EXT_ADV_DATA_LEN)
    {
      /* data needs to be fragmented */
      if (appSlaveCb.advDataOffset[advHandle][location] == 0)
      {
        op = HCI_ADV_DATA_OP_FRAG_FIRST;
      }
      else
      {
        op = HCI_ADV_DATA_OP_FRAG_INTER;
      }

      fragLen = HCI_EXT_ADV_DATA_LEN;
    }
    else
    {
      /* no fragmentation needed */
      if (appSlaveCb.advDataOffset[advHandle][location] == 0)
      {
        op = HCI_ADV_DATA_OP_COMP_FRAG;
      }
      else
      {
        op = HCI_ADV_DATA_OP_FRAG_LAST;
      }

      fragLen = remainLen;
    }

    /* send adv data */
    DmAdvSetData(advHandle, op, APP_LOC_2_DM_LOC(location), (uint8_t)fragLen,
                 &(pAdvData[appSlaveCb.advDataOffset[advHandle][location]]));

    /* store adv data offset */
    appSlaveCb.advDataOffset[advHandle][location] += fragLen;

    /* update remaining data length */
    remainLen -= fragLen;
  }
}

/*************************************************************************************************/
/*!
*  \fn     appSetAdvScanData
*
*  \brief  Set advertising and scan response data.
*
*  \param  mode      Discoverable/connectable mode.
*
*  \return None.
*/
/*************************************************************************************************/
static void appSetAdvScanData(uint8_t advHandle, uint8_t mode)
{
  uint8_t advLoc;
  uint8_t scanLoc;

  /* get advertising/scan data location based on mode */
  advLoc = APP_MODE_2_ADV_LOC(mode);
  scanLoc = APP_MODE_2_SCAN_LOC(mode);

  /* set advertising data */
  if (appSlaveCb.advDataOffset[advHandle][advLoc] < appSlaveCb.advDataLen[advHandle][advLoc])
  {
    appSetAdvScanDataFrag(advHandle, advLoc);
  }

  /* set scan data */
  if (appSlaveCb.advDataOffset[advHandle][scanLoc] < appSlaveCb.advDataLen[advHandle][scanLoc])
  {
    appSetAdvScanDataFrag(advHandle, scanLoc);
  }

  /* if all advertising/scan data have been sent */
  if ((appSlaveCb.advDataOffset[advHandle][advLoc] >= appSlaveCb.advDataLen[advHandle][advLoc]) &&
      (appSlaveCb.advDataOffset[advHandle][scanLoc] >= appSlaveCb.advDataLen[advHandle][scanLoc]))
  {
    appSlaveCb.advDataSynced[advHandle] = TRUE;
  }
}

/*************************************************************************************************/
/*!
 *  \fn     appConnUpdateTimerStart
 *        
 *  \brief  Start the connection update timer.
 *
 *  \param  connId    DM connection ID.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void appConnUpdateTimerStart(dmConnId_t connId)
{
  appSlaveCb.updateTimer.handlerId = appHandlerId;
  appSlaveCb.updateTimer.msg.event = APP_SLAVE_MSG_START;
  appSlaveCb.updateTimer.msg.param = connId;
  WsfTimerStartMs(&appSlaveCb.updateTimer, pAppUpdateCfg->idlePeriod);
}

/*************************************************************************************************/
/*!
 *  \fn     appConnUpdateTimerStop
 *        
 *  \brief  Stop the connection update timer.
 *
 *  \return None.
 */
/*************************************************************************************************/
void appConnUpdateTimerStop(void)
{
  /* stop connection update timer */
  if (pAppUpdateCfg->idlePeriod != 0)
  {
    WsfTimerStop(&appSlaveCb.updateTimer);
  }  
}

/*************************************************************************************************/
/*!
 *  \fn     appSlaveResolveAddr
 *
 *  \brief  Resolve the master's RPA.
 *
 *  \param  pMsg       Pointer to DM callback event message.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void appSlaveResolveAddr(dmEvt_t *pMsg)
{
  appDbHdl_t hdl;
  dmSecKey_t *pPeerKey;

  /* if address resolution's in progress */
  if (appSlaveCb.inProgress)
  {
    return;
  }

  /* get the first database record */
  hdl = AppDbGetNextRecord(APP_DB_HDL_NONE);

  /* if we have any bond records */
  if ((hdl != APP_DB_HDL_NONE) && ((pPeerKey = AppDbGetKey(hdl, DM_KEY_IRK, NULL)) != NULL))
  {
    /* reslove advertiser's RPA to see if we already have a bond with this device */
    DmPrivResolveAddr(pMsg->connOpen.peerAddr, pPeerKey->irk.key, pMsg->hdr.param);

    /* store database record handle for later */
    appSlaveCb.dbHdl = hdl;
    appSlaveCb.inProgress = TRUE;
    appSlaveCb.findLtk = FALSE;
  }
}

/*************************************************************************************************/
/*!
 *  \fn     appSecRspLtk
 *
 *  \brief  Respond to LTK request from peer.
 *
 *  \param  pCb     Connection control block.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void appSecRspLtk(appConnCb_t *pCb)
{
  dmSecKey_t  *pKey = NULL;
  uint8_t     secLevel;

  /* if there is a record */
  if (pCb->dbHdl != APP_DB_HDL_NONE)
  {
    /* get ltk */
    pKey = AppDbGetKey(pCb->dbHdl, DM_KEY_LOCAL_LTK, &secLevel);
  }

  if (pKey != NULL)
  {
    /* if not bonded we need to update bonding state when encrypted */
    pCb->bondByLtk = !pCb->bonded;

    /* we found the key */
    DmSecLtkRsp(pCb->connId, TRUE, secLevel, pKey->ltk.key);
  }
  else
  {
    pCb->bondByLtk = FALSE;

    /* key not found */
    DmSecLtkRsp(pCb->connId, FALSE, 0, NULL);
  }
}

/*************************************************************************************************/
/*!
 *  \fn     appSlaveConnOpen
 *
 *  \brief  Handle a DM_CONN_OPEN_IND event.
 *
 *  \param  pMsg    Pointer to DM callback event message.
 *  \param  pCb     Connection control block.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void appSlaveConnOpen(dmEvt_t *pMsg, appConnCb_t *pCb)
{
  /* process connection open event */
  appSlaveProcConnOpen(pMsg, pCb);

  if (appSlaveCb.advRestartCback != NULL)
  {
    /* restart advertising */
    (*appSlaveCb.advRestartCback)(pMsg);
  }
}

/*************************************************************************************************/
/*!
 *  \fn     appSlaveConnClose
 *
 *  \brief  Handle a DM_CONN_CLOSE_IND event.
 *
 *  \param  pMsg    Pointer to DM callback event message.
 *  \param  pCb     Connection control block.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void appSlaveConnClose(dmEvt_t *pMsg, appConnCb_t *pCb)
{
  uint8_t i;

  /* if actually advertising has timed out (for high duty cycle connectable directed advertising) */
  if (pMsg->hdr.status == HCI_ERR_ADV_TIMEOUT)
  {
    for (i = 0; i < DM_NUM_ADV_SETS; i++)
    {
      /* done advertising */
      appSlaveCb.advState[i] = APP_ADV_STOPPED;
    }
  }
  else
  {
    /* process connection close event */
    appSlaveProcConnClose(pMsg, pCb);

    if (appSlaveCb.advRestartCback != NULL)
    {
      /* restart advertising */
      (*appSlaveCb.advRestartCback)(pMsg);
    }
  }
}

/*************************************************************************************************/
/*!
 *  \fn     appSlaveProcConnOpen
 *        
 *  \brief  Process a DM_CONN_OPEN_IND event.
 *
 *  \param  pMsg    Pointer to DM callback event message.
 *  \param  pCb     Connection control block.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void appSlaveProcConnOpen(dmEvt_t *pMsg, appConnCb_t *pCb)
{ 
  /* store connection ID */
  pCb->connId = (dmConnId_t) pMsg->hdr.param;
  
  /* check if we should do connection parameter update */
  if ((pAppUpdateCfg->idlePeriod != 0) &&
      ((pMsg->connOpen.connInterval < pAppUpdateCfg->connIntervalMin) ||
       (pMsg->connOpen.connInterval > pAppUpdateCfg->connIntervalMax) ||
       (pMsg->connOpen.connLatency != pAppUpdateCfg->connLatency) ||
       (pMsg->connOpen.supTimeout != pAppUpdateCfg->supTimeout)))
  {
    pCb->connWasIdle = FALSE;
    pCb->attempts = 0;
    appConnUpdateTimerStart(pCb->connId);
  }
}

/*************************************************************************************************/
/*!
 *  \fn     appSlaveProcConnClose
 *        
 *  \brief  Process a DM_CONN_CLOSE_IND event.
 *
 *  \param  pMsg    Pointer to DM callback event message.
 *  \param  pCb     Connection control block.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void appSlaveProcConnClose(dmEvt_t *pMsg, appConnCb_t *pCb)
{
  uint8_t i;

  /* stop connection update timer */
  appConnUpdateTimerStop();
  
  /* clear connection ID */
  pCb->connId = DM_CONN_ID_NONE;

  /* cancel any address resolution in progress */
  appSlaveCb.inProgress = FALSE;

  /* if switching to connectable mode then set it up */
  if (pCb->setConnectable)
  {
    pCb->setConnectable = FALSE;
    appSlaveCb.discMode = APP_MODE_CONNECTABLE;
    
    /* for each advertising set */
    for (i = 0; i < DM_NUM_ADV_SETS; i++)
    {
      /* force update of advertising data */
      appSlaveForceAdvDataSync(i, APP_MODE_CONNECTABLE);
    }
  }
}

/*************************************************************************************************/
/*!
 *  \fn     appSlaveConnUpdate
 *        
 *  \brief  Handle a DM_CONN_UPDATE_IND event.
 *
 *  \param  pMsg    Pointer to DM callback event message.
 *  \param  pCb     Connection control block.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void appSlaveConnUpdate(dmEvt_t *pMsg, appConnCb_t *pCb)
{
  if (pAppUpdateCfg->idlePeriod != 0)
  {
    /* if successful */
    if (pMsg->hdr.status == HCI_SUCCESS)
    {
      /* stop connection update timer */
      appConnUpdateTimerStop();
    }
    /* else if update failed and still attempting to do update */
    else if (pCb->attempts < pAppUpdateCfg->maxAttempts)
    {
      /* start timer and try again */
      appConnUpdateTimerStart(pCb->connId);
    }
  }
}

/*************************************************************************************************/
/*!
 *  \fn     appSlaveResolvedAddrInd
 *
 *  \brief  Process a received privacy resolved address indication.
 *
 *  \param  pMsg    Pointer to DM message.
 *  \param  pCb     Connection control block.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void appSlaveResolvedAddrInd(dmEvt_t *pMsg, appConnCb_t *pCb)
{
  dmSecKey_t *pPeerKey;

  /* if address resolution is not in progress */
  if (!appSlaveCb.inProgress)
  {
    return;
  }

  /* if RPA resolved */
  if (pMsg->hdr.status == HCI_SUCCESS)
  {
    /* record found */
    pCb->dbHdl = appSlaveCb.dbHdl;

    /* if LTK was requested while resolving master's address */
    if (appSlaveCb.findLtk)
    {
      /* respond to LTK request */
      appSecRspLtk(pCb);

      appSlaveCb.findLtk = FALSE;
    }
  }
  /* if RPA did not resolve and there're more bonded records to go through */
  else if ((pMsg->hdr.status == HCI_ERR_AUTH_FAILURE) && (appSlaveCb.dbHdl != APP_DB_HDL_NONE))
  {
    /* get the next database record */
    appSlaveCb.dbHdl = AppDbGetNextRecord(appSlaveCb.dbHdl);

    /* if there's another bond record */
    if ((appSlaveCb.dbHdl != APP_DB_HDL_NONE) &&
        ((pPeerKey = AppDbGetKey(appSlaveCb.dbHdl, DM_KEY_IRK, NULL)) != NULL))
    {
      /* reslove RPA using the next stored IRK */
      DmPrivResolveAddr(DmConnPeerAddr(pCb->connId), pPeerKey->irk.key, pMsg->hdr.param);

      /* not done yet */
      return;
    }
  }

  /* done with this address resolution */
  appSlaveCb.inProgress = FALSE;
}

/*************************************************************************************************/
/*!
 *  \fn     appSlaveRemoteConnParamReq
 *
 *  \brief  Handle a DM_REM_CONN_PARAM_REQ_IND event.
 *
 *  \param  pMsg    Pointer to DM callback event message.
 *  \param  pCb     Connection control block.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void appSlaveRemoteConnParamReq(dmEvt_t *pMsg, appConnCb_t *pCb)
{
  /* if configured to accept the remote connection parameter request */
  if (pAppSlaveReqActCfg->remConnParamReqAct == APP_ACT_ACCEPT)
  {
    hciConnSpec_t connSpec;

    connSpec.connIntervalMin = pMsg->remConnParamReq.intervalMin;
    connSpec.connIntervalMax = pMsg->remConnParamReq.intervalMax;
    connSpec.connLatency = pMsg->remConnParamReq.latency;
    connSpec.supTimeout = pMsg->remConnParamReq.timeout;
    connSpec.minCeLen = connSpec.maxCeLen = 0;

    /* accept the remote device�s request to change connection parameters */
    DmRemoteConnParamReqReply(pCb->connId, &connSpec);
  }
  /* if configured to reject the remote connection parameter request */
  else if (pAppSlaveReqActCfg->remConnParamReqAct == APP_ACT_REJECT)
  {
    /* reject the remote device�s request to change connection parameters */
    DmRemoteConnParamReqNegReply(pCb->connId, HCI_ERR_UNSUP_FEAT);
  }
  /* else - app will handle the remote connection parameter request */
}

/*************************************************************************************************/
/*!
 *  \fn     appSlaveSecConnOpen
 *        
 *  \brief  Perform slave security procedures on connection open.
 *
 *  \param  pMsg    Pointer to DM callback event message.
 *  \param  pCb     Connection control block.
 *
 *  \return None.
 */
/*************************************************************************************************/
void appSlaveSecConnOpen(dmEvt_t *pMsg, appConnCb_t *pCb)
{
  /* initialize state variables */
  pCb->bonded = FALSE;
  pCb->bondByLtk = FALSE;
  pCb->bondByPairing = FALSE;
    
  /* find record for peer device */
  pCb->dbHdl = AppDbFindByAddr(pMsg->connOpen.addrType, pMsg->connOpen.peerAddr);

  /* if record not found and the peer device uses an RPA */
  if ((pCb->dbHdl == NULL) && DM_RAND_ADDR_RPA(pMsg->connOpen.peerAddr, pMsg->connOpen.addrType))
  {
    /* reslove master's RPA to see if we already have a bond with this device */
    appSlaveResolveAddr(pMsg);
  }

  /* send slave security request if configured to do so */
  if (pAppSecCfg->initiateSec && AppDbCheckBonded())
  {
    DmSecSlaveReq((dmConnId_t) pMsg->hdr.param, pAppSecCfg->auth);
  }
}

/*************************************************************************************************/
/*!
 *  \fn     appSecConnClose
 *        
 *  \brief  Perform security procedures on connection close.
 *
 *  \param  pMsg    Pointer to DM callback event message.
 *  \param  pCb     Connection control block.
 *
 *  \return None.
 */
/*************************************************************************************************/
void appSecConnClose(dmEvt_t *pMsg, appConnCb_t *pCb)
{
  /* if a device record was created check if it is valid */
  if (pCb->dbHdl != APP_DB_HDL_NONE)
  {
    AppDbCheckValidRecord(pCb->dbHdl);
  }
}

/*************************************************************************************************/
/*!
 *  \fn     appSecPairInd
 *        
 *  \brief  Handle pairing indication from peer.
 *
 *  \param  pMsg    Pointer to DM callback event message.
 *  \param  pCb     Connection control block.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void appSecPairInd(dmEvt_t *pMsg, appConnCb_t *pCb)
{
  uint8_t iKeyDist;
  uint8_t rKeyDist;
  
  /* if in bondable mode or if peer is not requesting bonding
   * or if already bonded with this device and link is encrypted
   */
  if (appSlaveCb.bondable ||
      ((pMsg->pairInd.auth & DM_AUTH_BOND_FLAG) != DM_AUTH_BOND_FLAG) ||
      (pCb->bonded && (DmConnSecLevel(pCb->connId) > DM_SEC_LEVEL_NONE)))
  {
    /* store bonding state:  if peer is requesting bonding and we want bonding */
    pCb->bondByPairing = (pMsg->pairInd.auth & pAppSecCfg->auth & DM_AUTH_BOND_FLAG) == DM_AUTH_BOND_FLAG;
    
    /* if bonding and no device record */
    if (pCb->bondByPairing && pCb->dbHdl == APP_DB_HDL_NONE)
    {
      /* create a device record if none exists */
      pCb->dbHdl = AppDbNewRecord(DmConnPeerAddrType(pCb->connId), DmConnPeerAddr(pCb->connId));
    }

    /* initialize stored keys */
    pCb->rcvdKeys = 0;

    /* initialize key distribution */
    rKeyDist = pAppSecCfg->rKeyDist;
    iKeyDist = pAppSecCfg->iKeyDist;
    
    /* if peer is using random address request IRK */
    if (DmConnPeerAddrType(pCb->connId) == DM_ADDR_RANDOM)
    {
      iKeyDist |= DM_KEY_DIST_IRK;
    }
    
    /* only distribute keys both sides have agreed to */
    rKeyDist &= pMsg->pairInd.rKeyDist;
    iKeyDist &= pMsg->pairInd.iKeyDist;
    
    /* accept pairing request */
    DmSecPairRsp(pCb->connId, pAppSecCfg->oob, pAppSecCfg->auth, iKeyDist, rKeyDist);
  }
  /* otherwise reject pairing request */
  else
  {
    DmSecCancelReq(pCb->connId, SMP_ERR_PAIRING_NOT_SUP);
  }
}

/*************************************************************************************************/
/*!
 *  \fn     appSecStoreKey
 *        
 *  \brief  Store security key.
 *
 *  \param  pMsg    Pointer to DM callback event message.
 *  \param  pCb     Connection control block.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void appSecStoreKey(dmEvt_t *pMsg, appConnCb_t *pCb)
{
  if (pCb->bondByPairing && pCb->dbHdl != APP_DB_HDL_NONE)
  {
    /* key was received */
    pCb->rcvdKeys |= pMsg->keyInd.type;
    
    /* store key in record */
    AppDbSetKey(pCb->dbHdl, &pMsg->keyInd);
  }
}

/*************************************************************************************************/
/*!
 *  \fn     appSecPairCmpl
 *        
 *  \brief  Handle pairing complete.
 *
 *  \param  pMsg    Pointer to DM callback event message.
 *  \param  pCb     Connection control block.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void appSecPairCmpl(dmEvt_t *pMsg, appConnCb_t *pCb)
{
  /* if bonding */
  if (pCb->bondByPairing)
  {
    /* set bonded state */
    pCb->bonded = TRUE;

    /* validate record and received keys */
    if (pCb->dbHdl != APP_DB_HDL_NONE)
    {
      AppDbValidateRecord(pCb->dbHdl, pCb->rcvdKeys);
    }    

    /* if bonded clear bondable mode */
    appSlaveCb.bondable = FALSE;
    
    /* if discoverable switch to connectable mode when connection closes */
    if (appSlaveCb.discMode == APP_MODE_DISCOVERABLE)
    {
      pCb->setConnectable = TRUE;
    }

    /*  if bonded and device is using static or public address add device to white list */

    /* if bonded, add device to resolving list */
    if (pCb->dbHdl != APP_DB_HDL_NONE)
    {
      AppAddDevToResList(pMsg, pCb->connId);
    }
  }
}

/*************************************************************************************************/
/*!
 *  \fn     appSecPairFailed
 *        
 *  \brief  Handle pairing failed
 *
 *  \param  pMsg    Pointer to DM callback event message.
 *  \param  pCb     Connection control block.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void appSecPairFailed(dmEvt_t *pMsg, appConnCb_t *pCb)
{
  return;
}

/*************************************************************************************************/
/*!
 *  \fn     appSecEncryptInd
 *        
 *  \brief  Handle encryption indication
 *
 *  \param  pMsg    Pointer to DM callback event message.
 *  \param  pCb     Connection control block.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void appSecEncryptInd(dmEvt_t *pMsg, appConnCb_t *pCb)
{
  /* check if bonding state should be set */
  if (pCb->bondByLtk && pMsg->encryptInd.usingLtk)
  {
    pCb->bonded = TRUE;
    pCb->bondByLtk = FALSE;
  }
}

/*************************************************************************************************/
/*!
 *  \fn     appSecFindLtk
 *        
 *  \brief  Handle LTK request from peer.
 *
 *  \param  pMsg    Pointer to DM callback event message.
 *  \param  pCb     Connection control block.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void appSecFindLtk(dmEvt_t *pMsg, appConnCb_t *pCb)
{
  /* if device record is not in place */
  if (pCb->dbHdl == APP_DB_HDL_NONE)
  {
    /* if ediv or rand is not zero */
    if ((pMsg->ltkReqInd.encDiversifier != 0) ||
        (memcmp(pMsg->ltkReqInd.randNum, calc128Zeros, SMP_RAND8_LEN) != 0))
    {
      /* find record */
      pCb->dbHdl = AppDbFindByLtkReq(pMsg->ltkReqInd.encDiversifier, pMsg->ltkReqInd.randNum);
      if (pCb->dbHdl != APP_DB_HDL_NONE)
      {
        /* record found, cancel any address resolution in progress */
        appSlaveCb.inProgress = FALSE;
      }
    }
    /* if address resolution in progress */
    else if (appSlaveCb.inProgress)
    {
      /* postpone LTK look up till address resolution is completed */
      appSlaveCb.findLtk = TRUE;
      return;
    }
  }

  /* respond to LTK request */
  appSecRspLtk(pCb);
}

/*************************************************************************************************/
/*!
*  \fn     appPrivSetAddrResEnableInd
*
*  \brief  Handle set address resolution enable indication.
*
*  \param  pMsg    Pointer to DM callback event message.
*
*  \return None.
*/
/*************************************************************************************************/
static void appPrivSetAddrResEnableInd(dmEvt_t *pMsg)
{
  if (pMsg->hdr.status == HCI_SUCCESS)
  {
    SvcCoreGapCentAddrResUpdate(DmLlPrivEnabled());
  }
}

/*************************************************************************************************/
/*!
*  \fn     appPrivAddDevToResListInd
*
*  \brief  Handle add device to resolving list indication.
*
*  \param  pMsg    Pointer to DM callback event message.
*  \param  pCb     Connection control block.
*
*  \return None.
*/
/*************************************************************************************************/
static void appPrivAddDevToResListInd(dmEvt_t *pMsg, appConnCb_t *pCb)
{
  if ((pMsg->hdr.status == HCI_SUCCESS) && (pCb->dbHdl != APP_DB_HDL_NONE))
  {
    dmSecKey_t *pPeerKey = AppDbGetKey(pCb->dbHdl, DM_KEY_IRK, NULL);
    if (pPeerKey != NULL)
    {
      /* set advertising peer address */
      AppSetAdvPeerAddr(pPeerKey->irk.addrType, pPeerKey->irk.bdAddr);
    }
  }
}

/*************************************************************************************************/
/*!
 *  \fn     appSlaveConnUpdateTimeout
 *        
 *  \brief  Handle a connection update timeout.
 *
 *  \param  pMsg    Pointer to DM callback event message.
 *  \param  pCb     Connection control block.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void appSlaveConnUpdateTimeout(wsfMsgHdr_t *pMsg, appConnCb_t *pCb)
{
  hciConnSpec_t connSpec;
  bool_t        idle;
  
  /* check if connection is idle */
  idle = (DmConnCheckIdle(pCb->connId) == 0);
  
  /* if connection is idle and was also idle on last check */
  if (idle && pCb->connWasIdle)
  {
    /* do update */
    pCb->attempts++;
    connSpec.connIntervalMin = pAppUpdateCfg->connIntervalMin;
    connSpec.connIntervalMax = pAppUpdateCfg->connIntervalMax;
    connSpec.connLatency = pAppUpdateCfg->connLatency;
    connSpec.supTimeout = pAppUpdateCfg->supTimeout;
    DmConnUpdate(pCb->connId, &connSpec);
  }
  else
  {
    pCb->connWasIdle = idle;
    appConnUpdateTimerStart(pCb->connId);
  }
}

/*************************************************************************************************/
/*!
 *  \fn     appSlaveProcMsg
 *        
 *  \brief  Process app framework messages for a slave.
 *
 *  \param  pMsg    Pointer to message.
 *
 *  \return None.
 */
/*************************************************************************************************/
void appSlaveProcMsg(wsfMsgHdr_t *pMsg)
{
  appConnCb_t *pCb;
  
  /* look up app connection control block from DM connection ID */
  pCb = &appConnCb[pMsg->param - 1];

  switch(pMsg->event)
  {
    case APP_CONN_UPDATE_TIMEOUT_IND:
      appSlaveConnUpdateTimeout(pMsg, pCb);
      break;

    default:
      break;
  }
}

/*************************************************************************************************/
/*!
 *  \fn     AppSlaveInit
 *        
 *  \brief  Initialize app framework slave.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppSlaveInit(void)
{
  uint8_t i;
   
  // for each advertising set
  for (i = 0; i < DM_NUM_ADV_SETS; i++)
  {
    /* initialize advertising state */
    appSlaveCb.advState[i] = APP_ADV_STOPPED;

    /* initialize advertising type */
    appSlaveCb.advType[i] = DM_ADV_CONN_UNDIRECT;

    /* initialize advertising type changed flag */
    appSlaveCb.advTypeChanged[i] = FALSE;
  }

  /* initialize discovery mode */
  appSlaveCb.discMode = APP_MODE_NONE;

  /* initialize advertising mode */
  appSlaveAdvModeInit();

  /* initialize address resolution */
  appSlaveCb.inProgress = FALSE;

  /* set up callback from main */
  appCb.slaveCback = appSlaveProcMsg;

  appSlaveCb.peerAddrType = HCI_ADDR_TYPE_PUBLIC;
  memset(appSlaveCb.peerAddr, 0, BDA_ADDR_LEN);
}

/*************************************************************************************************/
/*!
 *  \fn     AppSlaveProcDmMsg
 *
 *  \brief  Process connection-related DM messages for a slave.  This function should be called
 *          from the application's event handler.
 *
 *  \param  pMsg    Pointer to DM callback event message.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppSlaveProcDmMsg(dmEvt_t *pMsg)
{
  appConnCb_t *pCb = NULL;

  /* look up app connection control block from DM connection ID */
  if ((pMsg->hdr.event != DM_ADV_STOP_IND) &&
    (pMsg->hdr.event != DM_ADV_SET_STOP_IND))
  {
    pCb = &appConnCb[pMsg->hdr.param - 1];
  }

  switch (pMsg->hdr.event)
  {
    case DM_RESET_CMPL_IND:
      appSlaveAdvModeInit();
      break;

    case DM_ADV_SET_STOP_IND:
    case DM_ADV_STOP_IND:
      if (appSlaveCb.advStopCback != NULL)
      {
        (*appSlaveCb.advStopCback)(pMsg);
      }
      break;

    case DM_CONN_OPEN_IND:
      appSlaveConnOpen(pMsg, pCb);
      break;

    case DM_CONN_CLOSE_IND:
      appSlaveConnClose(pMsg, pCb);
      break;

    case DM_CONN_UPDATE_IND:
      appSlaveConnUpdate(pMsg, pCb);
      break;

    case DM_PRIV_RESOLVED_ADDR_IND:
      appSlaveResolvedAddrInd(pMsg, pCb);
      break;

    case DM_REM_CONN_PARAM_REQ_IND:
      appSlaveRemoteConnParamReq(pMsg, pCb);
      break;

    default:
      break;
  }
}

/*************************************************************************************************/
/*!
 *  \fn     appAdvSetData
 *
 *  \brief  Set advertising data for a given advertising set.
 *
 *  \param  advHandle Advertising handle.
 *  \param  location  Data location.
 *  \param  len       Length of the data.  Maximum length is 31 bytes.
 *  \param  pData     Pointer to the data.
 *  \param  bufLen    Length of the data buffer maintained by Application.  Minimum length is
 *                    31 bytes.
 *  \param  maxLen    Maximum advertising data length supported by Controller.
 *
 *  \return None.
 */
/*************************************************************************************************/
void appAdvSetData(uint8_t advHandle, uint8_t location, uint16_t len, uint8_t *pData, uint16_t bufLen,
                   uint16_t maxLen)
{
  /* store data for location */
  appSlaveCb.pAdvData[advHandle][location] = pData;
  appSlaveCb.advDataLen[advHandle][location] = len;

  /* set length of advertising data buffer maintained by Application */
  appSlaveCb.advDataBufLen[advHandle][location] = bufLen;

  /* set maximum advertising data length supported by Controller */
  appSlaveCb.maxAdvDataLen[advHandle] = maxLen;

  /* reset data offset */
  appSlaveCb.advDataOffset[advHandle][location] = 0;

  /* Set the data now if we are in the right mode and the data is complete (no fragmentation's required) */
  if ((appSlaveCb.advState[advHandle] != APP_ADV_STOPPED) &&
      (APP_LOC_2_MODE(location) == appSlaveCb.discMode)   &&
      (len <= HCI_EXT_ADV_DATA_LEN)                       &&
      (len <= maxLen))
  {
    appSetAdvScanDataFrag(advHandle, location);
  }
  /* Otherwise set it when advertising is started or mode changes */
  else
  {
    appSlaveCb.advDataSynced[advHandle] = FALSE;
  }
}

/*************************************************************************************************/
/*!
 *  \fn     appAdvStart
 *
 *  \brief  Utility function to start advertising.
 *
 *  \param  numSets       Number of advertising sets.
 *  \param  pAdvHandles   Advertising handles array.
 *  \param  pInterval     Advertising interval (in 0.625 ms units) array.
 *  \param  pDuration     Advertising duration (in ms) array.
 *  \param  pMaxEaEvents  Maximum number of extended advertising events array.
 *  \param  cfgAdvParam   Whether to configure advertising parameters
 *
 *  \return None.
 */
/*************************************************************************************************/
void appAdvStart(uint8_t numSets, uint8_t *pAdvHandles, uint16_t *pInterval, uint16_t *pDuration,
                 uint8_t *pMaxEaEvents, bool_t cfgAdvParam)
{
  uint8_t i;

  /* start advertising if not connectable advertising or multiple connections supported */
  if (!appSlaveConnectableAdv(numSets, pAdvHandles) || (appNumConns() < pAppSlaveCfg->connMax))
  {
    /* for each advertising set */
    for (i = 0; i < numSets; i++)
    {
      /* if advertising parameters to be configured */
      if (cfgAdvParam)
      {
        /* set min and max interval */
        DmAdvSetInterval(pAdvHandles[i], pInterval[i], pInterval[i]);

        /* set advertising parameters */
        DmAdvConfig(pAdvHandles[i], appSlaveCb.advType[pAdvHandles[i]], appSlaveCb.peerAddrType,
                    appSlaveCb.peerAddr);
      }

      /* if adv data to be synced */
      if (!appSlaveCb.advDataSynced[pAdvHandles[i]])
      {
        /* set advertising or scan response data */
        appSetAdvScanData(pAdvHandles[i], appSlaveCb.discMode);
      }
    }

    /* start advertising */
    DmAdvStart(numSets, pAdvHandles, pDuration, pMaxEaEvents);

    return;
  }

  /* done with all advertising states */
  for (i = 0; i < numSets; i++)
  {
    appSlaveCb.advState[pAdvHandles[i]] = APP_ADV_STOPPED;
  }
}

/*************************************************************************************************/
/*!
 *  \fn     appAdvStop
 *
 *  \brief  Stop advertising set(s).
 *
 *  \param  numSets     Number of advertising sets.
 *  \param  pAdvHandle  Advertising handles array.
 *
 *  \return None.
 */
/*************************************************************************************************/
void appAdvStop(uint8_t numSets, uint8_t *pAdvHandles)
{
  uint8_t i;

  for (i = 0; i < numSets; i++)
  {
    WSF_ASSERT(pAdvHandles[i] < DM_NUM_ADV_SETS);

    /* stop advertising */
    appSlaveCb.advState[pAdvHandles[i]] = APP_ADV_STOPPED;
  }

  appSlaveCb.discMode = APP_MODE_NONE;

  DmAdvStop(numSets, pAdvHandles);
}

/*************************************************************************************************/
/*!
 *  \fn     appSlaveAdvStart
 *
 *  \brief  Start advertising using the parameters for the given mode.
 *
 *  \param  numSets       Number of advertising sets.
 *  \param  pAdvHandles   Advertising handles array.
 *  \param  pInterval     Advertising interval (in 0.625 ms units) array.
 *  \param  pDuration     Advertising duration (in ms) array.
 *  \param  pMaxEaEvents  Maximum number of extended advertising events array.
 *  \param  cfgAdvParam   Whether to configure advertising parameters
 *  \param  mode          Discoverable/connectable mode.
 *
 *  \return None.
 */
/*************************************************************************************************/
void appSlaveAdvStart(uint8_t numSets, uint8_t *pAdvHandles, uint16_t *pInterval, uint16_t *pDuration,
                      uint8_t *pMaxEaEvents, bool_t cfgAdvParam, uint8_t mode)
{
  uint8_t i;
  uint8_t prevMode = appSlaveCb.discMode;

  /* handle auto init mode */
  if (mode == APP_MODE_AUTO_INIT)
  {
    if (AppDbCheckBonded() == FALSE)
    {
      AppSetBondable(TRUE);
      appSlaveCb.discMode = APP_MODE_DISCOVERABLE;
    }
    else
    {
      AppSetBondable(FALSE);
      appSlaveCb.discMode = APP_MODE_CONNECTABLE;

      /* init white list with bonded device addresses */
    }
  }
  else
  {
    appSlaveCb.discMode = mode;
  }

  /* if mode changed force update of advertising data */
  if (prevMode != appSlaveCb.discMode)
  {
    for (i = 0; i < numSets; i++)
    {
      /* force update of advertising data */
      appSlaveForceAdvDataSync(pAdvHandles[i], appSlaveCb.discMode);
    }
  }

  /* start advertising */
  appAdvStart(numSets, pAdvHandles, pInterval, pDuration, pMaxEaEvents, TRUE);
}

/*************************************************************************************************/
/*!
 *  \fn     appAdvSetAdValue
 *
 *  \brief  Set the value of an advertising data element in the advertising or scan
 *          response data.  If the element already exists in the data then it is replaced
 *          with the new value.  If the element does not exist in the data it is appended
 *          to it, space permitting.
 *
 *          There is special handling for the device name (AD type DM_ADV_TYPE_LOCAL_NAME).
 *          If the name can only fit in the data if it is shortened, the name is shortened
 *          and the AD type is changed to DM_ADV_TYPE_SHORT_NAME.
 *
 *  \param  advHandle Advertising handle.
 *  \param  location  Data location.
 *  \param  adType    Advertising data element type.
 *  \param  len       Length of the value.  Maximum length is 31 bytes.
 *  \param  pValue    Pointer to the value.
 *
 *  \return TRUE if the element was successfully added to the data, FALSE otherwise.
 */
/*************************************************************************************************/
bool_t appAdvSetAdValue(uint8_t advHandle, uint8_t location, uint8_t adType, uint8_t len,
                        uint8_t *pValue)
{
  uint8_t *pAdvData;
  uint16_t advDataLen;
  uint16_t advDataBufLen;
  bool_t  valueSet;

  WSF_ASSERT(location < APP_NUM_DATA_LOCATIONS);

  /* get pointer and length for location */
  pAdvData = appSlaveCb.pAdvData[advHandle][location];
  advDataLen = appSlaveCb.advDataLen[advHandle][location];
  advDataBufLen = appSlaveCb.advDataBufLen[advHandle][location];

  if (pAdvData != NULL)
  {
    /* set the new element value in the advertising data */
    if (adType == DM_ADV_TYPE_LOCAL_NAME)
    {
      valueSet = DmAdvSetName(len, pValue, &advDataLen, pAdvData, advDataBufLen);
    }
    else
    {
      valueSet = DmAdvSetAdValue(adType, len, pValue, &advDataLen, pAdvData, advDataBufLen);
    }

    if (valueSet)
    {
      /* if new value set update advertising data */
      appAdvSetData(advHandle, location, advDataLen, pAdvData, advDataBufLen,
                    appSlaveCb.maxAdvDataLen[advHandle]);

      return TRUE;
    }
  }

  return FALSE;
}

/*************************************************************************************************/
/*!
 *  \fn     appConnAccept
 *
 *  \brief  Accept a connection to a peer device with the given address using a given advertising set.
 *
 *  \param  advHandle    Advertising handle.
 *  \param  advType      Advertising type.
 *  \param  interval     Advertising interval (in 0.625 ms units).
 *  \param  duration     Advertising duration (in ms).
 *  \param  maxEaEvents  Maximum number of extended advertising events.
 *  \param  addrType     Address type.
 *  \param  pAddr        Peer device address.
 *
 *  \return None.
 */
/*************************************************************************************************/
void appConnAccept(uint8_t advHandle, uint8_t advType, uint16_t interval, uint16_t duration,
                   uint8_t maxEaEvents, uint8_t addrType, uint8_t *pAddr)
{
  /* if not already advertising and asked to enter direct connectable mode */
  if ((appSlaveCb.advState[advHandle] >= APP_ADV_STOPPED) &&
      ((advType == DM_ADV_CONN_DIRECT)                    ||
       (advType == DM_ADV_CONN_DIRECT_LO_DUTY)))
  {
    /* if this advertising state is being used or high duty cycle directed advertising */
    if ((interval > 0) || (advType == DM_ADV_CONN_DIRECT))
    {
      /* if LL Privacy has been enabled */
      if (DmLlPrivEnabled())
      {
        appDbHdl_t dbHdl = AppDbFindByAddr(addrType, pAddr);

        /* if LL address resolution is not supported on peer device */
        if ((dbHdl == APP_DB_HDL_NONE) || !AppDbGetPeerAddrRes(dbHdl))
        {
          /* refuse directed advertising */
          return;
        }
      }

      /* set advertising state */
      appSlaveCb.advState[advHandle] = APP_ADV_STATE1;

      /* if high duty cycle directed advertising */
      if (advType == DM_ADV_CONN_DIRECT)
      {
        /* for high duty cycle directed advertising (<= 3.75 ms advertising interval):
         * - min and max advertising intervals are not used
         * - duration must be less than or equal to 1.28 seconds and cannot be 0
         */
        if ((duration > HCI_ADV_DIRECTED_MAX_DURATION) || (duration == 0))
        {
          duration = HCI_ADV_DIRECTED_MAX_DURATION;
        }
      }
      else
      {
        /* set min and max advertising intervals */
        DmAdvSetInterval(advHandle, interval, interval);
      }

      /* accept connection */
      DmConnAccept(DM_CLIENT_ID_APP, advHandle, advType, duration, maxEaEvents, addrType, pAddr);
    }
  }
}

/*************************************************************************************************/
/*!
 *  \fn     appSetAdvType
 *
 *  \brief  Set advertising type for a given advertising set.
 *
 *  \param  advHandle    Advertising handle.
 *  \param  advType      Advertising type.
 *  \param  interval     Advertising interval (in 0.625 ms units).
 *  \param  duration     Advertising duration (in ms).
 *  \param  maxEaEvents  Maximum number of extended advertising events.
 *  \param  cfgAdvParam  Whether to configure advertising parameters
 *
 *  \return None.
 */
/*************************************************************************************************/
void appSetAdvType(uint8_t advHandle, uint8_t advType, uint16_t interval, uint16_t duration,
                   uint8_t maxEaEvents, bool_t cfgAdvParam)
{
  /* if new advertising type isn't allowed */
  if ((advType == DM_ADV_CONN_DIRECT) || (advType == DM_ADV_CONN_DIRECT_LO_DUTY))
  {
    return;
  }

  /* if advertising type is changed */
  if (appSlaveCb.advType[advHandle] != advType)
  {
    /* set new advertising type */
    appSlaveCb.advType[advHandle] = advType;

    /* if advertising has been started by the application */
    if (appSlaveCb.discMode != APP_MODE_NONE)
    {
      /* if not already advertising */
      if (appSlaveCb.advState[advHandle] == APP_ADV_STOPPED)
      {
        /* start advertising with new type */
        appSlaveCb.advState[advHandle] = APP_ADV_STATE1;

        appAdvStart(1, &advHandle, &interval, &duration, &maxEaEvents, cfgAdvParam);
      }
      /* else advertising with old type */
      else
      {
        /* set advertising type changed flag */
        appSlaveCb.advTypeChanged[advHandle] = TRUE;

        /* stop advertising before advertising with new type */
        appSlaveCb.advState[advHandle] = APP_ADV_STOPPED;

        DmAdvStop(1, &advHandle);
      }
    }
  }
}

/*************************************************************************************************/
/*!
 *  \fn     AppSlaveSecProcDmMsg
 *        
 *  \brief  Process security-related DM messages for a slave.  This function should be called
 *          from the application's event handler.
 *
 *  \param  pMsg    Pointer to DM callback event message.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppSlaveSecProcDmMsg(dmEvt_t *pMsg)
{
  appConnCb_t *pCb;
  
  /* look up app connection control block from DM connection ID */
  pCb = &appConnCb[pMsg->hdr.param - 1];

  switch(pMsg->hdr.event)
  {
    case DM_CONN_OPEN_IND:
      appSlaveSecConnOpen(pMsg, pCb);
      break;

    case DM_CONN_CLOSE_IND:
      appSecConnClose(pMsg, pCb);
      break;

    case DM_SEC_PAIR_CMPL_IND:
      appSecPairCmpl(pMsg, pCb);
      break;
      
    case DM_SEC_PAIR_FAIL_IND:
      appSecPairFailed(pMsg, pCb);
      break;

    case DM_SEC_ENCRYPT_IND:
      appSecEncryptInd(pMsg, pCb);
      break;
      
    case DM_SEC_ENCRYPT_FAIL_IND:
      break;

    case DM_SEC_KEY_IND:
      appSecStoreKey(pMsg, pCb);
      break;

    case DM_SEC_PAIR_IND:
      appSecPairInd(pMsg, pCb);
      break;
      
    case DM_SEC_LTK_REQ_IND:
      appSecFindLtk(pMsg, pCb);
      break;
      
    case DM_PRIV_SET_ADDR_RES_ENABLE_IND:
      appPrivSetAddrResEnableInd(pMsg);
      break;

    case DM_PRIV_ADD_DEV_TO_RES_LIST_IND:
      appPrivAddDevToResListInd(pMsg, pCb);
      break;

    default:
      break;
  }
}

/*************************************************************************************************/
/*!
 *  \fn     AppSetBondable
 *        
 *  \brief  Set bondable mode of device.
 *
 *  \param  bondable  TRUE to set device to bondable, FALSE to set to non-bondable.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppSetBondable(bool_t bondable)
{
  appSlaveCb.bondable = bondable;
}

/*************************************************************************************************/
/*!
 *  \fn     AppSetAdvPeerAddr
 *
 *  \brief  Set advertising peer address and its type.
 *
 *  \param  directAddrType  Peer address type.
 *  \param  pDirectAddr     Peer address.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppSetAdvPeerAddr(uint8_t peerAddrType, uint8_t *pPeerAddr)
{
  appSlaveCb.peerAddrType = peerAddrType;
  BdaCpy(appSlaveCb.peerAddr, pPeerAddr);
}

/*************************************************************************************************/
/*!
 *  \fn     AppSlaveSecurityReq
 *        
 *  \brief  Initiate a request for security as a slave device.  This function will send a 
 *          message to the master peer device requesting security.  The master device should
 *          either initiate encryption or pairing.
 *
 *  \param  connId    Connection identifier.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppSlaveSecurityReq(dmConnId_t connId)
{
  if (DmConnSecLevel(connId) == DM_SEC_LEVEL_NONE)
  {
    DmSecSlaveReq(connId, pAppSecCfg->auth);
  }
}

/*************************************************************************************************/
/*!
 *  \fn     AppSlaveIsAdvertising
 *
 *  \brief  Check if the local device's currently advertising.
 *
 *  \return TRUE if device's advertising. FALSE, otherwise.
 */
/*************************************************************************************************/
bool_t AppSlaveIsAdvertising(void)
{
  uint8_t i;

  // for each advertising set
  for (i = 0; i < DM_NUM_ADV_SETS; i++)
  {
    /* if advertising */
    if (appSlaveCb.advState[i] < APP_ADV_STOPPED)
    {
      return TRUE;
    }
  }

  return FALSE;
}