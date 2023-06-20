#include "dev_bat.h"
#include "drv_lcd.h"
#include "rtt_log.h"
#include "lib_error.h"
#include "com_apollo2.h"
#include "SEGGER_RTT.h"

#include <stdio.h>
#include <math.h>
#include "am_mcu_apollo.h"


#include <string.h>


#define MOUDLE_LOG_ENABLED 							0
#define MOUDLE_DEBUG_ENABLED   					0
#define MOUDLE_NAME                     "[DEV_BAT]:"

#if MOUDLE_LOG_ENABLED == 1 && RTT_LOG_ENABLED == 1
#define LOG_PRINTF(...)                      SEGGER_RTT_printf(0, ##__VA_ARGS__) 
#define LOG_HEXDUMP(p_data, len)             rtt_log_hexdump(p_data, len) 
#else
#define LOG_PRINTF(...) 
#define LOG_HEXDUMP(p_data, len)             
#endif

#if MOUDLE_DEBUG_ENABLED == 1 && RTT_LOG_ENABLED == 1
#define DEBUG_PRINTF(...)                    SEGGER_RTT_printf(0, ##__VA_ARGS__)
#define DEBUG_HEXDUMP(p_data, len)           rtt_log_hexdump(p_data, len)
#else
#define DEBUG_PRINTF(...)   
#define DEBUG_HEXDUMP(p_data, len) 
#endif




#define ADC_REFERENCE_VOLTAGE       (2.0f)            
#define ADC_RESOLUTION_BITS         (14) 
#define ADC_GAIN                    (0.97f)



static _dev_bat         m_dev_bat;
static float            m_battery_divider_factor; 
_dev_bat_event   g_bat_evt;

void dev_bat_adc_enable(void)
{

	am_hal_adc_config_t sADCConfig;
	
	am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_ADC);

	PIN_CONFIG(m_dev_bat.param.adc_pin, AM_HAL_PIN_34_ADCSE6);
	
	sADCConfig.ui32Clock = AM_HAL_ADC_CLOCK_HFRC;
	sADCConfig.ui32TriggerConfig = AM_HAL_ADC_TRIGGER_SOFT;
	sADCConfig.ui32Reference = AM_HAL_ADC_REF_INT_2P0;
	sADCConfig.ui32ClockMode = AM_HAL_ADC_CK_LOW_POWER;
	sADCConfig.ui32PowerMode = AM_HAL_ADC_LPMODE_1;
	sADCConfig.ui32Repeat = AM_HAL_ADC_NO_REPEAT;
	am_hal_adc_config(&sADCConfig);	

	am_hal_adc_int_enable(AM_HAL_ADC_INT_CNVCMP);
	
	am_hal_adc_slot_config(0, AM_HAL_ADC_SLOT_AVG_1 |
														AM_HAL_ADC_SLOT_14BIT |
														AM_HAL_ADC_SLOT_CHSEL_SE6 |
														AM_HAL_ADC_SLOT_ENABLE);	

	am_hal_adc_enable();
	am_hal_adc_trigger();

}


static void adc_disable(void)
{

	PIN_CONFIG(m_dev_bat.param.adc_pin, AM_HAL_PIN_DISABLE);
	am_hal_adc_disable();
	am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_ADC);

}

