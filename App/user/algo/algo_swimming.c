#include "algo_swimming.h"


SwimFilterDataStr SwimFilterData;
float g_sumfilter[3];
float g_SwimDataDiffFilterBuf[DIFF_SIZE][3];

DelInvalidWaveDataStr DelInvalidWaveData;

uint8_t g_strokechoice;

StrokeTemplateDataStr BackStrokeTemplateData;
uint8_t g_backdoublemidflag_z;
float g_backdoublemidvalue_z;

StrokeTemplateDataStr BreastStrokeTemplateData;

StrokeTemplateDataStr ButterflyStrokeTemplateData;
uint8_t g_buttermidflag_y;
float g_buttermidvalue_y;
uint8_t g_buttermidflag_z;
float g_buttermidvalue_z;

StrokeTemplateDataStr FreeStyleTemplateData;
float g_freemidvalue_z;

uint8_t g_swimresultcircle;		//游泳的圈数
uint16_t g_swimresultwavecount;		//总的划数
uint16_t g_swimpresentwavecount;
uint16_t g_swimpastwavecount;

uint32_t g_num;
uint32_t g_presentvalidnum;
uint32_t g_pastvalidnum;
uint32_t g_size[2];

void SwimAlgorithmInit()
{
		SwimFilterData.front = 0;
		SwimFilterData.rear = -1;
		SwimFilterData.size = 0;
    memset(SwimFilterData.data, 0, sizeof(SwimFilterData.data));
    memset(g_sumfilter, 0, sizeof(g_sumfilter));
    memset(g_SwimDataDiffFilterBuf, 0, sizeof(g_SwimDataDiffFilterBuf));

    memset(DelInvalidWaveData.pointnumb, 0, sizeof(DelInvalidWaveData.pointnumb));
    memset(DelInvalidWaveData.delwavestep, '0', sizeof(DelInvalidWaveData.delwavestep));
    memset(DelInvalidWaveData.wavevalue, 0, sizeof(DelInvalidWaveData.wavevalue));
    memset(DelInvalidWaveData.wavevaluetrend, 0, sizeof(DelInvalidWaveData.wavevaluetrend));

    g_strokechoice = 'u';
    memset(BackStrokeTemplateData.periodstep, '0', sizeof(BackStrokeTemplateData.periodstep));
    memset(BackStrokeTemplateData.partperiodcount, 0, sizeof(BackStrokeTemplateData.partperiodcount));
    memset(BackStrokeTemplateData.sumperiodcount, 0, sizeof(BackStrokeTemplateData.sumperiodcount));
    memset(BackStrokeTemplateData.continuityflag, 'f', sizeof(BackStrokeTemplateData.continuityflag));
    memset(BackStrokeTemplateData.sumwavecount, 0, sizeof(BackStrokeTemplateData.sumwavecount));
    memset(BackStrokeTemplateData.partwavecount, 0, sizeof(BackStrokeTemplateData.partwavecount));
    memset(BreastStrokeTemplateData.periodstep, '0', sizeof(BreastStrokeTemplateData.periodstep));
    memset(BreastStrokeTemplateData.partperiodcount, 0, sizeof(BreastStrokeTemplateData.partperiodcount));
    memset(BreastStrokeTemplateData.sumperiodcount, 0, sizeof(BreastStrokeTemplateData.sumperiodcount));
    memset(BreastStrokeTemplateData.continuityflag, 'f', sizeof(BreastStrokeTemplateData.continuityflag));
    memset(BreastStrokeTemplateData.partwavecount, 0, sizeof(BreastStrokeTemplateData.partwavecount));
    memset(BreastStrokeTemplateData.sumwavecount, 0, sizeof(BreastStrokeTemplateData.sumwavecount));
    memset(ButterflyStrokeTemplateData.periodstep, '0', sizeof(ButterflyStrokeTemplateData.periodstep));
    memset(ButterflyStrokeTemplateData.partperiodcount, 0, sizeof(ButterflyStrokeTemplateData.partperiodcount));
    memset(ButterflyStrokeTemplateData.sumperiodcount, 0, sizeof(ButterflyStrokeTemplateData.sumperiodcount));
    memset(ButterflyStrokeTemplateData.continuityflag, 'f', sizeof(ButterflyStrokeTemplateData.continuityflag));
    memset(ButterflyStrokeTemplateData.partwavecount, 0, sizeof(ButterflyStrokeTemplateData.partwavecount));
    memset(ButterflyStrokeTemplateData.sumwavecount, 0, sizeof(ButterflyStrokeTemplateData.sumwavecount));
    memset(FreeStyleTemplateData.periodstep, '0', sizeof(FreeStyleTemplateData.periodstep));
    memset(FreeStyleTemplateData.partperiodcount, 0, sizeof(FreeStyleTemplateData.partperiodcount));
    memset(FreeStyleTemplateData.partperiodcount, 0, sizeof(FreeStyleTemplateData.partperiodcount));
    memset(FreeStyleTemplateData.continuityflag, 'f', sizeof(FreeStyleTemplateData.continuityflag));
    memset(FreeStyleTemplateData.partwavecount, 0, sizeof(FreeStyleTemplateData.partwavecount));
    memset(FreeStyleTemplateData.sumwavecount, 0, sizeof(FreeStyleTemplateData.sumwavecount));

    g_backdoublemidflag_z = 'u';
    g_backdoublemidvalue_z = 0;
 
    g_buttermidflag_y = 'u';
    g_buttermidvalue_y = 0;
    g_buttermidflag_z = 'u';
    g_buttermidvalue_z = 0;
      
    g_freemidvalue_z = 0;
    g_swimresultwavecount	= 0;
		g_swimresultcircle = 0;
		g_swimpresentwavecount = 0;
    g_swimpastwavecount = 0;

    g_num = 0;
    g_presentvalidnum = 0;
    g_pastvalidnum = 0;
		memset(g_size, 0, sizeof(g_size));
}


void SwimDataMeanFilter(float* lfv_xyzc)
{    
     if (SwimFilterData.rear == FILTER_SIZE - 1)
     {
         SwimFilterData.rear = 0;
     }
     else
     {
         SwimFilterData.rear++;
     }
     for (uint8_t liv_dimenson = 0; liv_dimenson < 3; liv_dimenson++)
     {
         SwimFilterData.data[SwimFilterData.rear][liv_dimenson] = *(lfv_xyzc + liv_dimenson);
         g_sumfilter[liv_dimenson] += SwimFilterData.data[SwimFilterData.rear][liv_dimenson];
     }
     SwimFilterData.size++;
          
     int8_t* liv_input;
     
    if (SwimFilterData.size < FILTER_SIZE)
    {
        if (SwimFilterData.size == 1)
        {
            for (uint8_t liv_dimenson = 0; liv_dimenson < 3; liv_dimenson++)
            {
                g_SwimDataDiffFilterBuf[0][liv_dimenson] = SwimFilterData.data[0][liv_dimenson];
            }
						g_num++;
            int8_t liv_input[3] = { 0, 0, 0 };
            DelInvalidWave(SwimFilterData.data[0], liv_input);            
        } 
        
        if(SwimFilterData.size >=  (2 + (FILTER_SIZE - 1) / 2))
        {
            float lfv_output[3] = { 0,0,0 };
            for (uint8_t liv_dimenson = 0; liv_dimenson < 3; liv_dimenson++)
            {
                lfv_output[liv_dimenson] = (float)g_sumfilter[liv_dimenson] / SwimFilterData.size;
            }

            if (SwimFilterData.size == (2 + (FILTER_SIZE - 1) / 2))
            {
                for (uint8_t liv_dimenson = 0; liv_dimenson < 3; liv_dimenson++)
                {
                    g_SwimDataDiffFilterBuf[1][liv_dimenson] = lfv_output[liv_dimenson];
                }
            }
            else
            {
                for (uint8_t liv_dimenson = 0; liv_dimenson < 3; liv_dimenson++)
                {
                    g_SwimDataDiffFilterBuf[2][liv_dimenson] = lfv_output[liv_dimenson];
                }
								g_num++;
                liv_input = SwimDataWaveFind(g_SwimDataDiffFilterBuf[0], g_SwimDataDiffFilterBuf[1], g_SwimDataDiffFilterBuf[2]);
                DelInvalidWave(g_SwimDataDiffFilterBuf[1], liv_input);
                for (uint8_t liv_dimenson = 0; liv_dimenson < 3; liv_dimenson++)
                {
                    g_SwimDataDiffFilterBuf[0][liv_dimenson] = g_SwimDataDiffFilterBuf[1][liv_dimenson];
                    g_SwimDataDiffFilterBuf[1][liv_dimenson] = g_SwimDataDiffFilterBuf[2][liv_dimenson];
                }
            }           
        }        
    }
    
    if (SwimFilterData.size >= FILTER_SIZE)
    {
        float lfv_output[3] = {0,0,0};
        for (uint8_t liv_dimenson = 0; liv_dimenson < 3; liv_dimenson++)
        {
            lfv_output[liv_dimenson] = (float)g_sumfilter[liv_dimenson] / FILTER_SIZE;
            g_sumfilter[liv_dimenson] -= SwimFilterData.data[SwimFilterData.front][liv_dimenson];
        }
        if (SwimFilterData.front == FILTER_SIZE - 1)
        {
            SwimFilterData.front = 0;
        }
        else
        {
            SwimFilterData.front++;
        }
        for (uint8_t liv_dimenson = 0; liv_dimenson < 3; liv_dimenson++)
        {
            g_SwimDataDiffFilterBuf[2][liv_dimenson] = lfv_output[liv_dimenson];
        }
				g_num++;
        liv_input = SwimDataWaveFind(g_SwimDataDiffFilterBuf[0], g_SwimDataDiffFilterBuf[1], g_SwimDataDiffFilterBuf[2]);
        DelInvalidWave(g_SwimDataDiffFilterBuf[1], liv_input);
        for (uint8_t liv_dimenson = 0; liv_dimenson < 3; liv_dimenson++)
        {
            g_SwimDataDiffFilterBuf[0][liv_dimenson] = g_SwimDataDiffFilterBuf[1][liv_dimenson];
            g_SwimDataDiffFilterBuf[1][liv_dimenson] = g_SwimDataDiffFilterBuf[2][liv_dimenson];
        }
    }        
}


