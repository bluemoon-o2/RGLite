// test_vm.cpp - Test cases for the Virtual Machine
// This file contains test cases for the VM implementation

#include "TestFramework.h"
#include "VM.h"
#include "Bytecode.h"
#include <iostream>
#include <memory>

using namespace rglite;

// Test basic arithmetic operations
TEST(VMTests, ArithmeticOperations) {
    // Create a chunk with arithmetic operations
    Chunk chunk;
    
    // Add constants
    size_t const1 = chunk.addConstant(Value(static_cast<int64_t>(10)));
    size_t const2 = chunk.addConstant(Value(static_cast<int64_t>(5)));
    
    // Instructions: 10 + 5
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(const1)));
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(const2)));
    chunk.addInstruction(Instruction(OpCode::ADD));
    chunk.addInstruction(Instruction(OpCode::HALT));
    
    // Create VM and interpret the chunk
    VM vm;
    bool result = vm.interpret(chunk);
    
    EXPECT_TRUE(result);
    
    // Check the result on the stack (should be 15)
    if (vm.getStackSize() > 0) {
        Value stackTop = vm.peek();
        EXPECT_TRUE(stackTop.isInteger());
        EXPECT_EQ(15, stackTop.asInteger());
    }
}

// Test comparison operations
TEST(VMTests, ComparisonOperations) {
    // Create a chunk with comparison operations
    Chunk chunk;
    
    // Add constants
    size_t const1 = chunk.addConstant(Value(static_cast<int64_t>(10)));
    size_t const2 = chunk.addConstant(Value(static_cast<int64_t>(5)));
    
    // Instructions: 10 > 5
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(const1)));
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(const2)));
    chunk.addInstruction(Instruction(OpCode::GT));
    chunk.addInstruction(Instruction(OpCode::HALT));
    
    // Create VM and interpret the chunk
    VM vm;
    bool result = vm.interpret(chunk);
    
    EXPECT_TRUE(result);
    
    // Check the result on the stack (should be true)
    if (vm.getStackSize() > 0) {
        Value stackTop = vm.peek();
        EXPECT_TRUE(stackTop.isBoolean());
        EXPECT_TRUE(stackTop.asBoolean());
    }
}

// Test logical operations
TEST(VMTests, LogicalOperations) {
    // Create a chunk with logical operations
    Chunk chunk;
    
    // Add constants
    size_t constTrue = chunk.addConstant(Value(true));
    size_t constFalse = chunk.addConstant(Value(false));
    
    // Instructions: true && false
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(constTrue)));
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(constFalse)));
    chunk.addInstruction(Instruction(OpCode::AND));
    chunk.addInstruction(Instruction(OpCode::HALT));
    
    // Create VM and interpret the chunk
    VM vm;
    bool result = vm.interpret(chunk);
    
    EXPECT_TRUE(result);
    
    // Check the result on the stack (should be false)
    if (vm.getStackSize() > 0) {
        Value stackTop = vm.peek();
        EXPECT_TRUE(stackTop.isBoolean());
        EXPECT_FALSE(stackTop.asBoolean());
    }
}

// Test control flow
TEST(VMTests, ControlFlow) {
    // Create a chunk with control flow
    Chunk chunk;
    
    // Add constants
    size_t constTrue = chunk.addConstant(Value(true));
    size_t const1 = chunk.addConstant(Value(static_cast<int64_t>(1)));
    size_t const2 = chunk.addConstant(Value(static_cast<int64_t>(2)));
    
    // Instructions:
    // if true:
    //   result = 1
    // else:
    //   result = 2
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(constTrue)));
    chunk.addInstruction(Instruction(OpCode::JUMP_IF_FALSE, 3)); // Jump to else part if false
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(const1))); // then part
    chunk.addInstruction(Instruction(OpCode::JUMP, 1)); // Skip else part
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(const2))); // else part
    chunk.addInstruction(Instruction(OpCode::HALT));
    
    // Create VM and interpret the chunk
    VM vm;
    bool result = vm.interpret(chunk);
    
    EXPECT_TRUE(result);
    
    // Check the result on the stack (should be 1)
    if (vm.getStackSize() > 0) {
        Value stackTop = vm.peek();
        EXPECT_TRUE(stackTop.isInteger());
        EXPECT_EQ(1, stackTop.asInteger());
    }
}

// Test variable operations
TEST(VMTests, VariableOperations) {
    // Create a chunk with variable operations
    Chunk chunk;
    
    // Add constants
    size_t constVarName = chunk.addConstant(Value(std::string("x")));
    size_t constValue = chunk.addConstant(Value(static_cast<int64_t>(42)));
    
    // Instructions:
    // x = 42
    // load x
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(constValue)));
    chunk.addInstruction(Instruction(OpCode::STORE_VAR, static_cast<uint32_t>(constVarName)));
    chunk.addInstruction(Instruction(OpCode::LOAD_VAR, static_cast<uint32_t>(constVarName)));
    chunk.addInstruction(Instruction(OpCode::HALT));
    
    // Create VM and interpret the chunk
    VM vm;
    bool result = vm.interpret(chunk);
    
    EXPECT_TRUE(result);
    
    // Check the result on the stack (should be 42)
    if (vm.getStackSize() > 0) {
        Value stackTop = vm.peek();
        EXPECT_TRUE(stackTop.isInteger());
        EXPECT_EQ(42, stackTop.asInteger());
    }
}

