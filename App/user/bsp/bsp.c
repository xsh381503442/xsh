//*****************************************************************************
//
//! @file am_bsp.c
//!
//! @brief Top level functions for performing board initialization.
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
#include "bsp_config.h"
#include "bsp.h"

#include "drv_lcd.h"
#include "drv_key.h"
#include "bsp_uart.h"
#include "drv_heartrate.h"
#include "bsp_iom.h"
#include "drv_extflash.h"
#include "drv_battery.h"
#include "drv_hrt.h"



#if DEBUG_ENABLED == 1 && BSP_LOG_ENABLED == 1
	#define BSP_LOG_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define BSP_LOG_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define BSP_LOG_WRITESTRING(...)
	#define BSP_LOG_PRINTF(...)		        
#endif

void bsp_init(void)
{
	//将SWDIO和SWCLK内部上拉
	am_hal_gpio_pin_config(21, ((AM_HAL_GPIO_FUNC(0) | AM_HAL_PIN_DIR_INPUT)|AM_HAL_GPIO_PULLUP));
	am_hal_gpio_pin_config(20, ((AM_HAL_GPIO_FUNC(0) | AM_HAL_PIN_DIR_INPUT)|AM_HAL_GPIO_PULLUP));

	//SPI等初始化配置，如SPI--GT24L24/flash/Ublox电源为常开，片选信号需上拉
	am_hal_gpio_pin_config(BSP_GPIO_IOM0_SPI_MISO,AM_HAL_GPIO_INPUT|AM_HAL_GPIO_PULLUP);
	am_hal_gpio_pin_config(BSP_GPIO_IOM0_SPI_SCK,AM_HAL_GPIO_INPUT|AM_HAL_GPIO_PULLUP);
	am_hal_gpio_pin_config(BSP_GPIO_IOM0_SPI_MOSI,AM_HAL_GPIO_INPUT|AM_HAL_GPIO_PULLUP);

//	am_hal_gpio_pin_config(BSP_GPIO_UBLOX_CE,AM_HAL_GPIO_INPUT|AM_HAL_GPIO_PULLUP);
	am_hal_gpio_pin_config(BSP_GPIO_SPIFLASH_CE,AM_HAL_GPIO_INPUT|AM_HAL_GPIO_PULLUP);
   
	am_hal_gpio_pin_config(BSP_GPIO_GT24L24_CE,AM_HAL_GPIO_INPUT|AM_HAL_GPIO_PULLUP);
	
	am_hal_gpio_pin_config(EXTFLASH_READY_PIN,AM_HAL_PIN_INPUT | AM_HAL_GPIO_PULLUP);
	
	//心率 ENABLE脚拉低
	//am_hal_gpio_pin_config(BSP_GPIO_HR_EN,AM_HAL_PIN_OUTPUT);
	//am_hal_gpio_out_bit_clear(BSP_GPIO_HR_EN);//如果悬空32K晶振会有电流串到主控，关闭心率后需要将该脚拉低
	
	//USB检测IO口
	am_hal_gpio_pin_config(DRV_BAT_USB_DETECT_PIN,AM_HAL_GPIO_INPUT);
	
//	am_hal_gpio_pin_config(BSP_GPIO_HR_D,AM_HAL_GPIO_INPUT|AM_HAL_GPIO_PULLUP);
//	bsp_iom_disable(2);
#if defined( WATCH_MODEL_PLUS ) 
	bsp_iom_disable(BSP_HEARTRATE_IOM);
#endif
#ifdef WATCH_COD_BSP
	//am_hal_gpio_pin_config(BSP_GPIO_HR5V_EN,AM_HAL_GPIO_OUTPUT);
	//am_hal_gpio_out_bit_clear(BSP_GPIO_HR5V_EN);
	drv_hrt_5v_disable();
	drv_hrt_3v3_disable();
	bsp_iom_disable(BSP_HEARTRATE_IOM);
	am_hal_gpio_pin_config(BSP_GPIO_BUZZER_PWR,AM_HAL_GPIO_OUTPUT);
	am_hal_gpio_out_bit_clear(BSP_GPIO_BUZZER_PWR);
	
	am_hal_gpio_pin_config(BSP_GPIO_BUZZ_EN,AM_HAL_GPIO_OUTPUT);
	am_hal_gpio_out_bit_set(BSP_GPIO_BUZZ_EN);
	
	//未使用的引脚配置
	am_hal_gpio_pin_config(BSP_GPIO_MOTION_INT1,AM_HAL_GPIO_3STATE);
	am_hal_gpio_pin_config(BSP_GPIO_MOTION_INT2,AM_HAL_GPIO_3STATE);
	am_hal_gpio_pin_config(BSP_GPIO_DRDY,AM_HAL_GPIO_3STATE);
	am_hal_gpio_pin_config(BSP_GPIO_1PPS,AM_HAL_GPIO_3STATE);
	//spi1
	am_hal_gpio_pin_config(BSP_GPIO_IOM1_SPI_MISO,AM_HAL_GPIO_3STATE);
	am_hal_gpio_pin_config(BSP_GPIO_IOM1_SPI_SCK,AM_HAL_GPIO_INPUT|AM_HAL_GPIO_PULLUP);
	am_hal_gpio_pin_config(BSP_GPIO_IOM1_SPI_MOSI,AM_HAL_GPIO_INPUT|AM_HAL_GPIO_PULLUP);
	am_hal_gpio_pin_config(BSP_GPIO_LCD_CE,AM_HAL_GPIO_INPUT|AM_HAL_GPIO_PULLUP);
	//蓝牙下载关闭，调试是可以打开
	//am_hal_gpio_pin_config(BSP_GPIO_BT_SEL,AM_HAL_GPIO_INPUT);
	am_hal_gpio_pin_config(BSP_GPIO_BT_SEL,AM_HAL_GPIO_OUTPUT);
    am_hal_gpio_out_bit_set(BSP_GPIO_BT_SEL);

	 am_hal_gpio_pin_config(BSP_GPIO_BT_RST,AM_HAL_GPIO_OUTPUT);
     am_hal_gpio_out_bit_set(BSP_GPIO_BT_RST);
	 am_hal_gpio_out_bit_clear(BSP_GPIO_BT_RST);
	 am_util_delay_ms(5);
	 am_hal_gpio_out_bit_set(BSP_GPIO_BT_RST);

	#ifndef WATXCH_LCD_TRULY
	//打开屏电源
	LCD_Poweron();
	//初始化LCD
	LCD_Init();
	#endif
#else
	//初始化LCD
	LCD_Init();

	//打开屏电源
	LCD_Poweron();
	//将屏刷黑
	LCD_SetBackgroundColor(LCD_BLACK);
	LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
	//显示
	LCD_DisplayOn();
#endif
	
}

void bsp_low_power_init(void)
{
    //
    // Enable internal buck converters.
    //
    am_hal_pwrctrl_bucks_enable();

    //
    // Turn off the voltage comparator as this is enabled on reset.
    //
    am_hal_vcomp_disable();

    //
    // Run the RTC off the LFRC.
    //
    am_hal_rtc_osc_select(AM_HAL_RTC_OSC_LFRC);

    //
    // Stop the XT and LFRC.
    //
    am_hal_clkgen_osc_stop(AM_HAL_CLKGEN_OSC_XT);
    am_hal_clkgen_osc_stop(AM_HAL_CLKGEN_OSC_LFRC);

    //
    // Disable the RTC.
    //
    am_hal_rtc_osc_disable();


}


//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************





