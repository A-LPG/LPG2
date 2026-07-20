# LPG2 — AI / Agent 操作手册

面向 **AI coding agent**（Cursor、Copilot、Claude Code 等）。人类读者请从 [QUICKSTART.md](QUICKSTART.md) 开始。English: [en/AI.md](en/AI.md)。

**读完本文即可：** 拿到生成器 → 写/改 `.g` → 生成表 → 链接 runtime → 验证。不要把生成器当成完整 lexer/parser 框架。

## 0. 一句话心智模型

```
.g 语法  --(lpg-v2.3.0 -table)-->  解析表+动作/AST  --(链接 runtime)-->  可 parse
```

| 部件 | 做什么 | 不做什么 |
|------|--------|----------|
| **生成器** `lpg-v2.3.0` | 读语法，写目标语言的表与 AST/动作 | 不提供完整工业级 lexer；不算 runtime |
| **模板** `lpg-generator-templates-2.1.00/` | 决定生成代码形态 | — |
| **runtime** `runtime/*`（子模块） | 查表、shift/reduce、回溯、recover | 不读 `.g` |
| **用户代码** | 提供 token 流、驱动 parse、消费 AST | — |

## 1. 先定位任务类型

| 用户意图 | 你该做的 | 权威文档 |
|----------|----------|----------|
| 写/改语法、生成解析器、集成到某语言 | 走 §2–§5 工作流 | [USER.md](USER.md)、[GRAMMAR_REFERENCE.md](GRAMMAR_REFERENCE.md) |
| 跑通示例验证环境 | `./examples/calculator/scripts/run.sh <lang>` | [QUICKSTART.md](QUICKSTART.md) |
| 改生成器 C++ 源码 / 自举 / 新后端 | 构建 `lpg2/`，跑 ctest | [DEVELOPER.md](DEVELOPER.md)、`lpg2/BOOTSTRAP.md` |
| 查运行时版本 / 发版 | 读 compat | [ECOSYSTEM.md](ECOSYSTEM.md)、`ecosystem/compat.json` |

## 2. 拿到生成器（任选）

```bash
# A. 最快（无需克隆本仓）
npx lpg2 --help
npx lpg2 -programming_language=java -table your.g

# 脚手架 / Antlr 导入 / 冒烟（包装层子命令）
npx lpg2 init ./my-parser --lang=java
npx lpg2 from-antlr Expr.g4 -o ./out-expr   # 需 LPG2 checkout（antlr2lpg.py）
npx lpg2 test java                          # calculator 或 cwd grammar.g --dry-run

# B. 本仓源码构建（维护/联调）
cd lpg2 && cmake -S . -B build && cmake --build build -j
export LPG_BIN="$PWD/build/lpg-v2.3.0"
# 或：./scripts/lpg2 …

# C. Release 二进制
# https://github.com/A-LPG/LPG2/releases → 校验 SHA256SUMS → export LPG_BIN=.../bin/lpg-v2.3.0
```

**最短生成命令**（CMake 构建或 install 布局下会自动发现 `templates/` + `include/`；未指定时默认 `dtParserTemplateF.gi`）：

```bash
"$LPG_BIN" -programming_language=java -table -quiet -out_directory=./out grammar.g
# 分析不写文件：加 --dry-run（或 -nowrite）
```

仍可显式传 `-template` / `-include-directory`（见 calculator 脚本）。单独移动二进制时设 `LPG_TEMPLATE` / `LPG_INCLUDE` 或 `LPG2_RESOURCE_ROOT`。

## 3. 标准工作流（写语法 → 生成 → 集成）

复制此清单并勾选：

```
- [ ] 1. 确认目标语言与 runtime 子模块已 init
- [ ] 2. 编写/修改 .g（最小骨架见下）
- [ ] 3. -nowrite 先查冲突与错误
- [ ] 4. -table 生成到项目源码目录
- [ ] 5. 链接对应 runtime，写/改驱动（token + parse）
- [ ] 6. 跑通 accept/reject 或项目测试
```

### 3.1 最小 `.g` 骨架

```text
%Options automatic_ast=nested,var=nt,visitor=default
%Options template=dtParserTemplateF.gi
%options package=MyLang

%Terminals
    ID NUMBER PLUS
%End

%Eof
    EOF_TOKEN
%End

%Start
    Expr
%End

%Rules
    Expr$Expr ::= Expr PLUS NUMBER
           | NUMBER
%End
```

完整可运行参考：`examples/calculator/calculator.g`。

### 3.2 常用 CLI