int8_t* SwimDataWaveFind(float* lfv_diffp, float* lfv_diff, float* lfv_diffn)
{
    static int8_t liv_output[3];
    for (uint8_t liv_dimenson = 0; liv_dimenson < 3; liv_dimenson++)
    {
        float lfv_p = (float)(*(lfv_diff + liv_dimenson) - *(lfv_diffp + liv_dimenson));
        float lfv_n = (float)(*(lfv_diff + liv_dimenson) - *(lfv_diffn + liv_dimenson));
        if (lfv_p > 0)
        {
            if (lfv_n > 0)
            {
                liv_output[liv_dimenson] = 2;
            }
            else if (lfv_n == 0)
            {
                liv_output[liv_dimenson] = 1;
            }
            else
            {
                liv_output[liv_dimenson] = 0;
            }
        }
        else if (lfv_p == 0) 
        {
            if (lfv_n > 0)
            {
                liv_output[liv_dimenson] = 1;
            }
            else if (lfv_n == 0)
            {
                liv_output[liv_dimenson] = 0;
            }
            else
            {
                liv_output[liv_dimenson] = -1;
            }
        }
        else
        {
            if (lfv_n > 0)
            {
                liv_output[liv_dimenson] = 0;
            }
            else if(lfv_n == 0)
            {
                liv_output[liv_dimenson] = -1;
            }
            else
            {
                liv_output[liv_dimenson] = -2;
            }
        }       
    }
    return liv_output;
}


void DelInvalidWave(float* lfv_xyz, int8_t* liv_xyztrend)
{
    uint8_t liv_setpoints = 8;  //设置删除点数范围
    float lfv_setdist = 0.5;    //设置删除点之间的距离
    float lfv_output[2] = {0 ,0};
    int8_t liv_outputtrend[2] = { 0, 0 };
    int8_t liv_axischoice;
    uint8_t liv_size; 
    for (uint8_t liv_dimenson = 0; liv_dimenson < 3; liv_dimenson++)
    {              
        if (*(liv_xyztrend+liv_dimenson) != 0)
        {
            if (DelInvalidWaveData.delwavestep[liv_dimenson] == '0') //删除算法的开始
            {
                DelInvalidWaveData.wavevalue[0][liv_dimenson] = *(lfv_xyz + liv_dimenson)*1000;
                DelInvalidWaveData.wavevaluetrend[0][liv_dimenson] = *(liv_xyztrend + liv_dimenson);
                DelInvalidWaveData.delwavestep[liv_dimenson] = '1'; 
                if(liv_dimenson == 1)
								{		
									  g_size[0] = g_num;
								}									
            }
            else if (DelInvalidWaveData.delwavestep[liv_dimenson] == '1') //已存了一个数据，判断第二个数据。删除算法第二步
            {
                if ((DelInvalidWaveData.pointnumb[liv_dimenson] <= liv_setpoints) && (fabs(DelInvalidWaveData.wavevalue[0][liv_dimenson] - *(lfv_xyz + liv_dimenson) * 1000) <= lfv_setdist))   //前两个数据比较后，满足删除条件
                {
                    DelInvalidWaveData.wavevalue[1][liv_dimenson] = *(lfv_xyz + liv_dimenson)*1000;
                    DelInvalidWaveData.wavevaluetrend[1][liv_dimenson] = *(liv_xyztrend + liv_dimenson);
                    DelInvalidWaveData.delwavestep[liv_dimenson] = '2';
									  if(liv_dimenson == 1)
								    {		
									      g_size[1] = g_num;
								    }	
                }
                else //不满足删除条件，说明第一个数据是有效数据
                {
                    liv_axischoice = liv_dimenson;
                    liv_size = 1;
                    lfv_output[0] = DelInvalidWaveData.wavevalue[0][liv_dimenson];
                    liv_outputtrend[0] = DelInvalidWaveData.wavevaluetrend[0][liv_dimenson];
                    DelInvalidWaveData.wavevalue[0][liv_dimenson] = *(lfv_xyz + liv_dimenson)*1000;
                    DelInvalidWaveData.wavevaluetrend[0][liv_dimenson] = *(liv_xyztrend + liv_dimenson);
                    if(liv_dimenson == 1)
								    {		
									      g_presentvalidnum = g_size[0];
											  g_size[0] = g_num;
								    }	
                    TemplateCompare(liv_axischoice, liv_size, lfv_output, liv_outputtrend);
                }
            }
            else //已存了两个数据，判断第三个数据。看看第一个数据与第二个数据的去留。删除算法的第三步
            {
                if ((DelInvalidWaveData.pointnumb[liv_dimenson] <= liv_setpoints) && (fabs(DelInvalidWaveData.wavevalue[1][liv_dimenson] - *(lfv_xyz + liv_dimenson) * 1000) <= lfv_setdist))   //第三个数据与第二个数据对比，第二个数据满足删除条件
                {
                    DelInvalidWaveData.wavevalue[1][liv_dimenson] = *(lfv_xyz + liv_dimenson)*1000;
                    DelInvalidWaveData.wavevaluetrend[1][liv_dimenson] = *(liv_xyztrend + liv_dimenson);
									  if(liv_dimenson == 1)
								    {		
									      g_size[1] = g_num;
								    }
                }
                else //第二个与第三个数据之间不满足删除条件
                {
                    if (DelInvalidWaveData.wavevaluetrend[0][liv_dimenson] * (*(liv_xyztrend + liv_dimenson)) > 0)  //第一个数据与第三个数据趋势属性相同
                    {
                        if (DelInvalidWaveData.wavevaluetrend[0][liv_dimenson] > 0) //第一个数据与第三个数据趋势属性相同，且都大于零，即同为波峰
                        {
                            if (DelInvalidWaveData.wavevalue[0][liv_dimenson] > *(lfv_xyz + liv_dimenson)*1000) //第一个数据峰值大于第三个数据
                            {
                                liv_axischoice = liv_dimenson;
                                liv_size = 2;
                                lfv_output[0] = DelInvalidWaveData.wavevalue[0][liv_dimenson];
                                liv_outputtrend[0] = DelInvalidWaveData.wavevaluetrend[0][liv_dimenson];
                                lfv_output[1] = DelInvalidWaveData.wavevalue[1][liv_dimenson];
                                liv_outputtrend[1] = DelInvalidWaveData.wavevaluetrend[1][liv_dimenson];
															  if(liv_dimenson == 1)
								                {		
									                  g_presentvalidnum = g_size[1];
								                }
                                TemplateCompare(liv_axischoice, liv_size, lfv_output, liv_outputtrend);
                            }
                            else //第一个数据峰值小于或等于第三个数据
                            {
                                 //第一个数据的峰值比第二个数据谷值大0.5以上
                                if ((DelInvalidWaveData.wavevalue[0][liv_dimenson] - DelInvalidWaveData.wavevalue[1][liv_dimenson]) > lfv_setdist)
                                {
                                    liv_axischoice = liv_dimenson;
                                    liv_size = 2;
                                    lfv_output[0] = DelInvalidWaveData.wavevalue[0][liv_dimenson];
                                    liv_outputtrend[0] = DelInvalidWaveData.wavevaluetrend[0][liv_dimenson];
                                    lfv_output[1] = DelInvalidWaveData.wavevalue[1][liv_dimenson];
                                    liv_outputtrend[1] = DelInvalidWaveData.wavevaluetrend[1][liv_dimenson];
																	  if(liv_dimenson == 1)
								                    {		
									                      g_presentvalidnum = g_size[1];
								                    }
                                    TemplateCompare(liv_axischoice, liv_size, lfv_output, liv_outputtrend);
                                }
                            }
                        }
                        else  //第一个数据与第三个数据同为波谷
                        {
                            if (DelInvalidWaveData.wavevalue[0][liv_dimenson] < *(lfv_xyz + liv_dimenson)*1000) //第一个数据谷值小于第三个数据谷值
                            {
                                liv_axischoice = liv_dimenson;
                                liv_size = 2;
                                lfv_output[0] = DelInvalidWaveData.wavevalue[0][liv_dimenson];
                                liv_outputtrend[0] = DelInvalidWaveData.wavevaluetrend[0][liv_dimenson];
                                lfv_output[1] = DelInvalidWaveData.wavevalue[1][liv_dimenson];
                                liv_outputtrend[1] = DelInvalidWaveData.wavevaluetrend[1][liv_dimenson];
															  if(liv_dimenson == 1)
								                {		
									                  g_presentvalidnum = g_size[1];
								                }
                                TemplateCompare(liv_axischoice, liv_size, lfv_output, liv_outputtrend); 
                            }
                            else //第一个数据谷值大于或等于第三个数据
                            {
                                if ((DelInvalidWaveData.wavevalue[1][liv_dimenson] - DelInvalidWaveData.wavevalue[0][liv_dimenson]) > lfv_setdist)//else  //第二个数据峰值比第一个数据的谷值大0.5以上
                                {
                                    liv_axischoice = liv_dimenson;
                                    liv_size = 2;
                                    lfv_output[0] = DelInvalidWaveData.wavevalue[0][liv_dimenson];
                                    liv_outputtrend[0] = DelInvalidWaveData.wavevaluetrend[0][liv_dimenson];
                                    lfv_output[1] = DelInvalidWaveData.wavevalue[1][liv_dimenson];
                                    liv_outputtrend[1] = DelInvalidWaveData.wavevaluetrend[1][liv_dimenson];
																	  if(liv_dimenson == 1)
								                    {		
									                      g_presentvalidnum = g_size[1];
								                    }
                                    TemplateCompare(liv_axischoice, liv_size, lfv_output, liv_outputtrend);
                                }
                            }
                        }
                    }
                    else   //第一个数据与第三个数据的趋势属性不同
                    {
                        if (DelInvalidWaveData.wavevaluetrend[0][liv_dimenson] > 0) //第一个数据波峰，第三个数据波谷
                        {
                            if (DelInvalidWaveData.wavevalue[0][liv_dimenson] > DelInvalidWaveData.wavevalue[1][liv_dimenson]) //第一个峰值大于第二个峰值
                            {
                                liv_axischoice = liv_dimenson;
                                liv_size = 1;
                                lfv_output[0] = DelInvalidWaveData.wavevalue[0][liv_dimenson];
                                liv_outputtrend[0] = DelInvalidWaveData.wavevaluetrend[0][liv_dimenson];
                                if(liv_dimenson == 1)
								                {		
									                  g_presentvalidnum = g_size[0];
								                }															
                                TemplateCompare(liv_axischoice, liv_size, lfv_output, liv_outputtrend);
                            }
                            else   //第一个峰值小于或等于第二个峰值
                            {
                                liv_axischoice = liv_dimenson;                               
                                liv_size = 1;
                                lfv_output[0] = DelInvalidWaveData.wavevalue[1][liv_dimenson];
                                liv_outputtrend[0] = DelInvalidWaveData.wavevaluetrend[1][liv_dimenson];
															  if(liv_dimenson == 1)
								                {		
									                  g_presentvalidnum = g_size[1];
								                }
                                TemplateCompare(liv_axischoice, liv_size, lfv_output, liv_outputtrend);
                            }
                        }
                        else  //第一个数据波谷，第三个数据波峰
                        {
                            if (DelInvalidWaveData.wavevalue[0][liv_dimenson] < DelInvalidWaveData.wavevalue[1][liv_dimenson]) //第一个谷值小于第二个谷值
                            {
                                liv_axischoice = liv_dimenson;
                                liv_size = 1;
                                lfv_output[0] = DelInvalidWaveData.wavevalue[0][liv_dimenson];
                                liv_outputtrend[0] = DelInvalidWaveData.wavevaluetrend[0][liv_dimenson];
															  if(liv_dimenson == 1)
								                {		
									                  g_presentvalidnum = g_size[0];
								                }
                                TemplateCompare(liv_axischoice, liv_size, lfv_output, liv_outputtrend);
                            }
                            else   //第一个谷值大于或等于第二个谷值
                            {
                                liv_axischoice = liv_dimenson;
                                liv_size = 1;
                                lfv_output[0] = DelInvalidWaveData.wavevalue[1][liv_dimenson];
                                liv_outputtrend[0] = DelInvalidWaveData.wavevaluetrend[1][liv_dimenson];
															  if(liv_dimenson == 1)
								                {		
									                  g_presentvalidnum = g_size[1];
								                }
                                TemplateCompare(liv_axischoice, liv_size, lfv_output, liv_outputtrend);
                            }
                        }
                    }
                    DelInvalidWaveData.wavevalue[0][liv_dimenson] = *(lfv_xyz + liv_dimenson) * 1000;
                    DelInvalidWaveData.wavevaluetrend[0][liv_dimenson] = *(liv_xyztrend + liv_dimenson);
                    DelInvalidWaveData.delwavestep[liv_dimenson] = '1';
										g_size[0] = g_num;
                }
            }
            DelInvalidWaveData.pointnumb[liv_dimenson] = 0;
        }
        else
        {
            DelInvalidWaveData.pointnumb[liv_dimenson]++;
        }
    }
}


