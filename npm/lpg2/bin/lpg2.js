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

const binary = resolveBinary();
const args = process.argv.slice(2);
const result = spawnSync(binary, args, { stdio: 'inherit' });
if (result.error) {
  console.error(
    `lpg2: failed to launch "${binary}": ${result.error.message}\n` +
      'Re-run `npm install` in the lpg2 package, or set LPG_BIN to a local binary.'
  );
  process.exit(1);
}
process.exit(result.status == null ? 1 : result.status);
