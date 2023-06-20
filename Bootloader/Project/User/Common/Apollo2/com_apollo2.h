#ifndef __COM_APOLLO2_H__
#define __COM_APOLLO2_H__



#include <stdint.h>
#include "am_mcu_apollo.h"



#define APOLLO2_FLASH_SIZE       0x100000

#define PIN_CONFIG(pin,dir)    				am_hal_gpio_pin_config(pin, dir);
#define PIN_SET(pin)           				am_hal_gpio_out_bit_set(pin)
#define PIN_CLEAR(pin)         				am_hal_gpio_out_bit_clear(pin)
#define PIN_READ_INPUT(pin)    				am_hal_gpio_input_bit_read(pin)
#define PIN_READ_INT_POLARITY(pin)    am_hal_gpio_int_polarity_bit_get(pin)
#define PIN_READ_OUTPUT(pin)   				am_hal_gpio_out_bit_read(pin)
#define PIN_TOGGLE(pin)        				am_hal_gpio_out_bit_toggle(pin)




#define CRITICAL_REGION_ENTER() AM_CRITICAL_BEGIN_ASM
#define CRITICAL_REGION_EXIT() AM_CRITICAL_END_ASM



typedef void (* uart0_callback)(uint8_t *p_data);


void com_apollo2_gpio_int_uninit(uint32_t pin);
void com_apollo2_gpio_int_init(uint32_t pin, uint32_t polarity);
void uart0_handler_register(uart0_callback uart0_cb);





#endif //__COM_APOLLO2_H__