void TemplateCompare(int8_t liv_axischoice, uint8_t liv_size, float* lfv_in, int8_t* liv_intrend)
{  
    if (g_strokechoice == 'u')
    {     
			  BackStrokeCount(liv_axischoice, liv_size, lfv_in, liv_intrend);
        BackStrokeTemplateData.partwavecount[liv_axischoice] += liv_size;
        BackStrokeTemplateData.sumwavecount[liv_axischoice] += liv_size;
        BreastStrokeCount(liv_axischoice, liv_size, lfv_in, liv_intrend);
        BreastStrokeTemplateData.partwavecount[liv_axischoice] += liv_size;
        BreastStrokeTemplateData.sumwavecount[liv_axischoice] += liv_size;
        ButterflyStrokeCount(liv_axischoice, liv_size, lfv_in, liv_intrend);
        ButterflyStrokeTemplateData.partwavecount[liv_axischoice] += liv_size;
        ButterflyStrokeTemplateData.sumwavecount[liv_axischoice] += liv_size;
        FreeStyleCount(liv_axischoice, liv_size, lfv_in, liv_intrend);
        FreeStyleTemplateData.partwavecount[liv_axischoice] += liv_size; 
        FreeStyleTemplateData.sumwavecount[liv_axischoice] += liv_size;
				uint8_t liv_minwavecount = 0;
				BackStrokeWaveCountSet();			 
				liv_minwavecount = g_swimpresentwavecount;				
				BreastStrokeWaveCountSet();
			  if(g_swimpresentwavecount < liv_minwavecount)
				{
					  liv_minwavecount = g_swimpresentwavecount;
				}
				ButterflyStrokeWaveCountSet();
			  if(g_swimpresentwavecount < liv_minwavecount)
				{
					  liv_minwavecount = g_swimpresentwavecount;
				}
				FreeStyleWaveCountSet();
			  if(g_swimpresentwavecount < liv_minwavecount)
				{
					  liv_minwavecount = g_swimpresentwavecount;
				}
				g_swimpresentwavecount = liv_minwavecount;
			  uint8_t liv_maxpartperiod_back;			
        if(BackStrokeTemplateData.partperiodcount[1] > BackStrokeTemplateData.partperiodcount[2]) 
			  { 
						liv_maxpartperiod_back = BackStrokeTemplateData.partperiodcount[1];
			  }							
				else
				{
					  liv_maxpartperiod_back = BackStrokeTemplateData.partperiodcount[2];
				}
        uint8_t liv_maxpartperiod_breast;
        if(BreastStrokeTemplateData.partperiodcount[0] > BreastStrokeTemplateData.partperiodcount[1]) 
			  {
						liv_maxpartperiod_breast = BreastStrokeTemplateData.partperiodcount[0] ;
				}							
				else
				{
						liv_maxpartperiod_breast = BreastStrokeTemplateData.partperiodcount[1];
				}
        uint8_t liv_maxpartperiod_butter;
        if(ButterflyStrokeTemplateData.partperiodcount[1] > ButterflyStrokeTemplateData.partperiodcount[2]) 
				{
					  liv_maxpartperiod_butter = ButterflyStrokeTemplateData.partperiodcount[1];
				}							
			  else
				{
						liv_maxpartperiod_butter = ButterflyStrokeTemplateData.partperiodcount[2];
				}
        uint8_t liv_maxpartperiod_free;
        if(FreeStyleTemplateData.partperiodcount[1] > FreeStyleTemplateData.partperiodcount[2])
				{							
            liv_maxpartperiod_free = FreeStyleTemplateData.partperiodcount[1];
				} 
			  else
			  {							
						liv_maxpartperiod_free  = FreeStyleTemplateData.partperiodcount[2];
				}
        //里面的40是四种泳姿每圈y轴峰谷数最低值，也是每圈所有轴的峰谷数的最低值，是仰泳的y轴。
        if (BackStrokeTemplateData.partwavecount[1] >= 40 && BackStrokeTemplateData.partwavecount[1] <= 43 && BreastStrokeTemplateData.partwavecount[1] >= 40 && BreastStrokeTemplateData.partwavecount[1] <= 43
            && ButterflyStrokeTemplateData.partwavecount[1] >= 40 && ButterflyStrokeTemplateData.partwavecount[1] <= 43 && FreeStyleTemplateData.partwavecount[1] >= 40 && FreeStyleTemplateData.partwavecount[1] <= 43)
        {
            if (liv_maxpartperiod_free >= liv_maxpartperiod_butter && liv_maxpartperiod_free >= liv_maxpartperiod_breast && liv_maxpartperiod_free >= liv_maxpartperiod_back)
            {
                g_strokechoice = 'z';
            }
            else if (liv_maxpartperiod_breast >= liv_maxpartperiod_butter && liv_maxpartperiod_breast >= liv_maxpartperiod_free && liv_maxpartperiod_breast >= liv_maxpartperiod_back)
            {
                g_strokechoice = 'w';
            }
            else if (liv_maxpartperiod_back >= liv_maxpartperiod_butter && liv_maxpartperiod_back >= liv_maxpartperiod_free && liv_maxpartperiod_back >= liv_maxpartperiod_breast)
            {
                g_strokechoice = 'y';
            }
            else if (liv_maxpartperiod_butter >= liv_maxpartperiod_breast && liv_maxpartperiod_butter >= liv_maxpartperiod_free && liv_maxpartperiod_butter >= liv_maxpartperiod_back)
            {
                g_strokechoice = 'd';
            }
        }

    }
    else if (g_strokechoice == 'y')
    {
        /*
        lfv_back_periodwaves_y = 2;
        lfv_back_periodwaves_z = 3.2;
        lfv_back_circleperiods_max = 25;
        lfv_back_circleperiods_above = 20;
        lfv_back_circleperiods_min = 15;

        lfv_back_periodwaves_y * lfv_back_circleperiods_min = 30
        lfv_back_periodwaves_y * lfv_back_circleperiods_above = 40
        lfv_back_periodwaves_y * lfv_back_circleperiods_max = 50

        lfv_back_periodwaves_z * lfv_back_circleperiods_min = 48
        lfv_back_periodwaves_z * lfv_back_circleperiods_above = 64
        lfv_back_periodwaves_z * lfv_back_circleperiods_max = 80
        */
        BackStrokeCount(liv_axischoice, liv_size, lfv_in, liv_intrend);
        BackStrokeTemplateData.partwavecount[liv_axischoice] += liv_size;
        BackStrokeTemplateData.sumwavecount[liv_axischoice] += liv_size;
				BackStrokeWaveCountSet();
        if (BackStrokeTemplateData.partwavecount[1] >= 30 && BackStrokeTemplateData.partwavecount[2] >= 48)
        {
            if (BackStrokeTemplateData.continuityflag[1] == 'f' && BackStrokeTemplateData.continuityflag[2] == 'f'
                && (BackStrokeTemplateData.partwavecount[1] <= 50 || BackStrokeTemplateData.partwavecount[2] <= 80))
            {
                if ((BackStrokeTemplateData.partperiodcount[1] >= 17 && BackStrokeTemplateData.partperiodcount[1] <= 23)
                    || (BackStrokeTemplateData.partperiodcount[2] >= 17 && BackStrokeTemplateData.partperiodcount[2] <= 23))
                {
                    BackStrokeCircleSet();
                }
                else if ((BackStrokeTemplateData.partwavecount[1] >= 40 && BackStrokeTemplateData.partwavecount[1] <= 43)
                    || (BackStrokeTemplateData.partwavecount[2] >= 64 && BackStrokeTemplateData.partwavecount[2] <= 67))
                {
                    BackStrokeCircleSet();
										
                }
            }
            else if (BackStrokeTemplateData.partwavecount[1] >= 50 && BackStrokeTemplateData.partwavecount[2] >= 80)
            {
                BackStrokeCircleSet();
            }
        }
    }

    else if (g_strokechoice == 'w')
    {
        /*
        lfv_breast_periodwaves_x = 3.5;
        lfv_breast_periodwaves_y = 3.5;
        lfv_breast_circleperiods_max = 35;
        lfv_breast_circleperiods_above = 25;
        lfv_breast_circleperiods_min = 15;

        lfv_breast_periodwaves_x * lfv_breast_circleperiods_min = 52
        lfv_breast_periodwaves_x * lfv_breast_circleperiods_above = 87
        lfv_breast_periodwaves_x * lfv_breast_circleperiods_max = 122

        lfv_breast_periodwaves_y * lfv_breast_circleperiods_min = 52
        lfv_breast_periodwaves_y * lfv_breast_circleperiods_above = 87
        lfv_breast_periodwaves_y * lfv_breast_circleperiods_max = 122
        */
        BreastStrokeCount(liv_axischoice, liv_size, lfv_in, liv_intrend);
        BreastStrokeTemplateData.partwavecount[liv_axischoice] += liv_size;
        BreastStrokeTemplateData.sumwavecount[liv_axischoice] += liv_size;
				BreastStrokeWaveCountSet();
        if (BreastStrokeTemplateData.partwavecount[0] >= 52 && BreastStrokeTemplateData.partwavecount[1] >= 52)
        {
            if (BreastStrokeTemplateData.continuityflag[0] == 'f' && BreastStrokeTemplateData.continuityflag[1] == 'f'
                &&( BreastStrokeTemplateData.partwavecount[0] <= 122 || BreastStrokeTemplateData.partwavecount[1] <= 122))
            {
                if ((BreastStrokeTemplateData.partperiodcount[0] >= 17 && BreastStrokeTemplateData.partperiodcount[0] <= 28)
                    || (BreastStrokeTemplateData.partperiodcount[1] >= 17 && BreastStrokeTemplateData.partperiodcount[1] <= 28))
                {                    
                    BreastStrokeCircleSet();
                }
                else if ((BreastStrokeTemplateData.partwavecount[0] >= 87 && BreastStrokeTemplateData.partwavecount[0] <= 90)
                    || (BreastStrokeTemplateData.partwavecount[1] >= 87 && BreastStrokeTemplateData.partwavecount[1] <= 90))
                {
                    BreastStrokeCircleSet();
                }
            }
            else if (BreastStrokeTemplateData.partwavecount[0] >= 122 && BreastStrokeTemplateData.partwavecount[1] >= 122)
            {
                BreastStrokeCircleSet();
            }
        }
    }

    else if (g_strokechoice == 'd')
    {
        /*
        lfv_butter_periodwaves_y = 3.4;
        lfv_butter_periodwaves_z = 4.5;
        lfv_butter_circleperiods_max = 25;
        lfv_butter_circleperiods_above = 20;
        lfv_butter_circleperiods_min = 15;

        lfv_butter_periodwaves_y * lfv_butter_circleperiods_min = 51
        lfv_butter_periodwaves_y * lfv_butter_circleperiods_above = 68
        lfv_butter_periodwaves_y * lfv_butter_circleperiods_max = 85

        lfv_butter_periodwaves_z * lfv_butter_circleperiods_min = 67
        lfv_butter_periodwaves_z * lfv_butter_circleperiods_above = 90
        lfv_butter_periodwaves_z * lfv_butter_circleperiods_max = 112
        */
        ButterflyStrokeCount(liv_axischoice, liv_size, lfv_in, liv_intrend);
        ButterflyStrokeTemplateData.partwavecount[liv_axischoice] += liv_size;
        ButterflyStrokeTemplateData.sumwavecount[liv_axischoice] += liv_size;
			  ButterflyStrokeWaveCountSet();
        if (ButterflyStrokeTemplateData.partwavecount[1] >= 51 && ButterflyStrokeTemplateData.partwavecount[2] >= 67)
        {
            if (ButterflyStrokeTemplateData.continuityflag[1] == 'f' && ButterflyStrokeTemplateData.continuityflag[2] == 'f'
                && (ButterflyStrokeTemplateData.partwavecount[1] <= 85 ||  ButterflyStrokeTemplateData.partwavecount[2] <= 112))
            {
                if ((ButterflyStrokeTemplateData.partperiodcount[1] >= 17 && ButterflyStrokeTemplateData.partperiodcount[1] <= 23)
                    || (ButterflyStrokeTemplateData.partperiodcount[2] >= 17 && ButterflyStrokeTemplateData.partperiodcount[2] <= 23))
                {
                    ButterflyStrokeCircleSet();
                }
                else if ((ButterflyStrokeTemplateData.partwavecount[1] >= 68 && ButterflyStrokeTemplateData.partwavecount[1] <= 71)
                    || (ButterflyStrokeTemplateData.partwavecount[2] >= 90 && ButterflyStrokeTemplateData.partwavecount[2] <= 93))
                {
                    ButterflyStrokeCircleSet();
                }
            }
            else if (ButterflyStrokeTemplateData.partwavecount[1] >= 85 && ButterflyStrokeTemplateData.partwavecount[2] >= 112)
            {
                ButterflyStrokeCircleSet();
            }
        }
    }

       
    else if (g_strokechoice == 'z')
    {
        /*
        lfv_free_periodwaves_y = 4;
        lfv_free_periodwaves_z = 4;
        lfv_free_circleperiods_max = 30;
        lfv_free_circleperiods_above = 23;
        lfv_free_circleperiods_min = 15;

        lfv_free_periodwaves_y * lfv_free_circleperiods_min = 60
        lfv_free_periodwaves_y * lfv_free_circleperiods_above = 92
        lfv_free_periodwaves_y * lfv_free_circleperiods_max = 120

        lfv_free_periodwaves_z * lfv_free_circleperiods_min = 60
        lfv_free_periodwaves_z * lfv_free_circleperiods_above = 92
        lfv_free_periodwaves_z * lfv_free_circleperiods_max = 120
        */
        FreeStyleCount(liv_axischoice, liv_size, lfv_in, liv_intrend);
        FreeStyleTemplateData.partwavecount[liv_axischoice] += liv_size;
        FreeStyleTemplateData.sumwavecount[liv_axischoice] += liv_size;
			  FreeStyleWaveCountSet();
        if (FreeStyleTemplateData.partwavecount[1] >= 60 && FreeStyleTemplateData.partwavecount[2] >= 60)
        {
            if (FreeStyleTemplateData.continuityflag[1] == 'f' && FreeStyleTemplateData.continuityflag[2] == 'f'
                &&  (FreeStyleTemplateData.partwavecount[1] <= 120 ||  FreeStyleTemplateData.partwavecount[2] <= 120))
            {
                if ((FreeStyleTemplateData.partperiodcount[1] >= 20 && FreeStyleTemplateData.partperiodcount[1] <= 26)
                    || (FreeStyleTemplateData.partperiodcount[2] >= 20 && FreeStyleTemplateData.partperiodcount[2] <= 26))
                {
                    FreeStyleCircleSet();
                }
                else if ((FreeStyleTemplateData.partwavecount[1] >= 92 && FreeStyleTemplateData.partwavecount[1] <= 95)
                    || (FreeStyleTemplateData.partwavecount[2] >= 92 && FreeStyleTemplateData.partwavecount[2] <= 95))
                {
                    FreeStyleCircleSet();
                }
            }
            else if (FreeStyleTemplateData.partwavecount[1] >= 120 && FreeStyleTemplateData.partwavecount[2] >= 120)
            {
                FreeStyleCircleSet();
            }
        }
    }
		
		uint8_t liv_wavecountdiff;
		liv_wavecountdiff = g_swimpresentwavecount - g_swimpastwavecount;
		uint16_t liv_validnumdiff;
		liv_validnumdiff = g_presentvalidnum - g_pastvalidnum;
		if(liv_wavecountdiff>0 && liv_validnumdiff > 0)
		{
			  uint8_t liv_pervalidnum;
			  liv_pervalidnum = (int)(liv_validnumdiff/liv_wavecountdiff);
			  if(liv_pervalidnum >= TFILTER_POINTS)
			  {
				    g_swimresultwavecount += liv_wavecountdiff;
			  }
			  else
			  {
				    g_swimresultwavecount += (int)(liv_validnumdiff/TFILTER_POINTS);
			  }
			  g_swimpastwavecount = g_swimpresentwavecount;
			  g_pastvalidnum = g_presentvalidnum;
		  }
}


