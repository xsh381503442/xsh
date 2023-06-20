/*************************************************************************************************/
/*!
 *  \file   main_time.h
 *
 *  \brief  Tickless time function definitions.
 *
 *          $Date: 2016-07-22 14:16:10 -0700 (Fri, 22 Jul 2016) $
 *          $Revision: 7978 $
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

#ifndef MAIN_TIME_H
#define MAIN_TIME_H

/**************************************************************************************************
  Functions
**************************************************************************************************/

void mainTimeSleep(void);
void mainTimeUpdate(void);
void mainTimeInit(void);
void mainTimeAllowDeepSleep(bool_t allow);

#endif  /* MAIN_TIME_H */
