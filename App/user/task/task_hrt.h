#ifndef __TASK_HRT_H__
#define __TASK_HRT_H__

#include <stdint.h>
#include <stdbool.h>

extern uint8_t hr_on;

void task_hrt_init(void);
void task_hrt_handler(uint32_t evt, bool is_isr, uint8_t *p_data, uint32_t len);
extern void task_ohr_timer_stop(bool is_isr);

extern void task_hrt_start(void);
extern void task_hrt_close(void);










#endif //__TASK_HRT_H__