void BackStrokeCount(int8_t liv_axischoice, uint8_t liv_size, float* lfv_in, int8_t* liv_intrend)
{
    float lfv_peakmax, lfv_peakmin, lfv_midmax, lfv_midmin, lfv_valleymax, lfv_valleymin;
    if (liv_axischoice == 1)  //针对y轴的模版
    {
        lfv_peakmax = 3;
        lfv_peakmin = 1.6;
        lfv_valleymax = -0.5;
        lfv_valleymin = -1.35;
        for (uint8_t liv_i = 0; liv_i < liv_size; liv_i++)
        {
            if (BackStrokeTemplateData.periodstep[1] == '0')
            {
                if (*(liv_intrend + liv_i ) < 0 && *(lfv_in + liv_i ) >= lfv_valleymin && *(lfv_in + liv_i ) <= lfv_valleymax)
                {
                    BackStrokeTemplateData.periodstep[1] = '1';
                }
            }
            else if (BackStrokeTemplateData.periodstep[1] == '1')
            {
                if (*(lfv_in + liv_i ) >= lfv_peakmin && *(lfv_in + liv_i ) <= lfv_peakmax)
                {
                    BackStrokeTemplateData.periodstep[1] = '2';
                    BackStrokeTemplateData.partperiodcount[1]++;
                    BackStrokeTemplateData.sumperiodcount[1]++;
                }
                else
                {
                    BackStrokeTemplateData.periodstep[1] = '0';
                    BackStrokeTemplateData.continuityflag[1] = 'f';
                }
            }
            else if (BackStrokeTemplateData.periodstep[1] == '2')
            {
                if (*(liv_intrend + liv_i ) < 0 && *(lfv_in + liv_i ) >= lfv_valleymin && *(lfv_in + liv_i ) <= lfv_valleymax)
                {
                    BackStrokeTemplateData.periodstep[1] = '1';
                    BackStrokeTemplateData.continuityflag[1] = 't';
                }
                else
                {
                    BackStrokeTemplateData.periodstep[1] = '0';
                    BackStrokeTemplateData.continuityflag[1] = 'f';
                }
            }
        }
    }
    else if (liv_axischoice == 2)  //针对z轴的模板
    {
        lfv_peakmax = 3;
        lfv_peakmin = 1.8;
        lfv_midmax = 1.9;
        lfv_midmin = -0.1;
        lfv_valleymax = -2;
        lfv_valleymin = -3;
        for (uint8_t liv_i = 0; liv_i < liv_size; liv_i++)
        {
            if (BackStrokeTemplateData.periodstep[2] == '0')
            {
                if (*(liv_intrend + liv_i ) > 0 && *(lfv_in + liv_i ) >= lfv_midmin && *(lfv_in + liv_i ) <= lfv_peakmax)
                {
                    BackStrokeTemplateData.periodstep[2] = '1';
                    if (*(lfv_in + liv_i ) > lfv_midmax)
                    {
                        g_backdoublemidflag_z = 's';
                    }
                    else if (*(lfv_in + liv_i ) < lfv_peakmin)
                    {
                        g_backdoublemidflag_z = 'd';
                        g_backdoublemidvalue_z = *(lfv_in + liv_i );
                    }
                    else
                    {
                         g_backdoublemidflag_z = 'u';
                        g_backdoublemidvalue_z = *(lfv_in + liv_i );
                    }
                }
            }
            else if (BackStrokeTemplateData.periodstep[2] == '1')
            {
                if (g_backdoublemidflag_z == 'd')
                {
                    if (*(lfv_in + liv_i ) >= lfv_midmin && *(lfv_in + liv_i ) <= lfv_midmax)
                    {
                        BackStrokeTemplateData.periodstep[2] = '2';
                    }
                    else
                    {
                        BackStrokeTemplateData.periodstep[2] ='0';
                        BackStrokeTemplateData.continuityflag[2] = 'f';
                    }
                }
                else if (g_backdoublemidflag_z == 's')
                {
                    if (*(lfv_in + liv_i ) >= lfv_valleymin && *(lfv_in + liv_i ) <= lfv_valleymax)
                    {
                        BackStrokeTemplateData.periodstep[2] = '2';
                        BackStrokeTemplateData.partperiodcount[2]++;
                        BackStrokeTemplateData.sumperiodcount[2]++;
                    }
                    else
                    {
                        BackStrokeTemplateData.periodstep[2] = '0';
                        BackStrokeTemplateData.continuityflag[2] = 'f';
                    }
                }
                else if (g_backdoublemidflag_z == 'u')
                {
                    if (*(lfv_in + liv_i ) >= lfv_midmin && *(lfv_in + liv_i ) <= lfv_midmax)
                    {
                        BackStrokeTemplateData.periodstep[2] = '2';
                        g_backdoublemidflag_z = 'd';
                    }
                    else if (*(lfv_in + liv_i ) >= lfv_valleymin && *(lfv_in + liv_i ) <= lfv_valleymax)
                    {
                        BackStrokeTemplateData.periodstep[2] = '2';
                        BackStrokeTemplateData.partperiodcount[2]++;
                        BackStrokeTemplateData.sumperiodcount[2]++;
                        g_backdoublemidflag_z = 's';
                    }
                    else
                    {
                        BackStrokeTemplateData.periodstep[2] = '0';
                        BackStrokeTemplateData.continuityflag[2] = 'f';
                    }
                }
            }
            else if (BackStrokeTemplateData.periodstep[2] == '2')
            {
                if (g_backdoublemidflag_z == 'd')
                {
                    if (*(lfv_in + liv_i ) >= lfv_peakmin && *(lfv_in + liv_i ) <= lfv_peakmax)
                    {
                        if (*(lfv_in + liv_i ) > g_backdoublemidvalue_z)
                        {
                            BackStrokeTemplateData.periodstep[2] = '3';
                        }
                        else
                        {
                            BackStrokeTemplateData.periodstep[2] = '1';
                            BackStrokeTemplateData.continuityflag[2] = 'f';                            
                        }
                        if (*(lfv_in + liv_i ) > lfv_midmax)
                        {
                            g_backdoublemidflag_z = 's';
                        }
                        else
                        {
                            g_backdoublemidflag_z = 'u';
                            g_backdoublemidvalue_z = *(lfv_in + liv_i );
                        }
                    }
                    else if (*(lfv_in + liv_i ) >= lfv_midmin && *(lfv_in + liv_i ) < lfv_peakmin)
                    {
                        BackStrokeTemplateData.periodstep[2] = '1';
                        BackStrokeTemplateData.continuityflag[2] = 'f';
                        g_backdoublemidflag_z = 'd';
                        g_backdoublemidvalue_z = *(lfv_in + liv_i );
                    }
                    else
                    {
                        BackStrokeTemplateData.periodstep[2] = '0';
                        BackStrokeTemplateData.continuityflag[2] = 'f';
                    }
                }
                else if (g_backdoublemidflag_z == 's')
                {
                    if (*(lfv_in + liv_i ) >= lfv_midmin && *(lfv_in + liv_i ) <= lfv_peakmax)
                    {
                        BackStrokeTemplateData.periodstep[2] = '1';
                        BackStrokeTemplateData.continuityflag[2] = 't';
                        if (*(lfv_in + liv_i ) > lfv_midmax)
                        {
                            g_backdoublemidflag_z = 's';
                        }
                        else if (*(lfv_in + liv_i ) < lfv_peakmin)
                        {
                            g_backdoublemidflag_z = 'd';
                            g_backdoublemidvalue_z = *(lfv_in + liv_i );
                        }
                        else
                        {
                            g_backdoublemidflag_z = 'u';
                            g_backdoublemidvalue_z = *(lfv_in + liv_i );
                        }
                    }
                    else
                    {
                        BackStrokeTemplateData.periodstep[2] = '0';
                        BackStrokeTemplateData.continuityflag[2] = 'f';
                    }
                }                
            }
            else if (BackStrokeTemplateData.periodstep[2] == '3')
            { 
                if (*(lfv_in + liv_i ) >= lfv_valleymin && *(lfv_in + liv_i ) <= lfv_valleymax)
                {
                    BackStrokeTemplateData.periodstep[2] = '4';
                    BackStrokeTemplateData.partperiodcount[2]++;
                    BackStrokeTemplateData.sumperiodcount[2]++;
                }
                else
                {
                    BackStrokeTemplateData.continuityflag[2] = 'f';
                    if (*(lfv_in + liv_i ) >= lfv_valleymin && *(lfv_in + liv_i ) <= lfv_valleymax && g_backdoublemidflag_z == 'u')
                    {
                        BackStrokeTemplateData.periodstep[2] = '2';
                    }
                    else
                    {
                        BackStrokeTemplateData.periodstep[2] = '0';
                    }
                }
            }
            else if (BackStrokeTemplateData.periodstep[2] == '4')
            {
                if (*(lfv_in + liv_i ) >= lfv_midmin && *(lfv_in + liv_i ) <= lfv_peakmax)
                {
                    BackStrokeTemplateData.periodstep[2] = '1';
                    BackStrokeTemplateData.continuityflag[2] = 't';
                    if (*(lfv_in + liv_i ) > lfv_midmax)
                    {
                        g_backdoublemidflag_z = 's';
                    }
                    else if (*(lfv_in + liv_i ) < lfv_peakmin)
                    {
                        g_backdoublemidflag_z = 'd';
                        g_backdoublemidvalue_z = *(lfv_in + liv_i );
                    }
                    else
                    {
                        g_backdoublemidflag_z = 'u';
                        g_backdoublemidvalue_z = *(lfv_in + liv_i );
                    }
                }
                else
                {
                    BackStrokeTemplateData.periodstep[2] = '0';
                    BackStrokeTemplateData.continuityflag[2] = 'f';
                }
            }
        }
    }
}


