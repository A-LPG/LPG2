#!/usr/bin/env bash
# Configure GitHub Actions secrets for LPG2 ecosystem publish workflows.
# Requires: gh auth login (with repo + workflow scopes).
#
# Usage:
#   ./scripts/setup-publish-secrets.sh              # interactive prompts
#   NPM_TOKEN=... VSCE_PAT=... ./scripts/setup-publish-secrets.sh --from-env
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
FROM_ENV=0
[[ "${1:-}" == "--from-env" ]] && FROM_ENV=1

if ! command -v gh >/dev/null; then
  echo "Install GitHub CLI: https://cli.github.com/" >&2
  exit 1
fi
if ! gh auth status >/dev/null 2>&1; then
  echo "Not logged in. Run: gh auth login -h github.com -p https -s repo,workflow" >&2
  exit 1
fi

prompt_set() {
  local repo="$1" name="$2" hint="$3"
  local val=""
  if [[ "$FROM_ENV" -eq 1 ]]; then
    val="${!name:-}"
  else
    echo -n "Set $name on $repo ($hint) [empty=skip]: "
    read -r val
  fi
  if [[ -z "$val" ]]; then
    echo "  skip $repo / $name"
    return 0
  fi
  printf '%s' "$val" | gh secret set "$name" --repo "$repo"
  echo "  set $repo / $name"
}

echo "=== LPG2 publish secrets ==="
prompt_set "A-LPG/LPG2" "VSCE_PAT" "VS Code Marketplace PAT"
prompt_set "A-LPG/LPG-VScode" "VSCE_PAT" "VS Code Marketplace PAT (extension repo)"
prompt_set "A-LPG/LPG-typescript-runtime" "NPM_TOKEN" "npm access token"
prompt_set "A-LPG/LPG-csharp-runtime" "NUGET_API_KEY" "nuget.org API key"
prompt_set "A-LPG/LPG-rust-runtime" "CARGO_REGISTRY_TOKEN" "crates.io token"
prompt_set "A-LPG/lpg-runtime" "OSSRH_USERNAME" "Maven Central / OSSRH user"
prompt_set "A-LPG/lpg-runtime" "OSSRH_TOKEN" "Maven Central / OSSRH token"

echo
echo "Done. Workflows that need secrets will publish when tokens are present."
echo "Docs: docs/PUBLISH_SECRETS.md"
