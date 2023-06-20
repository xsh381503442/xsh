#ifndef __GUI_LACTATE_THRESHOLD_H__
#define __GUI_LACTATE_THRESHOLD_H__

#include <stdint.h>
#include "algo_hdr.h"

//display
extern void gui_lactate_threshold_value_paint(uint8_t hdr_value,uint32_t pace);
extern void gui_lactate_threshold_hint_paint(void);
extern void gui_lactate_mesuring_paint(float mile,Countdown_time_t countdown_time,uint8_t hdrValue,float min_km);

//key
extern void gui_lactate_threshold_value_btn_evt(uint32_t Key_Value);
extern void gui_lactate_threshold_hint_btn_evt(uint32_t Key_Value);
#endif
