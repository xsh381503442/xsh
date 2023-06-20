#ifndef __GUI_CUSTOMIZED_TOOLS_H__
#define __GUI_CUSTOMIZED_TOOLS_H__



#include <stdint.h>
#include <stdbool.h>


typedef enum
{
VALUE_NOT_CHANGE = 0,//值没有被修改
VALUE_BY_CHANGE
}VALUE_CHANGE_FLAG;




void gui_customized_tools_paint(void);
void gui_customized_tools_btn_evt(uint32_t evt);

void gui_customized_tools_hint_paint(void);
void gui_customized_tools_hint_btn_evt(uint32_t evt);





#endif //__GUI_CUSTOMIZED_TOOLS_H__
