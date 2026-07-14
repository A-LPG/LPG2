# LPG2

[![](https://vsmarketplacebadge.apphb.com/version-short/kuafuwang.lpg-vscode.svg)](https://marketplace.visualstudio.com/items?itemName=kuafuwang.lpg-vscode)

LPG2（Lookahead Parser Generator v2）将 `.g` / `.lpg` 语法编译为目标语言的解析表与语义动作代码。

**当前版本：2.2.03**（`lpg-v2.2.03`）

## 文档

文档按读者角色分为两层：

| 文档 | 面向谁 | 内容 |
|------|--------|------|
| [**用户文档**](docs/USER.md) | 语法作者、解析器集成者 | 获取工具、命令行用法、支持语言、运行时、Rust 集成、示例 |
| [**开发者文档**](docs/DEVELOPER.md) | 生成器维护者 | 源码构建、代码结构、自举、测试、后端扩展、子模块 |
| [**文档索引**](docs/README.md) | — | 按角色选择阅读路径 |

专题：

- [自举策略](lpg2/BOOTSTRAP.md) — 重新生成 `jikespg_*` 解析器的审查流程（开发者）

## 仓库一览

| 路径 | 说明 |
|------|------|
| [`lpg2/`](lpg2/) | 生成器可执行程序源码 |
| [`runtime/`](runtime/) | 各语言运行时（git 子模块） |
| [`tool/`](tool/) | VS Code 扩展与语言服务（子模块） |
| [`grammars-example/`](grammars-example/) | 示例语法（子模块） |
| [`docs/`](docs/) | 用户 / 开发者文档 |

## 快速体验

```bash
cd lpg2
cmake -S . -B build && cmake --build build -j
./build/lpg-v2.2.03 -programming_language=cpp -table \
  -out_directory=./out \
  ../grammars-example/your_grammar.g
```

详细步骤见 [用户文档](docs/USER.md)。

## 相关项目

- [LPG-rust-runtime](https://github.com/kuafuwang/LPG-rust-runtime) — Rust 解析运行时
- [LPG2-grammars-example](https://github.com/kuafuwang/LPG2-grammars-example) — 语法示例集合
- [lpg-vscode](https://github.com/kuafuwang/lpg-vscode) — VS Code 扩展
