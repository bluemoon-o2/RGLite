// RGLite Parser Implementation
// This file implements the recursive descent parser for RGLite

#include "Parser.h"
#include "ErrorHandler.h"
#include <memory>
#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>
#include <typeinfo>

namespace rglite {

Parser::Parser(std::unique_ptr<Lexer> lexer, std::shared_ptr<ErrorHandler> errorHandler)
    : lexer_(std::move(lexer)), errorHandler_(errorHandler) {
    
    if (!errorHandler_) {
        errorHandler_ = std::make_shared<StandardErrorHandler>();
    }
    
    // Initialize tokens
    nextToken();
    nextToken();
}

std::unique_ptr<Stmt> Parser::parse() {
    std::vector<std::unique_ptr<Stmt>> statements;
    
    while (!checkToken(TokenType::END_OF_FILE)) {
        try {
            auto stmt = parseStatement();
            if (stmt) {
                statements.push_back(std::move(stmt));
            }
        } catch (const std::exception&) {
            // Skip to next statement on error
            synchronize();
        }
    }
    
    // For now, return the first statement or create a block statement
    if (statements.empty()) {
        return nullptr;
    }
    
    if (statements.size() == 1) {
        return std::move(statements[0]);
    }
    
    // Create a block statement to hold all statements
    return std::make_unique<BlockStmt>(std::move(statements), SourceLocation{});
}

bool Parser::hasErrors() const {
    return hasErrors_;
}

void Parser::nextToken() {
    currentToken_ = peekToken_;
    peekToken_ = lexer_->nextToken();
}

void Parser::expectToken(TokenType expected) {
    if (matchToken(expected)) {
        return;
    }
    
    error("Expected " + tokenTypeToString(expected) + 
          ", but found " + tokenTypeToString(currentToken_.type));
}

bool Parser::matchToken(TokenType expected) {
    if (checkToken(expected)) {
        nextToken();
        return true;
    }
    return false;
}

bool Parser::checkToken(TokenType expected) const {
    return currentToken_.type == expected;
}

bool Parser::checkNextToken(TokenType expected) const {
    return peekToken_.type == expected;
}

void Parser::error(const std::string& message) {
    errorAt(currentToken_, message);
}

void Parser::errorAt(const Token& token, const std::string& message) {
    hasErrors_ = true;
    Diagnostic diagnostic(Severity::ERROR, message, token.location, ErrorCode::UNEXPECTED_TOKEN);
    errorHandler_->report(diagnostic);
}

void Parser::synchronize() {
    // Skip tokens until we find a statement boundary
    while (!checkToken(TokenType::END_OF_FILE)) {
        if (currentToken_.type == TokenType::NEWLINE) {
            nextToken();
            return;
        }
        
        // Check for statement start tokens
        if (checkToken(TokenType::KW_DEF) || checkToken(TokenType::KW_IF) ||
            checkToken(TokenType::KW_WHILE) || checkToken(TokenType::KW_FOR) ||
            checkToken(TokenType::KW_RETURN)) {
            return;
        }
        
        nextToken();
    }
}

std::unique_ptr<Stmt> Parser::parseStatement() {
    if (checkToken(TokenType::KW_DEF)) {
        return parseFunctionDefinition();
    }
    if (checkToken(TokenType::KW_IF)) {
        return parseIfStatement();
    }
    if (checkToken(TokenType::KW_WHILE)) {
        return parseWhileStatement();
    }
    if (checkToken(TokenType::KW_RETURN)) {
        return parseReturnStatement();
    }
    
    // In Python-style, all identifier = expression are treated as expression statements
    // Variable declaration is implicit through assignment
    return parseExpressionStatement();
}

std::unique_ptr<ExprStmt> Parser::parseExpressionStatement() {
    auto expr = parseExpression();
    expectToken(TokenType::NEWLINE);
    return std::make_unique<ExprStmt>(std::move(expr));
}

std::unique_ptr<FunctionDeclStmt> Parser::parseFunctionDefinition() {
    auto defToken = currentToken_;
    expectToken(TokenType::KW_DEF);
    
    auto nameToken = currentToken_;
    expectToken(TokenType::IDENTIFIER);
    
    expectToken(TokenType::PUNCT_LEFT_PAREN);
    auto parameters = parseParameterList();
    expectToken(TokenType::PUNCT_RIGHT_PAREN);
    
    expectToken(TokenType::PUNCT_COLON);
    expectToken(TokenType::NEWLINE);
    
    auto body = parseBlock();
    
    return std::make_unique<FunctionDeclStmt>(nameToken.lexeme, std::move(parameters), 
                                              std::move(body), defToken.location);
}

std::unique_ptr<IfStmt> Parser::parseIfStatement() {
    auto ifToken = currentToken_;
    expectToken(TokenType::KW_IF);
    
    auto condition = parseExpression();
    expectToken(TokenType::PUNCT_COLON);
    expectToken(TokenType::NEWLINE);
    
    auto thenBranch = parseBlock();
    
    std::unique_ptr<Stmt> elseBranch = nullptr;
    if (matchToken(TokenType::KW_ELSE)) {
        expectToken(TokenType::PUNCT_COLON);
        expectToken(TokenType::NEWLINE);
        elseBranch = parseBlock();
    }
    
    return std::make_unique<IfStmt>(std::move(condition), std::move(thenBranch), 
                                   std::move(elseBranch), ifToken.location);
}

std::unique_ptr<WhileStmt> Parser::parseWhileStatement() {
    auto whileToken = currentToken_;
    expectToken(TokenType::KW_WHILE);
    
    auto condition = parseExpression();
    expectToken(TokenType::PUNCT_COLON);
    expectToken(TokenType::NEWLINE);
    
    auto body = parseBlock();
    
    return std::make_unique<WhileStmt>(std::move(condition), std::move(body), whileToken.location);
}

std::unique_ptr<ReturnStmt> Parser::parseReturnStatement() {
    auto returnToken = currentToken_;
    expectToken(TokenType::KW_RETURN);
    
    std::unique_ptr<Expr> value = nullptr;
    if (!checkToken(TokenType::NEWLINE)) {
        value = parseExpression();
    }
    
    expectToken(TokenType::NEWLINE);
    
    return std::make_unique<ReturnStmt>(std::move(value), returnToken.location);
}

std::unique_ptr<Expr> Parser::parseExpression() {
    return parseAssignment();
}

std::unique_ptr<Expr> Parser::parseAssignment() {
    auto expr = parseLogicalOr();
    
    if (checkToken(TokenType::OP_ASSIGN)) {
        auto op = currentToken_;  // Save the assignment operator token
        nextToken();  // Consume the assignment operator
        auto value = parseAssignment();
        
        // Check if left side is an identifier
        if (auto identifier = dynamic_cast<IdentifierExpr*>(expr.get())) {
            return std::make_unique<BinaryExpr>(std::move(expr), op, std::move(value));
        }
        
        error("Invalid assignment target");
        return nullptr;
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::parseLogicalOr() {
    auto expr = parseLogicalAnd();
    
    while (checkToken(TokenType::KW_OR)) {
        auto op = currentToken_;  // Save the operator token before consuming it
        nextToken();  // Consume the 'or' operator
        auto right = parseLogicalAnd();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::parseLogicalAnd() {
    auto expr = parseEquality();
    
    while (checkToken(TokenType::KW_AND)) {
        auto op = currentToken_;  // Save the operator token before consuming it
        nextToken();  // Consume the 'and' operator
        auto right = parseEquality();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::parseEquality() {
    auto expr = parseComparison();
    
    while (checkToken(TokenType::OP_EQUAL) || checkToken(TokenType::OP_NOT_EQUAL)) {
        auto op = currentToken_;  // Save the operator token before consuming it
        nextToken();  // Consume the equality operator
        auto right = parseComparison();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::parseComparison() {
    auto expr = parseTerm();
    
    while (checkToken(TokenType::OP_LESS) || checkToken(TokenType::OP_GREATER) ||
           checkToken(TokenType::OP_LESS_EQUAL) || checkToken(TokenType::OP_GREATER_EQUAL)) {
        auto op = currentToken_;  // Save the operator token before consuming it
        nextToken();  // Consume the comparison operator
        auto right = parseTerm();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::parseTerm() {
    auto expr = parseFactor();
    
    while (checkToken(TokenType::OP_PLUS) || checkToken(TokenType::OP_MINUS)) {
        auto op = currentToken_;  // Save the operator token before consuming it
        nextToken();  // Consume the term operator
        auto right = parseFactor();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::parseFactor() {
    auto expr = parseUnary();
    
    while (checkToken(TokenType::OP_MULTIPLY) || checkToken(TokenType::OP_DIVIDE) ||
           checkToken(TokenType::OP_MODULO)) {
        auto op = currentToken_;  // Save the operator token before consuming it
        nextToken();  // Consume the factor operator
        auto right = parseUnary();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::parseUnary() {
    if (checkToken(TokenType::OP_MINUS) || checkToken(TokenType::KW_NOT)) {
        auto op = currentToken_;  // Save the operator token before consuming it
        nextToken();  // Consume the unary operator
        auto operand = parseUnary();
        return std::make_unique<UnaryExpr>(op.lexeme, std::move(operand), op.location);
    }
    
    return parseCall();
}

std::unique_ptr<Expr> Parser::parsePrimary() {
    if (checkToken(TokenType::IDENTIFIER)) {
        auto token = currentToken_;
        nextToken();
        return std::make_unique<IdentifierExpr>(token.lexeme, token.location);
    }
    
    if (checkToken(TokenType::INTEGER) || checkToken(TokenType::FLOAT) ||
        checkToken(TokenType::STRING) || checkToken(TokenType::KW_TRUE) ||
        checkToken(TokenType::KW_FALSE) || checkToken(TokenType::KW_NONE)) {
        // Save the current token before consuming it
        auto token = currentToken_;
        nextToken();
        
        auto expr = std::make_unique<LiteralExpr>(token);
        return expr;
    }
    
    if (matchToken(TokenType::PUNCT_LEFT_PAREN)) {
        auto expr = parseExpression();
        expectToken(TokenType::PUNCT_RIGHT_PAREN);
        return expr;
    }
    
    error("Expected expression");
    return nullptr;
}

std::unique_ptr<Expr> Parser::parseCall() {
    auto expr = parsePrimary();
    
    while (matchToken(TokenType::PUNCT_LEFT_PAREN)) {
        auto arguments = parseArgumentList();
        expectToken(TokenType::PUNCT_RIGHT_PAREN);
        
        expr = std::make_unique<CallExpr>(std::move(expr), std::move(arguments), currentToken_.location);
    }
    
    return expr;
}

std::unique_ptr<BlockStmt> Parser::parseBlock() {
    auto blockToken = currentToken_;
    
    // Expect indentation
    expectToken(TokenType::INDENT);
    
    std::vector<std::unique_ptr<Stmt>> statements;
    
    // Track the current indentation level
    // Start at 1 because we're already inside a block (function body, if/while body, etc.)
    int currentIndentLevel = 1;
    
    while (!checkToken(TokenType::END_OF_FILE)) {
        // If we encounter a DEDENT, check if it matches our current level
        if (checkToken(TokenType::DEDENT)) {
            currentIndentLevel--;
            
            if (currentIndentLevel == 0) {
                // This DEDENT matches our block level, consume it and exit
                nextToken();
                break;
            } else if (currentIndentLevel < 0) {
                // This should not happen, but if it does, break to avoid infinite loop
                break;
            } else {
                // This DEDENT is for a nested block, skip it and continue
                nextToken();
                continue;
            }
        }
        
        // Check if we're at the end of the block (currentIndentLevel == 0)
        if (currentIndentLevel == 0) {
            break;
        }
        
        // If we encounter an INDENT, it means we're entering a nested block
        if (checkToken(TokenType::INDENT)) {
            currentIndentLevel++;
            nextToken();
            continue;
        }
        
        // Skip NEWLINE tokens as they don't contain statements
        if (checkToken(TokenType::NEWLINE)) {
            nextToken();
            continue;
        }
        
        // Parse statements normally
        auto stmt = parseStatement();
        if (stmt) {
            statements.push_back(std::move(stmt));
        }
    }
    
    return std::make_unique<BlockStmt>(std::move(statements), blockToken.location);
}

std::vector<std::unique_ptr<Expr>> Parser::parseArgumentList() {
    std::vector<std::unique_ptr<Expr>> arguments;
    
    if (!checkToken(TokenType::PUNCT_RIGHT_PAREN)) {
        do {
            arguments.push_back(parseExpression());
        } while (matchToken(TokenType::PUNCT_COMMA));
    }
    
    return arguments;
}

std::vector<std::string> Parser::parseParameterList() {
    std::vector<std::string> parameters;
    
    if (!checkToken(TokenType::PUNCT_RIGHT_PAREN)) {
        do {
            if (checkToken(TokenType::IDENTIFIER)) {
                parameters.push_back(currentToken_.lexeme);
                nextToken();
            } else {
                error("Expected parameter name");
                break;
            }
        } while (matchToken(TokenType::PUNCT_COMMA));
    }
    
    return parameters;
}

// Helper function to convert token type to string
std::string Parser::tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::IDENTIFIER: return "identifier";
        case TokenType::INTEGER: return "integer";
        case TokenType::FLOAT: return "float";
        case TokenType::STRING: return "string";
        case TokenType::KW_IF: return "'if'";
        case TokenType::KW_ELSE: return "'else'";
        case TokenType::KW_WHILE: return "'while'";
        case TokenType::KW_FOR: return "'for'";
        case TokenType::KW_DEF: return "'def'";
        case TokenType::KW_RETURN: return "'return'";
        case TokenType::OP_ASSIGN: return "'='";
        case TokenType::PUNCT_LEFT_PAREN: return "'('";
        case TokenType::PUNCT_RIGHT_PAREN: return "')'";
        case TokenType::PUNCT_COLON: return "':'";
        case TokenType::NEWLINE: return "newline";
        case TokenType::INDENT: return "indent";
        case TokenType::DEDENT: return "dedent";
        case TokenType::END_OF_FILE: return "end of file";
        default: return "token";
    }
}

} // namespace rglite