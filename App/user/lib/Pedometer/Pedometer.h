
#ifndef __PEDOMETER_H__
#define __PEDOMETER_H__

#include <stdint.h>
#include <stdbool.h>
#include "define.h"
#include "stdint.h"

#define C_AXIS_IS_COMPOSITE_U8X     1  /**< Axis is Composite */
#define C_AXIS_IS_SINGLE_U8X        2  /**< Axis is Single */


#define C_SINGLE_AXIS_ACCEL_VALUE_BUFSIZE_U8R (U8)3 /**< Acceleration buffer Size */
#define C_SINGLE_AXIS_TREND_BUFSIZE_U8R       (U8)4 /**< Trend type buffer size (Keep it strictly 4) */
#define C_STRIDE_INTERVAL_BUFSIZE_U8R         (U8)5 /**< Stride interval buffer size */
#define C_STRIDE_INTERVAL_COUNT_MAXLIMIT_U8R  (U8)60 /**< Maximum limit for stride interval allowed */ 
//#define	C_STRIDE_INTERVAL_COUNT_MINLIMIT_U8R  (U8)
#define C_STRIDE_SHORTPEAK_TO_PP_RATIO_CUTOFF_F32R (F32)0.30 /**< Strides short peaks values to Peak-To-Peak values ratio */ //Ô­À´ÊÇ0.15

#define C_AXIS_UNIFORMITY_PASSED_NO_CUTOFF_U8R      (U8)4 /**< Minimum numbers of stride uniformity tests needs to be passed out of 5 tests */
#define C_AXIS_STRIDE_INTERVAL_PASSED_NO_CUTOFF_U8R (U8)3 /**< Minimum numbers of stride interval tests needs to be passed out of 5 tests */
#define C_STRIDE_INTERVAL_DIFF_MAX_LIMIT_U8R        (U8)5 /**< Maximum stride interval difference allowed b/w two successive strides */
#define C_DOMINATING_AXIS_BUFSIZE_U8R               (U8)5 /**< Ist and IInd dominant axis buffer size */
#define C_NO_DOMINANT_AXIS_OCC_MIN_LIMIT            (U8)4 /**< Minimum numbers of dominant occurences (out of 5) required for an axis to be dominant */
#define STABLE       (U8)1      /**< Indicated Stablility of an axis pattern */
#define UNSTABLE     (U8)0      /**< Indicated Unstablility of an axis pattern */

typedef struct 
{
  U8  V_PedoActivity_U8R;            /**< Pedometer Activity Type */
  U8  V_CadenceX10_U8R;              /**< Cadence (steps/sec) multiple of 10 */
  U16 V_StepLengthInCm_U16R;         /**< Step Length (in cm) */  
  U16 V_SpeedCmPerSec_U16R;          /**< Speed (in cm/sec) */
  
  U8  V_DeltaSteps_U8R;              /**< Delta Steps */
  U16 V_StepCountLastActivity_U16R;  /**< Steps for Last User Activity */
  U16 V_StepCountCurrActivity_U16R;  /**< Steps for Current User Activity */  
  U16 V_TotalStepCount_U16X;         /**< Total Step Count */

  F32 V_DeltaDistInMet_F32R;         /**< Delta Distance (In Meters) */
  U16 V_DistLastActivityInMet_U16R;  /**< Distance for Last User Activity (In Meters) */
  U16 V_DistCurrActivityInMet_U16R;  /**< Distance for Last User Activity (In Meters) */
  F32 V_TotalDistInMet_F32R;         /**< Total Distance (In Meters) */

}ts_PedoParam;

typedef struct
{
   /*FIFOs required for individual axis to save useful parameters for analysis */
   
       S16 A_AccelBuf_S16X[C_SINGLE_AXIS_ACCEL_VALUE_BUFSIZE_U8R];             /**< Buffer to hold the acceleration values */
       char  A_AccelTrendBuf_S8X[C_SINGLE_AXIS_TREND_BUFSIZE_U8R];               /**< Buffer to hold the trend type */
       S16 A_AccelTrendValueBuf_S16X[C_SINGLE_AXIS_TREND_BUFSIZE_U8R];         /**< Buffer to hold the accel values at trends */
       U8  A_AccelSuccecciveTrendTimeBuf_U8X[C_SINGLE_AXIS_TREND_BUFSIZE_U8R]; /**< Buffer to hold the time count between successive trends */

       U16 A_StridePeakToPeakValBuf_U16X[C_STRIDE_INTERVAL_BUFSIZE_U8R]; /**< Buffer to hold the stride peak-to-peak (Max val-Min val)*/
       U8  A_StrideIntervalBuf_U8X[C_STRIDE_INTERVAL_BUFSIZE_U8R];       /**< Buffer to hold the Stride Interval */
       char  A_StrideIntervalDiffBuf_U8X[C_STRIDE_INTERVAL_BUFSIZE_U8R];   /**< Buffer to hold the Stride Interval difference */
       U8  A_UniformityTestBuf_U8X[C_STRIDE_INTERVAL_BUFSIZE_U8R];       /**< Buffer to hold the result of uniformity test */

   /*Counts required for individual axis to update the FIFOs*/ 
   
       U8  V_AccelBufDataCount_U8X;             /**< Acceleration value buffer data packet count */
       U8  V_AccelTrendBufDataCount_U8X;        /**< Acceleration trend buffer data packet count*/
       U8  V_StrideIntervalBufDataCount_U8X;    /**< Stride Interval buffer data packet count */
       U8  V_SuccessiveTrendTimeCount_U8X;      /**< Time count between successive trends */

   /* Flags and other variables */   
       
       char  V_InstTrendType_S8X;                     /**< Instantaneous trend type */      
       U8  V_SuccessiveTrendTimeValidityFlag_U8X;   /**< Time count between successive trends validity flag */
       U8  V_ValidStridePatternOccuredFlag_U8X;     /**< Flag for valid stride patterns */
       U8  V_AxisIsStableFlag_U8X;                  /**< Axis stability flag */
       U16 V_CadenceX100_U16X;                      /**< single axis cadence(steps/sec) (in multiple of 100)*/
                   
}ts_SingleAxis;

