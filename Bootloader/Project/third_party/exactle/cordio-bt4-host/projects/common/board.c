/*************************************************************************************************/
/*!
 *  \file   board.c
 *
 *  \brief  Board services.
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

#include "cordio_sdk_version.h"
#include "SMD_TC2.h"

#include "wsf_types.h"
#include "wsf_trace.h"

#include "board.h"

#include "platform.h"
#include "llc.h"

#ifdef INIT_FLASH
#include "board_flash.h"
#endif
#ifdef INIT_I2C
#include "board_i2c.h"
#endif
#ifdef INIT_ITG3400
#include "board_itg3400.h"
#endif
#ifdef INIT_TMP102
#include "board_tmp102.h"
#endif

#include "cordio_tc2_defs.h"

/**************************************************************************************************
  Macros
**************************************************************************************************/

/*! \brief      Convert literal to string. */
#define BOARD_XSTRINGIFY(x)   #x
#define BOARD_STRINGIFY(x)    BOARD_XSTRINGIFY(x)

/*! \brief      Indicates demo mode of application. */
#define BOARD_GPIO_DEMO_MODE_SELECTION  GPIO_GPIO4

/*! \brief      Indicates test mode of application. */
#define BOARD_GPIO_TEST_MODE_SELECTION  GPIO_GPIO5

/*! \brief      I2C configuration. */
#define BOARD_I2C_CLOCK_MODE  400       /*!< Clock mode (speed in kHz). */
#define BOARD_I2C_TIMEOUT     1000      /*!< Timeout (1 second). */
#define BOARD_TMP102_ADDR     0x49      /*!< Address TMP102 on bus. */
#define BOARD_ITG3400_ADDR    0x68      /*!< Address of ITG3400 on bus. */

/*! \brief      GPO configuration. */
#define BOARD_GPIO_OUTPUT_QUANTITY  1u
#define BOARD_GPIO_OUTPUT_0         GPIO_GPIO3
#define BOARD_GPIO_OUTPUTS         {BOARD_GPIO_OUTPUT_0}
#define BOARD_GPIO_OUTPUTS_MASK    (BOARD_GPIO_OUTPUT_0)

/*************************************************************************************************/
/*!
 *  \brief  Sleep callback.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void boardSleep(void)
{

}

/*************************************************************************************************/
/*!
 *  \brief  Wake callback.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void boardWake(void)
{
  PlatformUartInit();

#ifdef INIT_I2C
  BoardI2cInit(BOARD_I2C_CLOCK_MODE, BOARD_I2C_TIMEOUT);
#endif

#ifdef INIT_FLASH
  BoardFlashInit();
#endif
}

/*************************************************************************************************/
/*!
 *  \brief  Initialize token service.
 *
 *  \return None.
 */
/*************************************************************************************************/
#if (WSF_TOKEN_ENABLED == TRUE)
static void boardTokenInit(void)
{
  uint64_t syncWord = 0xFFFFFFFFFFFFFFFF;
  PlatformUartTx((uint8_t *)&syncWord, sizeof(syncWord));
}
#endif

/*************************************************************************************************/
/*!
 *  \brief  Initialize board HW.
 *
 *  \return None.
 */
/*************************************************************************************************/
void BoardHwInit(void)
{
  /* Initialize the platform GPIO. */
  PlatformGpioInit();

  /* Request the 32-MHz XTAL. */
  LlcRequest32MXtal(TRUE);

  /* Initialize platform UART. */
  PlatformUartInit();

#if (WSF_TOKEN_ENABLED == TRUE)
  boardTokenInit();
#endif

  APP_TRACE_INFO0("ARM Cordio TC2 SDK v" BOARD_STRINGIFY(CORDIO_SDK_VERSION_MAJOR) "." BOARD_STRINGIFY(CORDIO_SDK_VERSION_MINOR) " " CORDIO_SDK_VERSION);

#ifdef INIT_GPO
  GPIOA->OUTENSET =  BOARD_GPIO_OUTPUTS_MASK;
  GPIOA->DATAOUT &= ~BOARD_GPIO_OUTPUTS_MASK;
#endif

#ifdef INIT_FLASH
  APP_TRACE_INFO0("mainboard: init SPI flash");
  if (!BoardFlashInit())
  {
    BoardFlashSleep();
    APP_TRACE_ERR0("mainboard: could not initialize SPI flash");
  }
#endif

#ifdef INIT_I2C
  APP_TRACE_INFO0("mainboard: init I2C");
  if (!BoardI2cInit(BOARD_I2C_CLOCK_MODE, BOARD_I2C_TIMEOUT))
  {
    APP_TRACE_ERR0("mainboard: I2C init failed");
  }
#endif

  /* Initialize sleep handling. */
  PlatformInitSleep(boardSleep, boardWake);
}