// Test print operation
TEST(VMTests, PrintOperation) {
    // Create a chunk with print operation
    Chunk chunk;
    
    // Add constants
    size_t constMessage = chunk.addConstant(Value(std::string("Hello, RGLite VM!")));
    
    // Instructions: print "Hello, RGLite VM!"
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(constMessage)));
    chunk.addInstruction(Instruction(OpCode::PRINT));
    chunk.addInstruction(Instruction(OpCode::HALT));
    
    // Create VM and interpret the chunk
    VM vm;
    bool result = vm.interpret(chunk);
    
    EXPECT_TRUE(result);
}

// Test stack operations
TEST(VMTests, StackOperations) {
    // Create a chunk with stack operations
    Chunk chunk;
    
    // Add constants
    size_t const1 = chunk.addConstant(Value(static_cast<int64_t>(1)));
    size_t const2 = chunk.addConstant(Value(static_cast<int64_t>(2)));
    size_t const3 = chunk.addConstant(Value(static_cast<int64_t>(3)));
    
    // Instructions:
    // push 1, 2, 3
    // pop 3
    // duplicate top (2)
    // add (2 + 2 = 4)
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(const1)));
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(const2)));
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(const3)));
    chunk.addInstruction(Instruction(OpCode::POP));
    chunk.addInstruction(Instruction(OpCode::DUP));
    chunk.addInstruction(Instruction(OpCode::ADD));
    chunk.addInstruction(Instruction(OpCode::HALT));
    
    // Create VM and interpret the chunk
    VM vm;
    bool result = vm.interpret(chunk);
    
    EXPECT_TRUE(result);
    
    // Check the result on the stack (should be 4)
    if (vm.getStackSize() > 0) {
        Value stackTop = vm.peek();
        EXPECT_TRUE(stackTop.isInteger());
        EXPECT_EQ(4, stackTop.asInteger());
    }
}

// Test 'in' operator error scenarios
TEST(VMTests, InOperatorErrorScenarios) {
    // Test 1: Non-iterable container type
    {
        Chunk chunk;
        
        // Add constants: number 42 and integer 1
        size_t constNumber = chunk.addConstant(Value(static_cast<int64_t>(42)));
        size_t constItem = chunk.addConstant(Value(static_cast<int64_t>(1)));
        
        // Instructions: 1 in 42 (should fail - integer is not iterable)
        chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(constNumber)));
        chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(constItem)));
        chunk.addInstruction(Instruction(OpCode::CONTAINS));
        chunk.addInstruction(Instruction(OpCode::HALT));
        
        VM vm;
        bool result = vm.interpret(chunk);
        
        EXPECT_FALSE(result); // Should fail with type error
        EXPECT_TRUE(vm.hasException());
        if (vm.hasException()) {
            Exception ex = vm.getException();
            EXPECT_EQ(ExceptionType::TYPE_ERROR, ex.getType());
            EXPECT_NE(std::string::npos, ex.getMessage().find("non-iterable"));
        }
    }
    
    // Test 2: Non-string key for dictionary
    {
        Chunk chunk;
        
        // Create a dictionary
        size_t constDict = chunk.addConstant(Value(std::string("dict")));
        size_t constKey = chunk.addConstant(Value(static_cast<int64_t>(123))); // Integer key
        
        // Instructions: 123 in dict (should fail - non-string key)
        chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(constDict)));
        chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(constKey)));
        chunk.addInstruction(Instruction(OpCode::CONTAINS));
        chunk.addInstruction(Instruction(OpCode::HALT));
        
        VM vm;
        bool result = vm.interpret(chunk);
        
        EXPECT_FALSE(result); // Should fail with type error
        EXPECT_TRUE(vm.hasException());
        if (vm.hasException()) {
            Exception ex = vm.getException();
            EXPECT_EQ(ExceptionType::TYPE_ERROR, ex.getType());
            EXPECT_NE(std::string::npos, ex.getMessage().find("non-string key"));
        }
    }
    
    // Test 3: Non-string operand for string containment
    {
        Chunk chunk;
        
        // Add constants: string "hello" and integer 42
        size_t constString = chunk.addConstant(Value(std::string("hello")));
        size_t constNumber = chunk.addConstant(Value(static_cast<int64_t>(42)));
        
        // Instructions: 42 in "hello" (should fail - non-string operand)
        chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(constString)));
        chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(constNumber)));
        chunk.addInstruction(Instruction(OpCode::CONTAINS));
        chunk.addInstruction(Instruction(OpCode::HALT));
        
        VM vm;
        bool result = vm.interpret(chunk);
        
        EXPECT_FALSE(result); // Should fail with type error
        EXPECT_TRUE(vm.hasException());
        if (vm.hasException()) {
            Exception ex = vm.getException();
            EXPECT_EQ(ExceptionType::TYPE_ERROR, ex.getType());
            EXPECT_NE(std::string::npos, ex.getMessage().find("non-string operand"));
        }
    }
    
    // Test 4: Stack underflow - not enough operands
    {
        Chunk chunk;
        
        // Add constant: string "hello"
        size_t constString = chunk.addConstant(Value(std::string("hello")));
        
        // Instructions: only push container, no item
        chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(constString)));
        chunk.addInstruction(Instruction(OpCode::CONTAINS)); // Missing item operand
        chunk.addInstruction(Instruction(OpCode::HALT));
        
        VM vm;
        bool result = vm.interpret(chunk);
        
        EXPECT_FALSE(result); // Should fail with runtime error
        EXPECT_TRUE(vm.hasException());
        if (vm.hasException()) {
            Exception ex = vm.getException();
            EXPECT_EQ(ExceptionType::RUNTIME_ERROR, ex.getType());
            EXPECT_NE(std::string::npos, ex.getMessage().find("Not enough operands"));
        }
    }
}

