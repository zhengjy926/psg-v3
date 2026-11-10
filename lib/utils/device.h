/*
 * Copyright (c) 2015 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief APIs and macros for the Zephyr device model.
 * @ingroup device_model
 */

#ifndef ZEPHYR_INCLUDE_DEVICE_H_
#define ZEPHYR_INCLUDE_DEVICE_H_

#include <stdint.h>

#include <init.h>
#include <iterable_sections.h>
#include <util.h>
#include <toolchain/toolchain.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Device Model
 * @defgroup device_model Device Model
 * @since 1.0
 * @version 1.1.0
 * @{
 */

/** @cond INTERNAL_HIDDEN */

/** @endcond */

/**
 * @brief Type used to represent a "handle" for a device.
 *
 * Every @ref device has an associated handle. You can get a pointer to a
 * @ref device from its handle and vice versa, but the handle uses less space
 * than a pointer. The device.h API mainly uses handles to store lists of
 * multiple devices in a compact way.
 *
 * The extreme values and zero have special significance. Negative values
 * identify functionality that does not correspond to a Zephyr device, such as
 * the system clock or a SYS_INIT() function.
 *
 * @see device_handle_get()
 * @see device_from_handle()
 */
typedef int16_t device_handle_t;

/** @brief Flag value used to identify an unknown device. */
#define DEVICE_HANDLE_NULL 0

/**
 * @brief Expands to the name of a global device object.
 *
 * Return the full name of a device object symbol created by DEVICE_DEFINE(),
 * using the `dev_id` provided to DEVICE_DEFINE(). This is the name of the
 * global variable storing the device structure, not a pointer to the string in
 * the @ref device.name field.
 *
 * It is meant to be used for declaring extern symbols pointing to device
 * objects before using the DEVICE_GET macro to get the device object.
 *
 * This macro is normally only useful within device driver source code. In other
 * situations, you are probably looking for device_get_binding().
 *
 * @param dev_id Device identifier.
 *
 * @return The full name of the device object defined by device definition
 * macros.
 */
#define DEVICE_NAME_GET(dev_id) _CONCAT(__device_, dev_id)

/**
 * @brief Create a device object and set it up for boot time initialization.
 *
 * This port has no devicetree, so every device is a software device: its
 * object is ``static const`` and is reachable by name through
 * device_get_binding().
 *
 * @param dev_id A unique token which is used in the name of the global device
 * structure as a C identifier.
 * @param name A string name for the device, which will be stored in
 * @ref device.name. This name can be used to look up the device with
 * device_get_binding(). This must be less than Z_DEVICE_MAX_NAME_LEN characters
 * (including terminating `NULL`).
 * @param init_fn Pointer to the device's initialization function, which will be
 * run during system initialization. Can be `NULL`.
 * @param data Pointer to the device's private mutable data, which will be
 * stored in @ref device.data.
 * @param config Pointer to the device's private constant data, which will be
 * stored in @ref device.config.
 * @param level The device's initialization level (PRE_KERNEL_1, PRE_KERNEL_2 or
 * POST_KERNEL).
 * @param prio The device's priority within its initialization level. See
 * SYS_INIT() for details.
 * @param api Pointer to the device's API structure. Can be `NULL`.
 */
#define DEVICE_DEFINE(dev_id, name, init_fn, data, config, level, prio,    \
		      api)                                                     \
	Z_DEVICE_STATE_DEFINE(dev_id);                                         \
	Z_DEVICE_DEFINE(dev_id, name, init_fn, 0U, data, config, level, prio,  \
			api, &Z_DEVICE_STATE_NAME(dev_id))

/**
 * @brief Obtain a pointer to a device object by name
 *
 * @details Return the address of a device object created by
 * DEVICE_DEFINE(), using the dev_id provided to DEVICE_DEFINE().
 *
 * @param dev_id Device identifier.
 *
 * @return A pointer to the device object created by DEVICE_DEFINE()
 */
#define DEVICE_GET(dev_id) (&DEVICE_NAME_GET(dev_id))

