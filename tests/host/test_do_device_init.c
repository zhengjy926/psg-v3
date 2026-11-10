/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Unit tests for do_device_init(): success, failure (negative errno recorded
 * as positive init_res), errno clamping, and the no-init (already-ready) case.
 */

#include <device.h>

#include "unity.h"

/* do_device_init is an internal routine (upstream: kernel/device.c); declare it
 * locally like upstream kernel/init.c, not in device.h. */
extern int do_device_init(const struct device *dev);

static int init_ok(const struct device *dev)
{
	ARG_UNUSED(dev);
	return 0;
}

static int init_fail_eio(const struct device *dev)
{
	ARG_UNUSED(dev);
	return -5; /* -EIO */
}

static int init_fail_big(const struct device *dev)
{
	ARG_UNUSED(dev);
	return -300; /* exceeds the 8-bit init_res range */
}

void test_do_device_init_success(void)
{
	struct device_state st = {0};
	struct device dev = { .ops = { .init = init_ok }, .state = &st };

	TEST_ASSERT_EQUAL_INT(0, do_device_init(&dev));
	TEST_ASSERT_TRUE(device_is_ready(&dev));
	TEST_ASSERT_EQUAL_UINT8(0, st.init_res);
}

void test_do_device_init_failure(void)
{
	struct device_state st = {0};
	struct device dev = { .ops = { .init = init_fail_eio }, .state = &st };

	TEST_ASSERT_EQUAL_INT(-5, do_device_init(&dev));
	TEST_ASSERT_FALSE(device_is_ready(&dev));
	TEST_ASSERT_EQUAL_UINT8(5, st.init_res);
}

void test_do_device_init_errno_clamp(void)
{
	struct device_state st = {0};
	struct device dev = { .ops = { .init = init_fail_big }, .state = &st };

	TEST_ASSERT_EQUAL_INT(-255, do_device_init(&dev));
	TEST_ASSERT_FALSE(device_is_ready(&dev));
	TEST_ASSERT_EQUAL_UINT8(255, st.init_res);
}

void test_do_device_init_no_init(void)
{
	struct device_state st = {0};
	struct device dev = { .ops = { .init = NULL }, .state = &st };

	TEST_ASSERT_EQUAL_INT(0, do_device_init(&dev));
	TEST_ASSERT_TRUE(device_is_ready(&dev));
}
