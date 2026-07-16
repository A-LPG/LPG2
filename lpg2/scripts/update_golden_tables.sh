#!/usr/bin/env bash
# Regenerate golden parse tables under lpg2/tests/golden/.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
LPG_BIN="${LPG_BIN:-}"
if [[ -z "$LPG_BIN" ]]; then
  shopt -s nullglob
  for candidate in "$ROOT/build"/lpg-v* "$ROOT/build-plan"/lpg-v*; do
    if [[ -x "$candidate" ]]; then
      LPG_BIN="$candidate"
      break
    fi
  done
  shopt -u nullglob
  if [[ -z "${LPG_BIN:-}" ]] && command -v lpg-v2.3.0 >/dev/null 2>&1; then
    LPG_BIN="$(command -v lpg-v2.3.0)"
  fi
  if [[ -z "${LPG_BIN:-}" || ! -x "$LPG_BIN" ]]; then
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
update_lang java minimalprs.java minimalsym.java
update_lang go minimalprs.go minimalsym.go
update_lang python3 minimalprs.py minimalsym.py
update_lang csharp minimalprs.cs minimalsym.cs
update_lang typescript minimalprs.ts minimalsym.ts
update_lang dart minimalprs.dart minimalsym.dart
