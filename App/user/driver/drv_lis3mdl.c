#include "am_mcu_apollo.h"
#include "am_util.h"
#include "math.h"
#include "bsp_iom.h"
#include "drv_config.h"
#include "drv_lis3mdl.h"
#include "com_data.h"
#include "drv_lcd.h"
#include "drv_lsm6dsl.h"

//暂时调用
#include "gui_tool_compass.h"

#if DEBUG_ENABLED == 1 && DRV_MS5837_LOG_ENABLED == 1
	#define DRV_MS5837_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define DRV_MS5837_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define DRV_MS5837_WRITESTRING(...)
	#define DRV_MS5837_PRINTF(...)		        
#endif


//float  Data_COM_TMP[3] = {0};

extern SemaphoreHandle_t Appolo_I2C_Semaphore;
//uint8_t		g_CompassCaliStatus;
extern SetValueStr SetValue;


const am_hal_iom_config_t g_sLIS3MDL_IOMConfig =
{
    .ui32InterfaceMode = AM_HAL_IOM_I2CMODE,
    .ui32ClockFrequency = AM_HAL_IOM_400KHZ,
    .bSPHA = 0,
    .bSPOL = 0,
    .ui8WriteThreshold = 4,
    .ui8ReadThreshold = 60,
};


void drv_lis3mdl_enable(void)
{
	xSemaphoreTake( Appolo_I2C_Semaphore, portMAX_DELAY );
	
	// Enable IOM
	bsp_iom_enable(BSP_LIS3MDL_IOM,g_sLIS3MDL_IOMConfig);
}

void drv_lis3mdl_disable(void)
{	
	//Disable IOM
	bsp_iom_disable(BSP_LIS3MDL_IOM);
	
	xSemaphoreGive( Appolo_I2C_Semaphore );	
}

static uint8_t lis3mdl_reg_read(uint8_t reg)
{
	am_hal_iom_buffer(1) sData;
	uint8_t ui8Offset;
	
	ui8Offset = reg;

	am_hal_iom_i2c_read(BSP_LIS3MDL_IOM,LIS3MDL_ADDR,sData.words,1,AM_HAL_IOM_OFFSET(ui8Offset));
	
	return sData.bytes[0];
}

static void lis3mdl_reg_write(uint8_t reg,uint8_t value)
{
	am_hal_iom_buffer(1) sData;
	uint8_t ui8Offset;
	
	ui8Offset = reg;
	sData.bytes[0] = value;
	am_hal_iom_i2c_write(BSP_LIS3MDL_IOM,LIS3MDL_ADDR,sData.words,1,AM_HAL_IOM_OFFSET(ui8Offset));
}


bool drv_lis3mdl_id(void)
{
	uint8_t data = 0;
	uint8_t MagID;
    drv_lis3mdl_enable();
    data = lis3mdl_reg_read(0x0F);
    drv_lis3mdl_disable();
    MagID = 0x3D;
	
	
	if(data == MagID)
	{
		return true;
	}
	else
	{
		return false;
	}
}

/*void drv_lis3mdl_init(void)
{
	drv_lis3mdl_enable();
	lis3mdl_reg_write(0x21,0x40);
	lis3mdl_reg_write(0x20,0xFC);
	lis3mdl_reg_write(0x23,0x0C);
	lis3mdl_reg_write(0x22,0x00);
	drv_lis3mdl_disable();
	
	//初始化校准
	akm_cal_init();	
	
}*/

bool drv_lis3mdl_data(int16_t *psdata)
{
	am_hal_iom_buffer(1) sData;
	am_hal_iom_buffer(6) sOutData = {0};
	uint8_t ui8Offset,MagDRDY;
	
	//读取status
	sData.bytes[0] = lis3mdl_reg_read(0x27);
	MagDRDY = 0x08;
	
	if(sData.bytes[0]&MagDRDY)
	{
		
		ui8Offset = 0x28 | 0x80;
		am_hal_iom_i2c_read(BSP_LIS3MDL_IOM,LIS3MDL_ADDR,sOutData.words,6,AM_HAL_IOM_OFFSET(ui8Offset));
		
	
		
		psdata[0] = (sOutData.bytes[1] << 8)|sOutData.bytes[0];
		psdata[1] = (sOutData.bytes[3] << 8)|sOutData.bytes[2];
		psdata[2] = (sOutData.bytes[5] << 8)|sOutData.bytes[4];
      
        return true;
		
	}
	else
	{

		return false;
	}
//	AM_LOG_PRINTF("x = %.2f,y = %.2f,z = %.2f\n",psdata[0]*0.04f,psdata[1]*0.04f,psdata[2]*0.04f);
}

/*void drv_lis3mdl_cail_init(void)
{
	//初始化校准数据
	g_CompassCaliStatus = 0;
	akm_cal_init();
}*/

/*uint8_t drv_lis3mdl_cail_status_get(void)
{
	//返回校准状态
	return g_CompassCaliStatus;
}*/

