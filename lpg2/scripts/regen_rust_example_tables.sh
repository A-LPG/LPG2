#!/usr/bin/env bash
# Regenerate LPG-rust-runtime/examples/generated_tables from the self-hosting grammar.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
LPG_BIN="${LPG_BIN:-}"
if [[ -z "$LPG_BIN" ]]; then
  if [[ -x "$ROOT/build/lpg-v2.3.0" ]]; then
    LPG_BIN="$ROOT/build/lpg-v2.3.0"
  elif command -v lpg-v2.3.0 >/dev/null 2>&1; then
    LPG_BIN="$(command -v lpg-v2.3.0)"
  else
    echo "Set LPG_BIN to the lpg2 executable" >&2
    exit 1
  fi
fi
OUT="$(cd "$ROOT/../LPG-rust-runtime/examples/generated_tables/src" && pwd)"
"$LPG_BIN" -programming_language=rust -table -quiet \
  -out_directory="$OUT" \
  "$ROOT/grammar/jikespg.g"
echo "Regenerated tables in $OUT"
