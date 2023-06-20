/*************************************************************************************************/
/*!
 *  \file       app_terminal.h
 *
 *  \brief      Terminal handler.
 *
 *          $Date: 2016-03-29 14:55:12 -0700 (Tue, 29 Mar 2016) $
 *          $Revision: 6524 $
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

#ifndef APP_TERMINAL_H
#define APP_TERMINAL_H

#include <stdarg.h>

#include "wsf_types.h"
#include "wsf_os.h"

/**************************************************************************************************
  Macros
**************************************************************************************************/

#define APP_TERMINAL_MAX_ARGC           8u          /*!< Maximum number of arguments to any command. */
#define APP_TERMINAL_MAX_COMMAND_LEN    100u        /*!< Maximum length of command line. */
#define APP_TERMINAL_PRINTF_MAX_LEN     128u        /*!< Maximum length of any printed output. */
#define APP_TERMINAL_STRING_PROMPT      "> "        /*!< Prompt string. */
#define APP_TERMINAL_STRING_ERROR       "ERROR: "   /*!< Error prefix. */
#define APP_TERMINAL_STRING_USAGE       "USAGE: "   /*!< Usage prefix. */
#define APP_TERMINAL_STRING_NEW_LINE    "\r\n"      /*!< New line string. */

/*! \brief    Terminal command error codes. */
enum
{
  APP_TERMINAL_ERROR_OK                 = 0,  /*!< Command completed. */
  APP_TERMINAL_ERROR_BAD_ARGUMENTS      = 1,  /*!< ERROR: Invalid argument(s) */
  APP_TERMINAL_ERROR_TOO_FEW_ARGUMENTS  = 2,  /*!< ERROR: Too few arguments */
  APP_TERMINAL_ERROR_TOO_MANY_ARGUMENTS = 3,  /*!< ERROR: Too many arguments */
  APP_TERMINAL_ERROR_EXEC               = 4   /*!< Command completed with execution error. */
};

/**************************************************************************************************
  Data Types
**************************************************************************************************/

/*************************************************************************************************/
/*!
 *  \brief  Handler for a terminal command.
 *
 *  \param  argc      The number of arguments passed to the command.
 *  \param  argv      The array of arguments; the 0th argument is the command.
 *
 *  \return Error code.
 */
/*************************************************************************************************/
typedef uint8_t (*appTerminalHandler_t)(uint32_t argc, char **argv);

typedef void appTerminalUartTx_t(const uint8_t *pBuf, uint32_t len);

/*************************************************************************************************/
/*!
 *  \brief  Terminal command.
 */
/*************************************************************************************************/
typedef struct appTerminalCommand_tag
{
  struct appTerminalCommand_tag   *pNext;     /*!< Pointer to next command in list. */
  const char                      *pName;     /*!< Name of command. */
  const char                      *pHelpStr;  /*!< Help String for command. */
  appTerminalHandler_t            handler;    /*!< Handler for command. */
} appTerminalCommand_t;

/**************************************************************************************************
  Function Prototypes
**************************************************************************************************/

/*************************************************************************************************/
/*!
 *  \fn     AppTerminalInit
 *
 *  \brief  Initialize terminal.
 *
 *  \param  handlerId   Handler ID for AppTerminalHandler().
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppTerminalInit(wsfHandlerId_t handlerId);

/*************************************************************************************************/
/*!
 *  \fn     AppTerminalRegisterUartTxFunc
 *
 *  \brief  Register the UART Tx Function for the platform.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppTerminalRegisterUartTxFunc(appTerminalUartTx_t uartTxFunc);

/*************************************************************************************************/
/*!
 *  \fn     AppTerminalRegisterCommand
 *
 *  \brief  Register command with terminal.
 *
 *  \param  pCommand    Command.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppTerminalRegisterCommand(appTerminalCommand_t *pCommand);

/*************************************************************************************************/
/*!
 *  \fn     AppTerminalHandler
 *
 *  \brief  Handler for terminal messages.
 *
 *  \param  event       WSF event mask.
 *  \param  pMsg        WSF message.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppTerminalHandler(wsfEventMask_t event, wsfMsgHdr_t *pMsg);

/*************************************************************************************************/
/*!
 *  \fn     AppTerminalRx
 *
 *  \brief  Called by application when a data byte is received.
 *
 *  \param  dataByte    received byte
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppTerminalRx(uint8_t dataByte);

/*************************************************************************************************/
/*!
 *  \fn     AppTerminalTxStr
 *
 *  \brief  Called by application to transmit string.
 *
 *  \param  pStr      String.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppTerminalTxStr(const char *pStr);

/*************************************************************************************************/
/*!
 *  \fn     AppTerminalTxChar
 *
 *  \brief  Called by application to transmit character.
 *
 *  \param  c         Character.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppTerminalTxChar(char c);

/*************************************************************************************************/
/*!
 *  \fn     AppTerminalTxPrint
 *
 *  \brief  Called by application to print formatted data.
 *
 *  \param  pStr      Message format string
 *  \param  ...       Additional arguments, printf-style
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppTerminalTxPrint(const char *pStr, ...);

/*************************************************************************************************/
/*!
 *  \fn     AppTerminalTx
 *
 *  \brief  Application function to transmit data..
 *
 *  \param  pData     Data.
 *  \param  len       Length of data, in bytes.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppTerminalTx(const uint8_t *pData, uint16_t len);

#endif /* APP_TERMINAL_H */