// Test 'in' operator success scenarios
TEST(VMTests, InOperatorSuccessScenarios) {
    // Test 1: String containment
    {
        Chunk chunk;
        
        // Add constants: string "hello world" and substring "world"
        size_t constString = chunk.addConstant(Value(std::string("hello world")));
        size_t constSubstring = chunk.addConstant(Value(std::string("world")));
        
        // Instructions: "world" in "hello world"
        chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(constString)));
        chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(constSubstring)));
        chunk.addInstruction(Instruction(OpCode::CONTAINS));
        chunk.addInstruction(Instruction(OpCode::HALT));
        
        VM vm;
        bool result = vm.interpret(chunk);
        
        EXPECT_TRUE(result);
        EXPECT_FALSE(vm.hasException());
        
        // Check the result on the stack (should be true)
        if (vm.getStackSize() > 0) {
            Value stackTop = vm.peek();
            EXPECT_TRUE(stackTop.isBoolean());
            EXPECT_TRUE(stackTop.asBoolean());
        }
    }
    
    // Test 2: String non-containment
    {
        Chunk chunk;
        
        // Add constants: string "hello" and substring "xyz"
        size_t constString = chunk.addConstant(Value(std::string("hello")));
        size_t constSubstring = chunk.addConstant(Value(std::string("xyz")));
        
        // Instructions: "xyz" in "hello"
        chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(constString)));
        chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(constSubstring)));
        chunk.addInstruction(Instruction(OpCode::CONTAINS));
        chunk.addInstruction(Instruction(OpCode::HALT));
        
        VM vm;
        bool result = vm.interpret(chunk);
        
        EXPECT_TRUE(result);
        EXPECT_FALSE(vm.hasException());
        
        // Check the result on the stack (should be false)
        if (vm.getStackSize() > 0) {
            Value stackTop = vm.peek();
            EXPECT_TRUE(stackTop.isBoolean());
            EXPECT_FALSE(stackTop.asBoolean());
        }
    }
}

// Test attribute assignment on dictionary using SET_ATTR
TEST(VMTests, AttributeAssignmentDict) {
    Chunk chunk;

    // Build empty dict and store to a variable
    chunk.addInstruction(Instruction(OpCode::BUILD_DICT, 0));
    size_t constVarName = chunk.addConstant(Value(std::string("d")));
    chunk.addInstruction(Instruction(OpCode::STORE_VAR, static_cast<uint32_t>(constVarName)));

    // Load dict, attribute name, and value, then assign
    chunk.addInstruction(Instruction(OpCode::LOAD_VAR, static_cast<uint32_t>(constVarName))); // object
    size_t constAttr = chunk.addConstant(Value(std::string("x")));
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(constAttr)));   // attribute
    size_t constVal = chunk.addConstant(Value(static_cast<int64_t>(42)));
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(constVal)));    // value
    chunk.addInstruction(Instruction(OpCode::SET_ATTR));

    // Read back the attribute to verify
    chunk.addInstruction(Instruction(OpCode::LOAD_VAR, static_cast<uint32_t>(constVarName))); // object
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(constAttr)));   // attribute
    chunk.addInstruction(Instruction(OpCode::GET_ATTR));
    chunk.addInstruction(Instruction(OpCode::HALT));

    VM vm;
    bool result = vm.interpret(chunk);

    EXPECT_TRUE(result);
    EXPECT_FALSE(vm.hasException());

    // Top of stack should be 42
    if (vm.getStackSize() > 0) {
        Value stackTop = vm.peek();
        EXPECT_TRUE(stackTop.isInteger());
        EXPECT_EQ(42, stackTop.asInteger());
    }
}

// Main test runner
RUN_ALL_TESTS()
