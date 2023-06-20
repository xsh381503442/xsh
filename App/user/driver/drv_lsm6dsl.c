

/*********************************************************************/
/* own header files */
#include "stdio.h"
#include "drv_lsm6dsl.h"
//#include "define.h"
#include "bsp.h"
#include "bsp_iom.h"
#include "Compass/akm_oss_wrapper.h"
#include "task_config.h"
#include "task_step.h"
#include "task_ble.h"
#include "com_data.h"
#include "time_notify.h"
#include "drv_lsm6dsl.h"
#if defined (WATCH_STEP_ALGO)
#include "algo_step_count.h"
#endif 

/*********************************************************************/


#define DRV_LSM6DSL_WRIST_TILT_POS_THS             55                      //arcsin(ths/64)
//#define DRV_LSM6DSL_WRIST_TILT_NEG_THS             32                      //arcsin(ths/64)
#define DRV_LSM6DSL_WRIST_TILT_NEG_THS             64                      //arcsin(ths/64)
//#define DRV_LSM6DSL_WRIST_TILT_POS_LAT             10                       // lat * 40ms
#define DRV_LSM6DSL_WRIST_TILT_NEG_LAT             1                       // lat * 40ms
#define DRV_LSM6DSL_WRIST_TILT_MASK                0x70						//Xneg,Ypos,Yneg
#if defined (WATCH_STEP_ALGO)
#define DRV_LSM6DSL_WRIST_TILT_Z_POS_THS           0x1BBA                  // 60 degree
#else
#define DRV_LSM6DSL_WRIST_TILT_Z_POS_THS           0x3774                 // 60 degree
#endif 
#define DRV_LSM6DSL_WRIST_TILT_Z_NEG_THS           -0x2009                 // 30 degree
#define DRV_LSM6DSL_WRIST_TILT_X_THS               -0x2000                   //x axes action

#define TURNING_UP_JUDGING_Q_SIZE                        6

//turning state
enum{
    TURNING_ST_DOWN,
    TURNING_ST_TURNING_UP,
    TURNING_ST_UP
};

int32_t turn_up_Q[TURNING_UP_JUDGING_Q_SIZE];


#define     BSP_LSM6DSL_IOM			3

unsigned short StepCount = 0; 
uint32_t LastStepCount =0;

TimerHandle_t wrist_tilt_timer;
wrist_tilt_cb_t wrist_tilt_cb = {0};

extern SemaphoreHandle_t Appolo_I2C_Semaphore;

#define DRV_LSM6DSL_WRIST_TILT_TURN_BACK    1

//int16_t		g_CompassCaliStatus;	//?????У???	3-???

//float           Compass_Offset_x = 0;       //?????XУ??
//float           Compass_Offset_y = 0;       //?????YУ??
//float           Compass_Offset_z = 0;       //?????ZУ??


/*********************************************************************/
/* static variables */


/*********************************************************************/
/* functions */

/*!
 * @brief: 
 *             
 * @param[in] : 
 * @param[out] : 
 * @param[in,out] : 
 *
 * @return :
 */ 

const am_hal_iom_config_t g_sLSM6DSL_IOMConfig =
{
    .ui32InterfaceMode = AM_HAL_IOM_I2CMODE,
    .ui32ClockFrequency = AM_HAL_IOM_400KHZ,
    .bSPHA = 0,
    .bSPOL = 0,
    .ui8WriteThreshold = 4,
    .ui8ReadThreshold = 60,
};
static void drv_lsm6dsl_enable(void)
{
	xSemaphoreTake( Appolo_I2C_Semaphore, portMAX_DELAY );
	
	//DRV_NDOF_LOG_PRINTF(" drv_bmi160_enable\n");
	
	// Enable IOM
	bsp_iom_enable(BSP_LSM6DSL_IOM,g_sLSM6DSL_IOMConfig);
}

static void drv_lsm6dsl_disable(void)
{
	//DRV_NDOF_LOG_PRINTF(" drv_bmi160_disable\n");
	
	//Disable IOM
	bsp_iom_disable(BSP_LSM6DSL_IOM);
	
	xSemaphoreGive( Appolo_I2C_Semaphore );	
}
/*!
 * @brief: configure the i2c write 
 *             
 * @param[in] : 
 * @param[out] : 
 * @param[in,out] : 
 *
 * @return :
 */ 
uint8_t Sensor_IO_Write( void *handle, uint8_t WriteAddr, uint8_t *pBuffer, uint16_t nBytesToWrite )
//int8_t SENSOR_I2C_WRITE(unsigned char device_addr, unsigned char register_addr, unsigned char *register_data, unsigned char length)
{
	//DrvContextTypeDef *ctx = (DrvContextTypeDef *)handle;
  
    int i;
    am_hal_iom_status_e ui32Status;
    am_hal_iom_buffer(32) sCommand ={0};
    
    for (i = 0; i < nBytesToWrite; i++)
    {
        sCommand.bytes[i] =  pBuffer[i];
    }
    
    drv_lsm6dsl_enable();
	ui32Status = am_hal_iom_i2c_write(BSP_LSM6DSL_IOM,0x6B, sCommand.words, nBytesToWrite,  AM_HAL_IOM_OFFSET(WriteAddr));
    drv_lsm6dsl_disable();
    return ui32Status;
}

uint8_t		Write_SIM( void )
{
    am_hal_iom_status_e ui32Status;
    am_hal_iom_buffer(32) sCommand ={0};
    uint8_t     address = 0;
    
    drv_lsm6dsl_enable();
    ui32Status = am_hal_iom_i2c_read(BSP_LSM6DSL_IOM,0x6B, sCommand.words, 1,  AM_HAL_IOM_OFFSET(LSM6DSL_ACC_GYRO_WHO_AM_I_REG));
    if( sCommand.bytes[0] == 0x6A )
    {
        address = 0x6B;
    }
    else
    {
        ui32Status = am_hal_iom_i2c_read(BSP_LSM6DSL_IOM,0x6A, sCommand.words, 1,  AM_HAL_IOM_OFFSET(LSM6DSL_ACC_GYRO_WHO_AM_I_REG));
        if( sCommand.bytes[0] == 0x6A )
        {
            address = 0x6A;
        }
        else
        {
            drv_lsm6dsl_disable();
            return 1;
        }
    }
    ui32Status = am_hal_iom_i2c_read(BSP_LSM6DSL_IOM,address, sCommand.words, 1,  AM_HAL_IOM_OFFSET(LSM6DSL_ACC_GYRO_CTRL3_C));
    sCommand.bytes[0] &= ~LSM6DSL_ACC_GYRO_SIM_MASK;
    sCommand.bytes[0] |=  0x08;
	ui32Status = am_hal_iom_i2c_write(BSP_LSM6DSL_IOM,address, sCommand.words, 1,  AM_HAL_IOM_OFFSET(LSM6DSL_ACC_GYRO_CTRL3_C));
    drv_lsm6dsl_disable();
    return 0;
}

