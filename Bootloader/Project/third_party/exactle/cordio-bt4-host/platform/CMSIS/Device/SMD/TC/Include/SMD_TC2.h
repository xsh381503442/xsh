#ifndef __SMD_TC2_H
#define __SMD_TC2_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CONFIG_HOST_REV
#define CONFIG_HOST_REV 0x20
#endif

/* ---------------------  Interrupt Number Definition  -------------------- */

typedef enum IRQn
{
/* ---------------  Cortex-M0 Processor Exceptions Numbers  --------------- */
  NonMaskableInt_IRQn           = -14,      /*!<  2 Non Maskable Interrupt  */
  HardFault_IRQn                = -13,      /*!<  3 HardFault Interrupt     */



  SVCall_IRQn                   =  -5,      /*!< 11 SV Call Interrupt       */

  PendSV_IRQn                   =  -2,      /*!< 14 Pend SV Interrupt       */
  SysTick_IRQn                  =  -1,      /*!< 15 System Tick Interrupt   */

/* ------------------  SMD TC1 Specific Interrupt Numbers  ---------------- */

  TIMER_32K_IRQn                =   0,      /* 32K sleep/wakeup timer */
  UART0_IRQn                    =   1,      /* UART 0 */
  GPIO_2_IRQn                   =   2,      /* GPIO_0 */
  GPIO_3_IRQn                   =   3,      /* GPIO_1 */
  SPI0_IRQn                     =   6,      /* SPI 0 */
  TIMER_DUAL_IRQn               =   7,      /* Dual Timer 0 */
  CLOCK_STATUS_IRQn             =   8,      /* Source change or calibration */
  WDT_IRQn                      =   9,      /* Watchdog */
  UART1_IRQn                    =  10,      /* UART 1 */
  TWI0_IRQn                     =  11,      /* TWI 0 */
  GPIO_10_IRQn                  =  12,      /* GPIO_2 */
  GPIO_11_IRQn                  =  13,      /* GPIO_3 */
  GPIO_12_IRQn                  =  14,      /* GPIO_4 */
  GPIO_13_IRQn                  =  15,      /* GPIO_5 */
  GPIO_COMBINED_IRQn            =  16,      /* Combined GPIO */
  LLCC_TXEVT_EMPTY_IRQn         =  24,      /* LLCC TXEVT EMPTY IRQ Handler */
  LLCC_TXCMD_EMPTY_IRQn         =  25,      /* LLCC TXCMD EMPTY IRQ Handler */
  LLCC_RXEVT_VALID_IRQn         =  26,      /* LLCC RXEVT VALID IRQ Handler */
  LLCC_RXCMD_VALID_IRQn         =  27,      /* LLCC RXCMD VALID IRQ Handler */
  LLCC_TXDMAL_DONE_IRQn         =  28,      /* LLCC TXDMAL DONE IRQ Handler */
  LLCC_RXDMAL_DONE_IRQn         =  29,      /* LLCC RXDMAL DONE IRQ Handler */
  LLCC_TXDMAH_DONE_IRQn         =  30,      /* LLCC TXDMAH DONE IRQ Handler */
  LLCC_RXDMAH_DONE_IRQn         =  31       /* LLCC RXDMAH DONE IRQ Handler */
} IRQn_Type;

/* ======================================================================== */
/* ============      Processor and Core Peripheral Section     ============ */
/* ======================================================================== */

/* ----  Configuration of the Cortex-M0+ Processor and Core Peripherals  --- */
#define __CM0PLUS_REV             0x0001    /*!< Core revision r0p1         */
#define __MPU_PRESENT             0         /*!< MPU present or not         */
#define __VTOR_PRESENT            1         /*!< VTOR present or not        */
#define __NVIC_PRIO_BITS          2         /*!< Number of Bits used for Priority Levels */
#define __Vendor_SysTickConfig    0         /*!< Set to 1 if different SysTick Config is used */

#include <core_cm0plus.h>                 /* Processor and core peripherals */
#include "system_smd_tc.h"                /* System Header                  */


/* ======================================================================== */
/* ============       Device Specific Peripheral Section       ============ */
/* ======================================================================== */

/* ---------------  Start of section using anonymous unions  -------------- */
#if defined(__CC_ARM)
  #pragma push
  #pragma anon_unions
#elif defined(__ICCARM__)
  #pragma language=extended
#elif defined(__GNUC__)
  /* anonymous unions are enabled by default */
#elif defined(__TMS470__)
/* anonymous unions are enabled by default */
#elif defined(__TASKING__)
  #pragma warning 586
#else
  #warning Not supported compiler type
#endif

/* ======================================================================== */
/* ============               LLCC/DMAC v1                     ============ */
/* ======================================================================== */

typedef struct
{
    __IO uint32_t BUF_STATE;  // +0x00
    __I  uint32_t STATUS;     // +0x00
    __IO uint32_t PTR_ADDR;   // +0x08
    __IO uint32_t PTR_CTRL;   // +0x0c
    __O  uint32_t NXT_ADDR;   // +0x10
    __O  uint32_t NXT_CTRL;   // +0x14
    __I  uint32_t rsvd_18[2]; // +0x18
    __IO uint32_t BUF0_ADDR;  // +0x20
    __IO uint32_t BUF0_CTRL;  // +0x24
    __I  uint32_t rsvd_28[2]; // +0x28
    __IO uint32_t BUF1_ADDR;  // +0x30
    __IO uint32_t BUF1_CTRL;  // +0x34
    __IO uint32_t INTEN;      // +0x38
    __IO uint32_t IRQSTATUS;  // +0x3c
} DMAC_CHAN_TypeDef;

