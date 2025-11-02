# GitHub Actions Workflows

This directory contains GitHub Actions workflows for the RGLite project.

## Workflows

### 1. Build and Test (`build-and-test.yml`)

**Triggers:**
- Push to `main` or `develop` branches
- Pull requests to `main` branch

**Features:**
- Multi-platform builds (Ubuntu, Windows, macOS)
- Multiple build configurations (Debug, Release)
- Automatic CMake configuration and building
- Running tests with CTest
- Windows-specific test for semantic analyzer

### 2. Deploy Documentation (`deploy-docs.yml`)

**Triggers:**
- Push to `main` branch
- Manual workflow dispatch

**Features:**
- Markdown linting and link checking
- Documentation build process
- Automatic deployment to GitHub Pages

### 3. Code Quality (`code-quality.yml`)

**Triggers:**
- Push to `main` or `develop` branches
- Pull requests to `main` branch

**Features:**
- Code formatting checks with clang-format
- Static analysis with cppcheck
- Python code style checks with flake8

### 4. Performance Benchmark (`benchmark.yml`)

**Triggers:**
- Push to `main` branch
- Weekly schedule (Mondays at midnight UTC)

**Features:**
- Release build configuration
- Performance benchmark execution
- Benchmark result storage and visualization

### 5. Release (`release.yml`)

**Triggers:**
- Push with version tags (e.g., `v1.0.0`)

**Features:**
- Release build configuration
- Package creation with CPack
- Automatic GitHub release creation with asset uploads

## Usage

These workflows will automatically run when their triggers are activated. You can also manually trigger the documentation deployment workflow from the GitHub Actions tab in your repository.

## Configuration

To customize these workflows for your specific needs:

1. Modify the matrix configurations in `build-and-test.yml` for different platforms or build types
2. Update the documentation build process in `deploy-docs.yml`
3. Adjust code quality checks in `code-quality.yml`
4. Customize benchmark commands in `benchmark.yml`
5. Modify packaging options in `release.yml`

## Secrets

Make sure to configure the following secrets in your repository settings:

- `GITHUB_TOKEN`: Automatically provided by GitHub Actions
- Any additional secrets needed for your specific deployment or testing requirements