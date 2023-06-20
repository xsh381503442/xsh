#include <stddef.h>
#include "SMD_TC2.h"

uint32_t SystemCoreClock;
extern void (* const systemVectors[])(void);

void SystemInit (void)
{
	/* Reset clocks and appropriate CPU registers */
	POWERCON->TURBO = 1;
	SystemCoreClockUpdate();
	SCB->VTOR = (uint32_t) systemVectors;
}

void SystemCoreClockUpdate (void)
{
	if (POWERCON->TURBO)
		SystemCoreClock = __XTAL;
	else
		SystemCoreClock = __XTAL / 2;
}

