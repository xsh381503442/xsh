#ifndef GUI_STEP_H
#define	GUI_STEP_H
#include <stdint.h>
#include <stdbool.h>
//Display
extern void gui_step_paint(void);
extern void gui_step_value_paint(void);
extern void gui_step_week_steps_paint(void);
extern void gui_step_week_mileage_paint(void);
extern void gui_step_calibration_hint_paint(void);
extern void gui_step_calibration_show_paint(float mile,uint32_t step,float stepLength);
extern void gui_step_calibration_confirm_save_paint(float mile,uint32_t step,float stepLength);
//Key Even
extern void gui_step_btn_evt(uint32_t Key_Value);
extern void gui_step_week_steps_btn_evt(uint32_t Key_Value);
extern void gui_step_week_mileage_btn_evt(uint32_t Key_Value);
extern void gui_step_calibration_hint_btn_evt(uint32_t Key_Value);
extern void gui_step_calibration_show_btn_evt(uint32_t Key_Value);
extern void gui_step_calibration_confirm_save_btn_evt(uint32_t Key_Value);

extern bool GetCalDistanceStatus(void);
extern void SetCalDistanceStatus(bool flag);
extern uint32_t get_weekmain_max_distance(void);
extern uint32_t get_week_common_distance(uint8_t day);
extern uint32_t get_week_sport_distance(uint8_t day);
extern void get_weekSteps(void);

extern float stepStride;
extern uint8_t menuIndex;
#endif