void BackStrokeCircleSet()
{
			g_swimresultcircle++;
			BackStrokeTemplateData.partwavecount[1] = 0;
			BackStrokeTemplateData.partwavecount[2] = 0;
			BackStrokeTemplateData.partperiodcount[1] = 0;
			BackStrokeTemplateData.partperiodcount[2] = 0;
}
void BackStrokeWaveCountSet()
{    
	  uint16_t liv_maxsumperiod_back;
    if (BackStrokeTemplateData.sumperiodcount[1] > BackStrokeTemplateData.sumperiodcount[2])
    {
				liv_maxsumperiod_back = BackStrokeTemplateData.sumperiodcount[1];
    }
		else
		{
        liv_maxsumperiod_back = BackStrokeTemplateData.sumperiodcount[2];
		}
    uint16_t liv_wavetoperiod_y_back = (int)(BackStrokeTemplateData.sumwavecount[1] / 2);
    uint16_t liv_wavetoperiod_z_back = (int)(BackStrokeTemplateData.sumwavecount[2] / 3.2);
    uint16_t liv_maxwavetoperiod_back;
    if (liv_wavetoperiod_y_back > liv_wavetoperiod_z_back)
    {
        liv_maxwavetoperiod_back = liv_wavetoperiod_y_back;
    }
    else
    {
        liv_maxwavetoperiod_back = liv_wavetoperiod_z_back;
    }
    if (liv_maxsumperiod_back > liv_maxwavetoperiod_back)
    {
        liv_maxwavetoperiod_back = liv_maxsumperiod_back;
    }
    g_swimpresentwavecount = liv_maxwavetoperiod_back;
}


