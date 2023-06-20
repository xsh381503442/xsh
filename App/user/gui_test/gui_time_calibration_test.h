#ifndef __GUI_TIME_CALIBRATION_TEST_H__
#define __GUI_TIME_CALIBRATION_TEST_H__

#include <stdint.h>
#include "algo_config.h"

extern void gui_time_calibration_realtime_test_paint(void);
extern void gui_time_calibration_status_test_paint(void);
#if defined(WATCH_PRECISION_TIME_CALIBRATION)
extern void gui_time_realtime_difftime_paint(void);
#endif
extern void gui_time_calibration_realtime_test_evt(uint32_t Key_Value);
extern void gui_time_calibration_status_test_evt(uint32_t Key_Value);
extern void gui_time_realtime_difftime_evt(uint32_t Key_Value);
#if defined(WATCH_AUTO_BACK_HOME)
extern void gui_post_auto_back_home_paint(void);
extern void gui_post_auto_back_home_btn_evt(uint32_t Key_Value);
#endif
#endif

