/**
 * Playground UI for the bundled TypeScript GLR forest demo.
 * Bundle: regenerate with `./scripts/build-playground-glr-demo.sh`.
 */

const catalan = [1, 1, 2, 5, 14, 42, 132, 429];

export function initGlrDemo() {
  const btn = document.getElementById('glr-run');
  const out = document.getElementById('glr-out');
  if (!btn || !out) return;

  btn.addEventListener('click', async () => {
    out.textContent = 'Running…';
    try {
      const mod = await import('./glr-demo.bundle.js');
      const rows = mod.runGlrDemoSuite();
      const lines = [
        'operands | root nextAst alts | Catalan trees | SPPF symbol nodes',
        '---------|-------------------|---------------|------------------',
      ];
      for (const r of rows) {
        const c = catalan[r.operands - 1];
        if (!r.ok) {
          lines.push(`n=${r.operands} FAILED: ${r.error || 'unknown'}`);
          continue;
        }
        lines.push(
          `n=${r.operands}     | ${String(r.rootAlts).padStart(3)}               | ${String(c).padStart(5)}         | ${r.sppfSymbols}`
        );
      }
      lines.push('');
      lines.push('Note: root nextAst chain length ≠ Catalan tree count for n>3;');
      lines.push('nested packing multiplies. SPPF symbol nodes stay far below 429 at n=8.');
      out.textContent = lines.join('\n');
    } catch (e) {
      out.textContent =
        'Failed to load glr-demo.bundle.js.\n' +
        'Regenerate: ./scripts/build-playground-glr-demo.sh\n\n' +
        String(e && e.message ? e.message : e);
    }
  });
}
