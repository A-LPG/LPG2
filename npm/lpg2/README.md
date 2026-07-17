# `lpg2` (npm)

Install the [LPG2](https://github.com/A-LPG/LPG2) generator via npm. The
`postinstall` script downloads the matching GitHub Release archive and unwraps
`lpg-v2.3.0` (plus templates when the archive includes `share/lpg2/`).

```bash
npx lpg2 --help
npx lpg2 -programming_language=java -table grammar.g
```

Or install globally:

```bash
npm i -g lpg2
lpg2 --version
```

## Environment

| Variable | Meaning |
| --- | --- |
| `LPG2_NPM_VERSION` | Release tag to download (default `v` + package version) |
| `LPG2_NPM_SKIP_DOWNLOAD` | Set to `1` to skip download (use `LPG_BIN` / PATH) |
| `LPG_BIN` | Absolute path to a local `lpg-v2.3.0` binary |
| `LPG2_RESOURCE_ROOT` | Override template root (`…/lpg-generator-templates-2.1.00`) |

## Supported platforms

- Linux x86_64 (`lpg2-linux-x86_64.tar.gz`)
- macOS (`lpg2-macos.tar.gz`)
- Windows x86_64 (`lpg2-windows-x86_64.zip`)

Other platforms: build from source under `lpg2/` or download a Release manually.
