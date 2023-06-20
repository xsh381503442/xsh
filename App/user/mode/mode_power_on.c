#include "mode_config.h"
#include "mode_power_on.h"

#include "drv_lcd.h"
#include "drv_lsm6dsl.h"

#include "task_display.h"
#include "task_ble.h"

#include "gui_dial.h"

#include "com_data.h"


#include "drv_lsm6dsl.h"
#include "drv_extflash.h"
#include "am_bootloader.h"
#include "lib_app_data.h"
#include "drv_battery.h"


#if DEBUG_ENABLED == 1 && MODE_POWER_ON_LOG_ENABLED == 1
	#define MODE_POWER_ON_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define MODE_POWER_ON_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define MODE_POWER_ON_WRITESTRING(...)
	#define MODE_POWER_ON_PRINTF(...)		        
#endif

extern SetValueStr SetValue;
void mode_button_pon(void)
{
#if !defined WATCH_HAS_NO_BLE 
	dev_ble_system_off();
#endif
	if(SetValue.SwBle > BLE_DISCONNECT)
	{
		SetValue.SwBle = BLE_DISCONNECT;
	}


//  电量百分比改为存在外部flash
//	//电量大于70%是，电量是假的，需要复位APOLLO需要保存复位前的电量
//	_lib_app_data lib_app_data;
//	lib_app_data_read(&lib_app_data);
//	lib_app_data.bat_adc = drv_battery_save_adc();
//	if(lib_app_data.crc != 0xFFFFFFFF)
//	{
//		lib_app_data.crc = am_bootloader_fast_crc32((uint8_t*)(&lib_app_data.crc)+sizeof(lib_app_data.crc),sizeof(_lib_app_data)-sizeof(lib_app_data.crc));
//	}
//	lib_app_data_write(&lib_app_data);	
	

	
	
//	SetValue.crc = am_bootloader_fast_crc32((uint8_t*)(&SetValue.crc)+sizeof(SetValue.crc), sizeof(SetValueStr)-sizeof(SetValue.crc));
//	dev_extFlash_enable();
//	dev_extFlash_erase(SET_VALUE_ADDRESS, sizeof(SetValueStr));
//	dev_extFlash_write(SET_VALUE_ADDRESS, (uint8_t*)(&SetValue), sizeof(SetValueStr));
//	dev_extFlash_disable();	
	
	taskENTER_CRITICAL();
	am_hal_reset_por();
	while(1);	
	
#if 0	
	//初始化LCD
	LCD_Init();	
	LCD_Poweron();
	LCD_SetBackgroundColor(LCD_BLACK);
	LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
	LCD_DisplayOn();

    //初始化计步
  drv_lsm6dsl_acc_init();
	drv_lsm6dsl_enable_pedometer();
	
	//显示待机界面
	gui_dial_paint();
	LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);

	ScreenState = DISPLAY_SCREEN_HOME;
	SetValue.AutoLockScreenFlag = 0;//开机不上锁
	if(SetValue.SwBle >= BLE_DISCONNECT)
	{
		MODE_POWER_ON_PRINTF("[MODE_POWER_ON]:SetValue.SwBle >= BLE_DISCONNECT\n");
		dev_ble_system_on();
	}	
#endif	
}



