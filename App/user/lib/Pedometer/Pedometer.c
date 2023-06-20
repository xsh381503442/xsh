#include "Pedometer.h"
#include "PedoSupport.h"

#include "SEGGER_RTT.h"
#include "task_step.h"
#include "rtos.h"

#define WALKING_LOWER_LIMIT                     (60>>V_DivideFactor_U8R)     /**< Stable Axis Lower Threshold for Walk step */
#define WALKING_UPPER_LIMIT                     (400>>V_DivideFactor_U8R)     /**< Stable Axis Upper Threshold for Walk step */
#define LAZY_WALKING_LOWER_LIMIT_Robust         (15>>V_DivideFactor_U8R)      /**< Stable Axis Lower Threshold for Lazy Walk step(Robust) */
#define LAZY_WALKING_LOWER_LIMIT_NoRobust       (10>>V_DivideFactor_U8R)      /**< Stable Axis Lower Threshold for Lazy Walk step(Non-Robust) */
#define LAZY_WALKING_UPPER_LIMIT                (160>>V_DivideFactor_U8R)     /**< Stable Axis Upper Threshold for Lazy Walk step */
#define JOG_LOWER_LIMIT                         (100>>V_DivideFactor_U8R)     /**< Stable Axis Lower Threshold for Jog step */
#define JOG_UPPER_LIMIT                         (17000>>V_DivideFactor_U8R)   /**< Stable Axis Upper Threshold for Jog step */
#define SWIM_LOWER_LIMIT                        (100>>V_DivideFactor_U8R)     /**< Stable Axis Lower Threshold for Swim step */
#define SWIM_UPPER_LIMIT                        (500>>V_DivideFactor_U8R)   /**< Stable Axis Upper Threshold for Swim step */
#define C_SENSOR_NOISE_LEVEL_U8X                (10>>V_DivideFactor_U8R) /**< Variation of Acceleration Value When kept idle? */
#define	INFLECTION_POINT_DIFF					(5>>V_DivideFactor_U8R)  //检测拐点时的差值定义
#define	POSITIVE			1
#define	NEGATIVE			2

/* PATTERN RECOGNITION ( TREND CYCLE LIMITS FOR DIFF ACTIVITIES ) */

#define WALKING_CYCLE_LOWER_LIMIT               5       /**< Trend cycle count lower limit for walking */
#define WALKING_CYCLE_UPPER_LIMIT               21      /**< Trend cycle count upper limit for walking */
#define LAZY_WALKING_CYCLE_LOWER_LIMIT          10      /**< Trend cycle count lower limit for lazy walking */
#define LAZY_WALKING_CYCLE_UPPER_LIMIT          32      /**< Trend cycle count upper limit for lazy walking */
#define JOG_CYCLE_LOWER_LIMIT                   4       /**< Trend cycle count lower limit for jogging */
#define JOG_CYCLE_UPPER_LIMIT                   15      /**< Trend cycle count upper limit for jogging */
#define SWIM_CYCLE_LOWER_LIMIT                  10      /**< Trend cycle count lower limit for swiming */
#define SWIM_CYCLE_UPPER_LIMIT                  50      /**< Trend cycle count upper limit for swiming */
#define MAX_COUNT_DIFFERENCE                    3       /**< Difference between 2 adjacent steps*/

/* PATTERN RECOGNITION ( STEP-TO-STEP INTERVAL LIMITS FOR DIFF ACTIVITIES ) */

#define C_LazyWalkStepToStepLowerLimit_U8X      12      /**< Step-To-Step count lower limit for lazy walk */
#define C_LazyWalkStepToStepUpperLimit_U8X      32      /**< Step-To-Step count upper limit for lazy walk */
#define C_WalkStepToStepLowerLimit_U8X          7       /**< Step-To-Step count lower limit for walk */
#define C_WalkStepToStepUpperLimit_U8X          30      /**< Step-To-Step count upper limit for walk */
#define C_JogStepToStepLowerLimit_U8X           5       /**< Step-To-Step count lower limit for Jog */
#define C_JogStepToStepUpperLimit_U8X           17      /**< Step-To-Step count upper limit for Jog */
#define C_SwimStepToStepLowerLimit_U8X          12       /**< Step-To-Step count lower limit for Swim */
#define C_SwimStepToStepUpperLimit_U8X          45      /**< Step-To-Step count upper limit for Swim */

/* MODE DEFINITIONS */

#define M_Walk_U8X                              (char)0x01    /**< Mask Bit for Walk*/
#define M_SlowWalk_U8X                          (char)0x02    /**< Mask Bit for Slow Walk*/
#define M_Jog_U8X                               (char)0x04    /**< Mask Bit for Jog*/
#define M_Swim_U8X                              (char)0x08    /**< Mask Bit for Swim*/

#define M_ModeDetection_U8X                     (char)0x01    /**< Mask Bit for Detection Mode*/
#define M_ModeCounting_U8X                      (char)0x02    /**< Mask Bit for Counting Mode*/
#define M_AlgoReset_U8X                         (char)0x04    /**< Mask Bit for Algo reset*/

#define M_Qualified_U8X                         (char)0x08    /**< Mask Bit for Qualified Step*/
#define M_UnQualified_U8X                       (char)0x10    /**< Mask Bit for UnQualified Step*/
#define M_PositiveTrend_U8X                     (char)0x20    /**< Mask Bit for Positive Trend Change*/
#define M_NegativeTrend_U8X                     (char)0x40    /**< Mask Bit for Negative Trend Change*/

#define M_DisableRobustness_U8X                 (char)0x80    /**< Mask Bit for Disable robustness feature*/
	

/* MACROS DECIDING THE STEP NATURE */

/**\ brief Macro Deciding Lazy walk step */

#define IS_LAZY_WALK_STEP(resultDiff_i16) \
        ((resultDiff_i16 >= V_LazyWalkLowerLimit_U8R &&  \
          resultDiff_i16 <= LAZY_WALKING_UPPER_LIMIT) && \
        (V_SampleCountForCycle_U8R >= LAZY_WALKING_CYCLE_LOWER_LIMIT && \
         V_SampleCountForCycle_U8R <= LAZY_WALKING_CYCLE_UPPER_LIMIT))

/**\ brief Macro deciding Walk step */

#define IS_WALK_STEP(resultDiff_i16) \
        ((resultDiff_i16 >= WALKING_LOWER_LIMIT &&  \
          resultDiff_i16 <= WALKING_UPPER_LIMIT) && \
        (V_SampleCountForCycle_U8R >= WALKING_CYCLE_LOWER_LIMIT && \
         V_SampleCountForCycle_U8R <= WALKING_CYCLE_UPPER_LIMIT))

/**\ brief Macro deciding Jog step */

#define IS_JOG_STEP(resultDiff_i16) \
        ((resultDiff_i16 >= JOG_LOWER_LIMIT &&  \
          resultDiff_i16 <= JOG_UPPER_LIMIT) && \
        (V_SampleCountForCycle_U8R >= JOG_CYCLE_LOWER_LIMIT && \
         V_SampleCountForCycle_U8R <= JOG_CYCLE_UPPER_LIMIT))
         
/**\ brief Macro deciding Swim step */

#define IS_SWIM_STEP(resultDiff_i16) \
        ((resultDiff_i16 >= SWIM_LOWER_LIMIT &&  \
          resultDiff_i16 <= SWIM_UPPER_LIMIT) && \
        (V_SampleCountForCycle_U8R >= SWIM_CYCLE_LOWER_LIMIT && \
         V_SampleCountForCycle_U8R <= SWIM_CYCLE_UPPER_LIMIT))


/* PEDOMETER OTHER CONSTANTS */

#define C_FilterTaps_U8X                   (char)24     /**< Filter taps */
#define C_Q15ConversionFactor_U8X          (char)15     /**< Division Factor for Q15 */
#define C_Clear_U8X                        (char)0      /**< For clearing to 0 */

#define C_CountZero_U8X                    (char)0      /**< For Counter purpose 0 */
#define C_CountOne_U8X                     (char)1      /**< For Counter purpose 1 */

#define C_DetectionModeTimeOut_U8X         (char)50     /**< Detection Mode time out upper limit (2 seconds) */
#define C_CountingModeTimeOut_U8X          (char)75    /**< Counting Mode time out upper limit (3 seconds) */
#define C_ErrorCountLimit_U8X              (char)2/**< Limit for Error count */
#define C_CorrectionCountLimit_U8X         (char)3      /**< Limit for Correction count */
#define C_InterStepCountLimit_U8X          (char)9      /**< Limit for Inter Step count (With Robustness) */
#define C_InterStepCountLimitNoRobustness  (char)3      /**< Limit for Inter Step count (Without Robustness) */

#define C_RunLowerRange_U8X                (char)36>>V_DivideFactor_U8R /**< Run Activity Step Peak-To-Peak to Step-Interval Ratio */
#define C_WalkLowerRange_U8X               (char)4      /**< Walk Activity Step Peak-To-Peak to Step-Interval Ratio */

#define C_ActivityFilterSize_U8X           (char)8      /**< Activity Filter Size */


/* ACTIVITY TYPE */
#define STATIONARY                         (char)0      /**< STATIONARY */
#define WALK                               (char)1      /**< WALK */
#define RUN                                (char)3      /**< RUN */

/* EasyCASE - */
#define C_Xaxis_U8X                        (char)0      /**< X_Axis */
#define C_Yaxis_U8X                        (char)1      /**< Y_Axis */
#define C_Zaxis_U8X                        (char)2      /**< Z_Axis */
#define C_CompositeAxis_U8X                (char)3      /**< Composite_Axis */

unsigned char    V_InterStepCount_U8R=0;       /**< Holds the steps counted while not in counting */
unsigned char    V_SampleCountForCycle_U8R=0;  /**< Holds the sample counts between trend changes */
unsigned char    V_Activity_U8R;               /**< Holds the pedometer activity */
unsigned char    V_DivideFactor_U8R;           /**< Holds the division factor for the threshold */
unsigned char    V_StatusFlags_U8R;            /**< Holds the Status Flags used */
static unsigned short V_StepCount_U16R;        /**< Holds the Total StepCount */

unsigned char V_SwimStatus =0;

#define M_Tocuh_U8X                           (char)0x01    /**< Mask Bit for Tocuh*/
#define M_Freestyle_U8X                       (char)0x02    /**< Mask Bit for Freestyle*/
#define M_Breaststroke_U8X                    (char)0x04    /**< Mask Bit for Breaststroke*/
#define M_Backstroke_U8X                      (char)0x08    /**< Mask Bit for Backstroke*/
#define M_Butterfly_U8X                       (char)0x10    /**< Mask Bit for Butterfly*/


static short  A_Raw_AccelC_S16R[C_FilterTaps_U8X];   /**< Holds the 24 input values of Composite-Axis to the filter */
static short  A_Raw_AccelX_S16R[C_FilterTaps_U8X];   /**< Holds the 24 input values of X-Axis to the filter */
static short  A_Raw_AccelY_S16R[C_FilterTaps_U8X];   /**< Holds the 24 input values of Y-Axis to the filter */
static short  A_Raw_AccelZ_S16R[C_FilterTaps_U8X];   /**< Holds the 24 input values of Z-Axis to the filter */
unsigned char V_LazyWalkLowerLimit_U8R =0;           /**< Holds the lazy walk lower limit threshold */
/* EasyCASE - */
U8 V_ActivityDetected_U8R=0;             /**< Holds the current activity type */
static U8 V_InterStepCountLimit_U8R = 0; /**< Holds the interstepcount limit (User selectable for robust and non-robust cases) */

static const short gc_CompositeFilterCoeff[C_FilterTaps_U8X] =
{
        58, 134, 173, 45, -324, -802, -999, -422, 1192, 3614, 6078, 7637,
        7637, 6078, 3614, 1192, -422, -999, -802, -324, 45, 173, 134, 58
};
/* EasyCASE > */
/* EasyCASE - */
/* EasyCASE < */

static const short gc_HighSpeedFilterCoeff[C_FilterTaps_U8X] =
{
     290, 288, -353, -597, -26, 1110, 857, -1154, -2584, 10, 6498, 12235,
     12235, 6498, 10, -2584, -1154, 857, 1110, -26, -597, -353, 288, 290
};
static U8 V_AxisSelectFlag_U8R = C_AXIS_IS_COMPOSITE_U8X;  /**< Axis Selection flag (Single/Composite)? */
/* EasyCASE - */
ts_SingleAxis S_Param_Xaxis;  /**< Structure variable to hold X-axis acceleration features  */
ts_SingleAxis S_Param_Yaxis;  /**< Structure variable to hold Y-axis acceleration features  */
ts_SingleAxis S_Param_Zaxis;  /**< Structure variable to hold Z-axis acceleration features  */
ts_SingleAxis S_Param_Caxis;  /**< Structure variable to hold C-axis acceleration features  */
/* EasyCASE - */


U16 V_SingleAxisStableCadence_U16R;    /**< Stride based cadence for stable axis */
U16 V_CompositeAxis_StrideRatio_U16R;  /**< Stride Peak-to-Peak value to Stride Interval ratio for composite axis */
U16 V_CompositeAxis_CadenceX100_U16R;  /**< Cadence for Composite axis */
int16_t adx_buf[3] = {0};

/* EasyCASE - */


/* EasyCASE ) */
/* EasyCASE ( 223
   Constant- and Table-Definitions in ROM/EEPROM */
/* EasyCASE ) */
/* EasyCASE ( 252
   Functions Prototype Declaration */
