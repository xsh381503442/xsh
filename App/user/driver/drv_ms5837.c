#include "drv_config.h"
#include "drv_ms5837.h"

#include "bsp.h"
#include "bsp_iom.h"

#include "com_data.h"

#if DEBUG_ENABLED == 1 && DRV_MS5837_LOG_ENABLED == 1
	#define DRV_MS5837_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define DRV_MS5837_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define DRV_MS5837_WRITESTRING(...)
	#define DRV_MS5837_PRINTF(...)		        
#endif

#ifndef DRV_MS5837_MODEL
#define DRV_MS5837_MODEL		02//30
#endif

extern SemaphoreHandle_t Appolo_I2C_Semaphore;
static uint16_t C1,C2,C3,C4,C5,C6;
static float m_pres_value,m_alt_value,m_temp_value;

const am_hal_iom_config_t g_sMS5837_IOMConfig =
{
    .ui32InterfaceMode = AM_HAL_IOM_I2CMODE,
    .ui32ClockFrequency = AM_HAL_IOM_400KHZ,
    .bSPHA = 0,
    .bSPOL = 0,
    .ui8WriteThreshold = 4,
    .ui8ReadThreshold = 60,
};

static void drv_ms5837_enable(void)
{
	xSemaphoreTake( Appolo_I2C_Semaphore, portMAX_DELAY );
	
	DRV_MS5837_PRINTF("[drv_ms5837]: drv_ms5837_enable\n");
	
	// Enable IOM
	bsp_iom_enable(BSP_MS5837_IOM,g_sMS5837_IOMConfig);
}

static void drv_ms5837_disable(void)
{
	DRV_MS5837_PRINTF("[drv_ms5837]: drv_ms5837_disable\n");
	
	//Disable IOM
	bsp_iom_disable(BSP_MS5837_IOM);
	
	xSemaphoreGive( Appolo_I2C_Semaphore );	
}

///向MS5837下发指令，指令都为1个字节
static void drv_ms5837_write(uint8_t ui8Command)
{
	am_hal_iom_buffer(1) sCommand;
	
	sCommand.bytes[0] = ui8Command;
	
	am_hal_iom_i2c_write(BSP_MS5837_IOM,DRV_MS5837_I2C_ADDR,sCommand.words,1,AM_HAL_IOM_RAW);
}

//从MS5837读取多个字节数据的值
static uint32_t drv_ms5837_read(uint8_t ui8Command,uint8_t ui8Number)
{
	am_hal_iom_buffer(1) sCommand;
	am_hal_iom_buffer(4) sResult = {0};
	uint32_t data = 0;
	
	sCommand.bytes[0] = ui8Command;
	
	am_hal_iom_i2c_write(BSP_MS5837_IOM,DRV_MS5837_I2C_ADDR,sCommand.words,1,AM_HAL_IOM_RAW);
	am_hal_iom_i2c_read(BSP_MS5837_IOM,DRV_MS5837_I2C_ADDR,sResult.words,ui8Number,AM_HAL_IOM_RAW);
	
	for (uint8_t i = 0; i < ui8Number; i++)
    {
		data <<= 8;
        data |= sResult.bytes[i];
    }
	
	return data;
}

void drv_ms5837_reset(void)
{
	//发送复位指令
	drv_ms5837_enable();
	drv_ms5837_write(DRV_MS5837_COMMAND_RESET);
	drv_ms5837_disable();
}

/*
static unsigned char drv_ms5837_crc4(unsigned int n_prom[]) // n_prom defined as 8x unsigned int (n_prom[8])
{
	int cnt; // simple counter
	unsigned int n_rem=0; // crc remainder
	unsigned char n_bit;
	
	n_prom[0]=((n_prom[0]) & 0x0FFF); // CRC byte is replaced by 0
	n_prom[7]=0; // Subsidiary value, set to 0
	for (cnt = 0; cnt < 16; cnt++) // operation is performed on bytes
	{ 
		// choose LSB or MSB
		if (cnt%2==1) 
			n_rem ^= (unsigned short) ((n_prom[cnt>>1]) & 0x00FF);
		else 
			n_rem ^= (unsigned short) (n_prom[cnt>>1]>>8);
		for (n_bit = 8; n_bit > 0; n_bit--)
		{
			if (n_rem & (0x8000)) 
				n_rem = (n_rem << 1) ^ 0x3000;
			else 
				n_rem = (n_rem << 1);
		}
	}
	n_rem= ((n_rem >> 12) & 0x000F); // final 4-bit remainder is CRC code
	
	return (n_rem ^ 0x00);
}
*/

void ambient_value_get(float *pPres,float *pAlt,float *pTemp)
{
	*pPres = m_pres_value;
	*pAlt = m_alt_value;
	*pTemp = m_temp_value;
}