/*!
 * @brief: configure the i2c read 
 *             
 * @param[in] : 
 * @param[out] : 
 * @param[in,out] : 
 *
 * @return :
 */
uint8_t Sensor_IO_Read( void *handle, uint8_t ReadAddr, uint8_t *pBuffer, uint16_t nBytesToRead )
//int8_t SENSOR_I2C_READ(unsigned char device_addr, unsigned char register_addr, unsigned char *register_data, unsigned char length)
{
	//DrvContextTypeDef *ctx = (DrvContextTypeDef *)handle;
    int i;
    am_hal_iom_status_e ui32Status;
    am_hal_iom_buffer(32) sResult = {0};
 
    drv_lsm6dsl_enable();
    ui32Status = am_hal_iom_i2c_read(BSP_LSM6DSL_IOM,0x6B, sResult.words, nBytesToRead,  AM_HAL_IOM_OFFSET(ReadAddr));
    drv_lsm6dsl_disable();
    for (i = 0; i < nBytesToRead; i++ )
	{
       *pBuffer++ = sResult.bytes[i];
    }
    return ui32Status;
 }

#if 0
/*!
 * @brief: configure the i2c burst read 
 *             
 * @param[in] : 
 * @param[out] : 
 * @param[in,out] : 
 *
 * @return :
 */ 
    am_hal_iom_buffer(1024) BURSTREAD ={0};

int8_t SENSOR_I2C_BURST_READ(unsigned char device_addr, unsigned char register_addr, unsigned char *register_data, uint32_t length)
{
  
    int i;
    //am_hal_iom_buffer(32) sResult ={0};
    memset(&BURSTREAD,0,1024);
    
    drv_lsm6dsl_enable();
    am_hal_iom_i2c_read(BSP_LSM6DSL_IOM,device_addr, BURSTREAD.words, length,  AM_HAL_IOM_OFFSET(register_addr));
    drv_lsm6dsl_disable();
    for (i = 0; i < length; i++ )
  {
       *register_data++ = BURSTREAD.bytes[i];
   }
    return COMPONENT_OK;
}
#endif
DrvStatusTypeDef drv_lsm6dsl_enable_pedometer(void)
{
	
	uint8_t value;
	void *handle = NULL;
	if( !LSM6DSL_ACC_GYRO_ReadReg(handle, LSM6DSL_ACC_GYRO_CTRL6_G, &value, 1))
    return COMPONENT_ERROR;
	
	value &= ~LSM6DSL_ACC_GYRO_LP_XL_MASK;
	value |= (uint8_t)LSM6DSL_ACC_GYRO_LP_XL_ENABLED;
	
	if( !LSM6DSL_ACC_GYRO_WriteReg(handle, LSM6DSL_ACC_GYRO_CTRL6_G, &value, 1) )
    return COMPONENT_ERROR;
	
	
	if( !LSM6DSL_ACC_GYRO_ReadReg(handle, LSM6DSL_ACC_GYRO_CTRL1_XL, &value, 1))
    return COMPONENT_ERROR;
	
	value &= ~LSM6DSL_ACC_GYRO_ODR_XL_MASK;	
	value |= ( uint8_t)LSM6DSL_ACC_GYRO_ODR_XL_26Hz;
	
	value &= ~LSM6DSL_ACC_GYRO_FS_XL_MASK;
	value |= (uint8_t)LSM6DSL_ACC_GYRO_FS_XL_2g;
	
	if( !LSM6DSL_ACC_GYRO_WriteReg(handle, LSM6DSL_ACC_GYRO_CTRL1_XL, &value, 1) )
    return COMPONENT_ERROR;


    //LSM6DSL_ACC_GYRO_W_PedoFS(handle,0x80);

	
	  /* Set pedometer threshold. */
	if ( LSM6DSL_ACC_GYRO_W_PedoThreshold( handle, LSM6DSL_PEDOMETER_THRESHOLD_MID_HIGH) != MEMS_SUCCESS )
	{
		return COMPONENT_ERROR;
	}
    
  //  /* Set pedometer DEB TIME, 31*80 =2480 ms */
 // if ( LSM6DSL_ACC_GYRO_W_PedoDebTime( handle, 0x1F ) != MEMS_SUCCESS )
//	{
//		return COMPONENT_ERROR;
 //	}
 
   /* Set pedometer DEB step */
    if ( LSM6DSL_ACC_GYRO_W_PedoDebStep( handle, 0x07 ) != MEMS_SUCCESS )
    {
        return COMPONENT_ERROR;
    }
	if ( LSM6DSL_ACC_GYRO_W_FUNC_EN( (void *)handle, LSM6DSL_ACC_GYRO_FUNC_EN_ENABLED ) == MEMS_ERROR )
    {
		return COMPONENT_ERROR;
    }
	
	/* Enable pedometer algorithm. */
	if ( LSM6DSL_ACC_GYRO_W_PEDO( (void *)handle, LSM6DSL_ACC_GYRO_PEDO_ENABLED ) == MEMS_ERROR )
	{
		return COMPONENT_ERROR;
	}
	
	if( !LSM6DSL_ACC_GYRO_ReadReg(handle, LSM6DSL_ACC_GYRO_CTRL2_G, &value, 1))
    return COMPONENT_ERROR;
	
//	/* Enable pedometer on INT1 pin */
   // if ( LSM6DSL_ACC_GYRO_W_STEP_DET_on_INT1( (void *)handle, LSM6DSL_ACC_GYRO_INT1_PEDO_ENABLED ) == MEMS_ERROR )
  //  {
	//	return COMPONENT_ERROR;
  //  }
    
 
	return COMPONENT_OK;
}

DrvStatusTypeDef drv_lsm6dsl_disable_pedometer(void)
{
        void *handle = NULL;
    /* Disable pedometer algorithm. */
       if ( LSM6DSL_ACC_GYRO_W_PEDO( (void *)handle, LSM6DSL_ACC_GYRO_PEDO_DISABLED ) == MEMS_ERROR )
        {
            return COMPONENT_ERROR;
        }
        return COMPONENT_OK;
}

