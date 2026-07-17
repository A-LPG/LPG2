# LPG2 源码 TODO 分级（2026-07）

Issues：#7–#13 closed。VS Code 扩展 0.0.18 已发 GitHub Release（Marketplace 需有效 `VSCE_PAT`）。

建议标签：`dead-code`、`needs-fix`、`keep`、`good-first-issue`。

## A. 删除死代码 / 清理注释

| 位置 | 摘要 | 状态 |
|------|------|------|
| 死注释 / Old bad idea / stubs | 已清理或删除 | done |
| `CppAction` / `cpp` 后端 | `cpp`/`c++`/`rt_cpp` → `CPP2` | done (N2) |
| `sp` / `diagnose` / `pda` / `scanner` 遗留 TODO | 改写成设计笔记或删除死注释 | done (N4) |

## B. 需修复 / 审查

| 位置 | 摘要 | 状态 |
|------|------|------|
| `resolve.cpp` priority | 已审计 | done (#10) |
| `grammar.cpp` alias worklist | 复核：jikespg.g ~0.022s，远低于 0.150 软阈值 | done (#11) |
| `optionDesc` / include 路径 | 缺失目录现报错并 exit 12 | done (#12) |
| recover / prosthetic AST | 全部后端完成闭环含 `$allocation`（`%Recover Sym /. expr ./`）：Java、C++、Rust、Go、C#、TypeScript、Dart、Python，各自 `*_automatic_ast_recover` e2e 覆盖 | done (all backends) |
| `base.cpp` ComputeRank | 有意保留 | done |
| unused 警告收紧 | 去掉 GCC `-Wno-unused-variable` / `-Wno-unused-but-set-variable`，改为 `-Werror=` | done (N4) |

## C. 保留注释（keep）

| 位置 | 原因 |
|------|------|
| `base.cpp:609` | DEPRECATED 路径警示 |

## GitHub Issues（首批）

1. #7–#13 — closed
