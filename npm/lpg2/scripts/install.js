#!/usr/bin/env node
'use strict';

/**
 * Download the LPG2 release package for this platform into vendor/ and
 * extract the generator binary (+ templates when present in the archive).
 *
 * Override with:
 *   LPG2_NPM_VERSION=v2.3.0   (default: package.json version with v prefix)
 *   LPG2_NPM_SKIP_DOWNLOAD=1  (skip; useful for offline CI that supplies PATH)
 */

const fs = require('fs');
const path = require('path');
const https = require('https');
const { execFileSync } = require('child_process');

const ROOT = path.join(__dirname, '..');
const pkg = JSON.parse(fs.readFileSync(path.join(ROOT, 'package.json'), 'utf8'));
const VERSION = process.env.LPG2_NPM_VERSION || `v${pkg.version}`;
const VENDOR = path.join(ROOT, 'vendor');

function platformAsset() {
  const p = process.platform;
  const a = process.arch;
  if (p === 'linux' && (a === 'x64' || a === 'x86_64')) {
    return { name: 'lpg2-linux-x86_64.tar.gz', kind: 'tar.gz' };
  }
  if (p === 'darwin') {
    // Release currently ships a universal/arch-tagged "macos" tarball.
    return { name: 'lpg2-macos.tar.gz', kind: 'tar.gz' };
  }
  if (p === 'win32' && (a === 'x64' || a === 'x86_64')) {
    return { name: 'lpg2-windows-x86_64.zip', kind: 'zip' };
  }
  throw new Error(
    `Unsupported platform ${p}/${a}. Install from ` +
      `https://github.com/A-LPG/LPG2/releases or build from source.`
  );
}

function download(url, dest) {
  return new Promise((resolve, reject) => {
    const file = fs.createWriteStream(dest);
    const get = (u, redirects) => {
      https
        .get(u, { headers: { 'User-Agent': 'lpg2-npm-install' } }, (res) => {
          if (
            res.statusCode >= 300 &&
            res.statusCode < 400 &&
            res.headers.location &&
            redirects < 5
          ) {
            res.resume();
            get(res.headers.location, redirects + 1);
            return;
          }
          if (res.statusCode !== 200) {
            reject(new Error(`GET ${u} → HTTP ${res.statusCode}`));
            res.resume();
            return;
          }
          res.pipe(file);
          file.on('finish', () => file.close(() => resolve()));
        })
        .on('error', reject);
    };
    get(url, 0);
  });
}

function findBinary(dir) {
  const want = process.platform === 'win32' ? 'lpg-v2.3.0.exe' : 'lpg-v2.3.0';
  const stack = [dir];
  while (stack.length) {
    const cur = stack.pop();
    let entries;
    try {
      entries = fs.readdirSync(cur, { withFileTypes: true });
    } catch {
      continue;
    }
    for (const ent of entries) {
      const full = path.join(cur, ent.name);
      if (ent.isDirectory()) stack.push(full);
      else if (ent.name === want || ent.name === 'lpg-v2.3.0') return full;
    }
  }
  return null;
}

async function main() {
  if (process.env.LPG2_NPM_SKIP_DOWNLOAD === '1') {
    console.log('lpg2: LPG2_NPM_SKIP_DOWNLOAD=1 — skipping binary download');
    return;
  }

  const asset = platformAsset();
  fs.mkdirSync(VENDOR, { recursive: true });
  const archive = path.join(VENDOR, asset.name);
  const url =
    `https://github.com/A-LPG/LPG2/releases/download/${VERSION}/${asset.name}`;

  console.log(`lpg2: downloading ${url}`);
  try {
    await download(url, archive);
  } catch (err) {
    console.error(`lpg2: download failed: ${err.message}`);
    console.error(
      'Install a release binary manually or set LPG2_NPM_SKIP_DOWNLOAD=1 ' +
        'and put lpg-v2.3.0 on PATH.'
    );
    process.exit(1);
  }

  const extractDir = path.join(VENDOR, 'prefix');
  fs.rmSync(extractDir, { recursive: true, force: true });
  fs.mkdirSync(extractDir, { recursive: true });

  if (asset.kind === 'tar.gz') {
    execFileSync('tar', ['-xzf', archive, '-C', extractDir], {
      stdio: 'inherit',
    });
  } else {
    // Prefer PowerShell Expand-Archive on Windows; fall back to unzip.
    try {
      execFileSync(
        'powershell.exe',
        [
          '-NoProfile',
          '-Command',
          `Expand-Archive -Path '${archive.replace(/'/g, "''")}' -DestinationPath '${extractDir.replace(/'/g, "''")}' -Force`,
        ],
        { stdio: 'inherit' }
      );
    } catch {
      execFileSync('unzip', ['-o', archive, '-d', extractDir], {
        stdio: 'inherit',
      });
    }
  }

  const binary = findBinary(extractDir);
  if (!binary) {
    console.error('lpg2: could not locate lpg-v2.3.0 inside the release archive');
    process.exit(1);
  }

  const marker = path.join(VENDOR, 'binary-path.txt');
  fs.writeFileSync(marker, binary + '\n', 'utf8');
  try {
    fs.chmodSync(binary, 0o755);
  } catch {
    /* windows */
  }
  console.log(`lpg2: installed ${binary}`);
}

main().catch((err) => {
  console.error(err);
  process.exit(1);
});
