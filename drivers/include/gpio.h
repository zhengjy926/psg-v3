/*
 * Copyright (c) 2019-2020 Nordic Semiconductor ASA
 * Copyright (c) 2019 Piotr Mienkowski
 * Copyright (c) 2017 ARM Ltd
 * Copyright (c) 2015-2016 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @ingroup gpio_interface
 * @brief Main header file for GPIO driver API.
 */

#ifndef ZEPHYR_INCLUDE_DRIVERS_GPIO_H_
#define ZEPHYR_INCLUDE_DRIVERS_GPIO_H_

#include <errno.h>
#include <__assert.h>
#include <slist.h>
#include <tracing_macros.h>

#include <types.h>
#include <stddef.h>
#include <device.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Interfaces for General Purpose Input/Output (GPIO)
 *        controllers.
 * @defgroup gpio_interface GPIO
 * @since 1.0
 * @version 1.0.0
 * @ingroup io_interfaces
 * @{
 *
 * @defgroup gpio_interface_ext Device-specific GPIO API extensions
 *
 * @{
 * @}
 */

/**
 * @name GPIO input/output configuration flags
 * @{
 */

/*
 * Pin active level, drive and bias flags. Values match Zephyr's
 * dt-bindings/gpio/gpio.h; this port has no devicetree, so they are plain
 * API flags here. Bits 0..6 are reserved for them, the direction flags
 * start at bit 16.
 */
#define GPIO_ACTIVE_LOW         (1 << 0)
#define GPIO_ACTIVE_HIGH        (0 << 0)

#define GPIO_SINGLE_ENDED       (1 << 1)
#define GPIO_PUSH_PULL          (0 << 1)
#define GPIO_LINE_OPEN_DRAIN    (1 << 2)
#define GPIO_LINE_OPEN_SOURCE   (0 << 2)
#define GPIO_OPEN_DRAIN         (GPIO_SINGLE_ENDED | GPIO_LINE_OPEN_DRAIN)
#define GPIO_OPEN_SOURCE        (GPIO_SINGLE_ENDED | GPIO_LINE_OPEN_SOURCE)

#define GPIO_PULL_UP            (1 << 4)
#define GPIO_PULL_DOWN          (1 << 5)

#define GPIO_INT_WAKEUP         (1 << 6)

/** Enables pin as input. */
#define GPIO_INPUT              BIT(16)

/** Enables pin as output, no change to the output state. */
#define GPIO_OUTPUT             BIT(17)

/** Disables pin for both input and output. */
#define GPIO_DISCONNECTED	0

/** @cond INTERNAL_HIDDEN */

/* Initializes output to a low state. */
#define GPIO_OUTPUT_INIT_LOW    BIT(18)

/* Initializes output to a high state. */
#define GPIO_OUTPUT_INIT_HIGH   BIT(19)

/* Initializes output based on logic level */
#define GPIO_OUTPUT_INIT_LOGICAL BIT(20)

/** @endcond */

/** Configures GPIO pin as output and initializes it to a low state. */
#define GPIO_OUTPUT_LOW         (GPIO_OUTPUT | GPIO_OUTPUT_INIT_LOW)
/** Configures GPIO pin as output and initializes it to a high state. */
#define GPIO_OUTPUT_HIGH        (GPIO_OUTPUT | GPIO_OUTPUT_INIT_HIGH)
/** Configures GPIO pin as output and initializes it to a logic 0. */
#define GPIO_OUTPUT_INACTIVE    (GPIO_OUTPUT |			\
				 GPIO_OUTPUT_INIT_LOW |		\
				 GPIO_OUTPUT_INIT_LOGICAL)
/** Configures GPIO pin as output and initializes it to a logic 1. */
#define GPIO_OUTPUT_ACTIVE      (GPIO_OUTPUT |			\
				 GPIO_OUTPUT_INIT_HIGH |	\
				 GPIO_OUTPUT_INIT_LOGICAL)

/** @} */

/**
 * @name GPIO interrupt configuration flags
 * The `GPIO_INT_*` flags are used to specify how input GPIO pins will trigger
 * interrupts. The interrupts can be sensitive to pin physical or logical level.
 * Interrupts sensitive to pin logical level take into account GPIO_ACTIVE_LOW
 * flag. If a pin was configured as Active Low, physical level low will be
 * considered as logical level 1 (an active state), physical level high will
 * be considered as logical level 0 (an inactive state).
 * The GPIO controller should reset the interrupt status, such as clearing the
 * pending bit, etc, when configuring the interrupt triggering properties.
 * Applications should use the `GPIO_INT_MODE_ENABLE_ONLY` and
 * `GPIO_INT_MODE_DISABLE_ONLY` flags to enable and disable interrupts on the
 * pin without changing any GPIO settings.
 * @{
 */

/** Disables GPIO pin interrupt. */
#define GPIO_INT_DISABLE               BIT(21)

/** @cond INTERNAL_HIDDEN */

/* Enables GPIO pin interrupt. */
#define GPIO_INT_ENABLE                BIT(22)

/* GPIO interrupt is sensitive to logical levels.
 *
 * This is a component flag that should be combined with other
 * `GPIO_INT_*` flags to produce a meaningful configuration.
 */
#define GPIO_INT_LEVELS_LOGICAL        BIT(23)

/* GPIO interrupt is edge sensitive.
 *
 * Note: by default interrupts are level sensitive.
 *
 * This is a component flag that should be combined with other
 * `GPIO_INT_*` flags to produce a meaningful configuration.
 */
#define GPIO_INT_EDGE                  BIT(24)

/* Trigger detection when input state is (or transitions to) physical low or
 * logical 0 level.
 *
 * This is a component flag that should be combined with other
 * `GPIO_INT_*` flags to produce a meaningful configuration.
 */
#define GPIO_INT_LOW_0                 BIT(25)

/* Trigger detection on input state is (or transitions to) physical high or
 * logical 1 level.
 *
 * This is a component flag that should be combined with other
 * `GPIO_INT_*` flags to produce a meaningful configuration.
 */
#define GPIO_INT_HIGH_1                BIT(26)

#ifdef CONFIG_GPIO_ENABLE_DISABLE_INTERRUPT
/* Disable/Enable interrupt functionality without changing other interrupt
 * related register, such as clearing the pending register.
 *
 * This is a component flag that should be combined with `GPIO_INT_ENABLE` or
 * `GPIO_INT_DISABLE` flags to produce a meaningful configuration.
 */
#define GPIO_INT_ENABLE_DISABLE_ONLY   BIT(27)
#endif /* CONFIG_GPIO_ENABLE_DISABLE_INTERRUPT */

#define GPIO_INT_MASK                  (GPIO_INT_DISABLE | \
					GPIO_INT_ENABLE | \
					GPIO_INT_LEVELS_LOGICAL | \
					GPIO_INT_EDGE | \
					GPIO_INT_LOW_0 | \
					GPIO_INT_HIGH_1)

/** @endcond */

/** Configures GPIO interrupt to be triggered on pin rising edge and enables it.
 */
#define GPIO_INT_EDGE_RISING           (GPIO_INT_ENABLE | \
					GPIO_INT_EDGE | \
					GPIO_INT_HIGH_1)

