/*************************************************************************************************/
/*!
 *  \file       app_terminal.c
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

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "app_terminal.h"
#include "app_ui.h"
#include "utils.h"

#include "wsf_types.h"
#include "wsf_assert.h"
#include "wsf_trace.h"
#include "bstream.h"

#include "dm_api.h"

/**************************************************************************************************
  Macros
**************************************************************************************************/

#define APP_TERMINAL_IS_SPACE(c)  ((c == '\n') || (c == '\t') || (c == '\r') || (c == ' ') || (c == '\v') || (c == '\f'))
#define APP_TERMINAL_IS_PRINT(c)  ((c >= 0x20) && (c != 0x7F))

/*! \brief  Terminal events. */
enum
{
  APP_TERMINAL_EVENT_COMMAND_RX = (1 << 0)
};

/**************************************************************************************************
  Data Types
**************************************************************************************************/

/*! \brief   Control block for terminal. */
typedef struct
{
  wsfHandlerId_t        handlerId;                              /*!< Handler ID for TerminalHandler(). */
  appTerminalCommand_t  *pFirstCommand;                         /*!< Pointer to first command. */
  char                  buf[APP_TERMINAL_MAX_COMMAND_LEN + 1];  /*!< Command buffer. */
  bool_t                isExecuting;                            /*!< TRUE if command in buffer is executing. */
  bool_t                doEcho;                                 /*!< TRUE if input should be echoed. */
  uint32_t              bufOffset;                              /*!< Offset within buffer. */
  appTerminalUartTx_t   *terminalTx;                            /*!< Function to transmit via UART. */
} appTerminalCtrlBlk_t;

/**************************************************************************************************
  Local Function Prototypes
**************************************************************************************************/

/*! \brief   Help Command Handler */
static uint8_t appTerminalCommandHelpHandler(uint32_t argc, char **argv);

/*! \brief   Echo Command Handler */
static uint8_t appTerminalCommandEchoHandler(uint32_t argc, char **argv);

/*! \brief   Button Command Handler */
static uint8_t appTerminalCommandBtnHandler(uint32_t argc, char **argv);

/*! \brief    Security Pin Code Command Handler. */
static uint8_t appTerminalPinCodeHandler(uint32_t argc, char **argv);

/**************************************************************************************************
  Local Variables
**************************************************************************************************/

/*! \brief   Control block for terminal. */
static appTerminalCtrlBlk_t appTerminalCb;

/*! \brief    Help command. */
static appTerminalCommand_t appTerminalCommandHelp = { NULL, "help", "help", appTerminalCommandHelpHandler };

/*! \brief    Echo command. */
static appTerminalCommand_t appTerminalCommandEcho = { NULL, "echo", "echo <on|off>", appTerminalCommandEchoHandler };

/*! \brief    Button command. */
static appTerminalCommand_t appTerminalButtonPress = { NULL, "btn", "btn <ID> <s|m|l|x>", appTerminalCommandBtnHandler };

/*! \brief    Security Pin Code commands. */
static appTerminalCommand_t appTerminalPinCode = { NULL, "pin", "pin <ConnID> <Pin Code>", appTerminalPinCodeHandler };

