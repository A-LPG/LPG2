#!/usr/bin/env bash
# Mirror the Ubuntu GitHub Actions CI locally via Docker before push.
#
# Usage:
#   ./scripts/ci-docker.sh              # main generator + ctest (ubuntu job)
#   ./scripts/ci-docker.sh runtimes     # also Python/Go/Dart/Rust runtime smokes
#   ./scripts/ci-docker.sh all          # generator + runtimes
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
MODE="${1:-main}"
IMAGE="${LPG2_CI_DOCKER_IMAGE:-ubuntu:24.04}"
WORKDIR=/src
BUILD=/tmp/lpg2-ci-docker-build

die() { echo "ci-docker: $*" >&2; exit 1; }
need() { command -v "$1" >/dev/null || die "missing $1"; }

need docker

run_main() {
  echo "======== main CI (cmake + ctest, WARNINGS_AS_ERRORS=ON) ========"
  docker run --rm \
    -v "$ROOT:$WORKDIR:ro" \
    -v "${LPG2_CI_DOCKER_BUILD_CACHE:-$ROOT/lpg2/build-ci-docker}:$BUILD" \
    -w "$WORKDIR" \
    -e DEBIAN_FRONTEND=noninteractive \
    "$IMAGE" bash -lc "
      set -euo pipefail
      apt-get update -qq
      apt-get install -y -qq cmake ninja-build g++ git python3 python3-pip \
        openjdk-17-jdk-headless golang-go curl ca-certificates pkg-config \
        libicu-dev >/dev/null
      # Rust via rustup (stable)
      if ! command -v rustc >/dev/null; then
        curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y -q
        . \"\$HOME/.cargo/env\"
      fi
      cmake -S lpg2 -B '$BUILD' -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DLPG2_REQUIRE_RUST_TESTS=ON \
        -DLPG2_WARNINGS_AS_ERRORS=ON
      cmake --build '$BUILD' --config Release --parallel
      ctest --test-dir '$BUILD' -C Release --output-on-failure --timeout 300
      cmake --build '$BUILD' --config Release --target package
      echo 'ci-docker main: OK'
    "
}

run_runtimes() {
  echo "======== runtime CI smokes ========"
  # Source tree is mounted read-only; copy runtimes into /tmp for writable
  # pip/cargo caches and egg-info (matches GitHub Actions checkout writability).
  docker run --rm \
    -v "$ROOT:$WORKDIR:ro" \
    -w "$WORKDIR" \
    -e DEBIAN_FRONTEND=noninteractive \
    "$IMAGE" bash -lc "
      set -euo pipefail
      apt-get update -qq
      apt-get install -y -qq python3 python3-pip python3-venv golang-go \
        curl ca-certificates >/dev/null
      if ! command -v rustc >/dev/null; then
        curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y -q
      fi
      . \"\$HOME/.cargo/env\"

      cp -a runtime/LPG-python-runtime /tmp/LPG-python-runtime
      cp -a runtime/LPG-go-runtime /tmp/LPG-go-runtime
      cp -a runtime/LPG-rust-runtime /tmp/LPG-rust-runtime

      echo '--- python ---'
      python3 -m venv /tmp/lpg-py
      /tmp/lpg-py/bin/pip -q install -U pip setuptools wheel
      /tmp/lpg-py/bin/pip -q install -e /tmp/LPG-python-runtime/Python3
      /tmp/lpg-py/bin/python -c \"import lpg2; from lpg2 import GLRParser; print('python ok', GLRParser)\"

      echo '--- go ---'
      (cd /tmp/LPG-go-runtime && go test ./...)

      echo '--- rust ---'
      (cd /tmp/LPG-rust-runtime/lpg2 && cargo test --quiet)

      echo 'ci-docker runtimes: OK'
    "
}

case "$MODE" in
  main) run_main ;;
  runtimes) run_runtimes ;;
  all) run_main; run_runtimes ;;
  *) die "unknown mode '$MODE' (main|runtimes|all)" ;;
esac