/** Configures GPIO interrupt to be triggered on pin falling edge and enables
 * it.
 */
#define GPIO_INT_EDGE_FALLING          (GPIO_INT_ENABLE | \
					GPIO_INT_EDGE | \
					GPIO_INT_LOW_0)

/** Configures GPIO interrupt to be triggered on pin rising or falling edge and
 * enables it.
 */
#define GPIO_INT_EDGE_BOTH             (GPIO_INT_ENABLE | \
					GPIO_INT_EDGE | \
					GPIO_INT_LOW_0 | \
					GPIO_INT_HIGH_1)

/** Configures GPIO interrupt to be triggered on pin physical level low and
 * enables it.
 */
#define GPIO_INT_LEVEL_LOW             (GPIO_INT_ENABLE | \
					GPIO_INT_LOW_0)

/** Configures GPIO interrupt to be triggered on pin physical level high and
 * enables it.
 */
#define GPIO_INT_LEVEL_HIGH            (GPIO_INT_ENABLE | \
					GPIO_INT_HIGH_1)

/** Configures GPIO interrupt to be triggered on pin state change to logical
 * level 0 and enables it.
 */
#define GPIO_INT_EDGE_TO_INACTIVE      (GPIO_INT_ENABLE | \
					GPIO_INT_LEVELS_LOGICAL | \
					GPIO_INT_EDGE | \
					GPIO_INT_LOW_0)

/** Configures GPIO interrupt to be triggered on pin state change to logical
 * level 1 and enables it.
 */
#define GPIO_INT_EDGE_TO_ACTIVE        (GPIO_INT_ENABLE | \
					GPIO_INT_LEVELS_LOGICAL | \
					GPIO_INT_EDGE | \
					GPIO_INT_HIGH_1)

/** Configures GPIO interrupt to be triggered on pin logical level 0 and enables
 * it.
 */
#define GPIO_INT_LEVEL_INACTIVE        (GPIO_INT_ENABLE | \
					GPIO_INT_LEVELS_LOGICAL | \
					GPIO_INT_LOW_0)

/** Configures GPIO interrupt to be triggered on pin logical level 1 and enables
 * it.
 */
#define GPIO_INT_LEVEL_ACTIVE          (GPIO_INT_ENABLE | \
					GPIO_INT_LEVELS_LOGICAL | \
					GPIO_INT_HIGH_1)

/** @} */

/** @cond INTERNAL_HIDDEN */
#define GPIO_DIR_MASK		(GPIO_INPUT | GPIO_OUTPUT)
/** @endcond */

/**
 * @brief Identifies a set of pins associated with a port.
 *
 * The pin with index n is present in the set if and only if the bit
 * identified by (1U << n) is set.
 */
typedef uint32_t gpio_port_pins_t;

/**
 * @brief Provides values for a set of pins associated with a port.
 *
 * The value for a pin with index n is high (physical mode) or active
 * (logical mode) if and only if the bit identified by (1U << n) is set.
 * Otherwise the value for the pin is low (physical mode) or inactive
 * (logical mode).
 *
 * Values of this type are often paired with a `gpio_port_pins_t` value
 * that specifies which encoded pin values are valid for the operation.
 */
typedef uint32_t gpio_port_value_t;

/**
 * @brief Provides a type to hold a GPIO pin index.
 *
 * This reduced-size type is sufficient to record a pin number.
 */
typedef uint8_t gpio_pin_t;

/**
 * @brief Provides a type to hold GPIO configuration flags.
 *
 * This type is sufficient to hold all flags used to control GPIO
 * configuration, whether pin or interrupt.
 */
typedef uint32_t gpio_flags_t;

/**
 * @brief Maximum number of pins that are supported by `gpio_port_pins_t`.
 */
#define GPIO_MAX_PINS_PER_PORT (sizeof(gpio_port_pins_t) * __CHAR_BIT__)

/**
 * @brief Check whether a pin is in range and supported by a GPIO controller.
 *
 * @note @p pin is checked against @ref GPIO_MAX_PINS_PER_PORT before it is
 * used as a shift count. Drivers must use this helper instead of testing
 * `port_pin_mask & BIT(pin)` directly, as @p pin may originate from an
 * untrusted caller.
 *
 * @param port_pin_mask Mask identifying the pins supported by the controller,
 *                      i.e. @ref gpio_driver_config.port_pin_mask.
 * @param pin Pin number to check.
 *
 * @retval true If @p pin is in range and supported by the controller.
 * @retval false Otherwise.
 */
static inline bool gpio_port_pin_is_supported(gpio_port_pins_t port_pin_mask, gpio_pin_t pin)
{
	if (pin >= GPIO_MAX_PINS_PER_PORT) {
		return false;
	}

	return (port_pin_mask & (gpio_port_pins_t)BIT(pin)) != 0U;
}

/**
 * This structure is common to all GPIO drivers and is expected to be
 * the first element in the object pointed to by the config field
 * in the device structure.
 */
struct gpio_driver_config {
	/** Mask identifying pins supported by the controller.
	 *
	 * Initialization of this mask is the responsibility of device
	 * instance generation in the driver.
	 */
	gpio_port_pins_t port_pin_mask;
};

/**
 * This structure is common to all GPIO drivers and is expected to be the first
 * element in the driver's struct driver_data declaration.
 */
struct gpio_driver_data {
	/** Mask identifying pins that are configured as active low.
	 *
	 * Management of this mask is the responsibility of the
	 * wrapper functions in this header.
	 */
	gpio_port_pins_t invert;
};

struct gpio_callback;

/**
 * @brief Define the application callback handler function signature
 *
 * @param port Device struct for the GPIO device.
 * @param cb Original struct gpio_callback owning this handler
 * @param pins Mask of pins that triggers the callback handler
 *
 * Note: cb pointer can be used to retrieve private data through
 * CONTAINER_OF() if original struct gpio_callback is stored in
 * another private structure.
 */
typedef void (*gpio_callback_handler_t)(const struct device *port,
					struct gpio_callback *cb,
					gpio_port_pins_t pins);

/**
 * @brief GPIO callback structure
 *
 * Used to register a callback in the driver instance callback list.
 * As many callbacks as needed can be added as long as each of them
 * are unique pointers of struct gpio_callback.
 * Beware such structure should not be allocated on stack.
 *
 * Note: To help setting it, see gpio_init_callback() below
 */
struct gpio_callback {
	/** This is meant to be used in the driver and the user should not
	 * mess with it (see drivers/gpio/gpio_utils.h)
	 */
	sys_snode_t node;

	/** Actual callback function being called when relevant. */
	gpio_callback_handler_t handler;

	/** A mask of pins the callback is interested in, if 0 the callback
	 * will never be called. Such pin_mask can be modified whenever
	 * necessary by the owner, and thus will affect the handler being
	 * called or not. The selected pins must be configured to trigger
	 * an interrupt.
	 */
	gpio_port_pins_t pin_mask;
};

/**
 * @def_driverbackendgroup{GPIO,gpio_interface}
 * @{
 */

/**
 * @brief Interrupt modes for the pin_interrupt_configure driver operation.
 *
 * Used by driver api function pin_interrupt_configure, these are defined
 * in terms of the public flags so we can just mask and pass them
 * through to the driver api.
 */