/*************************************************************************************************/
/*!
 *  \fn     AppTerminalInit
 *
 *  \brief  Initialize terminal.
 *
 *  \param  handlerId   Handler ID for TerminalHandler().
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppTerminalInit(wsfHandlerId_t handlerId)
{
  APP_TRACE_INFO0("terminal: init");

  appTerminalCb.handlerId     = handlerId;
  appTerminalCb.pFirstCommand = NULL;
  appTerminalCb.isExecuting   = FALSE;
  appTerminalCb.doEcho        = TRUE;
  appTerminalCb.bufOffset     = 0;

  AppTerminalRegisterCommand(&appTerminalCommandHelp);
  AppTerminalRegisterCommand(&appTerminalCommandEcho);
  AppTerminalRegisterCommand(&appTerminalButtonPress);
  AppTerminalRegisterCommand(&appTerminalPinCode);
}

/*************************************************************************************************/
/*!
 *  \fn     AppTerminalRegisterUartTxFunc
 *
 *  \brief  Register the UART Tx Function for the platform.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppTerminalRegisterUartTxFunc(appTerminalUartTx_t *uartTxFunc)
{
  appTerminalCb.terminalTx = uartTxFunc;
}

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
void AppTerminalRegisterCommand(appTerminalCommand_t *pCommand)
{
  appTerminalCommand_t *pCommandTemp = appTerminalCb.pFirstCommand;

  if (pCommandTemp == NULL)
  {
    appTerminalCb.pFirstCommand = pCommand;
  }
  else
  {
    while (pCommandTemp->pNext != NULL)
    {
      pCommandTemp = pCommandTemp->pNext;
    }

    pCommandTemp->pNext = pCommand;
  }

  pCommand->pNext = NULL;
}

/*************************************************************************************************/
/*!
 *  \fn       appTerminalExecute
 *
 *  \brief    Execute a received command.
 *
 *  \param    pBuf      Command string.
 *
 *  \return   None.
 */
/*************************************************************************************************/
static void appTerminalExecute(char *pBuf)
{
  uint32_t argc = 0;
  char    *argv[APP_TERMINAL_MAX_ARGC + 1];
  uint32_t length;
  char    *pBufCur;
  int      state;

  enum
  {
    STATE_OUTSIDE_OF_ARG,
    STATE_JUST_GOT_QUOTE,
    STATE_INSIDE_OF_ARG,
    STATE_INSIDE_OF_ARG_IN_QUOTES
  };

  /* Parse arguments in command */
  state  = STATE_OUTSIDE_OF_ARG;
  length = strlen(pBuf);
  for (pBufCur = pBuf; pBufCur < pBuf + length; pBufCur++)
  {
    switch (state)
    {
      case STATE_OUTSIDE_OF_ARG:
      {
        if (*pBufCur == '\"')
        {
          state = STATE_JUST_GOT_QUOTE;
        }
        else if (!APP_TERMINAL_IS_SPACE(*pBufCur))
        {
          state = STATE_INSIDE_OF_ARG;
          if (argc < APP_TERMINAL_MAX_ARGC)
          {
            argv[argc] = pBufCur;
          }
          argc++;
        }
        break;
      }
      case STATE_JUST_GOT_QUOTE:
      {
        if (argc < APP_TERMINAL_MAX_ARGC)
        {
          argv[state] = pBufCur;
        }
        argc++;
        if (*pBufCur == '\"')
        {
          state = STATE_OUTSIDE_OF_ARG;
          *pBufCur = '\0';
        }
        else
        {
          state = STATE_INSIDE_OF_ARG_IN_QUOTES;
        }
        break;
      }
      case STATE_INSIDE_OF_ARG:
      {
        if (APP_TERMINAL_IS_SPACE(*pBufCur))
        {
          state = STATE_OUTSIDE_OF_ARG;
          *pBufCur = '\0';
        }
        else if (*pBufCur == '\"')
        {
          state = STATE_JUST_GOT_QUOTE;
          *pBufCur = '\0';
        }
        break;
      }
      case STATE_INSIDE_OF_ARG_IN_QUOTES:
      {
        if (*pBufCur == '\"')
        {
          state = STATE_OUTSIDE_OF_ARG;
          *pBufCur = '\0';
        }
        break;
      }
    }
  }

  /* Find & invoke command. */
  if (argc > APP_TERMINAL_MAX_ARGC)
  {
    AppTerminalTxStr(APP_TERMINAL_STRING_ERROR "too many arguments" APP_TERMINAL_STRING_NEW_LINE);
  }
  else if (argc > 0)
  {
    appTerminalCommand_t *pCommand = appTerminalCb.pFirstCommand;

    while (pCommand != NULL)
    {
      if (strcmp(pCommand->pName, argv[0]) == 0)
      {
        break;
      }
      pCommand = pCommand->pNext;
    }

    if (pCommand == NULL)
    {
      AppTerminalTxStr(APP_TERMINAL_STRING_ERROR "unrecognized command \"");
      AppTerminalTxStr(argv[0]);
      AppTerminalTxStr("\"" APP_TERMINAL_STRING_NEW_LINE);
    }
    else
    {
      uint8_t r = pCommand->handler(argc, argv);
      switch (r)
      {
        case APP_TERMINAL_ERROR_EXEC:
        case APP_TERMINAL_ERROR_OK:
          break;
        case APP_TERMINAL_ERROR_BAD_ARGUMENTS:
          AppTerminalTxStr(APP_TERMINAL_STRING_ERROR "Invalid argument(s)" APP_TERMINAL_STRING_NEW_LINE);
          break;
        case APP_TERMINAL_ERROR_TOO_FEW_ARGUMENTS:
          AppTerminalTxStr(APP_TERMINAL_STRING_ERROR "Too few arguments" APP_TERMINAL_STRING_NEW_LINE);
          break;
        case APP_TERMINAL_ERROR_TOO_MANY_ARGUMENTS:
          AppTerminalTxStr(APP_TERMINAL_STRING_ERROR "Too many arguments" APP_TERMINAL_STRING_NEW_LINE);
          break;
        default:
          AppTerminalTxStr(APP_TERMINAL_STRING_ERROR "Unknown error" APP_TERMINAL_STRING_NEW_LINE);
          break;
      }
    }
  }
}

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
void AppTerminalHandler(wsfEventMask_t event, wsfMsgHdr_t *pMsg)
{
  if ((event & APP_TERMINAL_EVENT_COMMAND_RX) != 0)
  {
    appTerminalExecute(appTerminalCb.buf);
    AppTerminalTxStr(APP_TERMINAL_STRING_PROMPT);
    appTerminalCb.bufOffset   = 0;
    appTerminalCb.isExecuting = FALSE;
  }
}

