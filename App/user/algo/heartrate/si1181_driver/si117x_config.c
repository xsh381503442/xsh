
#include <stdio.h>
#include <stdint.h>
#include "am_mcu_apollo.h"
#include "am_util_delay.h"
#include "types.h"
#include "si117x_functions.h"
#include "si117x_config.h"
//#include "si117x_auth.h"
#ifdef OHRLIB_PRESENT	
#include "Ohrdriver.h"
#endif
#include "drv_lsm6dsl.h"

//#include "rtt_log.h"
#include "SEGGER_RTT.h"


#define MOUDLE_INFO_ENABLED 							0
#define MOUDLE_DEBUG_ENABLED   					  0
#define MOUDLE_NAME                     "[SI117X_CONFIG]:"

#if DEBUG_ENABLED == 1 && MOUDLE_INFO_ENABLED == 1
#define INFO_PRINTF(...)                  SEGGER_RTT_printf(0,##__VA_ARGS__)       
#define INFO_HEXDUMP(p_data, len)        SEGGER_RTT_Write(0, (const char*)p_data, len);           
#else
#define INFO_PRINTF(...) 
#define INFO_HEXDUMP(p_data, len)             
#endif

#if DEBUG_ENABLED == 1 && MOUDLE_DEBUG_ENABLED == 1
#define DEBUG_PRINTF(...)                 SEGGER_RTT_printf(0,##__VA_ARGS__)      
#define DEBUG_HEXDUMP(p_data, len)        SEGGER_RTT_Write(0, (const char*)p_data, len);      
#else
#define DEBUG_PRINTF(...)   
#define DEBUG_HEXDUMP(p_data, len) 
#endif




uint16_t si1181_setcurrenterror;

#define SI117X_FIFO_LENGTH   (SI117X_SAMPLERATE*4/READREQUENCY)
#define RETVAL_CHECK(retv)   if (retval<0)return retval;  

static 	s16 lastfifocount = 0;	
static uint8_t si1181_rev;
//uint8_t Timer_vled_on = 0;

int16_t setOhrcurrent(uint8_t value)
{
	int16_t  retval=0;
	if(value==0)
	{
			retval = Si117xParamSet(PARAM_PPG1_LED1_CONFIG, 0);
			RETVAL_CHECK(retv);
			retval = Si117xParamSet(PARAM_PPG1_LED2_CONFIG, 0);		
	}
  else
	{
			retval = Si117xParamSet(PARAM_PPG1_LED1_CONFIG, BIT6 + (value&0x3f));
			RETVAL_CHECK(retv);
			retval = Si117xParamSet(PARAM_PPG1_LED2_CONFIG, BIT6 + (value&0x3f));
	}
	return retval;  
}


