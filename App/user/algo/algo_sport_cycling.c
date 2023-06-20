#include "algo_sport_cycling.h"


#if DEBUG_ENABLED == 1 && ALGO_SPORT_CYCING_LOG_ENABLED == 1
	#define ALGO_SPORT_CYCING_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define ALGO_SPORT_CYCING_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define ALGO_SPORT_CYCING_WRITESTRING(...)
	#define ALGO_SPORT_CYCING_PRINTF(...)		        
#endif


int16_t g_acc_data[3]; //加速度三轴数据
uint8_t g_acc_static_count;	//加速度三轴差值，g_acc_static_count在静止阈值内的连续时间计数,秒为单位
uint8_t g_acc_nonstatic_count; //加速度三轴差值，g_acc_nonstatic_count实在非静止内的连续时间计数，秒为单位
uint32_t g_data_diff[3]; //1秒时间内，三轴数据的差值总和，最后用于计算差值平均值
uint32_t g_nonstatic_data_diff[3]; //1秒时间内，三轴数据的差值超出静止阈值数据的求和，最后用于计算非阈值数据内的差值平均值
uint16_t g_static_len[3],g_len; //1秒时间内，g_static_len是静止阈值内的数据数目，g_len是数据数目的总和
uint8_t g_staticjudgment;	//保存是否是静止判断的结果

/*
骑行算法的初始化函数
*/
void algo_sport_cycling_init()
{
	memset(&g_acc_data,0,sizeof(g_acc_data));
	g_acc_static_count = 0;
	g_acc_nonstatic_count = 0;
	memset(&g_data_diff, 0, sizeof(g_data_diff));
	memset(&g_nonstatic_data_diff, 0, sizeof(g_nonstatic_data_diff));
	memset(&g_static_len, 0, sizeof(g_static_len));
	g_len = 0;
	g_staticjudgment = SPORT_FLAG_MOVE;
}

/*
骑行算法接收数据的接口函数
*/
void algo_sport_cycling_start(int16_t* liv_acc_data, const uint16_t liv_i, const uint16_t len)
{
	//防止一秒钟数据超过200组，影响整个工程
	if (len < SECOND_DATA_NUM_THREHOLD)
	{
			g_len = len / AXIS_NUM;
	}
	else
	{
			g_len = SECOND_DATA_NUM_THREHOLD / AXIS_NUM - 1;
	}

	//防止一秒钟数据超过200组，影响整个工程
  if (liv_i > SECOND_DATA_NUM_THREHOLD)
	{
		return;
	}

	staticjudgment_diff(liv_acc_data);	

	if (liv_i == len - 1)
	{
		staticjudgment_cycling();
		memset(&g_data_diff, 0, sizeof(g_data_diff));
		memset(&g_nonstatic_data_diff, 0, sizeof(g_nonstatic_data_diff));
		memset(&g_static_len, 0, sizeof(g_static_len));
	}
}

/*
骑行静止判断算法1,计算当前数据点与前一点的差值，并绝对差值和，静止阈值判断。
*/
void staticjudgment_diff(int16_t* liv_acc_data)
{
	int16_t liv_acc_data_diff[3];
	for (uint8_t liv_i = 0; liv_i < 3; liv_i++)
	{
		liv_acc_data_diff[liv_i] = abs(liv_acc_data[liv_i] - g_acc_data[liv_i]);
		
		if (liv_acc_data_diff[liv_i] <= ACC_STATIC_SINGLE_THRESHOLD)
		{
			g_static_len[liv_i]++;
		}
		else
		{
			g_nonstatic_data_diff[liv_i] += liv_acc_data_diff[liv_i];
		}
		g_data_diff[liv_i] += liv_acc_data_diff[liv_i];
		g_acc_data[liv_i] = liv_acc_data[liv_i];
	}
}

/*
骑行静止判断汇总
*/
void staticjudgment_cycling()
{
	uint8_t liv_tmp = 0;
	for (uint8_t liv_i = 0; liv_i < 3; liv_i++)
	{
		if (g_len != 0)
		{
			g_data_diff[liv_i] /= g_len;
		}
		else
		{
			return;
		}
		if (g_data_diff[liv_i] <= ACC_STATIC_MEAN_THRESHOLD)
		{
			liv_tmp++;
		}
		else
		{
			if (2*g_static_len[liv_i] > g_len)
			{
				float liv_left, liv_right;
				liv_left = g_nonstatic_data_diff[liv_i]/(float)(g_len - g_static_len[liv_i]);
				liv_right = (float)(g_len - g_static_len[liv_i]) *(-1000) / g_static_len[liv_i] + 800;
				if (liv_left <= liv_right)
				{
					liv_tmp++;
				}
			}
		}
		
	}

	if (liv_tmp >= STATIC_JUDGMENT_AXIS)
	{
		g_acc_static_count++;
		g_acc_nonstatic_count = 0;
	}
	else
	{
		g_acc_static_count = 0;
		g_acc_nonstatic_count++;
	}

	if (g_acc_static_count >= ACC_STATIC_DELAY_TIME)
	{
		//g_staticjudgment = STATIC_STATUS_CYCLING;
		
		g_staticjudgment = SPORT_FLAG_STATIS;
	}
	else if (g_acc_nonstatic_count >= ACC_STATIC_DELAY_TIME)
	{
		//g_staticjudgment = NONSTATIC_STATUS_CYCLING;

		g_staticjudgment = SPORT_FLAG_MOVE;
	}
}

/*
骑行静止判断的输出接口函数
*/
uint8_t staticjudgment_cycling_result()
{
	return g_staticjudgment;
}
