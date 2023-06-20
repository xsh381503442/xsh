/*************************************************************************************************/
/*!
 *  \file   app_main.c
 *
 *  \brief  Application framework main module.
 *
 *          $Date: 2016-08-22 17:32:42 -0700 (Mon, 22 Aug 2016) $
 *          $Revision: 8489 $
 *
 *  Copyright (c) 2011 Wicentric, Inc., all rights reserved.
 *  Wicentric confidential and proprietary.
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
#include "sec_api.h"
#include "wsf_trace.h"
#include "wsf_timer.h"
#include "wsf_assert.h"
#include "bstream.h"
#include "dm_api.h"
#include "app_api.h"
#include "app_main.h"
#include "app_ui.h"

/**************************************************************************************************
  Global Variables
**************************************************************************************************/

/*! Configuration pointer for advertising */
appAdvCfg_t *pAppAdvCfg;

/*! Configuration pointer for extended advertising */
appExtAdvCfg_t *pAppExtAdvCfg;

/*! Configuration pointer for slave */
appSlaveCfg_t *pAppSlaveCfg;

/*! Configuration pointer for master */
appMasterCfg_t *pAppMasterCfg;

/*! Configuration pointer for security */
appSecCfg_t *pAppSecCfg;

/*! Configuration pointer for connection parameter update */
appUpdateCfg_t *pAppUpdateCfg;

/*! Configuration pointer for discovery */
appDiscCfg_t *pAppDiscCfg;

/*! Configuration pointer for application */
appCfg_t *pAppCfg;

/*! Connection control block array */
appConnCb_t appConnCb[DM_CONN_MAX];

/*! WSF handler ID */
wsfHandlerId_t appHandlerId;

/*! Main control block */
appCb_t appCb;

/*! Configuration structure for incoming request actions */
const appReqActCfg_t appReqActCfg =
{
  APP_ACT_ACCEPT        /*! Action for the remote connection parameter request */
};

/*! Configuration pointer for incoming request actions on master */
appReqActCfg_t *pAppMasterReqActCfg = (appReqActCfg_t *) &appReqActCfg;

/*! Configurable pointer for incoming request actions on slave */
appReqActCfg_t *pAppSlaveReqActCfg = (appReqActCfg_t *) &appReqActCfg;

/*************************************************************************************************/
/*!
 *  \fn     appProcMsg
 *        
 *  \brief  Process messages from the event handler.
 *
 *  \param  pMsg    Pointer to message.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void appProcMsg(wsfMsgHdr_t *pMsg)
{
  switch(pMsg->event)
  {
    case APP_BTN_POLL_IND:
      appUiBtnPoll();
      break;

    case APP_UI_TIMER_IND:
      appUiTimerExpired(pMsg);
      break;
            
    default:
      break;
  }
}

/*************************************************************************************************/
/*!
 *  \fn     appCheckBonded
 *        
 *  \brief  Check the bonded state of a connection.
 *
 *  \param  connId      DM connection ID.
 *
 *  \return Bonded state.
 */
/*************************************************************************************************/
bool_t appCheckBonded(dmConnId_t connId)
{
  WSF_ASSERT((connId > 0) && (connId <= DM_CONN_MAX));
  
  return appConnCb[connId - 1].bonded;
}

/*************************************************************************************************/
/*!
 *  \fn     appCheckBondByLtk
 *        
 *  \brief  Check the bond-by-LTK state of a connection.
 *
 *  \param  connId      DM connection ID.
 *
 *  \return Bond-by-LTK state.
 */
/*************************************************************************************************/
bool_t appCheckBondByLtk(dmConnId_t connId)
{
  WSF_ASSERT((connId > 0) && (connId <= DM_CONN_MAX));
  
  return appConnCb[connId - 1].bondByLtk;
}

/*************************************************************************************************/
/*!
 *  \fn     appNumConns
 *        
 *  \brief  Return the number of existing connections.
 *
 *  \return Number of connections.
 */
