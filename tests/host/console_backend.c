/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Host console log backend. Minimal v4.5.0 log_backend_api implementation:
 * drops messages (the host tests do not assert on log output). Kept so LOG_*
 * calls link and the log_backend iterable section is populated.
 */

#include <logging/log_backend.h>

static void process(const struct log_backend *const backend, union log_msg_generic *msg)
{
	(void)backend;
	(void)msg;
}

static void panic(const struct log_backend *const backend)
{
	(void)backend;
}

static const struct log_backend_api host_console_api = {
	.process = process,
	.panic = panic,
};

LOG_BACKEND_DEFINE(console, host_console_api, false);