static void  PEDO_TrendFinder( short, short* ); 
static short PEDO_GetAbsoluteShort( short );
static short  Max_Abs( short Val1,short Val2,short Val3);
static void  PEDO_Get_SingleAxisAccelFeatures( S16, ts_SingleAxis* );
static void  PEDO_Reset_SingleAxisAccelFeatures( ts_SingleAxis* );
static void  PEDO_Find_StableSingleAxis_StableCadenceX100( ts_SingleAxis*, ts_SingleAxis*, ts_SingleAxis*, ts_SingleAxis*, U8, U8, U8, U8*, U16*);
static void  PEDO_FindDominantAxisOrder( ts_SingleAxis*, ts_SingleAxis*, ts_SingleAxis*, U8, U8*,  U8*, U8* );

unsigned long GetStepCount(void)
{
		return (V_StepCount_U16R );
}
   
void  SetStepCount( U16  Steps )
 {
	 V_StepCount_U16R =  Steps;
 }

bool GetSwimTouch(void)
{
    return (V_SwimStatus & M_Tocuh_U8X);
}

void  ClearSwimStatus( char  status )
 {
	 V_SwimStatus &= ~status;
 }

void SetSwimStatus( char  status )
{
    V_SwimStatus |= status;
}
unsigned char GetActivity(void)
{
   /*Activities are identified based on the acceleration pattern features */
   return (V_ActivityDetected_U8R);
}

void InitAlgo(unsigned char v_GRange_u8r)
{
   /* Initialze the variables that are being used in the Pedometer Algorithm. */
   /* EasyCASE - */
   V_Activity_U8R = C_Clear_U8X;  /* Clear User Activity Type */
   
   /* Reset the step count */
   V_StepCount_U16R = C_Clear_U8X;
   
   /* Clear the Status Flag and Set the MODE as Detection to startc algo afresh */
   V_StatusFlags_U8R = C_Clear_U8X;
   V_StatusFlags_U8R |= (M_ModeDetection_U8X|M_AlgoReset_U8X);
   V_DivideFactor_U8R=v_GRange_u8r;
   
   /*Default lower limit for lazy walk is limit with robustness enabled */
   V_LazyWalkLowerLimit_U8R=LAZY_WALKING_LOWER_LIMIT_Robust;
   
   /* Enable Robustness feature */
   PEDO_EnableRobustness();     

   //增加初始化单轴参数结构体，否则V_InstTrendType_S8X为0，无法使用
      /* Reset single axis parameters for the X, Y, Z and composite axis. */
   PEDO_Reset_SingleAxisAccelFeatures( &S_Param_Xaxis );
   PEDO_Reset_SingleAxisAccelFeatures( &S_Param_Yaxis );
   PEDO_Reset_SingleAxisAccelFeatures( &S_Param_Zaxis );
   PEDO_Reset_SingleAxisAccelFeatures( &S_Param_Caxis );
}

void PEDO_EnableRobustness(void)
{
   /* Enable Robustness feature of the Pedometer */
   V_StatusFlags_U8R&=~(M_DisableRobustness_U8X);
   /*Lower threshold for robust mode*/
   V_LazyWalkLowerLimit_U8R=LAZY_WALKING_LOWER_LIMIT_Robust;
}

#if 0
void PEDO_DisableRobustness(void)
{
   /* Disable robustness feature for Pedometer Algorithm */
   V_StatusFlags_U8R|=(M_DisableRobustness_U8X);
    /*Set Lasy walk lower thresold for non_robust mode*/
   V_LazyWalkLowerLimit_U8R=LAZY_WALKING_LOWER_LIMIT_NoRobust;
}
#endif 

U8 GetBandwidthSwitchInfo(void)
{
   return V_AxisSelectFlag_U8R;
}

