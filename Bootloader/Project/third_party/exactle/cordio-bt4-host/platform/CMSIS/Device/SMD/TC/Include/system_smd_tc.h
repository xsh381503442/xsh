#ifndef SYSTEM_smd_tc1_H
#define SYSTEM_smd_tc1_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

#define __XTAL (32000000UL) /* Oscillator Freq */
#define SystemClockApb (__XTAL / 2)

extern uint32_t SystemCoreClock;     /*!< System Clock Frequency (Core Clock)  */
    
/**
 * Initialize the system
 *
 * @param  none
 * @return none
 *
 * @brief  Setup the microcontroller system.
 *         Initialize the System and update the SystemCoreClock variable.
 */
extern void SystemInit (void);

/**
 * Update SystemCoreClock variable
 *
 * @param  none
 * @return none
 *
 * @brief  Updates the SystemCoreClock with current core Clock
 *         retrieved from cpu registers.
 */
extern void SystemCoreClockUpdate (void);

#ifdef __cplusplus
}
#endif

#endif /* SYSTEM_smd_tc1_H */
