## Agent skills

### Issue tracker

Issues live in GitHub Issues for `zhengjy926/psg-v3`. See `docs/agents/issue-tracker.md`.

### Triage labels

Default vocabulary: `needs-triage`, `needs-info`, `ready-for-agent`, `ready-for-human`, `wontfix`. See `docs/agents/triage-labels.md`.

### Domain docs

Single-context: one root `GLOSSARY.md` and `docs/adr/`. See `docs/agents/domain.md`.

### Research reports

Research reports are committed to `main` under `docs/research/`.

### Coding standards

Judgement-call rules live in `CODING_STANDARDS.md` (read during review). Mechanical checks run in CI.

### Target builds

目标固件改动必须同时通过两套构建：`tools/build_target.ps1`（脚本 / CI）与 Keil uVision 工程 `project/MDK-ARM/PFA_PSG.uvprojx`（`UV4 -r`，要求 0 Error）。见 `docs/build.md`。
