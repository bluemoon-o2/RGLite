// VM.h - Virtual Machine for RGLite
// This file defines the virtual machine architecture and core data structures

#ifndef RGLITE_VM_H
#define RGLITE_VM_H

#include <vector>
#include <stack>
#include <memory>
#include <string>
#include <unordered_map>
#include <functional>
#include <variant>
#include <tuple>

#include "Bytecode.h"
#include "Exception.h"
#include "ListValue.h"
#include "DictValue.h"
#include "TupleValue.h"
#include "SetValue.h"
#include "Iterator.h"
#include "MemoryManager.h"

namespace rglite {

// Forward declarations
class VM;
class CodeGenerator;

// Native function type definition
using NativeFunction = std::function<Value(VM&, const std::vector<Value>&)>;

// Stack frame for function calls
struct CallFrame {
    const Function* function;  // Function being executed
    size_t ip;                 // Instruction pointer (index in instruction vector)
    size_t slotsStart;         // Start index of stack slots for this frame
    int line;                  // Line number for error reporting
    std::unordered_map<std::string, Value> locals; // Local variables for this frame
    
    CallFrame(const Function* func, size_t instructionPtr, size_t stackSlotStart, int lineNum = 1)
        : function(func), ip(instructionPtr), slotsStart(stackSlotStart), line(lineNum) {}
};

// Virtual Machine class
class VM {
public:
    // Constructor and destructor
    VM();
    VM(CodeGenerator* codeGenerator);
    ~VM();
    
    // Execute a chunk of bytecode
    bool interpret(const Chunk& chunk);
    bool interpret(const Chunk& chunk, const std::string& filename);
    
    // Execute a function
    bool call(const Function* function, int argCount, size_t returnIP, uint32_t line = 1);
    
    // Stack operations
    void push(const Value& value);
    Value pop();
    Value peek(size_t distance = 0) const;
    
    // Native function registration
    void registerNativeFunction(const std::string& name, NativeFunction function);
    
    // Error handling
    void runtimeError(const std::string& message);
    
    // Exception handling
    void throwException(const Exception& exception);
    void throwException(ExceptionType type, const std::string& message);
    bool hasException() const;
    const Exception& getCurrentException() const;
    // Convenience accessor for tests and external callers
    Exception getException() const;
    void clearException();
    
    // Exception handler management
    void pushExceptionHandler(size_t handlerIndex);
    bool popExceptionHandler();
    bool hasExceptionHandlers() const;
    
    // Get the current stack size
    size_t getStackSize() const { return stack_.size(); }
    
    // Get the current instruction pointer
    size_t getIP() const;
    
    // Get the current call frame
    const CallFrame* getCurrentFrame() const;
    
    // Collect call stack information for exception handling
    std::vector<std::tuple<std::string, int, std::string>> collectCallStack() const;
    
    // Get the list storage
    ListStorage& getListStorage() { return listStorage_; }
    const ListStorage& getListStorage() const { return listStorage_; }
    
    // Get the dictionary storage
    DictStorage& getDictStorage() { return dictStorage_; }
    const DictStorage& getDictStorage() const { return dictStorage_; }
    
    // Get the tuple storage
    TupleStorage& getTupleStorage() { return tupleStorage_; }
    const TupleStorage& getTupleStorage() const { return tupleStorage_; }
    
    // Get the set storage
    SetStorage& getSetStorage() { return setStorage_; }
    const SetStorage& getSetStorage() const { return setStorage_; }
    
    // Get the iterator storage
    IteratorStorage& getIteratorStorage() { return iteratorStorage_; }
    const IteratorStorage& getIteratorStorage() const { return iteratorStorage_; }
    
    // Memory manager access
    MemoryManager& getMemoryManager() { return memoryManager_; }
    const MemoryManager& getMemoryManager() const { return memoryManager_; }
    
    // Set variable name mapping for error reporting
    void setVariableName(uint32_t index, const std::string& name);

private:
    // Execution methods
    bool run();
    void resetStack();
    
    // Binary operations
    bool binaryOp(OpCode opcode);
    bool compareOp(OpCode opcode);
    
    // Stack frame management
    bool pushFrame(const Function* function, int argCount, size_t returnIP, uint32_t line = 1);
    bool popFrame();
    
    // Native function handling
    bool callNativeFunction(const NativeFunction& function, int argCount);
    
    // Exception handling methods
    bool unwindStack();
    void handleException();
    
    // Data members
    CodeGenerator* codeGenerator_;                // Code generator for accessing function definitions
    std::vector<Value> stack_;                    // Value stack
    std::vector<CallFrame> frames_;               // Call frame stack
    const Chunk* currentChunk_;                   // Currently executing chunk
    const Chunk* entryChunk_;                     // Entry (module) chunk for top-level execution
    size_t ip_;                                   // Instruction pointer
    std::string filename_;                        // Current filename for error reporting
    
    // Global variables
    std::unordered_map<std::string, Value> globals_;
    
    // Variable name to index mapping for error reporting
    std::unordered_map<uint32_t, std::string> variableNames_;
    
    // Native functions
    std::unordered_map<std::string, NativeFunction> nativeFunctions_;
    
    // Execution state
    bool hadError_;
    
    // Exception handling state
    ExceptionState exceptionState_;
    
    // Exception storage
    std::vector<Exception> exceptions_;           // Store exception objects
    
    // List storage
    ListStorage listStorage_;                     // Store list objects
    
    // Dictionary storage
    DictStorage dictStorage_;                     // Store dictionary objects
    
    // Tuple storage
    TupleStorage tupleStorage_;                   // Store tuple objects
    
    // Set storage
    SetStorage setStorage_;                       // Store set objects
    
    // Iterator storage
    IteratorStorage iteratorStorage_;             // Store iterator objects
    
    // Memory manager
    MemoryManager memoryManager_;                // Memory management for objects
};

} // namespace rglite

#endif // RGLITE_VM_H