void drv_ms5837_data_get(float *pPres,float *pAlt,float *pTemp)
{
	uint32_t D1,D2;
	
	drv_ms5837_enable();
	
	//Reset
	drv_ms5837_write(DRV_MS5837_COMMAND_RESET);
	
	//压力灵敏度
	C1 = drv_ms5837_read(DRV_MS5837_COMMAND_PROM_C1,2) & 0xFFFF;
	
	//压力补偿值
	C2 = drv_ms5837_read(DRV_MS5837_COMMAND_PROM_C2,2) & 0xFFFF;
	
	//压力灵敏度温度系数
	C3 = drv_ms5837_read(DRV_MS5837_COMMAND_PROM_C3,2) & 0xFFFF;
	
	//压力补偿温度系数
	C4 = drv_ms5837_read(DRV_MS5837_COMMAND_PROM_C4,2) & 0xFFFF;
	
	//参考温度
	C5 = drv_ms5837_read(DRV_MS5837_COMMAND_PROM_C5,2) & 0xFFFF;
	
	//温度传感器温度系数
	C6 = drv_ms5837_read(DRV_MS5837_COMMAND_PROM_C6,2) & 0xFFFF;
	
	DRV_MS5837_PRINTF("C1= %d\r C2= %d\r C3= %d\r C4= %d\r C5= %d\r C6= %d\r\n",C1,C2,C3,C4,C5,C6);
	
	//使用8192采样率读取压力数据
	drv_ms5837_write(DRV_MS5837_COMMAND_D1_8192);
	am_util_delay_ms(20);
	D1 = drv_ms5837_read(DRV_MS5837_COMMAND_ADC,3) & 0xFFFFFF;
	
	//使用8192采样率读取温度数据
	drv_ms5837_write(DRV_MS5837_COMMAND_D2_8192);
	am_util_delay_ms(20);
	D2 = drv_ms5837_read(DRV_MS5837_COMMAND_ADC,3) & 0xFFFFFF;
	
	drv_ms5837_disable();
	
	DRV_MS5837_PRINTF("D1= %d\r D2= %d\r\n",D1,D2);
	
	//计算气压和温度，并进行温度补偿，气压单位mbar
	drv_ms5837_CalcPT2nd(pPres,pTemp,D1,D2);
	
	if(*pPres == 0)
	{
		*pAlt = 0;
		*pTemp = 0;
	}
	else
	{
		//计算绝对高度，单位为m
//		*pAlt = drv_ms5837_abs_altitude((uint32_t)(*pPres*100))/1000.f;
		*pAlt = drv_ms5837_rel_altitude((uint32_t)(*pPres*100),SetValue.AppSet.Sea_Level_Pressure);
	}
	
	m_pres_value = *pPres;
	m_alt_value = *pAlt;
	m_temp_value = *pTemp;
	
	DRV_MS5837_PRINTF("P="LOG_FLOAT_MARKER"\r A="LOG_FLOAT_MARKER"\r T="LOG_FLOAT_MARKER"\r\n",
						LOG_FLOAT(*pPres),LOG_FLOAT(*pAlt),LOG_FLOAT(*pTemp));
}

/*
static void drv_ms5837_CalcPT(float *pPres,float *pTemp,uint32_t d1,uint32_t d2)
{
	int32_t temp1,pres1;
	int64_t dt,off,sens;
	
	dt = (int64_t)(d2 - (C5 * powf(2,8)));
	temp1 = (int32_t)(2000 + ((dt * C6) / powf(2,23)));
#if DRV_MS5837_MODEL == 02
//MS5837-02BA01
	off = (int64_t)((C2 * powf(2,17)) + ((C4 * dt) / powf(2,6)));
	sens = (int64_t)(C1 * powf(2,16) + ((C3 * dt) / powf(2,7)));
	pres1 = (int32_t)(((d1 * sens / powf(2,21)) - off) / powf(2,15));
	
	//单位 mbar
	*pPres = pres1 / 100.f;
	//单位 度
	*pTemp = temp1 / 100.f;
#else
//MS5837-30BA	
	off = (C2 * powf(2,16)) + ((C4 * dt) / powf(2,7));
	sens = C1 * powf(2,15) + ((C3 * dt) / powf(2,8));
	pres1 = ((d1 * sens / powf(2,21)) - off) / powf(2,13);
	
	//单位 mbar
	*pPres = pres1 / 10.f;
	//单位 度
	*pTemp = temp1 / 100.f;
#endif	
}
*/

