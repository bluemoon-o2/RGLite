// perf_list_append.cpp - Measure performance of list.append via VM

#include "TestFramework.h"
#include "VM.h"
#include "Bytecode.h"
#include <chrono>

using namespace rglite;

TEST(Perf, ListAppend2000) {
    const int N = 2000;
    Chunk chunk;

    // Constants
    size_t mAppend = chunk.addConstant(Value(std::string("append")));
    size_t zero = chunk.addConstant(Value(static_cast<int64_t>(0)));

    // list = [0]
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(zero)));
    chunk.addInstruction(Instruction(OpCode::BUILD_LIST, static_cast<uint32_t>(1)));

    // Append N integers: 1..N
    for (int i = 1; i <= N; ++i) {
        size_t vi = chunk.addConstant(Value(static_cast<int64_t>(i)));
        // list.append(i)
        chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(mAppend)));
        chunk.addInstruction(Instruction(OpCode::GET_ATTR));
        chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(vi)));
        chunk.addInstruction(Instruction(OpCode::CALL, static_cast<uint32_t>(2))); // receiver + 1 arg
    }

    // Finalize
    chunk.addInstruction(Instruction(OpCode::HALT));

    auto t0 = std::chrono::high_resolution_clock::now();
    VM vm;
    bool ok = vm.interpret(chunk);
    auto t1 = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

    EXPECT_TRUE(ok);
    // Print elapsed time for reference
    std::cout << "[Perf] ListAppend " << N << " ops: " << ms << " ms" << std::endl;
}

RUN_ALL_TESTS();
