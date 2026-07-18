# LPG2 语法参考（v1）

面向日常编写 `.g` / `.lpg` 的作者。权威细节仍以生成器行为与 [USER.md](USER.md) 为准；历史 AST 说明见 [`lpg-generator-templates-2.1.00/docs/Ast.txt`](../lpg-generator-templates-2.1.00/docs/Ast.txt)。

**新手请先读：** [CONCEPTS.md](CONCEPTS.md)（心智模型）→ [tutorial.md](tutorial.md)（计算器跟读）→ 再查本页。

English summary: [en/GRAMMAR_REFERENCE.md](en/GRAMMAR_REFERENCE.md)。

## 文件形态

- 扩展名：`.g`、`.lpg`、`.gi`（模板/片段）
- 区块以 `%Name` … `%End` 组织（大小写不敏感于多数关键字）
- 动作块默认定界符：`/.` … `./`（可用选项改写）

完整最小示例：[`examples/calculator/calculator.g`](../examples/calculator/calculator.g)。

## 常用指令

| 指令 | 作用 |
|------|------|
| `%Terminals` | 声明终结符 |
| `%Eof` | 声明文件结束符（常为 `EOF_TOKEN`） |
| `%Error` | 错误 token（可选） |
| `%Start` | 开始符号 |
| `%Rules` | 产生式 |
| `%Import` | 导入另一语法/模板片段 |
| `%DropActions` | 丢弃导入语法中的动作，只保留结构 |
| `%Left` / `%Right` / `%Nonassoc` | 运算符优先级与结合性 |
| `%Priority` | 显式优先级（与冲突消解相关） |
| `%Recover` | 声明可合成的非终结符（prosthetic AST） |
| `%Headers` / `%Globals` / `%Trailers` | 注入生成代码的前后文 |
| `$Ast` … `$End` | 向 AST 根类注入字段/方法（automatic AST） |

## 产生式与命名

```text
%Rules
    Expr$Expr ::= Expr PLUS Term
               | Term
    Factor$Factor ::= NUMBER
                   | LPAREN Expr RPAREN
%End
```

- `Nonterminal$ClassName`：automatic AST 时生成的节点类名
- 右部可含终结符、非终结符；空产生式用空右部表示（依模板/语言惯例）

## `%options`（节选）

| 选项 | 含义 |
|------|------|
| `programming_language=…` | 也可由 CLI `-programming_language=` 指定 |
| `automatic_ast=nested` | 嵌套 AST（常用） |
| `visitor=default` / `visitor=preorder` | 生成访问者 |
| `var=nt` | AST 变量命名风格 |
| `template=dtParserTemplateF.gi` | 确定性 parser 模板 |
| `package=Name` | 生成代码包名/命名空间（语言相关） |
| `verbose` | 更详细 listing |
| `backtrack` | 回溯表（配合 `btParserTemplateF.gi`） |
| `glr` | GLR 冲突表（与 backtrack 同编码；配合 `glrParserTemplateF.gi`；Java / C++ / TypeScript runtime 有 `GLRParser`） |

完整列表：`lpg-v2.3.0 -help`。

## 动作块

语义动作写在产生式旁或专用区块中，定界 `/.` … `./`：

```text
Stmt ::= ID EQ Expr /.
            // host-language statements; may use $symbol macros from templates
         ./
```

模板（`dtParserTemplateF.gi` 等）定义可用的 `$` 宏（如 `$rule_number`、`$setSym1`）。不同语言模板宏集合略有差异。

## 冲突与优先级

- 默认：冲突打印警告，**不**失败（退出 0）
- CI 建议：`-fail_on_conflicts` → 冲突时退出 12
- 用 `%Left` / `%Right` / `%Priority` 或改写规则消歧
- 诊断含源行摘录、caret 与示例 lookahead
- 跟读练习：[tutorial.md](tutorial.md) §6

## Backtracking

需要消歧多路径时：

1. 语法/选项启用 backtrack
2. 使用 `btParserTemplateF.gi`（及语言对应 include）
3. 链接支持 `BacktrackingParser` 的运行时

八后端均有 `backtrack_*` 生成烟雾测试。

## GLR（Java / C++ / TypeScript）

当语言需要**同时保留**多棵合法解析树（而非 backtrack 试错取一条）时：

1. 启用 `-glr` / `%Options glr`（生成与 backtrack 相同的多候选冲突表，并打 `isGLR()` 标志；AST 带 `nextAst` 链）
2. 使用 `glrParserTemplateF.gi`（Java：`templates/java/`；C++：`templates/rt_cpp/`；TypeScript：`templates/typescript/`）
3. 链接对应 runtime 的 `GLRParser`（Go/Rust/C#/Python3/Dart 仍仅有脚手架）

