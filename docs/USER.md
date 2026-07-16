# LPG2 用户文档

面向**使用 LPG2 编写语法、生成解析器**的读者。若你要修改生成器本身，请参阅 [开发者文档](DEVELOPER.md)。

## LPG2 是什么

LPG2（Lookahead Parser Generator v2）读取 `.g` / `.lpg` 语法文件，生成目标语言的**解析表**和**语义动作代码**。支持 LALR 解析、回溯消歧、语法继承与 AST 生成。

当前版本：**2.3.0**（可执行文件名 `lpg-v2.3.0`）。

## 获取生成器

### 方式一：GitHub Release（推荐）

从 [GitHub Releases](https://github.com/A-LPG/LPG2/releases) 下载
Linux、macOS 或 Windows 压缩包并校验
`SHA256SUMS`。解压后的 `bin/` 包含生成器，`share/lpg2/` 包含各语言模板。

压缩包保持原目录结构时，生成器会自动发现对应语言的模板。若单独移动二进制，
可用环境变量覆盖搜索路径。例如 Rust：

```bash
export LPG_TEMPLATE="/path/to/templates/rust"
export LPG_INCLUDE="/path/to/include/rust"
```

### 方式二：VS Code 扩展

从 Marketplace 安装 [lpg-vscode](https://marketplace.visualstudio.com/items?itemName=kuafuwang.lpg-vscode) 的**发布包**后，扩展会自带模板、语言服务与生成器二进制，可直接在编辑器中编写语法并触发生成。

本仓库中的扩展源码位于 `tool/LPG-VScode/`（子模块）。干净克隆不会包含已装配的 `templates/` 与 `server/`（见 `.gitignore`）；本地联调或打 VSIX 前请运行 `tool/LPG-VScode/scripts/assemble-release.sh`（见 [开发者文档](DEVELOPER.md)）。

### 方式三：从源码编译

```bash
cd lpg2
cmake -S . -B build
cmake --build build -j
cmake --install build --prefix ./install
./install/bin/lpg-v2.3.0 --help
```

### 安装树布局

`cmake --install` / CPack 包解压后典型结构：

```text
prefix/
├── bin/
│   └── lpg-v2.3.0              # 生成器可执行文件
├── share/lpg2/
│   └── lpg-generator-templates-2.1.00/
│       ├── templates/           # 各语言模板（java、rt_cpp、rust、…）
│       └── include/             # 对应 include 片段
└── share/doc/lpg2/              # 或 doc/（依平台 CMAKE_INSTALL_DOCDIR）
    ├── README.md
    ├── LICENSE
    └── USER.md
```

Release 压缩包使用相同布局：`bin/` + `share/lpg2/…`。生成器会相对自身位置自动发现模板；若单独移动二进制，请设置 `LPG_TEMPLATE` / `LPG_INCLUDE`（见上）。

## 基本工作流

1. 编写或修改 `.g` / `.lpg` 语法文件
2. 选择目标语言，运行生成器
3. 将生成的解析表与动作代码集成到对应语言的**运行时库**中

典型命令：

```bash
lpg-v2.3.0 -programming_language=cpp -table \
  -out_directory=./out \
  path/to/grammar.g
```

### 常用命令行参数

| 参数 | 说明 |
|------|------|
| `-programming_language=` | 目标语言（见下表） |
| `-table` | 生成解析表 |
| `-out_directory=` | 输出目录 |
| `-quiet` | 减少控制台输出 |
| `-nowrite` | 仅分析语法，不写文件 |

完整参数列表：`lpg-v2.3.0 -help`

无参数、`-h`、`-help`、`--help` 和 `--version` 都成功返回 0；语法或选项错误
稳定返回 12，并把错误诊断写入 stderr。生成文件采用事务式发布：失败不会覆盖已有
产物，也不会留下半成品。`-out_directory` 同时控制解析表、动作文件和 `.l`
listing 文件的位置。

## 支持的目标语言

| 语言 | 参数值 | 状态 |
|------|--------|------|
| C++ | `cpp` / `rt_cpp` | 完整支持；`rt_cpp` 用于链接 `LPG-cpp-runtime` 的 parser / automatic AST |
| Java | `java` | 生成完整支持；CI 以生成烟雾为主（compile-run 覆盖见开发者文档，当前以 C++/Rust 为主） |
| C# | `csharp` | 生成完整支持（同上） |
| Go | `go` | 生成完整支持（同上） |
| Python 2 | `python2` | 生成完整支持（同上） |
| Python 3 | `python3` | 生成完整支持（同上） |
| TypeScript | `typescript` | 生成完整支持（同上） |
| Dart | `dart` | 生成完整支持（同上） |
| Rust | `rust` | 解析表、确定性/回溯 parser；automatic AST 已覆盖 `nested`（含无 `parent_saved` 的 `get_children`）、list、`parent_saved`、`needs_environment`、interface/`dyn` RHS 恢复、`visitor=default` / `visitor=preorder`（行为测试见 `rust_automatic_ast_*_behavior`）。复杂语法仍建议小步验证，不宣称与 Java/C++ 全量对等（不含 `toplevel`/GLR 全量） |

> **迁移说明：** 旧桩后端 `c` / `ml` / `plx` / `plxasm` / `xml` 已移除。请改用 `java`、`cpp`、`rt_cpp` 或其他完整后端。

## 运行时库

生成器只产出解析表与动作代码；实际解析需要各语言的**运行时**。本仓库 `runtime/` 目录包含对应子模块：

| 目录 | 语言 |
|------|------|
| `runtime/LPG-cpp-runtime` | C++ |
| `runtime/LPG-csharp-runtime` | C# |
| `runtime/LPG-go-runtime` | Go |
| `runtime/LPG-python-runtime` | Python |
| `runtime/LPG-typescript-runtime` | TypeScript |
| `runtime/LPG-Dart-runtime` | Dart |
| `runtime/lpg-runtime` | Java |

**Rust** 运行时在独立仓库 [LPG-rust-runtime](https://github.com/A-LPG/LPG-rust-runtime)，不在本仓库子模块中。

克隆含子模块的完整仓库：

```bash
git clone --recursive https://github.com/A-LPG/LPG2.git
```

## 示例语法

- 本仓库：`grammars-example/`（子模块）
- 独立集合：[LPG2-grammars-example](https://github.com/A-LPG/LPG2-grammars-example)

入门模板与更详细的 LPG 语法说明见 `lpg-generator-templates-2.1.00/docs/`。

## Rust 项目集成

1. 添加 [LPG-rust-runtime](https://github.com/A-LPG/LPG-rust-runtime) 依赖
2. 用生成器产出 `*prs.rs` 与 `*sym.rs`：

```bash
lpg-v2.3.0 -programming_language=rust -table \
  -out_directory=src/generated \
  my_grammar.g
```

3. 在项目中 `mod` 引入生成文件，使用运行时提供的 `ParseTable` trait 驱动解析

生成文件需与 `LPG-rust-runtime` 版本匹配；升级任一侧时请重新生成并测试。

Rust 的 parser/table、backtracking 与 automatic AST（`nested`/`get_children`、list、
`parent_saved`、environment、interface、default / preorder visitor）会在回归测试中执行
`cargo test`（`rust_automatic_ast_*_behavior`）。请使用与生成器匹配的 `LPG-rust-runtime`；
复杂语法仍建议先用小 fixture 验证；不宣称 `toplevel`/GLR 全量对等。

## 语法特性：`%DropActions`

在 `import` 其他语法时，可使用 `%DropActions`：保留被导入语法的**规则结构**用于解析，但**忽略**其中的动作代码块。适用于复用别处的词法/句法，同时编写自己的语义。

## 工具链

| 组件 | 路径 / 链接 | 用途 |
|------|-------------|------|
| VS Code 扩展 | `tool/LPG-VScode` | 语法高亮、补全、诊断、一键生成 |
| 语言服务 | `tool/LPG-language-server` | 扩展后端 LSP |

## 常见问题

**诊断里的源码摘录与 `= help:` 是什么？**
自 2.3.0 起，错误/警告会附带源码摘录、插入符，以及对常见问题的修复建议。

**出现 `Shift/reduce conflict … (example lookahead: X)` 怎么办？**
用 `%Left` / `%Right` / `%Priority` 消歧、改写规则，或在 CI 中加 `-fail_on_conflicts` 让冲突直接失败。

**`Block not properly terminated`**
动作块必须用匹配的结束标记关闭（默认 `/.` … `./`）。

**旧桩后端 `c`/`ml`/`plx`/`plxasm`/`xml` 报错？**
这些语言值已移除。请改用 `java`、`cpp`、`rt_cpp`、`csharp`、`typescript`、`python3`、`dart`、`go` 或 `rust`。

**生成后如何编译进我的项目？**
将 `-out_directory` 设为项目源码树中的目录，并按对应 `runtime/` 子模块 README 的说明链接运行时库。

**`-nowrite` 有什么用？**
在改语法时快速检查冲突与错误，而不覆盖已有生成文件。

**Rust 与 C++/Go 等流程有何不同？**
命令相同。Rust 运行时在外部 crate `LPG-rust-runtime`，生成物为 Rust 模块。C++ 表生成可用 `cpp`；需要链接 `runtime/LPG-cpp-runtime`（`cpplpg2`）的 parser / nested automatic AST 时使用 `-programming_language=rt_cpp` 与对应 `rt_cpp` 模板。

入门教程：[tutorial.md](tutorial.md) · 示例：[../examples/calculator/](../examples/calculator/)

---

下一步：[开发者文档](DEVELOPER.md) · [英文文档](en/README.md) · [仓库首页](../README.md)
