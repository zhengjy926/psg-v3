/*
 * Copyright (c) 2025 STMicroelectronics
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef STM32_BITOPS_H_
#define STM32_BITOPS_H_

#include <stdint.h>

#include <toolchain/toolchain.h>

static ALWAYS_INLINE void stm32_reg_write(volatile uint32_t *addr, uint32_t value)
{
	*addr = value;
}

static ALWAYS_INLINE uint32_t stm32_reg_read(const volatile uint32_t *addr)
{
	return *addr;
}

static ALWAYS_INLINE void stm32_reg_set_bits(volatile uint32_t *addr, uint32_t mask)
{
	*addr |= mask;
}

static ALWAYS_INLINE void stm32_reg_clear_bits(volatile uint32_t *addr, uint32_t mask)
{
	*addr &= ~mask;
}

static ALWAYS_INLINE uint32_t stm32_reg_read_bits(const volatile uint32_t *addr, uint32_t mask)
{
	return *addr & mask;
}

static ALWAYS_INLINE void stm32_reg_modify_bits(volatile uint32_t *addr, uint32_t mask,
						uint32_t value)
{
	stm32_reg_write(addr, (stm32_reg_read(addr) & ~mask) | value);
}

#endif /* STM32_BITOPS_H_ */
