# LPG2 开发者文档

面向**维护、扩展 LPG2 生成器**的读者。使用生成器编写语法请参阅 [用户文档](USER.md)。

## 仓库结构

```
LPG2/
├── lpg2/                    # 生成器源码（本项目的核心）
├── runtime/                 # 各语言运行时（git 子模块）
├── tool/                    # VS Code 扩展与语言服务（子模块）
├── grammars-example/        # 示例语法（子模块）
├── lpg-generator-templates-2.1.00/   # 模板与历史文档
└── docs/
    ├── USER.md              # 用户文档
    └── DEVELOPER.md         # 本文档
```

生成器实现集中在 `lpg2/`：

```
lpg2/
├── CMakeLists.txt
├── include/                 # 头文件（含自举生成的 jikespg_*）
├── src/                     # 生成器与已编译的自举解析器
├── grammar/
│   ├── jikespg.g            # 自举语法（权威来源）
│   ├── jikespg.lpg          # 与 .g 保持同步
│   └── .lpg/                # 重新生成时的暂存区
├── tests/                   # ctest 回归（fixtures + runner）
├── scripts/
└── BOOTSTRAP.md             # 自举策略详解
```

## 开发环境

- CMake ≥ 3.1
- C++17 编译器（GCC、Clang 或 MSVC）

## 从源码构建

```bash
cd lpg2
cmake -S . -B build
cmake --build build -j
```

产物默认位于 `build/lpg-v2.2.03`。

### CMake 选项

| 选项 | 默认值 | 说明 |
|------|--------|------|
| `LPG2_OUTPUT_DIR` | `${CMAKE_BINARY_DIR}` | 可执行文件输出目录 |
| `LPG2_DEPLOY_TO_VSCODE` | `OFF` | 为 `ON` 时将二进制部署到 VS Code 扩展 server 目录（仅本地扩展开发） |

示例：

```bash
cmake -S . -B build -DLPG2_OUTPUT_DIR=/usr/local/bin
cmake --build build -j
```

## 代码架构概览

| 模块 | 主要文件 | 职责 |
|------|----------|------|
| 驱动与选项 | `control.cpp`, `options.cpp`, `option*.cpp` | CLI、版本号、流程控制 |
| 语法分析 | `grammar.cpp`, `parser.cpp`, `jikespg_*.cpp` | 读取 `.g` / `.lpg`，构建内部表示 |
| 表生成 | `*Table.cpp`（如 `CppTable.cpp`, `RustTable.cpp`） | 输出各语言解析表 |
| 动作生成 | `*Action.cpp`（如 `CppAction.cpp`, `RustAction.cpp`） | 输出语义动作代码 |
| 冲突与诊断 | `resolve.cpp`, `diagnose.cpp` | 冲突报告与错误信息 |

新增目标语言时，通常需要实现一对 `*Table` / `*Action`，并在 `grammar.cpp`、`control.cpp`、`options.cpp` 中注册 `Option::<LANG>`。