/*************************************************************************************************/
uint8_t appNumConns(void)
{
  appConnCb_t   *pCcb = appConnCb;
  uint8_t       i, j;
  
  for (i = DM_CONN_MAX, j = 0; i > 0; i--, pCcb++)
  {
    if (pCcb->connId != DM_CONN_ID_NONE)
    {      
      j++;
    }
  }
  return j;
}

/*************************************************************************************************/
/*!
 *  \fn     AppHandlerInit
 *        
 *  \brief  App framework handler init function called during system initialization.
 *
 *  \param  handlerID  WSF handler ID for App.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppHandlerInit(wsfHandlerId_t handlerId)
{
  appHandlerId = handlerId;
  
  AppDbInit();
}

/*************************************************************************************************/
/*!
 *  \fn     AppHandler
 *        
 *  \brief  WSF event handler for app framework.
 *
 *  \param  event   WSF event mask.
 *  \param  pMsg    WSF message.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppHandler(wsfEventMask_t event, wsfMsgHdr_t *pMsg)
{
  if (pMsg != NULL)
  {
    APP_TRACE_INFO1("App got evt %d", pMsg->event);
    
    if (pMsg->event >= APP_MASTER_MSG_START)
    {
      /* pass event to master handler */
      (*appCb.masterCback)(pMsg);    
    }
    else if (pMsg->event >= APP_SLAVE_MSG_START)
    {
      /* pass event to slave handler */
      (*appCb.slaveCback)(pMsg);    
    }
    else
    {
      appProcMsg(pMsg);
    }
  }
  else
  {
    if (event & APP_BTN_DOWN_EVT)
    {
      AppUiBtnPressed();
    }
  }
}

/*************************************************************************************************/
/*!
 *  \fn     AppHandlePasskey
 *        
 *  \brief  Handle a passkey request during pairing.  If the passkey is to displayed, a
 *          random passkey is generated and displayed.  If the passkey is to be entered
 *          the user is prompted to enter the passkey.
 *
 *  \param  pAuthReq  DM authentication requested event structure.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppHandlePasskey(dmSecAuthReqIndEvt_t *pAuthReq)
{
  uint32_t passkey;
  uint8_t  buf[SMP_PIN_LEN];
  
  if (pAuthReq->display)
  {
    /* generate random passkey, limit to 6 digit max */
    SecRand((uint8_t *) &passkey, sizeof(uint32_t));
    passkey %= 1000000;
    
    /* convert to byte buffer */
    buf[0] = UINT32_TO_BYTE0(passkey);
    buf[1] = UINT32_TO_BYTE1(passkey);
    buf[2] = UINT32_TO_BYTE2(passkey);
    
    /* send authentication response to DM */
    DmSecAuthRsp((dmConnId_t) pAuthReq->hdr.param, SMP_PIN_LEN, buf);
    
    /* display passkey */
    AppUiDisplayPasskey(passkey);
  }
  else
  {
    /* prompt user to enter passkey */
    AppUiAction(APP_UI_PASSKEY_PROMPT);
  } 
}

/*************************************************************************************************/
/*!
*  \fn     AppHandleNumericComparison
*
*  \brief  Handle a numeric comparison indication during pairing.  The confirmation value is
*          displayed and the user is prompted to verify that the local and peer confirmation
*          values match.
*
*  \param  pCnfInd  DM confirmation indication event structure.
*
*  \return None.
*/
/*************************************************************************************************/
void AppHandleNumericComparison(dmSecCnfIndEvt_t *pCnfInd)
{
  uint32_t confirm = DmSecGetCompareValue(pCnfInd->confirm);

  /* display confirmation value */
  AppUiDisplayConfirmValue(confirm);

  /* TODO: Verify that local and peer confirmation values match */
  DmSecCompareRsp((dmConnId_t)pCnfInd->hdr.param, TRUE);
}