// DMA buffer control state machine
#define     DMAC_BUFSTATE_MT	0
#define     DMAC_BUFSTATE_A	1
#define     DMAC_BUFSTATE_AB	5
#define     DMAC_BUFSTATE_B	2
#define     DMAC_BUFSTATE_BA	6
#define     DMAC_BUFSTATE_FULL_IDX	2

// DMA Control structure MASKs
#define     DMAC_CHAN_ADDR_MASK 	0xfffffffc
#define     DMAC_CHAN_COUNT_MASK 	0x0000ffff
#define     DMAC_CHAN_SIZE_MASK 	0x00030000
#define     DMAC_CHAN_AFIX_MASK 	0x00040000
#define     DMAC_CHAN_LOOP_MASK 	0x00080000
#define     DMAC_CHAN_ATTR_MASK 	0xfff00000
#define     DMAC_CHAN_COUNT_IDX_LO      0
#define     DMAC_CHAN_COUNT_IDX_HI      15
#define     DMAC_CHAN_SIZE_IDX_LO       16
#define     DMAC_CHAN_SIZE_IDX_HI       17
#define     DMAC_CHAN_AFIX_IDX          18
#define     DMAC_CHAN_LOOP_IDX          19
#define     DMAC_CHAN_TRIG_IDX_LO       20
#define     DMAC_CHAN_TRIG_IDX_HI       23
#define     DMAC_CHAN_ATTR_IDX_LO       24
#define     DMAC_CHAN_ATTR_IDX_HI       31
#define     DMAC_CHAN_IRQ_IDX           0
#define     DMAC_CHAN_ERR_IDX           1

typedef struct
{
    __I  uint32_t ID_MAIN;	     // +0x0000
    __I  uint32_t ID_REV;	     // +0x0004
    __I  uint32_t rsvd_0008[30];     // +0x0008
    __IO uint32_t STANDBY_CTRL;      // +0x0080
} LLCC_CTL_TypeDef;

typedef struct
{
    __I  uint32_t CMD_DATA0;        // +0x2000
    __I  uint32_t CMD_DATA1;        // +0x2004
    __I  uint32_t rsvd_008[14];     // +0x2008
    __I  uint32_t DMAH_DATA0;       // +0x2040
    __I  uint32_t DMAH_DATA1;       // +0x2044
    __I  uint32_t rsvd_048[6];      // +0x2048
    __I  uint32_t DMAL_DATA0;       // +0x2060
    __I  uint32_t DMAL_DATA1;       // +0x2064
    __I  uint32_t rsvd_068[6];      // +0x2068
    __I  uint32_t EVT_DATA0;        // +0x2080
    __I  uint32_t EVT_DATA1;        // +0x2084
    __I  uint32_t rsvd_088[14];     // +0x2088
    __I  uint32_t INTERRUPT;        // +0x20c0
    __IO uint32_t INTENMASK;        // +0x20c4
    __IO uint32_t INTENMASK_SET;    // +0x20c8
    __IO uint32_t INTENMASK_CLR;    // +0x20cc
    __I  uint32_t REQUEST;	    // +0x20d0
    __I  uint32_t rsvd_0d4[3];      // +0x20d4
    __I  uint32_t XFERREQ;	    // +0x20e0
    __I  uint32_t XFERACK;	    // +0x20e4
    __I  uint32_t rsvd_0e8[6];      // +0x20e8
} LLCC_RXD_TypeDef;

typedef struct
{
    __IO uint32_t CMD_DATA0;        // +0x3000
    __IO uint32_t CMD_DATA1;        // +0x3004
    __I  uint32_t rsvd_008[14];     // +0x3008
    __IO uint32_t DMAH_DATA0;       // +0x3040
    __IO uint32_t DMAH_DATA1;       // +0x3044
    __I  uint32_t rsvd_048[6];      // +0x3048
    __IO uint32_t DMAL_DATA0;       // +0x3060
    __IO uint32_t DMAL_DATA1;       // +0x3064
    __I  uint32_t rsvd_068[6];      // +0x3068
    __IO uint32_t EVT_DATA0;        // +0x3080
    __IO uint32_t EVT_DATA1;        // +0x3084
    __I  uint32_t rsvd_088[14];     // +0x3088
    __I  uint32_t INTERRUPT;        // +0x30c0
    __IO uint32_t INTENMASK;        // +0x30c4
    __IO uint32_t INTENMASK_SET;    // +0x30c8
    __IO uint32_t INTENMASK_CLR;    // +0x30cc
    __I  uint32_t REQUEST;	    // +0x30d0
    __I  uint32_t ACTIVE;	    // +0x30d4
    __I  uint32_t VCREADY;	    // +0x30d8
    __I  uint32_t rsvd_0dc;         // +0x30dc
    __IO uint32_t XFERREQ;	    // +0x30e0
    __I  uint32_t XFERACK;	    // +0x30e4
    __I  uint32_t rsvd_0e8[6];      // +0x30e8
} LLCC_TXD_TypeDef;