/*************************************************************************************************/
/*!
 *  \fn     AppTerminalRx
 *
 *  \brief  Called by application when a data byte is received.
 *
 *  \param  dataByte  Received byte.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppTerminalRx(uint8_t dataByte)
{
  /* Hands off buf if command is executing. */
  if (!appTerminalCb.isExecuting)
  {
    /* If this is the end of a line, signal task. */
    if ((dataByte == '\n') || (dataByte == '\r'))
    {
      AppTerminalTxStr(APP_TERMINAL_STRING_NEW_LINE);
      appTerminalCb.buf[appTerminalCb.bufOffset] = '\0';
      WsfSetEvent(appTerminalCb.handlerId, APP_TERMINAL_EVENT_COMMAND_RX);
      appTerminalCb.isExecuting = TRUE;
    }

    /* Check for delete. */
    else if (dataByte == 0x7F)
    {
      if (appTerminalCb.bufOffset > 0)
      {
        appTerminalCb.bufOffset--;
        if (appTerminalCb.doEcho)
        {
          AppTerminalTxStr("\b \b");
        }
      }
    }

    /* If we still have room in the buf, put it in buf.  Othewise ignore it. */
    else if (appTerminalCb.bufOffset < APP_TERMINAL_MAX_COMMAND_LEN)
    {
      /* Ignore non-printable characters. */
      if (APP_TERMINAL_IS_PRINT(dataByte))
      {
        appTerminalCb.buf[appTerminalCb.bufOffset] = dataByte;
        appTerminalCb.bufOffset++;
        if (appTerminalCb.doEcho)
        {
          AppTerminalTxChar(dataByte);
        }
      }
    }
  }
}

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
void AppTerminalTxStr(const char *pStr)
{
  AppTerminalTx((const uint8_t *)pStr, strlen(pStr));
}

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
void AppTerminalTxChar(char c)
{
  AppTerminalTx((const uint8_t *)&c, 1);
}

