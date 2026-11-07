# Zephyr v4.5.0 设备模型 + 初始化机制 + 日志框架 → 裸机 STM32G474 + Keil MDK(ARMCC) 最小可移植闭包

> 研究票 #2 · 执行日期 2026-11-07
> 源码基线：`D:\Zephyr\zephyrproject\zephyr` @ tag `v4.5.0`（detached HEAD，commit `41d10b59730406bbab4ca9f1fbbe9e96aafb4966`，只读，未做任何 git 操作）。
> 所有引用均落到具体文件与行号，文件路径均相对 Zephyr 仓库根目录；行号以 `v4.5.0` 检出为准。

---

## 0. 结论摘要（TL;DR）

1. Zephyr 的「设备模型 + 初始化」本质是 **两套互相关联的 iterable-section**：`struct device` 本体落在 `._device.static.*`（`ITERABLE_SECTION_ROM_NUMERIC(device)`），而真正驱动调用顺序的是 `struct init_entry`，落在 `.z_init_<LEVEL>_P_<prio>_SUB_<sub_prio>_`（`CREATE_OBJ_LEVEL(init, LEVEL)`）。内核 `z_sys_init_run_level()` 只是按链接脚本生成的 `__init_<LEVEL>_start/end` 符号线性遍历调用，**与 Zephyr 调度器/内核对象无必然耦合**，是唯一需要移植的「运行时」。
2. **Zephyr 不使用 `__attribute__((constructor))` 做 SYS_INIT**。初始化顺序由链接脚本的 `KEEP(*(SORT(.z_init_...)))` 以**定宽十进制段名 + 字典序**实现，优先级 0–999 通过「1 位 / 2 位 / 3 位」三组 SORT 保证。这决定了裸机移植必须自写等价 scatter 文件或改用「构建期生成排序表」的方式。
3. ARMCC 里 Zephyr **只支持 armclang（ARM Compiler 6，宏 `__ARMCOMPILER_VERSION`）**，`__attribute__((section/aligned/used/weak))` 全部走 `toolchain/armclang.h → llvm.h → gcc.h` 的 GNU 属性映射；**ARMCC5（`__CC_ARM`）没有对应的 `toolchain/armcc5.h`，未受支持**。
4. 日志框架「头文件 + 宏层」高度自洽（模块注册 = `log_const`/`log_dynamic` iterable section，等级过滤 = 编译期 `__log_level` + 运行期 `filters` 位域，后端 = `log_backend` iterable section），但**消息打包强依赖 cbprintf**，是闭包里最重的一坨；裸机最小路径是 `CONFIG_LOG_MODE_MINIMAL`（退化为 `printk`），或自写一个极简 backend + `snprintf` 替代 cbprintf。
5. `DEVICE_AND_API_INIT` 在 v4.5.0 **已删除**（全树 grep 无匹配），其职能由 `DEVICE_DEFINE` + `DEVICE_API_GET`/`DEVICE_API_IS`/`DEVICE_API_EXTENDS` 取代。

---

## 1. 核心头文件及其中间依赖的完整文件清单（文件级）

### 1.1 五个入口文件的直接 `#include`

| 文件 | 直接依赖（源码 `include/zephyr/…`） | 关键行 |
|---|---|---|
| `device.h` | `<stdint.h>`, `devicetree.h`, `init.h`, `linker/sections.h`, `pm/state.h`, `sys/device_mmio.h`, `sys/iterable_sections.h`, `sys/util.h`, `toolchain.h`, `llext/symbol.h`(仅 `CONFIG_LLEXT`), `syscalls/device.h`(文件末尾无条件) | `device.h:16-31, 1501` |
| `init.h` | `<stdint.h>`, `<stddef.h>`, `sys/util.h`, `toolchain.h` | `init.h:16-20` |
| `sys/iterable_sections.h` | `sys/__assert.h`, `toolchain.h` | `iterable_sections.h:16-17` |
| `sys/util.h` | `sys/util_macro.h`, `toolchain.h`, `<stdbool.h>`, `sys/__assert.h`, `types.h`, `<stddef.h>`, `<stdint.h>`, `<string.h>`, `sys/time_units.h` | `util.h:17-31, 1048` |
| `logging/log.h` | `logging/log_instance.h`, `logging/log_core.h`, `sys/iterable_sections.h`, `sys/atomic.h`, `sys/util_macro.h`, `app_memory/app_memdomain.h`(仅 `CONFIG_USERSPACE && CONFIG_LOG_ALWAYS_RUNTIME`), `kernel.h`(仅 `CONFIG_LOG_RATELIMIT`), `zephyr_custom_log.h`(仅 `CONFIG_LOG_CUSTOM_HEADER`) | `log.h:16-28, 897-900` |

### 1.2 完整闭包分层（`device.h` ∪ `init.h` ∪ `iterable_sections.h` ∪ `util.h` ∪ `log.h`）

#### A 层：纯预处理宏，可原样复制（无平台/内核依赖）

| 文件 | 说明 | 依据 |
|---|---|---|
| `include/zephyr/toolchain.h` | 编译器抽象分派头（按 `__ARMCOMPILER_VERSION`/`__GNUC__`/… 选子头） | `toolchain.h:49-69` |
| `include/zephyr/toolchain/common.h` | `Z_DECL_ALIGN`、`__syscall`、`STRINGIFY`、`_CONCAT`、`BUILD_ASSERT`、`EXTERN_C` | `toolchain/common.h:190-212, 252` |
| `include/zephyr/toolchain/gcc.h` | GNU 属性映射：`__aligned/__used/__weak/__packed/__in_section/__noasan` 等 | `toolchain/gcc.h:198-326, 683-687` |
| `include/zephyr/toolchain/llvm.h` | clang 分支，`#include <zephyr/toolchain/gcc.h>` | `toolchain/llvm.h:33` |
| `include/zephyr/toolchain/armclang.h` | armclang 分支，重定义 `__GENERIC_SECTION` 带 `used` | `toolchain/armclang.h:14-24` |
| `include/zephyr/sys/util_macro.h` | `IS_ENABLED/COND_CODE_1/FOR_EACH/GET_ARG_N/UTIL_CAT/UTIL_LISTIFY/…` | `util_macro.h:1`（`#include <zephyr/sys/util_internal.h>`） |
| `include/zephyr/sys/util_internal.h` + `util_loops.h` + `util_internal_is_eq.h` + `util_internal_util_{inc,dec,x2}.h` | `util_macro` 的实现底座，纯宏 | `util_internal.h` 的 5 个 include |
| `include/zephyr/types.h` | 仅 `<stddef.h>` `<stdint.h>`，Zephyr 定宽类型约定 | `types.h:1-2` |

