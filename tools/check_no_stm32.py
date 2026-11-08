#!/usr/bin/env python3
"""Build-time hard constraint: the host-clean layers (osal, service, app and
drivers/api) must not include STM32 or other MCU headers.

Scans the given paths for C/C++ sources and fails with a non-zero exit if any
of them includes an STM32/board device header. Wired into the host build as a
`check_no_stm32` target (added to ALL), so a violation breaks the build.

Usage: python tools/check_no_stm32.py <dir> [<dir> ...]
"""

import os
import re
import sys

# Paths that are allowed to touch MCU headers are simply not passed in.
INCLUDE_RE = re.compile(
    r'^\s*#\s*include\s*[<"](?:stm32|board/stm32|core_cm|cmsis)',
    re.IGNORECASE,
)

SOURCE_SUFFIXES = (".c", ".h")


def scan(paths):
    violations = []
    for path in paths:
        for root, _dirs, files in os.walk(path):
            for name in files:
                if not name.endswith(SOURCE_SUFFIXES):
                    continue
                full = os.path.join(root, name)
                with open(full, "r", encoding="utf-8", errors="ignore") as fh:
                    for lineno, line in enumerate(fh, 1):
                        if INCLUDE_RE.match(line):
                            violations.append(f"{full}:{lineno}: {line.strip()}")
    return violations


def main():
    if len(sys.argv) < 2:
        print(__doc__, file=sys.stderr)
        return 2
    violations = scan(sys.argv[1:])
    if violations:
        print("ERROR: host-clean layers must not include STM32/MCU headers:",
              file=sys.stderr)
        for v in violations:
            print(f"  {v}", file=sys.stderr)
        return 1
    print("OK: no STM32 headers in host-clean layers")
    return 0


if __name__ == "__main__":
    sys.exit(main())
