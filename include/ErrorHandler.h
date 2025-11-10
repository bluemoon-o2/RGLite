// RGLite Error Handling System
// This file defines the error handling infrastructure for the compiler

#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <string>
#include <vector>
#include <memory>
#include "Token.h"

namespace rglite {

/**
 * @brief Severity levels for compiler messages
 */
enum class Severity {
    INFO,       // Informational message
    WARNING,    // Warning that doesn't stop compilation
    ERROR,      // Error that stops compilation
    FATAL       // Fatal error that terminates immediately
};

/**
 * @brief Compiler diagnostic message
 */
struct Diagnostic {
    Severity severity;
    std::string message;
    SourceLocation location;
    std::string code;  // Error code (e.g., "R001")
    std::string sourceLine;  // Source code line where error occurred
    int column;  // Column where error occurred
    int end_column;  // End column for multi-line errors
    int end_line;  // End line for multi-line errors
    std::string filename;  // Filename for error reporting
    std::string function_name;  // Function name for context
    bool displayTracebackHeader = false; // Whether to display the "Traceback (most recent call last):" header
    
    Diagnostic(Severity s, const std::string& msg, const SourceLocation& loc, 
               const std::string& c = "", const std::string& srcLine = "", 
               int col = 0, int endCol = 0, int endLine = 0, 
               const std::string& fname = "<stdin>", const std::string& funcName = "",
               bool displayTraceback = false)
        : severity(s), message(msg), location(loc), code(c), sourceLine(srcLine), 
          column(col), end_column(endCol), end_line(endLine), filename(fname), function_name(funcName), displayTracebackHeader(displayTraceback) {}
    
    std::string toString() const;
};

/**
 * @brief Error codes for RGLite compiler
 */
namespace ErrorCode {
    // Lexical errors (R001-R099)
    constexpr const char* UNEXPECTED_CHAR = "R001";
    constexpr const char* UNTERMINATED_STRING = "R002";
    constexpr const char* UNTERMINATED_COMMENT = "R003";
    constexpr const char* INVALID_NUMBER = "R004";
    
    // Syntax errors (R100-R199)
    constexpr const char* UNEXPECTED_TOKEN = "R100";
    constexpr const char* EXPECTED_TOKEN = "R101";
    constexpr const char* MISSING_SEMICOLON = "R102";
    constexpr const char* MISSING_PAREN = "R103";
    constexpr const char* MISSING_BRACE = "R104";
    constexpr const char* MISSING_COLON = "R105";
    constexpr const char* INVALID_INDENTATION = "R106";
    
    // Semantic errors (R200-R299)
    constexpr const char* UNDEFINED_VARIABLE = "R200";
    constexpr const char* UNDEFINED_FUNCTION = "R201";
    constexpr const char* TYPE_MISMATCH = "R202";
    constexpr const char* DUPLICATE_DECLARATION = "R203";
    constexpr const char* INVALID_ASSIGNMENT = "R204";
    constexpr const char* INVALID_OPERATION = "R205";
    constexpr const char* INVALID_ARGUMENT_COUNT = "R206";
    
    // Runtime errors (R300-R399)
    constexpr const char* DIVISION_BY_ZERO = "R300";
    constexpr const char* INDEX_OUT_OF_BOUNDS = "R301";
    constexpr const char* KEY_NOT_FOUND = "R302";
    constexpr const char* STACK_OVERFLOW = "R303";
    constexpr const char* OUT_OF_MEMORY = "R304";
}

/**
 * @brief Error handler interface
 */
class ErrorHandler {
public:
    virtual ~ErrorHandler() = default;
    
    /**
     * @brief Report a diagnostic message
     * @param diagnostic The diagnostic to report
     */
    virtual void report(const Diagnostic& diagnostic) = 0;
    
    /**
     * @brief Check if any errors have been reported
     * @return True if errors exist
     */
    virtual bool hasErrors() const = 0;
    
    /**
     * @brief Get all reported diagnostics
     * @return Vector of diagnostics
     */
    virtual const std::vector<Diagnostic>& getDiagnostics() const = 0;
    
    /**
     * @brief Clear all diagnostics
     */
    virtual void clear() = 0;
};

/**
 * @brief Standard error handler implementation
 */
class StandardErrorHandler : public ErrorHandler {
public:
    StandardErrorHandler() = default;
    
    void report(const Diagnostic& diagnostic) override;
    bool hasErrors() const override;
    const std::vector<Diagnostic>& getDiagnostics() const override;
    void clear() override;
    
private:
    std::vector<Diagnostic> diagnostics_;
    bool has_errors_ = false;
};

/**
 * @brief Utility functions for creating common diagnostics
 */
class DiagnosticBuilder {
public:
    static Diagnostic unexpectedChar(char c, const SourceLocation& loc);
    static Diagnostic unexpectedToken(const Token& token, const std::string& expected = "");
    static Diagnostic undefinedVariable(const std::string& name, const SourceLocation& loc);
    static Diagnostic typeMismatch(const std::string& expected, const std::string& actual, 
                                   const SourceLocation& loc);
    static Diagnostic divisionByZero(const SourceLocation& loc);
    
    // Syntax error helpers
    static Diagnostic missingSemicolon(const SourceLocation& loc);
    static Diagnostic missingParen(const std::string& type, const SourceLocation& loc);
    static Diagnostic missingColon(const SourceLocation& loc);
    static Diagnostic invalidIndentation(const SourceLocation& loc);
    
    // Multi-line error support
    static Diagnostic multiLineError(const Token& startToken, const Token& endToken, 
                                     const std::string& message, const std::string& code = "");
    static Diagnostic syntaxError(const SourceLocation& loc, const std::string& message, 
                                 const std::string& sourceLine = "", int column = 0, 
                                 int endColumn = 0, int endLine = 0);
    
    // Additional methods for Python-style error messages
    static Diagnostic parenthesisNotClosed(const Token& token, const std::string& filename = "<stdin>");
    static Diagnostic bracketNotClosed(const Token& token, const std::string& filename = "<stdin>");
    static Diagnostic braceNotClosed(const Token& token, const std::string& filename = "<stdin>");
};

} // namespace rglite

#endif // ERROR_HANDLER_H
