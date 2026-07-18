#!/usr/bin/env node
'use strict';

const fs = require('fs');
const path = require('path');
const { spawnSync } = require('child_process');

const ROOT = path.join(__dirname, '..');
const marker = path.join(ROOT, 'vendor', 'binary-path.txt');

function resolveBinary() {
  if (process.env.LPG_BIN && fs.existsSync(process.env.LPG_BIN)) {
    return process.env.LPG_BIN;
  }
  if (fs.existsSync(marker)) {
    const p = fs.readFileSync(marker, 'utf8').trim();
    if (p && fs.existsSync(p)) return p;
  }
  // Fall back to PATH (useful when LPG2_NPM_SKIP_DOWNLOAD=1).
  return process.platform === 'win32' ? 'lpg-v2.3.0.exe' : 'lpg-v2.3.0';
}

function findRepoRoot() {
  if (process.env.LPG2_REPO_ROOT && fs.existsSync(process.env.LPG2_REPO_ROOT)) {
    return process.env.LPG2_REPO_ROOT;
  }
  // npm/lpg2 → repo root when installed from a checkout (or linked).
  const candidates = [
    path.resolve(ROOT, '..', '..'),
    path.resolve(ROOT, '..'),
    process.cwd(),
  ];
  for (const c of candidates) {
    if (
      fs.existsSync(path.join(c, 'lpg-generator-templates-2.1.00')) &&
      fs.existsSync(path.join(c, 'examples', 'calculator'))
    ) {
      return c;
    }
  }
  // Walk up from cwd.
  let dir = process.cwd();
  for (let i = 0; i < 8; i++) {
    if (
      fs.existsSync(path.join(dir, 'lpg-generator-templates-2.1.00')) &&
      fs.existsSync(path.join(dir, 'examples', 'calculator'))
    ) {
      return dir;
    }
    const parent = path.dirname(dir);
    if (parent === dir) break;
    dir = parent;
  }
  return null;
}

function printSubcommandHelp() {
  console.log(`lpg2 subcommands:
  init <dir> [--lang=<lang>]   scaffold a minimal grammar project
  from-antlr <file.g4> -o <dir>  convert Antlr4 grammar via antlr2lpg.py
  test [lang|all] [--grammar=<file.g>]  smoke-test (calculator or -nowrite)

Generator flags are passed through when the first argument is not a subcommand.
Examples:
  npx lpg2 -programming_language=java -table --dry-run grammar.g
  npx lpg2 init ./my-parser --lang=java
  npx lpg2 from-antlr Expr.g4 -o ./out-expr
  npx lpg2 test java
`);
}

function cmdInit(args) {
  const dirArg = args.find((a) => !a.startsWith('-')) || '.';
  let lang = 'java';
  for (const a of args) {
    const m = /^--?lang=(.+)$/.exec(a);
    if (m) lang = m[1];
  }
  const dest = path.resolve(process.cwd(), dirArg);
  fs.mkdirSync(dest, { recursive: true });

  const packageName = path.basename(dest).replace(/[^A-Za-z0-9_]/g, '_') || 'Parser';
  // Omit %Eof here: dtParserTemplateF.gi already declares EOF_TOKEN.
  const grammar = `%Options automatic_ast=nested,var=nt,visitor=default
%Options template=dtParserTemplateF.gi
%options package=${packageName}

%Terminals
    NUMBER
    PLUS
    STAR
    LPAREN
    RPAREN
%End

%Start
    Expr
%End

%Rules
    Expr$Expr ::= Expr PLUS Term
           | Term

    Term$Term ::= Term STAR Factor
           | Factor

    Factor$Factor ::= NUMBER
             | LPAREN Expr RPAREN
%End
`;
  const grammarPath = path.join(dest, 'grammar.g');
  if (!fs.existsSync(grammarPath)) {
    fs.writeFileSync(grammarPath, grammar, 'utf8');
  }

  const generateSh = `#!/usr/bin/env bash
set -euo pipefail
LANG_OPT="\${1:-${lang}}"
LPG_BIN="\${LPG_BIN:-lpg-v2.3.0}"
OUT="\${OUT_DIR:-./out-\$LANG_OPT}"
mkdir -p "\$OUT"
"\$LPG_BIN" -programming_language="\$LANG_OPT" -table -quiet \\
  -out_directory="\$OUT" \\
  grammar.g
echo "Generated into \$OUT"
`;
  const genPath = path.join(dest, 'generate.sh');
  if (!fs.existsSync(genPath)) {
    fs.writeFileSync(genPath, generateSh, 'utf8');
    try {
      fs.chmodSync(genPath, 0o755);
    } catch (_) {
      /* ignore on Windows */
    }
  }

  const readme = `# ${packageName}

Scaffolded by \`lpg2 init\`.

\`\`\`bash
# Analyze only
npx lpg2 -programming_language=${lang} -table --dry-run grammar.g

# Generate tables (template/include auto-discovered from install or LPG2 checkout)
./generate.sh ${lang}
\`\`\`

Link the matching LPG runtime for \`${lang}\`, then feed tokens and call \`parse()\`.
See https://github.com/A-LPG/LPG2/blob/main/docs/QUICKSTART.md
`;
  const readmePath = path.join(dest, 'README.md');
  if (!fs.existsSync(readmePath)) {
    fs.writeFileSync(readmePath, readme, 'utf8');
  }

  console.log(`lpg2 init: created scaffold in ${dest}`);
  console.log(`  language: ${lang}`);
  console.log(`  next: npx lpg2 -programming_language=${lang} -table --dry-run ${path.join(dirArg, 'grammar.g')}`);
  return 0;
}

