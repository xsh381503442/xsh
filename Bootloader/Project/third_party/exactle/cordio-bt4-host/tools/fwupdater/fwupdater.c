/*************************************************************************************************/
/*!
 *  \file   fwupdater.c
 *
 *  \brief  Firmware updater for TC2.
 *
 *          $Date: 2016-07-28 10:57:25 -0700 (Thu, 28 Jul 2016) $
 *          $Revision: 8071 $
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

#define _BSD_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "serial.h"

/***************************************************************************************************
** DEFINES
***************************************************************************************************/
#define CONFIG_HOST_REV 0x22

#if (CONFIG_HOST_REV >= 0x20)
#define FLASH_SST25WF080B
#else
#define FLASH_25AA1024
#endif

/*--------------------------------------------------------------------------------------------------
** DEFAULT_BAUD
**
** DESCRIPTION: Specifies default baud rate.  The bootloader may require a custom rate of 108000.
**------------------------------------------------------------------------------------------------*/
#define DEFAULT_BAUD  115200

/*--------------------------------------------------------------------------------------------------
** MAX_IMG_SIZE
**
** DESCRIPTION: The maximum size of an image.  The bootloader does not check for any maximum, either
**              for the hardware (i.e., the SPI flash size) or the application (i.e., the size of
**              the SPI flash devoted to the image).
**------------------------------------------------------------------------------------------------*/
#ifdef FLASH_SST25WF080B
#define MAX_IMG_SIZE     (1020u * 1024u) /* 1024 - 4KB for nvdata */
#endif

#ifdef FLASH_25AA1024
#define MAX_IMG_SIZE     (126u * 1024u) /* 128 - 2KB for nvdata */
#endif

/*--------------------------------------------------------------------------------------------------
** TIMEOUT_SECS, DECLARE_TIMEOUT(), RESET_TIMEOUT(), CHECK_TIMEOUT()
**
** DESCRIPTION: Convenience functions for timeout handling.
**------------------------------------------------------------------------------------------------*/
#define TIMEOUT_SECS 60
#define DECLARE_TIMEOUT()   time_t start = time(NULL);
#define RESET_TIMEOUT()   { start = time(NULL); }
#define CHECK_TIMEOUT()   { time_t now = time(NULL); \
                            if (difftime(now, start) > TIMEOUT_SECS) goto label_timeout; }

/***************************************************************************************************
** FUNCTION PROTOTYPES
***************************************************************************************************/

static uint32_t get_file_size(FILE *f);

/***************************************************************************************************
** FUNCTIONS
***************************************************************************************************/

/*------------------------------------------------------------------------------------------------*/

int main (int argc, char **argv)
{
    uint8_t  *data;
    char      buf[32];
    uint32_t  offset;
    uint32_t  size;
    FILE     *f_in;
    SERHANDLE serfd;
    int       cfg_rtn;
    int32_t   write_rtn;
    size_t    fread_rtn;
    int       baud = DEFAULT_BAUD;
    DECLARE_TIMEOUT();

    if ((argc != 3) && (argc != 4)) {
        fprintf(stderr, "\n");
        fprintf(stderr, "usage: %s infile outdev\n", argv[0]);
        fprintf(stderr, "       %s infile outdev baud\n\n", argv[0]);
        exit(1);
    }

    if (argc >= 4) {
        baud = atoi(argv[3]);
        if ((baud < 38400) || (baud > 115200)) {
            fprintf(stderr, "invalid baud rate %d\n", baud);
            exit(2);
        }
    }

    /* Open and read in file */
    f_in = fopen(argv[1], "rb");
    if (!f_in) {
        fprintf(stderr, "input file \"%s\" cannot be opened\n", argv[1]);
        exit(3);
    }

    size = get_file_size(f_in);
    if (size > MAX_IMG_SIZE) {
        fprintf(stderr, "image too large for SPI flash (%u > %u)\n", size, MAX_IMG_SIZE);
        exit(4);
    }
    data = (uint8_t *)malloc(size);
    if (data == NULL) {
        fprintf(stderr, "failed to allocate memory\n");
        exit(5);
    }
    fread_rtn = fread(data, 1, size, f_in);
    fclose(f_in);
    if (fread_rtn < (size_t)size) {
        fprintf(stderr, "failed to read file\n");
        exit(6);
    }

    /* Open serial port */
    cfg_rtn = open_serial_port(argv[2], &serfd);
    if (cfg_rtn) {
        exit(7);
    }

    /* Configure serial port */
    cfg_rtn = configure_serial_port(serfd, baud);
    if (cfg_rtn) {
        close_serial_port(serfd);
        fprintf(stderr, "failed to configure serial port\n");
        exit(9);
    }

    /* Send over data size */
    sprintf(buf, "%u\r", size);
    write_rtn = write_serial_port(serfd, (uint8_t *)buf, strlen(buf));
    if (write_rtn != (int32_t)strlen(buf)) {
        goto label_writeerr;
    }

    RESET_TIMEOUT();
    offset = 0;
    while(offset < size) {
        char *ptr = buf;
        int   blklen;

        /* Wait for 'R' of 'RD###: ' */
        memset(ptr, 0, sizeof(buf));
        for(;;) {
            int32_t len = read_serial_port(serfd, (uint8_t *)ptr, 1);
            if (len == 1) {
              if (*ptr == 'R') {
                  ptr++;
                  break;
              }
            }
            CHECK_TIMEOUT();
        }

        /* Wait for ' ' of 'RD###: ' */
        for(;;) {
            int32_t len = read_serial_port(serfd, (uint8_t *)ptr, 1);
            if (len == 1) {
              if (*ptr == ' ') {
                  ptr = 0;
                  break;
              }
              /* Sanity check. */
              ptr++;
              if (ptr > buf + sizeof(buf)) {
                  goto label_overrun;
              }
            }
            CHECK_TIMEOUT();
        }

        /* Send data. */
        blklen = atoi(buf+2);
        while (blklen > 0) {
            write_rtn = write_serial_port(serfd, data + offset, blklen);
            if (write_rtn != (int32_t)blklen) {
                goto label_writeerr;
            }

            offset += write_rtn;
            blklen -= write_rtn;
            printf("%u/%u\r", offset, size);
            fflush(stdout);
            RESET_TIMEOUT();
        }
    }
    goto label_end;

label_writeerr:
    printf("write error occurred while writing to device");
    goto label_error;

label_overrun:
    printf("overrun occurred while reading block length from device");
    goto label_error;

label_timeout:
    printf("timeout while trying to update firmware");
    goto label_error;

label_end:
    printf("\n\n");
    while (1) {
        uint8_t b;
        int32_t len;

        if ((len = read_serial_port(serfd, &b, 1)) != 1) {
            continue;
        }
        printf("%c", b);
        fflush(stdout);
    }

label_error:
    close_serial_port(serfd);
    return 0;
}

/*------------------------------------------------------------------------------------------------*/

static uint32_t get_file_size(FILE *f)
{
    long off;
    long size;

    off  = ftell(f);
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, off, SEEK_SET);
    return (uint32_t)size; /* xxx should handle error */
}
