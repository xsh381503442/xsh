

#include "algo_imu_calibration.h"
#include "string.h"
#include "stdio.h"
#include "algo_step_count.h"
#include "task_step.h"
#include "task_display.h"
#include "drv_lsm6dsl.h"
#include "lsm6dsl/LSM6DSL_ACC_GYRO_driver.h"
#include "drv_pc.h"
#include "drv_extFlash.h"
#include "com_data.h"
#if defined WATCH_IMU_CALIBRATION

#define DELAY_MS(X)                 vTaskDelay(X / portTICK_PERIOD_MS)
#define ALGO_IMU_LOG_ENABLED 0
#if DEBUG_ENABLED == 1 && ALGO_IMU_LOG_ENABLED == 1
	#define ALGO_IMU_LOG_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define ALGO_IMU_LOG_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else       
	#define ALGO_IMU_LOG_WRITESTRING(...)
	#define ALGO_IMU_LOG_PRINTF(...)		        
#endif

uint8_t adjustacc_count = 0,sample_count = 0,gyro_count = 0;//   ;当前采样点个数
uint8_t imu_cal_flag = 0;
uint8_t start_cal_flag = 0;
uint8_t quit_cal_flag = 0;
imu_acc_data acc_cal_data[ACC_CAL_SUM]; 

imu_6d_data acc_gyro_data[ACC_SAMPLE_SUM];
 
data_flag acc_cal_flag,gyro_cal_flag;

acc_matrix_parameter acc_parameter ={{1,0,0,0,1,0,0,0,1},{0,0,0,},{0}};
gyro_matrix_parameter gyro_parameter = {{0,0,0,},{0}};
extern bool m_initialized;


