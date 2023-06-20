#include <stdint.h>
#include <math.h>
#include <stdio.h>

#include "algo_gauss.h"

static double m_L0;   //���뾭��
static double m_num;	//����
/*��֪����ֵL�����뾭�߼��㷽��
1.����� n
���ȴ���n = (int)(L/6) + 1
���ȴ���n = (int)((L-1.5)/3) + 1
2.�����뾭�� L0
���ȴ���L0 = n * 6 - 3;
���ȴ���L0 = n * 3;
*/

/*-------------------------------------------------------------------------------
����:��ת��Ϊ����
����:��γ��(��λ:��)
����:����
˵��:****
 --------------------------------------------------------------------------------*/
static double Degree2Rad(double Degree)
{
	const double PI = 3.1415926535898;

	return (Degree * PI / 180.0);
}

//��ʼ����˹�㷨
void algo_gauss_init(uint8_t type, double lon)
{
	if(type == 6)
	{
		//���ȴ�
		m_num = (int)(lon/6) + 1;
		m_L0 = m_num * 6 - 3;
	}
	else
	{
		//���ȴ�
		m_num = (int)((lon-1.5)/3) + 1;
		m_L0 = m_num * 3;
	}
}

//���ش���
uint8_t algo_gauss_number_get(void)
{
	return (uint8_t)m_num;
}

//�������뾭��
double algo_gauss_center_get(void)
{
	return m_L0;
}

 /*-------------------------------------------------------------------------------
����:���������ת��Ϊ��˹ƽ�����꣨WGS84����
����:gpsPoint��GPS�����
����:��˹ƽ������λ��
˵��:�ڵ��ú���֮ǰ�����ȼ���L0
��˹������ʾ��ʱ�� �� y ����ֵ���� 500000�����ҽ�������ʾ����ǰ����λ
�������˹���� y = 90557.642������Ϊ 38 ������ʾֵΪ y = 38590557.642
--------------------------------------------------------------------------------*/
ST_GPSXY_POINT algo_gauss_point_get(ST_GPS_POINT gpsPoint)
{
	double MyL0;  //���뾭��
	double a,f,e2,e12;
	double A1,A2,A3,A4;
	double B,L;  //γ�Ȼ��ȣ����Ȼ���
	double X;   //�ɳ����γ��B�������߻���   (P106   5-41)
	double N;   //�����î��Ȧ���ʰ뾶
	double t,t2,m,m2,ng2,cosB,sinB;
	ST_GPSXY_POINT ret;

	//WGS84��������
	a = 6378137;  //���򳤰뾶
	f = 298.257223563;   //���ʵĵ��� (����:(a-b)/a)
	e2 = 1 - ((f - 1) / f) * ((f - 1) / f);        //��һƫ���ʵ�ƽ��
	e12 = (f / (f - 1)) * (f / (f - 1)) - 1;       //�ڶ�ƫ���ʵ�ƽ��

	// WGS84�����׼
	A1 = 6367449.14582;
	A2 = -16038.50866;
	A3 = 16.83261;
	A4 = -0.02198;

	//���㵱�����뾭�ߣ�3�ȴ�

	//MyL0 = ( (int32_t)( ( gpsPoint.sgp_lon - 1.5)/3) + 1 ) * 3;

	MyL0 = Degree2Rad(m_L0); //�����뾭��ת��Ϊ����
	B = Degree2Rad(gpsPoint.sgp_lat);   //γ��ת��Ϊ����
	L = Degree2Rad(gpsPoint.sgp_lon);   //����ת��Ϊ����

	X = A1 * B  + A2 * sin(2 * B) + A3 * sin(4 * B) + A4 * sin(6 * B);
	sinB = sin(B);
	cosB = cos(B);
	t = tan(B);
	t2 = t * t;

	N = a / sqrt(1 - e2 * sinB * sinB);
	m   =  cosB * (L - MyL0);
	m2 = m * m;
	ng2 = cosB * cosB * e2 / (1 - e2);
	ret.x = X + N * t *((0.5 + ((5 - t2 + 9 * ng2 + 4 * ng2 * ng2)/ 24.0 + (61 - 58 * t2 + t2 * t2) * m2 / 720.0) * m2)* m2);
	ret.y   = N * m * ( 1 + m2 * ((1 - t2 + ng2) / 6.0 + m2 * (5 - 18 * t2 + t2 * t2 + 14 * ng2 - 58 *  ng2 * t2) / 120.0));  

	return ret;
}

