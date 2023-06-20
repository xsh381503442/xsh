#include "algo_trackjudge.h"
#include "drv_ublox.h"

#if DEBUG_ENABLED == 1 && ALGO_TRACKJUDGE_LOG_ENABLED == 1
	#define ALGO_TRACKJUDGE_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define ALGO_TRACKJUDGE_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define ALGO_TRACKJUDGE_WRITESTRING(...)
	#define ALGO_TRACKJUDGE_PRINTF(...)		        
#endif

OfftrackDetailStr back_offtrackdata;
OfftrackDetailStr navi_offtrackdata;
OfftrackDetailStr distCP_offtrackdata;

extern int32_t trackgpsstore[6000];//经纬度顺序存储

extern uint16_t gpsbacklen, loadgpslen;         //gps存储长度

extern int32_t loadgpsstore[3000];//加载经纬度顺序存储

/*返航剩余距离*/
OfftrackDetailStr get_back_OfftrackDetail(int32_t  lonx, int32_t  laty, uint32_t  trackdistance)
{
	memset(&back_offtrackdata, 0, sizeof(back_offtrackdata));
	log_back_trackjudge(lonx, laty, trackdistance);
	return back_offtrackdata;
}


/*导航剩余距离*/
OfftrackDetailStr get_navi_OfftrackDetail(int32_t  lonx, int32_t  laty, uint32_t  trackdistance)
{
	memset(&navi_offtrackdata, 0, sizeof(navi_offtrackdata));
	log_navi_trackjudge(lonx, laty, trackdistance);
	return navi_offtrackdata;
}

/*越野跑赛事，CP距离*/
OfftrackDetailStr get_distCP_OfftrackDetail(int32_t  lonx, int32_t  laty, int32_t  destlonx, int32_t  destlaty, uint32_t  trackdistance)
{
	memset(&distCP_offtrackdata, 0, sizeof(distCP_offtrackdata));
	log_distCP_trackjudge(lonx, laty, destlonx, destlaty, trackdistance);
	return distCP_offtrackdata;	
}

//运动轨迹中精度5位，统一用5位处理
float GpsDistance5Bit(int32_t  LonA, int32_t  LatA, int32_t  LonB, int32_t  LatB)
{
    float distance = 0;
    float  lona = LonA / 100000.f;
    float  lata = LatA / 100000.f;
    float  lonb = LonB / 100000.f;
    float  latb = LatB / 100000.f;
	
    if ((lona == lonb) && (lata == latb))
    {
        distance = 0;
    }
    else
    {
        double	Temp_C;
        Temp_C = acos(cos((double)lata  * PI / 180) * cos((double)latb * PI / 180) * cos((double)(lona - lonb)  * PI / 180)
            + sin((double)lata  * PI / 180) * sin((double)latb  * PI / 180));
        distance = (float)R_EARTH *Temp_C;  //??m 
    }

    return  distance;
}
 


void log_back_trackjudge(int32_t  lonx, int32_t  laty, uint32_t  trackdistance)
{
	//经纬度为0过滤判断
	if((lonx == 0 && laty == 0) || gpsbacklen == 0)
	{
		return;
	}
	
	back_offtrackdata.WhetherinTrack = trackjudge(trackgpsstore, gpsbacklen, lonx, laty);
	back_offtrackdata.RemainDistance = back_remaindistance(trackgpsstore, gpsbacklen, trackdistance, lonx, laty);
	if (back_offtrackdata.WhetherinTrack == 0)
	{
		back_offtrackdata.DirectionAngle = get_DirectionAngle(trackgpsstore, gpsbacklen, lonx, laty);
		back_offtrackdata.NearestDistance = back_remaindistance(trackgpsstore, gpsbacklen, trackdistance, lonx, laty);
	}
}

