/*************************************************************************************************/
/*!
 *  \file   platform.c
 *
 *  \brief  Platform services.
 *
 *          $Date: 2016-06-02 14:19:49 -0700 (Thu, 02 Jun 2016) $
 *          $Revision: 7314 $
 *
 *  Copyright (c) 2016 ARM, Ltd., all rights reserved.
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

#include "wsf_types.h"

#include "SMD_TC2.h"
#include "platform.h"

/**************************************************************************************************
  Data Types
**************************************************************************************************/

/*! \brief    Platform UART control block. */
static struct
{
  struct
  {
    const uint8_t             *pBuf;      /*!< Buffer being transmitted. */
    uint32_t                  len;        /*!< Length of transmit buffer. */
    uint32_t                  offset;     /*!< Offset of next byte to transmit within buffer. */
    platformUartTxCmplCback_t cback;      /*!< Tx complete callback. */
  } tx;

  struct
  {
    platformUartRxCback_t     cback;      /*!< Rx callback. */
  } rx;

  bool_t                      init;       /*!< TRUE if UART initialized. */

} platformUartCb = { { 0 }, { 0 }, FALSE };

/*! \brief    Counter to keep MCU from entering deep sleep. */
static uint8_t platformWakeLock = 0;

/*! \brief    Sleep and wake callbacks. */
static platformSleepCback_t platformSleepCback = NULL;
static platformSleepCback_t platformWakeCback = NULL;

/*! \brief    Wakeup input callback. */
static platformWakeupCback_t platformWakeupCback = NULL;

/**************************************************************************************************
  Function Prototypes
**************************************************************************************************/

/*! \brief  UART interrupt handler. */
void UART0_Handler(void);

/*! \brief  32K timer interrupt handler. */
void TIMER_32K_Handler(void);

/*************************************************************************************************/
/*!
 *  \brief  Initialize the platform GPIO.
 *
 *  \return None.
 */
/*************************************************************************************************/
void PlatformGpioInit(void)
{
  /* Configure IOMux. */
  IOM->MODESEL = IOM_MODE_SMDEVAL;

  /* Clear GPIOs. */
  GPIOA->ALTFUNCCLR = 0xFFFF;
  GPIOA->OUTENCLR = 0xFFFF;
}

/*************************************************************************************************/
/*!
 *  \brief  Initialize the platform UART.
 *
 *  \return None.
 */
/*************************************************************************************************/
void PlatformUartInit(void)
{
  /* Stop UART. */
  UART0->CTRL = 0;

  /* Explicitly clear state */
  UART0->DATA;
  UART0->STATE    = UART_STATE_TX_ORN | UART_STATE_RX_ORN;
  UART0->INTCLEAR = 0xFFFF;

  /* Set up divisor. */
  UART0->BAUDDIV = 139;   /* 115200 baud */

  /* Enable TX and RX. */
  UART0->CTRL = UART_CTRL_TX_EN;

  /* Set up MUX appropriately. */
  GPIOA->ALTFUNCSET = GPIO0_AF_UART0_TX | GPIO0_AF_UART0_RX;

  if (!platformUartCb.init)
  {
    /* Enable interrupts. */
    NVIC_SetPriority(UART0_IRQn, 0);
    NVIC_ClearPendingIRQ(UART0_IRQn);
    NVIC_EnableIRQ(UART0_IRQn);
  }
  else
  {
    if (platformUartCb.rx.cback != NULL)
    {
      PlatformUartRxAsyncStart(platformUartCb.rx.cback);
    }
  }

  platformUartCb.init = TRUE;
}

/*************************************************************************************************/
/*!
 *  \brief  Start receiving data asynchronously on UART.
 *
 *  \brief  cback   Callback to invoke when byte is received.
 *
 *  \return None.
 */
/*************************************************************************************************/
void PlatformUartRxAsyncStart(platformUartRxCback_t cback)
{
  platformUartCb.rx.cback = cback;

  /* Start receiving data. */
  UART0->CTRL |= UART_CTRL_RX_EN | UART_CTRL_RXI_EN;
}

