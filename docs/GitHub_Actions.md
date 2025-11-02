# GitHub Actions Configuration and Usage Guide

## 徽章说明 | Badge Description

在README文件中，我们添加了以下GitHub Actions工作流徽章，用于直观显示各个工作流的状态：

In the README file, we have added the following GitHub Actions workflow badges to visually display the status of each workflow:

- **构建和测试徽章** | **Build and Test Badge**:
  ```markdown
  [![Build and Test](https://github.com/your-username/RGLite/actions/workflows/build-and-test.yml/badge.svg)](https://github.com/your-username/RGLite/actions/workflows/build-and-test.yml)
  ```

- **代码质量徽章** | **Code Quality Badge**:
  ```markdown
  [![Code Quality](https://github.com/your-username/RGLite/actions/workflows/code-quality.yml/badge.svg)](https://github.com/your-username/RGLite/actions/workflows/code-quality.yml)
  ```

- **文档部署徽章** | **Documentation Badge**:
  ```markdown
  [![Documentation](https://github.com/your-username/RGLite/actions/workflows/deploy-docs.yml/badge.svg)](https://github.com/your-username/RGLite/actions/workflows/deploy-docs.yml)
  ```

- **性能基准徽章** | **Benchmark Badge**:
  ```markdown
  [![Benchmark](https://github.com/your-username/RGLite/actions/workflows/benchmark.yml/badge.svg)](https://github.com/your-username/RGLite/actions/workflows/benchmark.yml)
  ```

- **发布流程徽章** | **Release Badge**:
  ```markdown
  [![Release](https://github.com/your-username/RGLite/actions/workflows/release.yml/badge.svg)](https://github.com/your-username/RGLite/actions/workflows/release.yml)
  ```

> **注意** | **Note**: 请将 `your-username` 替换为您的GitHub用户名或组织名。
> Please replace `your-username` with your GitHub username or organization name.

This document explains the GitHub Actions workflows set up for the RGLite project and how to use them.

## 目录 | Table of Contents

