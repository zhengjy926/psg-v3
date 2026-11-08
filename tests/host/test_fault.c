/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Verifies the fault word: set/clear/get of individual bits.
 */

#include "service/fault/fault.h"

#include <zephyr/sys/util.h>

#include "unity.h"

void test_fault_word(void)
{
	fault_clear(0xFFFFFFFFu);
	TEST_ASSERT_EQUAL_UINT32(0U, fault_get());

	fault_set(BIT(FAULT_BIT_NOT_READY));
	TEST_ASSERT_EQUAL_UINT32(BIT(FAULT_BIT_NOT_READY), fault_get());

	/* Setting an unrelated bit leaves bit0 intact. */
	fault_set(BIT(FAULT_BIT_OVERCURRENT_LATCH));
	TEST_ASSERT_EQUAL_UINT32(BIT(FAULT_BIT_NOT_READY) |
				     BIT(FAULT_BIT_OVERCURRENT_LATCH),
				 fault_get());

	fault_clear(BIT(FAULT_BIT_NOT_READY));
	TEST_ASSERT_EQUAL_UINT32(BIT(FAULT_BIT_OVERCURRENT_LATCH), fault_get());

	fault_clear(0xFFFFFFFFu);
	TEST_ASSERT_EQUAL_UINT32(0U, fault_get());
}
