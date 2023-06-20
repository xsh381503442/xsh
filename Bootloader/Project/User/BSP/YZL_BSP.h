#ifndef __YZL_BSP_H__
#define __YZL_BSP_H__

#define COD //codoon 项目功能


#define WATCH_COD_BSP
#define WATXCH_LCD_TRULY
/*
* JLINK DEBUG PIN
*/
#define BSP_APOLLO2_SWDIO            21
#define BSP_APOLLO2_SWCLK            20
/*
* 外部闪存引脚
*/
#define BSP_APOLLO2_EXTFLASH_CLK                   5
#define BSP_APOLLO2_EXTFLASH_CLK_CONFIG            AM_HAL_PIN_5_M0SCK
#define BSP_APOLLO2_EXTFLASH_MISO                  6
#define BSP_APOLLO2_EXTFLASH_MISO_CONFIG           AM_HAL_PIN_6_M0MISO
#define BSP_APOLLO2_EXTFLASH_MOSI                  7
#define BSP_APOLLO2_EXTFLASH_MOSI_CONFIG           AM_HAL_PIN_7_M0MOSI
#define BSP_APOLLO2_EXTFLASH_CS                    45
#define BSP_APOLLO2_EXTFLASH_CS_CONFIG             AM_HAL_PIN_45_M0nCE3
#define BSP_APOLLO2_EXTFLASH_MOUDLE_INDEX          0
#define BSP_APOLLO2_EXTFLASH_CHIP_SELECT_INDEX     3
#define BSP_APOLLO2_EXTFLASH_READY                 49

/*
* 字库
*/
#ifdef WATCH_COD_BSP
#define BSP_APOLLO2_FONT_SPI2_SS_PIN          44
#else
#define BSP_APOLLO2_FONT_SPI2_SS_PIN          48
#endif
/*
* 按键
*/
#define BSP_APOLLO2_BTN_PIN_PWR                24
#ifdef WATCH_COD_BSP
#define BSP_APOLLO2_BTN_PIN_BL                 36
#else
#define BSP_APOLLO2_BTN_PIN_BL                 39
#endif
#define BSP_APOLLO2_BTN_PIN_UP                 28
#define BSP_APOLLO2_BTN_PIN_OK                 26
#define BSP_APOLLO2_BTN_PIN_DOWN               47

#define BSP_APOLLO2_BTN_PIN_PWR_POLARITY                0
#define BSP_APOLLO2_BTN_PIN_BL_POLARITY                 0
#define BSP_APOLLO2_BTN_PIN_UP_POLARITY                 0
#define BSP_APOLLO2_BTN_PIN_OK_POLARITY                 0
#define BSP_APOLLO2_BTN_PIN_DOWN_POLARITY               0
#ifdef WATCH_COD_BSP
//SPI1
#define		BSP_GPIO_IOM1_SPI_MISO				9
#define 	BSP_GPIO_CFG_IOM1_SPI_MISO			AM_HAL_PIN_9_M1MISO
#define 	BSP_GPIO_IOM1_SPI_MOSI				10
#define 	BSP_GPIO_CFG_IOM1_SPI_MOSI			AM_HAL_PIN_10_M1MOSI
#define 	BSP_GPIO_IOM1_SPI_SCK				8
#define 	BSP_GPIO_CFG_IOM1_SPI_SCK			(AM_HAL_PIN_8_M1SCK | AM_HAL_GPIO_HIGH_DRIVE)
#endif

/*
* 背光
*/
#ifdef WATCH_COD_BSP
#define BSP_APOLLO2_BACKLIGHT_PIN                 48
#else
#define BSP_APOLLO2_BACKLIGHT_PIN               40
#endif
/*
* PC串口
*/
#define BSP_APOLLO2_DRV_PC_UART_MOUDLE                       0
#define BSP_APOLLO2_DRV_PC_UART_TX_PIN                       22
#define BSP_APOLLO2_DRV_PC_UART_RX_PIN                       23
#define	DRV_PV_UART_TX_CONFIG					AM_HAL_PIN_22_UART0TX
#define	DRV_PV_UART_RX_CONFIG					AM_HAL_PIN_23_UART0RX


