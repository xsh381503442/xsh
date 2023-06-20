/*************************************************************************************************/
/*!
 *  \file   wsf_trace.c
 *
 *  \brief  Trace message implementation.
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

#include <string.h>
#include <stdarg.h>

#include "platform.h"
#include "utils.h"

#include "wsf_types.h"
#include "wsf_trace.h"
#include "wsf_assert.h"
#include "wsf_cs.h"

/**************************************************************************************************
  Macros
**************************************************************************************************/

#ifndef WSF_RING_BUF_SIZE
/*! \brief      Size of token ring buffer (multiple of 2^N). */
#define WSF_RING_BUF_SIZE               32
#endif

/*! \brief      Ring buffer flow control condition detected. */
#define WSF_TOKEN_FLAG_FLOW_CTRL        (1 << 28)

/**************************************************************************************************
  Data types
**************************************************************************************************/

/*! \brief      Trace control block. */
struct
{
#if WSF_TOKEN_ENABLED
  struct
  {
    uint32_t token;             /*!< Token. */
    uint32_t param;             /*!< Parameter. */
  } ringBuf[WSF_RING_BUF_SIZE]; /*!< Tokenized tracing ring buffer. */

  volatile uint32_t prodIdx;    /*!< Ring buffer producer index. */
  volatile uint32_t consIdx;    /*!< Ring buffer consumer index. */

  WsfTokenHandler_t pendCback;  /*!< Pending event handler. */

  bool_t ringBufEmpty;          /*!< Ring buffer state. */
#endif

  bool_t enabled;               /*!< Tracing state. */
} wsfTraceCb;

#if WSF_TRACE_ENABLED == TRUE

/*************************************************************************************************/
/*!
 *  \fn     WsfTrace
 *
 *  \brief  Print a trace message.
 *
 *  \param  pStr      Message format string
 *  \param  ...       Additional aguments, printf-style
 *
 *  \return None.
 */
/*************************************************************************************************/
void WsfTrace(const char *pStr, ...)
{
  int32_t len;
  char    buf[WSF_PRINTF_MAX_LEN];
  va_list args;

  if (!wsfTraceCb.enabled)
  {
    return;
  }

  va_start(args, pStr);
  len = Util_VSNPrintf(buf, WSF_PRINTF_MAX_LEN, pStr, args);
  va_end(args);

  PlatformUartTx((uint8_t *)buf, len);
  PlatformUartTx((uint8_t *)"\r\n", 2);
}

#elif WSF_TOKEN_ENABLED == TRUE

/*************************************************************************************************/
/*!
 *  \fn     WsfToken
 *
 *  \brief  Output tokenized message.
 *
 *  \param  tok       Token
 *  \param  var       Variable
 *
 *  \return None.
 */
/*************************************************************************************************/
void WsfToken(uint32_t tok, uint32_t var)
{
  static uint32_t flags = 0;

  if (!wsfTraceCb.enabled)
  {
    return;
  }

  WSF_CS_INIT(cs);
  WSF_CS_ENTER(cs);

  uint32_t prodIdx = (wsfTraceCb.prodIdx + 1) & (WSF_RING_BUF_SIZE - 1);

  if (prodIdx != wsfTraceCb.consIdx)
  {
    wsfTraceCb.ringBuf[wsfTraceCb.prodIdx].token = tok | flags;
    wsfTraceCb.ringBuf[wsfTraceCb.prodIdx].param = var;
    wsfTraceCb.prodIdx = prodIdx;
    flags = 0;
  }
  else
  {
    flags = WSF_TOKEN_FLAG_FLOW_CTRL;
  }

  WSF_CS_EXIT(cs);

  if (wsfTraceCb.pendCback && wsfTraceCb.ringBufEmpty)
  {
    wsfTraceCb.ringBufEmpty = FALSE;
    wsfTraceCb.pendCback();
  }
}

/*************************************************************************************************/
/*!
 *  \fn     WsfTokenService
 *
 *  \brief  Service the trace ring buffer.
 *
 *  \return TRUE if trace messages pending, FALSE otherwise.
 *
 *  This routine is called in the main loop for a "push" type trace systems.
 */
/*************************************************************************************************/
bool_t WsfTokenService(void)
{
  static uint8_t outBuf[sizeof(wsfTraceCb.ringBuf[0])];
  static uint8_t outBufIdx = sizeof(wsfTraceCb.ringBuf[0]);

  if (outBufIdx < sizeof(wsfTraceCb.ringBuf[0]))
  {
    outBufIdx += WsfTokenIOWrite(outBuf + outBufIdx, sizeof(wsfTraceCb.ringBuf[0]) - outBufIdx);

    if (outBufIdx < sizeof(wsfTraceCb.ringBuf[0]))
    {
      /* I/O device is flow controlled. */
      return TRUE;
    }
  }

  if (wsfTraceCb.consIdx != wsfTraceCb.prodIdx)
  {
    memcpy(&outBuf, &wsfTraceCb.ringBuf[wsfTraceCb.consIdx], sizeof(wsfTraceCb.ringBuf[0]));
    outBufIdx = 0;

    wsfTraceCb.consIdx = (wsfTraceCb.consIdx + 1) & (WSF_RING_BUF_SIZE - 1);

    return TRUE;
  }

  return FALSE;
}

/*************************************************************************************************/
/*!
 *  \brief  Write token data.
 *
 *  \param  pBuf    Buffer to write.
 *  \param  len     Length of buffer.
 *
 *  \return Number of actual bytes written.
 */
/*************************************************************************************************/
uint8_t WsfTokenIOWrite(uint8_t *pBuf, uint8_t len)
{
  PlatformUartTx(pBuf, len);
  return len;
}

#endif

/*************************************************************************************************/
/*!
 *  \fn     WsfTraceEnable
 *
 *  \brief  Enable trace messages.
 *
 *  \param  enable    TRUE to enable, FALSE to disable
 *
 *  \return None.
 */
/*************************************************************************************************/
void WsfTraceEnable(bool_t enable)
{
  wsfTraceCb.enabled = enable;
}
