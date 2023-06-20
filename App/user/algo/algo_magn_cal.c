#include "algo_magn_cal.h"
#include "drv_extFlash.h"
#include "algo_step_count.h"
#include "drv_config.h"
#include "com_data.h"
#include "drv_lsm6dsl.h"
#include "gui_tool_compass.h"
#include "drv_lcd.h"
#include "task_step.h"

#include <math.h>
/*temp file*/
#include "task.h"

#if DEBUG_ENABLED == 1 && DRV_MS5837_LOG_ENABLED == 1
	
	#define ALGO_IMU_LOG_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
    #define ALGO_IMU_LOG_PRINTF(...) 	

#endif
/*variable of temp*/

extern TaskHandle_t TaskToolHandle;



uint8_t magn_caling_state[MAGN_CAL_STATE_NUM] = {0};//flag and num of point


magn_cal_data cal_magn_para[MAGN_CAL_PARA_MIN];
magn_cal_data magn_point_save[MAGN_CAL_POINT_MIN];
magn_cal_data magn_point_dis[MAGN_CAL_POINT_MIN];
magn_cal_data magn_point_zero;



magn_matrix_parameter magn_cal_para = {{0,0,0},{1,0,0,0,1,0,0,0,1},{0}};
magn_matrix_parameter mag_output_data = {{0,0,0},{1,0,0,0,1,0,0,0,1},{0}};
magn_matrix_parameter algo_mag_output_data = {{0,0,0},{1,0,0,0,1,0,0,0,1},{0}};




double magn_s_inverse[MAGN_MATRIX_ORDER][MAGN_MATRIX_ORDER];//逆矩阵


double magn_data_com[MAGN_CAL_PARA_MIN][10];

float  Data_COM_TMP[3] = {0};

acc_data sample_acc;

uint8_t acc_sample_count = 0;
uint8_t		g_CompassCaliStatus;

float std_acc_magn = 100.0,std_acc_x = 100.0,mean_acc_magn = 1.0f;//初始化赋值大点
float azimuth_init = 0;

float radtodeg = (180/3.141592654f);

uint8_t magn_cal_flag_get(void)
{

//返回校准状态
	return g_CompassCaliStatus;
}
void save_magn_calibration_paremeter(void)
{
 
     magn_cal_para.flag = 137;
	dev_extFlash_enable();
	dev_extFlash_erase(MAGN_CALIBRATION_PARAMETER_STORAGE_ADDRESS,sizeof(magn_matrix_parameter));
	dev_extFlash_write(MAGN_CALIBRATION_PARAMETER_STORAGE_ADDRESS,(uint8_t*)(&magn_cal_para),sizeof(magn_matrix_parameter));
	dev_extFlash_disable();
	
}
void read_magn_calibration_paremeter(void)
{
   
	dev_extFlash_enable();
	dev_extFlash_read(MAGN_CALIBRATION_PARAMETER_STORAGE_ADDRESS,(uint8_t*)(&magn_cal_para),sizeof(magn_matrix_parameter));
    dev_extFlash_disable();
	
}

void magn_para_reset(void)
{
	 uint8_t i;
	
		g_CompassCaliStatus = 0;
		sample_acc.static_flag = 0;
	    sample_acc.static_count = 0;
		for(i=0;i<MAGN_CAL_STATE_NUM;i++)
			{
			magn_caling_state[i] = 0;
			}
		
		
		for(i=0;i<MAGN_CAL_PARA_MIN;i++)
			{
	
			cal_magn_para[i].x = MAGN_CAL_SMAX;
			cal_magn_para[i].y = MAGN_CAL_SMAX;
			cal_magn_para[i].z = MAGN_CAL_SMAX;
	
			}
		for(i=0;i<MAGN_CAL_POINT_MIN;i++)
			{
	
			magn_point_save[i].x = MAGN_CAL_SMAX;
			magn_point_save[i].y = MAGN_CAL_SMAX;
			magn_point_save[i].z = MAGN_CAL_SMAX;
	
			}


}



void magn_para_init(void)
{
	uint8_t i,j;

	g_CompassCaliStatus = 0;
	sample_acc.static_flag = 0;
	sample_acc.static_count = 0;
	for(i=0;i<MAGN_CAL_PARA_MIN;i++)
		{

		cal_magn_para[i].x = MAGN_CAL_SMAX;
		cal_magn_para[i].y = MAGN_CAL_SMAX;
		cal_magn_para[i].z = MAGN_CAL_SMAX;

		}
	for(i=0;i<MAGN_CAL_POINT_MIN;i++)
		{

		magn_point_save[i].x = MAGN_CAL_SMAX;
		magn_point_save[i].y = MAGN_CAL_SMAX;
		magn_point_save[i].z = MAGN_CAL_SMAX;

		}
	
read_magn_calibration_paremeter();
 
 if(magn_cal_para.flag== 137)
 	{
      for(i = 0;i<3;i++)
	 	{
		mag_output_data.hi_bias[i] = magn_cal_para.hi_bias[i];
	 	}
	 for(i = 0;i<3;i++)
	 	{
	    for(j = 0;j<3;j++)
	    	{
	    	
	    	 mag_output_data.sf_matrix[i][j] = magn_cal_para.sf_matrix[i][j];

		   }

	   }
    }
else
	{

	 for(i = 0;i<3;i++)
	 	{
		mag_output_data.hi_bias[i] = 0;
	 	}
	 for(i = 0;i<3;i++)
	 	{
	    for(j = 0;j<3;j++)
	    	{
	    	if(i==j)
	    		{
	             mag_output_data.sf_matrix[i][j] = 1;
	    		}
			else
				{

	              mag_output_data.sf_matrix[i][j] = 0;
			   }

		   }

	   }
   }



}
static void lis3mdl_reg_write(uint8_t reg,uint8_t value)
{
	am_hal_iom_buffer(1) sData;
	uint8_t ui8Offset;
	
	ui8Offset = reg;
	sData.bytes[0] = value;
	am_hal_iom_i2c_write(BSP_LIS3MDL_IOM,LIS3MDL_ADDR,sData.words,1,AM_HAL_IOM_OFFSET(ui8Offset));
}

void magn_dev_para_init(void)
{
	drv_lis3mdl_enable();
	lis3mdl_reg_write(0x21,0x40);//12GS
	lis3mdl_reg_write(0x20,0xFC);//80HZ
	lis3mdl_reg_write(0x23,0x0C);
	lis3mdl_reg_write(0x22,0x00);
	drv_lis3mdl_disable();
	magn_para_init();

}

