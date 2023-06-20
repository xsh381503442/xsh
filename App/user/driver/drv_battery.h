#ifndef __DRV_BATTERY_H__
#define __DRV_BATTERY_H__



#include "am_mcu_apollo.h"
#include "am_util.h"


#include <stdint.h>
#include "watch_config.h"


#define DRV_BAT_ADC_PIN                  					 34
#if defined( WATCH_MODEL_PLUS ) 
 #define DRV_BAT_USB_DETECT_PIN           					 2
 #define DRV_BAT_CHG_STATE_PIN           					 36
#else
#ifdef WATCH_COD_BSP 
#define DRV_BAT_USB_DETECT_PIN           					 16
#else
#define DRV_BAT_USB_DETECT_PIN           					 25
#endif
#define DRV_BAT_CHG_STATE_PIN           					 31
#endif

#define USB_DETECT_PIN_CONNECT_POLARITY        AM_HAL_GPIO_RISING       //connect       
#define USB_DETECT_PIN_DISCONNECT_POLARITY     AM_HAL_GPIO_FALLING       //disconnect 

#define DRV_BAT_EVENT_DATA                          1               
#define DRV_BAT_EVENT_10_PERCENT                    2  
#define DRV_BAT_EVENT_5_PERCENT                     3
#define DRV_BAT_EVENT_3_PERCENT                     4
#define DRV_BAT_EVENT_FULL                          5                         
#define DRV_BAT_EVENT_USB_CONN_CHARGING             6            
#define DRV_BAT_EVENT_USB_CONN_CHARGING_FINISHED    7   
#define DRV_BAT_EVENT_USB_DISCONN                   8   
#define DRV_BAT_EVENT_CHARGING											9
#define DRV_BAT_EVENT_DISCHARGING										10


#define DRV_BAT_TEMP_DATA_NUM  15

typedef struct
{
    uint32_t                    type;    
		uint32_t                    cable_status;
    uint16_t                    voltage_mv;         
    uint8_t                     level_percent;  
		uint8_t                     usb_detect_pin_polarity; 
		uint8_t                     temperature;
		bool                        charge_detect_full;
}_drv_bat_event;



typedef void (*bat_callback)(_drv_bat_event * p_event);

typedef struct
{
    uint32_t r_1_ohm;
    uint32_t r_2_ohm;
}voltage_divider_t;

typedef struct{
    uint8_t                         adc_pin;                     
    uint8_t                         usb_detect_pin;    
		uint8_t                         usb_detect_pin_polarity;
    uint8_t                         batt_chg_stat_pin;               
    uint16_t            						batt_voltage_limit_low;        
    uint16_t            						batt_voltage_limit_full;       
    voltage_divider_t   						voltage_divider;                

}_drv_bat_param;


typedef struct{
	
    _drv_bat_param              param;
		bat_callback                evt_handler;       
		uint8_t                    bat_pct;
}_drv_bat;

extern uint32_t TMP_ADC;
extern long m_charging_sec;
extern uint8_t CHARGE_PWON;

void drv_bat_init(_drv_bat *p_drv_bat, bool *charging_s);
void drv_bat_adc_enable(void);
uint32_t drv_battery_save_adc(void);
extern void drv_bat_management(uint32_t *tmp_adc);
extern void charging_timer_start(uint32_t second);
extern void charging_timer_delete(void);
extern void* charging_timer_get(void);

extern uint8_t system_voltage_flag_get(void);
extern void system_voltage_flag_set(void);
extern void system_voltage_flag_clear(void);


#endif //__DRV_BATTERY_H__