#### B 层：核心「设备/初始化」声明，可原样复制

| 文件 | 说明 | 关键行 |
|---|---|---|
| `include/zephyr/init.h` | `struct init_entry`、`SYS_INIT`/`SYS_INIT_NAMED`、`Z_INIT_ENTRY_SECTION`、`INIT_LEVEL_ORD` | `init.h:74-85, 119-121, 133-181` |
| `include/zephyr/sys/iterable_sections.h` | `STRUCT_SECTION_ITERABLE`/`FOREACH`/`COUNT`/`GET` 系列宏 | `iterable_sections.h:50-349` |

#### C 层：`sys/util.h` — 可复制，但需处理 3 处内核引用

`util.h` 本身几乎纯宏/内联，但有两处依赖需处理：

- `#include <zephyr/sys/time_units.h>`（`util.h:1048`）→ 后者 `#include <zephyr/syscalls/time_units.h>`（**构建期生成**，源码树不存在），并用到 `k_us_to_cyc_ceil32`/`k_cycle_get_32` 等内核函数。
- `WAIT_FOR()`（`util.h:1102-1113`）内部调用 `k_us_to_cyc_ceil32`/`k_cycle_get_32` → **内核依赖**，仅在使用该宏时触发；裸机可自写基于 `DWT->CYCCNT`/`SysTick` 的等价实现或直接不提供。
- `Z_SPIN_DELAY`（`util.h:1081-1085`）在 `CONFIG_ARCH_POSIX` 下调用 `k_busy_wait`，非 POSIX 下为空，裸机无影响。

#### D 层：设备模型运行时（内核侧，必须改写为裸机版）

| 文件 | 说明 | 依赖 |
|---|---|---|
| `kernel/device.c` | `do_device_init`、`z_impl_device_*`、`device_get_binding`、`device_is_ready`、`z_device_get_all_static`、`device_required/supported_foreach` | `errno.h`、`sys/atomic.h`、`sys/kobject.h`、`internal/syscall_handler.h`、`pm/device_runtime.h`、`syscalls/*_mrsh.c`(仅 userspace) | `device.c:1-16, 18-69, 91-108, 189-210` |
| `kernel/init.c` | `z_sys_init_run_level`、`z_cstart`、`bg_thread_main`（**唯一驱动 init 顺序的地方**） | `kernel.h`、`linker/linker-defs.h`、`offsets_short.h`、`logging/log_ctrl.h` 等 | `init.c:111-131, 207-237, 265-355, 535-616` |

#### E 层：日志框架（宏层可复制，运行时要重写/降级）

| 文件 | 说明 | 依赖 | 关键行 |
|---|---|---|---|
| `include/zephyr/logging/log_instance.h` | `log_source_const_data`/`log_source_dynamic_data`、`Z_LOG_CONST_ITEM_REGISTER`、`LOG_INSTANCE_REGISTER` | `types.h`、`sys/iterable_sections.h` | `log_instance.h:26-38, 71-78, 181-182` |
| `include/zephyr/logging/log_core.h` | 等级常量、`Z_LOG*` 宏、`Z_LOG_CONST_LEVEL_CHECK`、`log_const_source_id` | `logging/log_msg.h`、`logging/log_instance.h`、`sys/util.h` | `log_core.h:38-46, 175-183, 343-371, 523-528` |
| `include/zephyr/logging/log.h` | 用户宏 `LOG_ERR/…`、`LOG_MODULE_REGISTER/DECLARE`、限流/hexdump/实例宏 | 见 1.1 | `log.h:65-96, 739-853` |
| `include/zephyr/logging/log_msg.h` | **消息打包 = cbprintf**：`CBPRINTF_STATIC_PACKAGE`、`z_log_msg_static_create`/`z_log_msg_runtime_create` | `sys/cbprintf.h`、`sys/mpsc_packet.h`、`syscalls/log_msg.h`(生成) | `log_msg.h:18, 108-118, 347-392, 526-548` |
| `include/zephyr/logging/log_backend.h` | `log_backend_api`、`log_backend`、`LOG_BACKEND_DEFINE` | `log_msg.h`、`log_output.h`、`iterable_sections.h` | `log_backend.h:74-148, 200-206` |
| `include/zephyr/logging/log_ctrl.h` | `log_core_init`、`LOG_CORE_INIT()` 宏 | `kernel.h`、`log_backend.h`、`log_internal.h`、`syscalls/log_ctrl.h`(生成) | `log_ctrl.h:16-19, 57, 313-329` |
| `include/zephyr/logging/log_output.h` | 输出格式化（`log_format_func_t`） | `kernel.h`、`log_msg.h`、`atomic.h` | `log_output.h:16-20` |
| `include/zephyr/logging/log_backend_std.h` | 后端标准格式化助手 | `log_msg.h`、`log_output.h`、`kernel.h` | `log_backend_std.h:16-19` |
| `include/zephyr/logging/log_internal.h` | 内部状态 | `types.h`、`__assert.h`、`log_core.h`、`sys/mpsc_pbuf.h` | `log_internal.h:16-19` |
| `subsys/logging/log_core.c` | 后端激活/过滤/处理线程 | kernel 全栈 | — |
| `subsys/logging/log_msg.c` | cbprintf 消息创建/处理 | kernel、cbprintf | — |
| `subsys/logging/log_minimal.c` | **MINIMAL 模式**：全部退化到 `printk`，无 cbprintf/backend | `sys/printk.h`、`log.h`、`llext/symbol.h` | `log_minimal.c:14-22, 53-66` |
| `subsys/logging/backends/log_backend_rtt.c` | RTT 后端示例 | `SEGGER_RTT.h`、`log_output.h`、`log_backend_std.h` | 见 §5.3 |

