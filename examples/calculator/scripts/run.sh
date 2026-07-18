#!/usr/bin/env bash
# Generate + build + run accept/reject checks for calculator quickstarts.
# Usage: ./examples/calculator/scripts/run.sh [cpp|rust|java|typescript|go|python|csharp|dart|all]
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
  rt_srcs=()
  while IFS= read -r f; do
    rt_srcs+=("$f")
  done < <(find "$rt/src" -name '*.java' | sort)
  javac -encoding UTF-8 -d "$classes" "${rt_srcs[@]}"
  if [[ -f "$rt/src/lpg/runtime/messages.properties" ]]; then
    mkdir -p "$classes/lpg/runtime"
    cp "$rt/src/lpg/runtime/messages.properties" "$classes/lpg/runtime/"
  fi
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

# Rewrite generated Go files to package main (generator emits package Calculator).
_go_copy_as_main() {
  local src="$1" dest="$2"
  python3 - "$src" "$dest" <<'PY'
import pathlib, sys
src, dest = pathlib.Path(sys.argv[1]), pathlib.Path(sys.argv[2])
text = src.read_text(encoding="utf-8")
lines = text.splitlines(keepends=True)
if lines and lines[0].startswith("package "):
    lines = lines[1:]
    if lines and lines[0].strip() == "":
        lines = lines[1:]
dest.write_text("package main\n" + "".join(lines), encoding="utf-8")
PY
}

run_go() {
  "$GEN" go
  local proj="$ROOT/go/run"
  rm -rf "$proj"
  mkdir -p "$proj"
  _go_copy_as_main "$ROOT/out-go/calculator.go" "$proj/calculator.go"
  _go_copy_as_main "$ROOT/out-go/calculatorprs.go" "$proj/calculatorprs.go"
  _go_copy_as_main "$ROOT/out-go/calculatorsym.go" "$proj/calculatorsym.go"
  cp "$ROOT/go/main.go" "$proj/main.go"
  cat > "$proj/go.mod" <<EOF
module lpg2_calculator_go

go 1.21

require github.com/A-LPG/LPG-go-runtime v0.0.0

replace github.com/A-LPG/LPG-go-runtime => $REPO/runtime/LPG-go-runtime
EOF
  (cd "$proj" && go mod tidy && go run .)
}

run_python() {
  "$GEN" python3
  local gen="$ROOT/python/generated"
  rm -rf "$gen"
  mkdir -p "$gen"
  cp "$ROOT/out-python/calculatorprs.py" \
     "$ROOT/out-python/calculatorsym.py" \
     "$gen/"
  # Generated AST annotations forward-reference sibling classes; defer evaluation.
  {
    echo "from __future__ import annotations"
    cat "$ROOT/out-python/calculator.py"
  } > "$gen/calculator.py"
  PYTHONPATH="$REPO/runtime/LPG-python-runtime/Python3/src:$gen" \
    python3 "$ROOT/python/main.py"
}

run_csharp() {
  "$GEN" csharp
  cp "$ROOT/out-csharp/calculator.cs" \
     "$ROOT/out-csharp/calculatorprs.cs" \
     "$ROOT/out-csharp/calculatorsym.cs" \
     "$ROOT/csharp/"
  (cd "$ROOT/csharp" && dotnet run -c Release --project calculator_lpg_example.csproj)
}

run_dart() {
  "$GEN" dart
  mkdir -p "$ROOT/dart/generated"
  cp "$ROOT/out-dart/calculator.dart" \
     "$ROOT/out-dart/calculatorprs.dart" \
     "$ROOT/out-dart/calculatorsym.dart" \
     "$ROOT/dart/generated/"
  (cd "$ROOT/dart" && dart pub get && dart run bin/main.dart)
}

case "$LANG" in
  cpp) run_cpp ;;
  rust) run_rust ;;
  java) run_java ;;
  typescript|ts) run_typescript ;;
  go) run_go ;;
  python|python3) run_python ;;
  csharp|cs) run_csharp ;;
  dart) run_dart ;;
  all)
    run_cpp
    run_rust
    run_java
    run_typescript
    run_go
    run_python
    run_csharp
    run_dart
    ;;
  *)
    echo "Usage: $0 [cpp|rust|java|typescript|go|python|csharp|dart|all]" >&2
    exit 1
    ;;
esac
