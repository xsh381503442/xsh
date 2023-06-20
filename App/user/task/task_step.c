#include "task_step.h"
#include "gui_gps_test.h"
#include "task_config.h"
#include "task_sport.h"

#include "drv_motor.h"
#include "drv_lsm6dsl.h"
//#include "ndof/drv_ndof.h"
#include "Pedometer/Pedometer.h"
#include "Pedometer/PedoSupport.h"
#include "com_data.h"
#include "com_sport.h"
//#include "algo_sport_cycling.h"
#include "algo_swimming.h"
#include "algo_step_count.h"
#include "algo_sport_cycling.h"
#include "gui_system.h"
#include "algo_HF_swimming_main.h"
#include "task_ble.h"
#include "drv_heartrate.h"
#include "com_ringbuffer.h"


TaskHandle_t	TaskStepHandle = NULL; 
uint8_t		move_int;
uint8_t   step_pattern_flag = 0;//日常 0，or运动 1

uint32_t step_count = 0;
uint16_t get_gensor_interval = 1000;
extern int time_i;

#if defined WATCH_SIM_LIGHTS_ON
extern uint8_t backlight_always_on;  //背光常亮标志
#endif

uint8_t rtc_hw_flag = 0;

#define STEP_COUNT_LIMIT 20 //有效步数阈值
#define STEP_COUNT_TIME  4	//静止判断阈值
extern struct ring_buffer_header acc_rb;
extern struct ring_buffer_header cod_sensor_acc_rb;
extern struct ring_buffer_header cod_sensor_gyro_rb;
extern uint8_t cod_sensor_first_flag;
extern bool hr_enabled;
 SemaphoreHandle_t StepSemaphore = NULL;

 acc_s	 acc_xyzc;

extern uint32_t acc_num;
extern uint32_t gyro_num;


 //获取日常步数
uint32_t get_filt_stepcount(void)
{
#ifdef WATCH_STEP_ALGO
	return (step_data_str.passometer_mun);

#else
	return step_count;
#endif
  
}

/*
加速度数据精度转化
FS = ±2 	0.061	mg/LSB
FS = ±4 	0.122	
FS = ±8 	0.244	
FS = ±16 	0.488	

*/
uint16_t cod_acc_sensitivity_conv(int16_t acc)
{	
	
	return ((uint16_t)(acc * (-0.488) + 32000));
}

/*
角速度数据精度转化
FS = ±125 	4.375	mdps/LSB
FS = ±250 	8.75	
FS = ±500 	17.50	
FS = ±1000 	35	
FS = ±2000 	70	
*/

uint16_t cod_gyro_sensitivity_conv(int16_t gyro)
{
	return ((uint16_t)(gyro * 35 *0.017453293 + 32000));
}
//量程减半换算如16G转8G,8G转4G
int16_t range_in_half(int16_t data)
{
 	int32_t temp;
	temp = data *2;
	if (temp > 32767)
	{
		temp = 32767;
	}
	else if (temp < -32768 )
	{
		temp = -32768;
	}
	return (int16_t)temp;
}
//设置步数
void set_filt_stepcount(uint32_t steps,uint32_t sport_steps)
{

#ifdef WATCH_STEP_ALGO
	init_steps(steps,sport_steps);

#else
	step_count = steps;
#endif
		
}

extern unsigned char V_StatusFlags_U8R;



void *LSM6DSL_X_0_handle = NULL;

LSM6DSL_ACC_GYRO_SIM_t   Reg_Value_t;

void CreateStepTask(void)
{
	if( TaskStepHandle == NULL )
	{
		TASK_STEP_PRINTF("[task_step]:CreateStepTask!\n");

		xTaskCreate(TaskStep, "Task Step",TaskStep_StackDepth, 0, TaskStep_Priority, &TaskStepHandle);
	}
	if (StepSemaphore == NULL)
	{
		StepSemaphore = xSemaphoreCreateBinary();  //创建二值信号量
	}
}
void CloseStepTask(void)
{
	move_int = 3;
	vTaskDelete(TaskStepHandle);
	TaskStepHandle = NULL;
	TASK_STEP_PRINTF("[task_step]:CloseStepTask!\n");
}

void drv_acc_gyro_clear_fifo(void)
{
	uint16_t len,i;
	void *handle = NULL;
	uint8_t data;
	LSM6DSL_ACC_GYRO_R_FIFONumOfEntries(handle,&len);
	for(i=0;i< len ;i++)
	{
	    LSM6DSL_ACC_GYRO_Get_GetFIFOData( handle, &data);
	}
	TASK_STEP_PRINTF("[task_step]: drv_acc_gyro_clear_fifo = %d\r\n",len);
	
}

