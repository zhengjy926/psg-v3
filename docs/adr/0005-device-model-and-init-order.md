# 设备模型与 init 顺序

## 背景

固件移植 Zephyr v4.5.0 的设备模型（`struct device` + 按类别 api ops）与 init level + priority 机制，现落于 `lib/utils/device.{c,h}` 与 `lib/utils/init_runner.c`，不引入 RTOS、devicetree 与 Kconfig（无设备树，设备均为软件设备）。

## 决策

- 设备经 `DEVICE_DEFINE`/`DEVICE_GET` 注册，按名经 `device_get_binding`/`device_is_ready` 查找；无运行期依赖检查。
- **init 顺序由构建期生成的排序表驱动**，不依赖链接器排序：`tools/generate_init_order.py` 从 `.o` 提取 `__init_*` 符号及其段名里的 (level, prio, sub)，按数字排序后生成 `lib/utils/init_order.inc`（target，fromelf 后端）与 `lib/utils/init_order_host.inc`（host，readelf 后端）。`init_runner.c` 遍历该表，提供 `z_sys_init_run_all()`（全量）与 `z_sys_init_run_level(level)`（按级）。
- 两套排序表均为**入库生成文件**：target 由 `build_target.ps1` 每次重生成（单遍链接：编译其余对象 → 生成表 → 编译 init_runner → 链接）；host 由 CI 用 readelf 后端重生成并 drift 校验。Keil 直接编译入库的 `init_order.inc`。
- 为使 runner 能按符号名引用 init 项，init 项去掉 `static`（`init.h` 的 `SYS_INIT_NAMED`、`device.h` 的 `Z_DEVICE_INIT_ENTRY_DEFINE`），这是相对上游的最小必要偏离。
- `device_handle_t`/`device_handle_get`/`device_from_handle` 已删除（无 deps 后为死代码）。
- iterable 段遍历是既定机制，其 MISRA 偏差见 `tools/misra/README.md`。

## 理由

armlink 无 GNU ld `SORT` 等价物，且 GNU ld 的 `SORT` 是字典序（优先级 1/5/10 会排成 1/10/5），两者都不能可靠地按数字排序 init 段。改用构建期生成的排序表后，顺序由生成器显式确定、单遍链接即可，且 target 与 host 走同一份排序逻辑，host 测试能覆盖 target 的排序行为。

## 依据

- 实现规格「设备模型移植（#2）」「启动序列（#5）」：issue #15「Implementation Decisions」。
- [ADR-0002（armclang AC6）](0002-armclang-ac6.md)：armlink 无 GNU ld `SORT` 等价物。