enum gpio_int_mode {
	/** Disable pin interrupt. */
	GPIO_INT_MODE_DISABLED = GPIO_INT_DISABLE,
	/** Enable level-triggered pin interrupt. */
	GPIO_INT_MODE_LEVEL = GPIO_INT_ENABLE,
	/** Enable edge-triggered pin interrupt. */
	GPIO_INT_MODE_EDGE = GPIO_INT_ENABLE | GPIO_INT_EDGE,
#if defined(CONFIG_GPIO_ENABLE_DISABLE_INTERRUPT) || defined(__DOXYGEN__)
	/**
	 * Disable a previously configured pin interrupt without deconfiguring it.
	 * @kconfig_dep{CONFIG_GPIO_ENABLE_DISABLE_INTERRUPT}
	 */
	GPIO_INT_MODE_DISABLE_ONLY = GPIO_INT_DISABLE | GPIO_INT_ENABLE_DISABLE_ONLY,
	/**
	 * Re-enable a previously configured pin interrupt.
	 * @kconfig_dep{CONFIG_GPIO_ENABLE_DISABLE_INTERRUPT}
	 */
	GPIO_INT_MODE_ENABLE_ONLY = GPIO_INT_ENABLE | GPIO_INT_ENABLE_DISABLE_ONLY,
#endif /* CONFIG_GPIO_ENABLE_DISABLE_INTERRUPT */
};

/**
 * @brief Interrupt triggers for the pin_interrupt_configure driver operation.
 */
enum gpio_int_trig {
	/** Trigger detection when input state is (or transitions to)
	 * physical low. (Edge Falling or Active Low)
	 */
	GPIO_INT_TRIG_LOW = GPIO_INT_LOW_0,
	/** Trigger detection when input state is (or transitions to)
	 * physical high. (Edge Rising or Active High) */
	GPIO_INT_TRIG_HIGH = GPIO_INT_HIGH_1,
	/** Trigger detection on pin rising or falling edge. */
	GPIO_INT_TRIG_BOTH = GPIO_INT_LOW_0 | GPIO_INT_HIGH_1,
	/** Trigger a system wakeup. */
	GPIO_INT_TRIG_WAKE = GPIO_INT_WAKEUP,
	/** Trigger a system wakeup when input state is (or transitions to)
	 * physical low. (Edge Falling or Active Low)
	 */
	GPIO_INT_TRIG_WAKE_LOW = GPIO_INT_LOW_0 | GPIO_INT_WAKEUP,
	/** Trigger a system wakeup when input state is (or transitions to)
	 * physical high. (Edge Rising or Active High)
	 */
	GPIO_INT_TRIG_WAKE_HIGH = GPIO_INT_HIGH_1 | GPIO_INT_WAKEUP,
	/** Trigger a system wakeup on pin rising or falling edge. */
	GPIO_INT_TRIG_WAKE_BOTH = GPIO_INT_LOW_0 | GPIO_INT_HIGH_1 | GPIO_INT_WAKEUP,
};

/**
 * @brief Callback API to configure a single pin.
 * See gpio_pin_configure() for argument description.
 */
typedef int (*gpio_api_pin_configure_t)(const struct device *port, gpio_pin_t pin,
					gpio_flags_t flags);

/**
 * @brief Callback API to get the configuration of a single pin.
 * See gpio_pin_get_config() for argument description.
 */
typedef int (*gpio_api_pin_get_config_t)(const struct device *port, gpio_pin_t pin,
					 gpio_flags_t *flags);

/**
 * @brief Callback API to get the physical level of all input pins in a port.
 * See gpio_port_get_raw() for argument description.
 */
typedef int (*gpio_api_port_get_raw_t)(const struct device *port,
				       gpio_port_value_t *value);

/**
 * @brief Callback API to set the physical level of selected output pins in a port.
 * See gpio_port_set_masked_raw() for argument description.
 */
typedef int (*gpio_api_port_set_masked_raw_t)(const struct device *port,
					      gpio_port_pins_t mask,
					      gpio_port_value_t value);

/**
 * @brief Callback API to set the physical level of selected output pins to high.
 * See gpio_port_set_bits_raw() for argument description.
 */
typedef int (*gpio_api_port_set_bits_raw_t)(const struct device *port,
					    gpio_port_pins_t pins);

/**
 * @brief Callback API to set the physical level of selected output pins to low.
 * See gpio_port_clear_bits_raw() for argument description.
 */
typedef int (*gpio_api_port_clear_bits_raw_t)(const struct device *port,
					      gpio_port_pins_t pins);

/**
 * @brief Callback API to toggle the level of selected output pins.
 * See gpio_port_toggle_bits() for argument description.
 */
typedef int (*gpio_api_port_toggle_bits_t)(const struct device *port,
					   gpio_port_pins_t pins);

/**
 * @brief Callback API to configure a pin interrupt.
 * See gpio_pin_interrupt_configure() for argument description, noting that the
 * public @c GPIO_INT_* flags are decomposed into @p mode and @p trig before
 * being passed to the driver.
 */
typedef int (*gpio_api_pin_interrupt_configure_t)(const struct device *port,
						  gpio_pin_t pin,
						  enum gpio_int_mode mode,
						  enum gpio_int_trig trig);

/**
 * @brief Callback API to add or remove an application callback.
 * See gpio_add_callback() and gpio_remove_callback() for argument description.
 */
typedef int (*gpio_api_manage_callback_t)(const struct device *port,
					  struct gpio_callback *cb,
					  bool set);

/**
 * @brief Callback API to get pending interrupts.
 * See gpio_get_pending_int() for argument description.
 */
typedef uint32_t (*gpio_api_get_pending_int_t)(const struct device *dev);

/**
 * @brief Callback API to get the direction of selected pins in a port.
 * See gpio_port_get_direction() for argument description.
 */
typedef int (*gpio_api_port_get_direction_t)(const struct device *port, gpio_port_pins_t map,
					     gpio_port_pins_t *inputs, gpio_port_pins_t *outputs);

/**
 * @driver_ops{GPIO}
 */
__subsystem struct gpio_driver_api {
	/** @driver_ops_mandatory @copybrief gpio_pin_configure */
	gpio_api_pin_configure_t pin_configure;
#if defined(CONFIG_GPIO_GET_CONFIG) || defined(__DOXYGEN__)
	/**
	 * @driver_ops_optional @copybrief gpio_pin_get_config
	 * @kconfig_dep{CONFIG_GPIO_GET_CONFIG}
	 */
	gpio_api_pin_get_config_t pin_get_config;
#endif /* CONFIG_GPIO_GET_CONFIG */
	/** @driver_ops_mandatory @copybrief gpio_port_get_raw */
	gpio_api_port_get_raw_t port_get_raw;
	/** @driver_ops_mandatory @copybrief gpio_port_set_masked_raw */
	gpio_api_port_set_masked_raw_t port_set_masked_raw;
	/** @driver_ops_mandatory @copybrief gpio_port_set_bits_raw */
	gpio_api_port_set_bits_raw_t port_set_bits_raw;
	/** @driver_ops_mandatory @copybrief gpio_port_clear_bits_raw */
	gpio_api_port_clear_bits_raw_t port_clear_bits_raw;
	/** @driver_ops_mandatory @copybrief gpio_port_toggle_bits */
	gpio_api_port_toggle_bits_t port_toggle_bits;
	/** @driver_ops_optional @copybrief gpio_pin_interrupt_configure */
	gpio_api_pin_interrupt_configure_t pin_interrupt_configure;
	/**
	 * @driver_ops_optional Add or remove an application callback.
	 * See gpio_add_callback() and gpio_remove_callback().
	 */
	gpio_api_manage_callback_t manage_callback;
	/** @driver_ops_optional @copybrief gpio_get_pending_int */
	gpio_api_get_pending_int_t get_pending_int;
#if defined(CONFIG_GPIO_GET_DIRECTION) || defined(__DOXYGEN__)
	/**
	 * @driver_ops_optional @copybrief gpio_port_get_direction
	 * @kconfig_dep{CONFIG_GPIO_GET_DIRECTION}
	 */
	gpio_api_port_get_direction_t port_get_direction;
#endif /* CONFIG_GPIO_GET_DIRECTION */
};

