# 教程：从零写一个计算器语法

本教程用 LPG2 生成一个极简整数表达式解析器，覆盖 **Java / TypeScript / C++ / Rust** 四条链路。完整文件在 [`examples/calculator/`](../examples/calculator/)。

English: [en/tutorial.md](en/tutorial.md)。概念背景：[CONCEPTS.md](CONCEPTS.md)。尚未跑通？先看 [QUICKSTART.md](QUICKSTART.md)。

**一键跑通（推荐）：** 准备好 `LPG_BIN` 与对应 runtime 子模块后：

```bash
# 仓库根目录；默认跟读 Java（需 JDK）
./examples/calculator/scripts/run.sh java
# 或：typescript | cpp | rust | all
```

## 0. 目标与目录

| 路径 | 角色 |
|------|------|
| `examples/calculator/calculator.g` | 共享语法 |
| `examples/calculator/scripts/generate.sh` | 只生成表 |
| `examples/calculator/scripts/run.sh` | 生成 + 构建 + accept/reject |
| `examples/calculator/{java,typescript,cpp,rust}/` | 各语言驱动 |

成功标准：accept `NUMBER + NUMBER * NUMBER`，reject 以 `PLUS` 开头的序列。

## 1. 读懂 `calculator.g`

源文件：[`examples/calculator/calculator.g`](../examples/calculator/calculator.g)。

### 选项

```text
%Options automatic_ast=nested,var=nt,visitor=default
%Options template=dtParserTemplateF.gi
%options verbose
%options package=Calculator
```

- `automatic_ast=nested`：生成嵌套 AST 节点（类名来自规则上的 `$ClassName`）
- `template=dtParserTemplateF.gi`：确定性 parser 模板（脚本还会用 CLI `-template=` 指到语言目录）
- `package=Calculator`：生成代码的包名/命名空间（语言相关）

### 终结符与起止

```text
%Terminals
    NUMBER PLUS STAR LPAREN RPAREN
%End

%Eof
    EOF_TOKEN
%End

%Start
    Expr
%End
```

这些名字会进入 `*sym.*` 常量。驱动里注入 token 时必须使用**相同编号**（见各语言 `Main`）。

### 规则分层（消歧）

```text
%Rules
    Expr$Expr ::= Expr PLUS Term | Term
    Term$Term ::= Term STAR Factor | Factor
    Factor$Factor ::= NUMBER | LPAREN Expr RPAREN
%End
```

- `Expr` 处理 `+`，`Term` 处理 `*`，`Factor` 处理数字与括号 → `*` 比 `+` 紧，且无 shift/reduce 冲突
- `Expr$Expr` 中 `$` 右侧是 automatic AST 的类名

### EBNF 语法糖（可选）

calculator 仍用手写分层 BNF，方便讲优先级。可选/列表糖用 `%Options ebnf`（或 `-ebnf`）：

```text
%Options ebnf,automatic_ast=nested,var=nt,visitor=default
%Rules
    Call ::= ID LPAREN (Expr (COMMA Expr)*)? RPAREN
    Trailing ::= [SEMICOLON]    -- ISO 可选
    Items ::= {Item}            -- ISO 零次或多次
%End
```

- 生成表前降级为稳定的 `__ebnf_*` 辅助非终结符；`*` / `{…}` 与手写 `List$$Elem` 同 list AST 形状
- 运算符终结符请用引号或别名（`'+'` / `PLUS`）；启用 `ebnf` 后裸 `+` `*` `?` `( )` `[ ]` `{ }` 为元字符
- 需要自定义 list 类名或非 list AST 时，继续手写 BNF
- 可运行示例：[`examples/ebnf-call/`](../examples/ebnf-call/)；详见 [GRAMMAR_REFERENCE.md](GRAMMAR_REFERENCE.md)

## 2. 只分析、不写入

先确认语法本身干净（把 `LPG_BIN` 换成你的路径）：

```bash
"$LPG_BIN" -programming_language=java -table -nowrite -quiet \
  -template=lpg-generator-templates-2.1.00/templates/java/dtParserTemplateF.gi \
  -include-directory=lpg-generator-templates-2.1.00/include/java \
  examples/calculator/calculator.g
```

或用脚本生成（会写入 `out-*`）：

```bash
./examples/calculator/scripts/generate.sh java
```

若出现 `Shift/reduce conflict`，诊断通常包含：

- 源码行摘录与插入符 `^`
- `example lookahead: …`
- `= help:` 修复建议

本示例应无冲突。语法/选项错误时退出码为 **12**。

