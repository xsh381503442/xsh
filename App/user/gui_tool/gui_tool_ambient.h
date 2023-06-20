#ifndef	GUI_TOOL_AMBIENT_H
#define	GUI_TOOL_AMBIENT_H

typedef struct
{
	float min;			//下限
	float max;			//上限
	uint8_t number;		//刻度数
	float step;			//刻度值间距
}GraphStr;

extern void Get_Axes_Tick(GraphStr *graph);

//Display
extern void gui_tool_ambient_paint(float pres,float alt,float temp);
extern void gui_tool_ambient_value(float pres,float alt,float temp);
extern void gui_tool_ambient_menu_paint(void);
extern void gui_tool_ambient_pressure_paint(float pres);
extern void gui_tool_ambient_altitude_paint(float alt);
extern void gui_tool_ambient_altitude_calibration_paint(void);
extern void gui_tool_ambient_pressure_chart(void);

//Key Event
extern void gui_tool_ambient_btn_evt(uint32_t Key_Value);
extern void gui_tool_ambient_menu_btn_evt(uint32_t Key_Value);
extern void gui_tool_ambient_pressure_btn_evt(uint32_t Key_Value);
extern void gui_tool_ambient_altitude_btn_evt(uint32_t Key_Value);
extern void gui_tool_ambient_altitude_calibration_btn_evt(uint32_t Key_Value);

#endif
