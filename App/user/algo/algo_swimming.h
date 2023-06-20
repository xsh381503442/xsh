#ifndef ALGO_SWIMMING_H
#define ALGO_SWIMMING_H


#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define FILTER_SIZE 9
#define DIFF_SIZE 3
#define TFILTER_POINTS 20

typedef struct
{
    int8_t front;  //表头对应的数组下标
    int8_t rear;   //表尾对应的数组下标
    uint32_t size;   //数组有效元素的个数
    float data[FILTER_SIZE][3];     //存储三维x,y,z数据
}SwimFilterDataStr;


typedef struct
{
    float wavevalue[2][3]; //存储三个峰谷值，用于判断是否是有效峰谷
    int8_t wavevaluetrend[2][3]; //存储的三个峰谷值的趋势
    uint8_t pointnumb[3];  //峰谷之间的点数
    uint8_t delwavestep[3]; //删除无效峰谷的步骤
}DelInvalidWaveDataStr;

typedef struct
{
		uint8_t periodstep[3]; //泳姿模板数据的步骤
    uint8_t partperiodcount[3];    //一圈之内的划数
    uint16_t sumperiodcount[3]; //总有效划数
    uint8_t continuityflag[3]; //连续性标志
    uint16_t partwavecount[3];   //一圈内的峰谷数
    uint16_t sumwavecount[3];    //总的峰谷数
}StrokeTemplateDataStr;



extern void SwimAlgorithmInit(void);
extern void SwimDataMeanFilter(float*);
int8_t* SwimDataWaveFind(float*, float*, float*);
void DelInvalidWave(float*, int8_t*);
void TemplateCompare(int8_t, uint8_t, float*, int8_t*);
void BackStrokeCount(int8_t, uint8_t, float*, int8_t*);
void BreastStrokeCount(int8_t, uint8_t, float*, int8_t*);
void ButterflyStrokeCount(int8_t, uint8_t, float*, int8_t*);
void FreeStyleCount(int8_t, uint8_t, float*, int8_t*);
void BackStrokeCircleSet(void);
void BreastStrokeCircleSet(void);
void ButterflyStrokeCircleSet(void);
void FreeStyleCircleSet(void);
void BackStrokeWaveCountSet(void);
void BreastStrokeWaveCountSet(void);
void ButterflyStrokeWaveCountSet(void);
void FreeStyleWaveCountSet(void);
extern void SwimAlgorithmEnd(void);
extern uint8_t SwimDataCircleResult(void);
extern uint16_t SwimDataWaveCountResult(void);
//using namespace std;

#endif