## 3. 生成与产物清单

```bash
./examples/calculator/scripts/generate.sh java|typescript|cpp|rust
```

输出目录：`examples/calculator/out-<lang>/`。

| 语言 | 脚本参数 | 典型产物 |
|------|----------|----------|
| Java | `java` | `calculator.java`, `calculatorprs.java`, `calculatorsym.java` |
| TypeScript | `typescript` | `calculator.ts`, `calculatorprs.ts`, `calculatorsym.ts` |
| C++ | `cpp`（内部用 `rt_cpp`） | `calculator*.h` / `*.cpp` 等 |
| Rust | `rust` | `calculatorprs.rs`, `calculatorsym.rs` 等 |

另有 listing 文件 `*.l`（详细分析日志）。失败不会覆盖已有成功产物。

## 4. 跑通一条语言（默认 Java）

前置：

```bash
git submodule update --init runtime/lpg-runtime
export LPG_BIN=…   # Release 或 lpg2/build/lpg-v2.3.0
./examples/calculator/scripts/run.sh java
```

其它语言（细节见子目录 README）：

| 语言 | 命令 | 子模块 |
|------|------|--------|
| TypeScript | `./examples/calculator/scripts/run.sh typescript` | `runtime/LPG-typescript-runtime` |
| C++ | `./examples/calculator/scripts/run.sh cpp` | `runtime/LPG-cpp-runtime` |
| Rust | `./examples/calculator/scripts/run.sh rust` | `runtime/LPG-rust-runtime` |

语言特有构建说明：

- [Java](../examples/calculator/java/README.md)
- [TypeScript](../examples/calculator/typescript/README.md)
- [C++](../examples/calculator/cpp/README.md)
- [Rust](../examples/calculator/rust/README.md)

## 5. 驱动在做什么

各语言 `Main`（或测试）大致相同：

1. 构造一串 **token 种类**（`NUMBER`, `PLUS`, `STAR`, …），**没有**完整字符级 lexer
2. 用生成的符号常量与解析表，调用运行时进入 `parse`
3. 合法序列应成功；以 `PLUS` 开头应失败

这是刻意简化：证明生成表 + 运行时闭环。真实项目里你需要自己写 lexer（或用模板/其它工具），再把 token 流交给 runtime。

## 6. 小练习

1. **优先级声明**：在 `%Rules` 外尝试 `%Left PLUS` / `%Left STAR`（或查阅 [GRAMMAR_REFERENCE.md](GRAMMAR_REFERENCE.md)），对比「分层规则」与「优先级声明」两种风格。
2. **看冲突**：临时把 `Expr` 与 `Term` 合成一条含糊规则，用 `-nowrite` 或去掉 `-quiet` 观察诊断中的 caret 与 `= help:`。
3. **换语言**：同一语法对 `typescript` 再跑一遍 `run.sh`，对比 `out-*` 文件后缀。

改语法后记得重新 `generate.sh` / `run.sh`。

## 7. 故障排查

| 现象 | 可能原因 | 处理 |
|------|----------|------|
| `Set LPG_BIN to lpg2 executable` | 未找到生成器 | `export LPG_BIN=…` 或先构建 `lpg2/build` |
| 子模块目录几乎为空 | 未 init | `git submodule update --init runtime/…` |
| 找不到模板 / include | 未从仓库根运行，或模板路径错 | 在仓库根执行脚本；确认 `lpg-generator-templates-2.1.00/` 存在 |
| 退出码 12 | 语法/选项错误或（若开启）冲突失败 | 读 stderr 诊断；默认冲突只警告 |
| Java `javac` 失败 | 子模块缺文件或 JDK 过旧 | 确认 `runtime/lpg-runtime/src`；JDK 8+ |
| C++ 链接失败 | runtime 路径不对 | 脚本传入 `LPG2_CPP_RUNTIME_DIR`；检查子模块 |
| TS `npm` 失败 | 网络或 runtime 未 init | init `LPG-typescript-runtime` 后重试 |

## 8. 下一步

- 集成到自己的工程：[USER.md](USER.md)
- 指令与选项速查：[GRAMMAR_REFERENCE.md](GRAMMAR_REFERENCE.md)
- 更多语法样本：`grammars-example/` 子模块 / [LPG2-grammars-example](https://github.com/A-LPG/LPG2-grammars-example)
- 各语言运行时版本：[ECOSYSTEM.md](ECOSYSTEM.md)
- 维护生成器：[DEVELOPER.md](DEVELOPER.md)
