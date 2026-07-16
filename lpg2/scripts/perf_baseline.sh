#!/usr/bin/env bash
# Measure generator wall time / peak RSS on a large grammar (jdt.core by default).
# Usage:
#   ./scripts/perf_baseline.sh
#   LPG_BIN=./build/lpg-v2.3.0 GRAMMAR=../grammars-example/java/GJavaParser.g RUNS=5 ./scripts/perf_baseline.sh
set -euo pipefail

LPG_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
ROOT="$(cd "$LPG_ROOT/.." && pwd)"
LPG_BIN="${LPG_BIN:-}"
GRAMMAR="${GRAMMAR:-$LPG_ROOT/grammar/jikespg.g}"
LPG_LANGUAGE="${LPG_LANGUAGE:-cpp}"
RUNS="${RUNS:-3}"
OUT_DIR="${OUT_DIR:-$(mktemp -d -t lpg2-perf-XXXXXX)}"

if [[ -z "$LPG_BIN" ]]; then
  shopt -s nullglob
  for candidate in "$LPG_ROOT/build"/lpg-v* "$LPG_ROOT/build-plan"/lpg-v*; do
    if [[ -x "$candidate" ]]; then
      LPG_BIN="$candidate"
      break
    fi
  done
  shopt -u nullglob
fi

if [[ -z "${LPG_BIN:-}" || ! -x "$LPG_BIN" ]]; then
  echo "Set LPG_BIN to the lpg2 executable" >&2
  exit 1
fi

if [[ ! -f "$GRAMMAR" ]]; then
  echo "Grammar not found: $GRAMMAR" >&2
  echo "Initialize submodule: git submodule update --init grammars-example" >&2
  exit 1
fi

mkdir -p "$OUT_DIR"
RESULT="$OUT_DIR/perf_baseline.txt"
VERSION="$(basename "$LPG_BIN")"

{
  echo "# LPG2 performance baseline"
  echo "binary: $LPG_BIN"
  echo "version: $VERSION"
  echo "grammar: $GRAMMAR"
  echo "language: $LPG_LANGUAGE"
  echo "runs: $RUNS"
  echo "timestamp: $(date -u +%Y-%m-%dT%H:%M:%SZ)"
  echo
  echo "run,wall_sec,max_rss_kb,exit_code"
} | tee "$RESULT"

sum_wall=0
sum_rss=0
ok=0

for i in $(seq 1 "$RUNS"); do
  run_out="$OUT_DIR/run-$i"
  mkdir -p "$run_out"
  time_err="$OUT_DIR/time-$i.txt"
  set +e
  # Prefer high-resolution wall clock; fall back to /usr/bin/time for RSS.
  start_ns=$(python3 -c 'import time; print(time.perf_counter_ns())')
  "$LPG_BIN" -programming_language="$LPG_LANGUAGE" -table -quiet \
    -out_directory="$run_out" "$GRAMMAR" >"$OUT_DIR/stdout-$i.txt" 2>"$OUT_DIR/stderr-$i.txt"
  ec=$?
  end_ns=$(python3 -c 'import time; print(time.perf_counter_ns())')
  wall=$(python3 -c "print(f'{($end_ns - $start_ns)/1e9:.6f}')")
  rss=0
  if /usr/bin/time -l true >/dev/null 2>&1; then
    /usr/bin/time -l "$LPG_BIN" -programming_language="$LPG_LANGUAGE" -table -quiet \
      -out_directory="$run_out-rss" "$GRAMMAR" >/dev/null 2>"$time_err"
    rss_bytes=$(awk '/maximum resident set size/{print $1; exit}' "$time_err")
    rss=$((${rss_bytes:-0} / 1024))
  elif /usr/bin/time -f '%M' -o "$time_err" true >/dev/null 2>&1; then
    /usr/bin/time -f '%M' -o "$time_err" "$LPG_BIN" \
      -programming_language="$LPG_LANGUAGE" -table -quiet \
      -out_directory="$run_out-rss" "$GRAMMAR" >/dev/null 2>/dev/null
    rss=$(cat "$time_err")
  fi
  set -e
  wall="${wall:-0}"
  rss="${rss:-0}"
  echo "$i,$wall,$rss,$ec" | tee -a "$RESULT"
  if [[ "$ec" -eq 0 ]]; then
    sum_wall=$(awk -v a="$sum_wall" -v b="$wall" 'BEGIN{printf "%.6f", a+b}')
    sum_rss=$((sum_rss + rss))
    ok=$((ok + 1))
  fi
done

if [[ "$ok" -gt 0 ]]; then
  avg_wall=$(awk -v s="$sum_wall" -v n="$ok" 'BEGIN{printf "%.3f", s/n}')
  avg_rss=$((sum_rss / ok))
else
  avg_wall=0
  avg_rss=0
fi

{
  echo
  echo "successful_runs: $ok"
  echo "average_wall_sec: $avg_wall"
  echo "average_max_rss_kb: $avg_rss"
  echo "artifacts: $OUT_DIR"
} | tee -a "$RESULT"

echo
echo "Baseline written to $RESULT"
