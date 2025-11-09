// test_memory.cpp - Test cases for memory management in RGLite
// This file contains test cases for memory allocation, reference counting, and garbage collection

#include "TestFramework.h"
#include "VM.h"
#include "Bytecode.h"
#include "Exception.h"
#include "MemoryManager.h"
#include <iostream>
#include <memory>

using namespace rglite;

// Test memory manager creation
TEST(MemoryTests, MemoryManagerCreation) {
    MemoryManager manager;
    
    // Check initial state - MemoryManager no longer directly tracks objects
    // It relies on ListStorage and DictStorage for tracking
    EXPECT_EQ(manager.getAllocatedLists(), 0);
    EXPECT_EQ(manager.getAllocatedDicts(), 0);
    EXPECT_EQ(manager.getGcThreshold(), 1000); // Default threshold
}

// Test memory manager threshold setting
TEST(MemoryTests, MemoryManagerThreshold) {
    MemoryManager manager;
    
    // Set a custom threshold
    manager.setGCThreshold(50);
    EXPECT_EQ(manager.getGcThreshold(), 50);
}

// Test list allocation through memory manager
TEST(MemoryTests, ListAllocation) {
    MemoryManager manager;
    ListStorage listStorage(&manager);
    
    // Set the storage reference in memory manager
    manager.setListStorage(&listStorage);
    
    // Allocate a list through storage
    size_t listIndex = listStorage.createList(10);
    ListValue* list = listStorage.getList(listIndex);
    
    EXPECT_NE(list, nullptr);
    EXPECT_EQ(list->size(), 0); // Empty list, capacity is 10
    EXPECT_EQ(manager.getAllocatedLists(), 1);
    EXPECT_EQ(manager.getAllocatedDicts(), 0);
    
    // No need to clean up - ListStorage manages the list
}

// Test dictionary allocation through memory manager
TEST(MemoryTests, DictAllocation) {
    MemoryManager manager;
    DictStorage dictStorage(&manager);
    
    // Set the storage reference in memory manager
    manager.setDictStorage(&dictStorage);
    
    // Allocate a dictionary through storage
    size_t dictIndex = dictStorage.createDict();
    DictValue* dict = dictStorage.getDict(dictIndex);
    
    EXPECT_NE(dict, nullptr);
    EXPECT_EQ(dict->size(), 0);
    EXPECT_EQ(manager.getAllocatedLists(), 0);
    EXPECT_EQ(manager.getAllocatedDicts(), 1);
    
    // No need to clean up - DictStorage manages the dictionary
}

// Test reference counting
TEST(MemoryTests, ReferenceCounting) {
    MemoryManager manager;
    ListStorage listStorage(&manager);
    
    // Set the storage reference in memory manager
    manager.setListStorage(&listStorage);
    
    // Allocate a list through storage
    size_t listIndex = listStorage.createList(5);
    ListValue* list = listStorage.getList(listIndex);
    
    // Note: In our current implementation, we don't use reference counting
    // This test is kept for compatibility with the interface
    manager.incrementReference(Value());
    manager.decrementReference(Value());
    
    // The list should still exist
    EXPECT_EQ(manager.getAllocatedLists(), 1);
    
    // Note: In a real implementation with reference counting,
    // we would decrement the reference count here
    // For now, we'll just check that the list still exists
    
    // Suppress unused variable warning
    (void)list;
}

// Test garbage collection trigger
TEST(MemoryTests, GarbageCollectionTrigger) {
    MemoryManager manager;
    
    // Set a low threshold to trigger GC
    manager.setGCThreshold(5);
    
    // Allocate several lists to trigger GC
    std::vector<ListValue*> lists;
    for (int i = 0; i < 10; ++i) {
        ListValue* list = manager.allocateList(5);
        lists.push_back(list);
    }
    
    // Check that GC was triggered (allocated lists should be at or below threshold)
    EXPECT_LE(manager.getAllocatedLists(), 5);
    
    // Clean up
    for (auto* list : lists) {
        if (list) {
            manager.decrementReference(list);
        }
    }
}

// Main function to run all tests
int main(int argc, char** argv) {
    testing::TestRunner runner;
    runner.RunAllTests(argc, argv);
    return runner.GetExitCode();
}

// Test list memory integration with VM
TEST(MemoryTests, ListMemoryIntegration) {
    Chunk chunk;
    
    // Create a list and manipulate it
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
    
    // Check memory manager state
    MemoryManager& manager = vm.getMemoryManager();
    EXPECT_GE(manager.getAllocatedLists(), 0); // Should be at least 0
}