// TX?RX buffer handshake/interrupt fields
#define     LLCC_CMD0_MASK	   0x01
#define     LLCC_CMD1_MASK	   0x02
#define     LLCC_CMD_MASK	   0x03
#define     LLCC_CMD_IRQ_MASK	   LLCC_CMD_MASK
#define     LLCC_DMAH1_MASK	   0x04
#define     LLCC_DMAH2_MASK	   0x08
#define     LLCC_DMAH_MASK	   0x0c
#define     LLCC_DMAL1_MASK	   0x10
#define     LLCC_DMAL2_MASK	   0x20
#define     LLCC_DMAL_MASK	   0x30
#define     LLCC_EVT0_MASK	   0x40
#define     LLCC_EVT1_MASK	   0x80
#define     LLCC_EVT_IRQ_MASK	   LLCC_EVT1_MASK
#define     LLCC_EVT_MASK	   0xc0
#define     LLCC_CMD0_IDX	   0
#define     LLCC_CMD1_IDX	   1
#define     LLCC_CMD_IDX           LLCC_CMD1_IDX
#define     LLCC_CMD_IRQ_IDX	   1
#define     LLCC_DMAH1_IDX	   2
#define     LLCC_DMAH2_IDX	   3
#define     LLCC_DMAL1_IDX	   4
#define     LLCC_DMAL2_IDX	   5
#define     LLCC_EVT0_IDX	   6
#define     LLCC_EVT1_IDX	   7
#define     LLCC_EVT_IDX	   LLCC_EVT1_IDX

/* ======================================================================== */
/* ============              IO MUX (IOM)                  ============ */
/* ======================================================================== */
typedef struct
{
  __IO uint32_t MODESEL;       /* Offset: 0x000 (R/W)  Mode Select */
} SMD_IOM_TypeDef;

#define IOM_MODE_SMDEVAL      0
#define IOM_MODE_EVAL         1
#define IOM_MODE_SMDCHIPLET   2
#define IOM_MODE_CHIPLET      3
#define IOM_MODE_DIGITAL_TEST 4

/* ======================================================================== */
/* ============              Dual Timer (TIM)                  ============ */
/* ======================================================================== */
typedef struct
{
  __IO uint32_t Timer1Load;       /* Offset: 0x000 (R/W)  Timer 1 Load */
  __I  uint32_t Timer1Value;      /* Offset: 0x004 (R/ )  Timer 1 Counter Current Value */
  __IO uint32_t Timer1Control;    /* Offset: 0x008 (R/W)  Timer 1 Control */
  __O  uint32_t Timer1IntClr;     /* Offset: 0x00C ( /W)  Timer 1 Interrupt Clear */
  __I  uint32_t Timer1RIS;        /* Offset: 0x010 (R/ )  Timer 1 Raw Interrupt Status */
  __I  uint32_t Timer1MIS;        /* Offset: 0x014 (R/ )  Timer 1 Masked Interrupt Status */
  __IO uint32_t Timer1BGLoad;     /* Offset: 0x018 (R/W)  Background Load Register */
       uint32_t RESERVED0[1];
  __IO uint32_t Timer2Load;       /* Offset: 0x020 (R/W)  Timer 2 Load */
  __I  uint32_t Timer2Value;      /* Offset: 0x024 (R/ )  Timer 2 Counter Current Value */
  __IO uint32_t Timer2Control;    /* Offset: 0x028 (R/W)  Timer 2 Control */
  __O  uint32_t Timer2IntClr;     /* Offset: 0x02C ( /W)  Timer 2 Interrupt Clear */
  __I  uint32_t Timer2RIS;        /* Offset: 0x030 (R/ )  Timer 2 Raw Interrupt Status */
  __I  uint32_t Timer2MIS;        /* Offset: 0x034 (R/ )  Timer 2 Masked Interrupt Status */
  __IO uint32_t Timer2BGLoad;     /* Offset: 0x038 (R/W)  Background Load Register */
       uint32_t RESERVED1[945];
  __IO uint32_t ITCR;         // <h> Integration Test Control Register </h>
  __O  uint32_t ITOP;         // <h> Integration Test Output Set Register </h>
} ARM_TIM_DUAL_TypeDef;

#define TIMER_CONTROL_ONESHOT          (1 << 0)
#define TIMER_CONTROL_32BIT            (1 << 1)
#define TIMER_CONTROL_PRESCALER_1      (0 << 2)
#define TIMER_CONTROL_PRESCALER_4      (1 << 2)
#define TIMER_CONTROL_PRESCALER_8      (2 << 2)
#define TIMER_CONTROL_PRESCALER_3      (3 << 2)
#define TIMER_CONTROL_INTERRUPT_ENABLE (1 << 5)
#define TIMER_CONTROL_MODE_PERIODIC    (1 << 6)
#define TIMER_CONTROL_ENABLE           (1 << 7)

#define TIMER_INTERRUPT                (1 << 0)

