#include "drv_battery.h"
#include <stdio.h>
#include <math.h>
#include "am_mcu_apollo.h"
#include <string.h>
#include "SEGGER_RTT.h"
#include "com_data.h"
#include "task_battery.h"

#define	DRV_BATTERY_LOG_ENABLED		0

#if DEBUG_ENABLED == 1 && DRV_BATTERY_LOG_ENABLED == 1
	#define DRV_BATTERY_LOG_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define DRV_BATTERY_LOG_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else       
	#define DRV_BATTERY_LOG_WRITESTRING(...)
	#define DRV_BATTERY_LOG_PRINTF(...)		        
#endif


#define ADC_REFERENCE_VOLTAGE       (2.0f)            
#define ADC_RESOLUTION_BITS         (14) 
//#define ADC_GAIN                    (0.99f)
#define ADC_GAIN                    (0.96f)


#define TIMER_INIT_INTERVAL_MS      (1000)
#define TIMER_PERIOD                (60)					//60秒

static uint8_t system_first_get_vol = 0;

extern am_hal_rtc_time_t RTC_time;


static _drv_bat         m_drv_bat = {.evt_handler = NULL};
static float            m_battery_divider_factor; 
_drv_bat_event   g_bat_evt;
long m_charging_sec = 0;
uint8_t CHARGE_PWON = 0;
uint8_t m_bat_init_flag = 0;
//static uint8_t m_adc_charging_gain = 0;
//static bool m_adc_charging_gain_flag = false;
static uint32_t m_previous_pct = 0;          //改为记录电压百分比
//static uint32_t ADC_VAL_REC = 0;
static uint8_t PERCENT_REC;
uint32_t TMP_ADC;
extern uint32_t shutdown_val;

uint16_t charge_temp_vol[DRV_BAT_TEMP_DATA_NUM] = {3500}; 

uint8_t charge_temp_num = 0;
uint8_t charge_drv_flag = 0;
uint8_t first_charge_drv_flag = 0;



uint16_t dischar_temp_vol[DRV_BAT_TEMP_DATA_NUM] = {3500}; 

uint8_t dischar_temp_num = 0;
uint8_t dischar_drv_flag = 0;
uint8_t first_dischar_drv_flag = 0;




uint16_t temp_previous_data[DRV_BAT_TEMP_DATA_NUM] = {3500}; 



static TimerHandle_t ChargingTimer = NULL;    //timer



void * charging_timer_get(void)
{
	return ChargingTimer;
}

static void vTimerChargingCallback(TimerHandle_t xTimer)
{
		if(g_bat_evt.level_percent < 100)
				g_bat_evt.level_percent++;
		
//		SEGGER_RTT_printf(0,"vTimerChargingCallback(): g_bat_evt.level_percent = %u\n", g_bat_evt.level_percent);
}

void charging_timer_start(uint32_t second)
{
	 ChargingTimer = xTimerCreate("ChargingTimer", second * (TIMER_INIT_INTERVAL_MS / portTICK_PERIOD_MS),
																 pdTRUE,
																 NULL,
																 vTimerChargingCallback);
	
	if(NULL != ChargingTimer)
	{
		if(xTimerStart(ChargingTimer, 100) != pdPASS)
		{
			
		}
	}
//	SEGGER_RTT_printf(0,"charging_timer is created.\n");
}

/*
void charging_timer_stop(void)
{
	if(ChargingTimer != NULL)
		xTimerStop(ChargingTimer, 100);
}

void charging_timer_restart(void)
{
	if(ChargingTimer != NULL)
		xTimerStart(ChargingTimer, 100);
}
*/

void charging_timer_delete(void)
{
//	SEGGER_RTT_printf(0,"IN charging_timer_delete()\n");
	if(ChargingTimer != NULL)
	{
//		SEGGER_RTT_printf(0,"TimerActive..\n");
		xTimerDelete(ChargingTimer, 100);
//		SEGGER_RTT_printf(0,"ChargingTimer is %x..\n", ChargingTimer);
		ChargingTimer = NULL;
	}
}

uint8_t system_voltage_flag_get(void)
{
	return system_first_get_vol;
}

void system_voltage_flag_set(void)
{
	system_first_get_vol = 1;
}

void system_voltage_flag_clear(void)
{
	system_first_get_vol = 0;
}


uint32_t drv_battery_save_adc(void)
{
	return m_previous_pct;
}

void drv_bat_adc_enable(void)
{
	am_hal_adc_config_t sADCConfig;
	
	am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_ADC);

	am_hal_gpio_pin_config(m_drv_bat.param.adc_pin, AM_HAL_PIN_34_ADCSE6);
	
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
	am_hal_gpio_pin_config(m_drv_bat.param.adc_pin, AM_HAL_PIN_DISABLE);
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

