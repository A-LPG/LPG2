# LPG2 生态 backlog（2026-07）

历史源码 TODO（#7–#13、死代码清理、八后端 recover 实现）已关闭。本文件跟踪**生态健康**后续项。建议标签：`good-first-issue`、`docs`、`ci`、`runtime`、`tooling`。

机器可读兼容矩阵：[ECOSYSTEM.md](ECOSYSTEM.md) / [ecosystem/compat.json](../ecosystem/compat.json)。

## A. CI / 可信度

| 项 | 摘要 | 标签 | 状态 |
|----|------|------|------|
| recover 进 CI | 八后端 `*_automatic_ast_recover` 与 nested 同级必跑 | `ci` | done |
| compat 清单 | `ECOSYSTEM.md` + `compat.json` | `docs` | done |
| Python 2 removed | 生成器不再注册/编译；CLI 拒绝；compat/docs 同步 | `generator` | done |
| 运行时独立 CI | 各 runtime 仓最小 build/test workflow | `ci` `runtime` | done |
| scheduled head 探测 | `runtime-head-probe.yml` 对 origin/HEAD 漂移告警 | `ci` | done |

## B. 工具链

| 项 | 摘要 | 标签 | 状态 |
|----|------|------|------|
| VS Code 扩展 CI | `tool-ci.yml`：编译、assemble、VSIX | `ci` `tooling` | done |
| Language Server CI | `tool-ci.yml`：macOS/Ubuntu 构建 | `ci` `tooling` | done |
| VSIX Release | `vscode-release.yml`（tag `vscode-v*`） | `tooling` | done |
| Marketplace 发布 | 配置有效 `VSCE_PAT` 后由 release workflow 推送 | `tooling` | open |
| `dev-bootstrap.sh` | 检测工具链、按需 submodule、报告跳过的 e2e | `good-first-issue` `tooling` | done |
| 扩展真实测试 | `extension.test.ts` 激活/语言贡献冒烟 | `tooling` | done |

## C. 文档 / 上手

| 项 | 摘要 | 标签 | 状态 |
|----|------|------|------|
| 可运行 calculator | 八后端 drivers + CI（#14） | `docs` | done |
| recover cookbook | Java `%Recover` 示例 + `$allocation` 决策树 | `docs` | done |
| GRAMMAR_REFERENCE | 指令、动作、AST、recover、CLI | `docs` | done |
| runtime README 模板 | 统一章节；各 runtime README 已对齐 | `docs` `runtime` | done |
| EN tutorial | `docs/en/tutorial.md` + calculator README | `docs` | done |
| 文档漂移 | EN DEVELOPER warnings、CHANGELOG stub、安装包 EN/ECOSYSTEM | `docs` | done |
| 初学者路径 | QUICKSTART / CONCEPTS / 扩展 tutorial（中英）+ 导航 + FAQ + `check-doc-links.sh` | `docs` | done |
| AI / Agent 手册 | `docs/AI.md` + `docs/en/AI.md` + `AGENTS.md` + `.cursor/skills/lpg2` | `docs` | done |

## D. 发布 / 治理

| 项 | 摘要 | 标签 | 状态 |
|----|------|------|------|
| Release Checklist | ECOSYSTEM.md + `scripts/release-checklist.sh` | `docs` | done |
| 包发布自动化 | npm / NuGet / Cargo publish workflows（需 secrets） | `runtime` | done |
| 过时 toolchain | C# → netstandard2.0;net8.0；Dart SDK `<4`；Java 1.9.0 / Java 8 | `runtime` | done |
| SECURITY.md | 安全披露流程 | `docs` | done |
| Publish secrets | `docs/PUBLISH_SECRETS.md` + `scripts/setup-publish-secrets.sh` | `tooling` | open — scripts ready; awaiting operator PATs |

## 仍开放（需密钥取值）

**Phase 4 不能标 done**：文档与 `setup-publish-secrets.sh` 已就绪，但 **尚未** 用真实 PAT 写入各仓 Actions secrets。勿因脚本存在而降级为「已完成」。

| Secret | 目标仓 | 用途 |
|--------|--------|------|
| `VSCE_PAT` | `A-LPG/LPG2`, `A-LPG/LPG-VScode` | Marketplace（`vscode-release.yml`） |
| `NPM_TOKEN` | `A-LPG/LPG-typescript-runtime` | `npm publish` |
| `NUGET_API_KEY` | `A-LPG/LPG-csharp-runtime` | `dotnet nuget push` |
| `CARGO_REGISTRY_TOKEN` | `A-LPG/LPG-rust-runtime` | `cargo publish` |
| `OSSRH_USERNAME` / `OSSRH_TOKEN` | `A-LPG/lpg-runtime` | Maven deploy |

操作员步骤（取值到位后）：

