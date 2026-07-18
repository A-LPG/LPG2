#!/usr/bin/env bash
# Mirror Tool CI language-server + VS Code compile locally (incl. Linux via Docker).
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT/tool/LPG-language-server"

python3 - <<'PY'
from pathlib import Path
import re, subprocess, sys
root = Path('.').resolve()
pat = re.compile(r'#include\s+"([^"]+)"')
bad = []
all_lsp = subprocess.check_output(
    ['git', '-C', str(root / 'third_party/LspCpp'), 'ls-files', 'include/LibLsp'],
    text=True).splitlines()
all_lpg = subprocess.check_output(
    ['git', '-C', str(root / 'third_party/LPG-cpp-runtime'), 'ls-files', 'include/lpg2'],
    text=True).splitlines()
for p in (root / 'LPG-language-server').rglob('*'):
    if p.suffix not in {'.h', '.hpp', '.cpp', '.c'}:
        continue
    for m in pat.finditer(p.read_text(errors='ignore')):
        inc = m.group(1)
        if inc.startswith('LibLsp/'):
            rel = 'include/' + inc
            if rel not in all_lsp:
                ci = [f for f in all_lsp if f.lower() == rel.lower()]
                if ci:
                    bad.append((str(p.relative_to(root)), inc, ci[0]))
        elif inc.startswith('lpg2/'):
            rel = 'include/' + inc
            if rel not in all_lpg:
                ci = [f for f in all_lpg if f.lower() == rel.lower()]
                if ci:
                    bad.append((str(p.relative_to(root)), inc, ci[0]))
if bad:
    print('CASE MISMATCHES (fail on Linux):')
    for b in bad:
        print(' ', b)
    sys.exit(1)
print('case-audit: OK')
PY

cd "$ROOT/tool/LPG-VScode"
yarn install --frozen-lockfile || yarn install
yarn compile

if command -v docker >/dev/null && docker info >/dev/null 2>&1; then
  docker run --rm -v "$ROOT:/src:ro" -v /tmp/lpg2-ci-tool-linux:/build ubuntu:24.04 bash -lc '
    set -euo pipefail
    apt-get update -qq
    DEBIAN_FRONTEND=noninteractive apt-get install -y -qq \
      cmake ninja-build g++ pkg-config libicu-dev zlib1g-dev \
      libboost-filesystem-dev libboost-program-options-dev \
      libboost-system-dev libboost-thread-dev >/dev/null
    cmake -S /src/tool/LPG-language-server -B /build -G Ninja -DCMAKE_BUILD_TYPE=Release
    cmake --build /build --parallel
    test -x /build/lpg-language-server
    echo LINUX_BUILD_OK
  '
else
  echo "WARN: docker unavailable; Linux case-sensitive build skipped" >&2
fi
echo "ci-tool-local: ALL GREEN"
