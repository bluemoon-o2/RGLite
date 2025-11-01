// RGLite Lexer - Tokenizer for RGLite language
// This file defines the lexical analyzer that converts source code into tokens

#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include <memory>
#include "Token.h"
#include "ErrorHandler.h"
#include "StringUtils.h"

namespace rglite {

/**
 * @brief Lexical analyzer for RGLite language
 * 
 * The lexer converts source code into a stream of tokens that can be
 * processed by the parser. It handles:
 * - Whitespace and comments
 * - Identifiers and keywords
 * - Numeric literals (integers, floats)
 * - String literals (with escape sequences)
 * - Operators and punctuation
 * - Error reporting for invalid tokens
 */
class Lexer {
public:
    /**
     * @brief Construct a lexer for the given source code
     * @param source The source code to tokenize
     * @param filename The source filename (for error reporting)
     * @param errorHandler The error handler to use
     */
    Lexer(const std::string& source, const std::string& filename = "",
          std::shared_ptr<ErrorHandler> errorHandler = nullptr);
    
    /**
     * @brief Get the next token from the source
     * @return The next token
     */
    Token nextToken();
    
    /**
     * @brief Peek at the next token without consuming it
     * @return The next token
     */
    Token peekToken();
    
    /**
     * @brief Check if there are more tokens to process
     * @return True if more tokens are available
     */
    bool hasMoreTokens() const;
    
    /**
     * @brief Get all tokens from the source
     * @return Vector of all tokens
     */
    std::vector<Token> tokenize();
    
    /**
     * @brief Get the current position in the source
     * @return Current source location
     */
    SourceLocation getCurrentLocation() const;
    
private:
    // Source code and position tracking
    std::string source_;
    std::string filename_;
    size_t position_;
    size_t line_;
    size_t column_;
    size_t start_;
    
    // Error handling
    std::shared_ptr<ErrorHandler> errorHandler_;
    
    // Token buffer for peeking
    Token peekedToken_;
    bool hasPeeked_;
    
    // Indentation handling
    std::vector<int> indentStack_;
    bool atLineStart_;
    bool pendingDedents_;
    int currentIndent_;
    
    /**
     * @brief Advance to the next character
     * @return The current character before advancing
     */
    char advance();
    
    /**
     * @brief Peek at the current character without consuming it
     * @return The current character
     */
    char peek() const;
    
    /**
     * @brief Peek at the character at offset from current position
     * @param offset The offset from current position
     * @return The character at offset
     */
    char peek(size_t offset) const;
    
    /**
     * @brief Check if current position is at end of source
     * @return True if at end of source
     */
    bool isAtEnd() const;
    
    /**
     * @brief Skip whitespace and comments
     */
    void skipWhitespaceAndComments();
    
    /**
     * @brief Skip single-line comment
     */
    void skipLineComment();
    
    /**
     * @brief Skip multi-line comment
     */
    void skipBlockComment();
    
    /**
     * @brief Create a token with current lexeme
     * @param type The token type
     * @return The created token
     */
    Token makeToken(TokenType type);
    
    /**
     * @brief Create a token with literal value
     * @param type The token type
     * @param literal The literal value
     * @return The created token
     */
    Token makeToken(TokenType type, const std::string& literal);
    
    /**
     * @brief Scan an identifier or keyword
     * @return The identifier or keyword token
     */
    Token scanIdentifierOrKeyword();
    
    /**
     * @brief Scan a numeric literal
     * @return The numeric literal token
     */
    Token scanNumber();
    
    /**
     * @brief Scan a string literal
     * @return The string literal token
     */
    Token scanString();
    
    /**
     * @brief Scan an operator or punctuation
     * @return The operator or punctuation token
     */
    Token scanOperator();
    
    /**
     * @brief Check if a string is a keyword
     * @param str The string to check
     * @return The corresponding token type if keyword, or IDENTIFIER
     */
    TokenType checkKeyword(const std::string& str);
    
    /**
     * @brief Report an error at current position
     * @param message The error message
     */
    void error(const std::string& message);
    
    /**
     * @brief Report an error at specific location
     * @param message The error message
     * @param location The error location
     */
    void error(const std::string& message, const SourceLocation& location);
    
    /**
     * @brief Handle line start and indentation
     * @return INDENT token or next token
     */
    Token handleLineStart();
    
    /**
     * @brief Handle pending dedent tokens
     * @return DEDENT token or next token
     */
    Token handleDedents();
};

} // namespace rglite

#endif // LEXER_H