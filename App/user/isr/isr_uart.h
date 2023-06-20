#ifndef _ISR_UART_H__
#define _ISR_UART_H__

#include <stdint.h>
#include <stdbool.h>
extern void SetHdrFlag(bool isEnable);
extern bool GetHdrFlag(void);


typedef void (* uart0_callback)(uint8_t *p_data);
void uart0_handler_register(uart0_callback uart0_cb,bool is_pc);
void uart0_handler_unregister(void);


#endif
