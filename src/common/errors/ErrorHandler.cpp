// RGLite Error Handling System - Implementation
// This file implements the error handling infrastructure for the compiler

#include "ErrorHandler.h"
#include <sstream>

namespace rglite {

// Diagnostic implementation
std::string Diagnostic::toString() const {
    std::stringstream ss;
    
    // Add error code if present
    if (!code.empty()) {
        ss << "[" << code << "] ";
    }
    
    // Add severity
    switch (severity) {
        case Severity::INFO: ss << "INFO: "; break;
        case Severity::WARNING: ss << "WARNING: "; break;
        case Severity::ERROR: ss << "ERROR: "; break;
        case Severity::FATAL: ss << "FATAL: "; break;
    }
    
    // Add location
    ss << "(" << location.line << ":" << location.column << ") ";
    
    // Add message
    ss << message;
    
    return ss.str();
}

// StandardErrorHandler implementation
void StandardErrorHandler::report(const Diagnostic& diagnostic) {
    diagnostics_.push_back(diagnostic);
    
    // Update error state
    if (diagnostic.severity == Severity::ERROR || diagnostic.severity == Severity::FATAL) {
        has_errors_ = true;
    }
}

bool StandardErrorHandler::hasErrors() const {
    return has_errors_;
}

const std::vector<Diagnostic>& StandardErrorHandler::getDiagnostics() const {
    return diagnostics_;
}

void StandardErrorHandler::clear() {
    diagnostics_.clear();
    has_errors_ = false;
}

// DiagnosticBuilder implementation
Diagnostic DiagnosticBuilder::unexpectedChar(char c, const SourceLocation& loc) {
    std::string message = "Unexpected character: '" + std::string(1, c) + "'";
    return Diagnostic(Severity::ERROR, message, loc, ErrorCode::UNEXPECTED_CHAR);
}

Diagnostic DiagnosticBuilder::unexpectedToken(const Token& token, const std::string& expected) {
    std::string message = "Unexpected token: " + token.lexeme;
    if (!expected.empty()) {
        message += ", expected: " + expected;
    }
    return Diagnostic(Severity::ERROR, message, token.location, ErrorCode::UNEXPECTED_TOKEN);
}

Diagnostic DiagnosticBuilder::undefinedVariable(const std::string& name, const SourceLocation& loc) {
    std::string message = "Undefined variable: " + name;
    return Diagnostic(Severity::ERROR, message, loc, ErrorCode::UNDEFINED_VARIABLE);
}

Diagnostic DiagnosticBuilder::typeMismatch(const std::string& expected, const std::string& actual, 
                                          const SourceLocation& loc) {
    std::string message = "Type mismatch: expected " + expected + ", got " + actual;
    return Diagnostic(Severity::ERROR, message, loc, ErrorCode::TYPE_MISMATCH);
}

Diagnostic DiagnosticBuilder::divisionByZero(const SourceLocation& loc) {
    return Diagnostic(Severity::ERROR, "Division by zero", loc, ErrorCode::DIVISION_BY_ZERO);
}

Diagnostic DiagnosticBuilder::missingSemicolon(const SourceLocation& loc) {
    return Diagnostic(Severity::ERROR, "Missing semicolon", loc, ErrorCode::MISSING_SEMICOLON);
}

Diagnostic DiagnosticBuilder::missingParen(const std::string& type, const SourceLocation& loc) {
    std::string message = "Missing " + type + " parenthesis";
    return Diagnostic(Severity::ERROR, message, loc, ErrorCode::MISSING_PAREN);
}

Diagnostic DiagnosticBuilder::missingColon(const SourceLocation& loc) {
    return Diagnostic(Severity::ERROR, "Missing colon", loc, ErrorCode::MISSING_COLON);
}

Diagnostic DiagnosticBuilder::invalidIndentation(const SourceLocation& loc) {
    return Diagnostic(Severity::ERROR, "Invalid indentation", loc, ErrorCode::INVALID_INDENTATION);
}

} // namespace rglite