#### F 层：必须剔除的子系统头（见 §2）

`devicetree.h`(+`devicetree/*`)、`pm/state.h`、`llext/symbol.h`、`sys/device_mmio.h`、`sys/kobject.h`、`internal/syscall_handler.h`、`app_memory/app_memdomain.h`、`kernel.h`、`syscalls/*`（生成）、`linker/linker-defs.h` 的 devicetree 段、`offsets*.h`（生成）。

> 说明：`syscalls/device.h`、`syscalls/time_units.h`、`syscalls/log_msg.h`、`syscalls/log_ctrl.h` 均**不在源码树**（`include/zephyr/syscalls/` 目录为空），由 `scripts/build/gen_syscalls.py` 在构建时生成到 `include/generated/zephyr/syscalls/`。`__syscall` 在无 userspace 时被定义为 `static inline`（`toolchain/common.h:206-208`），函数体由生成的 syscalls 头提供——裸机移植需手写这 5~8 个 `static inline` 包装。

---

## 2. 必须剔除的依赖及其替代 shim 的最小形态

### 2.1 devicetree（`devicetree.h` + `devicetree/*`，6515 行 + 生成头）

`device.h` 对 DT 的依赖点：`DT_INVALID_NODE`、`DT_NODE_EXISTS`、`DT_PROP`、`DT_PROP_OR`、`DT_DEP_ORD`、`DT_DEP_ORD_STR_SORTABLE`、`DT_NODE_HASH`、`DT_NUM_NODELABELS`/`DT_NODELABEL_STRING_ARRAY`、`DT_FOREACH_STATUS_OKAY_NODE`、`DT_DRV_INST`、`DT_COMPAT_*`、`DT_PHANDLE_BY_IDX` 等（`device.h:49-144, 200-430, 1019-1020, 1211-1213, 1411-1415`）。

**最小 shim 策略（只支持「软件设备」，即 `dev_id` 直传）**：自写 `shim/devicetree.h`，把 DT 节点 id 恒等于 `DT_INVALID_NODE`，全部属性宏返回 0/NULL/空，并屏蔽 `DT_FOREACH_STATUS_OKAY_NODE`：

```c
/* shim/devicetree.h —— 最小形态（示意） */
#define DT_INVALID_NODE 0
#define DT_NODE_EXISTS(node) 0
#define DT_PROP(node, p) 0
#define DT_PROP_OR(node, p, dflt) (dflt)
#define DT_DEP_ORD(node) 0
#define DT_DEP_ORD_STR_SORTABLE(node) 0
#define DT_NODE_HASH(node) 0
#define DT_NUM_NODELABELS(node) 0
#define DT_NODELABEL_STRING_ARRAY(node) { 0 }
#define DT_FOREACH_STATUS_OKAY_NODE(fn)   /* 空 */
/* 所有 DEVICE_DT_* 宏不再编译（驱动改用 DEVICE_DEFINE/DEVICE_GET） */
```

同时把 `device.h` 中 `#include <zephyr/devicetree.h>` 替换为 `#include "shim/devicetree.h"`，并删除 `DT_FOREACH_STATUS_OKAY_NODE(Z_MAYBE_DEVICE_DECLARE_INTERNAL)`（`device.h:1415`）。

### 2.2 Kconfig（`CONFIG_*`）

闭包里实际引用的 `CONFIG_*` 开关（按「裸机默认值」收敛）：

| 开关 | 裸机固定值 | 作用/出处 |
|---|---|---|
| `CONFIG_LOG` | 1（或 0 关闭） | `log_core.h:58-61, 175` |
| `CONFIG_LOG_MODE_MINIMAL` | **建议 1** | 走 `printk`，绕开 cbprintf/backend | `log_core.h:347, 499` |
| `CONFIG_LOG_DEFAULT_LEVEL` / `CONFIG_LOG_MAX_LEVEL` / `CONFIG_LOG_OVERRIDE_LEVEL` | 按需（如 4/4/0） | 编译期过滤 | `log_core.h:59-60, 175-183` |
| `CONFIG_LOG_RUNTIME_FILTERING` | 0（可省 `log_dynamic`） | `log.h:755, 803` |
| `CONFIG_LOG_FMT_SECTION` | 0 | 决定模块名是否入 section | `log.h:740-748` |
| `CONFIG_LOG_RATELIMIT` | 0 | 省 `kernel.h` 依赖 | `log.h:26-28` |
| `CONFIG_DEVICE_DEPS` / `CONFIG_DEVICE_DEPS_DYNAMIC` | 0 | 省 `device_handle_t *deps` 与 `gen_device_deps.py` 后处理 | `device.h:511-515, 558-568` |
| `CONFIG_DEVICE_DEINIT_SUPPORT` | 0 | 省 `ops.deinit` 字段 | `device.h:534-537` |
| `CONFIG_PM_DEVICE` / `CONFIG_PM` | 0 | 删 `struct device` 的 pm 字段 | `device.h:569-582` |
| `CONFIG_DEVICE_DT_METADATA` | 0 | 省 `dt_meta` 字段与 nodelabel 元数据 | `device.h:583-589` |
| `CONFIG_DEVICE_MUTABLE` | 0 | 设备放 ROM | `device.h:58-59, 1320-1324` |
| `CONFIG_LLEXT` / `CONFIG_LLEXT_EXPORT_DEV_IDS_BY_HASH` | 0 | 删 LLEXT 导出 | `device.h:27-29, 128-144, 1397-1399` |
| `CONFIG_USERSPACE` | 0 | `__syscall`→`static inline`；删 memdomain | `toolchain/common.h:206-208` |
| `CONFIG_SMP` | 0 | 无 SMP level | `init.h:97-103` |

