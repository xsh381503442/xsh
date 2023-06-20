#ifndef __GUI_VO2MAX_H__
#define __GUI_VO2MAX_H__

#include <stdint.h>
#include "algo_hdr.h"
#include "watch_config.h"
//display
extern void gui_VO2max_value_paint(uint16_t VO2MaxValue);
extern void gui_VO2max_hint_mesuring_paint(void);
extern void gui_VO2max_countdown_paint(uint8_t value);
extern void gui_VO2max_mesuring_paint(float mile,Countdown_time_t countdown_time,uint8_t hdrValue,float min_km);
extern void gui_VO2max_lactate_eraly_end_hint_paint(void);
#if  defined WATCH_COM_SHORTCUNT_UP
extern void gui_end_unfinished_sport_hint_paint(uint8_t m_screenflag);
#else
extern void gui_end_unfinished_sport_hint_paint(void);
#endif
//key
extern void gui_VO2max_value_btn_evt(uint32_t Key_Value);
extern void gui_VO2max_hint_mesuring_btn_evt(uint32_t Key_Value);
extern void gui_VO2max_mesuring_btn_evt(uint32_t Key_Value);
extern void gui_VO2max_lactate_eraly_end_hint_btn_evt(uint32_t Key_Value);
extern void gui_end_unfinished_sport_hint_btn_evt(uint32_t Key_Value);
#endif

