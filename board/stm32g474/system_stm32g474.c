/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Minimal system setup for STM32G474. SystemInit() runs from Reset_Handler and
 * only performs the Flash prefetch/ART baseline (FPU enable lives in the
 * startup assembly). The PLL -> 170 MHz tree runs later in the clock_control
 * driver at PRE_KERNEL_1 (#5: CMSIS SystemInit stays out of the init levels).
 */

#include "stm32g474.h"

/* Reset clock is HSI16; the clock_control driver overwrites this to 170 MHz. */
uint32_t SystemCoreClock = 16000000UL;

void SystemInit(void)
{
	/* Flash prefetch + instruction/data cache baseline. Wait states are
	 * raised by the clock_control driver before switching to 170 MHz. */
	FLASH->ACR |= FLASH_ACR_PRFTEN_Msk | FLASH_ACR_ICEN_Msk |
		      FLASH_ACR_DCEN_Msk;
}

void SystemCoreClockUpdate(void)
{
	if ((RCC->CFGR & RCC_CFGR_SWS_Msk) == RCC_CFGR_SWS_PLL) {
		SystemCoreClock = 170000000UL;
	} else {
		SystemCoreClock = 16000000UL;
	}
}
