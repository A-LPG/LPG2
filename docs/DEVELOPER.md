# LPG2 开发者文档

面向**维护、扩展 LPG2 生成器**的读者。使用生成器编写语法请参阅 [用户文档](USER.md)。

## 仓库结构

```
LPG2/
├── lpg2/                    # 生成器源码（本项目的核心）
├── runtime/                 # 各语言运行时（git 子模块）
├── tool/                    # VS Code 扩展与语言服务（子模块）
├── grammars-example/        # 示例语法（子模块）
├── lpg-generator-templates-2.1.00/   # 模板与历史文档
└── docs/
    ├── QUICKSTART.md / CONCEPTS.md / tutorial.md  # 新手路径
    ├── USER.md / DEVELOPER.md / GRAMMAR_REFERENCE.md
    └── en/                  # 英文入门与 USER/DEVELOPER
```

生成器实现集中在 `lpg2/`：

```
lpg2/
├── CMakeLists.txt
├── include/                 # 头文件（含自举生成的 jikespg_*）
├── src/                     # 生成器与已编译的自举解析器
├── grammar/
│   ├── jikespg.g            # 自举语法（权威来源）
│   ├── jikespg.lpg          # 与 .g 保持同步
│   └── .lpg/                # 重新生成时的暂存区
├── tests/                   # ctest 回归（fixtures + runner）
├── scripts/
└── BOOTSTRAP.md             # 自举策略详解
```

## 开发环境

- CMake ≥ 3.16
- C++17 编译器（GCC、Clang 或 MSVC）

一键检测工具链、按需初始化子模块并打印推荐 CMake 参数：

```bash
./scripts/dev-bootstrap.sh --init-submodules
```

生态兼容矩阵与发版清单：[ECOSYSTEM.md](ECOSYSTEM.md) · `../ecosystem/compat.json`。只读校验：`./scripts/release-checklist.sh`。

## 从源码构建

```bash
cd lpg2
cmake -S . -B build
cmake --build build -j
```

产物默认位于 `build/lpg-v2.3.0`。

### CMake 选项

| 选项 | 默认值 | 说明 |
|------|--------|------|
| `LPG2_OUTPUT_DIR` | `${CMAKE_BINARY_DIR}` | 可执行文件输出目录 |
| `LPG2_DEPLOY_DIR` | 空 | 可选的通用二进制部署目录 |
| `LPG2_WARNINGS_AS_ERRORS` | `OFF` | MSVC/GCC 将警告视为错误；Clang 当前只提升 `return-type` / `uninitialized` |
| `LPG2_ENABLE_SANITIZERS` | `OFF` | 为 Clang/GCC 开启 ASan + UBSan |
| `LPG2_CPP_RUNTIME_DIR` | `../runtime/LPG-cpp-runtime` | C++ 运行时（`cpplpg2`），用于 `cpp_automatic_ast_nested` |
| `LPG2_RUST_RUNTIME_DIR` | `runtime/LPG-rust-runtime/lpg2` | 完整 Rust parser / AST 测试 |
| `LPG2_CSHARP_RUNTIME_DIR` | `../runtime/LPG-csharp-runtime/LPG2.Runtime` | C# nested AST e2e |
| `LPG2_DART_RUNTIME_DIR` | `../runtime/LPG-Dart-runtime` | Dart nested AST e2e |

示例：

```bash
cmake -S . -B build -DLPG2_DEPLOY_DIR=/path/to/lpg-vscode/server
cmake --build build -j
```

## 代码架构概览

| 模块 | 主要文件 | 职责 |
|------|----------|------|
| 驱动与选项 | `control.cpp`, `options.cpp`, `option*.cpp` | CLI、版本号、流程控制 |
| 语法分析 | `grammar.cpp`, `parser.cpp`, `jikespg_*.cpp` | 读取 `.g` / `.lpg`，构建内部表示 |
| 表生成 | `*Table.cpp`（如 `CppTable.cpp`, `RustTable.cpp`） | 输出各语言解析表 |
| 动作生成 | `*Action.cpp`（如 `CppAction.cpp`, `RustAction.cpp`） | 输出语义动作代码 |
| 冲突与诊断 | `resolve.cpp`, `diagnose.cpp` | 冲突报告与错误信息 |

