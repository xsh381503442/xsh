
#include "algo_step_count.h"
#include "com_data.h"
#include "com_sport.h"
#include "Task_sport.h"
#include "watch_config.h"

#if defined WATCH_RAISE_BRIGHT_SCREEN_ALGO
#include "Time_notify.h"
#else
#endif

#if defined WATCH_RAISE_BRIGHT_SCREEN_ALGO
#define pi_para 57.2958

#else
#endif
static uint8_t stepstatus; 
step_monitor_count data_monitor_count;//监视&计数模式结构体
step_data  step_data_str;
pedo_data pedometer_data;
uint8_t combo_speed_step = SPEED_NORMAL;;//加速度计判断出的运动状态
extern uint8_t   step_pattern_flag;
extern acc_s	 acc_xyzc;
uint8_t flag_compare = 0;

float mean_x,mean_absx,std_x,differ_x,std_y,mean_y,differ_y,mean_z,std_z,std_c,mean_c,max_c,min_c,differ_c;

#if defined WATCH_RAISE_BRIGHT_SCREEN_ALGO
screen_data screen_acc;
#else
#endif


/*获取计步状态*/
uint8_t get_step_status(void)
{
   
	return stepstatus;//测试时暂时置为1
}

/* 设置计步状态*/
void set_step_status(uint8_t status)
{
    stepstatus = status;
}

void monitor_count_init(void)
{
	data_monitor_count.step_count_num = 15;
	data_monitor_count.step_count_time = 3;
	acc_xyzc.get_data_flag = 0;
	acc_xyzc.get_data_signle_len = 0;
	
}
#if defined WATCH_RAISE_BRIGHT_SCREEN_ALGO
void brighten_screen_init()
{
	screen_acc.num = 0;
	screen_acc.num_neg = 0;
	screen_acc.angle_initial = -20.0;
	screen_acc.screen_flag = 0;
  
}
#else
#endif
int32_t get_data(float*data)
{
	int32_t data_raw;
	
	data_raw = (int32_t)((*data)/0.000122f);
	
	return data_raw;

}


/*预先判别运动状态*/
uint8_t cross_mean(acc_s* accp,float mean_c,uint8_t len,uint8_t flag)
{
    int i,wave_num=0;
   if (len == 0)
   {
		return wave_num;
   }

   if(flag==0)
   	{

	  for (i=0;i<len-1;i++)
	  	{
         
		 	if((accp->acc_cpmplex[i]>mean_c)&&(accp->acc_cpmplex[i+1]<mean_c))
           wave_num+=1;
                
	  	}

   	}

   else if(flag==1)
   	{
     for (i=0;i<len-1;i++)
	  	{
         
		 	if((accp->acc_x[i]<mean_c)&&(accp->acc_x[i+1]>mean_c))
           wave_num+=1;
                
	  	}



    }

     else if(flag==2)
   	{
     for (i=0;i<len-1;i++)
	  	{
         
		 	if((accp->acc_y[i]>mean_c)&&(accp->acc_y[i+1]<mean_c))
           wave_num+=1;
                
	  	}



    }

	 else if(flag==3)
   	{
     for (i=0;i<len-1;i++)
	  	{
         
		 	if((accp->acc_z[i]<mean_c)&&(accp->acc_z[i+1]>mean_c))
           wave_num+=1;
                
	  	}



    }
	 else;
   return wave_num;
}




int8_t sign_sum(float*data,uint16_t k,uint8_t flag,step_data*acc_str)
	{
		int i,differ_num;
		uint8_t length_ago,length_area;
		int8_t sign = 0;
		differ_num = acc_str->next_num - acc_str->remain_data_size;
		if((flag%2) == 0)
		  {
			length_area = flag/2;
		  }
		/*if((flag&1) == 0)
		  {
			length_area = flag>>1;
		  }*/
		else
			 length_area = 5;
		
		if(k<length_area)
		 {
		   length_ago = k;
		 }
		else
		{
		  length_ago = length_area;
		}
		
		for(i=1;i<=length_ago;i++)
		 {
			if((*data)>=(*(data-i)))
			  {
			   sign += 1;
			  }
			else
			  {
				sign -= 1;
			  }
		}
		for(i=1;i<=length_area;i++)
		{
		   if((*data)>=(*(data+i)))
			 {
			   sign += 1;
			 }
			else
			{
			   sign -= 1;
			}
		 }
	
	  if(k <length_area)
		{
		 /*if(sign == (k + length_area))
			{
			  sign = flag;
			}
		   else if(sign == -(k + length_area))
			{
			  sign = -flag;
		   }
		   else;*/
         for(i=0;i<length_area-k;i++)
          	{
              if((*data)>=(acc_str->save_next_acc[differ_num-1-i]))
				 {
				   sign += 1;
				 }
			 else
				{
				   sign -= 1;
				}  
		    }
		 
		   
		 }
	  
		return sign;
	}


