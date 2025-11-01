// RGLite Token Definitions
// This file defines the token types and structures for lexical analysis

#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <cstdint>
#include <ostream>

namespace rglite {

/**
 * @brief Token types in RGLite language
 */
enum class TokenType : uint16_t {
    // Special tokens
    END_OF_FILE = 0,
    INVALID,
    
    // Identifiers and literals
    IDENTIFIER,
    INTEGER,
    FLOAT,
    STRING,
    
    // Keywords
    KW_IF,
    KW_ELSE,
    KW_ELIF,
    KW_WHILE,
    KW_FOR,
    KW_DEF,
    KW_RETURN,
    KW_CLASS,
    KW_TRUE,
    KW_FALSE,
    KW_NONE,
    KW_AND,
    KW_OR,
    KW_NOT,
    KW_IN,
    
    // Operators
    OP_PLUS,        // +
    OP_MINUS,       // -
    OP_MULTIPLY,    // *
    OP_DIVIDE,      // /
    OP_MODULO,      // %
    OP_ASSIGN,      // =
    OP_EQUAL,       // ==
    OP_NOT_EQUAL,   // !=
    OP_LESS,        // <
    OP_GREATER,     // >
    OP_LESS_EQUAL,  // <=
    OP_GREATER_EQUAL, // >=
    
    // Punctuation
    PUNCT_LEFT_PAREN,    // (
    PUNCT_RIGHT_PAREN,   // )
    PUNCT_LEFT_BRACE,    // {
    PUNCT_RIGHT_BRACE,   // }
    PUNCT_LEFT_BRACKET,  // [
    PUNCT_RIGHT_BRACKET, // ]
    PUNCT_COMMA,         // ,
    PUNCT_COLON,         // :
    PUNCT_DOT,           // .
    PUNCT_SEMICOLON,     // ;
    
    // Comments
    COMMENT_LINE,    // # ...
    COMMENT_BLOCK,   // /* ... */
    
    // Indentation
    INDENT,
    DEDENT,
    NEWLINE
};

/**
 * @brief Source code location information
 */
struct SourceLocation {
    uint32_t line = 1;      // 1-based line number
    uint32_t column = 1;    // 1-based column number
    uint32_t offset = 0;    // Byte offset in source
    
    SourceLocation() = default;
    SourceLocation(uint32_t l, uint32_t c, uint32_t o = 0) : line(l), column(c), offset(o) {}
    
    std::string toString() const {
        return "line " + std::to_string(line) + ", column " + std::to_string(column);
    }
};

/**
 * @brief Token structure representing a lexical unit
 */
struct Token {
    TokenType type = TokenType::INVALID;
    std::string lexeme;           // The actual text of the token
    SourceLocation location;      // Where the token appears in source
    
    // For literals
    union {
        int64_t int_value;
        double float_value;
    };
    
    Token() = default;
    
    Token(TokenType t, const std::string& l, const SourceLocation& loc)
        : type(t), lexeme(l), location(loc) {}
    
    Token(TokenType t, const std::string& l, uint32_t line, uint32_t col)
        : type(t), lexeme(l), location(line, col) {}
    
    // Copy constructor
    Token(const Token& other)
        : type(other.type), lexeme(other.lexeme), location(other.location) {
        // Copy union values
        if (type == TokenType::INTEGER) {
            int_value = other.int_value;
        } else if (type == TokenType::FLOAT) {
            float_value = other.float_value;
        }
    }
    
    // Copy assignment operator
    Token& operator=(const Token& other) {
        if (this != &other) {
            type = other.type;
            lexeme = other.lexeme;
            location = other.location;
            
            // Copy union values
            if (type == TokenType::INTEGER) {
                int_value = other.int_value;
            } else if (type == TokenType::FLOAT) {
                float_value = other.float_value;
            }
        }
        return *this;
    }
    
    // Check if token is a specific type
    bool is(TokenType t) const { return type == t; }
    
    // Check if token is a keyword
    bool isKeyword() const {
        return type >= TokenType::KW_IF && type <= TokenType::KW_IN;
    }
    
    // Check if token is an operator
    bool isOperator() const {
        return type >= TokenType::OP_PLUS && type <= TokenType::OP_GREATER_EQUAL;
    }
    
    // Get token type name as string
    std::string typeName() const;
    
    // Convert to string representation
    std::string toString() const;
};

} // namespace rglite

