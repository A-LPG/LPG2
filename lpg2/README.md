# lpg2 — 生成器源码

本目录包含 LPG2 生成器（`lpg-v2.2.03`）的 C++ 实现与自举语法 `grammar/jikespg.g`。

## 文档入口

| 角色 | 文档 |
|------|------|
| 使用生成器写语法 | [用户文档](../docs/USER.md) |
| 维护 / 扩展生成器 | [开发者文档](../docs/DEVELOPER.md) |
| 自举重新生成流程 | [BOOTSTRAP.md](BOOTSTRAP.md) |

## 快速构建（开发者）

```bash
cmake -S . -B build
cmake --build build -j
ctest --test-dir build --output-on-failure
```