short ProcessAccelarationData(short v_RawAccelX_s16r, short v_RawAccelY_s16r, short v_RawAccelZ_s16r)
{
   /* EasyCASE ( 101532
      Variable Declerations */
   signed char  v_Index_u8r = 0; /* Local Index variable */
   static unsigned char  v_CorrectionCount_u8r=0; /* Correction Count */
   static unsigned char  v_ErrorCount_u8r=0; /* Error Count between two steps */
   /* EasyCASE - */
   static unsigned char   v_CountStepToStep_u8r=0; /* Time count b/w last and current step */
   static unsigned char   v_CountStepToStepPrev1_u8r=0; /* Time count b/w second last and last step */
   static unsigned char   v_CountStepToStepPrev2_u8r=0; /* Time count b/w third last and second last step */
   static unsigned char   v_ModeTimeoutCount_u8r=0; /* Detection/Counting Mode timeout counter */
   /* EasyCASE - */
   short  v_RawAccelC_s16r=0; /* Raw Composite Acceleration value */
   short  v_FiltAccelTrendAxis_s16r=0;  /* Filtered acceleration value for stable single axis */
   short  v_FiltAccel4Axis_s16r[4]; /* Filtered acceleration value for all 4 axis (X, Y, Z and Composite) */
   
   /* Initialise the filter result value to 0 */
   long  v_FilterResultAxisX_u32r=C_Clear_U8X; /* Filtered X-Axis Accel Value */
   long  v_FilterResultAxisY_u32r=C_Clear_U8X; /* Filtered Y-Axis Accel Value */
   long  v_FilterResultAxisZ_u32r=C_Clear_U8X; /* Filtered Z-Axis Accel Value */
   long  v_FilterResultAxisC_u32r=C_Clear_U8X; /* Filtered Composite-Axis Accel Value */
   //static char v_PreStatusFlags_u8r=0; /* Previous status */
   /* EasyCASE - */
   static short v_NegTrendAccelVal_s16r=0x7FFF; /* Acceleration value at Negetive Trend */
   /* Previous acceleration value difference from negetive to positive trends */
   static short v_PrevPosNegTrendAccelValDiff_s16r=0;
   /* EasyCASE - */
   /* EasyCASE < */
   unsigned short v_AvgActivityDetectionRatio_u16r=0; /* Average step peak-to-peak value versus step-interval ratio */
   static unsigned short a_AvgActivityDetectionRatioBuf_u16r[C_ActivityFilterSize_U8X]={0}; /* step peak-to-peak value versus step-interval ratio buffer */
   static unsigned char v_TrendDominantAxis_u8r=C_CompositeAxis_U8X; /* Trend dominant axis out of 4 axis (X, Y, Z and Composite ) */
   /* EasyCASE > */
   /* EasyCASE - */
   /* Variables for the single axis accel features */
   static unsigned char v_XaxisStrideDetectedFlag_u8r=0; /* X-axis stride detection flag */
   static unsigned char v_YaxisStrideDetectedFlag_u8r=0; /* Y-axis stride detection flag */
   static unsigned char v_ZaxisStrideDetectedFlag_u8r=0; /* Z-axis stride detection flag */
   static unsigned char v_CaxisStrideDetectedFlag_u8r=0; /* Composite-axis stride detection flag */
   
   unsigned char v_IstDomAxis_u8r=0; /* First dominant axis */
   unsigned char v_IIndDomAxis_u8r=0; /* Second dominant axis */
   unsigned char v_IIIrdDomAxis_u8r=0; /* Third dominant axis */
   unsigned char v_Activity_u8r=0; /* User activity */
   static   char v_PreviousActivity_u8r=0; /* Previous Activity */
   unsigned char v_SingleStableAxis_u8r = C_CompositeAxis_U8X; /* Stable axis out of 4 axis ( X, Y, Z and Composite ) */
   unsigned long v_TempU32_u32r=0; /* Temp unsigned int variable */

   /* EasyCASE ) */
   /* EasyCASE ( 101534
      Reset Previous data */
	if (V_StatusFlags_U8R & M_AlgoReset_U8X)
	{
      /* Set the reset the previous data as 0 */
      V_StatusFlags_U8R &= ~M_AlgoReset_U8X;
      /*Holds the previous activity type*/
      v_PreviousActivity_u8r=C_Clear_U8X;
      
      /* Holds the steps counted while not in counting */
      V_InterStepCount_U8R=C_Clear_U8X;
      /* 0 if there is no correction. correction value if there needs to be a correction */
      v_CorrectionCount_u8r=C_Clear_U8X;
      /*Holds error count between 2 steps*/
      v_ErrorCount_u8r=C_Clear_U8X;
      /*Holds the count between two consecutive counts*/
      v_ModeTimeoutCount_u8r=C_Clear_U8X;
      /* Holds the sample counts between trend changes */
      V_SampleCountForCycle_U8R=C_Clear_U8X;
      /* Holds the time count between last & this step */
      v_CountStepToStep_u8r=C_Clear_U8X;
      /* Holds the time count between second last & last step */
      v_CountStepToStepPrev1_u8r=C_Clear_U8X;
      /* Holds the time count between third last & second last step */
      v_CountStepToStepPrev2_u8r=C_Clear_U8X;
      /*Initialise interstep count*/
      V_InterStepCountLimit_U8R = C_InterStepCountLimit_U8X;      
      /* Holds the composite high for the step cycle */
      v_NegTrendAccelVal_s16r=0x7FFF;
      /*Holds prev Result Diff*/
      v_PrevPosNegTrendAccelValDiff_s16r=C_Clear_U8X;
      /*The variable holding filter result are cleared*/
      v_FilterResultAxisC_u32r=C_Clear_U8X;
      

      /* EasyCASE - */
      /* Clear all axis accel buffers */
      for (v_Index_u8r=0;v_Index_u8r<C_FilterTaps_U8X;v_Index_u8r++)
      {
         A_Raw_AccelX_S16R[v_Index_u8r] = C_Clear_U8X;
         A_Raw_AccelY_S16R[v_Index_u8r] = C_Clear_U8X;
         A_Raw_AccelZ_S16R[v_Index_u8r] = C_Clear_U8X;
         A_Raw_AccelC_S16R[v_Index_u8r] = C_Clear_U8X;
      }
      /* Clear activity detection ratio buffer */
      for (v_Index_u8r=(C_ActivityFilterSize_U8X-1);v_Index_u8r>=C_CountZero_U8X;v_Index_u8r--)
      {
         a_AvgActivityDetectionRatioBuf_u16r[v_Index_u8r]=0;
      }
      V_ActivityDetected_U8R = (U8) STATIONARY; /* Default activity is Rest */
      v_TrendDominantAxis_u8r = C_CompositeAxis_U8X; /* Default trend dominating axis is Composite */
      V_AxisSelectFlag_U8R = C_AXIS_IS_COMPOSITE_U8X; /* Default axis selection is "AxisIsComposite" */
   }
   /* EasyCASE ) */
   /* EasyCASE ( 101536
      Count Updation */
   V_SampleCountForCycle_U8R++; /* Increment the count for cycle */
   v_CountStepToStep_u8r++;     /* Increment the step to step count */
   v_ModeTimeoutCount_u8r++;    /* Increment Count Mode Elapse Timer*/
   /* EasyCASE ) */
   /* EasyCASE ( 101537
      Find Trend Properties */
   /* EasyCASE ( 101538
      计算三轴向量近似值 ，命名为C轴*/
   v_RawAccelC_s16r = (((int)(PEDO_GetAbsoluteShort(v_RawAccelX_s16r)+ PEDO_GetAbsoluteShort(v_RawAccelY_s16r) + PEDO_GetAbsoluteShort(v_RawAccelZ_s16r)))*45
						+ ((int)Max_Abs(v_RawAccelX_s16r,v_RawAccelY_s16r,v_RawAccelZ_s16r))*77)>>8;
   /* EasyCASE ) */
   /* EasyCASE ( 101539
      Calculate the FIR-LPF Filtered Acceleration Values */
   /* shift the acceleration values into respective FIFOs for Composite axis  */
   for (v_Index_u8r=C_CountZero_U8X; v_Index_u8r<(C_FilterTaps_U8X-1); v_Index_u8r++)
   {
      A_Raw_AccelX_S16R[C_FilterTaps_U8X-v_Index_u8r-1] = A_Raw_AccelX_S16R[C_FilterTaps_U8X-v_Index_u8r-2];
      A_Raw_AccelY_S16R[C_FilterTaps_U8X-v_Index_u8r-1] = A_Raw_AccelY_S16R[C_FilterTaps_U8X-v_Index_u8r-2];
      A_Raw_AccelZ_S16R[C_FilterTaps_U8X-v_Index_u8r-1] = A_Raw_AccelZ_S16R[C_FilterTaps_U8X-v_Index_u8r-2];
      A_Raw_AccelC_S16R[C_FilterTaps_U8X-v_Index_u8r-1] = A_Raw_AccelC_S16R[C_FilterTaps_U8X-v_Index_u8r-2];
   }
   A_Raw_AccelX_S16R[C_CountZero_U8X] = v_RawAccelX_s16r;
   A_Raw_AccelY_S16R[C_CountZero_U8X] = v_RawAccelY_s16r;
   A_Raw_AccelZ_S16R[C_CountZero_U8X] = v_RawAccelZ_s16r;
   A_Raw_AccelC_S16R[C_CountZero_U8X] = v_RawAccelC_s16r;
   /* EasyCASE - */
   /* Clear all the variables holding filtered acceleration value for X, Y, Z and Composite axis */
   /* EasyCASE - */
   v_FilterResultAxisX_u32r=C_Clear_U8X;
   v_FilterResultAxisY_u32r=C_Clear_U8X;
   v_FilterResultAxisZ_u32r=C_Clear_U8X;
   v_FilterResultAxisC_u32r=C_Clear_U8X;
   /* EasyCASE - */
   /* Calculate the FIR LPF filtered values for X, Y, Z and Composite axis */
   for (v_Index_u8r=C_CountZero_U8X;v_Index_u8r<C_FilterTaps_U8X;v_Index_u8r++)
   {
      v_FilterResultAxisX_u32r = v_FilterResultAxisX_u32r + 
      (long)((long)A_Raw_AccelX_S16R[(C_FilterTaps_U8X-C_CountOne_U8X)-v_Index_u8r]
      *(long)gc_HighSpeedFilterCoeff[v_Index_u8r]);
      /* EasyCASE - */
      v_FilterResultAxisY_u32r = v_FilterResultAxisY_u32r + 
      (long)((long)A_Raw_AccelY_S16R[(C_FilterTaps_U8X-C_CountOne_U8X)-v_Index_u8r]
      *(long)gc_HighSpeedFilterCoeff[v_Index_u8r]);
      /* EasyCASE - */
      v_FilterResultAxisZ_u32r = v_FilterResultAxisZ_u32r + 
      (long)((long)A_Raw_AccelZ_S16R[(C_FilterTaps_U8X-C_CountOne_U8X)-v_Index_u8r]
      *(long)gc_HighSpeedFilterCoeff[v_Index_u8r]);
      /* EasyCASE - */
      v_FilterResultAxisC_u32r = v_FilterResultAxisC_u32r +
       (long)((long)A_Raw_AccelC_S16R[(C_FilterTaps_U8X-C_CountOne_U8X)-v_Index_u8r]
      *(long)gc_CompositeFilterCoeff[v_Index_u8r]);
   }
   /* Divide by 32768 to compensate Q15 format multiplication.*/
   /* EasyCASE - */
   v_FiltAccel4Axis_s16r[C_Xaxis_U8X] = (short)(v_FilterResultAxisX_u32r>>C_Q15ConversionFactor_U8X);
   v_FiltAccel4Axis_s16r[C_Yaxis_U8X] = (short)(v_FilterResultAxisY_u32r>>C_Q15ConversionFactor_U8X);
   v_FiltAccel4Axis_s16r[C_Zaxis_U8X] = (short)(v_FilterResultAxisZ_u32r>>C_Q15ConversionFactor_U8X);
   v_FiltAccel4Axis_s16r[C_CompositeAxis_U8X] = (short)(v_FilterResultAxisC_u32r>>C_Q15ConversionFactor_U8X);
   adx_buf[0] = v_FiltAccel4Axis_s16r[C_Xaxis_U8X];
   adx_buf[1] = v_FiltAccel4Axis_s16r[C_Yaxis_U8X];
   adx_buf[2] = v_FiltAccel4Axis_s16r[C_Zaxis_U8X];
   
   /* EasyCASE ) */
   /* EasyCASE ( 101540
      Find Stable Single Axis ( X, Y, Z or Composite Axis ) */
   /* Find all the axis (X, Y, Z and Composite Axis) properties */ 
   PEDO_Get_SingleAxisAccelFeatures( v_FiltAccel4Axis_s16r[C_Xaxis_U8X], &S_Param_Xaxis);
   PEDO_Get_SingleAxisAccelFeatures( v_FiltAccel4Axis_s16r[C_Yaxis_U8X], &S_Param_Yaxis);
   PEDO_Get_SingleAxisAccelFeatures( v_FiltAccel4Axis_s16r[C_Zaxis_U8X], &S_Param_Zaxis);
   PEDO_Get_SingleAxisAccelFeatures( v_FiltAccel4Axis_s16r[C_CompositeAxis_U8X], &S_Param_Caxis);
   /* EasyCASE - */
   /* Get the PedoActivity --> STATIONARY, WALK or RUN */ 
   v_Activity_u8r = GetActivity();
   /* EasyCASE - */
   /* EasyCASE < */
   /* Check for X, Y, Z and Composite Axis if any stride pattern got detected?? */ 
    if (S_Param_Xaxis.V_ValidStridePatternOccuredFlag_U8X==1)
            v_XaxisStrideDetectedFlag_u8r = 1; /* For X-axis Stride Pattern got detected */
    
    if (S_Param_Yaxis.V_ValidStridePatternOccuredFlag_U8X==1)
            v_YaxisStrideDetectedFlag_u8r = 1; /* For Y-axis Stride Pattern got detected */
    
    if (S_Param_Zaxis.V_ValidStridePatternOccuredFlag_U8X==1)
            v_ZaxisStrideDetectedFlag_u8r = 1; /* For Z-axis Stride Pattern got detected */
    
    if (S_Param_Caxis.V_ValidStridePatternOccuredFlag_U8X==1)
            v_CaxisStrideDetectedFlag_u8r = 1; /* For Composite-axis Stride Pattern got detected */
    
    /* 三轴中至少有两轴检测到有效步伐 */
    if( ((v_XaxisStrideDetectedFlag_u8r==1)&&(v_YaxisStrideDetectedFlag_u8r==1))||
        ((v_YaxisStrideDetectedFlag_u8r==1)&&(v_ZaxisStrideDetectedFlag_u8r==1))||
        ((v_ZaxisStrideDetectedFlag_u8r==1)&&(v_XaxisStrideDetectedFlag_u8r==1))
      )
    {   
//		SEGGER_RTT_WriteString(0,"检测有效的计步\r\n");
//		SEGGER_RTT_printf(0, "间隔时间 : %d.\r\n", );
          PEDO_FindDominantAxisOrder( &S_Param_Xaxis, &S_Param_Yaxis, &S_Param_Zaxis, v_Activity_u8r, &v_IstDomAxis_u8r, &v_IIndDomAxis_u8r, &v_IIIrdDomAxis_u8r );
          v_XaxisStrideDetectedFlag_u8r=0;
          v_YaxisStrideDetectedFlag_u8r=0;
          v_ZaxisStrideDetectedFlag_u8r=0;
          v_CaxisStrideDetectedFlag_u8r=0;
    }
   /* EasyCASE > */
   /* EasyCASE - */
    /* Call PEDO_Find_StableSingleAxis_StableCadenceX100_StableCadenceX100API, every 40 msec cycle,
       to find the Stable axis out of 4-axis X,Y,Z and Composite axis and Stable Cadence for the selected axis */
   PEDO_Find_StableSingleAxis_StableCadenceX100( &S_Param_Xaxis, &S_Param_Yaxis, &S_Param_Zaxis, &S_Param_Caxis, v_IstDomAxis_u8r, v_IIndDomAxis_u8r,
                                                  v_Activity_u8r, &v_SingleStableAxis_u8r, &V_SingleAxisStableCadence_U16R  );
   V_CompositeAxis_StrideRatio_U16R = (U16)(2*S_Param_Caxis.A_StridePeakToPeakValBuf_U16X[0]/(F32)S_Param_Caxis.A_StrideIntervalBuf_U8X[0]);
   V_CompositeAxis_CadenceX100_U16R = S_Param_Caxis.V_CadenceX100_U16X;
   /* EasyCASE ) */
   /* EasyCASE ( 101541
      Axis Selection --> Single/Composite Axis ? */
   /* Do the Axis selection Single/Composite based on the activity detected */
   if (v_Activity_u8r == RUN)
      {
      V_AxisSelectFlag_U8R=C_AXIS_IS_SINGLE_U8X;
      }
   else
      {
      V_AxisSelectFlag_U8R=C_AXIS_IS_COMPOSITE_U8X;
      }
   /* EasyCASE ) */
   /* EasyCASE ( 101542
      Assign Trend Dominating Axis and filtered acceleration value of Trend Dominating Axis */
   /* Assign Trend Dominating Axis based on the axis selection (Single Axis or Composite Axis) */
   if (V_AxisSelectFlag_U8R==C_AXIS_IS_SINGLE_U8X)
      {
      v_TrendDominantAxis_u8r=v_SingleStableAxis_u8r;
      }
   else
      {
      v_TrendDominantAxis_u8r=C_CompositeAxis_U8X;
      }
   /* Assign acceleration value of Trend Dominating Axis */
   /* EasyCASE - */
   v_FiltAccelTrendAxis_s16r=v_FiltAccel4Axis_s16r[v_TrendDominantAxis_u8r];
   /* EasyCASE ) */
   /* EasyCASE ( 101543
      Find Trends (Positive/Negetive)? */
   PEDO_TrendFinder(v_FiltAccelTrendAxis_s16r, &v_FiltAccelTrendAxis_s16r);
   /* EasyCASE ) */

   /* EasyCASE ) */
	
   /* EasyCASE ( 101545
      Human Activity */
   /* Check whether the trend is Positive or Negative */
   if (V_StatusFlags_U8R & M_NegativeTrend_U8X)
   {
      /* Save the value of acceleration at Negetive Trend */
      v_NegTrendAccelVal_s16r = v_FiltAccelTrendAxis_s16r;
   }
   else
   {	   
      if (V_StatusFlags_U8R & M_PositiveTrend_U8X)
      {

         /* Set the status as Step Qualified or Unqualified ? */
         V_StatusFlags_U8R &=~(M_Qualified_U8X|M_UnQualified_U8X);
         
         /* Calculate the acceleration value difference for successive trends 
            (Negetive trend value - Positive trend value) */
         v_FiltAccelTrendAxis_s16r = v_NegTrendAccelVal_s16r - v_FiltAccelTrendAxis_s16r;
         TASK_STEP_PRINTF("v_FiltAccelTrendAxis_s16r : %d.\r\n",v_FiltAccelTrendAxis_s16r );	
		 TASK_STEP_PRINTF("v_CountStepToStep_u8r : %d.\r\n",v_CountStepToStep_u8r );
	
#ifdef DATA_TEST		  
		uint32_t t_data = 0;
		t_data = (v_CountStepToStep_u8r<<16) | (v_FiltAccelTrendAxis_s16r);
		
		StoreStepTestData((uint8_t *)(&t_data));
#endif		  
		  
         /* Clear the Activity */
         V_Activity_U8R = C_Clear_U8X;
         /* EasyCASE - */
         /* Check whether the step is Walk, Jog or Run? */
         /* EasyCASE ( 101546
            Lazy Walk */
         /* Check whether the step is lazy walk step */
         if (IS_LAZY_WALK_STEP(v_FiltAccelTrendAxis_s16r)&& (V_SwimStatus==0))
         {
			TASK_STEP_PRINTF("LAZY WALK    ");
            if (((v_CountStepToStep_u8r > C_LazyWalkStepToStepLowerLimit_U8X) 
                && (v_CountStepToStep_u8r < C_LazyWalkStepToStepUpperLimit_U8X)))
            {
               V_StatusFlags_U8R |= M_Qualified_U8X;
			   TASK_STEP_PRINTF("M_Qualified_U8X\r\n");
            }
            else 
            {
               V_StatusFlags_U8R |= M_UnQualified_U8X;
				TASK_STEP_PRINTF("M_UnQualified_U8X\r\n");
            }
            /* Slow activity */
            V_Activity_U8R |= M_SlowWalk_U8X;
         }
         /* EasyCASE ) */
         /* EasyCASE ( 101547
            Walk */
         if (IS_WALK_STEP(v_FiltAccelTrendAxis_s16r)&& (V_SwimStatus==0))
            {
			TASK_STEP_PRINTF("WALK");
            if (((PEDO_GetAbsoluteShort(v_CountStepToStep_u8r -  v_CountStepToStepPrev1_u8r) <= MAX_COUNT_DIFFERENCE) ||
                ( PEDO_GetAbsoluteShort(v_CountStepToStep_u8r -  v_CountStepToStepPrev2_u8r) <=MAX_COUNT_DIFFERENCE) )&&
                ((v_CountStepToStep_u8r > C_WalkStepToStepLowerLimit_U8X) && (v_CountStepToStep_u8r < C_WalkStepToStepUpperLimit_U8X)))
               {
					V_StatusFlags_U8R |= M_Qualified_U8X;
					TASK_STEP_PRINTF("M_Qualified_U8X\r\n");
               }
            else
               {
               V_StatusFlags_U8R |= M_UnQualified_U8X;
				   TASK_STEP_PRINTF("M_UnQualified_U8X\r\n");
               }
            /* Medium activity */
            V_Activity_U8R |= M_Walk_U8X;
            }
         /* EasyCASE ) */
         /* EasyCASE ( 101548
            Jog */
         /* Check whether the step is Jog step */
         if (IS_JOG_STEP(v_FiltAccelTrendAxis_s16r)&& (V_SwimStatus==0))
         {
			TASK_STEP_PRINTF("JOG");
            if (((PEDO_GetAbsoluteShort(v_CountStepToStep_u8r -  v_CountStepToStepPrev1_u8r) <= MAX_COUNT_DIFFERENCE) ||
                ( PEDO_GetAbsoluteShort(v_CountStepToStep_u8r -  v_CountStepToStepPrev2_u8r) <=MAX_COUNT_DIFFERENCE) )&&
                ((v_CountStepToStep_u8r > C_JogStepToStepLowerLimit_U8X) && (v_CountStepToStep_u8r < C_JogStepToStepUpperLimit_U8X)))
               {
					V_StatusFlags_U8R |= M_Qualified_U8X;
					TASK_STEP_PRINTF("M_Qualified_U8X\r\n");
               }
            else
               {
               V_StatusFlags_U8R |= M_UnQualified_U8X;
				   TASK_STEP_PRINTF("M_UnQualified_U8X\r\n");
               }
            /* Brisk activity */
            V_Activity_U8R |= M_Jog_U8X;
         }
		          /* EasyCASE ) */
         /* EasyCASE ( 101548
            Jog */
         /* Check whether the step is Swim step */
         if (IS_SWIM_STEP(v_FiltAccelTrendAxis_s16r)&& (V_SwimStatus!=0))
         {
			TASK_STEP_PRINTF("SWIM");
            if (((PEDO_GetAbsoluteShort(v_CountStepToStep_u8r -  v_CountStepToStepPrev1_u8r) <= MAX_COUNT_DIFFERENCE) ||
                ( PEDO_GetAbsoluteShort(v_CountStepToStep_u8r -  v_CountStepToStepPrev2_u8r) <=MAX_COUNT_DIFFERENCE) )&&
                ((v_CountStepToStep_u8r > C_SwimStepToStepLowerLimit_U8X) && (v_CountStepToStep_u8r < C_SwimStepToStepUpperLimit_U8X)))
               {
					V_StatusFlags_U8R |= M_Qualified_U8X;
					TASK_STEP_PRINTF("M_Qualified_U8X\r\n");
               }
            else
               {
               V_StatusFlags_U8R |= M_UnQualified_U8X;
				   TASK_STEP_PRINTF("M_UnQualified_U8X\r\n");
               }
            /* Brisk activity */
            V_Activity_U8R |= M_Swim_U8X;
         }
         /* EasyCASE ) */
         /* EasyCASE ( 101549
            Step counting */
         /* EasyCASE ( 101550 
            Time out in Detection */
      if ((v_ModeTimeoutCount_u8r>C_DetectionModeTimeOut_U8X)&&( V_StatusFlags_U8R&M_ModeDetection_U8X)
             /* Check whether steps ate detected for 2.8 seconds in Detection Mode or not? */)
      {
         /* Clear the mode timeout count */
         v_ModeTimeoutCount_u8r=C_Clear_U8X;
         if (((V_StatusFlags_U8R & M_DisableRobustness_U8X)==0))
         {
            /* No activity in detection mode; so clear the Temporary step count*/
            v_CorrectionCount_u8r=C_Clear_U8X;
            V_InterStepCount_U8R=C_Clear_U8X;
            V_StatusFlags_U8R &=~(M_Qualified_U8X|M_UnQualified_U8X);
            v_ErrorCount_u8r=C_Clear_U8X;

         }
      }
      /* EasyCASE ) */
      /* EasyCASE ( 101551
            Time out in Counting */
      if ((v_ModeTimeoutCount_u8r>C_CountingModeTimeOut_U8X)&&( V_StatusFlags_U8R&M_ModeCounting_U8X)
             /* No steps for 4 seconds in Count Mode*/)
      {
         /* Clear the mode timeout count */
         v_ModeTimeoutCount_u8r=C_Clear_U8X;
         if (((V_StatusFlags_U8R & M_DisableRobustness_U8X)==0))
         {
               /* No activity in counting mode; so clear the Temporary step count*/
             v_CorrectionCount_u8r=C_Clear_U8X;
             V_StatusFlags_U8R &=~(M_Qualified_U8X|M_UnQualified_U8X);
             v_ErrorCount_u8r=C_Clear_U8X;
             V_StatusFlags_U8R|=M_ModeDetection_U8X;
             V_StatusFlags_U8R&=~M_ModeCounting_U8X;

         }
      }
      /* EasyCASE ) */
         if (/*Check whether step is valid or not*/
             ((V_StatusFlags_U8R & (M_Qualified_U8X|M_UnQualified_U8X))!=0))
         {
            /* If there is change in activity and the current result diff are greater than certain "Threshold" then
               temporary counts are cleared.Threshold=(Largest of Current Result Diff and previous Result Diff)/2.
               This is applicable in detection mode */
            /* EasyCASE ( 101552
               Activity Monitor */
            if (((v_PreviousActivity_u8r & V_Activity_U8R)==C_Clear_U8X) && (v_PreviousActivity_u8r !=C_Clear_U8X) &&
                ( V_StatusFlags_U8R&M_ModeDetection_U8X) && ((V_StatusFlags_U8R & M_DisableRobustness_U8X)==0) &&
                (((PEDO_GetAbsoluteShort(v_FiltAccelTrendAxis_s16r-v_PrevPosNegTrendAccelValDiff_s16r))<<C_CountOne_U8X)>
                ((v_FiltAccelTrendAxis_s16r>v_PrevPosNegTrendAccelValDiff_s16r)?v_FiltAccelTrendAxis_s16r:v_PrevPosNegTrendAccelValDiff_s16r)))
            {
				
               /* Activities differs in Detection state;  So, clear the temporary step count */
               V_InterStepCount_U8R = C_Clear_U8X;
               v_CorrectionCount_u8r=C_Clear_U8X;
               V_StatusFlags_U8R &=~(M_Qualified_U8X|M_UnQualified_U8X);

            }
            /* EasyCASE ) */
            v_PreviousActivity_u8r=V_Activity_U8R; /*Stores the current Activity Type*/
            v_PrevPosNegTrendAccelValDiff_s16r=v_FiltAccelTrendAxis_s16r; /* Stores the current Negetive-To-Positive Trend Value Difference */
            v_ErrorCount_u8r=C_Clear_U8X; /*Error Count cleared*/
            v_ModeTimeoutCount_u8r=C_Clear_U8X; /* Reset the Mode Timeout Count */
            /* EasyCASE - */
            /* Find instantaneous activity */
            /* EasyCASE - */
            v_AvgActivityDetectionRatio_u16r = v_FiltAccelTrendAxis_s16r/V_SampleCountForCycle_U8R;

            /* EasyCASE - */
            /* Check whether the  step is Qualified */
            if (V_StatusFlags_U8R & M_Qualified_U8X)
            {
               /* Check whether the mode is counting mode */
               /* EasyCASE ( 101553
                  Activity Detection Block - FIFO */
               for (v_Index_u8r=(C_ActivityFilterSize_U8X-1);v_Index_u8r>C_CountZero_U8X;v_Index_u8r--)
               {
                  a_AvgActivityDetectionRatioBuf_u16r[v_Index_u8r]=a_AvgActivityDetectionRatioBuf_u16r[v_Index_u8r-1];
               }
               a_AvgActivityDetectionRatioBuf_u16r[C_CountZero_U8X]= (U16)((F32)v_FiltAccelTrendAxis_s16r/(F32)V_SampleCountForCycle_U8R);
               /* EasyCASE - */
               /* Calculate the sum of the ActivityDetectionRatio buffer values */
               v_TempU32_u32r=0;
               for (v_Index_u8r=(C_ActivityFilterSize_U8X-1);v_Index_u8r>=C_CountZero_U8X;v_Index_u8r--)
                  {
                  v_TempU32_u32r += a_AvgActivityDetectionRatioBuf_u16r[v_Index_u8r];
                  }
               /*Taking Moving average of ratio*/
               v_AvgActivityDetectionRatio_u16r=(U16)((F32)v_TempU32_u32r/(F32)C_ActivityFilterSize_U8X);
               /* EasyCASE - */
               /* Check the current activity using Average Activity Detection Ratio */
               if (/*If Activity threshold ratio > Lower threshold for run*/
                   (v_AvgActivityDetectionRatio_u16r>C_RunLowerRange_U8X))
                  {
                  V_ActivityDetected_U8R = (U8)RUN;
                  }
               else
                  {
                  if (/*If threshold is <= Run Lower range and != 0*/
                      (v_AvgActivityDetectionRatio_u16r>C_WalkLowerRange_U8X))
                     {
                     /* Check Composite Axis Stride Peak-To-Peak Value and Stride Interval Time Ratio to varify whether activity is RUN or WALK? */
                     /* EasyCASE - */
                     V_ActivityDetected_U8R = (U8)WALK;
                     }
                  else
                     {
                     V_ActivityDetected_U8R = (U8) STATIONARY;
                     }
                  }
               /* EasyCASE ) */
               if (V_StatusFlags_U8R&M_ModeCounting_U8X)
                  {
                  /* EasyCASE ( 101554
                     Counting Mode */
                  if (/*Check whether correction count >3 in Counting Mode*/
                      v_CorrectionCount_u8r>C_CorrectionCountLimit_U8X)
                     {
                     /* Increment Step Count with Correction Count added into it */
                     V_StepCount_U16R+=(v_CorrectionCount_u8r+C_CountOne_U8X);
                     
                     /* Reset the Correction Count */
                     v_CorrectionCount_u8r = C_Clear_U8X;                     

                     }
                  else
                     {
                     /* Increment the Step Count */
                      V_StepCount_U16R++;          
 

                     }
				  
			      if ((v_NegTrendAccelVal_s16r >180 )&&(v_CountStepToStep_u8r <15)&&(V_SwimStatus !=0))
	   	          {
				     V_SwimStatus |= M_Tocuh_U8X;
					 V_StepCount_U16R--;
	              }
					 RTOS_LOG_PRINTF("**STEPS  :  %d.\r\n",V_StepCount_U16R );
                  /* EasyCASE ) */
                  }
               else
                  {
                  /*Check whether current mode is Detection Mode*/
                  /* EasyCASE ( 101555
                     Detection Mode */
                  if (V_StatusFlags_U8R&M_ModeDetection_U8X)
                     {
                     if (/* Correction count is added to InterStepCount when correction count > 3 in detection mode */
                         v_CorrectionCount_u8r>C_CorrectionCountLimit_U8X)
                        {
                        /* Increment the InterStepCount with CorrectionCount added into it. */
                        V_InterStepCount_U8R+=(v_CorrectionCount_u8r+1);
                        
                        /* Reset the Correction Count */
                        v_CorrectionCount_u8r = C_Clear_U8X;
                        

                        }
                     else
                        {
                        /* Increment the InterStepCount */
                         V_InterStepCount_U8R++;                         

                        }
                     if (/* When interstep count > 9 mode changed to counting in case if Robustness feature enabled;
                            When interstep count > 3 mode changed to counting in case if Robustness feature disabled */
                         ((V_InterStepCount_U8R > V_InterStepCountLimit_U8R)&&((V_StatusFlags_U8R & M_DisableRobustness_U8X)==0))||
                         ((V_InterStepCount_U8R > V_InterStepCountLimit_U8R)&&((V_StatusFlags_U8R & M_DisableRobustness_U8X)==M_DisableRobustness_U8X)))
                        {
							TASK_STEP_PRINTF("**START STEP\r\n");
							TASK_STEP_PRINTF("V_StatusFlags_U8R : 0x%x.\r\n",V_StatusFlags_U8R );
							TASK_STEP_PRINTF("V_InterStepCount_U8R : %d.\r\n",V_InterStepCount_U8R );
							TASK_STEP_PRINTF("V_InterStepCountLimit_U8R : %d.\r\n",V_InterStepCountLimit_U8R );
                        /* Set the mode to MODE_COUNTING */
                        V_StatusFlags_U8R|=M_ModeCounting_U8X;
                        V_StatusFlags_U8R&=~M_ModeDetection_U8X;
                        
                        /* Increment the StepCount */
                        V_StepCount_U16R += (V_InterStepCount_U8R+v_CorrectionCount_u8r);
                        
                        /* Reset the InterStepCount */
                        V_InterStepCount_U8R = C_Clear_U8X;
                        
                        /* Reset the CorrectionCount */
                         v_CorrectionCount_u8r = C_Clear_U8X;
                         

                        }
                     }
                  /* EasyCASE ) */
                  }
               }
            else
               {
               /* EasyCASE ( 101556
                  Correction Count */
               if (/*Check whether Step is unqualified*/
                   V_StatusFlags_U8R & M_UnQualified_U8X)
                  {
                  /* Increment the Correction Count */
                  v_CorrectionCount_u8r++;

                  }
               /* EasyCASE ) */
               }
            }
         else
            {
            /* EasyCASE ( 101557
               Error Count */
            if (/*Error count is incremented if the step is not valid and not due to noise*/
                (v_FiltAccelTrendAxis_s16r>C_SENSOR_NOISE_LEVEL_U8X))
               {
               /*Error Count is incremented*/
               v_ErrorCount_u8r++;
				TASK_STEP_PRINTF("v_ErrorCount_u8r : 0x%x.\r\n",v_ErrorCount_u8r );   
               }
            if (/*When the error count becomes greater than 3 the temporary counts are cleared*/
                v_ErrorCount_u8r>C_ErrorCountLimit_U8X)
               {
               /*The mode changed to detection and counts are cleared*/
               V_StatusFlags_U8R|=M_ModeDetection_U8X;
               V_StatusFlags_U8R&=~M_ModeCounting_U8X;
               v_ErrorCount_u8r=C_Clear_U8X;
               v_CorrectionCount_u8r=C_Clear_U8X;
               V_InterStepCount_U8R=C_Clear_U8X;
               V_StatusFlags_U8R &=~(M_Qualified_U8X|M_UnQualified_U8X);
               v_ModeTimeoutCount_u8r = C_Clear_U8X;
               

               }
            /* EasyCASE ) */
            }
         /* EasyCASE ( 101558
            Step to Step Count Updation */
         if (/*Count step to step is updated if the trend change is not due to noise*/
             v_FiltAccelTrendAxis_s16r>C_SENSOR_NOISE_LEVEL_U8X)
            {
            /* Update the last, secondlast and thridlast count variables */
            v_CountStepToStepPrev2_u8r = v_CountStepToStepPrev1_u8r;
            v_CountStepToStepPrev1_u8r = v_CountStepToStep_u8r;
            v_CountStepToStep_u8r = C_Clear_U8X;
            }
         /* EasyCASE ) */
         /* EasyCASE ) */
         /* Reset the sample count for cycle */
         V_SampleCountForCycle_U8R = C_Clear_U8X;
         }
      else
         {
         /* EasyCASE ( 101559
            Time out in Detection */
         if (/* No steps for 2.8 seconds in Detection Mode */
             /* No steps for 4 seconds in Counting Mode */
             ((v_ModeTimeoutCount_u8r>C_DetectionModeTimeOut_U8X)&&( V_StatusFlags_U8R&M_ModeDetection_U8X))||
             ((v_ModeTimeoutCount_u8r>C_CountingModeTimeOut_U8X)&&( V_StatusFlags_U8R&M_ModeCounting_U8X)))
            {
            v_AvgActivityDetectionRatio_u16r=0;
            V_ActivityDetected_U8R = (U8) STATIONARY;
            for (v_Index_u8r=(C_ActivityFilterSize_U8X-1);v_Index_u8r>=C_CountZero_U8X;v_Index_u8r--)
               {
               a_AvgActivityDetectionRatioBuf_u16r[v_Index_u8r]=0;
               }
            }
         /* EasyCASE ) */
         }
      }
   /* EasyCASE ) */
   /*Current status are stored*/
   //v_PreStatusFlags_u8r=V_StatusFlags_U8R;
   /* EasyCASE - */
   /* return the composite value */
   return v_FiltAccelTrendAxis_s16r;
   }



