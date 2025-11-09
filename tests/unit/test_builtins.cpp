// test_builtins.cpp - Tests for builtin functions (extend, len, any, all)

#include "TestFramework.h"
#include "VM.h"
#include "Bytecode.h"

using namespace rglite;

// Helper: add native function constant
static size_t addNative(Chunk& chunk, const std::string& name) {
    return chunk.addConstant(Value(name, true));
}

// extend(list, list)
TEST(Builtins, ExtendWithList) {
    Chunk chunk;

    size_t c1 = chunk.addConstant(Value(static_cast<int64_t>(1)));
    size_t c2 = chunk.addConstant(Value(static_cast<int64_t>(2)));
    size_t c3 = chunk.addConstant(Value(static_cast<int64_t>(3)));
    size_t idx1 = chunk.addConstant(Value(static_cast<int64_t>(1)));
    size_t fExtend = addNative(chunk, "extend");

    // list1 = [1]
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(c1)));
    chunk.addInstruction(Instruction(OpCode::BUILD_LIST, static_cast<uint32_t>(1)));

    // list2 = [2, 3]
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(c2)));
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(c3)));
    chunk.addInstruction(Instruction(OpCode::BUILD_LIST, static_cast<uint32_t>(2)));

    // extend(list1, list2)
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(fExtend)));
    chunk.addInstruction(Instruction(OpCode::CALL, static_cast<uint32_t>(2)));

    // Assert: element at index 1 is 2
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

// extend(list, tuple)
TEST(Builtins, ExtendWithTuple) {
    Chunk chunk;

    size_t c1 = chunk.addConstant(Value(static_cast<int64_t>(1)));
    size_t c2 = chunk.addConstant(Value(static_cast<int64_t>(2)));
    size_t c3 = chunk.addConstant(Value(static_cast<int64_t>(3)));
    size_t idx2 = chunk.addConstant(Value(static_cast<int64_t>(2)));
    size_t fExtend = addNative(chunk, "extend");

    // list1 = [1]
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(c1)));
    chunk.addInstruction(Instruction(OpCode::BUILD_LIST, static_cast<uint32_t>(1)));

    // tuple = (2, 3)
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(c2)));
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(c3)));
    chunk.addInstruction(Instruction(OpCode::BUILD_TUPLE, static_cast<uint32_t>(2)));

    // extend(list1, tuple)
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(fExtend)));
    chunk.addInstruction(Instruction(OpCode::CALL, static_cast<uint32_t>(2)));

    // Assert: element at index 2 is 3
    chunk.addInstruction(Instruction(OpCode::DUP));
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(idx2)));
    chunk.addInstruction(Instruction(OpCode::GET_ITEM));
    chunk.addInstruction(Instruction(OpCode::HALT));

    VM vm;
    bool ok = vm.interpret(chunk);
    EXPECT_TRUE(ok);
    Value top = vm.peek();
    EXPECT_TRUE(top.isInteger());
    EXPECT_EQ(3, top.asInteger());
}

// extend(list, set) -> order not guaranteed; assert membership count
TEST(Builtins, ExtendWithSet) {
    Chunk chunk;

    size_t c1 = chunk.addConstant(Value(static_cast<int64_t>(1)));
    size_t c2 = chunk.addConstant(Value(static_cast<int64_t>(2)));
    size_t c3 = chunk.addConstant(Value(static_cast<int64_t>(3)));
    size_t fExtend = addNative(chunk, "extend");
    size_t fCount = addNative(chunk, "count");

    // list1 = [1]
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(c1)));
    chunk.addInstruction(Instruction(OpCode::BUILD_LIST, static_cast<uint32_t>(1)));

    // set = {2, 3}
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(c2)));
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(c3)));
    chunk.addInstruction(Instruction(OpCode::BUILD_SET, static_cast<uint32_t>(2)));

    // extend(list1, set)
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(fExtend)));
    chunk.addInstruction(Instruction(OpCode::CALL, static_cast<uint32_t>(2)));

    // count(list1, 2)
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(c2))); // target
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(fCount)));
    chunk.addInstruction(Instruction(OpCode::CALL, static_cast<uint32_t>(2)));
    chunk.addInstruction(Instruction(OpCode::HALT));

    VM vm;
    bool ok = vm.interpret(chunk);
    EXPECT_TRUE(ok);
    Value top = vm.peek();
    EXPECT_TRUE(top.isInteger());
    EXPECT_EQ(1, top.asInteger());
}

// extend(list, string) -> appends characters
TEST(Builtins, ExtendWithString) {
    Chunk chunk;

    size_t s = chunk.addConstant(Value(std::string("ab")));
    size_t idx1 = chunk.addConstant(Value(static_cast<int64_t>(1)));
    size_t fExtend = addNative(chunk, "extend");

    // list = []
    chunk.addInstruction(Instruction(OpCode::BUILD_LIST, static_cast<uint32_t>(0)));

    // extend(list, "ab")
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(s)));
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(fExtend)));
    chunk.addInstruction(Instruction(OpCode::CALL, static_cast<uint32_t>(2)));

    // Assert second char == "b"
    chunk.addInstruction(Instruction(OpCode::DUP));
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