float accX[ACC_SAMPLE_SUM]  = {0};
float accY[ACC_SAMPLE_SUM]  = {0};
float accZ[ACC_SAMPLE_SUM]  = {0};
float gyroX[ACC_SAMPLE_SUM] = {0};
float gyroY[ACC_SAMPLE_SUM] = {0};
float gyroZ[ACC_SAMPLE_SUM] = {0};
float var_acc_gyro[6] = {0};
float acc_gyro_mean[6];
float static_acc[3],static_gyro[3];
const float position_static[10][3] = {{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1},{0.433f,0.25f,0.866f},{0.433f,-0.25f,0.866f},{-0.433f,0.25f,0.866f},{-0.433f,-0.25f,0.866f}};
double acc_inverse[MATRIX_ORDER][MATRIX_ORDER];//逆矩阵
float acc_array_A[9];
int8_t position_index = -1;	
float acc_x[60],acc_y[60],acc_z[60],gyro_x[60],gyro_y[60],gyro_z[60];
uint8_t cap[12] = {0};
float fav_data[3][3] = {{1,0,0},{0,1,0},{0,0,1}};
float zero_offset[3] = {0}; 

 static void pc_uart_command_hardware_test_acc_data(uint8_t *buf, uint8_t len)			
{
	if(len == 31)
	{
		uint8_t acc_data[39];
		acc_data[0] = '$';
		acc_data[1] = 'a';
		acc_data[2] = 'c';
		acc_data[3] = 'c';
		acc_data[4] = ':';
		memcpy(&acc_data[5],buf,len);
		acc_data[len+5] = '&';
		acc_data[len+6] = '\r';
		acc_data[len+7] = '\n';	
	    drv_pc_send_data_buf(acc_data, sizeof(acc_data));
	}
	else if(len == 30)
	{
		 uint8_t acc_data[38];
		 acc_data[0] = '$';
		 acc_data[1] = 'a';
		 acc_data[2] = 'c';
		 acc_data[3] = 'c';
		 acc_data[4] = ':';
		 memcpy(&acc_data[5],buf,len);
		 acc_data[len+5] = '&';
		 acc_data[len+6] = '\r';
		 acc_data[len+7] = '\n'; 
		 drv_pc_send_data_buf(acc_data, sizeof(acc_data));
	}
	//else if(len == 107)
	else if(len == 140)
	{
         //uint8_t acc_data[116];
         uint8_t acc_data[149];
		 acc_data[0] = '$';
		 acc_data[1] = 'p';
		 acc_data[2] = 'a';
		 acc_data[3] = 'r';
		 acc_data[4] = 'a';
		 acc_data[5] = ',';
		 memcpy(&acc_data[6],buf,len);
		 acc_data[len+6] = ',';
		 acc_data[len+7] = '\r';
		 acc_data[len+8] = '\n';
		// acc_data[11] = ',';
		// acc_data[12] = '\r';
		// acc_data[13] = '\n'; 
		 drv_pc_send_data_buf(acc_data, sizeof(acc_data));
  

	 }
	else if(len == 112)
		{
        uint8_t acc_data[121];
		 acc_data[0] = '$';
		 acc_data[1] = 't';
		 acc_data[2] = 'e';
		 acc_data[3] = 's';
		 acc_data[4] = 'a';
		 acc_data[5] = ',';
		 memcpy(&acc_data[6],buf,len);
		 acc_data[len+6] = ',';
		 acc_data[len+7] = '\r';
		 acc_data[len+8] = '\n'; 
		 drv_pc_send_data_buf(acc_data, sizeof(acc_data));

	  }
	else;
}
 void pc_uart_cal_back(uint8_t flag)
 {
    
    	 uint8_t acc_data[5];
		 if(flag==0)
    	{
		 acc_data[0] = '!';//校准完成
		}
		 else if(flag==1)
		 	{
             acc_data[0] = '~';//已校准acc
		   }
		 else if(flag==2)
		 	{
             acc_data[0] = '@';//强制退出校准
		   }
		  else if(flag==3)
		 	{
             acc_data[0] = '^';//已校准gyro
		   }
		 acc_data[1] = 'o';
		 acc_data[2] = 'k';
		 acc_data[3] = '\r'; 
		 acc_data[4] = '\n'; 
		 drv_pc_send_data_buf(acc_data, sizeof(acc_data));
      

 }
 /*void pc_uart_gyro_cal_back(uint8_t flag)
  {
	 
		  uint8_t acc_data[5];
		  if(flag==0)
		 {
		  acc_data[0] = '^';
		 }
		  else if(flag==1)
			 {
			  acc_data[0] = '~';
			}
		  else if(flag==2)
			 {
			  acc_data[0] = '@';
			}
		  acc_data[1] = 'o';
		  acc_data[2] = 'k';
		  acc_data[3] = '\r'; 
		  acc_data[4] = '\n'; 
		  drv_pc_send_data_buf(acc_data, sizeof(acc_data));
	   
 
  }*/
	/* void pass_back_parameter()
	 {
	   
		  char buffer_back[107];
		  sprintf(buffer_back,"%1.6f",acc_parameter.factor_and_vertical[0][0]);
		  buffer_back[8] = ',';
		  sprintf(&buffer_back[9],"%1.6f",acc_parameter.factor_and_vertical[0][1]);
		  buffer_back[17] = ',';
		  sprintf(&buffer_back[18],"%1.6f",acc_parameter.factor_and_vertical[0][2]);
		  buffer_back[26] = ',';
		  sprintf(&buffer_back[27],"%1.6f",acc_parameter.factor_and_vertical[1][0]);
		  buffer_back[35] = ',';
		  sprintf(&buffer_back[36],"%1.6f",acc_parameter.factor_and_vertical[1][1]);
		  buffer_back[44] = ',';
		  sprintf(&buffer_back[45],"%1.6f",acc_parameter.factor_and_vertical[1][2]);
		  buffer_back[53] = ',';
		  sprintf(&buffer_back[54],"%1.6f",acc_parameter.factor_and_vertical[2][0]);
		  buffer_back[62] = ',';
		  sprintf(&buffer_back[63],"%1.6f",acc_parameter.factor_and_vertical[2][1]);
		  buffer_back[71] = ',';
		  sprintf(&buffer_back[72],"%1.6f",acc_parameter.factor_and_vertical[2][2]);
		  buffer_back[80] = ',';
		  sprintf(&buffer_back[81],"%1.6f",acc_parameter.acc_zero_offset[0]);
		  buffer_back[89] = ',';
		  sprintf(&buffer_back[90],"%1.6f",acc_parameter.acc_zero_offset[1]);
		  buffer_back[98] = ',';
		  sprintf(&buffer_back[99],"%1.6f",acc_parameter.acc_zero_offset[2]);
			 //buffer[107] = ',';	 
		  pc_uart_command_hardware_test_acc_data((uint8_t *)buffer_back,107);
	 
	 }*/

 void pass_back_parameter()
{
  
	 int acc_value = 0,gyro_value = 0,i;
	  char buffer_back[140];
	 read_acc_calibration_paremeter();
	 read_gyro_calibration_paremeter();
	 
			 if((acc_parameter.factor_and_vertical[0][0]>0.9f)&&(acc_parameter.factor_and_vertical[0][0]<1.1f))
			 	{
			 	acc_value++;
			 	}
			 else;

			 if(acc_parameter.factor_and_vertical[0][1]==0)
			 	{
			 	acc_value++;
				}
			 else;
		     if(acc_parameter.factor_and_vertical[0][2]==0)
			 	{
			 	acc_value++;
				}
			 else;
			 if(fabs(acc_parameter.factor_and_vertical[1][0])<0.1f)
			 	{
			 	acc_value++;
			 	}
			 else;
			  if((acc_parameter.factor_and_vertical[1][1]>0.9f)&&(acc_parameter.factor_and_vertical[1][1]<1.1f))
			 	{
			 	acc_value++;
			 	}
			 else;
			  if(acc_parameter.factor_and_vertical[1][2]==0)
			 	{
			 	acc_value++;
			 	}
			 else;

			 if(fabs(acc_parameter.factor_and_vertical[2][0])<0.1f)
			 	{
			 	acc_value++;
			    }
			 else;
			  if(fabs(acc_parameter.factor_and_vertical[2][1])<0.1f)
			 	{
			 	acc_value++;
			    }
			 else;
			  if((acc_parameter.factor_and_vertical[2][2]>0.9f)&&(acc_parameter.factor_and_vertical[2][2]<1.1f))
			 	{
			 	acc_value++;
			 	}
			 else;
			for(i=0;i<3;i++)
			{
			 if(fabs(acc_parameter.acc_zero_offset[i])<0.2f)
			 	{
			        acc_value++;
			    }
			 else;
		    }
	
	 	if(acc_value==12)
	 	{
              if((acc_parameter.factor_and_vertical[0][0]==1.0f)&&(acc_parameter.factor_and_vertical[1][1]==1.0f)&&(acc_parameter.factor_and_vertical[2][2]==1.0f)
            &&(acc_parameter.factor_and_vertical[1][0]==0)&&(acc_parameter.factor_and_vertical[2][0]==0)&&(acc_parameter.factor_and_vertical[2][1]==0)
            &&(acc_parameter.acc_zero_offset[0]==0)&&(acc_parameter.acc_zero_offset[1]==0)&&(acc_parameter.acc_zero_offset[2]==0))
	 	     {

              acc_value = -1;//参数初始化，请重新校准!
	         }
            else;//校准完成，参数正确!

		 }
		else;//校准参数有误，请重新校准!
	if(((gyro_parameter.gryo_zero[0]!=0)||(gyro_parameter.gryo_zero[1]!=0)||(gyro_parameter.gryo_zero[2]!=0))&&(gyro_parameter.flag==137))
	  {
		gyro_value++;//校准完成，参数正确!
	  }
	else if((gyro_parameter.gryo_zero[0]==0)&&(gyro_parameter.gryo_zero[1]==0)&&(gyro_parameter.gryo_zero[2]==0)&&(gyro_parameter.flag==0))
		{
              gyro_value=-1;//参数初始化，请点击陀螺仪校准按钮开始校准!
		 }
	 else;//未校准，请校准
	
	 sprintf(buffer_back,"%2d",acc_value);
	 buffer_back[2]=',';
	 sprintf(&buffer_back[3],"%2d",gyro_value);
	 buffer_back[5]=',';
	 sprintf(&buffer_back[6],"%1.6f",acc_parameter.factor_and_vertical[0][0]);
	 buffer_back[14] = ',';
	 sprintf(&buffer_back[15],"%1.6f",acc_parameter.factor_and_vertical[0][1]);
	 buffer_back[23] = ',';
	 sprintf(&buffer_back[24],"%1.6f",acc_parameter.factor_and_vertical[0][2]);
	 buffer_back[32] = ',';
	 sprintf(&buffer_back[33],"%1.6f",acc_parameter.factor_and_vertical[1][0]);
	 buffer_back[41] = ',';
	 sprintf(&buffer_back[42],"%1.6f",acc_parameter.factor_and_vertical[1][1]);
	 buffer_back[50] = ',';
	 sprintf(&buffer_back[51],"%1.6f",acc_parameter.factor_and_vertical[1][2]);
	 buffer_back[59] = ',';
	 sprintf(&buffer_back[60],"%1.6f",acc_parameter.factor_and_vertical[2][0]);
	 buffer_back[68] = ',';
	 sprintf(&buffer_back[69],"%1.6f",acc_parameter.factor_and_vertical[2][1]);
	 buffer_back[77] = ',';
	 sprintf(&buffer_back[78],"%1.6f",acc_parameter.factor_and_vertical[2][2]);
	 buffer_back[86] = ',';
	 sprintf(&buffer_back[87],"%1.6f",acc_parameter.acc_zero_offset[0]);
	 buffer_back[95] = ',';
	 sprintf(&buffer_back[96],"%1.6f",acc_parameter.acc_zero_offset[1]);
	 buffer_back[104] = ',';
	 sprintf(&buffer_back[105],"%1.6f",acc_parameter.acc_zero_offset[2]);
	 buffer_back[113] = ',';

	 
	 sprintf(&buffer_back[114],"%1.6f",gyro_parameter.gryo_zero[0]);
	 buffer_back[122] = ',';
	 sprintf(&buffer_back[123],"%1.6f",gyro_parameter.gryo_zero[1]);
	 buffer_back[131] = ',';
	 sprintf(&buffer_back[132],"%1.6f",gyro_parameter.gryo_zero[2]);
	// buffer_back[140] = ',';
	  pc_uart_command_hardware_test_acc_data((uint8_t *)buffer_back,140);

}
void bubble_sort(float *ac, int n) 
{
    int i, j;
    float temp;
    for (j = 0; j < n - 1; j++)
    {
       for (i = 0; i < n - 1 - j; i++)
       {
          if(ac[i] > ac[i + 1])
          {
             temp=ac[i];
             ac[i]=ac[i+1]; 
             ac[i+1]=temp;
          }
       }
    }
}

float variance_data(float *data,uint16_t len)
{

	uint8_t i;
	float sum = 0.0f,mean_data = 0.0f,var_data = 0.0f;

	for(i=0;i<len;i++)
	{
	sum += *(data+i);

	}
	mean_data = sum/len;//鍧囧�?

	for(i=0;i<len;i++)
	{  

	var_data += pow((*(data+i))-mean_data,2);
	}
	var_data /= (len-1);
		
	return var_data;
}

uint8_t find_min(float *data,uint8_t length)
{
	uint8_t min = 0,j = 0;
	for(j=1;j<length;j++)
		{	
		 if(data[min]>data[j])
				min = j;	
	   }
	return min;

}

