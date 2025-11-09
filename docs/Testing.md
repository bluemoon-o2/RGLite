# RGLite 测试指南

本项目使用自研的轻量测试框架（位于 `tests/TestFramework.h`），并通过 CMake/CTest 进行管理与执行。本文档介绍：

- 测试目录结构与分类
- 如何运行核心单元测试与性能测试
- 如何使用测试框架新建测试

## 目录结构

- `tests/unit/`：核心单元测试，覆盖词法、语法、语义、字节码、VM、内建函数与对象方法等。
- `tests/perf/`：性能测试，用于衡量在 VM 上的典型操作（如 `list.append`、`dict.get`）的执行耗时。
- `tests/TestFramework.h`：自研测试框架头文件，提供 `TEST`、断言宏与 `RUN_ALL_TESTS()`。

## 运行测试

- 构建：使用 CMake 标准流程（示例为 Windows 路径）。
  - `cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug`
  - `cmake --build build --config Debug`

- 运行全部测试：
  - `ctest --test-dir build -C Debug -VV`

- 按标签运行：
  - 仅运行单元测试：`ctest --test-dir build -C Debug -L unit`
  - 仅运行性能测试：`ctest --test-dir build -C Debug -L perf`

说明：请勿使用 `--gtest_filter` 等外部框架参数，本项目只使用自研测试框架与 CTest 标签。

## 新建单元测试

1. 在 `tests/unit/` 下创建新文件，例如：`tests/unit/test_example.cpp`
2. 引入测试框架与需要的头文件：
   ```cpp
   #include "TestFramework.h"
   #include "VM.h"
   #include "Bytecode.h"
   using namespace rglite;
   
   TEST(ExampleSuite, BasicCase) {
       Chunk chunk;
       // 构造字节码并运行
       VM vm;
       bool ok = vm.interpret(chunk);
       EXPECT_TRUE(ok);
   }
   
   RUN_ALL_TESTS();
   ```
3. 在 `tests/unit/CMakeLists.txt` 中注册测试：
   ```cmake
   create_unit_test(test_example)
   ```
4. 重新构建并运行：
   - `cmake --build build --config Debug`
   - `ctest --test-dir build -C Debug -L unit`

## 新建性能测试

1. 在 `tests/perf/` 下创建新文件，例如：`tests/perf/perf_my_case.cpp`
2. 引入测试框架与需要的头文件，使用 `std::chrono` 计时：
   ```cpp
   #include "TestFramework.h"
   #include "VM.h"
   #include "Bytecode.h"
   #include <chrono>
   using namespace rglite;
   
   TEST(Perf, MyCase) {
       Chunk chunk;
       // 构造字节码 ...
       auto t0 = std::chrono::high_resolution_clock::now();
       VM vm;
       bool ok = vm.interpret(chunk);
       auto t1 = std::chrono::high_resolution_clock::now();
       auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
       EXPECT_TRUE(ok);
       std::cout << "[Perf] MyCase: " << ms << " ms" << std::endl;
   }
   
   RUN_ALL_TESTS();
   ```
3. 在 `tests/perf/CMakeLists.txt` 中注册测试：
   ```cmake
   create_perf_test(perf_my_case)
   ```
4. 运行：`ctest --test-dir build -C Debug -L perf`

## 断言与夹具

- 断言宏：`EXPECT_TRUE(expr)`、`EXPECT_EQ(a, b)` 等，失败会记录但不中断进程。
- 测试夹具：继承 `testing::TestFixture`，覆盖 `SetUp()`/`TearDown()`，在 `TEST(Suite, Case)` 中实例化后使用。

## 迁移与清理说明

- 已移除重复/调试型测试：`debug_nested_indent.cpp`（与 `test_indent.cpp` 覆盖重复）与未被构建的旧式 `test_tuple_set.cpp`。
- 所有核心测试已迁移到 `tests/unit/` 并通过标签 `unit` 管理；新增性能测试置于 `tests/perf/` 并通过标签 `perf` 运行。

## 常见问题

- 链接失败：确保测试通过 `target_link_libraries(<test> PRIVATE rglite)` 链接核心库。
- 头文件找不到：测试目标已自动包含 `${PROJECT_SOURCE_DIR}/tests`、`include`、`src` 目录。
- 运行找不到测试：确认已在对应子目录 `CMakeLists.txt` 注册，并重新构建。

