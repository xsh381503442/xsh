#ifndef __DEV_BAT_H__
#define __DEV_BAT_H__



#include <stdint.h>


#define USB_DETECT_PIN_CONNECT_POLARITY        0       //connect       
#define USB_DETECT_PIN_DISCONNECT_POLARITY     1       //disconnect 

#define DEV_BAT_EVENT_DATA                          1               
#define DEV_BAT_EVENT_LOW                           2                        
#define DEV_BAT_EVENT_FULL                          3                         
#define DEV_BAT_EVENT_USB_CONN_CHARGING             4            
#define DEV_BAT_EVENT_USB_CONN_CHARGING_FINISHED    5   
#define DEV_BAT_EVENT_USB_DISCONN                   6   

typedef struct
{
    uint32_t                    type;    
		uint32_t                    cable_status;
    uint16_t                    voltage_mv;         
    uint8_t                     level_percent;  
		uint8_t                     usb_detect_pin_polarity; 
}_dev_bat_event;



typedef void (*bat_callback)(_dev_bat_event * p_event);

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

}_dev_bat_param;


typedef struct{
	
    _dev_bat_param              param;
		bat_callback                evt_handler;       
   
}_dev_bat;


void dev_bat_init(_dev_bat *p_dev_bat);
void dev_bat_adc_enable(void);

#endif //__DEV_BAT_H__
