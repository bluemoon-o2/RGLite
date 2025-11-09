// Exception.h - Exception handling for RGLite VM
// This file defines exception classes and exception handling mechanisms

#ifndef RGLITE_EXCEPTION_H
#define RGLITE_EXCEPTION_H

#include <string>
#include <vector>
#include <cstdint>
#include <ostream>
#include "Bytecode.h"

namespace rglite {

// Exception types
enum class ExceptionType : uint8_t {
    RUNTIME_ERROR,    // General runtime error
    TYPE_ERROR,       // Type mismatch error
    VALUE_ERROR,      // Invalid value error
    INDEX_ERROR,      // Index out of bounds error
    KEY_ERROR,        // Key not found error
    ZERO_DIVISION,    // Division by zero error
    NAME_ERROR,       // Name not found error
    ATTRIBUTE_ERROR,  // Attribute not found error
    MEMORY_ERROR,     // Memory allocation error
    STACK_OVERFLOW,   // Stack overflow error
    USER_EXCEPTION    // User-defined exception
};

// Exception class for RGLite VM
class Exception {
public:
    // Default constructor
    Exception() : type_(ExceptionType::RUNTIME_ERROR), message_(""), name_(""), value_(Value()) {}
    
    // Constructor for built-in exception types
    Exception(ExceptionType type, const std::string& message, 
             const std::string& name = "", Value value = Value());
    
    // Constructor with call stack information
    Exception(ExceptionType type, const std::string& message, 
             const std::vector<std::tuple<std::string, int, std::string>>& callStack,
             const std::string& name = "", Value value = Value());
    
    // Getters
    ExceptionType getType() const { return type_; }
    const std::string& getMessage() const { return message_; }
    const std::string& getName() const { return name_; }
    const Value& getValue() const { return value_; }
    const std::vector<std::tuple<std::string, int, std::string>>& getCallStack() const { return callStack_; }
    
    // Convert exception to string for debugging
    std::string toString() const;
    
    // Set call stack information
    void setCallStack(const std::vector<std::tuple<std::string, int, std::string>>& callStack) { callStack_ = callStack; }
    
private:
    ExceptionType type_;
    std::string message_;
    std::string name_;  // Exception name (for user-defined exceptions)
    Value value_;       // Associated value (optional)
    std::vector<std::tuple<std::string, int, std::string>> callStack_; // Call stack: (file, line, function) tuples
};

// Exception handler stack frame
struct ExceptionHandler {
    size_t instructionPointer;  // IP to jump to when exception is caught
    size_t stackPointer;        // SP to restore when exception is caught
    uint32_t handlerIndex;      // Index of the handler in the bytecode
    bool isFinally;             // Whether this is a finally block
    
    ExceptionHandler(size_t ip, size_t sp, uint32_t handlerIdx, bool isFinally = false)
        : instructionPointer(ip), stackPointer(sp), handlerIndex(handlerIdx), isFinally(isFinally) {}
};

// Exception handling state
class ExceptionState {
public:
    ExceptionState() : hasException_(false) {}
    
    bool hasException() const { return hasException_; }
    const Exception& getCurrentException() const { return currentException_; }
    
    void setException(const Exception& exception) {
        currentException_ = exception;
        hasException_ = true;
    }
    
    void clearException() {
        hasException_ = false;
    }
    
    // Exception stack management
    void pushHandler(const ExceptionHandler& handler);
    bool popHandler();
    const ExceptionHandler& peekHandler() const;
    bool hasHandlers() const { return !handlerStack_.empty(); }
    void clearHandlers() { handlerStack_.clear(); }
    
    // Exception handling control
    bool isUnwinding() const { return isUnwinding_; }
    void setUnwinding(bool unwinding) { isUnwinding_ = unwinding; }
    
private:
    bool hasException_;
    Exception currentException_;
    std::vector<ExceptionHandler> handlerStack_;
    bool isUnwinding_ = false;
};

// Utility functions for creating common exceptions
class ExceptionBuilder {
public:
    static Exception runtimeError(const std::string& message);
    static Exception typeError(const std::string& expected, const std::string& actual);
    // Python-style unsupported operand type error for binary ops, e.g.,
    // "unsupported operand type(s) for +: 'int' and 'str'"
    static Exception unsupportedBinaryOperand(const std::string& op,
                                             const std::string& leftType,
                                             const std::string& rightType);
    // Python-style unsupported comparison error, e.g.,
    // "'>' not supported between instances of 'int' and 'str'"
    static Exception unsupportedComparison(const std::string& op,
                                           const std::string& leftType,
                                           const std::string& rightType);
    static Exception valueError(const std::string& message);
    static Exception indexError(size_t index, size_t size);
    static Exception keyError(const std::string& key);
    static Exception zeroDivision();
    static Exception nameError(const std::string& name);
    static Exception attributeError(const std::string& attribute, const std::string& object);
    static Exception memoryError(const std::string& message);
    static Exception stackOverflow();
    static Exception userException(const std::string& name, const Value& value = Value());
};

// Stream operator for ExceptionType to aid test output and diagnostics
std::ostream& operator<<(std::ostream& os, ExceptionType type);

} // namespace rglite

#endif // RGLITE_EXCEPTION_H
