/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * fault — see fault.h.
 */

#include "fault.h"

static volatile uint32_t fault_word;

void fault_set(uint32_t bits)
{
	fault_word |= bits;
}

void fault_clear(uint32_t bits)
{
	fault_word &= ~bits;
}

uint32_t fault_get(void)
{
	return fault_word;
}