**shim**：用一个手写 `include/zephyr/autoconf.h`（或 Keil 的 `-D` 集）提供上表固定值，替代 Zephyr 的 `autoconf.h` 生成机制。

### 2.3 PM（`pm/state.h` + `pm/device*.h`）

- `device.h:21` 无条件 `#include <zephyr/pm/state.h>`，但 `struct device` 只在 `CONFIG_PM_DEVICE` 时才有 pm 字段（`device.h:569-582`）。裸机置 `CONFIG_PM_DEVICE=0` 后，只需一个**空 `shim/pm/state.h`**（或直接删除该 include + 三处前向声明 `device.h:504-506`）。
- `kernel/device.c:16` 的 `pm/device_runtime.h`（`pm_device_runtime_auto_enable`，`device.c:62`）在自写运行时里删除即可。

### 2.4 LLEXT（`llext/symbol.h`）

`EXPORT_SYMBOL`/`EXPORT_SYMBOL_GROUP` 用于把符号加入 LLEXT 链接组（`subsys/logging/log_minimal.c:22` 用到 `EXPORT_SYMBOL`）。裸机 shim 为空宏：

```c
#define EXPORT_SYMBOL(x)            /* 空 */
#define EXPORT_SYMBOL_GROUP(...)    /* 空 */
#define EXPORT_GROUP_SYMBOL(...)    /* 空 */
```

### 2.5 kernel（`kernel.h`、`sys/kobject.h`、`internal/syscall_handler.h`）

- `kernel/init.c:192` 的 `k_object_init(dev)` 是 userspace 对象权限登记；裸机删除。
- `syscalls/*_mrsh.c` 仅 userspace；裸机不生成。
- `kernel.h` 只被 `log.h`(限流) / `log_output.h` / `log_backend_std.h` / `log_ctrl.h` / `app_memdomain.h` 拉入；全部收敛到 MINIMAL 模式 + 自写 backend 后即可去掉。

---

## 3. ARMCC 对 section/aligned/used/构造函数式 init 的支持，及 scatter 保证顺序

### 3.1 工具链分派（源码确证）

`toolchain.h:57-59` 明确：

```c
#elif defined(__ARMCOMPILER_VERSION)
#include <zephyr/toolchain/armclang.h>
```

即 Zephyr 认的是 **armclang（ARM Compiler 6）** 的 `__ARMCOMPILER_VERSION` 宏，不是 ARMCC5。全树没有 `toolchain/armcc5.h`；`__CC_ARM`（ARMCC5 宏）只零星出现在 `portability/cmsis_os2.h:48`、`modules/littlefs/zephyr_lfs_config.h:106,135` 等，与设备模型/日志无关。**结论：目标编译链必须是 armclang（AC6），ARMCC5 不支持（未确证 AC5 下能编译，源码无对应适配层）。**

### 3.2 关键属性的映射链（源码确证）

`toolchain/armclang.h:14-24` → `toolchain/llvm.h:33` → `toolchain/gcc.h`：

| 宏 | 展开 | 出处 |
|---|---|---|
| `__aligned(x)` | `__attribute__((__aligned__(x)))` | `gcc.h:263-265` |
| `__used` | `__attribute__((__used__))` | `gcc.h:290` |
| `__weak` | `__attribute__((__weak__))` | `gcc.h:324-326` |
| `__packed` | `__attribute__((__packed__))` | `gcc.h:259-261` |
| `__GENERIC_SECTION(seg)` | armclang 重定义为 `__attribute__((section(STRINGIFY(seg)), used))` | `armclang.h:23-24` |
| `__in_section(a,b,c)` | `__attribute__((section("." a "." b "." c)))` | `gcc.h:205-209` |
| `__noasan` | 空（非 ASAN 构建） | `gcc.h:683-687` |
| `Z_DECL_ALIGN(type)` | `__aligned(__alignof(type)) type` | `common.h:252` |

armclang 兼容 GNU `__attribute__` 语法（`section`/`aligned`/`used`/`weak`/`packed`），因此 `SYS_INIT_NAMED`（`init.h:178-181`）与 `STRUCT_SECTION_ITERABLE`（`iterable_sections.h:50-52`）在 AC6 下可直接编译。`used` 在 armclang 下**会额外把符号保留**，这对「仅靠遍历引用、无显式引用」的 iterable 对象是必须的（对应 armclang.h 把 `section` 与 `used` 绑在一起的原因）。

### 3.3 构造函数式 init（`__attribute__((constructor))`）

**Zephyr 的 SYS_INIT/设备 init 不使用 `__attribute__((constructor))`**，而是「链接脚本排序 + 内核显式遍历」（见 §4）。唯一与 constructor 相关的机制是 `CONFIG_STATIC_INIT_GNU`，把 `.init_array`/`.ctors` 收集到 `__zephyr_init_array_start/end` 由 `z_static_init_gnu()` 手动调用：

- 链接脚本：`include/zephyr/linker/common-rom/common-rom-init.ld:3-16`（`KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*) …)) KEEP(*(.init_array .ctors))`）。
- 运行时：`kernel/init.c:239-257`（`z_static_init_gnu`）+ `kernel/init.c:305-307`（在 APPLICATION 之前调用）。

对 armclang：`__attribute__((constructor))` 生成的函数指针落入 `.init_array`（AC6 用 `SHT_INIT_ARRAY`），这与 GNU 约定一致，所以若裸机要用构造式 init，可在 scatter 里保留 `.init_array` 区并由启动代码调用；但**这不是 Zephyr 设备模型的机制**，不要混用。

### 3.4 init 顺序如何由 scatter 文件保证

