#ifndef GUI_TOOL_H
#define GUI_TOOL_H

#include "task_display.h"

typedef struct{
	uint8_t x_axis;		//Destination X coordinate
	uint8_t y_axis;		//Destination Y coordinate				
}Index_axis;

extern ScreenState_t Get_Curr_Tool_Index(void);
extern ScreenState_t Get_Tool_Index(uint8_t Num);

extern ScreenState_t Set_Tool_Index( ScreenState_t ScreenSta );
extern void Set_Select_Tool_Index(void);
extern ScreenState_t Set_Curr_Tool_Index( ScreenState_t ScreenSta);
extern void CreateToolDisplay(void);

//Display
extern void gui_tool_select_paint(ScreenState_t state);
extern void gui_tool_function_menu_paint(void);
extern void gui_tool_add_menu_paint(void);

//Key Event
extern void gui_tool_switch(uint32_t Key_Value);
extern void gui_tool_btn_evt(uint32_t Key_Value);
extern void gui_tool_function_menu_btn_evt(uint32_t Key_Value);
extern void gui_tool_add_menu_btn_evt(uint32_t Key_Value);
   
extern uint8_t is_custom_tool(ScreenState_t index);
extern ScreenState_t tool_menu_index_record(void);
extern uint8_t ToolIndexRecAdjust(void);
extern ScreenState_t AdjustToolIndex(void);


#endif

