
#include "define.h"
#include "PedoSupport.h"

U8 A_ActivityInternalBuffer_U8R[INTERNAL_FIFO_MAX_SIZE];         /**< Internal Buffer to hold Instantaneous Activity */
U8 A_Peak2PeakCountInternalBuffer_U8R[INTERNAL_FIFO_MAX_SIZE];   /**< Internal Buffer to hold Peak-To-Peak Time Counts */
U8 A_StepToStepCountInternalBuffer_U8R[INTERNAL_FIFO_MAX_SIZE];  /**< Internal Buffer to hold Step-To-Step Time Counts */

U8 A_ActivityExternalBuffer_U8R[EXTERNAL_FIFO_MAX_SIZE];         /**< External Buffer to hold Instantaneous Activity */
U8 A_Peak2PeakCountExternalBuffer_U8R[EXTERNAL_FIFO_MAX_SIZE];   /**< External Buffer to hold Peak-To-Peak Time Counts */
U8 A_StepToStepCountExternalBuffer_U8R[EXTERNAL_FIFO_MAX_SIZE];  /**< External Buffer to hold Step-To-Step Time Counts */

U8 V_InternalFIFOSize_U8R = 0;  /**<  FIFO size for Internal Buffer */
U8 V_ExternalFIFOSize_U8R = 0;  /**<  FIFO size for External Buffer */


void ShiftFIFO_U8(U8 *p_InputBuffer_u8r, U8 v_FifoSize_u8r, U8 v_ShiftCount_u8r)
   {
   /* EasyCASE ( 101013
      Local Variables */
   U8 v_Index_u8r = 0;
   /* EasyCASE ) */
   /* EasyCASE ( 101014
      Shift FIFO values */
   for (v_Index_u8r = 0; v_Index_u8r < (v_FifoSize_u8r-v_ShiftCount_u8r); v_Index_u8r++)
      {
      p_InputBuffer_u8r[v_FifoSize_u8r-v_Index_u8r-1] = p_InputBuffer_u8r[v_FifoSize_u8r-v_Index_u8r-1-v_ShiftCount_u8r];
      }
   /* EasyCASE ) */
   }


void SetInternalBuffer(U8 v_StepToStepCount_u8r, U8 v_Peak2PeakCount_u8r, U8 v_Activity_u8r)
   {
   /* EasyCASE ( 101016
      Push the current Step-To-Step, Peak-To-Peak time counts and instantaneous activity values into internal FIFOs. */
   ShiftFIFO_U8( A_StepToStepCountInternalBuffer_U8R,INTERNAL_FIFO_MAX_SIZE,1 );
   ShiftFIFO_U8( A_Peak2PeakCountInternalBuffer_U8R,INTERNAL_FIFO_MAX_SIZE,1 );
   ShiftFIFO_U8( A_ActivityInternalBuffer_U8R,INTERNAL_FIFO_MAX_SIZE,1 );
   
   A_StepToStepCountInternalBuffer_U8R[FIFO_START_INDEX] = v_StepToStepCount_u8r;
   A_Peak2PeakCountInternalBuffer_U8R[FIFO_START_INDEX] = v_Peak2PeakCount_u8r;
   A_ActivityInternalBuffer_U8R[FIFO_START_INDEX] = v_Activity_u8r;
   /* EasyCASE ) */
   /* EasyCASE ( 101015
      Increment the Internal FIFO data size counter */
   if (V_InternalFIFOSize_U8R < INTERNAL_FIFO_MAX_SIZE)
      {
      V_InternalFIFOSize_U8R++;
      }
   /* EasyCASE ) */
   }


void GetInternalBuffer(U8 *p_StepToStepCount_u8r, U8 *p_Peak2PeakCount_u8r, U8 *p_Activity_u8r)
   {
   /* EasyCASE ( 101017
      Local Variables */
   U8 v_Index_u8r;
   /* EasyCASE ) */
   /* EasyCASE ( 101018
      Copy the values from internal buffer to the external buffer */
   for (v_Index_u8r = 0; v_Index_u8r < V_InternalFIFOSize_U8R; v_Index_u8r++)
      {
      *(p_StepToStepCount_u8r + v_Index_u8r) = A_StepToStepCountInternalBuffer_U8R[v_Index_u8r];
      *(p_Peak2PeakCount_u8r + v_Index_u8r) = A_Peak2PeakCountInternalBuffer_U8R[v_Index_u8r];
      *(p_Activity_u8r + v_Index_u8r) = A_ActivityInternalBuffer_U8R[v_Index_u8r];
      }
   /* EasyCASE ) */
   /* EasyCASE ( 101019
      Reset the internal buffer data size counter variable */
   ResetInternalFIFO();
   /* EasyCASE ) */
   }


 void ResetInternalFIFO(void)
   {
   /* EasyCASE ( 101020
      Clear the Internal FIFO data size counter */
   V_InternalFIFOSize_U8R = 0;
   /* EasyCASE ) */
   }


 U8 GetInternalFIFOSize(void)
   {
   /* EasyCASE ( 101021
      Return Internal FIFO size */
   return V_InternalFIFOSize_U8R;
   /* EasyCASE ) */
   }