uint8_t check_point(magn_cal_data*data_array)
{

 if((data_array->x >= MAGN_CAL_SMAX)||(data_array->y >= MAGN_CAL_SMAX)||(data_array->z >= MAGN_CAL_SMAX))
   {
     return 1;
   }
else
  return 0;
    

}
float cal_point_dis(magn_cal_data *point_base,magn_cal_data*point_cal)
{
	float r;

	r = (point_cal->x - point_base->x)*(point_cal->x - point_base->x)+(point_cal->y - point_base->y)*(point_cal->y - point_base->y)
	   +(point_cal->z - point_base->z)*(point_cal->z - point_base->z);
	r = sqrt(r);
	return r;
}
uint8_t get_four_point(magn_cal_data*point_data,uint8_t num,uint8_t flag,magn_cal_data four_data[])
{

uint8_t i,j,r_flag = 0,data_same_flag = 0;
magn_cal_data tempv = {0,0,0};
magn_cal_data cross_vector = {0,0,0};
uint16_t update_num = 0;
float dis_magn,dis_temp,data_radius = 0,dis_point[3] = {0};
uint8_t point_serial[5] = {0};
float value_a,value_ab;
/*0*/
 four_data[0] = point_data[flag];
/*1*/
 dis_magn = 0.0f;
 for(i=1;i<num/2;i++)
 	{
    dis_temp = cal_point_dis(&four_data[0],&point_data[i]);
	if(dis_magn<dis_temp)
		{
         dis_magn = dis_temp;
		 four_data[1] = point_data[i];
		  point_serial[0] = i; 
	    }

   }
 
 
 ALGO_IMU_LOG_PRINTF("0000001:x = %d,y = %d,z = %d;DIS = %d,dis = %d\r\n" ,(int)(four_data[1].x*10),(int)(four_data[1].y*10),(int)(four_data[1].z*10),(int)(MAGN_CAL_PARA_DIS*10),(int)(dis_magn*10));
 if(dis_magn<MAGN_CAL_PARA_DIS)
 	{
	
	ALGO_IMU_LOG_PRINTF("1111111:x = %d,y = %d,z = %d;DIS = %d,dis = %d\r\n" ,(int)(four_data[1].x*10),(int)(four_data[1].y*10),(int)(four_data[1].z*10),(int)(MAGN_CAL_PARA_DIS*10),(int)(dis_magn*10));
    return MAGN_CAL_DIS_SHORT;
  }

/*2*/
 dis_magn = 0.0f;
 

 magn_point_dis[0].x = four_data[1].x - four_data[0].x;
 magn_point_dis[0].y = four_data[1].y - four_data[0].y;
 magn_point_dis[0].z = four_data[1].z - four_data[0].z;

	for(i = 1;i < MAGN_CAL_POINT_MIN/2;i++)

 	{
	  magn_point_dis[i].x = point_data[i].x - four_data[0].x;
	  magn_point_dis[i].y = point_data[i].y - four_data[0].y;
	  magn_point_dis[i].z = point_data[i].z - four_data[0].z;
	  /*坐标*/
      tempv.x =  magn_point_dis[0].y* magn_point_dis[i].z - magn_point_dis[0].z*magn_point_dis[i].y;
	  
	  tempv.y =  magn_point_dis[0].z* magn_point_dis[i].x - magn_point_dis[0].x*magn_point_dis[i].z;

	  tempv.z =  magn_point_dis[0].x* magn_point_dis[i].y - magn_point_dis[0].y*magn_point_dis[i].x;
	
    
	   dis_temp = sqrt(tempv.x*tempv.x + tempv.y*tempv.y + tempv.z*tempv.z);
        
	 if(dis_magn < dis_temp)
	  	{
        dis_magn = dis_temp;
		four_data[2] = point_data[i];
		 point_serial[1] = i; 
		cross_vector = tempv;

	    }
	
 
    }
	
	ALGO_IMU_LOG_PRINTF("000002:x = %d,y = %d,z = %d;AREA = %d,area = %d\r\n" ,(int)(four_data[2].x*10),(int)(four_data[2].y*10),(int)(four_data[2].z*10),(int)(MAGN_CAL_PARA_AREA*10),(int)(dis_magn*10));
 if(dis_magn<MAGN_CAL_PARA_AREA)
	 {
     
	 ALGO_IMU_LOG_PRINTF("2222222:x = %d,y = %d,z = %d;AREA = %d,area= %d\r\n" ,(int)(four_data[2].x*10),(int)(four_data[2].y*10),(int)(four_data[2].z*10),(int)(MAGN_CAL_PARA_AREA*10),(int)(dis_magn*10));
	 return MAGN_CAL_DIS_SHORT;
   }


/*3*/
dis_magn = 0.0f;

	for (i = 1; i < MAGN_CAL_POINT_MIN/2; i++) 

	{
	  dis_temp = magn_point_dis[i].x * cross_vector.x + magn_point_dis[i].y * cross_vector.y + magn_point_dis[i].z * cross_vector.z;
		dis_temp = fabs(dis_temp);
		if (dis_magn < dis_temp) 
		{
			dis_magn = dis_temp;
			four_data[3] = point_data[i];
			point_serial[2] = i; 
		}
	}
	
	ALGO_IMU_LOG_PRINTF("000003:x = %d,y = %d,z = %d;VOL = %d,vol = %d\r\n" ,(int)(four_data[3].x*10),(int)(four_data[3].y*10),(int)(four_data[3].z*10),(int)(MAGN_CAL_PARA_VOL*10),(int)(dis_magn*10));
if(dis_magn<MAGN_CAL_PARA_VOL)
	 {
	 
	 ALGO_IMU_LOG_PRINTF("3333333:x = %d,y = %d,z = %d;VOL = %d,vol= %d\r\n" ,(int)(four_data[3].x*10),(int)(four_data[3].y*10),(int)(four_data[3].z*10),(int)(MAGN_CAL_PARA_VOL*10),(int)(dis_magn*10));
	 return MAGN_CAL_DIS_SHORT;
   }


/*4*/
 dis_magn = 0.0f;
for(i=1;i<num/2;i++)
 	{
 	
    dis_temp = cal_point_dis(&four_data[3],&point_data[i]);
	if((i ==point_serial[0])||(i ==point_serial[1])||(i ==point_serial[2]))
		{

		dis_temp = 0;
		}
	if(dis_magn<dis_temp)
		{
         dis_magn = dis_temp;
		 four_data[4] = point_data[i];
		 point_serial[3] = i;
	    }

   }
 
 ALGO_IMU_LOG_PRINTF("0000004:x = %d,y = %d,z = %d;DIS_AGAIN = %d,dis_again = %d\r\n" ,(int)(four_data[4].x*10),(int)(four_data[4].y*10),(int)(four_data[4].z*10),(int)((MAGN_CAL_PARA_DIS-0.3f)*10),(int)(dis_magn*10));
 
 if(dis_magn<(MAGN_CAL_PARA_DIS-0.3f))
 	{
	
	ALGO_IMU_LOG_PRINTF("4444444:x = %d,y = %d,z = %d;DIS_AGAIN = %d,dis_again= %d\r\n" ,(int)(four_data[4].x*10),(int)(four_data[4].y*10),(int)(four_data[4].z*10),(int)((MAGN_CAL_PARA_DIS-0.3f)*10),(int)(dis_magn*10));
    return MAGN_CAL_DIS_SHORT;
  }

/*5*/
 dis_magn = 0.0f;
 

 magn_point_dis[0].x = four_data[4].x - four_data[3].x;
 magn_point_dis[0].y = four_data[4].y - four_data[3].y;
 magn_point_dis[0].z = four_data[4].z - four_data[3].z;

for(i = 1;i < MAGN_CAL_POINT_MIN/2;i++)

 	{
	  magn_point_dis[i].x = point_data[i].x - four_data[3].x;
	  magn_point_dis[i].y = point_data[i].y - four_data[3].y;
	  magn_point_dis[i].z = point_data[i].z - four_data[3].z;
	  /*坐标*/
      tempv.x =  magn_point_dis[0].y* magn_point_dis[i].z - magn_point_dis[0].z*magn_point_dis[i].y;
	  
	  tempv.y =  magn_point_dis[0].z* magn_point_dis[i].x - magn_point_dis[0].x*magn_point_dis[i].z;

	  tempv.z =  magn_point_dis[0].x* magn_point_dis[i].y - magn_point_dis[0].y*magn_point_dis[i].x;
	
    
	   dis_temp = sqrt(tempv.x*tempv.x + tempv.y*tempv.y + tempv.z*tempv.z);
        if((i ==point_serial[0])||(i ==point_serial[1])||(i ==point_serial[2])||(i ==point_serial[3]))
		{

		dis_temp = 0;
		}
	 if(dis_magn < dis_temp)
	  	{
        dis_magn = dis_temp;
		four_data[5] = point_data[i];
		cross_vector = tempv;
		point_serial[4] = i; 

	    }
	
 
    }
 ALGO_IMU_LOG_PRINTF("0000005:x = %d,y = %d,z = %d;AREA_AGAIN= %d,area_again= %d\r\n" ,(int)(four_data[5].x*10),(int)(four_data[5].y*10),(int)(four_data[5].z*10),(int)((MAGN_CAL_PARA_AREA-0.5f)*10),(int)(dis_magn*10));

 if(dis_magn<(MAGN_CAL_PARA_AREA-0.5f))
	 {
	 
	 ALGO_IMU_LOG_PRINTF("5555555:x = %d,y = %d,z = %d;AREA_AGAIN= %d,area_again= %d\r\n" ,(int)(four_data[5].x*10),(int)(four_data[5].y*10),(int)(four_data[5].z*10),(int)((MAGN_CAL_PARA_AREA-0.5f)*10),(int)(dis_magn*10));
	 return MAGN_CAL_DIS_SHORT;
   }

/*6*/
dis_magn = 0.0f;

//for (i = 1; i < MAGN_CAL_POINT_MIN; i++) 
	for (i = 1; i < MAGN_CAL_POINT_MIN/2; i++) 

	{
	 dis_temp = magn_point_dis[i].x * cross_vector.x + magn_point_dis[i].y * cross_vector.y + magn_point_dis[i].z * cross_vector.z;
	if((i ==point_serial[0])||(i ==point_serial[1])||(i ==point_serial[2])||(i ==point_serial[3])||(i ==point_serial[4]))
		{

		dis_temp = 0;
		}	
		dis_temp = fabs(dis_temp);
		if (dis_magn < dis_temp) 
		{
			dis_magn = dis_temp;
			four_data[6] = point_data[i];
			
		}
	}
	
	ALGO_IMU_LOG_PRINTF("0000006:x = %d,y = %d,z = %d;VOL_AGAIN= %d,vol_again= %d\r\n" ,(int)(four_data[6].x*10),(int)(four_data[6].y*10),(int)(four_data[6].z*10),(int)((MAGN_CAL_PARA_VOL-0.5f)*10),(int)(dis_magn*10));
if(dis_magn<(MAGN_CAL_PARA_VOL-0.7f))
	 {
	 
	 ALGO_IMU_LOG_PRINTF("666666:x = %d,y = %d,z = %d;VOL_AGAIN= %d,vol_again= %d\r\n" ,(int)(four_data[6].x*10),(int)(four_data[6].y*10),(int)(four_data[6].z*10),(int)((MAGN_CAL_PARA_VOL-0.5f)*10),(int)(dis_magn*10));
	 return MAGN_CAL_DIS_SHORT;
   }


/*update data*/
for(i = MAGN_CAL_POINT_MIN-1;i>=(MAGN_CAL_POINT_MIN/2);i--)

{
	update_num++;

 magn_point_save[i].x= MAGN_CAL_SMAX;
 magn_point_save[i].y= MAGN_CAL_SMAX;
 magn_point_save[i].z= MAGN_CAL_SMAX;

}
/*for(i = 0;i < 7;i++)
 {

 data_radius = sqrt(four_data[i].x*four_data[i].x + four_data[i].y*four_data[i].y + four_data[i].z*four_data[i].z);
	if(data_radius < MAGN_CAL_ANGLE_RADIUS)
	{ 
    
	ALGO_IMU_LOG_PRINTF("TTTTTTT:x = %d,y = %d,z = %d;Data_radius = %d,data_radius= %d\r\n" ,(int)(four_data[6].x*10),(int)(four_data[6].y*10),(int)(four_data[6].z*10),(int)(MAGN_CAL_ANGLE_RADIUS*10),(int)(data_radius*10));
	   return MAGN_CAL_RADIUS_SAMLL;//直接返回
	}

 }*/

data_same_flag = judge_value(four_data,7);
/*if(data_same_flag==0)
  {
  
  ALGO_IMU_LOG_PRINTF("data same error\r\n");
  return MAGN_CAL_DATA_CON;
  
  }*/
dis_magn = 0.0f;

for(i = 0;i < 7;i++)
 {
	for(j = 0;j < magn_caling_state[1];j++)
	{
	 dis_point[0] = fabs(four_data[i].x-cal_magn_para[j].x);
	 dis_point[1] = fabs(four_data[i].y-cal_magn_para[j].y);
	 dis_point[2] = fabs(four_data[i].z-cal_magn_para[j].z);
	

	 
	 if((dis_point[0]<0.1f)&&(dis_point[1]<0.1f)&&(dis_point[2]<0.1f))
	 	{
	 	ALGO_IMU_LOG_PRINTF("error and dis_data too small\r\n");
		  return MAGN_CAL_RADIUS_SAMLL;
	     }
	/*dis_magn =sqrt((four_data[i].x-cal_magn_para[j].x)*(four_data[i].x-cal_magn_para[j].x) + (four_data[i].y-cal_magn_para[j].y)*(four_data[i].y-cal_magn_para[j].y)
	 	+ (four_data[i].z-cal_magn_para[j].z)*(four_data[i].z-cal_magn_para[j].z));
	
	ALGO_IMU_LOG_PRINTF("000007:POINT_DIS = %d,dis_magn= %d\r\n" ,(int)(MAGN_CAL_POINT_DIS*10),(int)(dis_magn*10));
       if(dis_magn<MAGN_CAL_POINT_DIS)
	 	{
		   ALGO_IMU_LOG_PRINTF("777777:POINT_DIS = %d,dis_magn= %d\r\n" ,(int)(MAGN_CAL_POINT_DIS*10),(int)(dis_magn*10));
		  return MAGN_CAL_RADIUS_SAMLL;
	     }*/
	 }
}


return MAGN_CAL_DIS_ENOUGH;


}
uint8_t judge_value(magn_cal_data *data,uint8_t num)
{

uint8_t i,j,num_c = 0,num_l = 0,num_d= 0;
for(i=0;i<num;i++)
{
  if((data[i].x)>0)
  	{
    num_c++;

  }
  else
  	{
    num_l++;

     }

}
if((num_c==num)||(num_l==num))
  {
  
  return MAGN_CAL_DATA_CON;
  
  }
else
 {
 
 ALGO_IMU_LOG_PRINTF("XXXXXX:num_c = %d,num_l = %d\r\n",num_c,num_l);
  num_c = 0;
  num_l = 0;
 }

for(i=0;i<num;i++)
{
  if((data[i].y)>0)
  	{
    num_c++;

  }
  else
  	{
    num_l++;

     }



}
if((num_c==num)||(num_l==num))
  {
  
  return MAGN_CAL_DATA_CON;
  
  }
 {
 	
	ALGO_IMU_LOG_PRINTF("YYYYYY:num_c = %d,num_l = %d\r\n",num_c,num_l);
  num_c = 0;
  num_l = 0;
 }

for(i=0;i<num;i++)
{
  if((data[i].z)>0)
  	{
    num_c++;

  }
  else
  	{
    num_l++;

     }



}
if((num_c==num)||(num_l==num))
  {
  
  return MAGN_CAL_DATA_CON;
  
  }
{
	
	ALGO_IMU_LOG_PRINTF("ZZZZZZZ:num_c = %d,num_l = %d\r\n",num_c,num_l);
  num_c = 0;
  num_l = 0;
 }


    return MAGN_CAL_DATA_FINE;

}
uint8_t magn_cal_auto(float*magn_data,magn_matrix_parameter*para)
{
  uint8_t i,temp_point = 0;
 
  //float temp_angle = 0.0f;
  
  float dis_temp = 0,dis_AB = 0;
  magn_cal_data get_four_data[7];
  magn_cal_data tem_data,tempdis_v;
  for(i = MAGN_CAL_POINT_MIN-1;i>0;i--)
  	{
     magn_point_save[i] = magn_point_save[i-1];
	}

    magn_point_save[0].x = magn_data[0];
	magn_point_save[0].y = magn_data[1];
	magn_point_save[0].z = magn_data[2];
	
	
 

 if(check_point(magn_point_save+MAGN_CAL_POINT_MIN-1)!=0)

  {

  para->flag = 0;
  return MAGN_CAL_POINT_LACK;
  }
 else
 	{
	if(magn_caling_state[0] == 0)
	 {
	  if(get_four_point(magn_point_save,MAGN_CAL_POINT_MIN,0,get_four_data) == 1)
	  	{
          
		  for(i=0;i<7;i++)
		  	{
	         cal_magn_para[magn_caling_state[1]+i] = get_four_data[i];
	 
		    }
		  magn_caling_state[0] = 1;
		  magn_caling_state[1] += 7;
		  magn_point_zero = magn_point_save[0];

	    }
	  else;
	  
	
	 }
	else
		
	 {
	          /*A:new;B:old*/
		  for( i = 0;i < (MAGN_CAL_POINT_MIN/2);i++)
		
		   {
		      
		     
			   tem_data.x =  magn_point_save[i].x - magn_point_zero.x;
	           tem_data.y =  magn_point_save[i].y - magn_point_zero.y;
		       tem_data.z =  magn_point_save[i].z - magn_point_zero.z;
		     
		   
			   magn_point_dis[0].x = cal_magn_para[magn_caling_state[1]-4].x - cal_magn_para[magn_caling_state[1]-7].x;
			   magn_point_dis[0].y = cal_magn_para[magn_caling_state[1]-4].y - cal_magn_para[magn_caling_state[1]-7].y;
			   magn_point_dis[0].z = cal_magn_para[magn_caling_state[1]-4].z - cal_magn_para[magn_caling_state[1]-7].z;

				
			
		  tempdis_v.x = tem_data.y*magn_point_dis[0].z - tem_data.z*magn_point_dis[0].y;
		   
		  tempdis_v.y = tem_data.z*magn_point_dis[0].x - tem_data.x*magn_point_dis[0].z;
	  
		   tempdis_v.z = tem_data.x*magn_point_dis[0].y - tem_data.y*magn_point_dis[0].x;
		   
		   dis_AB = sqrt(tempdis_v.x*tempdis_v.x + tempdis_v.y*tempdis_v.y + tempdis_v.z*tempdis_v.z);
		
		   		
         if(dis_AB>dis_temp)
           	{
           	  dis_temp = dis_AB;
         
              temp_point = i;
			
		    }
		  
		   else;
		    
	      
		}
		  ALGO_IMU_LOG_PRINTF("0000007:x = %d,y = %d,z = %d;INT_AREA = %d,area = %d\r\n" ,(int)(magn_point_save[temp_point].x*10),(int)(magn_point_save[temp_point].y*10),(int)(magn_point_save[temp_point].z*10),(int)(MAGN_CAL_PARA_INTERVAL_AREA*10),(int)(dis_temp*10));
		  if(dis_temp < MAGN_CAL_PARA_INTERVAL_AREA)
		 	{
        
		ALGO_IMU_LOG_PRINTF("777777:x = %d,y = %d,z = %d;INT_AREA = %d,area = %d\r\n" ,(int)(magn_point_save[temp_point].x*10),(int)(magn_point_save[temp_point].y*10),(int)(magn_point_save[temp_point].z*10),(int)(MAGN_CAL_PARA_INTERVAL_AREA*10),(int)(dis_temp*10));
		 	   magn_caling_state[2]++;
				if((magn_caling_state[2]>=5)&&(dis_temp> MAGN_CAL_PARA_AREA))
				 {
                    magn_caling_state[2] = 0;
				  }
				else 
					{
                     return MAGN_CAL_ANGLE_SMALL;
					}
		    }
		
			  magn_caling_state[2] = 0;
		
              if( get_four_point(magn_point_save,MAGN_CAL_POINT_MIN,temp_point,get_four_data)==1)
				{
				
				for(i=0;i<7;i++)
	  	          {
                  cal_magn_para[magn_caling_state[1]+i] = get_four_data[i];
  
	                }
	                magn_caling_state[1] += 7;
				    magn_point_zero = magn_point_save[temp_point];
               	}
			   else;
			 
		    
			ALGO_IMU_LOG_PRINTF("cal_num_point = %d\r\n",magn_caling_state[1]);
		 	
	 }
	
	if(magn_caling_state[1]== MAGN_CAL_PARA_MIN)
		{
		    para->flag = 1;
          return MAGN_CAL_PARA_ENOUGH;
	     }
          if(magn_caling_state[1]== 7)
          	{
            
			ALGO_IMU_LOG_PRINTF("cal_num_point = %d\r\n",magn_caling_state[1]);
		    }
			
			   
	return MAGN_CAL_POINT_ENOUGH;
 	}
}

