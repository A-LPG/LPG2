#!/usr/bin/env bash
# Generate ebnf-call tables+AST (Java by default).
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
REPO="$(cd "$ROOT/../.." && pwd)"
LANG_IN="${1:-java}"
LPG_BIN="${LPG_BIN:-}"

case "$LANG_IN" in
  java) LANG=java; OUT_KEY=java; TPL_LANG=java ;;
  *) echo "Usage: $0 java" >&2; exit 1 ;;
esac

if [[ -z "$LPG_BIN" ]]; then
  shopt -s nullglob
  for c in "$REPO/lpg2/build"/lpg-v*; do
    [[ -x "$c" ]] && LPG_BIN="$c" && break
  done
  shopt -u nullglob
fi
if [[ -z "${LPG_BIN:-}" || ! -x "$LPG_BIN" ]]; then
  echo "Set LPG_BIN to lpg2 executable" >&2
  exit 1
fi

TEMPLATES="$REPO/lpg-generator-templates-2.1.00"
out="$ROOT/out-$OUT_KEY"
mkdir -p "$out"
rm -f "$out"/*

"$LPG_BIN" -programming_language="$LANG" -table -quiet -fail_on_conflicts \
  -template="$TEMPLATES/templates/$TPL_LANG/dtParserTemplateF.gi" \
  -include-directory="$TEMPLATES/include/$TPL_LANG" \
  -out_directory="$out" \
  "$ROOT/call.g"

echo "Generated tables in $out"
ls -la "$out"
