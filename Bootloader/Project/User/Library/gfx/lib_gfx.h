#ifndef __LIB_GFX_H__
#define __LIB_GFX_H__

#include <stdint.h>
#include <stdbool.h>



typedef struct
{
    uint16_t x;                 
    uint16_t y;                 
}lib_gfx_point_t;

typedef struct
{
    uint16_t x_start;           
    uint16_t y_start;           
    uint16_t x_end;            
    uint16_t y_end;             
    uint16_t thickness;         
}lib_gfx_line_t;

typedef struct
{
    uint16_t x;                 /**< Vertical coordinate of the centre of the object. */
    uint16_t y;                 /**< Horizontal coordinate of the centre of the object. */
    uint16_t r;                 /*Max Circle : Center point (.x = (120-2) , .y = (120-2), .radius = (120-2))*/
}lib_gfx_circle_t;

typedef struct
{
    uint16_t x;            //Must be even number             
    uint16_t y;            //Must be even number     
    uint16_t width;        //Must be even number     
    uint16_t height;       //Must be even number     
}lib_gfx_rect_t;

#define LIB_GFX_POINT(_x, _y)   \
    {                           \
        .x = (_x),              \
        .y = (_y)               \
    }

#define LIB_GFX_LINE(_x_0, _y_0, _x_1, _y_1, _thickness)    \
    {                                               \
        .x_start = (_x_0),                          \
        .y_start = (_y_0),                          \
        .x_end = (_x_1),                            \
        .y_end = (_y_1),                            \
        .thickness = (_thickness)                   \
    }

#define LIB_GFX_CIRCLE(_x, _y, _radius)     \
    {                                       \
        .x = (_x),                          \
        .y = (_y),                          \
        .r = (_radius)                      \
    }
	
#define LIB_GFX_RECT(_x, _y, _width, _height)   \
    {                                           \
        .x = (_x),                              \
        .y = (_y),                              \
        .width = (_width),                      \
        .height = (_height)                     \
    }

void lib_gfx_point_draw(lib_gfx_point_t const * p_point,
                        uint8_t color);		
uint32_t lib_gfx_line_draw(lib_gfx_line_t const * p_line,
                             uint8_t color);		
uint32_t lib_gfx_circle_draw(lib_gfx_circle_t const * p_circle,
                             uint8_t color,
                             bool fill);		
uint32_t lib_gfx_rect_draw(lib_gfx_rect_t const * p_rect,
											 uint16_t thickness,
											 uint8_t color,
											 bool fill);
		
		
		
		
		
		
		
		
		
		
		
#endif //__LIB_GFX_H__
