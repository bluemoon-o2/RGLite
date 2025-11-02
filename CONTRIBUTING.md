<div align="center">
    <p>
        <img src="./docs/img/banner%20(4).png" alt="RGLite Banner">
    </p>

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](https://github.com/bluemoon-o2/RGLite)
[![License](https://img.shields.io/badge/license-Apache%202.0-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20macOS%20%7C%20Linux-lightgrey.svg)]()

**RGLite 是一种面向初学者与快速开发场景的解释型编程语言，语法贴近 Python 使用习惯，旨在提供简单、直观的编程体验。**
</div>

# 贡献指南

我们欢迎所有形式的贡献！无论是报告问题、提出功能建议、改进文档还是提交代码，我们都非常感谢您的参与。

## 目录

- [报告问题](#报告问题)
- [提交代码](#提交代码)
- [代码风格](#代码风格)
- [提交信息规范](#提交信息规范)
- [开发环境要求](#开发环境要求)
- [代码审查](#代码审查)
- [社区行为准则](#社区行为准则)

## 报告问题

如果您发现了bug或有功能建议，请通过以下方式报告：

1. **检查现有问题**：在提交新问题前，请先搜索是否已有相关问题
2. **使用清晰的问题标题**：简明扼要地描述问题
3. **提供详细信息**：
   - 操作系统和环境信息
   - 重现步骤
   - 期望行为与实际行为的差异
   - 相关代码片段或错误信息

## 提交代码

1. **Fork 仓库**：在GitHub上fork项目仓库到您的账户
2. **创建分支**：为您的贡献创建一个新分支
   ```bash
   git checkout -b feature/your-feature-name
   ```
3. **编写代码**：
   - 遵循项目的代码风格和约定
   - 添加必要的注释和文档
   - 确保代码通过所有现有测试
4. **测试您的更改**：
   ```bash
   # 运行所有测试
   cmake --build . --target test_semantic_analyzer
   .\Debug\test_semantic_analyzer.exe
   ```
5. **提交更改**：
   ```bash
   git add .
   git commit -m "feat: add your feature description"
   ```
6. **推送并创建PR**：
   ```bash
   git push origin feature/your-feature-name
   ```
   然后在GitHub上创建Pull Request

## 代码风格

- **缩进**：使用4个空格进行缩进
- **命名**：
   - 类名使用PascalCase（如`SemanticAnalyzer`）
   - 函数和变量使用camelCase（如`analyzeStatement`）
   - 常量使用UPPER_SNAKE_CASE（如`MAX_TOKEN_LENGTH`）
- **注释**：
   - 类和公共函数必须有文档注释
   - 复杂逻辑需要添加行内注释
   - 所有注释使用英文

## 提交信息规范

使用[约定式提交](https://www.conventionalcommits.org/zh-hans/v1.0.0/)格式：

```
<type>[optional scope]: <description>

[optional body]

[optional footer(s)]
```

类型（type）包括：
- `feat`: 新功能
- `fix`: 修复bug
- `docs`: 文档更新
- `style`: 代码格式调整（不影响功能）
- `refactor`: 重构代码
- `test`: 添加或修改测试
- `chore`: 构建过程或辅助工具的变动

示例：
```
feat(lexer): add support for Unicode identifiers

Add support for Unicode characters in identifiers to improve
internationalization support.

Closes #123
```

## 开发环境要求

- **编译器**：支持C++20的编译器（GCC 10+、Clang 12+、MSVC 19.30+）
- **构建工具**：CMake 3.16 或更高版本
- **版本控制**：Git
- **操作系统**：Windows、macOS 或 Linux

## 代码审查

所有提交的代码都需要经过代码审查，审查者会关注：

- 代码质量和可读性
- 是否符合项目编码规范
- 是否有适当的测试覆盖
- 是否可能引入新的bug
- 是否与项目整体架构一致

## 社区行为准则

我们致力于为每个人提供友好、安全和欢迎的环境，无论性别、性别认同和表达、性取向、残疾、外貌、体型、种族、年龄、宗教或国籍。请：

- 使用友好和包容的语言
- 尊重不同的观点和经验
- 优雅地接受建设性批评
- 关注对社区最有利的事情
- 对其他社区成员表示同理心