int8_t find_position(float *acc_data)
{
	uint8_t i = 0;//,min_index = 0;
	float length[10] = {0},threshold = 0;
  
	for(i = 0;i<10;i++)
		{
		 length[i] = sqrtf(powf((position_static[i][0] - acc_data[0]),2) + powf((position_static[i][1] - acc_data[1]),2) + powf((position_static[i][2] - acc_data[2]),2));
     
        }
	
      if((find_min(length,10)<=3))
		threshold = 0.15;
	else if((find_min(length,10)==4)&&(find_min(length,10)==5))
		threshold = 0.1;
	else
		threshold = 0.2;
	// printf("%f!r\\n",length[find_min(length,10)]);
	if((length[find_min(length,10)]<=threshold)&&(acc_cal_data[find_min(length,10)].captured == 0))
		{
		return find_min(length,10);
	    }
	else
		return -1;
		

}
void save_acc_calibration_paremeter(uint8_t flag)
{
  if(flag==0)
  	{
     acc_parameter.flag = 0;
  	}
  else
  	{
	  acc_parameter.flag = 137;


    }
  #if defined WATCH_IMU_CALIBRATION
	dev_extFlash_enable();
	dev_extFlash_erase(ACC_CALIBRATION_PARAMETER_STORAGE_ADDRESS,sizeof(acc_matrix_parameter));
	dev_extFlash_write(ACC_CALIBRATION_PARAMETER_STORAGE_ADDRESS,(uint8_t*)(&acc_parameter),sizeof(acc_matrix_parameter));
	dev_extFlash_disable();
	#endif
}
void read_acc_calibration_paremeter(void)
{
    #if defined WATCH_IMU_CALIBRATION
	dev_extFlash_enable();
	dev_extFlash_read(ACC_CALIBRATION_PARAMETER_STORAGE_ADDRESS,(uint8_t*)(&acc_parameter),sizeof(acc_matrix_parameter));
    dev_extFlash_disable();
	#endif
}
void save_gyro_calibration_paremeter(uint8_t flag)
{
  if(flag==0)
  	{
     gyro_parameter.flag = 0;
  	}
  else
  	{
	  gyro_parameter.flag = 137;


    }
   #if defined WATCH_IMU_CALIBRATION
	dev_extFlash_enable();
	dev_extFlash_erase(GYRO_CALIBRATION_PARAMETER_STORAGE_ADDRESS,sizeof(gyro_matrix_parameter));
	dev_extFlash_write(GYRO_CALIBRATION_PARAMETER_STORAGE_ADDRESS,(uint8_t*)(&gyro_parameter),sizeof(gyro_matrix_parameter));
	dev_extFlash_disable();
	#endif
}

void read_gyro_calibration_paremeter(void)
{
   #if defined WATCH_IMU_CALIBRATION
	dev_extFlash_enable();
	dev_extFlash_read(GYRO_CALIBRATION_PARAMETER_STORAGE_ADDRESS,(uint8_t*)(&gyro_parameter),sizeof(gyro_matrix_parameter));
    dev_extFlash_disable();
	#endif
}

void get_acc_use_parameter(float raw_acc[3])
{
	uint8_t i,j;
	float acc[3];
	for(i = 0;i < 3;i++)
	{
	 acc[i] = raw_acc[i];
	}

	for(i = 0;i < 3;i++)
	{
	  raw_acc[i] = 0;
	 for(j = 0;j < 3;j++)
	 	{
	    raw_acc[i]+=acc_parameter.factor_and_vertical[i][j]* acc[j];

	   }
	   raw_acc[i]+=acc_parameter.acc_zero_offset[i];

	}

}
void get_gyro_use_parameter(float raw_gyro[3])
{
	    uint8_t i;
	
		for(i = 0;i < 3;i++)
		{
		 raw_gyro[i] -= gyro_parameter.gryo_zero[i];
		}


}

void acc_parameter_reset(uint8_t flag)
{
	uint8_t i,j;
	acc_cal_flag.staticFlg = 0;
	acc_cal_flag.static_count = 0;
    acc_parameter.flag = 0;
	for(i = 0;i< ACC_CAL_SUM;i++)
	{
	 acc_cal_data[i].captured = 0;
	}
	adjustacc_count = 0;
	sample_count = 0;
	//gyro_count = 0;
	
      for(i = 0;i < 3;i++)
		{
         for(j = 0;j < 3;j++)
         	{
               acc_parameter.factor_and_vertical[i][j] = fav_data[i][j];
		   }

	    }
	for(i = 0;i < 3;i++)
		{
	    acc_parameter.acc_zero_offset[i] = zero_offset[i];
		//gyro_parameter.gryo_zero[i] = zero_offset[i];
		}
	  if(flag == 0)
	  	{
        save_acc_calibration_paremeter(0);
		//save_gyro_calibration_paremeter(0);
	    }
	  else
	  	{
        save_acc_calibration_paremeter(1);
		//save_gyro_calibration_paremeter(1);
	  	}
	

	 
	
	
}

void gyro_parameter_reset(uint8_t flag)
{
	uint8_t i,j;

	
	sample_count = 0;
	gyro_count = 0;
	
    
	for(i = 0;i < 3;i++)
		{
	   
		gyro_parameter.gryo_zero[i] = zero_offset[i];
		}
	  if(flag == 0)
	  	{
      
		save_gyro_calibration_paremeter(0);
	    }
	  else
	  	{
      
		save_gyro_calibration_paremeter(1);
	  	}
	

	 
	
	
}

void parameter_reset(uint8_t flag)
{
	uint8_t i,j;
	acc_cal_flag.staticFlg = 0;
	acc_cal_flag.static_count = 0;
    acc_parameter.flag = 0;
	for(i = 0;i< ACC_CAL_SUM;i++)
	{
	 acc_cal_data[i].captured = 0;
	}
	adjustacc_count = 0;
	sample_count = 0;
	gyro_count = 0;
	
      for(i = 0;i < 3;i++)
		{
         for(j = 0;j < 3;j++)
         	{
               acc_parameter.factor_and_vertical[i][j] = fav_data[i][j];
		   }

	    }
	for(i = 0;i < 3;i++)
		{
	    acc_parameter.acc_zero_offset[i] = zero_offset[i];
		gyro_parameter.gryo_zero[i] = zero_offset[i];
		}
	  if(flag == 0)
	  	{
        save_acc_calibration_paremeter(0);
		save_gyro_calibration_paremeter(0);
	    }
	  else
	  	{
        save_acc_calibration_paremeter(1);
		save_gyro_calibration_paremeter(1);
	  	}
	

	 
	
	
}

