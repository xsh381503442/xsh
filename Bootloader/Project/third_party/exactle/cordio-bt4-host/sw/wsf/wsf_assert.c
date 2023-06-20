/*************************************************************************************************/
/*!
 *  \file   wsf_assert.c
 *
 *  \brief  Assert implementation.
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

#include "wsf_types.h"
#include "wsf_assert.h"
#include "wsf_os.h"

#include "platform.h"

/*************************************************************************************************/
/*!
 *  \brief      Print integer
 *
 *  \param      val     Value.
 *
 *  \return     None.
 */
/*************************************************************************************************/
static void printInt(uint32_t val)
{
  char buf[15];
  uint32_t i = sizeof(buf);

  do
  {
    i--;
    uint8_t n = val % 10;
    buf[i] = '0' + n;
    val /= 10;
  } while (val > 0);

  PlatformUartTx((uint8_t *)buf + i, sizeof(buf) - i);
}

/*************************************************************************************************/
/*!
 *  \def    WsfAssert
 *
 *  \brief  Perform an assert action.
 *
 *  \param  pFile   Name of file originating assert.
 *  \param  line    Line number of assert statement.
 */
/*************************************************************************************************/
#if WSF_TOKEN_ENABLED == TRUE
void WsfAssert(uint16_t modId, uint16_t line)
#else
void WsfAssert(const char *pFile, uint16_t line)
#endif
{
  volatile uint8_t escape=0;

  PlatformUartTx((uint8_t *)"\r\n*** Assert:\r\n", 15);
#if WSF_TOKEN_ENABLED == TRUE
  PlatformUartTx((uint8_t *)"  Module ID \"", 8);
  printInt(modId);
  PlatformUartTx((uint8_t *)"\r\n",      2);
#else
  PlatformUartTx((uint8_t *)"  File \"", 8);
  PlatformUartTx((uint8_t *)pFile,       strlen(pFile));
  PlatformUartTx((uint8_t *)"\"\r\n",    3);
#endif
  PlatformUartTx((uint8_t *)"  Line ",   7);
  printInt(line);
  PlatformUartTx((uint8_t *)"\r\n",      2);

  /* spin forever if fatal error occurred */
  for(;;)
  {
    /*
     *  However, you can exit with a debugger by setting variable 'escape'.
     *  Handy to see where the assert happened if you cannot view the call stack.
    */
    if (escape)
    {
      break;
    }
  }
}
