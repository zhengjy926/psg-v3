/*
 * Host (Linux/GCC) build configuration. Mirrors app/autoconf.h for the x86_64
 * host: no MCU/SoC specifics, GCC atomic builtins, no devicetree.
 */
#define CONFIG_X86 1
#define CONFIG_LITTLE_ENDIAN 1
#define CONFIG_NO_DT 1
#define CONFIG_ATOMIC_OPERATIONS_BUILTIN 1

#ifndef DT_HAS_COMPAT_STATUS_OKAY
#define DT_HAS_COMPAT_STATUS_OKAY(compat) 0
#endif
