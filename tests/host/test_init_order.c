/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Verifies that SYS_INIT entries run in level+priority order. Three entries are
 * registered at PRE_KERNEL_2 with out-of-order priorities in the source; the
 * linker must place them (10, 20, 30) so they execute in that order.
 */

#include <zephyr/init.h>

#include "unity.h"

static int order[8];
static int order_n;

static int rec_prio10(void)
{
	order[order_n++] = 10;
	return 0;
}

static int rec_prio30(void)
{
	order[order_n++] = 30;
	return 0;
}

static int rec_prio20(void)
{
	order[order_n++] = 20;
	return 0;
}

SYS_INIT(rec_prio30, PRE_KERNEL_2, 30);
SYS_INIT(rec_prio10, PRE_KERNEL_2, 10);
SYS_INIT(rec_prio20, PRE_KERNEL_2, 20);

void test_init_order(void)
{
	order_n = 0;
	z_sys_init_run_level(Z_INIT_ORD_PRE_KERNEL_2);

	TEST_ASSERT_EQUAL_INT(3, order_n);
	TEST_ASSERT_EQUAL_INT(10, order[0]);
	TEST_ASSERT_EQUAL_INT(20, order[1]);
	TEST_ASSERT_EQUAL_INT(30, order[2]);
}
