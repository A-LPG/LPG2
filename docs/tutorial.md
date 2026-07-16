# 教程：从零写一个计算器语法

本教程用 LPG2 生成一个极简整数表达式解析器，覆盖 **C++** 与 **Rust** 两条链路。完整文件在 [`examples/calculator/`](../examples/calculator/)。

## 1. 语法

`examples/calculator/calculator.g`：

- 终结符：`NUMBER`、`PLUS`、`STAR`、`LPAREN`、`RPAREN`、`EOF_TOKEN`
- 规则：`Expr` → 加减，`Term` → 乘，`Factor` → 数字或括号

先只生成解析表，确认语法本身无冲突：

```bash
cd lpg2
cmake -S . -B build && cmake --build build -j
BIN=./build/lpg-v2.3.0   # 或 build-plan 中的同名二进制

"$BIN" -programming_language=cpp -table -quiet \
  -out_directory=../examples/calculator/out-cpp \
  ../examples/calculator/calculator.g
```

若出现 `Shift/reduce conflict`，诊断会给出源码行、`example lookahead` 与 `= help:` 建议。本示例已用分层非终结符消歧，应无冲突。

## 2. C++ 链路

```bash
"$BIN" -programming_language=cpp -table \
  -out_directory=../examples/calculator/out-cpp \
  ../examples/calculator/calculator.g
```

产物：`calculatorprs.h` / `calculatorprs.cpp` / `calculatorsym.h`。  
链接时需要 [LPG-cpp-runtime](../runtime/LPG-cpp-runtime)（子模块）。驱动代码见 `examples/calculator/cpp/main.cpp`（表驱动骨架 + 手写 lexer 示意）。

一键脚本：

```bash
../examples/calculator/scripts/generate.sh cpp
```

## 3. Rust 链路

```bash
"$BIN" -programming_language=rust -table \
  -out_directory=../examples/calculator/out-rust \
  ../examples/calculator/calculator.g
```

将生成的 `calculatorprs.rs` / `calculatorsym.rs` 拷入依赖 [LPG-rust-runtime](https://github.com/A-LPG/LPG-rust-runtime) 的 crate（见 `examples/calculator/rust/`）。

```bash
../examples/calculator/scripts/generate.sh rust
```

## 4. 下一步

- 为 `PLUS` / `STAR` 增加 `%Left` 优先级实验，对比冲突报告
- 换成 `-programming_language=rt_cpp` + automatic AST 模板
- 阅读 [USER.md](USER.md) 的 FAQ 与 [DEVELOPER.md](DEVELOPER.md) 的测试说明
