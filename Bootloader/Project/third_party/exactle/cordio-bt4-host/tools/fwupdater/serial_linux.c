/*************************************************************************************************/
/*!
 *  \file   serial_linux.c
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
    int fd;

#ifdef __APPLE__
    fd = open(s, O_RDWR | O_NOCTTY | O_NONBLOCK);
#else
    fd = open(s, O_RDWR);
#endif
    if (fd < 0) {
        fprintf(stderr, "cannot open port \"%s\"\n", s);
        return -1;
    }
#ifdef __APPLE__
    if (ioctl(fd, TIOCEXCL)) {
        fprintf(stderr, "cannot gain exclusive access to port \"%s\"\n", s);
        return -1;
    }
/*
    if (fcntl(fd, F_SETFL, 0)) {
        fprintf(stderr, "cannot clean non-blocking access to port \"%s\"\n", s);
        return -1;
    }
*/
#endif
    if (!isatty(fd)) {
        close(fd);
        fprintf(stderr, "device \"%s\" is not a terminal device\n", s);
        return -1;
    }

    *serfd = fd;
    return 0;
}

/*------------------------------------------------------------------------------------------------*/

void close_serial_port(SERHANDLE serfd)
{
    close(serfd);
}

/*------------------------------------------------------------------------------------------------*/

int configure_serial_port(SERHANDLE serfd, int baud)
{
    struct termios cfg;
    int            r;
    int            flag = 0; /* Default for nonstandard baud. */

    /* Baud assumed clipped to [38400,115200]. */
    switch (baud) {
        case 38400:  flag = B38400;  break;
        case 57600:  flag = B57600;  break;
        case 115200: flag = B115200; break;
        default:                     break;
    }
    /* Configure serial port */
    if (tcgetattr(serfd, &cfg)) {
    	perror("Failed to get port attributes");
    	return 1;
    }
    cfmakeraw(&cfg);
    if(cfsetspeed(&cfg, (flag == 0) ? B38400 : flag)) {
    	perror("Failed to set port speed");
    	return 1;
    }
    cfg.c_cflag |= CS8;  /* 8 data bits */
    cfg.c_cflag &= ~CSTOPB; /* 1 stop bit */
    cfg.c_cflag &= ~PARENB; /* No parity */
    cfg.c_cc[VMIN] = 1;   /* 1 byte minimum */
    cfg.c_cc[VTIME] = 0;  /* No timeout */
    cfg.c_iflag &= ~CRTSCTS; /* No HWFLOW */
    cfg.c_iflag &= ~(IXON|IXOFF|IXANY); /* No SWFLOW */
    cfg.c_iflag &= ~ECHO; /* Disable echo */
    cfg.c_cflag |= CLOCAL; /* No modem lines */
    cfg.c_cflag &= ~HUPCL; /* No modem lines */
    cfg.c_cflag |= CREAD; /* Enable Receiver */
    cfg.c_iflag |= PARMRK;
    cfg.c_iflag &= ~(IGNPAR | BRKINT | IGNBRK); /* Trap on parity/brk/etc */
    if (tcsetattr(serfd, TCSANOW, &cfg)) {
    	perror("Failed to set port attributes");
    	return 1;
    }

    /* For non-standard baud, set up port. */
    if (flag == 0) {
#ifdef __APPLE__
    	printf("Configuring nonstandard baud rate.\r\n");
    	const int serbaud = baud;
    	if (ioctl(serfd, IOSSIOSPEED, &baud)) {
    		perror("Can't set baud rate");
    		return 1;
    	}
#else
        struct serial_struct serinfo;

        if (ioctl(serfd, TIOCGSERIAL, &serinfo) < 0) {
            perror("Can't get serial info:");
            return 1;
        }
        serinfo.flags &= ~ASYNC_SPD_MASK;
        serinfo.flags |= ASYNC_SPD_CUST;
        serinfo.custom_divisor = serinfo.baud_base / baud;
        if (ioctl(serfd, TIOCSSERIAL, &serinfo) < 0) {
            perror("Can't get serial info:");
            return 1;
        }
#endif
    }

    return 0;
}

/*------------------------------------------------------------------------------------------------*/

int32_t write_serial_port(SERHANDLE serfd, const uint8_t *data, uint32_t len)
{
    return write(serfd, data, len);
}

/*------------------------------------------------------------------------------------------------*/

int32_t read_serial_port(SERHANDLE serfd, uint8_t *data, uint32_t len)
{
    return read(serfd, data, len);
}