当前 **Rust** 后端：`src/RustTable.cpp`、`src/RustAction.cpp`（对接 [LPG-rust-runtime](https://github.com/kuafuwang/LPG-rust-runtime) 的 `ParseTable` trait）。

## 自举（Bootstrap）

LPG2 用自身语法 `grammar/jikespg.g` 实现。仓库中 `src/jikespg_*.cpp` 与 `include/jikespg_*.h` 是**经审查后**纳入构建的生成物，而非每次构建自动重生成。

重新生成命令：

```bash
lpg-v2.2.03 -programming_language=cpp -table \
  -out_directory=grammar/.lpg \
  grammar/jikespg.g
```

**重要：** `grammar/.lpg/` 仅为暂存区。直接复制到 `src/` 可能引入更多 shift/reduce 冲突并改变解析行为。晋升流程、冲突对比与测试要求见 [lpg2/BOOTSTRAP.md](../lpg2/BOOTSTRAP.md)。

## 测试

生成器回归测试在 [`lpg2/tests/`](../lpg2/tests/)：fixture 语法 + `run_generation_test.cmake`（校验退出码与 `*prs.*` / `*sym.*` 输出文件存在且非空）。

```bash
cd lpg2
cmake -S . -B build && cmake --build build -j
ctest --test-dir build --output-on-failure          # 全量（当前 12 cases）
ctest --test-dir build -L smoke --output-on-failure # 日常快速冒烟
ctest --test-dir build -L integration               # 自举集成
```

| 标签 | 用例 | 说明 |
|------|------|------|
| `smoke` | `minimal_{cpp,java,go,python3,csharp,typescript,dart,rust}` | 微型语法覆盖 8 个完整后端 |
| `smoke` + `feature` | `dropactions_import` | `%Import` + `%DropActions` |
| `smoke` + `feature` | `invalid_lang_fails` | 非法 `-programming_language` 必须非零退出 |
| `integration` | `bootstrap_cpp`, `bootstrap_rust` | `grammar/jikespg.g` 自举表生成 |

### 新增 case

在 [`lpg2/tests/CMakeLists.txt`](../lpg2/tests/CMakeLists.txt) 调用：

```cmake
lpg2_add_generation_test(<name> <grammar.g> <lang>
    [PREFIX <file_prefix>]   # 默认取语法文件基名
    [EXPECT_FAIL]            # 期望生成器失败
    [LABELS smoke|integration|feature ...])
```

需要时把 fixture 放进 `lpg2/tests/fixtures/`。

### 下游 Rust 运行时（可选）

将 [LPG-rust-runtime](https://github.com/kuafuwang/LPG-rust-runtime) 克隆为 `LPG2` 的同级目录后：

```bash
cd ../LPG-rust-runtime
cargo test -p generated_tables
```

刷新示例表：

```bash
cd lpg2
./scripts/regen_rust_example_tables.sh
# 或：LPG_BIN=/path/to/lpg-v2.2.03 ./scripts/regen_rust_example_tables.sh
```

## 维护脚本

| 脚本 | 用途 |
|------|------|
| `lpg2/scripts/regen_rust_example_tables.sh` | 用自举语法更新 Rust 运行时示例表 |
| `lpg2/scripts/go_to_rust_action.py` | 从 Go 后端迁移/维护 Rust 后端的辅助工具 |

## 版本与命名

- 版本字符串：`control.cpp` 中 `Control::VERSION`（当前 `2.2.03`）
- 可执行文件名：CMake `OUTPUT_NAME` → `lpg-v2.2.03`

修改版本时需同步上述两处及用户可见文档。

## 子模块与扩展开发

初始化子模块：

```bash
git submodule update --init --recursive
```

扩展开发时，可开启 `LPG2_DEPLOY_TO_VSCODE` 将新构建的二进制直接放入本地安装的 `lpg-vscode` server 路径，便于联调。

## 已知限制（维护者备忘）

| 区域 | 说明 |
|------|------|
| `resolve.cpp` | 部分冲突消解逻辑仍有 TODO |
| C / ML / Plx / Xml 后端 | 桩实现，非生产级 |
| `grammar/.lpg/` vs `src/` | 可能存在表漂移；以 `src/` 编译结果为准，晋升需走 BOOTSTRAP 流程 |
| `RustAction.cpp` | 复杂 AST 语法可能需要针对具体语法再调优 |

## 相关仓库

| 仓库 | 角色 |
|------|------|
| [LPG2](https://github.com/kuafuwang/LPG2) | 生成器（本仓库） |
| [LPG-rust-runtime](https://github.com/kuafuwang/LPG-rust-runtime) | Rust 运行时 |
| [LPG2-grammars-example](https://github.com/kuafuwang/LPG2-grammars-example) | 示例语法 |
| [lpg-vscode](https://github.com/kuafuwang/lpg-vscode) | VS Code 扩展 |

---

下一步：[用户文档](USER.md) · [自举策略](../lpg2/BOOTSTRAP.md) · [仓库首页](../README.md)