//放电曲线
static uint8_t batt_voltage_to_percent_release(uint16_t voltage_mv)
	{
				uint8_t battery_level_percent;
			//DRV_BATTERY_LOG_PRINTF("[DRV_BATTERY]:batt_voltage_to_percent_release --> %d\n",voltage_mv);
			
			if (voltage_mv >= 4284)   //电压上限
			{
				battery_level_percent = 100;
			}
				else if (voltage_mv >= 4015 && voltage_mv < 4284)	//第一段曲线
				{
						battery_level_percent = (100 - (uint8_t)(22.5 * (4284 - voltage_mv) / (4284 - 4015)));
				}
				else if (voltage_mv >= 3954 && voltage_mv < 4015)	 //第二段
				{
						battery_level_percent = (100 - 22.5 - (uint8_t)(4.5 * (4015 - voltage_mv) / (4015 - 3954)));
				}
				else if (voltage_mv >= 3678 && voltage_mv < 3954)	 //第三段
				{
						battery_level_percent = (100 - 22.5 - 4.5 - (uint8_t)(58 * (3954 - voltage_mv) / (3954 - 3678)));
				}
				else if (voltage_mv > 3612 && voltage_mv < 3678)	//第四段
				{
	
						battery_level_percent = (100 - 22.5 - 4.5 - 58 - (uint8_t)(10 * (3678 - voltage_mv) / (3678 - 3612)));
	
					
	
				}
				else if(voltage_mv > 3400 && voltage_mv < 3612)
				{
                       battery_level_percent = (100 - 22.5 - 4.5 - 58 - 10 -  (uint8_t)(5 * (3612 - voltage_mv) / (3612 - 3400)));

				} 
			else if (voltage_mv <= 3400)  //电压下限
			{
				battery_level_percent = 0;
			}
				
				if (PERCENT_REC != 0 && battery_level_percent > PERCENT_REC)	//防止充电后切换到放电模式电量低的情况。
					{
					battery_level_percent = PERCENT_REC;
				//	DRV_BATTERY_LOG_PRINTF("[DRV_BATTERY]:batt_voltage_to_percent_release --> %d\n",battery_level_percent);
					}
				//DRV_BATTERY_LOG_PRINTF("[DRV_BATTERY]:batt_voltage_to_percent_release --> %d\n",battery_level_percent);
				return battery_level_percent;
		}

 
//充电曲线
static uint8_t batt_voltage_to_percent_charge(uint16_t voltage_mv)
	{
		uint8_t battery_level_percent;
		if(voltage_mv <= 3445)	 //电压下限
	  {
		  battery_level_percent = 0;
	  }
		else if(voltage_mv > 3445 && voltage_mv <= 3721)   //第一段曲线
		{
				battery_level_percent = (uint8_t)(4.9 * (voltage_mv - 3445) / (3721 - 3445));
		}
		else if(voltage_mv > 3721 && voltage_mv <= 3806)
		{
				battery_level_percent = 4.9 + (uint8_t)(15.1 * (voltage_mv - 3721) / (3806 - 3721));
		}
		else if(voltage_mv > 3806 && voltage_mv <= 4095)
			
		{
		
		battery_level_percent = 4.9 + 15.1 + (uint8_t)(50.7 * (voltage_mv - 3806) / (4095 - 3806));//70.7
				
		}
		else if(voltage_mv > 4095 && voltage_mv <= 4284)
			
		{
		
		battery_level_percent = 4.9 + 15.1 + 50.7+(uint8_t)(21.3* (voltage_mv - 4095) / (4284 - 4095));//92
				
		}
		
		else if(voltage_mv > 4284)
		{		
			if(g_bat_evt.level_percent != 0)
			{
				if(ChargingTimer == NULL && g_bat_evt.level_percent < 100)
					charging_timer_start(3.0 * TIMER_PERIOD);
				
				if(g_bat_evt.level_percent == 100 && ChargingTimer != NULL)
					charging_timer_delete();
				
				battery_level_percent = g_bat_evt.level_percent;
				//SEGGER_RTT_printf(0,"(9):battery_level_percent=%d\n", battery_level_percent);
			}
			else
			{
				battery_level_percent = 4.9 + 15.1 + 50.7  + 21.3 + (uint8_t)(8.0 * (voltage_mv - 4284) / (4350 - 4284));
			}
		}
		DRV_BATTERY_LOG_PRINTF("voltage_mv:%d\r\n",voltage_mv);
		
		if (battery_level_percent < PERCENT_REC)
			battery_level_percent = PERCENT_REC;
		
		return battery_level_percent;
	}

 