源码侧（GNU ld）的实现分两步（均可确证）：

1. `struct init_entry` 落段名 `.z_init_<LEVEL>_P_<prio>_SUB_<sub_prio>_`（`init.h:119-121`）。
2. `linker-defs.h:70-74` 的 `CREATE_OBJ_LEVEL(init, LEVEL)`：

```c
PLACE_SYMBOL_HERE(__##object##_##level##_start);
KEEP(*(SORT(.z_##object##_##level##_P_?_*)));      /* 1 位优先级 0-9   */
KEEP(*(SORT(.z_##object##_##level##_P_??_*)));     /* 2 位优先级 10-99 */
KEEP(*(SORT(.z_##object##_##level##_P_???_*)));    /* 3 位优先级 100-999 */
```

`SORT()` 按段名**字典序**排序；因为优先级被编码为定宽十进制（`P_0_`…`P_999_`），「按 1/2/3 位分组再各自 SORT」等价于数值 0→999 升序。设备对象本体（`struct device`）用 `ITERABLE_SECTION_ROM_NUMERIC(device)`（`common-rom-kernel-devices.ld:23` → `linker/iterable_sections.h:22-29`，`Z_LINK_ITERABLE_NUMERIC` 按 `*_?_* / *_??_* / …` 分组 SORT）成数组，供 `device_handle_get()` 按下标取 handle。

**Keil armlink 的差异（须注意，此条为 ARMCC 侧通用知识，非 Zephyr 源码，标注「未确证」级别）**：armlink 的 scatter 输入段选择符 `*(.z_init_*)` **不提供与 GNU ld `SORT()` 等价的按段名字典序排序**，通常按目标文件/链接顺序摆放。因此裸机移植有两条等价路径：

- **路径 A（推荐，最贴近原语义）**：保留「定宽十进制段名」约定，在 scatter 文件里**显式列出优先级分组**（把 0-9、10-99、100-999 三组用多个 `ER`/选择符依次排布），并用 `__init_<LEVEL>_start/end` 符号夹住每一级；或
- **路径 B（构建期生成）**：加一个链接后/脚本步骤，扫描 `.z_init_*` 段生成一张按 (level, prio, sub_prio) 排序的 `const struct init_entry *[]` 表（类似 Zephyr 的 `gen_device_deps.py`/`gen_isr_tables.py` 思路），启动代码直接按表调用，摆脱对段内顺序的依赖。

无论哪条路径，`Z_DECL_ALIGN(struct init_entry)` 的对齐（`init.h:179`）必须保留，否则遍历时对象间可能被填充而破坏数组假设（`common.h:236-252` 的注释明确说明这一点）。

---

## 4. `SYS_INIT` / `DEVICE_DEFINE` / `DEVICE_AND_API_INIT` 的无内核 boot 序列映射

### 4.1 `DEVICE_AND_API_INIT` 已删除

v4.5.0 全树 grep `DEVICE_AND_API_INIT` **无任何匹配**。其职责由三件套替代（`device.h`）：

- `DEVICE_DEFINE(dev_id, name, init_fn, pm, data, config, level, prio, api)`（`device.h:194-197`）——定义设备对象 + 挂 init entry。
- `DEVICE_API(_class, _name)` / `DEVICE_API_GET(_class, _dev)` / `DEVICE_API_IS(_class, _dev)` / `DEVICE_API_EXTENDS(child, parent, member)`（`device.h:1418-1495`）——类型化 API 的声明、取用与继承判断。
- 取设备对象：`DEVICE_GET(dev_id)`（`device.h:442`）；取 init entry：`DEVICE_INIT_GET(dev_id)`（`device.h:478`）。

### 4.2 宏展开产物（源码确证）

`DEVICE_DEFINE` → `DEVICE_DEINIT_DEFINE`（`device.h:182-187`）→ `Z_DEVICE_DEFINE`（`device.h:1380-1399`）最终展开为三件对象：

1. **设备状态**（RAM）：`Z_DEVICE_STATE_DEFINE(dev_id)` → `static Z_DECL_ALIGN(struct device_state) __devstate_<dev_id> __attribute__((section(".z_devstate")))`（`device.h:1003-1012`）。
2. **设备对象**（ROM）：`Z_DEVICE_BASE_DEFINE` → `STRUCT_SECTION_ITERABLE_NAMED_ALTERNATE(device, …, Z_DEVICE_SECTION_NAME(level, prio), DEVICE_NAME_GET(dev_id))`，落段 `._device.static.<ORD>_<prio>_`（`device.h:1318-1326`），其中 `Z_DEVICE_SECTION_NAME = _CONCAT(INIT_LEVEL_ORD(level), _##prio)`（`device.h:1296-1297`）。
3. **init entry**（ROM）：`Z_DEVICE_INIT_ENTRY_DEFINE` → `struct init_entry` 落段 `.z_init_<LEVEL>_P_<prio>_SUB_<sub_prio>_`，`.dev = &__device_<dev_id>, .init_fn = NULL`（`device.h:1348-1356`）。

`SYS_INIT(fn, level, prio)` 只产生第 3 类对象，`.init_fn = fn, .dev = NULL`（`init.h:160-161, 178-181`）。

> 关键点：**真正被启动代码遍历的是 `struct init_entry` 数组**。设备的 `ops.init` 通过 `do_device_init()` 间接调用；`SYS_INIT` 函数被直接调用。两者在 `z_sys_init_run_level` 里分叉（`kernel/init.c:223-236`）。

### 4.3 Zephyr 原始 boot 序列（源码确证，`kernel/init.c`）