/*求最大&小值函数*/

float max_min_data(float *data,uint16_t len,uint8_t flag)
{
 int i;
 float acc;
 acc = *data;
 if(flag == 1)//最大值
 	{
	 for(i=1;i<len;i++)
	 {
	 if(acc < (*(data+i)))
	 	{
	      acc = *(data+i);
	    }
	 }
 	}

 else //最小值
 	{
      for(i=1;i<len;i++)
	 {
	 if(acc > (*(data+i)))
	 	{
	     acc = *(data+i);
	    }
	 }
    }
 return acc;
}


/*求均值函数*/

float mean_data(float *data,uint16_t len,uint8_t flag)
{

uint16_t i;
float sum = 0.0,mean_data = 0.0;
if(flag==0)
{
	for(i=0;i<len;i++)
	{
	sum += *(data+i);

	}
}
else if(flag==1)
{
	for(i=0;i<len;i++)
		{
		sum += fabs(*(data+i));
	
		}


}
mean_data = sum/(len*1.0);

return mean_data;
}

/*求标准差函数*/


float std_seconds_data (float *data,uint16_t len)
{
uint16_t i;
float sum = 0.0,mean_data = 0.0,var_data = 0.0,data_std = 0.0;

for(i=0;i<len;i++)
{
sum += *(data+i);

}
mean_data = sum/(len*1.0);//鍧囧�?

for(i=0;i<len;i++)
{  

var_data += pow((*(data+i))-mean_data,2);
}
var_data/=((len-1)*1.0);//标准差分母值n或n-1
data_std = sqrt(var_data);//鏍囧噯宸?
return data_std;
}


uint8_t pedometer(acc_s* accp,uint8_t size,uint8_t size_seconds,step_data *data,step_monitor_count*step )



