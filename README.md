# LPG2

[![VS Code Marketplace version](https://vsmarketplacebadge.apphb.com/version-short/kuafuwang.lpg-vscode.svg)](https://marketplace.visualstudio.com/items?itemName=kuafuwang.lpg-vscode)

LPG2（Lookahead Parser Generator v2）将 `.g` / `.lpg` 语法编译为目标语言的解析表与语义动作代码。

**当前版本：2.3.0**（`lpg-v2.3.0`）

## 文档

文档按读者角色分层；**第一次用 LPG 请从上手文档开始**：

| 文档 | 面向谁 | 内容 |
| --- | --- | --- |
| [**5 分钟上手**](docs/QUICKSTART.md) | 新手 | 下载/构建生成器 → 跑通 calculator |
| [**概念模型**](docs/CONCEPTS.md) | 新手 | 生成器 / 模板 / 运行时如何协作 |
| [**入门教程**](docs/tutorial.md) | 新手 | 计算器语法分步讲解（四语言） |
| [**用户文档**](docs/USER.md) | 语法作者、解析器集成者 | 获取工具、命令行、语言、运行时、FAQ |
| [**开发者文档**](docs/DEVELOPER.md) | 生成器维护者 | 源码构建、代码结构、自举、测试、后端扩展 |
| [**English docs**](docs/en/README.md) | English readers | Quick Start / Concepts / Tutorial / USER |
| [**贡献指南**](CONTRIBUTING.md) | 贡献者 | 构建、golden、PR 流程 |
| [**文档索引**](docs/README.md) | — | 按角色选择阅读路径 |
| [**生态兼容**](docs/ECOSYSTEM.md) | 集成者 / 发版 | 运行时版本、包坐标、Release Checklist |

专题：

- [语法参考](docs/GRAMMAR_REFERENCE.md) — 指令、动作、AST、recover、CLI
- [自举策略](lpg2/BOOTSTRAP.md) — 重新生成 `jikespg_*` 解析器的审查流程（开发者）
- 机器可读兼容清单：[ecosystem/compat.json](ecosystem/compat.json)

## 仓库一览

| 路径 | 说明 |
| --- | --- |
| [`lpg2/`](lpg2/) | 生成器可执行程序源码 |
| [`runtime/`](runtime/) | 各语言运行时（git 子模块） |
| [`tool/`](tool/) | VS Code 扩展与语言服务（子模块） |
| [`grammars-example/`](grammars-example/) | 示例语法（子模块） |
| [`examples/calculator/`](examples/calculator/) | 可运行计算器入门示例 |
| [`docs/`](docs/) | 用户 / 开发者文档 |

## 快速体验

**最快：npm**

```bash
npx lpg2 --help
npx lpg2 -programming_language=java -table your.g
```

浏览器 playground（WASM，由 CI 构建）：见 [`playground/`](playground/)。

C++ runtime 提供 **token 级增量重词法 + 语句级增量重解析**（不是 tree-sitter 子树复用）；说明见 [USER.md](docs/USER.md) / [ECOSYSTEM.md](docs/ECOSYSTEM.md)。

**推荐：Release 二进制 + calculator**

1. 从 [GitHub Releases](https://github.com/A-LPG/LPG2/releases) 下载平台包并校验 `SHA256SUMS`
2. 初始化运行时子模块（按语言选，例如 Java）：

```bash
git submodule update --init runtime/lpg-runtime
export LPG_BIN=/path/to/bin/lpg-v2.3.0
./examples/calculator/scripts/run.sh java
```

完整步骤见 [docs/QUICKSTART.md](docs/QUICKSTART.md)。

**或者从源码构建生成器：**

```bash
cd lpg2
cmake -S . -B build && cmake --build build -j
export LPG_BIN="$PWD/build/lpg-v2.3.0"
cd ..
./examples/calculator/scripts/run.sh java
```

集成与命令行细节见 [用户文档](docs/USER.md)。

## 相关项目

- [LPG-rust-runtime](https://github.com/A-LPG/LPG-rust-runtime) — Rust 解析运行时
- [LPG2-grammars-example](https://github.com/A-LPG/LPG2-grammars-example) — 语法示例集合
- [LPG-VScode](https://github.com/A-LPG/LPG-VScode) — VS Code 扩展
