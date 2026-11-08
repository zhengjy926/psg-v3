/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * osal — minimal OS abstraction layer. Host-clean: must never include STM32 or
 * other MCU headers. The firmware maps this to SysTick + software timers + an
 * event queue (POST_KERNEL); the host test build drives time via osal_tick().
 */

#ifndef PSG_OSAL_OSAL_H_
#define PSG_OSAL_OSAL_H_

#include <stdint.h>

/** One-shot boot init (registered as a POST_KERNEL SYS_INIT). */
int osal_init(void);

/** Advance the millisecond clock by one tick (SysTick ISR / test clock). */
void osal_tick(void);

/** Monotonic millisecond counter since boot. */
uint32_t osal_uptime_ms(void);

#endif /* PSG_OSAL_OSAL_H_ */