static void drv_ms5837_CalcPT2nd(float *pPres,float *pTemp,uint32_t d1,uint32_t d2)
{
	int32_t pres1,temp1;
	int64_t dt,off,sens;
	int64_t ti,offi,sensi;
	int64_t off2,sens2;
	
	dt = (int64_t)(d2 - (C5 * powf(2,8)));
	temp1 = (int32_t)(2000 + ((dt * C6) / powf(2,23)));
#if DRV_MS5837_MODEL == 02
//MS5837-02BA01
	off = (int64_t)((C2 * powf(2,17)) + ((C4 * dt) / powf(2,6)));
	sens = (int64_t)(C1 * powf(2,16) + ((C3 * dt) / powf(2,7)));
	pres1 = (int32_t)(((d1 * sens / powf(2,21)) - off) / powf(2,15));
	
	if(temp1 < 2000)
	{
		//Low temperature
		ti = (int64_t)(11 * powf(dt,2) / powf(2,35));
		offi = (int64_t)(31 * powf(temp1 - 2000,2) / powf(2,3));
		sensi = (int64_t)(63 * powf(temp1 - 2000,2) / powf(2,5));
		
		off2 = off - offi;
		sens2 = sens - sensi;
		
		temp1 = temp1 - (int32_t)ti;
		pres1 = (int32_t)((((d1 * sens2) / powf(2,21) - off2) / powf(2,15)));
	}
	
	//单位C
	*pTemp = (float)temp1 / 100.f;
	//mbar
	*pPres = (float)pres1 / 100.f;
#else
//MS5837-30BA
	off = (int64_t)((C2 * powf(2,16)) + ((C4 * dt) / powf(2,7)));
	sens = (int64_t)(C1 * powf(2,15) + ((C3 * dt) / powf(2,8)));
//	pres1 = (int32_t)(((d1 * sens / powf(2,21)) - off) / powf(2,13));
	
	if(temp1 < 2000)
	{
		//Low temperature
		ti = (int64_t)(3 * powf(dt,2) / powf(2,33));
		offi = (int64_t)(3 * powf(temp1 - 2000,2) / 2);
		sensi = (int64_t)(5 * powf(temp1 - 2000,2) / powf(2,3));
		
		if(temp1 < -1500)
		{
			//very low temerature
			offi = (int64_t)(offi + 7 * powf(temp1 + 1500,2));
			sensi = (int64_t)(sensi + 4 * powf(temp1 + 1500,2));
		}
	}
	else
	{
		//High temperature
		ti = (int64_t)(2 * powf(dt,2) / powf(2,37));
		offi = (int64_t)(1 * powf(temp1 - 2000,2) / powf(2,4));
		sensi = 0;
	}
	
	off2 = off - offi;
	sens2 = sens - sensi;
	
	//单位C
	*pTemp = (float)(temp1 - (int32_t)ti) / 100.f;
	//mbar
	*pPres = (float)(((d1 * sens2) / powf(2,21) - off2) / powf(2,13)) / 10.f;
#endif
}

//气压单位：pa
//高度单位：m
float drv_ms5837_rel_altitude(float Press, float Ref_P)								//Calculate Relative Altitude
{
	return 44330 * (1 - pow(((float)Press / (float)Ref_P), (1/5.255)));
}

//气压单位：百pa
//高度单位：m
//气压计出现异常时通过高度反算出气压计值
float algo_altitude_to_press(float altit, float Ref_P)							
{
  
  float h_pa_data;
	
	h_pa_data =  Ref_P * pow((1 - altit/44300), 5.255)/100;
	return h_pa_data;
}