/*************************************************************************************************/
/*!
 *  \brief  Transmit buffer on platform UART asynchronously.
 *
 *  \param  pBuf    Buffer to transmit.
 *  \param  len     Length of buffer in octets.
 *  \param  cback   Callback to invoke when transmission is complete.
 *
 *  \return None.
 */
/*************************************************************************************************/
void PlatformUartTxAsync(const uint8_t *pBuf, uint32_t len, platformUartTxCmplCback_t cback)
{
  platformUartCb.tx.pBuf   = pBuf;
  platformUartCb.tx.len    = len;
  platformUartCb.tx.offset = 0;
  platformUartCb.tx.cback  = cback;

  /* Transmit first octet. */
  platformUartCb.tx.offset++;
  UART0->CTRL |= UART_CTRL_TXI_EN;
  UART0->DATA  = *pBuf;

  PlatformWakeLockInc();
}

/*************************************************************************************************/
/*!
 *  \brief  Transmit buffer on platform UART.
 *
 *  \param  pBuf    Buffer to transmit.
 *  \param  len     Length of buffer in octets.
 *
 *  \return None.
 */
/*************************************************************************************************/
void PlatformUartTx(const uint8_t *pBuf, uint32_t len)
{
  while (len-- > 0)
  {
    while ((UART0->STATE & UART_STATE_TXFULL) != 0) { }
    UART0->DATA = *pBuf++;
  }
}

/*************************************************************************************************/
/*!
 *  \brief  Platform UART interrupt handler.
 *
 *  \return None.
 */
/*************************************************************************************************/
void UART0_Handler(void)
{
  if (UART0->INTSTATUS & UART_INT_TX)
  {
    UART0->INTCLEAR = UART_INT_TX;        /* clear */
    if ((platformUartCb.tx.pBuf != NULL) && (platformUartCb.tx.offset < platformUartCb.tx.len))
    {
      UART0->DATA = *(platformUartCb.tx.pBuf + platformUartCb.tx.offset);
      platformUartCb.tx.offset++;
    }
    else
    {
      platformUartTxCmplCback_t cback = platformUartCb.tx.cback;
      const uint8_t             *pBuf = platformUartCb.tx.pBuf;

      UART0->CTRL &= ~UART_CTRL_TXI_EN;

      platformUartCb.tx.pBuf   = NULL;
      platformUartCb.tx.len    = 0u;
      platformUartCb.tx.offset = 0u;
      platformUartCb.tx.cback  = NULL;

      if (cback != NULL)
      {
        cback(pBuf);
      }

      PlatformWakeLockDec();
    }
  }
  if (UART0->INTSTATUS & UART_INT_RX)
  {
    if (platformUartCb.rx.cback != NULL)
    {
      platformUartCb.rx.cback(UART0->DATA);
    }
    UART0->INTCLEAR = UART_INT_RX;        /* clear */
  }
  if (UART0->INTSTATUS & UART_INT_TXORN)
  {
    UART0->INTCLEAR = UART_INT_TXORN;     /* clear */
    UART0->STATE = UART_STATE_TX_ORN;
  }
  if (UART0->INTSTATUS & UART_INT_RXORN)
  {
    UART0->INTCLEAR = UART_INT_RXORN;     /* clear */
    UART0->STATE = UART_STATE_RX_ORN;
  }
}

/*************************************************************************************************/
/*!
 *  \brief  Increment counter to keep MCU from entering deep sleep.
 *
 *  \return None.
 */
/*************************************************************************************************/
void PlatformWakeLockInc(void)
{
  platformWakeLock++;
}

/*************************************************************************************************/
/*!
 *  \brief  Increment counter to keep MCU from entering deep sleep.
 *
 *  \return None.
 */
/*************************************************************************************************/
void PlatformWakeLockDec(void)
{
  platformWakeLock--;
}