typedef struct 
{
U16  InstStep_PP_Val;      /* Instantaneous step peak-to-peak value 
                                                     (Acceleration value difference at Negetive trend and Positive trend) */
U8   InstStep_PP_Time;     /* Instantaneous step peak-to-peak time
                                                     (Time count difference at Negetive trend and Positevee trend)*/
U16  InstStepRatio;        /* Instantaneous Activity detectin ratio */
U8   InstActivity;         /* Instantaneous Activity */

U16  InstCadence;          /* Instantaneous cadence (steps per sec) */
F32  LpfInstCadence;       /* LPF cadence (steps per sec)*/
F32  StableStepCadence;    /* Stable step cadence(steps/sec) */
F32  InstStepLength;       /* Instantaneous step length (in cm)*/
F32  InstSpeed;            /* Instantaneous Speed (cm/sec) */
F32  LpfSpeed;             /*  LPF Speed (cm/sec)*/

S16  RawCompAccel;             /* Raw composite acceleration */
S16  v_FiltAccel4Axis_s16r[4]; /* Filtered acceleration for all four axis */

U8   v_TrendDominantAxis_u8r;         /* Trend dominant axis */
U8   v_SingleOrCompositeAxisFlag_u8r; /* Single or Composite axis selection flag */

/*Added for axis switching */

U16 X_StrideRatio; /* Stride Ratio(Stride_PP_Val/(StrideInterval/2)) for X-Axis */
U16 Y_StrideRatio; /* Stride Ratio(Stride_PP_Val/(StrideInterval/2)) for Y-Axis */
U16 Z_StrideRatio; /* Stride Ratio(Stride_PP_Val/(StrideInterval/2)) for Z-Axis */
U16 C_StrideRatio; /* Stride Ratio(Stride_PP_Val/(StrideInterval/2)) for Composite-Axis */

U8 IstDomAxis;   /* First dominant axis */
U8 IIndDomAxis;  /* Second dominant axis */
U8 IIIrdDomAxis; /* Third dominant axis */

U8 X_Stability;  /* X-axis acceleration pattern stability flag */
U8 Y_Stability;  /* Y-axis acceleration pattern stability flag */
U8 Z_Stability;  /* Z-axis acceleration pattern stability flag */
U8 C_Stability;  /* Composite-axis acceleration pattern stability flag */
U8 CurrentStableAxis; /* Selected stable axis to find pedometer parameters */

U16 X_CadenceX100;  /* Stride based Cadence(Steps/sec) (multiple of 100) for X-axis */
U16 Y_CadenceX100;  /* Stride based Cadence(Steps/sec) (multiple of 100) for Y-axis */
U16 Z_CadenceX100;  /* Stride based Cadence(Steps/sec) (multiple of 100) for Z-axis */
U16 C_CadenceX100;  /* Stride based Cadence(Steps/sec) (multiple of 100) for C-axis */
U16 StableStrideCadenceX100; /* Stride based Cadence(Steps/sec) (multiple of 100) for stable axis */
U8 PedoActivity;             /* Pedometer Activity */

}ts_ExtraVar;


unsigned long GetStepCount(void);

bool GetSwimTouch(void);
void ClearSwimStatus( char status );
void SetSwimStatus( char  status );

void PEDO_StopDetection(void);


void PEDO_StartDetection(void);


void ResetStepCount(void);
void  SetStepCount( U16  Steps );

unsigned char GetActivity(void);


void InitAlgo(unsigned char v_GRange_u8r);

void PEDO_EnableRobustness(void);

void PEDO_SetInterStepCount(U8 v_InterStepCountLimit_u8r );

void PEDO_DisableRobustness(void);

U8 GetBandwidthSwitchInfo(void);

short ProcessAccelarationData(short v_RawAccelX_s16r, short v_RawAccelY_s16r, short v_RawAccelZ_s16r);

void  PEDO_UpdatePedometerParameters(ts_PedoParam* s_PedometerData_xxr);


 U32 PEDO_MedianFilter( U32 *p_InputArray_u32r, U8 v_SizeInputArray_u8r);

#endif
