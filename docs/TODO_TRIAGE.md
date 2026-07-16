# LPG2 源码 TODO 分级（2026-07）

Issues：#8–#10、#12–#13 closed；#11 closed（alias worklist 足够快，无进一步热点）。

建议标签：`dead-code`、`needs-fix`、`keep`、`good-first-issue`。

## A. 删除死代码 / 清理注释

| 位置 | 摘要 | 状态 |
|------|------|------|
| 死注释 / Old bad idea / stubs | 已清理或删除 | done |
| `CppAction.cpp` | 见 N2：`cpp` 将别名到 `CPP2` | in progress |

## B. 需修复 / 审查

| 位置 | 摘要 | 状态 |
|------|------|------|
| `resolve.cpp` priority | 已审计 | done (#10) |
| `grammar.cpp` alias worklist | 复核：jikespg.g ~0.022s，远低于 0.150 软阈值；嵌套扫描可接受 | done (#11) |
| `optionDesc` / include 路径 | 缺失目录现报错并 exit 12 | done (#12) |
| recover / prosthetic AST | 有意推迟 | deferred |
| `base.cpp` ComputeRank | 有意保留 | done |
| `sp` / `diagnose` / `pda` / `scanner` | 低优先级遗留注释 | open (N4) |

## C. 保留注释（keep）

| 位置 | 原因 |
|------|------|
| `base.cpp:609` | DEPRECATED 路径警示 |

## GitHub Issues（首批）

1. #8–#10, #12–#13 — closed
2. #11 alias performance — closed（无优化必要）
3. #7 install layout — closed（USER 安装树文档）
