#ifndef GUI_HEARTRATE_H
#define	GUI_HEARTRATE_H
#include <stdint.h>
#include "gui_home_config.h"

extern void gui_heartrate_data_init(void);
extern uint8_t gui_heartrate_statu_get(void);
extern void gui_heartrate_wave_paint(void);
//Display
extern void gui_heartrate_average_based_paint(void);
extern void gui_heartrate_hint_measuring_paint(void);
extern void gui_heartrate_paint(uint8_t value);
extern void gui_heartrate_stable_paint(uint8_t hdrValue);

extern void gui_monitor_hdr_time_explain_paint(void);
extern void DisplayPostHdr( uint8_t hdr_value );
extern void DisplayPostSettings(void);
extern void DisplayPostFactoryReset(void);
extern void DisplayPostHdr_SNR( uint8_t hdr_value );

//Key Even
extern void gui_heartrate_average_based_btn_evt(uint32_t Key_Value);
extern void gui_heartrate_hint_measuring_btn_evt(uint32_t Key_Value);
extern void gui_heartrate_btn_evt(uint32_t Key_Value);
extern void gui_heartrate_stable_btn_evt(uint32_t Key_Value);

extern void gui_monitor_hdr_time_explain_btn_evt(uint32_t Key_Value);
extern void DisplayPostHdr_btn_evt(uint32_t Key_Value);
extern void DisplayPostSettings_btn_evt(uint32_t Key_Value);
extern void DisplayPostFactoryReset_btn_evt(uint32_t Key_Value);
extern void gui_tick_cross(void);
extern void DisplayPostHdr_SNR_btn_evt(uint32_t Key_Value);

extern uint8_t stableHdr;
#endif

