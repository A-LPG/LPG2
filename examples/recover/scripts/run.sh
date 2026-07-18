#!/usr/bin/env bash
# Generate, compile, and run the Java %Recover cookbook.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
REPO="$(cd "$ROOT/../.." && pwd)"
LPG_BIN="${LPG_BIN:-}"

if [[ -z "$LPG_BIN" ]]; then
  shopt -s nullglob
  for candidate in \
    "$REPO/lpg2/build/lpg-v2.3.0" \
    "$REPO/lpg2"/build-*/lpg-v2.3.0; do
    if [[ -x "$candidate" ]]; then
      LPG_BIN="$candidate"
      break
    fi
  done
  shopt -u nullglob
fi

if [[ -z "${LPG_BIN:-}" || ! -x "$LPG_BIN" ]]; then
  echo "Set LPG_BIN to an lpg-v2.3.0 executable" >&2
  exit 1
fi

RUNTIME="$REPO/runtime/lpg-runtime"
if [[ ! -d "$RUNTIME/src/lpg/runtime" ]]; then
  echo "Initialize the Java runtime first:" >&2
  echo "  git submodule update --init runtime/lpg-runtime" >&2
  exit 1
fi

TEMPLATES="$REPO/lpg-generator-templates-2.1.00"
OUT="$ROOT/out-java"
CLASSES="$ROOT/java/classes"
rm -rf "$OUT" "$CLASSES"
mkdir -p "$OUT" "$CLASSES"

"$LPG_BIN" -programming_language=java -table -quiet \
  -template="$TEMPLATES/templates/java/dtParserTemplateF.gi" \
  -include-directory="$TEMPLATES/include/java" \
  -out_directory="$OUT" \
  "$ROOT/recover.g" >/dev/null

runtime_sources=()
while IFS= read -r source; do
  runtime_sources+=("$source")
done < <(find "$RUNTIME/src" -name '*.java' | sort)

javac -encoding UTF-8 -d "$CLASSES" "${runtime_sources[@]}"
if [[ -f "$RUNTIME/src/lpg/runtime/messages.properties" ]]; then
  mkdir -p "$CLASSES/lpg/runtime"
  cp "$RUNTIME/src/lpg/runtime/messages.properties" "$CLASSES/lpg/runtime/"
fi

javac -encoding UTF-8 -cp "$CLASSES" -d "$CLASSES" \
  "$OUT/recover.java" \
  "$OUT/recoverprs.java" \
  "$OUT/recoversym.java" \
  "$ROOT/java/Main.java"

java -cp "$CLASSES" Recover.Main