static short PEDO_GetAbsoluteShort(short v_Val_s16r)
{
   return (v_Val_s16r < 0)? -v_Val_s16r : v_Val_s16r;
}

static short  Max_Abs( short Val1,short Val2,short Val3)
{
	short Val = 0;
	if( PEDO_GetAbsoluteShort(Val1) > PEDO_GetAbsoluteShort(Val2) )
	{
		Val = PEDO_GetAbsoluteShort(Val1);
	}
	else
	{
		Val = PEDO_GetAbsoluteShort(Val2);
	}
	return (Val < PEDO_GetAbsoluteShort(Val3))? PEDO_GetAbsoluteShort(Val3): Val;
}

static void PEDO_TrendFinder(short v_AccelVal_s16r, short *v_OptimalAccelVal_s16r)
{
   /* EasyCASE ( 101627
      静态局部变量，退出本函数有继续存在，但只在本函数内有效 */
   static signed short     v_Prev1AccelVal_s16r = 0;  /* Previous Acceleration value */
   static signed short     v_Prev2AccelVal_s16r = 0;  /* Previous to previous Acceleration value */
   static signed char      v_CurrTrend_s8r = NEGATIVE;     /* Current trend (default trend in Negetive trend )*/
   /* EasyCASE ) */
   /* EasyCASE ( 101778
      Find Positive/Negetive trends and value at trends */
   /* Clear the Positive nad Negetive trend bit in the Status Flag */
   /* EasyCASE - */
   V_StatusFlags_U8R &=~(M_NegativeTrend_U8X|M_PositiveTrend_U8X); /*Status flag to hold the change in trend */
   if (v_CurrTrend_s8r == POSITIVE)
   {
      /* EasyCASE ( 101628
         Current trend is Positive, Check for Negetive trend */
      if (( (v_AccelVal_s16r+2) < v_Prev1AccelVal_s16r) && ( (v_AccelVal_s16r+2) < v_Prev2AccelVal_s16r))
      {
         v_CurrTrend_s8r = NEGATIVE; /* Set the current trend as negative */
         V_StatusFlags_U8R |= M_NegativeTrend_U8X; /* Set the change in trend as negative */
         
         /* Return the optimal value of acceleration at trend change
          (in this case maximum of prev and prevToPrev acceleration values) */
         *v_OptimalAccelVal_s16r = (v_Prev1AccelVal_s16r > v_Prev2AccelVal_s16r)?
                                    v_Prev1AccelVal_s16r : v_Prev2AccelVal_s16r;
      }
      /* EasyCASE ) */
   }
   else
   {
      /* EasyCASE ( 101629
         Current trend is Negetive, Check for Positive trend */
      if ((v_AccelVal_s16r > (v_Prev1AccelVal_s16r+2) ) && (v_AccelVal_s16r > (v_Prev2AccelVal_s16r+2)))
      {
         v_CurrTrend_s8r = POSITIVE;    /* Set the current trend as Positive */
         V_StatusFlags_U8R |= M_PositiveTrend_U8X;  /* Set the change in trend as Positive */
         
         /* Return the optimal value of acceleration at trend change
          (in this case maximum of prev and prevToPrev acceleration values) */
         *v_OptimalAccelVal_s16r = (v_Prev1AccelVal_s16r < v_Prev2AccelVal_s16r)?
                                    v_Prev1AccelVal_s16r : v_Prev2AccelVal_s16r;
      }
      /* EasyCASE ) */
   }
   /* Update the second last composite value */
   v_Prev2AccelVal_s16r = v_Prev1AccelVal_s16r;
   /* Update the last composite values */
   v_Prev1AccelVal_s16r = v_AccelVal_s16r;
   /* Return the Change in trend */
   /* EasyCASE ) */
}