/** @} */


/**
 * @brief Configure pin interrupt.
 *
 * @note This function can also be used to configure interrupts on pins
 *       not controlled directly by the GPIO module. That is, pins which are
 *       routed to other modules such as I2C, SPI, UART.
 *
 * @isr_ok
 *
 * @param[in] port Pointer to device structure for the driver instance.
 * @param[in] pin Pin number.
 * @param[in] flags Interrupt configuration flags as defined by GPIO_INT_*.
 *
 * @retval 0 If successful.
 * @retval -ENOSYS If the operation is not implemented by the driver.
 * @retval -ENOTSUP If any of the configuration options is not supported
 *                  (unless otherwise directed by flag documentation).
 * @retval -EINVAL  Invalid argument.
 * @retval -EBUSY   Interrupt line required to configure pin interrupt is
 *                  already in use.
 * @retval -EIO I/O error when accessing an external GPIO chip.
 * @retval -EWOULDBLOCK if operation would block.
 */
static inline int gpio_pin_interrupt_configure(const struct device *port,
						      gpio_pin_t pin,
						      gpio_flags_t flags)
{
	const struct gpio_driver_api *api = DEVICE_API_GET(gpio, port);
	__unused const struct gpio_driver_config *const cfg =
		(const struct gpio_driver_config *)port->config;
	const struct gpio_driver_data *const data =
		(const struct gpio_driver_data *)port->data;
	enum gpio_int_trig trig;
	enum gpio_int_mode mode;
	int ret;

	SYS_PORT_TRACING_FUNC_ENTER(gpio_pin, interrupt_configure, port, pin, flags);

	if (api->pin_interrupt_configure == NULL) {
		SYS_PORT_TRACING_FUNC_EXIT(gpio_pin, interrupt_configure, port, pin, -ENOSYS);
		return -ENOSYS;
	}

	__ASSERT((flags & (GPIO_INT_DISABLE | GPIO_INT_ENABLE))
		 != (GPIO_INT_DISABLE | GPIO_INT_ENABLE),
		 "Cannot both enable and disable interrupts");

	__ASSERT((flags & (GPIO_INT_DISABLE | GPIO_INT_ENABLE)) != 0U,
		 "Must either enable or disable interrupts");

	__ASSERT(((flags & GPIO_INT_ENABLE) == 0) ||
		 ((flags & GPIO_INT_EDGE) != 0) ||
		 ((flags & (GPIO_INT_LOW_0 | GPIO_INT_HIGH_1)) !=
		  (GPIO_INT_LOW_0 | GPIO_INT_HIGH_1)),
		 "Only one of GPIO_INT_LOW_0, GPIO_INT_HIGH_1 can be "
		 "enabled for a level interrupt.");

/** @cond INTERNAL_HIDDEN */
#ifdef CONFIG_GPIO_ENABLE_DISABLE_INTERRUPT
#define GPIO_INT_ENABLE_DISABLE_ONLY_VALUE  GPIO_INT_ENABLE_DISABLE_ONLY
#else
#define GPIO_INT_ENABLE_DISABLE_ONLY_VALUE  0
#endif /* CONFIG_GPIO_ENABLE_DISABLE_INTERRUPT */
/** @endcond */

	__ASSERT(((flags & GPIO_INT_ENABLE) == 0) ||
			 ((flags & (GPIO_INT_LOW_0 | GPIO_INT_HIGH_1)) != 0) ||
			 (flags & GPIO_INT_ENABLE_DISABLE_ONLY_VALUE) != 0,
		 "At least one of GPIO_INT_LOW_0, GPIO_INT_HIGH_1 has to be enabled.");
#undef GPIO_INT_ENABLE_DISABLE_ONLY_VALUE
	__ASSERT(gpio_port_pin_is_supported(cfg->port_pin_mask, pin), "Unsupported pin");

	/* See the note in gpio_pin_configure(). */
	if (pin >= GPIO_MAX_PINS_PER_PORT) {
		SYS_PORT_TRACING_FUNC_EXIT(gpio_pin, interrupt_configure, port, pin, -EINVAL);
		return -EINVAL;
	}

	if (((flags & GPIO_INT_LEVELS_LOGICAL) != 0) &&
	    ((data->invert & (gpio_port_pins_t)BIT(pin)) != 0)) {
		/* Invert signal bits */
		flags ^= (GPIO_INT_LOW_0 | GPIO_INT_HIGH_1);
	}

	trig = (enum gpio_int_trig)(flags & (GPIO_INT_LOW_0 | GPIO_INT_HIGH_1 | GPIO_INT_WAKEUP));
#ifdef CONFIG_GPIO_ENABLE_DISABLE_INTERRUPT
	mode = (enum gpio_int_mode)(flags & (GPIO_INT_EDGE | GPIO_INT_DISABLE | GPIO_INT_ENABLE |
					     GPIO_INT_ENABLE_DISABLE_ONLY));
#else
	mode = (enum gpio_int_mode)(flags & (GPIO_INT_EDGE | GPIO_INT_DISABLE | GPIO_INT_ENABLE));
#endif /* CONFIG_GPIO_ENABLE_DISABLE_INTERRUPT */

	ret = api->pin_interrupt_configure(port, pin, mode, trig);
	SYS_PORT_TRACING_FUNC_EXIT(gpio_pin, interrupt_configure, port, pin, ret);
	return ret;
}


/**
 * @brief Configure a single pin.
 *
 * @param[in] port Pointer to device structure for the driver instance.
 * @param[in] pin Pin number to configure.
 * @param[in] flags Flags for pin configuration: 'GPIO input/output configuration
 *        flags', 'GPIO pin drive flags', 'GPIO pin bias flags'.
 *
 * @retval 0 If successful.
 * @retval -ENOTSUP if any of the configuration options is not supported
 *                  (unless otherwise directed by flag documentation).
 * @retval -EINVAL Invalid argument.
 * @retval -EIO I/O error when accessing an external GPIO chip.
 * @retval -EWOULDBLOCK if operation would block.
 */