/*
* BLE串口
*/
#define BSP_APOLLO2_DRV_BLE_UART_MOUDLE                       1
#define BSP_APOLLO2_DRV_BLE_UART_TX_PIN                       18
#define BSP_APOLLO2_DRV_BLE_UART_RX_PIN                       19
#define BSP_APOLLO2_DRV_BLE_UART_CTS_PIN                      29
#define BSP_APOLLO2_DRV_BLE_UART_RTS_PIN                      30
#define BSP_APOLLO2_DRV_BLE_POLARITY_CTS_PIN                  1
/*
* BLE低功耗开关
*/
#ifdef WATCH_COD_BSP
#define BSP_APOLLO2_DEV_BLE_SYS_OFF_PIN             1
#else
#define BSP_APOLLO2_DEV_BLE_SYS_OFF_PIN            37	
#endif
/*
* 充电脚
*/
#define BSP_APOLLO2_TASK_CHARGE_ADC_PIN                  					 34
#ifdef WATCH_COD_BSP
#define BSP_APOLLO2_TASK_CHARGE_USB_DETECT_PIN           					 16
#else
#define BSP_APOLLO2_TASK_CHARGE_USB_DETECT_PIN           					 25
#endif
#define BSP_APOLLO2_TASK_CHARGE_BAT_CHG_STATE_PIN        					 31


/*
* LCD
*/
#ifdef WATCH_COD_BSP
//lcd 
#define		BSP_GPIO_LCD_CE					13
#define		BSP_GPIO_CFG_LCD_CE				AM_HAL_PIN_13_M1nCE1
#define		BSP_LCD_IOM						1	//SPI1
#define		BSP_LCD_CS						1
#define		BSP_GPIO_LCD_PWR					3
#define		BSP_GPIO_LCD_LSA0					11
#define		BSP_GPIO_LCD_REST					12

#define		BSP_GPIO_LCD_TE						9


#define		BSP_GPIO_BUZZ_EN				38 //BUZZ_EN
#else
#define		BSP_APOLLO2_GPIO_LCD_VDD1					          13
#define		BSP_APOLLO2_GPIO_LCD_VDD2					          38

#define		BSP_APOLLO2_GPIO_LCD_XFRP					          35
#define		BSP_APOLLO2_GPIO_CFG_LCD_XFRP				        AM_HAL_PIN_35_TCTA1
#define		BSP_APOLLO2_LCD_XFRF_CTIMER_SEGMENT			    AM_HAL_CTIMER_TIMERA
#define		BSP_APOLLO2_LCD_XFRP_CTIMER_NUM				      1
#define		BSP_APOLLO2_GPIO_LCD_VCOM					          33
#define		BSP_APOLLO2_GPIO_CFG_LCD_VCOM				        AM_HAL_PIN_33_TCTB1
#define		BSP_APOLLO2_LCD_VCOM_CTIMER_SEGMENT			    AM_HAL_CTIMER_TIMERB
#define		BSP_APOLLO2_LCD_VCOM_CTIMER_NUM				      1

#define		BSP_APOLLO2_GPIO_LCD_HST					0
#define		BSP_APOLLO2_GPIO_LCD_VST					3
#define		BSP_APOLLO2_GPIO_LCD_HCK					9
#define		BSP_APOLLO2_GPIO_LCD_VCK					8
#define		BSP_APOLLO2_GPIO_LCD_ENB					1
#define		BSP_APOLLO2_GPIO_LCD_R0						16
#define		BSP_APOLLO2_GPIO_LCD_G0						15
#define		BSP_APOLLO2_GPIO_LCD_B0						14
#define		BSP_APOLLO2_GPIO_LCD_R1						12
#define		BSP_APOLLO2_GPIO_LCD_G1						11
#define		BSP_APOLLO2_GPIO_LCD_B1						10
#define		BSP_APOLLO2_GPIO_LCD_XRST					41	
#endif










#endif //__JW901_BSP_H__