| 阶段 | 调用点 | 位置 |
|---|---|---|
| EARLY | `z_cstart()` 最先执行 `z_sys_init_run_level(INIT_LEVEL_EARLY)` | `init.c:535-541` |
| arch 初始化 | `arch_kernel_init()` | `init.c:544` |
| 日志核心 | `LOG_CORE_INIT()`（= `log_core_init()`） | `init.c:546` |
| 设备状态登记 | `z_device_state_init()`（遍历 `device` 段，userspace 下 `k_object_init`） | `init.c:189-194, 552` |
| 早期 hooks | `soc_early_init_hook()` / `board_early_init_hook()` | `init.c:554-556` |
| PRE_KERNEL_1 | `z_sys_init_run_level(INIT_LEVEL_PRE_KERNEL_1)` | `init.c:568` |
| PRE_KERNEL_2 | `z_sys_init_run_level(INIT_LEVEL_PRE_KERNEL_2)` | `init.c:572` |
| 切 main 线程 | `switch_to_main_thread()` → `bg_thread_main()` | `init.c:588, 265-266` |
| POST_KERNEL | `z_sys_init_run_level(INIT_LEVEL_POST_KERNEL)`（在 `z_sys_post_kernel=true` 之后） | `init.c:280-295` |
| 晚期 hooks | `soc_late_init_hook()` / `board_late_init_hook()` | `init.c:297-299` |
| 构造式 init | `z_static_init_gnu()`（仅 `CONFIG_STATIC_INIT_GNU`） | `init.c:305-307` |
| APPLICATION | `z_sys_init_run_level(INIT_LEVEL_APPLICATION)` | `init.c:310` |
| main() | `(void)main()` | `init.c:327-342` |

`z_sys_init_run_level` 本体（`init.c:207-237`）：按 `levels[] = {__init_EARLY_start, __init_PRE_KERNEL_1_start, …, __init_end}` 的符号边界线性遍历；`dev != NULL` 走 `do_device_init(dev)`，否则 `entry->init_fn()`。`do_device_init`（`kernel/device.c:18-69`）调用 `dev->ops.init(dev)`，把负 errno 转成 `state->init_res`（uint8_t 正数），成功则置 `state->initialized = true`。

### 4.4 无 Zephyr 内核的裸机映射

自写 `bare_init.c`，等价替换 `z_cstart` + `z_sys_init_run_level`：

```c
/* bare_init.c —— 最小形态（示意） */
extern const struct init_entry __init_start[];
extern const struct init_entry __init_EARLY_start[];
extern const struct init_entry __init_PRE_KERNEL_1_start[];
extern const struct init_entry __init_PRE_KERNEL_2_start[];
extern const struct init_entry __init_POST_KERNEL_start[];
extern const struct init_entry __init_APPLICATION_start[];
extern const struct init_entry __init_end[];

static const struct init_entry *const levels[] = {
    __init_EARLY_start, __init_PRE_KERNEL_1_start, __init_PRE_KERNEL_2_start,
    __init_POST_KERNEL_start, __init_APPLICATION_start, __init_end,
};

static void run_level(int lv) {
    for (const struct init_entry *e = levels[lv]; e < levels[lv+1]; e++) {
        if (e->dev) do_device_init(e->dev); else e->init_fn();
    }
}

void zephyr_bare_boot(void) {          /* 由 Keil 的 __main/Reset_Handler 调用 */
    SystemInit();                       /* CMSIS 时钟/FPU */
    run_level(0);  /* EARLY */
    run_level(1);  /* PRE_KERNEL_1 */
    run_level(2);  /* PRE_KERNEL_2 */
    /* 外设时钟、NVIC、SysTick 就绪后： */
    run_level(3);  /* POST_KERNEL */
    run_level(4);  /* APPLICATION */
    main();
}
```

对应 `do_device_init` 的裸机版（砍掉 `pm_device_runtime_auto_enable` 与 userspace）：

```c
int do_device_init(const struct device *dev) {
    int rc = 0;
    if (dev->ops.init) {
        rc = dev->ops.init(dev);
        dev->state->init_res = (rc < 0) ? (uint8_t)(-rc) : 0;
    }
    if (rc == 0) dev->state->initialized = true;
    return rc < 0 ? rc : 0;
}
```

映射关系：`EARLY`→上电后立刻；`PRE_KERNEL_1/2`→外设/时钟/内存就绪前后；`POST_KERNEL`→「内核服务就绪」等价点（裸机即中断与 SysTick 就绪后）；`APPLICATION`→`main()` 前。裸机无 SMP，去掉 SMP level。

---

## 5. logging 的模块注册 / 等级过滤 / backend / RTT 最小实现要点

### 5.1 模块注册（iterable section，无内核依赖）

- 常量源数据：`struct log_source_const_data { const char *name; uint8_t level; }`（`log_instance.h:26-29`）。
- 运行期源数据（可选）：`struct log_source_dynamic_data { uint32_t filters; }`（`log_instance.h:32-38`）。
- `LOG_MODULE_REGISTER(name[, level])`（`log.h:845-853`）→ `_LOG_MODULE_DATA_CREATE`（`log.h:758-760`）：
  - 常量项入 `log_const` 段（`STRUCT_SECTION_ITERABLE_ALTERNATE(log_const, log_source_const_data, …)`，`log.h:739-748`），当 `CONFIG_LOG_FMT_SECTION` 时模块名也做成 `log_strings` 段字符串。
  - `CONFIG_LOG_RUNTIME_FILTERING` 时另建 `log_dynamic` 段项（`log.h:750-756`）。
- 源 id 通过「段内地址差」计算：`log_const_source_id()`（`log_core.h:523-528`）。
- 链接脚本收集：`ITERABLE_SECTION_ROM(log_const)`、`ITERABLE_SECTION_ROM(log_backend)`、`ITERABLE_SECTION_ROM(log_strings)`（`common-rom-logging.ld:21-32`），RAM 侧 `log_msg_ptr`/`log_dynamic`（`common-ram.ld:64-66`）。

### 5.2 等级过滤（两层）

