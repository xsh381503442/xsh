#ifndef	GUI_TOOL_COUNTDOWN_H
#define	GUI_TOOL_COUNTDOWN_H


#define GUI_TOOL_COUNTDOWN_MENU_TIME	0 //倒计时设置时间选项
#define GUI_TOOL_COUNTDOWN_MENU_START	1  //倒计时开始选项


extern uint32_t tool_countdown_real_time_get(void);
extern uint32_t tool_countdown_target_time_get(void);
extern void tool_countdown_finish(void);

extern void gui_tool_countdown_init(void);
extern void set_tool_countdown_index(uint8_t index);
//Display
//extern void gui_tool_countdown_paint(void);
extern void gui_tool_countdown_setting_paint(void);
extern void gui_tool_countdown_menu_paint(void);
extern void gui_tool_countdown_time_paint(void);
extern void gui_tool_countdown_exit_paint(void);
extern void gui_tool_countdown_finish_paint(void);

//Key Event
extern void gui_tool_countdown_btn_evt(uint32_t Key_Value);
extern void gui_tool_countdown_setting_btn_evt(uint32_t Key_Value);
extern void gui_tool_countdown_menu_btn_evt(uint32_t Key_Value);
extern void gui_tool_countdown_time_btn_evt(uint32_t Key_Value);
extern void gui_tool_countdown_exit_btn_evt(uint32_t Key_Value);
extern void gui_tool_countdown_finish_btn_evt(uint32_t Key_Value);

#endif
