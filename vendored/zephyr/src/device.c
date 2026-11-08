/*
 * Copyright (c) 2015-2016 Intel Corporation.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Bare-metal device model runtime.
 * Derived from Zephyr v4.5.0: kernel/device.c
 */

#include <string.h>
#include <zephyr/device.h>
#include <zephyr/sys/iterable_sections.h>

#if defined(__ARMCOMPILER_VERSION)
extern char Image$$ER_DEVICE$$Base;
extern char Image$$ER_DEVICE$$Limit;
#define Z_DEVICE_START ((const struct device *)&Image$$ER_DEVICE$$Base)
#define Z_DEVICE_END ((const struct device *)&Image$$ER_DEVICE$$Limit)
#else
extern const struct device _device_list_start[];
extern const struct device _device_list_end[];
#define Z_DEVICE_START _device_list_start
#define Z_DEVICE_END _device_list_end
#endif

int do_device_init(const struct device *dev)
{
	int rc = 0;

	if (dev->ops.init != NULL) {
		rc = dev->ops.init(dev);
		if (rc != 0) {
			if (rc < 0) {
				rc = -rc;
			}
			if (rc > 255) {
				rc = 255;
			}
		}
		dev->state->init_res = (uint8_t)rc;
	}

	if (rc == 0) {
		dev->state->initialized = true;
	}

	return (rc == 0) ? 0 : -rc;
}

bool device_is_ready(const struct device *dev)
{
	return (dev != NULL) && (dev->state != NULL) && dev->state->initialized;
}

const struct device *device_get_binding(const char *name)
{
	const struct device *dev;

	if ((name == NULL) || (name[0] == '\0')) {
		return NULL;
	}

	for (dev = Z_DEVICE_START; dev < Z_DEVICE_END; dev++) {
		if (strcmp(name, dev->name) == 0) {
			return device_is_ready(dev) ? dev : NULL;
		}
	}

	return NULL;
}
