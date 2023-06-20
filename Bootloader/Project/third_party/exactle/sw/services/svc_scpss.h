/*************************************************************************************************/
/*!
 *  \file   svc_scpss.h
 *        
 *  \brief  Example Scan Parameter Service Server implementation.
 *
 *          $Date: 2014-02-06 11:36:43 -0800 (Thu, 06 Feb 2014) $
 *          $Revision: 1113 $
 *  
 *  Copyright (c) 2016 ARM, Inc., all rights reserved.
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

#ifndef SVC_SCPSS_H
#define SVC_SCPSS_H

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************************
 Handle Ranges
**************************************************************************************************/

/* Scan Parameter Service */
#define SCPSS_START_HDL               0x0300
#define SCPSS_END_HDL                 (SCPSS_MAX_HDL - 1)

/**************************************************************************************************
 Handles
**************************************************************************************************/

/* Scan Parameter Service Handles */
enum
{
  SCPSS_SVC_HDL = SCPSS_START_HDL,    /* Scan Parameter Server Service declaration */
  SCPSS_SIW_CH_HDL,                   /* Scan Interval Window characteristic */ 
  SCPSS_SIW_HDL,                      /* Scan Interval Window */
  SCPSS_MAX_HDL
};

/**************************************************************************************************
  Function Declarations
**************************************************************************************************/

void SvcScpssAddGroup(void);
void SvcScpssRemoveGroup(void);
void SvcScpssCbackRegister(attsWriteCback_t writeCback);

#ifdef __cplusplus
};
#endif

#endif /* SVC_SCPSS_H */
