#ifndef __GUI_THEME_H__
#define __GUI_THEME_H__



#include <stdint.h>
#include "watch_config.h"

#define	GUI_THEME_NAME_SIZE				16

typedef struct
{
	char name[GUI_THEME_NAME_SIZE];	//主题名称
	uint8_t number;					//主题序号（对应存储外部flash中的空间）
}ThemeDataStr;

void gui_theme_init(uint8_t type);

void gui_theme_paint(void);
void gui_theme_btn_evt(uint32_t evt);

#if defined WACTH_THEME_APP_DOWNLOAD
void gui_theme_preview_paint(void);
void gui_theme_preview_btn_evt(uint32_t evt);	

void gui_theme_app_menu_paint(void);
void gui_theme_app_menu_btn_evt(uint32_t evt);

void gui_theme_app_success_paint(void);
void gui_theme_app_success_btn_evt(uint32_t evt);

void gui_theme_invalid_paint(void);
void gui_theme_invalid_btn_evt(uint32_t evt);
#endif

#endif //__GUI_THEME_H__
