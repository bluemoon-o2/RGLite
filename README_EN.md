# RGLite - Lightweight Programming Language

[![Build and Test](https://github.com/your-username/RGLite/actions/workflows/build-and-test.yml/badge.svg)](https://github.com/your-username/RGLite/actions/workflows/build-and-test.yml)
[![Code Quality](https://github.com/your-username/RGLite/actions/workflows/code-quality.yml/badge.svg)](https://github.com/your-username/RGLite/actions/workflows/code-quality.yml)
[![Documentation](https://github.com/your-username/RGLite/actions/workflows/deploy-docs.yml/badge.svg)](https://github.com/your-username/RGLite/actions/workflows/deploy-docs.yml)
[![Benchmark](https://github.com/your-username/RGLite/actions/workflows/benchmark.yml/badge.svg)](https://github.com/your-username/RGLite/actions/workflows/benchmark.yml)
[![Release](https://github.com/your-username/RGLite/actions/workflows/release.yml/badge.svg)](https://github.com/your-username/RGLite/actions/workflows/release.yml)

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](https://github.com/bluemoon-o2/RGLite)
[![License](https://img.shields.io/badge/license-Apache%202.0-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20macOS%20%7C%20Linux-lightgrey.svg)]()

## Language Switch | 语言切换

[English](README_EN.md) | [中文](README_CN.md)

RGLite is an interpreted, dynamically-typed, object-oriented programming language designed for beginners and rapid development scenarios, with syntax that closely follows Python conventions to provide a simple, intuitive programming experience.

## Project Overview

- **Language Name**: RGLite (Lightweight Programming Language)
- **Type**: Interpreted, dynamically-typed, object-oriented programming language
- **Core Positioning**: 
  - Designed for programming education with extremely simple and easy-to-understand syntax
  - Supports rapid script development to reduce coding costs
- **Environment Requirements**: Cross-platform (Windows/macOS/Linux), requires RGLite interpreter (approximately 5MB lightweight package)

## Quick Start | 快速开始

### Build Project | 构建项目

```bash
# Clone repository | 克隆仓库
git clone https://github.com/bluemoon-o2/RGLite.git
cd RGLite

# Create build directory | 创建构建目录
mkdir build
cd build

# Configure CMake | 配置CMake
cmake ..

# Build project | 构建项目
cmake --build .
```

### Run Tests | 运行测试

```bash
# Run all tests in build directory | 在build目录下运行所有测试
ctest

# Run specific tests | 运行特定测试
ctest -R LexerTest
ctest -R ParserTest
ctest -R SemanticAnalyzerTest
ctest -R ASTTest

# Verbose output | 详细输出
ctest --output-on-failure
```

### GitHub Actions | GitHub Actions

This project uses GitHub Actions for continuous integration and deployment, including:
- Multi-platform build and test (Ubuntu, Windows, macOS)
- Code quality checks (formatting, static analysis)
- Automatic documentation deployment
- Performance benchmarking
- Automated release process

For detailed usage instructions, see: [GitHub Actions Configuration and Usage Guide](docs/GitHub_Actions.md)

本项目使用GitHub Actions进行持续集成和部署，包括：
- 多平台构建和测试（Ubuntu、Windows、macOS）
- 代码质量检查（格式化、静态分析）
- 文档自动部署
- 性能基准测试
- 自动化发布流程

详细使用说明请参考：[GitHub Actions 配置和使用指南](docs/GitHub_Actions.md)

## Language Features

### Basic Syntax

- **Indentation Rules**: Supports 4 spaces or 1 tab as 1 level of indentation
- **Statement Separation**: One statement per line, no semicolon `;` required
- **Comment Rules**: 
  - Single-line comments start with `#`
  - Multi-line comments are wrapped with `/* */`

```python
# Indentation example: Both tab and space are acceptable
if 3 > 2:
	print("Tab indentation works")  # 1 tab
    print("Space indentation works")  # 4 spaces

# Multi-line statement example: No backslash needed
total = (10 + 20 + 30
         + 40 + 50)  # Multi-line within parentheses
```

### Variables and Data Types

| Type | Description | Example |
|------|-------------|---------|
| Numeric | Includes integers (int) and floating-point numbers (float) | `5`, `3.14`, `-10` |
| String | Wrapped in single/double quotes, supports triple quotes for multi-line | `"hello"`, `'RGLite'`, `"""multi-line text"""` |
| Boolean | Only two values: True, False | `3 > 2 → True`, `1 == 0 → False` |
| List | Ordered, modifiable collection, wrapped in `[]` | `[1, "apple", True]` |
| Dictionary | Key-value pair collection, wrapped in `{}`, keys are unique | `{"name": "Tom", "age": 18}` |
| Null Value | Use `None` to represent "no value" state | `x = None` |

### Control Flow

```python
# Conditional statements
score = 85
if score >= 90:
    print("Excellent")
elif score >= 70:
    print("Good")
else:
    print("Needs improvement")

# Loop statements
fruits = ["apple", "banana"]
for fruit in fruits:
    print(fruit)

count = 0
while count < 3:
    print(count)
    count = count + 1
```

### Functions and Classes

```python
# Function definition
def calculate(a, b=2):
    return a * b

# Class definition
class Dog:
    def __init__(self, name, age):
        self.name = name
        self.age = age
    
    def shout(self):
        print(f"{self.name} ({self.age} years old) is barking")

# Usage example
dog = Dog("Buddy", 2)
dog.shout()
```

## Project Structure

```
RGLite/
├── include/              # Header files directory
│   ├── AST.h             # Abstract Syntax Tree definition
│   ├── Lexer.h           # Lexical analyzer
│   ├── Parser.h          # Syntax analyzer
│   ├── SemanticAnalyzer.h # Semantic analyzer
│   └── ...
├── src/                  # Source code directory
│   ├── frontend/         # Frontend modules
│   │   ├── lexer/        # Lexical analysis implementation
│   │   ├── parser/       # Syntax analysis implementation
│   │   └── semantic/     # Semantic analysis implementation
│   ├── backend/          # Backend modules
│   │   ├── bytecode/     # Bytecode generation
│   │   ├── codegen/      # Code generation
│   │   └── vm/           # Virtual machine
│   ├── runtime/          # Runtime system
│   │   ├── builtins/     # Built-in functions
│   │   └── memory/       # Memory management
│   └── common/           # Common components
├── tests/                # Test directory
├── docs/                 # Documentation directory
├── examples/             # Example code
└── CMakeLists.txt        # Build configuration
```

## Technical Architecture

### Frontend System
- **Lexical Analyzer**: Converts source code into token streams
- **Syntax Analyzer**: Based on recursive descent algorithm, converts token streams into AST
- **Semantic Analyzer**: Performs type checking, scope analysis, and other semantic checks

### Backend System
- **Bytecode Generator**: Converts AST into bytecode instructions
- **Virtual Machine**: Executes bytecode instructions, implements language runtime
- **Memory Management**: Hybrid GC mechanism with reference counting + mark and sweep

### Runtime System
- **Built-in Function Library**: Provides common functions like print, input, etc.
- **Type System**: Dynamic type system with type inference support
- **Exception Handling**: Runtime error detection and handling

## Contributing

We welcome all forms of contributions! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for details.

## License

This project is licensed under the Apache License 2.0 - see the [LICENSE](LICENSE) file for details.

## Contact Us

- Project Homepage: https://github.com/bluemoon-o2/RGLite
- Issue Reporting: https://github.com/bluemoon-o2/RGLite/issues
- Email: your-email@example.com

## Acknowledgments

Thanks to all developers who have contributed to the RGLite project!