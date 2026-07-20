#!/usr/bin/env bash
# Generate + build + run accept/reject checks for ebnf-call (Java).
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
REPO="$(cd "$ROOT/../.." && pwd)"
GEN="$ROOT/scripts/generate.sh"
export LPG_BIN="${LPG_BIN:-}"

"$GEN" java
rt="$REPO/runtime/lpg-runtime"
classes="$ROOT/java/classes"
rm -rf "$classes"
mkdir -p "$classes"
rt_srcs=()
while IFS= read -r f; do
  rt_srcs+=("$f")
done < <(find "$rt/src" -name '*.java' | sort)
javac -encoding UTF-8 -d "$classes" "${rt_srcs[@]}"
if [[ -f "$rt/src/lpg/runtime/messages.properties" ]]; then
  mkdir -p "$classes/lpg/runtime"
  cp "$rt/src/lpg/runtime/messages.properties" "$classes/lpg/runtime/"
fi
mkdir -p "$ROOT/java/src/EbnfCall"
cp "$ROOT/out-java/call.java" \
   "$ROOT/out-java/callprs.java" \
   "$ROOT/out-java/callsym.java" \
   "$ROOT/java/Main.java" \
   "$ROOT/java/src/EbnfCall/"
javac -encoding UTF-8 -cp "$classes" -d "$classes" \
  "$ROOT/java/src/EbnfCall"/*.java
java -cp "$classes" EbnfCall.Main
