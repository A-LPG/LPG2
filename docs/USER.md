# LPG2 用户文档

面向**使用 LPG2 编写语法、生成解析器**的读者。若你要修改生成器本身，请参阅 [开发者文档](DEVELOPER.md)。

## LPG2 是什么

LPG2（Lookahead Parser Generator v2）读取 `.g` / `.lpg` 语法文件，生成目标语言的**解析表**和**语义动作代码**。支持 LALR 解析、回溯消歧、语法继承与 AST 生成。

当前版本：**2.2.03**（可执行文件名 `lpg-v2.2.03`）。

## 获取生成器

### 方式一：VS Code 扩展（推荐）

安装 [lpg-vscode](https://marketplace.visualstudio.com/items?itemName=kuafuwang.lpg-vscode) 扩展后，扩展会自带 LPG 语言服务与生成器二进制，可直接在编辑器中编写语法并触发生成。

本仓库中的扩展源码位于 `tool/LPG-VScode/`（子模块）。

### 方式二：从源码编译

```bash
cd lpg2
cmake -S . -B build
cmake --build build -j
./build/lpg-v2.2.03 -help
```

## 基本工作流

1. 编写或修改 `.g` / `.lpg` 语法文件
2. 选择目标语言，运行生成器
3. 将生成的解析表与动作代码集成到对应语言的**运行时库**中

典型命令：

```bash
lpg-v2.2.03 -programming_language=cpp -table \
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

完整参数列表：`lpg-v2.2.03 -help`

## 支持的目标语言

| 语言 | 参数值 | 状态 |
|------|--------|------|
| C++ | `cpp` | 完整支持 |
| Java | `java` | 完整支持 |
| C# | `csharp` | 完整支持 |
| Go | `go` | 完整支持 |
| Python 2 | `python2` | 完整支持 |
| Python 3 | `python3` | 完整支持 |
| TypeScript | `typescript` | 完整支持 |
| Dart | `dart` | 完整支持 |
| Rust | `rust` | 完整支持 |
| C | `c` | 有限 / 桩实现 |
| ML | `ml` | 有限 / 桩实现 |
| Plx | `plx` | 有限 / 桩实现 |
| Xml | `xml` | 有限 / 桩实现 |

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

**Rust** 运行时在独立仓库 [LPG-rust-runtime](https://github.com/kuafuwang/LPG-rust-runtime)，不在本仓库子模块中。

克隆含子模块的完整仓库：

```bash
git clone --recursive https://github.com/kuafuwang/LPG2.git
```

## 示例语法

- 本仓库：`grammars-example/`（子模块）
- 独立集合：[LPG2-grammars-example](https://github.com/kuafuwang/LPG2-grammars-example)

入门模板与更详细的 LPG 语法说明见 `lpg-generator-templates-2.1.00/docs/`。

## Rust 项目集成

1. 添加 [LPG-rust-runtime](https://github.com/kuafuwang/LPG-rust-runtime) 依赖
2. 用生成器产出 `*prs.rs` 与 `*sym.rs`：

```bash
lpg-v2.2.03 -programming_language=rust -table \
  -out_directory=src/generated \
  my_grammar.g
```

3. 在项目中 `mod` 引入生成文件，使用运行时提供的 `ParseTable` trait 驱动解析

生成文件需与 `LPG-rust-runtime` 版本匹配；升级任一侧时请重新生成并测试。

## 语法特性：`%DropActions`

在 `import` 其他语法时，可使用 `%DropActions`：保留被导入语法的**规则结构**用于解析，但**忽略**其中的动作代码块。适用于复用别处的词法/句法，同时编写自己的语义。

## 工具链

| 组件 | 路径 / 链接 | 用途 |
|------|-------------|------|
| VS Code 扩展 | `tool/LPG-VScode` | 语法高亮、补全、诊断、一键生成 |
| 语言服务 | `tool/LPG-language-server` | 扩展后端 LSP |

## 常见问题

**生成后如何编译进我的项目？**  
将 `-out_directory` 设为项目源码树中的目录，并按对应 `runtime/` 子模块 README 的说明链接运行时库。

**`-nowrite` 有什么用？**  
在改语法时快速检查冲突与错误，而不覆盖已有生成文件。

**Rust 与 C++/Go 等流程有何不同？**  
命令相同，但运行时在外部 crate `LPG-rust-runtime`，生成物为 Rust 模块而非 C++ 头文件/源文件。

---

下一步：[开发者文档](DEVELOPER.md) · [仓库首页](../README.md)
