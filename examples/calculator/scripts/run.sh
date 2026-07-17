#!/usr/bin/env bash
# Generate + build + run accept/reject checks for calculator quickstarts.
# Usage: ./examples/calculator/scripts/run.sh [cpp|rust|java|typescript|all]
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
REPO="$(cd "$ROOT/../.." && pwd)"
LANG="${1:-all}"
GEN="$ROOT/scripts/generate.sh"
export LPG_BIN="${LPG_BIN:-}"

run_cpp() {
  "$GEN" cpp
  cmake -S "$ROOT/cpp" -B "$ROOT/cpp/build" \
    -DLPG2_CPP_RUNTIME_DIR="$REPO/runtime/LPG-cpp-runtime"
  cmake --build "$ROOT/cpp/build" -j
  "$ROOT/cpp/build/calculator_check"
}

run_rust() {
  "$GEN" rust
  (cd "$ROOT/rust" && cargo test)
}

run_java() {
  "$GEN" java
  local rt="$REPO/runtime/lpg-runtime"
  local classes="$ROOT/java/classes"
  rm -rf "$classes"
  mkdir -p "$classes"
  # Compile runtime
  rt_srcs=()
  while IFS= read -r f; do
    rt_srcs+=("$f")
  done < <(find "$rt/src" -name '*.java' | sort)
  javac -encoding UTF-8 -d "$classes" "${rt_srcs[@]}"
  if [[ -f "$rt/src/lpg/runtime/messages.properties" ]]; then
    mkdir -p "$classes/lpg/runtime"
    cp "$rt/src/lpg/runtime/messages.properties" "$classes/lpg/runtime/"
  fi
  # Generated sources land in package Calculator
  mkdir -p "$ROOT/java/src/Calculator"
  cp "$ROOT/out-java/calculator.java" \
     "$ROOT/out-java/calculatorprs.java" \
     "$ROOT/out-java/calculatorsym.java" \
     "$ROOT/java/Main.java" \
     "$ROOT/java/src/Calculator/"
  javac -encoding UTF-8 -cp "$classes" -d "$classes" \
    "$ROOT/java/src/Calculator"/*.java
  java -cp "$classes" Calculator.Main
}

run_typescript() {
  "$GEN" typescript
  mkdir -p "$ROOT/typescript/generated"
  cp "$ROOT/out-typescript/calculator.ts" \
     "$ROOT/out-typescript/calculatorprs.ts" \
     "$ROOT/out-typescript/calculatorsym.ts" \
     "$ROOT/typescript/generated/"
  if [[ -f "$REPO/runtime/LPG-typescript-runtime/lpg2ts/package.json" ]]; then
    (cd "$REPO/runtime/LPG-typescript-runtime/lpg2ts" && \
      npm install --no-fund --no-audit >/dev/null 2>&1 || true)
    if grep -q '"build"' "$REPO/runtime/LPG-typescript-runtime/lpg2ts/package.json"; then
      (cd "$REPO/runtime/LPG-typescript-runtime/lpg2ts" && npm run build || true)
    fi
  fi
  (cd "$ROOT/typescript" && npm install --no-fund --no-audit && npm start)
}

case "$LANG" in
  cpp) run_cpp ;;
  rust) run_rust ;;
  java) run_java ;;
  typescript|ts) run_typescript ;;
  all)
    run_cpp
    run_rust
    run_java
    run_typescript
    ;;
  *)
    echo "Usage: $0 [cpp|rust|java|typescript|all]" >&2
    exit 1
    ;;
esac
