#include "lib_gfx.h"
#include "rtt_log.h"
#include "lib_error.h"
#include "SEGGER_RTT.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include "drv_lcd.h"
#define MOUDLE_LOG_ENABLED 							0
#define MOUDLE_DEBUG_ENABLED   					0
#define MOUDLE_NAME                     "[LIB_GFX]:"

#if MOUDLE_LOG_ENABLED == 1 && RTT_LOG_ENABLED == 1
#define LOG_PRINTF(...)                      SEGGER_RTT_printf(0, ##__VA_ARGS__) 
#define LOG_HEXDUMP(p_data, len)             rtt_log_hexdump(p_data, len) 
#else
#define LOG_PRINTF(...) 
#define LOG_HEXDUMP(p_data, len)             
#endif

#if MOUDLE_DEBUG_ENABLED == 1 && RTT_LOG_ENABLED == 1
#define DEBUG_PRINTF(...)                    SEGGER_RTT_printf(0, ##__VA_ARGS__)
#define DEBUG_HEXDUMP(p_data, len)           rtt_log_hexdump(p_data, len)
#else
#define DEBUG_PRINTF(...)   
#define DEBUG_HEXDUMP(p_data, len) 
#endif



extern uint8_t g_lcd_ram[DRV_LCD_HIGHT*2][DRV_LCD_WIDTH/2];



static inline void pixel_draw(uint16_t x,
                              uint16_t y,
                              uint8_t color)
{
//    uint16_t lcd_width = DRV_LCD_WIDTH/2;
//    uint16_t lcd_height = DRV_LCD_HIGHT*2;

//    if ((x >= lcd_width) || (y >= lcd_height))
//    {
//        return;
//    }	
/**
		when y axis move one continously:
0x77 0x77 0x77 0x77 0x77 0x77 0x77 0x77 0x77 0x77				
0x77 0x77 0x77 0x77 0x77 0x77 0x77 0x77 0x77 0x77
								| |
		            | |
		            \ /
0x17 0x77 0x77 0x77 0x77 0x77 0x77 0x77 0x77 0x77				
0x17 0x77 0x77 0x77 0x77 0x77 0x77 0x77 0x77 0x77	
								| |
		            | |
		            \ /		
0x11 0x77 0x77 0x77 0x77 0x77 0x77 0x77 0x77 0x77				
0x11 0x77 0x77 0x77 0x77 0x77 0x77 0x77 0x77 0x77	
								| |
		            | |
		            \ /		
0x11 0x17 0x77 0x77 0x77 0x77 0x77 0x77 0x77 0x77				
0x11 0x17 0x77 0x77 0x77 0x77 0x77 0x77 0x77 0x77	
								| |
		            | |
		            \ /		
0x11 0x11 0x77 0x77 0x77 0x77 0x77 0x77 0x77 0x77				
0x11 0x11 0x77 0x77 0x77 0x77 0x77 0x77 0x77 0x77	
								| |
		            | |
		            \ /	
0x11 0x11 0x17 0x77 0x77 0x77 0x77 0x77 0x77 0x77				
0x11 0x11 0x17 0x77 0x77 0x77 0x77 0x77 0x77 0x77

so need to find y index and y index least 4 bits and most 4 bits
*/
		DEBUG_PRINTF(MOUDLE_NAME"y --> %d\n",y);
//		float y_temp = (y*4.0)/8.0;
		float y_temp = y/2.0;
		DEBUG_PRINTF("y_temp --> "LOG_FLOAT_MARKER"\n", LOG_FLOAT(y_temp));
		if((y_temp - (uint16_t)y_temp))
		{
			DEBUG_PRINTF(MOUDLE_NAME"least 4 bits\n");
			DEBUG_PRINTF(MOUDLE_NAME"y index --> %d\n",(uint16_t)y_temp);
			g_lcd_ram[x][(uint16_t)y_temp] = ((color&0xF0)>>4) | (g_lcd_ram[x][(uint16_t)y_temp]&0xF0);	
			g_lcd_ram[x+1][(uint16_t)y_temp] = (color&0x0F) | (g_lcd_ram[x+1][(uint16_t)y_temp]&0xF0);			
		}else{
			DEBUG_PRINTF(MOUDLE_NAME"most 4 bits\n");
			DEBUG_PRINTF(MOUDLE_NAME"y index --> %d\n",(uint16_t)y_temp);
			g_lcd_ram[x][(uint16_t)y_temp] = (color&0xF0) | (g_lcd_ram[x][(uint16_t)y_temp]&0x0F);	
			g_lcd_ram[x+1][(uint16_t)y_temp] = ((color&0x0F)<<4) | (g_lcd_ram[x+1][(uint16_t)y_temp]&0x0F);			
		}
	
		
//		_drv_lcd_graph graph =
//		{
//			.x = x,
//			.height = 2,
//		};
//		drv_lcd_set_one_frame(&graph);		
		
		
		
}