/*****************矩阵原地转置BEGIN********************/

/* 后继 */
int get_next(int i, int m, int n)
{
	  return (i%n)*m + i / n;
	}

/* 前驱 */
int get_pre(int i, int m, int n)
{
	  return (i%m)*n + i / m;
	}

/* 处理以下标i为起点的环 */
void move_data(double *mtx, int i, int m, int n)
{
	  double temp = mtx[i]; // 暂存
	  int cur = i;	  // 当前下标
	  int pre = get_pre(cur, m, n);
	  while (pre != i)
		  {
			mtx[cur] = mtx[pre];
			cur = pre;
			pre = get_pre(cur, m, n);
		  }
	  mtx[cur] = temp;
	}

/* 转置，即循环处理所有环 */
 void transpose_m(double *mtx, int m, int n)
 {
	   for (int i = 0; i<m*n; ++i)
		   {
		     int next = get_next(i, m, n);
		     while (next > i) // 若存在后继小于i说明重复,就不进行下去了（只有不重复时进入while循环）
			       next = get_next(next, m, n);
		     if (next == i)  // 处理当前环
			       move_data(mtx, i, m, n);
		   }
	 }

 void magn_solve(double*L, double*U, int*P, double*b,double*x,uint8_t order)
 {
	   uint8_t MATRIX_ORDER_MAGN;
	   MATRIX_ORDER_MAGN = order;
	     double *y =(double*)pvPortMalloc(MATRIX_ORDER_MAGN*sizeof(double));
	
		     //正向替换
		     for (int i = 0; i < MATRIX_ORDER_MAGN; i++)
		     {
		        y[i] = b[P[i]];
	         for (int j = 0; j < i; j++)
			        {
			             y[i] = y[i] - L[i*MATRIX_ORDER_MAGN + j] * y[j];
			         }
		     }
	     //反向替换
		     for (int i = MATRIX_ORDER_MAGN - 1; i >= 0; i--)
		     {
		         x[i] = y[i];
		         for (int j = MATRIX_ORDER_MAGN - 1; j > i; j--)
			         {
			             x[i] = x[i] - U[i*MATRIX_ORDER_MAGN + j] * x[j];
			         }
		         x[i] /= U[i*MATRIX_ORDER_MAGN + i];
		     }
	        vPortFree(y);
			 y = NULL;
	 }