void BreastStrokeCount(int8_t liv_axischoice, uint8_t liv_size, float* lfv_in, int8_t* liv_intrend)
{
    float lfv_peakmax, lfv_peakmin, lfv_midmax, lfv_midmin, lfv_valleymax, lfv_valleymin;
    if (liv_axischoice == 0)  //针对x轴的模板
    {
        lfv_peakmax = 2.5;
        lfv_peakmin = 0.5;
        lfv_midmax = -0.5;
        lfv_midmin = -2;
        lfv_valleymax = -1;
        lfv_valleymin = -3;
        for (uint8_t liv_i = 0; liv_i < liv_size; liv_i++)
        {
            if (BreastStrokeTemplateData.periodstep[0] == '0')   //进入蛙泳模板的第一步，蛙泳周期不连续
            {
                if ((*(liv_intrend + liv_i ) > 0) && (*(lfv_in + liv_i ) >= lfv_peakmin) && (*(lfv_in + liv_i ) <= lfv_peakmax))
                {
                    BreastStrokeTemplateData.periodstep[0] = '1';
                }
            }
            else if (BreastStrokeTemplateData.periodstep[0] == '1')  //进入蛙泳模板的第二步
            {
                if ((*(lfv_in + liv_i ) >= lfv_valleymin) && (*(lfv_in + liv_i ) <= lfv_valleymax))
                {
                    BreastStrokeTemplateData.periodstep[0] = '2';
                    BreastStrokeTemplateData.partperiodcount[0]++;
                    BreastStrokeTemplateData.sumperiodcount[0]++;
                }
                else
                {
                    BreastStrokeTemplateData.continuityflag[0] = 'f';
                    BreastStrokeTemplateData.periodstep[0] = '0';
                }
            }
            else if (BreastStrokeTemplateData.periodstep[0] == '2') //蛙泳模板进入可重复低谷阶段
            {
                if ((*(lfv_in + liv_i ) >= lfv_midmin) && (*(lfv_in + liv_i ) <= lfv_midmax))
                {
                    BreastStrokeTemplateData.periodstep[0] ='3';
                }
                else if ((*(liv_intrend + liv_i) > 0) && (*(lfv_in + liv_i) >= lfv_peakmin) && (*(lfv_in + liv_i) <= lfv_peakmax))
                {
                    BreastStrokeTemplateData.periodstep[0] = '1';
                    BreastStrokeTemplateData.continuityflag[0] = 't';
                }
                else
                {
                    BreastStrokeTemplateData.continuityflag[0] = 'f';
                    BreastStrokeTemplateData.periodstep[0] = '0';
                }
            }
            else if (BreastStrokeTemplateData.periodstep[0] == '3')
            {
                if ((*(lfv_in + liv_i ) >= lfv_valleymin) && (*(lfv_in + liv_i ) <= lfv_valleymax))
                {
                    BreastStrokeTemplateData.periodstep[0] = '4';
                }             
                else
                {
                    BreastStrokeTemplateData.continuityflag[0] = 'f';
                    BreastStrokeTemplateData.periodstep[0] = '0';
                }
            }
            else if (BreastStrokeTemplateData.periodstep[0] == '4')
            {
                if ((*(liv_intrend + liv_i ) > 0) && (*(lfv_in + liv_i ) >= lfv_peakmin) && (*(lfv_in + liv_i ) <= lfv_peakmax))
                {
                    BreastStrokeTemplateData.continuityflag[0] = 't';
                    BreastStrokeTemplateData.periodstep[0] = '1';
                }
                else
                {
                    BreastStrokeTemplateData.continuityflag[0] = 'f';
                    BreastStrokeTemplateData.periodstep[0] = '0';
                }
            }
        }
    }
    else if (liv_axischoice == 1)  //针对y轴的模板
    {
        lfv_peakmax = 3.6;
        lfv_peakmin = 1.75;
        lfv_midmax = 1.75;
        lfv_midmin = 0.3;
        lfv_valleymax = 0.5;
        lfv_valleymin = -0.65;
        for (uint8_t liv_i = 0; liv_i < liv_size; liv_i++)
        {
            if (BreastStrokeTemplateData.periodstep[1] == '0')   //进入蛙泳模板的第一步，蛙泳周期不连续
            {
                if ((*(liv_intrend + liv_i) > 0) && (*(lfv_in + liv_i) > lfv_peakmin) && (*(lfv_in + liv_i) < lfv_peakmax))
                {
                    BreastStrokeTemplateData.periodstep[1] = '1';
                }
            }
            else if (BreastStrokeTemplateData.periodstep[1] =='1')  //进入蛙泳模板的第二步
            {
                if ((*(lfv_in + liv_i) > lfv_valleymin) && (*(lfv_in + liv_i) < lfv_valleymax))
                {
                    BreastStrokeTemplateData.periodstep[1] = '2';
                    BreastStrokeTemplateData.partperiodcount[1]++;
                    BreastStrokeTemplateData.sumperiodcount[1]++;
                }
                else
                {
                    BreastStrokeTemplateData.continuityflag[1] = 'f';
                    BreastStrokeTemplateData.periodstep[1] = '0';
                }
            }
            else if (BreastStrokeTemplateData.periodstep[1] == '2') //蛙泳模板进入可重复低谷阶段
            {
                if ((*(lfv_in + liv_i) > lfv_midmin) && (*(lfv_in + liv_i) < lfv_midmax))
                {
                    BreastStrokeTemplateData.periodstep[1] = '3';
                }
                else if ((*(liv_intrend + liv_i) > 0) && (*(lfv_in + liv_i) > lfv_peakmin) && (*(lfv_in + liv_i) < lfv_peakmax))
                {
                    BreastStrokeTemplateData.periodstep[1] = '1';
                    BreastStrokeTemplateData.continuityflag[1] = 't';
                }
                else
                {
                    BreastStrokeTemplateData.continuityflag[1] = 'f';
                    BreastStrokeTemplateData.periodstep[1] = '0';
                }
            }
            else if (BreastStrokeTemplateData.periodstep[1] == '3')
            {
                if ((*(lfv_in + liv_i) > lfv_valleymin) && (*(lfv_in + liv_i) < lfv_valleymax))
                {
                    BreastStrokeTemplateData.periodstep[1] = '4';
                }
                else
                {
                    BreastStrokeTemplateData.continuityflag[1] = 'f';
                    BreastStrokeTemplateData.periodstep[1] = '0';
                }
            }
            else if (BreastStrokeTemplateData.periodstep[1] == '4')
            {
                if ((*(liv_intrend + liv_i) > 0) && (*(lfv_in + liv_i) > lfv_peakmin) && (*(lfv_in + liv_i) < lfv_peakmax))
                {
                    BreastStrokeTemplateData.continuityflag[1] = 't';
                    BreastStrokeTemplateData.periodstep[1] = '1';
                }
                else
                {
                    BreastStrokeTemplateData.continuityflag[1] = 'f';
                    BreastStrokeTemplateData.periodstep[1] = '0';
                }
            }
        }
    }  
}


