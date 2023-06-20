#include "compass.h"
#include "QSTApi.h"
	

float oldCompass = 0;
bool  ComCailFlag = false;

/********************************************************
Function	:void Magnet_Init(void)
Description	:初始化地磁,100HZ
Input		:
Output		:
Return		:
Other		:
*********************************************************/
void Magnet_Init(void)
{
	uint8_t data = 0x0C;
	
	I2C_MySendData(LIS3MDL_ADDR,0x21,&data,1);
	delay_mms(200);
//	data = 0xfc;
//	I2C_MySendData(LIS3MDL_ADDR,0x20,&data,1);
//	data = 0x40;
//	I2C_MySendData(LIS3MDL_ADDR,0x21,&data,1);
//	data = 0x00;
//	I2C_MySendData(LIS3MDL_ADDR,0x22,&data,1);
//	data = 0x0c;
//	I2C_MySendData(LIS3MDL_ADDR,0x23,&data,1);
//	data = 0x00;
//	I2C_MySendData(LIS3MDL_ADDR,0x24,&data,1);
	/* set ODR 100Hz*/
	data = 0x40;
	I2C_MySendData(LIS3MDL_ADDR,0x21,&data,1);
	data = 0xfc;
	I2C_MySendData(LIS3MDL_ADDR,0x20,&data,1);
	data = 0x0c;
	I2C_MySendData(LIS3MDL_ADDR,0x23,&data,1);
	data = 0x00;
	I2C_MySendData(LIS3MDL_ADDR,0x22,&data,1);
}

/********************************************************
Function	:void Magnet_Start(void)
Description	:打开地磁
Input		:
Output		:
Return		:
Other		:
*********************************************************/
void Magnet_Start(void)
{
	Heartrate_PowerOn();	//打开传感器电源 
	Timerx_Start(TIMER5,100);		/* 延时50ms */
	while(1)
	{
		/*Enter SLeep Mode*/
		EnterSleep();
		if(TimeoverFlag5)			/* 延时标志位 */
		{
			Timerx_Close(TIMER5);
			TimeoverFlag5 = false;
			break;
		}
	}
	
	I2C_Start();			//打开I2C1
	if(Heartrate_Who_Am_I() != false)
	{
		Heartrate_EnterPowerDown();	//心率进入低功耗模式
	}
	else
	{
		I2C_Start();
	}

	if(Step_Who_AM_I() != false)
	{		
		Step_Init();
	
		Step_Walking();
	}
	else
	{
		I2C_Start();
	}
	
	if(Magnet_Who_AM_I() != false)
	{		
		Magnet_Init();
		
		akm_cal_init();	//初始化校准
	}
	else
	{
		I2C_Start();
	}
	
}

/********************************************************
Function	:void Magnet_Close(void)
Description	:关闭地磁
Input		:
Output		:
Return		:
Other		:
*********************************************************/
void Magnet_Close(void)
{
	I2C_Close();
	
	/* 关闭电源 */
	Heartrate_PowerOff();
}

/********************************************************
Function	:bool Magnet_Who_AM_I(void)
Description	:读取地磁ID，判断是否正常
Input		:
Output		:
Return		:地磁状态
Other		:
*********************************************************/
bool Magnet_Who_AM_I(void)
{
	uint8_t data = 0;
	TypeState state = ERROR;
	
	state = I2C_MyReceData(LIS3MDL_ADDR,0x0F,&data,1);
	if((data == 0x3D) && (state != ERROR))
	{
		//地磁正常
		BKP_WriteBackupRegister(BKP_ERROR, READ_ERROR | STATE_MAGNET);
		return true;
	}
	else
	{
		//地磁异常
		BKP_WriteBackupRegister(BKP_ERROR, READ_ERROR & (~STATE_MAGNET));
		return false;
	}
	
}

/********************************************************
Function	:void Magnet_GetData(float *hw_d)
Description	:读取地磁三轴数据
Input		:
Output		:
Return		:
Other		:
*********************************************************/
void Magnet_GetData(float *hw_d)
{
	uint8_t data = 0;
	uint8_t mag_data[6];
	int16_t buf[3];
	
	I2C_MyReceData(LIS3MDL_ADDR,0x27,&data,1);
	if(data & 0x08)
	{	
		I2C_MyReceData(LIS3MDL_ADDR,0x28,&data,1);
		mag_data[0] = data;
		I2C_MyReceData(LIS3MDL_ADDR,0x29,&data,1);
		mag_data[1] = data;
		I2C_MyReceData(LIS3MDL_ADDR,0x2A,&data,1);
		mag_data[2] = data;
		I2C_MyReceData(LIS3MDL_ADDR,0x2B,&data,1);
		mag_data[3] = data;
		I2C_MyReceData(LIS3MDL_ADDR,0x2C,&data,1);
		mag_data[4] = data;
		I2C_MyReceData(LIS3MDL_ADDR,0x2D,&data,1);
		mag_data[5] = data;
		buf[0] = ((mag_data[1]<<8) | mag_data[0]);
		buf[1] = ((mag_data[3]<<8) | mag_data[2]);
		buf[2] = ((mag_data[5]<<8) | mag_data[4]);
			
		hw_d[0] = (float)buf[0] * 0.04;
		hw_d[1] = (float)buf[1] * 0.04;
		hw_d[2] = (float)buf[2] * 0.04;
		
	}
	else
	{
		hw_d[0] = 0;
		hw_d[1] = 0;
		hw_d[2] = 0;
	}
}

