/**
 * Minimal playground demo: mirrors lpg2ts incrementalResetAtCharacterOffset behavior.
 * Educational only — not tree-sitter subtree reuse.
 */

const sourceEl = document.getElementById('incr-source');
const offsetEl = document.getElementById('incr-offset');
const beforeEl = document.getElementById('incr-before');
const afterEl = document.getElementById('incr-after');
const affectedEl = document.getElementById('incr-affected');
const noteEl = document.getElementById('incr-note');

const POSITIONING =
  'Token-level re-lex + statement-level re-parse — NOT tree-sitter subtree reuse.';

/** Build one-char tokens like the contract test stub. */
function seedTokens(text) {
  const tokens = [{ start: 0, end: 0, kind: 'bad' }];
  for (let i = 0; i < text.length; i++) {
    tokens.push({ start: i, end: i, kind: 'char', text: text[i] });
  }
  tokens.push({ start: text.length, end: text.length, kind: 'EOF' });
  return tokens;
}

/** Simplified damage reset: truncate suffix from token at/after damage offset. */
function incrementalResetAtCharacterOffset(tokens, damageOffset) {
  let tokenIndex = 0;
  for (let i = 0; i < tokens.length; i++) {
    const t = tokens[i];
    if (damageOffset >= t.start && damageOffset <= t.end) {
      tokenIndex = i;
      break;
    }
    if (damageOffset > t.end) {
      tokenIndex = i;
    }
  }

  const affected = tokens.slice(tokenIndex);
  const kept = tokens.slice(0, tokenIndex);
  return { kept, affected, repairOffset: tokens[tokenIndex]?.start ?? damageOffset };
}

function formatTokens(tokens) {
  return tokens
    .map((t, i) => {
      const label = t.text != null ? `"${t.text}"` : t.kind;
      return `[${i}] ${label} @${t.start}`;
    })
    .join('\n');
}

function refresh() {
  const text = sourceEl.value || '';
  const damage = Number(offsetEl.value);
  const tokens = seedTokens(text);
  const { kept, affected, repairOffset } = incrementalResetAtCharacterOffset(tokens, damage);

  beforeEl.textContent = formatTokens(tokens);
  afterEl.textContent = formatTokens(kept);
  affectedEl.textContent =
    affected.map((t) => `${t.text != null ? `"${t.text}"` : t.kind} @${t.start}`).join(', ') ||
    '(none)';
  noteEl.textContent = `${POSITIONING}\nrepairOffset=${repairOffset}`;
}

export function initIncrementalDemo() {
  if (!sourceEl) return;
  sourceEl.value = '0123456789';
  offsetEl.value = '5';
  offsetEl.min = '0';
  offsetEl.max = String(sourceEl.value.length);
  sourceEl.addEventListener('input', () => {
    offsetEl.max = String(sourceEl.value.length);
    refresh();
  });
  offsetEl.addEventListener('input', refresh);
  refresh();
}
