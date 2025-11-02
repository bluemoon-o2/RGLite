// RGLite Lexer Implementation
// This file implements the lexical analyzer for RGLite language

#include "Lexer.h"
#include <unordered_map>
#include <cctype>
#include <iostream>

namespace rglite {

// Keyword mapping
static const std::unordered_map<std::string, TokenType> KEYWORDS = {
    // Control flow
    {"if", TokenType::KW_IF},
    {"else", TokenType::KW_ELSE},
    {"elif", TokenType::KW_ELIF},
    {"while", TokenType::KW_WHILE},
    {"for", TokenType::KW_FOR},
    {"in", TokenType::KW_IN},
    {"return", TokenType::KW_RETURN},
    
    // Function and class definitions
    {"def", TokenType::KW_DEF},
    {"class", TokenType::KW_CLASS},
    
    // Boolean literals
    {"true", TokenType::KW_TRUE},
    {"false", TokenType::KW_FALSE},
    {"none", TokenType::KW_NONE},
    
    // Logical operators
    {"and", TokenType::KW_AND},
    {"or", TokenType::KW_OR},
    {"not", TokenType::KW_NOT}
};

Lexer::Lexer(const std::string& source, const std::string& filename,
             std::shared_ptr<ErrorHandler> errorHandler)
    : source_(source), filename_(filename), errorHandler_(errorHandler),
      position_(0), line_(1), column_(1), start_(0),
      peekedToken_(), hasPeeked_(false), atLineStart_(true), pendingDedents_(false), currentIndent_(0), targetIndentLevel_(0) {
    
    if (!errorHandler_) {
        errorHandler_ = std::make_shared<StandardErrorHandler>();
    }
    
    // Initialize indent stack with 0 (base level)
    indentStack_.push_back(0);
}

Token Lexer::nextToken() {
    if (hasPeeked_) {
        hasPeeked_ = false;
        return peekedToken_;
    }
    
    // Handle pending dedents first
    if (pendingDedents_) {
        return handleDedents();
    }
    
    // Handle line start and indentation
    if (atLineStart_ && !isAtEnd()) {
        return handleLineStart();
    }
    
    // Skip whitespace and comments
    while (!isAtEnd()) {
        char c = peek();
        
        if (c == '\n') {
            // Handle newline
            start_ = position_;
            advance(); // Consume the newline
            atLineStart_ = true;
            return makeToken(TokenType::NEWLINE);
        } else if (StringUtils::isWhitespace(c) && c != '\n') {
            // Skip other whitespace
            advance();
        } else if (c == '#') {
            skipLineComment();
        } else if (c == '/' && peek(1) == '*') {
            skipBlockComment();
        } else {
            break;
        }
    }
    
    if (isAtEnd()) {
        // Handle end of file - generate any pending dedents
        if (!indentStack_.empty() && indentStack_.back() > 0) {
            pendingDedents_ = true;
            // Set target indentation level to 0 for EOF
            targetIndentLevel_ = 0;
            return handleDedents();
        }
        
        // Create EOF token with empty lexeme
        SourceLocation loc = {static_cast<uint32_t>(line_), static_cast<uint32_t>(column_)};
        return Token{TokenType::END_OF_FILE, "", loc};
    }
    
    start_ = position_;
    char c = advance();
    
    // Identifiers and keywords
    if (StringUtils::isIdentifierStart(c)) {
        return scanIdentifierOrKeyword();
    }
    
    // Numbers
    if (StringUtils::isDigit(c)) {
        return scanNumber();
    }
    
    // Strings
    if (c == '"' || c == '\'') {
        return scanString();
    }
    
    // Operators and punctuation
    return scanOperator();
}

Token Lexer::peekToken() {
    if (!hasPeeked_) {
        peekedToken_ = nextToken();
        hasPeeked_ = true;
    }
    return peekedToken_;
}

bool Lexer::hasMoreTokens() const {
    if (hasPeeked_) {
        return peekedToken_.type != TokenType::END_OF_FILE;
    }
    
    // Save current state (unused variables removed)
    
    // Create temporary lexer to check next token
    Lexer tempLexer(source_.substr(position_), filename_, errorHandler_);
    tempLexer.line_ = line_;
    tempLexer.column_ = column_;
    
    Token next = tempLexer.nextToken();
    return next.type != TokenType::END_OF_FILE;
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    
    while (true) {
        Token token = nextToken();
        tokens.push_back(token);
        
        if (token.type == TokenType::END_OF_FILE) {
            break;
        }
    }
    
    return tokens;
}

SourceLocation Lexer::getCurrentLocation() const {
    return SourceLocation{static_cast<uint32_t>(line_), static_cast<uint32_t>(column_)};
}

char Lexer::advance() {
    if (isAtEnd()) {
        return '\0';
    }
    
    char c = source_[position_++];
    
    if (c == '\n') {
        line_++;
        column_ = 1;
        atLineStart_ = true;
    } else {
        column_++;
    }
    
    return c;
}

char Lexer::peek() const {
    return peek(0);
}

char Lexer::peek(size_t offset) const {
    if (position_ + offset >= source_.length()) {
        return '\0';
    }
    return source_[position_ + offset];
}

bool Lexer::isAtEnd() const {
    return position_ >= source_.length();
}

void Lexer::skipWhitespaceAndComments() {
    while (!isAtEnd()) {
        char c = peek();
        
        if (StringUtils::isWhitespace(c)) {
            advance();
        } else if (c == '#') {
            skipLineComment();
        } else if (c == '/' && peek(1) == '*') {
            skipBlockComment();
        } else {
            break;
        }
    }
}

void Lexer::skipLineComment() {
    while (!isAtEnd() && peek() != '\n') {
        advance();
    }
}

void Lexer::skipBlockComment() {
    // Skip opening /*
    advance(); // /
    advance(); // *
    
    while (!isAtEnd()) {
        if (peek() == '*' && peek(1) == '/') {
            advance(); // *
            advance(); // /
            return;
        }
        advance();
    }
    
    // Unterminated block comment
    error("Unterminated block comment");
}

Token Lexer::makeToken(TokenType type) {
    std::string lexeme = source_.substr(start_, position_ - start_);
    SourceLocation loc = {static_cast<uint32_t>(line_), static_cast<uint32_t>(column_ - lexeme.length())};
    return Token{type, lexeme, loc};
}

Token Lexer::makeToken(TokenType type, const std::string& literal) {
    SourceLocation loc = {static_cast<uint32_t>(line_), static_cast<uint32_t>(column_ - literal.length())};
    return Token{type, literal, loc};
}

Token Lexer::scanIdentifierOrKeyword() {
    while (StringUtils::isIdentifierChar(peek())) {
        advance();
    }
    
    std::string lexeme = source_.substr(start_, position_ - start_);
    TokenType type = checkKeyword(lexeme);
    
    return makeToken(type);
}

Token Lexer::scanNumber() {
    bool isFloat = false;
    bool isHex = false;
    
    // Check for hexadecimal prefix (0x or 0X)
    // Note: The first character has already been consumed by nextToken()
    // So we need to check the current character (which is the first character of the number)
    // and the next character using peek()
    if (source_[start_] == '0' && (peek() == 'x' || peek() == 'X')) {
        isHex = true;
        advance(); // Consume 'x' or 'X'
        
        // Hexadecimal digits
        while (StringUtils::isDigit(peek()) || 
               (peek() >= 'a' && peek() <= 'f') || 
               (peek() >= 'A' && peek() <= 'F')) {
            advance();
        }
    } else {
        // Integer part (decimal)
        while (StringUtils::isDigit(peek())) {
            advance();
        }
        
        // Decimal point
        if (peek() == '.') {
            isFloat = true;
            advance();
            
            // Fractional part
            while (StringUtils::isDigit(peek())) {
                advance();
            }
        }
        
        // Exponent
        if (peek() == 'e' || peek() == 'E') {
            isFloat = true;
            advance();
            
            // Optional sign
            if (peek() == '+' || peek() == '-') {
                advance();
            }
            
            // Exponent digits
            while (StringUtils::isDigit(peek())) {
                advance();
            }
        }
    }
    
    // Check if the number is followed by identifier characters (invalid format)
    if (StringUtils::isIdentifierChar(peek())) {
        error("Invalid number format: unexpected character after number");
        return makeToken(TokenType::INVALID);
    }
    
    // Extract the number literal
    std::string numberLiteral = source_.substr(start_, position_ - start_);
    
    if (isFloat) {
        Token token = makeToken(TokenType::FLOAT, numberLiteral);
        return token;
    } else {
        Token token = makeToken(TokenType::INTEGER, numberLiteral);
        return token;
    }
}

Token Lexer::scanString() {
    char quote = source_[start_]; // " or '
    std::string value;
    
    while (!isAtEnd()) {
        char c = advance();
        
        if (c == quote) {
            // End of string
            return makeToken(TokenType::STRING, value);
        }
        
        if (c == '\\') {
            // Escape sequence
            if (isAtEnd()) {
                error("Unterminated string literal");
                return makeToken(TokenType::STRING, value);
            }
            
            char escape = advance();
            switch (escape) {
                case 'n': value += '\n'; break;
                case 'r': value += '\r'; break;
                case 't': value += '\t'; break;
                case '\\': value += '\\'; break;
                case '"': value += '"'; break;
                case '\'': value += '\''; break;
                default: value += escape; break;
            }
        } else {
            value += c;
        }
    }
    
    // Unterminated string
    error("Unterminated string literal");
    return makeToken(TokenType::STRING, value);
}

Token Lexer::scanOperator() {
    char c = source_[start_];
    
    switch (c) {
        // Single-character operators
        case '(': return makeToken(TokenType::PUNCT_LEFT_PAREN);
        case ')': return makeToken(TokenType::PUNCT_RIGHT_PAREN);
        case '[': return makeToken(TokenType::PUNCT_LEFT_BRACKET);
        case ']': return makeToken(TokenType::PUNCT_RIGHT_BRACKET);
        case '{': return makeToken(TokenType::PUNCT_LEFT_BRACE);
        case '}': return makeToken(TokenType::PUNCT_RIGHT_BRACE);
        case ',': return makeToken(TokenType::PUNCT_COMMA);
        case ';': return makeToken(TokenType::PUNCT_SEMICOLON);
        case ':': return makeToken(TokenType::PUNCT_COLON);
        case '.': return makeToken(TokenType::PUNCT_DOT);
        
        // Potential multi-character operators
        case '=':
            if (peek() == '=') {
                advance();
                return makeToken(TokenType::OP_EQUAL);
            }
            return makeToken(TokenType::OP_ASSIGN);
            
        case '!':
            if (peek() == '=') {
                advance();
                return makeToken(TokenType::OP_NOT_EQUAL);
            }
            return makeToken(TokenType::KW_NOT);
            
        case '<':
            if (peek() == '=') {
                advance();
                return makeToken(TokenType::OP_LESS_EQUAL);
            }
            return makeToken(TokenType::OP_LESS);
            
        case '>':
            if (peek() == '=') {
                advance();
                return makeToken(TokenType::OP_GREATER_EQUAL);
            }
            return makeToken(TokenType::OP_GREATER);
            
        case '+':
            if (peek() == '=') {
                advance();
                return makeToken(TokenType::OP_PLUS);
            }
            return makeToken(TokenType::OP_PLUS);
            
        case '-':
            if (peek() == '=') {
                advance();
                return makeToken(TokenType::OP_MINUS);
            }
            return makeToken(TokenType::OP_MINUS);
            
        case '*':
            if (peek() == '=') {
                advance();
                return makeToken(TokenType::OP_MULTIPLY);
            }
            return makeToken(TokenType::OP_MULTIPLY);
            
        case '/':
            if (peek() == '=') {
                advance();
                return makeToken(TokenType::OP_DIVIDE);
            }
            return makeToken(TokenType::OP_DIVIDE);
            
        case '%':
            if (peek() == '=') {
                advance();
                return makeToken(TokenType::OP_MODULO);
            }
            return makeToken(TokenType::OP_MODULO);
            
        case '&':
            if (peek() == '&') {
                advance();
                return makeToken(TokenType::KW_AND);
            }
            return makeToken(TokenType::INVALID);
            
        case '|':
            if (peek() == '|') {
                advance();
                return makeToken(TokenType::KW_OR);
            }
            return makeToken(TokenType::INVALID);
            
        default:
            // Invalid character
            error("Unexpected character: " + std::string(1, c));
            return makeToken(TokenType::INVALID);
    }
}

TokenType Lexer::checkKeyword(const std::string& str) {
    auto it = KEYWORDS.find(str);
    if (it != KEYWORDS.end()) {
        return it->second;
    }
    return TokenType::IDENTIFIER;
}

void Lexer::error(const std::string& message) {
    SourceLocation loc = {static_cast<uint32_t>(line_), static_cast<uint32_t>(column_)};
    error(message, loc);
}

void Lexer::error(const std::string& message, const SourceLocation& location) {
    if (errorHandler_) {
        errorHandler_->report(Diagnostic(Severity::ERROR, message, location));
    }
}

Token Lexer::handleLineStart() {
    // Calculate current indentation level
    int indentLevel = 0;
    bool hasSpaces = false;
    bool hasTabs = false;
    
    // Save the starting position for INDENT token creation
    start_ = position_;
    
    while (!isAtEnd()) {
        char c = peek();
        if (c == ' ') {
            indentLevel++;
            hasSpaces = true;
            advance();
        } else if (c == '\t') {
            // Treat tabs as 4 spaces (Python convention)
            indentLevel += 4;
            hasTabs = true;
            advance();
        } else if (c == '\n') {
            // Empty line, skip it and continue with next line
            advance();
            line_++;
            column_ = 1;
            // Continue processing the next line's indentation
            indentLevel = 0;
            hasSpaces = false;
            hasTabs = false;
            // Update start position for the next line
            start_ = position_;
        } else {
            break;
        }
    }
    
    // If we're at end of file after whitespace
    if (isAtEnd()) {
        atLineStart_ = false;
        return nextToken();
    }
    
    // Check if this is a comment line
    if (peek() == '#') {
        // Skip the comment line
        skipLineComment();
        atLineStart_ = true;
        return nextToken();
    }
    
    // Check for mixed indentation (spaces and tabs in the same line)
    if (hasSpaces && hasTabs) {
        error("Mixed indentation: tabs and spaces cannot be mixed");
        // Continue processing despite the error
    }
    
    atLineStart_ = false;
    
    // Compare with current indent level
    int currentLevel = indentStack_.back();
    
    if (indentLevel > currentLevel) {
        // Increase indentation
        indentStack_.push_back(indentLevel);
        currentIndent_ = indentLevel;
        return makeToken(TokenType::INDENT);
    } else if (indentLevel < currentLevel) {
        // Decrease indentation - generate only one DEDENT at a time
        pendingDedents_ = true;
        // Set target to one level less than current (not the final target)
        targetIndentLevel_ = currentLevel - 1;
        // Pop one level from the stack
        indentStack_.pop_back();
        currentIndent_ = indentStack_.back();
        // Check if we need more DEDENTs
        if (indentStack_.back() > indentLevel) {
            pendingDedents_ = true;
        } else {
            pendingDedents_ = false;
        }
        return makeToken(TokenType::DEDENT);
    } else {
        // Same indentation level, continue with normal token
        return nextToken();
    }
}

Token Lexer::handleDedents() {
    if (!pendingDedents_) {
        return nextToken();
    }
    
    // Generate DEDENT tokens until we match the target indentation
    if (indentStack_.size() > 1 && indentStack_.back() > targetIndentLevel_) {
        // Pop from stack
        indentStack_.pop_back();
        currentIndent_ = indentStack_.back();
        
        // Check if we need more DEDENTs
        if (indentStack_.back() > targetIndentLevel_) {
            // Set pending flag to check if we need more DEDENTs in the next call
            pendingDedents_ = true;
        } else {
            // We've reached the target indentation level, reset the pending flag
            pendingDedents_ = false;
        }
        
        // DEDENT token should have empty lexeme for proper column calculation
        return makeToken(TokenType::DEDENT);
    } else {
        // No more DEDENTs needed
        pendingDedents_ = false;
        return nextToken();
    }
}

} // namespace rglite