//启动翻腕亮屏
DrvStatusTypeDef drv_lsm6dsl_wrist_tilt_enable(void)
{
     void *handle = NULL; 
   
     /* Set wrist tilt recognition*/
        if ( LSM6DSL_ACC_GYRO_W_WristTiltThreshold(handle, DRV_LSM6DSL_WRIST_TILT_NEG_THS, DRV_LSM6DSL_WRIST_TILT_NEG_LAT , DRV_LSM6DSL_WRIST_TILT_MASK) != MEMS_SUCCESS)
        {
		    return COMPONENT_ERROR;
        }  
    
	
	return COMPONENT_OK;
}

//禁用翻腕亮屏
DrvStatusTypeDef drv_lsm6dsl_wrist_tilt_disable(void)
{   
    void *handle = NULL;
 
      
      if( !LSM6DSL_ACC_GYRO_AWT_DISABLE(handle))
        return COMPONENT_ERROR;
 
	
	return COMPONENT_OK;
}

//清计步数
void drv_lsm6dsl_clear_stepcount(void)
{
    void *handle = NULL;
    LSM6DSL_ACC_GYRO_W_PedoStepReset(handle,LSM6DSL_ACC_GYRO_PEDO_RST_STEP_ENABLED);
    am_util_delay_ms(10);
    LSM6DSL_ACC_GYRO_W_PedoStepReset(handle,LSM6DSL_ACC_GYRO_PEDO_RST_STEP_DISABLED);
		am_util_delay_ms(10);
    StartSteps = 0;
    LastStepCount = 0;
}

//获取总步数
uint32_t drv_lsm6dsl_get_stepcount( void )
{
#if defined WATCH_STEP_ALGO
  return (step_data_str.passometer_mun + step_data_str.step_num_sport);

#else
 void *handle = NULL;

	if (LSM6DSL_ACC_GYRO_Get_GetStepCounter(handle,(uint8_t *)(&StepCount))==MEMS_SUCCESS)
	{
		if (LastStepCount > StepCount)//防止传感器数值减少
		{
			StartSteps +=(LastStepCount - StepCount);
		}
		LastStepCount = StepCount;

		return (StepCount + StartSteps);
	}
	else
	{
		return (LastStepCount + StartSteps); //防止i2c出错，出错时用上个值
	}		
#endif
}


DrvStatusTypeDef  drv_lsm6dsl_reset( void)
{	
    void *handle = NULL;
    
//	LSM6DSL_ACC_GYRO_W_ODR_XL( handle, LSM6DSL_ACC_GYRO_ODR_XL_POWER_DOWN );
//	LSM6DSL_ACC_GYRO_W_ODR_G( handle, LSM6DSL_ACC_GYRO_ODR_G_POWER_DOWN );
	LSM6DSL_ACC_GYRO_W_SW_RESET( handle, LSM6DSL_ACC_GYRO_SW_RESET_RESET_DEVICE);
    Write_SIM();
	LSM6DSL_ACC_GYRO_W_PULL_UP_EN(handle,LSM6DSL_ACC_GYRO_PULL_UP_EN_ENABLED);
	return COMPONENT_OK;
}

bool drv_lsm6dsl_id(void)
{
    uint8_t id;
    void *handle = NULL;
    LSM6DSL_ACC_GYRO_R_WHO_AM_I(handle,&id);

    	
	if(id == 0x6a)
	{
		return true;
	}
	else
	{
		return false;
	}

}

bool drv_lsm6dsl_sleep_status(void)
{
    LSM6DSL_ACC_GYRO_SLEEP_EV_STATUS_t status;
    void *handle = NULL;
    LSM6DSL_ACC_GYRO_R_SLEEP_EV_STATUS(handle, &status);

    	
	if(status == LSM6DSL_ACC_GYRO_SLEEP_EV_STATUS_DETECTED)
	{
		return true;
	}
	else
	{
		return false;
	}

}

static uint8_t turn_up_state_check(int32_t g)
{
    uint8_t i;
    static uint8_t q_head = 0;
    wrist_tilt_cb.up_state_count=0;
    if(q_head >= TURNING_UP_JUDGING_Q_SIZE)
        q_head = 0;
    turn_up_Q[q_head++] = g;

    for(i=0;i<TURNING_UP_JUDGING_Q_SIZE;i++)
    {
        if(turn_up_Q[i] > DRV_LSM6DSL_WRIST_TILT_Z_POS_THS)
            wrist_tilt_cb.up_state_count++;
    }
    if(wrist_tilt_cb.up_state_count >= TURNING_UP_JUDGING_Q_SIZE-1)
        return true;
    else
        return false;
}

