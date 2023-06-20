#include "df_conversion.h"
#include <string.h>
#include "rtt_log.h"
#include "SEGGER_RTT.h"


#define MOUDLE_LOG_ENABLED 							0
#define MOUDLE_DEBUG_ENABLED   					0
#define MOUDLE_NAME                     "[DF_CONVERSION]:"

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


typedef struct{

	union{
		uint8_t byte;
		struct{
			uint8_t FIRST   :    1;
			uint8_t SECOND  :    1;
			uint8_t THIRD   :    1;
			uint8_t FORTH   :    1;
			uint8_t FIFTH   :    1;
			uint8_t SIXTH   :    1;
			uint8_t SEVENTH :    1;
			uint8_t EIGHTH  :    1;
		}bit;
	}byte_bit_union;

}_byte_to_bit;
#define bit0         byte_bit_union.bit.FIRST     
#define bit1         byte_bit_union.bit.SECOND    
#define bit2         byte_bit_union.bit.THIRD    
#define bit3         byte_bit_union.bit.FORTH    
#define bit4         byte_bit_union.bit.FIFTH    
#define bit5         byte_bit_union.bit.SIXTH    
#define bit6         byte_bit_union.bit.SEVENTH    
#define bit7         byte_bit_union.bit.EIGHTH       


extern uint8_t g_lcd_ram[DRV_LCD_HIGHT*2][DRV_LCD_WIDTH/2];
void df_conversion_color(_drv_lcd_graph *graph, _df_dot_param *dot_param)
{
	uint16_t height = 0;
	uint16_t width = 0;
	for(height = 0;height<dot_param->height;height++)
	{
		for(width = 0;width<dot_param->width;width++)
		{
			_byte_to_bit *byte = (_byte_to_bit *)(dot_param->dot+height*dot_param->width+width);
			DEBUG_PRINTF(MOUDLE_NAME"height --> %d,width --> %d\n",height,width);
			DEBUG_PRINTF(MOUDLE_NAME"byte->byte_bit_union.byte --> 0x%02X\n",byte->byte_bit_union.byte);
			g_lcd_ram[height*2+graph->x][0+graph->y+width*4]     = byte->bit7 ? (dot_param->fg_color&0xF0) : (dot_param->bg_color&0xF0);
			g_lcd_ram[height*2+1+graph->x][0+graph->y+width*4]   = byte->bit7 ? (dot_param->fg_color&0xF0) : (dot_param->bg_color&0xF0);
			g_lcd_ram[height*2+graph->x][0+graph->y+width*4]    |= byte->bit6 ? (dot_param->fg_color&0x0F) : (dot_param->bg_color&0x0F);
			g_lcd_ram[height*2+1+graph->x][0+graph->y+width*4]  |= byte->bit6 ? (dot_param->fg_color&0x0F) : (dot_param->bg_color&0x0F);			
			DEBUG_PRINTF(MOUDLE_NAME"g_lcd_ram[%d][%d] --> 0x%02X\n",height*2+graph->x,0+graph->y+width*4,g_lcd_ram[height*2+graph->x][0+graph->y+width*4]);
			DEBUG_PRINTF(MOUDLE_NAME"g_lcd_ram[%d][%d] --> 0x%02X\n",height*2+1+graph->x,0+graph->y+width*4,g_lcd_ram[height*2+1+graph->x][0+graph->y+width*4]);
			
			g_lcd_ram[height*2+graph->x][1+graph->y+width*4]     = byte->bit5 ? (dot_param->fg_color&0xF0) : (dot_param->bg_color&0xF0);
			g_lcd_ram[height*2+1+graph->x][1+graph->y+width*4]   = byte->bit5 ? (dot_param->fg_color&0xF0) : (dot_param->bg_color&0xF0);
			g_lcd_ram[height*2+graph->x][1+graph->y+width*4]    |= byte->bit4 ? (dot_param->fg_color&0x0F) : (dot_param->bg_color&0x0F);
			g_lcd_ram[height*2+1+graph->x][1+graph->y+width*4]  |= byte->bit4 ? (dot_param->fg_color&0x0F) : (dot_param->bg_color&0x0F);				
			DEBUG_PRINTF(MOUDLE_NAME"g_lcd_ram[%d][%d] --> 0x%02X\n",height*2+graph->x,1+graph->y+width*4,g_lcd_ram[height*2+graph->x][1+graph->y+width*4]);
			DEBUG_PRINTF(MOUDLE_NAME"g_lcd_ram[%d][%d] --> 0x%02X\n",height*2+1+graph->x,1+graph->y+width*4,g_lcd_ram[height*2+1+graph->x][1+graph->y+width*4]);						
			
			g_lcd_ram[height*2+graph->x][2+graph->y+width*4]     = byte->bit3 ? (dot_param->fg_color&0xF0) : (dot_param->bg_color&0xF0);
			g_lcd_ram[height*2+1+graph->x][2+graph->y+width*4]   = byte->bit3 ? (dot_param->fg_color&0xF0) : (dot_param->bg_color&0xF0);
			g_lcd_ram[height*2+graph->x][2+graph->y+width*4]    |= byte->bit2 ? (dot_param->fg_color&0x0F) : (dot_param->bg_color&0x0F);
			g_lcd_ram[height*2+1+graph->x][2+graph->y+width*4]  |= byte->bit2 ? (dot_param->fg_color&0x0F) : (dot_param->bg_color&0x0F);				
			DEBUG_PRINTF(MOUDLE_NAME"g_lcd_ram[%d][%d] --> 0x%02X\n",height*2+graph->x,2+graph->y+width*4,g_lcd_ram[height*2+graph->x][2+graph->y+width*4]);
			DEBUG_PRINTF(MOUDLE_NAME"g_lcd_ram[%d][%d] --> 0x%02X\n",height*2+1+graph->x,2+graph->y+width*4,g_lcd_ram[height*2+1+graph->x][2+graph->y+width*4]);				
			
			g_lcd_ram[height*2+graph->x][3+graph->y+width*4]     = byte->bit1 ? (dot_param->fg_color&0xF0) : (dot_param->bg_color&0xF0);
			g_lcd_ram[height*2+1+graph->x][3+graph->y+width*4]   = byte->bit1 ? (dot_param->fg_color&0xF0) : (dot_param->bg_color&0xF0);
			g_lcd_ram[height*2+graph->x][3+graph->y+width*4]    |= byte->bit0 ? (dot_param->fg_color&0x0F) : (dot_param->bg_color&0x0F);
			g_lcd_ram[height*2+1+graph->x][3+graph->y+width*4]  |= byte->bit0 ? (dot_param->fg_color&0x0F) : (dot_param->bg_color&0x0F);	
			DEBUG_PRINTF(MOUDLE_NAME"g_lcd_ram[%d][%d] --> 0x%02X\n",height*2+graph->x,3+graph->y+width*4,g_lcd_ram[height*2+graph->x][3+graph->y+width*4]);
			DEBUG_PRINTF(MOUDLE_NAME"g_lcd_ram[%d][%d] --> 0x%02X\n",height*2+1+graph->x,3+graph->y+width*4,g_lcd_ram[height*2+1+graph->x][3+graph->y+width*4]);		

			

		}
	}
	
}