static void rect_draw(uint16_t x,
                      uint16_t y,
                      uint16_t width,
                      uint16_t height,
                      uint32_t color)
{
//    uint16_t lcd_width = DRV_LCD_WIDTH/2;
//    uint16_t lcd_height = DRV_LCD_HIGHT*2;

//    if ((x >= lcd_width) || (y >= lcd_height))
//    {
//        return;
//    }

//    if (width > (lcd_width - x))
//    {
//        width = lcd_width - x;
//    }

//    if (height > (lcd_height - y))
//    {
//        height = lcd_height - y;
//    }

		for(uint16_t i=0;i<height;i++)
		{
			memset(g_lcd_ram[x+i]+y/2,color,width/2);
		}
			
		
		
}


static void line_draw(uint16_t x_0,
                      uint16_t y_0,
                      uint16_t x_1,
                      int16_t y_1,
                      uint32_t color)
{
    uint16_t x = x_0;
    uint16_t y = y_0;
    int16_t d;
    int16_t d_1;
    int16_t d_2;
    int16_t ai;
    int16_t bi;
    int16_t xi = (x_0 < x_1) ? 1 : (-1);
    int16_t yi = (y_0 < y_1) ? 1 : (-1);
    bool swapped = false;

    d_1 = abs(x_1 - x_0);
    d_2 = abs(y_1 - y_0);

    pixel_draw(x, y, color);

    if (d_1 < d_2)
    {
        d_1 = d_1 ^ d_2;
        d_2 = d_1 ^ d_2;
        d_1 = d_2 ^ d_1;
        swapped = true;
    }

    ai = (d_2 - d_1) * 2;
    bi = d_2 * 2;
    d = bi - d_1;

    while ((y != y_1) || (x != x_1))
    {
        if (d >= 0)
        {
            x += xi;
            y += yi;
            d += ai;
        }
        else
        {
            d += bi;
            if (swapped)
            {
                y += yi;
            }
            else
            {
                x += xi;
            }
        }
        pixel_draw(x, y, color);
    }
}


void lib_gfx_point_draw(lib_gfx_point_t const * p_point,
                        uint8_t color)
{
    pixel_draw(p_point->x, p_point->y, color);
}

uint32_t lib_gfx_line_draw(lib_gfx_line_t const * p_line,
                             uint8_t color)
{

    uint16_t x_thick = 0;
    uint16_t y_thick = 0;

    if (abs(p_line->x_start - p_line->x_end) > abs(p_line->y_start - p_line->y_end))
    {
        y_thick = p_line->thickness;
    }
    else
    {
        x_thick = p_line->thickness;
    }
	

	

//    if ((p_line->x_start == p_line->x_end) || (p_line->y_start == p_line->y_end))
//    {
//        rect_draw(p_line->x_start,
//                  p_line->y_start,
//                  abs(p_line->x_end - p_line->x_start) + x_thick,
//                  abs(p_line->y_end - p_line->y_start) + y_thick,
//                  color);
//    }
//    else
    {
        if (x_thick > 0)
        {
            for (uint16_t i = 0; i < p_line->thickness; i++)
            {
                line_draw(p_line->x_start + i,
                          p_line->y_start,
                          p_line->x_end + i,
                          p_line->y_end,
                          color);
            }
        }
        else if (y_thick > 0)
        {
            for (uint16_t i = 0; i < p_line->thickness; i++)
            {
                line_draw(p_line->x_start,
                          p_line->y_start + i,
                          p_line->x_end,
                          p_line->y_end + i,
                          color);
            }
        }
//        else
//        {
//            line_draw(p_line->x_start + x_thick,
//                      p_line->y_start + y_thick,
//                      p_line->x_end + x_thick,
//                      p_line->y_end + y_thick,
//                      color);
//        }
    }

    return ERR_SUCCESS;
}