void magn_descomposition(double *A, double *L, double *U, int *P,uint8_t order)
 {        
        uint8_t MATRIX_ORDER_MAGN;
	 MATRIX_ORDER_MAGN = order;
	     int row = 0;
	     for (int i = 0; i<MATRIX_ORDER_MAGN; i++)
		     {
		         P[i] = i;
		     }
	     for (int i = 0; i<MATRIX_ORDER_MAGN - 1; i++)
		     {
				 double p = 0.0;
		         for (int j = i; j<MATRIX_ORDER_MAGN; j++)
			         {
			             if (fabs(A[j*MATRIX_ORDER_MAGN + i])>p)
				             {
			                 p = fabs(A[j*MATRIX_ORDER_MAGN + i]);
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
		        for (int j = 0; j<MATRIX_ORDER_MAGN; j++)
			        {
			             //交换A[i][j]和 A[row][j]
				             tmp2 = A[i*MATRIX_ORDER_MAGN + j];
			            A[i*MATRIX_ORDER_MAGN + j] = A[row*MATRIX_ORDER_MAGN + j];
			            A[row*MATRIX_ORDER_MAGN + j] = tmp2;
			         }
		
			        //以下同LU分解
			         double u = A[i*MATRIX_ORDER_MAGN + i], l = 0.0;
		         for (int j = i + 1; j<MATRIX_ORDER_MAGN; j++)
		         {
			             l = A[j*MATRIX_ORDER_MAGN + i] / u;
			            A[j*MATRIX_ORDER_MAGN + i] = l;
			             for (int k = i + 1; k<MATRIX_ORDER_MAGN; k++)
			            {
				            A[j*MATRIX_ORDER_MAGN + k] = A[j*MATRIX_ORDER_MAGN + k] - A[i*MATRIX_ORDER_MAGN + k] * l;
				        }
			     }
		
			 }
	
		     //构造L和U
		     for (int i = 0; i<MATRIX_ORDER_MAGN; i++)
		     {
		         for (int j = 0; j <= i; j++)
			         {
			             if (i != j)
				            {
				                L[i*MATRIX_ORDER_MAGN+ j] = A[i*MATRIX_ORDER_MAGN + j];
				            }
			             else
				             {
			                 L[i*MATRIX_ORDER_MAGN + j] = 1;
				             }
			         }
	            for (int k = i; k<MATRIX_ORDER_MAGN; k++)
			        {
			             U[i*MATRIX_ORDER_MAGN + k] = A[i*MATRIX_ORDER_MAGN + k];
			         }
		     }
	
		 }



 void magn_solve_inverse(double *A,double *mtx,uint8_t order_num)

 {
	 uint8_t MATRIX_ORDER_MAGN;
	 MATRIX_ORDER_MAGN = order_num;
	 int i,j,k;
	 double *A_mirror = (double *)pvPortMalloc(MATRIX_ORDER_MAGN*MATRIX_ORDER_MAGN*sizeof(double));

	// double *inv_A = (double *)malloc(N*N*sizeof(double));;//最终的逆矩阵（还需要转置）

	 double *inv_A_each = (double *)pvPortMalloc(MATRIX_ORDER_MAGN*sizeof(double));//矩阵逆的各列
	 double *b = (double *)pvPortMalloc(MATRIX_ORDER_MAGN*sizeof(double));//b阵为B阵的列矩阵分量

		     for ( i = 0; i<MATRIX_ORDER_MAGN; i++)
	     {
	    
			 double *L = (double*)pvPortMalloc(MATRIX_ORDER_MAGN*MATRIX_ORDER_MAGN*sizeof(double));
			 double *U = (double*)pvPortMalloc(MATRIX_ORDER_MAGN*MATRIX_ORDER_MAGN*sizeof(double));
			 int *P = (int *)pvPortMalloc(MATRIX_ORDER_MAGN*sizeof(int));
		
			         //构造单位阵的每一列
			         for (j= 0; j<MATRIX_ORDER_MAGN; j++)
			         {
			             b[j] = 0;
			         }
		         b[i] = 1;
		
		         //每次都需要重新将A复制一份
			        for ( k = 0; k<MATRIX_ORDER_MAGN*MATRIX_ORDER_MAGN; k++)
			        {
			             A_mirror[k] = A[k];
			         }
		
		         magn_descomposition(A_mirror, L, U, P,order_num);
	
		        magn_solve(L, U, P, b,inv_A_each,order_num);
		         memcpy(mtx + i*MATRIX_ORDER_MAGN, inv_A_each, MATRIX_ORDER_MAGN*sizeof(double));//将各列拼接起来
				 vPortFree(L);
				 vPortFree(U);
				 vPortFree(P);
				 L = NULL;
				 U = NULL;
				 P = NULL;
		     }
	    transpose_m(mtx, MATRIX_ORDER_MAGN,MATRIX_ORDER_MAGN);//由于现在根据每列b算出的x按行存储，因此需转置
		
		vPortFree(A_mirror);
		
		vPortFree(inv_A_each);
		vPortFree(b);
		A_mirror = NULL;
		
		inv_A_each = NULL;
		b = NULL;
		//  ALGO_IMU_LOG_PRINTF("size_2= %d\r\n" ,xPortGetFreeHeapSize());
			  	
			//  uxTaskGetStackHighWaterMark(TaskToolHandle);
		     // ALGO_IMU_LOG_PRINTF("TaskTool_2= %d\r\n" , uxTaskGetStackHighWaterMark(TaskToolHandle));
	     return;
	 }

void evector_judge(matrix_data *A_raw, matrix_data *eigenVector, matrix_data *eigenValue,uint8_t flag)
	{
		int num = 3, temp_count = 0,i;
		double eValue;
		matrix_data A;
		//InitMatrix(&A, A_raw->row, A_raw->column);
		A.data = (double*)pvPortMalloc(3*3*sizeof(double));//给矩阵分配空间
		A.column = 3;
		A.row = 3;
		
		if (flag == 0)
		{
			for (i = 0; i < 9;i++)
			{
				A.data[i] = A_raw->data[i];
			}
			for (int count = 0; count < num; count++)
			{
				if (eigenValue->data[count] > 0)
				{
					eValue = eigenValue->data[count];//当前的特征值
					temp_count = count;
				}
			}
			for (int i = 0; i < A.row; i++)
			{
				A.data[i * A.column + i] -= eValue;
			}
			//将temp化为阶梯型矩阵(归一性)对角线值为1
			for (int i = 0; i < A.row - 1; i++)
			{
				double coe = A.data[i * A.column + i];
				/*每行元素除于每行首元素*/
				for (int j = i; j < A.column; j++)
				{
					A.data[i * A.column + j] /= coe;
				}
				for (int i1 = i + 1; i1 < A.row; i1++)
				{
					coe = A.data[i1 * A.column + i];
					for (int j1 = i; j1 < A.column; j1++)
					{
						A.data[i1 * A.column + j1] -= coe * A.data[i * A.column + j1];
					}
				}
			}
			//让最后一行为1
			double sum1 = eigenVector->data[(eigenVector->row - 1) * eigenVector->column + temp_count] = 1;
			for (int i2 = A.row - 2; i2 >= 0; i2--)
			{
				double sum2 = 0;
				for (int j2 = i2 + 1; j2 < A.column; j2++)
				{
					sum2 += A.data[i2 * A.column + j2] * eigenVector->data[j2 * eigenVector->column + temp_count];
				}
				sum2 = -sum2 / A.data[i2 * A.column + i2];
				sum1 += sum2 * sum2;
				eigenVector->data[i2 * eigenVector->column + temp_count] = sum2;
			}
			sum1 = sqrt(sum1);//当前列的模
			for (int i = 0; i < eigenVector->row; i++)
			{
				//单位化
				eigenVector->data[i * eigenVector->column + temp_count] /= sum1;
			}
	
		}
		else if (flag == 1)
		{
			for (int count = 0; count < num; count++)
			{
				for (i = 0; i < 9; i++)
				{
					A.data[i] = A_raw->data[i];
				}
				
	
					eValue = eigenValue->data[count];//当前的特征值
	
					temp_count = count;
				
	
				for (int i = 0; i < A.row; i++)
				{
					A.data[i * A.column + i] -= eValue;
				}
				//将temp化为阶梯型矩阵(归一性)对角线值为1
				for (int i = 0; i < A.row - 1; i++)
				{
					double coe = A.data[i * A.column + i];
					/*每行元素除于每行首元素*/
					for (int j = i; j < A.column; j++)
					{
						A.data[i * A.column + j] /= coe;
					}
					for (int i1 = i + 1; i1 < A.row; i1++)
					{
						coe = A.data[i1 * A.column + i];
						for (int j1 = i; j1 < A.column; j1++)
						{
							A.data[i1 * A.column + j1] -= coe * A.data[i * A.column + j1];
						}
					}
				}
				//让最后一行为1
				double sum1 = eigenVector->data[(eigenVector->row - 1) * eigenVector->column + temp_count] = 1;
				for (int i2 = A.row - 2; i2 >= 0; i2--)
				{
					double sum2 = 0;
					for (int j2 = i2 + 1; j2 < A.column; j2++)
					{
						sum2 += A.data[i2 * A.column + j2] * eigenVector->data[j2 * eigenVector->column + temp_count];
					}
					sum2 = -sum2 / A.data[i2 * A.column + i2];
					sum1 += sum2 * sum2;
					eigenVector->data[i2 * eigenVector->column + temp_count] = sum2;
				}
				sum1 = sqrt(sum1);//当前列的模
				for (int i = 0; i < eigenVector->row; i++)
				{
					//单位化
					eigenVector->data[i * eigenVector->column + temp_count] /= sum1;
				}
				//}
				//free(&temp);
			}
		}
		vPortFree(A.data);
		//return temp_count;
	}




void evalue_judge(double*para_poly, double*para_x)
	{
		double A, B, C;
		double delt;//判别式
	//	double X1, X2, X3;
		A = pow(para_poly[1], 2) - 3 * para_poly[0] * para_poly[2];
		B = para_poly[1] * para_poly[2] - 9 * para_poly[0] * para_poly[3];
		C = pow(para_poly[2], 2) - 3 * para_poly[1] * para_poly[3];
		delt = pow(B, 2) - 4 * A*C;
	
		
		if ((A == 0 && B == 0) || (delt == 0 && A == 0))
		{
			para_x[0] = -para_poly[1] / (3 * para_poly[0]);
			para_x[1] = para_x[0];
			para_x[2] = para_x[0];
		}
	
		
		if (delt>0)
		{
			double Y1 = A*para_poly[1] + 3 * para_poly[0] * ((-B + pow(delt, 0.5)) / 2);
			double Y2 = A*para_poly[1] + 3 * para_poly[0] * ((-B - pow(delt, 0.5)) / 2);
			//解决负数开三方出问题
			double Y1_three, Y2_three;
			if (Y1<0)
				Y1_three = -pow(-Y1, 1.0 / 3.0);
			else
				Y1_three = pow(Y1, 1.0 / 3.0);
			if (Y2<0)
				Y2_three = -pow(-Y2, 1.0 / 3.0);
			else
				Y2_three = pow(Y2, 1.0 / 3.0);
			para_x[0] = (-para_poly[1] - (Y1_three + Y2_three)) / (3 * para_poly[0]);
			//X1,X2为复数，这里不需要，未实现。
		}
	
		
		if (delt == 0 && (A != 0))
		{
			double K = B / A;
			para_x[0] = -para_poly[1] / para_poly[0] + K;
			para_x[1] = -K / 2.0;
			para_x[2] = para_x[1];
		}
	
		
		if (delt<0)
		{
			double T = (2 * A*para_poly[1] - 3 * para_poly[0] * B) / (2 * A*pow(A, 0.5));//（A>0，－1<T<1）
			double theita = acos(T);
			para_x[0] = (-para_poly[1] - 2 * pow(A, 0.5)*cos(theita / 3.0)) / (3 * para_poly[0]);
			para_x[1] = (-para_poly[1] + pow(A, 0.5)*(cos(theita / 3.0) + pow(3, 0.5)*sin(theita / 3.0))) / (3 * para_poly[0]);
			para_x[2] = (-para_poly[1] + pow(A, 0.5)*(cos(theita / 3.0) - pow(3, 0.5)*sin(theita / 3.0))) / (3 * para_poly[0]);
		}
	
	}

uint8_t fit_ellipsoid(magn_cal_data*cal_ellipsold,uint8_t cal_num)
{
  uint8_t i,j,k;
	
  double para_m[4];
  double para_x[3] = {0};
  matrix_data matrix_raw_data, e_vector,e_value;
  int e_value_sequence = 0;
 // float *D1_mirror = (float *)pvPortMalloc(MAGN_CAL_PARA_MIN*3*sizeof(double));
 
  double evalue[3] = {0};
  double para_al[3] = {0};
  double para_a[10] = {0};
  double para_e[3][3] = {{0},{0},{0}};
  double para_inve[3][3] = {{0},{0},{0}};
  double para_f[3] = {0};
  double para_w[3] = {0};
  double para_k[3][3] = {{0},{0},{0}};
  double para_g;
  double para_temp1[3] = {0};
  double para_temp_var = 0;
  double H2;
  double para_R[3][3]= {{0},{0},{0}};
  double para_temp2[3][3] = {{0},{0},{0}};
  double para_N[3][3] = {{0},{0},{0}};
  double (*S1_mirror)[3] = (double(*)[3])pvPortMalloc(sizeof(double)*3*3); 
  double (*S2_mirror)[7] = (double(*)[7])pvPortMalloc(sizeof(double)*3*7); 
  double (*S3_mirror)[7] = (double(*)[7])pvPortMalloc(sizeof(double)*7*7); 
  double (*T_mirror)[3] = (double(*)[3])pvPortMalloc(sizeof(double)*7*3); 
  double (*M_mirror)[3] = (double(*)[3])pvPortMalloc(sizeof(double)*3*3); 
  double (*matrix_raw)[3] = (double(*)[3])pvPortMalloc(sizeof(double)*3*3); 
 
  for(i=0;i<cal_num;i++)
	{
        magn_data_com[i][0] = cal_ellipsold[i].x*cal_ellipsold[i].x;
		magn_data_com[i][1] = cal_ellipsold[i].y*cal_ellipsold[i].y;
		magn_data_com[i][2] = cal_ellipsold[i].z*cal_ellipsold[i].z;
		magn_data_com[i][3] = 2*(cal_ellipsold[i].x*cal_ellipsold[i].y);
		magn_data_com[i][4] = 2*(cal_ellipsold[i].x*cal_ellipsold[i].z);
		magn_data_com[i][5] = 2*(cal_ellipsold[i].y*cal_ellipsold[i].z);
		magn_data_com[i][6] = cal_ellipsold[i].x*2;
		magn_data_com[i][7] = cal_ellipsold[i].y*2;
		magn_data_com[i][8] = cal_ellipsold[i].z*2;
		magn_data_com[i][9] = 1.0f;
	

	}
   
  for(i=0;i<3;i++)
	 {
	  for(j=0;j<3;j++)
		 {
		  S1_mirror[i][j] = 0;
		  for(k=0;k<cal_num;k++)
			 {
			  S1_mirror[i][j] += magn_data_com[k][i]*magn_data_com[k][j];
			 }
		 } 
 
	 }

   

  for(i=0;i<3;i++)
	  {
	   for(j=0;j<7;j++)
		  {
		   S2_mirror[i][j] = 0;
		   for(k=0;k<cal_num;k++)
			  {
			   S2_mirror[i][j] += magn_data_com[k][i]*magn_data_com[k][j+3];
			  }
		  } 
  
	  }

  for(i=0;i<7;i++)
	  {
	   for(j=0;j<7;j++)
		  {
		   S3_mirror[i][j] = 0;
		   for(k=0;k<cal_num;k++)
			  {
			   S3_mirror[i][j] += magn_data_com[k][i+3]*magn_data_com[k][j+3];
			  }
		  } 
  
	  }



 magn_solve_inverse(&S3_mirror[0][0],&magn_s_inverse[0][0],7);

 for(i=0;i<7;i++)
 	{
     for(j=0;j<3;j++)
     	{
         T_mirror[i][j] = 0;
		 for(k=0;k<7;k++)
		 	{
               T_mirror[i][j] += magn_s_inverse[i][k]*S2_mirror[j][k];

		    }
       
           T_mirror[i][j] = -T_mirror[i][j];

	   }
	 
    }

 for(i=0;i<3;i++)
 	{
     for(j=0;j<3;j++)
     	{
         M_mirror[i][j] = 0;
		 for(k=0;k<7;k++)
		 	{
               M_mirror[i][j] += S2_mirror[i][k]*T_mirror[k][j];

		    }
       
         

	   }
	 
    }

for(i = 0;i < 3;i++)
 {
  for(j=0;j<3;j++)
 	{
     M_mirror[i][j] =  M_mirror[i][j] + S1_mirror[i][j];
    }
 }
  
 /*特征矩阵*/
for(i=0;i<3;i++)
  {
 
	  matrix_raw[0][i] = M_mirror[2][i]/2.0;
   }
for(i=0;i<3;i++)
  {
 
	  matrix_raw[1][i] = -M_mirror[1][i];
   }

for(i=0;i<3;i++)
  {
 
	  matrix_raw[2][i] = M_mirror[0][i]/2.0;
   }

para_m[0] = -1;
para_m[1] = matrix_raw[0][0] + matrix_raw[1][1] + matrix_raw[2][2];
para_m[2] = matrix_raw[0][1] * matrix_raw[1][0] + matrix_raw[1][2] * matrix_raw[2][1] - matrix_raw[0][0] * matrix_raw[1][1]
          + matrix_raw[0][2] * matrix_raw[2][0] - matrix_raw[2][2] * matrix_raw[0][0] - matrix_raw[1][1] * matrix_raw[2][2];
para_m[3] = matrix_raw[0][0] * matrix_raw[1][1] * matrix_raw[2][2] + matrix_raw[0][1] * matrix_raw[1][2] * matrix_raw[2][0]
          + matrix_raw[0][2] * matrix_raw[1][0] * matrix_raw[2][1] - matrix_raw[0][2] * matrix_raw[1][1] * matrix_raw[2][0]
          - matrix_raw[0][0] * matrix_raw[1][2] * matrix_raw[2][1] - matrix_raw[0][1] * matrix_raw[1][0] * matrix_raw[2][2];

evalue_judge(para_m, para_x);//求特征值

/*矩阵元素*/
matrix_raw_data.row = 3;
matrix_raw_data.column = 3;
matrix_raw_data.data = &matrix_raw[0][0];

/*特征向量*/
e_vector.row = 3;
e_vector.column = 3;
e_vector.data = (double *)pvPortMalloc(3 * 3 * sizeof(double));

/*特征值*/
e_value.row = 3;
e_value.column = 3;
e_value.data = para_x;
/*增加各个特征值对应的特征向量的计算*/
 evector_judge(&matrix_raw_data, &e_vector, &e_value,1);



for(i = 0;i < 3;i++)
{
    evalue[i] = 0;
	evalue[i] = 4*e_vector.data[i]*e_vector.data[i+6] - e_vector.data[i+3]*e_vector.data[i+3];
	if(evalue[i] > 0)
	  {
         e_value_sequence = i;
		 break;
	   }
}


for (i = 0; i < 3;i++)
	{
	 para_al[i] = e_vector.data[i * 3 + e_value_sequence];
	}

for(i = 0;i < 3;i++)
{
  
  para_a[i] = para_al[i];
}

for(i = 0;i < 7;i++)
{
    para_a[i+3] = 0;
	for(j = 0;j < 3;j++)
	{
	 para_a[i+3] += T_mirror[i][j]*para_al[j];
	}
}



para_e[0][0] = para_a[0];
para_e[0][1] = para_a[3];
para_e[0][2] = para_a[4];
para_e[1][0] = para_a[3];
para_e[1][1] = para_a[1];
para_e[1][2] = para_a[5];
para_e[2][0] = para_a[4];
para_e[2][1] = para_a[5];
para_e[2][2] = para_a[2];


para_f[0] = para_a[6];
para_f[1] = para_a[7];
para_f[2] = para_a[8];

para_g = para_a[9];


magn_solve_inverse(&para_e[0][0],&para_inve[0][0],3);

for(i = 0;i < 3;i++)
{
  para_w[i] = 0;
  for(j = 0;j < 3;j++)
  	{
        para_w[i] += para_inve[i][j]*para_f[j];
    }

   para_w[i] = -para_w[i];
}
for(i = 0;i < 3;i++)
{
 algo_mag_output_data.hi_bias[i] = para_w[i];
}



 for(i = 0;i < 3;i++)
 {
  para_temp1[i] = 0;
  for(j = 0;j < 3;j++)
  	{
      para_temp1[i] += para_w[j]*para_e[j][i];
    } 

 }
 para_temp_var = 0;
for( i = 0;i < 3;i++)
	{
	para_temp_var += para_temp1[i]*para_w[i];
	}

para_temp_var -= para_g;

para_temp_var = 1.0/para_temp_var;

H2 = sqrt( cal_ellipsold[0].x*cal_ellipsold[0].x +  cal_ellipsold[0].y*cal_ellipsold[0].y +  cal_ellipsold[0].z*cal_ellipsold[0].z);

/*对矩阵K作SVD分解*/
for(i = 0;i < 3;i++)
{
 for(j = 0;j < 3;j++)
 	{
     para_k[i][j] = H2*para_temp_var*para_e[i][j];
    }

}

para_m[0] = -1;
para_m[1] = para_k[0][0] + para_k[1][1] + para_k[2][2];
para_m[2] = para_k[0][1] * para_k[1][0] + para_k[1][2] * para_k[2][1] - para_k[0][0] * para_k[1][1]
          + para_k[0][2] * para_k[2][0] - para_k[2][2] * para_k[0][0] - para_k[1][1] * para_k[2][2];
para_m[3] = para_k[0][0] * para_k[1][1] * para_k[2][2] + para_k[0][1] * para_k[1][2] * para_k[2][0]
          + para_k[0][2] * para_k[1][0] * para_k[2][1] - para_k[0][2] * para_k[1][1] * para_k[2][0]
          - para_k[0][0] * para_k[1][2] * para_k[2][1] - para_k[0][1] * para_k[1][0] * para_k[2][2];
for(i = 0;i < 3;i++)
{
	para_x[i] = 0;

}

	 
/*特征值*/
 evalue_judge(para_m, para_x);//前面e_value已经指向para_x


matrix_raw_data.row = 3;
matrix_raw_data.column = 3;
matrix_raw_data.data = &para_k[0][0];

 
 evector_judge(&matrix_raw_data,&e_vector, &e_value,1);//e_vector特征向量
 

  for(i = 0;i < 3;i++)
 {
   
	if (para_x[i]< 0)
		{

         para_x[i] = -para_x[i];
	   }
	else;
 
 }


 for(i = 0;i < 3;i++)
 	{
     for(j = 0;j < 3;j++)
     	{
     	 if(i ==j)
     	 	{
             para_R[i][j] = sqrt(e_value.data[i]);
		    }
		 else
		 	{
             para_R[i][j] = 0;

		    }
     	}  

    }
for(i = 0;i < 3;i++)
   {
     for(j = 0;j < 3;j++)
     	{
        para_temp2[i][j] = 0;
		 for(k = 0;k < 3;k++)
		 	{
               para_temp2[i][j] += e_vector.data[i*3+k]*para_R[k][j];
		     }
	    }

   }
for(i = 0;i < 3;i++)
   {
    for(j = 0;j < 3;j++)
    	{
          para_N[i][j] = 0;
           for(k = 0;k < 3;k++)
		 	{
               para_N[i][j] += para_temp2[i][k]* e_vector.data[j*3+k];
		     }
    
	    }

   }
  
  for(i = 0;i < 3;i++)
	 {
	  for(j = 0;j < 3;j++)
		  {
			algo_mag_output_data.sf_matrix[i][j] = para_N[i][j];
	  
		  }
  
	 }
	  


 /* ALGO_IMU_LOG_PRINTF("size_2= %d\r\n" ,xPortGetFreeHeapSize());
			  	
 uxTaskGetStackHighWaterMark(TaskToolHandle);
  
  ALGO_IMU_LOG_PRINTF("TaskTool_2= %d\r\n" , uxTaskGetStackHighWaterMark(TaskToolHandle));*/


algo_mag_output_data.flag = 3;


vPortFree(S1_mirror);
vPortFree(S2_mirror);
vPortFree(S3_mirror);
vPortFree(T_mirror);
vPortFree(M_mirror);
vPortFree(matrix_raw);
vPortFree(e_vector.data);


return 1;

}


float magn_angle_and_cal(uint8_t magn_cal_flag)
{
		float data_magn[3];
		float data_acc[3] = {0};
		double cal_x, cal_y, cal_z;
		float av;
		double pitch, roll, azimuth,Bx,By;
		const double rad2deg = 180 / AKFS_PI;
		lis3mdl_sample(1) magdata;
		int16_t a_buf[3] = {1,1,1};
		
		int ret = 0;
		uint8_t i,j;
		//读取地磁数据
		drv_lis3mdl_enable();
		am_util_delay_ms(1);
		ret = drv_lis3mdl_data(magdata.words);
		drv_lis3mdl_disable();
	
		if (ret == true)
		 {
		     /*
             地磁场的平均强度是0.5-0.6G
		     1T=10000G;
               1T=1000mT=1000000uT;
               */
			   Data_COM_TMP[0] = magdata.samples[0]*MAGN_CONVER_COFF*10;//高斯扩大10倍,便于计算
			   Data_COM_TMP[1] = magdata.samples[1]*MAGN_CONVER_COFF*10;
			   Data_COM_TMP[2] = magdata.samples[2]*MAGN_CONVER_COFF*10;
			   
			/*   ALGO_IMU_LOG_PRINTF("X = %d,Y = %d,Z = %d C = %d\r\n",(int)(Data_COM_TMP[0]*100),(int)(Data_COM_TMP[1]*100),(int)(Data_COM_TMP[2]*100),
			   (int)(sqrt(Data_COM_TMP[0]*Data_COM_TMP[0]+Data_COM_TMP[1]*Data_COM_TMP[1]+Data_COM_TMP[2]*Data_COM_TMP[2])*100));*/
			  
			}
		


			       
                    /*统一磁力计和加速度计坐标到北东天坐标系*/
                       
					// data_magn[0] = -Data_COM_TMP[0]; 
					 //data_magn[1] = -Data_COM_TMP[1]; 
					 //data_magn[2] = -Data_COM_TMP[2];
                  
				  //咕咚定制手表轴向:左X下Y且Z向上
			         data_magn[0] = -Data_COM_TMP[1]; 
					 data_magn[1] = -Data_COM_TMP[0]; 
					 data_magn[2] = Data_COM_TMP[2];
		
		
			if(magn_cal_flag == 1)
			 {
			 
              magn_cal_auto(data_magn,&algo_mag_output_data);
			
			  if(algo_mag_output_data.flag == 1)
			  	{
			 
			  	ALGO_IMU_LOG_PRINTF("ok,get para point enough\r\n");
              
					/*增加同值判断及剔除*/

		
              	
			/*	for(i=0;i<MAGN_CAL_PARA_MIN;i++)
              	{
				 	ALGO_IMU_LOG_PRINTF("save:X = %d,Y = %d,Z = %d\r\n",(int)(cal_magn_para[i].x*10),(int)(cal_magn_para[i].y*10),(int)(cal_magn_para[i].z*10));
					
              	}*/
				fit_ellipsoid(cal_magn_para,MAGN_CAL_PARA_MIN);


			   
			  	}


		
				g_CompassCaliStatus = algo_mag_output_data.flag;
			 if( g_CompassCaliStatus == 3)
				{
			
				
				   for( i = 0;i<3;i++)
					{
					mag_output_data.hi_bias[i] = algo_mag_output_data.hi_bias[i];
					magn_cal_para.hi_bias[i] = algo_mag_output_data.hi_bias[i];
					}
				  for( i = 0;i<3;i++)
					{
					 for( j = 0;j<3;j++)
						{
						
						 mag_output_data.sf_matrix[i][j] = algo_mag_output_data.sf_matrix[i][j];
						 magn_cal_para.sf_matrix[i][j] = algo_mag_output_data.sf_matrix[i][j];
	
					   }
	
				   }
				  
				  save_magn_calibration_paremeter();
				  
				}
					
			   }
			else;
		
		
			cal_x = ((data_magn[0] - mag_output_data.hi_bias[0])* mag_output_data.sf_matrix[0][0]
						+ (data_magn[1] - mag_output_data.hi_bias[1])* mag_output_data.sf_matrix[0][1]
						+ (data_magn[2] - mag_output_data.hi_bias[2])* mag_output_data.sf_matrix[0][2])/50;//50~20or250*MAGN_CONVER_COFF?
		   cal_y =	((data_magn[0] - mag_output_data.hi_bias[0])* mag_output_data.sf_matrix[1][0]
						+ (data_magn[1] - mag_output_data.hi_bias[1])* mag_output_data.sf_matrix[1][1]
						+ (data_magn[2] - mag_output_data.hi_bias[2])* mag_output_data.sf_matrix[1][2])/50;
		   cal_z =	 ((data_magn[0] - mag_output_data.hi_bias[0])* mag_output_data.sf_matrix[2][0]
						+ (data_magn[1] - mag_output_data.hi_bias[1])* mag_output_data.sf_matrix[2][1]
						+ (data_magn[2] - mag_output_data.hi_bias[2])* mag_output_data.sf_matrix[2][2])/50;
		
			
		
	
		
	
		 
			drv_lsm6dsl_acc_axes_raw(a_buf);
			
	
 
       /*统一磁力计和加速度计坐标到北东天坐标系*/
	   if (rtc_hw_flag == 1)
		{
            data_acc[0] = -a_buf[0];
			data_acc[1] =  a_buf[1];
			data_acc[2] = -a_buf[2];
	   	}
	   	else
	   	{
	        data_acc[0] = -a_buf[1];
			data_acc[1] =  -a_buf[0];
			data_acc[2] = -a_buf[2];
	   	}
	   
           

			if(acc_sample_count < ACC_STD_SUM)
		 {
			sample_acc.acc_x[acc_sample_count] = data_acc[0]* 0.000122f;
			sample_acc.acc_y[acc_sample_count] = data_acc[1]* 0.000122f;
			sample_acc.acc_z[acc_sample_count] = data_acc[2]* 0.000122f;
			sample_acc.acc_c[acc_sample_count] = sqrt(sample_acc.acc_x[acc_sample_count]*sample_acc.acc_x[acc_sample_count]
													 +sample_acc.acc_y[acc_sample_count]*sample_acc.acc_y[acc_sample_count]
													 +sample_acc.acc_z[acc_sample_count]*sample_acc.acc_z[acc_sample_count]);
			
			acc_sample_count++;
		 }
		 else
		 {
			 for(i = 0;i < ACC_STD_SUM-1;i++)
			 {
			  sample_acc.acc_x[i] = sample_acc.acc_x[i+1];
			  sample_acc.acc_y[i] = sample_acc.acc_y[i+1];
			  sample_acc.acc_z[i] = sample_acc.acc_z[i+1];
			  sample_acc.acc_c[i] = sample_acc.acc_c[i+1];
			  
			 }
			 sample_acc.acc_x[ACC_STD_SUM-1] = data_acc[0]* 0.000122f;
			 sample_acc.acc_y[ACC_STD_SUM-1] = data_acc[1]* 0.000122f;
			 sample_acc.acc_z[ACC_STD_SUM-1] = data_acc[2]* 0.000122f;
			 sample_acc.acc_c[ACC_STD_SUM-1] = sqrt(sample_acc.acc_x[ACC_STD_SUM-1]*sample_acc.acc_x[ACC_STD_SUM-1]
													+sample_acc.acc_y[ACC_STD_SUM-1]*sample_acc.acc_y[ACC_STD_SUM-1]
													+sample_acc.acc_z[ACC_STD_SUM-1]*sample_acc.acc_z[ACC_STD_SUM-1]);
			
		 }
		 if(acc_sample_count >= ACC_STD_SUM)
			{
			 acc_sample_count = ACC_STD_SUM;
			 std_acc_x = std_seconds_data(sample_acc.acc_x,ACC_STD_SUM);
			 std_acc_magn = std_seconds_data(sample_acc.acc_c,ACC_STD_SUM);
			 mean_acc_magn =  mean_data(sample_acc.acc_c,ACC_STD_SUM,1);
			}
		
			
			av = sqrtf(data_acc[0]*data_acc[0] + data_acc[1]*data_acc[1] + data_acc[2]*data_acc[2]);
			if(av == 0)
			{
				//防止除数为0
				av = 1;
			}
		
			pitch = asin(-data_acc[0] / av);
			roll = -atan(data_acc[1]/data_acc[2]);
		
			Bx = cal_x*cos(pitch)+cal_y*sin(pitch)*sin(roll)-cal_z*sin(pitch)*cos(roll);
			
			By = cal_y*cos(roll)+cal_z*sin(roll);

	
			if(By>0)
			{
			  azimuth = 90.0 - atan(Bx/By)*rad2deg;
				
			 }
			 else if(By<0)
				{
				 azimuth = 270.0 - atan(Bx/By)*rad2deg;
		
			   }
			 else 
				{
				 if(Bx>0)
					{
					 azimuth = 0;
		
					}
				 else
					{
					azimuth = 180;
				   }
		
		
			   }
			pitch = pitch * rad2deg;
			roll = roll * rad2deg;
			drv_lis3mdl_bubble_get(pitch,roll);
		
			
			azimuth = 360.0-azimuth;
	
	if (rtc_hw_flag == 1)
	{
		if((std_acc_magn<0.01f)&&(std_acc_x<0.01f)&&(mean_acc_magn<1.15f)&&(mean_acc_magn>0.85f))
		 {
	  		sample_acc.static_flag = 1;
	  	 	sample_acc.static_count++;
	  	}

		else
	 	{
        	sample_acc.static_flag = 0;
			sample_acc.static_count = 0;
     	}
	}
	else
	{
	  if((std_acc_magn<0.005f)&&(std_acc_x<0.005f)&&(mean_acc_magn<1.1f)&&(mean_acc_magn>0.9f))
	
	  {
	   sample_acc.static_flag = 1;
	   sample_acc.static_count++;
	
	  }
	  else
	 {
        sample_acc.static_flag = 0;
		sample_acc.static_count = 0;
      }
	}

	if((sample_acc.static_flag == 1)&&(sample_acc.static_count>=5))
      {
           if(fabs(azimuth-azimuth_init)<3.0f)
			{
			 azimuth = azimuth_init;
			}
		  else if(fabs(azimuth-azimuth_init)>=3.0f)
		  	{
	            azimuth = 0.5*(azimuth +azimuth_init);
		    }
	  	

	   }
	else;
		azimuth_init = azimuth;
		
			return	azimuth;
}


void drv_lis3mdl_bubble_get(float pitch, float roll)
{
	float MaxAngle = 88;	//最大倾斜角度
	//uint8_t Width = 88;	//气泡运动宽度
	
	uint8_t Width = 33;	//气泡运动宽度
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
}