/*************************************************************************************************/
/*!
 *  \brief  Get status of counter to keep MCU from entering deep sleep.
 *
 *  \return TRUE if MCU prevented from deep sleep.
 */
/*************************************************************************************************/
bool_t PlatformGetWakeLockStatus(void)
{
  return (platformWakeLock > 0);
}

/*************************************************************************************************/
/*!
 *  \brief  Initialize sleep handling.
 *
 *  \param  sleepCback        Sleep callback.
 *  \param  wakeCback         Wake callback.
 *
 *  \return None.
 */
/*************************************************************************************************/
void PlatformInitSleep(platformSleepCback_t sleepCback, platformSleepCback_t wakeCback)
{
  platformSleepCback = sleepCback;
  platformWakeCback = wakeCback;

  POWERCON->TIMER_32K_CONTROL    = POWERCON_TIMER_32K_CONTROL_ENABLE;
  POWERCON->TIMER_TIMEOUT_STATUS = 0u;

  /* Enable the interrupt. */
  NVIC_SetPriority(TIMER_32K_IRQn, 0);
  NVIC_ClearPendingIRQ(TIMER_32K_IRQn);
  NVIC_EnableIRQ(TIMER_32K_IRQn);
}

/*************************************************************************************************/
/*!
 *  \brief  Sleep or deep sleep for a number of ticks.
 *
 *  \param  ticks     Number of ticks to sleep.
 *  \param  deep      TRUE to enable deep sleep.
 *
 *  \return None.
 */
/*************************************************************************************************/
void PlatformSleep(uint32_t ticks, bool_t deep)
{
  uint32_t timerCount;
  uint32_t timerCountNext;

  if (PlatformGetWakeLockStatus())
  {
    deep = FALSE;
  }

  /* Calculate count at timeout & set timer value.  If zero, we will just sleep forever. */
  if (ticks != 0)
  {
    POWERCON->TIMER_32K_CONTROL &= ~POWERCON_TIMER_32K_CONTROL_TO_ENABLE;
    timerCount                   =  POWERCON->TIMER_32K_COUNT;
    timerCountNext               =  timerCount + ticks;
    POWERCON->TIMER_32K_HIGH16   = (timerCountNext & 0xFFFF0000);
    POWERCON->TIMER_32K_LOW16    = (timerCountNext & 0x0000FFFF);
    POWERCON->TIMER_32K_CONTROL |=  POWERCON_TIMER_32K_CONTROL_TO_ENABLE;
  }

  /* Set appropriate sleep and wake settings */
  POWERCON->SLEEP_NEXT_CFG = 0x15;
  POWERCON->SLEEP_NEXT_CLK = POWERCON->DYNAMIC_CLK;
  POWERCON->WAKE_NEXT_CLK  = POWERCON->DYNAMIC_CLK;

  if (deep)
  {
    /* Prepare peripherals for sleep. */
    if (platformSleepCback != NULL)
    {
      platformSleepCback();
    }

    /* Enable deep sleep in CPU. */
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
  }

  __WFI();

  if (deep)
  {
    /* Clear disable bit. */
    SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;

    /* Wake peripherals up. */
    if (platformWakeCback != NULL)
    {
      platformWakeCback();
    }
  }

  /* Disable the timeout. */
  POWERCON->TIMER_32K_CONTROL   &= ~POWERCON_TIMER_32K_CONTROL_TO_ENABLE;
  POWERCON->TIMER_TIMEOUT_STATUS = 0;
}

/*************************************************************************************************/
/*!
 *  \brief  Read value of sleep timer.
 *
 *  \return Value of sleep timer.
 */
/*************************************************************************************************/
uint32_t PlatformReadSleepTimer(void)
{
  return POWERCON->TIMER_32K_COUNT;
}

/*************************************************************************************************/
/*!
 *  \brief  Enable wakeup input.
 *
 *  \param  wakeupCback       Wakeup input callback.
 *
 *  \return None.
 */
