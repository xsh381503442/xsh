#ifndef SI117X_CONFIG
#define SI117X_CONFIG
#include <stdint.h>
#include <stdbool.h>

#define OHRLIB_PRESENT   

#define SI117X_SAMPLERATE    16  
#define READREQUENCY         4 // read times per second, SHOULD BE 4,2,1 

/* start of si117x, 
///input : current , led current at start up
//          authkey, key to do authentication of si117x, it is initialized with OHRL_init, and changed after call Si117xAuthenticate, and the OHRL lib will verify it internally
//          set authkey to NULL to bypass authentication
// return  0 - inital success
//         <0  - error
*/
int16_t Si117x_Start(uint8_t current,uint8_t sportmode);

/**************************************************************************//**
 * @brief  interrupt processing routine for Si117x, call at INT pin interrupt 
 *****************************************************************************/
void HeartRateMonitor_interrupt_handle(void);

int16_t Si117x_checkstart(void);

void Si117x_Stop(void);
extern uint16_t si1181_setcurrenterror;

#endif
