// Exception.cpp - Exception handling implementation for RGLite VM

#include "Exception.h"
#include "Bytecode.h"
#include <sstream>
#include <ostream>

namespace rglite {

// Exception implementation
Exception::Exception(ExceptionType type, const std::string& message, 
                     const std::string& name, Value value)
    : type_(type), message_(message), name_(name), value_(value) {}

Exception::Exception(ExceptionType type, const std::string& message, 
                     const std::vector<std::tuple<std::string, int, std::string>>& callStack,
                     const std::string& name, Value value)
    : type_(type), message_(message), name_(name), value_(value), callStack_(callStack) {}

std::string Exception::toString() const {
    std::stringstream ss;
    
    // Python-style traceback for runtime errors (exact match)
    ss << "Traceback (most recent call last):\n";
    
    // Display call stack if available
    if (!callStack_.empty()) {
        // Display call stack from most recent to least recent (reverse order)
        for (auto it = callStack_.rbegin(); it != callStack_.rend(); ++it) {
            const auto& frame = *it;
            ss << "  File \"" << std::get<0>(frame) << "\", line " << std::get<1>(frame) << ", in " << std::get<2>(frame) << "\n";
        }
    } else {
        // Default to single frame if no call stack is available
        ss << "  File \"<stdin>\", line 1, in <module>\n";
    }
    
    // Add appropriate Python exception type
    switch (type_) {
        case ExceptionType::RUNTIME_ERROR:
            ss << "RuntimeError: " << message_;
            break;
        case ExceptionType::TYPE_ERROR:
            ss << "TypeError: " << message_;
            break;
        case ExceptionType::VALUE_ERROR:
            ss << "ValueError: " << message_;
            break;
        case ExceptionType::INDEX_ERROR:
            ss << "IndexError: " << message_;
            break;
        case ExceptionType::KEY_ERROR:
            ss << "KeyError: " << message_;
            break;
        case ExceptionType::ZERO_DIVISION:
            ss << "ZeroDivisionError: " << message_;
            break;
        case ExceptionType::NAME_ERROR:
            ss << "NameError: " << message_;
            break;
        case ExceptionType::ATTRIBUTE_ERROR:
            ss << "AttributeError: " << message_;
            break;
        case ExceptionType::MEMORY_ERROR:
            ss << "MemoryError: " << message_;
            break;
        case ExceptionType::STACK_OVERFLOW:
            ss << "RecursionError: " << message_;
            break;
        case ExceptionType::USER_EXCEPTION:
            ss << name_ << ": " << message_;
            break;
    }
    
    return ss.str();
}

// ExceptionState implementation
void ExceptionState::pushHandler(const ExceptionHandler& handler) {
    handlerStack_.push_back(handler);
}

bool ExceptionState::popHandler() {
    if (handlerStack_.empty()) {
        return false;
    }
    handlerStack_.pop_back();
    return true;
}

const ExceptionHandler& ExceptionState::peekHandler() const {
    static ExceptionHandler defaultHandler(0, 0, 0);
    if (handlerStack_.empty()) {
        return defaultHandler;
    }
    return handlerStack_.back();
}

// ExceptionBuilder implementation
Exception ExceptionBuilder::runtimeError(const std::string& message) {
    return Exception(ExceptionType::RUNTIME_ERROR, message);
}

Exception ExceptionBuilder::typeError(const std::string& expected, const std::string& actual) {
    return Exception(ExceptionType::TYPE_ERROR, "Expected " + expected + " but got " + actual);
}

Exception ExceptionBuilder::unsupportedBinaryOperand(const std::string& op,
                                                    const std::string& leftType,
                                                    const std::string& rightType) {
    return Exception(
        ExceptionType::TYPE_ERROR,
        std::string("unsupported operand type(s) for ") + op + ": '" + leftType + "' and '" + rightType + "'"
    );
}

Exception ExceptionBuilder::unsupportedComparison(const std::string& op,
                                                  const std::string& leftType,
                                                  const std::string& rightType) {
    return Exception(
        ExceptionType::TYPE_ERROR,
        std::string("'") + op + "' not supported between instances of '" + leftType + "' and '" + rightType + "'"
    );
}

Exception ExceptionBuilder::valueError(const std::string& message) {
    return Exception(ExceptionType::VALUE_ERROR, message);
}

Exception ExceptionBuilder::indexError(size_t index, size_t size) {
    return Exception(ExceptionType::INDEX_ERROR, 
                    "Index " + std::to_string(index) + " out of bounds (size: " + std::to_string(size) + ")");
}

Exception ExceptionBuilder::keyError(const std::string& key) {
    return Exception(ExceptionType::KEY_ERROR, std::string("'") + key + "'");
}

Exception ExceptionBuilder::zeroDivision() {
    // Match Python wording exactly
    return Exception(ExceptionType::ZERO_DIVISION, "division by zero");
}

Exception ExceptionBuilder::nameError(const std::string& name) {
    return Exception(ExceptionType::NAME_ERROR, std::string("name '") + name + "' is not defined");
}

Exception ExceptionBuilder::attributeError(const std::string& attribute, const std::string& object) {
    return Exception(ExceptionType::ATTRIBUTE_ERROR, 
                    "Attribute '" + attribute + "' not found in " + object);
}

Exception ExceptionBuilder::memoryError(const std::string& message) {
    return Exception(ExceptionType::MEMORY_ERROR, message);
}

Exception ExceptionBuilder::stackOverflow() {
    return Exception(ExceptionType::STACK_OVERFLOW, "Stack overflow");
}

Exception ExceptionBuilder::userException(const std::string& name, const Value& value) {
    return Exception(ExceptionType::USER_EXCEPTION, "User exception: " + name, name, value);
}

// Value constructor for Exception
Value::Value(const Exception& exception) : type_(ValueType::EXCEPTION), data_(exception.toString()) {}

// Stream operator for ExceptionType for test diagnostics
std::ostream& operator<<(std::ostream& os, rglite::ExceptionType type) {
    switch (type) {
        case rglite::ExceptionType::RUNTIME_ERROR:    os << "RUNTIME_ERROR"; break;
        case rglite::ExceptionType::TYPE_ERROR:       os << "TYPE_ERROR"; break;
        case rglite::ExceptionType::VALUE_ERROR:      os << "VALUE_ERROR"; break;
        case rglite::ExceptionType::INDEX_ERROR:      os << "INDEX_ERROR"; break;
        case rglite::ExceptionType::KEY_ERROR:        os << "KEY_ERROR"; break;
        case rglite::ExceptionType::ZERO_DIVISION:    os << "ZERO_DIVISION"; break;
        case rglite::ExceptionType::NAME_ERROR:       os << "NAME_ERROR"; break;
        case rglite::ExceptionType::ATTRIBUTE_ERROR:  os << "ATTRIBUTE_ERROR"; break;
        case rglite::ExceptionType::MEMORY_ERROR:     os << "MEMORY_ERROR"; break;
        case rglite::ExceptionType::STACK_OVERFLOW:   os << "STACK_OVERFLOW"; break;
        case rglite::ExceptionType::USER_EXCEPTION:   os << "USER_EXCEPTION"; break;
    }
    return os;
}

} // namespace rglite