// Define a callback function that will be used by multiple timer instances.
// The callback function does nothing but count the number of times the
// associated timer expires, and stop the timer once the timer has expired
// 10 times.
static void vTimerCallback( TimerHandle_t pxTimer )
{
    int16_t axes_data[3];
    const int32_t xMaxExpiryCountBeforeStopping = 19;       //hand raise timeout 2.09s
    
#ifdef DRV_LSM6DSL_WRIST_TILT_TURN_BACK
  //  void *handle = NULL;
#endif
	// Optionally do something if the pxTimer parameter is NULL.
	configASSERT( pxTimer );

    // Increment the number of times that pxTimer has expired.
    wrist_tilt_cb.lExpireCounters += 1;
    
    //Read acc data
    drv_lsm6dsl_acc_axes_raw(axes_data);
    TASK_STEP_PRINTF("x:%x, y:%x, z:%x state: %x\n", axes_data[0], axes_data[1], axes_data[2],wrist_tilt_cb.turning_state);
    //check wrist turn up
//    if(TURNING_ST_DOWN == wrist_tilt_cb.turning_state && axes_data[0] < DRV_LSM6DSL_WRIST_TILT_X_THS)
//    {
//        wrist_tilt_cb.turning_state = TURNING_ST_TURNING_UP;
//    }
    
    
     // If the timer has expired xMaxExpiryCountBeforeStopping times then stop it from running for low power consideration.
    if( wrist_tilt_cb.lExpireCounters == xMaxExpiryCountBeforeStopping )
    {
         // Do not use a block time if calling a timer API function from a
         // timer callback function, as doing so could cause a deadlock!
         xTimerStop( pxTimer, 0 );
         wrist_tilt_cb.lExpireCounters = 0;
         wrist_tilt_cb.is_verify_timer_started = 0;
         wrist_tilt_cb.up_state_count = 0;
         memset(turn_up_Q,0,sizeof(turn_up_Q));
    }
    
    //temparaly use 
    //if( wrist_tilt_cb.turning_state == TURNING_ST_TURNING_UP )
    {    
        if(turn_up_state_check(axes_data[2])){
            TASK_STEP_PRINTF("up state count:%x\n",wrist_tilt_cb.up_state_count);
            timer_notify_backlight_start(SetValue.backlight_timeout_mssec,true);
            xTimerStop( pxTimer, 0 );
            wrist_tilt_cb.lExpireCounters = 0;
            wrist_tilt_cb.is_verify_timer_started = 0;
            wrist_tilt_cb.up_state_count = 0;
            memset(turn_up_Q,0,sizeof(turn_up_Q));
            
#ifdef DRV_LSM6DSL_WRIST_TILT_TURN_BACK
//            wrist_tilt_cb.turning_state = TURNING_ST_UP;
#endif
        }
    }
    
#ifdef DRV_LSM6DSL_WRIST_TILT_TURN_BACK
//    if(wrist_tilt_cb.turning_state == TURNING_ST_UP && /*wrist_tilt_cb.negative_count >= 3*/ -DRV_LSM6DSL_WRIST_TILT_Z_NEG_THS >  axes_data[2]){
//        wrist_tilt_cb.turning_state = TURNING_ST_DOWN;
//        memset(turn_up_Q,0,sizeof(turn_up_Q));
//    }
#endif

		
}

 
void timer_wrist_tilt_init(void)
{
    wrist_tilt_timer = xTimerCreate(    "Timer",       // Just a text name, not used by the kernel.
                                        ( 60  ),   // The timer period in ticks.
                                        pdTRUE,        // The timers will auto-reload themselves when they expire.
                                        ( void * ) 46,  // Assign each timer a unique id equal to its array index.
                                        vTimerCallback // Each timer calls the same callback when it expires.
                                    );
}

DrvStatusTypeDef  drv_lsm6dsl_acc_init(void)
{
	uint8_t value;
	void *handle = NULL;
	LSM6DSL_ACC_GYRO_W_SW_RESET( handle, LSM6DSL_ACC_GYRO_SW_RESET_RESET_DEVICE);
	Write_SIM();
	LSM6DSL_ACC_GYRO_W_PULL_UP_EN(handle,LSM6DSL_ACC_GYRO_PULL_UP_EN_ENABLED);
	if( !LSM6DSL_ACC_GYRO_ReadReg(handle, LSM6DSL_ACC_GYRO_CTRL1_XL, &value, 1))
    return COMPONENT_ERROR;
	
	value &= ~LSM6DSL_ACC_GYRO_ODR_XL_MASK;
	

	value |= ( uint8_t)LSM6DSL_ACC_GYRO_ODR_XL_26Hz;  //????ж???? =n*512/26 = n * 19.7s   

	value &= ~LSM6DSL_ACC_GYRO_FS_XL_MASK;
	#if defined (WATCH_STEP_ALGO)
	value |= (uint8_t)LSM6DSL_ACC_GYRO_FS_XL_8g;
	#else
	value |= (uint8_t)LSM6DSL_ACC_GYRO_FS_XL_2g;
	#endif
  
	if( !LSM6DSL_ACC_GYRO_WriteReg(handle, LSM6DSL_ACC_GYRO_CTRL1_XL, &value, 1) )
    return COMPONENT_ERROR;
    

	LSM6DSL_ACC_GYRO_W_SLEEP_DUR(handle,1); //SLEEP 19.7S  WAKEUP 0
	
	LSM6DSL_ACC_GYRO_W_WAKE_DUR(handle,1);
	
	LSM6DSL_ACC_GYRO_W_WK_THS(handle,3); // n*XL_FS/64


	value = 0xE0;
	
	if( !LSM6DSL_ACC_GYRO_WriteReg(handle, LSM6DSL_ACC_GYRO_TAP_CFG1, &value, 1) )
    return COMPONENT_ERROR;

#if !defined WATCH_RAISE_BRIGHT_SCREEN_ALGO
    if(system_raiselight_mode_get() == 1)
     {
        drv_lsm6dsl_wrist_tilt_enable();
     }
#endif


#if defined (WATCH_STEP_ALGO)
	drv_lsm6dsl_acc_lowpower();
#endif

	 return COMPONENT_OK;
	
    
//	value = 0x80;
//	if( !LSM6DSL_ACC_GYRO_WriteReg(handle, LSM6DSL_ACC_GYRO_MD1_CFG, &value, 1) )
//    return COMPONENT_ERROR;	
	
}

void  drv_lsm6dsl_acc_powerdown(void)
{	
   void *handle = NULL;
   LSM6DSL_ACC_GYRO_W_ODR_XL(handle,LSM6DSL_ACC_GYRO_ODR_XL_POWER_DOWN);

}

void  drv_lsm6dsl_acc_lowpower(void)
{	
   void *handle = NULL;
   LSM6DSL_ACC_GYRO_W_LowPower_XL(handle,LSM6DSL_ACC_GYRO_LP_XL_ENABLED);

}

void  drv_lsm6dsl_gyro_lowpower(void)
{	
   void *handle = NULL;
   LSM6DSL_ACC_GYRO_W_LP_Mode(handle,LSM6DSL_ACC_GYRO_LP_EN_ENABLED);

}


void  drv_lsm6dsl_gyro_init(void)
{	
    void *handle = NULL;
    LSM6DSL_ACC_GYRO_W_ODR_G(handle,LSM6DSL_ACC_GYRO_ODR_G_26Hz);
    LSM6DSL_ACC_GYRO_W_FS_G(handle,LSM6DSL_ACC_GYRO_FS_G_1000dps);
    drv_lsm6dsl_gyro_lowpower();

}

void  drv_lsm6dsl_gyro_powerdown(void)
{	
   void *handle = NULL;
   LSM6DSL_ACC_GYRO_W_ODR_G(handle,LSM6DSL_ACC_GYRO_ODR_G_POWER_DOWN);

}

