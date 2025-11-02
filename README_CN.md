# RGLite - 轻量级编程语言

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](https://github.com/bluemoon-o2/RGLite)
[![License](https://img.shields.io/badge/license-Apache%202.0-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20macOS%20%7C%20Linux-lightgrey.svg)]()
[![Build and Test](https://github.com/your-username/RGLite/actions/workflows/build-and-test.yml/badge.svg)](https://github.com/your-username/RGLite/actions/workflows/build-and-test.yml)
[![Code Quality](https://github.com/your-username/RGLite/actions/workflows/code-quality.yml/badge.svg)](https://github.com/your-username/RGLite/actions/workflows/code-quality.yml)
[![Documentation](https://github.com/your-username/RGLite/actions/workflows/deploy-docs.yml/badge.svg)](https://github.com/your-username/RGLite/actions/workflows/deploy-docs.yml)
[![Benchmark](https://github.com/your-username/RGLite/actions/workflows/benchmark.yml/badge.svg)](https://github.com/your-username/RGLite/actions/workflows/benchmark.yml)
[![Release](https://github.com/your-username/RGLite/actions/workflows/release.yml/badge.svg)](https://github.com/your-username/RGLite/actions/workflows/release.yml)

## 语言切换 | Language Switch

[中文](README_CN.md) | [English](README_EN.md)

RGLite 是一种面向初学者与快速开发场景的解释型编程语言，语法贴近 Python 使用习惯，旨在提供简单、直观的编程体验。

## 项目概述

- **语言名称**: RGLite（轻量级编程语言）
- **类型**: 解释型、动态类型、面向对象编程语言
- **核心定位**: 
  - 面向编程入门教学，语法极简易理解
  - 支持快速脚本开发，降低编码成本
- **依赖环境**: 跨平台（Windows/macOS/Linux），需安装RGLite解释器（约5MB轻量包）

## 快速开始 | Quick Start

### 构建项目 | Build Project

```bash
# 克隆仓库 | Clone repository
git clone https://github.com/bluemoon-o2/RGLite.git
cd RGLite

# 创建构建目录 | Create build directory
mkdir build
cd build

# 配置CMake | Configure CMake
cmake ..

# 构建项目 | Build project
cmake --build .
```

### 运行测试 | Run Tests

```bash
# 在build目录下运行所有测试 | Run all tests in build directory
ctest

# 运行特定测试 | Run specific tests
ctest -R LexerTest
ctest -R ParserTest
ctest -R SemanticAnalyzerTest
ctest -R ASTTest

# 详细输出 | Verbose output
ctest --output-on-failure
```

### GitHub Actions | GitHub Actions

本项目使用GitHub Actions进行持续集成和部署，包括：
- 多平台构建和测试（Ubuntu、Windows、macOS）
- 代码质量检查（格式化、静态分析）
- 文档自动部署
- 性能基准测试
- 自动化发布流程

详细使用说明请参考：[GitHub Actions 配置和使用指南](docs/GitHub_Actions.md)

This project uses GitHub Actions for continuous integration and deployment, including:
- Multi-platform build and test (Ubuntu, Windows, macOS)
- Code quality checks (formatting, static analysis)
- Automatic documentation deployment
- Performance benchmarking
- Automated release process

For detailed usage instructions, see: [GitHub Actions Configuration and Usage Guide](docs/GitHub_Actions.md)

## 语言特性

### 基本语法

- **缩进规则**: 支持4个空格或1个Tab作为1级缩进
- **语句分隔**: 单个语句占1行，无需加分号`;`
- **注释规则**: 
  - 单行注释用`#`开头
  - 多行注释用`/* */`包裹

```python
# 缩进示例：Tab与空格均可
if 3 > 2:
	print("Tab缩进生效")  # 1个Tab
    print("空格缩进生效")  # 4个空格

# 多行语句示例：无需反斜杠
total = (10 + 20 + 30
         + 40 + 50)  # 括号内多行
```

### 变量与数据类型

| 类型 | 说明 | 示例 |
|------|------|------|
| 数值型 | 包含整数（int）、浮点数（float） | `5`、`3.14`、`-10` |
| 字符串 | 单引号/双引号包裹，支持三引号多行字符串 | `"hello"`、`'RGLite'`、`"""多行文本"""` |
| 布尔型 | 仅两个值：True（真）、False（假） | `3 > 2 → True`、`1 == 0 → False` |
| 列表 | 有序可修改的集合，用`[]`包裹 | `[1, "apple", True]` |
| 字典 | 键值对集合，用`{}`包裹，键唯一 | `{"name": "Tom", "age": 18}` |
| 空值 | 用`None`表示"无值"状态 | `x = None` |

### 控制流

```python
# 条件语句
score = 85
if score >= 90:
    print("优秀")
elif score >= 70:
    print("良好")
else:
    print("需努力")

# 循环语句
fruits = ["apple", "banana"]
for fruit in fruits:
    print(fruit)

count = 0
while count < 3:
    print(count)
    count = count + 1
```

### 函数与类

```python
# 函数定义
def calculate(a, b=2):
    return a * b

# 类定义
class Dog:
    def __init__(self, name, age):
        self.name = name
        self.age = age
    
    def shout(self):
        print(f"{self.name}（{self.age}岁）在汪汪叫")

# 使用示例
dog = Dog("小白", 2)
dog.shout()
```

## 项目结构

```
RGLite/
├── include/              # 头文件目录
│   ├── AST.h             # 抽象语法树定义
│   ├── Lexer.h           # 词法分析器
│   ├── Parser.h          # 语法分析器
│   ├── SemanticAnalyzer.h # 语义分析器
│   └── ...
├── src/                  # 源代码目录
│   ├── frontend/         # 前端模块
│   │   ├── lexer/        # 词法分析实现
│   │   ├── parser/       # 语法分析实现
│   │   └── semantic/     # 语义分析实现
│   ├── backend/          # 后端模块
│   │   ├── bytecode/     # 字节码生成
│   │   ├── codegen/      # 代码生成
│   │   └── vm/           # 虚拟机
│   ├── runtime/          # 运行时系统
│   │   ├── builtins/     # 内置函数
│   │   └── memory/       # 内存管理
│   └── common/           # 公共组件
├── tests/                # 测试目录
├── docs/                 # 文档目录
├── examples/             # 示例代码
└── CMakeLists.txt        # 构建配置
```

## 技术架构

### 前端系统
- **词法分析器**: 将源代码转换为Token流
- **语法分析器**: 基于递归下降算法，将Token流转换为AST
- **语义分析器**: 进行类型检查、作用域分析等语义检查

### 后端系统
- **字节码生成器**: 将AST转换为字节码指令
- **虚拟机**: 执行字节码指令，实现语言运行时
- **内存管理**: 引用计数+标记清除的混合GC机制

### 运行时系统
- **内置函数库**: 提供常用功能如print、input等
- **类型系统**: 动态类型系统，支持类型推断
- **异常处理**: 运行时错误检测与处理

## 贡献指南

我们欢迎所有形式的贡献！请查看 [CONTRIBUTING.md](CONTRIBUTING.md) 了解详细信息。

## 许可证

本项目采用 Apache License 2.0 - 查看 [LICENSE](LICENSE) 文件了解详情。

## 联系我们

- 项目主页: https://github.com/bluemoon-o2/RGLite
- 问题反馈: https://github.com/bluemoon-o2/RGLite/issues
- 邮箱: your-email@example.com

## 致谢

感谢所有为 RGLite 项目做出贡献的开发者！