#ifndef ALGO_SWIMMINGCOEFFICIENTS_H_
#define ALGO_SWIMMINGCOEFFICIENTS_H_

#ifdef __cplusplus
extern "C" {
#endif
	
#include "algo_MatFun.h"
	
/*External(Global) constants*/
//configuration of IMU
//static const float GYRO_COE = 8.75; //mdps/LSB(FS=+-250 deg/s)
static const DBL_or_FLT GYRO_COE = 35.0; //(FS=+-1000 deg/s)
static const DBL_or_FLT ACC_COE = 0.122; //mg/LSB
static const int F_SAMPLE_INS = 26; //Sampling Frequency(Hz)
static const DBL_or_FLT STEP_T = 1.0 / F_SAMPLE_INS; //time step

//threshold used for counting the number of strokes
static const DBL_or_FLT OMEGA_X_MIN_PEAK_HEIGHT = 80;//deg/s
//static const DBL_or_FLT OMEGA_X_MIN_NEIGHBOR_PEAK_HEIGHT_DIFFERENCE = 1;//deg/s----------(NOT USED)
static const int MIN_PEAK_DISTANCE = 35;//samples(last version:26)
//static int MIN_PEAK_NUM = 10;//per length		***(1)
static const int MIN_STROKE_COUNT_DIFF = 4;
static const DBL_or_FLT MAD_THRESHOLD_FACTOR = 3.0;// 1.4826;
static const int MAX_INITIAL_PEAK_NUMBER = 1e4;

//threshold used for counting the number of lengths
static const int LENGTH_COUNTER_INITIAL_COUNT = 0;
//static const int LENGTH_COUNTER_MAX_COUNT = 60; ----------(NOT USED)
static const DBL_or_FLT OMEGA_X_INPUT_SIGNAL_LOWER_BOUND = -OMEGA_X_MIN_PEAK_HEIGHT;//-120
static const DBL_or_FLT OMEGA_X_INPUT_SIGNAL_UPPER_BOUND = OMEGA_X_MIN_PEAK_HEIGHT;//120
static const DBL_or_FLT DYNAMIC_ABS_GREATER_BOUND_COEF = 0.2;
static const DBL_or_FLT DYNAMIC_ABS_LESS_BOUND_COEF = 0.3;
//static DBL_or_FLT OMEGA_X_DYNAMIC_LOWER_BOUND = -0.0;//		***(7)
//static DBL_or_FLT OMEGA_X_DYNAMIC_UPPER_BOUND = 0.0;//		***(8)
static const DBL_or_FLT STANDARD_SWIMMING_PERIOD_TIME = 2.0;//s
static const DBL_or_FLT INPUT_SIGNAL_DURATION_TIME = 1.8;//s	//2.16
static const DBL_or_FLT DYNAMIC_SIGNAL_DURATION_TIME_DEVIATION_LB = 0.0;//s
static const DBL_or_FLT DYNAMIC_SIGNAL_DURATION_TIME_DEVIATION_UB = 0.16;//s
//static DBL_or_FLT DYNAMIC_SIGNAL_DURATION_TIME;//s			***(9)
static const DBL_or_FLT DISCRETE_INTEGRATOR_OUTPUT_Y_INITIAL_VALUE = 0.0;
static const DBL_or_FLT DISCRETE_INTEGRATOR_OUTPUT_Y_LOWER_SATURATION_LIMIT = 0.0;
static const DBL_or_FLT DISCRETE_INTEGRATOR_OUTPUT_Y_UPPER_SATURATION_LIMIT = INPUT_SIGNAL_DURATION_TIME;
//static int INITIAL_ELIMINATION_TIME_I = F_SAMPLE_INS * 20;//start counting if time value is greater than this		***(2)
//static int MIN_LENGTHS_TIME_I_INTERVAL = F_SAMPLE_INS * 20;//minimum time interval between any two lengths		***(3)
//static int INITIAL_ELIMINATION_NSTROKES = 12;//start counting if the number of strokes is greater than this		***(4)
//static int MIN_LENGTHS_NSTROKES_INTERVAL = 12;//minimum number of strokes between any two lengths		***(5)
//static int BR_ID_FOR_LENGTH_COUNT_STARTING_NUM_STROKES = 8;//the starting number of strokes to decide if the current sport is breaststroke		***(6)
static const int BR_ID_DURATION_N_STROKES = 3;//the judgement duration time of breaststroke
static const DBL_or_FLT BR_BT_ID_OMEGA_X_MAX_DIFFERENCE = 280;//the maximum difference of peak and valley value of omega_x(deg/s)
static const DBL_or_FLT BR_ID_ACC_Y_MAX_DIFFERENCE = 1.70;//the maximum difference of peak and valley value of acc_y(g)
//static const DBL_or_FLT ACC_Z_MIN_PEAK_HEIGHT = -4;//the minimum height of acc_z----------(NOT USED)
static const DBL_or_FLT ACC_Z_SUDDEN_PEAK = 0.5;
static const int N_BR_ID_PEAK_INFO_ELEMENTS = 4;
static const int BT_ID_N_OMEGA_X_INDEX = 13;
static const int BT_ID_OMEGA_X_MINOR_PEAKS_IN_INTERVAL = 2;

//used for stroke style identification
//define stroke style constants(used in the stroke style identification function and main program)
static const DBL_or_FLT PI_HF = 3.14159265358979; 
static const int MIN_NUMBER_STROKES_FOR_STROKEID_START = 3;
//static const int MIN_SMOOTH_TIME_I = 6 * F_SAMPLE_INS;
static const int MAX_SMOOTH_TIME_I = 3600 * F_SAMPLE_INS;
static const int STROKEID_ESTIMATED_START_TIME_I = 15 * F_SAMPLE_INS;
static const int STROKEID_DURATION_TIME_I = 12 * F_SAMPLE_INS;
static const int WINDOW_SIZE_TIME_I = 2 * F_SAMPLE_INS;
static const int SLIDE_TIME_I = F_SAMPLE_INS/2;
static const int MAX_N_SLIDE = (STROKEID_DURATION_TIME_I - WINDOW_SIZE_TIME_I) / SLIDE_TIME_I;//truncation is applied to integer division




/*
N.B. 
1. The static specifier, instead of extern, is used to avoid violating the one definition rule. That is, only one
file can contain the definition and the other files have to provide the declarations using the extern keyword.
2. In C++, it is said that the const modifier alters the default external linkage of a global variable to
inernal linkage. (i.e. if I omit the "static" above, it's the same) REF:P473~474
3. In C, however, it doesn't. Therefore, the "static" specifier is added to ensure its performance
accross different platforms and explicitly indicates its linkage type, thereby increasing the readability of the code.
4. Some variabls are not defined as constants because their values should be altered later according to the length of the swimming pool.
*/

#ifdef __cplusplus
} // closing brace for extern "C"
#endif

#endif