void get_static_gryo_data(void)
{
	 uint16_t i = 0;
	 
	 uint16_t imu_data_len = 0,len_m = 0;
	 uint8_t aData[2];
	 
	 /*2:读取传感器FIFO数据大小*/	 
	 uint16_t pattern = 0;
	 void *handle = NULL;
	 
	 LSM6DSL_ACC_GYRO_R_FIFONumOfEntries(handle,&imu_data_len);
	 LSM6DSL_ACC_GYRO_R_FIFOPattern(handle,&pattern);	 
	 //数据xyz顺序对齐
	 if ((pattern > 0) && (pattern < 6)){
		 for(i=0;i<6-pattern;i++){
			 LSM6DSL_ACC_GYRO_Get_GetFIFOData( handle, aData);
			 if(imu_data_len > 0){
				 imu_data_len--;
			 }
		 } 
	 }	 
	 
	 if(imu_data_len > 360){
		 len_m = 360;
		}
	 else {
		len_m = imu_data_len;
		}
	 //ALGO_IMU_LOG_PRINTF("\r\n******************pattern:%d,imu_data_len:%d,len_m:%d*******************************\r\n",pattern,imu_data_len,len_m);
	 /*3:取len_m大小的数据*/
	  uint16_t j = 0;
	 for(i = 0;i<len_m; i++)
	 {
		 if(!LSM6DSL_ACC_GYRO_Get_GetFIFOData( handle, aData ))
		 	{
		 	;
			// ALGO_IMU_LOG_PRINTF("LSM6DSL_ACC_GYRO_Get_GetFIFOData EEROR\r\n");
		 }
	
		 j = i/6;
		 switch(i%6)
		 {
			 case 0:
				 gyro_x[j] = ((int16_t)(( aData[1] << 8 ) | aData[0])) * 0.035f;				
				 break;
			 case 1:
				 gyro_y[j] = ((int16_t)(( aData[1] << 8 ) | aData[0]))* 0.035f;
				 break;
			 case 2:
				 gyro_z[j] = ((int16_t)(( aData[1] << 8 ) | aData[0]))* 0.035f;
				 break;
			 case 3:
				 acc_x[j] = ((int16_t)(( aData[1] << 8 ) | aData[0]))* 0.000122f;
				 break;
			 case 4:
				 acc_y[j] = ((int16_t)(( aData[1] << 8 ) | aData[0]))* 0.000122f;
				 break;
			 case 5:
				 acc_z[j] = ((int16_t)(( aData[1] << 8 ) | aData[0]))* 0.000122f;
				 break; 			 
			 default:
				 break;
		 }
		
	 }
	 len_m = len_m/6;
	 memset(acc_gyro_mean,0,sizeof(acc_gyro_mean));
	 for(i=0;i<len_m;i++)
	 {		 
	  acc_gyro_mean[0] += acc_x[i];
	  acc_gyro_mean[1] += acc_y[i];
	  acc_gyro_mean[2] += acc_z[i];
	  acc_gyro_mean[3] += gyro_x[i];
	  acc_gyro_mean[4] += gyro_y[i];
	  acc_gyro_mean[5] += gyro_z[i];
	 }
	 /*均值作为某个位置和时间点的实际值*/
	 acc_gyro_mean[0] /= len_m;
	 acc_gyro_mean[1] /= len_m;
	 acc_gyro_mean[2] /= len_m;
	 acc_gyro_mean[3] /= len_m;
	 acc_gyro_mean[4] /= len_m;
	 acc_gyro_mean[5] /= len_m;
	
	if(sample_count < ACC_SAMPLE_SUM)
	 {
		acc_gyro_data[sample_count].x_acc  = acc_gyro_mean[0];
		acc_gyro_data[sample_count].y_acc  = acc_gyro_mean[1];
		acc_gyro_data[sample_count].z_acc  = acc_gyro_mean[2];
		acc_gyro_data[sample_count].x_gyro = acc_gyro_mean[3];
		acc_gyro_data[sample_count].y_gyro = acc_gyro_mean[4];
		acc_gyro_data[sample_count].z_gyro = acc_gyro_mean[5];
		sample_count++;
	 }
	 else
	 {
		 for(i = 0;i < ACC_SAMPLE_SUM-1;i++)
		 {
		  acc_gyro_data[i].x_acc  = acc_gyro_data[i+1].x_acc;
		  acc_gyro_data[i].y_acc  = acc_gyro_data[i+1].y_acc;
		  acc_gyro_data[i].z_acc  = acc_gyro_data[i+1].z_acc;
		  acc_gyro_data[i].x_gyro = acc_gyro_data[i+1].x_gyro;
		  acc_gyro_data[i].y_gyro = acc_gyro_data[i+1].y_gyro;
		  acc_gyro_data[i].z_gyro = acc_gyro_data[i+1].z_gyro;
		 }
		 acc_gyro_data[ACC_SAMPLE_SUM-1].x_acc	= acc_gyro_mean[0];
		 acc_gyro_data[ACC_SAMPLE_SUM-1].y_acc	= acc_gyro_mean[1];
		 acc_gyro_data[ACC_SAMPLE_SUM-1].z_acc	= acc_gyro_mean[2];
		 acc_gyro_data[ACC_SAMPLE_SUM-1].x_gyro = acc_gyro_mean[3];
		 acc_gyro_data[ACC_SAMPLE_SUM-1].y_gyro = acc_gyro_mean[4];
		 acc_gyro_data[ACC_SAMPLE_SUM-1].z_gyro = acc_gyro_mean[5];
	 }
	
	  for(i = 0;i < ACC_SAMPLE_SUM;i++)
	  {
		 accX[i]  = acc_gyro_data[i].x_acc;
		 accY[i]  = acc_gyro_data[i].y_acc;
		 accZ[i]  = acc_gyro_data[i].z_acc;
		 gyroX[i] = acc_gyro_data[i].x_gyro;
		 gyroY[i] = acc_gyro_data[i].y_gyro;
		 gyroZ[i] = acc_gyro_data[i].z_gyro;
	  }
	 if(sample_count >= ACC_SAMPLE_SUM)
	 {
	   sample_count = ACC_SAMPLE_SUM;
	   var_acc_gyro[0] = variance_data(accX,ACC_SAMPLE_SUM);
	   var_acc_gyro[1] = variance_data(accY,ACC_SAMPLE_SUM);
	   var_acc_gyro[2] = variance_data(accZ,ACC_SAMPLE_SUM);
	   var_acc_gyro[3] = variance_data(gyroX,ACC_SAMPLE_SUM);
	   var_acc_gyro[4] = variance_data(gyroY,ACC_SAMPLE_SUM);
	   var_acc_gyro[5] = variance_data(gyroZ,ACC_SAMPLE_SUM);
	   
	  if((var_acc_gyro[0] < 0.01f) && (var_acc_gyro[1] < 0.01f) && (var_acc_gyro[2] < 0.01f)
		 &&(var_acc_gyro[3] < 0.03f) && (var_acc_gyro[4] < 0.03f) && (var_acc_gyro[5] < 0.03f))
		  {
		   gyro_cal_flag.staticFlg = 1;
		   gyro_cal_flag.static_count++;
			//gyro_count += 1;
			//gyro_parameter.gryo_zero[0]+=acc_gyro_mean[3];
			//gyro_parameter.gryo_zero[1]+=acc_gyro_mean[4];
			//gyro_parameter.gryo_zero[2]+=acc_gyro_mean[5];
			   
				
		  }
	  else
		  {
			 gyro_cal_flag.staticFlg = 0;
		     gyro_cal_flag.static_count =0;
			 gyro_count = 0; 
			 gyro_parameter.gryo_zero[0] = 0;
			 gyro_parameter.gryo_zero[1] = 0;
			 gyro_parameter.gryo_zero[2] = 0;
		  }
	
	 }


	  bubble_sort(gyro_x,len_m);
      bubble_sort(gyro_y,len_m);
      bubble_sort(gyro_z,len_m);
      memset(static_gyro,0,sizeof(static_gyro));
	  if(len_m > 10)
	  {
		  for (i = 0;i < (len_m-10);i++)
	      {
		    static_gyro[0] += gyro_x[i+5];
		    static_gyro[1] += gyro_y[i+5];
		    static_gyro[2] += gyro_z[i+5];
		  }
		  static_gyro[0] /= (len_m - 10);
		  static_gyro[1] /= (len_m - 10);
		  static_gyro[2] /= (len_m - 10);
	  }
	  else
	  {
		  for (i = 0;i < len_m;i++)
		  {
		    static_gyro[0] += gyro_x[i];
		    static_gyro[1] += gyro_y[i];
		    static_gyro[2] += gyro_z[i];
		  }
		  static_gyro[0] /= len_m;
		  static_gyro[1] /= len_m;
		  static_gyro[2] /= len_m;
	  }

	 char buffer[31];
	 sprintf(buffer,"%1.6f",static_gyro[0]);
	 buffer[8] = ',';
	 sprintf(&buffer[9],"%1.6f",static_gyro[1]);
	 buffer[17] = ',';
	 sprintf(&buffer[18],"%1.6f",static_gyro[2]);
	 buffer[26] = '*';	 
	 buffer[27] = gyro_cal_flag.staticFlg+'0';
	  buffer[28] = '#';
	  sprintf(&buffer[29],"%d",10);
	
	 pc_uart_command_hardware_test_acc_data((uint8_t *)buffer,31);

	 if((gyro_cal_flag.staticFlg == 1)&&(gyro_cal_flag.static_count >= 5))
   	{  
	    gyro_count++;
	    buffer[27] = '2';
		
	    sprintf(&buffer[29],"%d",10);
	    pc_uart_command_hardware_test_acc_data((uint8_t *)buffer,31);
	    gyro_parameter.gryo_zero[0]+=static_gyro[0];
	    gyro_parameter.gryo_zero[1]+=static_gyro[1];
		gyro_parameter.gryo_zero[2]+=static_gyro[2];
	 
     }

 


}

