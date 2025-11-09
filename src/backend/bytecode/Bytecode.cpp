// Bytecode.cpp - Bytecode instruction implementation for RGLite
// This file implements the bytecode instruction set and related data structures

#include "Bytecode.h"
#include <sstream>

namespace rglite {

// Function implementation
Function::Function(const std::string& name, int arity) 
    : name_(name), arity_(arity) {
}

// Convert OpCode to string for debugging
std::string opCodeToString(OpCode opcode) {
    switch (opcode) {
        case OpCode::LOAD_CONST: return "LOAD_CONST";
        case OpCode::LOAD_VAR: return "LOAD_VAR";
        case OpCode::STORE_VAR: return "STORE_VAR";
        case OpCode::POP: return "POP";
        case OpCode::DUP: return "DUP";
        
        case OpCode::ADD: return "ADD";
        case OpCode::SUB: return "SUB";
        case OpCode::MUL: return "MUL";
        case OpCode::DIV: return "DIV";
        case OpCode::MOD: return "MOD";
        case OpCode::POW: return "POW";
        
        case OpCode::EQ: return "EQ";
        case OpCode::NEQ: return "NEQ";
        case OpCode::LT: return "LT";
        case OpCode::LTE: return "LTE";
        case OpCode::GT: return "GT";
        case OpCode::GTE: return "GTE";
        
        case OpCode::AND: return "AND";
        case OpCode::OR: return "OR";
        case OpCode::NOT: return "NOT";
        
        case OpCode::JUMP: return "JUMP";
        case OpCode::JUMP_IF_FALSE: return "JUMP_IF_FALSE";
        case OpCode::JUMP_IF_TRUE: return "JUMP_IF_TRUE";
        
        case OpCode::CALL: return "CALL";
        case OpCode::RETURN: return "RETURN";
        
        case OpCode::BUILD_LIST: return "BUILD_LIST";
        case OpCode::BUILD_DICT: return "BUILD_DICT";
        case OpCode::BUILD_TUPLE: return "BUILD_TUPLE";
        case OpCode::BUILD_SET: return "BUILD_SET";
        case OpCode::GET_ITEM: return "GET_ITEM";
        case OpCode::SET_ITEM: return "SET_ITEM";
        case OpCode::CONTAINS: return "CONTAINS";
        case OpCode::CREATE_ITER: return "CREATE_ITER";
        case OpCode::HAS_NEXT: return "HAS_NEXT";
        case OpCode::GET_NEXT: return "GET_NEXT";
        
        case OpCode::TRY: return "TRY";
        case OpCode::CATCH: return "CATCH";
        case OpCode::END_TRY: return "END_TRY";
        case OpCode::THROW: return "THROW";
        case OpCode::PUSH_HANDLER: return "PUSH_HANDLER";
        case OpCode::POP_HANDLER: return "POP_HANDLER";
        
        case OpCode::PRINT: return "PRINT";
        case OpCode::HALT: return "HALT";
        
        default: return "UNKNOWN";
    }
}

// Helper function to convert Value to string
std::string valueToString(const Value& value) {
    std::ostringstream oss;
    
    if (value.isNil()) {
        oss << "nil";
    } else if (value.isBoolean()) {
        oss << (value.asBoolean() ? "true" : "false");
    } else if (value.isInteger()) {
        oss << value.asInteger();
    } else if (value.isFloat()) {
        oss << value.asFloat();
    } else if (value.isString()) {
        oss << "\"" << value.asString() << "\"";
    } else if (value.isList()) {
        // For list values, we need to get the actual list from storage
        // Since we don't have access to VM here, we'll just show the index
        oss << "[list:" << value.asIndex() << "]";
    } else if (value.isDict()) {
        oss << "{dict:" << value.asIndex() << "}";
    } else if (value.isTuple()) {
        oss << "(tuple:" << value.asIndex() << ")";
    } else if (value.isSet()) {
        oss << "{set:" << value.asIndex() << "}";
    } else if (value.isFunction()) {
        oss << "function:" << value.asIndex();
    } else if (value.isNativeFunction()) {
        // Try to get the function name
        std::string funcName = value.asNativeFunctionName();
        if (!funcName.empty()) {
            oss << "native_function:" << funcName;
        } else {
            oss << "native_function:" << value.asIndex();
        }
    } else if (value.isException()) {
        // For exception values, we need to extract the actual exception message
        // The exception data is stored as a string in the value
        oss << value.asString();
    }
    
    return oss.str();
}

// Value equality comparison implementation
bool Value::equals(const Value& other) const {
    return valuesEqual(*this, other);
}

// Static helper function for value comparison
bool Value::valuesEqual(const Value& a, const Value& b) {
    // If types are different, values are not equal
    if (a.type_ != b.type_) {
        return false;
    }
    
    // Compare based on type
    switch (a.type_) {
        case ValueType::NIL:
            return true; // All nil values are equal
            
        case ValueType::BOOLEAN:
            return a.asBoolean() == b.asBoolean();
            
        case ValueType::INTEGER:
            return a.asInteger() == b.asInteger();
            
        case ValueType::FLOAT:
            return a.asFloat() == b.asFloat();
            
        case ValueType::STRING:
            return a.asString() == b.asString();
            
        case ValueType::LIST:
        case ValueType::DICT:
        case ValueType::TUPLE:
        case ValueType::SET:
        case ValueType::FUNCTION:
        case ValueType::NATIVE_FUNCTION:
        case ValueType::ITERATOR:
        case ValueType::EXCEPTION:
            // For complex types, compare by index/reference
            return a.asIndex() == b.asIndex();
            
        default:
            return false;
    }
}

} // namespace rglite
