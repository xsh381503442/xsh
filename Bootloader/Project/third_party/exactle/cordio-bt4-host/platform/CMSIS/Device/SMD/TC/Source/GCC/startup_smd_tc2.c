/*************************************************************************************************/
/*!
 *  \file
 *
 *  \brief  Cordio TC2 core system startup for GCC compiler directives.
 *
 *  $Date: 2015-10-13 12:12:37 -0400 (Tue, 13 Oct 2015) $
 *  $Revision: 4202 $
 *
 *  Copyright (c) 2015 ARM, all rights reserved.
 *  ARM confidential and proprietary.
 *
 *  IMPORTANT.  Your use of this file is governed by a Software License Agreement
 *  ("Agreement") that must be accepted in order to download or otherwise receive a
 *  copy of this file.  You may not use or copy this file for any purpose other than
 *  as described in the Agreement.  If you do not agree to all of the terms of the
 *  Agreement do not use this file and delete all copies in your possession or control;
 *  if you do not have a copy of the Agreement, you must contact ARM prior
 *  to any use, copying or further distribution of this software.
 */
/*************************************************************************************************/

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "SMD_TC2.h"

/**************************************************************************************************
  Macros
**************************************************************************************************/

/*! Linker memory section definition. */
#define SECTION(x)  __attribute__ ((section(x)))

/*! Weak symbol reference. */
#define WEAK        __attribute__ ((weak))

/**************************************************************************************************
  Functions
**************************************************************************************************/

extern int main(void);
extern void SystemInit(void);
extern void PlatformUartTx(const uint8_t *pBuf, uint32_t len);
static void SystemDefaultHandler(void);

/* Core vectors. */
void WEAK Reset_Handler(void);
void WEAK NMI_Handler(void);
void WEAK HardFault_Handler(void);
void WEAK SVC_Handler(void);
void WEAK PendSV_Handler(void);
void WEAK SysTick_Handler(void);
/* Additional vectors. */
void WEAK TIMER_32K_Handler(void);
void WEAK GPIO_2_Handler(void);
void WEAK GPIO_3_Handler(void);
void WEAK UART0_Handler(void);
void WEAK SPI0_Handler(void);
void WEAK TIMER_DUAL_Handler(void);
void WEAK CLOCKSTATUS_Handler(void);
void WEAK WDT_Handler(void);
void WEAK UART1_Handler(void);
void WEAK TWI0_Handler(void);
void WEAK GPIO_10_Handler(void);
void WEAK GPIO_11_Handler(void);
void WEAK GPIO_12_Handler(void);
void WEAK GPIO_13_Handler(void);
void WEAK GPIO_COMBINED_Handler(void);
/* LLCC vectors. */
void WEAK LLCC_TXEVT_EMPTY_Handler(void);
void WEAK LLCC_TXCMD_EMPTY_Handler(void);
void WEAK LLCC_RXEVT_VALID_Handler(void);
void WEAK LLCC_RXCMD_VALID_Handler(void);
void WEAK LLCC_TXDMAL_DONE_Handler(void);
void WEAK LLCC_RXDMAL_DONE_Handler(void);
void WEAK LLCC_TXDMAH_DONE_Handler(void);
void WEAK LLCC_RXDMAH_DONE_Handler(void);

/* Assign default weak references. Override these values by defining a new function with the same name. */
#pragma weak NMI_Handler              = SystemDefaultHandler
#pragma weak SVC_Handler              = SystemDefaultHandler
#pragma weak PendSV_Handler           = SystemDefaultHandler
#pragma weak SysTick_Handler          = SystemDefaultHandler
#pragma weak TIMER_32K_Handler        = SystemDefaultHandler
#pragma weak GPIO_2_Handler           = SystemDefaultHandler
#pragma weak GPIO_3_Handler           = SystemDefaultHandler
#pragma weak UART0_Handler            = SystemDefaultHandler
#pragma weak SPI0_Handler             = SystemDefaultHandler
#pragma weak TIMER_DUAL_Handler       = SystemDefaultHandler
#pragma weak CLOCKSTATUS_Handler      = SystemDefaultHandler
#pragma weak WDT_Handler              = SystemDefaultHandler
#pragma weak UART1_Handler            = SystemDefaultHandler
#pragma weak TWI0_Handler             = SystemDefaultHandler
#pragma weak GPIO_10_Handler          = SystemDefaultHandler
#pragma weak GPIO_11_Handler          = SystemDefaultHandler
#pragma weak GPIO_12_Handler          = SystemDefaultHandler
#pragma weak GPIO_13_Handler          = SystemDefaultHandler
#pragma weak GPIO_COMBINED_Handler    = SystemDefaultHandler
#pragma weak LLCC_TXEVT_EMPTY_Handler = SystemDefaultHandler
#pragma weak LLCC_TXCMD_EMPTY_Handler = SystemDefaultHandler
#pragma weak LLCC_RXEVT_VALID_Handler = SystemDefaultHandler
#pragma weak LLCC_RXCMD_VALID_Handler = SystemDefaultHandler
#pragma weak LLCC_TXDMAL_DONE_Handler = SystemDefaultHandler
#pragma weak LLCC_RXDMAL_DONE_Handler = SystemDefaultHandler
#pragma weak LLCC_TXDMAH_DONE_Handler = SystemDefaultHandler
#pragma weak LLCC_RXDMAH_DONE_Handler = SystemDefaultHandler

