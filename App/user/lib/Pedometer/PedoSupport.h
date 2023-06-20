
#ifndef __PEDO_SUPPORT_H__
#define __PEDO_SUPPORT_H__

#include "define.h"

#define INTERNAL_FIFO_MAX_SIZE  20 /**< Internal FIFO buffer size */
#define EXTERNAL_FIFO_MAX_SIZE  20 /**< External FIFO buffer size */
#define FIFO_START_INDEX        0  /**< FIFO start Index */

typedef struct LOW_PASS_FILTER
{
     F32 V_FilterValue_F32R; /**< Filtered value */
     U16 V_CurSample_U16R;   /**< Current Sample */
     F32 V_ALPHA_F32R;       /**< Value of ALPHA for LPF */ 

}ts_LPF;

typedef struct SMOOTH_FILTER
{
        U16 V_FilteredValue_U16R; /**< Filtered Value */
        U16 V_NewSample_U16R;     /**< New Sample */
        U8  V_FilterLength_U8R;   /**< Smooth Buffer Size */
        U8  V_FilterIndex_U8R;    /**< Smooth Buffer Index */
        U16 A_SmoothBuffer_U16R[20]; /**< Smooth Buffer array */
        
}ts_SmoothFilter;

void ShiftFIFO_U8( U8*, U8, U8 );

void SetInternalBuffer( U8, U8, U8 );

void GetInternalBuffer( U8*, U8*, U8* );

 void ResetInternalFIFO(void);

 U8 GetInternalFIFOSize(void);

void SetExternalBuffer(U8*, U8*, U8*, U8 );

void GetExternalBuffer( U8*, U8*, U8* );

 void ResetExternalFIFO(void);


 U8 GetExternalFIFOSize(void);


 void ProcessLPF(ts_LPF *p_ObjLPF);


 void GetSmoothValue(ts_SmoothFilter *p_SmoothFilter_xxr);

#endif