- **编译期**：`__log_level`（文件级 `static const`，由 `_LOG_LEVEL_RESOLVE` 解析，`log.h:727-734, 806-807`）+ `Z_LOG_CONST_LEVEL_CHECK(_level)`（`log_core.h:175-183`）。级别常量 `NONE=0/ERR=1/WRN=2/INF=3/DBG=4`（`log_core.h:38-46`）。
- **运行期**：`log_source_dynamic_data.filters` 是一个 `uint32_t`，按每 3 bit 一个 slot 编码「聚合等级 + 每个 backend 等级」：`LOG_FILTER_SLOT_*`、`Z_LOG_RUNTIME_FILTER`（`log_core.h:427-475`）。
- `Z_LOG(_level, ...)` → `Z_LOG2` → `Z_LOG_LEVEL_ALL_CHECK_BREAK`（编译期 + 静态实例 + 运行期三级检查，`log_core.h:343-371, 228-254`）。

### 5.3 backend 接口与 RTT 后端要点

- 后端 API：`struct log_backend_api { process; dropped; panic; init; is_ready; format_set; notify; }`（`log_backend.h:74-95`）；`process` 与 `panic` 必填。
- 控制块 `struct log_backend_control_block` + 描述 `struct log_backend`（`log_backend.h:103-123`）。
- 注册宏 `LOG_BACKEND_DEFINE(_name, _api, _autostart, …)` → `STRUCT_SECTION_ITERABLE(log_backend, _name)`（`log_backend.h:134-148`），由 `log_core_init()` 遍历激活。
- **RTT 后端**（`subsys/logging/backends/log_backend_rtt.c`）最小实现要素：
  - 输出函数 `data_out`（block/overwrite/drop 三种模式，`log_backend_rtt.c:74-282`），直接调 `SEGGER_RTT_WriteSkipNoLock`/`WriteWithOverwriteNoLock`。
  - 用 `LOG_OUTPUT_EXT_DEFINE(log_output_rtt, data_out, …, char_buf, …)` 挂格式化（`log_backend_rtt.c:305-307`）。
  - `process()` 取 `log_format_func_t_get(log_format_current)` 并调 `log_output_func(&log_output_rtt, &msg->log, flags)`（`log_backend_rtt.c:349-359`）。
  - API 结构 `log_backend_rtt_api = { .process, .panic, .init, .dropped, .format_set }`，再 `LOG_BACKEND_DEFINE(log_backend_rtt, log_backend_rtt_api, true)`（`log_backend_rtt.c:369-377`）。
  - 依赖 `modules/segger/SEGGER_RTT_zephyr.c`（官方 SEGGER RTT 源码适配）。

### 5.4 裸机最小实现建议（重点）

**完整移植 cbprintf（`log_msg.h` + `lib/os/cbprintf.c` + `sys/mpsc_pbuf.h` + 消息处理线程）成本最高，不建议作为第一步。** 三条递减成本路径：

1. **MINIMAL 模式**（零 backend、零 cbprintf）：`CONFIG_LOG_MODE_MINIMAL=1` 时 `Z_LOG2` 直接走 `Z_LOG_TO_PRINTK`（`log_core.h:347-350, 272-282`），`subsys/logging/log_minimal.c` 用 `printk` 实现（`log_minimal.c:14-22, 53-66`）。裸机只需提供一个 `printk`→UART/RTT 的 `_write`（或用 CMSIS `ITM`/`SEGGER_RTT_printf`）。
2. **极简自写 backend**：实现 `log_backend_api.process`，在 `process` 里用 `cbprintf` 解包或直接 `vsnprintf` 格式化后经 `SEGGER_RTT_WriteString` 输出；`LOG_BACKEND_DEFINE(..., true)` 注册。此路径仍要 `log_msg.h`（含 cbprintf 打包），除非消息打包层也改写。
3. **完整闭包**：移植 `log_msg.c`/`log_core.c`/`log_output.c` + cbprintf + RTT backend，等价于「把日志子系统整体搬过去」。

`printk.h` 与日志耦合：`include/zephyr/sys/printk.h` 末尾 `#include <zephyr/logging/log_core.h>`（`printk.h:193` 附近），所以 `printk` 与 `LOG_MODULE_REGISTER` 的模块名会互相引用；裸机若用 MINIMAL 模式，需要给 `printk` 一个落地实现（CMSIS 的 `fputc`/RTT）。

---

## 6. 可复制代码片段清单

### 6.1 几乎原样复制（纯头文件/宏，无内核与 DT 依赖）

| 文件 | 处理 |
|---|---|
| `include/zephyr/toolchain.h` | 原样 |
| `include/zephyr/toolchain/common.h` | 原样 |
| `include/zephyr/toolchain/gcc.h` | 原样 |
| `include/zephyr/toolchain/llvm.h` | 原样 |
| `include/zephyr/toolchain/armclang.h` | 原样 |
| `include/zephyr/sys/util_macro.h` + `util_internal.h` + `util_loops.h` + `util_internal_{is_eq,util_inc,util_dec,util_x2}.h` | 原样 |
| `include/zephyr/types.h` | 原样 |
| `include/zephyr/init.h` | 原样（`SYS_INIT` 语义完整，无 DT/内核依赖） |
| `include/zephyr/sys/iterable_sections.h` | 原样 |
| `include/zephyr/logging/log_instance.h` | 原样 |
| `include/zephyr/logging/log_backend.h`（仅结构体/宏部分） | 原样（`process` 等 inline 走 `log_msg.h`，需先定 `log_msg` 形态） |

### 6.2 需小改（保留主体，裁掉少数引用）

| 文件 | 改动点 |
|---|---|
| `include/zephyr/sys/util.h` | 删/替换 `sys/time_units.h` 依赖与 `WAIT_FOR`（内核 `k_*`）；其余保留 |
| `include/zephyr/logging/log_core.h` | 删 userspace 分支与 `app_memory`；保留等级/过滤宏与 `Z_LOG*` 骨架 |
| `include/zephyr/logging/log.h` | 删 `kernel.h`（限流）、`app_memory`、`zephyr_custom_log.h`；保留 `LOG_MODULE_REGISTER/DECLARE` 与 `LOG_*` |
| `subsys/logging/log_minimal.c` | 删 `EXPORT_SYMBOL`（LLEXT）；`printk` 换裸机实现；其余原样 |