/*小往大排列*/
void drv_bubble_sort(uint16_t *data, int n) 
{
    int i, j;
    uint16_t temp;
    for (j = 0; j < n - 1; j++)
    {
       for (i = 0; i < n - 1 - j; i++)
       {
          if(data[i] > data[i + 1])
          {
             temp=data[i];
             data[i]=data[i+1]; 
             data[i+1]=temp;
          }
       }
    }
}

void drv_bat_management(uint32_t *tmp_adc)
{
		uint16_t tmp_vol;		
		uint8_t tmp_pct,i;
		uint32_t temp_data_sum;
        uint8_t tmp_pct_b,tmp_pct_r;
		
		if(m_charging_sec != 0)
		{	
			//充电	
			dischar_temp_num = 0;
			dischar_drv_flag = 0;
			adc_to_batt_voltage(*tmp_adc, &tmp_vol);
			
			charge_temp_vol[charge_temp_num%DRV_BAT_TEMP_DATA_NUM] = tmp_vol;
           if(charge_drv_flag==1)
				{
                  for(i=0;i<DRV_BAT_TEMP_DATA_NUM;i++)
                     {
                       temp_previous_data[i] = charge_temp_vol[i];
					 }
				   drv_bubble_sort(temp_previous_data,DRV_BAT_TEMP_DATA_NUM);//从小往大排列
		           temp_data_sum = 0;
				   for(i=1;i<DRV_BAT_TEMP_DATA_NUM-6;i++)
					 {
					  temp_data_sum += temp_previous_data[i];
					 }
					  tmp_vol = temp_data_sum/8;
			           tmp_pct = batt_voltage_to_percent_charge(tmp_vol);
                       
					   shutdown_val = tmp_vol;
			       }
			else
				{
					if((charge_temp_num==0)&&(first_charge_drv_flag==0))
						{
						  
						  tmp_pct_r = m_previous_pct;
						  tmp_pct_b = batt_voltage_to_percent_charge(tmp_vol);
						  first_charge_drv_flag = 1;
	                      	if(tmp_pct_b > tmp_pct_r)
					       {
	                          tmp_pct = tmp_pct_r;
					        }
					      else
					       {
	                    
						    tmp_pct = tmp_pct_b;
					       }

					    }
					else
						{
					      tmp_pct =  g_bat_evt.level_percent;
					    }
					
					shutdown_val = tmp_vol;
					
			   }
			
          
			
			if(1 == system_voltage_flag_get())				//系统第一次上电。
			{		
				if(tmp_pct > m_previous_pct)
					m_previous_pct = tmp_pct;
				
				g_bat_evt.level_percent = m_previous_pct;				
			}
			else
			{
				if(tmp_pct < m_previous_pct)           //防止充电过程中电量百分比下降
				{
					g_bat_evt.level_percent = m_previous_pct;
				}
				else
				{
					g_bat_evt.level_percent = tmp_pct;
				}
			}

			   charge_temp_num++;	
                if(charge_temp_num==DRV_BAT_TEMP_DATA_NUM)
                  {
                     charge_drv_flag=1;
				  }
				charge_temp_num = charge_temp_num%DRV_BAT_TEMP_DATA_NUM;
			PERCENT_REC = g_bat_evt.level_percent;
		}
		else
		{		

		    charge_temp_num = 0;
			charge_drv_flag = 0;
			//放电	
			adc_to_batt_voltage(*tmp_adc, &tmp_vol);
			
			dischar_temp_vol[dischar_temp_num%DRV_BAT_TEMP_DATA_NUM] = tmp_vol;
			if(dischar_drv_flag==1)
				{
                  for(i=0;i<DRV_BAT_TEMP_DATA_NUM;i++)
                     {
                       temp_previous_data[i] = dischar_temp_vol[i];

					 }
				   drv_bubble_sort(temp_previous_data,DRV_BAT_TEMP_DATA_NUM);//从小往大排列
		           temp_data_sum = 0;
				   for(i=5;i<DRV_BAT_TEMP_DATA_NUM-2;i++)
					 {
					  temp_data_sum += temp_previous_data[i];
					 }
					  tmp_vol = temp_data_sum/8;
				
								    		
			    tmp_pct = batt_voltage_to_percent_release(tmp_vol);
                 
				 shutdown_val = tmp_vol;
			

			  }
			else
				{
					if((dischar_temp_num==0)&&(first_dischar_drv_flag==0))
						{
						
						tmp_pct_r =  m_previous_pct;
						tmp_pct_b = batt_voltage_to_percent_release(tmp_vol);
						first_dischar_drv_flag = 1;
	                      	if(tmp_pct_b > tmp_pct_r)
					       {
	                          tmp_pct = tmp_pct_r;
					        }
					      else
					       {
	                    
						    tmp_pct = tmp_pct_b;
					       }

					    }
					else
						{

                     
					 tmp_pct =	g_bat_evt.level_percent;


					   }
					
					shutdown_val = tmp_vol;
						
			   }

			
			if(1 == system_voltage_flag_get())				//系统第一次上电。
			{
				if(tmp_pct < m_previous_pct)
					m_previous_pct = tmp_pct;
				
				g_bat_evt.level_percent = m_previous_pct;
				
			}
			else
			{
				
				
				if(tmp_pct > m_previous_pct)               //防止放电过程中电量百分比增加
				{
					g_bat_evt.level_percent = m_previous_pct;
				}
				else
				{
					g_bat_evt.level_percent = tmp_pct;
				}
			}
			dischar_temp_num++;	
                if(dischar_temp_num==DRV_BAT_TEMP_DATA_NUM)
                	{
                     dischar_drv_flag=1;
					 
				  }
				dischar_temp_num = dischar_temp_num%DRV_BAT_TEMP_DATA_NUM;
			PERCENT_REC = g_bat_evt.level_percent;
		}
        
		m_previous_pct = g_bat_evt.level_percent;	//记录percent
}


