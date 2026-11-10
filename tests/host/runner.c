/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Unity test runner: boots the (host) device model, then runs all tests.
 */

#include <init.h>

#include "unity.h"

void test_init_order(void);
void test_device_model(void);
void test_heartbeat(void);
void test_do_device_init_success(void);
void test_do_device_init_failure(void);
void test_do_device_init_errno_clamp(void);
void test_do_device_init_no_init(void);
void test_device_init_traversal(void);
void test_device_init_continues_after_failure(void);
void test_failed_device_not_ready_sets_bit0(void);
void test_fault_word(void);

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
	RUN_TEST(test_do_device_init_success);
	RUN_TEST(test_do_device_init_failure);
	RUN_TEST(test_do_device_init_errno_clamp);
	RUN_TEST(test_do_device_init_no_init);
	RUN_TEST(test_device_init_traversal);
	RUN_TEST(test_device_init_continues_after_failure);
	RUN_TEST(test_failed_device_not_ready_sets_bit0);
	RUN_TEST(test_fault_word);
	return UNITY_END();
}
