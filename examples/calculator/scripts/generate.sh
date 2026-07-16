#!/usr/bin/env bash
# Generate calculator tables for cpp or rust.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
REPO="$(cd "$ROOT/../.." && pwd)"
LANG="${1:-cpp}"
LPG_BIN="${LPG_BIN:-}"

if [[ -z "$LPG_BIN" ]]; then
  shopt -s nullglob
  for c in "$REPO/lpg2/build"/lpg-v* "$REPO/lpg2/build-plan"/lpg-v*; do
    [[ -x "$c" ]] && LPG_BIN="$c" && break
  done
  shopt -u nullglob
fi
if [[ -z "${LPG_BIN:-}" || ! -x "$LPG_BIN" ]]; then
  echo "Set LPG_BIN to lpg2 executable" >&2
  exit 1
fi

out="$ROOT/out-$LANG"
mkdir -p "$out"
"$LPG_BIN" -programming_language="$LANG" -table -quiet \
  -out_directory="$out" \
  "$ROOT/calculator.g"
echo "Generated tables in $out"
ls -la "$out"
