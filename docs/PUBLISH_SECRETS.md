# Publish secrets setup

CI publish workflows are wired but stay **build-only** until secrets exist.

| Secret | Repository | Used by |
|--------|------------|---------|
| `VSCE_PAT` | `A-LPG/LPG2`, `A-LPG/LPG-VScode` | Marketplace publish (`vscode-release.yml`) |
| `NPM_TOKEN` | `A-LPG/LPG-typescript-runtime` | `npm publish` |
| `NUGET_API_KEY` | `A-LPG/LPG-csharp-runtime` | `dotnet nuget push` |
| `CARGO_REGISTRY_TOKEN` | `A-LPG/LPG-rust-runtime` | `cargo publish` |
| `OSSRH_USERNAME` / `OSSRH_TOKEN` | `A-LPG/lpg-runtime` | Maven deploy |

## One-shot (after `gh auth login`)

```bash
gh auth login -h github.com -p https -s repo,workflow
./scripts/setup-publish-secrets.sh
# or non-interactive:
VSCE_PAT=... NPM_TOKEN=... NUGET_API_KEY=... CARGO_REGISTRY_TOKEN=... \
  OSSRH_USERNAME=... OSSRH_TOKEN=... \
  ./scripts/setup-publish-secrets.sh --from-env
```

Create a Marketplace PAT at https://marketplace.visualstudio.com/manage with **Publish** scope.
