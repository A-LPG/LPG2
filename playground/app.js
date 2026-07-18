const grammarEl = document.getElementById('grammar');
const diagnosticsEl = document.getElementById('diagnostics');
const filesEl = document.getElementById('files');
const outputEl = document.getElementById('output');
const statusEl = document.getElementById('status');
const runBtn = document.getElementById('run');
const langEl = document.getElementById('lang');

let ModuleFactory = null;
let wasmReady = false;
/** @type {Map<string, string>} */
let generated = new Map();

const glrEl = document.getElementById('glr');

async function loadSample(name = 'sample.g') {
  const res = await fetch(name);
  grammarEl.value = await res.text();
}

function glrTemplateFor(lang) {
  // Paths as seen inside the WASM preloaded template tree.
  if (lang === 'typescript')
    return '/share/lpg2/lpg-generator-templates-2.1.00/templates/typescript/glrParserTemplateF.gi';
  if (lang === 'java')
    return '/share/lpg2/lpg-generator-templates-2.1.00/templates/java/glrParserTemplateF.gi';
  if (lang === 'cpp' || lang === 'c++' || lang === 'rt_cpp')
    return '/share/lpg2/lpg-generator-templates-2.1.00/templates/rt_cpp/glrParserTemplateF.gi';
  return null;
}

function setStatus(text, cls) {
  statusEl.textContent = text;
  statusEl.className = 'status' + (cls ? ' ' + cls : '');
}

async function tryLoadWasm() {
  try {
    // Emscripten MODULARIZE + EXPORT_NAME=createLpg2Module emits a classic
    // script; load via <script> tag when ES module export is unavailable.
    if (!window.createLpg2Module) {
      await new Promise((resolve, reject) => {
        const s = document.createElement('script');
        s.src = 'wasm/lpg2.js';
        s.onload = resolve;
        s.onerror = () => reject(new Error('failed to load wasm/lpg2.js'));
        document.head.appendChild(s);
      });
    }
    ModuleFactory = window.createLpg2Module;
    wasmReady = typeof ModuleFactory === 'function';
    if (wasmReady) {
      setStatus('WASM ready', 'ok');
      runBtn.disabled = false;
    } else {
      setStatus('WASM module shape unexpected', 'err');
    }
  } catch (err) {
    setStatus('WASM not bundled — documentation mode', 'err');
    diagnosticsEl.textContent =
      'This GitHub Pages build does not include playground/wasm/lpg2.js yet.\n' +
      'CI workflow wasm-playground.yml produces it via Emscripten.\n\n' +
      'Locally:\n' +
      '  npx lpg2 -programming_language=typescript -table your.g\n' +
      '  or: ./scripts/build-wasm.sh && serve playground/\n\n' +
      String(err && err.message ? err.message : err);
    runBtn.disabled = true;
  }
}

function collectGenerated(FS, dir, prefix = '') {
  generated.clear();
  let names;
  try {
    names = FS.readdir(dir);
  } catch {
    return;
  }
  for (const name of names) {
    if (name === '.' || name === '..') continue;
    const full = dir + '/' + name;
    const rel = prefix ? prefix + '/' + name : name;
    let stat;
    try {
      stat = FS.stat(full);
    } catch {
      continue;
    }
    if (FS.isDir(stat.mode)) collectGenerated(FS, full, rel);
    else if (FS.isFile(stat.mode)) {
      try {
        generated.set(rel, FS.readFile(full, { encoding: 'utf8' }));
      } catch {
        /* binary skip */
      }
    }
  }
}

function refreshFileList() {
  filesEl.innerHTML = '';
  for (const name of generated.keys()) {
    const opt = document.createElement('option');
    opt.value = name;
    opt.textContent = name;
    filesEl.appendChild(opt);
  }
  const first = generated.keys().next().value;
  if (first) {
    filesEl.value = first;
    outputEl.textContent = generated.get(first) || '';
  } else {
    outputEl.textContent = '(no generated text files)';
  }
}

/** Resolve Emscripten side-car files next to wasm/lpg2.js (not the page root). */
function locateWasmAsset(path) {
  const name = String(path).split('/').pop();
  return new URL(`wasm/${name}`, window.location.href).href;
}

async function runGenerate() {
  if (!wasmReady || !ModuleFactory) return;
  setStatus('Running…');
  diagnosticsEl.textContent = '';
  generated.clear();
  refreshFileList();

  const lang = langEl.value;
  const wantGlr = glrEl && glrEl.checked;
  const source = grammarEl.value;
  const stdout = [];
  const stderr = [];

  if (wantGlr && !glrTemplateFor(lang)) {
    diagnosticsEl.textContent =
      `GLR generate is wired for typescript / java / cpp in the playground.\n` +
      `Language "${lang}" still has nextAst scaffolding only (no glrParserTemplateF.gi).\n` +
      `Uncheck GLR or switch language.`;
    setStatus('GLR N/A for ' + lang, 'err');
    return;
  }

  const module = await ModuleFactory({
    locateFile: locateWasmAsset,
    print: (t) => stdout.push(t),
    printErr: (t) => stderr.push(t),
    noInitialRun: true,
  });

  const FS = module.FS;
  FS.mkdir('/work');
  FS.writeFile('/work/grammar.g', source);
  FS.mkdir('/work/out');

  // Prefer packed templates when the wasm build preloaded them.
  if (module.ENV) {
    module.ENV.LPG2_RESOURCE_ROOT = '/share/lpg2/lpg-generator-templates-2.1.00';
  }

  const args = [
    '-programming_language=' + lang,
    '-table',
    '-quiet',
    '-out_directory=/work/out',
  ];
  if (wantGlr) {
    args.push('-glr');
    const tmpl = glrTemplateFor(lang);
    if (tmpl) args.push('-template=' + tmpl);
  }
  args.push('/work/grammar.g');

  let rc = 0;
  try {
    rc = module.callMain(args);
  } catch (e) {
    // Emscripten may throw on exit(n).
    if (typeof e === 'number') rc = e;
    else if (e && typeof e.status === 'number') rc = e.status;
    else {
      diagnosticsEl.textContent = String(e);
      setStatus('Failed', 'err');
      return;
    }
  }

  collectGenerated(FS, '/work/out');
  refreshFileList();

  const diag = [...stdout, ...stderr].join('\n').trim();
  diagnosticsEl.textContent =
    diag ||
    (rc === 0
      ? `(exit ${rc}) — generation succeeded`
      : `(exit ${rc}) — see generator diagnostics above`);
  setStatus(rc === 0 ? 'OK' : 'exit ' + rc, rc === 0 ? 'ok' : 'err');
}

filesEl.addEventListener('change', () => {
  outputEl.textContent = generated.get(filesEl.value) || '';
});
document.getElementById('sample').addEventListener('click', () => loadSample('sample.g'));
document.getElementById('sample-glr').addEventListener('click', () => {
  if (glrEl) glrEl.checked = true;
  if (langEl) langEl.value = 'typescript';
  return loadSample('sample-glr.g');
});
runBtn.addEventListener('click', () => {
  runGenerate().catch((e) => {
    diagnosticsEl.textContent = String(e);
    setStatus('Failed', 'err');
  });
});

runBtn.disabled = true;
loadSample('sample.g').then(tryLoadWasm);