void log_navi_trackjudge(int32_t  lonx, int32_t  laty, uint32_t  trackdistance)
{
	//经纬度为0过滤判断
	if((lonx == 0 &&  laty == 0 )|| loadgpslen == 0)
	{
		return;
	}
	navi_offtrackdata.WhetherinTrack = trackjudge(loadgpsstore, loadgpslen, lonx, laty);
	navi_offtrackdata.RemainDistance = navi_remaindistance(loadgpsstore, loadgpslen, trackdistance, lonx, laty);
	if (navi_offtrackdata.WhetherinTrack == 0)
	{
		navi_offtrackdata.DirectionAngle = get_DirectionAngle(loadgpsstore, loadgpslen, lonx, laty);
		navi_offtrackdata.NearestDistance = navi_remaindistance(loadgpsstore, loadgpslen, trackdistance, lonx, laty);
	}
}

void log_distCP_trackjudge(int32_t  lonx, int32_t  laty,  int32_t  destlonx, int32_t  destlaty, uint32_t  trackdistance)
{
	//经纬度为0过滤判断
	if((lonx == 0 ||  laty == 0 )|| loadgpslen == 0 || (destlonx == 0) || (destlaty == 0))
	{
		return;
	}
	distCP_offtrackdata.WhetherinTrack = trackjudge(loadgpsstore, loadgpslen, lonx, laty);
	distCP_offtrackdata.RemainDistance = distCP_remaindistance(loadgpsstore, loadgpslen, trackdistance, lonx, laty, destlonx, destlaty);
	if (distCP_offtrackdata.WhetherinTrack == 0)
	{
		distCP_offtrackdata.DirectionAngle = get_DirectionAngle(loadgpsstore, loadgpslen, lonx, laty);
		distCP_offtrackdata.NearestDistance = distCP_remaindistance(loadgpsstore, loadgpslen, trackdistance, lonx, laty, destlonx, destlaty);
	}
}


float get_DirectionAngle(int32_t trackpoints[], int trackpointnum, float lonx, float laty)
{
	int trackindex = findtrackindex(trackpoints, trackpointnum, lonx, laty);
	return getangle(lonx, laty, trackpoints[trackindex], trackpoints[trackindex + 1]);
}

//寻找轨迹中所在点，可以换种不够精准的算法，提高运行速度
int findtrackindex(int32_t trackpoints[], int trackpointnum, float lonx, float laty)
{
	int liv_i;
	float min_sum = (pow((trackpoints[0] - lonx), 2) + pow((trackpoints[1] - laty), 2));
	int trackindex = 0;
	float tmp;

	for (liv_i = 0; liv_i < trackpointnum - 1; liv_i += 2)
	{
		tmp = (pow((trackpoints[liv_i] - lonx), 2) + pow((trackpoints[liv_i + 1] - laty), 2));

		if (tmp < min_sum)
		{
			min_sum = tmp;
			trackindex = liv_i;
		}

	}
	return trackindex;
}

//坐标B相对于坐标A的方位角，从正北方向顺时针旋转
float getangle(int32_t  LonA, int32_t  LatA, int32_t  LonB, int32_t  LatB)
{
	double  Lona = LonA / 100000.0;
	double  Lata = LatA / 100000.0;
	double  Lonb = LonB / 100000.0;
	double  Latb = LatB / 100000.0;
	Lona = Lona * PI / 180;
	Lata = Lata * PI / 180;
	Lonb = Lonb * PI / 180;
	Latb = Latb * PI / 180;

	double	Temp_D;

	
	
	Temp_D = sin(Lata) * sin(Latb) + cos(Lata) * cos(Latb) * cos(Lonb - Lona);
	Temp_D = sqrt(1 - Temp_D * Temp_D);
	Temp_D = sin((cos(Latb) * sin(Lonb - Lona)) / Temp_D);
	Temp_D = asin(Temp_D) * 180 / PI;//单位为角度
	

	if (Lona == Lonb)
	{
		if (Lata <= Latb)
		{
			return 0;
		}
		else if (Lata > Latb)
		{
			return 180;
		}
	}
	/*
	Temp_D = 4 * pow(sin((Lata - Latb) / 2), 2) - pow(sin((Lona - Lonb) / 2) * (cos(Lata) - cos(Latb)), 2);
	Temp_D = sqrt(Temp_D);
	Temp_D = Temp_D / (sin(abs(Lona - Lonb) / 2) * (cos(Lata) + cos(Latb)));
	Temp_D = atan(Temp_D) * 180 / PI;  //单位为角度
	*/

	if (Lata < Latb)
	{
		if (Lona > Lonb)
		{
			return 360 + Temp_D;
		}
		else 
		{
			return Temp_D;
		}
	}
	else
	{
		return 180 - Temp_D;
	}
}