// extend(list, dict) -> appends keys; verify via sorted
TEST(Builtins, ExtendWithDictKeys) {
    Chunk chunk;

    size_t kx = chunk.addConstant(Value(std::string("x")));
    size_t vx = chunk.addConstant(Value(static_cast<int64_t>(1)));
    size_t ky = chunk.addConstant(Value(std::string("y")));
    size_t vy = chunk.addConstant(Value(static_cast<int64_t>(2)));
    size_t idx1 = chunk.addConstant(Value(static_cast<int64_t>(1)));
    size_t fExtend = addNative(chunk, "extend");
    size_t fSorted = addNative(chunk, "sorted");

    // list = []
    chunk.addInstruction(Instruction(OpCode::BUILD_LIST, static_cast<uint32_t>(0)));

    // dict = {"x":1, "y":2}
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(kx)));
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(vx)));
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(ky)));
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(vy)));
    chunk.addInstruction(Instruction(OpCode::BUILD_DICT, static_cast<uint32_t>(2)));

    // extend(list, dict)
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(fExtend)));
    chunk.addInstruction(Instruction(OpCode::CALL, static_cast<uint32_t>(2)));

    // sorted(keys)
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(fSorted)));
    chunk.addInstruction(Instruction(OpCode::CALL, static_cast<uint32_t>(1)));

    // Assert: second key is "y"
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(idx1)));
    chunk.addInstruction(Instruction(OpCode::GET_ITEM));
    chunk.addInstruction(Instruction(OpCode::HALT));

    VM vm;
    bool ok = vm.interpret(chunk);
    EXPECT_TRUE(ok);
    Value top = vm.peek();
    EXPECT_TRUE(top.isString());
    EXPECT_EQ(std::string("y"), top.asString());
}

// extend type error: non-iterable second argument
TEST(Builtins, ExtendTypeError) {
    Chunk chunk;

    size_t one = chunk.addConstant(Value(static_cast<int64_t>(1)));
    size_t five = chunk.addConstant(Value(static_cast<int64_t>(5)));
    size_t fExtend = addNative(chunk, "extend");

    // list = [1]
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(one)));
    chunk.addInstruction(Instruction(OpCode::BUILD_LIST, static_cast<uint32_t>(1)));

    // extend(list, 5) -> type error
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(five)));
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(fExtend)));
    chunk.addInstruction(Instruction(OpCode::CALL, static_cast<uint32_t>(2)));
    chunk.addInstruction(Instruction(OpCode::HALT));

    VM vm;
    bool ok = vm.interpret(chunk);
    EXPECT_FALSE(ok);
    EXPECT_TRUE(vm.hasException());
}

// any([]) over string "" => false
TEST(Builtins, AnyEmptyString) {
    Chunk chunk;

    size_t empty = chunk.addConstant(Value(std::string("")));
    size_t fAny = addNative(chunk, "any");

    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(empty)));
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(fAny)));
    chunk.addInstruction(Instruction(OpCode::CALL, static_cast<uint32_t>(1)));
    chunk.addInstruction(Instruction(OpCode::HALT));

    VM vm;
    bool ok = vm.interpret(chunk);
    EXPECT_TRUE(ok);
    Value top = vm.peek();
    EXPECT_TRUE(top.isBoolean());
    EXPECT_FALSE(top.asBoolean());
}

// all([1, "a", true]) => true
TEST(Builtins, AllTruthyList) {
    Chunk chunk;

    size_t one = chunk.addConstant(Value(static_cast<int64_t>(1)));
    size_t a = chunk.addConstant(Value(std::string("a")));
    size_t tru = chunk.addConstant(Value(true));
    size_t fAll = addNative(chunk, "all");

    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(one)));
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(a)));
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(tru)));
    chunk.addInstruction(Instruction(OpCode::BUILD_LIST, static_cast<uint32_t>(3)));

    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(fAll)));
    chunk.addInstruction(Instruction(OpCode::CALL, static_cast<uint32_t>(1)));
    chunk.addInstruction(Instruction(OpCode::HALT));

    VM vm;
    bool ok = vm.interpret(chunk);
    EXPECT_TRUE(ok);
    Value top = vm.peek();
    EXPECT_TRUE(top.isBoolean());
    EXPECT_TRUE(top.asBoolean());
}

// len("hello") => 5
TEST(Builtins, LenString) {
    Chunk chunk;

    size_t s = chunk.addConstant(Value(std::string("hello")));
    size_t fLen = addNative(chunk, "len");

    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(s)));
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(fLen)));
    chunk.addInstruction(Instruction(OpCode::CALL, static_cast<uint32_t>(1)));
    chunk.addInstruction(Instruction(OpCode::HALT));

    VM vm;
    bool ok = vm.interpret(chunk);
    EXPECT_TRUE(ok);
    Value top = vm.peek();
    EXPECT_TRUE(top.isInteger());
    EXPECT_EQ(5, top.asInteger());
}

RUN_ALL_TESTS();

