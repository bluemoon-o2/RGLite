// perf_dict_lookup.cpp - Measure performance of dict.get via VM

#include "TestFramework.h"
#include "VM.h"
#include "Bytecode.h"
#include <chrono>
#include <string>

using namespace rglite;

TEST(Perf, DictLookup500) {
    const int K = 500; // dictionary size
    const int Q = 1000; // queries
    Chunk chunk;

    // Build constants for keys/values
    std::vector<size_t> keyConsts;
    std::vector<size_t> valConsts;
    keyConsts.reserve(K);
    valConsts.reserve(K);
    for (int i = 0; i < K; ++i) {
        keyConsts.push_back(chunk.addConstant(Value(std::string("k") + std::to_string(i))));
        valConsts.push_back(chunk.addConstant(Value(static_cast<int64_t>(i))));
    }

    // dict = {"k0":0, "k1":1, ...}
    for (int i = 0; i < K; ++i) {
        chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(keyConsts[i])));
        chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(valConsts[i])));
    }
    chunk.addInstruction(Instruction(OpCode::BUILD_DICT, static_cast<uint32_t>(K)));

    // Perform Q lookups cycling keys using GET_ITEM (dict["k{idx}"]) 
    for (int q = 0; q < Q; ++q) {
        int idx = q % K;
        // Duplicate dict for safe GET_ITEM (keeps original dict on stack)
        chunk.addInstruction(Instruction(OpCode::DUP));
        // dict["k{idx}"]
        chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(keyConsts[idx])));
        chunk.addInstruction(Instruction(OpCode::GET_ITEM));
        // Discard retrieved value to avoid stack growth
        chunk.addInstruction(Instruction(OpCode::POP));
    }

    chunk.addInstruction(Instruction(OpCode::HALT));

    auto t0 = std::chrono::high_resolution_clock::now();
    VM vm;
    bool ok = vm.interpret(chunk);
    auto t1 = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

    EXPECT_TRUE(ok);
    std::cout << "[Perf] DictLookup K=" << K << ", Q=" << Q << ": " << ms << " ms" << std::endl;
}

RUN_ALL_TESTS();