/**
 * @brief Declare a static device object
 *
 * This macro can be used at the top-level to declare a device, such
 * that DEVICE_GET() may be used before the full declaration in
 * DEVICE_DEFINE().
 *
 * This is often useful when configuring interrupts statically in a
 * device's init or per-instance config function, as the init function
 * itself is required by DEVICE_DEFINE() and use of DEVICE_GET()
 * inside it creates a circular dependency.
 *
 * @param dev_id Device identifier.
 */
#define DEVICE_DECLARE(dev_id)                                                 \
	static const struct device DEVICE_NAME_GET(dev_id)

/**
 * @brief Get a @ref init_entry reference from a device identifier.
 *
 * @param dev_id Device identifier.
 *
 * @return A pointer to the init_entry object created for that device
 */
#define DEVICE_INIT_GET(dev_id) (&Z_INIT_ENTRY_NAME(DEVICE_NAME_GET(dev_id)))

/**
 * @brief Runtime device dynamic structure (in RAM) per driver instance
 *
 * Fields in this are expected to be default-initialized to zero. The
 * kernel driver infrastructure and driver access functions are
 * responsible for ensuring that any non-zero initialization is done
 * before they are accessed.
 */
struct device_state {
	/**
	 * Device initialization return code (positive errno value).
	 *
	 * Device initialization functions return a negative errno code if they
	 * fail. In Zephyr, errno values do not exceed 255, so we can store the
	 * positive result value in a uint8_t type.
	 */
	uint8_t init_res;

	/** Indicates the device initialization function has been
	 * invoked and succeeded.
	 */
	bool initialized : 1;
};

/** Device flags */
typedef uint8_t device_flags_t;

/**
 * @name Device flags
 * @{
 */

/** Device initialization is deferred */
#define DEVICE_FLAG_INIT_DEFERRED BIT(0)

/** @} */

/** Device operations */
struct device_ops {
	/** Initialization function */
	int (*init)(const struct device *dev);
};

/**
 * @brief Runtime device structure (in ROM) per driver instance
 */
struct device {
	/** Name of the device instance */
	const char *name;
	/** Address of device instance config information */
	const void *config;
	/** Address of the API structure exposed by the device instance */
	const void *api;
	/** Address of the common device state */
	struct device_state *state;
	/** Address of the device instance private data */
	void *data;
	/** Device operations */
	struct device_ops ops;
	/** Device flags */
	device_flags_t flags;
};

/**
 * @brief Get the handle for a given device
 *
 * @param dev the device for which a handle is desired.
 *
 * @return the handle for the device, or DEVICE_HANDLE_NULL if the device does
 * not have an associated handle.
 */
static inline device_handle_t device_handle_get(const struct device *dev)
{
	device_handle_t ret = DEVICE_HANDLE_NULL;
	STRUCT_SECTION_START_EXTERN(device);

	/* TODO: If/when devices can be constructed that are not part of the
	 * fixed sequence we'll need another solution.
	 */
	if (dev != NULL) {
		ret = 1 + (device_handle_t)(dev - STRUCT_SECTION_START(device));
	}

	return ret;
}

/**
 * @brief Get the device corresponding to a handle.
 *
 * @param dev_handle the device handle
 *
 * @return the device that has that handle, or a null pointer if @p dev_handle
 * does not identify a device.
 */
static inline const struct device *
device_from_handle(device_handle_t dev_handle)
{
	STRUCT_SECTION_START_EXTERN(device);
	const struct device *dev = NULL;
	size_t numdev;

	STRUCT_SECTION_COUNT(device, &numdev);

	if ((dev_handle > 0) && ((size_t)dev_handle <= numdev)) {
		dev = &STRUCT_SECTION_START(device)[dev_handle - 1];
	}

	return dev;
}

/**
 * @brief Get a @ref device reference from its @ref device.name field.
 *
 * This function iterates through the devices on the system. If a device with
 * the given @p name field is found, and that device initialized successfully at
 * boot time, this function returns a pointer to the device.
 *
 * If no device has the given @p name, this function returns `NULL`.
 *
 * This function also returns NULL when a device is found, but it failed to
 * initialize successfully at boot time. (To troubleshoot this case, set a
 * breakpoint on your device driver's initialization function.)
 *
 * @param name device name to search for. A null pointer, or a pointer to an
 * empty string, will cause NULL to be returned.
 *
 * @return pointer to device structure with the given name; `NULL` if the device
 * is not found or if the device with that name's initialization function
 * failed.
 */
