/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Verifies the init-section traversal for device entries: devices run in
 * level+priority order, a failing device's init does not stop later entries
 * (continue-boot), and a failed device is not ready (device_get_binding()
 * returns NULL), which is what sets fault word bit0.
 */

#include <zephyr/device.h>
#include <zephyr/init.h>

#include "service/fault/fault.h"
#include "unity.h"

static int order[8];
static int order_n;

static int dev_init_prio5(const struct device *dev)
{
	ARG_UNUSED(dev);
	order[order_n++] = 5;
	return 0;
}

static int dev_init_prio25(const struct device *dev)
{
	ARG_UNUSED(dev);
	order[order_n++] = 25;
	return 0;
}

static int dev_init_prio15(const struct device *dev)
{
	ARG_UNUSED(dev);
	order[order_n++] = 15;
	return 0;
}

/* Declared out of priority order in the source; the linker must sort them. */
DEVICE_DEFINE(trav_a, "trav_a", dev_init_prio25, NULL, NULL, NULL,
	      PRE_KERNEL_2, 25, NULL);
DEVICE_DEFINE(trav_b, "trav_b", dev_init_prio5, NULL, NULL, NULL,
	      PRE_KERNEL_2, 5, NULL);
DEVICE_DEFINE(trav_c, "trav_c", dev_init_prio15, NULL, NULL, NULL,
	      PRE_KERNEL_2, 15, NULL);

void test_device_init_traversal(void)
{
	order_n = 0;
	z_sys_init_run_level(Z_INIT_ORD_PRE_KERNEL_2);

	TEST_ASSERT_EQUAL_INT(3, order_n);
	TEST_ASSERT_EQUAL_INT(5, order[0]);
	TEST_ASSERT_EQUAL_INT(15, order[1]);
	TEST_ASSERT_EQUAL_INT(25, order[2]);
}

/* --- continue-boot after a device init failure --- */

static int dev_init_fail(const struct device *dev)
{
	ARG_UNUSED(dev);
	order[order_n++] = -1;
	return -5;
}

static int dev_init_after(const struct device *dev)
{
	ARG_UNUSED(dev);
	order[order_n++] = 99;
	return 0;
}

/* PRE_KERNEL_1 on host: these two plus the fake gpio (which records nothing). */
DEVICE_DEFINE(trav_fail, "trav_fail", dev_init_fail, NULL, NULL, NULL,
	      PRE_KERNEL_1, 1, NULL);
DEVICE_DEFINE(trav_after, "trav_after", dev_init_after, NULL, NULL, NULL,
	      PRE_KERNEL_1, 2, NULL);

void test_device_init_continues_after_failure(void)
{
	order_n = 0;
	z_sys_init_run_level(Z_INIT_ORD_PRE_KERNEL_1);

	/* The failing device ran first, then the next device still ran. */
	TEST_ASSERT_EQUAL_INT(2, order_n);
	TEST_ASSERT_EQUAL_INT(-1, order[0]);
	TEST_ASSERT_EQUAL_INT(99, order[1]);
}

void test_failed_device_not_ready_sets_bit0(void)
{
	/* A failed device is not ready, so device_get_binding() returns NULL
	 * for it; the device after it is ready and is returned. This is the
	 * predicate the composition root uses to set fault word bit0. */
	TEST_ASSERT_NULL(device_get_binding("trav_fail"));
	TEST_ASSERT_NOT_NULL(device_get_binding("trav_after"));

	fault_clear(0xFFFFFFFFu);
	if (device_get_binding("trav_fail") == NULL) {
		fault_set(BIT(FAULT_BIT_NOT_READY));
	}
	TEST_ASSERT_EQUAL_UINT32(BIT(FAULT_BIT_NOT_READY), fault_get());
	fault_clear(0xFFFFFFFFu);
}
