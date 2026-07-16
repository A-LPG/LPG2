# Rust wiring notes

1. Generate tables: `../scripts/generate.sh rust`
2. Copy `out-rust/calculatorprs.rs` and `calculatorsym.rs` into a crate that depends on [LPG-rust-runtime](https://github.com/A-LPG/LPG-rust-runtime).
3. Implement `TokenStream` for a hand-written or generated lexer.

See the Rust runtime’s `generated_tables` examples for the `ParseTable` trait shape.
