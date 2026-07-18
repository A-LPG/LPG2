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
| Publish secrets | `docs/PUBLISH_SECRETS.md` + `scripts/setup-publish-secrets.sh` | `tooling` | open (needs PAT values) |

## 仍开放（需密钥取值）

Workflows 与脚本已就绪；各仓目前 **尚无** Actions secrets（`gh secret list` 为空）。

1. 准备 PAT 后执行：`./scripts/setup-publish-secrets.sh`（见 [PUBLISH_SECRETS.md](PUBLISH_SECRETS.md)）
2. Tracking issues：
   - [#16](https://github.com/A-LPG/LPG2/issues/16) 配置 publish secrets
   - [#15](https://github.com/A-LPG/LPG2/issues/15) 首次 crates.io / 确认 npm+NuGet
   - [#14](https://github.com/A-LPG/LPG2/issues/14) calculator 八后端 — **done**（Go/Python/C#/Dart 已齐）

已完成的跟进：子模块 CI/README/toolchain 已推送；父仓 submodule 指针已 bump；calculator 八后端 quickstart 已落地。

## E. 语法语料（grammars-v4 → grammars-example）

| 项 | 摘要 | 标签 | 状态 |
|----|------|------|------|
| catalog + tiers | `grammars-example/catalog.json` + `tools/build_catalog.py` | `docs` `tooling` | done |
| Java parse harness | `grammars-example/harness/run-one.sh` + CI `grammars-example.yml` | `ci` `tooling` | done |
| Wave A（小语法可解析） | tier A 全量 `parse_ok` | `docs` | done |
| Wave B | tier B 全量 `parse_ok` | `docs` | done |
| Wave C/D | 主流语言 / SQL·mode：**scaffold + smoke done；`language_port` incomplete**（多数仍为 `token_stream_smoke`；`java/java` 已为 `language_port`，`java/java8` 为 `language_subset`） | `docs` | open (smoke done; ports incomplete) |

进度与质量分级：`cd grammars-example && python3 tools/classify_quality.py && python3 tools/report.py`。见 `catalog.json` → `quality_schema`；贡献说明见子模块 `CONTRIBUTING.md`。CI 必跑门只含 `language_port` + `language_subset`。

## F. GLR

| 项 | 摘要 | 标签 | 状态 |
|----|------|------|------|
| Java GLR v1 | `-glr` 表标志 + `glrParserTemplateF.gi` + runtime `GLRParser`（symbol-aware 配置分叉/合并 + 同语法符号同 token span 的 `nextAst` 森林）；Catalan、相关性、RR/nullable、entry、循环拒绝、非 AST 行为测试 | `runtime` `generator` | done (Java) |
| compat / glr bit | `ecosystem/compat.json` → `features.glr` + 每后端 `glr` 字段；`ECOSYSTEM.md` 矩阵列 | `docs` | done |
| C++ GLR 驱动 | `isGLR` 表标志 + `rt_cpp/glrParserTemplateF.gi` + runtime `GLRParser` + Catalan e2e | `runtime` | done |
| GLR Recover | `error_repair_count>0` 时 GLR 失败回退 `BacktrackingParser.fuzzyParse*`（`%Recover` 义肢）；模板挂 Diagnose；`java_glr_recover_e2e` / `cpp_glr_recover_e2e` | `runtime` | done (Java/C++) |
| 其它后端 GLR 驱动 | 表编码与 AST `nextAst` 脚手架已就绪；Go/Rust/TS/… runtime 驱动 | `runtime` | open |
| GLR SPPF (v2) | GSS 前缀共享 + 共享包解析森林（`GssNode`/`SppfNode`）；`getSppfRoot()` / `getSppfSymbolCount()`；`nextAst` 为兼容投影；Java/C++ e2e（含 SPPF 共享断言） | `runtime` | done (Java/C++) |

## 不做（本阶段）

- 新目标语言后端
- 生成器大规模重写 / toplevel AST 全量对等
- monorepo 迁移
- 独立营销网站
