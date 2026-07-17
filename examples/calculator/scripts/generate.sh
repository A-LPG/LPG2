#!/usr/bin/env bash
# Generate calculator tables+AST for cpp|rust|java|typescript (rt_cpp for C++).
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
REPO="$(cd "$ROOT/../.." && pwd)"
LANG_IN="${1:-}"
LPG_BIN="${LPG_BIN:-}"

usage() {
  echo "Usage: $0 cpp|rust|java|typescript" >&2
  exit 1
}
[[ -n "$LANG_IN" ]] || usage

case "$LANG_IN" in
  cpp|c++|rt_cpp) LANG=rt_cpp; OUT_KEY=cpp; TPL_LANG=rt_cpp ;;
  rust) LANG=rust; OUT_KEY=rust; TPL_LANG=rust ;;
  java) LANG=java; OUT_KEY=java; TPL_LANG=java ;;
  typescript|ts) LANG=typescript; OUT_KEY=typescript; TPL_LANG=typescript ;;
  *) usage ;;
esac

if [[ -z "$LPG_BIN" ]]; then
  shopt -s nullglob
  for c in "$REPO/lpg2/build"/lpg-v* "$REPO/lpg2/build-plan"/lpg-v* "$REPO/lpg2/build-prosthetic-b"/lpg-v*; do
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

"$LPG_BIN" -programming_language="$LANG" -table -quiet \
  -template="$TEMPLATES/templates/$TPL_LANG/dtParserTemplateF.gi" \
  -include-directory="$TEMPLATES/include/$TPL_LANG" \
  -out_directory="$out" \
  "$ROOT/calculator.g"

echo "Generated tables in $out"
ls -la "$out"
