#!/usr/bin/env bash
# Developer bootstrap: check toolchains, init needed submodules, print CMake tips.
#
# Usage (from repo root):
#   ./scripts/dev-bootstrap.sh [--init-submodules] [--full]
#
# --init-submodules  git submodule update --init for generator-critical paths
# --full             also init all runtime/ + tool/ submodules
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

INIT_SUB=0
FULL=0
for arg in "$@"; do
  case "$arg" in
    --init-submodules) INIT_SUB=1 ;;
    --full) FULL=1; INIT_SUB=1 ;;
    -h|--help)
      sed -n '2,12p' "$0" | sed 's/^# \{0,1\}//'
      exit 0
      ;;
  esac
done

have() { command -v "$1" >/dev/null 2>&1; }

echo "=== LPG2 developer bootstrap ==="
echo "repo: $ROOT"
echo

ok=0
miss=0
report() {
  local name="$1" status="$2" detail="${3:-}"
  if [[ "$status" == "ok" ]]; then
    printf "  [ok]   %-14s %s\n" "$name" "$detail"
    ok=$((ok + 1))
  else
    printf "  [miss] %-14s %s\n" "$name" "$detail"
    miss=$((miss + 1))
  fi
}

if have cmake; then report cmake ok "$(cmake --version | head -1)"; else report cmake miss "install CMake ≥ 3.16"; fi
if have c++ || have clang++ || have g++; then
  cxx="$(command -v c++ || command -v clang++ || command -v g++)"
  report cxx ok "$cxx"
else
  report cxx miss "need a C++17 compiler"
fi
if have rustc && have cargo; then report rust ok "$(rustc --version)"; else report rust miss "optional: rustup (Rust table/parser tests)"; fi
if have java && have javac; then report jdk ok "$(java -version 2>&1 | head -1)"; else report jdk miss "optional: JDK 8+ (Java e2e)"; fi
if have python3; then report python3 ok "$(python3 --version)"; else report python3 miss "optional: Python 3 (Python e2e)"; fi
if have go; then report go ok "$(go version)"; else report go miss "optional: Go 1.17+ (Go e2e)"; fi
if have node && have npm; then report node ok "$(node --version)"; else report node miss "optional: Node 18+ (TypeScript e2e / VS Code)"; fi
if have dotnet; then report dotnet ok "$(dotnet --version)"; else report dotnet miss "optional: .NET 8 (C# e2e)"; fi
if have dart; then report dart ok "$(dart --version 2>&1 | head -1)"; else report dart miss "optional: Dart SDK (Dart e2e)"; fi
if have yarn; then report yarn ok "$(yarn --version)"; else report yarn miss "optional: yarn (VS Code extension)"; fi

echo
echo "=== Submodules ==="
if [[ "$INIT_SUB" -eq 1 ]]; then
  if [[ "$FULL" -eq 1 ]]; then
    echo "Initializing all submodules (may take a while)…"
    git submodule update --init --recursive
  else
    echo "Initializing core + common runtime submodules…"
    git submodule update --init \
      runtime/LPG-rust-runtime \
      runtime/LPG-cpp-runtime \
      runtime/lpg-runtime \
      tool/LPG-VScode \
      tool/LPG-language-server \
      grammars-example || true
  fi
else
  echo "Skip init (pass --init-submodules or --full)."
fi

check_sub() {
  local path="$1"
  if [[ -e "$path/.git" || -f "$path/.git" ]] || [[ -d "$path" && "$(ls -A "$path" 2>/dev/null)" ]]; then
    echo "  [present] $path"
  else
    echo "  [empty]   $path  (git submodule update --init $path)"
  fi
}
check_sub runtime/LPG-rust-runtime
check_sub runtime/LPG-cpp-runtime
check_sub runtime/lpg-runtime
check_sub runtime/LPG-typescript-runtime
check_sub runtime/LPG-python-runtime
check_sub runtime/LPG-go-runtime
check_sub runtime/LPG-csharp-runtime
check_sub runtime/LPG-Dart-runtime
check_sub tool/LPG-VScode
check_sub tool/LPG-language-server

echo
echo "=== Recommended CMake configure ==="
cmake_flags=(
  -S lpg2
  -B lpg2/build
  -DCMAKE_BUILD_TYPE=Release
  -DLPG2_REQUIRE_RUST_TESTS=ON
)
e2e_notes=()

