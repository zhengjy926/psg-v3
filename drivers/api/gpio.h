/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * gpio driver API. This is the host-clean seam: it must never include STM32 or
 * other MCU headers. Target and host (fake) drivers implement the same ops.
 */

#ifndef PSG_DRIVERS_API_GPIO_H_
#define PSG_DRIVERS_API_GPIO_H_

#include <stdint.h>
#include <zephyr/device.h>

#define GPIO_OUTPUT 1U
#define GPIO_INPUT 0U

#define GPIO_HIGH 1U
#define GPIO_LOW 0U

struct gpio_driver_api {
	int (*configure)(const struct device *dev, uint32_t pin, uint32_t mode);
	int (*set)(const struct device *dev, uint32_t pin, uint32_t value);
	int (*get)(const struct device *dev, uint32_t pin);
};

static inline int gpio_configure(const struct device *dev, uint32_t pin,
				 uint32_t mode)
{
	const struct gpio_driver_api *api = DEVICE_API_GET(gpio, dev);

	return api->configure(dev, pin, mode);
}

static inline int gpio_set(const struct device *dev, uint32_t pin,
			   uint32_t value)
{
	const struct gpio_driver_api *api = DEVICE_API_GET(gpio, dev);

	return api->set(dev, pin, value);
}

static inline int gpio_get(const struct device *dev, uint32_t pin)
{
	const struct gpio_driver_api *api = DEVICE_API_GET(gpio, dev);

	return api->get(dev, pin);
}

#endif /* PSG_DRIVERS_API_GPIO_H_ */