void get_static_data(void)
{
 	uint16_t i = 0;
	
	uint16_t imu_data_len = 0,len_m = 0;
	uint8_t aData[2];
	
	/*2:读取传感器FIFO数据大小*/	
	uint16_t pattern = 0;
	void *handle = NULL;

	
	

	
	LSM6DSL_ACC_GYRO_R_FIFONumOfEntries(handle,&imu_data_len);
	LSM6DSL_ACC_GYRO_R_FIFOPattern(handle,&pattern);	
	//数据xyz顺序对齐
	if ((pattern > 0) && (pattern < 6)){
		for(i=0;i<6-pattern;i++){
			LSM6DSL_ACC_GYRO_Get_GetFIFOData( handle, aData);
			if(imu_data_len > 0){
				imu_data_len--;
			}
		} 
	}	
	
	if(imu_data_len > 360){
        len_m = 360;
	   }
	else {
       len_m = imu_data_len;
       }
	//ALGO_IMU_LOG_PRINTF("\r\n******************pattern:%d,imu_data_len:%d,len_m:%d*******************************\r\n",pattern,imu_data_len,len_m);
	/*3:取len_m大小的数据*/
     uint16_t j = 0;
	for(i = 0;i<len_m; i++)
	{
		if(!LSM6DSL_ACC_GYRO_Get_GetFIFOData( handle, aData )){
			ALGO_IMU_LOG_PRINTF("LSM6DSL_ACC_GYRO_Get_GetFIFOData EEROR\r\n");
		}
		//ALGO_IMU_LOG_PRINTF("data:%d,  ",(( aData[1] << 8 ) | aData[0]));
        j = i/6;
		switch(i%6)
		{
			case 0:
				gyro_x[j] = ((int16_t)(( aData[1] << 8 ) | aData[0])) * 0.035f;                
				break;
			case 1:
				gyro_y[j] = ((int16_t)(( aData[1] << 8 ) | aData[0]))* 0.035f;
				break;
			case 2:
				gyro_z[j] = ((int16_t)(( aData[1] << 8 ) | aData[0]))* 0.035f;
				break;
			case 3:
				acc_x[j] = ((int16_t)(( aData[1] << 8 ) | aData[0]))* 0.000122f;
				break;
			case 4:
				acc_y[j] = ((int16_t)(( aData[1] << 8 ) | aData[0]))* 0.000122f;
				break;
			case 5:
				acc_z[j] = ((int16_t)(( aData[1] << 8 ) | aData[0]))* 0.000122f;
				break;				
			default:
				break;
		}
		//if (i%6 == 5){
			//ALGO_IMU_LOG_PRINTF("\r\n");
        //	ALGO_IMU_LOG_PRINTF("acc_x:%d,  acc_y:%d,  acc_z:%d,  gyro_x:%d,  gyro_y:%d,  gyro_z:%d\r\n",acc_x[j],acc_y[j],acc_z[j],gyro_x[j],gyro_y[j],gyro_z[j]);
		//}
    }
	len_m = len_m/6;
	memset(acc_gyro_mean,0,sizeof(acc_gyro_mean));
	for(i=0;i<len_m;i++)
	{		
	 acc_gyro_mean[0] += acc_x[i];
	 acc_gyro_mean[1] += acc_y[i];
	 acc_gyro_mean[2] += acc_z[i];
	 acc_gyro_mean[3] += gyro_x[i];
	 acc_gyro_mean[4] += gyro_y[i];
	 acc_gyro_mean[5] += gyro_z[i];
	}
	/*均值作为某个位置和时间点的实际值*/
	acc_gyro_mean[0] /= len_m;
	acc_gyro_mean[1] /= len_m;
	acc_gyro_mean[2] /= len_m;
	acc_gyro_mean[3] /= len_m;
	acc_gyro_mean[4] /= len_m;
	acc_gyro_mean[5] /= len_m;

   if(sample_count < ACC_SAMPLE_SUM)
	{
	   acc_gyro_data[sample_count].x_acc  = acc_gyro_mean[0];
	   acc_gyro_data[sample_count].y_acc  = acc_gyro_mean[1];
	   acc_gyro_data[sample_count].z_acc  = acc_gyro_mean[2];
	   acc_gyro_data[sample_count].x_gyro = acc_gyro_mean[3];
	   acc_gyro_data[sample_count].y_gyro = acc_gyro_mean[4];
	   acc_gyro_data[sample_count].z_gyro = acc_gyro_mean[5];
	   sample_count++;
	}
	else
	{
		for(i = 0;i < ACC_SAMPLE_SUM-1;i++)
		{
		 acc_gyro_data[i].x_acc  = acc_gyro_data[i+1].x_acc;
		 acc_gyro_data[i].y_acc  = acc_gyro_data[i+1].y_acc;
		 acc_gyro_data[i].z_acc  = acc_gyro_data[i+1].z_acc;
		 acc_gyro_data[i].x_gyro = acc_gyro_data[i+1].x_gyro;
		 acc_gyro_data[i].y_gyro = acc_gyro_data[i+1].y_gyro;
		 acc_gyro_data[i].z_gyro = acc_gyro_data[i+1].z_gyro;
		}
		acc_gyro_data[ACC_SAMPLE_SUM-1].x_acc  = acc_gyro_mean[0];
		acc_gyro_data[ACC_SAMPLE_SUM-1].y_acc  = acc_gyro_mean[1];
		acc_gyro_data[ACC_SAMPLE_SUM-1].z_acc  = acc_gyro_mean[2];
		acc_gyro_data[ACC_SAMPLE_SUM-1].x_gyro = acc_gyro_mean[3];
		acc_gyro_data[ACC_SAMPLE_SUM-1].y_gyro = acc_gyro_mean[4];
		acc_gyro_data[ACC_SAMPLE_SUM-1].z_gyro = acc_gyro_mean[5];
	}
 
     for(i = 0;i < ACC_SAMPLE_SUM;i++)
     {
		accX[i]  = acc_gyro_data[i].x_acc;
		accY[i]  = acc_gyro_data[i].y_acc;
		accZ[i]  = acc_gyro_data[i].z_acc;
	    gyroX[i] = acc_gyro_data[i].x_gyro;
		gyroY[i] = acc_gyro_data[i].y_gyro;
	    gyroZ[i] = acc_gyro_data[i].z_gyro;
	 }
	if(sample_count >= ACC_SAMPLE_SUM)
	{
	  sample_count = ACC_SAMPLE_SUM;
	  var_acc_gyro[0] = variance_data(accX,ACC_SAMPLE_SUM);
	  var_acc_gyro[1] = variance_data(accY,ACC_SAMPLE_SUM);
	  var_acc_gyro[2] = variance_data(accZ,ACC_SAMPLE_SUM);
	  var_acc_gyro[3] = variance_data(gyroX,ACC_SAMPLE_SUM);
	  var_acc_gyro[4] = variance_data(gyroY,ACC_SAMPLE_SUM);
	  var_acc_gyro[5] = variance_data(gyroZ,ACC_SAMPLE_SUM);
	  
	 if((var_acc_gyro[0] < 0.02f) && (var_acc_gyro[1] < 0.02f) && (var_acc_gyro[2] < 0.02f)
	 	&&(var_acc_gyro[3] < 0.03f) && (var_acc_gyro[4] < 0.03f) && (var_acc_gyro[5] < 0.03f))
		 {
			 acc_cal_flag.staticFlg = 1;
			 if((fabs(acc_gyro_mean[3])<8.0f)&&(fabs(acc_gyro_mean[4])<8.0f)&&(fabs(acc_gyro_mean[5])<8.0f))
			    acc_cal_flag.static_count++;
			 else
				acc_cal_flag.static_count = 0;
		 }
	 else
		 {
		 	
			acc_cal_flag.staticFlg = 0; 
			acc_cal_flag.static_count = 0;
		 }

	}

	  bubble_sort(acc_x,len_m);
      bubble_sort(acc_y,len_m);
      bubble_sort(acc_z,len_m);
      memset(static_acc,0,sizeof(static_acc));
	  if(len_m > 10)
	  {
		  for (i = 0;i < (len_m-10);i++)
	      {
		    static_acc[0] += acc_x[i+5];
		    static_acc[1] += acc_y[i+5];
		    static_acc[2] += acc_z[i+5];
		  }
		  static_acc[0] /= (len_m - 10);
		  static_acc[1] /= (len_m - 10);
		  static_acc[2] /= (len_m - 10);
	  }
	  else
	  {
		  for (i = 0;i < len_m;i++)
		  {
		    static_acc[0] += acc_x[i];
		    static_acc[1] += acc_y[i];
		    static_acc[2] += acc_z[i];
		  }
		  static_acc[0] /= (len_m);
		  static_acc[1] /= (len_m);
		  static_acc[2] /= (len_m);
	  }

	  
	 /*4:此处回传 static_acc[0],static_acc[1],static_acc[2]：acc_cal_flag.staticFlg #\r\n */
	 char buffer[31];
	 sprintf(buffer,"%1.6f",static_acc[0]);
	 buffer[8] = ',';
	 sprintf(&buffer[9],"%1.6f",static_acc[1]);
	 buffer[17] = ',';
	 sprintf(&buffer[18],"%1.6f",static_acc[2]);
	 buffer[26] = '*';	 
	 buffer[27] = acc_cal_flag.staticFlg+'0';
	  buffer[28] = '#';
	  sprintf(&buffer[29],"%d",position_index);
	
	 pc_uart_command_hardware_test_acc_data((uint8_t *)buffer,31);
	 
	/* char buffer[112];
	 sprintf(buffer,"%1.6f",static_acc[0]);
	 buffer[8] = ',';
	 sprintf(&buffer[9],"%1.6f",static_acc[1]);
	 buffer[17] = ',';
	 sprintf(&buffer[18],"%1.6f",static_acc[2]);
	  buffer[26] = ',';
	  sprintf(&buffer[27],"%1.6f",acc_gyro_mean[3]);
	   buffer[35] = ',';
	   sprintf(&buffer[36],"%1.6f",acc_gyro_mean[4]);
	    buffer[44] = ',';
	    sprintf(&buffer[45],"%1.6f",acc_gyro_mean[5]);
	    buffer[53] = ',';	
	 
	 sprintf(&buffer[54],"%1.6f",var_acc_gyro[0]);
		 buffer[62] = ',';	
		 
		  sprintf(&buffer[63],"%1.6f",var_acc_gyro[1]);
	 buffer[71] = ',';	
	  sprintf(&buffer[72],"%1.6f",var_acc_gyro[2]);
	  
	 buffer[80] = ',';	
     sprintf(&buffer[81],"%1.6f",var_acc_gyro[3]);
	  
	 buffer[89] = ',';
	  sprintf(&buffer[90],"%1.6f",var_acc_gyro[4]);
	  
	 buffer[98] = ',';
	  sprintf(&buffer[99],"%1.6f",var_acc_gyro[5]);
	  
	 buffer[107] = ':';
	 
	 buffer[108] = acc_cal_flag.staticFlg+'0';
	 buffer[109] = ',';
	 sprintf(&buffer[110],"%d",position_index);
	 
	  
	 pc_uart_command_hardware_test_acc_data((uint8_t *)buffer,112);*/
	 
   if((acc_cal_flag.staticFlg == 1)&&(acc_cal_flag.static_count >= 7))
   	{  
	  
	 buffer[27] = '2';
	  position_index = find_position(static_acc);
	  sprintf(&buffer[29],"%d",position_index);
     if(position_index == -1)
       {
          pc_uart_command_hardware_test_acc_data((uint8_t *)buffer,31);
	   }
	 else
	   {
          pc_uart_command_hardware_test_acc_data((uint8_t *)buffer,30);
	   }
	 
      if(position_index >= 0) 
	    {
	      acc_cal_data[position_index].acc_x = static_acc[0];
	      acc_cal_data[position_index].acc_y = static_acc[1];
	      acc_cal_data[position_index].acc_z = static_acc[2];
	      acc_cal_data[position_index].captured = 1;
	    }
     	acc_cal_flag.static_count = 0;	 
		position_index = -1;
	}

  adjustacc_count = 0;
  for(i = 0;i < 10;i++)
  {
    if(acc_cal_data[i].captured == 1)
      {
      adjustacc_count++;
      }
  }
}
	