void BreastStrokeCircleSet()
{
    g_swimresultcircle++;
    BreastStrokeTemplateData.partwavecount[0] = 0;
    BreastStrokeTemplateData.partwavecount[1] = 0;
    BreastStrokeTemplateData.partperiodcount[0] = 0;
    BreastStrokeTemplateData.partperiodcount[1] = 0;        	 	
}
void BreastStrokeWaveCountSet()
{
	  uint16_t liv_maxsumperiod_breast;
    if (BreastStrokeTemplateData.sumperiodcount[0] > BreastStrokeTemplateData.sumperiodcount[1])
    {
        liv_maxsumperiod_breast = BreastStrokeTemplateData.sumperiodcount[0];
    }
    else
    {
        liv_maxsumperiod_breast = BreastStrokeTemplateData.sumperiodcount[1];
    }
    uint16_t liv_wavetoperiod_y_breast = (int)(BreastStrokeTemplateData.sumwavecount[0] / 3.5);
    uint16_t liv_wavetoperiod_z_breast = (int)(BreastStrokeTemplateData.sumwavecount[1] / 3.5);
    uint16_t liv_maxwavetoperiod_breast;
    if (liv_wavetoperiod_y_breast > liv_wavetoperiod_z_breast)
    {
        liv_maxwavetoperiod_breast = liv_wavetoperiod_y_breast;
    }
    else
    {
        liv_maxwavetoperiod_breast = liv_wavetoperiod_z_breast;
    }
    if (liv_maxsumperiod_breast > liv_maxwavetoperiod_breast)
    {
        liv_maxwavetoperiod_breast = liv_maxsumperiod_breast;
    }
    g_swimpresentwavecount = liv_maxwavetoperiod_breast;
}
void ButterflyStrokeCount(int8_t liv_axischoice, uint8_t liv_size, float* lfv_in, int8_t* liv_intrend)
{
    float lfv_peakmax, lfv_peakmin, lfv_midmax, lfv_midmin, lfv_valleymax, lfv_valleymin;
    if (liv_axischoice == 1)   //针对y轴模板
    {
        lfv_peakmin = 6.2;
        lfv_midmax = 7.2;
        lfv_midmin = 3.7;
        lfv_valleymax = 1.3;
        lfv_valleymin = -0.5;
        for (uint8_t liv_i = 0; liv_i < liv_size; liv_i++)
        {
            if (ButterflyStrokeTemplateData.periodstep[1] == '0')
            {
                if (*(liv_intrend + liv_i) < 0 && *(lfv_in + liv_i) >= lfv_valleymin && *(lfv_in + liv_i) <= lfv_valleymax)
                {
                    ButterflyStrokeTemplateData.periodstep[1] = '1';
                }
            }
            else if (ButterflyStrokeTemplateData.periodstep[1] == '1')
            {
                if (*(lfv_in + liv_i) >= lfv_midmin)
                {
                    ButterflyStrokeTemplateData.periodstep[1] = '2';
                    if (*(lfv_in + liv_i) < lfv_peakmin)
                    {
                        g_buttermidflag_y = 'd';
                        g_buttermidvalue_y = *(lfv_in + liv_i);
                    }
                    else
                    {
                        ButterflyStrokeTemplateData.partperiodcount[1]++;
                        ButterflyStrokeTemplateData.sumperiodcount[1]++;
                        if (*(lfv_in + liv_i) > lfv_midmax)
                        {
                            g_buttermidflag_y = 's';
                        }
                        else
                        {
                            g_buttermidflag_y = 'u';
                            g_buttermidvalue_y = *(lfv_in + liv_i);
                        }
                    }
                }
                else
                {
                    ButterflyStrokeTemplateData.periodstep[1] = '0';
                    ButterflyStrokeTemplateData.continuityflag[1] = 'f';
                }
            }
            else if (ButterflyStrokeTemplateData.periodstep[1] == '2')
            {
                if (g_buttermidflag_y == 's')
                {
                    if (*(liv_intrend + liv_i) < 0 && *(lfv_in + liv_i) >= lfv_valleymin && *(lfv_in + liv_i) <= lfv_valleymax)
                    {
                        ButterflyStrokeTemplateData.periodstep[1] = '1';
                        ButterflyStrokeTemplateData.continuityflag[1] = 't';
                    }
                    else
                    {
                        ButterflyStrokeTemplateData.periodstep[1] = '0';
                        ButterflyStrokeTemplateData.continuityflag[1] = 'f';
                    }
                }
                else if (g_buttermidflag_y == 'd')
                {
                    if (*(lfv_in + liv_i) >= lfv_midmin  && *(lfv_in + liv_i) <= lfv_midmax)
                    {
                        ButterflyStrokeTemplateData.periodstep[1] = '3';
                    }
                    else
                    {
                        ButterflyStrokeTemplateData.continuityflag[1] = 'f';
                        if (*(liv_intrend + liv_i) < 0 && *(lfv_in + liv_i) >= lfv_valleymin && *(lfv_in + liv_i) <= lfv_valleymax)
                        {
                            ButterflyStrokeTemplateData.periodstep[1] = '1';

                        }
                        else
                        {
                            ButterflyStrokeTemplateData.periodstep[1] = '0';
                        }
                    }
                }
                else if (g_buttermidflag_y == 'u')
                {
                    if (*(liv_intrend + liv_i) < 0 && *(lfv_in + liv_i) >= lfv_valleymin && *(lfv_in + liv_i) <= lfv_valleymax)
                    {
                        ButterflyStrokeTemplateData.periodstep[1] = '1';
                        ButterflyStrokeTemplateData.continuityflag[1] = 't';
                        g_buttermidflag_y = 's';
                    }
                    else if (*(lfv_in + liv_i) >= lfv_midmin  && *(lfv_in + liv_i) <= lfv_midmax)
                    {
                        ButterflyStrokeTemplateData.periodstep[1] = '3';
                        g_buttermidflag_y = 'd';

                    }
                    else
                    {
                        ButterflyStrokeTemplateData.periodstep[1] = '0';
                        ButterflyStrokeTemplateData.continuityflag[1] = 'f';
                    }
                }
            }
            else if (ButterflyStrokeTemplateData.periodstep[1] == '3')
            {
                if (*(lfv_in + liv_i) >= lfv_peakmin && *(lfv_in + liv_i) > g_buttermidvalue_y)
                {
                    ButterflyStrokeTemplateData.periodstep[1] = '4';
                    if (g_buttermidvalue_y < lfv_peakmin)
                    {
                        ButterflyStrokeTemplateData.partperiodcount[1]++;
                        ButterflyStrokeTemplateData.sumperiodcount[1]++;
                    }
                }
                else
                {
                    ButterflyStrokeTemplateData.periodstep[1] = '0';
                    ButterflyStrokeTemplateData.continuityflag[1] = 'f';
                }
            }
            else if (ButterflyStrokeTemplateData.periodstep[1] == '4')
            {
                if (*(liv_intrend + liv_i) < 0 && *(lfv_in + liv_i) >= lfv_valleymin && *(lfv_in + liv_i) <= lfv_valleymax)
                {
                    ButterflyStrokeTemplateData.periodstep[1] = '1';
                    ButterflyStrokeTemplateData.continuityflag[1] = 't';
                }
                else
                {
                    ButterflyStrokeTemplateData.periodstep[1] = '0';
                    ButterflyStrokeTemplateData.continuityflag[1] = 'f';
                }
            }
        }
                
    }
    else if (liv_axischoice == 2)   //针对z轴模板
    {
        lfv_peakmax = 4.2;
        lfv_peakmin = 2.6;
        lfv_midmax = 3.1;
        lfv_midmin = 0.3;
        lfv_valleymax = -0.4;
        lfv_valleymin = -3;
        for (uint8_t liv_i = 0; liv_i < liv_size; liv_i++)
        {
            if (ButterflyStrokeTemplateData.periodstep[2] == '0')
            {
                g_buttermidflag_z = 'u';
                if (*(liv_intrend + liv_i) < 0 && * (lfv_in + liv_i) >= lfv_midmin && *(lfv_in + liv_i) <= lfv_midmax)
                {
                    ButterflyStrokeTemplateData.periodstep[2] = '1';
                }
                else if (*(liv_intrend + liv_i) < 0 && *(lfv_in + liv_i) >= lfv_valleymin && *(lfv_in + liv_i) <= lfv_valleymax)
                {
                    ButterflyStrokeTemplateData.periodstep[2] = '1';
                    g_buttermidflag_z = 's';
                }
            }
            else if (ButterflyStrokeTemplateData.periodstep[2] == '1')
            {
                if (g_buttermidflag_z == 's')
                {
                    if (*(lfv_in + liv_i) > lfv_peakmin && *(lfv_in + liv_i) <= lfv_peakmax)
                    {
                        ButterflyStrokeTemplateData.periodstep[2] = '2';
                        ButterflyStrokeTemplateData.partperiodcount[2]++;
                        ButterflyStrokeTemplateData.sumperiodcount[2]++;
                    }
                    else
                    {
                        ButterflyStrokeTemplateData.periodstep[2] = '0';
                        ButterflyStrokeTemplateData.continuityflag[2] = 'f';
                    }
                }
                else
                {
                    if (*(lfv_in + liv_i) > lfv_midmin && *(lfv_in + liv_i) <= lfv_midmax)
                    {
                        ButterflyStrokeTemplateData.periodstep[2] = '2';
                        g_buttermidvalue_z = *(lfv_in + liv_i);
                    }
                    else
                    {
                        ButterflyStrokeTemplateData.periodstep[2] = '0';
                        ButterflyStrokeTemplateData.continuityflag[2] = 'f';
                    }
                }
            }
            else if (ButterflyStrokeTemplateData.periodstep[2] == '2')
            {
                if (g_buttermidflag_z == 's')
                {
                    if (*(liv_intrend + liv_i) < 0 && *(lfv_in + liv_i) >= lfv_midmin && *(lfv_in + liv_i) <= lfv_midmax)
                    {
                        ButterflyStrokeTemplateData.periodstep[2] = '1';
                        ButterflyStrokeTemplateData.continuityflag[2] = 't';
                        g_buttermidflag_z = 'u';
                    }
                    else if (*(liv_intrend + liv_i) < 0 && *(lfv_in + liv_i) >= lfv_valleymin && *(lfv_in + liv_i) <= lfv_valleymax)
                    {
                        ButterflyStrokeTemplateData.periodstep[2] = '1';
                        ButterflyStrokeTemplateData.continuityflag[2] = 't';
                        g_buttermidflag_z = 's';
                    }
                    else
                    {
                        ButterflyStrokeTemplateData.periodstep[2] = '0';
                        ButterflyStrokeTemplateData.continuityflag[2] = 'f';
                    }
                }
                else
                {
                    if ((*(lfv_in + liv_i) >= lfv_midmin && *(lfv_in + liv_i) < lfv_midmax))
                    {
                        ButterflyStrokeTemplateData.periodstep[2] = '3';
                        g_buttermidflag_z = 't';
                    }
                    else if (*(lfv_in + liv_i) >= lfv_valleymin && *(lfv_in + liv_i) <= lfv_valleymax)
                    {
                        ButterflyStrokeTemplateData.periodstep[2] = '3';
                        g_buttermidflag_z = 'd';
                    }
                    else
                    {
                        ButterflyStrokeTemplateData.periodstep[2] = '0';
                        ButterflyStrokeTemplateData.continuityflag[2] = 'f';
                    }
                }
            }
            else if (ButterflyStrokeTemplateData.periodstep[2] == '3')
            {
                
                if (g_buttermidflag_z == 'd')
                {
                    if (*(lfv_in + liv_i) >= lfv_peakmin && *(lfv_in + liv_i) <= lfv_peakmax && *(lfv_in + liv_i) > g_buttermidvalue_z)
                    {
                        ButterflyStrokeTemplateData.periodstep[2] = '4';
                        ButterflyStrokeTemplateData.partperiodcount[2]++;
                        ButterflyStrokeTemplateData.sumperiodcount[2]++;
                    } 
                    else
                    {
                        ButterflyStrokeTemplateData.periodstep[2] = '0';
                        ButterflyStrokeTemplateData.continuityflag[2] = 'f';
                    }
                }
                else if (g_buttermidflag_z == 't')
                {
                    if (*(lfv_in + liv_i ) >= lfv_midmin && *(lfv_in + liv_i ) < lfv_midmax)
                    {
                        ButterflyStrokeTemplateData.periodstep[2] = '4';
                        if (*(lfv_in + liv_i) > g_buttermidvalue_z)
                        {
                            g_buttermidvalue_z = *(lfv_in + liv_i);
                        }
                    }
                    else
                    {
                        ButterflyStrokeTemplateData.periodstep[2] ='0';
                        ButterflyStrokeTemplateData.continuityflag[2] = 'f';
                    }
                }
            }
            else if (ButterflyStrokeTemplateData.periodstep[2] == '4')
            {
                if (g_buttermidflag_z == 'd')
                {
                    if (*(liv_intrend + liv_i ) < 0 && *(lfv_in + liv_i ) >= lfv_midmin && *(lfv_in + liv_i ) <= lfv_midmax)
                    {
                        ButterflyStrokeTemplateData.periodstep[2] = '1';
                        ButterflyStrokeTemplateData.continuityflag[2] = 't';
                        g_buttermidflag_z = 'u';
                    }
                    else if (*(liv_intrend + liv_i) < 0 && *(lfv_in + liv_i) >= lfv_valleymin && *(lfv_in + liv_i) <= lfv_valleymax)
                    {
                        ButterflyStrokeTemplateData.periodstep[2] = '1';
                        ButterflyStrokeTemplateData.continuityflag[2] = 't';
                        g_buttermidflag_z = 's';
                    }
                    else
                    {
                        ButterflyStrokeTemplateData.periodstep[2] = '0';
                        ButterflyStrokeTemplateData.continuityflag[2] = 'f';
                    }
                }
                else if (g_buttermidflag_z == 't')
                {
                    if (*(lfv_in + liv_i ) >= lfv_valleymin && *(lfv_in + liv_i ) <= lfv_valleymax)
                    {
                        ButterflyStrokeTemplateData.periodstep[2] = '5';
                    }
                    else if (*(lfv_in + liv_i ) >= lfv_midmin && *(lfv_in + liv_i ) <= lfv_midmax)
                    {
                        ButterflyStrokeTemplateData.periodstep[2] = '3';
                        ButterflyStrokeTemplateData.continuityflag[2] = 'f';
                    }
                    else
                    {
                        ButterflyStrokeTemplateData.periodstep[2] = '0';
                        ButterflyStrokeTemplateData.continuityflag[2] = 'f';
                    }
                }                                
            }
            else if (ButterflyStrokeTemplateData.periodstep[2] == '5')
            {
                if ((*(lfv_in + liv_i ) >= lfv_peakmin && *(lfv_in + liv_i ) <= lfv_peakmax))
                {
                    ButterflyStrokeTemplateData.periodstep[2] = '6';
                    ButterflyStrokeTemplateData.partperiodcount[2]++;
                    ButterflyStrokeTemplateData.sumperiodcount[2]++;
                }
                else
                {
                    ButterflyStrokeTemplateData.periodstep[2] = '0';
                    ButterflyStrokeTemplateData.continuityflag[2] = 'f';
                }
            }
            else if (ButterflyStrokeTemplateData.periodstep[2] == '6')
            {
                if (*(liv_intrend + liv_i) < 0 && *(lfv_in + liv_i) >= lfv_midmin && *(lfv_in + liv_i) <= lfv_midmax)
                {
                    ButterflyStrokeTemplateData.periodstep[2] = '1';
                    ButterflyStrokeTemplateData.continuityflag[2] = 't';
                    g_buttermidflag_z = 'u';
                }
                else if (*(liv_intrend + liv_i) < 0 && *(lfv_in + liv_i) >= lfv_valleymin && *(lfv_in + liv_i) <= lfv_valleymax)
                {
                    ButterflyStrokeTemplateData.periodstep[2] = '1';
                    ButterflyStrokeTemplateData.continuityflag[2] = 't';
                    g_buttermidflag_z = 's';
                }
                else
                {
                    ButterflyStrokeTemplateData.periodstep[2] = '0';
                    ButterflyStrokeTemplateData.continuityflag[2] = 'f';
                }
            }
        }        
    }
}


