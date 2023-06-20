#ifndef __DF_CONVERSION_H__
#define __DF_CONVERSION_H__


#include "drv_lcd.h"


typedef struct {

	uint16_t height;//num of rows
	uint16_t width;//num of bytes in a column
	uint8_t bg_color;//background color
	uint8_t fg_color;//foreground color
	uint8_t *dot;//dot data pointer
}_df_dot_param;

void df_conversion_color(_drv_lcd_graph *graph, _df_dot_param *dot_param);



#endif //__DF_CONVERSION_H__