/*************************************************************************************************/
/*!
 *  \fn     AppConnClose
 *        
 *  \brief  Close the connection with the give connection identifier.
 *
 *  \param  connId    Connection identifier.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppConnClose(dmConnId_t connId)
{
  DmConnClose(DM_CLIENT_ID_APP, connId, HCI_ERR_REMOTE_TERMINATED);
}

/*************************************************************************************************/
/*!
 *  \fn     AppConnIsOpen
 *        
 *  \brief  Check if a connection is open.
 *
 *  \return Connection ID of open connection or DM_CONN_ID_NONE if no open connections.
 */
/*************************************************************************************************/
dmConnId_t AppConnIsOpen(void)
{
  appConnCb_t   *pCcb = appConnCb;
  uint8_t       i;
  
  for (i = DM_CONN_MAX; i > 0; i--, pCcb++)
  {
    if (pCcb->connId != DM_CONN_ID_NONE)
    {      
      return pCcb->connId;
    }
  }
  
  return DM_CONN_ID_NONE;
}

/*************************************************************************************************/
/*!
 *  \fn     AppDbGetHdl
 *        
 *  \brief  Get the device database record handle associated with an open connection.
 *
 *  \param  connId    Connection identifier.
 *
 *  \return Database record handle or APP_DB_HDL_NONE.
 */
/*************************************************************************************************/
appDbHdl_t AppDbGetHdl(dmConnId_t connId)
{
  return appConnCb[connId-1].dbHdl;
}

/*************************************************************************************************/
/*!
*  \fn     AppAddDevToResList
*
*  \brief  Add device to resolving list.
*
*  \param  pMsg    Pointer to DM callback event message.
*  \param  connId  Connection identifier.
*
*  \return None.
*/
/*************************************************************************************************/
void AppAddDevToResList(dmEvt_t *pMsg, dmConnId_t connId)
{
  dmSecKey_t *pPeerKey;
  appDbHdl_t hdl = appConnCb[connId - 1].dbHdl;

  /* if LL Privacy is supported and the peer device has distributed its IRK */
  if (HciLlPrivacySupported() && ((pPeerKey = AppDbGetKey(hdl, DM_KEY_IRK, NULL))!= NULL))
  {
    uint8_t *pPeerIrk;
    uint8_t *pLocalIrk;
    uint8_t zeroIrk[SMP_KEY_LEN] = { 0 };

    /* if the peer device uses RPA and non-zero IRK */
    if ((!BdaIsZeros(DmConnPeerRpa(connId)) ||
         DM_RAND_ADDR_RPA(DmConnPeerAddr(connId), DmConnPeerAddrType(connId))) &&
        (memcmp(pPeerKey->irk.key, zeroIrk, SMP_KEY_LEN) != 0))
    {
      /* use peer's IRK */
      pPeerIrk = pPeerKey->irk.key;
    }
    else
    {
      /* use all zeros for peer's IRK so the LL will use or accept the peer Identity Address */
      pPeerIrk = zeroIrk;
    }

    /* if the local device uses RPA and non-zero IRK */
    if ((!BdaIsZeros(DmConnLocalRpa(connId)) ||
         DM_RAND_ADDR_RPA(DmConnLocalAddr(connId), DmConnLocalAddrType(connId))) &&
        (memcmp(DmSecGetLocalIrk(), zeroIrk, SMP_KEY_LEN) != 0))
    {
      /* use local IRK */
      pLocalIrk = DmSecGetLocalIrk();
    }
    else
    {
      /* use all zeros for local IRK so the LL will use or accept the local Identity Address */
      pLocalIrk = zeroIrk;
    }

    /* if either of devices uses non-zero IRK */
    if ((pPeerIrk != zeroIrk) || (pLocalIrk != zeroIrk))
    {
      /* add device to resolving list */
      DmPrivAddDevToResList(pPeerKey->irk.addrType, pPeerKey->irk.bdAddr,
                            pPeerIrk, pLocalIrk, TRUE, pMsg->hdr.param);
    }
  }
}