void ButterflyStrokeCircleSet()
{
    g_swimresultcircle++;
    ButterflyStrokeTemplateData.partwavecount[1] = 0;
    ButterflyStrokeTemplateData.partwavecount[2] = 0;
    ButterflyStrokeTemplateData.partperiodcount[1] = 0;
    ButterflyStrokeTemplateData.partperiodcount[2] = 0;	
}
void ButterflyStrokeWaveCountSet()
{
	  uint16_t liv_maxsumperiod_butter;
    if (ButterflyStrokeTemplateData.sumperiodcount[1] > ButterflyStrokeTemplateData.sumperiodcount[2])
    {
        liv_maxsumperiod_butter = ButterflyStrokeTemplateData.sumperiodcount[1];
    }
    else
    {
        liv_maxsumperiod_butter = ButterflyStrokeTemplateData.sumperiodcount[2];
    }
    uint16_t liv_wavetoperiod_y_butter = (int)(ButterflyStrokeTemplateData.sumwavecount[1] / 3.4);
    uint16_t liv_wavetoperiod_z_butter = (int)(ButterflyStrokeTemplateData.sumwavecount[2] / 4.5);
    uint16_t liv_maxwavetoperiod_butter;
    if (liv_wavetoperiod_y_butter > liv_wavetoperiod_z_butter)
    {
        liv_maxwavetoperiod_butter = liv_wavetoperiod_y_butter;
    }
    else
    {
        liv_maxwavetoperiod_butter = liv_wavetoperiod_z_butter;
    }
    if (liv_maxsumperiod_butter > liv_maxwavetoperiod_butter)
    {
        liv_maxwavetoperiod_butter = liv_maxsumperiod_butter;
    }
    g_swimpresentwavecount = liv_maxwavetoperiod_butter;
}

void FreeStyleCount(int8_t liv_axischoice, uint8_t liv_size, float* lfv_in, int8_t* liv_intrend)
{
    float lfv_peakmax, lfv_peakmin, lfv_midmax, lfv_midmin, lfv_valleymax, lfv_valleymin;
    if (liv_axischoice == 1)  //针对y轴的模版
    {
        lfv_peakmax = 5;
        lfv_peakmin = 3.3;
        lfv_midmax = 3.2;
        lfv_midmin = 1.2;
        lfv_valleymax = 0.4;
        lfv_valleymin = -0.6;
        for (uint8_t liv_i = 0; liv_i < liv_size; liv_i++)
        {
            if (FreeStyleTemplateData.periodstep[1] == '0')
            {
                if (*(liv_intrend + liv_i ) > 0 && *(lfv_in + liv_i ) >= lfv_peakmin && *(lfv_in + liv_i ) <= lfv_peakmax)
                {
                    FreeStyleTemplateData.periodstep[1] = '1';
                }
            }
            else if (FreeStyleTemplateData.periodstep[1] == '1')
            {
                if (*(lfv_in + liv_i ) >= lfv_midmin && *(lfv_in + liv_i ) <= lfv_midmax)
                {
                    FreeStyleTemplateData.periodstep[1] = '2';
                }
                else
                {
                    FreeStyleTemplateData.periodstep[1] = '0';
                    FreeStyleTemplateData.continuityflag[1] = 'f';
                }
            }
            else if (FreeStyleTemplateData.periodstep[1] == '2')
            {
                if (*(lfv_in + liv_i ) >= lfv_midmin && *(lfv_in + liv_i ) <= lfv_midmax)
                {
                    FreeStyleTemplateData.periodstep[1] = '3';
                }
                else
                {
                    FreeStyleTemplateData.continuityflag[1] = 'f';
                    if (*(lfv_in + liv_i ) >= lfv_peakmin && *(lfv_in + liv_i ) <= lfv_peakmax)
                    {
                        FreeStyleTemplateData.periodstep[1] = '1';
                    }
                    else
                    {

                        FreeStyleTemplateData.periodstep[1] = '0';
                    }
                }                
            }
            else if (FreeStyleTemplateData.periodstep[1] == '3')
            {
                if (*(lfv_in + liv_i ) >= lfv_valleymin && *(lfv_in + liv_i ) <= lfv_valleymax)
                {
                    FreeStyleTemplateData.periodstep[1] = '4';
                    FreeStyleTemplateData.partperiodcount[1]++;
                    FreeStyleTemplateData.sumperiodcount[1]++;
                }
                else
                {
                    FreeStyleTemplateData.periodstep[1] = '0';
                    FreeStyleTemplateData.continuityflag[1] = 'f';                    
                }
            }
            else if (FreeStyleTemplateData.periodstep[1] == '4')
            {
                if (*(lfv_in + liv_i ) >= lfv_peakmin && *(lfv_in + liv_i ) <= lfv_peakmax)
                {
                    FreeStyleTemplateData.periodstep[1] = '1';
                    FreeStyleTemplateData.continuityflag[1] = 't';
                    
                }
                else
                {
                    FreeStyleTemplateData.periodstep[1] = '0';
                    FreeStyleTemplateData.continuityflag[1] = 'f';
                }
            }
        }
    }
    else if (liv_axischoice == 2)
    {
        lfv_peakmax = 3.6;
        lfv_peakmin = 1.7;
        lfv_midmax = 1.9;
        lfv_midmin = 0.7;
        lfv_valleymax = 0.8;
        lfv_valleymin = -0.7;
        for (uint8_t liv_i = 0; liv_i < liv_size; liv_i++)
        {
            if (FreeStyleTemplateData.periodstep[2] == '0')
            {
                if (*(liv_intrend + liv_i ) < 0 && *(lfv_in + liv_i ) >= lfv_valleymin && *(lfv_in + liv_i ) <= lfv_valleymax)
                {
                    FreeStyleTemplateData.periodstep[2] = '1';
                }
            }
            else if (FreeStyleTemplateData.periodstep[2] == '1')
            {
                if (*(lfv_in + liv_i ) >= lfv_midmin && *(lfv_in + liv_i ) <= lfv_midmax)
                {
                    FreeStyleTemplateData.periodstep[2] = '2';
                    g_freemidvalue_z = *(lfv_in + liv_i );
                }
                else
                {
                    FreeStyleTemplateData.periodstep[2] = '0';
                    FreeStyleTemplateData.continuityflag[2] = 'f';                  
                }
            }
            else if (FreeStyleTemplateData.periodstep[2] == '2')
            {
                if (*(lfv_in + liv_i ) >= lfv_valleymin && *(lfv_in + liv_i ) <= lfv_valleymax)
                {
                    FreeStyleTemplateData.periodstep[2] = '3';

                }
                else
                {
                    FreeStyleTemplateData.periodstep[2] = '0';
                    FreeStyleTemplateData.continuityflag[2] = 'f';
                }
            }
            else if (FreeStyleTemplateData.periodstep[2] == '3')
            {
                if (*(lfv_in + liv_i ) >= lfv_peakmin && *(lfv_in + liv_i ) <= lfv_peakmax)
                {
                    if (*(lfv_in + liv_i ) > g_freemidvalue_z)
                    {
                        FreeStyleTemplateData.periodstep[2] = '4';
                        FreeStyleTemplateData.partperiodcount[2]++;
                        FreeStyleTemplateData.sumperiodcount[2]++;
                    }
                    else
                    {
                        FreeStyleTemplateData.periodstep[2] = '2';
                        FreeStyleTemplateData.continuityflag[2] = 'f';
                    }
                }
                else 
                {                   
                    FreeStyleTemplateData.continuityflag[2] = 'f';
                    if (*(lfv_in + liv_i ) >= lfv_midmin && *(lfv_in + liv_i ) < lfv_peakmin)
                    {
                        FreeStyleTemplateData.periodstep[2] = '2';
                    }
                    else
                    {
                        FreeStyleTemplateData.periodstep[2] = '0';
                    }
                }
            }
            else if (FreeStyleTemplateData.periodstep[2] == '4')
            {
                if (*(liv_intrend + liv_i ) < 0 && *(lfv_in + liv_i ) >= lfv_valleymin && *(lfv_in + liv_i ) <= lfv_valleymax)
                {
                    FreeStyleTemplateData.periodstep[2] = '1';
                    FreeStyleTemplateData.continuityflag[2] = 't';
                }
                else
                {
                    FreeStyleTemplateData.periodstep[2] = '0';
                    FreeStyleTemplateData.continuityflag[2] = 'f';
                }
            }
        }
    }
}


void FreeStyleCircleSet()
{
    g_swimresultcircle++;
    FreeStyleTemplateData.partwavecount[1] = 0;
    FreeStyleTemplateData.partwavecount[2] = 0;
    FreeStyleTemplateData.partperiodcount[1] = 0;
    FreeStyleTemplateData.partperiodcount[2] = 0;	  
}
void FreeStyleWaveCountSet()
{
	  uint16_t liv_maxsumperiod_free;
    if (FreeStyleTemplateData.sumperiodcount[1] > FreeStyleTemplateData.sumperiodcount[2])
    {
        liv_maxsumperiod_free = FreeStyleTemplateData.sumperiodcount[1];
    }
    else
    {
        liv_maxsumperiod_free = FreeStyleTemplateData.sumperiodcount[2];
    }
    uint16_t liv_wavetoperiod_y_free = (int)(FreeStyleTemplateData.sumwavecount[1] / 4);
    uint16_t liv_wavetoperiod_z_free = (int)(FreeStyleTemplateData.sumwavecount[2] / 4);
    uint16_t liv_maxwavetoperiod_free;
    if (liv_wavetoperiod_y_free > liv_wavetoperiod_z_free)
    {
        liv_maxwavetoperiod_free = liv_wavetoperiod_y_free;
    }
    else
    {
        liv_maxwavetoperiod_free = liv_wavetoperiod_z_free;
    }
    if (liv_maxsumperiod_free > liv_maxwavetoperiod_free)
    {
        liv_maxwavetoperiod_free = liv_maxsumperiod_free;
    }
    g_swimpresentwavecount = liv_maxwavetoperiod_free;
}
void SwimAlgorithmEnd()
{
	  if (g_strokechoice == 'y' && (BackStrokeTemplateData.partwavecount[1]> 20 || BackStrokeTemplateData.partwavecount[2]>32))
    {
        g_swimresultcircle++;
    }
    else if (g_strokechoice == 'w' && (BreastStrokeTemplateData.partwavecount[0]>43 || BreastStrokeTemplateData.partwavecount[1]>43))
    {
        g_swimresultcircle++;
    }
    else if (g_strokechoice == 'd' && (ButterflyStrokeTemplateData.partwavecount[1]>34 || ButterflyStrokeTemplateData.partwavecount[2]>45))
    {
        g_swimresultcircle++;
    }
    else if (g_strokechoice == 'z' && (FreeStyleTemplateData.partwavecount[1] >46 || FreeStyleTemplateData.partwavecount[2]>46))
    {
        g_swimresultcircle++;
    }
}
//获取趟数
uint8_t SwimDataCircleResult()
{
	  return g_swimresultcircle;
}
//获取划数
uint16_t SwimDataWaveCountResult()
{
	
    return g_swimresultwavecount;
}

