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
    int8_t front;  //��ͷ��Ӧ�������±�
    int8_t rear;   //��β��Ӧ�������±�
    uint32_t size;   //������ЧԪ�صĸ���
    float data[FILTER_SIZE][3];     //�洢��άx,y,z����
}SwimFilterDataStr;


typedef struct
{
    float wavevalue[2][3]; //�洢�������ֵ�������ж��Ƿ�����Ч���
    int8_t wavevaluetrend[2][3]; //�洢���������ֵ������
    uint8_t pointnumb[3];  //���֮��ĵ���
    uint8_t delwavestep[3]; //ɾ����Ч��ȵĲ���
}DelInvalidWaveDataStr;

typedef struct
{
		uint8_t periodstep[3]; //Ӿ��ģ�����ݵĲ���
    uint8_t partperiodcount[3];    //һȦ֮�ڵĻ���
    uint16_t sumperiodcount[3]; //����Ч����
    uint8_t continuityflag[3]; //�����Ա�־
    uint16_t partwavecount[3];   //һȦ�ڵķ����
    uint16_t sumwavecount[3];    //�ܵķ����
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