/**
----------------------------------------------------------------------------------------
Dot bit :               10      01     11      00
                      0x10    0x01   0x11    0x00
Color byte :          0x10    0x01   0x11    0x00
----------------------------------------------------------------------------------------

Dot for 1                      Conversion                      Color for 1
----------------------------------------------------------------------------------------
0x00,                                                        0x00,0x00,0x00,0x00,
																														 0x00,0x00,0x00,0x00,
----------------------------------------------------------------------------------------
0x00,                                                        0x00,0x00,0x00,0x00,
                                                             0x00,0x00,0x00,0x00,
----------------------------------------------------------------------------------------
0x00,                                                        0x00,0x00,0x00,0x00,
																														 0x00,0x00,0x00,0x00,
----------------------------------------------------------------------------------------
0x10,                                                        0x00,0x07,0x00,0x00,
																													   0x00,0x07,0x00,0x00,
----------------------------------------------------------------------------------------
														 -------------- \
														 -------------- /
0x70,                                                        0x07,0x77,0x00,0x00,
                                                             0x07,0x77,0x00,0x00,
----------------------------------------------------------------------------------------
0x10,                                                        0x00,0x07,0x00,0x00,
																														 0x00,0x07,0x00,0x00,
----------------------------------------------------------------------------------------																														 
0x10,																												 0x00,0x07,0x00,0x00,
																														 0x00,0x07,0x00,0x00,
----------------------------------------------------------------------------------------																															 
0x10,                                                        0x00,0x07,0x00,0x00,
                                                             0x00,0x07,0x00,0x00,
----------------------------------------------------------------------------------------	
0x10,                                                        0x00,0x07,0x00,0x00,
																											       0x00,0x07,0x00,0x00,
----------------------------------------------------------------------------------------	
0x10,                                                        0x00,0x07,0x00,0x00,
                                                             0x00,0x07,0x00,0x00,
----------------------------------------------------------------------------------------	
0x10,                                                        0x00,0x07,0x00,0x00,
                                                             0x00,0x07,0x00,0x00,
----------------------------------------------------------------------------------------	
0x10,                                                        0x00,0x07,0x00,0x00,
                                                             0x00,0x07,0x00,0x00,
----------------------------------------------------------------------------------------	
0x10,                                                        0x00,0x07,0x00,0x00,
                                                             0x00,0x07,0x00,0x00,
----------------------------------------------------------------------------------------	
0x7C,                                                        0x07,0x77,0x77,0x00,
                                                             0x07,0x77,0x77,0x00,
----------------------------------------------------------------------------------------	
0x00,                                                        0x00,0x00,0x00,0x00,
                                                             0x00,0x00,0x00,0x00,
----------------------------------------------------------------------------------------	
0x00,                                                        0x00,0x00,0x00,0x00,
                                                             0x00,0x00,0x00,0x00,
----------------------------------------------------------------------------------------	


*/