DrvStatusTypeDef drv_lsm6dsl_acc_axes_raw(int16_t *pData)
{
  void *handle = NULL;
  uint8_t regValue[6] = {0, 0, 0, 0, 0, 0};

  if ( LSM6DSL_ACC_GYRO_GetRawAccData( (void *)handle, ( uint8_t* )regValue ) == MEMS_ERROR )
  {
    return COMPONENT_ERROR;
  }

  /* Format the data. */
  pData[0] = ( ( ( ( int16_t )regValue[1] ) << 8 ) + ( int16_t )regValue[0] );
  pData[2] = ( ( ( ( int16_t )regValue[5] ) << 8 ) + ( int16_t )regValue[4] );
  pData[1] = ( ( ( ( int16_t )regValue[3] ) << 8 ) + ( int16_t )regValue[2] );

  return COMPONENT_OK;
}

DrvStatusTypeDef drv_lsm6dsl_gyro_axes_raw(int16_t *pData)
{

  uint8_t regValue[6] = {0, 0, 0, 0, 0, 0};
  void *handle = NULL;
 
  if ( LSM6DSL_ACC_GYRO_GetRawGyroData( (void *)handle, ( uint8_t* )regValue ) == MEMS_ERROR )
  {
    return COMPONENT_ERROR;
  }

  /* Format the data. */
  pData[0] = ( ( ( ( int16_t )regValue[1] ) << 8 ) + ( int16_t )regValue[0] );
  pData[1] = ( ( ( ( int16_t )regValue[3] ) << 8 ) + ( int16_t )regValue[2] );
  pData[2] = ( ( ( ( int16_t )regValue[5] ) << 8 ) + ( int16_t )regValue[4] );

  return COMPONENT_OK;
}





void drv_lsm6dsl_fifo_init( void)
{	
    void *handle = NULL;
    LSM6DSL_ACC_GYRO_W_DEC_FIFO_XL(handle,LSM6DSL_ACC_GYRO_DEC_FIFO_XL_NO_DECIMATION);
	LSM6DSL_ACC_GYRO_W_DEC_FIFO_G(handle,LSM6DSL_ACC_GYRO_DEC_FIFO_G_NO_DECIMATION);
	 
    LSM6DSL_ACC_GYRO_W_ODR_FIFO(handle,LSM6DSL_ACC_GYRO_ODR_FIFO_25Hz);
    LSM6DSL_ACC_GYRO_W_FIFO_Watermark(handle,400);
    LSM6DSL_ACC_GYRO_W_STOP_ON_FTH(handle,LSM6DSL_ACC_GYRO_STOP_ON_FTH_ENABLED);
    LSM6DSL_ACC_GYRO_W_FIFO_MODE(handle,LSM6DSL_ACC_GYRO_FIFO_MODE_FIFO);

}

void  drv_lsm6dsl_fifo_uninit(void)
{	
    void *handle = NULL;
    LSM6DSL_ACC_GYRO_W_FIFO_MODE(handle,LSM6DSL_ACC_GYRO_FIFO_MODE_BYPASS);
    LSM6DSL_ACC_GYRO_W_STOP_ON_FTH(handle,LSM6DSL_ACC_GYRO_STOP_ON_FTH_DISABLED);
    LSM6DSL_ACC_GYRO_W_FIFO_Watermark(handle,0);
    LSM6DSL_ACC_GYRO_W_ODR_FIFO(handle,LSM6DSL_ACC_GYRO_ODR_FIFO_disable);
    LSM6DSL_ACC_GYRO_W_DEC_FIFO_G(handle,LSM6DSL_ACC_GYRO_DEC_FIFO_G_DATA_NOT_IN_FIFO);
    LSM6DSL_ACC_GYRO_W_DEC_FIFO_XL(handle,LSM6DSL_ACC_GYRO_DEC_FIFO_XL_DATA_NOT_IN_FIFO);

}

#ifdef COD 

DrvStatusTypeDef  drv_lsm6dsl_cod_acc_init(LSM6DSL_ACC_GYRO_ODR_XL_t ODR)
{
	uint8_t value;
	void *handle = NULL;
	LSM6DSL_ACC_GYRO_W_SW_RESET( handle, LSM6DSL_ACC_GYRO_SW_RESET_RESET_DEVICE);
	Write_SIM();
	LSM6DSL_ACC_GYRO_W_PULL_UP_EN(handle,LSM6DSL_ACC_GYRO_PULL_UP_EN_ENABLED);
	if( !LSM6DSL_ACC_GYRO_ReadReg(handle, LSM6DSL_ACC_GYRO_CTRL1_XL, &value, 1))
    return COMPONENT_ERROR;
	
	value &= ~LSM6DSL_ACC_GYRO_ODR_XL_MASK;
	

	value |= ( uint8_t)ODR;  //????ж???? =n*512/26 = n * 19.7s   

	value &= ~LSM6DSL_ACC_GYRO_FS_XL_MASK;

	value |= (uint8_t)LSM6DSL_ACC_GYRO_FS_XL_16g;

	if( !LSM6DSL_ACC_GYRO_WriteReg(handle, LSM6DSL_ACC_GYRO_CTRL1_XL, &value, 1) )
    return COMPONENT_ERROR;
    

	LSM6DSL_ACC_GYRO_W_SLEEP_DUR(handle,1); //SLEEP 19.7S  WAKEUP 0
	
	LSM6DSL_ACC_GYRO_W_WAKE_DUR(handle,1);
	
	LSM6DSL_ACC_GYRO_W_WK_THS(handle,3* LSM6DSL_ACC_GYRO_ODR_XL_26Hz/ODR); // n*XL_FS/64


	value = 0x80;
	
	if( !LSM6DSL_ACC_GYRO_WriteReg(handle, LSM6DSL_ACC_GYRO_TAP_CFG1, &value, 1) )
    return COMPONENT_ERROR;

	drv_lsm6dsl_acc_lowpower();

	 return COMPONENT_OK;
	
    
//	value = 0x80;
//	if( !LSM6DSL_ACC_GYRO_WriteReg(handle, LSM6DSL_ACC_GYRO_MD1_CFG, &value, 1) )
//    return COMPONENT_ERROR;	
	
}


void  drv_lsm6dsl_cod_gyro_init(LSM6DSL_ACC_GYRO_ODR_G_t ODR)
{	
    void *handle = NULL;
    LSM6DSL_ACC_GYRO_W_ODR_G(handle,ODR);
    LSM6DSL_ACC_GYRO_W_FS_G(handle,LSM6DSL_ACC_GYRO_FS_G_1000dps);
    drv_lsm6dsl_gyro_lowpower();

}

