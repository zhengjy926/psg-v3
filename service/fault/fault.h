/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * fault — the device fault word (GLOSSARY: 故障字). Host-clean leaf module:
 * not one of the six business services. app, services, and the board-layer
 * composition root (which owns the "all devices ready" final check) may
 * set/clear/read it.
 * bit0 = board not ready (a required device failed init, #5 §4); bit1 is
 * reserved for the over-current latch (a later ticket).
 */

#ifndef PSG_SERVICE_FAULT_FAULT_H_
#define PSG_SERVICE_FAULT_FAULT_H_

#include <stdint.h>

/* Board not ready: a required device did not reach device_is_ready(). */
#define FAULT_BIT_NOT_READY 0U
/* Reserved: over-current latch hardware fault (later ticket). */
#define FAULT_BIT_OVERCURRENT_LATCH 1U

/** Set (OR) fault bits. */
void fault_set(uint32_t bits);
/** Clear fault bits. */
void fault_clear(uint32_t bits);
/** Read the current fault word. */
uint32_t fault_get(void);

#endif /* PSG_SERVICE_FAULT_FAULT_H_ */