| 标志 | 用途 |
|------|------|
| `-programming_language=<lang>` | 目标后端（见 §4） |
| `-table` | 生成解析表 |
| `-out_directory=<dir>` | 表/动作/`.l` listing 输出目录 |
| `-quiet` | 少打日志 |
| `-nowrite` / `--dry-run` | 只分析，不写文件（等价） |
| `-fail_on_conflicts` | 冲突时退出 12（**CI 推荐**） |
| `-ebnf` | 可选启用 postfix EBNF 语法糖（`?` `*` `+`、分组、`[ ]`/`{ }`、组内 action、量词字段宏）；默认关闭 |
| `-help` / `--version` | 退出 0 |

退出码：**0** = 成功（含仅警告的冲突，除非 `-fail_on_conflicts`）；**12** = 语法/选项错误。失败时事务式发布：不覆盖旧产物、不留半成品。

### 3.3 一键验证环境

```bash
git submodule update --init runtime/lpg-runtime   # 或其它语言 runtime
export LPG_BIN=...   # 若未构建到 lpg2/build/lpg-v*
./examples/calculator/scripts/run.sh java          # cpp|rust|java|typescript|go|python|csharp|dart|all
```

生成脚本等价命令见 `examples/calculator/scripts/generate.sh`。

## 4. 语言与 runtime 矩阵

| CLI 值 | Runtime 子模块 | 备注 |
|--------|----------------|------|
| `java` | `runtime/lpg-runtime` | 入门首选之一；含 `GLRParser`（`-glr` + `glrParserTemplateF.gi`） |
| `cpp` / `c++` / `rt_cpp` | `runtime/LPG-cpp-runtime` | 三者等价；含 `GLRParser`（`-glr` + `rt_cpp/glrParserTemplateF.gi`） |
| `typescript` | `runtime/LPG-typescript-runtime` | 含 `GLRParser`（`-glr` + `templates/typescript/glrParserTemplateF.gi`）；Playground 浏览器 demo |
| `python3` | `runtime/LPG-python-runtime` | **不要**用已移除的 `python2` |
| `go` | `runtime/LPG-go-runtime` | |
| `csharp` | `runtime/LPG-csharp-runtime` | |
| `dart` | `runtime/LPG-Dart-runtime` | |
| `rust` | `runtime/LPG-rust-runtime` | nested + toplevel AST、recover、GLR v2 |

已移除：`c` / `ml` / `plx` / `plxasm` / `xml` / `python2`。版本钉见 `ecosystem/compat.json`。

## 5. 语法要点（agent 易错）