/* ======================================================================== */
/* ========== Universal Asyncronous Receiver / Transmitter (UART) ========= */
/* ======================================================================== */
typedef struct
{
  __IO uint32_t DATA;             /* Offset: 0x000 (R/W)  Data */
  __IO uint32_t STATE;            /* Offset: 0x004 (R/W)  State */
  __IO uint32_t CTRL;             /* Offset: 0x008 (R/W)  Control */
  union {
    __I    uint32_t  INTSTATUS;   /*!< Offset: 0x00C Interrupt Status Register (R/ ) */
    __O    uint32_t  INTCLEAR;    /*!< Offset: 0x00C Interrupt Clear Register ( /W) */
    };
  __IO uint32_t BAUDDIV;          /* Offset: 0x010 (R/W)  Baud rate divisor */
} ARM_UART_TypeDef;

#define UART_DATA_MASK     (0xff)

#define UART_STATE_TXFULL  (1 << 0)
#define UART_STATE_RXFULL  (1 << 1)
#define UART_STATE_TX_ORN  (1 << 2)  /* Write to clear */
#define UART_STATE_RX_ORN  (1 << 3)  /* Write to clear */

#define UART_CTRL_TX_EN    (1 << 0)
#define UART_CTRL_RX_EN    (1 << 1)
#define UART_CTRL_TXI_EN   (1 << 2)
#define UART_CTRL_RXI_EN   (1 << 3)
#define UART_CTRL_TXOI_EN  (1 << 4)
#define UART_CTRL_RXOI_EN  (1 << 5)
#define UART_CTRL_TESTMODE (1 << 6) /* DO NOT USE OUTSIDE OF SIMULATION */

#define UART_INT_TX        (1 << 0)  /* Write to clear */
#define UART_INT_RX        (1 << 1)  /* Write to clear */
#define UART_INT_TXORN     (1 << 2)  /* Write to clear */
#define UART_INT_RXORN     (1 << 3)  /* Write to clear */

#define UART_BAUDDIV_MASK  (0x000fffff)  /* Min 16, max 1048575 */

/* ======================================================================== */
/* ============                   GPIO                         ============ */
/* ======================================================================== */
typedef struct
{
  __IO uint32_t DATA;             /* Offset: 0x000 (R/W)  Data input */
  __IO uint32_t DATAOUT;          /* Offset: 0x004 (R/W)  Data output */
       uint32_t RESERVED0[2];
  __IO uint32_t OUTENSET;         /* Offset: 0x010 (R/W)  Output Enable Set */
  __IO uint32_t OUTENCLR;         /* Offset: 0x014 (R/W)  Output Enable Clear */
  __IO uint32_t ALTFUNCSET;       /* Offset: 0x018 (R/W)  Alternate Function Set */
  __IO uint32_t ALTFUNCCLR;       /* Offset: 0x01C (R/W)  Alternate Function Clear */
  __IO uint32_t INTENSET;         /* Offset: 0x020 (R/W)  Interrupt Enable Set */
  __IO uint32_t INTENCLR;         /* Offset: 0x024 (R/W)  Interrupt Enable Clear */
  __IO uint32_t INTTYPESET;       /* Offset: 0x028 (R/W)  Interrupt Type Set */
  __IO uint32_t INTTYPECLR;       /* Offset: 0x02C (R/W)  Interrupt Type Clear */
  __IO uint32_t INTPOLSET;        /* Offset: 0x030 (R/W)  Interrupt Polarity Set */
  __IO uint32_t INTPOLCLR;        /* Offset: 0x034 (R/W)  Interrupt Polarity Clear */
  union {
    __I    uint32_t  INTSTATUS;      /*!< Offset: 0x038 Interrupt Status Register (R/ ) */
    __O    uint32_t  INTCLEAR;       /*!< Offset: 0x038 Interrupt Clear Register ( /W) */
    };
       uint32_t RESERVED1[241];
  __IO uint32_t LB_MASKED[256];    /*!< Offset: 0x400 - 0x7FC Lower byte Masked Access Register (R/W) */
  __IO uint32_t UB_MASKED[256];    /*!< Offset: 0x800 - 0xBFC Upper byte Masked Access Register (R/W) */
} ARM_GPIO_TypeDef;

#define GPIO_INTPOL_LOW_FALL  0
#define GPIO_INTPOL_HIGH_RISE 1
#define GPIO_INTTYPE_LEVEL    0
#define GPIO_INTTYPE_EDGE     1

#define GPIO0_AF_UART0_RX     (1 << 0)
#define GPIO0_AF_UART0_TX     (1 << 1)
#define GPIO0_AF_UART1_RX     (1 << 15)
#define GPIO0_AF_UART1_TX     (1 << 16)
#define GPIOA_AF_TWI          0x000C

#define GPIO_BOOT             (1 << 8)
#define GPIO_ONETWO           (1 << 11)

#define GPIO_UART0_RXD        (1 << 0)
#define GPIO_UART0_TXD        (1 << 1)
#define GPIO_TWI_SCL          (1 << 2)
#define GPIO_TWI_SDA          (1 << 3)
#define GPIO_SSP_RXD          (1 << 4)
#define GPIO_SSP_TXD          (1 << 5)
#define GPIO_SSP_CLK          (1 << 6)
#define GPIO_SSP_FSS          (1 << 7)
#define GPIO_GPIO1            (1 << 8)
#define GPIO_GPIO2            (1 << 9)
#define GPIO_GPIO3            (1 << 10)
#define GPIO_GPIO4            (1 << 11)
#define GPIO_GPIO5            (1 << 12)
#define GPIO_GPIO6            (1 << 13)
#define GPIO_GPIO7            (1 << 14)
#define GPIO_UART1_RXD        (1 << 15)
#define GPIO_UART1_TXD        (1 << 16)

