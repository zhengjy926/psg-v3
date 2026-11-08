## Agent skills

### Issue tracker

Issues live in GitHub Issues for `zhengjy926/psg-v3`. See `docs/agents/issue-tracker.md`.

### Triage labels

Default vocabulary: `needs-triage`, `needs-info`, `ready-for-agent`, `ready-for-human`, `wontfix`. See `docs/agents/triage-labels.md`.

### Domain docs

Single-context: one root `GLOSSARY.md` and `docs/adr/`. See `docs/agents/domain.md`.

### Research reports

Research reports live on `research/<topic>` branches, not `main`: `git branch -a`, then `git show <branch>:docs/research/<file>`.

### Coding standards

Judgement-call rules live in `CODING_STANDARDS.md` (read during review). Mechanical checks run in CI.
