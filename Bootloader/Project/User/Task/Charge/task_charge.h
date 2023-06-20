#ifndef __TASK_CHARGE_H__
#define __TASK_CHARGE_H__


#include "board.h"


#define TASK_CHARGE_ADC_PIN                  					 BSP_APOLLO2_TASK_CHARGE_ADC_PIN
#define TASK_CHARGE_USB_DETECT_PIN           					 BSP_APOLLO2_TASK_CHARGE_USB_DETECT_PIN
#define TASK_CHARGE_BAT_CHG_STATE_PIN        					 BSP_APOLLO2_TASK_CHARGE_BAT_CHG_STATE_PIN

//#define TASK_CHARGE_ADC_PIN                  					 34
//#define TASK_CHARGE_USB_DETECT_PIN           					 25
//#define TASK_CHARGE_BAT_CHG_STATE_PIN        					 31


void task_charge_init(void);






#endif //__TASK_CHARGE_H__
