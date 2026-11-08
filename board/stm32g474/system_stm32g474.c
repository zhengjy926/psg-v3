/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Minimal system setup for STM32G474 (skeleton). The full PWR->regulator
 * boost->Flash wait-state->HSE->PLL->SYSCLK 170 MHz tree lands in the clock
 * tree ticket; here the device runs from the default HSI/MSI clock.
 */

#include "stm32g474.h"

uint32_t SystemCoreClock = 16000000UL;

void SystemInit(void)
{
	/* Skeleton: nothing to configure yet. */
}

void SystemCoreClockUpdate(void)
{
	SystemCoreClock = 16000000UL;
}
