# 编码规范

评审代理（code-review 的 Standards 轴）读本文件。实现代理不必逐条背诵；机械规则已由 CI 强制（`tools/check_no_stm32.py`、`.clang-tidy`、`tools/static_analysis.sh`）。领域词一律按 `GLOSSARY.md`；分层与接缝的权威出处是 issue #15「Implementation Decisions」。

## 分层与接缝（硬约束）

- 五层 + 两底座：`board / drivers / osal / service / app`；`vendored/zephyr/`（设备模型 + init + 日志）；`third_party/ecsp/`（协议库）。
- **主机清洁线 = `drivers/api` 的 ops 结构体**。此线以上（`osal/service/app` 与 `drivers/api` 自身）禁含 STM32/MCU 头；此线以下（`drivers/<drv>/`、`board/`）可用。构建期由 `tools/check_no_stm32.py` 强制（直接 include 级）。
- `app` 不碰 `device_get_binding`；设备绑定在 `board/devices.c` 组合根完成，注入 service config。
- service 之间零互调；能力/机制归 service，生命周期决策归 app。

## 语言与工具链

- C11（`-std=c11`），手写守 C99 风格子集 + MISRA-C:2012 + 偏差表（`tools/misra/README.md`）。
- 目标只用 armclang AC6（`docs/adr/0002-armclang-ac6.md`），不用 ARMCC5。
- 禁动态内存 / 递归 / 变长数组。

## 命名与错误

- 领域词一律按 `GLOSSARY.md`，不得用其 `_Avoid_` 同义词（例：「正向过渡」而非「上升沿」）。
- 错误码用负 errno（Zephyr errno 风格）。

## 设备模型与 init

- 设备经 `DEVICE_DEFINE`/`DEVICE_GET` 注册；init 顺序只由 level+prio 编码进链接段（`stm32g474.sct` / `tests/host/init_sections.ld`），无运行期依赖检查。
- iterable 段遍历与链接符号边界是既定机制，其 MISRA 偏差见 `tools/misra/README.md`。
