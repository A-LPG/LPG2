# C++ wiring notes

1. Generate tables: `../scripts/generate.sh cpp`
2. Add `out-cpp/calculatorprs.cpp` (and headers) to a small CMake target.
3. Link against `runtime/LPG-cpp-runtime` (`cpplpg2`).
4. Provide a lexer that yields `NUMBER` / `PLUS` / `STAR` / `LPAREN` / `RPAREN` / `EOF_TOKEN` token kinds matching `calculatorsym.h`.

A full interactive REPL is intentionally out of scope for this smoke example; the tutorial focuses on generation + conflict-free tables.
