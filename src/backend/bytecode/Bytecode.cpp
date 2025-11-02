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
        case OpCode::GET_ITEM: return "GET_ITEM";
        case OpCode::SET_ITEM: return "SET_ITEM";
        
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
        oss << "[list:" << value.asIndex() << "]";
    } else if (value.isDict()) {
        oss << "{dict:" << value.asIndex() << "}";
    } else if (value.isFunction()) {
        oss << "function:" << value.asIndex();
    } else if (value.isNativeFunction()) {
        oss << "native_function:" << value.asIndex();
    }
    
    return oss.str();
}

} // namespace rglite