/*************************************************************************************************/
void PlatformEnableWakeup(platformWakeupCback_t wakeupCback)
{
  platformWakeupCback = wakeupCback;
  if (wakeupCback != NULL)
  {
    POWERCON->TIMER_32K_CONTROL |= POWERCON_TIMER_32K_CONTROL_GPIO_WAKEUP_EN;
  }
  else
  {
    POWERCON->TIMER_32K_CONTROL &= ~POWERCON_TIMER_32K_CONTROL_GPIO_WAKEUP_EN;
  }
}

/*************************************************************************************************/
/*!
 *  \brief  32K timer handler.
 *
 *  \return None.
 */
/*************************************************************************************************/
void TIMER_32K_Handler(void)
{
  static bool_t falling = TRUE;
  
  NVIC_ClearPendingIRQ(TIMER_32K_IRQn);

  if (POWERCON->TIMER_TIMEOUT_STATUS & POWERCON_TIMER_TIMEOUT_STATUS_TIMER)
  {
    POWERCON->TIMER_TIMEOUT_STATUS &= ~POWERCON_TIMER_TIMEOUT_STATUS_TIMER;
    POWERCON->TIMER_32K_CONTROL    &= ~POWERCON_TIMER_32K_CONTROL_TO_ENABLE;
  }

  if (POWERCON->TIMER_TIMEOUT_STATUS & POWERCON_TIMER_TIMEOUT_STATUS_GPIO)
  {
    /* Reverse polarity so interrupt does not continually trigger. */
    POWERCON->TIMER_32K_CONTROL ^= POWERCON_TIMER_32K_CONTROL_GPIO_WAKEUP_POLARITY;

    /* Clear status. */
    POWERCON->TIMER_TIMEOUT_STATUS &= ~POWERCON_TIMER_TIMEOUT_STATUS_GPIO;
    
    /* Only call callback on falling  edge. */
    if (platformWakeupCback != NULL && falling)
    {
      falling = FALSE;
      platformWakeupCback();
    }
      
    /* Reset falling boolean on rising edge */
    if ((POWERCON->TIMER_32K_CONTROL & POWERCON_TIMER_32K_CONTROL_GPIO_WAKEUP_POLARITY) == 0)
    {
      falling = TRUE;
    }
  }
}

/*************************************************************************************************/
/*!
 *  \brief  Delay for a certain time, in microseconds.
 *
 *  \param  delayUsec       Microseconds to delay.
 *
 *  \return None.
 */
/*************************************************************************************************/
void PlatformDelayUsec(uint32_t delayUsec)
{
  const uint32_t usecPerSec = 1000000;
  const uint32_t usecPerLoop = 8;

  volatile uint32_t loops = SystemCoreClock / usecPerSec * delayUsec / usecPerLoop;
  while (loops--);
}

/*************************************************************************************************/
/*!
 *  \brief  Reset after a delay.
 *
 *  \param  delayMs           Delay, in milliseconds.
 *
 *  \return None.
 */
/*************************************************************************************************/
void PlatformReset(uint16_t delayMs)
{
  uint32_t delayTicks;

  /* Enable the interrupt. */
  NVIC_SetPriority(WDT_IRQn, 0);
  NVIC_ClearPendingIRQ(WDT_IRQn);
  NVIC_EnableIRQ(WDT_IRQn);

  delayTicks = delayMs * (SystemCoreClock / 1000u);
  if (delayTicks == 0u)
  {
    delayTicks = 1u; /* minimum value */
  }

  if ((ARM_WDOG->LOCK & WDG_LOCK_WRDIS) == 0u)
  {
    ARM_WDOG->LOCK = WDG_LOCK_DIS;
  }
  ARM_WDOG->CONTROL &= ~WDG_CTRL_RESEN;
  ARM_WDOG->LOAD     = delayTicks;
  ARM_WDOG->CONTROL |= WDG_CTRL_RESEN;
  ARM_WDOG->CONTROL |= WDG_CTRL_INTEN;
}
