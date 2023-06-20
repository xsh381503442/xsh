#ifndef GUI_TOOL_COMPASS_H
#define	GUI_TOOL_COMPASS_H

extern void gui_tool_compass_init(void);
extern void gui_tool_compass_bubble_set(uint16_t x, uint16_t y);

//Display
extern void gui_tool_compass_paint(float angle);
extern void gui_tool_compass_cali_paint(uint8_t type);
extern void gui_tool_compass_cail_update(uint8_t type);

//Key Event
extern void gui_tool_compass_btn_evt(uint32_t Key_Value);
extern void gui_tool_compass_cali_btn_evt(uint32_t Key_Value);

#endif

