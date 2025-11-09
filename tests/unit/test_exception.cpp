// test_exception.cpp - Test exception handling in RGLite VM
#include "TestFramework.h"
#include "VM.h"
#include "Bytecode.h"
#include <iostream>
#include <memory>

using namespace rglite;

// Test basic try-catch with runtime exception
TEST(ExceptionTests, BasicTryCatch) {
    // Create a chunk with arithmetic operations
    Chunk chunk;
    
    // Add constants
    size_t constErrorMsg = chunk.addConstant(Value(std::string("Test runtime error")));
    
    // Instructions:
    // try {
    //   throw "Test runtime error"
    // } catch (e) {
    //   print "Caught exception"
    // }
    size_t handlerIndex = 4; // Jump to handler if exception occurs
    
    chunk.addInstruction(Instruction(OpCode::PUSH_HANDLER, static_cast<uint32_t>(handlerIndex)));
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(constErrorMsg)));
    chunk.addInstruction(Instruction(OpCode::THROW, 0));  // Throw using value from stack
    chunk.addInstruction(Instruction(OpCode::JUMP, static_cast<uint32_t>(6))); // Jump over handler if no exception
    // Handler starts here
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(chunk.addConstant(Value(std::string("Caught exception"))))));
    chunk.addInstruction(Instruction(OpCode::PRINT));
    chunk.addInstruction(Instruction(OpCode::POP_HANDLER)); // Pop handler after handling
    chunk.addInstruction(Instruction(OpCode::HALT));
    
    VM vm;
    bool result = vm.interpret(chunk);
    
    EXPECT_TRUE(result);
}

// Test nested try-catch
TEST(ExceptionTests, NestedTryCatch) {
    Chunk chunk;
    
    // Add constants
    size_t constInnerMsg = chunk.addConstant(Value(std::string("Inner error")));
    
    // Instructions:
    // try {
    //   try {
    //     throw "Inner error"
    //   } catch (e) {
    //     print "Caught inner"
    //     throw e  // Rethrow to outer handler
    //   }
    // } catch (e) {
    //   print "Caught outer"
    // }
    
    // Outer handler at position 13
    chunk.addInstruction(Instruction(OpCode::PUSH_HANDLER, 13));
    // Inner handler at position 5 (after JUMP instruction)
    chunk.addInstruction(Instruction(OpCode::PUSH_HANDLER, 5));
    // Throw inner exception
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(constInnerMsg)));
    chunk.addInstruction(Instruction(OpCode::THROW, 0));  // Throw using value from stack
    // Jump over inner handler if no exception
    chunk.addInstruction(Instruction(OpCode::JUMP, 5));
    // Inner handler: just print and rethrow
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(chunk.addConstant(Value(std::string("Caught inner"))))));
    chunk.addInstruction(Instruction(OpCode::PRINT));
    chunk.addInstruction(Instruction(OpCode::POP_HANDLER)); // Pop inner handler
    chunk.addInstruction(Instruction(OpCode::THROW, 0));  // Rethrow using value from stack
    // Outer handler: print and halt
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(chunk.addConstant(Value(std::string("Caught outer"))))));
    chunk.addInstruction(Instruction(OpCode::PRINT));
    chunk.addInstruction(Instruction(OpCode::POP_HANDLER)); // Pop outer handler
    chunk.addInstruction(Instruction(OpCode::HALT));
    VM vm;
    bool result = vm.interpret(chunk);
    EXPECT_TRUE(result);
}

// Test exception with no handler
TEST(ExceptionTests, UnhandledException) {
    // Create a chunk with arithmetic operations
    Chunk chunk;
    
    // Add constants
    size_t constErrorMsg = chunk.addConstant(Value(std::string("Unhandled error")));
    
    // Instructions:
    // throw "Unhandled error"
    // No handler is set up, so this should cause a runtime error
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(constErrorMsg)));
    chunk.addInstruction(Instruction(OpCode::THROW));
    chunk.addInstruction(Instruction(OpCode::HALT));
    
    VM vm;
    bool result = vm.interpret(chunk);
    
    EXPECT_FALSE(result);
}

// Test exception propagation
TEST(ExceptionTests, ExceptionPropagation) {
    // Create a chunk with arithmetic operations
    Chunk chunk;
    
    // Add constants
    size_t constErrorMsg = chunk.addConstant(Value(std::string("Error in function")));
    size_t constResult = chunk.addConstant(Value(std::string("Error handled")));
    
    // Instructions:
    // try {
    //   call function_that_throws()
    // } catch (e) {
    //   print "Error handled"
    // }
    size_t handlerIndex = 5; // Jump to handler if exception occurs
    
    // Try block
    chunk.addInstruction(Instruction(OpCode::PUSH_HANDLER, static_cast<uint32_t>(handlerIndex)));
    
    // Simulate a function call that throws
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(constErrorMsg)));
    chunk.addInstruction(Instruction(OpCode::THROW, 0));  // Throw using value from stack
    
    // Jump over handler if no exception
    chunk.addInstruction(Instruction(OpCode::JUMP, static_cast<uint32_t>(5)));
    
    // Handler
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, static_cast<uint32_t>(constResult)));
    chunk.addInstruction(Instruction(OpCode::PRINT));
    chunk.addInstruction(Instruction(OpCode::POP_HANDLER)); // Pop handler after handling
    chunk.addInstruction(Instruction(OpCode::HALT));
    
    VM vm;
    bool result = vm.interpret(chunk);
    
    EXPECT_TRUE(result);
}

// Main test runner
RUN_ALL_TESTS()