const struct device *device_get_binding(const char *name);

/**
 * @brief Get access to the static array of static devices.
 *
 * @param devices where to store the pointer to the array of statically
 * allocated devices. The array must not be mutated through this pointer.
 *
 * @return the number of statically allocated devices.
 */
size_t z_device_get_all_static(const struct device **devices);

/**
 * @brief Verify that a device is ready for use.
 *
 * Indicates whether the provided device pointer is for a device known to be
 * in a state where it can be used with its standard API.
 *
 * This can be used with device pointers obtained from DEVICE_GET(), which
 * does not include the readiness checks of device_get_binding(). At minimum
 * this means that the device has been successfully initialized.
 *
 * @param dev pointer to the device in question.
 *
 * @retval true If the device is ready for use.
 * @retval false If the device is not ready for use or if a NULL device pointer
 * is passed as argument.
 */
bool device_is_ready(const struct device *dev);

/**
 * @brief Writes a "device not ready" warning message to the log.
 *
 * @details Writes a "device not ready" warning message to the log using the
 * device name as reference, meant to be used in device_is_ready checks.
 *
 * @param dev pointer to a struct device.
 */
#define LOG_WRN_DEVICE_NOT_READY(dev) \
	LOG_WRN("%s device not ready", (dev) ? (dev)->name : "(null)")

/**
 * @brief Writes a "device not ready" error message to the log.
 *
 * @details Writes a "device not ready" error message to the log using the
 * device name as reference, meant to be used in device_is_ready checks.
 *
 * @param dev pointer to a struct device.
 */
#define LOG_ERR_DEVICE_NOT_READY(dev) \
	LOG_ERR("%s device not ready", (dev) ? (dev)->name : "(null)")

/**
 * @brief Writes a "device not ready" warning message to the log for the logging instance.
 *
 * @details Writes a "device not ready" warning message to the log using the
 * device name as reference, meant to be used in device_is_ready checks.
 *
 * @param _log_inst pointer to the log structure associated with the instance.
 * @param dev pointer to a struct device.
 */
#define LOG_INST_WRN_DEVICE_NOT_READY(_log_inst, dev) \
	LOG_INST_WRN(_log_inst, "%s device not ready", (dev) ? (dev)->name : "(null)")

/**
 * @brief Writes a "device not ready" error message to the log for the logging instance.
 *
 * @details Writes a "device not ready" error message to the log using the
 * device name as reference, meant to be used in device_is_ready checks.
 *
 * @param _log_inst pointer to the log structure associated with the instance.
 * @param dev pointer to a struct device.
 */
#define LOG_INST_ERR_DEVICE_NOT_READY(_log_inst, dev) \
	LOG_INST_ERR(_log_inst, "%s device not ready", (dev) ? (dev)->name : "(null)")

/**
 * @brief Initialize a device.
 *
 * A device whose initialization was deferred needs to be initialized manually
 * via this call. Devices whose initialization failed can be initialized again
 * via this call.
 *
 * @param dev device to be initialized.
 *
 * @retval 0 If successful
 * @retval -EALREADY Device is already initialized.
 * @retval -errno Error returned by the device initialization function.
 */
int device_init(const struct device *dev);

/**
 * @}
 */

/** @cond INTERNAL_HIDDEN */

/**
 * @brief Synthesize a unique name for the device state associated with
 * @p dev_id.
 */
#define Z_DEVICE_STATE_NAME(dev_id) _CONCAT(__devstate_, dev_id)

/**
 * @brief Utility macro to define and initialize the device state.
 *
 * @param dev_id Device identifier.
 */
#define Z_DEVICE_STATE_DEFINE(dev_id)                                          \
	static Z_DECL_ALIGN(struct device_state) Z_DEVICE_STATE_NAME(dev_id)   \
		__attribute__((__section__(".z_devstate")))

/**
 * @brief Maximum device name length.
 *
 * The maximum length is set so that device_get_binding() can be used from
 * userspace.
 */
#define Z_DEVICE_MAX_NAME_LEN 48U

