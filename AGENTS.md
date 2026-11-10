## Agent skills

### Issue tracker

Issues live in GitHub Issues for `zhengjy926/psg-v3`. See `docs/agents/issue-tracker.md`.

### Triage labels

Default vocabulary: `needs-triage`, `needs-info`, `ready-for-agent`, `ready-for-human`, `wontfix`. See `docs/agents/triage-labels.md`.

### Domain docs

Single-context: one root `GLOSSARY.md` and `docs/adr/`. See `docs/agents/domain.md`.

### Zephyr upstream source

复用外设驱动时优先复用 Zephyr 上游驱动，不重复造轮子。上游 Zephyr 检出在
`D:\Zephyr\zephyrproject\zephyr`（west 工作区根为 `D:\Zephyr\zephyrproject`）。
移植或适配某个驱动前，先读该驱动的上游源码（例如
`D:\Zephyr\zephyrproject\zephyr\drivers\interrupt_controller\intc_gpio_stm32.c`、
`D:\Zephyr\zephyrproject\zephyr\drivers\gpio\gpio_stm32.c`），判断能否原样复用或最小改动，
而不是另写平行实现。仓库内 `drivers/` 是移植副本，可能已偏离上游；对照时以上游为准。

### Research reports

Research reports are committed to `main` under `docs/research/`.

### Coding standards

Judgement-call rules live in `CODING_STANDARDS.md` (read during review). Mechanical checks run in CI.

### Target builds

目标固件改动必须同时通过两套构建：`tools/build_target.ps1`（脚本 / CI）与 Keil uVision 工程 `project/MDK-ARM/PFA_PSG.uvprojx`（`UV4 -r`，要求 0 Error）。见 `docs/build.md`。
