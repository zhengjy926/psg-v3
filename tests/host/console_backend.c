/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Host console log backend: emits formatted log lines to stdout.
 */

#include <zephyr/logging/log_backend.h>

#include <stdio.h>

static void host_output(const char *data, size_t length)
{
	(void)fwrite(data, 1, length, stdout);
	(void)fflush(stdout);
}

static void host_init(void)
{
}

static const struct log_backend_api host_console_api = {
	.init = host_init,
	.output = host_output,
};

LOG_BACKEND_DEFINE(console, &host_console_api);