1. [概述 | Overview](#概述--overview)
2. [工作流详情 | Workflow Details](#工作流详情--workflow-details)
3. [本地开发与测试 | Local Development and Testing](#本地开发与测试--local-development-and-testing)
4. [触发工作流 | Triggering Workflows](#触发工作流--triggering-workflows)
5. [查看工作流状态 | Viewing Workflow Status](#查看工作流状态--viewing-workflow-status)
6. [常见问题与解决方案 | Common Issues and Solutions](#常见问题与解决方案--common-issues-and-solutions)
7. [高级用法 | Advanced Usage](#高级用法--advanced-usage)

## 概述 | Overview

The project includes several automated workflows that handle:

1. **Build and Test** - Automated building and testing across multiple platforms
2. **Code Quality** - Code formatting and static analysis checks
3. **Documentation Deployment** - Automatic documentation building and deployment
4. **Performance Benchmarking** - Regular performance testing
5. **Release Management** - Automated release creation

## 工作流详情 | Workflow Details

### Build and Test Workflow | 构建和测试工作流

- **Triggers | 触发条件**: Push to main/develop branches, pull requests to main
- **Platforms | 平台**: Ubuntu, Windows, macOS
- **Build Types | 构建类型**: Debug and Release
- **Features | 功能**:
  - Multi-platform CMake configuration | 多平台CMake配置
  - Automatic building with CMake | 使用CMake自动构建
  - Test execution with CTest | 使用CTest执行测试
  - Windows-specific semantic analyzer tests | Windows特定的语义分析器测试

### Code Quality Workflow | 代码质量工作流

- **Triggers | 触发条件**: Push to main/develop branches, pull requests to main
- **Features | 功能**:
  - Code formatting checks with clang-format | 使用clang-format进行代码格式检查
  - Static analysis with cppcheck | 使用cppcheck进行静态分析
  - Python code style checks with flake8 | 使用flake8进行Python代码风格检查

### Documentation Deployment Workflow | 文档部署工作流

- **Triggers | 触发条件**: Push to main branch, manual dispatch
- **Features | 功能**:
  - Markdown linting and link checking | Markdown检查和链接验证
  - Documentation building | 文档构建
  - Automatic deployment to GitHub Pages | 自动部署到GitHub Pages

### Performance Benchmark Workflow | 性能基准测试工作流

- **Triggers | 触发条件**: Push to main branch, weekly schedule
- **Features | 功能**:
  - Release build configuration | 发布构建配置
  - Performance benchmark execution | 性能基准测试执行
  - Benchmark result storage and visualization | 基准测试结果存储和可视化

### Release Workflow | 发布工作流

- **Triggers | 触发条件**: Version tags (e.g., v1.0.0)
- **Features | 功能**:
  - Release build configuration | 发布构建配置
  - Package creation with CPack | 使用CPack创建包
  - Automatic GitHub release creation with asset uploads | 自动创建GitHub发布并上传资产

## 本地开发与测试 | Local Development and Testing

### 1. 克隆仓库 | Clone Repository

```bash
git clone https://github.com/bluemoon-o2/RGLite.git
cd RGLite
```

### 2. 构建项目 | Build Project

```bash
# Create build directory | 创建构建目录
mkdir build
cd build

# Configure CMake | 配置CMake
cmake ..

# Build project | 构建项目
cmake --build .
```

### 3. 运行测试 | Run Tests

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

### 4. 代码质量检查 | Code Quality Checks

```bash
# Check code formatting | 检查代码格式
find src include tests -name "*.cpp" -o -name "*.h" | xargs clang-format --dry-run --Werror

# Run static analysis | 运行静态分析
cppcheck --enable=all src/ include/ tests/

# Check Python code style | 检查Python代码风格
find . -name "*.py" -not -path "./.git/*" | xargs flake8 --max-line-length=120
```

## CMake集成 | CMake Integration

The project has been updated to support CMake testing:

1. Main `CMakeLists.txt` now includes the tests directory | 主`CMakeLists.txt`现在包含测试目录
2. `tests/CMakeLists.txt` defines all test executables and registers them with CTest | `tests/CMakeLists.txt`定义所有测试可执行文件并用CTest注册
3. Tests can be run locally with `ctest` in the build directory | 可以在构建目录中使用`ctest`本地运行测试
4. GitHub Actions automatically runs tests as part of the CI/CD pipeline | GitHub Actions作为CI/CD管道的一部分自动运行测试

## 触发工作流 | Triggering Workflows

### 自动触发 | Automatic Triggers

以下操作会自动触发相应的工作流 | The following operations will automatically trigger corresponding workflows:

1. **推送代码 | Pushing Code**
   - 推送到`main`或`develop`分支会触发构建和测试、代码质量检查
   - Push to `main` or `develop` branches triggers build and test, code quality checks
   - 推送到`main`分支还会触发文档部署和性能基准测试
   - Push to `main` branch also triggers documentation deployment and performance benchmarking

2. **创建拉取请求 | Creating Pull Requests**
   - 创建针对`main`分支的PR会触发构建和测试、代码质量检查
   - Creating a PR to `main` branch triggers build and test, code quality checks

3. **创建版本标签 | Creating Version Tags**
   - 创建格式为`v*`的标签（如`v1.0.0`）会触发发布管理流程
   - Creating tags in format `v*` (e.g., `v1.0.0`) triggers the release management process

### 手动触发 | Manual Triggers

1. **文档部署 | Documentation Deployment**
   - 进入GitHub仓库的"Actions"选项卡
   - Go to the "Actions" tab of the GitHub repository
   - 选择"Deploy Documentation"工作流
   - Select the "Deploy Documentation" workflow
   - 点击"Run workflow"按钮
   - Click the "Run workflow" button

## 查看工作流状态 | Viewing Workflow Status

### 1. 通过GitHub界面 | Through GitHub Interface

1. 进入GitHub仓库 | Go to the GitHub repository
2. 点击"Actions"选项卡 | Click the "Actions" tab
3. 选择要查看的工作流 | Select the workflow to view
4. 查看运行历史、状态和日志 | View run history, status, and logs

### 2. 通过工作流徽章 | Through Workflow Badges

在README文件中显示工作流状态徽章 | Display workflow status badges in README file:

```markdown
![Build Status](https://github.com/bluemoon-o2/RGLite/workflows/Build%20and%20Test/badge.svg)
```

### 3. 工作流运行状态解读 | Interpreting Workflow Run Status

| 状态 | 含义 | Status | Meaning |
|------|------|--------|---------|
| ⚪ 等待 | 工作流已排队，等待执行 | ⚪ Waiting | Workflow is queued, waiting to run |
| 🟡 进行中 | 工作流正在运行 | 🟡 In Progress | Workflow is running |
| 🟢 成功 | 工作流成功完成 | 🟢 Success | Workflow completed successfully |
| 🔴 失败 | 工作流执行失败 | 🔴 Failure | Workflow execution failed |
| 🟠 取消 | 工作流被取消 | 🟠 Canceled | Workflow was canceled |

## 常见问题与解决方案 | Common Issues and Solutions

### 1. 构建失败 | Build Failures

**问题 | Problem**：构建步骤失败 | Build step failed
**解决方案 | Solution**：
1. 检查工作流日志中的错误信息 | Check error messages in workflow logs
2. 确保CMakeLists.txt配置正确 | Ensure CMakeLists.txt is configured correctly
3. 验证所有依赖项都已正确安装 | Verify all dependencies are installed correctly
4. 本地复现错误并修复 | Reproduce the error locally and fix it

### 2. 测试失败 | Test Failures

**问题 | Problem**：一个或多个测试失败 | One or more tests failed
**解决方案 | Solution**：
1. 查看测试日志了解具体失败原因 | Check test logs to understand specific failure reasons
2. 本地运行相同测试复现问题 | Run the same tests locally to reproduce the issue
3. 修复代码或更新测试用例 | Fix the code or update test cases
4. 确保测试环境与CI环境一致 | Ensure test environment is consistent with CI environment

### 3. 代码质量检查失败 | Code Quality Check Failures

**问题 | Problem**：代码格式或静态分析检查失败 | Code formatting or static analysis checks failed
**解决方案 | Solution**：
1. 使用clang-format自动格式化代码 | Use clang-format to automatically format code:
   ```bash
   find src include tests -name "*.cpp" -o -name "*.h" | xargs clang-format -i
   ```
2. 修复cppcheck报告的问题 | Fix issues reported by cppcheck
3. 确保Python代码符合flake8标准 | Ensure Python code meets flake8 standards

### 4. 文档部署失败 | Documentation Deployment Failures

**问题 | Problem**：文档构建或部署失败 | Documentation build or deployment failed
**解决方案 | Solution**：
1. 检查Markdown文件格式和链接有效性 | Check Markdown file format and link validity
2. 确保GitHub Pages已正确配置 | Ensure GitHub Pages is configured correctly
3. 验证文档构建脚本是否正确 | Verify documentation build script is correct

### 5. 性能基准测试失败 | Performance Benchmark Failures

**问题 | Problem**：性能测试失败或结果异常 | Performance test failed or results are abnormal
**解决方案 | Solution**：
1. 检查基准测试代码是否正确 | Check if benchmark test code is correct
2. 确保测试环境一致 | Ensure test environment is consistent
3. 分析性能回归原因 | Analyze performance regression reasons

## 高级用法 | Advanced Usage

### 1. 自定义工作流 | Customizing Workflows

您可以通过修改`.github/workflows/`目录下的YAML文件来自定义工作流 | You can customize workflows by modifying YAML files in the `.github/workflows/` directory:

- 修改构建矩阵（添加/删除平台或构建类型） | Modify build matrix (add/remove platforms or build types)
- 调整测试命令 | Adjust test commands
- 更新代码质量检查规则 | Update code quality check rules
- 自定义发布流程 | Customize release process

### 2. 使用Secrets | Using Secrets

对于需要敏感信息的工作流（如部署密钥），可以在GitHub仓库设置中添加Secrets | For workflows that require sensitive information (like deployment keys), you can add Secrets in GitHub repository settings:

1. 进入仓库设置 | Go to repository settings
2. 点击"Secrets" > "Actions" | Click "Secrets" > "Actions"
3. 添加新的Secret | Add new Secret
4. 在工作流中使用`${{ secrets.YOUR_SECRET_NAME }}`引用 | Reference in workflows using `${{ secrets.YOUR_SECRET_NAME }}`

### 3. 工作流优化 | Workflow Optimization

- 使用缓存加速构建 | Use caching to speed up builds:
  ```yaml
  - name: Cache CMake build
    uses: actions/cache@v3
    with:
      path: ${{ github.workspace }}/build
      key: ${{ runner.os }}-build-${{ hashFiles('**/CMakeLists.txt') }}
  ```

- 并行执行独立任务 | Execute independent tasks in parallel
- 使用条件执行减少不必要的运行 | Use conditional execution to reduce unnecessary runs

## 总结 | Summary

通过这套GitHub Actions工作流系统，RGLite项目实现了 | Through this GitHub Actions workflow system, the RGLite project achieves:

1. **自动化CI/CD流程**，确保代码质量和稳定性 | **Automated CI/CD process**, ensuring code quality and stability
2. **多平台兼容性**，支持不同操作系统和构建配置 | **Multi-platform compatibility**, supporting different operating systems and build configurations
3. **自动化测试**，快速发现和修复问题 | **Automated testing**, quickly discovering and fixing issues
4. **自动化文档部署**，保持文档与代码同步 | **Automated documentation deployment**, keeping documentation in sync with code
5. **性能监控**，跟踪项目性能变化 | **Performance monitoring**, tracking project performance changes
6. **自动化发布流程**，简化版本发布过程 | **Automated release process**, simplifying version release process

这套系统不仅提高了开发效率，也确保了代码质量和项目的持续健康发展 | This system not only improves development efficiency but also ensures code quality and sustainable healthy development of the project.