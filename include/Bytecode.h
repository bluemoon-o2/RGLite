// Bytecode.h - Bytecode instruction definitions for RGLite
// This file defines the bytecode instruction set and related data structures

#ifndef RGLITE_BYTECODE_H
#define RGLITE_BYTECODE_H

#include <cstdint>
#include <vector>
#include <string>
#include <variant>
#include <memory>

namespace rglite {

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
    GET_ITEM,      // Get item from container
    SET_ITEM,      // Set item in container
    
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
    FUNCTION,
    NATIVE_FUNCTION
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
    Value(const std::string& s) : type_(ValueType::STRING), data_(s) {}
    Value(uint32_t index, ValueType type) : type_(type), data_(index) {
        // Only allow LIST, DICT, FUNCTION, NATIVE_FUNCTION types
        if (type != ValueType::LIST && type != ValueType::DICT && 
            type != ValueType::FUNCTION && type != ValueType::NATIVE_FUNCTION) {
            type_ = ValueType::NIL;
            data_ = std::monostate{};
        }
    }
    
    // Getters
    ValueType getType() const { return type_; }
    
    bool isNil() const { return type_ == ValueType::NIL; }
    bool isBoolean() const { return type_ == ValueType::BOOLEAN; }
    bool isInteger() const { return type_ == ValueType::INTEGER; }
    bool isFloat() const { return type_ == ValueType::FLOAT; }
    bool isString() const { return type_ == ValueType::STRING; }
    bool isList() const { return type_ == ValueType::LIST; }
    bool isDict() const { return type_ == ValueType::DICT; }
    bool isFunction() const { return type_ == ValueType::FUNCTION; }
    bool isNativeFunction() const { return type_ == ValueType::NATIVE_FUNCTION; }
    
    // Value getters
    bool asBoolean() const { return std::get<bool>(data_); }
    int64_t asInteger() const { return std::get<int64_t>(data_); }
    double asFloat() const { return std::get<double>(data_); }
    std::string asString() const { return std::get<std::string>(data_); }
    uint32_t asIndex() const { return std::get<uint32_t>(data_); }
    
private:
    ValueType type_;
    std::variant<std::monostate, bool, int64_t, double, std::string, uint32_t> data_;
};

// Instruction structure
struct Instruction {
    OpCode opcode;
    uint32_t operand;  // Can be an index, jump offset, or count
    
    Instruction(OpCode op, uint32_t opnd = 0) : opcode(op), operand(opnd) {}
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
        return constants_.size() - 1;
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
    
private:
    std::string name_;
    int arity_;  // Number of parameters
    Chunk chunk_;
};

// Convert OpCode to string for debugging
std::string opCodeToString(OpCode opcode);

// Convert Value to string for debugging
std::string valueToString(const Value& value);

} // namespace rglite

#endif // RGLITE_BYTECODE_H