static inline int gpio_pin_configure(const struct device *port,
					    gpio_pin_t pin,
					    gpio_flags_t flags)
{
	const struct gpio_driver_api *api = DEVICE_API_GET(gpio, port);
	__unused const struct gpio_driver_config *const cfg =
		(const struct gpio_driver_config *)port->config;
	struct gpio_driver_data *data =
		(struct gpio_driver_data *)port->data;
	int ret;

	SYS_PORT_TRACING_FUNC_ENTER(gpio_pin, configure, port, pin, flags);

	__ASSERT((flags & GPIO_INT_MASK) == 0,
		 "Interrupt flags are not supported");

	__ASSERT((flags & (GPIO_PULL_UP | GPIO_PULL_DOWN)) !=
		 (GPIO_PULL_UP | GPIO_PULL_DOWN),
		 "Pull Up and Pull Down should not be enabled simultaneously");

	__ASSERT(!((flags & GPIO_INPUT) && !(flags & GPIO_OUTPUT) && (flags & GPIO_SINGLE_ENDED)),
		 "Input cannot be enabled for 'Open Drain', 'Open Source' modes without Output");

	__ASSERT_NO_MSG((flags & GPIO_SINGLE_ENDED) != 0 ||
			(flags & GPIO_LINE_OPEN_DRAIN) == 0);

	__ASSERT((flags & (GPIO_OUTPUT_INIT_LOW | GPIO_OUTPUT_INIT_HIGH)) == 0
		 || (flags & GPIO_OUTPUT) != 0,
		 "Output needs to be enabled to be initialized low or high");

	__ASSERT((flags & (GPIO_OUTPUT_INIT_LOW | GPIO_OUTPUT_INIT_HIGH))
		 != (GPIO_OUTPUT_INIT_LOW | GPIO_OUTPUT_INIT_HIGH),
		 "Output cannot be initialized low and high");

	if (((flags & GPIO_OUTPUT_INIT_LOGICAL) != 0)
	    && ((flags & (GPIO_OUTPUT_INIT_LOW | GPIO_OUTPUT_INIT_HIGH)) != 0)
	    && ((flags & GPIO_ACTIVE_LOW) != 0)) {
		flags ^= GPIO_OUTPUT_INIT_LOW | GPIO_OUTPUT_INIT_HIGH;
	}

	flags &= ~GPIO_OUTPUT_INIT_LOGICAL;

	__ASSERT(gpio_port_pin_is_supported(cfg->port_pin_mask, pin), "Unsupported pin");

	/*
	 * pin is passed on to the driver as-is, where it is commonly used as an
	 * array index, so it must be range checked even when assertions are
	 * compiled out.
	 */
	if (pin >= GPIO_MAX_PINS_PER_PORT) {
		SYS_PORT_TRACING_FUNC_EXIT(gpio_pin, configure, port, pin, -EINVAL);
		return -EINVAL;
	}

	if ((flags & GPIO_ACTIVE_LOW) != 0) {
		data->invert |= (gpio_port_pins_t)BIT(pin);
	} else {
		data->invert &= ~(gpio_port_pins_t)BIT(pin);
	}

	ret = api->pin_configure(port, pin, flags);
	SYS_PORT_TRACING_FUNC_EXIT(gpio_pin, configure, port, pin, ret);
	return ret;
}


/**
 * @brief Get direction of select pins in a port.
 *
 * Retrieve direction of each pin specified in @p map.
 *
 * If @p inputs or @p outputs is NULL, then this function does not get the
 * respective input or output direction information.
 *
 * @param port Pointer to the device structure for the driver instance.
 * @param map Bitmap of pin directions to query.
 * @param inputs Pointer to a variable where input directions will be stored.
 * @param outputs Pointer to a variable where output directions will be stored.
 *
 * @retval 0 If successful.
 * @retval -ENOSYS if the underlying driver does not support this call.
 * @retval -EIO I/O error when accessing an external GPIO chip.
 * @retval -EWOULDBLOCK if operation would block.
 */
#ifdef CONFIG_GPIO_GET_DIRECTION
static inline int gpio_port_get_direction(const struct device *port, gpio_port_pins_t map,
						 gpio_port_pins_t *inputs,
						 gpio_port_pins_t *outputs)
{
	const struct gpio_driver_api *api = DEVICE_API_GET(gpio, port);
	int ret;

	SYS_PORT_TRACING_FUNC_ENTER(gpio_port, get_direction, port, map, inputs, outputs);

	if (api->port_get_direction == NULL) {
		SYS_PORT_TRACING_FUNC_EXIT(gpio_port, get_direction, port, -ENOSYS);
		return -ENOSYS;
	}

	ret = api->port_get_direction(port, map, inputs, outputs);
	SYS_PORT_TRACING_FUNC_EXIT(gpio_port, get_direction, port, ret);
	return ret;
}
#else
static inline int gpio_port_get_direction(const struct device *port, gpio_port_pins_t map,
					  gpio_port_pins_t *inputs,
					  gpio_port_pins_t *outputs)
{
	ARG_UNUSED(port);
	ARG_UNUSED(map);
	ARG_UNUSED(inputs);
	ARG_UNUSED(outputs);
	return -ENOSYS;
}
#endif /* CONFIG_GPIO_GET_DIRECTION */

/**
 * @brief Check if @p pin is configured for input
 *
 * @param port Pointer to device structure for the driver instance.
 * @param pin Pin number to query the direction of
 *
 * @retval 1 if @p pin is configured as @ref GPIO_INPUT.
 * @retval 0 if @p pin is not configured as @ref GPIO_INPUT.
 * @retval -ENOSYS if the underlying driver does not support this call.
 * @retval -EIO I/O error when accessing an external GPIO chip.
 * @retval -EWOULDBLOCK if operation would block.
 */
static inline int gpio_pin_is_input(const struct device *port, gpio_pin_t pin)
{
	int rv;
	gpio_port_pins_t pins;
	__unused const struct gpio_driver_config *cfg =
		(const struct gpio_driver_config *)port->config;

	__ASSERT(gpio_port_pin_is_supported(cfg->port_pin_mask, pin), "Unsupported pin");

	rv = gpio_port_get_direction(port, BIT(pin), &pins, NULL);
	if (rv < 0) {
		return rv;
	}

	return (int)!!((gpio_port_pins_t)BIT(pin) & pins);
}


/**
 * @brief Check if @p pin is configured for output
 *
 * @param port Pointer to device structure for the driver instance.
 * @param pin Pin number to query the direction of
 *
 * @retval 1 if @p pin is configured as @ref GPIO_OUTPUT.
 * @retval 0 if @p pin is not configured as @ref GPIO_OUTPUT.
 * @retval -ENOSYS if the underlying driver does not support this call.
 * @retval -EIO I/O error when accessing an external GPIO chip.
 * @retval -EWOULDBLOCK if operation would block.
 */
static inline int gpio_pin_is_output(const struct device *port, gpio_pin_t pin)
{
	int rv;
	gpio_port_pins_t pins;
	__unused const struct gpio_driver_config *cfg =
		(const struct gpio_driver_config *)port->config;

	__ASSERT(gpio_port_pin_is_supported(cfg->port_pin_mask, pin), "Unsupported pin");

	rv = gpio_port_get_direction(port, BIT(pin), NULL, &pins);
	if (rv < 0) {
		return rv;
	}

	return (int)!!((gpio_port_pins_t)BIT(pin) & pins);
}