void drv_lsm6dsl_cod_fifo_init(uint8_t acc_flag,uint8_t gyro_flag,LSM6DSL_ACC_GYRO_ODR_FIFO_t ODR)
{	
    void *handle = NULL;
	if (acc_flag == 0)
	{
		LSM6DSL_ACC_GYRO_W_DEC_FIFO_XL(handle,LSM6DSL_ACC_GYRO_DEC_FIFO_XL_DATA_NOT_IN_FIFO);
	}
	else{
    	LSM6DSL_ACC_GYRO_W_DEC_FIFO_XL(handle,LSM6DSL_ACC_GYRO_DEC_FIFO_XL_NO_DECIMATION);
	}
	if (acc_flag == 0)
	{
		LSM6DSL_ACC_GYRO_W_DEC_FIFO_G(handle,LSM6DSL_ACC_GYRO_DEC_FIFO_G_DATA_NOT_IN_FIFO);
	}
	else{
    	LSM6DSL_ACC_GYRO_W_DEC_FIFO_G(handle,LSM6DSL_ACC_GYRO_DEC_FIFO_G_NO_DECIMATION);
	}
	 
    LSM6DSL_ACC_GYRO_W_ODR_FIFO(handle,ODR);
    LSM6DSL_ACC_GYRO_W_FIFO_Watermark(handle,400);
    LSM6DSL_ACC_GYRO_W_STOP_ON_FTH(handle,LSM6DSL_ACC_GYRO_STOP_ON_FTH_ENABLED);
    LSM6DSL_ACC_GYRO_W_FIFO_MODE(handle,LSM6DSL_ACC_GYRO_FIFO_MODE_FIFO);

}

void  drv_lsm6dsl_cod_open(uint8_t acc_flag,uint8_t gyro_flag, uint8_t freq)
{	
	LSM6DSL_ACC_GYRO_ODR_XL_t acc_odr;
	LSM6DSL_ACC_GYRO_ODR_G_t gyro_odr;
 	LSM6DSL_ACC_GYRO_ODR_FIFO_t fifo_odr;
	if (freq ==100)
	{
		acc_odr = LSM6DSL_ACC_GYRO_ODR_XL_104Hz;
		gyro_odr = LSM6DSL_ACC_GYRO_ODR_G_104Hz;
		fifo_odr = LSM6DSL_ACC_GYRO_ODR_FIFO_100Hz;
	}
	else
	{
		acc_odr = LSM6DSL_ACC_GYRO_ODR_XL_52Hz;
		gyro_odr = LSM6DSL_ACC_GYRO_ODR_G_52Hz;
		fifo_odr = LSM6DSL_ACC_GYRO_ODR_FIFO_50Hz;
	}
	if (acc_flag == 1)
	{
		drv_lsm6dsl_cod_acc_init(acc_odr);	
	}
	else
	{
		drv_lsm6dsl_acc_powerdown();
	}
	if (gyro_flag == 1)
	{
		drv_lsm6dsl_cod_gyro_init(gyro_odr);	
	}
	else
	{
		drv_lsm6dsl_gyro_powerdown();
	}

	drv_lsm6dsl_cod_fifo_init(acc_flag,gyro_flag,fifo_odr);

	
}
#endif

#ifdef WATCH_COD_BSP

void  drv_lsm6dsl_cod_close(void)
{	
  //  void *handle = NULL;
	drv_lsm6dsl_fifo_uninit();
	drv_lsm6dsl_gyro_powerdown();
	drv_lsm6dsl_acc_init();
	drv_lsm6dsl_acc_fifo_init();
}

int dev_hrt_acc_init(void)
{
	uint8_t data;
	#ifdef COD 
	if (cod_user_sensor_req.sensor_open && (cod_user_sensor_req.acc_flag|| cod_user_sensor_req.gyro_flag))
	{
		return 1;
	}
	#endif
	LSM6DSL_ACC_GYRO_W_SW_RESET(NULL, LSM6DSL_ACC_GYRO_SW_RESET_RESET_DEVICE);
	LSM6DSL_ACC_GYRO_R_WHO_AM_I(NULL, &data);
	LSM6DSL_ACC_GYRO_W_IF_Addr_Incr(NULL, LSM6DSL_ACC_GYRO_IF_INC_ENABLED);
	LSM6DSL_ACC_GYRO_W_BDU(NULL, LSM6DSL_ACC_GYRO_BDU_BLOCK_UPDATE);
	LSM6DSL_ACC_GYRO_W_FIFO_MODE(NULL, LSM6DSL_ACC_GYRO_FIFO_MODE_BYPASS);
	LSM6DSL_ACC_GYRO_W_ODR_XL(NULL, LSM6DSL_ACC_GYRO_ODR_XL_POWER_DOWN);
	LSM6DSL_ACC_GYRO_W_FS_XL(NULL, LSM6DSL_ACC_GYRO_FS_XL_8g);
	LSM6DSL_ACC_GYRO_W_ODR_G(NULL, LSM6DSL_ACC_GYRO_ODR_G_POWER_DOWN);		//close gyro
	return 0;
}

int dev_hrt_acc_pwron(void)
{
	#ifdef COD 
	if (cod_user_sensor_req.sensor_open && (cod_user_sensor_req.acc_flag|| cod_user_sensor_req.gyro_flag))
	{
		return 1;
	}
	#endif
	LSM6DSL_ACC_GYRO_W_LowPower_XL(NULL, LSM6DSL_ACC_GYRO_LP_XL_ENABLED);
	LSM6DSL_ACC_GYRO_W_ODR_XL(NULL, LSM6DSL_ACC_GYRO_ODR_XL_26Hz);
	LSM6DSL_ACC_GYRO_W_FIFO_MODE(NULL, LSM6DSL_ACC_GYRO_FIFO_MODE_FIFO);
	LSM6DSL_ACC_GYRO_W_ODR_FIFO(NULL, LSM6DSL_ACC_GYRO_ODR_FIFO_25Hz);
	LSM6DSL_ACC_GYRO_W_DEC_FIFO_XL(NULL, LSM6DSL_ACC_GYRO_DEC_FIFO_XL_NO_DECIMATION);
	return 0;
}

#include "com_ringbuffer.h"
extern struct ring_buffer_header acc_rb;