int trackjudge(int32_t trackpoints[], int trackpointnum, float lonx, float laty)
{
	int liv_index = findtrackindex(trackpoints, trackpointnum, lonx, laty);
	float lfv_min_distance = GpsDistance5Bit(trackpoints[liv_index], trackpoints[liv_index + 1], lonx, laty);

	if (lfv_min_distance < THREHOLD_TRACK)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

float trackdistance(int32_t trackpoints[], int trackpointnum)
{
	int liv_i;
	float lfv_partdistancesum = 0;
	

	for (liv_i = 0; liv_i < trackpointnum - 1; liv_i += 2)
	{
		lfv_partdistancesum += GpsDistance5Bit(trackpoints[liv_i], trackpoints[liv_i + 1], trackpoints[liv_i + 2], trackpoints[liv_i + 3]);
	}

	return lfv_partdistancesum;
}

//部分距离的比例值，因为轨迹是抽稀的一部分轨迹
float partdistancerate(int32_t parttrackpoints[], int parttrackpointnum, float trackdistance)
{
	int liv_i;
	float lfv_partdistancesum = 0;
	float lfv_partdistancerate;

	for (liv_i = 0; liv_i < parttrackpointnum - 3; liv_i +=2)
	{
		lfv_partdistancesum += GpsDistance5Bit(parttrackpoints[liv_i], parttrackpoints[liv_i + 1], parttrackpoints[liv_i + 2], parttrackpoints[liv_i + 3]);
	}

	lfv_partdistancerate = trackdistance / (lfv_partdistancesum * 100);
	return lfv_partdistancerate;
}

float back_remaindistance(int32_t trackpoints[], int trackpointnum, float trackdistance, float lonx, float laty)
{
	
	int liv_trackjudge = trackjudge(trackpoints, trackpointnum, lonx, laty);
	int liv_index = findtrackindex(trackpoints, trackpointnum, lonx, laty);
	float lfv_partdistancerate = partdistancerate(trackpoints, trackpointnum, trackdistance);

	int liv_i;
	float lfv_init_remaindistance;
	float lfv_remaindistance = 0;
	float lfv_nearestdistance = GpsDistance5Bit(trackpoints[liv_index], trackpoints[liv_index + 1], lonx, laty) * 100;

	if (liv_trackjudge == 1)
	{
			if (liv_index < 2)  //判断是否离起点最近
			{
				if(lfv_nearestdistance < THREHOLD_TRACK_END_DIST)
				{
					return 0;
				}
				else
				{
				return lfv_nearestdistance;
				}
			}
			else
			{
				lfv_init_remaindistance = GpsDistance5Bit(trackpoints[liv_index - 2], trackpoints[liv_index - 1], lonx, laty);

				for (liv_i = 0; liv_i < liv_index - 2; liv_i += 2)
				{
					lfv_remaindistance += GpsDistance5Bit(trackpoints[liv_i], trackpoints[liv_i + 1], trackpoints[liv_i + 2], trackpoints[liv_i + 3]);
				}

				lfv_remaindistance = (lfv_init_remaindistance + lfv_remaindistance * lfv_partdistancerate) * 100;
				if(lfv_remaindistance < THREHOLD_TRACK_END_DIST)
				{
					return 0;
				}
				else
				{
				return lfv_remaindistance;
				}
			}
	}
	else
	{
		return lfv_nearestdistance;
	}

}

float navi_remaindistance(int32_t trackpoints[], int trackpointnum, float trackdistance, float lonx, float laty)
{
	
	int liv_trackjudge = trackjudge(trackpoints, trackpointnum, lonx, laty);
	int liv_index = findtrackindex(trackpoints, trackpointnum, lonx, laty);
	float lfv_partdistancerate = partdistancerate(trackpoints, trackpointnum, trackdistance);
	
	int liv_i;
	float lfv_init_remaindistance;
	float lfv_remaindistance = 0;
	float lfv_nearestdistance = GpsDistance5Bit(trackpoints[liv_index], trackpoints[liv_index + 1], lonx, laty) * 100;
	if (liv_trackjudge == 1)
	{
		if (liv_index > trackpointnum - 4) //判断是否离终点最近
		{
			if(lfv_nearestdistance < THREHOLD_TRACK_END_DIST)
			{
				return 0;
			}
			else
			{
				return lfv_nearestdistance;
			}
		}
		else
		{
			lfv_init_remaindistance = GpsDistance5Bit(trackpoints[liv_index + 2], trackpoints[liv_index + 3], lonx, laty);

			for (liv_i = liv_index + 2; liv_i < trackpointnum - 2; liv_i += 2)
			{
				lfv_remaindistance += GpsDistance5Bit(trackpoints[liv_i], trackpoints[liv_i + 1], trackpoints[liv_i + 2], trackpoints[liv_i + 3]);
			}

			lfv_remaindistance = (lfv_init_remaindistance + lfv_remaindistance * lfv_partdistancerate) * 100;
			if(lfv_remaindistance < THREHOLD_TRACK_END_DIST)
			{
				return 0;
			}
			else
			{
				return lfv_remaindistance;
			}
		}
	}
	else
	{
		return lfv_nearestdistance;
	}
}


//越野，距离CP，剩余距离的计算
float distCP_remaindistance(int32_t trackpoints[], int trackpointnum, float trackdistance, float lonx, float laty, float destlonx, float destlaty)
{
	
	int liv_trackjudge = trackjudge(trackpoints, trackpointnum, lonx, laty);
	int liv_index = findtrackindex(trackpoints, trackpointnum, lonx, laty);
	int liv_destindex = findtrackindex(trackpoints, trackpointnum, destlonx, destlaty);
	float lfv_partdistancerate = partdistancerate(trackpoints, trackpointnum, trackdistance);
	
	int liv_i;
	float lfv_remaindistance = 0;
	float lfv_nearestdistance = GpsDistance5Bit(trackpoints[liv_index], trackpoints[liv_index + 1], lonx, laty) * 100;
	if(liv_destindex < liv_index)
	{
		return 0;
	}
	
	if (liv_trackjudge == 1)
	{		

			float lfv_end_remaindistance = GpsDistance5Bit(trackpoints[liv_destindex], trackpoints[liv_destindex + 1], destlonx, destlaty);

			for (liv_i = liv_index; liv_i <= liv_destindex - 2; liv_i += 2)
			{
				lfv_remaindistance += GpsDistance5Bit(trackpoints[liv_i], trackpoints[liv_i + 1], trackpoints[liv_i + 2], trackpoints[liv_i + 3]);
			}

			lfv_remaindistance = (lfv_end_remaindistance + lfv_remaindistance * lfv_partdistancerate) * 100;
			if(lfv_remaindistance < THREHOLD_TRACK_END_CPDIST)
			{
				return 0;
			}
			else
			{
				return lfv_remaindistance;
			}
	}
	else
	{
		return lfv_nearestdistance;
	}
}