/********************************************************
Function	:uin16_t Magnet_Cail_Auto(void)
Description	:自动校准，得到方向角
Input		:
Output		:
Return		:
Other		:
*********************************************************/
float Magnet_Cail_Auto(void)
{
	float data[3];
	float data_C[3];
	int16_t	mstat=0;
	float tmpx, tmpy, tmpz;
	int16_t tmp_accuracy;
	float Data_ACC[3];
	float av;
	float pitch, roll, azimuth;
	const float rad2deg = 180 / 3.1415926;
	int16_t s_x,s_y,s_z;
	
	//读取地磁数据
	Magnet_GetData(data);
	
	data_C[0]=1*data[0];
	data_C[1]=1*data[1];
	data_C[2]=0.83*data[2];
	data_C[2]=1*data[2];
	
#ifdef COMPASS_DEBUG
	printf("1---x=%.2f,y=%.2f,z=%.2f\n",data_C[0],data_C[1],data_C[2]);
#endif		
	convert_magnetic(data_C,mstat,&tmpx, &tmpy, &tmpz, &tmp_accuracy);
#ifdef COMPASS_DEBUG
	printf("tmp_accuracy = %d\n",tmp_accuracy);
#endif
	if(tmp_accuracy == 3)
	{
		ComCailFlag = true;
		BKP_WriteBackupRegister(BKP_OFFSET_X,(int16_t)(data_C[0]-tmpx));
		BKP_WriteBackupRegister(BKP_OFFSET_Y,(int16_t)(data_C[1]-tmpy));
		BKP_WriteBackupRegister(BKP_OFFSET_Z,(int16_t)(data_C[2]-tmpz));
	}
	else
	{
		s_x = BKP_ReadBackupRegister(BKP_OFFSET_X);
		tmpx = data_C[0]-s_x;
		s_y = BKP_ReadBackupRegister(BKP_OFFSET_Y);
		tmpy = data_C[1]-s_y;
		s_z = BKP_ReadBackupRegister(BKP_OFFSET_Z);
		tmpz = data_C[2]-s_z;
	}
#ifdef COMPASS_DEBUG
	printf("2---x=%.2f,y=%.2f,z=%.2f\n",tmpx,tmpy,tmpz);
#endif		

	//读取计步三轴数据
	Data_ACC[0] = ReadGSensorX();
	Data_ACC[1] = ReadGSensorY();
	Data_ACC[2] = ReadGSensorZ();
	
#ifdef COMPASS_DEBUG
	printf("Ax=%.2f,Ay=%.2f,Az=%.2f\n",Data_ACC[0],Data_ACC[1],Data_ACC[2]);
#endif
	
	av = sqrtf(Data_ACC[0]*Data_ACC[0] + Data_ACC[1]*Data_ACC[1] + Data_ACC[2]*Data_ACC[2]);
	pitch = asinf(-Data_ACC[1] / av);
	roll = asinf(Data_ACC[0] / av);

	azimuth = atan2(-(tmpx) * cosf(roll) + tmpz * sinf(roll),
			tmpx*sinf(pitch)*sinf(roll) + tmpy*cosf(pitch) + tmpz*sinf(pitch)*cosf(roll));
	azimuth =  azimuth * rad2deg;
	pitch = pitch * rad2deg;
	roll = roll * rad2deg;

	azimuth -= 180;
	if(azimuth<0)
	{
		azimuth+= 360.0f;
	}
	
#ifdef COMPASS_DEBUG
	printf("Angle = %f\n",azimuth);
#endif	
	return  azimuth;
}

/********************************************************
Function	:bool Magnet_Cail_Manual(void)
Description	:手动校准
Input		:
Output		:
Return		:返回校准状态
Other		:
*********************************************************/
bool Magnet_Cail_Manual(void)
{
	float data[3];
	float data_C[3];
	int16_t	mstat=0;
	float tmpx, tmpy, tmpz;
	int16_t tmp_accuracy;
	
	//读取地磁数据
	Magnet_GetData(data);
	
	data_C[0]=1*data[0];
	data_C[1]=1*data[1];
	data_C[2]=0.83*data[2];
	data_C[2]=1*data[2];
		
	convert_magnetic(data_C,mstat,&tmpx, &tmpy, &tmpz, &tmp_accuracy);
	
	if(tmp_accuracy == 3)
	{
		return true;
	}
	else
	{
		return false;
	}
}





