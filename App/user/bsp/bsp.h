//*****************************************************************************
//
//! @file am_bsp.h
//!
//! @brief Functions to aid with configuring the GPIOs.
//!
//! @addtogroup BSP Board Support Package (BSP)
//! @addtogroup apollo_evk_sensor_bsp BSP for the Apollo EVK Sensor board
//! @ingroup BSP
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
// Copyright (c) 2016, Ambiq Micro
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
// 
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// This is part of revision 1.0.6 of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef BSP_H
#define BSP_H

#include <stdint.h>
#include <stdbool.h>
#include "watch_config.h"

#ifdef __cplusplus
extern "C"
{
#endif
	

	
//LCD
#define		BSP_GPIO_LCD_VDD1					13


#define		BSP_GPIO_LCD_XFRP					35
#define		BSP_GPIO_CFG_LCD_XFRP				AM_HAL_PIN_35_TCTA1
#define		BSP_LCD_XFRF_CTIMER_SEGMENT			AM_HAL_CTIMER_TIMERA
#define		BSP_LCD_XFRP_CTIMER_NUM				1
#define		BSP_GPIO_LCD_VCOM					33
#define		BSP_GPIO_CFG_LCD_VCOM				AM_HAL_PIN_33_TCTB1
#define		BSP_LCD_VCOM_CTIMER_SEGMENT			AM_HAL_CTIMER_TIMERB
#define		BSP_LCD_VCOM_CTIMER_NUM				1

#define		BSP_GPIO_LCD_HST					0
#define		BSP_GPIO_LCD_VST					3
#define		BSP_GPIO_LCD_HCK					9
#define		BSP_GPIO_LCD_VCK					8
#define		BSP_GPIO_LCD_ENB					1
#define		BSP_GPIO_LCD_R0						16
#define		BSP_GPIO_LCD_G0						15
#define		BSP_GPIO_LCD_B0						14
#define		BSP_GPIO_LCD_R1						12
#define		BSP_GPIO_LCD_G1						11
#define		BSP_GPIO_LCD_B1						10
#define		BSP_GPIO_LCD_XRST					41	
	

#define		BSP_GPIO_LCD_VDD2					38
	//Motor
#define		BSP_GPIO_MOTOR_PWR					17	//马达控制

	//Buzzer
#define		BSP_GPIO_BUZZER_PWR					32	//蜂鸣器
#define		BSP_GPIO_CFG_BUZZER_PWR				AM_HAL_PIN_32_TCTB3



#define		BSP_BUZZER_CTIMER_SEGMENT			AM_HAL_CTIMER_TIMERB
#define		BSP_BUZZER_CTIMER_NUM				3

//SPI0
#define		BSP_GPIO_IOM0_SPI_MISO				6
#define 	BSP_GPIO_CFG_IOM0_SPI_MISO			AM_HAL_PIN_6_M0MISO
#define 	BSP_GPIO_IOM0_SPI_MOSI				7
#define 	BSP_GPIO_CFG_IOM0_SPI_MOSI			AM_HAL_PIN_7_M0MOSI
#define 	BSP_GPIO_IOM0_SPI_SCK				5
#define 	BSP_GPIO_CFG_IOM0_SPI_SCK			(AM_HAL_PIN_5_M0SCK | AM_HAL_GPIO_HIGH_DRIVE)
#ifdef WATCH_COD_BSP 
//SPI1
#define		BSP_GPIO_IOM1_SPI_MISO				9
#define 	BSP_GPIO_CFG_IOM1_SPI_MISO			AM_HAL_PIN_9_M1MISO
#define 	BSP_GPIO_IOM1_SPI_MOSI				10
#define 	BSP_GPIO_CFG_IOM1_SPI_MOSI			AM_HAL_PIN_10_M1MOSI
#define 	BSP_GPIO_IOM1_SPI_SCK				8
#define 	BSP_GPIO_CFG_IOM1_SPI_SCK			(AM_HAL_PIN_8_M1SCK | AM_HAL_GPIO_HIGH_DRIVE)
#endif

//I2C2
#define		BSP_GPIO_IOM2_I2C_SCL        		27
#define		BSP_GPIO_CFG_IOM2_I2C_SCL    		(AM_HAL_PIN_27_M2SCL | AM_HAL_GPIO_PULL1_5K)
#define		BSP_GPIO_IOM2_I2C_SDA        		25
#define		BSP_GPIO_CFG_IOM2_I2C_SDA    		(AM_HAL_PIN_25_M2SDA | AM_HAL_GPIO_PULL1_5K)

//I2C3
#define		BSP_GPIO_IOM3_I2C_SCL        		42
#define		BSP_GPIO_CFG_IOM3_I2C_SCL    		(AM_HAL_PIN_42_M3SCL | AM_HAL_GPIO_PULL1_5K)
#define		BSP_GPIO_IOM3_I2C_SDA        		43
#define		BSP_GPIO_CFG_IOM3_I2C_SDA    		(AM_HAL_PIN_43_M3SDA | AM_HAL_GPIO_PULL1_5K)

//I2C4
#define		BSP_GPIO_IOM4_I2C_SCL        		39
#define		BSP_GPIO_CFG_IOM4_I2C_SCL    		(AM_HAL_PIN_39_M4SCL | AM_HAL_GPIO_PULL1_5K)
#define		BSP_GPIO_IOM4_I2C_SDA        		40
#define		BSP_GPIO_CFG_IOM4_I2C_SDA    		(AM_HAL_PIN_40_M4SDA | AM_HAL_GPIO_PULL1_5K)

//GT24L24
#ifdef WATCH_COD_BSP 
#define		BSP_GPIO_GT24L24_CE					44
#define		BSP_GPIO_CFG_GT24L24_CE				AM_HAL_PIN_44_M0nCE2
#define		BSP_GT24L24_IOM						0
#define		BSP_GT24L24_CS						2
#else
#define		BSP_GPIO_GT24L24_CE					48
#define		BSP_GPIO_CFG_GT24L24_CE				AM_HAL_PIN_48_M0nCE6
#define		BSP_GT24L24_IOM						0
#define		BSP_GT24L24_CS						6
#endif
//UBLOX

#define   	BSP_UBLOX_I2C_ADDRESS      (0x42)


#if defined( WATCH_MODEL_PLUS )  

	#define		BSP_GPIO_BL_PWR						23  //背光
	#define		BSP_GPIO_UBLOX_PWR					44	//GPS电源
	#define		BSP_UBLOX_IOM						2
	#define		BSP_HEARTRATE_IOM					4
	#define     BSP_GPIO_MOTION_INT1                30	//计步中断
	#define     BSP_GPIO_HR_EN            			46
	#define  	BSP_GPIO_1PPS          				31
#else

#ifdef WATCH_COD_BSP
#define  	BSP_GPIO_1PPS          				14	//GPS_1PPS
#define		BSP_GPIO_BL_PWR						48  //背光
#define		BSP_GPIO_UBLOX_PWR					35	//GPS电源
#define		BSP_UBLOX_IOM						4	//GPS i2c4
#define     BSP_GPIO_DRDY              			41	//地磁数据准备脚
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
//ble
#define		BSP_GPIO_BT_RST					0
//心率
#define		BSP_HEARTRATE_IOM					2	//心率i2c2
#define 	GPIO_PIN_NUM_HRT_5V_ENABLE			37
#define 	GPIO_PIN_NUM_HRT_3V3_ENABLE			46
#define 	GPIO_PIN_NUM_HRT_INT				33


#else
#define  	BSP_GPIO_1PPS          				27
#define		BSP_GPIO_BL_PWR						40  //背光
#define		BSP_GPIO_UBLOX_PWR					36	//GPS电源
#define		BSP_UBLOX_IOM						3
#define		BSP_HEARTRATE_IOM					3
#endif
#define     BSP_GPIO_MOTION_INT1                4	//计步中断1

#define     BSP_GPIO_HR_EN            			46
#endif

//FLASH
#define		BSP_GPIO_SPIFLASH_CE				45  //SPI片选   Flash
#define		BSP_GPIO_CFG_SPIFLASH_CE			AM_HAL_PIN_45_M0nCE3
#define		BSP_SPIFLASH_IOM					0
#define		BSP_SPIFLASH_CS						3

//MS5837
#define		BSP_MS5837_IOM						3


//心率
#define		BSP_GPIO_UART0_RX					23
#define 	BSP_GPIO_CFG_UART0_RX				AM_HAL_PIN_23_UART0RX
#define		BSP_GPIO_UART0_TX					22
#define 	BSP_GPIO_CFG_UART0_TX				AM_HAL_PIN_22_UART0TX
#define     BSP_GPIO_HR_D             			44


//蓝牙
#define		BSP_GPIO_UART1_RX					19
#define 	BSP_GPIO_CFG_UART1_RX				AM_HAL_PIN_19_UART1RX
#define		BSP_GPIO_UART1_TX					18
#define 	BSP_GPIO_CFG_UART1_TX				AM_HAL_PIN_18_UART1TX



//G-SENSOR


#define     BSP_GPIO_MOTION_INT2                2

#ifdef WATCH_COD_BSP 
#define BSP_GPIO_BT_SEL 						15
#endif 
	
//*****************************************************************************
//
// Convenience macros for enabling and disabling pins by function.
//
//*****************************************************************************
#define am_bsp_pin_enable(name)                                               \
    am_hal_gpio_pin_config(BSP_GPIO_ ## name, BSP_GPIO_CFG_ ## name);

#define am_bsp_pin_disable(name)                                              \
    am_hal_gpio_pin_config(BSP_GPIO_ ## name, AM_HAL_PIN_DISABLE);
	
//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void bsp_init(void);
extern void bsp_low_power_init(void);
extern void bsp_debug_init(void);
extern void bsp_debug_printf_enable(void);
extern void bsp_debug_printf_disable(void);

#ifdef __cplusplus
}
#endif

#endif // AM_BSP_H
//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