/**
 * @brief Compile time check for device name length
 *
 * @param name Device name.
 */
#define Z_DEVICE_NAME_CHECK(name)                                              \
	BUILD_ASSERT(sizeof(Z_STRINGIFY(name)) <= Z_DEVICE_MAX_NAME_LEN,       \
			    Z_STRINGIFY(name) " too long")

/**
 * @brief Fill in the struct device_ops
 *
 * @param init_fn_ Initialization function
 */
#define Z_DEVICE_OPS(init_fn_)                                                 \
	{                                                                      \
		.init = (init_fn_),                                            \
	}

/**
 * @brief Initializer for @ref device.
 *
 * @param name_ Name of the device.
 * @param init_fn_ Init function (optional).
 * @param flags_ Device flags.
 * @param data_ Reference to device data.
 * @param config_ Reference to device config.
 * @param api_ Reference to device API ops.
 * @param state_ Reference to device state.
 */
#define Z_DEVICE_INIT(name_, init_fn_, flags_, data_, config_, api_, state_) \
	{                                                                      \
		.name = name_,                                                 \
		.config = (config_),                                           \
		.api = (api_),                                                 \
		.state = (state_),                                             \
		.data = (data_),                                               \
		.ops = Z_DEVICE_OPS(init_fn_),                                 \
		.flags = (flags_),                                             \
	}

/**
 * @brief Device section name (used for sorting purposes).
 *
 * @param level Initialization level
 * @param prio Initialization priority
 */
#define Z_DEVICE_SECTION_NAME(level, prio)                                     \
	_CONCAT(INIT_LEVEL_ORD(level), _##prio)

/**
 * @brief Define a @ref device
 *
 * @param dev_id Device identifier (used to name the defined @ref device).
 * @param name Name of the device.
 * @param init_fn Init function.
 * @param flags Device flags.
 * @param data Reference to device data.
 * @param config Reference to device config.
 * @param level Initialization level.
 * @param prio Initialization priority.
 * @param api Reference to device API.
 * @param state Reference to device state.
 */
#define Z_DEVICE_BASE_DEFINE(dev_id, name, init_fn, flags, data, config,               \
			     level, prio, api, state)                                  \
	static const STRUCT_SECTION_ITERABLE_NAMED_ALTERNATE(                              \
		device, device, Z_DEVICE_SECTION_NAME(level, prio),                        \
		DEVICE_NAME_GET(dev_id)) =                                                 \
		Z_DEVICE_INIT(name, init_fn, flags, data, config, api, state)

/**
 * @brief Issue an error if the given init level is not supported.
 *
 * @param level Init level
 */
#define Z_DEVICE_CHECK_INIT_LEVEL(level)                                       \
	COND_CASE_1(Z_INIT_PRE_KERNEL_1_##level, (),                           \
		    Z_INIT_PRE_KERNEL_2_##level, (),                           \
		    Z_INIT_POST_KERNEL_##level, (),                            \
		    (ZERO_OR_COMPILE_ERROR(0)))

/**
 * @brief Define the init entry for a device.
 *
 * @param dev_id Device identifier.
 * @param level Initialization level.
 * @param prio Initialization priority.
 */
#define Z_DEVICE_INIT_ENTRY_DEFINE(dev_id, level, prio)                                           \
	Z_DEVICE_CHECK_INIT_LEVEL(level)                                                           \
                                                                                                   \
	static const Z_DECL_ALIGN(struct init_entry) __used __noasan Z_INIT_ENTRY_SECTION(         \
		level, prio, 0)                                                                    \
		Z_INIT_ENTRY_NAME(DEVICE_NAME_GET(dev_id)) = {                                     \
			.init_fn = NULL,                                                           \
			.dev = (const struct device *)&DEVICE_NAME_GET(dev_id),                    \
		}

/**
 * @brief Define a @ref device and all other required objects.
 *
 * @param dev_id Device identifier (used to name the defined @ref device).
 * @param name Name of the device.
 * @param init_fn Device init function.
 * @param flags Device flags.
 * @param data Reference to device data.
 * @param config Reference to device config.
 * @param level Initialization level.
 * @param prio Initialization priority.
 * @param api Reference to device API.
 * @param state Reference to device state.
 */
