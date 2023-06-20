/*************************************************************************************************/
/*!
 *  \file   serial.h
 *
 *  \brief  Serial I/O functions.
 *
 *          $Date: 2015-10-25 06:16:48 -0700 (Sun, 25 Oct 2015) $
 *          $Revision: 4293 $
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

#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>

/***************************************************************************************************
** ADAPTATIONS
***************************************************************************************************/

/*--------------------------------------------------------------------------------------------------
** WINDOWS
**------------------------------------------------------------------------------------------------*/
#ifdef WIN32
#include <Windows.h>
#define SERHANDLE HANDLE

#else
/*--------------------------------------------------------------------------------------------------
** LINUX
**------------------------------------------------------------------------------------------------*/
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <termios.h>
#ifdef __APPLE__
#include <IOKit/serial/ioss.h>
#else
#include <linux/tty.h>
#include <linux/serial.h>
#endif
#define SERHANDLE int
#endif

/***************************************************************************************************
** API
***************************************************************************************************/

int      open_serial_port     (const char* s, SERHANDLE *serfd);
void     close_serial_port    (SERHANDLE serfd);
int      configure_serial_port(SERHANDLE serfd, int baud);
int32_t  write_serial_port    (SERHANDLE serfd, const uint8_t *data, uint32_t len);
int32_t  read_serial_port     (SERHANDLE serfd, uint8_t *data, uint32_t len);

#endif /* SERIAL_H */