- 区块：`%Name` … `%End`；动作块默认 `/.` … `./`（必须成对关闭）
- `Nonterminal$ClassName`（如 `Expr$Expr`）在 `automatic_ast=nested` 时决定 AST 类名
- 启用 `-ebnf` / `%Options ebnf` 后，裸 `+` `*` `?` `( )` `[ ]` `{ }` 为元字符；运算符终结符请用引号或别名；示例见 `examples/ebnf-call/`
- IDE：[LPG-language-server](https://github.com/A-LPG/LPG-language-server) + [lpg-vscode](https://github.com/A-LPG/LPG-VScode) 按 **EBNF 糖 AST** 做高亮/大纲/跳转（不在 LS 内跑生成器的 `EbnfExpander` / `__ebnf_*` 冲突分析）
- 冲突：默认只警告、退出 0；按下方决策树消歧
- `%Import` + `%DropActions`：复用结构、丢掉外来动作
- **Lexer 通常要你自己写**；calculator 故意用手写 token 列表验证表+runtime
- C++ 增量：token 前缀复用 + 语句级重解析，**不是** tree-sitter 子树复用

指令速查：[GRAMMAR_REFERENCE.md](GRAMMAR_REFERENCE.md)。

### 5.1 冲突消解决策树

按顺序判断；每一步后重新运行 `-nowrite -fail_on_conflicts`。GLR 是例外：
`-glr -fail_on_conflicts` 允许由 GLR 表保留的冲突，仍在 diagnostics 中报告数量：

1. **仅是运算符优先级/结合性？**
   - AST 层级也应体现优先级 → 改写为 `Expr` / `Term` / `Factor`（最直观、默认首选）。
   - 需要保留扁平规则 → 用 `%Left` / `%Right`；仅在前两者不够时用 `%Priority` 明确规则优先级。
2. **两个结构只需再看固定数量的 token 就能区分？** → 设最小可用的 `lalr=N`。适合有界、多 token lookahead；不要靠不断增大 `N` 掩盖真正歧义。
3. **关键字在部分位置也允许作标识符？** → 声明关键字并启用 `soft_keywords`。它解决 keyword/identifier 的上下文重叠，不是通用冲突开关。
4. **语言确实要求保留多条候选路径，或公共前缀无实用固定上界？** → 启用 `backtrack`，并把模板切到 `btParserTemplateF.gi`；同时确认目标 runtime 支持 `BacktrackingParser`。回溯有运行时成本，应晚于改写、优先级和有界 lookahead。
5. **需要同时保留多棵合法解析树（歧义打包）？** → 启用 `-glr`，模板切到 `glrParserTemplateF.gi`，链接对应语言 runtime 的 `GLRParser`（八后端均为 v2：GSS + SPPF）；同语法符号、同 token-index span 的候选经 `getNextAst()` 投影；真共享读 `getSppfRoot()`。打包面向纯 AST 构造动作；`parser(N)`（`N>0`）在 Java/C++ 上 GLR 失败时回退 BT `%Recover` 义肢（单树）；不支持循环/ε 环文法（非循环 nullable 可用）。Playground WASM 可生成；浏览器内解析 demo 仅 TypeScript。
6. **仍有冲突？** → 检查 listing 中的状态/lookahead，缩小出问题的最小规则；不要仅因默认退出 0 就忽略警告。

### 5.2 `%Recover`

只把可安全合成的非终结符放入 `%Recover`；错误恢复可能产生 prosthetic AST，消费端必须容忍占位节点/token。可运行模式见 [`examples/recover/`](../examples/recover/)。

### 5.3 结构化诊断（`--diagnostics=json`）

机器消费入口：`--diagnostics=json`（等价 `-diagnostics=json`）。默认仍是人类可读诊断。JSON 模式下：

- **stdout**：恰好一个 JSON 对象（末尾可有换行），无人类 prose、无生成源码
- **stderr**：保持为空（致命 OS/内部失败除外；JSON 模式会抑制多数 `***ERROR` 横幅）
- 建议配合 `-nowrite -quiet` 做「发版前自检」；`-fail_on_conflicts` 时冲突以 error 进入 `diagnostics[]`

| 字段 | 说明 |
|------|------|
| `schema_version` | 当前为 `1` |
| `diagnostics[]` | `file` / `span{start,end: line,column,offset}` / `code` / `severity` / `message` / `help`；冲突时可选 `conflict_kind`、`example_lookahead` |
| `health` | `available` / `healthy` / `conflict_count` / `shift_reduce_conflicts` / `reduce_reduce_conflicts` / `backtrack` / `glr` / `soft_keywords` / `soft_conflicts` / `recover_symbols[]` / `programming_language` / `write_enabled` / `warning_summary` |

常用 code：`LPG0001` error、`LPG0002` warning、`LPG1001` 未闭合 action block、`LPG2001` shift/reduce、`LPG2002` reduce/reduce、`LPG2003` fail_on_conflicts。

## 6. 仓库地图（别找错目录）

| 路径 | 用途 |
|------|------|
| `lpg2/` | 生成器源码 |
| `lpg-generator-templates-2.1.00/` | 模板与 include（主仓内，非子模块） |
| `runtime/*` | 各语言 runtime（**git 子模块**，干净克隆可能为空） |
| `examples/calculator/` | 端到端入门示例 |
| `grammars-example/` | 更多语法样例（子模块） |
| `tool/` | VS Code 扩展 / LSP（子模块） |
| `docs/` | 人类文档；本文是 AI 入口 |
| `npm/lpg2/` | `npx lpg2` 包装 |

## 7. 反模式（不要做）

1. 生成 `*prs*` 后不链接 runtime、不提供 token 就声称「能 parse」
2. 期望生成器自动产出完整工业 lexer
3. 使用已删除的语言值（`python2`、`c`、`xml`…）
4. 把二进制挪出 install/`share/lpg2` 布局却不设 `LPG2_RESOURCE_ROOT` / `LPG_TEMPLATE`，然后报找不到模板（源码树 CMake 构建通常已自动发现）
5. 把 conflict 警告当成失败（除非加了 `-fail_on_conflicts`）
6. 修改 `src/jikespg_*` 却不走 `lpg2/BOOTSTRAP.md` 审查流程
7. 为空的 `runtime/` 目录困惑却不 `git submodule update --init`

## 8. 深入阅读（按需打开，勿一次全读）

| 需要时 | 打开 |
|--------|------|
| 5 分钟跑通 | [QUICKSTART.md](QUICKSTART.md) |
| 概念 | [CONCEPTS.md](CONCEPTS.md) |
| 跟读 calculator | [tutorial.md](tutorial.md) |
| 集成 / FAQ | [USER.md](USER.md) |
| 指令与 CLI | [GRAMMAR_REFERENCE.md](GRAMMAR_REFERENCE.md) |
| 改生成器 | [DEVELOPER.md](DEVELOPER.md) |
| 版本矩阵 | [ECOSYSTEM.md](ECOSYSTEM.md) |
| Cursor 项目 skill | [../.cursor/skills/lpg2/SKILL.md](../.cursor/skills/lpg2/SKILL.md) |

相对链接检查：`./scripts/check-doc-links.sh`