新增目标语言时，通常需要实现一对 `*Table` / `*Action`，并在 `grammar.cpp`、`control.cpp`、`options.cpp` 中注册 `Option::<LANG>`。

当前 **Rust** 后端：`src/RustTable.cpp`、`src/RustAction.cpp`（对接 [LPG-rust-runtime](https://github.com/A-LPG/LPG-rust-runtime) 的 `ParseTable` trait）。

## 自举（Bootstrap）

LPG2 用自身语法 `grammar/jikespg.g` 实现。仓库中 `src/jikespg_*.cpp` 与 `include/jikespg_*.h` 是**经审查后**纳入构建的生成物，而非每次构建自动重生成。

重新生成命令：

```bash
lpg-v2.3.0 -programming_language=cpp -table \
  -out_directory=grammar/.lpg \
  grammar/jikespg.g
```

**重要：** `grammar/.lpg/` 仅为暂存区。直接复制到 `src/` 可能引入更多 shift/reduce 冲突并改变解析行为。晋升流程、冲突对比与测试要求见 [lpg2/BOOTSTRAP.md](../lpg2/BOOTSTRAP.md)。

## 测试

生成器回归测试在 [`lpg2/tests/`](../lpg2/tests/)：fixture 语法 +
`run_generation_test.cmake`。除退出码和文件检查外，C++/Rust 生成物会被编译并
执行；Rust 表测试默认使用仓库内 ABI shim，完整 parser 测试在提供
`LPG2_RUST_RUNTIME_DIR` 后链接 `LPG-rust-runtime`。C++ `rt_cpp` nested AST
测试在提供 `LPG2_CPP_RUNTIME_DIR`（默认指向 `runtime/LPG-cpp-runtime`）后链接
`cpplpg2`。8 个 `minimal_*_golden` 用 SHA256 对照
`tests/golden/minimal/{cpp,rust,java,go,python3,csharp,typescript,dart}/`。错误路径还覆盖损坏输入语料、输出事务、CLI
退出码与 listing 目录，以及 `-fail_on_conflicts` 冲突失败。

```bash
cd lpg2
cmake -S . -B build && cmake --build build -j
ctest --test-dir build --output-on-failure          # 全量
ctest --test-dir build -L smoke --output-on-failure # 日常快速冒烟
ctest --test-dir build -L integration               # 自举集成
ctest --test-dir build -R 'conflict_|minimal_.*_golden'   # 冲突 + 8 后端 golden
ctest --test-dir build -R 'cpp_automatic_ast_(nested|recover)'  # C++ nested + recover AST e2e
```

| 标签 | 用例 | 说明 |
|------|------|------|
| `smoke` | `minimal_{cpp,java,go,python3,csharp,typescript,dart,rust}` | 微型语法覆盖 8 个完整后端 |
| `smoke` + `feature` | `dropactions_import` | `%Import` + `%DropActions` |
| `smoke` + `feature` | `conflict_warns_ok` / `conflict_fail_fast` | 默认警告冲突；`-fail_on_conflicts` 退出 12 |
| `smoke` + `golden` | `minimal_*_golden`（8 后端） | `minimal.g` 表与 `tests/golden/minimal/{cpp,rust,java,go,python3,csharp,typescript,dart}/` 对照 |
| `smoke` + `backtracking` | `backtrack_{java,go,...}` | 回溯表跨后端生成烟雾 |
| `smoke` + `cpp` | `cpp_bt_template_smoke` / `cpp_lexer_template_smoke` | `rt_cpp` btParser / Lexer 模板生成 |
| `smoke` + `feature` | `invalid_*_fails` | 精确校验失败状态码、诊断及无残留输出 |
| `compile` + `parser` | `generated_*_compile_run` | 编译表并接受有效输入、拒绝无效输入 |
| `rust` + `parser` | `rust_*_parser_run` / `rust_automatic_ast_*_behavior` | 确定性/回溯 parser；nested children、list、`parent_saved`、env、interface、preorder AST 行为断言 |
| `cpp` + `ast` | `cpp_automatic_ast_nested` / `cpp_automatic_ast_recover` | `rt_cpp` + `cpplpg2`：token 注入、`parser()`、AST root；`%Recover` prosthetic AST |
| `bootstrap` | `bootstrap_stage2` | 重建 stage-1 生成器并比较 stage-2 输出与冲突数 |
| `output` + `unit` | `output_transaction` | 验证失败回滚与成功原子发布 |
| `fuzz` + `negative` | `malformed_input_corpus` | 损坏语法不得 abort、崩溃或遗留产物 |
| `cli` | `cli_contract` | 帮助/版本/退出码、stderr、listing 目录和生成摘要 |

### 新增 case

在 [`lpg2/tests/CMakeLists.txt`](../lpg2/tests/CMakeLists.txt) 调用：

```cmake
lpg2_add_generation_test(<name> <grammar.g> <lang>
    [PREFIX <file_prefix>]   # 默认取语法文件基名
    [EXPECT_EXIT_CODE <n>]   # 精确失败状态码
    [EXPECT_MESSAGE <text>]  # 诊断必须包含的文本
    [EXTRA_ARGS <cli...>]    # 额外传给 lpg2 的参数
    [CHECK_GOLDEN GOLDEN_DIR <dir>]  # 对 *prs.* 做 SHA256 对照
    [CHECK_CPP|CHECK_RUST|CHECK_JAVA]   # 编译并运行生成物
    [LABELS smoke|integration|feature ...])
```

需要时把 fixture 放进 `lpg2/tests/fixtures/`。更新 8 个完整后端的 golden 表：

```bash
cd lpg2
./scripts/update_golden_tables.sh
# 或：LPG_BIN=/path/to/lpg-v2.3.0 ./scripts/update_golden_tables.sh
```

### 下游 Java 运行时（可选）

仓库已含子模块 `runtime/lpg-runtime`。开启可执行解析 / nested AST e2e：

```bash
cmake -S lpg2 -B build \
  -DLPG2_REQUIRE_JAVA_PARSER_TESTS=ON \
  -DLPG2_JAVA_RUNTIME_DIR=$PWD/runtime/lpg-runtime
cmake --build build -j
ctest --test-dir build -R '^java_automatic_ast_(nested|recover)$' --output-on-failure
```

需要本机 `javac` / `java`（JDK 8+）。CI 的 `java-runtime-integration` job 会跑 nested 与 recover。

其余语言可执行 e2e 已齐：TypeScript / C# / Dart / Go / Python（`*_automatic_ast_nested` + `*_automatic_ast_recover`）。CI 各语言 integration job 均必跑这两类用例。

### 下游 Rust 运行时（可选）

Rust runtime 是 git 子模块 [`runtime/LPG-rust-runtime`](../runtime/LPG-rust-runtime)。递归克隆后：

```bash
git submodule update --init runtime/LPG-rust-runtime

cmake -S lpg2 -B build \
  -DLPG2_REQUIRE_RUST_TESTS=ON \
  -DLPG2_REQUIRE_RUST_PARSER_TESTS=ON \
  -DLPG2_RUST_RUNTIME_DIR=$PWD/runtime/LPG-rust-runtime/lpg2
cmake --build build -j
ctest --test-dir build -R '^rust_' --output-on-failure
```

默认缓存路径：`${LPG2}/runtime/LPG-rust-runtime/lpg2`。
也可设置 `LPG2_RUST_RUNTIME_DIR` 指向任意 checkout。

在子模块内跑示例表测试：

```bash
cd runtime/LPG-rust-runtime
cargo test -p generated_tables
```

刷新示例表：

```bash
cd lpg2
./scripts/regen_rust_example_tables.sh
# 或：LPG_BIN=/path/to/lpg-v2.3.0 ./scripts/regen_rust_example_tables.sh
```

## 维护脚本

| 脚本 | 用途 |
|------|------|
| `lpg2/scripts/update_golden_tables.sh` | 刷新 `tests/golden/minimal/{cpp,rust,java,go,python3,csharp,typescript,dart}` |
| `lpg2/scripts/regen_rust_example_tables.sh` | 用自举语法更新 Rust 运行时示例表 |
| `lpg2/scripts/go_to_rust_action.py` | 从 Go 后端迁移/维护 Rust 后端的辅助工具 |
| `lpg2/scripts/perf_baseline.sh` | 生成耗时 / RSS 基线（默认 `grammar/jikespg.g`） |

## 版本与命名

- 单一版本源：`lpg2/CMakeLists.txt` 中的 `LPG2_VERSION`
- `lpg_version.h`、`Control::VERSION`、可执行文件名和 CPack 包名均由其生成

修改版本时只需更新 `LPG2_VERSION` 与用户可见文档。

## 安装与发布

```bash
cmake --install build --prefix ./install
cmake --build build --target package
```

安装包包含 `bin/lpg-v2.3.0`、用户文档以及
`share/lpg2/lpg-generator-templates-2.1.00/`。`.github/workflows/release.yml`
在 `v*` 标签上构建 Linux、macOS、Windows 包，生成 `SHA256SUMS` 并发布 GitHub
Release；CI 的三平台 job 也会先验证 CPack。

## 子模块与扩展开发

初始化子模块：

```bash
git submodule update --init --recursive
```

扩展开发时，可设置 `LPG2_DEPLOY_DIR` 将新构建的二进制放入本地
`lpg-vscode` server 路径，便于联调；仓库不再包含个人机器绝对路径。

VS Code 扩展的 `templates/` 与 `server/` 被 gitignore，发布前需装配：

```bash
# 从 LPG2 仓库根目录；需已构建生成器，并可选提供 LSP 二进制
./tool/LPG-VScode/scripts/assemble-release.sh \
  --lpg-bin path/to/lpg-v2.3.0 \
  [--lsp-bin path/to/lpg-language-server]
# 然后在 tool/LPG-VScode 内 yarn && npx vsce package
```

CI 的 `runtime-integration` job 会使用子模块 `runtime/LPG-rust-runtime`，并
clone `LPG-cpp-runtime`（含嵌套子模块），开启 `LPG2_REQUIRE_CPP_PARSER_TESTS` 与
`LPG2_REQUIRE_RUST_PARSER_TESTS`。`java-runtime-integration` 使用子模块
`runtime/lpg-runtime` 并开启 `LPG2_REQUIRE_JAVA_PARSER_TESTS`。

## 已知限制（维护者备忘）

| 区域 | 说明 |
|------|------|
| C / ML / Plx / Plxasm / Xml 后端 | **已移除**（#13）；请改用 `java` / `cpp` / `rt_cpp` 等完整后端 |
| `grammar/.lpg/` vs `src/` | 可能存在表漂移；以 `src/` 编译结果为准，晋升需走 BOOTSTRAP 流程 |
| Rust automatic AST | Generator-side closed loop for `nested`（无 `parent_saved` 亦有 `get_children`）、list、`parent_saved`、`needs_environment`、interface/`dyn` RHS、`visitor=default` / `visitor=preorder`：由 `rust_automatic_ast_*_behavior` 断言。复杂语法仍建议小步验证；不宣称 `toplevel`/GLR 或与 Java/C++ 全量 AST 变体对等 |
| Incremental parse | C++：token 级 damage-offset 重词法 + 语句级增量步进（见 `incremental_prs_stream` / `cpp_automatic_ast_incremental`）；**不是** tree-sitter 子树复用 |
| Cross-backend AST dump | `ast_shape_diff_nested` / `ast_shape_diff_list`：各后端 harness 写出统一 S-expr（`expected/*.sexpr`） |
| expected-tokens | C++/TS `expectedTerminalNames(prs, state)`（补全基石）；`cpp_expected_tokens` / `typescript_expected_tokens` |
| C++ automatic AST | `rt_cpp` + `dtParserTemplateF.gi` 的 `nested` AST 由 `cpp_automatic_ast_nested` 覆盖（需 `LPG2_CPP_RUNTIME_DIR`） |
| Java automatic AST | `java` + `dtParserTemplateF.gi` 的 `nested` AST 由 `java_automatic_ast_nested` 覆盖（需 JDK + `LPG2_JAVA_RUNTIME_DIR`） |
| Python automatic AST | `python3` + `dtParserTemplateF.gi` 的 `nested` AST 由 `python_automatic_ast_nested` 覆盖（需 python3 + `LPG2_PYTHON_RUNTIME_DIR`） |
| Go automatic AST | `go` + `dtParserTemplateF.gi` 的 `nested` AST 由 `go_automatic_ast_nested` 覆盖（需 go + `LPG2_GO_RUNTIME_DIR`） |
| TypeScript automatic AST | `typescript` + `dtParserTemplateF.gi` 的 `nested` AST 由 `typescript_automatic_ast_nested` 覆盖（需 node/npm + `LPG2_TYPESCRIPT_RUNTIME_DIR`） |
| C# automatic AST | `csharp` + `dtParserTemplateF.gi` 的 `nested` AST 由 `csharp_automatic_ast_nested` 覆盖（需 dotnet + `LPG2_CSHARP_RUNTIME_DIR`） |
| Dart automatic AST | `dart` + `dtParserTemplateF.gi` 的 `nested` AST 由 `dart_automatic_ast_nested` 覆盖（需 dart + `LPG2_DART_RUNTIME_DIR`） |
| 其它语言 e2e | 八后端 nested AST 可执行 e2e 已齐（Java / Python / C++ / Rust / Go / TypeScript / C# / Dart） |
| `cpp` / `c++` / `rt_cpp` | 三者等价，均走 `CppAction2`/`CppTable2` |
| `cpp_legacy` | 自举专用（旧 `CppTable` + `*.cpp` 表）；见 [BOOTSTRAP.md](../lpg2/BOOTSTRAP.md) |
| recover / prosthetic AST | **全部后端已完成**（Java、C++、Rust、Go、C#、TypeScript、Dart、Python）：`%Recover` 可带可选 action block（`/. expr ./`，表达式可引用 `error_token`）作为 `$allocation`；无 block 时占位 `AstToken`（或各后端等价类型）。经 `getProsthesisIndex` + `EmitProstheticAstFactories` + 运行时 `ProstheticAst`/`BacktrackingParser` 闭环，各后端由 `*_automatic_ast_recover` e2e 覆盖。各后端以惯用方式提供可选访问器（默认接口方法 / 结构化或可选接口 / Dart mixin / Python 鸭子类型） |
| TODO 分级 | 见 [TODO_TRIAGE.md](TODO_TRIAGE.md) |

## 相关仓库

| 仓库 | 角色 |
|------|------|
| [LPG2](https://github.com/A-LPG/LPG2) | 生成器（本仓库） |
| [LPG-rust-runtime](https://github.com/A-LPG/LPG-rust-runtime) | Rust 运行时 |
| [LPG2-grammars-example](https://github.com/A-LPG/LPG2-grammars-example) | 示例语法 |
| [LPG-VScode](https://github.com/A-LPG/LPG-VScode) | VS Code 扩展 |

---

下一步：[用户文档](USER.md) · [自举策略](../lpg2/BOOTSTRAP.md) · [仓库首页](../README.md)
