/*
 * Copyright (c) 2014 Wind River Systems, Inc.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Weak default assertion post-handler. The host test build and the firmware
 * build each provide their own strong definition; this default keeps the port
 * linkable when neither is present.
 */

#include <zephyr/sys/__assert.h>
#include <zephyr/toolchain.h>

__weak void z_assert_post(const char *file, unsigned int line)
{
	ARG_UNUSED(file);
	ARG_UNUSED(line);
}
