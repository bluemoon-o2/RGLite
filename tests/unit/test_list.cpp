// test_list.cpp - Test cases for list operations in RGLite
// This file contains test cases for list creation, access, and manipulation

#include "TestFramework.h"
#include "VM.h"
#include "Bytecode.h"
#include "Exception.h"
#include <iostream>
#include <memory>

using namespace rglite;

// Test list creation
TEST(ListTests, CreateEmptyList) {
    Chunk chunk;
    
    // Create an empty list: []
    chunk.addInstruction(Instruction(OpCode::BUILD_LIST, static_cast<uint32_t>(0)));
    chunk.addInstruction(Instruction(OpCode::PRINT));
    chunk.addInstruction(Instruction(OpCode::HALT));
    
    VM vm;
    bool result = vm.interpret(chunk);
    
    EXPECT_TRUE(result);
}

// Test list creation with items
TEST(ListTests, CreateListWithItems) {
    Chunk chunk;
    
    // Create a list with three integers: [1, 2, 3]
    chunk.addConstant(Value(static_cast<int64_t>(1)));
    chunk.addConstant(Value(static_cast<int64_t>(2)));
    chunk.addConstant(Value(static_cast<int64_t>(3)));
    
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, 0));
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, 1));
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, 2));
    chunk.addInstruction(Instruction(OpCode::BUILD_LIST, static_cast<uint32_t>(3)));
    chunk.addInstruction(Instruction(OpCode::PRINT));
    chunk.addInstruction(Instruction(OpCode::HALT));
    
    VM vm;
    bool result = vm.interpret(chunk);
    
    EXPECT_TRUE(result);
}

// Test list item access
TEST(ListTests, AccessListItem) {
    Chunk chunk;
    
    // Create a list and access the second item
    chunk.addConstant(Value(static_cast<int64_t>(1)));
    chunk.addConstant(Value(static_cast<int64_t>(2)));
    chunk.addConstant(Value(static_cast<int64_t>(3)));
    chunk.addConstant(Value(static_cast<int64_t>(1))); // Index
    
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, 0));
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, 1));
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, 2));
    chunk.addInstruction(Instruction(OpCode::BUILD_LIST, static_cast<uint32_t>(3)));
    chunk.addInstruction(Instruction(OpCode::DUP));
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, 3));
    chunk.addInstruction(Instruction(OpCode::GET_ITEM));
    chunk.addInstruction(Instruction(OpCode::PRINT));
    chunk.addInstruction(Instruction(OpCode::HALT));
    
    VM vm;
    bool result = vm.interpret(chunk);
    
    EXPECT_TRUE(result);
}

// Test list item setting
TEST(ListTests, SetListItem) {
    Chunk chunk;
    
    // Create a list, set the second item, and print the list
    chunk.addConstant(Value(static_cast<int64_t>(1)));
    chunk.addConstant(Value(static_cast<int64_t>(2)));
    chunk.addConstant(Value(static_cast<int64_t>(3)));
    chunk.addConstant(Value(static_cast<int64_t>(1))); // Index
    chunk.addConstant(Value(static_cast<int64_t>(99))); // New value
    
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, 0));
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, 1));
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, 2));
    chunk.addInstruction(Instruction(OpCode::BUILD_LIST, static_cast<uint32_t>(3)));
    chunk.addInstruction(Instruction(OpCode::DUP));
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, 3));
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, 4));
    chunk.addInstruction(Instruction(OpCode::SET_ITEM));
    chunk.addInstruction(Instruction(OpCode::PRINT));
    chunk.addInstruction(Instruction(OpCode::HALT));
    
    VM vm;
    bool result = vm.interpret(chunk);
    
    EXPECT_TRUE(result);
}

// Test list length
TEST(ListTests, ListLength) {
    std::cerr << "=== Starting ListLength test ===" << std::endl;
    Chunk chunk;
    
    // Create a list and get its length
    chunk.addConstant(Value(static_cast<int64_t>(1)));
    chunk.addConstant(Value(static_cast<int64_t>(2)));
    chunk.addConstant(Value(static_cast<int64_t>(3)));
    chunk.addConstant(Value(std::string("len"))); // Variable name - explicitly create a string
    
    std::cerr << "Constants added to chunk" << std::endl;
    
    // Build the list first
    std::cerr << "Adding LOAD_CONST instructions..." << std::endl;
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, 0));
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, 1));
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, 2));
    std::cerr << "Adding BUILD_LIST instruction..." << std::endl;
    chunk.addInstruction(Instruction(OpCode::BUILD_LIST, static_cast<uint32_t>(3)));
    std::cerr << "Adding DUP instruction..." << std::endl;
    chunk.addInstruction(Instruction(OpCode::DUP)); // Duplicate the list for debugging
    std::cerr << "Adding PRINT instruction..." << std::endl;
    chunk.addInstruction(Instruction(OpCode::PRINT)); // Print the list
    
    // Load the len function
    std::cerr << "Adding LOAD_VAR instruction..." << std::endl;
    chunk.addInstruction(Instruction(OpCode::LOAD_VAR, 3));
    
    // Call the len function with the list as argument
    std::cerr << "Adding CALL instruction..." << std::endl;
    chunk.addInstruction(Instruction(OpCode::CALL, static_cast<uint32_t>(1)));
    std::cerr << "Adding PRINT instruction..." << std::endl;
    chunk.addInstruction(Instruction(OpCode::PRINT));
    std::cerr << "Adding HALT instruction..." << std::endl;
    chunk.addInstruction(Instruction(OpCode::HALT));
    
    std::cerr << "Instructions added to chunk" << std::endl;
    
    // Debug: Print the instruction sequence
    std::cerr << "Instruction sequence:" << std::endl;
    for (size_t i = 0; i < chunk.getInstructions().size(); ++i) {
        const auto& instr = chunk.getInstruction(i);
        std::cerr << "  IP " << i << ": " << static_cast<int>(instr.opcode) 
                  << " (operand: " << instr.operand << ")" << std::endl;
    }
    
    VM vm;
    std::cerr << "=== VM created, starting interpretation ===" << std::endl;
    bool result = vm.interpret(chunk);
    std::cerr << "=== Interpretation completed with result: " << result << " ===" << std::endl;
    
    EXPECT_TRUE(result);
    std::cerr << "=== ListLength test completed ===" << std::endl;
}

