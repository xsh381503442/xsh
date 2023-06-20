;***************************************************************************************************
;* startup-smd_tc2.s
;*
;* Startup code
;***************************************************************************************************

;***************************************************************************************************
;* STACK CONFIGURATION
;***************************************************************************************************

Stack_Size      EQU     0x00000400

                AREA    STACK, NOINIT, READWRITE, ALIGN=3
Stack_Mem       SPACE   Stack_Size
__initial_sp

;***************************************************************************************************
;* HEAP CONFIGURATION
;***************************************************************************************************

Heap_Size       EQU     0x00000000

                AREA    HEAP, NOINIT, READWRITE, ALIGN=3
__heap_base
Heap_Mem        SPACE   Heap_Size
__heap_limit

                PRESERVE8
                THUMB

;***************************************************************************************************
;* VECTOR TABLE
;***************************************************************************************************

                AREA    RESET, DATA, READONLY
                EXPORT  systemVectors

Sign_Value      EQU     0x5A5A5A5A

systemVectors   DCD     __initial_sp                            ;  0  Top of Stack
                DCD     Reset_Handler                           ;  1  Reset Handler
                DCD     NMI_Handler                             ;  2  NMI Handler
                DCD     HardFault_Handler                       ;  3  Hard Fault Handler
                DCD     0                                       ;  4  Reserved
                DCD     0                                       ;  5  Reserved
                DCD     0                                       ;  6  Reserved
                DCD     0                                       ;  7  Reserved
                DCD     0                                       ;  8  Reserved
                DCD     0                                       ;  9  Reserved
                DCD     0                                       ; 10  Reserved
                DCD     SVC_Handler                             ; 11  SVCall Handler
                DCD     0                                       ; 12  Debug Monitor Handler
                DCD     0                                       ; 13  Reserved
                DCD     PendSV_Handler                          ; 14  PendSV Handler
                DCD     SysTick_Handler                         ; 15  SysTick Handler

                ; External Interrupts
                DCD     TIMER_32K_Handler
                DCD     UART0_Handler
                DCD     GPIO_2_Handler
                DCD     GPIO_3_Handler
                DCD     0
                DCD     0
                DCD     SPI0_Handler
                DCD     TIMER_DUAL_Handler
                DCD     CLOCKSTATUS_Handler
                DCD     WDT_Handler
                DCD     UART1_Handler
                DCD     TWI0_Handler
                DCD     GPIO_10_Handler
                DCD     GPIO_11_Handler
                DCD     GPIO_12_Handler
                DCD     GPIO_13_Handler
                DCD     GPIO_COMBINED_Handler
                DCD     0
                DCD     0
                DCD     0
                DCD     0
                DCD     0
                DCD     0
                DCD     0
                DCD     LLCC_TXEVT_EMPTY_Handler
                DCD     LLCC_TXCMD_EMPTY_Handler
                DCD     LLCC_RXEVT_VALID_Handler
                DCD     LLCC_RXCMD_VALID_Handler
                DCD     LLCC_TXDMAL_DONE_Handler
                DCD     LLCC_RXDMAL_DONE_Handler
                DCD     LLCC_TXDMAH_DONE_Handler
                DCD     LLCC_RXDMAH_DONE_Handler

                AREA    |.text|, CODE, READONLY

;***************************************************************************************************
;* RESET HANDLER
;***************************************************************************************************

Reset_Handler\
                PROC
                EXPORT  Reset_Handler          [WEAK]
                IMPORT  SystemInit
                IMPORT  __main
                LDR     R0, =SystemInit
                BLX     R0
                LDR     R0, =__main
                BX      R0
                ENDP

;***************************************************************************************************
;* DUMMY EXCEPTION HANDLERS
;***************************************************************************************************

NMI_Handler\
                PROC
                EXPORT  NMI_Handler            [WEAK]
                B       .
                ENDP

HardFault_Handler\
                PROC
                EXPORT  HardFault_Handler      [WEAK]
                B       .
                ENDP

SVC_Handler\
                PROC
                EXPORT  SVC_Handler            [WEAK]
                B       .
                ENDP

PendSV_Handler\
                PROC
                EXPORT  PendSV_Handler         [WEAK]
                B       .
                ENDP

SysTick_Handler\
                PROC
                EXPORT  SysTick_Handler        [WEAK]
                B       .
                ENDP

Default_Handler\
                PROC
                EXPORT  TIMER_32K_Handler      [WEAK]
                EXPORT  UART0_Handler          [WEAK]
                EXPORT  GPIO_2_Handler         [WEAK]
                EXPORT  GPIO_3_Handler         [WEAK]
                EXPORT  SPI0_Handler           [WEAK]
                EXPORT  TIMER_DUAL_Handler     [WEAK]
                EXPORT  CLOCKSTATUS_Handler    [WEAK]
                EXPORT  WDT_Handler            [WEAK]
                EXPORT  UART1_Handler          [WEAK]
                EXPORT  TWI0_Handler           [WEAK]
                EXPORT  GPIO_10_Handler        [WEAK]
                EXPORT  GPIO_11_Handler        [WEAK]
                EXPORT  GPIO_12_Handler        [WEAK]
                EXPORT  GPIO_13_Handler        [WEAK]
                EXPORT  GPIO_COMBINED_Handler  [WEAK]
                EXPORT  LLCC_TXEVT_EMPTY_Handler   [WEAK]
                EXPORT  LLCC_TXCMD_EMPTY_Handler   [WEAK]
                EXPORT  LLCC_RXEVT_VALID_Handler   [WEAK]
                EXPORT  LLCC_RXCMD_VALID_Handler   [WEAK]
                EXPORT  LLCC_TXDMAL_DONE_Handler   [WEAK]
                EXPORT  LLCC_RXDMAL_DONE_Handler   [WEAK]
                EXPORT  LLCC_TXDMAH_DONE_Handler   [WEAK]
                EXPORT  LLCC_RXDMAH_DONE_Handler   [WEAK]

TIMER_32K_Handler
UART0_Handler
GPIO_2_Handler
GPIO_3_Handler
SPI0_Handler
TIMER_DUAL_Handler
CLOCKSTATUS_Handler
WDT_Handler
UART1_Handler
TWI0_Handler
GPIO_10_Handler
GPIO_11_Handler
GPIO_12_Handler
GPIO_13_Handler
GPIO_COMBINED_Handler
LLCC_TXEVT_EMPTY_Handler
LLCC_TXCMD_EMPTY_Handler
LLCC_RXEVT_VALID_Handler
LLCC_RXCMD_VALID_Handler
LLCC_TXDMAL_DONE_Handler
LLCC_RXDMAL_DONE_Handler
LLCC_TXDMAH_DONE_Handler
LLCC_RXDMAH_DONE_Handler
                B       .
                ENDP

                ALIGN

;***************************************************************************************************
;* HEAP & STACK SETUP
;***************************************************************************************************

            IF :DEF:__MICROLIB

                EXPORT  __initial_sp
                EXPORT  __heap_base
                EXPORT  __heap_limit

            ELSE

                IMPORT  __use_two_region_memory
                EXPORT  __user_initial_stackheap

__user_initial_stackheap
                LDR     R0, = Heap_Mem
                LDR     R1, =(Stack_Mem + Stack_Size)
                LDR     R2, =(Heap_Mem +  Heap_Size)
                LDR     R3, = Stack_Mem
                BX      LR

                ALIGN

            ENDIF

                AREA    |.text|,CODE, READONLY
getPC           PROC
                EXPORT  getPC

                MOV     R0,LR
                BX      LR

                ENDP

                END