void PEDO_Get_SingleAxisAccelFeatures( S16 v_AccelValFilt_s16r, ts_SingleAxis *p_AccelerationPatternFeatures_xxr)
   {
   /* EasyCASE ( 101636
      Local Variables */
   U8  v_Index_u8r=0;   
   U8  v_TrendChangeFoundFlag_u8r=0;
   U8  v_StrideInterval_u8r=0;
   U8  v_InstStrideUniformity_u8r=0;
   U8 v_UniformityTestPassedFlag_u8r=0;
   U8 v_StrideIntervalTestPassedFlag_u8r=0;
   U8 v_TestCount_u8r=0;
   
   S16 v_InstTrendValue_s16r=0;
   S16 v_StrideMinVal_s16r=0;
   S16 v_StrideMaxVal_s16r=0;
   U16 v_StridePeakToPeakVal_u16r=0;
   U16 v_Temp_u16r=0;
   /* EasyCASE ) */
   /* EasyCASE ( 101662
      Increment V_StrideIntervalCount_U8X counter */
   if (p_AccelerationPatternFeatures_xxr->V_SuccessiveTrendTimeValidityFlag_U8X==1)
   {
      p_AccelerationPatternFeatures_xxr->V_SuccessiveTrendTimeCount_U8X++; /* Increment the count every 40msec */	  

      /* If there is no trend detected for 2.4 seconds disable V_SuccessiveTrendTimeValidityFlag_U8X flag */
      /* 长时间没检测到有效的数据变动，将标志位清零。清零后将重新寻找拐点*/
      if (p_AccelerationPatternFeatures_xxr->V_SuccessiveTrendTimeCount_U8X >= C_STRIDE_INTERVAL_COUNT_MAXLIMIT_U8R)
      {
         p_AccelerationPatternFeatures_xxr->V_SuccessiveTrendTimeValidityFlag_U8X=0; /* Reset V_StrideIntervalValidityFlag_U8X flag */
         p_AccelerationPatternFeatures_xxr->V_SuccessiveTrendTimeCount_U8X=C_STRIDE_INTERVAL_COUNT_MAXLIMIT_U8R; /* Clear V_StrideIntervalCount_U8X  count */
      }
   }
   /* EasyCASE ) */
   /* EasyCASE ( 101661
      Update the acceleration value FIFO */
   if ((p_AccelerationPatternFeatures_xxr->V_AccelBufDataCount_U8X < C_SINGLE_AXIS_ACCEL_VALUE_BUFSIZE_U8R))
   {
      p_AccelerationPatternFeatures_xxr->A_AccelBuf_S16X[p_AccelerationPatternFeatures_xxr->V_AccelBufDataCount_U8X] = v_AccelValFilt_s16r;
      p_AccelerationPatternFeatures_xxr->V_AccelBufDataCount_U8X ++;
   }
   else
   {
      for (v_Index_u8r=0; v_Index_u8r<(C_SINGLE_AXIS_ACCEL_VALUE_BUFSIZE_U8R-1); v_Index_u8r++)
      {
         p_AccelerationPatternFeatures_xxr->A_AccelBuf_S16X[C_SINGLE_AXIS_ACCEL_VALUE_BUFSIZE_U8R-1-v_Index_u8r]=p_AccelerationPatternFeatures_xxr->A_AccelBuf_S16X[C_SINGLE_AXIS_ACCEL_VALUE_BUFSIZE_U8R-2-v_Index_u8r];
      }
      p_AccelerationPatternFeatures_xxr->A_AccelBuf_S16X[0]=v_AccelValFilt_s16r;
   }
   /* EasyCASE ) */
   /* EasyCASE ( 101660
      Check for trends ( Positive or Negetive Trend ) ?? */
   //检测数据走势，寻找拐点
   if (/* Check for Negetive Trend if Positive Trend got detected ?? */
	   //增加 C_SENSOR_NOISE_LEVEL_U8X的值间距，避免一两个数值的跳动而误进入，并修改为严格的趋势
       ( p_AccelerationPatternFeatures_xxr->A_AccelBuf_S16X[0] < ( p_AccelerationPatternFeatures_xxr->A_AccelBuf_S16X[1] - INFLECTION_POINT_DIFF ) ) &&
       ( p_AccelerationPatternFeatures_xxr->A_AccelBuf_S16X[1] < ( p_AccelerationPatternFeatures_xxr->A_AccelBuf_S16X[2] - INFLECTION_POINT_DIFF ) ) &&
//		( p_AccelerationPatternFeatures_xxr->A_AccelBuf_S16X[0] < p_AccelerationPatternFeatures_xxr->A_AccelBuf_S16X[1] ) &&
//       ( p_AccelerationPatternFeatures_xxr->A_AccelBuf_S16X[0] < p_AccelerationPatternFeatures_xxr->A_AccelBuf_S16X[2] ) &&
       (p_AccelerationPatternFeatures_xxr->V_InstTrendType_S8X == POSITIVE))
   {
      /* Set the current trend as Negative trend ??  */
      p_AccelerationPatternFeatures_xxr->V_InstTrendType_S8X = NEGATIVE;  //将所有原来的-1值改为2，否则比较的时候出问题

                  
      /* Find the optimal value for the Negetive Trend */
		 //保存波峰值
//      v_InstTrendValue_s16r = (p_AccelerationPatternFeatures_xxr->A_AccelBuf_S16X[1] > p_AccelerationPatternFeatures_xxr->A_AccelBuf_S16X[2])?
//      p_AccelerationPatternFeatures_xxr->A_AccelBuf_S16X[1] : p_AccelerationPatternFeatures_xxr->A_AccelBuf_S16X[2];
		v_InstTrendValue_s16r = p_AccelerationPatternFeatures_xxr->A_AccelBuf_S16X[2];
      
      /* Set the trend change found flag */                    
      v_TrendChangeFoundFlag_u8r = 1;
   }
   else
   {
      if (/* Check for Positive Trend if Negetive Trend got detected ?? */
          ( p_AccelerationPatternFeatures_xxr->A_AccelBuf_S16X[0] > ( p_AccelerationPatternFeatures_xxr->A_AccelBuf_S16X[1] + INFLECTION_POINT_DIFF ) ) &&
			( p_AccelerationPatternFeatures_xxr->A_AccelBuf_S16X[1] > ( p_AccelerationPatternFeatures_xxr->A_AccelBuf_S16X[2] + INFLECTION_POINT_DIFF ) )&&
//          ( p_AccelerationPatternFeatures_xxr->A_AccelBuf_S16X[0] > p_AccelerationPatternFeatures_xxr->A_AccelBuf_S16X[1] ) &&
//          ( p_AccelerationPatternFeatures_xxr->A_AccelBuf_S16X[0] > p_AccelerationPatternFeatures_xxr->A_AccelBuf_S16X[2] )&&
          (p_AccelerationPatternFeatures_xxr->V_InstTrendType_S8X == NEGATIVE))
      {
         /* Set the current trend as Positive trend */
         p_AccelerationPatternFeatures_xxr->V_InstTrendType_S8X = POSITIVE;
                           
         /* Find the optimal value for the Positive Trend */
		  //保存波谷值
//         v_InstTrendValue_s16r =( p_AccelerationPatternFeatures_xxr->A_AccelBuf_S16X[1] < p_AccelerationPatternFeatures_xxr->A_AccelBuf_S16X[2])?
//         p_AccelerationPatternFeatures_xxr->A_AccelBuf_S16X[1] : p_AccelerationPatternFeatures_xxr->A_AccelBuf_S16X[2];
		v_InstTrendValue_s16r = p_AccelerationPatternFeatures_xxr->A_AccelBuf_S16X[2];
         
         /* Set the trend change found flag */                            
         v_TrendChangeFoundFlag_u8r = 1;
      }
   }
   /* EasyCASE ) */
   /* EasyCASE ( 101777
      IF valid trend is found, (1)Check for Valid Stride Patterns; (2)Update the Stride Parameters
      ( Stride Peak-To-Peak Value, Stride Interval, Cadence, Axis_Stability_Flag etc.). */
   /* Check whether a valid trend (Positive or negetive) detected  ?? */
   if (v_TrendChangeFoundFlag_u8r == 1 /* a valid trend got detected 检测到拐点*/)
   {
      /* EasyCASE ( 101650
         Update the trend values like TYPE, VALUE, SUCCESSIVE_TREND_TIME_COUNT into FIFO */
      if (p_AccelerationPatternFeatures_xxr->V_AccelTrendBufDataCount_U8X < C_SINGLE_AXIS_TREND_BUFSIZE_U8R)
      {
         p_AccelerationPatternFeatures_xxr->A_AccelTrendBuf_S8X[p_AccelerationPatternFeatures_xxr->V_AccelTrendBufDataCount_U8X]       = p_AccelerationPatternFeatures_xxr->V_InstTrendType_S8X;
         p_AccelerationPatternFeatures_xxr->A_AccelTrendValueBuf_S16X[p_AccelerationPatternFeatures_xxr->V_AccelTrendBufDataCount_U8X] = v_InstTrendValue_s16r;
         p_AccelerationPatternFeatures_xxr->A_AccelSuccecciveTrendTimeBuf_U8X[p_AccelerationPatternFeatures_xxr->V_AccelTrendBufDataCount_U8X] = p_AccelerationPatternFeatures_xxr->V_SuccessiveTrendTimeCount_U8X;
         p_AccelerationPatternFeatures_xxr->V_AccelTrendBufDataCount_U8X++;
      }
      else
      {
         for (v_Index_u8r=0; v_Index_u8r<(C_SINGLE_AXIS_TREND_BUFSIZE_U8R-1); v_Index_u8r++)
         {
            p_AccelerationPatternFeatures_xxr->A_AccelTrendBuf_S8X[C_SINGLE_AXIS_TREND_BUFSIZE_U8R-1-v_Index_u8r]       = p_AccelerationPatternFeatures_xxr->A_AccelTrendBuf_S8X[C_SINGLE_AXIS_TREND_BUFSIZE_U8R-2-v_Index_u8r];
             p_AccelerationPatternFeatures_xxr->A_AccelTrendValueBuf_S16X[C_SINGLE_AXIS_TREND_BUFSIZE_U8R-1-v_Index_u8r] = p_AccelerationPatternFeatures_xxr->A_AccelTrendValueBuf_S16X[C_SINGLE_AXIS_TREND_BUFSIZE_U8R-2-v_Index_u8r];
            p_AccelerationPatternFeatures_xxr->A_AccelSuccecciveTrendTimeBuf_U8X[C_SINGLE_AXIS_TREND_BUFSIZE_U8R-1-v_Index_u8r] = p_AccelerationPatternFeatures_xxr->A_AccelSuccecciveTrendTimeBuf_U8X[C_SINGLE_AXIS_TREND_BUFSIZE_U8R-2-v_Index_u8r];
         }
         p_AccelerationPatternFeatures_xxr->A_AccelTrendBuf_S8X[0]       = p_AccelerationPatternFeatures_xxr->V_InstTrendType_S8X;
         p_AccelerationPatternFeatures_xxr->A_AccelTrendValueBuf_S16X[0] = v_InstTrendValue_s16r;
         p_AccelerationPatternFeatures_xxr->A_AccelSuccecciveTrendTimeBuf_U8X[0] = p_AccelerationPatternFeatures_xxr->V_SuccessiveTrendTimeCount_U8X;
      }
      v_TrendChangeFoundFlag_u8r = 0; /* clear v_TrendChangeFoundFlag_u8r flag*/
      p_AccelerationPatternFeatures_xxr->V_SuccessiveTrendTimeCount_U8X = 0; /* Clear Successive Trend Time Counter */
      p_AccelerationPatternFeatures_xxr->V_SuccessiveTrendTimeValidityFlag_U8X=1; /* 置位趋势时间计数标志，V_SuccessiveTrendTimeCount_U8X将会开始计时 */
      /* EasyCASE ) */
      /* Check for Valid Stride Pattern */
      if (/* Check for Stride Pattern whether it is proper or not i.e. {-1, +1,-1, +1 } or alternate positive and negetive trends ?? */
		 //V_InstTrendType_S8X已经从 -1改为2
       (p_AccelerationPatternFeatures_xxr->A_AccelTrendBuf_S8X[0] == NEGATIVE)&& (p_AccelerationPatternFeatures_xxr->A_AccelTrendBuf_S8X[1] == POSITIVE)&&
       (p_AccelerationPatternFeatures_xxr->A_AccelTrendBuf_S8X[2] == NEGATIVE)&& (p_AccelerationPatternFeatures_xxr->A_AccelTrendBuf_S8X[3] == POSITIVE))
       {
         /* EasyCASE ( 101651
            Get the Stride Interval by adding the successive trends time counts */
         for (v_Index_u8r=0; v_Index_u8r<C_SINGLE_AXIS_TREND_BUFSIZE_U8R; v_Index_u8r++)
         {
            v_StrideInterval_u8r += p_AccelerationPatternFeatures_xxr->A_AccelSuccecciveTrendTimeBuf_U8X[v_Index_u8r];
         }
         /* EasyCASE ) */
         /*Check whether any discontinuity in the stride patterns by cheking Stride Interval value??*/
         if (( v_StrideInterval_u8r > 0 ) && ( v_StrideInterval_u8r < C_STRIDE_INTERVAL_COUNT_MAXLIMIT_U8R ))
         {
            /* EasyCASE ( 101659
               Set V_ValidStridePatternOccuredFlag_U8X flag and Clear A_AccelTrendBuf_S8X buffer */
            /* Set V_ValidStridePatternOccuredFlag_U8X If Stride Pattern don't have any discontinuity */
             p_AccelerationPatternFeatures_xxr->V_ValidStridePatternOccuredFlag_U8X=1;
            for (v_Index_u8r=1; v_Index_u8r<C_SINGLE_AXIS_TREND_BUFSIZE_U8R; v_Index_u8r++)
            {
               p_AccelerationPatternFeatures_xxr->A_AccelTrendBuf_S8X[v_Index_u8r] = 0;
            }
            /* EasyCASE ) */
            /* Check the Trend Peaks whether we are having uniform peaks or not?? */
            /* EasyCASE ( 101652
               Find the Stride Min, Max and PeakToPeak Values */
            v_StrideMinVal_s16r = p_AccelerationPatternFeatures_xxr->A_AccelTrendValueBuf_S16X[0];
            v_StrideMaxVal_s16r = p_AccelerationPatternFeatures_xxr->A_AccelTrendValueBuf_S16X[0];
            for (v_Index_u8r=1; v_Index_u8r < C_SINGLE_AXIS_TREND_BUFSIZE_U8R; v_Index_u8r++ /* Find Min-Max */)
            {
               if (p_AccelerationPatternFeatures_xxr->A_AccelTrendValueBuf_S16X[v_Index_u8r]<v_StrideMinVal_s16r)
               {
                  v_StrideMinVal_s16r = p_AccelerationPatternFeatures_xxr->A_AccelTrendValueBuf_S16X[v_Index_u8r];
               }
               else
               {
                  if (p_AccelerationPatternFeatures_xxr->A_AccelTrendValueBuf_S16X[v_Index_u8r]>v_StrideMaxVal_s16r)
                  {
                     v_StrideMaxVal_s16r = p_AccelerationPatternFeatures_xxr->A_AccelTrendValueBuf_S16X[v_Index_u8r];
                  }
               }
            }
            v_StridePeakToPeakVal_u16r = (v_StrideMaxVal_s16r-v_StrideMinVal_s16r);

            /* EasyCASE ) */
            /* EasyCASE ( 101658
               判断步幅是否均匀，C_STRIDE_SHORTPEAK_TO_PP_RATIO_CUTOFF_F32R原来为0.15，感觉太小，改为0.3 */
            if (((p_AccelerationPatternFeatures_xxr->A_AccelTrendValueBuf_S16X[0]-p_AccelerationPatternFeatures_xxr->A_AccelTrendValueBuf_S16X[1])> (C_STRIDE_SHORTPEAK_TO_PP_RATIO_CUTOFF_F32R*v_StridePeakToPeakVal_u16r))&&
                ((p_AccelerationPatternFeatures_xxr->A_AccelTrendValueBuf_S16X[2]-p_AccelerationPatternFeatures_xxr->A_AccelTrendValueBuf_S16X[1]) > (C_STRIDE_SHORTPEAK_TO_PP_RATIO_CUTOFF_F32R*v_StridePeakToPeakVal_u16r))&&
                ((p_AccelerationPatternFeatures_xxr->A_AccelTrendValueBuf_S16X[2]-p_AccelerationPatternFeatures_xxr->A_AccelTrendValueBuf_S16X[3]) > (C_STRIDE_SHORTPEAK_TO_PP_RATIO_CUTOFF_F32R*v_StridePeakToPeakVal_u16r)))
            {
               /*All three short stride peaks pass the minimum ratio test then this stride is uniform */
               v_InstStrideUniformity_u8r = 1;
            }
            else
            {
               v_InstStrideUniformity_u8r = 0;
            }
            /* EasyCASE ) */
            /* EasyCASE ( 101653
               Update the Stride Peak-To-Peak Value, Stride Interval, Stride Interval Diff, Stride Uniformity Test Result FIFOs */
            if (p_AccelerationPatternFeatures_xxr->V_StrideIntervalBufDataCount_U8X < C_STRIDE_INTERVAL_BUFSIZE_U8R)
            {
               p_AccelerationPatternFeatures_xxr->A_StridePeakToPeakValBuf_U16X[p_AccelerationPatternFeatures_xxr->V_StrideIntervalBufDataCount_U8X] = v_StridePeakToPeakVal_u16r;
               p_AccelerationPatternFeatures_xxr->A_StrideIntervalBuf_U8X[p_AccelerationPatternFeatures_xxr->V_StrideIntervalBufDataCount_U8X] = v_StrideInterval_u8r;
               p_AccelerationPatternFeatures_xxr->A_UniformityTestBuf_U8X[p_AccelerationPatternFeatures_xxr->V_StrideIntervalBufDataCount_U8X] = v_InstStrideUniformity_u8r;
               p_AccelerationPatternFeatures_xxr->V_StrideIntervalBufDataCount_U8X++;
               if (p_AccelerationPatternFeatures_xxr->V_StrideIntervalBufDataCount_U8X >1)
               {
                  p_AccelerationPatternFeatures_xxr->A_StrideIntervalDiffBuf_U8X[p_AccelerationPatternFeatures_xxr->V_StrideIntervalBufDataCount_U8X-2] =                                  
                  (p_AccelerationPatternFeatures_xxr->A_StrideIntervalBuf_U8X[p_AccelerationPatternFeatures_xxr->V_StrideIntervalBufDataCount_U8X-1]
                  -p_AccelerationPatternFeatures_xxr->A_StrideIntervalBuf_U8X[p_AccelerationPatternFeatures_xxr->V_StrideIntervalBufDataCount_U8X-2]);
               }
            }
            else
            {
               for (v_Index_u8r=0; v_Index_u8r<(C_STRIDE_INTERVAL_BUFSIZE_U8R-1); v_Index_u8r++)
               {
                  p_AccelerationPatternFeatures_xxr->A_StridePeakToPeakValBuf_U16X[C_STRIDE_INTERVAL_BUFSIZE_U8R-1-v_Index_u8r] = p_AccelerationPatternFeatures_xxr->A_StridePeakToPeakValBuf_U16X[C_STRIDE_INTERVAL_BUFSIZE_U8R-2-v_Index_u8r];
                  p_AccelerationPatternFeatures_xxr->A_StrideIntervalBuf_U8X[C_STRIDE_INTERVAL_BUFSIZE_U8R-1-v_Index_u8r]       = p_AccelerationPatternFeatures_xxr->A_StrideIntervalBuf_U8X[C_STRIDE_INTERVAL_BUFSIZE_U8R-2-v_Index_u8r];
                  p_AccelerationPatternFeatures_xxr->A_StrideIntervalDiffBuf_U8X[C_STRIDE_INTERVAL_BUFSIZE_U8R-1-v_Index_u8r]   = p_AccelerationPatternFeatures_xxr->A_StrideIntervalDiffBuf_U8X[C_STRIDE_INTERVAL_BUFSIZE_U8R-2-v_Index_u8r];
                  p_AccelerationPatternFeatures_xxr->A_UniformityTestBuf_U8X[C_STRIDE_INTERVAL_BUFSIZE_U8R-1-v_Index_u8r]       = p_AccelerationPatternFeatures_xxr->A_UniformityTestBuf_U8X[C_STRIDE_INTERVAL_BUFSIZE_U8R-2-v_Index_u8r];
               }
               p_AccelerationPatternFeatures_xxr->A_StridePeakToPeakValBuf_U16X[0] = v_StridePeakToPeakVal_u16r;
               p_AccelerationPatternFeatures_xxr->A_StrideIntervalBuf_U8X[0]       = v_StrideInterval_u8r;
               p_AccelerationPatternFeatures_xxr->A_StrideIntervalDiffBuf_U8X[0]   = (v_StrideInterval_u8r-p_AccelerationPatternFeatures_xxr->A_StrideIntervalBuf_U8X[1]);
               p_AccelerationPatternFeatures_xxr->A_UniformityTestBuf_U8X[0]       = v_InstStrideUniformity_u8r;
            }
            /* EasyCASE ) */
            /* EasyCASE ( 101654
               Update the Stable Cadence for this particular axis */
            for (v_Index_u8r=0; v_Index_u8r <(p_AccelerationPatternFeatures_xxr->V_StrideIntervalBufDataCount_U8X); v_Index_u8r++)
            {
               v_Temp_u16r += p_AccelerationPatternFeatures_xxr->A_StrideIntervalBuf_U8X[v_Index_u8r];
            }
            /* CadenceX100 = 100*(2*Strides/sec) = 100*(2*(1/StrideInterval))= 100*(2*(1/(0.040*AvgStrideIntervalCount)) = (5000/AvgStrideInterval) */
            p_AccelerationPatternFeatures_xxr->V_CadenceX100_U16X = (U16)((5000*(p_AccelerationPatternFeatures_xxr->V_StrideIntervalBufDataCount_U8X))/(F32)v_Temp_u16r);
            /* EasyCASE ) */
            /* AXIS STABILITY ??-->To decide this check the history of Uniformity Test and Stride Interval Diff Test*/
            /* EasyCASE ( 101655
               Check the History of Uniformity Test whether it is consistant?? */
            for (v_Index_u8r=0; v_Index_u8r<C_STRIDE_INTERVAL_BUFSIZE_U8R; v_Index_u8r++)
            {
               if (p_AccelerationPatternFeatures_xxr->A_UniformityTestBuf_U8X[v_Index_u8r]==1)
               {
                  v_TestCount_u8r++;
               }
            }
            if (v_TestCount_u8r>=C_AXIS_UNIFORMITY_PASSED_NO_CUTOFF_U8R /* Atleast 4 out of 5 tests should be passed */)
            {
               v_UniformityTestPassedFlag_u8r=1;
            }
            else
            {
               v_UniformityTestPassedFlag_u8r=0;
            }
            /* EasyCASE ) */
            /* EasyCASE ( 101656
               Check the History of Stride Interval Difference whether abrupt jumps in StrideInterval or not ?? */
            v_TestCount_u8r=0;
            for (v_Index_u8r=0; v_Index_u8r<C_STRIDE_INTERVAL_BUFSIZE_U8R; v_Index_u8r++)
            {
               if ((p_AccelerationPatternFeatures_xxr->A_StrideIntervalDiffBuf_U8X[v_Index_u8r])<C_STRIDE_INTERVAL_DIFF_MAX_LIMIT_U8R)
               {
                  v_TestCount_u8r++;
               }
            }
            if (v_TestCount_u8r>=C_AXIS_STRIDE_INTERVAL_PASSED_NO_CUTOFF_U8R /* Only two fluctuations are allowed in the Stride Interval Difference */)
            {
               v_StrideIntervalTestPassedFlag_u8r=1;
            }
            else
            {
               v_StrideIntervalTestPassedFlag_u8r=0;
            }
            /* EasyCASE ) */
            /* EasyCASE ( 101657
               Check whether Uniformity and Stride Interval both tests got passed ?? */
            if ((v_UniformityTestPassedFlag_u8r==1)&&(v_StrideIntervalTestPassedFlag_u8r==1))
            {
               p_AccelerationPatternFeatures_xxr->V_AxisIsStableFlag_U8X = 1;
            }
            else
            {
               p_AccelerationPatternFeatures_xxr->V_AxisIsStableFlag_U8X = 0;
            }
            /* EasyCASE ) */
         }
      }
      else
      {
         /* else of stride pattern check */
      }
   }
   else
   {
      /* else of IF (v_TrendChangeFoundFlag_u8r == 1) */
      /* Clear V_ValidStridePatternOccuredFlag_U8X flag, if valid stride is not detected */
      p_AccelerationPatternFeatures_xxr->V_ValidStridePatternOccuredFlag_U8X = 0;
   }
   /* EasyCASE ) */
}

