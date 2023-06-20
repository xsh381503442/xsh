#include <stdint.h>
#include <math.h>
#include <stdio.h>

#include "algo_gauss.h"

static double m_L0;   //中央经线
static double m_num;	//带号
/*已知经度值L求中央经线计算方法
1.求带号 n
六度带：n = (int)(L/6) + 1
三度带：n = (int)((L-1.5)/3) + 1
2.求中央经线 L0
六度带：L0 = n * 6 - 3;
三度带：L0 = n * 3;
*/

/*-------------------------------------------------------------------------------
功能:度转换为弧度
参数:经纬度(单位:度)
返回:弧度
说明:****
 --------------------------------------------------------------------------------*/
static double Degree2Rad(double Degree)
{
	const double PI = 3.1415926535898;

	return (Degree * PI / 180.0);
}

//初始化高斯算法
void algo_gauss_init(uint8_t type, double lon)
{
	if(type == 6)
	{
		//六度带
		m_num = (int)(lon/6) + 1;
		m_L0 = m_num * 6 - 3;
	}
	else
	{
		//三度带
		m_num = (int)((lon-1.5)/3) + 1;
		m_L0 = m_num * 3;
	}
}

//返回带号
uint8_t algo_gauss_number_get(void)
{
	return (uint8_t)m_num;
}

//返回中央经线
double algo_gauss_center_get(void)
{
	return m_L0;
}

 /*-------------------------------------------------------------------------------
功能:将大地坐标转换为高斯平面坐标（WGS84椭球）
参数:gpsPoint：GPS坐标点
返回:高斯平面坐标位置
说明:在调用函数之前，请先计算L0
高斯坐标显示的时候 将 y 坐标值加上 500000，并且将带号显示在最前面两位
如输出高斯坐标 y = 90557.642，带号为 38 ，则显示值为 y = 38590557.642
--------------------------------------------------------------------------------*/
ST_GPSXY_POINT algo_gauss_point_get(ST_GPS_POINT gpsPoint)
{
	double MyL0;  //中央经线
	double a,f,e2,e12;
	double A1,A2,A3,A4;
	double B,L;  //纬度弧度，经度弧度
	double X;   //由赤道至纬度B的子午线弧长   (P106   5-41)
	double N;   //椭球的卯酉圈曲率半径
	double t,t2,m,m2,ng2,cosB,sinB;
	ST_GPSXY_POINT ret;

	//WGS84椭球数据
	a = 6378137;  //椭球长半径
	f = 298.257223563;   //扁率的倒数 (扁率:(a-b)/a)
	e2 = 1 - ((f - 1) / f) * ((f - 1) / f);        //第一偏心率的平方
	e12 = (f / (f - 1)) * (f / (f - 1)) - 1;       //第二偏心率的平方

	// WGS84椭球基准
	A1 = 6367449.14582;
	A2 = -16038.50866;
	A3 = 16.83261;
	A4 = -0.02198;

	//计算当地中央经线，3度带

	//MyL0 = ( (int32_t)( ( gpsPoint.sgp_lon - 1.5)/3) + 1 ) * 3;

	MyL0 = Degree2Rad(m_L0); //将中央经线转换为弧度
	B = Degree2Rad(gpsPoint.sgp_lat);   //纬度转换为弧度
	L = Degree2Rad(gpsPoint.sgp_lon);   //经度转换为弧度

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