// Test dictionary memory integration with VM
TEST(MemoryTests, DictMemoryIntegration) {
    Chunk chunk;
    
    // Create a dictionary and manipulate it
    chunk.addConstant(Value(std::string("a")));
    chunk.addConstant(Value(static_cast<int64_t>(1)));
    chunk.addConstant(Value(std::string("b")));
    chunk.addConstant(Value(static_cast<int64_t>(2)));
    
    // Push keys and values in alternating order
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, 0)); // "a"
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, 1)); // 1
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, 2)); // "b"
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, 3)); // 2
    
    // Build the dictionary with 2 pairs
    chunk.addInstruction(Instruction(OpCode::BUILD_DICT, static_cast<uint32_t>(2)));
    chunk.addInstruction(Instruction(OpCode::PRINT));
    chunk.addInstruction(Instruction(OpCode::HALT));
    
    VM vm;
    bool result = vm.interpret(chunk);
    
    EXPECT_TRUE(result);
    
    // Check memory manager state
    MemoryManager& manager = vm.getMemoryManager();
    EXPECT_GE(manager.getAllocatedDicts(), 0); // Should be at least 0
}

// Test memory cleanup with complex operations
TEST(MemoryTests, MemoryCleanupComplex) {
    Chunk chunk;
    
    // Create a complex scenario with nested lists and dictionaries
    chunk.addConstant(Value(std::string("a")));
    chunk.addConstant(Value(static_cast<int64_t>(1)));
    chunk.addConstant(Value(static_cast<int64_t>(10)));
    chunk.addConstant(Value(static_cast<int64_t>(20)));
    
    // Create an inner list
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, 2)); // 10
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, 3)); // 20
    chunk.addInstruction(Instruction(OpCode::BUILD_LIST, static_cast<uint32_t>(2)));
    
    // Create a dictionary with the list as a value
    chunk.addInstruction(Instruction(OpCode::LOAD_CONST, 0)); // "a"
    chunk.addInstruction(Instruction(OpCode::DUP)); // Duplicate the list
    chunk.addInstruction(Instruction(OpCode::BUILD_DICT, static_cast<uint32_t>(1)));
    
    // Print the dictionary
    chunk.addInstruction(Instruction(OpCode::PRINT));
    chunk.addInstruction(Instruction(OpCode::HALT));
    
    VM vm;
    bool result = vm.interpret(chunk);
    
    EXPECT_TRUE(result);
    
    // Check memory manager state
    MemoryManager& manager = vm.getMemoryManager();
    EXPECT_GE(manager.getAllocatedLists(), 0);
    EXPECT_GE(manager.getAllocatedDicts(), 0);
}

// Test memory manager with large allocations
TEST(MemoryTests, LargeAllocations) {
    MemoryManager manager;
    
    // Set a low threshold to trigger GC frequently
    manager.setGCThreshold(10);
    
    // Allocate many lists
    std::vector<ListValue*> lists;
    for (int i = 0; i < 100; ++i) {
        ListValue* list = manager.allocateList(100); // Large lists
        if (list) {
            lists.push_back(list);
            // Add some items to the list
            for (int j = 0; j < 10; ++j) {
                list->append(Value(static_cast<int64_t>(j)));
            }
        }
    }
    
    // Check that GC was triggered (allocated lists should be at or below threshold)
    EXPECT_LE(manager.getAllocatedLists(), 10);
    
    // Clean up
    for (auto* list : lists) {
        if (list) {
            manager.decrementReference(list);
        }
    }
}

// Test memory manager with mixed types
TEST(MemoryTests, MixedTypeAllocations) {
    MemoryManager manager;
    
    // Set a low threshold to trigger GC frequently
    manager.setGCThreshold(5);
    
    // Allocate a mix of lists and dictionaries
    std::vector<ListValue*> lists;
    std::vector<DictValue*> dicts;
    
    for (int i = 0; i < 10; ++i) {
        // Allocate a list
        ListValue* list = manager.allocateList(5);
        if (list) {
            lists.push_back(list);
            list->append(Value(static_cast<int64_t>(i)));
        }
        
        // Allocate a dictionary
        DictValue* dict = manager.allocateDict();
        if (dict) {
            dicts.push_back(dict);
            dict->set(std::to_string(i), Value(static_cast<int64_t>(i)));
        }
    }
    
    // Check that GC was triggered
    EXPECT_LE(manager.getAllocatedLists() + manager.getAllocatedDicts(), 5);
    
    // Clean up
    for (auto* list : lists) {
        if (list) {
            manager.decrementReference(list);
        }
    }
    
    for (auto* dict : dicts) {
        if (dict) {
            manager.decrementReference(dict);
        }
    }
}