/* ======================================================================== */
/* ==========                  Watchdog                           ========= */
/* ======================================================================== */
typedef struct
{
  __IO uint32_t LOAD;             /* Offset: 0x000 (R/W)  Load Register */
  __I  uint32_t VALUE;            /* Offset: 0x004 (R/ )  Current Value */
  __IO uint32_t CONTROL;          /* Offset: 0x008 (R/W)  Control */
  __O  uint32_t INTCLR;           /* Offset: 0x00C ( /W)  Interrupt Clear */
  __I  uint32_t RIS;              /* Offset: 0x010 (R/ )  Raw Interrupt Status */
  __I  uint32_t MIS;              /* Offset: 0x014 (R/ )  Interrupt Status */
       uint32_t RESERVED0[762];
  __IO uint32_t LOCK;             /* Offset: 0xC00 (R/W)  Lock */
       uint32_t  RESERVED1[191];
  __IO uint32_t  ITCR;          // <h> Watchdog Integration Test Control Register </h>
  __O  uint32_t  ITOP;          // <h> Watchdog Integration Test Output Set Register </h>
} ARM_WATCHDOG_TypeDef;

#define WDG_LOAD        (1 << 0)
#define WDG_VALUE       (1 << 0)
#define WDG_CTRL_RESEN  (1 << 1)
#define WDG_CTRL_INTEN  (1 << 0)
#define WDG_CTRL_INTCLR (1 << 0)
#define WDG_RIS_IRQ     (1 << 0)
#define WDG_MIS_IRQ     (1 << 0)
#define WDG_LOCK_DIS    (0x1ACCE551)
#define WDG_LOCK_WRDIS  (1 << 0)

/* =================================================== */
/* ==========  Serial Stream Processor (SSP) ========= */
/* =================================================== */
typedef struct
{
  __IO uint32_t CR0;             /* Offset: 0x00 (R/W)  Control 1 */
  __IO uint32_t CR1;             /* Offset: 0x04 (R/W)  Control 2 */
  __IO uint32_t DR;              /* Offset: 0x08 (R/W)  Data */
  __I  uint32_t SR;              /* Offset: 0x0C (R/ )  Status */
  __IO uint32_t CPSR;            /* Offset: 0x10 (R/W)  Clock Prescaler Status */
  __IO uint32_t IMSC;            /* Offset: 0x14 (R/W)  Interrupt Mask Set Clear */
  __IO uint32_t RIS;             /* Offset: 0x18 (R/W)  Raw Interrupt Status */
  __IO uint32_t MIS;             /* Offset: 0x1C (R/W)  Masked Interrupt Status */
  __O  uint32_t ICR;             /* Offset: 0x20 ( /W)  Interrupt Clear */
  __IO uint32_t DMACR;           /* Offset: 0x24 (R/W)  DMA Control */
       uint32_t RESERVED0[22];
  __IO uint32_t TCR;            /* Offset: 0x80 (R/W)  Test Control */
  __IO uint32_t ITIP;           /* Offset: 0x84 (R/W)  Test Control */
  __IO uint32_t ITOP;           /* Offset: 0x88 (R/W)  Test Control */
  __IO uint32_t TDR;            /* Offset: 0x8C (R/W)  Test Control */
       uint32_t RESERVED1[980];
  __I  uint32_t PERIPH_ID0;     /* Offset: 0xFE0 (R/ ) Peripheral ID 0 */
  __I  uint32_t PERIPH_ID1;     /* Offset: 0xFE4 (R/ ) Peripheral ID 1 */
  __I  uint32_t PERIPH_ID2;     /* Offset: 0xFE8 (R/ ) Peripheral ID 2 */
  __I  uint32_t PERIPH_ID3;     /* Offset: 0xFEC (R/ ) Peripheral ID 3 */
  __I  uint32_t PCELL_ID0;      /* Offset: 0xFF0 (R/ ) PrimeCell ID 0 */
  __I  uint32_t PCELL_ID1;      /* Offset: 0xFF4 (R/ ) PrimeCell ID 1 */
  __I  uint32_t PCELL_ID2;      /* Offset: 0xFF8 (R/ ) PrimeCell ID 2 */
  __I  uint32_t PCELL_ID3;      /* Offset: 0xFFC (R/ ) PrimeCell ID 3 */
} ARM_SSP_TypeDef;

#define SSP_CR0_SCR_MASK         0xff00 /* RATE = SSPCLK/(CPSDVR * (1+SCR) */
#define SSP_CR0_SCR_SELECT(__n)  ((((__n) - 1) & 0xff) << 8)  /* __n == 1-256 */
#define SSP_CR0_SPH              (1<<7) /* CLK Phase for Moto format */
#define SSP_CR0_SPO              (1<<6) /* CLK Polarity for Moto format */
#define SSP_CR0_FRF_Moto         (0x00)
#define SSP_CR0_FRF_TI           (0x10)
#define SSP_CR0_FRF_National     (0x20)
#define SSP_CR0_DSS_SELECT(__n)  (((__n) - 1) & 0xf)  /* __n == 4-16 */

