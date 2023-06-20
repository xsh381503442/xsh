#include "algo_sport_cycling.h"


#if DEBUG_ENABLED == 1 && ALGO_SPORT_CYCING_LOG_ENABLED == 1
	#define ALGO_SPORT_CYCING_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define ALGO_SPORT_CYCING_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define ALGO_SPORT_CYCING_WRITESTRING(...)
	#define ALGO_SPORT_CYCING_PRINTF(...)		        
#endif


int16_t g_acc_data[3]; //���ٶ���������
uint8_t g_acc_static_count;	//���ٶ������ֵ��g_acc_static_count�ھ�ֹ��ֵ�ڵ�����ʱ�����,��Ϊ��λ
uint8_t g_acc_nonstatic_count; //���ٶ������ֵ��g_acc_nonstatic_countʵ�ڷǾ�ֹ�ڵ�����ʱ���������Ϊ��λ
uint32_t g_data_diff[3]; //1��ʱ���ڣ��������ݵĲ�ֵ�ܺͣ�������ڼ����ֵƽ��ֵ
uint32_t g_nonstatic_data_diff[3]; //1��ʱ���ڣ��������ݵĲ�ֵ������ֹ��ֵ���ݵ���ͣ�������ڼ������ֵ�����ڵĲ�ֵƽ��ֵ
uint16_t g_static_len[3],g_len; //1��ʱ���ڣ�g_static_len�Ǿ�ֹ��ֵ�ڵ�������Ŀ��g_len��������Ŀ���ܺ�
uint8_t g_staticjudgment;	//�����Ƿ��Ǿ�ֹ�жϵĽ��

/*
�����㷨�ĳ�ʼ������
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
�����㷨�������ݵĽӿں���
*/
void algo_sport_cycling_start(int16_t* liv_acc_data, const uint16_t liv_i, const uint16_t len)
{
	//��ֹһ�������ݳ���200�飬Ӱ����������
	if (len < SECOND_DATA_NUM_THREHOLD)
	{
			g_len = len / AXIS_NUM;
	}
	else
	{
			g_len = SECOND_DATA_NUM_THREHOLD / AXIS_NUM - 1;
	}

	//��ֹһ�������ݳ���200�飬Ӱ����������
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
���о�ֹ�ж��㷨1,���㵱ǰ���ݵ���ǰһ��Ĳ�ֵ�������Բ�ֵ�ͣ���ֹ��ֵ�жϡ�
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
���о�ֹ�жϻ���
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
���о�ֹ�жϵ�����ӿں���
*/
uint8_t staticjudgment_cycling_result()
{
	return g_staticjudgment;
}