int dev_hrt_acc_fifo_data(int16_t *accbuf)
{
	uint8_t datatmp[2];
	uint16_t len = 0;
	#ifdef COD 
	if ((cod_user_sensor_req.sensor_open) && (cod_user_sensor_req.acc_flag))
	{
		len = ring_buffer_data_left(&acc_rb)/2;
		if (len > 128)
		{
			len = 128;
		}
		for(int i = 0; i < len; i++)
	  {			
			ring_buffer_get(&acc_rb,&datatmp[0]);
			ring_buffer_get(&acc_rb,&datatmp[1]);
			accbuf[i] = (int16_t)((datatmp[1] << 8) | datatmp[0]);
	  }
	}
	else
	#endif
	{
		LSM6DSL_ACC_GYRO_R_FIFONumOfEntries(NULL, &len);
		//TASK_STEP_PRINTF("len:%d\r\n",len);
		if (len > 128)
		{
			len = 128;
		}
		for(int i = 0; i < len; i++) {
			LSM6DSL_ACC_GYRO_Get_GetFIFOData(NULL, datatmp);
			accbuf[i] = (int16_t)((datatmp[1] << 8) | datatmp[0]);
			//TASK_STEP_PRINTF("%d,",accbuf[i] );
			if(!ring_buffer_is_full(&acc_rb)) 
			{
				ring_buffer_put(&acc_rb, datatmp[0]);
				ring_buffer_put(&acc_rb, datatmp[1]);
    			}
		}
		//TASK_STEP_PRINTF("\r\n");
	}	
	return len;
}


#endif

void drv_lsm6dsl_fifo_data(int16_t *dataRaw)
{
    //t16_t dataRaw[6];
    
    uint8_t aData[2];
    uint16_t len,i;
    uint16_t pattern = 0;
    void *handle = NULL;
    LSM6DSL_ACC_GYRO_R_FIFONumOfEntries(handle,&len);
    
 //GYRO数据在前，ACC数据在后
     for(i = 0;i<len; i++)
     {
           LSM6DSL_ACC_GYRO_R_FIFOPattern(handle,&pattern);
           LSM6DSL_ACC_GYRO_Get_GetFIFOData( handle, aData );
           switch(pattern)
            {
                case 0:
                   dataRaw[0] = ( aData[1] << 8 ) | aData[0];
                   break;
                       
                case 1:
                   dataRaw[1] = ( aData[1] << 8 ) | aData[0];
                   break;
                       
                case 2:
                   dataRaw[2] = ( aData[1] << 8 ) | aData[0];
                   break;
                case 3:
                   dataRaw[3] = ( aData[1] << 8 ) | aData[0];
                   break;
                       
                case 4:
                   dataRaw[4] = ( aData[1] << 8 ) | aData[0];
                    break;
                       
                case 5:
                   dataRaw[5] = ( aData[1] << 8 ) | aData[0];
                   break;
                default:
                   break;
             }
                   
     }    
 
}


//??????????fifo
void drv_lsm6dsl_acc_fifo_init( void)
{	
    void *handle = NULL;
    LSM6DSL_ACC_GYRO_W_DEC_FIFO_XL(handle,LSM6DSL_ACC_GYRO_DEC_FIFO_XL_NO_DECIMATION);
    LSM6DSL_ACC_GYRO_W_DEC_FIFO_G(handle,LSM6DSL_ACC_GYRO_DEC_FIFO_G_DATA_NOT_IN_FIFO);
    LSM6DSL_ACC_GYRO_W_ODR_FIFO(handle,LSM6DSL_ACC_GYRO_ODR_FIFO_25Hz);
    LSM6DSL_ACC_GYRO_W_FIFO_Watermark(handle,200);
    LSM6DSL_ACC_GYRO_W_STOP_ON_FTH(handle,LSM6DSL_ACC_GYRO_STOP_ON_FTH_ENABLED);
    LSM6DSL_ACC_GYRO_W_FIFO_MODE(handle,LSM6DSL_ACC_GYRO_FIFO_MODE_FIFO);

}

void  drv_lsm6dsl_acc_fifo_uninit(void)
{	
    void *handle = NULL;
    LSM6DSL_ACC_GYRO_W_FIFO_MODE(handle,LSM6DSL_ACC_GYRO_FIFO_MODE_BYPASS);
    LSM6DSL_ACC_GYRO_W_STOP_ON_FTH(handle,LSM6DSL_ACC_GYRO_STOP_ON_FTH_DISABLED);
    LSM6DSL_ACC_GYRO_W_FIFO_Watermark(handle,0);
    LSM6DSL_ACC_GYRO_W_ODR_FIFO(handle,LSM6DSL_ACC_GYRO_ODR_FIFO_disable);
    LSM6DSL_ACC_GYRO_W_DEC_FIFO_XL(handle,LSM6DSL_ACC_GYRO_DEC_FIFO_XL_DATA_NOT_IN_FIFO);

}


