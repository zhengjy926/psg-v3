/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 * SPDX-License-Identifier: Apache-2.0
 *
 * Bare-metal log core (MINIMAL path). Messages are formatted with a small
 * self-contained formatter (no libc printf) and emitted through every
 * registered backend. Derived from Zephyr v4.5.0 subsys/logging/log_minimal.c.
 */

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>
#include <zephyr/logging/log_backend.h>
#include <zephyr/sys/iterable_sections.h>

#if defined(__ARMCOMPILER_VERSION)
extern char Image$$ER_LOG_BACKEND$$Base;
extern char Image$$ER_LOG_BACKEND$$Limit;
#define Z_LOG_BACKEND_START                                                    \
	((const struct log_backend *)&Image$$ER_LOG_BACKEND$$Base)
#define Z_LOG_BACKEND_END ((const struct log_backend *)&Image$$ER_LOG_BACKEND$$Limit)
#else
extern const struct log_backend _log_backend_list_start[];
extern const struct log_backend _log_backend_list_end[];
#define Z_LOG_BACKEND_START _log_backend_list_start
#define Z_LOG_BACKEND_END _log_backend_list_end
#endif

#define Z_LOG_LINE_MAX 128U

/* Emit one character, always reserving the final byte for the NUL. */
static void z_emit(char **dst, size_t *rem, char c)
{
	if (*rem > 1U) {
		**dst = c;
		(*dst)++;
		(*rem)--;
	}
}

static void z_emit_uint(char **dst, size_t *rem, unsigned int value,
			unsigned int base, int upper)
{
	char tmp[11];
	int i = 0;
	const char *digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";

	if (value == 0U) {
		z_emit(dst, rem, '0');
		return;
	}
	while (value != 0U) {
		tmp[i] = digits[value % base];
		i++;
		value /= base;
	}
	while (i > 0) {
		i--;
		z_emit(dst, rem, tmp[i]);
	}
}

static void z_emit_int(char **dst, size_t *rem, int value)
{
	if (value < 0) {
		z_emit(dst, rem, '-');
		z_emit_uint(dst, rem, (unsigned int)(-(value + 1)) + 1U, 10, 0);
	} else {
		z_emit_uint(dst, rem, (unsigned int)value, 10, 0);
	}
}

/* Minimal formatter: %s %c %d %i %u %x %X %p %%. */
static void z_vformat(char **dst, size_t *rem, const char *fmt, va_list ap)
{
	while (*fmt != '\0') {
		if (*fmt != '%') {
			z_emit(dst, rem, *fmt);
			fmt++;
			continue;
		}
		fmt++;
		if (*fmt == '\0') {
			break;
		}
		if (*fmt == '%') {
			z_emit(dst, rem, '%');
			fmt++;
			continue;
		}
		switch (*fmt) {
		case 's': {
			const char *s = va_arg(ap, const char *);

			if (s == NULL) {
				s = "(null)";
			}
			while (*s != '\0') {
				z_emit(dst, rem, *s);
				s++;
			}
			break;
		}
		case 'c':
			z_emit(dst, rem, (char)va_arg(ap, int));
			break;
		case 'd':
		case 'i':
			z_emit_int(dst, rem, va_arg(ap, int));
			break;
		case 'u':
			z_emit_uint(dst, rem, va_arg(ap, unsigned int), 10, 0);
			break;
		case 'x':
			z_emit_uint(dst, rem, va_arg(ap, unsigned int), 16, 0);
			break;
		case 'X':
			z_emit_uint(dst, rem, va_arg(ap, unsigned int), 16, 1);
			break;
		case 'p':
			z_emit(dst, rem, '0');
			z_emit(dst, rem, 'x');
			z_emit_uint(dst, rem, (uintptr_t)va_arg(ap, void *), 16, 0);
			break;
		default:
			z_emit(dst, rem, *fmt);
			break;
		}
		fmt++;
	}
	**dst = '\0';
}

static const char *z_level_char(uint32_t level)
{
	switch (level) {
	case LOG_LEVEL_ERR:
		return "E";
	case LOG_LEVEL_WRN:
		return "W";
	case LOG_LEVEL_INF:
		return "I";
	case LOG_LEVEL_DBG:
		return "D";
	default:
		return "?";
	}
}

void z_log_printf(const struct log_source_const_data *src, uint32_t level,
		  const char *fmt, ...)
{
	char line[Z_LOG_LINE_MAX];
	char *dst = line;
	size_t rem = sizeof(line);
	size_t used;
	const struct log_backend *backend;
	const char *name;
	va_list ap;

	/* Prefix: "[<level> <module>] ". */
	z_emit(&dst, &rem, '[');
	z_emit(&dst, &rem, z_level_char(level)[0]);
	z_emit(&dst, &rem, ' ');
	name = (src != NULL) ? src->name : NULL;
	if (name == NULL) {
		name = "?";
	}
	while (*name != '\0') {
		z_emit(&dst, &rem, *name);
		name++;
	}
	z_emit(&dst, &rem, ']');
	z_emit(&dst, &rem, ' ');

	va_start(ap, fmt);
	z_vformat(&dst, &rem, fmt, ap);
	va_end(ap);

	used = sizeof(line) - rem;

	for (backend = Z_LOG_BACKEND_START; backend < Z_LOG_BACKEND_END;
	     backend++) {
		if ((backend->api != NULL) && (backend->api->output != NULL)) {
			backend->api->output(line, used);
		}
	}
}

void log_core_init(void)
{
	const struct log_backend *backend;

	for (backend = Z_LOG_BACKEND_START; backend < Z_LOG_BACKEND_END;
	     backend++) {
		if ((backend->api != NULL) && (backend->api->init != NULL)) {
			backend->api->init();
		}
	}
}

/* Initialize logging at boot (EARLY, before drivers), on target and host. */
static int z_log_init_entry(void)
{
	log_core_init();
	return 0;
}

SYS_INIT(z_log_init_entry, EARLY, 0);
