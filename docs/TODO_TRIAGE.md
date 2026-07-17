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

## 不做（本阶段）

- 新目标语言后端
- 生成器大规模重写 / GLR / toplevel AST 全量对等
- monorepo 迁移
- 独立营销网站