void am_adc_isr(void)
{
		uint32_t ui32Status;		
		
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
	
/********************************************************************************
		if (ui32Status & AM_HAL_ADC_INT_WCINC)
		{
			DRV_BATTERY_LOG_PRINTF("[DRV_BATTERY]:AM_HAL_ADC_INT_WCINC\n");
		}
		else if (ui32Status & AM_HAL_ADC_INT_WCEXC)
		{
			DRV_BATTERY_LOG_PRINTF("[DRV_BATTERY]:AM_HAL_ADC_INT_WCEXC\n");
		}
		else if (ui32Status & AM_HAL_ADC_INT_FIFOOVR2)
		{
			DRV_BATTERY_LOG_PRINTF("[DRV_BATTERY]:AM_HAL_ADC_INT_FIFOOVR2\n");
		}	
		else if (ui32Status & AM_HAL_ADC_INT_FIFOOVR1)
		{
			DRV_BATTERY_LOG_PRINTF("[DRV_BATTERY]:AM_HAL_ADC_INT_FIFOOVR1\n");
		}			
		else if (ui32Status & AM_HAL_ADC_INT_SCNCMP)
		{
			DRV_BATTERY_LOG_PRINTF("[DRV_BATTERY]:AM_HAL_ADC_INT_SCNCMP\n");
		}						
    else 
************************************************************************************/		
		if (ui32Status & AM_HAL_ADC_INT_CNVCMP)
    {
			//DRV_BATTERY_LOG_PRINTF("[DRV_BATTERY]:AM_HAL_ADC_INT_CNVCMP\n");
			//
			// Read the value from the FIFO.
			//
			
			TMP_ADC = (am_hal_adc_fifo_pop()>>6)&0x3FFF;  //高14位有效
			adc_disable();
		
			bool is_charging = false;
						
			//DRV_BATTERY_LOG_PRINTF("[DRV_BATTERY]:actual adc --> %d\n",tmp_adc);
		//	DRV_BATTERY_LOG_PRINTF("[DRV_BATTERY]:m_previous_pct --> %d\n",m_previous_pct);	

			if(m_charging_sec != 0)
			{													
							//充电状态
				is_charging = true;
			}
			
			if(is_charging)
			{
				g_bat_evt.type = DRV_BAT_EVENT_CHARGING;
			}
			else
			{		
				g_bat_evt.type = DRV_BAT_EVENT_DISCHARGING;
			}
									
			if(m_drv_bat.evt_handler != NULL)
			{
				m_drv_bat.evt_handler(&g_bat_evt);
			}					
		}
}

static void delay(void)          //T: 1 / 4.8^10(7)
{
	for(int i = 0; i < 1920; i++)
	{
		for(int j = 0; j < 1000; j++)
			;
	}
}