void get_matrix_surplus(float acc_BBT[9][9], int n, float ans[9][9])//计算每一行每一列的每个元素所对应的余子式，组成A*
{	
	if (n == 1)
		{		
		ans[0][0] = 1;		
		return;
		}	
	int i, j, k, t;	
	float temp[9][9];	
	for (i = 0; i<n; i++)
		{		
		for (j = 0; j<n; j++)	
			{			
			for (k = 0; k<n - 1; k++)	
				{				
				for (t = 0; t<n - 1; t++)	
					{					
					temp[k][t] = acc_BBT[k >= i ? k + 1 : k][t >= j ? t + 1 : t];		
					}			
				}			
			ans[j][i] = get_det(temp, n - 1);		
			if ((i + j) % 2 == 1)			
				{				
				ans[j][i] = -ans[j][i];		
				}		
			}	
		}
}

float get_det(float acc_BBT[9][9], uint8_t n)//求行列式的值
{
	if(n==1)	
		{		
		return acc_BBT[0][0];	
		}
	float ans = 0;	
	float temp[9][9];
	int i,j,k;	
	for(i=0;i<n;i++)	
		{		
		for(j=0;j<n-1;j++)	
			{			
			for(k=0;k<n-1;k++)			
				{				
				temp[j][k] = acc_BBT[j+1][(k>=i)?k+1:k];					
				}		
			}		
		float t = get_det(temp,n-1); 	
		if(i%2==0)		
			{			
			ans += acc_BBT[0][i]*t;		
			}		
		else	
			{		
			ans -=	acc_BBT[0][i]*t;	
			}	
		}	
	return ans;
	


}

void acc_matrix_inversion(float matrix_acc[9][9])//
{
	float acc_complement[9][9];//代数余子式
	float det_value;
	uint8_t i,j; 
	det_value = get_det(matrix_acc,9);
	get_matrix_surplus(matrix_acc,9,acc_complement);

	for (i = 0; i < 9; i++)		
		{				
		for (j = 0; j < 9; j++)		
			{				
			 acc_inverse[i][j] =acc_complement[i][j]/det_value;	
			}				
			
		}

}
/*****************矩阵原地转置BEGIN********************/

/* 后继 */
int getNext(int i, int m, int n)
{
	  return (i%n)*m + i / n;
	}

/* 前驱 */
int getPre(int i, int m, int n)
{
	  return (i%m)*n + i / m;
	}

/* 处理以下标i为起点的环 */
void movedata(double *mtx, int i, int m, int n)
{
	  double temp = mtx[i]; // 暂存
	  int cur = i;	  // 当前下标
	  int pre = getPre(cur, m, n);
	  while (pre != i)
		  {
			mtx[cur] = mtx[pre];
			cur = pre;
			pre = getPre(cur, m, n);
		  }
	  mtx[cur] = temp;
	}

/* 转置，即循环处理所有环 */
 void transpose(double *mtx, int m, int n)
 {
	   for (int i = 0; i<m*n; ++i)
		   {
		     int next = getNext(i, m, n);
		     while (next > i) // 若存在后继小于i说明重复,就不进行下去了（只有不重复时进入while循环）
			       next = getNext(next, m, n);
		     if (next == i)  // 处理当前环
			       movedata(mtx, i, m, n);
		   }
	 }

 void LUP_Solve(double L[MATRIX_ORDER*MATRIX_ORDER], double U[MATRIX_ORDER*MATRIX_ORDER], int P[MATRIX_ORDER], double b[MATRIX_ORDER],double*x)
 {
	  
	     double *y =(double*)pvPortMalloc(MATRIX_ORDER*sizeof(double));
	
		     //正向替换
		     for (int i = 0; i < MATRIX_ORDER; i++)
		     {
		        y[i] = b[P[i]];
	         for (int j = 0; j < i; j++)
			        {
			             y[i] = y[i] - L[i*MATRIX_ORDER + j] * y[j];
			         }
		     }
	     //反向替换
		     for (int i = MATRIX_ORDER - 1; i >= 0; i--)
		     {
		         x[i] = y[i];
		         for (int j = MATRIX_ORDER - 1; j > i; j--)
			         {
			             x[i] = x[i] - U[i*MATRIX_ORDER + j] * x[j];
			         }
		         x[i] /= U[i*MATRIX_ORDER + i];
		     }
	        vPortFree(y);
			 y = NULL;
	 }