#define SSP_CR1_SOD              (1<<3)
#define SSP_CR1_MS               (1<<2)  /* 1 == slave */
#define SSP_CR1_SSE              (1<<1)
#define SSP_CR1_LOOP             (1<<0)

#define SSP_SR_BSY               (1<<4) /* BUSY */
#define SSP_SR_RFF               (1<<3) /* RX FIFO Full */
#define SSP_SR_RNE               (1<<2) /* RX FIFO Not Empty */
#define SSP_SR_TNF               (1<<1) /* TX FIFO Full */
#define SSP_SR_TFE               (1<<0) /* TX FIFO Empty */

#define SSP_CSPR_CVPSDVR_MASK    (0x00fe)
#define SSP_CSPR_CVPSDVR(__n)    ((__n) & SSP_CSPR_CVPSDVR_MASK)

#define SSP_INT_TX               (1<<3)
#define SSP_INT_RX               (1<<2)
#define SSP_INT_RT               (1<<1)
#define SSP_INT_ROR              (1<<0)

#define SSP_DMA_TXE              (1<<1)
#define SSP_DMA_RXE              (1<<0)

/* =================================================== */
/* ============  Two-Wire Interface (TWI) ============ */
/* =================================================== */
typedef struct
{
  __IO uint32_t CONTROL;            /* Offset: 0x00 (R/W)  Control */
  __IO uint32_t CLK_CONFIG;         /* Offset: 0x04 (R/W)  Clock configuration */
  __IO uint32_t IRQ_MASK;           /* Offset: 0x08 (R/W)  IRQ mask pattern */
  __IO uint32_t OUTPUT_ADDR;        /* Offset: 0x0C (R/W)  Transmit address */
  __IO uint32_t OUTPUT_DATA;        /* Offset: 0x10 (R/W)  Transmit data */
  __I  uint32_t RECEIVE_DATA;       /* Offset: 0x14 (R/ )  Receive data */
  __I  uint32_t IRQ_STATUS;         /* Offset: 0x18 (R/ )  IRQ status */
  __I  uint32_t STATE;              /* Offset: 0x1C (R/ )  System state */
} SMD_TWI_TypeDef;

#define TWI_CONTROL_CLEAR       0x00
#define TWI_CONTROL_RESET       (1<<0)
#define TWI_CONTROL_MORE_WRITES (1<<1)
#define TWI_CONTROL_READ_START  (1<<2)
#define TWI_CONTROL_RESTART     (1<<3)
#define TWI_CONTROL_STOP        (1<<4)
#define TWI_CONTROL_ACK         (1<<5)
#define TWI_CONTROL_NACK        (1<<6)

#define TWI_CLK_MODE            (1<<0)
#define TWI_CLK_DIV_MASK        (0xff<<1)

#define TWI_CLK_CONFIG_100KHZ   40
#define TWI_CLK_CONFIG_400KHZ   11

#define TWI_IRQ_ADDR_ACK        0x01
#define TWI_IRQ_ADDR_NACK       0x02
#define TWI_IRQ_DATA_ACK        0x03
#define TWI_IRQ_DATA_NACK       0x04
#define TWI_IRQ_RCV             0x05

#define TWI_IRQ_MASK_ADDR_XMIT  (1 << 0)
#define TWI_IRQ_MASK_DATA_XMIT  (1 << 1)
#define TWI_IRQ_MASK_RCV        (1 << 2)
#define TWI_IRQ_MASK_ENABLE_ALL	0x07

#define TWI_TX_ADDR_MASK	    (0xff)
#define TWI_TX_DATA_MASK	    (0xff)
#define TWI_RX_DATA_MASK	    (0xff)
#define TWI_IRQ_STATUS_MASK	    (0xf)
#define TWI_STATE_MASK	            (0x1f)

#define TWI_ADDR_READ            0x01
#define TWI_ADDR_WRITE           0x00
#define TWI_ADDR(__addr, __dir)  ((__addr << 1) | __dir)

/* ======================================================================== */
/* ==========             CMSDK System Controller                 ========= */
/* ======================================================================== */
typedef struct
{
  __IO uint32_t REMAP;   /* Offset: 0x000 (R/W) Remap Feature */
  __IO uint32_t PMUCTRL; /* Offset: 0x004 (R/W) Power Management Unit */
  __IO uint32_t RESETOP; /* Offset: 0x008 (R/W) Reset if Lockup */
  __IO uint32_t RESERVED;
  __IO uint32_t RSTINFO; /* Offset: 0x010 (R/W) Reset Info */
} ARM_SysCon_TypeDef;

#define SYSCON_REMAP_ENABLE    (1<<0)  /* Enable Remapping */
#define SYSCON_PMUCTRL_ENABLE  (1<<0)  /* Enable PMU */
#define SYSCON_RESETOP_ENABLE  (1<<0)  /* Reboot automatically */
#define SYSCON_RSTINFO_SYSRESETREQ (1<<0) /* SYSRESETREQ triggered */
#define SYSCON_RSTINFO_WATCHDOG (1<<1)    /* Watchdog triggered */
#define SYSCON_RSTINFO_LOCKUP (1<<2)      /* Lockup triggered */

/* ======================================================================== */
/* ==========             SMD TC1 Power Controller                ========= */
/* ======================================================================== */