/**
 * @brief Get a configuration of a single pin.
 *
 * @param port Pointer to device structure for the driver instance.
 * @param pin Pin number which configuration is get.
 * @param flags Pointer to variable in which the current configuration will
 *              be stored if function is successful.
 *
 * @retval 0 If successful.
 * @retval -ENOSYS if getting current pin configuration is not implemented
 *                  by the driver.
 * @retval -EINVAL Invalid argument.
 * @retval -EIO I/O error when accessing an external GPIO chip.
 * @retval -EWOULDBLOCK if operation would block.
 */
#ifdef CONFIG_GPIO_GET_CONFIG
static inline int gpio_pin_get_config(const struct device *port,
					     gpio_pin_t pin,
					     gpio_flags_t *flags)
{
	const struct gpio_driver_api *api = DEVICE_API_GET(gpio, port);
	int ret;

	SYS_PORT_TRACING_FUNC_ENTER(gpio_pin, get_config, port, pin, *flags);

	if (api->pin_get_config == NULL) {
		SYS_PORT_TRACING_FUNC_EXIT(gpio_pin, get_config, port, pin, -ENOSYS);
		return -ENOSYS;
	}

	/* See the note in gpio_pin_configure(). */
	if (pin >= GPIO_MAX_PINS_PER_PORT) {
		SYS_PORT_TRACING_FUNC_EXIT(gpio_pin, get_config, port, pin, -EINVAL);
		return -EINVAL;
	}

	ret = api->pin_get_config(port, pin, flags);
	SYS_PORT_TRACING_FUNC_EXIT(gpio_pin, get_config, port, pin, ret);
	return ret;
}
#else
static inline int gpio_pin_get_config(const struct device *port, gpio_pin_t pin,
				      gpio_flags_t *flags)
{
	ARG_UNUSED(port);
	ARG_UNUSED(pin);
	ARG_UNUSED(flags);
	return -ENOSYS;
}
#endif


/**
 * @brief Get physical level of all input pins in a port.
 *
 * A low physical level on the pin will be interpreted as value 0. A high
 * physical level will be interpreted as value 1. This function ignores
 * GPIO_ACTIVE_LOW flag.
 *
 * Value of a pin with index n will be represented by bit n in the returned
 * port value.
 *
 * @param port Pointer to the device structure for the driver instance.
 * @param value Pointer to a variable where pin values will be stored.
 *
 * @retval 0 If successful.
 * @retval -EIO I/O error when accessing an external GPIO chip.
 * @retval -EWOULDBLOCK if operation would block.
 */
static inline int gpio_port_get_raw(const struct device *port, gpio_port_value_t *value)
{
	const struct gpio_driver_api *api = DEVICE_API_GET(gpio, port);
	int ret;

	SYS_PORT_TRACING_FUNC_ENTER(gpio_port, get_raw, port, value);

	ret = api->port_get_raw(port, value);
	SYS_PORT_TRACING_FUNC_EXIT(gpio_port, get_raw, port, ret);
	return ret;
}

/**
 * @brief Get logical level of all input pins in a port.
 *
 * Get logical level of an input pin taking into account GPIO_ACTIVE_LOW flag.
 * If pin is configured as Active High, a low physical level will be interpreted
 * as logical value 0. If pin is configured as Active Low, a low physical level
 * will be interpreted as logical value 1.
 *
 * Value of a pin with index n will be represented by bit n in the returned
 * port value.
 *
 * @param port Pointer to the device structure for the driver instance.
 * @param value Pointer to a variable where pin values will be stored.
 *
 * @retval 0 If successful.
 * @retval -EIO I/O error when accessing an external GPIO chip.
 * @retval -EWOULDBLOCK if operation would block.
 */
static inline int gpio_port_get(const struct device *port,
				       gpio_port_value_t *value)
{
	const struct gpio_driver_data *const data =
			(const struct gpio_driver_data *)port->data;
	int ret;

	ret = gpio_port_get_raw(port, value);
	if (ret == 0) {
		*value ^= data->invert;
	}

	return ret;
}

/**
 * @brief Set physical level of output pins in a port.
 *
 * Writing value 0 to the pin will set it to a low physical level. Writing
 * value 1 will set it to a high physical level. This function ignores
 * GPIO_ACTIVE_LOW flag.
 *
 * Pin with index n is represented by bit n in mask and value parameter.
 *
 * @param port Pointer to the device structure for the driver instance.
 * @param mask Mask indicating which pins will be modified.
 * @param value Value assigned to the output pins.
 *
 * @retval 0 If successful.
 * @retval -EIO I/O error when accessing an external GPIO chip.
 * @retval -EWOULDBLOCK if operation would block.
 */
static inline int gpio_port_set_masked_raw(const struct device *port,
						  gpio_port_pins_t mask,
						  gpio_port_value_t value)
{
	const struct gpio_driver_api *api = DEVICE_API_GET(gpio, port);
	int ret;

	SYS_PORT_TRACING_FUNC_ENTER(gpio_port, set_masked_raw, port, mask, value);

	ret = api->port_set_masked_raw(port, mask, value);
	SYS_PORT_TRACING_FUNC_EXIT(gpio_port, set_masked_raw, port, ret);
	return ret;
}

/**
 * @brief Set logical level of output pins in a port.
 *
 * Set logical level of an output pin taking into account GPIO_ACTIVE_LOW flag.
 * Value 0 sets the pin in logical 0 / inactive state. Value 1 sets the pin in
 * logical 1 / active state. If pin is configured as Active High, the default,
 * setting it in inactive state will force the pin to a low physical level. If
 * pin is configured as Active Low, setting it in inactive state will force the
 * pin to a high physical level.
 *
 * Pin with index n is represented by bit n in mask and value parameter.
 *
 * @param port Pointer to the device structure for the driver instance.
 * @param mask Mask indicating which pins will be modified.
 * @param value Value assigned to the output pins.
 *
 * @retval 0 If successful.
 * @retval -EIO I/O error when accessing an external GPIO chip.
 * @retval -EWOULDBLOCK if operation would block.
 */
static inline int gpio_port_set_masked(const struct device *port,
					      gpio_port_pins_t mask,
					      gpio_port_value_t value)
{
	const struct gpio_driver_data *const data =
			(const struct gpio_driver_data *)port->data;

	value ^= data->invert;

	return gpio_port_set_masked_raw(port, mask, value);
}

/**
 * @brief Set physical level of selected output pins to high.
 *
 * @param port Pointer to the device structure for the driver instance.
 * @param pins Value indicating which pins will be modified.
 *
 * @retval 0 If successful.
 * @retval -EIO I/O error when accessing an external GPIO chip.
 * @retval -EWOULDBLOCK if operation would block.
 */
static inline int gpio_port_set_bits_raw(const struct device *port,
						gpio_port_pins_t pins)
{
	const struct gpio_driver_api *api = DEVICE_API_GET(gpio, port);
	int ret;

	SYS_PORT_TRACING_FUNC_ENTER(gpio_port, set_bits_raw, port, pins);

	ret = api->port_set_bits_raw(port, pins);
	SYS_PORT_TRACING_FUNC_EXIT(gpio_port, set_bits_raw, port, ret);
	return ret;
}

