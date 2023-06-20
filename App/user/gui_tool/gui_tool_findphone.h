#ifndef __GUI_TOOL_FINDPHONE_H__
#define __GUI_TOOL_FINDPHONE_H__





#include <stdint.h>


void gui_tool_findphone_ready_paint(void);
void gui_tool_findphone_ready_btn_evt(uint32_t Key_Value);

void gui_tool_findphone_paint(void);
void gui_tool_findphone_btn_evt(uint32_t Key_Value);

void gui_tool_findphone_timer_init(void);
void gui_tool_findphone_timer_uninit(void);
void gui_tool_findphone_timer_start(uint32_t timeout_msec);
void gui_tool_findphone_timer_stop(void);



#endif //__GUI_TOOL_FINDPHONE_H__
