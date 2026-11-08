/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Host assertion post-handler (strong): report and abort so a failed
 * __ASSERT fails the running test. Overrides the weak default in the port.
 */

#include <stdio.h>
#include <stdlib.h>

void z_assert_post(const char *file, unsigned int line)
{
	(void)fprintf(stderr, "ASSERT FAILED: %s:%u\n", file, line);
	abort();
}