/**
 * @brief Set logical level of selected output pins to active.
 *
 * @param port Pointer to the device structure for the driver instance.
 * @param pins Value indicating which pins will be modified.
 *
 * @retval 0 If successful.
 * @retval -EIO I/O error when accessing an external GPIO chip.
 * @retval -EWOULDBLOCK if operation would block.
 */
static inline int gpio_port_set_bits(const struct device *port,
					    gpio_port_pins_t pins)
{
	return gpio_port_set_masked(port, pins, pins);
}

/**
 * @brief Set physical level of selected output pins to low.
 *
 * @param port Pointer to the device structure for the driver instance.
 * @param pins Value indicating which pins will be modified.
 *
 * @retval 0 If successful.
 * @retval -EIO I/O error when accessing an external GPIO chip.
 * @retval -EWOULDBLOCK if operation would block.
 */
static inline int gpio_port_clear_bits_raw(const struct device *port,
						  gpio_port_pins_t pins)
{
	const struct gpio_driver_api *api = DEVICE_API_GET(gpio, port);
	int ret;

	SYS_PORT_TRACING_FUNC_ENTER(gpio_port, clear_bits_raw, port, pins);

	ret = api->port_clear_bits_raw(port, pins);
	SYS_PORT_TRACING_FUNC_EXIT(gpio_port, clear_bits_raw, port, ret);
	return ret;
}

/**
 * @brief Set logical level of selected output pins to inactive.
 *
 * @param port Pointer to the device structure for the driver instance.
 * @param pins Value indicating which pins will be modified.
 *
 * @retval 0 If successful.
 * @retval -EIO I/O error when accessing an external GPIO chip.
 * @retval -EWOULDBLOCK if operation would block.
 */
static inline int gpio_port_clear_bits(const struct device *port,
					      gpio_port_pins_t pins)
{
	return gpio_port_set_masked(port, pins, 0);
}

/**
 * @brief Toggle level of selected output pins.
 *
 * @param port Pointer to the device structure for the driver instance.
 * @param pins Value indicating which pins will be modified.
 *
 * @retval 0 If successful.
 * @retval -EIO I/O error when accessing an external GPIO chip.
 * @retval -EWOULDBLOCK if operation would block.
 */
static inline int gpio_port_toggle_bits(const struct device *port,
					       gpio_port_pins_t pins)
{
	const struct gpio_driver_api *api = DEVICE_API_GET(gpio, port);
	int ret;

	SYS_PORT_TRACING_FUNC_ENTER(gpio_port, toggle_bits, port, pins);

	ret = api->port_toggle_bits(port, pins);
	SYS_PORT_TRACING_FUNC_EXIT(gpio_port, toggle_bits, port, ret);
	return ret;
}

/**
 * @brief Set physical level of selected output pins.
 *
 * @param port Pointer to the device structure for the driver instance.
 * @param set_pins Value indicating which pins will be set to high.
 * @param clear_pins Value indicating which pins will be set to low.
 *
 * @retval 0 If successful.
 * @retval -EIO I/O error when accessing an external GPIO chip.
 * @retval -EWOULDBLOCK if operation would block.
 */
static inline int gpio_port_set_clr_bits_raw(const struct device *port,
						    gpio_port_pins_t set_pins,
						    gpio_port_pins_t clear_pins)
{
	__ASSERT((set_pins & clear_pins) == 0, "Set and Clear pins overlap");

	return gpio_port_set_masked_raw(port, set_pins | clear_pins, set_pins);
}

/**
 * @brief Set logical level of selected output pins.
 *
 * @param port Pointer to the device structure for the driver instance.
 * @param set_pins Value indicating which pins will be set to active.
 * @param clear_pins Value indicating which pins will be set to inactive.
 *
 * @retval 0 If successful.
 * @retval -EIO I/O error when accessing an external GPIO chip.
 * @retval -EWOULDBLOCK if operation would block.
 */
static inline int gpio_port_set_clr_bits(const struct device *port,
						gpio_port_pins_t set_pins,
						gpio_port_pins_t clear_pins)
{
	__ASSERT((set_pins & clear_pins) == 0, "Set and Clear pins overlap");

	return gpio_port_set_masked(port, set_pins | clear_pins, set_pins);
}

/**
 * @brief Get physical level of an input pin.
 *
 * A low physical level on the pin will be interpreted as value 0. A high
 * physical level will be interpreted as value 1. This function ignores
 * GPIO_ACTIVE_LOW flag.
 *
 * @param port Pointer to the device structure for the driver instance.
 * @param pin Pin number.
 *
 * @retval 1 If pin physical level is high.
 * @retval 0 If pin physical level is low.
 * @retval -EIO I/O error when accessing an external GPIO chip.
 * @retval -EWOULDBLOCK if operation would block.
 */
static inline int gpio_pin_get_raw(const struct device *port, gpio_pin_t pin)
{
	__unused const struct gpio_driver_config *const cfg =
		(const struct gpio_driver_config *)port->config;
	gpio_port_value_t value;
	int ret;

	__ASSERT(gpio_port_pin_is_supported(cfg->port_pin_mask, pin), "Unsupported pin");

	ret = gpio_port_get_raw(port, &value);
	if (ret == 0) {
		ret = (value & (gpio_port_pins_t)BIT(pin)) != 0 ? 1 : 0;
	}

	return ret;
}

/**
 * @brief Get logical level of an input pin.
 *
 * Get logical level of an input pin taking into account GPIO_ACTIVE_LOW flag.
 * If pin is configured as Active High, a low physical level will be interpreted
 * as logical value 0. If pin is configured as Active Low, a low physical level
 * will be interpreted as logical value 1.
 *
 * Note: If pin is configured as Active High, the default, gpio_pin_get()
 *       function is equivalent to gpio_pin_get_raw().
 *
 * @param port Pointer to the device structure for the driver instance.
 * @param pin Pin number.
 *
 * @retval 1 If pin logical value is 1 / active.
 * @retval 0 If pin logical value is 0 / inactive.
 * @retval -EIO I/O error when accessing an external GPIO chip.
 * @retval -EWOULDBLOCK if operation would block.
 */
static inline int gpio_pin_get(const struct device *port, gpio_pin_t pin)
{
	__unused const struct gpio_driver_config *const cfg =
		(const struct gpio_driver_config *)port->config;
	gpio_port_value_t value;
	int ret;

	__ASSERT(gpio_port_pin_is_supported(cfg->port_pin_mask, pin), "Unsupported pin");

	ret = gpio_port_get(port, &value);
	if (ret == 0) {
		ret = (value & (gpio_port_pins_t)BIT(pin)) != 0 ? 1 : 0;
	}

	return ret;
}


/**
 * @brief Set physical level of an output pin.
 *
 * Writing value 0 to the pin will set it to a low physical level. Writing any
 * value other than 0 will set it to a high physical level. This function
 * ignores GPIO_ACTIVE_LOW flag.
 *
 * @param port Pointer to the device structure for the driver instance.
 * @param pin Pin number.
 * @param value Value assigned to the pin.
 *
 * @retval 0 If successful.
 * @retval -EIO I/O error when accessing an external GPIO chip.
 * @retval -EWOULDBLOCK if operation would block.
 */
