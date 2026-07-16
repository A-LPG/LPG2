# LPG2 源码 TODO 分级（2026-07）

共约 53 处 `TODO`/`FIXME`。未登录 `gh` 时无法自动建 issue；本文件按类别列出，
可直接复制到 GitHub Issues。建议标签：`dead-code`、`needs-fix`、`keep`、`good-first-issue`。

## A. 删除死代码 / 清理注释（good-first-issue）

| 位置 | 摘要 | 建议 |
|------|------|------|
| `Action.cpp:886,908` | 已注释掉的 list AST 旧逻辑 | 删除注释块 |
| `produce.cpp` ×4 | `TODO(1): REMOVE THIS!` | 确认无引用后删除 |
| `jikespg.cpp:61` | `lex_stream.Dump()` 调试 | 删除或 `#ifdef DEBUG` |
| `generator.cpp:222,1944,2562` | 调试 / 过时注释 | 删除 |
| `*Action.cpp`（多语言） | `Old bad idea` / GC 提醒注释块 | 删除死注释；活代码上提基类 |
| `C/Ml/Plx/Plxasm/Xml*Action.cpp` | stub preprocess/postprocess TODO | 随桩后端 deprecate 一并处理 |
| `CppAction.cpp:9,15` | 旧 cpp 路径 stub TODO | 确认是否仍入口；否则 deprecate |

## B. 需修复 / 审查（needs-fix）

| 位置 | 摘要 | 优先级 |
|------|------|--------|
| `resolve.cpp:364,413,453` | 冲突优先级路径「TAKE A HARD LOOK」 | 高；阶段 1 冲突报告时复核 |
| `grammar.cpp:454` | Alias 偏序迭代效率 | 中；阶段 3 性能 |
| `grammar.cpp:1448,1451` | AST abstract 声明生成缺口 | 中 |
| `base.cpp:1132` | 临时算法需更深分析 | 中 |
| `sp.cpp:147` | `REVIEW THIS` | 低 |
| `optionDesc.cpp:388` | 验证路径是否存在 | 低；good-first-issue |
| `diagnose.cpp:1996,2095` | 「不应再是 option」 | 低 |
| `pda.cpp:603` | 「Do I need this?」 | 低 |
| `scanner.cpp:1322,1507` | 遗留测试注释 | 低 |

## C. 保留注释（keep）

| 位置 | 原因 |
|------|------|
| `base.cpp:609` | 明确标记 DEPRECATED 路径，保留警示 |
| 各后端 AST 分配中的活逻辑旁注 | 行为依赖现有生成；仅清理已注释死代码 |

## 建议的首批 GitHub Issues

1. **[good-first-issue]** Remove dead TODO comment blocks in `Action.cpp` / `produce.cpp` / `generator.cpp`
2. **[good-first-issue]** Remove duplicated "Old bad idea" commented-out code across `*Action.cpp`
3. **[needs-fix]** Audit `resolve.cpp` priority conflict tracing (lines ~364–500)
4. **[needs-fix]** Speed up alias nonterminal closure in `grammar.cpp` (~454)
5. **[good-first-issue]** Validate option paths exist (`optionDesc.cpp:388`)
6. **[needs-fix]** Deprecate stub backends C/ML/Plx/Plxasm/Xml

创建后：`gh issue create --title "..." --body-file ... --label good-first-issue`
