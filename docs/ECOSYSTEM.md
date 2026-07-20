# LPG2 生态兼容与发布

机器可读清单：[ecosystem/compat.json](../ecosystem/compat.json)。

生成器 **2.3.0** 与当前钉住的运行时 / 扩展子模块构成一套可验证生态。发版或 bump 子模块时，先更新 `compat.json`，再改本文。

## 支持的后端

| 后端 | CI（nested / recover） | GLR | 状态 |
|------|------------------------|-----|------|
| `cpp` / `c++` / `rt_cpp` | 是（另含 glr e2e） | **driver**（`GLRParser`） | supported |
| `java` | 是（另含 glr e2e） | **driver**（`GLRParser`） | supported |
| `python3` | 是（另含 glr e2e） | **driver**（`GLRParser`） | supported |
| `csharp` | 是（另含 glr e2e） | **driver**（`GLRParser`） | supported |
| `go` | 是（另含 glr e2e） | **driver**（`GLRParser`） | supported |
| `typescript` | 是 | **driver**（`GLRParser`；Playground 浏览器 demo） | supported |
| `dart` | 是（另含 glr e2e） | **driver**（`GLRParser`） | supported |
| `rust` | 是（另含 glr e2e） | **driver**（`GLRParser`） | supported |
| `cpp_legacy` | bootstrap | — | 内部自举 |

特性：`nested` AST、`%Recover` prosthetic AST、backtracking、八后端 golden 表。
GLR：见 `compat.json` → `features.glr`（八后端 v2 GSS/SPPF 驱动，`sppf: true`；Playground WASM 可 `-glr` 生成，浏览器内解析 demo 仅 TypeScript）。

## 运行时与包坐标

| 语言 | 子模块 | 包 | 版本 | 发布状态 |
|------|--------|----|------|----------|
| C++ | `runtime/LPG-cpp-runtime` | CMake 源码 | 1.0.0 | source |
| Java | `runtime/lpg-runtime` | `org.lpg2:lpg.runtime` | 1.9.0 | workflow |
| Rust | `runtime/LPG-rust-runtime` | crate `lpg2` | 1.0.0 | workflow |
| TypeScript | `runtime/LPG-typescript-runtime` | npm `lpg2ts` | 0.0.11 | workflow |
| C# | `runtime/LPG-csharp-runtime` | NuGet `LPG2.Runtime` | 1.0.2 | workflow |
| Python 3 | `runtime/LPG-python-runtime` | `lpg2-python3-runtime` | 0.0.1 | planned |
| Go | `runtime/LPG-go-runtime` | `github.com/A-LPG/LPG-go-runtime` | tags | tags |
| Dart | `runtime/LPG-Dart-runtime` | pub `lpg2` | 1.0.0 | manual |

钉住 SHA 见 `compat.json` 的 `pinned` 字段。

## 工具链

| 组件 | 版本 | 说明 |
|------|------|------|
| 生成器 | 2.3.0 | GitHub Releases + CPack |
| 模板 | `lpg-generator-templates-2.1.00` | 随安装包分发 |
| VS Code 扩展 | 0.0.22 | Marketplace / GitHub Release；Problems 生成器诊断 / analyzeOnSave；EBNF 着色 |
| Language Server | 0.2.5（子模块 tip） | 糖 AST EBNF；语义着色区分终结符/非终结符；由扩展 assemble 可选打包 |

## Release Checklist

一次生成器小版本发布建议按序勾选：

1. [ ] 更新 `lpg2/CMakeLists.txt` 中 `LPG2_VERSION`
2. [ ] 更新用户可见文档与 `CHANGELOG.md`
3. [ ] `./lpg2/scripts/update_golden_tables.sh`（若表格式变化）
4. [ ] 本地 / CI：smoke + 八后端 golden + nested/recover
5. [ ] 需要时 bump 运行时 / 扩展子模块指针
6. [ ] 更新 [`ecosystem/compat.json`](../ecosystem/compat.json)（版本、pinned、包状态）
7. [ ] 同步本文表格
8. [ ] tag `vX.Y.Z` → 触发 [release.yml](../.github/workflows/release.yml)
9. [ ] 装配 VS Code VSIX（`tool/LPG-VScode/scripts/assemble-release.sh`）并发布
10. [ ] 按价值发布语言包（npm → NuGet → Cargo → Maven / PyPI / pub / Go tag）
11. [ ] 运行 `scripts/release-checklist.sh` 做只读校验（可选）

## 支持策略

- **Supported：** 上表八后端；CI 必跑 nested + recover。
- **Removed：** `python2` / `c` / `ml` / `plx` / `plxasm` / `xml`（#13；CLI 拒绝）。请用 `python3` 替代 `python2`。
- 英文版：[`en/ECOSYSTEM.md`](en/ECOSYSTEM.md)。
- **Incremental parsing（诚实定位）：** C++ runtime 提供 **token 级增量重词法**（`PrsStream::incrementalResetAtCharacterOffset` + lexer `incrementalLexer`）与 **语句级增量重解析**（`DeterministicParser::parse(vector, int)` 步进）；TypeScript runtime 另有 `IncrementalParse` 辅助 API（playground demo）。契约测试：`incremental_prs_stream`、`cpp_automatic_ast_incremental`。这**不是** tree-sitter 式子树复用（无 `tree.edit()` / 子树 reuse）。
- **Cross-backend AST shape：** nested/list fixture 的统一 S-expr dump + `ast_shape_diff_*` ctest。
- **expected-tokens：** 八后端 runtime 暴露 `expectedTerminalNames`（或等价 API；按状态枚举合法终结符），供编辑器补全。CI：`*_expected_tokens`。
- 安全问题见 [SECURITY.md](../SECURITY.md)。

## Runtime README 约定

统一章节模板：[runtime-README.template.md](runtime-README.template.md)。各 `runtime/*` README 已按此结构整理。

## 工作流一览（本仓）

| Workflow | 作用 |
|----------|------|
| `ci.yml` | 生成器 + 八后端 nested/recover + calculator quickstarts |
| `tool-ci.yml` | VS Code assemble/VSIX + Language Server 构建 |
| `vscode-release.yml` | tag `vscode-v*` → VSIX Release（可选 Marketplace） |
| `runtime-head-probe.yml` | 周扫 runtime origin/HEAD 与 pinned SHA |
| `release.yml` | 生成器 `v*` 二进制发布 |

包发布模板与 runtime CI 模板：[`ecosystem/workflows/`](../ecosystem/workflows/)。

## 相关文档

- [USER.md](USER.md) — 语法作者
- [DEVELOPER.md](DEVELOPER.md) — 维护者
- [TODO_TRIAGE.md](TODO_TRIAGE.md) — 生态 backlog
- [GRAMMAR_REFERENCE.md](GRAMMAR_REFERENCE.md) — 语法参考
- [PUBLISH_SECRETS.md](PUBLISH_SECRETS.md) — 发布密钥配置
- [SECURITY.md](../SECURITY.md) — 安全披露

跟踪 issue：[#16 secrets](https://github.com/A-LPG/LPG2/issues/16) · [#15 首次发包](https://github.com/A-LPG/LPG2/issues/15) · [#14 calculator 扩展](https://github.com/A-LPG/LPG2/issues/14)
