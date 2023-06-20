#ifndef __DRV_BUTTON_H__
#define __DRV_BUTTON_H__







#include <stdint.h>
#include "board.h"





#define BTN_PIN_PWR 															BSP_APOLLO2_BTN_PIN_PWR
#define BTN_PIN_PWR_POLARITY 											BSP_APOLLO2_BTN_PIN_PWR_POLARITY
#define BTN_PIN_BL 																BSP_APOLLO2_BTN_PIN_BL
#define BTN_PIN_BL_POLARITY                       BSP_APOLLO2_BTN_PIN_BL_POLARITY
#define BTN_PIN_UP                                BSP_APOLLO2_BTN_PIN_UP
#define BTN_PIN_UP_POLARITY                       BSP_APOLLO2_BTN_PIN_UP_POLARITY
#define BTN_PIN_OK                                BSP_APOLLO2_BTN_PIN_OK
#define BTN_PIN_OK_POLARITY                       BSP_APOLLO2_BTN_PIN_OK_POLARITY
#define BTN_PIN_DOWN                              BSP_APOLLO2_BTN_PIN_DOWN
#define BTN_PIN_DOWN_POLARITY                     BSP_APOLLO2_BTN_PIN_DOWN_POLARITY


//#define BTN_PIN_PWR 24
//#define BTN_PIN_PWR_POLARITY 0
//#define BTN_PIN_BL 39
//#define BTN_PIN_BL_POLARITY 0
//#define BTN_PIN_UP 28
//#define BTN_PIN_UP_POLARITY 0
//#define BTN_PIN_OK 26
//#define BTN_PIN_OK_POLARITY 0
//#define BTN_PIN_DOWN 47
//#define BTN_PIN_DOWN_POLARITY 0



#define BTN_PIN_UP_DOWN 100

typedef void (* button_callback)(uint32_t evt);


void drv_button_init(button_callback task_cb);




#endif //__DRV_BUTTON_H__