function resolveAntlr2lpg(repoRoot) {
  const candidates = [
    process.env.LPG2_ANTLR2LPG,
    repoRoot && path.join(repoRoot, 'grammars-example', 'tools', 'antlr2lpg.py'),
    path.join(ROOT, '..', '..', 'grammars-example', 'tools', 'antlr2lpg.py'),
  ].filter(Boolean);
  for (const c of candidates) {
    if (fs.existsSync(c)) return c;
  }
  return null;
}

function cmdFromAntlr(args) {
  let g4 = null;
  let outDir = null;
  for (let i = 0; i < args.length; i++) {
    const a = args[i];
    if (a === '-o' || a === '--out' || a === '--dest') {
      outDir = args[++i];
      continue;
    }
    const m = /^(?:-o=|--out=|--dest=)(.+)$/.exec(a);
    if (m) {
      outDir = m[1];
      continue;
    }
    if (!a.startsWith('-') && !g4) {
      g4 = a;
    }
  }
  if (!g4 || !outDir) {
    console.error('Usage: lpg2 from-antlr <file.g4> -o <dir>');
    return 2;
  }
  const repoRoot = findRepoRoot();
  const script = resolveAntlr2lpg(repoRoot);
  if (!script) {
    console.error(
      'lpg2 from-antlr: antlr2lpg.py not found.\n' +
        'Set LPG2_REPO_ROOT to an LPG2 checkout, or LPG2_ANTLR2LPG to the script path.'
    );
    return 1;
  }
  const absG4 = path.resolve(process.cwd(), g4);
  const absOut = path.resolve(process.cwd(), outDir);
  if (!fs.existsSync(absG4)) {
    console.error(`lpg2 from-antlr: missing input ${absG4}`);
    return 1;
  }
  const result = spawnSync(
    process.env.PYTHON || 'python3',
    [script, '--g4', absG4, '--dest', absOut],
    { stdio: 'inherit' }
  );
  if (result.error) {
    console.error(`lpg2 from-antlr: ${result.error.message}`);
    return 1;
  }
  if (result.status === 0) {
    console.log(
      `Next: npx lpg2 -programming_language=java -table --dry-run ${path.join(outDir, '*Parser.g')}`
    );
  }
  return result.status == null ? 1 : result.status;
}

function cmdTest(args) {
  let lang = 'java';
  let grammar = null;
  for (const a of args) {
    if (a === 'all' || (!a.startsWith('-') && a !== 'test')) {
      if (!a.startsWith('-')) lang = a;
    }
    const m = /^--grammar=(.+)$/.exec(a);
    if (m) grammar = m[1];
  }
  // Prefer local grammar check when --grammar or ./grammar.g exists.
  const localGrammar =
    grammar ||
    (fs.existsSync(path.join(process.cwd(), 'grammar.g'))
      ? path.join(process.cwd(), 'grammar.g')
      : null);
  if (localGrammar) {
    const binary = resolveBinary();
    const result = spawnSync(
      binary,
      [
        `-programming_language=${lang === 'all' ? 'java' : lang}`,
        '-table',
        '--dry-run',
        '-fail_on_conflicts',
        '-quiet',
        localGrammar,
      ],
      { stdio: 'inherit' }
    );
    if (result.error) {
      console.error(`lpg2 test: failed to launch "${binary}": ${result.error.message}`);
      return 1;
    }
    return result.status == null ? 1 : result.status;
  }

  const repoRoot = findRepoRoot();
  if (!repoRoot) {
    console.error(
      'lpg2 test: no grammar.g in cwd and LPG2 checkout not found.\n' +
        'Pass --grammar=<file.g> or set LPG2_REPO_ROOT.'
    );
    return 1;
  }
  const runSh = path.join(repoRoot, 'examples', 'calculator', 'scripts', 'run.sh');
  if (!fs.existsSync(runSh)) {
    console.error(`lpg2 test: missing ${runSh}`);
    return 1;
  }
  const result = spawnSync('bash', [runSh, lang === 'all' ? 'all' : lang], {
    stdio: 'inherit',
    cwd: path.join(repoRoot, 'examples', 'calculator'),
    env: process.env,
  });
  if (result.error) {
    console.error(`lpg2 test: ${result.error.message}`);
    return 1;
  }
  return result.status == null ? 1 : result.status;
}

const argv = process.argv.slice(2);
const sub = argv[0];

if (sub === 'help' || sub === '--help' || sub === '-h') {
  printSubcommandHelp();
  const binary = resolveBinary();
  spawnSync(binary, ['-help'], { stdio: 'inherit' });
  process.exit(0);
}

if (sub === 'init') {
  process.exit(cmdInit(argv.slice(1)));
}
if (sub === 'from-antlr') {
  process.exit(cmdFromAntlr(argv.slice(1)));
}
if (sub === 'test' || sub === '--test') {
  process.exit(cmdTest(sub === '--test' ? argv.slice(1) : argv.slice(1)));
}

const binary = resolveBinary();
const result = spawnSync(binary, argv, { stdio: 'inherit' });
if (result.error) {
  console.error(
    `lpg2: failed to launch "${binary}": ${result.error.message}\n` +
      'Re-run `npm install` in the lpg2 package, or set LPG_BIN to a local binary.'
  );
  process.exit(1);
}
process.exit(result.status == null ? 1 : result.status);
