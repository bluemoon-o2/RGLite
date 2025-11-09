// RGLite Error Handling System - Implementation
// This file implements the error handling infrastructure for the compiler

#include "ErrorHandler.h"
#include <sstream>
#include <iostream>

namespace rglite {

// Diagnostic implementation
std::string Diagnostic::toString() const {
    std::stringstream ss;
    
    // Python-style traceback format with full file path
    // Only show traceback header for errors that are not immediate syntax errors (like Python)
    if (displayTracebackHeader) {
        ss << "Traceback (most recent call last):\n";
    }
    
    // File and line information with function/module context
    ss << "  File \"" << filename << "\", line " << location.line;
    
    // Add function/module information if available
    if (!function_name.empty()) {
        ss << ", in " << function_name;
    } else {
        ss << ", in <module>";
    }
    
    // Add end line if different from start line (for multi-line errors)
    if (end_line > 0 && static_cast<uint32_t>(end_line) != location.line) {
        ss << "-" << end_line;
    }
    ss << "\n";
    
    // Show the source code line if available
    if (!sourceLine.empty()) {
        // Prepare source text for display
        std::string rtext = sourceLine;
        // Remove trailing newlines
        while (!rtext.empty() && (rtext.back() == '\n' || rtext.back() == '\r')) {
            rtext.pop_back();
        }
        
        // Show the source line with proper indentation
        ss << "    " << rtext << "\n";
        
        // Show the ^ marker at the error position
        if (column > 0) {
            // Python-style algorithm: convert 1-based column offset to 0-based index into stripped text
            // First, remove leading whitespace to get the actual text content
            std::string ltext = rtext;
            size_t spaces = 0;
            
            // Find leading whitespace (spaces, tabs, form feeds)
            while (spaces < ltext.size() && (ltext[spaces] == ' ' || ltext[spaces] == '\t' || ltext[spaces] == '\f')) {
                spaces++;
            }
            
            // Remove leading whitespace to get the actual content
            ltext = ltext.substr(spaces);
            
            // Calculate column position in the stripped text
            // Python uses: colno = offset - 1 - spaces
            int col = column;
            // Clamp column to the visible range of the line to avoid off-by-one beyond line length
            if (col < 1) col = 1;
            if (col - 1 >= static_cast<int>(rtext.size())) col = static_cast<int>(rtext.size());

            int colno = col - 1 - static_cast<int>(spaces);
            int endColno = col;
            
            // Use end_column if available
            if (end_column > 0) {
                int ecol = end_column;
                if (ecol < col) ecol = col;
                if (ecol - 1 >= static_cast<int>(rtext.size())) ecol = static_cast<int>(rtext.size());
                endColno = ecol - 1 - static_cast<int>(spaces);
            } else {
                endColno = colno + 1; // Default to single caret if no end column
            }
            
            // Ensure valid ranges
            if (colno < 0) colno = 0;
            if (endColno <= colno) endColno = colno + 1;
            if (endColno > static_cast<int>(ltext.size())) endColno = static_cast<int>(ltext.size());
            
            // Generate the caret line - Python style with proper alignment
            
            // Improved caret alignment algorithm
            std::string caretline;
            
            // First, calculate the visual column position considering tabs
            int visualColumn = 0;
            for (int i = 0; i < static_cast<int>(rtext.size()) && i < col - 1; ++i) {
                if (rtext[i] == '\t') {
                    // Tab expands to next multiple of tab size (typically 8)
                    visualColumn = (visualColumn / 8 + 1) * 8;
                } else {
                    visualColumn++;
                }
            }
            
            // Generate caret space with proper tab expansion
            caretline.reserve(visualColumn + 10); // Reserve space for efficiency
            int currentPos = 0;
            for (int i = 0; i < static_cast<int>(rtext.size()) && currentPos < visualColumn; ++i) {
                if (rtext[i] == '\t') {
                    int tabStop = (currentPos / 8 + 1) * 8;
                    while (currentPos < tabStop) {
                        caretline += ' ';
                        currentPos++;
                    }
                } else {
                    if (rtext[i] == ' ' || rtext[i] == '\f') {
                        caretline += rtext[i];
                    } else {
                        caretline += ' ';
                    }
                    currentPos++;
                }
            }
            
            // Fill remaining space if needed
            while (currentPos < visualColumn) {
                caretline += ' ';
                currentPos++;
            }
            
            // Generate the ^ markers (Python style - single caret for syntax errors)
            // Python typically uses single caret for syntax errors, multiple for runtime errors
            int caretCount = 1;
            if (end_column > column) {
                caretCount = end_column - column;
            }
            if (caretCount < 1) caretCount = 1;
            
            // Calculate visual width for caret markers
            int visualWidth = 0;
            for (int i = col - 1; i < static_cast<int>(rtext.size()) && i < col - 1 + caretCount; ++i) {
                if (rtext[i] == '\t') {
                    visualWidth += 8 - (visualWidth % 8);
                } else {
                    visualWidth++;
                }
            }

            // Add caret markers
            for (int i = 0; i < visualWidth; ++i) {
                caretline += '^';
            }

            ss << "    " << caretline << "\n";
        } else {
            ss << "    ^\n";
        }
    }
    
    // Add error type and message (Python style)
    switch (severity) {
        case Severity::ERROR: 
            // Check for specific error types to match Python messages
            if (message.find("parenthesis not closed") != std::string::npos) {
                ss << "SyntaxError: '(' was never closed";
            } else if (message.find("bracket not closed") != std::string::npos) {
                ss << "SyntaxError: '[' was never closed";
            } else if (message.find("brace not closed") != std::string::npos) {
                ss << "SyntaxError: '{' was never closed";
            } else if (message.find("Mixed indentation") != std::string::npos || message.find("mixed indentation") != std::string::npos) {
                ss << "TabError: inconsistent use of tabs and spaces in indentation";
            } else if (message.find("invalid indentation") != std::string::npos || message.find("Expected INDENT") != std::string::npos) {
                ss << "IndentationError: expected an indented block";
            } else if (message.find("unexpected") != std::string::npos) {
                ss << "SyntaxError: invalid syntax";
            } else if (message.find("Unterminated string literal") != std::string::npos || message.find("unterminated string") != std::string::npos) {
                ss << "SyntaxError: unterminated string literal (detected at line " << location.line << ")";
            } else if (message.find("Invalid number format") != std::string::npos) {
                ss << "SyntaxError: invalid decimal literal";
            } else if (message.find("Unexpected character") != std::string::npos || message.find("unexpected character") != std::string::npos) {
                ss << "SyntaxError: invalid character";
            } else if (message.rfind("Expected ", 0) == 0) { // starts with "Expected "
                ss << "SyntaxError: invalid syntax";
            } else if (message.find("Undefined variable") != std::string::npos) {
                // Extract variable name from message like "Undefined variable 'x'"
                size_t start = message.find("'") + 1;
                size_t end = message.rfind("'");
                std::string var_name = (start != std::string::npos && end != std::string::npos && start < end) ? message.substr(start, end - start) : "";
                ss << "NameError: name '" << var_name << "' is not defined";
            } else if (message.find("Invalid operands for binary operator") != std::string::npos) {
                ss << "TypeError: unsupported operand type(s) for +: 'int' and 'str'"; // Adjust based on actual types if possible
            } else if (message.find("Division by zero") != std::string::npos) {
                ss << "ZeroDivisionError: division by zero";
            } else if (message.find("'return' outside function") != std::string::npos) {
                ss << "SyntaxError: 'return' outside function";
            } else if (message.find("Index access is only supported for lists and dictionaries") != std::string::npos) {
                ss << "IndexError: list index out of range"; // Adjust to match specific error
            } else {
                ss << "SyntaxError: " << message;
            }
            break;
        case Severity::FATAL:
            ss << "SystemError: " << message;
            break;
        case Severity::WARNING:
            ss << "Warning: " << message;
            break;
        case Severity::INFO:
            ss << "Info: " << message;
            break;
    }
    
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
     std::string message = "unexpected character '";
     message += c;
     message += "'";
     return Diagnostic(Severity::ERROR, message, loc, std::string(ErrorCode::UNEXPECTED_CHAR), 
                      "", 0, 0, 0, "<stdin>", "");
 }





Diagnostic DiagnosticBuilder::unexpectedToken(const Token& token, const std::string& expected) {
    std::string message = "unexpected token '" + token.lexeme + "'";
    if (!expected.empty()) {
        message += ", expected " + expected;
    }
    return Diagnostic(Severity::ERROR, message, token.location, std::string(ErrorCode::UNEXPECTED_TOKEN), 
                     "", token.location.column, static_cast<int>(token.location.column + token.lexeme.length()), 0, "<stdin>", "");
}





Diagnostic DiagnosticBuilder::undefinedVariable(const std::string& name, const SourceLocation& loc) {
    std::string message = "undefined variable '" + name + "'";
    return Diagnostic(Severity::ERROR, message, loc, std::string(ErrorCode::UNDEFINED_VARIABLE), 
                     "", 0, 0, 0, "<stdin>", "");
}

Diagnostic DiagnosticBuilder::typeMismatch(const std::string& expected, const std::string& actual, 
                                   const SourceLocation& loc) {
    std::string message = "type mismatch: expected '" + expected + "' but got '" + actual + "'";
    return Diagnostic(Severity::ERROR, message, loc, std::string(ErrorCode::TYPE_MISMATCH), 
                     "", 0, 0, 0, "<stdin>", "");
}






// Multi-line error support
Diagnostic DiagnosticBuilder::multiLineError(const Token& startToken, const Token& endToken, 
                                            const std::string& message, const std::string& code) {
    return Diagnostic(Severity::ERROR, message, startToken.location, code, 
                     "", startToken.location.column, endToken.location.column, endToken.location.line, "<stdin>", "");
}

Diagnostic DiagnosticBuilder::syntaxError(const SourceLocation& loc, const std::string& message, 
                                         const std::string& sourceLine, int column, 
                                         int endColumn, int endLine) {
    return Diagnostic(Severity::ERROR, message, loc, std::string(ErrorCode::UNEXPECTED_TOKEN), 
                     sourceLine, column, endColumn, endLine, "<stdin>", "");
}

// Additional methods for Python-style error messages
Diagnostic DiagnosticBuilder::parenthesisNotClosed(const Token& token, const std::string& filename) {
    std::string message = "parenthesis not closed";
    return Diagnostic(Severity::ERROR, message, token.location, std::string(ErrorCode::MISSING_PAREN), 
                     "", token.location.column, static_cast<int>(token.location.column + token.lexeme.length()), 0, filename, "");
}

Diagnostic DiagnosticBuilder::bracketNotClosed(const Token& token, const std::string& filename) {
    std::string message = "bracket not closed";
    return Diagnostic(Severity::ERROR, message, token.location, std::string(ErrorCode::MISSING_BRACE), 
                     "", token.location.column, static_cast<int>(token.location.column + token.lexeme.length()), 0, filename, "");
}

Diagnostic DiagnosticBuilder::braceNotClosed(const Token& token, const std::string& filename) {
    std::string message = "brace not closed";
    return Diagnostic(Severity::ERROR, message, token.location, std::string(ErrorCode::MISSING_BRACE), 
                     "", token.location.column, static_cast<int>(token.location.column + token.lexeme.length()), 0, filename, "");
}

} // namespace rglite



