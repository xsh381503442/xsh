#include "gui_config.h"
#include "gui_pointing.h"

#include "drv_lcd.h"

#include "font_config.h"

#if DEBUG_ENABLED == 1 && GUI_POINTING_LOG_ENABLED == 1
	#define GUI_POINTING_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_POINTING_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define GUI_POINTING_WRITESTRING(...)
	#define GUI_POINTING_PRINTF(...)		        
#endif

void gui_page_up(uint8_t color)
{
	//上翻提示
	LCD_SetPicture(10, LCD_CENTER_JUSTIFIED, color, LCD_NONE, &Img_Pointing_Up_12X8);
}

void gui_page_down(uint8_t color)
{
	//下翻提示
	LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, color, LCD_NONE, &Img_Pointing_Down_12X8);
}

void gui_button_up(uint8_t color)
{
	//up键提示
	LCD_SetPicture(116, 10, color, LCD_NONE, &Img_Pointing_Left_12X8);
}

void gui_button_down(uint8_t color)
{
	//down键提示
	LCD_SetPicture(116, 10, color, LCD_NONE, &Img_Pointing_Left_12X8);
}

void gui_pointing_ok(uint8_t color)
{
	//ok键提示
	LCD_SetPicture(116, 10, color, LCD_NONE, &Img_Pointing_Left_12X8);
}

