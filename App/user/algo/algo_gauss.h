#ifndef ALGO_GAUSS_H
#define	ALGO_GAUSS_H

typedef struct
{
	double x;  // 高斯X坐标
	double y;  // 高斯Y坐标
}ST_GPSXY_POINT;

// GPS点
typedef struct 
{
	double sgp_lon;  // 经度
	double sgp_lat;  // 纬度
}ST_GPS_POINT;

extern void algo_gauss_init(uint8_t type, double lon);
extern ST_GPSXY_POINT algo_gauss_point_get(ST_GPS_POINT gpsPoint);
extern uint8_t algo_gauss_number_get(void);
extern double algo_gauss_center_get(void);

#endif