int16_t Si117x_checkstart(void)
{
  uint8_t data1; 

	INFO_PRINTF(MOUDLE_NAME"si1171 initialize!\r\n");

	data1= Si117xReadFromRegister(REG_PART_ID);

	if(data1 < 0x70 || data1 >0x85)return -99;	
		
	return 0 ;
}
/// start of si117x, 
///input : current , led current at start up
// return  0 - inital success
//         <0  - error
int16_t Si117x_Start(uint8_t current,uint8_t sportmode)
{

	int16_t  retval=0;
  uint8_t data1;

	si1181_setcurrenterror = 0;

	data1= Si117xReadFromRegister(REG_PART_ID);
	
	INFO_PRINTF(MOUDLE_NAME"sI1181 read id = %X\r\n", data1);
	
	if(data1 < 0x70 || data1 >0x85)return -99;	
	
	data1= Si117xReadFromRegister(REG_REV_ID);
  	
	if(data1>=2)
	{
		 si1181_rev = data1;
	}
	else return -98;	
		
	retval = Si117xSendCmd(1);//reset	
	RETVAL_CHECK(retval);
	
	am_util_delay_ms(10);
	
	Si117xWriteToRegister(REG_HW_KEY, 0x00); // Set HW_KEY to 0
	
	retval = Si117xReadFromRegister (REG_HW_KEY);
	
	if(retval!=0)return -2; // Read back HW_KEY to check
		
	retval = Si117xParamSet(PARAM_TASK_ENABLE, BIT0+BIT1);
	RETVAL_CHECK();
	
	//set Set 16Hz Measure Rate ((1000000/SI117X_SAMPLERATE)/ 50) = 1250 = 0X4E2
	#define FREQ_SET ((1000000/SI117X_SAMPLERATE)/ 50)
	retval = Si117xParamSet(PARAM_MEASRATE_H, HIGH_BYTE(FREQ_SET));
	RETVAL_CHECK();
	retval = Si117xParamSet(PARAM_MEASRATE_L, LOW_BYTE(FREQ_SET));	
	RETVAL_CHECK();
	retval = Si117xParamSet(PARAM_PPG_MEASCOUNT, 0x01);
	RETVAL_CHECK();

	if(current>0)
	{
		retval = Si117xParamSet(PARAM_PPG1_LED1_CONFIG, BIT6 + (current&0x3f));
		RETVAL_CHECK();
		retval = Si117xParamSet(PARAM_PPG1_LED2_CONFIG, BIT6 + (current&0x3f));
		RETVAL_CHECK();
	}
	
	/*  PPG_MODE
	VDIODE  -   BIT7:BIT6  0:0.25V  1:0.35V 2:0.45V 3:0.55V
  EXT_PD  -   BIT5:BIT4  0:internal PD  1:EXT_PD1   2:EXT_PD2
  PD_CFG  -   BIT3:BIT0  BIT0:PD1  BIT1:PD2:  BIT2:PD3  BIT3:PD4	
	*/
	retval = Si117xParamSet(PARAM_PPG1_MODE, 0x0f);
	RETVAL_CHECK();
	/*PPG_MEASCONFIG:   
	PPG_INTERRUPT_MODE  -  BIT5       0: wrsit on dectection, 1: wrist off detection
  PPG_RAW             -  BIT4       0: PPG pure             1: PPG raw
  ADC_AVG             -  BIT2:BIT0  ADC samples 1 - 1, 2 - 2, 3 - 4, 5 - 8
	*/
	retval = Si117xParamSet(PARAM_PPG1_MEASCONFIG, 5);   //2 ADC SAMPLES 
	RETVAL_CHECK();
	/*
	DECIM      BIT7:BIT6, 0:64 1:128 2:256 3:512
	ADC_RANGE  BIT5:BIT4, 0:3
	CLK_DIV    BIT3:BIT0  0 TO 12 ,CLK_ADC = Fadc / power(2,CLK_DIV)
	*/
	if (sportmode == 1)
	{
		retval = Si117xParamSet(PARAM_PPG1_ADCCONFIG, BIT7+BIT6+BIT5+BIT4);
	}
	else
	{
		retval = Si117xParamSet(PARAM_PPG1_ADCCONFIG, BIT6+BIT5+BIT4);
	}
	RETVAL_CHECK();
	
	retval = Si117xParamSet(PARAM_PPG2_MODE,1);
	RETVAL_CHECK();
	retval = Si117xParamSet(PARAM_PPG2_MEASCONFIG, 2+BIT4);   //2 ADC SAMPLES + ppg raw
	RETVAL_CHECK();
	retval = Si117xParamSet(PARAM_PPG2_ADCCONFIG, BIT4);// + BIT5+BIT4);
	RETVAL_CHECK();
	retval = Si117xParamSet(PARAM_FIFO_INT_LEVEL_H, HIGH_BYTE(SI117X_FIFO_LENGTH));
  RETVAL_CHECK();	
	retval = Si117xParamSet(PARAM_FIFO_INT_LEVEL_L, LOW_BYTE(SI117X_FIFO_LENGTH));	
	RETVAL_CHECK();
	/*  MEAS_CONTL
	FIFO_TEST      BIT7   1 - FIFO test enabled
	FIFO_DISABLE   BIT6   1 - FIFO disabled
	PPG_SW_AVG     BIT5:BIT3  OSR = power(2,PPG_SW_AVG)
  ECG_SZ         BIT2   1 - 3bytes,  0 - 2 bytes
  PPG_SZ         BIT0   1 - 3bytes,  0 - 2 bytes		
	*/	
	retval = Si117xParamSet(PARAM_MEAS_CNTL, 0); //2 bytes per PPG sample
	RETVAL_CHECK();
	
	retval = Si117xFlushFIFO();
	RETVAL_CHECK();
	
	/* REG_IRQ_ENABLE
	SAMPLE_INT_EN  BIT1
	FIFO_INT_EN    BIT0
	*/
	retval = Si117xWriteToRegister (REG_IRQ_ENABLE, BIT0); // Enable FIFO interrupt
	RETVAL_CHECK();
	
	lastfifocount = 0;
	
  retval = Si117xStart();
//	Timer_vled_on = 4;
	
	INFO_PRINTF(MOUDLE_NAME"sI1181 start!\r\n");
	return retval;
}

