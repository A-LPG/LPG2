#!/usr/bin/env bash
# Read-only release checklist validator for LPG2.
# Does not publish; exits non-zero if required files/versions look inconsistent.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
fail=0

check() {
  local msg="$1"
  shift
  if "$@"; then
    echo "  [ok] $msg"
  else
    echo "  [FAIL] $msg"
    fail=1
  fi
}

echo "=== LPG2 release checklist (read-only) ==="

VERSION="$(grep -E 'set\(LPG2_VERSION' lpg2/CMakeLists.txt | head -1 | sed -E 's/.*"([^"]+)".*/\1/')"
echo "Generator version: $VERSION"

check "ecosystem/compat.json exists" test -f ecosystem/compat.json
check "docs/ECOSYSTEM.md exists" test -f docs/ECOSYSTEM.md
check "CHANGELOG.md mentions version or Unreleased" \
  grep -qE "^## (${VERSION}|Unreleased)" CHANGELOG.md
check "compat.json generator.version matches CMake" \
  python3 -c "import json,sys; c=json.load(open('ecosystem/compat.json')); sys.exit(0 if c['generator']['version']==sys.argv[1] else 1)" "$VERSION"
check "README mentions current version" grep -q "$VERSION" README.md
check "USER.md mentions current version" grep -q "$VERSION" docs/USER.md
check "python2 marked deprecated in compat.json" \
  python3 -c "import json,sys; c=json.load(open('ecosystem/compat.json')); sys.exit(0 if c['backends']['python2']['status']=='deprecated' else 1)"
check "dev-bootstrap.sh executable bit" test -x scripts/dev-bootstrap.sh

if [[ -f ecosystem/compat.json ]]; then
  echo
  echo "Pinned submodules (from compat.json):"
  python3 - <<'PY'
import json
c = json.load(open("ecosystem/compat.json"))
print(f"  extension {c['extension']['version']} @ {c['extension']['pinned'][:12]}")
print(f"  language_server @ {c['language_server']['pinned'][:12]}")
for r in c["runtimes"]:
    print(f"  {r['id']} @ {r['pinned'][:12]}  package={r['package'].get('publish','?')}")
PY
fi

echo
if [[ "$fail" -ne 0 ]]; then
  echo "Checklist FAILED — fix items above before tagging."
  exit 1
fi
echo "Checklist passed (still complete manual steps in docs/ECOSYSTEM.md)."
exit 0
