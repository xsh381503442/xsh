#include "bsp_config.h"
#include "bsp_iom.h"

#include "bsp.h"

#if DEBUG_ENABLED == 1 && BSP_IOM_LOG_ENABLED == 1
	#define BSP_IOM_LOG_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define BSP_IOM_LOG_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define BSP_IOM_LOG_WRITESTRING(...)
	#define BSP_IOM_LOG_PRINTF(...)		        
#endif

void bsp_iom_enable(uint32_t ui32Module,am_hal_iom_config_t iomConfig)
{
	BSP_IOM_LOG_PRINTF("[bsp_iom]: bsp_iom_enable %d\n",ui32Module);
	
	// Power on the IOM
	am_hal_iom_pwrctrl_enable(ui32Module);

	// Configure IOM
	am_hal_iom_config(ui32Module,&iomConfig);

	switch(ui32Module)
	{
		case 0:
		//SPI0
			am_hal_gpio_pin_config(BSP_GPIO_IOM0_SPI_MISO,BSP_GPIO_CFG_IOM0_SPI_MISO);
			am_hal_gpio_pin_config(BSP_GPIO_IOM0_SPI_SCK,BSP_GPIO_CFG_IOM0_SPI_SCK);
			am_hal_gpio_pin_config(BSP_GPIO_IOM0_SPI_MOSI,BSP_GPIO_CFG_IOM0_SPI_MOSI);
			break;

		case 1:
		//SPI1
		#ifdef WATCH_COD_BSP
			//am_hal_gpio_pin_config(BSP_GPIO_IOM1_SPI_MISO,BSP_GPIO_CFG_IOM1_SPI_MISO);
			am_hal_gpio_pin_config(BSP_GPIO_IOM1_SPI_SCK,BSP_GPIO_CFG_IOM1_SPI_SCK);
		#ifdef WATCH_LCD_ESD
			am_hal_gpio_pin_config(BSP_GPIO_IOM1_SPI_MOSI,AM_HAL_PIN_10_M1WIR3);
		#else
			am_hal_gpio_pin_config(BSP_GPIO_IOM1_SPI_MOSI,BSP_GPIO_CFG_IOM1_SPI_MOSI);
		#endif
		#endif
			break;
		case 2:
		//
		// Set pins high to prevent bus dips.
		//
			am_hal_gpio_out_bit_set(BSP_GPIO_IOM2_I2C_SCL);
			am_hal_gpio_out_bit_set(BSP_GPIO_IOM2_I2C_SDA);

			//I2C2
			am_hal_gpio_pin_config(BSP_GPIO_IOM2_I2C_SCL,BSP_GPIO_CFG_IOM2_I2C_SCL);
			am_hal_gpio_pin_config(BSP_GPIO_IOM2_I2C_SDA,BSP_GPIO_CFG_IOM2_I2C_SDA);
			break;
		case 3:
		//
		// Set pins high to prevent bus dips.
		//
			am_hal_gpio_out_bit_set(BSP_GPIO_IOM3_I2C_SCL);
			am_hal_gpio_out_bit_set(BSP_GPIO_IOM3_I2C_SDA);

			//I2C3
			am_hal_gpio_pin_config(BSP_GPIO_IOM3_I2C_SCL,BSP_GPIO_CFG_IOM3_I2C_SCL);
			am_hal_gpio_pin_config(BSP_GPIO_IOM3_I2C_SDA,BSP_GPIO_CFG_IOM3_I2C_SDA);
			break;
		case 4:
					//
		// Set pins high to prevent bus dips.
		//
			am_hal_gpio_out_bit_set(BSP_GPIO_IOM4_I2C_SCL);
			am_hal_gpio_out_bit_set(BSP_GPIO_IOM4_I2C_SDA);

			//I2C4
			am_hal_gpio_pin_config(BSP_GPIO_IOM4_I2C_SCL,BSP_GPIO_CFG_IOM4_I2C_SCL);
			am_hal_gpio_pin_config(BSP_GPIO_IOM4_I2C_SDA,BSP_GPIO_CFG_IOM4_I2C_SDA);
			break;
			default:
			BSP_IOM_LOG_PRINTF("[bsp_iom]: bsp_iom_enable ui32Modeule is ERROR\n");
			break;
	}
	
	// Enable IOM
	am_hal_iom_enable(ui32Module);
}