/*函数说明:加速度fifo数据游泳处理
入参说明:is_step_algo 是否使用计步算法
         is_swim  是否处于游泳模式
*/

//float  acc_x[128],acc_y[128],acc_z[128],acc_cpmplex[256];//计步功能新增数组
uint16_t acce_data[60];
uint16_t gyro_data[60];
uint32_t group_num = 0;

void algo_swim_acc_fifo__data(uint8_t is_step_algo,uint8_t is_swim)
{
	/*HF*/
	int16_t imu_raw_data_arr[6];
	
  int16_t dataRawacc[3], dataRawgyro[3],data_temp_acc = 0,data_temp_gyro = 0;
  uint8_t group,data_len,heart;
  uint8_t group_div;
	//float lfv_xyzc[3];
	uint8_t aData[2];
	uint16_t len,i,j,k,tmp_i,tmp_len;
	uint16_t pattern = 0;
	uint8_t datanums  = 3;	
	
	//float  acc_x[64],acc_y[64],acc_z[64],acc_cpmplex[64];//计步功能新增数组
	void *handle = NULL;
    
	if(step_pattern_flag == STEP_PATTERN_DAILY)//日常模式
	    {
		
		 get_gensor_interval = 1000;	

		//SEGGER_RTT_printf(0,"*get_gensor_interval=%d*\r\n", get_gensor_interval);
		}
	else if(step_pattern_flag == STEP_PATTERN_SPORT)//运动模式
		{
		 
		  get_gensor_interval = 500;
		
		 }
	 else;
	LSM6DSL_ACC_GYRO_R_FIFONumOfEntries(handle,&len);
	#ifdef COD 
	if ((cod_user_sensor_req.sensor_open) && ((cod_user_sensor_req.acc_flag)|| (cod_user_sensor_req.gyro_flag)))
	{
		//分支里面处理数据xyz顺序对齐
	}
	else
	#endif
	{
	
	LSM6DSL_ACC_GYRO_R_FIFOPattern(handle,&pattern);
//	TASK_STEP_PRINTF("[task_step]: LSM6DSL_ACC_GYRO_R_FIFONumOfEntries len= %d,pattern =%d\r\n",len,pattern);
	//数据xyz顺序对齐
	if (pattern == 0)
	{
		//
	}
	else if (pattern == 1)
	{
		LSM6DSL_ACC_GYRO_Get_GetFIFOData( handle, aData);	
		LSM6DSL_ACC_GYRO_Get_GetFIFOData( handle, aData);
		if(len >2)
		{
			len = len -2;
		}
		else
		{
			len= 0;

		}
	}
	else if (pattern == 2)
	{
		LSM6DSL_ACC_GYRO_Get_GetFIFOData( handle, aData);
		if(len >1)
		{
			len = len -1;
		}
		else
		{
			len= 0;

		}
		
	}
	}
	#ifdef COD 
	//acc数据
	if(len==0)
	 {
	 	if ((cod_user_sensor_req.sensor_open) && ((cod_user_sensor_req.acc_flag)|| (cod_user_sensor_req.gyro_flag)))
		{
		 get_gensor_interval = 20;
	 	}
       	 return;
      }
	else;
	if ((cod_user_sensor_req.sensor_open) && ((cod_user_sensor_req.acc_flag)|| (cod_user_sensor_req.gyro_flag)))
	{
		if (cod_sensor_first_flag)
		{
			drv_acc_gyro_clear_fifo();
			cod_sensor_first_flag =0;
			get_gensor_interval = 1;
			return;
		}
		 get_gensor_interval = 20;
		 if ((cod_user_sensor_req.acc_flag)&& (cod_user_sensor_req.gyro_flag))
		 {
		 	//数据xyz顺序对齐
		 	LSM6DSL_ACC_GYRO_R_FIFOPattern(handle,&pattern);
			if (pattern != 0 )
			{
				tmp_len = 6 - pattern;
				for (tmp_i = 0;tmp_i< tmp_len;tmp_i++)
				{
					LSM6DSL_ACC_GYRO_Get_GetFIFOData( handle, aData );
				}
				
				if(len >tmp_len)
				{
					len = len -tmp_len;
				}
				else
				{
					len= 0;
				}
				
			}
			datanums  = 6;
		 }
		 else
		 {
		 	//数据xyz顺序对齐
		 	LSM6DSL_ACC_GYRO_R_FIFOPattern(handle,&pattern);
			if (pattern != 0)
			{
				tmp_len = 3 - pattern;
				
				for (tmp_i = 0;tmp_i< tmp_len;tmp_i++)
				{
					LSM6DSL_ACC_GYRO_Get_GetFIFOData( handle, aData );
				}
				
				if(len >tmp_len)
				{
					len = len -tmp_len;
				}
				else
				{
					len= 0;
				}
			  }
			 datanums  = 3;
		 }
		//TASK_STEP_PRINTF("[task_step]: 1 cod_user_sensor_req len= %d,datanums=%d\r\n",len,datanums);
		for(i = 0;i<len; i++)
		{
			
			LSM6DSL_ACC_GYRO_Get_GetFIFOData( handle, aData );
			
					
			switch(i%datanums)
			{
        	case 0:
         	 imu_raw_data_arr[0] = ( aData[1] << 8 ) | aData[0];
         	 break;            
        	case 1:
             imu_raw_data_arr[1] = ( aData[1] << 8 ) | aData[0];
            break;
                       
            case 2:
             imu_raw_data_arr[2] = ( aData[1] << 8 ) | aData[0];
              break;
								
            case 3:
              imu_raw_data_arr[3] = ( aData[1] << 8 ) | aData[0];
              break;
                       
            case 4:
             imu_raw_data_arr[4] = ( aData[1] << 8 ) | aData[0];
             break;
            case 5:
             imu_raw_data_arr[5] = ( aData[1] << 8 ) | aData[0];
             break;						
            default:
              break;
		}
	   
		if ((i+1)%datanums == 0)
		{
		
		  group = (i+1)/datanums -1;//数据组数，从0开始
		
		 
		  if (group > 19)
		  {
			group = 19;
		  }
			
     	 if(datanums == 6)
		 {	
		 	
			  gyro_data[3*group] = cod_gyro_sensitivity_conv(imu_raw_data_arr[0]);//gyro_x
			  gyro_data[3*group +1] = cod_gyro_sensitivity_conv(imu_raw_data_arr[1]);//gyro_y
			  gyro_data[3*group +2] = cod_gyro_sensitivity_conv(imu_raw_data_arr[2]);//gyro_z
			  acce_data[3*group] = cod_acc_sensitivity_conv(imu_raw_data_arr[3]);//a_x
			  acce_data[3*group +1] = cod_acc_sensitivity_conv(imu_raw_data_arr[4]);//a_y
			  acce_data[3*group +2] = cod_acc_sensitivity_conv(imu_raw_data_arr[5]);//a_z
			  if (hr_enabled == true)
			  {	
			  	//16G转8G
			  	dataRawacc[0] = range_in_half(imu_raw_data_arr[3]);//a_x
			  	dataRawacc[1] = range_in_half(imu_raw_data_arr[4]);//a_y
			    dataRawacc[2] = range_in_half(imu_raw_data_arr[5]);//a_z
			  }
			//  TASK_STEP_PRINTF("[task_step]:imu_raw_data_arr %d,%d,%d,%d,%d,%d\r\n",imu_raw_data_arr[0],imu_raw_data_arr[1],imu_raw_data_arr[2],imu_raw_data_arr[3],imu_raw_data_arr[4],imu_raw_data_arr[5]);
		  }
		  else
		  {
		  	  if (cod_user_sensor_req.acc_flag)
		  	  {
			  	acce_data[3*group] = cod_acc_sensitivity_conv(imu_raw_data_arr[0]);//a_x
			  	acce_data[3*group +1] = cod_acc_sensitivity_conv(imu_raw_data_arr[1]);//a_y
			  	acce_data[3*group +2] = cod_acc_sensitivity_conv(imu_raw_data_arr[2]);//a_z
				if (hr_enabled == true)
				{	
					//16G转8G
					dataRawacc[0] = range_in_half(imu_raw_data_arr[0]);//a_x
			  		dataRawacc[1] = range_in_half(imu_raw_data_arr[1]);//a_y
			 		dataRawacc[2] = range_in_half(imu_raw_data_arr[2]);//a_z
				}
		  	  }
			  if (cod_user_sensor_req.gyro_flag)
		  	  {
			  	gyro_data[3*group] = cod_gyro_sensitivity_conv(imu_raw_data_arr[0]);//g_x
			  	gyro_data[3*group +1] = cod_gyro_sensitivity_conv(imu_raw_data_arr[1]);//g_y
			  	gyro_data[3*group +2] = cod_gyro_sensitivity_conv(imu_raw_data_arr[2]);//g_z

				
		  	  }
		  }
		  
		  if (hr_enabled == true)
		  {
		      group_num++;
		      if (cod_user_sensor_req.six_freq ==100)
		      {
		          group_div = 4;
		      }
		      else 
		      {
			      group_div = 2;
		      }

		      if ((group_num % group_div) == 0)
		      {
		      	
				  
				  if(!ring_buffer_is_full(&acc_rb)) 
				   {
					  ring_buffer_put(&acc_rb, (uint8_t)dataRawacc[0]);
					  ring_buffer_put(&acc_rb, (uint8_t)(dataRawacc[0] >>8));
					  ring_buffer_put(&acc_rb, (uint8_t)dataRawacc[1]);
					  ring_buffer_put(&acc_rb, (uint8_t)(dataRawacc[1]>>8));
					  ring_buffer_put(&acc_rb, (uint8_t)dataRawacc[2]);
					  ring_buffer_put(&acc_rb, (uint8_t)(dataRawacc[2]>>8));
				  }
		      }
		  }
        }
	  }
		if (len != 0)
		{
		    data_len = (group +1) *6; 
		}
		else
		{
		    data_len = 0;
		}
		
		//TASK_STEP_PRINTF("[task_step]: 2 cod_user_sensor_req data_len=%d,group%d\r\n",data_len,(group +1));
		
		if (cod_user_sensor_req.acc_flag)
		{
		    for (k = 0; k < (data_len/6); k++) 
			{
				 
				 if(ring_buffer_space_left(&cod_sensor_gyro_rb) >= 6) 
				 {
				 	acc_num++;
					if (acc_num %26 !=0)
					{
        		     ring_buffer_put(&cod_sensor_acc_rb, (uint8_t)(acce_data[3*k ]>> 8));
					 ring_buffer_put(&cod_sensor_acc_rb, (uint8_t) acce_data[3*k ]);
					 ring_buffer_put(&cod_sensor_acc_rb, (uint8_t)(acce_data[3*k + 1]>> 8));
					 ring_buffer_put(&cod_sensor_acc_rb, (uint8_t) acce_data[3*k + 1]);
					 ring_buffer_put(&cod_sensor_acc_rb, (uint8_t)(acce_data[3*k + 2 ]>> 8));
					 ring_buffer_put(&cod_sensor_acc_rb, (uint8_t) acce_data[3*k + 2 ]);
					}
				 }
    		}
			//  TASK_STEP_PRINTF("[task_step]:acce_data %x,%x,%x\r\n",acce_data[0],acce_data[1],acce_data[2]);
		 }
		 if (cod_user_sensor_req.gyro_flag)
		 {
			  for (k = 0; k < (data_len/6); k++) 
			  {
			  	
			      if(ring_buffer_space_left(&cod_sensor_gyro_rb) >= 6) 
			      {
			      	 gyro_num ++;
					 if (gyro_num %26 !=0)
					 {
        		      ring_buffer_put(&cod_sensor_gyro_rb, (uint8_t)(gyro_data[3*k] >> 8));
					  ring_buffer_put(&cod_sensor_gyro_rb,  (uint8_t)gyro_data[3*k]);
					  ring_buffer_put(&cod_sensor_gyro_rb, (uint8_t)(gyro_data[3*k + 1] >> 8));
					  ring_buffer_put(&cod_sensor_gyro_rb,  (uint8_t)gyro_data[3*k + 1]);
					  ring_buffer_put(&cod_sensor_gyro_rb, (uint8_t)(gyro_data[3*k + 2] >> 8));
					  ring_buffer_put(&cod_sensor_gyro_rb,  (uint8_t)gyro_data[3*k + 2]);
					  }
				  }
    		  }	
			 // TASK_STEP_PRINTF("[task_step]:gyro_data %x,%x,%x\r\n",gyro_data[0],gyro_data[1],gyro_data[2]);
		 }
		
	}
	else
	#endif
		{
	if(step_pattern_flag == STEP_PATTERN_SPORT)
		{
	     acc_xyzc.acc_position = acc_xyzc.get_data_flag*acc_xyzc.get_data_signle_len;
		}
	if (is_swim == 1)
	{
		 datanums  = 6;
	}
	else
	{
		 datanums  = 3;
	}
	if (hr_enabled == true)
	{
		len = ring_buffer_data_left(&acc_rb)/2;
	}
	for(i = 0;i<len; i++)
	{
		if (hr_enabled == true)
		{
			//if (ring_buffer_data_left(&acc_rb))
	  		//{
				ring_buffer_get(&acc_rb,&aData[0]);
				ring_buffer_get(&acc_rb,&aData[1]);
	  		//}
		}
		else
		{
			LSM6DSL_ACC_GYRO_Get_GetFIFOData( handle, aData );
		}
		
		switch(i%datanums)
		{
        case 0:
          imu_raw_data_arr[0] = ( aData[1] << 8 ) | aData[0];
          break;
                       
        case 1:
          imu_raw_data_arr[1] = ( aData[1] << 8 ) | aData[0];
          break;
                       
        case 2:
          imu_raw_data_arr[2] = ( aData[1] << 8 ) | aData[0];
          break;
								
        case 3:
          imu_raw_data_arr[3] = ( aData[1] << 8 ) | aData[0];
          break;
                       
        case 4:
          imu_raw_data_arr[4] = ( aData[1] << 8 ) | aData[0];
          break;
                       
        case 5:
          imu_raw_data_arr[5] = ( aData[1] << 8 ) | aData[0];
          break;
								
        default:
          break;
		}
		//数据输入算法								
		if ((i+1)%datanums == 0)
		{	
      if(datanums == 6)
		  {
			  dataRawgyro[0] = imu_raw_data_arr[0];//gyro_x
			  dataRawgyro[1] = imu_raw_data_arr[1];//gyro_y
			  dataRawgyro[2] = imu_raw_data_arr[2];//gyro_z
			  dataRawacc[0] = imu_raw_data_arr[3];//a_x
			  dataRawacc[1] = imu_raw_data_arr[4];//a_y
			  dataRawacc[2] = imu_raw_data_arr[5];//a_z
		  }
		  else
		  {
			  dataRawacc[0] = imu_raw_data_arr[0];//a_x
			  dataRawacc[1] = imu_raw_data_arr[1];//a_y
			  dataRawacc[2] = imu_raw_data_arr[2];//a_z
		  }

		  
		 dataRawacc[0] = range_in_half(dataRawacc[0]);//计步使用量程是4G
		 dataRawacc[1] = range_in_half(dataRawacc[1]);//a_y
		 dataRawacc[2] = range_in_half(dataRawacc[2]);//a_z
		  
#if defined WATCH_RAISE_BRIGHT_SCREEN_ALGO
			if((is_swim != true && is_step_algo == true && get_cyclingsta()==0)|| (system_raiselight_mode_get() == 1) )
#else
			if(is_swim != true && is_step_algo == true && get_cyclingsta()==0)
#endif
			{//计步

				//咕咚定制手表轴向:X=Y;Y=-X(咕咚轴向转为轴向):俯视手表正面:左X下Y
			    if(((i+1)/datanums) >= 129)
				{
				 for(j = 0;j < 127;j++)
				 	{
                     acc_xyzc.acc_x[j] = acc_xyzc.acc_x[j+1];
					 acc_xyzc.acc_y[j] = acc_xyzc.acc_y[j+1];
					 acc_xyzc.acc_z[j] = acc_xyzc.acc_z[j+1];
					 acc_xyzc.acc_cpmplex[j] = acc_xyzc.acc_cpmplex[j+1];

				    }
				    if (rtc_hw_flag == 1)
				    {
						 acc_xyzc.acc_x[127] = (float)dataRawacc[0] * 0.000122f;
					    acc_xyzc.acc_y[127] = (float)dataRawacc[1] * 0.000122f;
					    acc_xyzc.acc_z[127] = (float)dataRawacc[2] * 0.000122f;
				    }
					else
					{
						 acc_xyzc.acc_x[127] = (float)dataRawacc[1] * 0.000122f;
					 acc_xyzc.acc_y[127] = -(float)dataRawacc[0] * 0.000122f;
					 acc_xyzc.acc_z[127] = (float)dataRawacc[2] * 0.000122f;
					}
					
					
			   acc_xyzc.acc_cpmplex[127] = sqrt(acc_xyzc.acc_x[127]*acc_xyzc.acc_x[127]+acc_xyzc.acc_y[127]*acc_xyzc.acc_y[127]
			   	      +acc_xyzc.acc_z[127]*acc_xyzc.acc_z[127]);
				}
			 else
			 	{
				/*计步功能新增数组赋值*/
				if (step_pattern_flag ==STEP_PATTERN_DAILY)
					{
					if (rtc_hw_flag == 1)
				    {
					acc_xyzc.acc_x[((i+1)/datanums)-1] = (float)dataRawacc[0] * 0.000122f;
				    acc_xyzc.acc_y[((i+1)/datanums)-1] = (float)dataRawacc[1] * 0.000122f;
				    acc_xyzc.acc_z[((i+1)/datanums)-1] = (float)dataRawacc[2] * 0.000122f;
					}
					else
					{
					acc_xyzc.acc_x[((i+1)/datanums)-1] = (float)dataRawacc[1] * 0.000122f;
				    acc_xyzc.acc_y[((i+1)/datanums)-1] = -(float)dataRawacc[0] * 0.000122f;
				    acc_xyzc.acc_z[((i+1)/datanums)-1] = (float)dataRawacc[2] * 0.000122f;
					}
					
				
				acc_xyzc.acc_cpmplex[((i+1)/datanums)-1] = sqrt(acc_xyzc.acc_x[((i+1)/datanums)-1] * acc_xyzc.acc_x[((i+1)/datanums)-1] 
				 + acc_xyzc.acc_y[((i+1)/datanums)-1] * acc_xyzc.acc_y[((i+1)/datanums)-1] + acc_xyzc.acc_z[((i+1)/datanums)-1] * acc_xyzc.acc_z[((i+1)/datanums)-1]); 
					}

				  
				else if(step_pattern_flag == STEP_PATTERN_SPORT)
					{
					if (rtc_hw_flag == 1)
					{
					acc_xyzc.acc_x[((i+1)/datanums)-1 + acc_xyzc.acc_position] = (float)dataRawacc[0] * 0.000122f;
				    acc_xyzc.acc_y[((i+1)/datanums)-1 + acc_xyzc.acc_position] = (float)dataRawacc[1] * 0.000122f;
				    acc_xyzc.acc_z[((i+1)/datanums)-1 + acc_xyzc.acc_position] = (float)dataRawacc[2] * 0.000122f;
					}
					else
					{
					acc_xyzc.acc_x[((i+1)/datanums)-1 + acc_xyzc.acc_position] = (float)dataRawacc[1] * 0.000122f;
				    acc_xyzc.acc_y[((i+1)/datanums)-1 + acc_xyzc.acc_position] = -(float)dataRawacc[0] * 0.000122f;
				    acc_xyzc.acc_z[((i+1)/datanums)-1 + acc_xyzc.acc_position] = (float)dataRawacc[2] * 0.000122f;
					}
					
					
				    acc_xyzc.acc_cpmplex[((i+1)/datanums)-1 + acc_xyzc.acc_position] = sqrt(acc_xyzc.acc_x[((i+1)/datanums)-1 + acc_xyzc.acc_position] * acc_xyzc.acc_x[((i+1)/datanums)-1 + acc_xyzc.acc_position] 
				    + acc_xyzc.acc_y[((i+1)/datanums)-1 + acc_xyzc.acc_position] * acc_xyzc.acc_y[((i+1)/datanums)-1 + acc_xyzc.acc_position] 
				    + acc_xyzc.acc_z[((i+1)/datanums)-1 + acc_xyzc.acc_position] * acc_xyzc.acc_z[((i+1)/datanums)-1 + acc_xyzc.acc_position]); 

				   }
				else;
					
			 	}
			}
#if defined WATCH_RAISE_BRIGHT_SCREEN_ALGO
			if (is_swim == 1)
#else
			else if (is_swim == 1)
#endif
			{//游泳
//#if defined WATCH_STEP_ALGO
//				lfv_xyzc[0] = (float)dataRawacc[0]/3276800;
//				lfv_xyzc[1] = (float)dataRawacc[1]/3276800;
//				lfv_xyzc[2] = (float)dataRawacc[2]/3276800;
//#else
//				lfv_xyzc[0] = (float)dataRawacc[0]/6553600;
//				lfv_xyzc[1] = (float)dataRawacc[1]/6553600;
//				lfv_xyzc[2] = (float)dataRawacc[2]/6553600;
//				
//#endif
//				SwimDataMeanFilter(lfv_xyzc);
				    if ( ActivityData.ActTime!=0 && !Get_PauseSta())
		        {	
              /*==========================HF=========================================*/
				      //the reading of gyroscope tends to reach its maximum when staying still
				    
                     
                    if (rtc_hw_flag == 1)
					{
                      imu_raw_data_arr[0] = imu_raw_data_arr[0];
		              imu_raw_data_arr[1] = imu_raw_data_arr[1];
		              imu_raw_data_arr[2] = imu_raw_data_arr[2];
                      imu_raw_data_arr[3] = imu_raw_data_arr[3];
		              imu_raw_data_arr[4] = imu_raw_data_arr[4];
		              imu_raw_data_arr[5] = imu_raw_data_arr[5];
                    }
					else
					{
					  data_temp_gyro = imu_raw_data_arr[0];
                      imu_raw_data_arr[0] = imu_raw_data_arr[1];
		              imu_raw_data_arr[1] = -data_temp_gyro;
		              imu_raw_data_arr[2] = imu_raw_data_arr[2];
				      data_temp_acc = imu_raw_data_arr[3];
                      imu_raw_data_arr[3] = imu_raw_data_arr[4];
		              imu_raw_data_arr[4] = -data_temp_acc;
		              imu_raw_data_arr[5] = imu_raw_data_arr[5];
					}
					
					  imu_raw_data_arr[3] = range_in_half(imu_raw_data_arr[3]);//8G量程转4G
					  imu_raw_data_arr[4] = range_in_half(imu_raw_data_arr[4]);//a_y
					  imu_raw_data_arr[5] = range_in_half(imu_raw_data_arr[5]);//a_z
					  
				      IncidentPreprocessing(imu_raw_data_arr);
				      IncidentPreprocessing(imu_raw_data_arr+1);
				      IncidentPreprocessing(imu_raw_data_arr+2);
				      //store raw data for simulation
							
							#if defined STORE_ORG_DATA_TEST_VERSION
				     // Store_swimaxisData(imu_raw_data_arr[0], imu_raw_data_arr[1], imu_raw_data_arr[2], imu_raw_data_arr[3], imu_raw_data_arr[4], imu_raw_data_arr[5]);
							Store_OrgAxisData(imu_raw_data_arr[0], imu_raw_data_arr[1], imu_raw_data_arr[2]);
							Store_OrgAxisData(imu_raw_data_arr[3], imu_raw_data_arr[4], imu_raw_data_arr[5]);
							#endif
							
				      //update the time index
							++time_i;//increasement from -1, so the first value passed to the function is 0
							//run the main algorithm
							
							//ALGO_HF_SWIM_PRINTF("1: %d\n",uxTaskGetStackHighWaterMark(TaskStepHandle));
							SwimmingAlgorithmsAll(imu_raw_data_arr, time_i);
							
							//print useful information for debug
							//ALGO_HF_SWIM_PRINTF("[Time]: %d\n",time_i);
							/*======================================================================*/
			     }
			 			}
			else if (get_cyclingsta()==1)
			{
				//骑行
			 if (rtc_hw_flag == 1)
			 {
                  dataRawacc[0] = dataRawacc[0];
		          dataRawacc[1] = dataRawacc[1];
		          dataRawacc[2] = dataRawacc[2];
			 }
			 else
			 {
			      data_temp_acc = dataRawacc[0];
                  dataRawacc[0] = dataRawacc[1];
		          dataRawacc[1] = -data_temp_acc;
		          dataRawacc[2] = dataRawacc[2];
			 }
			
				 
				algo_sport_cycling_start(dataRawacc, i, len);
				
			}
		}
	}
	        if(step_pattern_flag == STEP_PATTERN_SPORT)
	        	{
	             acc_xyzc.get_data_sum = len/datanums + acc_xyzc.acc_position;
				
				 acc_xyzc.get_data_signle_len = len/datanums;
	        	}

	

#if defined WATCH_RAISE_BRIGHT_SCREEN_ALGO
#if defined WATCH_SIM_LIGHTS_ON //背光常亮时不需要调用抬手亮屏
	if((system_raiselight_mode_get() == 1)&& (backlight_always_on == 0)) 
#else
	if(system_raiselight_mode_get() == 1)
#endif
	{
		if(step_pattern_flag == STEP_PATTERN_DAILY)//日常模式
	  	{
		   brighten_screen_entrance(len/datanums,&acc_xyzc,0);
	  	}
       else if(step_pattern_flag == STEP_PATTERN_SPORT)
       	{
			 if(acc_xyzc.get_data_flag == 0)
		   	{
				brighten_screen_entrance(acc_xyzc.get_data_signle_len,&acc_xyzc,0);
		   	}
		   else
		   	{
	         	brighten_screen_entrance(acc_xyzc.get_data_signle_len,&acc_xyzc, acc_xyzc.acc_position);

		    }
       	}
	}
#else
	
#endif
 
	/*调用计步函数*/
	if(is_swim != true && is_step_algo == true && get_cyclingsta()==0)
	{
	    	
		//非运动、稍后继续状态去除震动
		//if (((Get_TaskSportSta() == false || IS_SPORT_CONTINUE_LATER ==1) &&(motorflag == 0) &&(move_int ==0)) ||((Get_TaskSportSta() == true && IS_SPORT_CONTINUE_LATER ==0)))
		if ((step_pattern_flag == STEP_PATTERN_DAILY) &&(motorflag == 0) &&(move_int ==0))//日常计步
		{
			if(len > 128*datanums)
			{
				step_count_entrance(128*3,&acc_xyzc);
			}
		 	else
			{
			
				step_count_entrance(len/(datanums/3),&acc_xyzc);
			
	
		 	}
		}

	//	else if((step_pattern_flag == STEP_PATTERN_SPORT)&&(acc_xyzc.get_data_flag == 1))//运动模式计步
	   else if(step_pattern_flag == STEP_PATTERN_SPORT)//运动模式计步
			{
             	
               
			
				if(acc_xyzc.get_data_flag == 0)
				 {
				  acc_xyzc.get_data_flag = 1;
				 }
			    else
				{
					  if(len > 128*datanums)
		 			{
		 				step_count_entrance(128*3,&acc_xyzc);
		 			}
			 		else
					{

						
						step_count_entrance(acc_xyzc.get_data_sum*3,&acc_xyzc);
			
				 	}
					 acc_xyzc.get_data_flag = 0;
				}
            
		    }
		else;
	}
  }
}

