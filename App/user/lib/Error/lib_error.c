#include "lib_error.h"
#include "SEGGER_RTT.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "drv_lcd.h"
#include "FreeRTOS.h"
#include "task.h"
#include "bsp.h"
#include "am_mcu_apollo.h"
#include "drv_extFlash.h"


#define LIB_ERROR_LOG_ENABLED 1

#if DEBUG_ENABLED == 1 && LIB_ERROR_LOG_ENABLED == 1
	#define LIB_ERROR_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define LIB_ERROR_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define LIB_ERROR_WRITESTRING(...)
	#define LIB_ERROR_PRINTF(...)		        
#endif

extern uint8_t i2c_err;
__weak void error_save_and_stop(uint32_t info)
{
    /* static error variables - in order to prevent removal by optimizers */
    static volatile struct
    {
        uint32_t        err_code;
        uint32_t        line_num;
        const uint8_t * p_file_name;
    } m_error_data = {0};

    // The following variable helps Keil keep the call stack visible, in addition, it can be set to
    // 0 in the debugger to continue executing code after the error check.
    volatile bool loop = true;

		m_error_data.err_code     = ((error_info_t *)(info))->err_code;
		m_error_data.line_num     = (unsigned int) ((error_info_t *)(info))->line_num;
		m_error_data.p_file_name  = ((error_info_t *)(info))->p_file_name;
	    SPI_No_Semaphore = 1;

		LCD_SetBackgroundColor(LCD_BLACK);
		SetWord_t word = {0};
		word.x_axis = 40;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_16_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_BLACK;
		word.kerning = 0;		

		char p_char[23];	
		memset(p_char,0,sizeof(p_char));
		sprintf(p_char,"error : 0x%08X",m_error_data.err_code);		
		LCD_SetErrCode(p_char,&word);	
		word.x_axis += 32;
		memset(p_char,0,sizeof(p_char));
		sprintf(p_char,"line : %05d",m_error_data.line_num);		
		LCD_SetErrCode(p_char,&word);
		word.x_axis += 32;
		memset(p_char,0,sizeof(p_char));	
		uint8_t len = strlen((char *)m_error_data.p_file_name)>22?(strlen((char *)m_error_data.p_file_name)-22):0;
		sprintf(p_char,"%s",m_error_data.p_file_name+len);		
		LCD_SetErrCode(p_char,&word);				
		LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
		
    // If printing is disrupted, remove the irq calls, or set the loop variable to 0 in the debugger.
    __disable_irq();
		
    while (loop);

    __enable_irq();
}



static void error_fault_handler(uint32_t info)
{
	LIB_ERROR_PRINTF("[LIB_ERROR]:Hit error_fault_handler\n");

  error_save_and_stop(info);

}


void error_handler(ret_code_t error_code, uint32_t line_num, const uint8_t * p_file_name)
{
    error_info_t error_info =
    {
        .line_num    = line_num,
        .p_file_name = p_file_name,
        .err_code    = error_code,
    };
    error_fault_handler((uint32_t)(&error_info));
}

void ble_error_handler(_ble_app_error *ble_app_error)
{
	__disable_irq();
	
	
	volatile bool loop = true;
	
		SPI_No_Semaphore = 1;
		
		

		//if (Get_TaskSportSta())
		//{
		//	SaveSport(1);
		//}
		//mode_power_off_store_set_value();
		
		LCD_SetBackgroundColor(LCD_BLACK);
		SetWord_t word = {0};
		word.x_axis = 40;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_16_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_BLACK;
		word.kerning = 0;		

		char p_char[23];
		memset(p_char,0,sizeof(p_char));
		sprintf(p_char,"BLE");		
		LCD_SetErrCode(p_char,&word);	
		word.x_axis += 32;			
		memset(p_char,0,sizeof(p_char));
		sprintf(p_char,"error : 0x%08X",ble_app_error->err_code);		
		LCD_SetErrCode(p_char,&word);	
		word.x_axis += 32;
		memset(p_char,0,sizeof(p_char));
		sprintf(p_char,"line : %05d",ble_app_error->line_num);		
		LCD_SetErrCode(p_char,&word);
		word.x_axis += 32;
		memset(p_char,0,sizeof(p_char));
		uint8_t len = strlen((char *)ble_app_error->p_file_name)>22?(strlen((char *)ble_app_error->p_file_name)-22):0;
		sprintf(p_char,"%s",ble_app_error->p_file_name+len);			
		LCD_SetErrCode(p_char,&word);				
		LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);	
	
    
		
    while (loop);

    __enable_irq();	
}
#ifdef DEBUG_ERR  
void error_currenttask(void)
{

    LCD_SetBackgroundColor(LCD_BLACK);
    SetWord_t word = {0};
    word.x_axis = 40;
    word.y_axis = LCD_CENTER_JUSTIFIED;
    word.size = LCD_FONT_16_SIZE;
    word.forecolor = LCD_WHITE;
    word.bckgrndcolor = LCD_BLACK;
    word.kerning = 0; 
    
    char p_char[23];    
    memset(p_char,0,sizeof(p_char));
    sprintf(p_char,"error : 0x%x",0x5a);
    LCD_SetErrCode(p_char,&word);

    word.x_axis += 32;
	memset(p_char,0,sizeof(p_char));
	sprintf(p_char,"task : %s",getpxCurrentTCBName());		
	LCD_SetErrCode(p_char,&word);
    
   

    word.x_axis += 32;
	memset(p_char,0,sizeof(p_char));
	sprintf(p_char,"LR = 0x%08X",getpxCurrentTCBLR());		
	LCD_SetErrCode(p_char,&word);
    
   word.x_axis += 32;
	memset(p_char,0,sizeof(p_char));
	sprintf(p_char,"PC = 0x%08X",getpxCurrentTCBPC());		
	LCD_SetErrCode(p_char,&word);

    am_hal_gpio_pin_config(EXTFLASH_READY_PIN,AM_HAL_PIN_INPUT);
	
    word.x_axis += 32;
	memset(p_char,0,sizeof(p_char));
	sprintf(p_char,"READY_PIN:%d",am_hal_gpio_input_bit_read(EXTFLASH_READY_PIN));		
	LCD_SetErrCode(p_char,&word);
   word.x_axis += 32;
	memset(p_char,0,sizeof(p_char));
	sprintf(p_char,"I2C_err:%d",i2c_err);		
	LCD_SetErrCode(p_char,&word);
    LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);    


}
#endif




