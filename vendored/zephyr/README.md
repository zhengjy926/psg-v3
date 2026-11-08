# vendored/zephyr — 设备模型 + 初始化 + 日志的最小可移植闭包

本目录是从 **Zephyr v4.5.0**（Apache-2.0，见 `LICENSE.txt`）移植到裸机
STM32G474 + armclang（AC6）的**最小闭包**，覆盖三块：

- **设备模型**（`include/zephyr/device.h`、`src/device.c`）：`struct device` /
  `struct device_state` / `struct device_ops`，`DEVICE_DEFINE()` /
  `DEVICE_GET()` / `device_get_binding()` / `device_is_ready()` /
  `do_device_init()`。软件设备路线（devicetree/PM/LLEXT/依赖 全部 shim 掉）。
- **初始化**（`include/zephyr/init.h`、`src/init.c`）：`SYS_INIT()` /
  `SYS_INIT_NAMED()`，`struct init_entry` 落 iterable section，
  `z_sys_init_run_level()` / `z_sys_init_run_all()` 按 level+priority 遍历调用。
- **日志**（`include/zephyr/logging/*`、`src/logging.c`）：`LOG_MODULE_REGISTER()`
  （模块注册入 `log_const` 段）+ `LOG_ERR/WRN/INF/DBG`（编译期等级裁剪）+
  `log_backend` 段 + 自写 `vsnprintf` 直出（MINIMAL 路径，无 cbprintf、无 libc printf）。

## 与上游的差异（要点）

1. **init 段名零填充**：`.z_init_<LEVEL>_P_<prio>_SUB_<sub>_` 中的 `prio`/`sub`
   固定为 3 位十进制（`P_001`、`P_010`…）。armlink 对输入段按段名**字典序**排序、
   没有 GNU ld 的 `SORT()`；零填充让字典序等于数值序，使 scatter 文件用单个
   `*(.z_init_<LEVEL>_P_*)` 选择符即可得到正确的 level+priority 顺序（见研究票 #2
   的「未确证」项，本仓已实证确认）。上游用未填充的 `P_<prio>` + GNU ld 的
   1/2/3 位分组 SORT。
2. **level 边界由链接器提供**：armlink 下每个 init level 各占一个执行区
   （`ER_INIT_<LEVEL>`），边界取 `Image$$<区名>$$Base/Limit`；GNU ld（主机测试）
   下由链接脚本 `__init_<LEVEL>_start/end` 符号提供。
3. **设备对象 extern 链接**：便于 `board/devices.c` 组合根跨编译单元用
   `DEVICE_GET()` 引用（上游软件设备为 `static`）。
4. 其余见各文件头注释与 `docs/research`（票 #2）。

## 上游来源

| 本仓文件 | Zephyr v4.5.0 来源 |
| --- | --- |
| `include/zephyr/toolchain.h` | `toolchain.h` + `toolchain/common.h` + `toolchain/gcc.h` |
| `include/zephyr/sys/util_macro.h` | `sys/util_macro.h` + `sys/util_internal.h` |
| `include/zephyr/sys/util.h` | `sys/util.h` |
| `include/zephyr/sys/iterable_sections.h` | `sys/iterable_sections.h` |
| `include/zephyr/sys/__assert.h` | `sys/__assert.h` |
| `include/zephyr/init.h` | `init.h` |
| `include/zephyr/device.h` | `device.h` |
| `include/zephyr/logging/*` | `logging/{log,log_core,log_instance,log_backend}.h` |
| `src/init.c` | `kernel/init.c` |
| `src/device.c` | `kernel/device.c` |
| `src/logging.c` | `subsys/logging/log_minimal.c` |