uint32_t lib_gfx_circle_draw(lib_gfx_circle_t const * p_circle,
                             uint8_t color,
                             bool fill)
{
    int16_t y = 0;
    int16_t err = 0;
    int16_t x = p_circle->r;

	
	
//    if ((p_circle->x - p_circle->r > DRV_LCD_WIDTH/2)     ||
//        (p_circle->y - p_circle->r > DRV_LCD_HIGHT*2))
//    {
//        return ERR_INVALID_PARAM;
//    }

    while (x >= y)
    {
        if (fill)
        {
						rect_draw( (-x + p_circle->x)<<1, (-y + p_circle->y)+1, (2 * y + 1), 2, color);
						rect_draw( (-y + p_circle->x)<<1, (-x + p_circle->y)+1, (2 * x + 1), 2, color);
						rect_draw( (y + p_circle->x)<<1, (-x + p_circle->y)+1, (2 * x + 1), 2, color);
						rect_draw( (x + p_circle->x)<<1, (-y + p_circle->y)+1, (2 * y + 1), 2, color);
            
        }
        else
        {
            pixel_draw( (y + p_circle->x)<<1, (x + p_circle->y), color);
            pixel_draw( (-y + p_circle->x)<<1, (x + p_circle->y), color);
            pixel_draw( (x + p_circle->x)<<1, (y + p_circle->y), color);
            pixel_draw( (-x + p_circle->x)<<1, (y + p_circle->y), color);
            pixel_draw( (-y + p_circle->x)<<1, (-x + p_circle->y), color);
            pixel_draw( (y + p_circle->x)<<1, (-x + p_circle->y), color);
            pixel_draw( (-x + p_circle->x)<<1, (-y + p_circle->y), color);
            pixel_draw( (x + p_circle->x)<<1, (-y + p_circle->y), color);
        }

        if (err <= 0)
        {
            y += 1;
            err += 2 * y + 1;
        }
        if (err > 0)
        {
            x -= 1;
            err -= 2 * x + 1;
        }
    }

    return ERR_SUCCESS;
}



uint32_t lib_gfx_rect_draw(lib_gfx_rect_t const * p_rect,
													 uint16_t thickness,
													 uint8_t color,
													 bool fill)
{
    uint16_t rect_width = p_rect->width - thickness;
    uint16_t rect_height = p_rect->height - thickness;

//    if ((p_rect->width == 1)                ||
//        (p_rect->height == 1)               ||
//        (thickness * 2 > p_rect->width)     ||
//        (thickness * 2 > p_rect->height)    ||
//        ((p_rect->x > DRV_LCD_WIDTH/2) &&
//        (p_rect->y > DRV_LCD_HIGHT*2)))
//    {
//        return ERR_INVALID_PARAM;
//    }


    if (fill)
    {
        rect_draw(p_rect->x,
                  p_rect->y,
                  p_rect->width,
                  p_rect->height,
                  color);
    }
    else
    {
        lib_gfx_line_t line;

        // Left vertical line.
        line.x_start = p_rect->x;
        line.y_start = p_rect->y;
        line.x_end = p_rect->x + p_rect->height;
        line.y_end = p_rect->y;
        line.thickness = thickness;
        (void)lib_gfx_line_draw( &line, color);
        // Right vertical line.
        line.x_start = p_rect->x;
        line.y_start = p_rect->y + rect_width;
        line.x_end = p_rect->x + p_rect->height;
        line.y_end = p_rect->y + rect_width;
        (void)lib_gfx_line_draw( &line, color);		
        // Top horizontal line.
        line.x_start = p_rect->x;
        line.y_start = p_rect->y + thickness;
        line.x_end = p_rect->x;
        line.y_end = p_rect->y + rect_width;
        (void)lib_gfx_line_draw( &line, color);
        // Bottom horizontal line.
        line.x_start = p_rect->x + rect_height+1;
        line.y_start = p_rect->y + thickness;
        line.x_end = p_rect->x + rect_height+1;
        line.y_end = p_rect->y + rect_width;
        (void)lib_gfx_line_draw( &line, color);


    }

    return ERR_SUCCESS;
}






