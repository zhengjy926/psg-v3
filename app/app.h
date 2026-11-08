/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * app — application layer. Host-clean: must never include STM32 or other MCU
 * headers. It composes the services and owns the main-loop body; it never
 * calls device_get_binding() (the composition root binds devices first).
 */

#ifndef PSG_APP_APP_H_
#define PSG_APP_APP_H_

#include <stdint.h>

#include "service/heartbeat/heartbeat.h"

struct app_config {
	struct heartbeat_config heartbeat;
};

/** Build the running application from an already-bound configuration. */
void app_init(const struct app_config *cfg);

/** Advance the application one main-loop step. */
void app_step(void);

#endif /* PSG_APP_APP_H_ */