//气压单位：pa
//高度单位：mm
int32_t drv_ms5837_abs_altitude(uint32_t Press)
{
	int8_t P0 = 0;			
	int16_t hs1 = 0, dP0 = 0;			
	int32_t h0 = 0, hs0 = 0, HP1 = 0, HP2 = 0;		
	
	if(Press >= 103000)
	{	
		P0	=	103;
		h0	=	-138507;
		hs0	=	-21007;
		hs1	=	311;
	}	
	else if(Press >= 98000)
	{	
		P0	=	98;
		h0	=	280531;
		hs0	=	-21869;
		hs1	=	338;
	}	
	else if(Press >= 93000)
	{	
		P0	=	93;
		h0	=	717253;
		hs0	=	-22813;
		hs1	=	370;
	}	
				
	else if(Press >= 88000)
	{	
		P0	=	88;
		h0	=	1173421;
		hs0	=	-23854;
		hs1	=	407;
	}	
	else if(Press >= 83000)
	{	
		P0	=	83;
		h0	=	1651084;
		hs0	=	-25007;
		hs1	=	450;
	}	
	else if(Press >= 78000)
	{	
		P0	=	78;
		h0	=	2152645;
		hs0	=	-26292;
		hs1	=	501;
	}	
	else if(Press >= 73000)
	{	
		P0	=	73;
		h0	=	2680954;
		hs0	=	-27735;
		hs1	=	560;
	}	
	else if(Press >= 68000)
	{	
		P0	=	68;
		h0	=	3239426;
		hs0	=	-29366;
		hs1	=	632;
	}	
	else if(Press >= 63000)
	{	
		P0	=	63;
		h0	=	3832204;
		hs0	=	-31229;
		hs1	=	719;
	}	
	else if(Press >= 58000)
	{	
		P0	=	58;
		h0	=	4464387;
		hs0	=	-33377;
		hs1	=	826;
	}	
	else if(Press >= 53000)
	{	
		P0	=	53;
		h0	=	5142359;
		hs0	=	-35885;
		hs1	=	960;
	}		
	else if(Press >= 48000)
	{	
		P0	=	48;
		h0	=	5874268;
		hs0	=	-38855;
		hs1	=	1131;
	}	
	else if(Press >= 43000)
	{	
		P0	=	43;
		h0	=	6670762;
		hs0	=	-42434;
		hs1	=	1354;
	}	
	else if(Press >= 38000)
	{	
		P0	=	38;
		h0	=	7546157;
		hs0	=	-46841;
		hs1	=	1654;
	}	
	else if(Press >= 33000)
	{	
		P0	=	33;
		h0	=	8520395;
		hs0	=	-52412;
		hs1	=	2072;
	}	
	else if(Press > 0)
	{	
		P0	=	28;
		h0	=	9622536;
		hs0	=	-59704;
		hs1	=	2682;
	}
					
	dP0	=	Press - P0 * 1000;
				
	HP1	=	(hs0 * dP0) >> 2;
	HP2	=	(((hs1 * dP0) >> 10)* dP0) >> 4;			

	//单位 mm
	return	((h0 << 6) + HP1 + HP2) >> 6;
}

//气压单位：mbar/hpa
//高度单位：m
float drv_ms5837_Pressure2altitude(float p)
{
    float i, j, h;

    if (p<700.f)              				// **** [300:700[    mbar **** 
    {
        if (p<499.f)          				// **** [300:499[    mbar **** 
        {
            if (p<400.5f)      				// **** [300:400.5[  mbar **** 
            {
                if (p<349.f)  				// **** [300:349[    mbar ****
                {
                    i = 21.f;
                    j = 15458.f;
                }
                else                		// **** [349:400.5[  mbar **** 
                {
                    i = 18.6f;
                    j = 14620.f;
                }
            }
            else                    		// **** [400.5:499[  mbar **** 
            {
                if (p<448.5f)  				// **** [400.5:448.5[ mbar **** 
                {
                    i = 16.8f;
                    j = 13899.f;
                }
                else                		// **** [448.5:499[  mbar **** 
                {
                    i = 15.4f;
                    j = 13271.f;
                }
            }
        }
        else                        		// **** [499:700[    mbar **** 
        {
            if (p<599.f)      				// **** [499:599[    mbar **** 
            {
                if (p<549.f)  				// **** [499:549[    mbar **** 
                {
                    i = 14.2f;
                    j = 12672.f;
                }
                else                		// **** [549:599[    mbar **** 
                {
                    i = 13.2f;
                    j = 12123.f;
                }
            }
            else                    		// **** [599:700[    mbar **** 
            {
                if (p<648.5f)  				// **** [599:648.5[  mbar **** 
                {
                    i = 12.3f;
                    j = 11584.f;
                }
                else                		// **** [648.5:700[  mbar **** 
                {
                    i = 11.6f;
                    j = 11130.f;
                }
            }
        }
    }
    else                            		// **** [700:1100[   mbar **** 
    {
        if (p<897.5f)          				// **** [700:897.5[  mbar **** 
        {
            if (p<798.f)      				// **** [700:798[    mbar ****
            {
                if (p<744.f)  				// **** [700:744[    mbar **** 
                {
                    i = 10.9f;
                    j = 10640.f;
                }
                else                		// **** [744:798[    mbar **** 
                {
                    i = 10.4f;
                    j = 10266.f;
                }
            }
            else                    		// **** [798:897.5[  mbar **** 
            {
                if (p<850.f)  				// **** [798:850[    mbar **** 
                {
                    i = 9.8f;
                    j = 9787.f;
                }
                else                		// **** [850:897.5[  mbar **** 
                {
                    i = 9.4f;
                    j = 9447.f;
                }
            }
        }
        else                        		// **** [897.5:1100[ mbar **** 
        {
            if (p<1006.f)      				// **** [897.5:1006[ mbar **** 
            {
                if (p<945.f)  				// **** [897.5:945[  mbar **** 
                {
                    i = 9.f;
                    j = 9088.f;
                }
                else                		// **** [945:1006[   mbar **** 
                {
                    i = 8.6f;
                    j = 8710.f;
                }
            }
            else                    		// **** [1006:1100[  mbar **** 
            {
                i = 8.1f;
                j = 8207.f;
            }

        }
    }

    h = j - p * i;
    return(h);
}
