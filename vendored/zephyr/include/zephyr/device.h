/*
 * Copyright (c) 2015-2016 Intel Corporation.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Minimal portable closure of Zephyr's device model.
 * Derived from Zephyr v4.5.0: include/zephyr/device.h
 *
 * Trimmed to the software-device route: devicetree, PM, LLEXT, dependencies
 * and userspace are removed. DEVICE_DEFINE()/DEVICE_GET() and
 * device_get_binding()/device_is_ready() are the supported surface.
 */

#ifndef ZEPHYR_INCLUDE_DEVICE_H_
#define ZEPHYR_INCLUDE_DEVICE_H_

#include <stdbool.h>
#include <stdint.h>

#include <zephyr/autoconf.h>
#include <zephyr/init.h>
#include <zephyr/shim/devicetree.h>
#include <zephyr/shim/pm_state.h>
#include <zephyr/sys/iterable_sections.h>
#include <zephyr/sys/util.h>
#include <zephyr/toolchain.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Device initialization result (positive errno value). */
struct device_state {
	uint8_t init_res;
	bool initialized : 1;
};

typedef uint8_t device_flags_t;

/** Device operations. */
struct device_ops {
	int (*init)(const struct device *dev);
};

/**
 * Runtime device structure (in ROM) per driver instance. device_get_binding()
 * finds it by name; device_is_ready() checks its state.
 */
struct device {
	const char *name;
	const void *config;
	const void *api;
	struct device_state *state;
	void *data;
	struct device_ops ops;
	device_flags_t flags;
};

/* --- device object naming --- */

#define DEVICE_NAME_GET(dev_id) _CONCAT(__device_, dev_id)
#define Z_DEVICE_STATE_NAME(dev_id) _CONCAT(__devstate_, dev_id)

/** Declare a device object so DEVICE_GET() may be used before its definition. */
#define DEVICE_DECLARE(dev_id) extern const struct device DEVICE_NAME_GET(dev_id)

/** Obtain a pointer to a device object by dev_id. */
#define DEVICE_GET(dev_id) (&DEVICE_NAME_GET(dev_id))

/** Obtain a pointer to the init entry created for a device. */
#define DEVICE_INIT_GET(dev_id) (&Z_INIT_ENTRY_NAME(DEVICE_NAME_GET(dev_id)))

/* --- device definition internals --- */

#define Z_DEVICE_INIT(name_, init_fn_, data_, config_, api_, state_, flags_)   \
	{                                                                      \
		.name = (name_), .config = (config_), .api = (api_),           \
		.state = (state_), .data = (data_),                            \
		.ops = {.init = (init_fn_)}, .flags = (flags_),                \
	}

/* Define the device object (extern linkage so the composition root in
 * board/devices.c can reference it via DEVICE_GET across translation units).
 */
#define Z_DEVICE_BASE_DEFINE(dev_id, name_, init_fn_, data_, config_, api_,    \
			     state_, flags_)                                   \
	const STRUCT_SECTION_ITERABLE_ALTERNATE(device, device,               \
						DEVICE_NAME_GET(dev_id)) =     \
		Z_DEVICE_INIT(name_, init_fn_, data_, config_, api_, state_,   \
			      flags_)

/* Define the init entry that drives this device's ops.init at boot. */
#define Z_DEVICE_INIT_ENTRY_DEFINE(dev_id, level, prio)                        \
	static const Z_DECL_ALIGN(struct init_entry)                           \
		Z_INIT_ENTRY_SECTION(level, prio, 0) __used __noasan           \
		Z_INIT_ENTRY_NAME(DEVICE_NAME_GET(dev_id)) = {                 \
			.init_fn = NULL,                                       \
			.dev = (const struct device *)&DEVICE_NAME_GET(dev_id),\
		}

#define Z_DEVICE_STATE_DEFINE(dev_id)                                          \
	static Z_DECL_ALIGN(struct device_state) Z_DEVICE_STATE_NAME(dev_id)

/**
 * Create a device object, its runtime state and its boot init entry.
 *
 * @param dev_id  Device identifier token (used to name the object).
 * @param name    Human-readable name (used by device_get_binding()).
 * @param init_fn Device init function `int fn(const struct device *)`, or NULL.
 * @param pm      Ignored (kept for signature compatibility); pass NULL.
 * @param data    Pointer to the device's private mutable data, or NULL.
 * @param config  Pointer to the device's private constant data, or NULL.
 * @param level   Init level (PRE_KERNEL_1/2, POST_KERNEL, APPLICATION).
 * @param prio    Init priority within the level, 0..999.
 * @param api     Pointer to the device's API structure, or NULL.
 */
#define DEVICE_DEFINE(dev_id, name, init_fn, pm, data, config, level, prio,    \
		      api)                                                     \
	Z_DEVICE_STATE_DEFINE(dev_id);                                         \
	Z_DEVICE_BASE_DEFINE(dev_id, name, init_fn, data, config, api,         \
			     &Z_DEVICE_STATE_NAME(dev_id), 0);                 \
	Z_DEVICE_INIT_ENTRY_DEFINE(dev_id, level, prio)

/* --- typed API access --- */

#define Z_DEVICE_API_TYPE(class) _CONCAT(class, _driver_api)

/** Cast a device's opaque api pointer to its typed API structure. */
#define DEVICE_API_GET(class, dev)                                             \
	((const struct Z_DEVICE_API_TYPE(class) *)(dev)->api)

/** Define a device API instance in an iterable section. */
#define DEVICE_API(class, name)                                                \
	const STRUCT_SECTION_ITERABLE(Z_DEVICE_API_TYPE(class), name)

/* --- runtime API (implemented in src/device.c) --- */

/**
 * Run a device's init function and record the result. Returns 0 on success or
 * a negative errno on failure.
 */
int do_device_init(const struct device *dev);

/** True if the device was initialized successfully. */
bool device_is_ready(const struct device *dev);

/**
 * Return the device whose name matches, only if it is ready. NULL when no
 * ready device matches (or when name is NULL/empty).
 */
const struct device *device_get_binding(const char *name);

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_DEVICE_H_ */