/*************************************************************************************************/
/*!
 *  \brief  Initialize board I2C devices.
 *
 *  \return None.
 */
/*************************************************************************************************/
#ifdef INIT_I2C
void BoardI2cDevicesInit(void)
{
#ifdef INIT_TMP102
  /* Initialize TMP102 */
  APP_TRACE_INFO0("mainboard: init TMP102");
  if (!BoardTmp102Init(BOARD_TMP102_ADDR))
  {
    APP_TRACE_ERR0("mainboard: TMP102 init failed");
  }
#endif

#ifdef INIT_ITG3400
  /* Initialize ITG3400 */
  APP_TRACE_INFO0("mainboard: init ITG3400");
  if (!BoardItg3400Init(BOARD_ITG3400_ADDR))
  {
    APP_TRACE_ERR0("mainboard: ITG3400 init failed");
  }
#endif
}
#endif

/*************************************************************************************************/
/*!
 *  \brief  Get application mode.
 *
 *  \return Application mode.
 */
/*************************************************************************************************/
uint8_t BoardGetMode(void)
{
  uint8_t mode = BOARD_MODE_SENSOR;

  if ((GPIOA->DATA & BOARD_GPIO_TEST_MODE_SELECTION) != 0)
  {
    mode = BOARD_MODE_DTM;
  }
  if (mode == BOARD_MODE_SENSOR)
  {
    if ((GPIOA->DATA & BOARD_GPIO_DEMO_MODE_SELECTION) != 0)
    {
      mode = BOARD_MODE_BEACON;
    }
  }

  return mode;
}

/*************************************************************************************************/
/*!
 *  \brief  Set mux mode.
 *
 *  \param  mode      Mux mode.
 *
 *  \return None.
 */
/*************************************************************************************************/
void BoardSetMuxMode(uint8_t mode)
{
  switch (mode)
  {
    case BOARD_MUXMODE_NORMAL:
      IOM->MODESEL = IOM_MODE_SMDEVAL;
      break;
    case BOARD_MUXMODE_DIGITAL_TEST:
      IOM->MODESEL = IOM_MODE_DIGITAL_TEST;
      break;
  }
}

/*************************************************************************************************/
/*!
 *  \brief  Set GPO.
 *
 *  \param  num       GPO to set.
 *
 *  \return None.
 */
/*************************************************************************************************/
#ifdef INIT_GPO
void BoardGpoSet(uint8_t num)
{
  static uint32_t gpio[] = BOARD_GPIO_OUTPUTS;

  if (num <= BOARD_GPIO_OUTPUT_QUANTITY)
  {
    GPIOA->DATAOUT |= gpio[num];
  }
}
#endif

/*************************************************************************************************/
/*!
 *  \brief  Clear GPO.
 *
 *  \param  num       GPO to clear.
 *
 *  \return None.
 */
/*************************************************************************************************/
#ifdef INIT_GPO
void BoardGpoClr(uint8_t num)
{
  static uint32_t gpio[] = BOARD_GPIO_OUTPUTS;

  if (num <= BOARD_GPIO_OUTPUT_QUANTITY)
  {
    GPIOA->DATAOUT &= ~gpio[num];
  }
}
#endif

/*************************************************************************************************/
/*!
 *  \brief  Toggle GPO.
 *
 *  \param  num       GPO to toggle.
 *
 *  \return None.
 */
/*************************************************************************************************/
#ifdef INIT_GPO
void BoardGpoToggle(uint8_t num)
{
  static uint32_t gpio[] = BOARD_GPIO_OUTPUTS;

  if (num <= BOARD_GPIO_OUTPUT_QUANTITY)
  {
    GPIOA->DATAOUT ^= gpio[num];
  }
}
#endif

/*************************************************************************************************/
/*!
 *  \brief  Read the buttons on the board.
 *
 *  \param  none.
 *
 *  \return bitmask with button state.
 */
/*************************************************************************************************/
uint32_t BoardReadBtns()
{
  uint32_t mask = 0;
  
  if ((GPIOA->DATA & GPIO_GPIO2) == 0)
  {
    mask |= 0x1;
  }
  
  return mask;
}