void PEDO_Reset_SingleAxisAccelFeatures(ts_SingleAxis *p_AccelerationPatternFeatures_xxr)
{
   /* EasyCASE ( 101670
      Local Variable */
   U8 v_Index_u8r=0;
   /* EasyCASE ) */
   /* EasyCASE ( 101671
      Reset single axis acceleration features Buffers */
   for (v_Index_u8r=0; v_Index_u8r<C_SINGLE_AXIS_ACCEL_VALUE_BUFSIZE_U8R; v_Index_u8r++)
   {
      p_AccelerationPatternFeatures_xxr->A_AccelBuf_S16X[v_Index_u8r]=0;
   }
   for (v_Index_u8r=0; v_Index_u8r<C_SINGLE_AXIS_TREND_BUFSIZE_U8R; v_Index_u8r++)
   {
      p_AccelerationPatternFeatures_xxr->A_AccelTrendBuf_S8X[v_Index_u8r]=0;
      p_AccelerationPatternFeatures_xxr->A_AccelTrendValueBuf_S16X[v_Index_u8r]=0; 
      p_AccelerationPatternFeatures_xxr->A_AccelSuccecciveTrendTimeBuf_U8X[v_Index_u8r]=0;
   }
   for (v_Index_u8r=0; v_Index_u8r<C_STRIDE_INTERVAL_BUFSIZE_U8R; v_Index_u8r++)
   {
      p_AccelerationPatternFeatures_xxr->A_StridePeakToPeakValBuf_U16X[v_Index_u8r]=0;
      p_AccelerationPatternFeatures_xxr->A_StrideIntervalBuf_U8X[v_Index_u8r]=0;
      p_AccelerationPatternFeatures_xxr->A_StrideIntervalDiffBuf_U8X[v_Index_u8r]=0;
      p_AccelerationPatternFeatures_xxr->A_UniformityTestBuf_U8X[v_Index_u8r]=0;
   }
   /* EasyCASE ) */
   /* EasyCASE ( 101672
      Reset single axis acceleration features variables */
   p_AccelerationPatternFeatures_xxr->V_AccelBufDataCount_U8X=0;
   p_AccelerationPatternFeatures_xxr->V_AccelTrendBufDataCount_U8X=0;
   p_AccelerationPatternFeatures_xxr->V_StrideIntervalBufDataCount_U8X=0;
   p_AccelerationPatternFeatures_xxr->V_SuccessiveTrendTimeCount_U8X=0;
          
   p_AccelerationPatternFeatures_xxr->V_InstTrendType_S8X = NEGATIVE; /* Set default trend as Negetive Trend */
   p_AccelerationPatternFeatures_xxr->V_ValidStridePatternOccuredFlag_U8X=0;
   p_AccelerationPatternFeatures_xxr->V_SuccessiveTrendTimeValidityFlag_U8X=0;
   p_AccelerationPatternFeatures_xxr->V_AxisIsStableFlag_U8X=0;
   p_AccelerationPatternFeatures_xxr->V_CadenceX100_U16X=0;
   /* EasyCASE ) */
}


