#ifndef ALGO_GAUSS_H
#define	ALGO_GAUSS_H

typedef struct
{
	double x;  // ��˹X����
	double y;  // ��˹Y����
}ST_GPSXY_POINT;

// GPS��
typedef struct 
{
	double sgp_lon;  // ����
	double sgp_lat;  // γ��
}ST_GPS_POINT;

extern void algo_gauss_init(uint8_t type, double lon);
extern ST_GPSXY_POINT algo_gauss_point_get(ST_GPS_POINT gpsPoint);
extern uint8_t algo_gauss_number_get(void);
extern double algo_gauss_center_get(void);

#endif