/*************************************************************************************************/
/*!
 *  \fn     AppTerminalTxPrint
 *
 *  \brief  Called by application to print formatted data.
 *
 *  \param  pStr      Message format string
 *  \param  ...       Additional aguments, printf-style
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppTerminalTxPrint(const char *pStr, ...)
{
  int32_t len;
  char    buf[APP_TERMINAL_PRINTF_MAX_LEN];
  va_list args;

  va_start(args, pStr);
  len = Util_VSNPrintf(buf, APP_TERMINAL_PRINTF_MAX_LEN, pStr, args);
  va_end(args);

  AppTerminalTx((uint8_t *)buf, len);
}

/*************************************************************************************************/
/*!
 *  \fn     appTerminalCommandHelpHandler
 *
 *  \brief  Handler for a terminal command.
 *
 *  \param  argc      The number of arguments passed to the command.
 *  \param  argv      The array of arguments; the 0th argument is the command.
 *
 *  \return Error code.
 */
/*************************************************************************************************/
static uint8_t appTerminalCommandHelpHandler(uint32_t argc, char **argv)
{
  appTerminalCommand_t *pCommand = appTerminalCb.pFirstCommand;

  if (argc > 1)
  {
    return APP_TERMINAL_ERROR_TOO_MANY_ARGUMENTS;
  }

  while (pCommand != NULL)
  {
    AppTerminalTxStr(pCommand->pHelpStr);
    AppTerminalTxStr(APP_TERMINAL_STRING_NEW_LINE);

    pCommand = pCommand->pNext;
  }

  AppTerminalTxStr(APP_TERMINAL_STRING_NEW_LINE);
  return APP_TERMINAL_ERROR_OK;
}


/*************************************************************************************************/
/*!
 *  \fn     AppTerminalTx
 *
 *  \brief  Transmit buffer on platform UART.
 *
 *  \param  pBuf    Buffer to transmit.
 *  \param  len     Length of buffer in octets.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppTerminalTx(const uint8_t *pData, uint16_t len)
{
  WSF_ASSERT(appTerminalCb.terminalTx);

  (*appTerminalCb.terminalTx)(pData, len);
}

/*************************************************************************************************/
/*!
 *  \fn     appTerminalCommandEchoHandler
 *
 *  \brief  Handler for a terminal command.
 *
 *  \param  argc      The number of arguments passed to the command.
 *  \param  argv      The array of arguments; the 0th argument is the command.
 *
 *  \return Error code.
 */
/*************************************************************************************************/
static uint8_t appTerminalCommandEchoHandler(uint32_t argc, char **argv)
{
  if (argc < 2)
  {
    return APP_TERMINAL_ERROR_TOO_FEW_ARGUMENTS;
  }
  else if (argc == 2)
  {
    if (strcmp(argv[1], "on") == 0)
    {
      appTerminalCb.doEcho = TRUE;
      AppTerminalTxStr("echo on" APP_TERMINAL_STRING_NEW_LINE);
    }
    else if (strcmp(argv[1], "off") == 0)
    {
      appTerminalCb.doEcho = FALSE;
      AppTerminalTxStr("echo off" APP_TERMINAL_STRING_NEW_LINE);
    }
    else
    {
      return APP_TERMINAL_ERROR_BAD_ARGUMENTS;
    }
  }
  else
  {
    return APP_TERMINAL_ERROR_TOO_MANY_ARGUMENTS;
  }

  return APP_TERMINAL_ERROR_OK;
}

/*************************************************************************************************/
/*!
 *  \fn     appTerminalCommandBtnHandler
 *
 *  \brief  Handler for a button press terminal command.
 *
 *  \param  argc      The number of arguments passed to the command.
 *  \param  argv      The array of arguments; the 0th argument is the command.
 *
 *  \return Error code.
 */
