// test_methods.cpp - Tests for object method calls (list/dict)

#include "TestFramework.h"
#include "VM.h"
#include "Bytecode.h"

using namespace rglite;

// Helper: add native function constant
static size_t addNative(Chunk& chunk, const std::string& name) {
    return chunk.addConstant(Value(name, true));
}

// list.append(x) method call modifies list in-place
TEST(Methods, ListAppend) {
    Chunk chunk;

    size_t one = chunk.addConstant(Value(static_cast<int64_t>(1)));
    size_t two = chunk.addConstant(Value(static_cast<int64_t>(2)));
    size_t idx1 = chunk.addConstant(Value(static_cast<int64_t>(1)));
    size_t mAppend = chunk.addConstant(Value(std::string("append")));

    // list = [1]
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(one)));
    chunk.addInstruction(Instruction(OpCode::BUILD_LIST, static_cast<uint32_t>(1)));

    // list.append(2)
    // GET_ATTR: pushes (callable, receiver)
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(mAppend)));
    chunk.addInstruction(Instruction(OpCode::GET_ATTR));
    // argument 2
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(two)));
    // CALL with argCount = 2 (receiver + 1 arg)
    chunk.addInstruction(Instruction(OpCode::CALL, static_cast<uint32_t>(2)));

    // Assert list[1] == 2
    chunk.addInstruction(Instruction(OpCode::DUP));
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(idx1)));
    chunk.addInstruction(Instruction(OpCode::GET_ITEM));
    chunk.addInstruction(Instruction(OpCode::HALT));

    VM vm;
    bool ok = vm.interpret(chunk);
    EXPECT_TRUE(ok);
    Value top = vm.peek();
    EXPECT_TRUE(top.isInteger());
    EXPECT_EQ(2, top.asInteger());
}

// dict.keys() returns list of keys
TEST(Methods, DictKeys) {
    Chunk chunk;

    size_t ka = chunk.addConstant(Value(std::string("a")));
    size_t va = chunk.addConstant(Value(static_cast<int64_t>(1)));
    size_t kb = chunk.addConstant(Value(std::string("b")));
    size_t vb = chunk.addConstant(Value(static_cast<int64_t>(2)));
    size_t mKeys = chunk.addConstant(Value(std::string("keys")));
    size_t mSorted = addNative(chunk, "sorted");
    size_t idx1 = chunk.addConstant(Value(static_cast<int64_t>(1)));

    // dict = {"a":1, "b":2}
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(ka)));
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(va)));
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(kb)));
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(vb)));
    chunk.addInstruction(Instruction(OpCode::BUILD_DICT, static_cast<uint32_t>(2)));

    // dict.keys() -> (callable, receiver)
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(mKeys)));
    chunk.addInstruction(Instruction(OpCode::GET_ATTR));
    chunk.addInstruction(Instruction(OpCode::CALL, static_cast<uint32_t>(1))); // receiver only

    // sorted(keys)
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(mSorted)));
    chunk.addInstruction(Instruction(OpCode::CALL, static_cast<uint32_t>(1)));

    // Assert: second key is "b"
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(idx1)));
    chunk.addInstruction(Instruction(OpCode::GET_ITEM));
    chunk.addInstruction(Instruction(OpCode::HALT));

    VM vm;
    bool ok = vm.interpret(chunk);
    EXPECT_TRUE(ok);
    Value top = vm.peek();
    EXPECT_TRUE(top.isString());
    EXPECT_EQ(std::string("b"), top.asString());
}

RUN_ALL_TESTS();
