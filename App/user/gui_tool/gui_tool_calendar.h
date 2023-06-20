#ifndef GUI_TOOL_CALENDAR_H
#define	GUI_TOOL_CALENDAR_H

#include <stdint.h>
#include "com_data.h"

/*typedef struct 
{
	uint8_t days;
	uint8_t day_index;
	
	AppWeatherSync appWeatherSync;
}AppWeatherInfo;*/

extern uint8_t gui_tool_calendar_weather_get(uint8_t year, uint8_t month, uint8_t day,uint8_t Hour );

//Display
extern void gui_tool_calendar_paint(void);
extern void gui_tool_calendar_date(void);
extern void gui_tool_calendar_weather_paint(void);
extern void gui_tool_calendar_weather_info_init(void);
extern uint16_t get_air_standard(uint16_t value_aqi);

//Key Event
extern void gui_tool_calendar_btn_evt(uint32_t Key_Value);
extern void gui_tool_calendar_weather_btn_evt(uint32_t Key_Value);
extern void gui_tool_calendar_weather_no_info(void);

//extern AppWeatherInfo m_weather_info;

#endif