void bsp_iom_disable(uint32_t ui32Module)
{
	BSP_IOM_LOG_PRINTF("[bsp_iom]: bsp_iom_disable %d\n",ui32Module);
	
    // Disable IOM
    am_hal_iom_disable(ui32Module);
	
	// Power down IOM

	
	// Re-configure IOM pins during sleep
	switch(ui32Module)
    {
        case 0:
			//SPI0
			am_hal_gpio_pin_config(BSP_GPIO_IOM0_SPI_MISO,AM_HAL_GPIO_INPUT|AM_HAL_GPIO_PULLUP);
			am_hal_gpio_pin_config(BSP_GPIO_IOM0_SPI_SCK,AM_HAL_GPIO_INPUT|AM_HAL_GPIO_PULLUP);
			am_hal_gpio_pin_config(BSP_GPIO_IOM0_SPI_MOSI,AM_HAL_GPIO_INPUT|AM_HAL_GPIO_PULLUP);
			//为了降低功耗，需要将MOSI拉低
//			am_hal_gpio_pin_config(BSP_GPIO_IOM0_SPI_MOSI,AM_HAL_PIN_DISABLE);
//			am_hal_gpio_out_bit_clear(BSP_GPIO_IOM0_SPI_MOSI);
            break;
		case 1:
		//SPI1
		#ifdef WATCH_COD_BSP
			//am_hal_gpio_pin_config(BSP_GPIO_IOM1_SPI_MISO,AM_HAL_GPIO_INPUT|AM_HAL_GPIO_PULLUP);

			am_hal_gpio_pin_config(BSP_GPIO_IOM1_SPI_SCK,AM_HAL_GPIO_3STATE);
			am_hal_gpio_pin_config(BSP_GPIO_IOM1_SPI_MOSI,AM_HAL_GPIO_3STATE);
			//am_hal_gpio_pin_config(BSP_GPIO_IOM1_SPI_SCK,AM_HAL_GPIO_INPUT|AM_HAL_GPIO_PULLUP);
			//am_hal_gpio_pin_config(BSP_GPIO_IOM1_SPI_MOSI,AM_HAL_GPIO_INPUT|AM_HAL_GPIO_PULLUP);
		#endif
			break;
        case 2:
			//I2C2
			am_hal_gpio_pin_config(BSP_GPIO_IOM2_I2C_SCL,AM_HAL_GPIO_INPUT|AM_HAL_GPIO_PULLUP);
			am_hal_gpio_pin_config(BSP_GPIO_IOM2_I2C_SDA,AM_HAL_GPIO_INPUT|AM_HAL_GPIO_PULLUP);
            break;
		case 3:
			//I2C3
			am_hal_gpio_pin_config(BSP_GPIO_IOM3_I2C_SCL,AM_HAL_GPIO_INPUT|AM_HAL_GPIO_PULLUP);
			am_hal_gpio_pin_config(BSP_GPIO_IOM3_I2C_SDA,AM_HAL_GPIO_INPUT|AM_HAL_GPIO_PULLUP);
            break;
		case 4:
			//I2C4
			//am_hal_gpio_pin_config(BSP_GPIO_IOM4_I2C_SCL,AM_HAL_GPIO_INPUT|AM_HAL_GPIO_PULLUP);
			//am_hal_gpio_pin_config(BSP_GPIO_IOM4_I2C_SDA,AM_HAL_GPIO_INPUT|AM_HAL_GPIO_PULLUP);
			am_hal_gpio_pin_config(BSP_GPIO_IOM4_I2C_SCL,AM_HAL_GPIO_DISABLE);
		    am_hal_gpio_pin_config(BSP_GPIO_IOM4_I2C_SDA,AM_HAL_GPIO_DISABLE);
			break;
        default:
            BSP_IOM_LOG_PRINTF("[bsp_iom]: bsp_iom_disable ui32Modeule is ERROR\n");
			break;
    }	
	
	am_hal_iom_pwrctrl_disable(ui32Module);
}
