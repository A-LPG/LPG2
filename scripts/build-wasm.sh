#!/usr/bin/env bash
# Build lpg2 with Emscripten for the browser playground.
# Requires: emcmake / emcc on PATH (emsdk activate).
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUT="${LPG2_WASM_OUT:-$ROOT/playground/wasm}"
BUILD="${LPG2_WASM_BUILD:-$ROOT/lpg2/build-wasm}"
TEMPLATES="$ROOT/lpg-generator-templates-2.1.00"

if ! command -v emcmake >/dev/null 2>&1; then
  echo "error: emcmake not found. Install emsdk and run 'source emsdk_env.sh'." >&2
  exit 1
fi

if [[ ! -d "$TEMPLATES/templates" || ! -d "$TEMPLATES/include" ]]; then
  echo "error: templates not found at $TEMPLATES" >&2
  exit 1
fi

mkdir -p "$OUT" "$BUILD"

# Preload templates at the same path FindLpgResourceRoot / LPG2_RESOURCE_ROOT expect.
PRELOAD_FLAGS=(
  --preload-file "$TEMPLATES@/share/lpg2/lpg-generator-templates-2.1.00"
)

emcmake cmake -S "$ROOT/lpg2" -B "$BUILD" \
  -DCMAKE_BUILD_TYPE=Release \
  -DLPG2_WASM=ON \
  -DLPG2_WARNINGS_AS_ERRORS=OFF \
  -DLPG2_ENABLE_SANITIZERS=OFF

cmake --build "$BUILD" --parallel

# Locate the generated js/wasm (CMake OUTPUT_NAME is lpg-v2.3.0).
JS="$(find "$BUILD" -maxdepth 2 -type f -name 'lpg*.js' | head -n1)"
WASM="$(find "$BUILD" -maxdepth 2 -type f -name 'lpg*.wasm' | head -n1)"
DATA="$(find "$BUILD" -maxdepth 2 -type f -name 'lpg*.data' | head -n1 || true)"

if [[ -z "${JS:-}" || -z "${WASM:-}" ]]; then
  echo "error: emscripten outputs not found under $BUILD" >&2
  echo "hint: ensure LPG2_WASM links with -sMODULARIZE and produces .js/.wasm" >&2
  exit 1
fi

cp "$JS" "$OUT/lpg2.js"
cp "$WASM" "$OUT/lpg2.wasm"
if [[ -n "${DATA:-}" ]]; then
  cp "$DATA" "$OUT/$(basename "$DATA")"
  # Keep filename referenced by the js glue if needed.
  if [[ "$(basename "$DATA")" != "lpg2.data" ]]; then
    cp "$DATA" "$OUT/lpg2.data" || true
  fi
fi

echo "Wrote WASM playground artifacts to $OUT"