void SetExternalBuffer(U8 *p_StepToStepCount_u8r, U8 *p_Peak2PeakCount_u8r, U8 *p_Activity_u8r, U8 v_BufferLength_u8r)
   {
   /* EasyCASE ( 101022
      Local Variables */
   U8 v_Index_u8r = 0;
   /* EasyCASE ) */
   /* EasyCASE ( 101023
      Shift the Step-To-Step, Peak-To-Peak time counts and instantaneous activity values FIFOs */
   ShiftFIFO_U8(A_StepToStepCountExternalBuffer_U8R,EXTERNAL_FIFO_MAX_SIZE,v_BufferLength_u8r );
   ShiftFIFO_U8(A_Peak2PeakCountExternalBuffer_U8R,EXTERNAL_FIFO_MAX_SIZE,v_BufferLength_u8r );
   ShiftFIFO_U8(A_ActivityExternalBuffer_U8R,EXTERNAL_FIFO_MAX_SIZE,v_BufferLength_u8r );
   /* EasyCASE ) */
   /* EasyCASE ( 101024
      Copy the new values into External FIFOs */
   for (v_Index_u8r = 0; v_Index_u8r < v_BufferLength_u8r; v_Index_u8r++)
      {
      A_StepToStepCountExternalBuffer_U8R[v_Index_u8r] = *(p_StepToStepCount_u8r+v_Index_u8r);
      A_Peak2PeakCountExternalBuffer_U8R[v_Index_u8r] = *(p_Peak2PeakCount_u8r+v_Index_u8r);
      A_ActivityExternalBuffer_U8R[v_Index_u8r] = *(p_Activity_u8r+v_Index_u8r);
      /* EasyCASE - */
      /* Increment the External FIFO data size counter  */
      if (V_ExternalFIFOSize_U8R < EXTERNAL_FIFO_MAX_SIZE)
         {
         V_ExternalFIFOSize_U8R++;
         }
      }
   /* EasyCASE ) */
   }


 void GetExternalBuffer(U8 *p_StepToStepCount_u8r, U8 *p_Peak2PeakCount_u8r, U8 *p_Activity_u8r)
   {
   /* EasyCASE ( 101025
      Local variables */
   U8 v_Index_u8r;
   /* EasyCASE ) */
   /* EasyCASE ( 101026
      Copy the values from external buffer */
   for (v_Index_u8r = 0; v_Index_u8r < V_ExternalFIFOSize_U8R; v_Index_u8r++)
      {
      *(p_StepToStepCount_u8r + v_Index_u8r) = A_StepToStepCountExternalBuffer_U8R[v_Index_u8r];
      *(p_Activity_u8r + v_Index_u8r) = A_ActivityExternalBuffer_U8R[v_Index_u8r];          
      *(p_Peak2PeakCount_u8r + v_Index_u8r) = A_Peak2PeakCountExternalBuffer_U8R[v_Index_u8r];
      }
   /* EasyCASE ) */
   /* EasyCASE ( 101027
      Reset the external buffer data size counter variable */
   ResetExternalFIFO();
   /* EasyCASE ) */
   }


 void ResetExternalFIFO(void)
   {
   /* EasyCASE ( 101028
      Clear the External FIFO data size counter */
   V_ExternalFIFOSize_U8R = 0;
   /* EasyCASE ) */
   }


 U8 GetExternalFIFOSize(void)
   {
   /* EasyCASE ( 101029
      Return external buffer size */
   return V_ExternalFIFOSize_U8R;
   /* EasyCASE ) */
   }


 void ProcessLPF(ts_LPF *p_ObjLPF)
   {
   /* EasyCASE ( 101030
      Calculate LPF value by using current sample value */
   p_ObjLPF->V_FilterValue_F32R =  (float)p_ObjLPF->V_CurSample_U16R * p_ObjLPF->V_ALPHA_F32R + (float)p_ObjLPF->V_FilterValue_F32R * (1 - p_ObjLPF->V_ALPHA_F32R);
   /* EasyCASE ) */
   }


 void GetSmoothValue(ts_SmoothFilter *p_SmoothFilter_xxr)
   {
   /* EasyCASE ( 101031
      Local Variables */
   /* Local Variable */
      U8 v_Index_u8r = 0;
      U32 v_TempSum_u32r = 0;
   /* EasyCASE ) */
   /* EasyCASE ( 101032
      Copy the new sample values into Smooth Buffer */
   p_SmoothFilter_xxr->A_SmoothBuffer_U16R[p_SmoothFilter_xxr->V_FilterIndex_U8R] = p_SmoothFilter_xxr->V_NewSample_U16R;
   /* EasyCASE ) */
   /* EasyCASE ( 101033
      Check whether Filter Index has crossed the Filter Length or not ?
      IF-->YES, reset the Filter Index. */
   if (p_SmoothFilter_xxr->V_FilterIndex_U8R >= (p_SmoothFilter_xxr->V_FilterLength_U8R-1))
      {
      p_SmoothFilter_xxr->V_FilterIndex_U8R = 0;
      }
   else
      {
      p_SmoothFilter_xxr->V_FilterIndex_U8R ++;
      }
   /* EasyCASE ) */
   /* EasyCASE ( 101034
      Take the moving average of the values of the Filter Buffer. */
   for (v_Index_u8r = 0; v_Index_u8r < p_SmoothFilter_xxr->V_FilterLength_U8R; v_Index_u8r++)
      {
      v_TempSum_u32r += p_SmoothFilter_xxr->A_SmoothBuffer_U16R[v_Index_u8r];
      }
   p_SmoothFilter_xxr->V_FilteredValue_U16R = (U16)(v_TempSum_u32r/p_SmoothFilter_xxr->V_FilterLength_U8R);
   /* EasyCASE ) */
   }