// Test list index out of bounds
TEST(ListTests, ListIndexOutOfBounds) {
    Chunk chunk;
    
    // Create a list and try to access an item at an invalid index
    chunk.addConstant(Value(static_cast<int64_t>(1)));
    chunk.addConstant(Value(static_cast<int64_t>(2)));
    chunk.addConstant(Value(static_cast<int64_t>(3)));
    chunk.addConstant(Value(static_cast<int64_t>(5))); // Out of bounds index
    
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, 0));
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, 1));
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, 2));
    chunk.addInstruction(Instruction(OpCode::BUILD_LIST, static_cast<uint32_t>(3)));
    chunk.addInstruction(Instruction(OpCode::DUP));
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, 3));
    chunk.addInstruction(Instruction(OpCode::GET_ITEM));
    chunk.addInstruction(Instruction(OpCode::PRINT));
    chunk.addInstruction(Instruction(OpCode::HALT));
    
    VM vm;
    bool result = vm.interpret(chunk);
    
    EXPECT_FALSE(result);
}

// Test list with mixed types
TEST(ListTests, ListWithMixedTypes) {
    Chunk chunk;
    
    // Create a list with mixed types: [1, "hello", true]
    chunk.addConstant(Value(static_cast<int64_t>(1)));
    chunk.addConstant(Value(std::string("hello")));
    chunk.addConstant(Value(true));
    
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, 0));
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, 1));
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, 2));
    chunk.addInstruction(Instruction(OpCode::BUILD_LIST, static_cast<uint32_t>(3)));
    chunk.addInstruction(Instruction(OpCode::PRINT));
    chunk.addInstruction(Instruction(OpCode::HALT));
    
    VM vm;
    bool result = vm.interpret(chunk);
    
    EXPECT_TRUE(result);
}

// Main test runner
int main(int argc, char* argv[]) {
    // Only run ListLength test for debugging
    testing::TestRegistry::Clear();
    
    // Register only ListLength test
    testing::TestRegistry::RegisterTest("ListTests", "ListLength", []() {
        Chunk chunk;
        
        // Create a list and get its length
        chunk.addConstant(Value(static_cast<int64_t>(1)));
        chunk.addConstant(Value(static_cast<int64_t>(2)));
        chunk.addConstant(Value(static_cast<int64_t>(3)));
        chunk.addConstant(Value("len")); // Variable name
        
        // Build the list first
        chunk.addInstruction(Instruction(OpCode::LOAD_CONST, 0));
        chunk.addInstruction(Instruction(OpCode::LOAD_CONST, 1));
        chunk.addInstruction(Instruction(OpCode::LOAD_CONST, 2));
        chunk.addInstruction(Instruction(OpCode::BUILD_LIST, static_cast<uint32_t>(3)));
        chunk.addInstruction(Instruction(OpCode::DUP)); // Duplicate the list for debugging
        chunk.addInstruction(Instruction(OpCode::PRINT)); // Print the list
        
        // Load the len function
        chunk.addInstruction(Instruction(OpCode::LOAD_VAR, 3));
        
        // Call the len function with the list as argument
        chunk.addInstruction(Instruction(OpCode::CALL, static_cast<uint32_t>(1)));
        chunk.addInstruction(Instruction(OpCode::PRINT));
        chunk.addInstruction(Instruction(OpCode::HALT));
        
        // Debug: Print the instruction sequence
        std::cerr << "Instruction sequence:" << std::endl;
        for (size_t i = 0; i < chunk.getInstructions().size(); ++i) {
            const auto& instr = chunk.getInstruction(i);
            std::cerr << "  IP " << i << ": " << static_cast<int>(instr.opcode) 
                      << " (operand: " << instr.operand << ")" << std::endl;
        }
        
        VM vm;
        bool result = vm.interpret(chunk);
        
        if (!result) {
            throw std::runtime_error("Test failed");
        }
    });
    
    testing::TestRunner runner;
    runner.RunAllTests(argc, argv);
    
    return 0;
}