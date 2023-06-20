/*************************************************************************************************/
/*!
 *  \file   plxps_main.h
 *
 *  \brief  Pulse Oximeter profile sensor internal interfaces.
 *
 *          $Date: 2016-04-19 13:50:01 -0700 (Tue, 19 Apr 2016) $
 *          $Revision: 6868 $
 *
 *  Copyright (c) 2012 Wicentric, Inc., all rights reserved.
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
#ifndef PLXPS_MAIN_H
#define PLXPS_MAIN_H

#include "app_hw.h"

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************************
  Macros
**************************************************************************************************/

/*! Minimum RACP write length */
#define PLXPS_RACP_MIN_WRITE_LEN       2

/*! RACP response length */
#define PLXPS_RACP_RSP_LEN             4

/*! Pulse Oximeter RACP number of stored records response length */
#define PLXPS_RACP_NUM_REC_RSP_LEN     4

/*! RACP operand maximum length */
#define PLXPS_OPERAND_MAX              ((CH_RACP_GLS_FILTER_TIME_LEN * 2) + 1)

/**************************************************************************************************
  Data Types
**************************************************************************************************/

/*! Pulse Oximeter measurement record */
typedef struct
{
  plxpScm_t   spotCheck;              /*! Pulse Oximeter spot check measurement */
} plxpsRec_t;

/*! Measurement database interface */
void plxpsDbInit(void);
uint8_t plxpsDbGetNextRecord(uint8_t oper, plxpsRec_t *pCurrRec,  plxpsRec_t **pRec);
uint8_t plxpsDbDeleteRecords(uint8_t oper);
uint8_t plxpsDbGetNumRecords(uint8_t oper, uint8_t *pNumRec);
void plxpsDbGenerateRecord(void);


#ifdef __cplusplus
};
#endif

#endif /* PLXPS_MAIN_H */
