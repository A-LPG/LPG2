# LPG2 Playground

Static web UI that runs the LPG2 generator via **WebAssembly** (Emscripten).

## Layout

| Path | Role |
| --- | --- |
| `index.html` / `app.js` / `styles.css` | UI |
| `sample.g` | Default deterministic grammar |
| `sample-glr.g` | Ambiguous `E ::= E + E \| NUMBER` sample (check **GLR** + TypeScript) |
| `glr-demo/` + `glr-demo.bundle.js` | Browser TypeScript `GLRParser` forest demo (GSS/SPPF) |
| `wasm/lpg2.js` + `wasm/lpg2.wasm` (+ `lpg2.data`) | Produced by CI / `scripts/build-wasm.sh` (not committed). `app.js` uses `locateFile` so side-cars load from `wasm/`, not the site root. |

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

## GLR demo (TypeScript in-browser)

WASM runs the **generator** only. Browser parse drivers are TypeScript (`lpg2ts`):

1. Check **GLR (`-glr`)** and pick `typescript` (or `java` / `cpp` to *generate* only).
2. **Load GLR sample** fills `sample-glr.g` and enables `-glr` + `glrParserTemplateF.gi`.
3. **Run GLR demo (n=1…8)** parses via the prebuilt `glr-demo.bundle.js` and reports
   root `nextAst` alt counts plus SPPF symbol-node counts (sharing signal).

Regenerate the demo sources/bundle after runtime or template changes:

```bash
./scripts/build-playground-glr-demo.sh
```

Java can be generated with GLR checked but cannot execute in the browser (no JVM).

## Incremental parsing demo

The bottom panel demonstrates **token-level damage reset** (same honest positioning
as `lpg2ts` / C++ runtime). It is **not** tree-sitter subtree reuse. Full API lives in
[`runtime/LPG-typescript-runtime/README.md`](../runtime/LPG-typescript-runtime/README.md).
