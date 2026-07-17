# LPG2 Playground

Static web UI that runs the LPG2 generator via **WebAssembly** (Emscripten).

## Layout

| Path | Role |
| --- | --- |
| `index.html` / `app.js` / `styles.css` | UI |
| `sample.g` | Default grammar |
| `wasm/lpg2.js` + `wasm/lpg2.wasm` | Produced by CI / `scripts/build-wasm.sh` (not committed) |

## Local build

Requires [Emscripten](https://emscripten.org/) (`emcc` on `PATH`):

```bash
./scripts/build-wasm.sh
# then serve the playground directory, e.g.
npx --yes serve playground
```

Without WASM artifacts the page still loads and explains how to use `npx lpg2`.

## GitHub Pages

Workflow [`.github/workflows/wasm-playground.yml`](../.github/workflows/wasm-playground.yml)
builds WASM, copies it into `playground/wasm/`, and can publish the folder as
Pages artifacts (configure the repo Pages source to that workflow).
