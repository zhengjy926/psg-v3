/*
 * Copyright (c) 2018 Intel Corporation.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Minimal errno subset for the bare-metal port. Derived from Zephyr's
 * include/zephyr/sys/errno_private.h (values match the Linux/Zephyr errno).
 *
 * The target toolchain's own <errno.h> is NOT used: the ARM C library assigns
 * different values (e.g. EINVAL=5) and omits EIO/ENODEV/ETIMEDOUT, which would
 * break the "negative errno" contract shared between target and host builds.
 */

#ifndef ZEPHYR_ERRNO_H_
#define ZEPHYR_ERRNO_H_

#define EPERM 1
#define EIO 5
#define ENODEV 19
#define EINVAL 22
#define ETIMEDOUT 110

#endif /* ZEPHYR_ERRNO_H_ */
