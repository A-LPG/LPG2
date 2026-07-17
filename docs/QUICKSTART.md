# 5 分钟上手 LPG2

目标：用现成生成器跑通仓库里的计算器示例，确认「语法 → 生成表 → 运行时解析」整条链路可用。

更细的讲解见 [CONCEPTS.md](CONCEPTS.md) 与 [tutorial.md](tutorial.md)。English: [en/QUICKSTART.md](en/QUICKSTART.md)。

## 1. 准备生成器

任选其一。

**A. GitHub Release（推荐）**

1. 从 [Releases](https://github.com/A-LPG/LPG2/releases) 下载对应平台压缩包并校验 `SHA256SUMS`
2. 解压后记下 `bin/lpg-v2.3.0` 的路径

```bash
export LPG_BIN=/path/to/bin/lpg-v2.3.0
```

**B. 从源码构建**

```bash
cd lpg2
cmake -S . -B build && cmake --build build -j
export LPG_BIN="$PWD/build/lpg-v2.3.0"
cd ..
```

## 2. 准备运行时子模块

示例会链接语言运行时。按你要跑的语言初始化（在仓库根目录）：

```bash
# Java（依赖少，适合先试）
git submodule update --init runtime/lpg-runtime

# 或 C++
git submodule update --init runtime/LPG-cpp-runtime

# 或一次拉齐常用项
git submodule update --init runtime/lpg-runtime runtime/LPG-cpp-runtime \
  runtime/LPG-rust-runtime runtime/LPG-typescript-runtime
```

模板目录 `lpg-generator-templates-2.1.00/` 已在主仓库内，无需子模块。

## 3. 一条命令跑通

在仓库根目录：

```bash
# Java（需本机 JDK）
./examples/calculator/scripts/run.sh java

# 或 C++ / TypeScript / Rust
./examples/calculator/scripts/run.sh cpp
./examples/calculator/scripts/run.sh typescript
./examples/calculator/scripts/run.sh rust
```

若未设置 `LPG_BIN`，脚本会尝试使用 `lpg2/build/lpg-v*`。

## 4. 期望结果

驱动会：

- **accept** 形如 `NUMBER + NUMBER * NUMBER` 的 token 序列
- **reject** 以 `PLUS` 开头的非法序列

控制台无报错、进程退出码为 0 即表示成功。

## 5. 你刚刚跑了什么

| 部件 | 路径 / 角色 |
|------|-------------|
| 语法 | `examples/calculator/calculator.g` |
| 生成物 | `examples/calculator/out-<lang>/` 下的解析表与符号表 |
| 驱动 + 运行时 | `examples/calculator/<lang>/` 链接对应 `runtime/` |

生成器**不会**替你写完整词法分析器；本示例用手写 token 注入，只验证解析表与运行时。

## 下一步

1. [CONCEPTS.md](CONCEPTS.md) — 心智模型（生成器 / 模板 / 运行时）
2. [tutorial.md](tutorial.md) — 分步读懂语法与产物
3. [USER.md](USER.md) — 集成到自己的项目