void LUP_Descomposition(double A[MATRIX_ORDER*MATRIX_ORDER], double L[MATRIX_ORDER*MATRIX_ORDER], double U[MATRIX_ORDER*MATRIX_ORDER], int P[MATRIX_ORDER])
 {
	     int row = 0;
	     for (int i = 0; i<MATRIX_ORDER; i++)
		     {
		         P[i] = i;
		     }
	     for (int i = 0; i<MATRIX_ORDER - 1; i++)
		     {
				 double p = 0.0;
		         for (int j = i; j<MATRIX_ORDER; j++)
			         {
			             if (fabs(A[j*MATRIX_ORDER + i])>p)
				             {
			                 p = fabs(A[j*MATRIX_ORDER + i]);
			                row = j;
				             }
			         }
		         if (0 == p)
			         {
			             
			            return;
			         }
		
			         //交换P[i]和P[row]
			         int tmp = P[i];
		         P[i] = P[row];
		         P[row] = tmp;
		
			        double tmp2 = 0.0;
		        for (int j = 0; j<MATRIX_ORDER; j++)
			        {
			             //交换A[i][j]和 A[row][j]
				             tmp2 = A[i*MATRIX_ORDER + j];
			            A[i*MATRIX_ORDER + j] = A[row*MATRIX_ORDER + j];
			            A[row*MATRIX_ORDER + j] = tmp2;
			         }
		
			        //以下同LU分解
			         double u = A[i*MATRIX_ORDER + i], l = 0.0;
		         for (int j = i + 1; j<MATRIX_ORDER; j++)
		         {
			             l = A[j*MATRIX_ORDER + i] / u;
			            A[j*MATRIX_ORDER + i] = l;
			             for (int k = i + 1; k<MATRIX_ORDER; k++)
			            {
				            A[j*MATRIX_ORDER + k] = A[j*MATRIX_ORDER + k] - A[i*MATRIX_ORDER + k] * l;
				        }
			     }
		
			 }
	
		     //构造L和U
		     for (int i = 0; i<MATRIX_ORDER; i++)
		     {
		         for (int j = 0; j <= i; j++)
			         {
			             if (i != j)
				            {
				                L[i*MATRIX_ORDER+ j] = A[i*MATRIX_ORDER + j];
				            }
			             else
				             {
			                 L[i*MATRIX_ORDER + j] = 1;
				             }
			         }
	            for (int k = i; k<MATRIX_ORDER; k++)
			        {
			             U[i*MATRIX_ORDER + k] = A[i*MATRIX_ORDER + k];
			         }
		     }
	
		 }

void LUP_solve_inverse(double A[MATRIX_ORDER*MATRIX_ORDER])
 {
	    
	 int i, j;
	 double *A_mirror = (double *)pvPortMalloc(MATRIX_ORDER*MATRIX_ORDER*sizeof(double));

	// double *inv_A = (double *)malloc(N*N*sizeof(double));;//最终的逆矩阵（还需要转置）

	 double *inv_A_each = (double *)pvPortMalloc(MATRIX_ORDER*sizeof(double));//矩阵逆的各列
	 double *b = (double *)pvPortMalloc(MATRIX_ORDER*sizeof(double));//b阵为B阵的列矩阵分量

		     for (int i = 0; i<MATRIX_ORDER; i++)
	     {
			 double *L = (double*)pvPortMalloc(MATRIX_ORDER*MATRIX_ORDER*sizeof(double));
			 double *U = (double*)pvPortMalloc(MATRIX_ORDER*MATRIX_ORDER*sizeof(double));
			 int *P = (int *)pvPortMalloc(MATRIX_ORDER*sizeof(int));
		
			         //构造单位阵的每一列
			         for (int i = 0; i<MATRIX_ORDER; i++)
			         {
			             b[i] = 0;
			         }
		         b[i] = 1;
		
		         //每次都需要重新将A复制一份
			        for (int i = 0; i<MATRIX_ORDER*MATRIX_ORDER; i++)
			        {
			             A_mirror[i] = A[i];
			         }
		
		         LUP_Descomposition(A_mirror, L, U, P);
	
		         LUP_Solve(L, U, P, b,inv_A_each);
		         memcpy(&acc_inverse[0][0] + i*MATRIX_ORDER, inv_A_each, MATRIX_ORDER*sizeof(double));//将各列拼接起来
				 vPortFree(L);
				 vPortFree(U);
				 vPortFree(P);
				 L = NULL;
				 U = NULL;
				 P = NULL;
		     }
	    transpose(&acc_inverse[0][0], MATRIX_ORDER,MATRIX_ORDER);//由于现在根据每列b算出的x按行存储，因此需转置
		
		vPortFree(A_mirror);
		
		vPortFree(inv_A_each);
		vPortFree(b);
		A_mirror = NULL;
		
		inv_A_each = NULL;
		b = NULL;
	     return;
	 }

void acc_matrix_multiply(float acc_matrix_inver[9][9],float acc_matrix_B[9][ACC_CAL_SUM])
{
	float B0[10] = {1,1,1,1,1,1,1,1,1,1};
	float acc_matrix_temp[9][ACC_CAL_SUM];
	uint8_t i,j,k;
	 for(i = 0;i < 9;i++)
	 	{
	     for(j = 0;j < ACC_CAL_SUM;j++)
	     	{
	     	 acc_matrix_temp[i][j] = 0;
	     	  for(k = 0;k < 9;k++ )
	     	  	{
	            acc_matrix_temp[i][j] += acc_matrix_inver[i][k] * acc_matrix_B[k][j];
			    }
	     	}

	   }
	 for(i = 0;i < 9;i++)
	 	{
	      acc_array_A[i] = 0;
		  for(j = 0;j < 10;j++ )
		  	{
	         acc_array_A[i] += acc_matrix_temp[i][j]*B0[j];
		    }
	    }

 
}
void get_acc_parameter()
{

	float v[3];//正交误差
	float z[3];//零偏
	float parameter_k[3][3] = {{0,0,0},{0,0,0},{0,0,0}};//标度因子构成的矩阵
	float parameter_v[3][3] = {{1,0,0},{0.05,1,0},{0.04,0.03,1}};//正交构成的矩阵
	uint8_t i,j,k;

	for(i = 0;i < 3;i++)
	{
	 parameter_k[i][i] = sqrt(acc_array_A[i]);

	}
	v[0] = 0.5f*(acc_array_A[5]/acc_array_A[2]);
	v[1] = 0.5f*(acc_array_A[4]/acc_array_A[2]);
	v[2] = 0.5f*(acc_array_A[3]/acc_array_A[1]);
	parameter_v[2][1] = v[0];
	parameter_v[2][0] = v[1];
	parameter_v[1][0] = v[2];
	for(i = 0;i < 3;i++)
	{
	 
	 for(j = 0;j < 3;j++)
	 	{
	    acc_parameter.factor_and_vertical[i][j] = 0;
		 for(k = 0;k < 3;k++)
		 	{
	            
	          acc_parameter.factor_and_vertical[i][j] += parameter_k[i][k]*parameter_v[k][j];
		    }
	    }

	}
	for(i = 0;i < 3;i++)
	{
	 acc_parameter.acc_zero_offset[i] = 0.5*(acc_array_A[6+i]/sqrt(acc_array_A[i]));
	}


}

