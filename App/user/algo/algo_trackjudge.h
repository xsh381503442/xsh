#ifndef ALGO_TRACKJUDGE_H
#define	ALGO_TRACKJUDGE_H


#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include "algo_trackoptimizing.h"



#define     R_EARTH 6371004      //����뾶����λ��

#define     PI 3.1415926

#define     THREHOLD_TRACK 30  //�켣ƫ��ľ�����ֵ

#define			THREHOLD_TRACK_END_DIST 1000	//����Ϊ�յ�ľ��뷶Χ����λcm

#define			THREHOLD_TRACK_END_CPDIST 5000	//����ΪCP�յ�ľ��뷶Χ����λcm

typedef struct{
	uint8_t WhetherinTrack;           //�Ƿ��ڹ켣�ϣ��ڹ켣��Ϊ1������Ϊ0��
	uint32_t RemainDistance;          //��켣�յ�ʣ�����
	uint16_t DirectionAngle;          //ƫ��켣ʱ�ĽǶȣ�˳ʱ�뷽��
	uint32_t NearestDistance;         //ƫ��켣ʱ��켣�������
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
