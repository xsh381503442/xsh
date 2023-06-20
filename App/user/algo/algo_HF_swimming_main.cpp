#include "algo_HF_swimming_main.h"

extern "C" {
//=============DEFINITION OF VARIABLES===========================================================
//These static variables have static storage duration and internal linkage, which means they can be used
//across the functions of this file(internal linkage) and they stay present as long as the program executes(static storage duration), 
//since the compiler allocates a fixed block of memory to hold all the static variables.

//preprocessing---------------------------------------------------------------
//int stopSignalTime_i = 0;//when pressing the stop button
//int Input_IMU_Raw_Data[6];/*INPUT*//*It's definition rather than reference because the "extern" is ommited*/
//raw data of acceleration and angular velocity
int time_i;
static Mat acc;//column vector
static Mat omega;//column vector

// filter design
//acc_x
static Mat accX_x_k1;
static DBL_or_FLT accX_u_k1[1];
//acc_y
static Mat accY_x_k1;
static DBL_or_FLT accY_u_k1[1];
//acc_z
static Mat accZ_x_k1;
static DBL_or_FLT accZ_u_k1[1];
//omega_x
static Mat omegaX_x_k1;
static DBL_or_FLT omegaX_u_k1[1];
//omega_y
static Mat omegaY_x_k1;
static DBL_or_FLT omegaY_u_k1[1];
//omega_x
static Mat omegaZ_x_k1;
static DBL_or_FLT omegaZ_u_k1[1];

//filtered data
static Mat acc_f;
static Mat omega_f;
static Mat A, B, C;
static DBL_or_FLT D;
//----------------------------------------------------------------------------
//for counting the number of strokes------------------------------------------
static int MIN_PEAK_NUM;//starting phase of the stroke count		***(1)
static DataSequence_Structure inputDataSequence1;
static DataSequence_Structure inputDataSequence2;
static int NumStrokeCount1;//NO. of strokes from the filtered data
static int NumStrokeCount2;//NO. of strokes from the additive inverse of the filtered data
int NumStrokeCount;/*OUTPUT*//*number of strokes*/
//short* peak_index;//peak indice
//DBL_or_FLT* peak_val;//peak values
static PeakInfo PossiblePeak1;
static PeakInfo PossiblePeak2;
//---------------------------------------------------------------------------
//for length count-----------------------------------------------------------
//omega_x
static int omega_x_counter_tk;//length number of the current step t(k)
static int omega_x_counter_tk_1;//length number of last last step t(k-1)
static int omega_x_NStrokes_tk;//number of strokes at the time of the current counter t(k)
static int omega_x_NStrokes_tk_1;//number of strokes at the time of the last counter t(k-1)
//a_z
static int a_z_counter_tk;//length number of the current step t(k)
static int a_z_counter_tk_1;//length number of last last step t(k-1)
static int a_z_NStrokes_tk;//number of strokes at the time of the current counter t(k)
static int a_z_NStrokes_tk_1;//number of strokes at the time of the last counter t(k-1)
static DBL_or_FLT integrator_output_y;//integrator output of interval duration test
static int isDurationTimePassed;//passes the duration test or not
int NLength;/*OUTPUT*//*number of length*/
int stop_time_i;/*Used in Call_main.cpp*/
//variables to decide the current stroke type
static DBL_or_FLT BR_ID_omega_max;
static DBL_or_FLT BR_ID_omega_min;
static DBL_or_FLT BR_ID_acc_y_max;
static DBL_or_FLT BR_ID_acc_y_min;
//static DBL_or_FLT BR_ID_acc_z_max;
//static DBL_or_FLT BR_ID_acc_z_min;
static bool BR_flag;
static int BR_az_start_counting_time_i;
static DataSequence_Structure a_z_Sequence1;
static DataSequence_Structure a_z_Sequence2;
static DataSequence_Structure omega_x_Sequence1;
static DataSequence_Structure omega_x_Sequence2;
static BR_ID_PeakInfo a_z_Possible_Peak1;
static BR_ID_PeakInfo a_z_Possible_Peak2;
static BR_ID_PeakInfo omega_x_Possible_Peak1;
static BR_ID_PeakInfo omega_x_Possible_Peak2;
static int N_a_z_Peak1;
static int N_a_z_Peak2;
static int N_omega_x_Peak1;
static int N_omega_x_Peak2;
static DBL_or_FLT a_z_Peak1_Sum;
static DBL_or_FLT a_z_Peak2_Sum;
static DBL_or_FLT omega_x_Peak1_Sum;
static DBL_or_FLT omega_x_Peak2_Sum;
static DBL_or_FLT a_z_Peak1_Mean;
static DBL_or_FLT a_z_Peak2_Mean;
static DBL_or_FLT omega_x_Peak1_Mean;
static DBL_or_FLT omega_x_Peak2_Mean;
static int omega_x_index1_Sum;
static int omega_x_index1_Mean;
static int omega_x_index2_Sum;
static int omega_x_index2_Mean;
static bool is_BR_ID_omega_az_MeanCalculationDone;
static bool hasPassedInitialEliminationPeriod;
static bool has_Entered_BR_BT_ID_Ub;
static BT_ID_omega_x_PeakInfo BT_ID_omega_x_PeakCountInfo1;
static DataSequence_Structure BT_ID_omega_x_Sequence1;
static BT_ID_omega_x_PeakInfo BT_ID_omega_x_PeakCountInfo2;
static DataSequence_Structure BT_ID_omega_x_Sequence2;
static bool BT_flag;
//--------------------------------------------------------------------------
//for stroke style identification-------------------------------------------
//static Mat a_x;
//static Mat a_y;
//static Mat a_z;
//static Mat a_Mag;
//static Mat omega_x;
//static Mat omega_y;
//static Mat omega_z;
//static Mat omega_Mag;
static float a_x[WINDOW_SIZE_TIME_I + 1];
static float a_y[WINDOW_SIZE_TIME_I + 1];
static float a_z[WINDOW_SIZE_TIME_I + 1];
static float a_Mag[WINDOW_SIZE_TIME_I + 1];
static float omega_x[WINDOW_SIZE_TIME_I + 1];
static float omega_y[WINDOW_SIZE_TIME_I + 1];
static float omega_z[WINDOW_SIZE_TIME_I + 1];
static float omega_Mag[WINDOW_SIZE_TIME_I + 1];
static bool smooth_start_flag;// enters the gliding phase or not
static int NSlide;//number of windows slided
static int smoothStrokeTime_i;//assign an extremely large value
static StrokeTypeNum StrokeTypeCountNumber;//stroke style judgement count
StrokeID swimmingStyle;/*OUTPUT*//*swimming stroke style*/
//---------------------------------------------------------------------------
/*===================================================================================*/

//variables whose values depend on the length of the swimming pool
static int INITIAL_ELIMINATION_TIME_I;//start counting if time value is greater than this		***(2)
static int MIN_LENGTHS_TIME_I_INTERVAL;//minimum time interval between any two lengths		***(3)
static int INITIAL_ELIMINATION_NSTROKES;//start counting if the number of strokes is greater than this		***(4)
static int MIN_LENGTHS_NSTROKES_INTERVAL;//minimum number of strokes between any two lengths		***(5)
static int BR_ID_FOR_LENGTH_COUNT_STARTING_NUM_STROKES;//the starting number of strokes to decide if the current sport is breaststroke		***(6)
//dynamic bound
static DBL_or_FLT OMEGA_X_DYNAMIC_LOWER_BOUND;//		***(7)
static DBL_or_FLT OMEGA_X_DYNAMIC_UPPER_BOUND;//		***(8)
static DBL_or_FLT DYNAMIC_SIGNAL_DURATION_TIME;//s			***(9)

/*===================================================================================*/

//===============================END OF THE DEFINITION OF VARIABLES===================================================



//==============================FUNCTIONS=============================================================================
//--------------0. Pre-processing--------------------------
// Parameter Initialization
void ParameterInitialization(uint8_t LengthSwimmingPool)
{
	/*Test Area*/
	ALGO_HF_SWIM_PRINTF("Initialization\n");
	//INPUT DATA---------------------------------------------------------------
	//	memset(Input_IMU_Raw_Data, 0, sizeof(Input_IMU_Raw_Data));/*INPUT*/
	time_i=-1;
	
	//lots of the initialization process may be unnecessary due to the default initialization or assignment mechanism inside the functions,
	//they're still kept to make the program easier to read

	//acceleration and angular velocity
	MatCreate(&acc, 3, 1);
	MatCreate(&omega, 3, 1);

	// filter design
	//acc_x
	MatCreate(&accX_x_k1, 4, 1);
	MatZeros(&accX_x_k1);
	accX_u_k1[0] = 0.0;
	//acc_y
	MatCreate(&accY_x_k1, 4, 1);
	MatZeros(&accY_x_k1);
	accY_u_k1[0] = 0.0;
	//acc_z
	MatCreate(&accZ_x_k1, 4, 1);
	MatZeros(&accZ_x_k1);
	accZ_u_k1[0] = 0.0;
	//omega_x
	MatCreate(&omegaX_x_k1, 4, 1);
	MatZeros(&omegaX_x_k1);
	omegaX_u_k1[0] = 0.0;
	//omega_y
	MatCreate(&omegaY_x_k1, 4, 1);
	MatZeros(&omegaY_x_k1);
	omegaY_u_k1[0] = 0.0;
	//omega_x
	MatCreate(&omegaZ_x_k1, 4, 1);
	MatZeros(&omegaZ_x_k1);
	omegaZ_u_k1[0] = 0.0;

	//filtered data
	MatCreate(&acc_f, 3, 1);
	MatCreate(&omega_f, 3, 1);

	//filter state-space parameters(they're obtained by using sos2ss function of matlab, where sos expression
	//is obtained directly from the filter design object)
	DBL_or_FLT A_arr_init[] = { 3.369237996467506, -4.299112464491480, 2.458177059845606, -0.530836492840990,
										 1.0,	     		   0,				  0,			      0,
										   0,				 1.0,				  0,				  0,
										   0,				   0,			    1.0,				  0 };
	DBL_or_FLT B_arr_init[] = { 1.0,
						0.0,
						0.0,
						0.0 };
	DBL_or_FLT C_arr_init[] = { 0.001167057479446,   0.000269367541252,   0.001022773839696,   0.000074300868065 };
	D = 1.583688137098529e-04;
	MatCreate(&A, 4, 4);
	MatCreate(&B, 4, 1);
	MatCreate(&C, 1, 4);
	MatSetVal(&A, A_arr_init);
	MatSetVal(&B, B_arr_init);
	MatSetVal(&C, C_arr_init);

	//----------------------------------------------------------------------------
	//for counting the number of strokes------------------------------------------
	//in practice static variables are assgined to 0 automatically when defined, the following assignment is just to make it even more explicit
	NumStrokeCount1 = 0;
	NumStrokeCount2 = 0;
	NumStrokeCount = 0;/*OUTPUT*/
	PossiblePeak1.index = 0;
	PossiblePeak1.val = 0;
	PossiblePeak2.index = 0;
	PossiblePeak2.val = 0;
	//---------------------------------------------------------------------------
	//for length count-----------------------------------------------------------
	omega_x_counter_tk = 0;
	omega_x_counter_tk_1 = 0;
	omega_x_NStrokes_tk = 0;
	omega_x_NStrokes_tk_1 = 0;
	a_z_counter_tk = 0;
	a_z_counter_tk_1 = 0;
	a_z_NStrokes_tk = 0;
	a_z_NStrokes_tk_1 = 0;
	integrator_output_y = 0;
	isDurationTimePassed = 0;
	NLength = 0;/*OUTPUT*/
	BR_ID_omega_max = -1000;
	BR_ID_omega_min = 1000;
	BR_ID_acc_y_max = -4;
	BR_ID_acc_y_min = 4;
	//BR_ID_acc_z_max = -4;
	//BR_ID_acc_z_min = 4;
	BR_flag = false;
	BR_az_start_counting_time_i = -1;
	//no initiliazation for dataSequence 
	memset(a_z_Possible_Peak1.index, 0, N_BR_ID_PEAK_INFO_ELEMENTS);
	memset(a_z_Possible_Peak1.val, 0, N_BR_ID_PEAK_INFO_ELEMENTS);
	memset(a_z_Possible_Peak2.index, 0, N_BR_ID_PEAK_INFO_ELEMENTS);
	memset(a_z_Possible_Peak2.val, 0, N_BR_ID_PEAK_INFO_ELEMENTS);
	memset(omega_x_Possible_Peak1.index, 0, N_BR_ID_PEAK_INFO_ELEMENTS);
	memset(omega_x_Possible_Peak1.val, 0, N_BR_ID_PEAK_INFO_ELEMENTS);
	memset(omega_x_Possible_Peak2.index, 0, N_BR_ID_PEAK_INFO_ELEMENTS);
	memset(omega_x_Possible_Peak2.val, 0, N_BR_ID_PEAK_INFO_ELEMENTS);
	N_a_z_Peak1 = 0;
	N_a_z_Peak2 = 0;
	N_omega_x_Peak1 = 0;
	N_omega_x_Peak2 = 0;
	a_z_Peak1_Sum = 0;
	a_z_Peak2_Sum = 0;
	omega_x_Peak1_Sum = 0;
	omega_x_Peak2_Sum = 0;
	a_z_Peak1_Mean = 0;
	a_z_Peak2_Mean = 0;
	omega_x_Peak1_Mean = 0;
	omega_x_Peak2_Mean = 0;
	omega_x_index1_Sum = 0;
	omega_x_index1_Mean = 0;
	omega_x_index2_Sum = 0;
	omega_x_index2_Mean = 0;
	is_BR_ID_omega_az_MeanCalculationDone = false;
	hasPassedInitialEliminationPeriod = false;
	has_Entered_BR_BT_ID_Ub = false;
	memset(BT_ID_omega_x_PeakCountInfo1.index, 0, BT_ID_N_OMEGA_X_INDEX);
	memset(BT_ID_omega_x_PeakCountInfo1.val, 0, BT_ID_N_OMEGA_X_INDEX);
	BT_ID_omega_x_PeakCountInfo1.N_ele = 0;
	memset(BT_ID_omega_x_PeakCountInfo1.major_index, 0, BT_ID_N_OMEGA_X_INDEX);
	memset(BT_ID_omega_x_PeakCountInfo1.major_val, 0, BT_ID_N_OMEGA_X_INDEX);
	BT_ID_omega_x_PeakCountInfo1.N_major_ele = 0;
	memset(BT_ID_omega_x_PeakCountInfo2.index, 0, BT_ID_N_OMEGA_X_INDEX);
	memset(BT_ID_omega_x_PeakCountInfo2.val, 0, BT_ID_N_OMEGA_X_INDEX);
	BT_ID_omega_x_PeakCountInfo2.N_ele = 0;
	memset(BT_ID_omega_x_PeakCountInfo2.major_index, 0, BT_ID_N_OMEGA_X_INDEX);
	memset(BT_ID_omega_x_PeakCountInfo2.major_val, 0, BT_ID_N_OMEGA_X_INDEX);
	BT_ID_omega_x_PeakCountInfo2.N_major_ele = 0;
	BT_flag = false;
	//--------------------------------------------------------------------------
	//for stroke style identification-------------------------------------------
	//MatCreate(&a_x, WINDOW_SIZE_TIME_I + 1, 1);
	//MatCreate(&a_y, WINDOW_SIZE_TIME_I + 1, 1);
	//MatCreate(&a_z, WINDOW_SIZE_TIME_I + 1, 1);
	//MatCreate(&a_Mag, WINDOW_SIZE_TIME_I + 1, 1);
	//MatCreate(&omega_x, WINDOW_SIZE_TIME_I + 1, 1);
	//MatCreate(&omega_y, WINDOW_SIZE_TIME_I + 1, 1);
	//MatCreate(&omega_z, WINDOW_SIZE_TIME_I + 1, 1);
	//MatCreate(&omega_Mag, WINDOW_SIZE_TIME_I + 1, 1);
	smooth_start_flag = true;
	NSlide = 0;
	smoothStrokeTime_i = MAX_SMOOTH_TIME_I;//assign an extremely large value
	StrokeTypeCountNumber.Num_FS=0;
	StrokeTypeCountNumber.Num_BK=0;
	StrokeTypeCountNumber.Num_BR=0;
	StrokeTypeCountNumber.Num_BT=0;
	swimmingStyle = UnknownStyle;/*OUTPUT*/

	//dynamic bound(the values are determined in the length count function)
	OMEGA_X_DYNAMIC_LOWER_BOUND = -0.0;//		***(7)
	OMEGA_X_DYNAMIC_UPPER_BOUND = 0.0;//		***(8)
	DYNAMIC_SIGNAL_DURATION_TIME = STANDARD_SWIMMING_PERIOD_TIME;//s			***(9)

	/*adjust values according to the length of the swimming pool*/
	if (LengthSwimmingPool>30)//length=40 or 50m
	{
		//variables whose values depend on the length of the swimming pool
		MIN_PEAK_NUM = 10;//starting phase of the stroke count		***(1)
		INITIAL_ELIMINATION_TIME_I = F_SAMPLE_INS * 20;//start counting if time value is greater than this		***(2)
		MIN_LENGTHS_TIME_I_INTERVAL = INITIAL_ELIMINATION_TIME_I;//minimum time interval between any two lengths		***(3)
		INITIAL_ELIMINATION_NSTROKES = 12;//start counting if the number of strokes is greater than this		***(4)
		MIN_LENGTHS_NSTROKES_INTERVAL = INITIAL_ELIMINATION_NSTROKES;//minimum number of strokes between any two lengths		***(5)
		BR_ID_FOR_LENGTH_COUNT_STARTING_NUM_STROKES = 8;//the starting number of strokes to decide if the current sport is breaststroke		***(6)
	}
	else//length=25 or 30m
	{
		MIN_PEAK_NUM = 5;//		***(1)
		INITIAL_ELIMINATION_TIME_I = F_SAMPLE_INS * 10;//	***(2)
		MIN_LENGTHS_TIME_I_INTERVAL = INITIAL_ELIMINATION_TIME_I;//	***(3)
		INITIAL_ELIMINATION_NSTROKES = 8;//		***(4)
		MIN_LENGTHS_NSTROKES_INTERVAL = INITIAL_ELIMINATION_NSTROKES;//	***(5)
		BR_ID_FOR_LENGTH_COUNT_STARTING_NUM_STROKES = 4;//		***(6)
	}
	
	//ALGO_HF_SWIM_PRINTF("MIN_PEAK_NUM = %d\n", MIN_PEAK_NUM);
	//设置原始数据存储类型
  #if defined STORE_ORG_DATA_TEST_VERSION
    Set_StoreData_Flag(2);
  #endif
}
//Preprocessing(convert the reading of 32767 of gyro to 0 when it's keeping still)
void IncidentPreprocessing(short* RawData)
{
	if(abs(*RawData)>=32767)
	{
		*RawData = 0;
	}
}
//--------------1. Getting Data------------------------------------
// Conversion from raw data to acceleration and angular velocity
static void RawDataPreprocessor(short* Input_IMU_Raw_Data, Mat * acc, Mat * omega)
{
	//debug
#ifdef MAT_LEGAL_CHECKING
	assert(acc->row == 3 && acc->col == 1);
	assert(omega->row == 3 && omega->col == 1);
#endif
	DBL_or_FLT temp = 1E-3;
	acc->element[0][0] = Input_IMU_Raw_Data[3] * ACC_COE * temp;
	acc->element[1][0] = Input_IMU_Raw_Data[4] * ACC_COE * temp;
	acc->element[2][0] = Input_IMU_Raw_Data[5] * ACC_COE * temp;

	omega->element[0][0] = Input_IMU_Raw_Data[0] * GYRO_COE * temp;
	omega->element[1][0] = Input_IMU_Raw_Data[1] * GYRO_COE * temp;
	omega->element[2][0] = Input_IMU_Raw_Data[2] * GYRO_COE * temp;
}

//---------------2. Filters------------------------------------
// Butterworth filter for float type
static float ButterworthFilter1(float u_k, const Mat * A, const Mat * B, const Mat * C, float D, Mat * x_k1, float* u_k1)
{
#ifdef MAT_LEGAL_CHECKING
	assert(A->row == 4 && A->col == 4);
	assert(B->row == 4 && B->col == 1);
	assert(C->row == 1 && C->col == 4);
	assert(x_k1->row == 4 && x_k1->col == 1);
#endif

	//calculate x(k)
	Mat x_k;
	MatCreate(&x_k, 4, 1);
	Mat temp1;//temp1=A*x(k-1), where x(k-1) is denoted by x_k1
	MatCreate(&temp1, 4, 1);
	MatMul(A, x_k1, &temp1);
	Mat temp2;//temp2=B*u(k-1), where u(k-1) is denoted by u_k1
	MatCreate(&temp2, 4, 1);
	MatMulNum(B, *u_k1, &temp2);
	MatAdd(&temp1, &temp2, &x_k);//x(k)=A*x(k-1)+B*u(k-1)
		
	//calculate y(k)
	Mat temp3;//temp3=C*x(k), 1-by-1
	MatCreate(&temp3, 1, 1);
	MatMul(C, &x_k, &temp3);
	float y_k;
	y_k = temp3.element[0][0] + D * u_k;//y(k)=C*x(k)+D*u(k)
		
	//reset the values of x_k1 and u_k1
	MatCopy(&x_k, x_k1);//x(k-1)=x(k)
	*u_k1 = u_k;//u(k-1)=u(k)

	//vPortFree memory
	MatDelete(&temp1);
	MatDelete(&temp2);
	MatDelete(&temp3);
	MatDelete(&x_k);

	return y_k;
}

//Butterworth filter for double type
static double ButterworthFilter2(double u_k, const Mat * A, const Mat * B, const Mat * C, double D, Mat * x_k1, double * u_k1)
{
#ifdef MAT_LEGAL_CHECKING
	assert(A->row == 4 && A->col == 4);
	assert(B->row == 4 && B->col == 1);
	assert(C->row == 1 && C->col == 4);
	assert(x_k1->row == 4 && x_k1->col == 1);
#endif

	//calculate x(k)
	Mat x_k;
	MatCreate(&x_k, 4, 1);
	Mat temp1;//temp1=A*x(k-1), where x(k-1) is denoted by x_k1
	MatCreate(&temp1, 4, 1);
	MatMul(A, x_k1, &temp1);
	Mat temp2;//temp2=B*u(k-1), where u(k-1) is denoted by u_k1
	MatCreate(&temp2, 4, 1);
	MatMulNum_DBL(B, *u_k1, &temp2);
	MatAdd(&temp1, &temp2, &x_k);

	//calculate y(k)
	Mat temp3;//temp3=C*x(k), 1-by-1
	MatCreate(&temp3, 1, 1);
	MatMul(C, &x_k, &temp3);
	double y_k;
	y_k = temp3.element[0][0] + D * u_k;//y(k)=C*x(k)+D*u(k)

	//reset the values of x_k1 and u_k1
	MatCopy(&x_k, x_k1);
	*u_k1 = u_k;

	//vPortFree memory
	MatDelete(&x_k);
	MatDelete(&temp1);
	MatDelete(&temp2);
	MatDelete(&temp3);

	return y_k;
}

//Data filter for float type
static void SwimDataFilter1(const Mat * acc, const Mat * omega, const Mat * A, const Mat * B, const Mat * C, float D,
	Mat * accX_x_k1, float * accX_u_k1, Mat * accY_x_k1, float * accY_u_k1, Mat * accZ_x_k1, float * accZ_u_k1,
	Mat * omegaX_x_k1, float * omegaX_u_k1, Mat * omegaY_x_k1, float * omegaY_u_k1, Mat * omegaZ_x_k1, float * omegaZ_u_k1,
	Mat * acc_f, Mat * omega_f)
{
	//filtered acceleration
	acc_f->element[0][0] = ButterworthFilter1(acc->element[0][0], A, B, C, D, accX_x_k1, accX_u_k1);
	acc_f->element[1][0] = ButterworthFilter1(acc->element[1][0], A, B, C, D, accY_x_k1, accY_u_k1);
	acc_f->element[2][0] = ButterworthFilter1(acc->element[2][0], A, B, C, D, accZ_x_k1, accZ_u_k1);
	//filtered angular velocity
	omega_f->element[0][0] = ButterworthFilter1(omega->element[0][0], A, B, C, D, omegaX_x_k1, omegaX_u_k1);
	omega_f->element[1][0] = ButterworthFilter1(omega->element[1][0], A, B, C, D, omegaY_x_k1, omegaY_u_k1);
	omega_f->element[2][0] = ButterworthFilter1(omega->element[2][0], A, B, C, D, omegaZ_x_k1, omegaZ_u_k1);
}


//Data filter for double type
static void SwimDataFilter2(const Mat * acc, const Mat * omega, const Mat * A, const Mat * B, const Mat * C, double D,
	Mat * accX_x_k1, double * accX_u_k1, Mat * accY_x_k1, double * accY_u_k1, Mat * accZ_x_k1, double * accZ_u_k1, 
	Mat * omegaX_x_k1, double * omegaX_u_k1, Mat * omegaY_x_k1, double * omegaY_u_k1, Mat * omegaZ_x_k1, double * omegaZ_u_k1,
	Mat * acc_f, Mat * omega_f)
{
	//filtered acceleration
	acc_f->element[0][0] = ButterworthFilter2(acc->element[0][0], A, B, C, D, accX_x_k1, accX_u_k1);
	acc_f->element[1][0] = ButterworthFilter2(acc->element[1][0], A, B, C, D, accY_x_k1, accY_u_k1);
	acc_f->element[2][0] = ButterworthFilter2(acc->element[2][0], A, B, C, D, accZ_x_k1, accZ_u_k1);
	//filtered angular velocity
	omega_f->element[0][0] = ButterworthFilter2(omega->element[0][0], A, B, C, D, omegaX_x_k1, omegaX_u_k1);
	omega_f->element[1][0] = ButterworthFilter2(omega->element[1][0], A, B, C, D, omegaY_x_k1, omegaY_u_k1);
	omega_f->element[2][0] = ButterworthFilter2(omega->element[2][0], A, B, C, D, omegaZ_x_k1, omegaZ_u_k1);
}

//----------------3. Main Algorithms-----------------------------
//3.1 Number of strokes--------------------------
void StrokeCount1(int time_i, DBL_or_FLT input_val_i, PeakInfo * PossiblePeak1, DataSequence_Structure * inputDataSequence1, int * NStrokes1)
{
	//assign value to inputDataSequence1
	if (time_i <= 0)//when pressing start(in fact it cannot be less than zero)
	{
		inputDataSequence1->k_2 = input_val_i;
		*NStrokes1 = 0;
	}
	else if (time_i == 1)
	{
		inputDataSequence1->k_1 = input_val_i;
	}
	else if (time_i == 2)
	{
		inputDataSequence1->k = input_val_i;
	}
	else//time_i > 2
	{
		inputDataSequence1->k_2 = inputDataSequence1->k_1;
		inputDataSequence1->k_1 = inputDataSequence1->k;
		inputDataSequence1->k = input_val_i;
	}
	inputDataSequence1->diff_B = inputDataSequence1->k_1 - inputDataSequence1->k_2;
	inputDataSequence1->diff_F = inputDataSequence1->k - inputDataSequence1->k_1;

	/*Find peaks*/
	if (time_i > 1)
	{
		if (inputDataSequence1->diff_B > 0 && inputDataSequence1->diff_F < 0 && inputDataSequence1->k_1 >= OMEGA_X_MIN_PEAK_HEIGHT)//get peaks(local maxima and larger than a threshold value)
		{
			if (*NStrokes1 < MAX_INITIAL_PEAK_NUMBER)//in case of finding two many peaks when the algorithm is likely to go wrong
			{
				if (time_i - 1 - PossiblePeak1->index >= MIN_PEAK_DISTANCE)//a new valid peak is found; if this is the first peak, index is not assigned but initialized to 0
				{
					++(*NStrokes1);
					//update the information of the latest peak
					PossiblePeak1->index = time_i - 1;
					PossiblePeak1->val = inputDataSequence1->k_1;
				}
				else
				{
					if (PossiblePeak1->val >= inputDataSequence1->k_1)//the last peak is the major peak(it's higher)
					{
						//do nothing right now
					}
					else
					{
						//update the information of the latest peak
						PossiblePeak1->index = time_i - 1;
						PossiblePeak1->val = inputDataSequence1->k_1;
					}
				}
			}
		}
	}
}


void StrokeCount2(int time_i, DBL_or_FLT input_val_i, PeakInfo * PossiblePeak2, DataSequence_Structure * inputDataSequence2, int * NStrokes2)
{
	input_val_i = -input_val_i;
	StrokeCount1(time_i, input_val_i, PossiblePeak2, inputDataSequence2, NStrokes2);
}


void StrokeCount(int NumLength, int time_i, DBL_or_FLT input_val_i, PeakInfo * PossiblePeak1, DataSequence_Structure * inputDataSequence1, int * NStrokes1, 
	PeakInfo * PossiblePeak2, DataSequence_Structure * inputDataSequence2, int * NStrokes2, int * NStrokes, int MIN_PEAK_NUM)
{
	//temp1 = *NStrokes1;
	//temp2 = *NStrokes2;
	StrokeCount1(time_i, input_val_i, PossiblePeak1, inputDataSequence1, NStrokes1);
	StrokeCount2(time_i, input_val_i, PossiblePeak2, inputDataSequence2, NStrokes2);
	//if (temp2!=*NStrokes2)
	//{
	//	std::cout << PossiblePeak2->index << std::endl;
	//}

	/*dealing with the logic of the two counts*/
	if (*NStrokes1 >= MIN_PEAK_NUM && *NStrokes2 >= MIN_PEAK_NUM) //both stroke counts at least work
	{
		if (abs((*NStrokes1) - (*NStrokes2)) < MIN_STROKE_COUNT_DIFF)//if the difference is small
		{
			(*NStrokes) = ((*NStrokes1) + (*NStrokes2)) / 2;//than use the average
		}
		else//if the difference is large
		{
			(*NStrokes) = MAX_2NO((*NStrokes1), (*NStrokes2));
		}
	}
	else//(*NStrokes1 < MIN_PEAK_NUM && *NStrokes2 < MIN_PEAK_NUM)
	{
		(*NStrokes) = MAX_2NO((*NStrokes1), (*NStrokes2));
	}
	*NStrokes -= NumLength;
	if (*NStrokes < 0)
	{
		*NStrokes = 0;
	}
}
//3.2 Number of lengths----------------------------
//-----------------Preparation for the Length Count----------------------------------------------
//this function is merely used in this file
static void BT_ID_Npeaks_between_major_peaks(int start_counting_time_i, DBL_or_FLT input_val_i, BT_ID_omega_x_PeakInfo * Peak, DataSequence_Structure * inputDataSequence)
{
	//assign value to inputDataSequence
	if (start_counting_time_i == 0)//when pressing start(in fact it cannot be less than zero)
	{
		inputDataSequence->k_2 = input_val_i;
	}
	else if (start_counting_time_i == 1)
	{
		inputDataSequence->k_1 = input_val_i;
	}
	else if (start_counting_time_i == 2)//fixed a bug for stroke count
	{
		inputDataSequence->k = input_val_i;
	}
	else//start_counting_time_i > 2
	{
		inputDataSequence->k_2 = inputDataSequence->k_1;
		inputDataSequence->k_1 = inputDataSequence->k;
		inputDataSequence->k = input_val_i;
	}
	inputDataSequence->diff_B = inputDataSequence->k_1 - inputDataSequence->k_2;
	inputDataSequence->diff_F = inputDataSequence->k - inputDataSequence->k_1;

	/*Find peaks*/
	if (start_counting_time_i > 1)//at least 3 points are found
	{
		//find all the peaks
		if (inputDataSequence->diff_B > 0 && inputDataSequence->diff_F < 0)
		{
			if (Peak->N_ele < BT_ID_N_OMEGA_X_INDEX)//in case of out-of-range error
			{
				//update the information of the latest peak
				Peak->index[Peak->N_ele] = start_counting_time_i - 1;
				Peak->val[Peak->N_ele] = inputDataSequence->k_1;
				++Peak->N_ele;//increase 1 to the peak number
			}
		}

		//only find the major peaks
		if (inputDataSequence->diff_B > 0 && inputDataSequence->diff_F < 0)// && inputDataSequence->k_1 >= OMEGA_X_MIN_PEAK_HEIGHT
		{
			if (Peak->N_major_ele == 0)//in fact it can only equal 0; this judgement is to avoid the situation when the first peak is close to the starting point
			{
				//update the information of the latest peak
				Peak->major_index[Peak->N_major_ele] = start_counting_time_i - 1;
				Peak->major_val[Peak->N_major_ele] = inputDataSequence->k_1;
				++Peak->N_major_ele;//it is likely to be a peak; add 1 to the number no matter it's a major or minor peak
			}
			else if (Peak->N_major_ele > 0 && Peak->N_major_ele < BT_ID_N_OMEGA_X_INDEX)//in case of out-of-range error
			{
				if (start_counting_time_i - 1 - Peak->major_index[Peak->N_major_ele - 1] >= MIN_PEAK_DISTANCE)//a new valid peak is found
				{
					if (Peak->N_major_ele < N_BR_ID_PEAK_INFO_ELEMENTS)//in case of finding too many peaks so that the number exceeds the boundary of the index of PossiblePeak
					{
						//update the information of the latest peak
						Peak->major_index[Peak->N_major_ele] = start_counting_time_i - 1;
						Peak->major_val[Peak->N_major_ele] = inputDataSequence->k_1;
						++Peak->N_major_ele;//it is likely to be a peak; add 1 to the number no matter it's a major or minor peak
					}
				}
				else//a neighborhood peak is found
				{
					if (Peak->major_val[Peak->N_major_ele - 1] >= inputDataSequence->k_1)//the last peak is the major peak(it's higher)
					{
						//do nothing right now
					}
					else//the current peak is the major one
					{
						//update the information of the latest peak(replace the old value with the new one)
						Peak->major_index[Peak->N_major_ele - 1] = start_counting_time_i - 1;
						Peak->major_val[Peak->N_major_ele - 1] = inputDataSequence->k_1;
					}
				}
			}
		}
	}
}

//this function is merely used in this file
static void BR_ID_Find_peak_valley(int start_counting_time_i, DBL_or_FLT input_val_i, BR_ID_PeakInfo* PossiblePeak, DataSequence_Structure* inputDataSequence, int* N_a_z_Peak)
{
	//assign value to inputDataSequence
	if (start_counting_time_i == 0)//when pressing start(in fact it cannot be less than zero)
	{
		inputDataSequence->k_2 = input_val_i;
	}
	else if (start_counting_time_i == 1)
	{
		inputDataSequence->k_1 = input_val_i;
	}
	else if (start_counting_time_i == 2)//fixed a bug for stroke count
	{
		inputDataSequence->k = input_val_i;
	}
	else//start_counting_time_i > 2
	{
		inputDataSequence->k_2 = inputDataSequence->k_1;
		inputDataSequence->k_1 = inputDataSequence->k;
		inputDataSequence->k = input_val_i;
	}
	inputDataSequence->diff_B = inputDataSequence->k_1 - inputDataSequence->k_2;
	inputDataSequence->diff_F = inputDataSequence->k - inputDataSequence->k_1;

	/*Find peaks*/
	if (start_counting_time_i > 1)//at least 3 points are found
	{
		if (inputDataSequence->diff_B > 0 && inputDataSequence->diff_F < 0)// && inputDataSequence->k_1 >= ACC_Z_MIN_PEAK_HEIGHT//get peaks(local maxima and larger than a threshold value)
		{
			if (*N_a_z_Peak == 0)//in fact it can only equal 0; this judgement is to avoid the situation when the first peak is close to the starting point
			{
				//update the information of the latest peak
				PossiblePeak->index[*N_a_z_Peak] = start_counting_time_i - 1;
				PossiblePeak->val[*N_a_z_Peak] = inputDataSequence->k_1;
				++(*N_a_z_Peak);//it is likely to be a peak; add 1 to the number no matter it's a major or minor peak
			}
			else if (*N_a_z_Peak > 0 && *N_a_z_Peak < N_BR_ID_PEAK_INFO_ELEMENTS)//in case of out-of-range error
			{
				if (start_counting_time_i - 1 - PossiblePeak->index[*N_a_z_Peak-1] >= MIN_PEAK_DISTANCE)//a new valid peak is found
				{
					if (*N_a_z_Peak < N_BR_ID_PEAK_INFO_ELEMENTS)//in case of finding too many peaks so that the number exceeds the boundary of the index of PossiblePeak
					{
						//update the information of the latest peak
						PossiblePeak->index[*N_a_z_Peak] = start_counting_time_i - 1;
						PossiblePeak->val[*N_a_z_Peak] = inputDataSequence->k_1;
						++(*N_a_z_Peak);//it is likely to be a peak; add 1 to the number no matter it's a major or minor peak
					}
				}
				else//a neighborhood peak is found
				{
					if (PossiblePeak->val[*N_a_z_Peak-1] >= inputDataSequence->k_1)//the last peak is the major peak(it's higher)
					{
						//do nothing right now
					}
					else//the current peak is the major one
					{
						//update the information of the latest peak(replace the old value with the new one)
						PossiblePeak->index[*N_a_z_Peak-1] = start_counting_time_i - 1;
						PossiblePeak->val[*N_a_z_Peak-1] = inputDataSequence->k_1;
					}
				}
			}
		}
	}
}
//preparation for the length count(main)
void InitialPreparationForLengthCount(int time_i, DBL_or_FLT inputData, DBL_or_FLT inputData1, DBL_or_FLT inputData2, bool* is_BR, int* BR_start_time_i, bool* has_entered_strokeID_upperbound,
	DBL_or_FLT* StrokeID_omega_x_max, DBL_or_FLT* StrokeID_omega_x_min, DBL_or_FLT* StrokeID_acc_y_max, DBL_or_FLT* StrokeID_acc_y_min,
	DataSequence_Structure* omega_x_Sequence1, BR_ID_PeakInfo* omega_x_Peak1, int* N_omega_x_Peak1, BR_ID_PeakInfo* omega_x_Peak2, DataSequence_Structure* omega_x_Sequence2, int* N_omega_x_Peak2,
	DataSequence_Structure* a_z_Sequence1, BR_ID_PeakInfo* a_z_Peak1, int* N_a_z_Peak1, BR_ID_PeakInfo* a_z_Peak2, DataSequence_Structure* a_z_Sequence2, int* N_a_z_Peak2,
	BT_ID_omega_x_PeakInfo* BT_ID_omega_x_Peak1, DataSequence_Structure* BT_ID_omega_x_Sequence1, BT_ID_omega_x_PeakInfo* BT_ID_omega_x_Peak2, DataSequence_Structure* BT_ID_omega_x_Sequence2, bool* is_BT,
	int NStrokes, int* NLength,
	int BR_ID_FOR_LENGTH_COUNT_STARTING_NUM_STROKES)
{
	*NLength = LENGTH_COUNTER_INITIAL_COUNT;//reset the counter to 0

	//decide if the current sport is breaststroke or butterfly stroke
	if (!*has_entered_strokeID_upperbound)
	{
		if (NStrokes == BR_ID_FOR_LENGTH_COUNT_STARTING_NUM_STROKES + BR_ID_DURATION_N_STROKES)
		{
			*has_entered_strokeID_upperbound = true;
		}
		//calculate some necessary parameters
		if (NStrokes >= BR_ID_FOR_LENGTH_COUNT_STARTING_NUM_STROKES && NStrokes <= BR_ID_FOR_LENGTH_COUNT_STARTING_NUM_STROKES + BR_ID_DURATION_N_STROKES)
		{
			//BR-------------------------------
			//update the max and min of omega_x
			if (inputData > *StrokeID_omega_x_max)
			{
				(*StrokeID_omega_x_max) = inputData;
			}
			else if (inputData < *StrokeID_omega_x_min)
			{
				(*StrokeID_omega_x_min) = inputData;
			}
			//update the max and min of acc_y
			if (inputData1 > *StrokeID_acc_y_max)
			{
				(*StrokeID_acc_y_max) = inputData1;
			}
			else if (inputData1 < *StrokeID_acc_y_min)
			{
				(*StrokeID_acc_y_min) = inputData1;
			}

			//make judgement when NStrokes increases to 11(50m)(the max of the if condition that decides if the current sport is breaststroke
			if (!*is_BR)//the initial value of *is_BR=false; once it's set to be true, the following code won't be executed again
			{
				if (NStrokes == BR_ID_FOR_LENGTH_COUNT_STARTING_NUM_STROKES + BR_ID_DURATION_N_STROKES)
				{
					if (abs(*StrokeID_omega_x_max - *StrokeID_omega_x_min) < BR_BT_ID_OMEGA_X_MAX_DIFFERENCE)//BR or BT
					{
						if (abs(*StrokeID_acc_y_max - *StrokeID_acc_y_min) < BR_ID_ACC_Y_MAX_DIFFERENCE)//BR
						{
							*is_BR = true;
							/*--------------DEBUG---------------------------*/
//							std::cout << "----------Breaststroke----------\n";
							/*-----------DEBUG ENDS-------------------------*/
						}
					}
				}
			}

			//BR-------------------
			//collect the peak information of a_z in this period of time no matter if the current sport is breaststroke or butterfly stroke
			//the mechanism was intended to be applied to breaststroke, but it seems it works for butterfly stroke
			++(*BR_start_time_i);//increase from -1 to 0 for the first time
			//a_z(inputData2)
			BR_ID_Find_peak_valley(*BR_start_time_i, inputData2, a_z_Peak1, a_z_Sequence1, N_a_z_Peak1);
			BR_ID_Find_peak_valley(*BR_start_time_i, -inputData2, a_z_Peak2, a_z_Sequence2, N_a_z_Peak2);
			//omega_x(inputData)
			BR_ID_Find_peak_valley(*BR_start_time_i, inputData, omega_x_Peak1, omega_x_Sequence1, N_omega_x_Peak1);
			BR_ID_Find_peak_valley(*BR_start_time_i, -inputData, omega_x_Peak2, omega_x_Sequence2, N_omega_x_Peak2);

			//BT--------------------
			//find all the peaks of omega_x for butterfly stroke identification(omega_x  inputData)
			BT_ID_Npeaks_between_major_peaks(*BR_start_time_i, inputData, BT_ID_omega_x_Peak1, BT_ID_omega_x_Sequence1);
			BT_ID_Npeaks_between_major_peaks(*BR_start_time_i, -inputData, BT_ID_omega_x_Peak2, BT_ID_omega_x_Sequence2);
		}

		//BT identification calculation and judgement
		if (*has_entered_strokeID_upperbound)//add this condition to judge only once
		{
			float temp1 = 0;
			float temp2 = 0;
			//claculate the absolute value of mean
			for (int i = 1; i < BT_ID_omega_x_Peak1->N_major_ele; i++)
			{
				temp1 += abs(BT_ID_omega_x_Peak1->major_val[i]);
			}
			temp1 /= (BT_ID_omega_x_Peak1->N_major_ele - 1);
			for (int i = 1; i < BT_ID_omega_x_Peak2->N_major_ele; i++)
			{
				temp2 += abs(BT_ID_omega_x_Peak2->major_val[i]);
			}
			temp2 /= (BT_ID_omega_x_Peak2->N_major_ele - 1);

			//now it's time to decide which one to use
			BT_ID_omega_x_PeakInfo BT_ID_omega_x_Peak;
			if (temp1 >= temp2)
			{
				BT_ID_omega_x_Peak = *BT_ID_omega_x_Peak1;
			}
			else
			{
				BT_ID_omega_x_Peak = *BT_ID_omega_x_Peak2;
			}

			//BT judgement
			if (!*is_BT)//the initial value is false
			{
				int temp_start_i = 0;
				int temp_end_i = 0;
				//find the starting point
				for (int i = 0; i < BT_ID_omega_x_Peak.N_ele; i++)
				{
					if (BT_ID_omega_x_Peak.major_index[1] == BT_ID_omega_x_Peak.index[i])//eliminate the first major peak because it is likely to be wrong
					{
						temp_start_i = i;
						break;
					}
				}
				//find the end point
				for (int i = BT_ID_omega_x_Peak.N_ele - 1; i >= 0; i--)
				{
					if (BT_ID_omega_x_Peak.major_index[BT_ID_omega_x_Peak.N_major_ele - 1] == BT_ID_omega_x_Peak.index[i])
					{
						temp_end_i = i;
						break;
					}
				}

				//make the judgement
				//the number of minor peaks(the condition is assumed to be sufficient but not necessary
				//numerator: N peaks between the second major peak and the last one - N major peaks in this interval = N minor peaks in this interval
				//denominator: N intervals between the second major peak and the last one
				if ((temp_end_i - temp_start_i + 1 - (BT_ID_omega_x_Peak.N_major_ele - 1)) * 1.0 / (BT_ID_omega_x_Peak.N_major_ele - 1 - 1) >= BT_ID_OMEGA_X_MINOR_PEAKS_IN_INTERVAL - 0.5)
				{
					*is_BT = true;
					/*--------------DEBUG---------------------------*/
//					std::cout << "----------Butterfly Stroke----------\n";
					/*-----------DEBUG ENDS-------------------------*/
				}
			}
		}
	}
}
//------------------------------End--------------------------------------------------------------------

//-----------------------Remove Outliers---------------------------------------------------------------
//ascending sort function applied to double or float arrays
static int cmpfunAsc_DBL_or_FLT(const void *x, const void *y)
{
	DBL_or_FLT xx = *(DBL_or_FLT*)x;
	DBL_or_FLT yy = *(DBL_or_FLT*)y;
	if (xx < yy)
		return -1;
	else if (xx > yy)
		return  1;
	else
		return 0;
}

//calculate the median of an array
static DBL_or_FLT median(DBL_or_FLT y[], int n)
{
	//create a variable of the same values as y
	DBL_or_FLT* y_temp = (DBL_or_FLT*)pvPortMalloc(n * sizeof(DBL_or_FLT));
	for (int i = 0; i < n; i++)
	{
		y_temp[i] = y[i];
	}

	//sort y_temp in ascending order
	qsort(y_temp, n, sizeof(DBL_or_FLT), cmpfunAsc_DBL_or_FLT);

	//calculate the median
	DBL_or_FLT output;
	if (n % 2 == 0) {
		// if there is an even number of elements, return mean of the two elements in the middle
		output = ((y_temp[n / 2] + y_temp[n / 2 - 1]) / 2.0);
	}
	else {
		// else return the element in the middle
		output = y_temp[n / 2];
	}

	//vPortFree memory
	vPortFree(y_temp);
	return output;
}

//remove outliers of an array(ref: Matlab function "rmoutliers")
static void RemoveOutliers(DBL_or_FLT y[], int size_arr_N, bool isRemoved[], int* NKeptPoints)
{
	//median(y), calculate the median of y, a scalar
	DBL_or_FLT y_median = median(y, size_arr_N);
	//|y-median(y)|, a vector
	DBL_or_FLT* absoluteDeviation = (DBL_or_FLT*)pvPortMalloc(size_arr_N * sizeof(DBL_or_FLT));
	for (int i = 0; i < size_arr_N; i++)
	{
		absoluteDeviation[i] = fabs((float)(y[i] - y_median));
	}
	//MAD, Median Absolute Deviation, a scalar
	DBL_or_FLT MAD;
	MAD = median(absoluteDeviation, size_arr_N);
	//scaled MAD(a scalar), which is defined as c*median(abs(A-median(A)))
	DBL_or_FLT scaled_MAD = MAD_THRESHOLD_FACTOR * MAD;
	//to decide which point should be removed
	for (int i = 0; i < size_arr_N; i++)
	{
		if (absoluteDeviation[i] > scaled_MAD)
		{
			isRemoved[i] = true;
		}
		else
		{
			isRemoved[i] = false;
		}
	}
	//vPortFree memory
	vPortFree(absoluteDeviation);

	//-----------------DEBUG---------------------------
	//for (int i = 0; i < size_arr_N; i++)
	//{
	//	std::cout << absoluteDeviation[i] << std::endl;
	//}
	//-----------------DEBUG END-----------------------

	//obtain the number of kept points
	int tempCountN = 0;
	for (int i = 0; i < size_arr_N; i++)
	{
		if (!isRemoved[i])
		{
			++tempCountN;
		}
	}
	(*NKeptPoints) = tempCountN;
}
//-----------------------End---------------------------------------------------------------------------

//-------------------------Different Length Detection Method-------------------------------------------
//FS and BK----------------
static void SmoothDurationTest_omega_x(DBL_or_FLT omega_x, int time_i, int flag_neighbor, int NStrokes,
	DBL_or_FLT* y, int* isDurationTimePassed, int* omega_x_counter_tk, int* omega_x_counter_tk_1, int* omega_x_NStrokes_tk, int* omega_x_NStrokes_tk_1, int* NLength,
	int MIN_LENGTHS_TIME_I_INTERVAL, int MIN_LENGTHS_NSTROKES_INTERVAL)
{
	//get input signal from the interval test
	int u;
	if (omega_x <= OMEGA_X_INPUT_SIGNAL_UPPER_BOUND && omega_x >= OMEGA_X_INPUT_SIGNAL_LOWER_BOUND)
	{
		//actually it should be bool true, 
		//but it's easy to use 1 instead and will be converted automatically later
		u = 1;//true
	}
	else
	{
		u = 0;//false
	}

	//discrete integrator(backward Euler method)
	if (!u)//when u=0
	{
		//reset the output y to its initial state
		*y = DISCRETE_INTEGRATOR_OUTPUT_Y_INITIAL_VALUE;
	}
	else//when u=1
	{
		(*y) = (*y) + STEP_T * u;//y(n)=y(n-1)+Delta(T)*u(n)
	}

	//output saturation(limit output)
	if ((*y) <= DISCRETE_INTEGRATOR_OUTPUT_Y_LOWER_SATURATION_LIMIT)
	{
		(*y) = DISCRETE_INTEGRATOR_OUTPUT_Y_LOWER_SATURATION_LIMIT;
	}
	else if ((*y) >= DISCRETE_INTEGRATOR_OUTPUT_Y_UPPER_SATURATION_LIMIT)
	{
		(*y) = DISCRETE_INTEGRATOR_OUTPUT_Y_UPPER_SATURATION_LIMIT;
	}

	//judge whether satisfy the duration time condition in the last step 
	if (!(*isDurationTimePassed))//isDurationTimePassed==false, duration time test failed in the last step
	{
		//duration time test
		if ((*y) >= INPUT_SIGNAL_DURATION_TIME)//duration time test failed in the current step, in fact it can only equal but never be larger than t_D due to the saturation limit above 
		{
			*isDurationTimePassed = 1;//true, change the value
			//rising edge for the counter is detected(from 0 to 1)
			++(*NLength);

			//test whether the above increasement is reasonable(whether the adjacent lengths are too close)
			if ((*omega_x_counter_tk == 0) && (*omega_x_counter_tk_1 == 0))//01. enter this judgement first
			{
				*omega_x_counter_tk = time_i;//t(k)=t
				*omega_x_NStrokes_tk = NStrokes;//N(k)=N
				//do nothing to "flag_neighbor" and leave it to be its initial value 0(false) because of the initial judgement
			}
			else if (*omega_x_counter_tk > 0)//&& (*omega_x_counter_tk_1 >= 0)always satisfy this condition //02. then enter this judgement  
			{
				//the neighborhood condition is judged by the minimum time and minimum number of strokes between any two adjacent lengths
				if (abs(time_i - (*omega_x_counter_tk)) < MIN_LENGTHS_TIME_I_INTERVAL || abs(NStrokes - (*omega_x_NStrokes_tk) < MIN_LENGTHS_NSTROKES_INTERVAL))
				{
					flag_neighbor = 1;
					//in this condition, do not update the values of the current and last counter time and NStrokes
				}
				else
				{
					//do nothing to "flag_neighbor" and leave it to be its initial value 0(false)

					//assign values to the last counters and NStrokes
					(*omega_x_counter_tk_1) = (*omega_x_counter_tk);//assignment, t(k-1)=t(k)
					(*omega_x_NStrokes_tk_1) = (*omega_x_NStrokes_tk);//assignment, N(k-1)=N(k)

					//another length is found and update the values of the current counter time and NStrokes
					*omega_x_counter_tk = time_i;
					*omega_x_NStrokes_tk = NStrokes;
				}
			}

			//deduct 1 if the neighborhood condition is satisfied
			if (flag_neighbor)
			{
				--(*NLength);
			}
		}
		//else, keep the isDurationTimePassed value to 0, so nothing is done
	}
	else//isDurationTimePassed==true, i.e. duration time test passed in the last step
	{
		//duration time test
		if ((*y) < INPUT_SIGNAL_DURATION_TIME)//duration time test failed in the current step
		{
			*isDurationTimePassed = 0;//false, change the value
		}
		//else, if it continues passing the test, then keep the isDurationTimePassed value of 1 and nothing should be done
	}
}

//--------FS and BK End-----

//BR----------------------
//calculate the upper and lower bound of omega_x and time interval for smooth duration test of BR
static void BR_Only_omega_x_a_z_DynamicBoundForDurationTest(BR_ID_PeakInfo* omega_x_Peak1, int* N_omega_x_Peak1, DBL_or_FLT* omega_x_Peak1_Sum, DBL_or_FLT* omega_x_Peak1_Mean,
	BR_ID_PeakInfo* omega_x_Peak2, int* N_omega_x_Peak2, DBL_or_FLT* omega_x_Peak2_Sum, DBL_or_FLT* omega_x_Peak2_Mean,
	BR_ID_PeakInfo* a_z_Peak1, int* N_a_z_Peak1, DBL_or_FLT* a_z_Peak1_Sum, DBL_or_FLT* a_z_Peak1_Mean, 
	BR_ID_PeakInfo* a_z_Peak2, int* N_a_z_Peak2, DBL_or_FLT* a_z_Peak2_Sum, DBL_or_FLT* a_z_Peak2_Mean,
	int* omega_x_Index1_Sum, int* omega_x_Index1_Mean, int* omega_x_Index2_Sum, int* omega_x_Index2_Mean, 
	bool* isOmegaAzMeanDone,
	DBL_or_FLT* OMEGA_X_DYNAMIC_LOWER_BOUND, DBL_or_FLT* OMEGA_X_DYNAMIC_UPPER_BOUND, DBL_or_FLT* DYNAMIC_SIGNAL_DURATION_TIME)
{
	//remove the outliers first
	bool* isRemoved1 = (bool*)pvPortMalloc((*N_a_z_Peak1) * sizeof(bool));
	bool* isRemoved2 = (bool*)pvPortMalloc((*N_a_z_Peak2) * sizeof(bool));
	bool* isRemoved3 = (bool*)pvPortMalloc((*N_omega_x_Peak1) * sizeof(bool));
	bool* isRemoved4 = (bool*)pvPortMalloc((*N_omega_x_Peak2) * sizeof(bool));
	int N1_kept, N2_kept, N3_kept, N4_kept;

	//the peak indices and values do not match after running the following function
	RemoveOutliers(a_z_Peak1->val, *N_a_z_Peak1, isRemoved1, &N1_kept);
	RemoveOutliers(a_z_Peak2->val, *N_a_z_Peak2, isRemoved2, &N2_kept);
	RemoveOutliers(omega_x_Peak1->val, *N_omega_x_Peak1, isRemoved3, &N3_kept);
	RemoveOutliers(omega_x_Peak2->val, *N_omega_x_Peak2, isRemoved4, &N4_kept);

	//calculate the sum of a_z peaks and valleys
	//peaks
	for (int i = 0; i < *N_a_z_Peak1; i++)
	{
		if (!isRemoved1[i])
		{
			*a_z_Peak1_Sum += a_z_Peak1->val[i];//exclude outliers and add
		}
	}
	//valleys
	for (int i = 0; i < *N_a_z_Peak2; i++)
	{
		if (!isRemoved2[i])
		{
			*a_z_Peak2_Sum += a_z_Peak2->val[i];
		}
	}

	//calculate the sum of omega_x peaks and valleys
	//peaks
	for (int i = 0; i < *N_omega_x_Peak1; i++)
	{
		if (!isRemoved3[i])
		{
			*omega_x_Peak1_Sum += omega_x_Peak1->val[i];//exclude outliers and add
			//*omega_x_Index1_Sum += omega_x_Peak1->index[i];
		}
	}
	//valleys
	for (int i = 0; i < *N_omega_x_Peak2; i++)
	{
		if (!isRemoved4[i])
		{
			*omega_x_Peak2_Sum += omega_x_Peak2->val[i];
			//*omega_x_Index2_Sum += omega_x_Peak2->index[i];
		}
	}
	//index(t_end - t_first)
	//find the first valid index
	for (int i = 0; i < *N_omega_x_Peak1; i++)
	{
		if (!isRemoved3[i])
		{
			*omega_x_Index1_Sum -= omega_x_Peak1->index[i];
			break;
		}
	}
	//find the last valid index
	for (int i = *N_omega_x_Peak1 - 1; i >= 0; i--)
	{
		if (!isRemoved3[i])
		{
			*omega_x_Index1_Sum += omega_x_Peak1->index[i];
			break;
		}
	}
	//same process as above
	//find the first valid index
	for (int i = 0; i < *N_omega_x_Peak2; i++)
	{
		if (!isRemoved4[i])
		{
			*omega_x_Index2_Sum -= omega_x_Peak2->index[i];
			break;
		}
	}
	//find the last valid index
	for (int i = *N_omega_x_Peak2 - 1; i >= 0; i--)
	{
		if (!isRemoved4[i])
		{
			*omega_x_Index2_Sum += omega_x_Peak2->index[i];
			break;
		}
	}

	//inverse the negative peaks to obtain valleys
	*a_z_Peak2_Sum = -(*a_z_Peak2_Sum);
	*omega_x_Peak2_Sum = -(*omega_x_Peak2_Sum);

	//calculate the mean of a_z peaks and valleys
	*a_z_Peak1_Mean = (*a_z_Peak1_Sum) / N1_kept;
	*a_z_Peak2_Mean = (*a_z_Peak2_Sum) / N2_kept;
	//calculate the mean of omega_x peaks and valleys
	*omega_x_Peak1_Mean = (*omega_x_Peak1_Sum) / N3_kept;
	*omega_x_Peak2_Mean = (*omega_x_Peak2_Sum) / N4_kept;
	//calculate the mean of the indices of omega_x peaks
	*omega_x_Index1_Mean = (*omega_x_Index1_Sum) / (N3_kept - 1);
	*omega_x_Index2_Mean = (*omega_x_Index2_Sum) / (N4_kept - 1);

	//determine the smooth time duration value
	if (abs(*omega_x_Index1_Mean - STANDARD_SWIMMING_PERIOD_TIME * F_SAMPLE_INS) <= abs(*omega_x_Index2_Mean - STANDARD_SWIMMING_PERIOD_TIME * F_SAMPLE_INS))
	{
		if (abs(*omega_x_Index1_Mean - STANDARD_SWIMMING_PERIOD_TIME * F_SAMPLE_INS) >= DYNAMIC_SIGNAL_DURATION_TIME_DEVIATION_LB * F_SAMPLE_INS &&
			abs(*omega_x_Index1_Mean - STANDARD_SWIMMING_PERIOD_TIME * F_SAMPLE_INS) <= DYNAMIC_SIGNAL_DURATION_TIME_DEVIATION_UB * F_SAMPLE_INS)
		{
			*DYNAMIC_SIGNAL_DURATION_TIME = (*omega_x_Index1_Mean)*1.0 / F_SAMPLE_INS;
		}
		else
		{
			*DYNAMIC_SIGNAL_DURATION_TIME = STANDARD_SWIMMING_PERIOD_TIME;
		}
	}
	else
	{
		if (abs(*omega_x_Index2_Mean - STANDARD_SWIMMING_PERIOD_TIME * F_SAMPLE_INS) >= DYNAMIC_SIGNAL_DURATION_TIME_DEVIATION_LB * F_SAMPLE_INS &&
			abs(*omega_x_Index2_Mean - STANDARD_SWIMMING_PERIOD_TIME * F_SAMPLE_INS) <= DYNAMIC_SIGNAL_DURATION_TIME_DEVIATION_UB * F_SAMPLE_INS)
		{
			*DYNAMIC_SIGNAL_DURATION_TIME = (*omega_x_Index2_Mean)*1.0 / F_SAMPLE_INS;
		}
		else
		{
			*DYNAMIC_SIGNAL_DURATION_TIME = STANDARD_SWIMMING_PERIOD_TIME;
		}
	}

	//determine the values of the upper and lower bound of the interval check
	if (*omega_x_Peak1_Mean >= *omega_x_Peak2_Mean)//hopefully, this will happen
	{
		if (abs(*omega_x_Peak1_Mean) >= abs(*omega_x_Peak2_Mean))
		{
			*OMEGA_X_DYNAMIC_UPPER_BOUND = (*omega_x_Peak1_Mean)*DYNAMIC_ABS_GREATER_BOUND_COEF;
			*OMEGA_X_DYNAMIC_LOWER_BOUND = (*omega_x_Peak2_Mean)*DYNAMIC_ABS_LESS_BOUND_COEF;
		}
		else
		{
			*OMEGA_X_DYNAMIC_UPPER_BOUND = (*omega_x_Peak1_Mean)*DYNAMIC_ABS_LESS_BOUND_COEF;
			*OMEGA_X_DYNAMIC_LOWER_BOUND = (*omega_x_Peak2_Mean)*DYNAMIC_ABS_GREATER_BOUND_COEF;
		}
	}
	else//(*omega_x_Peak1_Mean < *omega_x_Peak2_Mean)
	{
		*OMEGA_X_DYNAMIC_UPPER_BOUND = 20;//just set a fixed value
		*OMEGA_X_DYNAMIC_LOWER_BOUND = -20;
	}

	//turn off this condition block
	(*isOmegaAzMeanDone) = true;

	//vPortFree memory
	vPortFree(isRemoved1);
	vPortFree(isRemoved2);
	vPortFree(isRemoved3);
	vPortFree(isRemoved4);
}

//smooth time interval test using omega_x
static void SmoothDurationTest_omega_x_DynamicBound(DBL_or_FLT omega_x, int time_i, int flag_neighbor, int NStrokes,
	DBL_or_FLT* y, int* isDurationTimePassed, int* omega_x_counter_tk, int* omega_x_counter_tk_1, int* omega_x_NStrokes_tk, int* omega_x_NStrokes_tk_1, int* NLength,
	DBL_or_FLT* OMEGA_X_DYNAMIC_LOWER_BOUND, DBL_or_FLT* OMEGA_X_DYNAMIC_UPPER_BOUND, DBL_or_FLT* DYNAMIC_SIGNAL_DURATION_TIME, int MIN_LENGTHS_TIME_I_INTERVAL, int MIN_LENGTHS_NSTROKES_INTERVAL)
{
	//get input signal from the interval test
	int u;
	if (omega_x <= *OMEGA_X_DYNAMIC_UPPER_BOUND && omega_x >= *OMEGA_X_DYNAMIC_LOWER_BOUND)
	{
		//actually it should be bool true, 
		//but it's easy to use 1 instead and will be converted automatically later
		u = 1;//true
	}
	else
	{
		u = 0;//false
	}

	//discrete integrator(backward Euler method)
	if (!u)//when u=0
	{
		//reset the output y to its initial state
		*y = DISCRETE_INTEGRATOR_OUTPUT_Y_INITIAL_VALUE;
	}
	else//when u=1
	{
		(*y) = (*y) + STEP_T * u;//y(n)=y(n-1)+Delta(T)*u(n)
	}

	//output saturation(limit output)
	DBL_or_FLT dynamic_DISCRETE_INTEGRATOR_OUTPUT_Y_UPPER_SATURATION_LIMIT = *DYNAMIC_SIGNAL_DURATION_TIME;//once it is calculated, the value won't be altered again
	if ((*y) <= DISCRETE_INTEGRATOR_OUTPUT_Y_LOWER_SATURATION_LIMIT)
	{
		(*y) = DISCRETE_INTEGRATOR_OUTPUT_Y_LOWER_SATURATION_LIMIT;
	}
	else if ((*y) >= dynamic_DISCRETE_INTEGRATOR_OUTPUT_Y_UPPER_SATURATION_LIMIT)
	{
		(*y) = dynamic_DISCRETE_INTEGRATOR_OUTPUT_Y_UPPER_SATURATION_LIMIT;
	}

	//judge whether satisfy the duration time condition in the last step 
	if (!(*isDurationTimePassed))//isDurationTimePassed==false, duration time test failed in the last step
	{
		//duration time test
		if ((*y) >= *DYNAMIC_SIGNAL_DURATION_TIME)//duration time test failed in the current step, in fact it can only equal but never be larger than t_D due to the saturation limit above 
		{
			*isDurationTimePassed = 1;//true, change the value
			//rising edge for the counter is detected(from 0 to 1)
			++(*NLength);

			//test whether the above increasement is reasonable(whether the adjacent lengths are too close)
			if ((*omega_x_counter_tk == 0) && (*omega_x_counter_tk_1 == 0))//01. enter this judgement first
			{
				*omega_x_counter_tk = time_i;//t(k)=t
				*omega_x_NStrokes_tk = NStrokes;//N(k)=N
				//do nothing to "flag_neighbor" and leave it to be its initial value 0(false) because of the initial judgement
			}
			else if (*omega_x_counter_tk > 0)//&& (*omega_x_counter_tk_1 >= 0)always satisfy this condition //02. then enter this judgement  
			{
				//the neighborhood condition is judged by the minimum time and minimum number of strokes between any two adjacent lengths
				if (abs(time_i - (*omega_x_counter_tk)) < MIN_LENGTHS_TIME_I_INTERVAL || abs(NStrokes - (*omega_x_NStrokes_tk) < MIN_LENGTHS_NSTROKES_INTERVAL))
				{
					flag_neighbor = 1;
					//in this condition, do not update the values of the current and last counter time and NStrokes
				}
				else
				{
					//do nothing to "flag_neighbor" and leave it to be its initial value 0(false)

					//assign values to the last counters and NStrokes
					(*omega_x_counter_tk_1) = (*omega_x_counter_tk);//assignment, t(k-1)=t(k)
					(*omega_x_NStrokes_tk_1) = (*omega_x_NStrokes_tk);//assignment, N(k-1)=N(k)

					//another length is found and update the values of the current counter time and NStrokes
					*omega_x_counter_tk = time_i;
					*omega_x_NStrokes_tk = NStrokes;
				}
			}

			//deduct 1 if the neighborhood condition is satisfied
			if (flag_neighbor)
			{
				--(*NLength);
			}
		}
		//else, keep the isDurationTimePassed value to 0, so nothing is done
	}
	else//isDurationTimePassed==true, i.e. duration time test passed in the last step
	{
		//duration time test
		if ((*y) < *DYNAMIC_SIGNAL_DURATION_TIME)//duration time test failed in the current step
		{
			*isDurationTimePassed = 0;//false, change the value
		}
		//else, if it continues passing the test, then keep the isDurationTimePassed value of 1 and nothing should be done
	}
}
//---------BR End---------

//BT----------------------
//calculate the average of the peak and valley value of a_z for BT
static void BT_Average_acc_z_Peak_Valley(BR_ID_PeakInfo* a_z_Peak1, int* N_a_z_Peak1, DBL_or_FLT* a_z_Peak1_Sum, DBL_or_FLT* a_z_Peak1_Mean,
	BR_ID_PeakInfo* a_z_Peak2, int* N_a_z_Peak2, DBL_or_FLT* a_z_Peak2_Sum, DBL_or_FLT* a_z_Peak2_Mean, bool* isOmegaAzMeanDone)
{
	//remove the outliers first
	bool* isRemoved1 = (bool*)pvPortMalloc((*N_a_z_Peak1) * sizeof(bool));
	bool* isRemoved2 = (bool*)pvPortMalloc((*N_a_z_Peak2) * sizeof(bool));
	int N1_kept, N2_kept;

	//the peak indices and values do not match after running the following function
	RemoveOutliers(a_z_Peak1->val, *N_a_z_Peak1, isRemoved1, &N1_kept);
	RemoveOutliers(a_z_Peak2->val, *N_a_z_Peak2, isRemoved2, &N2_kept);

	//calculate the sum of a_z peaks and valleys
	//peaks
	for (int i = 0; i < *N_a_z_Peak1; i++)
	{
		if (!isRemoved1[i])
		{
			*a_z_Peak1_Sum += a_z_Peak1->val[i];//exclude outliers and add
		}
	}
	//valleys
	for (int i = 0; i < *N_a_z_Peak2; i++)
	{
		if (!isRemoved2[i])
		{
			*a_z_Peak2_Sum += a_z_Peak2->val[i];
		}
	}

	//inverse the negative peaks to obtain valleys
	*a_z_Peak2_Sum = -(*a_z_Peak2_Sum);

	//calculate the mean of a_z peaks and valleys
	*a_z_Peak1_Mean = (*a_z_Peak1_Sum) / N1_kept;
	*a_z_Peak2_Mean = (*a_z_Peak2_Sum) / N2_kept;

	(*isOmegaAzMeanDone) = true;

	//vPortFree memory
	vPortFree(isRemoved1);
	vPortFree(isRemoved2);
}
//--------BT End---------

//BR and BT---------------------
//abrupt change detection using acc_z
static void AbruptChangeProcessing_a_z(DBL_or_FLT acc_z, int time_i, int flag_neighbor, int NStrokes,
	int* a_z_counter_tk, int* a_z_counter_tk_1, int* a_z_NStrokes_tk, int* a_z_NStrokes_tk_1, int* NLength,
	int MIN_LENGTHS_TIME_I_INTERVAL, int MIN_LENGTHS_NSTROKES_INTERVAL)
{
	++(*NLength);
	if ((*a_z_counter_tk == 0) && (*a_z_counter_tk_1 == 0))
	{
		*a_z_counter_tk = time_i;//t(k)=t
		*a_z_NStrokes_tk = NStrokes;//N(k)=N
	}
	else if (a_z_counter_tk > 0)//looking for the new t(k) which represents the newly found length
	{
		if (abs(time_i - (*a_z_counter_tk)) < MIN_LENGTHS_TIME_I_INTERVAL || abs(NStrokes - (*a_z_NStrokes_tk) < MIN_LENGTHS_NSTROKES_INTERVAL))
		{
			flag_neighbor = 1;
		}
		else
		{
			(*a_z_counter_tk_1) = (*a_z_counter_tk);//assignment, t(k-1)=t(k)
			(*a_z_NStrokes_tk_1) = (*a_z_NStrokes_tk);//assignment, N(k-1)=N(k)

			*a_z_counter_tk = time_i;
			*a_z_NStrokes_tk = NStrokes;
		}
	}
	//if two consecutive length are too close
	if (flag_neighbor)
	{
		--(*NLength);
	}
}
//---------BR and BT End----------
//-------------------------------------End---------------------------------------------------------------

//================================The Main Length Count Function=========================================
void LengthCount(int time_i, DBL_or_FLT inputData, DBL_or_FLT inputData1, DBL_or_FLT inputData2, bool* is_BR, int* BR_start_time_i, bool* has_passed_initial_elimination, bool* has_entered_strokeID_upperbound,
	DBL_or_FLT* StrokeID_omega_x_max, DBL_or_FLT* StrokeID_omega_x_min, DBL_or_FLT* StrokeID_acc_y_max, DBL_or_FLT* StrokeID_acc_y_min,
	DataSequence_Structure* omega_x_Sequence1, BR_ID_PeakInfo* omega_x_Peak1, int* N_omega_x_Peak1, DBL_or_FLT* omega_x_Peak1_Sum, DBL_or_FLT* omega_x_Peak1_Mean, BR_ID_PeakInfo* omega_x_Peak2, DataSequence_Structure* omega_x_Sequence2, int* N_omega_x_Peak2, DBL_or_FLT* omega_x_Peak2_Sum, DBL_or_FLT* omega_x_Peak2_Mean,
	int* omega_x_Index1_Sum, int* omega_x_Index1_Mean, int* omega_x_Index2_Sum, int* omega_x_Index2_Mean,
	DataSequence_Structure* a_z_Sequence1, BR_ID_PeakInfo* a_z_Peak1, int* N_a_z_Peak1, DBL_or_FLT* a_z_Peak1_Sum, DBL_or_FLT* a_z_Peak1_Mean, BR_ID_PeakInfo* a_z_Peak2, DataSequence_Structure* a_z_Sequence2, int* N_a_z_Peak2, DBL_or_FLT* a_z_Peak2_Sum, DBL_or_FLT* a_z_Peak2_Mean, bool* isOmegaAzMeanDone,
	BT_ID_omega_x_PeakInfo* BT_ID_omega_x_Peak1, DataSequence_Structure* BT_ID_omega_x_Sequence1, BT_ID_omega_x_PeakInfo* BT_ID_omega_x_Peak2, DataSequence_Structure* BT_ID_omega_x_Sequence2, bool* is_BT,
	int* omega_x_counter_tk, int* omega_x_counter_tk_1, int* omega_x_NStrokes_tk, int* omega_x_NStrokes_tk_1,
	int* a_z_counter_tk, int* a_z_counter_tk_1, int* a_z_NStrokes_tk, int* a_z_NStrokes_tk_1, 
	DBL_or_FLT* y, int* isDurationTimePassed, int NStrokes, int* NLength,
	int INITIAL_ELIMINATION_TIME_I, int MIN_LENGTHS_TIME_I_INTERVAL, int INITIAL_ELIMINATION_NSTROKES, int MIN_LENGTHS_NSTROKES_INTERVAL, int BR_ID_FOR_LENGTH_COUNT_STARTING_NUM_STROKES,
	DBL_or_FLT* OMEGA_X_DYNAMIC_LOWER_BOUND, DBL_or_FLT* OMEGA_X_DYNAMIC_UPPER_BOUND, DBL_or_FLT* DYNAMIC_SIGNAL_DURATION_TIME)
{
	if (time_i < INITIAL_ELIMINATION_TIME_I || NStrokes < INITIAL_ELIMINATION_NSTROKES)//reset the counter to 0 and prepare for BR identification(length test preparation)
	{
		if (!*has_passed_initial_elimination)//preparation for the length test
		{
			//(1) Stroke type jugement
			//(2) Calculation of the valleys and peaks of omega_x and a_z
			InitialPreparationForLengthCount(time_i, inputData, inputData1, inputData2, is_BR, BR_start_time_i, has_entered_strokeID_upperbound,
				StrokeID_omega_x_max, StrokeID_omega_x_min, StrokeID_acc_y_max, StrokeID_acc_y_min,
				omega_x_Sequence1, omega_x_Peak1, N_omega_x_Peak1, omega_x_Peak2, omega_x_Sequence2, N_omega_x_Peak2,
				a_z_Sequence1, a_z_Peak1, N_a_z_Peak1, a_z_Peak2, a_z_Sequence2, N_a_z_Peak2,
				BT_ID_omega_x_Peak1, BT_ID_omega_x_Sequence1, BT_ID_omega_x_Peak2, BT_ID_omega_x_Sequence2, is_BT,
				NStrokes, NLength,
				BR_ID_FOR_LENGTH_COUNT_STARTING_NUM_STROKES);
		}
	}
	else//length test
	{
		//to ensure not to re-enter the above if condition
		*has_passed_initial_elimination = true;

		//set the neighbor condition to be false in advance every time and decide if the value should be altered later
		int flag_neighbor = 0;

		if (!*is_BR && !*is_BT)//FS or BK
		{
			//length detection with the duration test with fixed boundary condition
			SmoothDurationTest_omega_x(inputData, time_i, flag_neighbor, NStrokes, y, isDurationTimePassed, omega_x_counter_tk, omega_x_counter_tk_1, omega_x_NStrokes_tk, omega_x_NStrokes_tk_1, NLength,
				MIN_LENGTHS_TIME_I_INTERVAL, MIN_LENGTHS_NSTROKES_INTERVAL);
		}
		else if (*is_BR && !*is_BT)//BR
		{
			//preparation for the length count of breaststroke 
			if (!(*isOmegaAzMeanDone))//prepare for the following "if else"
			{
				//(1) calculate the mean of the peak/valley of a_z and omega_x(enter this condition once)
				//(2) calculate the dynamic boundary values of omega_x
				BR_Only_omega_x_a_z_DynamicBoundForDurationTest(omega_x_Peak1, N_omega_x_Peak1, omega_x_Peak1_Sum, omega_x_Peak1_Mean,
					omega_x_Peak2, N_omega_x_Peak2, omega_x_Peak2_Sum, omega_x_Peak2_Mean,
					a_z_Peak1, N_a_z_Peak1, a_z_Peak1_Sum, a_z_Peak1_Mean,
					a_z_Peak2, N_a_z_Peak2, a_z_Peak2_Sum, a_z_Peak2_Mean,
					omega_x_Index1_Sum, omega_x_Index1_Mean, omega_x_Index2_Sum, omega_x_Index2_Mean,
					isOmegaAzMeanDone,
					OMEGA_X_DYNAMIC_LOWER_BOUND, OMEGA_X_DYNAMIC_UPPER_BOUND, DYNAMIC_SIGNAL_DURATION_TIME);
			}

			//a flag of length increasement
			bool BR_Length_Increasement_flag = false;

			//detect new length using 2 methods
			if (inputData2 > *a_z_Peak1_Mean + ACC_Z_SUDDEN_PEAK || inputData2 < *a_z_Peak2_Mean - ACC_Z_SUDDEN_PEAK)
			{
				int temp1 = *NLength;

				//(1) a sudden increase / decrease is detected using a_z(primarily used)
				AbruptChangeProcessing_a_z(inputData2, time_i, flag_neighbor, NStrokes, a_z_counter_tk, a_z_counter_tk_1, a_z_NStrokes_tk, a_z_NStrokes_tk_1, NLength,
					MIN_LENGTHS_TIME_I_INTERVAL, MIN_LENGTHS_NSTROKES_INTERVAL);

				//detect length change
				if (temp1 != *NLength)
				{
					BR_Length_Increasement_flag = true;
				}
			}
			else
			{
				int temp2 = *NLength;

				//(2) smooth time interval condition : using omega_x
				SmoothDurationTest_omega_x_DynamicBound(inputData, time_i, flag_neighbor, NStrokes, y, isDurationTimePassed, omega_x_counter_tk, omega_x_counter_tk_1, omega_x_NStrokes_tk, omega_x_NStrokes_tk_1, NLength,
					OMEGA_X_DYNAMIC_LOWER_BOUND, OMEGA_X_DYNAMIC_UPPER_BOUND, DYNAMIC_SIGNAL_DURATION_TIME, MIN_LENGTHS_TIME_I_INTERVAL, MIN_LENGTHS_NSTROKES_INTERVAL);

				//detect length change
				if (temp2 != *NLength)
				{
					BR_Length_Increasement_flag = true;
				}
			}

			//in case of repeated detection of the same length
			if (BR_Length_Increasement_flag)
			{
				if (abs(*a_z_counter_tk - *omega_x_counter_tk) < MIN_LENGTHS_TIME_I_INTERVAL || abs(*a_z_NStrokes_tk - *omega_x_NStrokes_tk) < MIN_LENGTHS_NSTROKES_INTERVAL)
				{
					--(*NLength);
				}
			}

		}
		else//(*is_BT) *is_BR can be true or false
		{
			//preparation for the length count of butterfly stroke 
			if (!(*isOmegaAzMeanDone))
			{
				//calculate the mean of the peak/valley of a_z
				BT_Average_acc_z_Peak_Valley(a_z_Peak1, N_a_z_Peak1, a_z_Peak1_Sum, a_z_Peak1_Mean,
					a_z_Peak2, N_a_z_Peak2, a_z_Peak2_Sum, a_z_Peak2_Mean, isOmegaAzMeanDone);
			}

			//calculate new length by detecting the abrupt change of a_z
			if (inputData2 > *a_z_Peak1_Mean + ACC_Z_SUDDEN_PEAK || inputData2 < *a_z_Peak2_Mean - ACC_Z_SUDDEN_PEAK)//a sudden increase/decrease is detected
			{
				AbruptChangeProcessing_a_z(inputData2, time_i, flag_neighbor, NStrokes, a_z_counter_tk, a_z_counter_tk_1, a_z_NStrokes_tk, a_z_NStrokes_tk_1, NLength, 
					MIN_LENGTHS_TIME_I_INTERVAL, MIN_LENGTHS_NSTROKES_INTERVAL);
			}
		}
	}
	
	//in case of negative length
	if (*NLength < 0)
	{
		*NLength = 0;
	}
}

//============================The Post-processing Function===============================================
void LengthCountPostProcessing(int end_time_i, int NStrokes, int isDurationTimePassed_END, bool is_BR, bool is_BT,
	int* final_omega_x_counter_tk, int* final_omega_x_counter_tk_1, int* final_omega_x_NStrokes_tk, int* final_omega_x_NStrokes_tk_1,
	int* final_a_z_counter_tk, int* final_a_z_counter_tk_1, int* final_a_z_NStrokes_tk, int* final_a_z_NStrokes_tk_1,
	int MIN_LENGTHS_TIME_I_INTERVAL, int MIN_LENGTHS_NSTROKES_INTERVAL, int* NLength) 
{
	if (!is_BR && !is_BT)//FS or BK
	{
		if (!isDurationTimePassed_END && abs(end_time_i - *final_omega_x_counter_tk) > MIN_LENGTHS_TIME_I_INTERVAL && abs(NStrokes - *final_omega_x_NStrokes_tk) > MIN_LENGTHS_NSTROKES_INTERVAL)
		{
			++(*NLength);//the parentheses are unnecessary; they are added here to make the code clearer
			//update the value of some variables
			*final_omega_x_counter_tk_1 = *final_omega_x_counter_tk;
			*final_omega_x_counter_tk = end_time_i;
			*final_omega_x_NStrokes_tk_1 = *final_omega_x_NStrokes_tk;
			*final_omega_x_NStrokes_tk = NStrokes;
		}
	}
	else if (is_BR && !is_BT)//BR
	{
		//find the larger value of the two counter time
		int t_larger, N_strokes_larger;
		bool is_a_z_larger = false;
		if (*final_a_z_counter_tk > *final_omega_x_counter_tk)
		{
			t_larger = *final_a_z_counter_tk;
			N_strokes_larger = *final_a_z_NStrokes_tk;
			is_a_z_larger = true;
		}
		else
		{
			t_larger = *final_omega_x_counter_tk;
			N_strokes_larger = *final_omega_x_NStrokes_tk;
			is_a_z_larger = false;
		}
		if (abs(end_time_i - t_larger) > MIN_LENGTHS_TIME_I_INTERVAL && abs(NStrokes - N_strokes_larger) > MIN_LENGTHS_NSTROKES_INTERVAL)
		{
			++(*NLength);
			if (is_a_z_larger)
			{
				*final_a_z_counter_tk_1 = *final_a_z_counter_tk;
				*final_a_z_counter_tk = end_time_i;
				*final_a_z_NStrokes_tk_1 = *final_a_z_NStrokes_tk;
				*final_a_z_NStrokes_tk = NStrokes;
			}
			else
			{
				*final_omega_x_counter_tk_1 = *final_omega_x_counter_tk;
				*final_omega_x_counter_tk = end_time_i;
				*final_omega_x_NStrokes_tk_1 = *final_omega_x_NStrokes_tk;
				*final_omega_x_NStrokes_tk = NStrokes;
			}
		}

	}
	else//BT
	{
		if (abs(end_time_i - *final_a_z_counter_tk) > MIN_LENGTHS_TIME_I_INTERVAL && abs(NStrokes - *final_a_z_NStrokes_tk) > MIN_LENGTHS_NSTROKES_INTERVAL)
		{
			++(*NLength);
			*final_a_z_counter_tk_1 = *final_a_z_counter_tk;
			*final_a_z_counter_tk = end_time_i;
			*final_a_z_NStrokes_tk_1 = *final_a_z_NStrokes_tk;
			*final_a_z_NStrokes_tk = NStrokes;
		}
	}
}



//3.3 Stroke style
//Degrees to radians
static DBL_or_FLT Deg2Rad(DBL_or_FLT angle_deg)
{
	DBL_or_FLT angle_rad;
	angle_rad = PI_HF * angle_deg / 180.f;
	return angle_rad;
}
//Decision Tree algorithm
static StrokeID DecisionTree_C45(Mat * a_x_min, Mat * a_x_max, Mat * a_z_min, Mat * a_z_max, Mat * a_Mag_max, Mat * omega_x_max, Mat * omega_x_var, Mat * omega_y_max, Mat * omega_y_mean, Mat * omega_y_var, Mat * omega_z_min, Mat * omega_z_var, Mat * omega_Mag_min, Mat * omega_Mag_max)
{
	StrokeID StrokeType;
	if (omega_y_mean->element[0][0] <= 0.255643f)
	{
		if (omega_x_var->element[0][0] <= 2.736766f)
		{
			if (a_z_min->element[0][0] <= -1.744f)
			{
				if (omega_z_var->element[0][0] <= 1.461977f)
				{
					StrokeType = Backstroke;
				}
				else//omega_z_var > 1.461977
				{
					StrokeType = Breaststroke;
				}
			}
			else//a_z_min > -1.744
			{
				if (a_z_max->element[0][0] <= 2.248044f)
				{
					if (a_x_max->element[0][0] <= 0.089009f)
					{
						if (omega_Mag_max->element[0][0] <= 4.007098f)
						{
							if (omega_Mag_min->element[0][0] <= 0.227712f)
							{
								StrokeType = Breaststroke;
							}
							else//omega_Mag_min > 0.227712
							{
								StrokeType = Backstroke;
							}
						}
						else//omega_Mag_max > 4.007098
						{
							StrokeType = Breaststroke;
						}
					}
					else//a_x_max > 0.089009
					{
						if (omega_x_max->element[0][0] <= 2.900551f)
						{
							StrokeType = Breaststroke;
						}
						else//omega_x_max > 2.900551
						{
							if (a_x_min->element[0][0] <= -1.907003f)
							{
								StrokeType = ButterflyStroke;
							}
							else//a_x_min > -1.907003
							{
								StrokeType = Breaststroke;
							}
						}
					}
				}
				else//a_z_max > 2.248044
				{
					if (a_Mag_max->element[0][0] <= 2.621053f)
					{
						StrokeType = Backstroke;
					}
					else//a_Mag_max > 2.621053
					{
						if (a_z_max->element[0][0] <= 2.318463f)
						{
							StrokeType = Breaststroke;
						}
						else//a_z_max > 2.318463
						{
							StrokeType = ButterflyStroke;
						}
					}
				}
			}
		}
		else//omega_x_var > 2.736766
		{
			if (a_x_min->element[0][0] <= -1.895367f)
			{
				StrokeType = Backstroke;
			}
			else//a_x_min > -1.895367
			{
				if (omega_z_min->element[0][0] <= -2.705941f)
				{
					StrokeType = Breaststroke;
				}
				else//omega_z_min > -2.705941
				{
					if (a_z_min->element[0][0] <= -1.337918f)
					{
						StrokeType = Backstroke;
					}
					else//a_z_min > -1.337918
					{
						StrokeType = Freestyle;
					}
				}
			}
		}
	}
	else//omega_y_mean > 0.255643
	{
		if (omega_x_var->element[0][0] <= 0.881634f)
		{
			if (omega_y_max->element[0][0] <= 1.724999f)
			{
				StrokeType = Backstroke;
			}
			else//omega_y_max > 1.724999
			{
				StrokeType = Breaststroke;
			}
		}
		else//omega_x_var > 0.881634
		{
			if (omega_y_var->element[0][0] <= 3.357443f)
			{
				if (a_z_max->element[0][0] <= 2.417798f)
				{
					if (a_z_max->element[0][0] <= 1.332581f)
					{
						if (a_z_min->element[0][0] <= -1.04275f)
						{
							StrokeType = Breaststroke;
						}
						else//a_z_min > -1.04275
						{
							StrokeType = Freestyle;
						}
					}
					else//a_z_max > 1.332581
					{
						StrokeType = Freestyle;
					}
				}
				else//a_z_max > 2.417798
				{
					if (a_z_max->element[0][0] <= 2.448867f)
					{
						StrokeType = Freestyle;
					}
					else//a_z_max > 2.448867
					{
						StrokeType = ButterflyStroke;
					}
				}
			}
			else// omega_y_var > 3.357443
			{
				if (a_Mag_max->element[0][0] <= 3.747877f)
				{
					if (a_z_min->element[0][0] <= -0.808549f)
					{
						StrokeType = Breaststroke;
					}
					else//a_z_min > -0.808549
					{
						StrokeType = Freestyle;
					}
				}
				else//a_Mag_max > 3.747877
				{
					if (omega_x_max->element[0][0] <= 5.503671f)
					{
						StrokeType = ButterflyStroke;
					}
					else//omega_x_max > 5.503671
					{
						StrokeType = Freestyle;
					}
				}
			}
		}
	}
	return StrokeType;
}

//Stroke style identification main function
void StrokeStyleIdentification(int time_i, int strokeCount, const Mat * accelerometer_val, const Mat * gyroscope_val, int * NthSlide, int * smoothStrokeTime_i, StrokeTypeNum * StrokeTypeCountNumber, bool * smooth_flag, 
	float * a_x, float * a_y, float * a_z, float * a_Mag, float * omega_x, float * omega_y, float * omega_z, float * omega_Mag, StrokeID * swimmingStyle)
{
	/*
	NOTE:
	Because the unit of the input gyroscope_val is degree instead of radian, a conversion is necessary.
	*/
	//ALGO_HF_SWIM_PRINTF("[Stroke ID]: before running: %d\n",xPortGetFreeHeapSize());
	//1. Determine the start time of stroke style identification-----------------------------------------------

	if (strokeCount == MIN_NUMBER_STROKES_FOR_STROKEID_START && *smooth_flag)//time to start swimming stroke identification
	{
		//Though the peak detection is for the last step, I negelecte the slight difference
		//the value is only altered once
		*smoothStrokeTime_i = time_i;
		*smooth_flag = false;//enter this if just once
	}

	//define some temporary variable for later use
	bool window_begin_flag = false;//indicates whether a new window begins
	bool window_end_flag = false;//indicates whether a window ends
	int temp_row_index = -1;

	//stroke style identification can only start when the *smoothStrokeTime_i is assigned to a reasonable value
	if (*smoothStrokeTime_i < STROKEID_ESTIMATED_START_TIME_I)/* *smoothStrokeTime_i belongs to (0, STROKEID_ESTIMATED_START_TIME_I)*/
	{
		if (time_i < STROKEID_ESTIMATED_START_TIME_I)
		{
			/*time_i belongs to (0, STROKEID_ESTIMATED_START_TIME_I), not collecting data yet*/
			*swimmingStyle = UnknownStyle;//because of no judgement yet			
		}
		else if (time_i <= STROKEID_ESTIMATED_START_TIME_I + STROKEID_DURATION_TIME_I)
		{
			/*time_i belongs to [STROKEID_ESTIMATED_START_TIME_I,STROKEID_ESTIMATED_START_TIME_I + STROKEID_DURATION_TIME_I], collecting data from STROKEID_ESTIMATED_START_TIME_I*/
			temp_row_index = time_i - STROKEID_ESTIMATED_START_TIME_I;
			//when time_i is in this interval, classification with sliding window technique shall be done to return stroke style value later
		}
		/*else time_i > ..., do nothing because of out of the judgement interval*/
	}
	else/* if(*smoothStrokeTime_i >= STROKEID_ESTIMATED_START_TIME_I)*/
	{
		if (*smoothStrokeTime_i >= MAX_SMOOTH_TIME_I)//*smoothStrokeTime_i is not found and keeps its initial max value
		{
			/* *smoothStrokeTime_i belongs to [MAX_SMOOTH_TIME_I, +inf)*/
			*swimmingStyle = UnknownStyle;//first enter this condition
		}
		else//*smoothStrokeTime_i is finally found
		{
			/* *smoothStrokeTime_i belongs to [STROKEID_ESTIMATED_START_TIME_I, MAX_SMOOTH_TIME_I)*/
			/*i.e. time_i >= *smoothStrokeTime_i >= STROKEID_ESTIMATED_START_TIME_I*/
			if (time_i <= *smoothStrokeTime_i + STROKEID_DURATION_TIME_I)
			{
				/*time_i belongs to [*smoothStrokeTime_i, *smoothStrokeTime_i + STROKEID_DURATION_TIME_I], collecting data from *smoothStrokeTime_i*/
				temp_row_index = time_i - *smoothStrokeTime_i;
				//when time_i is in this interval, classification with sliding window technique shall be done to return stroke style value later
			}
			/*else time_i > ..., do nothing because of out of the judgement interval*/
		}
	}




	if (temp_row_index >= 0 && temp_row_index <= STROKEID_DURATION_TIME_I)
	{
		//2. Sliding window, now collecting data based on the above judgement----------------------------------------------------------------
		//set the value of window_begin_flag and update the value of *Nthslide
		if (temp_row_index == WINDOW_SIZE_TIME_I + 1 + (*NthSlide) * SLIDE_TIME_I)//one window, 0~52, 13~65, 26~78...
		{
			++(*NthSlide);//initial value of it is -1
			window_begin_flag = true;
		}
		else
		{
			window_begin_flag = false;
		}
		//set the value of window_end_flag
		if (temp_row_index == WINDOW_SIZE_TIME_I + (*NthSlide) * SLIDE_TIME_I)
		{
			window_end_flag = true;
		}
		else
		{
			window_end_flag = false;
		}

		//store data to windowData
		/*when temp_row_index belongs to [0,WINDOW_SIZE_TIME_I]*/
		if (*NthSlide == 0)//initialize the data of the first window
		{
			//accelerator: a_x, a_y, a_z, a_mag
			a_x[temp_row_index] = accelerometer_val->element[0][0];
			a_y[temp_row_index] = accelerometer_val->element[1][0];
			a_z[temp_row_index] = accelerometer_val->element[2][0];
			a_Mag[temp_row_index] = VecMagnitude(accelerometer_val);

			//gyroscope, omega_x, omega_y, omega_z, omega_mag
			omega_x[temp_row_index] = Deg2Rad(gyroscope_val->element[0][0]);
			omega_y[temp_row_index] = Deg2Rad(gyroscope_val->element[1][0]);
			omega_z[temp_row_index] = Deg2Rad(gyroscope_val->element[2][0]);
			omega_Mag[temp_row_index] = Deg2Rad((DBL_or_FLT)VecMagnitude(gyroscope_val));
		}
		else if (*NthSlide <= MAX_N_SLIDE && *NthSlide > 0)//sliding window, when NthSlide>=1, it goes to here
		{
			if (window_begin_flag)
			{
				//keep part of the original data but move it with a distance of SLIDE_TIME_I forward/leftward/upward
				//do this once for each slide
				for (int i = SLIDE_TIME_I; i <= WINDOW_SIZE_TIME_I; i++)
				{
					//sliding window technique
					a_x[i - SLIDE_TIME_I] = a_x[i];
					a_y[i - SLIDE_TIME_I] = a_y[i];
					a_z[i - SLIDE_TIME_I] = a_z[i];
					a_Mag[i - SLIDE_TIME_I] = a_Mag[i];
					omega_x[i - SLIDE_TIME_I] = omega_x[i];
					omega_y[i - SLIDE_TIME_I] = omega_y[i];
					omega_z[i - SLIDE_TIME_I] = omega_z[i];
					omega_Mag[i - SLIDE_TIME_I] = omega_Mag[i];
				}
			}
			//store new data
			//accelerometer
			a_x[temp_row_index - (*NthSlide)*SLIDE_TIME_I] = accelerometer_val->element[0][0];
			a_y[temp_row_index - (*NthSlide)*SLIDE_TIME_I] = accelerometer_val->element[1][0];
			a_z[temp_row_index - (*NthSlide)*SLIDE_TIME_I] = accelerometer_val->element[2][0];
			a_Mag[temp_row_index - (*NthSlide)*SLIDE_TIME_I] = VecMagnitude(accelerometer_val);

			//gyroscope
			omega_x[temp_row_index - (*NthSlide)*SLIDE_TIME_I] = Deg2Rad(gyroscope_val->element[0][0]);
			omega_y[temp_row_index - (*NthSlide)*SLIDE_TIME_I] = Deg2Rad(gyroscope_val->element[1][0]);
			omega_z[temp_row_index - (*NthSlide)*SLIDE_TIME_I] = Deg2Rad(gyroscope_val->element[2][0]);
			omega_Mag[temp_row_index - (*NthSlide)*SLIDE_TIME_I] = Deg2Rad((DBL_or_FLT)VecMagnitude(gyroscope_val));
		}


		//3. Supervised classification model established beforehand-----------------------------------------------------------

		//4. Classification----------------------------------------------------------------------------------------------
		if (window_end_flag)
		{
			//4.1 define the attributes-----------------

			//accelerator
			Mat a_x_min;
			Mat a_x_max;
			Mat a_z_max;
			Mat a_z_min;
			Mat a_Mag_max;

			//gyroscope
			Mat omega_x_max;
			Mat omega_x_var;
			Mat omega_y_max;
			Mat omega_y_mean;
			Mat omega_y_var;
			Mat omega_z_min;
			Mat omega_z_var;
			Mat omega_Mag_min;
			Mat omega_Mag_max;

			//decide the size of the matrix

			//accelerator
			MatCreate(&a_x_min, 1, 1);
			MatCreate(&a_x_max, 1, 1);
			MatCreate(&a_z_max, 1, 1);
			MatCreate(&a_z_min, 1, 1);
			MatCreate(&a_Mag_max, 1, 1);

			//gyroscope
			MatCreate(&omega_x_max, 1, 1);
			MatCreate(&omega_x_var, 1, 1);
			MatCreate(&omega_y_max, 1, 1);
			MatCreate(&omega_y_mean, 1, 1);
			MatCreate(&omega_y_var, 1, 1);
			MatCreate(&omega_z_min, 1, 1);
			MatCreate(&omega_z_var, 1, 1);
			MatCreate(&omega_Mag_min, 1, 1);
			MatCreate(&omega_Mag_max, 1, 1);

			//4.2 Calculate necessary attributes------------------

			//accelerator
			Mat a_x_Mat;
			MatCreate(&a_x_Mat, WINDOW_SIZE_TIME_I + 1, 1);
			MatSetVal(&a_x_Mat, a_x);
			MatMin(&a_x_Mat, 1, &a_x_min);
			MatMax(&a_x_Mat, 1, &a_x_max);
			MatDelete(&a_x_Mat);

			Mat a_z_Mat;
			MatCreate(&a_z_Mat, WINDOW_SIZE_TIME_I + 1, 1);
			MatSetVal(&a_z_Mat, a_z);
			MatMin(&a_z_Mat, 1, &a_z_min);
			MatMax(&a_z_Mat, 1, &a_z_max);
			MatDelete(&a_z_Mat);

			Mat a_Mag_Mat;
			MatCreate(&a_Mag_Mat, WINDOW_SIZE_TIME_I + 1, 1);
			MatSetVal(&a_Mag_Mat, a_Mag);
			MatMax(&a_Mag_Mat, 1, &a_Mag_max);
			MatDelete(&a_Mag_Mat);

			//gyroscope
			Mat omega_x_Mat;
			MatCreate(&omega_x_Mat, WINDOW_SIZE_TIME_I + 1, 1);
			MatSetVal(&omega_x_Mat, omega_x);
			MatMax(&omega_x_Mat, 1, &omega_x_max);
			MatVar(&omega_x_Mat, 0, 1, &omega_x_var);
			MatDelete(&omega_x_Mat);

			Mat omega_y_Mat;
			MatCreate(&omega_y_Mat, WINDOW_SIZE_TIME_I + 1, 1);
			MatSetVal(&omega_y_Mat, omega_y);
			MatMax(&omega_y_Mat, 1, &omega_y_max);
			MatMean(&omega_y_Mat, 1, &omega_y_mean);
			MatVar(&omega_y_Mat, 0, 1, &omega_y_var);
			MatDelete(&omega_y_Mat);
			
			Mat omega_z_Mat;
			MatCreate(&omega_z_Mat, WINDOW_SIZE_TIME_I + 1, 1);
			MatSetVal(&omega_z_Mat, omega_z);
			MatMin(&omega_z_Mat, 1, &omega_z_min);
			MatVar(&omega_z_Mat, 0, 1, &omega_z_var);
			MatDelete(&omega_z_Mat);

			Mat omega_Mag_Mat;
			MatCreate(&omega_Mag_Mat, WINDOW_SIZE_TIME_I + 1, 1);
			MatSetVal(&omega_Mag_Mat, omega_Mag);
			MatMin(&omega_Mag_Mat, 1, &omega_Mag_min);
			MatMax(&omega_Mag_Mat, 1, &omega_Mag_max);
			MatDelete(&omega_Mag_Mat);

			//----DEBUG---------------------------------------
			//using namespace std;
			//for (int row_i = 0; row_i < omega_x->row; row_i++)
			//{
			//	cout << omega_x->element[row_i][0] << endl;
			//}
			//cout << "--------------------\n\n\n";

			//-----DEBUG END----------------------------------
			//4.3 Classification algorithm
			*swimmingStyle = DecisionTree_C45(&a_x_min, &a_x_max, &a_z_min, &a_z_max, &a_Mag_max,
				&omega_x_max, &omega_x_var, &omega_y_max, &omega_y_mean, &omega_y_var, &omega_z_min, &omega_z_var, &omega_Mag_min, &omega_Mag_max);
			
			//Free memory
			//accelerator
			MatDelete(&a_x_min);
			MatDelete(&a_x_max);
			MatDelete(&a_z_max);
			MatDelete(&a_z_min);
			MatDelete(&a_Mag_max);

			//gyroscope
			MatDelete(&omega_x_max);
			MatDelete(&omega_x_var);
			MatDelete(&omega_y_max);
			MatDelete(&omega_y_mean);
			MatDelete(&omega_y_var);
			MatDelete(&omega_z_min);
			MatDelete(&omega_z_var);
			MatDelete(&omega_Mag_min);
			MatDelete(&omega_Mag_max);
			
			//the initial value of these numbers should be zero
			switch (*swimmingStyle)
			{
			case Freestyle:
				++StrokeTypeCountNumber->Num_FS;
				break;
			case Backstroke:
				++StrokeTypeCountNumber->Num_BK;
				break;
			case Breaststroke:
				++StrokeTypeCountNumber->Num_BR;
				break;
			case ButterflyStroke:
				++StrokeTypeCountNumber->Num_BT;
				break;
			default:
				break;//it's unnecessary to use break here but it's used anyway
			}
			//N.B. the precedence of indirect membership operator(.) is higher than that of prefix increament operator(++variable)
			//Therefore, the parentheses is unnecessary between the structure and the "++" operator.
		}
	}

	//reach the last step within the duration time
	//precedence: FS>BK>BR>BT
	if (temp_row_index == STROKEID_DURATION_TIME_I)
	{
		if (StrokeTypeCountNumber->Num_FS >= StrokeTypeCountNumber->Num_BK && StrokeTypeCountNumber->Num_FS >= StrokeTypeCountNumber->Num_BR && StrokeTypeCountNumber->Num_FS >= StrokeTypeCountNumber->Num_BT)
		{
			*swimmingStyle = Freestyle;
		}
		else if (StrokeTypeCountNumber->Num_BK > StrokeTypeCountNumber->Num_FS && StrokeTypeCountNumber->Num_BK >= StrokeTypeCountNumber->Num_BR && StrokeTypeCountNumber->Num_BK >= StrokeTypeCountNumber->Num_BT)
		{
			*swimmingStyle = Backstroke;
		}
		else if (StrokeTypeCountNumber->Num_BR > StrokeTypeCountNumber->Num_FS && StrokeTypeCountNumber->Num_BR > StrokeTypeCountNumber->Num_BK && StrokeTypeCountNumber->Num_BR >= StrokeTypeCountNumber->Num_BT)
		{
			*swimmingStyle = Breaststroke;
		}
		else if (StrokeTypeCountNumber->Num_BT > StrokeTypeCountNumber->Num_FS && StrokeTypeCountNumber->Num_BT > StrokeTypeCountNumber->Num_BK && StrokeTypeCountNumber->Num_BT > StrokeTypeCountNumber->Num_BR)
		{
			*swimmingStyle = ButterflyStroke;
		}
		else//in case of conditions not considered
		{
			*swimmingStyle = UnknownStyle;
		}
	}
}



//4. Algorithm collection-------------------------------------
void SwimmingAlgorithmsAll(short* data_arr, int time_i)//because time_i is defined as an "extern" variable, it can be ommited in the argument list
{
	//1. Conversion from raw IMU data to acceleration and angular velocity---------------------------
	RawDataPreprocessor(data_arr, &acc, &omega);
	
	//2. Butterworth filter---------------------------------------------------------------------------
	SwimDataFilter1(&acc, &omega, &A, &B, &C, D,
		&accX_x_k1, accX_u_k1, &accY_x_k1, accY_u_k1, &accZ_x_k1, accZ_u_k1,
		&omegaX_x_k1, omegaX_u_k1, &omegaY_x_k1, omegaY_u_k1, &omegaZ_x_k1, omegaZ_u_k1,
		&acc_f, &omega_f);
	

	//3. Main algorithm--------------------------------------------------------------------------------
	//3.1 Number of strokes
	//3.1 Number of strokes
	StrokeCount(NLength, time_i, omega_f.element[0][0], &PossiblePeak1, &inputDataSequence1, &NumStrokeCount1,
		 &PossiblePeak2, &inputDataSequence2, &NumStrokeCount2, &NumStrokeCount, MIN_PEAK_NUM);	//omega_x is used to calculate the period of the signal, i.e. #strokes
	//3.2 Number of lengths
	LengthCount(time_i, omega_f.element[0][0], acc_f.element[1][0], acc_f.element[2][0], &BR_flag, &BR_az_start_counting_time_i, &hasPassedInitialEliminationPeriod, &has_Entered_BR_BT_ID_Ub,
		&BR_ID_omega_max, &BR_ID_omega_min, &BR_ID_acc_y_max, &BR_ID_acc_y_min,
		&omega_x_Sequence1, &omega_x_Possible_Peak1, &N_omega_x_Peak1, &omega_x_Peak1_Sum, &omega_x_Peak1_Mean, &omega_x_Possible_Peak2, &omega_x_Sequence2, &N_omega_x_Peak2, &omega_x_Peak2_Sum, &omega_x_Peak2_Mean,
		&omega_x_index1_Sum, &omega_x_index1_Mean, &omega_x_index2_Sum, &omega_x_index2_Mean,
		&a_z_Sequence1, &a_z_Possible_Peak1, &N_a_z_Peak1, &a_z_Peak1_Sum, &a_z_Peak1_Mean, &a_z_Possible_Peak2, &a_z_Sequence2, &N_a_z_Peak2, &a_z_Peak2_Sum, &a_z_Peak2_Mean, &is_BR_ID_omega_az_MeanCalculationDone,
		&BT_ID_omega_x_PeakCountInfo1, &BT_ID_omega_x_Sequence1, &BT_ID_omega_x_PeakCountInfo2, &BT_ID_omega_x_Sequence2, &BT_flag,
		&omega_x_counter_tk, &omega_x_counter_tk_1, &omega_x_NStrokes_tk, &omega_x_NStrokes_tk_1, 
		&a_z_counter_tk, &a_z_counter_tk_1, &a_z_NStrokes_tk, &a_z_NStrokes_tk_1,
		&integrator_output_y, &isDurationTimePassed, NumStrokeCount, &NLength,
		INITIAL_ELIMINATION_TIME_I, MIN_LENGTHS_TIME_I_INTERVAL, INITIAL_ELIMINATION_NSTROKES, MIN_LENGTHS_NSTROKES_INTERVAL, BR_ID_FOR_LENGTH_COUNT_STARTING_NUM_STROKES,
		&OMEGA_X_DYNAMIC_LOWER_BOUND, &OMEGA_X_DYNAMIC_UPPER_BOUND, &DYNAMIC_SIGNAL_DURATION_TIME);
	
	//3.3 Stroke type identification
	StrokeStyleIdentification(time_i, NumStrokeCount, &acc_f, &omega_f, &NSlide, &smoothStrokeTime_i, &StrokeTypeCountNumber, &smooth_start_flag,
		a_x, a_y, a_z, a_Mag, omega_x, omega_y, omega_z, omega_Mag, &swimmingStyle);
	//ALGO_HF_SWIM_PRINTF("[After Main]: %d\n",xPortGetFreeHeapSize());
}

//5. Post-processing-------------------------------------------
void SwimmingAlgorithmsEnd(void)
{
	//vPortFree memory
	MatDelete(&acc);
	MatDelete(&omega);
	MatDelete(&accX_x_k1);
	MatDelete(&accY_x_k1);
	MatDelete(&accZ_x_k1);
	MatDelete(&omegaX_x_k1);
	MatDelete(&omegaY_x_k1);
	MatDelete(&omegaZ_x_k1);
	MatDelete(&acc_f);
	MatDelete(&omega_f);
	MatDelete(&A);
	MatDelete(&B);
	MatDelete(&C);
//	vPortFree(peak_index);
//	vPortFree(peak_val);
//	ALGO_HF_SWIM_PRINTF("------------ After Post-Processing: NStrokes = %d\n",SwimResultsOutputPort1_StrokeCount());
	//设置原始数据存储类型，结束运动后默认为1
  #if defined STORE_ORG_DATA_TEST_VERSION
    Set_StoreData_Flag(1);
  #endif
}
//6. Output Functions-------------------------------------------
//Return the number of strokes
int SwimResultsOutputPort1_StrokeCount(void)
{
	return NumStrokeCount;
}
//Return the number of lengths
int SwimResultsOutputPort2_LengthCount(void)
{
	return NLength;
}
//Return the stroke style
int SwimResultsOutputPort3_SwimmingStyle(void)
{
	return swimmingStyle;//enum is converted to type int automatically
}

//Update the number of lengths
int SwimResultsPostProcessing(void)
{
	//post-processing
	int t_f=time_i;
		LengthCountPostProcessing(t_f, NumStrokeCount, isDurationTimePassed, BR_flag, BT_flag,
		&omega_x_counter_tk, &omega_x_counter_tk_1, &omega_x_NStrokes_tk, &omega_x_NStrokes_tk_1,
		&a_z_counter_tk, &a_z_counter_tk_1, &a_z_NStrokes_tk, &a_z_NStrokes_tk_1, 
		MIN_LENGTHS_TIME_I_INTERVAL, MIN_LENGTHS_NSTROKES_INTERVAL, &NLength);
	return NLength;
}
//========================================END OF FUNCTIONS=======================================================================

}//end of extern "C"

