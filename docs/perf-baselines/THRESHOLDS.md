# Soft performance thresholds

Baseline grammar: `lpg2/grammar/jikespg.g`, language `cpp`.

| Metric | Soft max | Notes |
|--------|----------|-------|
| `average_wall_sec` | **0.150** | ~6× local v2.3.0 baseline (~0.025s on Apple Silicon). CI Ubuntu is often slower. |
| `average_max_rss_kb` | **65536** | Soft cap; informational on Linux `/usr/bin/time -f %M`. |

## Policy

- CI job `perf-baseline` remains `continue-on-error: true`.
- When the soft wall-time threshold is exceeded, the job prints a red marker in the GitHub Step Summary and exits with code `2` (non-blocking).
- If the threshold is exceeded on **3 consecutive** `main` runs, open a GitHub issue labeled `perf` and investigate before raising the soft max.

Recorded baseline: [v2.3.0/perf_baseline.txt](v2.3.0/perf_baseline.txt).

```bash
LPG_BIN=./lpg2/build-plan/lpg-v2.3.0 RUNS=3 \
  PERF_WALL_SOFT_MAX=0.150 \
  ./lpg2/scripts/perf_baseline.sh
```
