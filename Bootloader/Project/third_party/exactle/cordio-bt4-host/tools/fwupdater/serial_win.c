/*************************************************************************************************/
/*!
 *  \file   serial_win.c
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

/***************************************************************************************************
** INCLUDES
***************************************************************************************************/

#include <stdio.h>
#include "serial.h"

/***************************************************************************************************
** API
***************************************************************************************************/

/*------------------------------------------------------------------------------------------------*/

int open_serial_port(const char *s, SERHANDLE *serfd)
{
    HANDLE   hComm;
    uint32_t len;
    char    *s_buf;

    /* To handle ports above 9. */
    len = strlen(s);
    s_buf = (char *)malloc(len + 5);
    strcpy(s_buf, "\\\\.\\");
    strcat(s_buf, s);

    hComm = CreateFile(s_buf,
                       GENERIC_READ | GENERIC_WRITE,
                       0,
                       NULL,
                       OPEN_EXISTING,
                       0,
                       NULL);
    free(s_buf);
    if (hComm == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "cannot open port \"%s\"\n", s);
        return -1;
    }

    *serfd = hComm;
    return 0;
}

/*------------------------------------------------------------------------------------------------*/

void close_serial_port(SERHANDLE serfd)
{
    CloseHandle(serfd);
}

/*------------------------------------------------------------------------------------------------*/

int configure_serial_port(SERHANDLE serfd, int baud)
{
    DCB dcb;

    FillMemory(&dcb, sizeof(dcb), 0);
    dcb.DCBlength = sizeof(dcb);
    if (!BuildCommDCB("115200,n,8,1", &dcb)) {
        fprintf(stderr, "cannot build configuration\n");
        return -1;
    }
    dcb.BaudRate = baud;
    if (!SetCommState(serfd, &dcb)) {
        fprintf(stderr, "cannot configure port\n");
        return -1;
    }
    return 0;
}

/*------------------------------------------------------------------------------------------------*/

int32_t write_serial_port(SERHANDLE serfd, const uint8_t *data, uint32_t len)
{
    BOOL  ok;
    DWORD wrlen;

    ok = WriteFile(
        serfd,
        data,
        len,
        &wrlen,
        NULL);
    if (!ok) {
        fprintf(stderr, "failed to write file\n");
        return 0;
    }
    if (wrlen != len) {
        fprintf(stderr, "wrote only %u of %u bytes\n", wrlen, len);
    }
    return wrlen;
}

/*------------------------------------------------------------------------------------------------*/

int32_t read_serial_port(SERHANDLE serfd, uint8_t *data, uint32_t len)
{
    BOOL  ok;
    DWORD rdlen;

    ok = ReadFile(
        serfd,
        data,
        len,
        &rdlen,
        NULL);
    if (!ok) {
        fprintf(stderr, "failed to read file\n");
        return 0;
    }
    if (rdlen != len) {
        fprintf(stderr, "read only %u of %u bytes\n", rdlen, len);
    }
    return rdlen;
}