{
	
    uint16_t i;
	int tag,wave_num=0,n = 0,step_flag=0;
	float temp_valleyf,temp_valleyb;
  
	 pedometer_data.std_value = std_c;
			   
	/*行走:1~2.5Hz*/
    /*公交干扰标准差在0.03~0.1范围内(加波峰波谷差值判断小于0.2为无效步)
      打字干扰标准差0.03~0.2范围内
      正常拿手机走std大于0.06f
      慢走std都大于0.1
       取消0.03~0.06区间计步
       */
    if(max_c<=2.7f)//代表行走
          {

                if((pedometer_data.std_value >= 0.06f)&&(pedometer_data.std_value < 0.1f))
						{

					     
						    wave_num = cross_mean(accp,mean_c,size_seconds,0);
					
							 if((wave_num >=4)&&(mean_z>=0.95f)&&(mean_z<=1.05f))
							 	{

		                        step_flag = 1;
							   }
		                     pedometer_data.min_acc = 1.10f;
							// pedometer_data.limit_valley = 1.0f;
							 
							 pedometer_data.limit_valley = 1.2f;
						   	 pedometer_data.min_time = 0.5*size_seconds-1;
							 pedometer_data.max_time = 1.25*size_seconds;
			                // pedometer_data.sign_flag = (((size_seconds-(size_seconds%2))<14) ?(size_seconds-(size_seconds%2)):14);
							 
			                 pedometer_data.sign_flag = (((size_seconds-(size_seconds%2))<12) ?(size_seconds-(size_seconds%2)):12);
							 //step->step_count_num = 20;
							 
							 step->step_count_num = 15;
							 step->step_count_time = 2;
							 //pedometer_data.differ_wave_valley = 0.2f;
							 
							 pedometer_data.differ_wave_valley = 0.15f;
					   
						  combo_speed_step = SPEED_SLOW;
						
					   }

			   	/*慢走:1~2Hz/s*/
				/*
                min_time = 0.33*26;
                sign_flag = 12;

				*/
			     else if((pedometer_data.std_value >= 0.1f)&&(pedometer_data.std_value < 0.2f))
					 {

					      wave_num = cross_mean(accp,mean_c,size_seconds,0);
						 if ((wave_num >=4)&&(mean_z>=0.95f)&&(mean_z<=1.05f)&&(mean_x>(-0.1f))&&(mean_x<0.1f))
							 {

		                       step_flag = 1;
							 }
					   	 // pedometer_data.min_acc = ((mean_c*1.1f)>1.12f?(mean_c*1.1f):1.12f);
						  
					   	  pedometer_data.min_acc = ((mean_c>1.12f)?mean_c:1.12f);
					     // pedometer_data.limit_valley = 1.0f;
						  
					      pedometer_data.limit_valley = 1.3f;
					  	  pedometer_data.min_time = 0.33*26;
						  pedometer_data.max_time = 1*26;
		                //  pedometer_data.sign_flag = (((size_seconds-(size_seconds%2))<12) ?(size_seconds-(size_seconds%2)):12);
						  
		                  pedometer_data.sign_flag = (((size_seconds-(size_seconds%2))<10) ?(size_seconds-(size_seconds%2)):10);
		                  // pedometer_data.differ_wave_valley = 0.3f;
						   
		                   pedometer_data.differ_wave_valley = 0.2f;
					    if(step_pattern_flag == STEP_PATTERN_SPORT)//运动模式
					    	{
                            step->step_count_num = 7;
						    step->step_count_time = 3;
						    }
						else
							{
                             step->step_count_num = 12;
						     step->step_count_time = 3;
						    }
						 combo_speed_step = SPEED_GENERAL;
							
					  }
			  /*正常走&快走&部分慢走:1~2.5Hz/s*/
			 
				 // else  if((pedometer_data.std_value >= 0.2f) && (pedometer_data.std_value < 0.65f))
				  else  if(pedometer_data.std_value >= 0.2f)
				  	
					  	{
					   	 
						// pedometer_data.min_acc = (mean_c>1.3f?mean_c:1.3f);
						 
						 pedometer_data.min_acc = (mean_c>1.15f?mean_c:1.15f);
						 pedometer_data.min_time =5;
						 pedometer_data.max_time = 1*26;
						 pedometer_data.sign_flag = 8;
						 pedometer_data.limit_valley = 1.4f;
						// pedometer_data.differ_wave_valley = 0.35f;
						 
						 pedometer_data.differ_wave_valley = 0.25f;
				
					/*	if((std_x>=std_y)&&(mean_c>1.5f)) //快走
							{
					
                                pedometer_data.min_acc = mean_c;
								pedometer_data.min_time =5;
						        pedometer_data.max_time = 0.8*26;
						        pedometer_data.sign_flag = 8;
						        pedometer_data.limit_valley = 1.4f;
								pedometer_data.differ_wave_valley = 0.35f;
							  												
							 }
						 else if((std_z>0.15f)&&(mean_c>1.3f))
							{

						
                                pedometer_data.min_acc = mean_c*0.95f;
								pedometer_data.min_time =5;
						        pedometer_data.max_time =1*26;
						        pedometer_data.sign_flag = 6;
						        pedometer_data.limit_valley = 1.4f;
								//pedometer_data.differ_wave_valley = 0.27f;
								pedometer_data.differ_wave_valley = 0.32f;


						   }
						  else if((std_x<std_y))
						     {
						     if(mean_c<=1.2f)
						       	{
						         pedometer_data.min_acc =((mean_c*1.2f)<1.23f?(mean_c*1.2f):1.23f);
							     pedometer_data.min_time =7;
						         pedometer_data.max_time = 1*26;
						         pedometer_data.sign_flag = 10;
                                 pedometer_data.limit_valley = (1.2f>pedometer_data.min_acc?pedometer_data.min_acc:1.2f);
						         pedometer_data.differ_wave_valley = 0.4f;
						       	}
							   else if((mean_c<1.4f)&&(mean_c>1.2f))
							   	{
                                pedometer_data.min_acc = mean_c*1.0f;
							     pedometer_data.min_time =6;
						         pedometer_data.max_time = 1*26;
						         pedometer_data.sign_flag = 8;
                                 pedometer_data.limit_valley =(1.3f>pedometer_data.min_acc?pedometer_data.min_acc:1.3f);
						         pedometer_data.differ_wave_valley = 0.35f;

							     }
							   else;
							   
							 }
						  else;*/
                        
					      step->step_count_num = 12;
						   step->step_count_time = 3;
							  combo_speed_step = SPEED_NORMAL;
						 if(step_pattern_flag == STEP_PATTERN_SPORT)//运动模式
						  	{
							  	if(mean_absx>1.0f)
							  		{

									   if(mean_c>=2.0f)
									   	{
									 
										pedometer_data.min_acc = mean_c+0.4f*mean_c;
										pedometer_data.differ_wave_valley = 0.9*mean_c;
									   	}
									   else if((mean_c<2.0f)&&(mean_c>=1.5f))
									   	{
		                                
										   pedometer_data.min_acc = mean_c+0.3f*mean_c;
										    pedometer_data.differ_wave_valley = 0.8f*mean_c;
									   }
									   else if((mean_c<1.5f)&&(mean_c>=1.4f))
									   	{
		                               
										   pedometer_data.min_acc = mean_c+0.2f*mean_c;
										    pedometer_data.differ_wave_valley =0.7f*mean_c;

									    }
									   else
									   	{
		                                  // wave_num = cross_mean(accp,mean_c+0.18*mean_c,size_seconds,0);
										   pedometer_data.min_acc = mean_c;
										    pedometer_data.differ_wave_valley = 0.4f*mean_c;
										   
									    }
									    combo_speed_step = SPEED_FAST;
							  		}
							else;
							
	                           step->step_count_num = 7;
					           step->step_count_time = 3;
							   
						    }
						
					 	}	
			          

                else
					 {
			            data->tag = 1;
						step->step_count_num = 12;
						step->step_count_time = 3;
						combo_speed_step = SPEED_ZERO;
			            return (size_seconds-(size_seconds%2));
						
					 }
			 }
           /*跑步*/
	 else
				 	
		 {
	        /*较少见的部分数据区间*/
		    if((pedometer_data.std_value < 0.65f)||(max_c <=3.0f))
				{
				   if(std_x>std_y)
					 {
					   pedometer_data.min_time =5;
					   pedometer_data.max_time = 0.67*26;
					   pedometer_data.sign_flag = 8;
					   pedometer_data.differ_wave_valley = 0.27f;
					   pedometer_data.limit_valley = 1.5f;
							   	

					 }
					else
					 {
						pedometer_data.min_time =7;
						pedometer_data.max_time = 1*26;
						pedometer_data.sign_flag =10;
					    pedometer_data.differ_wave_valley = 0.35f;
					    pedometer_data.limit_valley = 1.4f;
									
					 }
                    combo_speed_step = SPEED_FAST;
					 if(step_pattern_flag == STEP_PATTERN_SPORT)//运动模式
						{
					    	if(mean_absx>1.0f)
							 {

								if(mean_c>=2.0f)
							 	{
											 
								 pedometer_data.min_acc = mean_c+0.4f*mean_c;
								 pedometer_data.differ_wave_valley = mean_c;
							 	}
							  else if((mean_c<2.0f)&&(mean_c>=1.5f))
								{
				                                
								 pedometer_data.min_acc = mean_c+0.35f*mean_c;
								 pedometer_data.differ_wave_valley = 0.85f*mean_c;
								}
							 else if((mean_c<1.5f)&&(mean_c>=1.4f))
					         	{
				                  pedometer_data.min_acc = mean_c+0.3f*mean_c;
								 pedometer_data.differ_wave_valley =0.7f*mean_c;
								}
							 else
								{
				                  pedometer_data.min_acc = mean_c;
								 pedometer_data.differ_wave_valley = 0.4f*mean_c;
												   
							   }
								 combo_speed_step = SPEED_RUN;
							}
						else;
								
		                    step->step_count_num = 7;
						    step->step_count_time = 3;
							   
			          }
 
				 }

			    else //跑
				       {

						 
							 combo_speed_step = SPEED_RUN;
				         
                            pedometer_data.min_time = 6;//5
					        pedometer_data.max_time = 0.67*26;
					        pedometer_data.sign_flag = 8;//8
								 if(mean_c>=2.0f)
							   	{
							   	wave_num = cross_mean(accp,mean_c+0.35f*mean_c,size_seconds,0);
								pedometer_data.min_acc = mean_c+0.40f*mean_c;
								pedometer_data.differ_wave_valley = mean_c;
							   	}
							   else if((mean_c<2.0f)&&(mean_c>=1.5f))
							   	{
                                   wave_num = cross_mean(accp,mean_c+0.30f*mean_c,size_seconds,0);
								   pedometer_data.min_acc = mean_c+0.35f*mean_c;
								    pedometer_data.differ_wave_valley = 0.85f*mean_c;
							   }
							   else if((mean_c<1.5f)&&(mean_c>=1.4f))
							   	{
                                   wave_num = cross_mean(accp,mean_c+0.25f*mean_c,size_seconds,0);
								   pedometer_data.min_acc = mean_c+0.3f*mean_c;
								    pedometer_data.differ_wave_valley =0.7f*mean_c;

							    }
							   else
							   	{
                                   wave_num = cross_mean(accp,mean_c+0.18f*mean_c,size_seconds,0);
								   pedometer_data.min_acc = mean_c+0.2f*mean_c;
								    pedometer_data.differ_wave_valley = 0.6f*mean_c;
								   
							    }
								if((wave_num>=4)&&(std_z>0.75f)&&(std_x>1.3f))
								{
								//pedometer_data.min_acc = 1.8f;
								pedometer_data.min_time =4;
                                pedometer_data.sign_flag = 6;
							    pedometer_data.limit_valley = 2.0f;
								    pedometer_data.max_time = 0.4*26;

							   }
							else if((wave_num<=2)&&(wave_num>0)&&(std_z<0.6f))
								{
								
                                     pedometer_data.min_time =7;//8
					                 pedometer_data.sign_flag = 12;//14
					                 pedometer_data.max_time = 1*26;
					                 pedometer_data.limit_valley = 1.5f;
									
								 
							   }
							else if(wave_num ==0)
								{
                                step_flag = 1;

							    }
							

								
							else
								{

                              
                               if((std_c>std_y)&&(differ_c>differ_y))
								 	{
                                    if((mean_y<0.8f)&&(std_z<0.6f))
                                    	{
                                          
										  pedometer_data.min_time =7;
							              pedometer_data.sign_flag = 10;
										   pedometer_data.max_time = 0.8*26;
										  pedometer_data.limit_valley = 1.6f;

									     }
									else if((mean_y>0.8f)&&(mean_y<1.0f))
										{
                                        if(std_z<0.6f)
                                        	{
	                                          pedometer_data.min_time =6;
								              pedometer_data.sign_flag = 8;
											  pedometer_data.limit_valley = 1.5f;
                                        	}
										else if(std_z>0.8f)
											{
                                            pedometer_data.min_time =4;
							                pedometer_data.sign_flag = 6;
										    pedometer_data.limit_valley = 2.0f;
										    }
											else
												{
                                                  pedometer_data.min_time =5;
							                      pedometer_data.sign_flag = 8;
										          pedometer_data.limit_valley = 1.6f;

											    }
									    }
									else if((mean_y>1.0f)&&(std_x > 1.3f)&&(std_z>0.7f))
										{
	                                     pedometer_data.min_time =4;
						                 pedometer_data.sign_flag = 6;
										 pedometer_data.limit_valley = 1.8f;
										}
									else
										{
										  pedometer_data.min_time =5;
							              pedometer_data.sign_flag = 8;
										  pedometer_data.limit_valley = 1.6f;
										}

									

								    }
                            
                             else if((std_c>std_y)||(differ_c>differ_y))//快慢待定 
                                 	{
                                 	
										if((std_x < 1.2f)&&(std_z<0.6f))
                                 		{
                                      
                                         pedometer_data.min_time =7;
					                     pedometer_data.sign_flag = 10;
									    pedometer_data.limit_valley = 1.4f;
									    }
										
										
										else if(((std_x > 1.4f)&&(std_z>0.8f))||((std_x > 1.3f)&&(mean_y>1.0f)))
											{
	                                 	     
	                                           pedometer_data.min_time =4;
						                       pedometer_data.sign_flag = 6;
											    pedometer_data.limit_valley = 1.6f;
											}
										
									else
										{

                                        
                                           pedometer_data.min_time =5;
					                       pedometer_data.sign_flag = 8;  
										    pedometer_data.limit_valley = 1.5f;
									   }
									
											
								  }

								
								
								 else if((std_y>std_c)&&(differ_y>differ_c))//快
								 	{
                                     
                                     
									 if((std_x > 1.4f)&&(mean_y > 1.1f)&&(std_z>0.7f))
									 	{

                                        pedometer_data.min_time = 4;
					                    pedometer_data.sign_flag = 6;
									    pedometer_data.limit_valley = 1.6f;
										 pedometer_data.max_time = 0.5*26;
									   }

									 else if((std_x < 1.4f)&&(mean_y < 1.0f))
									 	{
                                          pedometer_data.min_time = 6;
					                      pedometer_data.sign_flag = 8;
									      pedometer_data.limit_valley = 1.4f;

									    }
									 else
									 	{
                                          pedometer_data.min_time = 5;
					                     pedometer_data.sign_flag = 8;
									       pedometer_data.limit_valley = 1.5f;

									    }
									
         
								    }
								 else;
								 


							 }
							 if(step_pattern_flag == STEP_PATTERN_SPORT)
							{
                             
							
							  step->step_count_num = 7; 
							  step->step_count_time = 3;
						    }
						 else
						 	{
                              step->step_count_num = 10; 
							  step->step_count_time = 3;

						    }
				 
	    
				         	}
                }
						
          if(data->tag == 0)
			   	{
				   data->tag = 1;
				   tag  = 5;
                }
			   else
			   	{
			   	

			  
				 	 if(((data->remain_data_size)&1) ==0)
				   		{
						
						tag  = data->remain_data_size>>1;//除法改为移位

					    }
	              
					 else

					   {
	                     tag = 5;
					   }
					
			    }
		
			  if((size > 2*tag) &&(step_flag==0))
			  	{
			  	
			 
					for(i = tag;i<(size-(pedometer_data.sign_flag>>1));i++)//除法改为移位
				  {
				 
				  if((accp->acc_cpmplex[i])>pedometer_data.min_acc)
				  	{
				                             /*判断某个值是否是波峰*/
					   pedometer_data.sign = sign_sum(accp->acc_cpmplex+i,i,pedometer_data.sign_flag,data);
					  
						if(pedometer_data.sign == pedometer_data.sign_flag)
						   {
						   
						   	(data->amount) +=1;
								 n += 1;
						  	
								data->wave_point[n] = i;
								
								  if((n==1)&&(flag_compare==1));
								    

									   
								
							  else if((data->wave_point[n] - data->wave_point[n-1]) > pedometer_data.max_time)
									 {
										(data->amount) -= 1;
										
									 }
								else if((data->wave_point[n] - data->wave_point[n-1]) < pedometer_data.min_time)
									 {
										(data->amount) -= 1;
										  if(n==1)
											{
                                               if(accp->acc_cpmplex[data->wave_point[n]] >= accp->acc_zero);
											  
											  else
											  	{
	                                               
	                                               data->wave_point[n] = data->wave_point[0];
											    }
										    }
										  else
										  	{
										  	   if( data->wave_point[n-1]<0)
										  	   	{
                                                 if(accp->acc_cpmplex[data->wave_point[n]] >= accp->acc_zero);
												  	
												  else
												  	{
		                                                  data->wave_point[n] = data->wave_point[0];
												      
												    }
											    }
											   else
											   	{
												  if(accp->acc_cpmplex[data->wave_point[n]] >= accp->acc_cpmplex[data->wave_point[n-1]]);
												  
												  else
												  	{
		                                               
		                                               data->wave_point[n] = data->wave_point[n-1];
												      
												    }
											   	}
										  	}
									 }
								 else//判断两个波峰之间是否存在波谷
								 	{ 
								 	if(data->wave_point[n-1]<0)
								 		{
								 		temp_valleyf = max_min_data(data->acc_temp,data->temp_num,0);
										temp_valleyb = max_min_data(accp->acc_cpmplex,data->wave_point[n],0);
								 	    pedometer_data.value_valley = (temp_valleyf < temp_valleyb?temp_valleyf:temp_valleyb);
										 
										  
										  
								 		}
									else
										{

                                         pedometer_data.value_valley = max_min_data(accp->acc_cpmplex+(data->wave_point[n-1]),data->wave_point[n]-data->wave_point[n-1]+1,0);
									    }
								
								
									if((pedometer_data.value_valley>pedometer_data.limit_valley)||((accp->acc_cpmplex[data->wave_point[n]]-pedometer_data.value_valley)<pedometer_data.differ_wave_valley))
										{
                                           (data->amount) -= 1;
										    if(n==1)
											{
                                               if(accp->acc_cpmplex[data->wave_point[n]] >= accp->acc_zero);
											  
											  else
											  	{
	                                               data->wave_point[n] = data->wave_point[0];
											    }
										    }
										  else
										  	{
										  	   if(data->wave_point[n-1]<0)
										  	   	{
                                                 if(accp->acc_cpmplex[data->wave_point[n]] >= accp->acc_zero);
												  
												  else
												  	{
		                                               data->wave_point[n] = data->wave_point[0];
												      
												    }
											    }
											   else
											   	{
												  if(accp->acc_cpmplex[data->wave_point[n]] >= accp->acc_cpmplex[data->wave_point[n-1]]);
												  
												  else
												  	{
		                                               
		                                               data->wave_point[n] = data->wave_point[n-1];
												      
												    }
											   	}
										  	}

									    }
									else;
                                     
								   }
								 
						
						  }
					
						else;
					}
			
				  else;
				 }
				
			  }
			 else if((step_flag==1)&&(size > 2*tag))
			  {
			     data->last_point = -1;
                 return (pedometer_data.sign_flag>size?(size-(size%2)):(pedometer_data.sign_flag));
			  }
			 else 
			 	{
			 	 data->last_point = -1;
                 return (size-size%2);
			    }
			if(n == 0)
			  {
                 data->last_point = -1;
			   }
			 else
			 	{
               data->last_point =  data->wave_point[n];
			   }
	
	 
	/*返回留待下一次数据拼接*/
	return (pedometer_data.sign_flag > size?(size-(size%2)):(pedometer_data.sign_flag));
	
}





