/*
 * Copyright (c) 2015-2016 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stddef.h>
#include <string.h>

#include <device.h>
#include <iterable_sections.h>

#if defined(__ARMCC_VERSION)
extern struct device device_list_start[] __asm("Image$$ER_DEVICE$$Base");
extern struct device device_list_end[] __asm("Image$$ER_DEVICE$$Limit");
#define DEVICE_SECTION_START device_list_start
#define DEVICE_SECTION_END device_list_end
#else
STRUCT_SECTION_START_EXTERN(device);
STRUCT_SECTION_END_EXTERN(device);
#define DEVICE_SECTION_START STRUCT_SECTION_START(device)
#define DEVICE_SECTION_END STRUCT_SECTION_END(device)
#endif

int do_device_init(const struct device *dev)
{
	int rc = 0;

	if (dev->ops.init != NULL) {
		rc = dev->ops.init(dev);
		/* If initialization failed, record in dev->state->init_res
		 * the POSITIVE value of the resulting errno
		 */
		if (rc != 0) {
			/* device's init function should return:
			 *   0 on success
			 *   a negative value on failure (-errno)
			 * errno value maps to an uint8_t range as of now.
			 */
			__ASSERT(rc >= -UINT8_MAX && rc < 0, "device %s init: invalid error (%d)",
				 dev->name, rc);

			if (rc < 0) {
				rc = -rc;
			}
			/* handle error value overflow in production
			 * this is likely a bug in the device's init function. Signals it
			 */
			if (rc > UINT8_MAX) {
				rc = UINT8_MAX;
			}
		}

		/* Always record the result: a device that failed to initialize
		 * earlier must not keep that error once it initializes
		 * successfully.
		 */
		dev->state->init_res = rc;
	}

	if (rc == 0) {
		/* A failed init leaves the device uninitialized, so it can be
		 * retried by device_init().
		 */
		dev->state->initialized = true;
	}

	/* here, the value of rc is either 0 or +errno
	 * flip the sign to return a negative value on failure as expected
	 */
	return -rc;
}

int device_init(const struct device *dev)
{
	if (dev->state->initialized) {
		return -EALREADY;
	}

	return do_device_init(dev);
}

const struct device *device_get_binding(const char *name)
{
	/* A null string identifies no device.  So does an empty
	 * string.
	 */
	if ((name == NULL) || (name[0] == '\0')) {
		return NULL;
	}

	/* Return NULL if the device matching 'name' is not ready. */
	for (const struct device *dev = DEVICE_SECTION_START; dev < DEVICE_SECTION_END; dev++) {
		if ((dev->name == name) || (strcmp(name, dev->name) == 0)) {
			return device_is_ready(dev) ? dev : NULL;
		}
	}

	return NULL;
}

size_t z_device_get_all_static(const struct device **devices)
{
	size_t cnt;

	*devices = DEVICE_SECTION_START;
	cnt = (size_t)(DEVICE_SECTION_END - DEVICE_SECTION_START);

	return cnt;
}

bool device_is_ready(const struct device *dev)
{
	/*
	 * if an invalid device pointer is passed as argument, this call
	 * reports the `device` as not ready for usage.
	 */
	if (dev == NULL) {
		return false;
	}

	return dev->state->initialized;
}