**GLR v2** 使用图结构栈（GSS：`GssNode`/`GssEdge`）前缀共享，并在归约时构建
共享包解析森林（SPPF：`SppfNode`）。兼容 API 仍通过 `IAst.getNextAst()` /
`setNextAst` 按语法符号与 token-index span 投影局部歧义森林；需要真共享时读
`GLRParser.getSppfRoot()` / `getSppfSymbolCount()`。
`error_repair_count>0` 时：GLR 失败后回退到 `BacktrackingParser.fuzzyParse*`，
走与 bt 相同的 `RecoveryParser` + `%Recover` 义肢回放（单棵树，不打 `nextAst` 森林）；
再失败则模板侧 `DiagnoseParser` 后返回 `null`。`error_repair_count==0` 不修复。
循环/ε 环文法由保护上限拒绝；非循环 nullable 规则可用。
打包假定规则动作是生成器产出的纯 AST 构造；不同的非 AST 语义值不会被合并。

`GLRParser.parse*()` 在非法输入时抛 `BadParseException`；生成模板的
`parser()` / `parseX()` 为保持 LPG API 风格会捕获它并返回 `null`（C++ 返回空指针）。
GLR 中保留的冲突是预期输入，因此 `-glr -fail_on_conflicts` 不会失败，
`health.healthy` 可为 `true`，同时 `conflict_count` 仍报告实际冲突数。

测试：`glr_tables_golden_java`、`java_glr_ambiguous_e2e`（Catalan + SPPF 共享）、
`java_glr_entry_e2e`、`java_glr_rr_epsilon_e2e`、
`java_glr_correlation_e2e`、`java_glr_symbol_identity_e2e`、
`java_glr_cyclic_e2e`、`java_glr_non_ast_e2e`、`java_glr_recover_e2e`、
`cpp_glr_ambiguous_e2e`、`cpp_glr_recover_e2e`。
八后端另有 `glr_*_smoke`（含 `nextAst` 脚手架形状检查）。

## Automatic AST

```text
%Options automatic_ast=nested,var=nt,visitor=default
%Options template=dtParserTemplateF.gi
```

- `nested`：节点含子节点访问器；Rust 另有 behavior 测试覆盖 list / parent / env / visitor
- `toplevel` AST 布局与其它后端的 GLR 驱动仍不对等；Java/C++/TypeScript 已有 GLR v2（GSS/SPPF）
- 可运行示例：[examples/calculator](../examples/calculator/)（`automatic_ast=nested` + `dtParserTemplateF.gi`）

## `%Recover`（prosthetic AST）

```text
%Recover
    MissingExpr
    MissingStmt /. new MissingStmt(error_token, error_token) ./
%End
```

- 可选 action block 会成为工厂的 `$allocation`，其中可引用恢复运行时传入的 `error_token`
- 无 block 时，生成器默认发出类型化假肢：`Missing*(error_token, error_token)`（CTC/`AstToken` 路径）。若该符号 `needs_environment`，则回退为 `AstToken(error_token)`（lambda 中不能安全捕获 `this`）
- 需 automatic AST + 运行时 `ProstheticAst` / `BacktrackingParser` 路径
- CI：各语言 `*_automatic_ast_recover`
- 可运行 Java cookbook：[examples/recover](../examples/recover/)

### `$allocation` 决策树

1. **接收方接受生成的 `Missing*` 类型，且只需错误位置？**
   - 是：省略 block，使用默认类型化假肢。
   - 否：继续。
2. **父节点字段、rule action、visitor 或业务代码要求非默认构造／额外状态？**
   - 是：写 `$allocation` block，返回可赋给该类型的节点。
   - 否：默认 placeholder 通常足够。
3. **符号需要 environment（`needs_environment`）或自定义诊断字段？**
   - 是：写 `$allocation` block（默认会回退 `AstToken`，或你自行构造带环境的节点）。
   - 否：优先默认 placeholder，减少目标语言专属代码。

自定义 allocation 必须返回非 `null`、类型兼容的 AST，并使用 `error_token`
保留错误位置；只有左右边界的节点通常可将同一个 token 同时传给构造器两端：

```text
MissingStmt /. new MissingStmt(error_token, error_token) ./
```

## 导入

```text
%Import
    path/to/other.g
%End

%DropActions
    ImportedNonterminal
%End
```

用于模板复用与「只要结构不要动作」的裁剪。见测试 `dropactions_import`。

## 常用 CLI

| 参数 | 说明 |
|------|------|
| `-programming_language=` | `java` / `cpp` / `rt_cpp` / `rust` / `python3` / … |
| `-table` | 生成解析表 |
| `-out_directory=` | 输出目录（含 listing） |
| `-template=` | 模板 `.gi` 路径 |
| `-include-directory=` | 模板 include 搜索路径 |
| `-quiet` | 减少控制台输出 |
| `-nowrite` | 只分析不写文件 |
| `-fail_on_conflicts` | 冲突即失败（退出 12） |
| `-help` / `--version` | 帮助 / 版本（退出 0） |

环境变量：`LPG_TEMPLATE`、`LPG_INCLUDE`（移动二进制时）。

## 后端状态

见 [ECOSYSTEM.md](ECOSYSTEM.md)。`python2` 与 stub 后端（`c`/`ml`/`plx`/`plxasm`/`xml`）已移除。
