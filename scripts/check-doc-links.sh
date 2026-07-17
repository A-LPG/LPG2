#!/usr/bin/env bash
# Check relative markdown links in top-level docs (and a few entry READMEs).
# Usage: ./scripts/check-doc-links.sh
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

files=(
  README.md
  CONTRIBUTING.md
  docs/README.md
  docs/QUICKSTART.md
  docs/CONCEPTS.md
  docs/tutorial.md
  docs/USER.md
  docs/DEVELOPER.md
  docs/GRAMMAR_REFERENCE.md
  docs/ECOSYSTEM.md
  docs/TODO_TRIAGE.md
  docs/en/README.md
  docs/en/QUICKSTART.md
  docs/en/CONCEPTS.md
  docs/en/tutorial.md
  docs/en/USER.md
  docs/en/DEVELOPER.md
  docs/en/GRAMMAR_REFERENCE.md
  examples/calculator/README.md
  examples/calculator/java/README.md
  examples/calculator/typescript/README.md
  examples/calculator/cpp/README.md
  examples/calculator/rust/README.md
)

# Match markdown links [text](target) where target is relative (not http/https/mailto/#only)
# shellcheck disable=SC2016
pattern='\[[^]]*\]\(([^)]+)\)'

errors=0
checked=0

for f in "${files[@]}"; do
  [[ -f "$f" ]] || { echo "missing file: $f" >&2; errors=$((errors + 1)); continue; }
  dir="$(dirname "$f")"
  while IFS= read -r target; do
    # strip optional title: url "title"
    target="${target%% \"*}"
    target="${target%% \'*}"
    case "$target" in
      http://*|https://*|mailto:*|\#*) continue ;;
      '') continue ;;
    esac
    # drop fragment
    path_part="${target%%#*}"
    [[ -z "$path_part" ]] && continue
    resolved="$dir/$path_part"
    checked=$((checked + 1))
    if [[ ! -e "$resolved" ]]; then
      echo "BROKEN: $f -> $target (resolved $resolved)" >&2
      errors=$((errors + 1))
    fi
  done < <(grep -oE '\[[^]]*\]\([^)]+\)' "$f" | sed -E 's/^\[[^]]*\]\((.*)\)$/\1/' || true)
done

if [[ "$errors" -ne 0 ]]; then
  echo "check-doc-links: $errors broken link(s) ($checked checked)" >&2
  exit 1
fi
echo "check-doc-links: OK ($checked relative targets in ${#files[@]} files)"
