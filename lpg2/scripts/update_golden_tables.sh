#!/usr/bin/env bash
# Regenerate golden parse tables under lpg2/tests/golden/.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
LPG_BIN="${LPG_BIN:-}"
if [[ -z "$LPG_BIN" ]]; then
  if [[ -x "$ROOT/build/lpg-v2.2.03" ]]; then
    LPG_BIN="$ROOT/build/lpg-v2.2.03"
  elif [[ -x "$ROOT/build-plan/lpg-v2.2.03" ]]; then
    LPG_BIN="$ROOT/build-plan/lpg-v2.2.03"
  elif command -v lpg-v2.2.03 >/dev/null 2>&1; then
    LPG_BIN="$(command -v lpg-v2.2.03)"
  else
    echo "Set LPG_BIN to the lpg2 executable" >&2
    exit 1
  fi
fi

update_lang() {
  local lang="$1"
  shift
  local out="$ROOT/tests/golden/minimal/${lang}"
  mkdir -p "$out"
  "$LPG_BIN" -programming_language="$lang" -table -quiet \
    -out_directory="$out" \
    "$ROOT/tests/fixtures/minimal.g"
  # Keep only table golden artifacts listed by the caller.
  local keep=("$@")
  local find_args=()
  local f
  for f in "${keep[@]}"; do
    find_args+=(! -name "$f")
  done
  find "$out" -mindepth 1 -maxdepth 1 "${find_args[@]}" -exec rm -f {} +
  echo "Updated golden tables in $out"
  ls -la "$out"
}

update_lang cpp minimalprs.h minimalprs.cpp
update_lang rust minimalprs.rs minimalsym.rs