//================================================================
s16  clear1171int()
{
	uint8_t retval = Si117xGetIrqStatus(si1181_rev);
	Si117xWriteToRegister(REG_HOSTIN0, retval);
	return Si117xSendCmd(0xD);	//clear interrupt	
}


void Si117x_Stop(void)
{
	clear1171int();
	Si117xStop();
}
//==================================================================
void Si117xReadFifo(typ_fifodata * fifo)
{
	s16 bytesToRead = 0;
	s16 fifocount = 0;	
	
	fifocount = Si117xUpdateFifoCount(si1181_rev);
	  bytesToRead = fifocount - lastfifocount; 

	if(bytesToRead<0)bytesToRead+=2048;
	
	lastfifocount = fifocount;
	
	 if(bytesToRead==0)
 {
   fifo->datalen = 0;
  ///R_LOG("Si117x bytesToRead is 0");
   return;
 }
 
 if(bytesToRead>=256)
 {
  //R_LOG("Si117xFlushFIFO, data len = %d",bytesToRead);
  Si117xFlushFIFO();
  fifo->datalen = 0;
  
  return;
 }
 else
   Si117xBlockRead(REG_FIFO_READ, bytesToRead, fifo->data.bytes); 
#if 0


	
	if(bytesToRead==0 || bytesToRead>256)
	{
		Si117xFlushFIFO();
		//clear1171int();
		fifo->datalen = 0;
		DEBUG_PRINTF(MOUDLE_NAME"Si117xFlushFIFO\r\n");
		return;
	}
	else
			Si117xBlockRead(REG_FIFO_READ, bytesToRead, fifo->data.bytes);	
#endif
	fifo->datalen = bytesToRead;
	//clear1171int();
}

/**************************************************************************//**
 * @brief Main interrupt processing routine for Si117x.
 *****************************************************************************/
//#include "dev_acc_gyro.h"
#ifdef HRT_DEBUG_TEST
uint8_t acc_datalen;
uint8_t si11_datalen;
extern uint32_t hrt_int_count;
#endif
void HeartRateMonitor_interrupt_handle(void)
{
	drv_i2c_hw1_enable();
	clear1171int();
	
	typ_fifodata si1171_fifodata;	
	typ_fifodata acc_fifodata;

	Si117xReadFifo(&si1171_fifodata);//read si117x data from fifo
	drv_i2c_hw1_disable();
	
	////read acc data, 
	// NOTE: acc_fifodata.datalen SHOULD BE TOTAL BYTES OF DATA!!!!!! 
	acc_fifodata.datalen = 2*dev_hrt_acc_fifo_data(acc_fifodata.data.words);// gsensor的数据长度是字节数！
	
	//DEBUG_PRINTF(MOUDLE_NAME"read data ppg: %d, acc %d\r\n", si1171_fifodata.datalen,acc_fifodata.datalen);
	#ifdef HRT_DEBUG_TEST
	si11_datalen = si1171_fifodata.datalen;
	acc_datalen = acc_fifodata.datalen;
	hrt_int_count++;
	#endif
	#ifdef OHRLIB_PRESENT	
	if(OHRL_IrqDatatProcess(&si1171_fifodata, &acc_fifodata) != OHRL_CURRENT_NOTCHANGE)
	{
			drv_i2c_hw1_enable();
			if(setOhrcurrent(OHRL_GetLEDcurrent())<0)
			{
					 ++si1181_setcurrenterror;
					DEBUG_PRINTF(MOUDLE_NAME"s1181 set current error %d\r\n",si1181_setcurrenterror);
			}
			drv_i2c_hw1_disable();
	}	
	OHRL_algorithm();	
	#endif			
}