/*************************************************************************************************/
static uint8_t appTerminalCommandBtnHandler(uint32_t argc, char **argv)
{
  if (argc < 3)
  {
    return APP_TERMINAL_ERROR_TOO_FEW_ARGUMENTS;
  }
  else if (argc == 3)
  {
    uint8_t btnIndx;
    uint8_t event;

    if (strcmp(argv[1], "1") == 0)
    {
      btnIndx = 1;
    }
    else if (strcmp(argv[1], "2") == 0)
    {
      btnIndx = 2;
    }
    else
    {
      return APP_TERMINAL_ERROR_BAD_ARGUMENTS;
    }

    if (strcmp(argv[2], "d") == 0)
    {
      AppTerminalTxPrint("Button %s Press" APP_TERMINAL_STRING_NEW_LINE, argv[1]);
      event = (btnIndx == 1? APP_UI_BTN_1_DOWN : APP_UI_BTN_2_DOWN);
    }
    else if (strcmp(argv[2], "s") == 0)
    {
      AppTerminalTxPrint("Short Button %s Press" APP_TERMINAL_STRING_NEW_LINE, argv[1]);
      event = (btnIndx == 1? APP_UI_BTN_1_SHORT : APP_UI_BTN_2_SHORT);
    }
    else if (strcmp(argv[2], "m") == 0)
    {
      AppTerminalTxPrint("Medium Button %s Press" APP_TERMINAL_STRING_NEW_LINE, argv[1]);
      event = (btnIndx == 1? APP_UI_BTN_1_MED : APP_UI_BTN_2_MED);
    }
    else if (strcmp(argv[2], "l") == 0)
    {
      AppTerminalTxPrint("Long Button %s Press" APP_TERMINAL_STRING_NEW_LINE, argv[1]);
      event = (btnIndx == 1? APP_UI_BTN_1_LONG : APP_UI_BTN_2_LONG);
    }
    else if (strcmp(argv[2], "x") == 0)
    {
      AppTerminalTxPrint("Medium Button %s Press" APP_TERMINAL_STRING_NEW_LINE, argv[1]);
      event = (btnIndx == 1? APP_UI_BTN_1_EX_LONG : APP_UI_BTN_2_EX_LONG);
    }
    else
    {
      return APP_TERMINAL_ERROR_BAD_ARGUMENTS;
    }

    AppUiBtnTest(event);
  }
  else
  {
    return APP_TERMINAL_ERROR_TOO_MANY_ARGUMENTS;
  }

  return APP_TERMINAL_ERROR_OK;
}

/*************************************************************************************************/
/*!
 *  \fn     appTerminalPinCodeHandler
 *
 *  \brief  Handler for a pin code terminal command.
 *
 *  \param  argc      The number of arguments passed to the command.
 *  \param  argv      The array of arguments; the 0th argument is the command.
 *
 *  \return Error code.
 */
/*************************************************************************************************/
static uint8_t appTerminalPinCodeHandler(uint32_t argc, char **argv)
{
  if (argc < 2)
  {
    return APP_TERMINAL_ERROR_TOO_FEW_ARGUMENTS;
  }
  else if (argc == 3)
  {
    uint32_t passkey;
    uint8_t  buf[SMP_PIN_LEN];
    uint8_t connId;

    passkey = atoi(argv[2]);
    connId = atoi(argv[1]);

    if (connId < 1 || connId > DM_CONN_MAX)
    {
      AppTerminalTxPrint("ConnID must be in the range [1 .. %d]\n", DM_CONN_MAX);
      return APP_TERMINAL_ERROR_BAD_ARGUMENTS;
    }

    passkey %= 1000000;

    /* convert to byte buffer */
    buf[0] = UINT32_TO_BYTE0(passkey);
    buf[1] = UINT32_TO_BYTE1(passkey);
    buf[2] = UINT32_TO_BYTE2(passkey);

    /* send authentication response to DM */
    DmSecAuthRsp((dmConnId_t) connId, SMP_PIN_LEN, buf);
  }
  else
  {
    return APP_TERMINAL_ERROR_TOO_MANY_ARGUMENTS;
  }

  return APP_TERMINAL_ERROR_OK;
}