/**************************************************************************************************
  Global variables
**************************************************************************************************/

/* Defined by linker */
extern unsigned long _sidata;
extern unsigned long _sdata;
extern unsigned long _edata;
extern unsigned long _sbss;
extern unsigned long _ebss;
extern unsigned long _sstack;
extern unsigned long _estack;

const unsigned int INIT_STACK_VAL = 0xAFAFAFAF;

/*! Core vector table (must be placed at address VTOR, i.e. 0x00000000). */
SECTION(".isr_vectors")
void (* const systemVectors[])(void) =
{
  (void (*)(void))(&_estack), /* The stack pointer after relocation */
  Reset_Handler,              /* Reset Handler */
  NMI_Handler,                /* NMI Handler */
  HardFault_Handler,          /* Hard Fault Handler */
  0,                          /* Reserved */
  0,                          /* Reserved */
  0,                          /* Reserved */
  0,                          /* Reserved */
  0,                          /* Reserved */
  0,                          /* Reserved */
  0,                          /* Reserved */
  SVC_Handler,                /* SVCall Handler */
  0,                          /* Reserved */
  0,                          /* Reserved */
  PendSV_Handler,             /* PendSV Handler */
  SysTick_Handler,            /* SysTick Handler */

  /* External interrupts, 32 total on CM0+ */
  TIMER_32K_Handler,
  UART0_Handler,
  GPIO_2_Handler,
  GPIO_3_Handler,

  0, 0,
  SPI0_Handler,
  TIMER_DUAL_Handler,

  CLOCKSTATUS_Handler,
  WDT_Handler,
  UART1_Handler,
  TWI0_Handler,

  GPIO_10_Handler,
  GPIO_11_Handler,
  GPIO_12_Handler,
  GPIO_13_Handler,

  GPIO_COMBINED_Handler,
  0, 0, 0,
  0, 0, 0, 0,

  LLCC_TXEVT_EMPTY_Handler,
  LLCC_TXCMD_EMPTY_Handler,
  LLCC_RXEVT_VALID_Handler,
  LLCC_RXCMD_VALID_Handler,

  LLCC_TXDMAL_DONE_Handler,
  LLCC_RXDMAL_DONE_Handler,
  LLCC_TXDMAH_DONE_Handler,
  LLCC_RXDMAH_DONE_Handler,
};

/*************************************************************************************************/
/*!
 *  \brief      Reset handler (C).
 *
 *  \param      None.
 *
 *  \return     None.
 */
/*************************************************************************************************/
__attribute__((externally_visible)) void Reset_HandlerC(void)
{
  unsigned long *src, *dst;

  /* Explicitly disable all IRQs in case boot loader left things running */
  NVIC->ICER[0] = 0xffffffff;

  /* copy the data segment into ram */
  src = &_sidata;
  dst = &_sdata;
  if (src != dst)
  {
    while (dst < &_edata)
    {
      *(dst++) = *(src++);
    }
  }

  /* zero the bss segment */
  dst = &_sbss;
  while(dst < &_ebss)
  {
    *(dst++) = 0;
  }

  /* initialize stack */
  dst = &_sstack;
  while (dst < &_estack)
  {
    *(dst++) = INIT_STACK_VAL;
  }

  /* Set up base CPU stuffs.  This clobbers vector table. */
  SystemInit();

  /* Application entry. */
  main();

  /* Invoke semihosting exit on main() return. */
  __asm volatile ("mov    r0,#0x18");
  __asm volatile ("bkpt   0xAB");
}

/*************************************************************************************************/
/*!
 *  \brief      Reset handler.
 *
 *  \param      None.
 *
 *  \return     None.
 */