#define Z_DEVICE_DEFINE(dev_id, name, init_fn, flags, data, config, level, prio, api, state) \
	Z_DEVICE_NAME_CHECK(name);                                                         \
                                                                                           \
	Z_DEVICE_BASE_DEFINE(dev_id, name, init_fn, flags, data, config, level, prio, api, \
			     state);                                                       \
                                                                                           \
	Z_DEVICE_INIT_ENTRY_DEFINE(dev_id, level, prio)

/** @brief Expands to the full type. */
#define Z_DEVICE_API_TYPE(_class) _CONCAT(_class, _driver_api)

/** @brief Helper to get API pointer. */
#define Z_DEVICE_API_GET(_class, _dev) ((const struct Z_DEVICE_API_TYPE(_class) *)_dev->api)

/** @brief Linker symbol marking end of class section including child sections. */
#define Z_DEVICE_API_EXT_END(_struct_type) CONCAT(_, _struct_type, _ext_end)

/** @endcond */

/**
 * @brief Wrapper macro for declaring device API structs inside iterable sections.
 *
 * @param _class The device API class.
 * @param _name The API instance name.
 */
#define DEVICE_API(_class, _name) const STRUCT_SECTION_ITERABLE(Z_DEVICE_API_TYPE(_class), _name)

/**
 * @brief Declare that API class @p _child is an extension of device API class @p _parent.
 *
 * This registers the parent-child relationship so that DEVICE_API_IS(_parent, dev) returns
 * true for devices whose API is of the @p _child class. The parent API struct must be the
 * first member of the child API struct.
 *
 * @param _child Name of child (extending) API class.
 * @param _parent Name of parent (base) API class.
 * @param _member Name of the first member of @p _child API class struct.
 * This member must have the same type as the @p _parent API class struct.
 *
 * @internal
 * In addition to build-time validation, this macro is used as a sentinel by parse_syscalls.py
 * to identify the API classes that are extended from others and build an API class hierarchy.
 * @endinternal
 */
#define DEVICE_API_EXTENDS(_child, _parent, _member)                                               \
	CONTAINER_OF_VALIDATE((struct Z_DEVICE_API_TYPE(_parent) *)0,                              \
			      struct Z_DEVICE_API_TYPE(_child), _member)                           \
	BUILD_ASSERT(offsetof(struct Z_DEVICE_API_TYPE(_child), _member) == 0,                     \
		     "Parent API struct must be the first member of child API struct")

/**
 * @brief Macro that evaluates to a boolean that can be used to check if
 *        a device is of a particular class.
 *
 * @param _class The device API class.
 * @param _dev The device instance pointer.
 *
 * @retval true If the device is of the given class
 * @retval false If the device is not of the given class
 */
#define DEVICE_API_IS(_class, _dev)                                                                \
	({                                                                                         \
		STRUCT_SECTION_START_EXTERN(Z_DEVICE_API_TYPE(_class));                            \
		extern const uint8_t Z_DEVICE_API_EXT_END(Z_DEVICE_API_TYPE(_class))[];            \
                                                                                                   \
		((const uint8_t *)Z_DEVICE_API_GET(_class, _dev) <                                 \
			 Z_DEVICE_API_EXT_END(Z_DEVICE_API_TYPE(_class)) &&                        \
		 (const uint8_t *)Z_DEVICE_API_GET(_class, _dev) >=                                \
			 (const uint8_t *)STRUCT_SECTION_START(Z_DEVICE_API_TYPE(_class)));        \
	})

/**
 * @brief Expands to the pointer of a device's API for a given class.
 *
 * @param _class The device API class.
 * @param _dev The device instance pointer.
 *
 * @return the pointer to the device API.
 */
#define DEVICE_API_GET(_class, _dev)                                                               \
	({                                                                                         \
		IF_ENABLED(CONFIG_DEVICE_API_ASSERT, (                                             \
		__ASSERT(_dev != NULL, "device is NULL");                                          \
		__ASSERT(DEVICE_API_IS(_class, _dev), "device API is not %s", STRINGIFY(_class));  \
		));                                                                                \
		Z_DEVICE_API_GET(_class, _dev);                                                    \
	})

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_DEVICE_H_ */