void  step_amount_count(acc_s* acc,uint16_t size1,step_data *data,step_monitor_count *step)

 {
	
   uint8_t size,size_back;
   int i;
  
   if(data->tag == 0)
	  {
 
		  size	= size1;
		  data->wave_point[0] = 0;
		  data->last_point = -1;
		   acc->acc_zero = 1.0f;
		   data->next_num = 0;
		   data->remain_data_size = 0;
 
	   }
	else
	 {
	   /*for(i=0;i<size1;i++)
		   {
		   acc->acc_cpmplex[i+ data->remain_data_size] = acc->acc_cpmplex[i];
	 
		   }*/
	   for(i=size1-1;i>=0;i--)
		   {
		   acc->acc_cpmplex[i+ data->remain_data_size] = acc->acc_cpmplex[i];
	 
		   }
	  for(i=0;i<data->remain_data_size;i++)
		   {
		   acc->acc_cpmplex[i] = data->step_next[i];
		  
		   }
	   
	   size = data->remain_data_size+size1;
	 }
		
	   size_back =	pedometer(acc,size,size1,data,step);
	   for(i=0;i<size_back;i++)
		 {
		  data->step_next[i] = acc->acc_cpmplex[size-size_back+i];
		 
		 }
	   data->next_num = ((24>size1)?size1:24);
	   for(i=0;i<data->next_num;i++)//此处判断数据长度
		 {
			
			data->save_next_acc[i] =  acc->acc_cpmplex[size - data->next_num+i];
 
		 }
		data->remain_data_size = size_back;
		if(data->last_point <= (-1))
		  {
		   
			//data->wave_point[0] = -(size-size_back);
		   // acc->acc_zero = acc->acc_cpmplex[0];
			flag_compare = 1;
			
		  }
	   else
		  {
		  flag_compare = 0;
			data->wave_point[0] = (data->last_point) - (size-size_back);
			if( data->wave_point[0]>1)
			  {
				data->wave_point[0]+=1;
			  }
		   if((data->wave_point[0] < 0)&&(data->wave_point[0] > -24))
			 {
			  data->temp_num = - (data->wave_point[0]);
				for(i = 0 ;i< data->temp_num;i++)//异常处理
				 {
				   data->acc_temp[i] = acc->acc_cpmplex[i+ data->last_point];
				 }
			 }
			acc->acc_zero = acc->acc_cpmplex[data->last_point];
		  }
 
	 
   
  }




 void step_count_entrance(uint16_t len,acc_s *acc_data)
 	{

    

	 
               /*计算acc的标准差、平均值等特征值*/
              std_x = std_seconds_data(acc_data->acc_x,(len/3));
	         mean_x = mean_data(acc_data->acc_x,(len/3),0);
			  mean_absx =  mean_data(acc_data->acc_x,(len/3),1);
			 differ_x = (max_min_data(acc_data->acc_x,(len/3),1)-max_min_data(acc_data->acc_x,(len/3),0));//X值加速度最大变化值
			 
			 std_y = std_seconds_data(acc_data->acc_y,(len/3));
			  mean_y =  mean_data(acc_data->acc_y,(len/3),1);
			 differ_y = (max_min_data(acc_data->acc_y,(len/3),1)-max_min_data(acc_data->acc_y,(len/3),0));

			  mean_z = mean_data(acc_data->acc_z,(len/3),0);
			 std_z = std_seconds_data(acc_data->acc_z,(len/3));
			
			 
			
			 
             std_c = std_seconds_data(acc_data->acc_cpmplex,(len/3));
			 mean_c = mean_data(acc_data->acc_cpmplex,(len/3),0);
			 max_c =  max_min_data(acc_data->acc_cpmplex,(len/3),1);
			 min_c = max_min_data(acc_data->acc_cpmplex,(len/3),0);
			differ_c = max_c-min_c;
	

  
	       step_amount_count(acc_data,len/3,&step_data_str,&data_monitor_count);
				
				
		
        

				data_monitor_count.tmpcount = step_data_str.amount;
				     
                        if (data_monitor_count.tmpcount == data_monitor_count.last_tmpcount)
                        {
                            data_monitor_count.tmptime++;
                            if (data_monitor_count.tmptime >=data_monitor_count.step_count_time) //无步数增加切换到检测模式
                            {
                              data_monitor_count.last_checkstep = data_monitor_count.tmpcount ;
                              data_monitor_count.status_flag =0;
                              data_monitor_count.tmptime = 0;
                              
                            }
						            
                        }
                        else
                        {
                            data_monitor_count.tmptime = 0;
						            	
                        }
                        
                        if (data_monitor_count.status_flag ==1)//计数模式
                        {
                          if((int32_t)(data_monitor_count.tmpcount - data_monitor_count.last_tmpcount)>0)
                          	{
                          	 if(step_pattern_flag == STEP_PATTERN_DAILY)
                          	 	{
                                 step_data_str.passometer_mun +=(data_monitor_count.tmpcount - data_monitor_count.last_tmpcount);
                          	 	}
							  else if(step_pattern_flag == STEP_PATTERN_SPORT)
								{
                                  step_data_str.step_num_sport +=(data_monitor_count.tmpcount - data_monitor_count.last_tmpcount);

								 }
                          	}
                         
                        }

						if (data_monitor_count.status_flag ==0)
						// else //检测模式
                        {
                            data_monitor_count.tmp_checkstep = data_monitor_count.tmpcount ;
                            if((int32_t)(data_monitor_count.tmp_checkstep - data_monitor_count.last_checkstep) >= data_monitor_count.step_count_num)
                            {  
                              if(step_pattern_flag == STEP_PATTERN_DAILY)
                              	{
                                step_data_str.passometer_mun +=(data_monitor_count.tmp_checkstep - data_monitor_count.last_checkstep);
                              	}
							  else if(step_pattern_flag == STEP_PATTERN_SPORT)
							  	{
                                  step_data_str.step_num_sport +=(data_monitor_count.tmp_checkstep - data_monitor_count.last_checkstep);

							    }
                                data_monitor_count.status_flag =1;
                                data_monitor_count.last_checkstep = data_monitor_count.tmp_checkstep;
                                
                            }  
							
                            
                        }
                        data_monitor_count.last_tmpcount = data_monitor_count.tmpcount ;
	  	
	   
 }

