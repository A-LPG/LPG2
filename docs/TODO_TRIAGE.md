# LPG2 源码 TODO 分级（2026-07）

共约 53 处 `TODO`/`FIXME`（清理死注释后减少）。Issues 已建：#8–#13。

建议标签：`dead-code`、`needs-fix`、`keep`、`good-first-issue`。

## A. 删除死代码 / 清理注释（good-first-issue）

| 位置 | 摘要 | 状态 |
|------|------|------|
| `Action.cpp` list AST 旧逻辑注释 | 已删除 | done (#8) |
| `produce.cpp` `TODO(1): REMOVE THIS!` | 已删除 | done (#8) |
| `jikespg.cpp` Dump TODO 文案 | 已清理（仍 `#ifdef TEST`） | done (#8) |
| `generator.cpp` prostheses 调试 / 过时注释 | 已删除 | done (#8) |
| `*Action.cpp` “Old bad idea” 注释块 | 已删除 | done (#9) |
| Stub backends C/ML/Plx/Plxasm/Xml | 见 #13；P3 删除源码 | open |
| `CppAction.cpp` | AST stub；`cpp` 表仍用 `CppTable`；不随 stub 语言一并删除 | keep for now |

## B. 需修复 / 审查（needs-fix）

| 位置 | 摘要 | 优先级 | Issue |
|------|------|--------|-------|
| `resolve.cpp` ~364–500 | 冲突优先级路径行为审计（注释已重写，非旧 “TAKE A HARD LOOK”） | 高 | #10 |
| `grammar.cpp` alias / worklist | 2.3.0 已部分 worklist 化；复核剩余热点 | 中 | #11 |
| `grammar.cpp` ~1456+ | recover / prosthetic AST 生成缺口 | 中 | — |
| `base.cpp:1132` | 临时算法需更深分析 | 中 | — |
| `sp.cpp:147` | `REVIEW THIS` | 低 | — |
| `optionDesc.cpp:388` | 验证路径是否存在 | 低 | #12 |
| `diagnose.cpp:1996,2095` | 「不应再是 option」 | 低 | — |
| `pda.cpp:603` | 「Do I need this?」 | 低 | — |
| `scanner.cpp:1322,1507` | 遗留测试注释 | 低 | — |

## C. 保留注释（keep）

| 位置 | 原因 |
|------|------|
| `base.cpp:609` | 明确标记 DEPRECATED 路径，保留警示 |
| 各后端 AST 分配中的活逻辑旁注 | 行为依赖现有生成；仅清理已注释死代码 |

## GitHub Issues（首批）

1. #8 Remove dead TODO comment blocks in Action/produce/generator — **closed by cleanup**
2. #9 Remove "Old bad idea" commented-out code across `*Action.cpp` — **closed by cleanup**
3. #10 Audit `resolve.cpp` priority conflict tracing
4. #11 Review alias nonterminal closure performance
5. #12 Validate option paths exist (`optionDesc.cpp`)
6. #13 Remove stub backends C/ML/Plx/Plxasm/Xml