### 6.3 必须改写 / 重写

| 对象 | 说明 |
|---|---|
| `include/zephyr/device.h` | 删 DT/PM/LLEXT/`syscalls/device.h` 依赖；`struct device` 裁掉 pm/deps/dt_meta 字段；`DEVICE_DT_*`/`DT_*` 族整体移除，只留 `DEVICE_DEFINE`/`DEVICE_DEINIT_DEFINE`/`DEVICE_GET`/`DEVICE_INIT_GET`/`device_handle_get/from_handle`/`device_get_binding` 声明 |
| `syscalls/device.h`（生成） | 手写 5 个 `static inline`：`device_get_binding`/`device_is_ready`/`device_init`/`device_deinit`/`device_get_by_dt_nodelabel`（直接转发 `z_impl_*` 或内联实现） |
| `kernel/device.c` | 重写为 `bare_device.c`：`do_device_init` + `device_get_binding` + `device_is_ready` + `z_device_get_all_static`（去掉 kobject/syscall/pm_runtime） |
| `kernel/init.c` | 重写为 `bare_init.c`：`run_level` 遍历 `__init_*_start/end`（§4.4） |
| `include/zephyr/devicetree.h`（+`devicetree/*`） | 替换为 §2.1 的 shim |
| `include/zephyr/pm/state.h` | 替换为空 shim（或删 include） |
| `include/zephyr/llext/symbol.h` | 替换为 `EXPORT_*` 空宏 shim |
| `include/zephyr/sys/device_mmio.h` | 仅当需要 MMIO 辅助时移植；裸机可改用 CMSIS 寄存器头或自写 `sys_read32/write32` |
| `include/zephyr/logging/log_msg.h` + `sys/cbprintf.h` + `sys/mpsc_pbuf.h` + `log_msg.c` | **cbprintf 打包层**：建议整体替换为 `vsnprintf` 直出（配 MINIMAL 或自写 backend），否则需连 `lib/os/cbprintf*.c`、`sys/mpsc_*.c` 一起移植 |
| `include/zephyr/logging/log_ctrl.h` + `log_core.c` | 精简为 `log_core_init()`（遍历 `log_backend` 段激活 autostart）+ 过滤位域管理 |
| 链接脚本 | 自写 Keil `.sct`：复刻 `CREATE_OBJ_LEVEL`（`initlevel` 段）+ `._device.static.*`（device 数组）+ `.z_devstate` + `log_const/log_backend` 等 iterable 段（§3.4） |

### 6.4 建议的最小落地目录结构

```
port/zephyr/
  include/zephyr/toolchain.h, toolchain/{common,gcc,llvm,armclang}.h
  include/zephyr/types.h
  include/zephyr/sys/{util_macro.h, util_internal.h, util_loops.h, util_internal_*.h,
                      iterable_sections.h, util.h, __assert.h, zassert.h}
  include/zephyr/init.h
  include/zephyr/device.h                    # 裁剪版
  include/zephyr/syscalls/device.h           # 手写 static inline
  include/zephyr/logging/{log.h, log_core.h, log_instance.h, log_backend.h}  # 裁剪版
  include/zephyr/autoconf.h                  # 手写 CONFIG_* 固定值
  include/zephyr/shim/{devicetree.h, pm_state.h, llext_symbol.h}
  src/bare_init.c                            # run_level 遍历
  src/bare_device.c                          # do_device_init / device_get_binding / is_ready
  src/printk.c                               # CMSIS ITM 或 SEGGER_RTT 落地
  stm32g474.sct                              # scatter：initlevel/device/devstate/log_* 段
```

---

## 附：关键结论核对清单（每一条都有源码落点）

- init 顺序机制：`linker-defs.h:70-74` + `init.h:119-121` + `common-rom-kernel-devices.ld:5-21`。
- 设备本体与 init entry 分离：`device.h:1318-1326`（device 段）vs `device.h:1348-1356`（init entry 段）。
- boot 序列与 `do_device_init` 分叉：`kernel/init.c:207-237, 535-616`；`kernel/device.c:18-69`。
- ARMCC 仅 armclang：`toolchain.h:57-59` + `toolchain/armclang.h:14-24`；无 ARMCC5 适配层（全树无 `toolchain/armcc5.h`）。
- `DEVICE_AND_API_INIT` 已删除：全树 grep 无匹配。
- 日志模块注册/过滤/backend：`log_instance.h:26-38, 71-78`；`log_core.h:175-183, 427-475`；`log_backend.h:74-148`。
- cbprintf 是日志打包核心依赖：`log_msg.h:18, 347-392, 526-548`。
- 构造式 init 仅 `CONFIG_STATIC_INIT_GNU`：`common-rom-init.ld:3-16` + `kernel/init.c:239-257`。

## 未确证项（明确标注）

1. **armlink scatter 对 `SORT()` 的等价能力**：Zephyr 源码只定义了 GNU ld 的 `SORT` 排序（`linker-defs.h:70-74`）；armlink 无同名机制、其输入段排序/保留语义（`KEEP` 等价物、段名排序）**未从 Zephyr 源码确证**，属于 ARM 编译器手册范畴。本报告据此给出「显式分组 scatter」或「构建期生成排序表」两条规避路径，未声称 armlink 存在直接等价指令。
2. **ARMCC5 能否编译该闭包**：源码无 ARMCC5 适配层，判定为不支持；但「AC5 具体报错/替代写法」未在 Zephyr 源码内确证。
3. **armclang 下 `.init_array` 与 `SORT_BY_INIT_PRIORITY` 的精确行为**：`common-rom-init.ld` 的 `SORT_BY_INIT_PRIORITY` 是 GNU ld 特性，armclang/armlink 侧的等价行为未在源码内确证。
