// Bytecode.h - Bytecode instruction definitions for RGLite
// This file defines the bytecode instruction set and related data structures

#ifndef RGLITE_BYTECODE_H
#define RGLITE_BYTECODE_H

#include <cstdint>
#include <string>
#include <vector>
#include <variant>
#include <iostream>
#include <memory>

namespace rglite {

// Forward declarations
class Exception;

// Bytecode instruction opcodes
enum class OpCode : uint8_t {
    // Stack operations
    LOAD_CONST,    // Load constant onto stack
    LOAD_VAR,      // Load variable onto stack
    STORE_VAR,     // Store top of stack to variable
    POP,           // Pop top value from stack
    DUP,           // Duplicate top value on stack
    
    // Arithmetic operations
    ADD,           // Add top two values
    SUB,           // Subtract top two values
    MUL,           // Multiply top two values
    DIV,           // Divide top two values
    MOD,           // Modulo top two values
    POW,           // Power operation
    
    // Comparison operations
    EQ,            // Equal comparison
    NEQ,           // Not equal comparison
    LT,            // Less than
    LTE,           // Less than or equal
    GT,            // Greater than
    GTE,           // Greater than or equal
    
    // Logical operations
    AND,           // Logical AND
    OR,            // Logical OR
    NOT,           // Logical NOT
    
    // Control flow
    JUMP,          // Unconditional jump
    JUMP_IF_FALSE, // Jump if top of stack is false
    JUMP_IF_TRUE,  // Jump if top of stack is true
    
    // Function operations
    CALL,          // Call function
    RETURN,        // Return from function
    
    // Container operations
    BUILD_LIST,    // Build list from n items
    BUILD_DICT,    // Build dictionary from n key-value pairs
    BUILD_TUPLE,   // Build tuple from n items
    BUILD_SET,     // Build set from n items
    GET_ITEM,      // Get item from container
    SET_ITEM,      // Set item in container
    CONTAINS,      // Check if item is in container
    CREATE_ITER,   // Create iterator for container
    HAS_NEXT,      // Check if iterator has next element
    GET_NEXT,      // Get next element from iterator
    
    // Object operations
    GET_ATTR,      // Get attribute from object
    
    // Exception handling operations
    TRY,           // Start a try block
    CATCH,         // Start a catch block
    END_TRY,       // End a try-catch block
    THROW,         // Throw an exception
    PUSH_HANDLER,  // Push exception handler onto stack
    POP_HANDLER,   // Pop exception handler from stack
    
    // Miscellaneous
    PRINT,         // Print value
    HALT           // Stop execution
};

// Value types in the VM
enum class ValueType : uint8_t {
    NIL,
    BOOLEAN,
    INTEGER,
    FLOAT,
    STRING,
    LIST,
    DICT,
    TUPLE,         // Added for tuple support
    SET,           // Added for set support
    FUNCTION,
    NATIVE_FUNCTION,
    ITERATOR,      // Iterator type for containers
    EXCEPTION      // Exception value type
};

// Forward declaration for Value
class Value;

// Value representation using a wrapper class
class Value {
public:
    // Constructors for different types
    Value() : type_(ValueType::NIL), data_(std::monostate{}) {}
    Value(bool b) : type_(ValueType::BOOLEAN), data_(b) {}
    Value(int64_t i) : type_(ValueType::INTEGER), data_(i) {}
    Value(double d) : type_(ValueType::FLOAT), data_(d) {}
    Value(const char* s) : type_(ValueType::STRING), data_(std::string(s)) {}  // Added for C-style strings
    Value(const std::string& s) : type_(ValueType::STRING), data_(s) {}
    Value(uint32_t index, ValueType type) : type_(type), data_(index) {
        // Only allow LIST, DICT, TUPLE, SET, FUNCTION, NATIVE_FUNCTION, ITERATOR, EXCEPTION types
        if (type != ValueType::LIST && type != ValueType::DICT && type != ValueType::TUPLE &&
            type != ValueType::SET && type != ValueType::FUNCTION && type != ValueType::NATIVE_FUNCTION &&
            type != ValueType::ITERATOR && type != ValueType::EXCEPTION) {
            type_ = ValueType::NIL;
            data_ = std::monostate{};
        }
    }
    
    // Constructor for native function with name
    explicit Value(const std::string& name, bool isNative) : type_(ValueType::NATIVE_FUNCTION), data_(name) {
        (void)isNative; // Suppress unused parameter warning
    }
    
    // Constructor for exception
    Value(const Exception& exception); // Implementation in Exception.cpp
    
    // Getters
    ValueType getType() const { return type_; }
    size_t getDataIndex() const { return data_.index(); }
    
