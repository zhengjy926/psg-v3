#!/usr/bin/env bash
#
# Static analysis gate for the host-clean firmware layers (issue #21, AC #3).
# Runs clang-tidy + cppcheck (built-in) + cppcheck MISRA addon. Exits non-zero
# on any clang-analyzer finding, cppcheck warning/error, or MISRA violation
# (after the documented deviations in tools/misra/README.md).
#
# Usage: tools/static_analysis.sh <build-dir> <cppcheck-addons-dir>
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${1:-$ROOT/build/host}"
CPPCHECK_ADDONS="${2:-/usr/share/cppcheck/addons}"

# Host-clean sources under analysis (project code + Zephyr runtime under lib/utils).
# Mirrors the host test build in CMakeLists.txt; board/, drivers/<drv>/ are target-only.
mapfile -t SOURCES < <(
  find "$ROOT/osal" "$ROOT/service" "$ROOT/app" "$ROOT/lib/utils" -name '*.c' | sort
)

INCLUDES=(
  -I "$ROOT/lib" -I "$ROOT/lib/utils" -I "$ROOT/lib/os" -I "$ROOT/lib/toolchain"
  -I "$ROOT/lib/libc/armstdc/include"
  -I "$ROOT/subsys" -I "$ROOT/drivers/include" -I "$ROOT"
)

echo "== clang-tidy =="
clang-tidy -p "$BUILD_DIR" "${SOURCES[@]}" --quiet \
  --warnings-as-errors='clang-analyzer-*'

echo "== cppcheck (built-in) =="
cppcheck --std=c11 --enable=warning --error-exitcode=1 \
  --suppress=missingIncludeSystem --suppress=comparePointers \
  "${INCLUDES[@]}" "${SOURCES[@]}"

echo "== cppcheck MISRA addon =="
DUMP_DIR="$(mktemp -d)"
trap 'rm -rf "$DUMP_DIR"' EXIT
(
  cd "$DUMP_DIR"
  cppcheck --dump --quiet --std=c11 --suppress=missingIncludeSystem \
    "${INCLUDES[@]}" "${SOURCES[@]}"
)
python3 "$CPPCHECK_ADDONS/misra.py" \
  --rule-texts="$ROOT/tools/misra/misra_2012_rules.txt" \
  --suppress-rules=8.9,8.11,11.3,11.5,15.5,17.1,17.8,20.7,20.10,21.1 \
  "$DUMP_DIR"/*.dump > "$DUMP_DIR/misra.txt" || true

if grep -q 'misra-c2012-' "$DUMP_DIR/misra.txt"; then
  cat "$DUMP_DIR/misra.txt"
  echo "MISRA violations found (see tools/misra/README.md for the deviation table)" >&2
  exit 1
fi

echo "static analysis clean"