#if defined WATCH_RAISE_BRIGHT_SCREEN_ALGO

void brighten_screen_entrance(uint16_t len,acc_s *acc_data,uint16_t flag)
	{
 	int i,z_num = 0;
	
	float angle_x=0,angle_z=0,anglez_sum=0;
	if(step_pattern_flag == STEP_PATTERN_DAILY) //日常模式
		{
		  for(i=0;i<len;i++)
	         {
	             /*计算角度*/
					angle_x = atan2(acc_data->acc_x[i],sqrt(acc_data->acc_y[i]*acc_data->acc_y[i]+acc_data->acc_z[i]*acc_data->acc_z[i]))*pi_para;
		
		           angle_z = atan2(acc_data->acc_z[i],sqrt(acc_data->acc_x[i]*acc_data->acc_x[i]+acc_data->acc_y[i]*acc_data->acc_y[i]))*pi_para;

				/*满足要求的角度持续时间+*/
            	if((angle_x>-3.0f)&&((angle_z>15.0f))&&((fabs(acc_data->acc_y[i])-acc_data->acc_z[i])<=(-0.05f)))
				{
				  screen_acc.num+=1;
				  anglez_sum+=angle_z;
				  z_num+=1;
				  
				}
				
				//else if((angle_x<-10.0f)&&(angle_z<25.0f))
				else if((angle_x<-14.0f)&&(angle_z<25.0f))
				{
					 screen_acc.num = 0;
					anglez_sum = 0;
					z_num = 0; 
			         screen_acc.num_neg+=1;
					
				}
				
		        else
		          {
		        	screen_acc.num = 0;
					 anglez_sum = 0;
					 z_num = 0;
		           }
						/*满足要求的角度持续时间达到了则调用对应函数*/
					if((screen_acc.num >=8)&&(screen_acc.screen_flag==0)&&((anglez_sum/z_num)>25.0f))
			        {
			       
			           timer_notify_backlight_start(SetValue.backlight_timeout_mssec,false);
					   screen_acc.num = 0; 
					    screen_acc.num_neg = 0;
						 anglez_sum = 0;
						 z_num = 0;
					   screen_acc.screen_flag = 1;
					   
					   return;
					}
			   if((screen_acc.num_neg>=5)&&(screen_acc.screen_flag == 1))
			   	{
			   	screen_acc.screen_flag  = 0;
				timer_notify_backlight_stop(false);
				return;
			    }
		   }

	    }

	else if(step_pattern_flag == STEP_PATTERN_SPORT)//运动模式
		{
            for(i=0;i<len;i++)
			{
				 angle_x = atan2(acc_data->acc_x[i+flag],sqrt(acc_data->acc_y[i+flag]*acc_data->acc_y[i+flag]+acc_data->acc_z[i+flag]*acc_data->acc_z[i+flag]))*pi_para;
		
		         angle_z = atan2(acc_data->acc_z[i+flag],sqrt(acc_data->acc_x[i+flag]*acc_data->acc_x[i+flag]+acc_data->acc_y[i+flag]*acc_data->acc_y[i+flag]))*pi_para;
				
				if((angle_x>-7.0f)&&(angle_z>12.0f))
				{
				  screen_acc.num+=1;
				  anglez_sum+=angle_z;
				  z_num+=1;
					
				}
				//else if((angle_x<-10.0f)&&(angle_z<25.0f))
				else if((angle_x<-20.0f)&&(angle_z<20.0f))
				{
				  screen_acc.num = 0;
				  anglez_sum = 0;
				  z_num = 0; 
			      screen_acc.num_neg+=1;
					
				}
				
		        else
		        	{
		        	 screen_acc.num = 0;
					 anglez_sum = 0;
					 z_num = 0; 

				    }
				if((screen_acc.num >=7)&&(screen_acc.screen_flag==0)&&((anglez_sum/z_num)>20.0f))
			        {
			       
			           timer_notify_backlight_start(SetValue.backlight_timeout_mssec,false);
					   screen_acc.num = 0; 
					    screen_acc.num_neg = 0;
						 anglez_sum = 0;
						  z_num = 0;
					   screen_acc.screen_flag = 1;
					    return;
					   
					}
			   if((screen_acc.num_neg>=5)&&(screen_acc.screen_flag == 1))
			   	{
			   	screen_acc.screen_flag  = 0;
				timer_notify_backlight_stop(false);
				 return;
				}
			}

	    }
	else;
 	
}
#else
#endif

void init_steps(uint32_t steps,uint32_t steps_sport)	   
{
	step_data_str.amount = steps;
	step_data_str.passometer_mun = steps;
	step_data_str.tag = 0;
	step_data_str.step_num_sport = steps_sport;//运动模式计步总数

	data_monitor_count.last_checkstep = steps;
	data_monitor_count.last_tmpcount = steps;
	data_monitor_count.status_flag = 0;
	data_monitor_count.tmpcount = steps;
	data_monitor_count.tmptime = 0;
	data_monitor_count.tmp_checkstep = steps;
	data_monitor_count.step_count_num = 15;
	data_monitor_count.step_count_time = 3;
}