static void usb_detect_hdl(void)
{	
	delay();
//	DRV_BATTERY_LOG_PRINTF("[DRV_BATTERY]:usb_detect_hdl\n");
	
	if (am_hal_gpio_input_bit_read(m_drv_bat.param.usb_detect_pin))	
	{
		//DRV_BATTERY_LOG_PRINTF("[DRV_BATTERY]:high\n");
		if (am_hal_gpio_input_bit_read(m_drv_bat.param.batt_chg_stat_pin))
		{
			g_bat_evt.charge_detect_full = true;
		}
		else
		{
			g_bat_evt.charge_detect_full = false;
		}		
		g_bat_evt.type = DRV_BAT_EVENT_USB_CONN_CHARGING;
		g_bat_evt.cable_status = DRV_BAT_EVENT_USB_CONN_CHARGING;
		am_hal_rtc_time_get(&RTC_time);
		m_charging_sec = RTC_time.ui32Hour*3600+RTC_time.ui32Minute*60+RTC_time.ui32Second;
		am_hal_gpio_int_polarity_bit_set(m_drv_bat.param.usb_detect_pin,AM_HAL_GPIO_FALLING);
	}
	else
	{

	//	DRV_BATTERY_LOG_PRINTF("[DRV_BATTERY]:low\n");
		g_bat_evt.type = DRV_BAT_EVENT_USB_DISCONN;
		g_bat_evt.cable_status = DRV_BAT_EVENT_USB_DISCONN;
		g_bat_evt.charge_detect_full = false;
		m_charging_sec = 0;
		//m_adc_charging_gain = 0;
		//m_adc_charging_gain_flag = false;
	
		am_hal_gpio_int_polarity_bit_set(m_drv_bat.param.usb_detect_pin,AM_HAL_GPIO_RISING);

	}
	if(m_drv_bat.evt_handler != NULL)
	{
		m_drv_bat.evt_handler(&g_bat_evt);
	}	
}

void drv_bat_init(_drv_bat *p_drv_bat, bool *charging_s)
{
	//DISPLAY_MSG  msg = {0,0};

	m_drv_bat.evt_handler = p_drv_bat->evt_handler;
	m_drv_bat.param.adc_pin = p_drv_bat->param.adc_pin;
	m_drv_bat.param.batt_chg_stat_pin = p_drv_bat->param.batt_chg_stat_pin;
	m_drv_bat.param.usb_detect_pin = p_drv_bat->param.usb_detect_pin;
	m_drv_bat.param.batt_voltage_limit_low = p_drv_bat->param.batt_voltage_limit_low;
	m_drv_bat.param.batt_voltage_limit_full = p_drv_bat->param.batt_voltage_limit_full;
	m_drv_bat.param.voltage_divider.r_1_ohm = p_drv_bat->param.voltage_divider.r_1_ohm;
	m_drv_bat.param.voltage_divider.r_2_ohm = p_drv_bat->param.voltage_divider.r_2_ohm;
	m_battery_divider_factor = m_drv_bat.param.voltage_divider.r_2_ohm /
										 (float)(m_drv_bat.param.voltage_divider.r_1_ohm +
														 m_drv_bat.param.voltage_divider.r_2_ohm);	
  am_hal_gpio_pin_config(m_drv_bat.param.batt_chg_stat_pin,AM_HAL_GPIO_INPUT);
	am_hal_gpio_int_register(m_drv_bat.param.usb_detect_pin, usb_detect_hdl);
	am_hal_gpio_pin_config(m_drv_bat.param.usb_detect_pin,AM_HAL_GPIO_INPUT);
	am_hal_gpio_int_polarity_bit_set(m_drv_bat.param.usb_detect_pin,p_drv_bat->param.usb_detect_pin_polarity);
	am_hal_gpio_int_clear(AM_HAL_GPIO_BIT(m_drv_bat.param.usb_detect_pin));
	am_hal_gpio_int_enable(AM_HAL_GPIO_BIT(m_drv_bat.param.usb_detect_pin));											 
	
	
	m_previous_pct = p_drv_bat->bat_pct;
	system_voltage_flag_set();
	
	drv_bat_adc_enable();
										 
	if(*charging_s)
	{
		CHARGE_PWON = 1;
		ScreenStateSave = DISPLAY_SCREEN_HOME;
		ScreenState = DISPLAY_SCREEN_NOTIFY_BAT_CHG;
		DISPLAY_MSG  msg = {0,0};
		msg.cmd = MSG_DISPLAY_SCREEN;
		xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
																				//
		m_charging_sec = 0;									//关机充电时先获取一次放电电量，防止电量显示0%。
		timer_battery_start(500);		        //
		*charging_s = false;
	}
	else	
	{
		DISPLAY_MSG  msg = {0,0};
		ScreenState = DISPLAY_SCREEN_HOME;
		msg.cmd = MSG_DISPLAY_SCREEN;
		xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
	}
}