void  TaskStep( void* pvParameter)
{
	(void)pvParameter;	
  //  uint32_t i;
	DISPLAY_MSG  msg = {0};
    uint32_t tmpcount,tmptime,last_tmpcount,last_checkstep,tmp_checkstep;
    uint8_t status_flag = 0;
	get_gensor_interval = 1000;
	//LSM6DSL_ACC_GYRO_R_WHO_AM_I(LSM6DSL_X_0_handle,&ID_VALUE);
    Write_SIM();
	drv_lsm6dsl_acc_init();	

	LSM6DSL_ACC_GYRO_R_SPI_Mode(LSM6DSL_X_0_handle,&Reg_Value_t);
	
	#if defined (WATCH_STEP_ALGO)
	#else
	drv_lsm6dsl_enable_pedometer();
	#endif
	
   	#if defined (WATCH_STEP_ALGO)
	drv_lsm6dsl_acc_fifo_init();
	monitor_count_init();//监视&计数模式参数初始化
	#endif

	rtc_hw_flag = drv_rtc_flag_reg_read();
	
#if defined (WATCH_STEP_ALGO)
	move_int = 0;
#else
	move_int = 1;//初始任务获取传感器步数
#endif

#if defined WATCH_RAISE_BRIGHT_SCREEN_ALGO

	brighten_screen_init();
#else

#endif

	while(1)
	{		
		switch(move_int)
		{
			case 3:
				//计步进入suspend模式
	            drv_lsm6dsl_reset();
				//删除计步任务
				if(move_int == 3)
				{
					TaskStepHandle = NULL;
					vTaskDelete(NULL);
				}
				else
				{
					//重新初始化
					
				}
				break;
			case 2:
		
				vTaskSuspend(NULL);
				break;
			default:				
				if( move_int == 0 )
				{
					 //使用传感器默认计步值
					if((get_step_status() == NO_USE_STEP_ALGO)&&(Get_SwimSta() == 0) && get_cyclingsta()==0)
					{	
						if ((Get_TaskSportSta() == false || IS_SPORT_CONTINUE_LATER ==1) &&(motorflag == 0))//非震动；非运动状态、稍后继续模式日常步数计算,
						{
							tmpcount = drv_lsm6dsl_get_stepcount();

							if (tmpcount ==last_tmpcount)
							{
								tmptime++;
								if (tmptime >STEP_COUNT_TIME) //无步数增加切换到检测模式
								{
									last_checkstep = tmpcount ;
									status_flag =0;
									tmptime = 0;
								}
							}
							else
							{
								tmptime = 0;
							}

							if (status_flag ==1)//计数模式
							{
								if ((int32_t)(tmpcount - last_tmpcount) > 0)
								{
									step_count +=(tmpcount - last_tmpcount);
								}
							}

							if (status_flag ==0)//检测模式
							{
								tmp_checkstep = tmpcount ;
								if((int32_t)(tmp_checkstep -last_checkstep) >= STEP_COUNT_LIMIT)
								{
									step_count +=(tmp_checkstep -last_checkstep);
									status_flag =1;
									last_checkstep = tmp_checkstep;
								}  
							}
							last_tmpcount = tmpcount ;
						}
					}
					else
					{
						//计步游泳数据处理
						algo_swim_acc_fifo__data(get_step_status(),Get_SwimSta());
					}

					if(ScreenState == DISPLAY_SCREEN_STEP)
					{  
						DayMainData.CommonSteps = get_filt_stepcount();

						msg.cmd = MSG_UPDATE_STEP_VALUE;
						xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
					}
				}
				else if(move_int == 1)
				{
					if(ScreenState == DISPLAY_SCREEN_STEP)
					{   
						DayMainData.CommonSteps = get_filt_stepcount();
						msg.cmd = MSG_UPDATE_STEP_VALUE;
						xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
					}

#if defined WATCH_STEP_ALGO
					algo_swim_acc_fifo__data(get_step_status(),Get_SwimSta());
#endif
                    last_checkstep = drv_lsm6dsl_get_stepcount();
					status_flag = 0;
					move_int = 0;			
				}
				break;
			
		}
		xSemaphoreTake(StepSemaphore,get_gensor_interval/portTICK_PERIOD_MS);
		//vTaskDelay(get_gensor_interval/portTICK_PERIOD_MS);
	}
}