    bool isNil() const { return type_ == ValueType::NIL; }
    bool isBoolean() const { return type_ == ValueType::BOOLEAN; }
    bool isInteger() const { return type_ == ValueType::INTEGER; }
    bool isFloat() const { return type_ == ValueType::FLOAT; }
    bool isString() const { return type_ == ValueType::STRING; }
    bool isList() const { return type_ == ValueType::LIST; }
    bool isDict() const { return type_ == ValueType::DICT; }
    bool isTuple() const { return type_ == ValueType::TUPLE; }
    bool isSet() const { return type_ == ValueType::SET; }
    bool isFunction() const { return type_ == ValueType::FUNCTION; }
    bool isNativeFunction() const { return type_ == ValueType::NATIVE_FUNCTION; }
    bool isIterator() const { return type_ == ValueType::ITERATOR; }
    bool isException() const { return type_ == ValueType::EXCEPTION; }
    
    // Value getters
    bool asBoolean() const { 
        try {
            return std::get<bool>(data_);
        } catch (const std::bad_variant_access&) {
            std::cerr << "Error: Attempting to access non-boolean value as boolean" << std::endl;
            return false;
        }
    }
    int64_t asInteger() const { 
        try {
            return std::get<int64_t>(data_);
        } catch (const std::bad_variant_access&) {
            std::cerr << "Error: Attempting to access non-integer value as integer" << std::endl;
            return 0;
        }
    }
    double asFloat() const { 
        try {
            return std::get<double>(data_);
        } catch (const std::bad_variant_access&) {
            std::cerr << "Error: Attempting to access non-float value as float" << std::endl;
            return 0.0;
        }
    }
    std::string asString() const { 
        try {
            return std::get<std::string>(data_);
        } catch (const std::bad_variant_access&) {
            std::cerr << "Error: Attempting to access non-string value as string" << std::endl;
            return "";
        }
    }
    uint32_t asIndex() const { 
        try {
            return std::get<uint32_t>(data_);
        } catch (const std::bad_variant_access&) {
            std::cerr << "Error: Attempting to access non-index value as index" << std::endl;
            return 0; // Return a default value
        }
    }
    std::string asNativeFunctionName() const { 
        if (type_ == ValueType::NATIVE_FUNCTION) {
            // Check if the data is stored as a string (function name)
            try {
                if (std::holds_alternative<std::string>(data_)) {
                    return std::get<std::string>(data_);
                }
                // If stored as an index, we can't get the name from here
                // This would need to be handled by the VM
            } catch (const std::bad_variant_access&) {
                // Handle the case where the data is not a string
                return "";
            }
        }
        return "";
    }
    
    // Equality comparison
    bool equals(const Value& other) const;
    
    // Static helper function for value comparison
    static bool valuesEqual(const Value& a, const Value& b);
    
private:
    ValueType type_;
    std::variant<std::monostate, bool, int64_t, double, std::string, uint32_t> data_;
};

// Instruction structure
struct Instruction {
    OpCode opcode;
    uint32_t operand;  // Can be an index, jump offset, or count
    uint32_t line;     // Source code line number
    
    Instruction(OpCode op, uint32_t opnd = 0, uint32_t ln = 1) : opcode(op), operand(opnd), line(ln) {}
};

// Bytecode chunk containing instructions and constants
class Chunk {
public:
    Chunk() = default;
    
    // Add an instruction to the chunk
    size_t addInstruction(const Instruction& instruction) {
        instructions_.push_back(instruction);
        return instructions_.size() - 1;
    }
    
    // Add a constant to the constant pool
    size_t addConstant(const Value& value) {
        constants_.push_back(value);
        size_t index = constants_.size() - 1;
        return index;
    }
    
    // Getters
    const std::vector<Instruction>& getInstructions() const { return instructions_; }
    const std::vector<Value>& getConstants() const { return constants_; }
    
    // Get instruction at specific index
    const Instruction& getInstruction(size_t index) const {
        return instructions_[index];
    }
    
    // Get constant at specific index
    const Value& getConstant(size_t index) const {
        return constants_[index];
    }
    
    // Clear the chunk
    void clear() {
        instructions_.clear();
        constants_.clear();
    }
    
    // Get the size of the chunk
    size_t size() const { return instructions_.size(); }
    
private:
    std::vector<Instruction> instructions_;
    std::vector<Value> constants_;  // Constant pool for storing Value objects
};

// Function representation in bytecode
class Function {
public:
    Function(const std::string& name, int arity);
    
    // Getters
    const std::string& getName() const { return name_; }
    int getArity() const { return arity_; }
    const Chunk& getChunk() const { return chunk_; }
    
    // Get the chunk for modification
    Chunk& getChunk() { return chunk_; }

    // Docstring support
    void setDocstring(const std::string& doc) {
        docstring_ = doc;
        hasDocstring_ = true;
    }
    bool hasDocstring() const { return hasDocstring_; }
    const std::string& getDocstring() const { return docstring_; }
    
private:
    std::string name_;
    int arity_;  // Number of parameters
    Chunk chunk_;
    bool hasDocstring_ = false;
    std::string docstring_;
};

// Convert OpCode to string for debugging
std::string opCodeToString(OpCode opcode);

// Convert Value to string for debugging
std::string valueToString(const Value& value);

} // namespace rglite

#endif // RGLITE_BYTECODE_H
