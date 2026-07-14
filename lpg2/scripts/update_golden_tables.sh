#!/usr/bin/env bash
# Regenerate golden parse tables under lpg2/tests/golden/.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
LPG_BIN="${LPG_BIN:-}"
if [[ -z "$LPG_BIN" ]]; then
  if [[ -x "$ROOT/build/lpg-v2.2.03" ]]; then
    LPG_BIN="$ROOT/build/lpg-v2.2.03"
  elif command -v lpg-v2.2.03 >/dev/null 2>&1; then
    LPG_BIN="$(command -v lpg-v2.2.03)"
  else
    echo "Set LPG_BIN to the lpg2 executable" >&2
    exit 1
  fi
fi

OUT="$ROOT/tests/golden/minimal/cpp"
mkdir -p "$OUT"
"$LPG_BIN" -programming_language=cpp -table -quiet \
  -out_directory="$OUT" \
  "$ROOT/tests/fixtures/minimal.g"

# Keep only the prs golden artifacts (header + implementation).
find "$OUT" -mindepth 1 -maxdepth 1 ! -name 'minimalprs.h' ! -name 'minimalprs.cpp' -exec rm -f {} +

echo "Updated golden tables in $OUT"
ls -la "$OUT"