1. `gh auth login -h github.com -p https -s repo,workflow`
2. `./scripts/setup-publish-secrets.sh` 或 `--from-env`（见 [PUBLISH_SECRETS.md](PUBLISH_SECRETS.md)）
3. `gh secret list --repo <各仓>` 确认非空后再关 [#16](https://github.com/A-LPG/LPG2/issues/16)

Tracking：

- [#16](https://github.com/A-LPG/LPG2/issues/16) 配置 publish secrets — **仍 open**
- [#15](https://github.com/A-LPG/LPG2/issues/15) 首次 crates.io / 确认 npm+NuGet — 依赖上表 secrets
- [#14](https://github.com/A-LPG/LPG2/issues/14) calculator 八后端 — **done**

已完成的跟进（与 secrets 无关）：子模块 CI/README/toolchain；父仓 submodule 指针；calculator 八后端 quickstart；workflows 与 setup 脚本。

## E. 语法语料（grammars-v4 → grammars-example）

**硬禁令（不可协商）：不允许收缩，不允许降级。**

- 禁止为过 CI 砍薄 `*Parser.g`、玩具化例子、或 thin subset 冒充完工
- 禁止改 `quality` 降档、关 `parse_ok` 骗绿
- 允许：KW/lexer 终端名对齐、`checkForKeyWord()` 接线、字面量别名、**加宽**规则与例子、从收缩前 / grammars-v4 **恢复或扩展**结构

| 项 | 摘要 | 标签 | 状态 |
|----|------|------|------|
| catalog + tiers | `grammars-example/catalog.json` + `tools/build_catalog.py` | `docs` `tooling` | done |
| Java parse harness | `grammars-example/harness/run-one.sh` + CI `grammars-example.yml` | `ci` `tooling` | done |
| Wave A（小语法可解析） | tier A 全量 `parse_ok` | `docs` | done |
| Wave B | tier B 全量 `parse_ok` | `docs` | done |
| **043a1d6 收缩债** | `rust`/`z`/`vaxscan`/`tinyos_nesc`/`turing`/`wren`/`tnsnames`/`terraform`/`turtle`：已恢复结构并以 KW/lexer 对齐 + 必要左因子修绿（禁止再缩 / 关 `parse_ok`） | `docs` | done |
| Wave C/D | 主流语言 / SQL·mode：`java/java`=`language_port`；`java/java8|9|20`、`javascript/javascript|typescript` 已加宽且 `parse_ok`（仍为 `language_subset`，未冒充 full port）；SQL·mode 与其余主流语料仍 incomplete | `docs` | open (P0 widen done; ports incomplete) |

进度与质量分级：`cd grammars-example && python3 tools/classify_quality.py && python3 tools/report.py`。见 `catalog.json` → `quality_schema`；贡献说明见子模块 `CONTRIBUTING.md`。CI 必跑门只含 `language_port` + `language_subset`。

## F. GLR

| 项 | 摘要 | 标签 | 状态 |
|----|------|------|------|
| Java GLR v1 | `-glr` 表标志 + `glrParserTemplateF.gi` + runtime `GLRParser`（symbol-aware 配置分叉/合并 + 同语法符号同 token span 的 `nextAst` 森林）；Catalan、相关性、RR/nullable、entry、循环拒绝、非 AST 行为测试 | `runtime` `generator` | done (Java) |
| compat / glr bit | `ecosystem/compat.json` → `features.glr` + 每后端 `glr` 字段；`ECOSYSTEM.md` 矩阵列 | `docs` | done |
| C++ GLR 驱动 | `isGLR` 表标志 + `rt_cpp/glrParserTemplateF.gi` + runtime `GLRParser` + Catalan e2e | `runtime` | done |
| GLR Recover | `error_repair_count>0` 时 GLR 失败回退 `BacktrackingParser.fuzzyParse*`（`%Recover` 义肢）；模板挂 Diagnose；`java_glr_recover_e2e` / `cpp_glr_recover_e2e` | `runtime` | done (Java/C++) |
| 其它后端 GLR 驱动 | 表编码与 AST `nextAst` 脚手架已就绪；Go/Rust/Python3/Dart runtime 驱动仍待落地 | `runtime` | open (TS/C# done) |
| TypeScript GLR 驱动 | `isGLR` 表标志 + `templates/typescript/glrParserTemplateF.gi` + runtime `GLRParser`（GSS/SPPF + `nextAst` 投影） | `runtime` | done |
| C# GLR 驱动 | `isGLR` 表标志 + `templates/csharp/glrParserTemplateF.gi` + runtime `GLRParser`（GSS/SPPF + `nextAst` 投影）+ `csharp_glr_ambiguous_e2e` Catalan | `runtime` | done |
| Playground WASM GLR | `-glr` 生成（TS/Java/C++）+ 浏览器内 TS `GLRParser` 森林 demo（`sample-glr.g` / `glr-demo.bundle.js`） | `tooling` | done |
| GLR SPPF (v2) | GSS 前缀共享 + 共享包解析森林（`GssNode`/`SppfNode`）；`getSppfRoot()` / `getSppfSymbolCount()`；`nextAst` 为兼容投影；Java/C++/TS/C# | `runtime` | done (Java/C++/TS/C#) |

## 不做（本阶段）

- 新目标语言后端
- 生成器大规模重写 / toplevel AST 全量对等
- monorepo 迁移
- 独立营销网站
