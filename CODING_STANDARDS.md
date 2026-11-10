# 编码规范

评审代理（code-review 的 Standards 轴）读本文件。实现代理不必逐条背诵；机械规则已由 CI 强制（`tools/check_no_stm32.py`、`.clang-tidy`、`tools/static_analysis.sh`）。架构约束（分层与接缝、设备模型与 init）见 `docs/adr/0004-layering-and-host-clean-line.md` 与 `docs/adr/0005-device-model-and-init-order.md`。

## 语言与工具链

- C11（`-std=c11`）+ MISRA-C:2012 + 偏差表（`tools/misra/README.md`）。
- 目标只用 armclang AC6。
- 禁动态内存 / 递归 / 变长数组。

## 命名与错误

- 领域词一律按 `GLOSSARY.md`，不得用其 `_Avoid_` 同义词（例：「正向过渡」而非「上升沿」）。
- 错误码用负 errno（Zephyr errno 风格）。