//根据pitch和roll计算气泡所在位置
/*static void drv_lis3mdl_bubble_get(float pitch, float roll)
{
	float MaxAngle = 88;	//最大倾斜角度
	uint8_t Width = 88;	//气泡运动宽度
	uint8_t StartX = (LCD_LINE_CNT_MAX - Width)/2;	
	uint8_t StartY = (LCD_LINE_CNT_MAX - Width)/2;
	uint16_t x,y;
	
	if(pitch < (-MaxAngle))
	{
		x = StartX;
	}
	else if(pitch < MaxAngle)
	{
		x = StartX + (Width/2/MaxAngle*pitch) + (Width/2);
		if(x < StartX)
		{
			x = StartX;
		}
		else if(x > (StartX+Width))
		{
			x = StartX+Width;
		}
	}
	else
	{
		x = StartX + Width;
	}
	
	if(roll < (-MaxAngle))
	{
		y = StartY;
	}
	else if(roll < MaxAngle)
	{
		y = StartY + (Width/2/MaxAngle*roll) + (Width/2);
		if(y < StartY)
		{
			y = StartY;
		}
		else if(y > (StartY+Width))
		{
			y = StartY+Width;
		}
	}
	else
	{
		y = StartY + Width;
	}
	
	//暂时放在这
	gui_tool_compass_bubble_set(x,y);
}*/

/*float compass_cail_auto(void)
{
	float Data_COM[3] = {0};
	float Data_ACC[3] = {0};
	int16_t	mstat=0;
	float tmpx, tmpy, tmpz;
	float av;
	float pitch, roll, azimuth;
	const float rad2deg = 180 / AKFS_PI;
	lis3mdl_sample(1) magdata;
	int16_t a_buf[3] = {1,1,1};
	//uint8_t status;
    int ret = 0;
	
	//读取地磁数据
	drv_lis3mdl_enable();
	am_util_delay_ms(1);
	ret = drv_lis3mdl_data(magdata.words);
	drv_lis3mdl_disable();

    if (ret == true)
    {
	   Data_COM_TMP[0] = magdata.samples[0]*0.04f;
	   Data_COM_TMP[1] = magdata.samples[1]*0.04f;
	   Data_COM_TMP[2] = magdata.samples[2]*0.04f;
    }
    Data_COM[0] =-Data_COM_TMP[1]; 
    Data_COM[1] =-Data_COM_TMP[0]; 
    Data_COM[2] = Data_COM_TMP[2]; 
//	convert_magnetic(Data_COM,mstat,&tmpx, &tmpy, &tmpz, &g_CompassCaliStatus);
//	AM_LOG_PRINTF("Compass_Cali_Status ======= %d\n",g_CompassCaliStatus);
	
	if(g_CompassCaliStatus != 3)
	{
		tmpx = Data_COM[0] - SetValue.Compass_Offset_x;
		tmpy = Data_COM[1] - SetValue.Compass_Offset_y;
		tmpz = Data_COM[2] - SetValue.Compass_Offset_z;
	}
	else
	{
		SetValue.Compass_Offset_x = Data_COM[0] - tmpx;
		SetValue.Compass_Offset_y = Data_COM[1] - tmpy;
		SetValue.Compass_Offset_z = Data_COM[2] - tmpz;
	}

 
    drv_lsm6dsl_acc_axes_raw(a_buf);
    
    Data_ACC[1] = a_buf[0];
	Data_ACC[0] =-a_buf[1];
	Data_ACC[2] =a_buf[2];
    
//	AM_LOG_PRINTF("A_X = %d,A_Y = %d,A_Z = %d",(int)Data_ACC[0],(int)Data_ACC[1],(int)Data_ACC[2]);
	
	av = sqrtf(Data_ACC[0]*Data_ACC[0] + Data_ACC[1]*Data_ACC[1] + Data_ACC[2]*Data_ACC[2]);
	if(av == 0)
	{
		//防止除数为0
		av = 1;
	}
	pitch = asinf(-Data_ACC[1] / av);
	roll = asinf(Data_ACC[0] / av);

	azimuth = atan2(-(tmpx) * cosf(roll) + tmpz * sinf(roll),
					tmpx*sinf(pitch)*sinf(roll) + tmpy*cosf(pitch) + tmpz*sinf(pitch)*cosf(roll));
	azimuth =  azimuth * rad2deg;
	pitch = pitch * rad2deg;
	roll = roll * rad2deg;
	drv_lis3mdl_bubble_get(pitch,roll);


    //azimuth -= 180;

	
	if(azimuth<0)
	{
		azimuth+= 360.0f;
	}

	return  azimuth;
}*/

void drv_lis3mdl_standby(void)
{
	drv_lis3mdl_enable();
	lis3mdl_reg_write(0x22,0x03);
	drv_lis3mdl_disable();
}

void drv_lis3mdl_reset(void)
{
    drv_lis3mdl_enable();
	lis3mdl_reg_write(0x21,0x0C);
	drv_lis3mdl_disable();
}