typedef struct {
	__I  uint32_t RESET_SYNDROME;     /* 0000 */
	__IO uint32_t EMA;                /* 0004 */
	__IO uint32_t LLCCTRL;            /* 0008 */
	__IO uint32_t LLCSTATUS;          /* 000C */
	__IO uint32_t TIMER_32K_CONTROL;  /* 0010 */
	__IO uint32_t TIMER_32K_LOW16;    /* 0014 */
	__IO uint32_t TIMER_32K_HIGH16;   /* 0018 */
	__IO uint32_t TIMER_32K_COUNT;    /* 001C */
	__IO uint32_t TIMER_TIMEOUT_STATUS; /* 0020 */
	__IO uint32_t IRQ_ENABLE;   /* 0024 */
	__IO uint32_t IRQ_STATUS;   /* 0028 */
	     uint32_t RESERVEDB[1];
	__IO uint32_t WAKE_NEXT_CFG;    /* 0030 */
	__IO uint32_t WAKE_NEXT_CLK;    /* 0034 */
	__IO uint32_t SLEEP_NEXT_CFG;   /* 0038 */
	__IO uint32_t SLEEP_NEXT_CLK;   /* 003C */
	__IO uint32_t DYNAMIC_CLK;      /* 0040 */
	     uint32_t RESERVEDC[3];     /* 44, 48, 4C */
	__IO uint32_t TURBO;            /* 0050 */
} SMD_TC_POWERCON_TypeDef;

#define POWERCON_LLCCTRL_RESET                       (1<<0)
#define POWERCON_LLCCTRL_BATTERY_STATUS_REQUEST      (1<<1)
#define POWERCON_LLCCTRL_SWITCHING_REGULATOR_REQUEST_NOOP (1<<2)
#define POWERCON_LLCCTRL_32M_XTAL_REQUEST            (1<<4)

#define POWERCON_RESET_SYNDROME_V3V_POWERON  (1<<0)
#define POWERCON_RESET_SYNDROME_V1V_BROWNOUT (1<<1)
#define POWERCON_RESET_SYNDROME_EXTERNAL     (1<<2)
#define POWERCON_RESET_SYNDROME_WDOG         (1<<3)

#define POWERCON_EMA_RAM_8KX32_EMA_MASK      0x007  /* Bits 2:0 */
#define POWERCON_EMA_RAM_8KX32_EMAW_MASK     0x018  /* Bits 4:3 */
#define POWERCON_EMA_ROM_2KX32_EMA_MASK      0x0e0  /* Bits 7:5 */

#define POWERCON_TIMER_32K_CONTROL_ENABLE    (1<<0)
#define POWERCON_TIMER_32K_CONTROL_TO_ENABLE (1<<1)
#define POWERCON_TIMER_32K_CONTROL_XTAL_REQUEST (1<<2)
#define POWERCON_TIMER_32K_CONTROL_RCHAN_WAKEUP_EN (1<<3)
#define POWERCON_TIMER_32K_CONTROL_GPIO_WAKEUP_EN (1<<4)
#define POWERCON_TIMER_32K_CONTROL_GPIO_WAKEUP_POLARITY (1<<5)
#define POWERCON_TIMER_32K_CONTROL_SWITCHING_REGULATOR_REQUEST (1<<6)

#define POWERCON_TIMER_TIMEOUT_STATUS_TIMER   (1<<0)
#define POWERCON_TIMER_TIMEOUT_STATUS_GPIO    (1<<1)

#define POWERCON_CORDIO_STATUS_BATTERY_DET3V  (1<<0)
#define POWERCON_CORDIO_STATUS_BATTERY_STATUS (1<<1)
#define POWERCON_CORDIO_STATUS_V1V_STATUS     (1<<2)
#define POWERCON_CORDIO_STATUS_ACTIVE_32KXTAL (1<<3)
#define POWERCON_CORDIO_STATUS_ACTIVE_32MXTAL (1<<4)

#define POWERCON_CFG_CPU_MASK   (3<<0)  // Bits 1:0   00: off 01: retention 10: clkgate  11: run
#define POWERCON_CFG_RAM1_MASK  (3<<2)  // Bits 3:2   00: off 01: retention 10: clkgate 11: run
#define POWERCON_CFG_DEEP_MASK  (1<<4)  // Bit 4      0: clkgate, 1: pwrgate

/* Sleep, Wake, and Dynamic Clocks */
#define POWERCON_CLK_ESS_FCLK (1<<0) // SLEEP_NEXT, DYNAMIC_NEXT only
#define POWERCON_CLK_CPU_SCLK (1<<1) // SLEEP_NEXT, DYNAMIC_NEXT only
#define POWERCON_CLK_CPU   (1<<2)   // Not Dynamic
#define POWERCON_CLK_BUS   (1<<3)   // Not Dynamic
#define POWERCON_CLK_APB_P (1<<4)
#define POWERCON_CLK_ROM   (1<<6)
#define POWERCON_CLK_RAM1  (1<<7)
#define POWERCON_CLK_DMA   (1<<8)

/* Turbo Control */
#define POWERCON_CLK_TURBO (1<<0)

#define POWERCON_IRQ_V1V            (1<<1)
#define POWERCON_IRQ_BATT           (1<<2)
#define POWERCON_IRQ_32K_XTAL_USE   (1<<3)
#define POWERCON_IRQ_32M_XTAL_USE   (1<<4)

