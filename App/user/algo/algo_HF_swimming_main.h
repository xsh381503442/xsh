#ifndef ALGO_HF_SWIMMING_MAIN_H_
#define ALGO_HF_SWIMMING_MAIN_H_

#ifdef __cplusplus
extern "C" {
#endif
//The code above and that of the last 4 lines are used together to make the following content suitable for both C and C++ compilers

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "algo_MatFun.h"
#include "algo_swimmingParameters.h"
#include "algo_config.h"//debug
#include "com_sport.h"
	
	//==================================DEBUG=======================================================
#if DEBUG_ENABLED == 1 && ALGO_HF_SWIMMING_MAIN_LOG_ENABLED == 1
	#define ALGO_HF_SWIM_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define ALGO_HF_SWIM_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define ALGO_HF_SWIM_WRITESTRING(...)
	#define ALGO_HF_SWIM_PRINTF(...)		        
#endif
	//==================================DEBUG ENDS==================================================	


//add these 2 lines when needed and include the corresponding header file before including "MatFun.h"
//#undef DOUBLE_MAT
//#define DOUBLE_MAT
//-------------------for stroke count----------------------------------------
//for C++
//template<typename T1, typename T2>
//constexpr auto MIN_2NO(T1 a, T2  b) { return (((a) < (b)) ? (a) : (b)); }

//template<typename T1, typename T2>
//constexpr auto MAX_2NO(T1 a, T2 b) { return (((a) > (b)) ? (a) : (b)); }

//for C
#define MIN_2NO(a, b) (((a) < (b)) ? (a) : (b))

#define MAX_2NO(a, b) (((a) > (b)) ? (a) : (b))

//a structure to store the peak value and index
typedef struct
{
	float value; int index;
}structureCompare;
//assume y is the data, k is the discrete step
typedef struct
{
	//data value at time k-2, k-1, k
	DBL_or_FLT k_2;//y(k-2)
	DBL_or_FLT k_1;//y(k-1)
	DBL_or_FLT k;//y(k)
	//left and right difference(of y(k-1) )
	DBL_or_FLT diff_B;//y(k-1)-y(k-2) Backward difference
	DBL_or_FLT diff_F;//y(k)-y(k-1) Forward difference
}DataSequence_Structure;
typedef struct
{
	int index;
	DBL_or_FLT val;
}PeakInfo;
//--------------------for length count----------------------------------
typedef struct
{
	int index[N_BR_ID_PEAK_INFO_ELEMENTS];
	DBL_or_FLT val[N_BR_ID_PEAK_INFO_ELEMENTS];
}BR_ID_PeakInfo;

typedef struct
{
	//all the arrays are allocated more space than necessary
	int index[BT_ID_N_OMEGA_X_INDEX];
	DBL_or_FLT val[BT_ID_N_OMEGA_X_INDEX];
	int N_ele;
	int major_index[BT_ID_N_OMEGA_X_INDEX];
	DBL_or_FLT major_val[BT_ID_N_OMEGA_X_INDEX];
	int N_major_ele;
}BT_ID_omega_x_PeakInfo;
//-----------------------------for stroke type identification--------------------
typedef enum//typedef is used to ensure compatibility with C(it's not necessary in C++)
{
	Freestyle = 0 ,//0
	Backstroke,//1
	Breaststroke,//2
	ButterflyStroke,//3
	UnknownStyle,//4,
	Medley,
}StrokeID;

typedef struct
{
	int Num_FS;
	int Num_BK;
	int Num_BR;
	int Num_BT;
}StrokeTypeNum;

/*----------------------Functions-------------------------*/
//N.B. The prototype of static functions are not listed here because they aren't called by functions out of that file
//and they're defined according to the order of occurence.
//(1) Initialize all the static variables defined in this file(algo_HF_swimming_main.h)
void ParameterInitialization(uint8_t LengthSwimmingPool);
//(2) Dealing with static condition
void IncidentPreprocessing(short* RawData);
//(3) Main algorithm(INPUT PORT) ------------------------------------------------------------------------------------------------------------------------------
void SwimmingAlgorithmsAll(short* data_arr, int time_i);
//(4) Post-processing
void SwimmingAlgorithmsEnd(void);
//(5) Return outputs(OUTPUT PORT)
int SwimResultsOutputPort1_StrokeCount(void);
int SwimResultsOutputPort2_LengthCount(void);
int SwimResultsOutputPort3_SwimmingStyle(void);
int SwimResultsPostProcessing(void);

////1. Raw data preprocessing----------------------------------------------------------------------------------
//void RawDataPreprocessor(int* Input_IMU_Raw_Data, Mat * acc, Mat * omega);
///*
//Functionality: obtain acceleration, angular velocity and the corresponding timestamp from the raw data of IMU

//Inputs:
//1. RawData_arr[][6]: raw data
//2. time_i: the ith instant of time(zero-based)

//Outputs:
//1. acc: acceleration(1-by-3 row vector)
//2. omega: angular velocity(1-by-3 row vector)
//3. timestamp(returned value) (float scalar)
//*/


////2. Main algorithm-----------------------------------------------------------------------------------------------
////2.1 Filter design-----------------------------------------------------------------------------------------------
////Butterworth filter for float data
//float ButterworthFilter1(float u_k, const Mat* A, const Mat* B, const Mat* C, float D, Mat* x_k1, float* u_k1);

////Butterworth filter for double data
//double ButterworthFilter2(double u_k, const Mat* A, const Mat* B, const Mat* C, double D, Mat* x_k1, double* u_k1);

////obtain the filtered results(float)
//void SwimDataFilter1(const Mat * acc, const Mat * omega, const Mat * A, const Mat * B, const Mat * C, float D,
//	Mat * accX_x_k1, float * accX_u_k1, Mat * accY_x_k1, float * accY_u_k1, Mat * accZ_x_k1, float * accZ_u_k1,
//	Mat * omegaX_x_k1, float * omegaX_u_k1, Mat * omegaY_x_k1, float * omegaY_u_k1, Mat * omegaZ_x_k1, float * omegaZ_u_k1,
//	Mat * acc_f, Mat * omega_f);

////obtain the filtered results(double)
//void SwimDataFilter2(const Mat * acc, const Mat * omega, const Mat * A, const Mat * B, const Mat * C, double D,
//	Mat * accX_x_k1, double * accX_u_k1, Mat * accY_x_k1, double * accY_u_k1, Mat * accZ_x_k1, double * accZ_u_k1,
//	Mat * omegaX_x_k1, double * omegaX_u_k1, Mat * omegaY_x_k1, double * omegaY_u_k1, Mat * omegaZ_x_k1, double * omegaZ_u_k1,
//	Mat * acc_f, Mat * omega_f);
///*      Introduction
//The file incorporates miscellaneous filter functions.
//All the filters are designed in advance, so the corresponding transfer function coefficients/second-order sections(SOS)/state-space matrices
//are given directly.

//*/

///*
//---------------Butterworth filter-----------------------
//Inputs:
//1. u_k: u(k), current input data(3-by-1)
//2. A: state matrix
//3. B: input matrix
//4. C: output matrix
//5. D: direct transition matrix(here it's a scalar because the input is always a column vector)
//6. x_k1: x(k-1), initial state(value of the last step)
//7. u_k1: u(k-1), initial input(value of the last step) 

//Outputs:
//1. y: output data(3-by-1)


//N.B. the aforementioned parameters satisfy the equation
//x(k)=A*x(k-1)+B*u(k-1)
//y(k)=C*x(k)+D*u(k)
//*/


////2.2 Number of strokes-------------------------------------------------------------------------------------------------------------------
//void StrokeCount1(int NumLength, int time_i, int stopSignalTime_i, DBL_or_FLT input_val_i, DataSequence_Structure* inputDataSequence1, int* NStrokes1);

//void StrokeCount2(int NumLength, int time_i, int stopSignalTime_i, DBL_or_FLT input_val_i, DataSequence_Structure* inputDataSequence2, int* NStrokes2);

//void StrokeCount(int NumLength, int time_i, int stopSignalTime_i, DBL_or_FLT input_val_i, DataSequence_Structure* inputDataSequence1, int* NStrokes1,
//	DataSequence_Structure* inputDataSequence2, int* NStrokes2, int* NStrokes);

////2.3 Number of lengths--------------------------------------------------------------------------------------------------------------------------
//void LengthCount(int time_i, DBL_or_FLT inputData, int* counter_tk, int* counter_tk_1, DBL_or_FLT* y, int* isDurationTimePassed, int* NLength);
///*			Introduction
//Calculate the number of length

//Inputs:
//1. time_i:  0-based time index, i.e. the ith data
//2. inputData: the input signal, i.e. acceleration or angular velocity, used to detect the length

//Inputs and Outputs(Update each time)
//1. counter_tk: time of the latest counter, t(k)
//2. counter_tk_1: time of the last counter, t(k-1)
//3. y: discrete integrator output, it updates each time to initialize the next step
//4. isDurationTimePssed: although it's defined as int, it's bool actually, a judgement of whether a signal lasts for a certain time

//5.NLength: the number of length, the value of which is the ultimate goal of this function
//*/


////2.4 Stroke type identification
//void StrokeStyleIdentification(int time_i, int strokeCount, const Mat* accelerometer_val, const Mat* gyroscope_val, int* NthSlide, int* smoothStrokeTime_i, StrokeTypeNum* StrokeTypeCountNumber, bool* smooth_flag,
//	Mat* a_x, Mat* a_y, Mat* a_z, Mat* a_Mag, Mat* omega_x, Mat* omega_y, Mat* omega_z, Mat* omega_Mag, StrokeID* swimmingStyle);

#ifdef __cplusplus
} // closing brace for extern "C"
#endif

//endif for #ifndef the header file
#endif