/*************************************************************************************************/
__attribute__((naked)) void Reset_Handler(void)
{
  /* Explicitly clear all HW registers */
  __asm
  (
    ".syntax unified\n"
    "MOVS   R0,  #0  \n"
    "MOVS   R1,  #0  \n"
    "MOVS   R2,  #0  \n"
    "MOVS   R3,  #0  \n"
    "MOVS   R4,  #0  \n"
    "MOVS   R5,  #0  \n"
    "MOVS   R6,  #0  \n"
    "MOVS   R7,  #0  \n"
    "MOV    R8,  R0  \n"
    "MOV    R9,  R0  \n"
    "MOV    R10, R0  \n"
    "MOV    R11, R0  \n"
    "MOV    R12, R0  \n"
    "B     Reset_HandlerC  \n"
    ".syntax divided\n"
  );
}

/*************************************************************************************************/
/*!
 *  \brief      Default vector handler.
 *
 *  \param      None.
 *
 *  \return     None.
 */
/*************************************************************************************************/
void SystemDefaultHandler(void)
{
  for (;;)
  {
    ;
  }
}

/*************************************************************************************************/
/*!
 *  \brief      Print register value
 *
 *  \param      regName   Register name.
 *  \param      regVal    Register value.
 *
 *  \return     None.
 */
/*************************************************************************************************/
static void printReg(const char* regName, uint32_t regVal)
{
  char buf[8 + 1];
  uint32_t i;

  for (i = 0; i < 8; i++)
  {
    uint8_t n = regVal & 0x0F;
    buf[7- i] = (n < 10) ? ('0' + n) : ('A' + (n - 10));
    regVal >>= 4;
  }
  buf[8] = '\0';

  PlatformUartTx((uint8_t *)regName, strlen(regName));
  PlatformUartTx((uint8_t *)" = 0x", 5);
  PlatformUartTx((uint8_t *)buf, 8);
  PlatformUartTx((uint8_t *)"\r\n", 2);
}

/*************************************************************************************************/
/*!
 *  \brief      Hard fault handler (C).
 *
 *  \param      hardFaultRegs   Register values saved when hard fault occurred.
 *
 *  \return     None.
 */
/*************************************************************************************************/
__attribute__((externally_visible)) void HardFault_HandlerC(unsigned long hardFaultRegs[8])
{
  PlatformUartTx((uint8_t *)"\r\n*** Hard fault; reg summary:\r\n", 32);
  printReg("  R0",  hardFaultRegs[0]);
  printReg("  R1",  hardFaultRegs[1]);
  printReg("  R2",  hardFaultRegs[2]);
  printReg("  R3",  hardFaultRegs[3]);
  printReg("  R12", hardFaultRegs[4]);
  printReg("  LR",  hardFaultRegs[5]);
  printReg("  PC",  hardFaultRegs[6]);
  printReg("  PSR", hardFaultRegs[7]);
  printReg("  CFSR", (*((volatile unsigned long *)(0xE000ED28))));
  printReg("  HFSR", (*((volatile unsigned long *)(0xE000ED2C))));
  printReg("  DFSR", (*((volatile unsigned long *)(0xE000ED30))));
  printReg("  AFSR", (*((volatile unsigned long *)(0xE000ED3C))));
  printReg("  MMAR", (*((volatile unsigned long *)(0xE000ED34))));
  printReg("  BFAR", (*((volatile unsigned long *)(0xE000ED38))));

  for (;;)
  {
    ;
  }
}

/*************************************************************************************************/
/*!
 *  \brief      Hard fault handler.
 *
 *  \param      None.
 *
 *  \return     None.
 */
/*************************************************************************************************/
__attribute__((naked)) void HardFault_Handler(void)
{
  __asm(  ".syntax unified\n"
                  "MOVS   R0, #4  \n"
                  "MOV    R1, LR  \n"
                  "TST    R0, R1  \n"
                  "BEQ    _MSP    \n"
                  "MRS    R0, PSP \n"
                  "B      HardFault_HandlerC      \n"
          "_MSP:  \n"
                  "MRS    R0, MSP \n"
                  "B      HardFault_HandlerC      \n"
          ".syntax divided\n") ;
}

/*************************************************************************************************/
/*!
 *  \brief      Count stack usage.
 *
 *  \param      None.
 *
 *  \return     Number of bytes used by the stack.
 */
/*************************************************************************************************/
uint32_t SystemCountStackUsage(void)
{
  unsigned long *pUnused = &_sstack;

  while (pUnused < &_estack)
  {
    if (*pUnused != INIT_STACK_VAL)
    {
      break;
    }

    pUnused++;
  }

  return (uint32_t)(&_estack - pUnused) * sizeof(*pUnused);
}
