// RGLite Token Implementation
// This file implements the Token class methods

#include "Token.h"
#include <sstream>

namespace rglite {

std::string Token::typeName() const {
    switch (type) {
        case TokenType::END_OF_FILE: return "END_OF_FILE";
        case TokenType::INVALID: return "INVALID";
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::INTEGER: return "INTEGER";
        case TokenType::FLOAT: return "FLOAT";
        case TokenType::STRING: return "STRING";
        case TokenType::KW_IF: return "KW_IF";
        case TokenType::KW_ELSE: return "KW_ELSE";
        case TokenType::KW_ELIF: return "KW_ELIF";
        case TokenType::KW_WHILE: return "KW_WHILE";
        case TokenType::KW_FOR: return "KW_FOR";
        case TokenType::KW_DEF: return "KW_DEF";
        case TokenType::KW_RETURN: return "KW_RETURN";
        case TokenType::KW_CLASS: return "KW_CLASS";
        case TokenType::KW_TRUE: return "KW_TRUE";
        case TokenType::KW_FALSE: return "KW_FALSE";
        case TokenType::KW_NONE: return "KW_NONE";
        case TokenType::KW_AND: return "KW_AND";
        case TokenType::KW_OR: return "KW_OR";
        case TokenType::KW_NOT: return "KW_NOT";
        case TokenType::KW_IN: return "KW_IN";
        case TokenType::OP_PLUS: return "OP_PLUS";
        case TokenType::OP_MINUS: return "OP_MINUS";
        case TokenType::OP_MULTIPLY: return "OP_MULTIPLY";
        case TokenType::OP_DIVIDE: return "OP_DIVIDE";
        case TokenType::OP_MODULO: return "OP_MODULO";
        case TokenType::OP_ASSIGN: return "OP_ASSIGN";
        case TokenType::OP_EQUAL: return "OP_EQUAL";
        case TokenType::OP_NOT_EQUAL: return "OP_NOT_EQUAL";
        case TokenType::OP_LESS: return "OP_LESS";
        case TokenType::OP_GREATER: return "OP_GREATER";
        case TokenType::OP_LESS_EQUAL: return "OP_LESS_EQUAL";
        case TokenType::OP_GREATER_EQUAL: return "OP_GREATER_EQUAL";
        case TokenType::PUNCT_LEFT_PAREN: return "PUNCT_LEFT_PAREN";
        case TokenType::PUNCT_RIGHT_PAREN: return "PUNCT_RIGHT_PAREN";
        case TokenType::PUNCT_LEFT_BRACE: return "PUNCT_LEFT_BRACE";
        case TokenType::PUNCT_RIGHT_BRACE: return "PUNCT_RIGHT_BRACE";
        case TokenType::PUNCT_LEFT_BRACKET: return "PUNCT_LEFT_BRACKET";
        case TokenType::PUNCT_RIGHT_BRACKET: return "PUNCT_RIGHT_BRACKET";
        case TokenType::PUNCT_COMMA: return "PUNCT_COMMA";
        case TokenType::PUNCT_COLON: return "PUNCT_COLON";
        case TokenType::PUNCT_DOT: return "PUNCT_DOT";
        case TokenType::PUNCT_SEMICOLON: return "PUNCT_SEMICOLON";
        case TokenType::COMMENT_LINE: return "COMMENT_LINE";
        case TokenType::COMMENT_BLOCK: return "COMMENT_BLOCK";
        case TokenType::INDENT: return "INDENT";
        case TokenType::DEDENT: return "DEDENT";
        case TokenType::NEWLINE: return "NEWLINE";
        default: return "UNKNOWN";
    }
}

std::string Token::toString() const {
    std::ostringstream oss;
    oss << "Token(type=" << typeName() << ", lexeme='" << lexeme 
        << "', location=" << location.toString() << ")";
    return oss.str();
}

} // namespace rglite