void acc_matrix_operation()
{
  uint8_t k,i,j;
  float parameter_BT[ACC_CAL_SUM][9];
  float parameter_B[9][ACC_CAL_SUM];
  float parameter_BBT[9][9];
  
  for(k = 0;k < ACC_CAL_SUM;k++) 
	 {
	   parameter_BT[k][0] = acc_cal_data[k].acc_x * acc_cal_data[k].acc_x;
	   parameter_BT[k][1] = acc_cal_data[k].acc_y * acc_cal_data[k].acc_y;
	   parameter_BT[k][2] = acc_cal_data[k].acc_z * acc_cal_data[k].acc_z;
	   parameter_BT[k][3] = acc_cal_data[k].acc_x * acc_cal_data[k].acc_y;
	   parameter_BT[k][4] = acc_cal_data[k].acc_x * acc_cal_data[k].acc_z;
	   parameter_BT[k][5] = acc_cal_data[k].acc_y * acc_cal_data[k].acc_z;
	   parameter_BT[k][6] = acc_cal_data[k].acc_x;
	   parameter_BT[k][7] = acc_cal_data[k].acc_y;
	   parameter_BT[k][8] = acc_cal_data[k].acc_z;
	 }
  for(k = 0;k < ACC_CAL_SUM;k++)
  	{
  	  for(i = 0;i < 9;i++)
  	  	{
         parameter_B[i][k] = parameter_BT[k][i];
  	  	}

    }
   for(k = 0;k < 9;k++)
   	{
     for(i = 0;i < 9;i++)
     	{
         parameter_BBT[k][i] = 0;
         	for(j = 0;j < ACC_CAL_SUM;j++)
         		{
                 parameter_BBT[k][i] += parameter_B[k][j] * parameter_BT[j][i];
                }
        }
    }
   //acc_matrix_inversion(parameter_BBT);//求出了BBT的逆矩阵
   LUP_solve_inverse(&parameter_BBT[0][0]);
   acc_matrix_multiply(acc_inverse,parameter_B);//求出参数数组
   get_acc_parameter();//求出最终的参数
   
   
}

void calculate_acc_parameter(void)
{	
  acc_matrix_operation();//求逆矩阵 and 矩阵A
}

void acc_calibration_entrance(void)
{
  //  acc_cal_flag.cal_flag = CAL_ING;
  uint8_t i;
  uint16_t len;
  void *handle = NULL;
  start_cal_flag = 1;
  imu_cal_flag = CAL_ING;

    for(i=0;i<ACC_CAL_SUM;i++)
	{
	  memset(&acc_cal_data[i],0,sizeof(imu_acc_data));
	}
	read_acc_calibration_paremeter();
		if((adjustacc_count == ACC_CAL_SUM)||(acc_parameter.flag == 137))
		{
         pc_uart_cal_back(1);
		  imu_cal_flag = CAL_QUIT;
		 return;
		}
	/*1:关闭其他使用加速度计和陀螺仪的任务；重新配置加速度和陀螺仪的输出速率(208Hz)；使能传感器输出*/
	
	CloseStepTask();
	drv_lsm6dsl_reset();
	
	drv_lsm6dsl_acc_test_init();
	drv_lsm6dsl_gyro_test_init(LSM6DSL_ACC_GYRO_ODR_G_208Hz,LSM6DSL_ACC_GYRO_FS_G_1000dps);	
	drv_lsm6dsl_fifo_test_init();

	while(adjustacc_count < ACC_CAL_SUM)  //未采到10点
	{
	 if((quit_cal_flag == 1)||(m_initialized==false))//强制退出校准/充电线被断开
	 	{
	 	if(quit_cal_flag == 1)
	 	 {
	 	 quit_cal_flag = 0;
	 	 }
		
		 start_cal_flag = 0; 
		 sample_count = 0;
		 for(i=0;i<ACC_CAL_SUM;i++)
	     {
	      memset(&acc_cal_data[i],0,sizeof(imu_acc_data));
	      }
		 pc_uart_cal_back(2);
		 drv_lsm6dsl_acc_init();	
	     drv_lsm6dsl_gyro_init();	
	     drv_lsm6dsl_acc_fifo_init();
	     CreateStepTask();
		imu_cal_flag = CAL_QUIT;
		 return;
	 	}
	
	  DELAY_MS(200);
	  get_static_data();		
	}
    
	if(adjustacc_count >= ACC_CAL_SUM) //采到了10点
	{
	  calculate_acc_parameter();
	}
	  /*6:执行参数存储操作*/
    save_acc_calibration_paremeter(1);
	
	//acc_cal_flag.cal_flag = CAL_OVER;
    /*7:恢复加速度计和陀螺仪配置，恢复其他任务*/
	
	drv_lsm6dsl_acc_init();	
	drv_lsm6dsl_gyro_init();	
	drv_lsm6dsl_acc_fifo_init();
	CreateStepTask();
	pc_uart_cal_back(0);//参数计算完成
	 start_cal_flag = 0;
	sample_count = 0;
	 for(i=0;i<ACC_CAL_SUM;i++)
	    {
	     memset(&acc_cal_data[i],0,sizeof(imu_acc_data));
	     }

	LSM6DSL_ACC_GYRO_R_FIFONumOfEntries(handle,&len);
	imu_cal_flag = CAL_QUIT;
}
void gyro_calibration_entrance()
{
	 uint16_t len;
	 void *handle = NULL;

    start_cal_flag = 1;
	imu_cal_flag = CAL_ING;
	memset(&gyro_parameter,0,sizeof(gyro_matrix_parameter));
    /*读取参数存储区*/
	read_gyro_calibration_paremeter();
	if((gyro_count == GRYO_CAL_SUM)||(gyro_parameter.flag == 137))
	//if(gyro_count == GRYO_CAL_SUM)
	  {
        pc_uart_cal_back(3);//已校准
        imu_cal_flag = CAL_QUIT;
		return;
	  }
	/*关闭相关任务及设置传感器*/
	CloseStepTask();
	drv_lsm6dsl_reset();
	drv_lsm6dsl_acc_test_init();
	drv_lsm6dsl_gyro_test_init(LSM6DSL_ACC_GYRO_ODR_G_208Hz,LSM6DSL_ACC_GYRO_FS_G_1000dps);	
	drv_lsm6dsl_fifo_test_init();
	/*校准*/
	while(gyro_count < GRYO_CAL_SUM) 
	{
      if((quit_cal_flag == 1)||(m_initialized==false))//强制退出校准
	 	{
	 	if(quit_cal_flag == 1)
	 	 {
	 	   quit_cal_flag = 0;
	 	 }
		 start_cal_flag = 0;
		 gyro_count = 0;
		 memset(&gyro_parameter,0,sizeof(gyro_matrix_parameter));
		 pc_uart_cal_back(2);
		 drv_lsm6dsl_acc_init();	
	     drv_lsm6dsl_gyro_init();	
	     drv_lsm6dsl_acc_fifo_init();
	     CreateStepTask();
		 imu_cal_flag = CAL_QUIT;
		 return;
	 	}
	
		 DELAY_MS(200);
	    get_static_gryo_data();	
	}
	/*计算校准参数*/
	if(gyro_count >= GRYO_CAL_SUM)
		{
          gyro_parameter.gryo_zero[0] = gyro_parameter.gryo_zero[0]/gyro_count;
		  gyro_parameter.gryo_zero[1] = gyro_parameter.gryo_zero[1]/gyro_count;
		  gyro_parameter.gryo_zero[2] = gyro_parameter.gryo_zero[2]/gyro_count;
	   }
	/*执行校准参数存储*/
      save_gyro_calibration_paremeter(1);
	 /*7:恢复加速度计和陀螺仪配置，恢复其他任务*/
	
	drv_lsm6dsl_acc_init();	
	drv_lsm6dsl_gyro_init();	
	drv_lsm6dsl_acc_fifo_init();
	CreateStepTask();
	pc_uart_cal_back(0);//参数计算完成
	 start_cal_flag = 0;
     sample_count = 0;
	 memset(&gyro_parameter,0,sizeof(gyro_matrix_parameter));
	LSM6DSL_ACC_GYRO_R_FIFONumOfEntries(handle,&len);
	imu_cal_flag = CAL_QUIT;
}

#endif