void PEDO_FindDominantAxisOrder( ts_SingleAxis *p_Xaxis_xxr, ts_SingleAxis *p_Yaxis_xxr, ts_SingleAxis *p_Zaxis_xxr,
                                 U8 v_Activity_u8r, U8 *p_IstDomAxis_u8r, U8 *p_IIndDomAxis_u8r, U8 *p_IIIrdDomAxis_u8r )
   {
   /* EasyCASE ( 101757
      Local Variables */
   /* EasyCASE < */
   U8  v_Index_u8r=0, v_Index2_u8r=0;
   U8  v_AxisBufLength_u8r=3;
   U8  v_TempU8_u8r=0;
   U8  v_XaxisDominantCount_u8r=0;
   U8  v_YaxisDominantCount_u8r=0;
   U8  v_ZaxisDominantCount_u8r=0;
   U8  v_IstDomAxisDecidedFlag_u8r=0;
   
   U8  a_AxisBuf_u8r[3];
   U16 a_AxisValBuf_u16r[3];
   U16 v_Temp_u16r=0;
   
   /* Reset the variable Again when ACTIVITY is STATIONARY */
   static U8 v_DomAxisBufPacketCount_u8r=0;   
   static U8 v_IstDomAxisBuf_u8r[C_DOMINATING_AXIS_BUFSIZE_U8R]  ={0}; 
   static U8 v_IIndDomAxisBuf_u8r[C_DOMINATING_AXIS_BUFSIZE_U8R] ={0};
   /* EasyCASE > */
   /* EasyCASE ) */
   /* EasyCASE ( 101761
      Clear the v_DomAxisBufPacketCount_u8r when ACTIVITY == STATIONARY */
   if (v_Activity_u8r == STATIONARY)
      {
      v_DomAxisBufPacketCount_u8r=0;
      }
   /* EasyCASE ) */
   /* EasyCASE ( 101762
      Assign values to AxisBuf ( 0=X_Axis; 1=Y_Axis; 2=Z_Axis ) and AxisValueBuf ( Stride Peak-to-Peak Value ) */
   a_AxisBuf_u8r[0] = C_Xaxis_U8X; /* X-Axis */
   a_AxisBuf_u8r[1] = C_Yaxis_U8X; /* Y-Axis */
   a_AxisBuf_u8r[2] = C_Zaxis_U8X; /* Z-Axis */
   
   a_AxisValBuf_u16r[0] = p_Xaxis_xxr->A_StridePeakToPeakValBuf_U16X[0]; /* X-Axis Stride Peak-to-Peak Value */
   a_AxisValBuf_u16r[1] = p_Yaxis_xxr->A_StridePeakToPeakValBuf_U16X[0]; /* Y-Axis Stride Peak-to-Peak Value */
   a_AxisValBuf_u16r[2] = p_Zaxis_xxr->A_StridePeakToPeakValBuf_U16X[0]; /* Z-Axis Stride Peak-to-Peak Value */
   /* EasyCASE ) */
   /* EasyCASE ( 101763
      Sort the Stride Peak-To-Peak values in Descending Order */
   for (v_Index_u8r = 0; v_Index_u8r < (v_AxisBufLength_u8r-1); v_Index_u8r++)
      {
      for (v_Index2_u8r = (v_Index_u8r+1); v_Index2_u8r < v_AxisBufLength_u8r; v_Index2_u8r++)
         {
         if (a_AxisValBuf_u16r[v_Index_u8r] < a_AxisValBuf_u16r[v_Index2_u8r])
            {
            /* Swap the Axis */
            v_TempU8_u8r = a_AxisBuf_u8r[v_Index_u8r];
            a_AxisBuf_u8r[v_Index_u8r] = a_AxisBuf_u8r[v_Index2_u8r];
            a_AxisBuf_u8r[v_Index2_u8r] = v_TempU8_u8r; 
            
            /* Swap the Axis Values */
            v_Temp_u16r = a_AxisValBuf_u16r[v_Index_u8r];
            a_AxisValBuf_u16r[v_Index_u8r] = a_AxisValBuf_u16r[v_Index2_u8r];
            a_AxisValBuf_u16r[v_Index2_u8r] = v_Temp_u16r;
            }
         else
            {
            if (a_AxisValBuf_u16r[v_Index_u8r] == a_AxisValBuf_u16r[v_Index2_u8r])
               {
               /* Axis with less index will be given more priority in the descending order */
               if (a_AxisBuf_u8r[v_Index_u8r] > a_AxisBuf_u8r[v_Index2_u8r])
                  {
                  /* Swap the Axis */
                  v_TempU8_u8r = a_AxisBuf_u8r[v_Index_u8r];
                  a_AxisBuf_u8r[v_Index_u8r] = a_AxisBuf_u8r[v_Index2_u8r];
                  a_AxisBuf_u8r[v_Index2_u8r] = v_TempU8_u8r; 
                  
                  /* Swap the Axis Values */
                  v_Temp_u16r = a_AxisValBuf_u16r[v_Index_u8r];
                  a_AxisValBuf_u16r[v_Index_u8r] = a_AxisValBuf_u16r[v_Index2_u8r];
                  a_AxisValBuf_u16r[v_Index2_u8r] = v_Temp_u16r;
                  }
               }
            }
         }
      }
   /* EasyCASE ) */
   if (v_DomAxisBufPacketCount_u8r < C_DOMINATING_AXIS_BUFSIZE_U8R)
      {
      /* EasyCASE ( 101775
         Keep default dominating axis as COMPOSITE AXIS for first 5 Strides,
         untill Ist and IInd dominating axis FIFOs are filled. */
      v_IstDomAxisBuf_u8r[v_DomAxisBufPacketCount_u8r]  = a_AxisBuf_u8r[0];
      v_IIndDomAxisBuf_u8r[v_DomAxisBufPacketCount_u8r] = a_AxisBuf_u8r[1];
      v_DomAxisBufPacketCount_u8r++;
      
      /* Keep default dominating axis as composite axis for Ist 5 Strides untill Ist and IInd dominating axis FIFOs are filled*/ 
      *p_IstDomAxis_u8r = C_CompositeAxis_U8X;
      *p_IIndDomAxis_u8r = C_CompositeAxis_U8X;
      *p_IIIrdDomAxis_u8r = C_CompositeAxis_U8X;
      /* EasyCASE ) */
      }
   else
      {
      /* EasyCASE ( 101776
         Find dominating axis order */
      /* EasyCASE ( 101766
         Update the Ist and IInd dominant axis FIFOs */
      for (v_Index_u8r=0; v_Index_u8r<(C_DOMINATING_AXIS_BUFSIZE_U8R-1); v_Index_u8r++)
         {
         v_IstDomAxisBuf_u8r[C_DOMINATING_AXIS_BUFSIZE_U8R-1-v_Index_u8r]   = v_IstDomAxisBuf_u8r[C_DOMINATING_AXIS_BUFSIZE_U8R-2-v_Index_u8r];
         v_IIndDomAxisBuf_u8r[C_DOMINATING_AXIS_BUFSIZE_U8R-1-v_Index_u8r]  = v_IIndDomAxisBuf_u8r[C_DOMINATING_AXIS_BUFSIZE_U8R-2-v_Index_u8r];
         }
      v_IstDomAxisBuf_u8r[0]  = a_AxisBuf_u8r[0];
      v_IIndDomAxisBuf_u8r[0] = a_AxisBuf_u8r[1];
      /* EasyCASE ) */
      /* Find The Stable Ist, IInd and IIIrd dominating axis ?? */
      /* EasyCASE ( 101767
         Count the no. of Ist dominant occurances for X, Y and Z-axis out of 5?? */
      for (v_Index_u8r = 0; v_Index_u8r < C_DOMINATING_AXIS_BUFSIZE_U8R; v_Index_u8r++)
         {
         if (v_IstDomAxisBuf_u8r[v_Index_u8r]==C_Xaxis_U8X)
            {
            v_XaxisDominantCount_u8r++;
            }
         else
            {
            if (v_IstDomAxisBuf_u8r[v_Index_u8r]==C_Yaxis_U8X)
               {
               v_YaxisDominantCount_u8r++;
               }
            else
               {
               if (v_IstDomAxisBuf_u8r[v_Index_u8r]==C_Zaxis_U8X)
                  {
                  v_ZaxisDominantCount_u8r++;
                  }
               }
            }
         }
      /* EasyCASE ) */
      /* EasyCASE ( 101769
         Decide Ist Dominating Axis ?? */
      if (v_XaxisDominantCount_u8r>=C_NO_DOMINANT_AXIS_OCC_MIN_LIMIT)
         {
         *p_IstDomAxis_u8r = C_Xaxis_U8X; 
          v_IstDomAxisDecidedFlag_u8r=1;
         }
      else
         {
         if (v_YaxisDominantCount_u8r>=C_NO_DOMINANT_AXIS_OCC_MIN_LIMIT)
            {
            *p_IstDomAxis_u8r = C_Yaxis_U8X; 
             v_IstDomAxisDecidedFlag_u8r=1;
            }
         else
            {
            if (v_ZaxisDominantCount_u8r>=C_NO_DOMINANT_AXIS_OCC_MIN_LIMIT)
               {
               *p_IstDomAxis_u8r = C_Zaxis_U8X; 
                v_IstDomAxisDecidedFlag_u8r=1;
               }
            }
         }
      /* EasyCASE ) */
      if (v_IstDomAxisDecidedFlag_u8r ==1)
         {
         /* EasyCASE ( 101771
            Count the no of IInd dominant occurances for X, Y and Z-axis out of 5?? */
         for (v_Index_u8r = 0; v_Index_u8r < C_DOMINATING_AXIS_BUFSIZE_U8R; v_Index_u8r++)
            {
            if (v_IIndDomAxisBuf_u8r[v_Index_u8r]==C_Xaxis_U8X)
               {
               v_XaxisDominantCount_u8r++;
               }
            else
               {
               if (v_IIndDomAxisBuf_u8r[v_Index_u8r]==C_Yaxis_U8X)
                  {
                  v_YaxisDominantCount_u8r++;
                  }
               else
                  {
                  if (v_IIndDomAxisBuf_u8r[v_Index_u8r]==C_Zaxis_U8X)
                     {
                     v_ZaxisDominantCount_u8r++;
                     }
                  }
               }
            }
         /* EasyCASE ) */
         /* Decide IInd and IIrd Dominating Axis ?? */
         if (*p_IstDomAxis_u8r == C_Xaxis_U8X)
            {
            /* EasyCASE ( 101772
               Decide IInd and IIIrd dominating axis among Y and Z axis ? */
            if (v_YaxisDominantCount_u8r >= v_ZaxisDominantCount_u8r)
               {
               *p_IIndDomAxis_u8r  = C_Yaxis_U8X;
               *p_IIIrdDomAxis_u8r = C_Zaxis_U8X;
               }
            else
               {
               *p_IIndDomAxis_u8r  = C_Zaxis_U8X;
               *p_IIIrdDomAxis_u8r = C_Yaxis_U8X;
               }
            /* EasyCASE ) */
            }
         else
            {
            if (*p_IstDomAxis_u8r == C_Yaxis_U8X)
               {
               /* EasyCASE ( 101773
                  Decide IInd and IIIrd dominating axis among X and Y axis ? */
               if (v_XaxisDominantCount_u8r >= v_ZaxisDominantCount_u8r)
                  {
                  *p_IIndDomAxis_u8r  = C_Xaxis_U8X;
                  *p_IIIrdDomAxis_u8r = C_Zaxis_U8X;
                  }
               else
                  {
                  *p_IIndDomAxis_u8r  = C_Zaxis_U8X;
                  *p_IIIrdDomAxis_u8r = C_Xaxis_U8X;
                  }
               /* EasyCASE ) */
               }
            else
               {
               if (*p_IstDomAxis_u8r == C_Zaxis_U8X)
                  {
                  /* EasyCASE ( 101774
                     Decide IInd and IIIrd dominating axis among X and Y axis ? */
                  if (v_XaxisDominantCount_u8r >= v_YaxisDominantCount_u8r)
                     {
                     *p_IIndDomAxis_u8r  = C_Xaxis_U8X;
                     *p_IIIrdDomAxis_u8r = C_Yaxis_U8X;
                     }
                  else
                     {
                     *p_IIndDomAxis_u8r  = C_Yaxis_U8X;
                     *p_IIIrdDomAxis_u8r = C_Xaxis_U8X;
                     }
                  /* EasyCASE ) */
                  }
               }
            }
         }
      /* EasyCASE ) */
      }
   }



 void PEDO_Find_StableSingleAxis_StableCadenceX100( ts_SingleAxis *p_Xaxis_xxr, ts_SingleAxis *p_Yaxis_xxr, ts_SingleAxis *p_Zaxis_xxr, ts_SingleAxis *p_Caxis_xxr,
                                                    U8 v_IstDomAxis_u8r, U8 v_IIndDomAxis_u8r, U8 v_Activity_u8r, U8 *p_StableAxis_u8r, U16 *p_StableCadenceX100_u16r )
   {
   /* EasyCASE ( 101748
      Local Variables */
   //   U8 v_Index_u8r=0;
      static U8 v_LocalStableAxis_u8r = C_CompositeAxis_U8X;
     // static U16 v_LocalStableCadenceX100_u16r=0;
   //   U16 v_Temp_u16r=0;
   /* EasyCASE ) */
   /* EasyCASE ( 101749
      Check whether activity is STATIONARY ? 
      IF-->YES, (1) Set Stable axis to Composite axis as default
                (2) Clear stable axis cadence */
   if (v_Activity_u8r==STATIONARY)
      {
      v_LocalStableAxis_u8r = C_CompositeAxis_U8X;
    //  v_LocalStableCadenceX100_u16r = 0;
      }
   /* EasyCASE ) */
   /* EasyCASE ( 101750
      Find Stable Axis */
   /* CASE1 : CURRENT STABLE AXIS = COMPOSITE AXIS && STATUS == UNSTABLE */
   if ((v_LocalStableAxis_u8r == C_CompositeAxis_U8X)&&(p_Caxis_xxr->V_AxisIsStableFlag_U8X == UNSTABLE))
      {
      /* EasyCASE ( 101751
         Check whether COMPOSITE_AXIS == UNSTABLE ? */
      /* Check Ist dominating axis stability */
      if ((v_IstDomAxis_u8r == C_Xaxis_U8X)&&(p_Xaxis_xxr->V_AxisIsStableFlag_U8X == STABLE))
         {
         v_LocalStableAxis_u8r = C_Xaxis_U8X;
         }
      else
         {
         if ((v_IstDomAxis_u8r == C_Yaxis_U8X)&&(p_Yaxis_xxr->V_AxisIsStableFlag_U8X == STABLE))
            {
            v_LocalStableAxis_u8r = C_Yaxis_U8X;
            }
         else
            {
            if ((v_IstDomAxis_u8r == C_Zaxis_U8X)&&(p_Zaxis_xxr->V_AxisIsStableFlag_U8X == STABLE))
               {
               v_LocalStableAxis_u8r = C_Zaxis_U8X;
               }
            else
               {
               /* Check IInd dominating axis stability */
               if ((v_IIndDomAxis_u8r == C_Xaxis_U8X)&&(p_Xaxis_xxr->V_AxisIsStableFlag_U8X == STABLE))
                  {
                  v_LocalStableAxis_u8r = C_Xaxis_U8X;
                  }
               else
                  {
                  if ((v_IIndDomAxis_u8r == C_Yaxis_U8X)&&(p_Yaxis_xxr->V_AxisIsStableFlag_U8X == STABLE))
                     {
                     v_LocalStableAxis_u8r = C_Yaxis_U8X;
                     }
                  else
                     {
                     if ((v_IIndDomAxis_u8r == C_Zaxis_U8X)&&(p_Zaxis_xxr->V_AxisIsStableFlag_U8X == STABLE))
                        {
                        v_LocalStableAxis_u8r = C_Zaxis_U8X;
                        }
                     else
                        {
                        /* Keep it Composite axis only */
                        /* EasyCASE - */
                        v_LocalStableAxis_u8r = C_CompositeAxis_U8X;
                        }
                     }
                  }
               }
            }
         }
      /* EasyCASE ) */
      }
   else
      {
      /* CASE2 : CURRENT STABLE AXIS = X-AXIS && STATUS == UNSTABLE */
      if ((v_LocalStableAxis_u8r == C_Xaxis_U8X)&&(p_Xaxis_xxr->V_AxisIsStableFlag_U8X == UNSTABLE))
         {
         /* EasyCASE ( 101752
            Check whether X_AXIS == UNSTABLE ? */
         /* Check Composite Axis stability */
         if (p_Caxis_xxr->V_AxisIsStableFlag_U8X == STABLE)
            {
            v_LocalStableAxis_u8r = C_CompositeAxis_U8X;
            }
         else
            {
            /* Check Ist dominant axis stability  */
            if ((v_IstDomAxis_u8r == C_Yaxis_U8X)&&(p_Yaxis_xxr->V_AxisIsStableFlag_U8X == STABLE))
               {
               v_LocalStableAxis_u8r = C_Yaxis_U8X;
               }
            else
               {
               if ((v_IstDomAxis_u8r == C_Zaxis_U8X)&&(p_Zaxis_xxr->V_AxisIsStableFlag_U8X == STABLE))
                  {
                  v_LocalStableAxis_u8r = C_Zaxis_U8X;
                  }
               else
                  {
                  /* Check IInd dominating axis stability */
                  if ((v_IIndDomAxis_u8r == C_Yaxis_U8X)&&(p_Yaxis_xxr->V_AxisIsStableFlag_U8X == STABLE))
                     {
                     v_LocalStableAxis_u8r = C_Yaxis_U8X;
                     }
                  else
                     {
                     if ((v_IIndDomAxis_u8r == C_Zaxis_U8X)&&(p_Zaxis_xxr->V_AxisIsStableFlag_U8X == STABLE))
                        {
                        v_LocalStableAxis_u8r = C_Zaxis_U8X;
                        }
                     else
                        {
                        /* Keep it Composite axis only */
                        /* EasyCASE - */
                        v_LocalStableAxis_u8r = C_CompositeAxis_U8X;
                        }
                     }
                  }
               }
            }
         /* EasyCASE ) */
         }
      else
         {
         /* CASE3 : CURRENT STABLE AXIS = Y-AXIS && STATUS == UNSTABLE */
         if ((v_LocalStableAxis_u8r == C_Yaxis_U8X)&&(p_Yaxis_xxr->V_AxisIsStableFlag_U8X == UNSTABLE))
            {
            /* EasyCASE ( 101753
               Check whether Y_AXIS == UNSTABLE ? */
            /* Check Composite Axis stability */
            if (p_Caxis_xxr->V_AxisIsStableFlag_U8X == STABLE)
               {
               v_LocalStableAxis_u8r = C_CompositeAxis_U8X;
               }
            else
               {
               /* Check Ist dominant axis stability  */
               if ((v_IstDomAxis_u8r == C_Xaxis_U8X)&&(p_Xaxis_xxr->V_AxisIsStableFlag_U8X == STABLE))
                  {
                  v_LocalStableAxis_u8r = C_Xaxis_U8X;
                  }
               else
                  {
                  if ((v_IstDomAxis_u8r == C_Zaxis_U8X)&&(p_Zaxis_xxr->V_AxisIsStableFlag_U8X == STABLE))
                     {
                     v_LocalStableAxis_u8r = C_Zaxis_U8X;
                     }
                  else
                     {
                     /* Check IInd dominating axis stability */
                     if ((v_IIndDomAxis_u8r == C_Xaxis_U8X)&&(p_Xaxis_xxr->V_AxisIsStableFlag_U8X == STABLE))
                        {
                        v_LocalStableAxis_u8r = C_Xaxis_U8X;
                        }
                     else
                        {
                        if ((v_IIndDomAxis_u8r == C_Zaxis_U8X)&&(p_Zaxis_xxr->V_AxisIsStableFlag_U8X == STABLE))
                           {
                           v_LocalStableAxis_u8r = C_Zaxis_U8X;
                           }
                        else
                           {
                           /* Keep it Composite axis only */
                           /* EasyCASE - */
                           v_LocalStableAxis_u8r = C_CompositeAxis_U8X;
                           }
                        }
                     }
                  }
               }
            /* EasyCASE ) */
            }
         else
            {
            /* CASE4 : CURRENT STABLE AXIS = Z-AXIS && STATUS == UNSTABLE */
            if ((v_LocalStableAxis_u8r == C_Zaxis_U8X)&&(p_Zaxis_xxr->V_AxisIsStableFlag_U8X == UNSTABLE))
               {
               /* EasyCASE ( 101754
                  Check whether Z_AXIS == UNSTABLE ? */
               /* Check Composite Axis stability */
               if (p_Caxis_xxr->V_AxisIsStableFlag_U8X == STABLE)
                  {
                  v_LocalStableAxis_u8r = C_CompositeAxis_U8X;
                  }
               else
                  {
                  /* Check Ist dominant axis stability  */
                  if ((v_IstDomAxis_u8r == C_Xaxis_U8X)&&(p_Xaxis_xxr->V_AxisIsStableFlag_U8X == STABLE))
                     {
                     v_LocalStableAxis_u8r = C_Xaxis_U8X;
                     }
                  else
                     {
                     if ((v_IstDomAxis_u8r == C_Yaxis_U8X)&&(p_Yaxis_xxr->V_AxisIsStableFlag_U8X == STABLE))
                        {
                        v_LocalStableAxis_u8r = C_Yaxis_U8X;
                        }
                     else
                        {
                        /* Check IInd dominating axis stability */
                        if ((v_IIndDomAxis_u8r == C_Xaxis_U8X)&&(p_Xaxis_xxr->V_AxisIsStableFlag_U8X == STABLE))
                           {
                           v_LocalStableAxis_u8r = C_Xaxis_U8X;
                           }
                        else
                           {
                           if ((v_IIndDomAxis_u8r == C_Yaxis_U8X)&&(p_Yaxis_xxr->V_AxisIsStableFlag_U8X == STABLE))
                              {
                              v_LocalStableAxis_u8r = C_Yaxis_U8X;
                              }
                           else
                              {
                              /* Keep it Composite axis only */
                              /* EasyCASE - */
                              v_LocalStableAxis_u8r = C_CompositeAxis_U8X;
                              }
                           }
                        }
                     }
                  }
               /* EasyCASE ) */
               }
            }
         }
      }
   /* Copy the local stable axis */
      *p_StableAxis_u8r = v_LocalStableAxis_u8r;
   /* EasyCASE ) */
   /* EasyCASE ( 101755
      Find the Stable Cadence */
   switch (v_LocalStableAxis_u8r)
      {
      case C_Xaxis_U8X:
         *p_StableCadenceX100_u16r = p_Xaxis_xxr->V_CadenceX100_U16X;
         break;
      case C_Yaxis_U8X:
         *p_StableCadenceX100_u16r = p_Yaxis_xxr->V_CadenceX100_U16X;
         break;
      case C_Zaxis_U8X:
         *p_StableCadenceX100_u16r = p_Zaxis_xxr->V_CadenceX100_U16X;
         break;
      default:
         *p_StableCadenceX100_u16r = p_Caxis_xxr->V_CadenceX100_U16X;
         break;
      }
   /* EasyCASE ) */
   }
