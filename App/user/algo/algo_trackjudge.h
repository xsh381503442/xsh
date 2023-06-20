#ifndef ALGO_TRACKJUDGE_H
#define	ALGO_TRACKJUDGE_H


#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include "algo_trackoptimizing.h"



#define     R_EARTH 6371004      //地球半径，单位米

#define     PI 3.1415926

#define     THREHOLD_TRACK 30  //轨迹偏离的距离阈值

#define			THREHOLD_TRACK_END_DIST 1000	//被视为终点的距离范围，单位cm

#define			THREHOLD_TRACK_END_CPDIST 5000	//被视为CP终点的距离范围，单位cm

typedef struct{
	uint8_t WhetherinTrack;           //是否在轨迹上：在轨迹上为1；不在为0；
	uint32_t RemainDistance;          //离轨迹终点剩余距离
	uint16_t DirectionAngle;          //偏离轨迹时的角度，顺时针方向
	uint32_t NearestDistance;         //偏离轨迹时离轨迹最近距离
}OfftrackDetailStr;

extern OfftrackDetailStr get_back_OfftrackDetail(int32_t  lonx, int32_t  laty, uint32_t  trackdistance);
extern OfftrackDetailStr get_navi_OfftrackDetail(int32_t  lonx, int32_t  laty, uint32_t  trackdistance);
extern OfftrackDetailStr get_distCP_OfftrackDetail(int32_t  lonx, int32_t  laty, int32_t  destlonx, int32_t  destlaty, uint32_t  trackdistance);
void log_back_trackjudge(int32_t  lonx, int32_t  laty, uint32_t  trackdistance);
void log_navi_trackjudge(int32_t  lonx, int32_t  laty, uint32_t  trackdistance);
void log_distCP_trackjudge(int32_t  lonx, int32_t  laty,  int32_t  destlonx, int32_t  destlaty, uint32_t  trackdistance);
int findtrackindex(int32_t trackpoints[], int trackpointnum, float lonx, float laty);
float getangle(int32_t  LonA, int32_t  LatA, int32_t  LonB, int32_t  LatB);
int trackjudge(int32_t trackpoints[], int trackpointnum, float lonx, float laty);
float partdistancerate(int32_t parttrackpoints[], int parttrackpointnum, float trackdistance);
float back_remaindistance(int32_t trackpoints[], int trackpointnum, float trackdistance, float lonx, float laty);
float navi_remaindistance(int32_t trackpoints[], int trackpointnum, float trackdistance, float lonx, float laty);
float distCP_remaindistance(int32_t trackpoints[], int trackpointnum, float trackdistance, float lonx, float laty, float destlonx, float destlaty);
float get_DirectionAngle(int32_t trackpoints[], int trackpointnum, float lonx, float laty);

#endif