/* ----------------  End of section using anonymous unions  --------------- */
#if defined(__CC_ARM)
  #pragma pop
#elif defined(__ICCARM__)
  /* leave anonymous unions enabled */
#elif defined(__GNUC__)
  /* anonymous unions are enabled by default */
#elif defined(__TMS470__)
  /* anonymous unions are enabled by default */
#elif defined(__TASKING__)
  #pragma warning restore
#else
  #warning Not supported compiler type
#endif

/* ======================================================================== */
/* ============              Peripheral memory map             ============ */
/* ======================================================================== */

/* --------------------------  CPU memory map  ---------------------------- */
#define ROM_BASE             (0x00000000UL)
#define SRAM1_BASE           (0x20000000UL)

#define APB_BASE             (0x40000000UL)
#define AHB_BASE             (0x40010000UL)
#define SMD_TC_POWERCON_BASE (0xA1000000UL)
#define IOM_BASE             (0xB0000000UL)
#define ESS_BLE_BASE         (0xC0000000UL)

#define LLCC_CONT_BASE       (0xD0000000UL)
#define LLCC_CTRL_BASE       (LLCC_CONT_BASE)
#define LLCC_RXD_BASE        (LLCC_CONT_BASE+0x2000)
#define LLCC_TXD_BASE        (LLCC_CONT_BASE+0x3000)

#define DMAC_CONT_BASE       (0xD0001000UL)
#define DMAC_DMARH_BASE      (DMAC_CONT_BASE+0x00)
#define DMAC_DMARL_BASE      (DMAC_CONT_BASE+0x40)
#define DMAC_DMAWH_BASE      (DMAC_CONT_BASE+0x80)
#define DMAC_DMAWL_BASE      (DMAC_CONT_BASE+0xC0)
#define DMAC_HCIR_BASE       DMAC_DMARL_BASE
#define DMAC_HCIW_BASE       DMAC_DMAWL_BASE

  /* APB Peripherals */
#define ARM_TIM0_BASE        (APB_BASE + 0x02000)
#define ARM_TIM1_BASE        (ARM_TIM0_BASE)
#define ARM_TIM2_BASE        (ARM_TIM0_BASE + 0x20)
#define ARM_SSP_BASE         (APB_BASE + 0x03000)
#define ARM_UART0_BASE       (APB_BASE + 0x04000)
#define ARM_UART1_BASE       (APB_BASE + 0x05000)
#define ARM_WDOG_BASE        (APB_BASE + 0x08000)
#define SMD_TWI_BASE         (APB_BASE + 0x0D000)

  /* AHB peripherals */
#define GPIO0_BASE           (AHB_BASE + 0x00000)
#define ARM_SYSCON_BASE      (AHB_BASE + 0x0F000)

/* ======================================================================== */
/* ============             Peripheral declaration             ============ */
/* ======================================================================== */

#define IOM           ((SMD_IOM_TypeDef *)          IOM_BASE)

#define UART0         ((ARM_UART_TypeDef *)         ARM_UART0_BASE)
#define UART1         ((ARM_UART_TypeDef *)         ARM_UART1_BASE)

#define ARM_TIM0      ((ARM_TIM_DUAL_TypeDef *)     ARM_TIM0_BASE)
#define ARM_TIM1      ((ARM_TIM_SINGLE_TypeDef *)   ARM_TIM1_BASE)  /* single timer 1, part of dual tim0 */
#define ARM_TIM2      ((ARM_TIM_SINGLE_TypeDef *)   ARM_TIM2_BASE)  /* single timer 2, part of dual tim0 */
#define ARM_SSP0      ((ARM_SSP_TypeDef *)          ARM_SSP_BASE)
#define SMD_TWI0      ((SMD_TWI_TypeDef *)          SMD_TWI_BASE)
#define ARM_WDOG      ((ARM_WATCHDOG_TypeDef *)     ARM_WDOG_BASE)
#define GPIOA         ((ARM_GPIO_TypeDef *)         GPIO0_BASE)
#define ARM_SYSCON    ((ARM_SysCon_TypeDef *)       ARM_SYSCON_BASE)
#define POWERCON      ((SMD_TC_POWERCON_TypeDef *)  SMD_TC_POWERCON_BASE)

#define LLCC_CTL             ((LLCC_CTL_TypeDef *)  LLCC_CTRL_BASE)
#define LLCC_RXD             ((LLCC_RXD_TypeDef *)  LLCC_RXD_BASE)
#define LLCC_TXD             ((LLCC_TXD_TypeDef *)  LLCC_TXD_BASE)
#define DMAC_DMARH           ((DMAC_CHAN_TypeDef *) DMAC_DMARH_BASE)
#define DMAC_DMARL           ((DMAC_CHAN_TypeDef *) DMAC_DMARL_BASE)
#define DMAC_DMAWH           ((DMAC_CHAN_TypeDef *) DMAC_DMAWH_BASE)
#define DMAC_DMAWL           ((DMAC_CHAN_TypeDef *) DMAC_DMAWL_BASE)
#define DMAC_HCIR            DMAC_DMAWL
#define DMAC_HCIW            DMAC_DMARL

#ifdef __cplusplus
}
#endif

#endif  /* __SMD_TC1_H */