if [[ -d runtime/LPG-rust-runtime/lpg2 ]]; then
  cmake_flags+=(-DLPG2_REQUIRE_RUST_PARSER_TESTS=ON)
  cmake_flags+=(-DLPG2_RUST_RUNTIME_DIR="$ROOT/runtime/LPG-rust-runtime/lpg2")
  e2e_notes+=("rust nested/recover/behavior: enabled")
else
  e2e_notes+=("rust parser e2e: SKIPPED (no rust runtime submodule)")
fi
if [[ -d runtime/LPG-cpp-runtime ]]; then
  cmake_flags+=(-DLPG2_REQUIRE_CPP_PARSER_TESTS=ON)
  cmake_flags+=(-DLPG2_CPP_RUNTIME_DIR="$ROOT/runtime/LPG-cpp-runtime")
  e2e_notes+=("cpp nested/recover: enabled")
else
  e2e_notes+=("cpp parser e2e: SKIPPED")
fi
if [[ -d runtime/lpg-runtime ]] && have javac; then
  cmake_flags+=(-DLPG2_REQUIRE_JAVA_PARSER_TESTS=ON)
  cmake_flags+=(-DLPG2_JAVA_RUNTIME_DIR="$ROOT/runtime/lpg-runtime")
  e2e_notes+=("java nested/recover: enabled")
else
  e2e_notes+=("java parser e2e: SKIPPED")
fi
if [[ -d runtime/LPG-python-runtime/Python3/src ]] && have python3; then
  cmake_flags+=(-DLPG2_REQUIRE_PYTHON_PARSER_TESTS=ON)
  cmake_flags+=(-DLPG2_PYTHON_RUNTIME_DIR="$ROOT/runtime/LPG-python-runtime/Python3/src")
  e2e_notes+=("python nested/recover: enabled")
else
  e2e_notes+=("python parser e2e: SKIPPED")
fi
if [[ -d runtime/LPG-go-runtime ]] && have go; then
  cmake_flags+=(-DLPG2_REQUIRE_GO_PARSER_TESTS=ON)
  cmake_flags+=(-DLPG2_GO_RUNTIME_DIR="$ROOT/runtime/LPG-go-runtime")
  e2e_notes+=("go nested/recover: enabled")
else
  e2e_notes+=("go parser e2e: SKIPPED")
fi
if [[ -d runtime/LPG-typescript-runtime/lpg2ts ]] && have npm; then
  cmake_flags+=(-DLPG2_REQUIRE_TYPESCRIPT_PARSER_TESTS=ON)
  cmake_flags+=(-DLPG2_TYPESCRIPT_RUNTIME_DIR="$ROOT/runtime/LPG-typescript-runtime/lpg2ts")
  e2e_notes+=("typescript nested/recover: enabled")
else
  e2e_notes+=("typescript parser e2e: SKIPPED")
fi
if [[ -d runtime/LPG-csharp-runtime/LPG2.Runtime ]] && have dotnet; then
  cmake_flags+=(-DLPG2_REQUIRE_CSHARP_PARSER_TESTS=ON)
  cmake_flags+=(-DLPG2_CSHARP_RUNTIME_DIR="$ROOT/runtime/LPG-csharp-runtime/LPG2.Runtime")
  e2e_notes+=("csharp nested/recover: enabled")
else
  e2e_notes+=("csharp parser e2e: SKIPPED")
fi
if [[ -d runtime/LPG-Dart-runtime ]] && have dart; then
  cmake_flags+=(-DLPG2_REQUIRE_DART_PARSER_TESTS=ON)
  cmake_flags+=(-DLPG2_DART_RUNTIME_DIR="$ROOT/runtime/LPG-Dart-runtime")
  e2e_notes+=("dart nested/recover: enabled")
else
  e2e_notes+=("dart parser e2e: SKIPPED")
fi

printf 'cmake'
printf ' %q' "${cmake_flags[@]}"
echo
echo
echo "=== E2E coverage for this machine ==="
for n in "${e2e_notes[@]}"; do
  echo "  - $n"
done
echo
echo "Next:"
echo "  cmake --build lpg2/build -j"
echo "  ctest --test-dir lpg2/build -L smoke --output-on-failure"
echo "  docs: docs/ECOSYSTEM.md  |  compat: ecosystem/compat.json"
echo
echo "Summary: $ok tools found, $miss missing (optional tools are OK for core work)."
exit 0