#if 0
DrvStatusTypeDef  LSM6DSL_X_LIS3MDL_Init( void *handle)
{
    u8_t SlvAddr,Reg,value;
    LSM6DSL_ACC_GYRO_SENS_HUB_END_t op_cmpl = LSM6DSL_ACC_GYRO_SENS_HUB_END_STILL_ONGOING;
    
    LSM6DSL_ACC_GYRO_W_EmbeddedAccess(handle, LSM6DSL_ACC_GYRO_EMBEDDED_ACCESS_ENABLED);
    SlvAddr = 0x38;
    LSM6DSL_ACC_GYRO_WriteReg(handle, LSM6DSL_ACC_GYRO_SLV0_ADD, &SlvAddr, 1);
    Reg = 0x22;
    LSM6DSL_ACC_GYRO_WriteReg(handle, LSM6DSL_ACC_GYRO_SLV0_SUBADD, &Reg, 1);
    value = 0x00;
    LSM6DSL_ACC_GYRO_WriteReg(handle, LSM6DSL_ACC_GYRO_DATAWRITE_SRC_MODE_SUB_SLV0, &value,1);
    value = 0x10;
    LSM6DSL_ACC_GYRO_WriteReg(handle, LSM6DSL_ACC_GYRO_SLAVE0_CONFIG, &value, 1);
    value = 0x20;
    LSM6DSL_ACC_GYRO_WriteReg(handle, LSM6DSL_ACC_GYRO_SLAVE1_CONFIG, &value, 1);

    LSM6DSL_ACC_GYRO_W_EmbeddedAccess(handle, LSM6DSL_ACC_GYRO_EMBEDDED_ACCESS_DISABLED);

    LSM6DSL_ACC_GYRO_W_FUNC_EN(handle, LSM6DSL_ACC_GYRO_FUNC_EN_ENABLED);
    value = 0x09;
    LSM6DSL_ACC_GYRO_WriteReg(handle, LSM6DSL_ACC_GYRO_MASTER_CONFIG, &value, 1);
    
    value = 0x80;
    LSM6DSL_ACC_GYRO_WriteReg(handle, LSM6DSL_ACC_GYRO_CTRL1_XL, &value, 1);

   do
    {
    LSM6DSL_ACC_GYRO_R_SENS_HUB_END(handle, &op_cmpl);
   //RV_NDOF_LOG_PRINTF("LSM6DSL_X_LIS3MDL_Init %x\n",ID_VALUE);
    
    }
  while(op_cmpl != LSM6DSL_ACC_GYRO_SENS_HUB_END_OP_COMPLETED);

   LSM6DSL_ACC_GYRO_W_FUNC_EN(handle, LSM6DSL_ACC_GYRO_FUNC_EN_DISABLED);

   LSM6DSL_ACC_GYRO_W_I2C_MASTER_Enable(handle, LSM6DSL_ACC_GYRO_MASTER_ON_DISABLED);

    value = 0x00;
    LSM6DSL_ACC_GYRO_WriteReg(handle, LSM6DSL_ACC_GYRO_CTRL1_XL, &value, 1);

  SlvAddr |= 0x1; /* Raise the read op bit */
  LSM6DSL_ACC_GYRO_WriteReg(handle, LSM6DSL_ACC_GYRO_SLV0_ADD, &SlvAddr, 1);

  Reg = 0x28;
  LSM6DSL_ACC_GYRO_WriteReg(handle, LSM6DSL_ACC_GYRO_SLV0_SUBADD, &Reg, 1);

  value = 0x06;
  LSM6DSL_ACC_GYRO_WriteReg(handle, LSM6DSL_ACC_GYRO_SLAVE0_CONFIG, &value, 1);

  LSM6DSL_ACC_GYRO_W_EmbeddedAccess(handle, LSM6DSL_ACC_GYRO_EMBEDDED_ACCESS_DISABLED);

     LSM6DSL_ACC_GYRO_W_FUNC_EN(handle, LSM6DSL_ACC_GYRO_FUNC_EN_ENABLED);
    value = 0x09;
    LSM6DSL_ACC_GYRO_WriteReg(handle, LSM6DSL_ACC_GYRO_MASTER_CONFIG, &value, 1);
    
    value = 0x80;
    LSM6DSL_ACC_GYRO_WriteReg(handle, LSM6DSL_ACC_GYRO_CTRL1_XL, &value, 1);
    return COMPONENT_OK;

}




void LSM6DSL_X_LIS3MDL_DATA(void *handle,int16_t *psdata)
{
    u8_t dummy[6];

    LSM6DSL_ACC_GYRO_ReadReg(handle, LSM6DSL_ACC_GYRO_SENSORHUB1_REG, dummy ,6);

    	psdata[0] = (dummy[1] << 8)|dummy[0];
		psdata[1] = (dummy[3] << 8)|dummy[2];
		psdata[2] = (dummy[5] << 8)|dummy[4];

}
#endif


#if defined WATCH_IMU_CALIBRATION

DrvStatusTypeDef  drv_lsm6dsl_acc_test_init(void)
{
	uint8_t value;
	void *handle = NULL;
	LSM6DSL_ACC_GYRO_W_SW_RESET( handle, LSM6DSL_ACC_GYRO_SW_RESET_RESET_DEVICE);
	Write_SIM();
	if( !LSM6DSL_ACC_GYRO_ReadReg(handle, LSM6DSL_ACC_GYRO_CTRL1_XL, &value, 1))
    return COMPONENT_ERROR;
	
	value &= ~LSM6DSL_ACC_GYRO_ODR_XL_MASK;	
	value |= ( uint8_t)LSM6DSL_ACC_GYRO_ODR_XL_208Hz;
	
	value &= ~LSM6DSL_ACC_GYRO_FS_XL_MASK;
	value |= (uint8_t)LSM6DSL_ACC_GYRO_FS_XL_4g;
	
	if( !LSM6DSL_ACC_GYRO_WriteReg(handle, LSM6DSL_ACC_GYRO_CTRL1_XL, &value, 1) )
    return COMPONENT_ERROR;
	
	LSM6DSL_ACC_GYRO_W_SLEEP_DUR(handle,1); //SLEEP 19.7S  WAKEUP 0
	
	LSM6DSL_ACC_GYRO_W_WAKE_DUR(handle,1);
	
	LSM6DSL_ACC_GYRO_W_WK_THS(handle,3); // n*XL_FS/64
	
	//value = 0xE0;
	
	//if( !LSM6DSL_ACC_GYRO_WriteReg(handle, LSM6DSL_ACC_GYRO_TAP_CFG1, &value, 1) )
    return COMPONENT_ERROR;


	 return COMPONENT_OK;	
    
//	value = 0x80;
//	if( !LSM6DSL_ACC_GYRO_WriteReg(handle, LSM6DSL_ACC_GYRO_MD1_CFG, &value, 1) )
//    return COMPONENT_ERROR;	
	
}

void  drv_lsm6dsl_gyro_test_init(LSM6DSL_ACC_GYRO_ODR_G_t nHz,LSM6DSL_ACC_GYRO_FS_G_t nDps)
{	
    void *handle = NULL;
    LSM6DSL_ACC_GYRO_W_ODR_G(handle,nHz);
    LSM6DSL_ACC_GYRO_W_FS_G(handle,nDps);
    //drv_lsm6dsl_gyro_lowpower();
}

void drv_lsm6dsl_fifo_test_init( void)
{	
    void *handle = NULL;
    LSM6DSL_ACC_GYRO_W_DEC_FIFO_XL(handle,LSM6DSL_ACC_GYRO_DEC_FIFO_XL_NO_DECIMATION);
	LSM6DSL_ACC_GYRO_W_DEC_FIFO_G(handle,LSM6DSL_ACC_GYRO_DEC_FIFO_G_NO_DECIMATION);
	 
    LSM6DSL_ACC_GYRO_W_ODR_FIFO(handle,LSM6DSL_ACC_GYRO_ODR_FIFO_200Hz);
    LSM6DSL_ACC_GYRO_W_FIFO_Watermark(handle,0x7FF);
    LSM6DSL_ACC_GYRO_W_STOP_ON_FTH(handle,LSM6DSL_ACC_GYRO_STOP_ON_FTH_ENABLED);
    LSM6DSL_ACC_GYRO_W_FIFO_MODE(handle,LSM6DSL_ACC_GYRO_FIFO_MODE_FIFO);

}

#endif





/** @}*/