static inline int gpio_pin_set_raw(const struct device *port, gpio_pin_t pin,
					  int value)
{
	__unused const struct gpio_driver_config *const cfg =
		(const struct gpio_driver_config *)port->config;
	int ret;

	__ASSERT(gpio_port_pin_is_supported(cfg->port_pin_mask, pin), "Unsupported pin");

	if (value != 0)	{
		ret = gpio_port_set_bits_raw(port, (gpio_port_pins_t)BIT(pin));
	} else {
		ret = gpio_port_clear_bits_raw(port, (gpio_port_pins_t)BIT(pin));
	}

	return ret;
}

/**
 * @brief Set logical level of an output pin.
 *
 * Set logical level of an output pin taking into account GPIO_ACTIVE_LOW flag.
 * Value 0 sets the pin in logical 0 / inactive state. Any value other than 0
 * sets the pin in logical 1 / active state. If pin is configured as Active
 * High, the default, setting it in inactive state will force the pin to a low
 * physical level. If pin is configured as Active Low, setting it in inactive
 * state will force the pin to a high physical level.
 *
 * Note: If pin is configured as Active High, gpio_pin_set() function is
 *       equivalent to gpio_pin_set_raw().
 *
 * @param port Pointer to the device structure for the driver instance.
 * @param pin Pin number.
 * @param value Value assigned to the pin.
 *
 * @retval 0 If successful.
 * @retval -EIO I/O error when accessing an external GPIO chip.
 * @retval -EWOULDBLOCK if operation would block.
 */
static inline int gpio_pin_set(const struct device *port, gpio_pin_t pin,
				      int value)
{
	__unused const struct gpio_driver_config *const cfg =
		(const struct gpio_driver_config *)port->config;
	const struct gpio_driver_data *const data =
			(const struct gpio_driver_data *)port->data;

	__ASSERT(gpio_port_pin_is_supported(cfg->port_pin_mask, pin), "Unsupported pin");

	if (data->invert & (gpio_port_pins_t)BIT(pin)) {
		value = (value != 0) ? 0 : 1;
	}

	return gpio_pin_set_raw(port, pin, value);
}


/**
 * @brief Toggle pin level.
 *
 * @param port Pointer to the device structure for the driver instance.
 * @param pin Pin number.
 *
 * @retval 0 If successful.
 * @retval -EIO I/O error when accessing an external GPIO chip.
 * @retval -EWOULDBLOCK if operation would block.
 */
static inline int gpio_pin_toggle(const struct device *port, gpio_pin_t pin)
{
	__unused const struct gpio_driver_config *const cfg =
		(const struct gpio_driver_config *)port->config;

	__ASSERT(gpio_port_pin_is_supported(cfg->port_pin_mask, pin), "Unsupported pin");

	return gpio_port_toggle_bits(port, (gpio_port_pins_t)BIT(pin));
}


/**
 * @brief Helper to initialize a struct gpio_callback properly
 * @param callback A valid Application's callback structure pointer.
 * @param handler A valid handler function pointer.
 * @param pin_mask A bit mask of relevant pins for the handler
 */
static inline void gpio_init_callback(struct gpio_callback *callback,
				      gpio_callback_handler_t handler,
				      gpio_port_pins_t pin_mask)
{
	SYS_PORT_TRACING_FUNC_ENTER(gpio, init_callback, callback, handler, pin_mask);

	__ASSERT(callback, "Callback pointer should not be NULL");
	__ASSERT(handler, "Callback handler pointer should not be NULL");

	callback->handler = handler;
	callback->pin_mask = pin_mask;

	SYS_PORT_TRACING_FUNC_EXIT(gpio, init_callback, callback);
}

/**
 * @brief Add an application callback.
 * @param port Pointer to the device structure for the driver instance.
 * @param callback A valid Application's callback structure pointer.
 * @return 0 on success, negative errno value on failure.
 * @retval -ENOSYS Driver does not implement the operation.
 *
 * @note Callbacks may be added to the device from within a callback
 * handler invocation, but whether they are invoked for the current
 * GPIO event is not specified.
 *
 * Note: enables to add as many callback as needed on the same port.
 */
static inline int gpio_add_callback(const struct device *port,
				    struct gpio_callback *callback)
{
	const struct gpio_driver_api *api = DEVICE_API_GET(gpio, port);
	int ret;

	SYS_PORT_TRACING_FUNC_ENTER(gpio, add_callback, port, callback);

	if (api->manage_callback == NULL) {
		SYS_PORT_TRACING_FUNC_EXIT(gpio, add_callback, port, -ENOSYS);
		return -ENOSYS;
	}

	ret = api->manage_callback(port, callback, true);
	SYS_PORT_TRACING_FUNC_EXIT(gpio, add_callback, port, ret);
	return ret;
}


/**
 * @brief Remove an application callback.
 * @param port Pointer to the device structure for the driver instance.
 * @param callback A valid application's callback structure pointer.
 * @return 0 on success, negative errno value on failure.
 * @retval -ENOSYS Driver does not implement the operation.
 *
 * @warning It is explicitly permitted, within a callback handler, to
 * remove the registration for the callback that is running, i.e. @p
 * callback.  Attempts to remove other registrations on the same
 * device may result in undefined behavior, including failure to
 * invoke callbacks that remain registered and unintended invocation
 * of removed callbacks.
 *
 * Note: enables to remove as many callbacks as added through
 *       gpio_add_callback().
 */
static inline int gpio_remove_callback(const struct device *port,
				       struct gpio_callback *callback)
{
	const struct gpio_driver_api *api = DEVICE_API_GET(gpio, port);
	int ret;

	SYS_PORT_TRACING_FUNC_ENTER(gpio, remove_callback, port, callback);

	if (api->manage_callback == NULL) {
		SYS_PORT_TRACING_FUNC_EXIT(gpio, remove_callback, port, -ENOSYS);
		return -ENOSYS;
	}

	ret = api->manage_callback(port, callback, false);
	SYS_PORT_TRACING_FUNC_EXIT(gpio, remove_callback, port, ret);
	return ret;
}


/**
 * @brief Function to get pending interrupts
 *
 * The purpose of this function is to return the interrupt
 * status register for the device.
 * This is especially useful when waking up from
 * low power states to check the wake up source.
 *
 * @param dev Pointer to the device structure for the driver instance.
 *
 * @return status != 0 if at least one gpio interrupt is pending.
 * @retval 0 if no gpio interrupt is pending.
 * @retval -ENOSYS If driver does not implement the operation
 */
static inline int gpio_get_pending_int(const struct device *dev)
{
	const struct gpio_driver_api *api = DEVICE_API_GET(gpio, dev);
	int ret;

	SYS_PORT_TRACING_FUNC_ENTER(gpio, get_pending_int, dev);

	if (api->get_pending_int == NULL) {
		SYS_PORT_TRACING_FUNC_EXIT(gpio, get_pending_int, dev, -ENOSYS);
		return -ENOSYS;
	}

	ret = api->get_pending_int(dev);
	SYS_PORT_TRACING_FUNC_EXIT(gpio, get_pending_int, dev, ret);
	return ret;
}

/**
 * @}
 */

#ifdef __cplusplus
}
#endif


#endif /* ZEPHYR_INCLUDE_DRIVERS_GPIO_H_ */
