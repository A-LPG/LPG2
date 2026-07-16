# LPG2 源码 TODO 分级（2026-07）

Issues：#8–#9 closed；#10 audited；#11 open；#12 open；#13 closed（stubs removed）。

建议标签：`dead-code`、`needs-fix`、`keep`、`good-first-issue`。

## A. 删除死代码 / 清理注释

| 位置 | 摘要 | 状态 |
|------|------|------|
| `Action.cpp` / `produce.cpp` / `generator.cpp` / `jikespg.cpp` | 死注释清理 | done (#8) |
| `*Action.cpp` “Old bad idea” | 死注释清理 | done (#9) |
| Stub backends C/ML/Plx/Plxasm/Xml | 源码与 CLI 已删除；默认语言 `java` | done (#13) |
| `CppAction.cpp` | AST stub；`cpp` 表仍用 `CppTable`；保留 | keep |

## B. 需修复 / 审查

| 位置 | 摘要 | 优先级 | Issue / 状态 |
|------|------|--------|--------------|
| `resolve.cpp` priority helpers | 审计：三者一致；由 conflict_* 间接覆盖 | 高 | #10 done |
| `grammar.cpp` alias / worklist | 2.3.0 worklist 化；剩余热点可选 | 中 | #11 open |
| `grammar.cpp` recover / prosthetic AST | 设计笔记：有意推迟 | 中 | deferred |
| `base.cpp` ComputeRank | 审计：现算法有意保留；更深排序可选 | 中 | done |
| `sp.cpp:147` | `REVIEW THIS` | 低 | — |
| `optionDesc.cpp:388` | 验证路径是否存在 | 低 | #12 |
| `diagnose.cpp` / `pda.cpp` / `scanner.cpp` | 低优先级遗留注释 | 低 | — |

## C. 保留注释（keep）

| 位置 | 原因 |
|------|------|
| `base.cpp:609` | DEPRECATED 路径警示 |
| 各后端 AST 分配旁注 | 行为依赖现有生成 |

## GitHub Issues（首批）

1. #8 dead TODO blocks — closed
2. #9 Old bad idea blocks — closed
3. #10 resolve.cpp audit — closed (comment audit + conflict fixtures)
4. #11 alias closure performance — open
5. #12 Validate option paths — open
6. #13 Remove stub backends — closed