static void adc_to_batt_voltage(uint32_t adc_val, uint16_t * const voltage)
{
    uint16_t tmp_voltage;
    float tmp = adc_val / ((ADC_GAIN / ADC_REFERENCE_VOLTAGE) * pow(2, ADC_RESOLUTION_BITS));
    tmp_voltage =  (uint16_t) ((tmp / m_battery_divider_factor) * 1000);
    *voltage = ( (tmp_voltage + 5) / 10) * 10;  // Round the value.

}
static void batt_voltage_to_percent(uint16_t voltage_mv, uint8_t * const battery_level_percent)
{
    if (voltage_mv >= m_dev_bat.param.batt_voltage_limit_full)
    {
        *battery_level_percent = 100;
    }
    else if (voltage_mv <= m_dev_bat.param.batt_voltage_limit_low)
    {
        *battery_level_percent = 0;
    }
    else
    {
        *battery_level_percent = (100 * (voltage_mv - m_dev_bat.param.batt_voltage_limit_low) /
                                 (m_dev_bat.param.batt_voltage_limit_full - m_dev_bat.param.batt_voltage_limit_low));
    }
}
void am_adc_isr(void)
{
		uint32_t ui32Status;
		
		uint32_t tmp_adc = 0;	
    //
    // Read the interrupt status.
    //
    ui32Status = am_hal_adc_int_status_get(true);

    //
    // Clear the ADC interrupt.
    //
    am_hal_adc_int_clear(ui32Status);

    //
    // If we got a conversion completion interrupt (which should be our only
    // ADC interrupt), go ahead and read the data.
    //
    if (ui32Status & AM_HAL_ADC_INT_CNVCMP)
    {
			
				DEBUG_PRINTF(MOUDLE_NAME"AM_HAL_ADC_INT_CNVCMP\n");
        //
        // Read the value from the FIFO.
        //
				tmp_adc = (am_hal_adc_fifo_pop()>>6)&0x3FFF;//高14位有效
			
//				tmp_adc = AM_HAL_ADC_FIFO_SAMPLE(am_hal_adc_fifo_pop());
//				DEBUG_PRINTF(MOUDLE_NAME"Slot: %d\r\n", AM_HAL_ADC_FIFO_SLOT(tmp_adc));
//				DEBUG_PRINTF(MOUDLE_NAME"Data: %d\r\n", AM_HAL_ADC_FIFO_SAMPLE(tmp_adc));
//				DEBUG_PRINTF(MOUDLE_NAME"Fifo Size: %d\r\n", AM_HAL_ADC_FIFO_COUNT(tmp_adc));
			
				
				adc_disable();
				DEBUG_PRINTF(MOUDLE_NAME"tmp_adc : %d\n",tmp_adc);
				adc_to_batt_voltage(tmp_adc, &g_bat_evt.voltage_mv);
				DEBUG_PRINTF(MOUDLE_NAME"voltage_mv : %d\n",g_bat_evt.voltage_mv);
				batt_voltage_to_percent(g_bat_evt.voltage_mv, &g_bat_evt.level_percent);
				DEBUG_PRINTF(MOUDLE_NAME"level_percent : %d percent\n",g_bat_evt.level_percent);
				
		

//				if (g_bat_evt.voltage_mv <= m_dev_bat.param.batt_voltage_limit_low)
//				{
//						g_bat_evt.type = DEV_BAT_EVENT_LOW;
//					DEBUG_PRINTF(MOUDLE_NAME"DEV_BAT_EVENT_LOW\n");
//				}
//				else if (g_bat_evt.voltage_mv >= m_dev_bat.param.batt_voltage_limit_full)
//				{
//						g_bat_evt.type = DEV_BAT_EVENT_FULL;
//					DEBUG_PRINTF(MOUDLE_NAME"DEV_BAT_EVENT_FULL\n");
//				}
//				else
				{
						g_bat_evt.type = DEV_BAT_EVENT_DATA;
					DEBUG_PRINTF(MOUDLE_NAME"DEV_BAT_EVENT_DATA\n");
				}				
				
				m_dev_bat.evt_handler(&g_bat_evt);				
			
		}

}


static void usb_detect_hdl(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"usb_detect_hdl\n");
//	com_apollo2_gpio_int_uninit(m_dev_bat.param.usb_detect_pin);
	if (PIN_READ_INPUT(m_dev_bat.param.usb_detect_pin))
	{
		am_hal_gpio_int_polarity_bit_set(m_dev_bat.param.usb_detect_pin,USB_DETECT_PIN_DISCONNECT_POLARITY);
		if (PIN_READ_INPUT(m_dev_bat.param.batt_chg_stat_pin))
		{
			g_bat_evt.type = DEV_BAT_EVENT_USB_CONN_CHARGING_FINISHED;
			g_bat_evt.cable_status = DEV_BAT_EVENT_USB_CONN_CHARGING_FINISHED;
		}
		else
		{
			g_bat_evt.type = DEV_BAT_EVENT_USB_CONN_CHARGING;
			g_bat_evt.cable_status = DEV_BAT_EVENT_USB_CONN_CHARGING;
		}		
	}
	else{
		am_hal_gpio_int_polarity_bit_set(m_dev_bat.param.usb_detect_pin,USB_DETECT_PIN_CONNECT_POLARITY);
		g_bat_evt.type = DEV_BAT_EVENT_USB_DISCONN;
		g_bat_evt.cable_status = DEV_BAT_EVENT_USB_DISCONN;
	
	}
	m_dev_bat.evt_handler(&g_bat_evt);	
}



void dev_bat_init(_dev_bat *p_dev_bat)
{
	DEBUG_PRINTF(MOUDLE_NAME"dev_bat_init\n");	

	m_dev_bat.evt_handler = p_dev_bat->evt_handler;
	m_dev_bat.param.adc_pin = p_dev_bat->param.adc_pin;
	m_dev_bat.param.batt_chg_stat_pin = p_dev_bat->param.batt_chg_stat_pin;
	m_dev_bat.param.usb_detect_pin = p_dev_bat->param.usb_detect_pin;
	m_dev_bat.param.batt_voltage_limit_low = p_dev_bat->param.batt_voltage_limit_low;
	m_dev_bat.param.batt_voltage_limit_full = p_dev_bat->param.batt_voltage_limit_full;
	m_dev_bat.param.voltage_divider.r_1_ohm = p_dev_bat->param.voltage_divider.r_1_ohm;
	m_dev_bat.param.voltage_divider.r_2_ohm = p_dev_bat->param.voltage_divider.r_2_ohm;
	
	m_battery_divider_factor = m_dev_bat.param.voltage_divider.r_2_ohm /
										 (float)(m_dev_bat.param.voltage_divider.r_1_ohm +
														 m_dev_bat.param.voltage_divider.r_2_ohm);	
  PIN_CONFIG(m_dev_bat.param.batt_chg_stat_pin,AM_HAL_GPIO_INPUT);
	am_hal_gpio_int_register(m_dev_bat.param.usb_detect_pin, usb_detect_hdl);
	com_apollo2_gpio_int_init(m_dev_bat.param.usb_detect_pin, p_dev_bat->param.usb_detect_pin_polarity);	
	dev_bat_adc_enable();
}