// Declare TokenType operator<< first so it's available when Token operator<< is compiled
inline std::ostream& operator<<(std::ostream& os, const rglite::TokenType type) {
    switch (type) {
        case rglite::TokenType::END_OF_FILE: os << "END_OF_FILE"; break;
        case rglite::TokenType::INVALID: os << "INVALID"; break;
        case rglite::TokenType::IDENTIFIER: os << "IDENTIFIER"; break;
        case rglite::TokenType::INTEGER: os << "INTEGER"; break;
        case rglite::TokenType::FLOAT: os << "FLOAT"; break;
        case rglite::TokenType::STRING: os << "STRING"; break;
        case rglite::TokenType::KW_IF: os << "KW_IF"; break;
        case rglite::TokenType::KW_ELSE: os << "KW_ELSE"; break;
        case rglite::TokenType::KW_ELIF: os << "KW_ELIF"; break;
        case rglite::TokenType::KW_WHILE: os << "KW_WHILE"; break;
        case rglite::TokenType::KW_FOR: os << "KW_FOR"; break;
        case rglite::TokenType::KW_DEF: os << "KW_DEF"; break;
        case rglite::TokenType::KW_RETURN: os << "KW_RETURN"; break;
        case rglite::TokenType::KW_CLASS: os << "KW_CLASS"; break;
        case rglite::TokenType::KW_TRUE: os << "KW_TRUE"; break;
        case rglite::TokenType::KW_FALSE: os << "KW_FALSE"; break;
        case rglite::TokenType::KW_NONE: os << "KW_NONE"; break;
        case rglite::TokenType::KW_AND: os << "KW_AND"; break;
        case rglite::TokenType::KW_OR: os << "KW_OR"; break;
        case rglite::TokenType::KW_NOT: os << "KW_NOT"; break;
        case rglite::TokenType::KW_IN: os << "KW_IN"; break;
        case rglite::TokenType::OP_PLUS: os << "OP_PLUS"; break;
        case rglite::TokenType::OP_MINUS: os << "OP_MINUS"; break;
        case rglite::TokenType::OP_MULTIPLY: os << "OP_MULTIPLY"; break;
        case rglite::TokenType::OP_DIVIDE: os << "OP_DIVIDE"; break;
        case rglite::TokenType::OP_MODULO: os << "OP_MODULO"; break;
        case rglite::TokenType::OP_ASSIGN: os << "OP_ASSIGN"; break;
        case rglite::TokenType::OP_EQUAL: os << "OP_EQUAL"; break;
        case rglite::TokenType::OP_NOT_EQUAL: os << "OP_NOT_EQUAL"; break;
        case rglite::TokenType::OP_LESS: os << "OP_LESS"; break;
        case rglite::TokenType::OP_GREATER: os << "OP_GREATER"; break;
        case rglite::TokenType::OP_LESS_EQUAL: os << "OP_LESS_EQUAL"; break;
        case rglite::TokenType::OP_GREATER_EQUAL: os << "OP_GREATER_EQUAL"; break;
        case rglite::TokenType::PUNCT_LEFT_PAREN: os << "PUNCT_LEFT_PAREN"; break;
        case rglite::TokenType::PUNCT_RIGHT_PAREN: os << "PUNCT_RIGHT_PAREN"; break;
        case rglite::TokenType::PUNCT_LEFT_BRACE: os << "PUNCT_LEFT_BRACE"; break;
        case rglite::TokenType::PUNCT_RIGHT_BRACE: os << "PUNCT_RIGHT_BRACE"; break;
        case rglite::TokenType::PUNCT_LEFT_BRACKET: os << "PUNCT_LEFT_BRACKET"; break;
        case rglite::TokenType::PUNCT_RIGHT_BRACKET: os << "PUNCT_RIGHT_BRACKET"; break;
        case rglite::TokenType::PUNCT_COMMA: os << "PUNCT_COMMA"; break;
        case rglite::TokenType::PUNCT_COLON: os << "PUNCT_COLON"; break;
        case rglite::TokenType::PUNCT_DOT: os << "PUNCT_DOT"; break;
        case rglite::TokenType::PUNCT_SEMICOLON: os << "PUNCT_SEMICOLON"; break;
        case rglite::TokenType::COMMENT_LINE: os << "COMMENT_LINE"; break;
        case rglite::TokenType::COMMENT_BLOCK: os << "COMMENT_BLOCK"; break;
        case rglite::TokenType::INDENT: os << "INDENT"; break;
        case rglite::TokenType::DEDENT: os << "DEDENT"; break;
        case rglite::TokenType::NEWLINE: os << "NEWLINE"; break;
        default: os << "UNKNOWN(" << static_cast<int>(type) << ")"; break;
    }
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const rglite::Token& token) {
    os << "Token(type=";
    os << token.type;
    os << ", lexeme='" << token.lexeme 
       << "', location=" << token.location.toString() << ")";
    return os;
}

#endif // TOKEN_H