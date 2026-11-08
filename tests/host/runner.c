/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Unity test runner: boots the (host) device model, then runs all tests.
 */

#include <zephyr/init.h>

#include "unity.h"

void test_init_order(void);
void test_device_model(void);
void test_heartbeat(void);

/* Unity fixtures (empty for now). */
void setUp(void)
{
}

void tearDown(void)
{
}

int main(void)
{
	/* Run every init level so devices (the fake gpio) and services are
	 * ready before the tests assert on them. */
	z_sys_init_run_all();

	UNITY_BEGIN();
	RUN_TEST(test_init_order);
	RUN_TEST(test_device_model);
	RUN_TEST(test_heartbeat);
	return UNITY_END();
}
