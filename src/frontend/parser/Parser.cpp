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
#include <sstream>

namespace rglite {

Parser::Parser(std::unique_ptr<Lexer> lexer, std::shared_ptr<ErrorHandler> errorHandler)
    : lexer_(std::move(lexer)), errorHandler_(errorHandler) {
    
    if (!errorHandler_) {
        errorHandler_ = std::make_shared<StandardErrorHandler>();
    }
    
    if (lexer_) {
        source_ = lexer_->getSource();
        lexer_->setErrorHandler(errorHandler_);
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
        } catch (const std::exception& e) {
            // Report the exception as an error
            errorAt(currentToken_, std::string("Parse error: ") + e.what());
            // Skip to next statement on error
            synchronize();
        }
    }
    
    // For now, return the first statement or create a block statement
    if (statements.empty()) {
        // If we encountered errors but produced no statements, return a placeholder
        // expression statement to satisfy callers expecting a non-null statement.
        if (hasErrors()) {
            // Create a minimal placeholder: LiteralExpr(None)
            Token noneTok{TokenType::KW_NONE, "None", SourceLocation{}};
            auto placeholderExpr = std::make_unique<rglite::LiteralExpr>(noneTok);
            return std::make_unique<rglite::ExprStmt>(std::move(placeholderExpr));
        }
        return nullptr;
    }
    
    if (statements.size() == 1) {
        return std::move(statements[0]);
    }
    
    // Create a block statement to hold all statements
    return std::make_unique<rglite::BlockStmt>(std::move(statements), SourceLocation{});
}

bool Parser::hasErrors() const {
    return hasErrors_ || (errorHandler_ && errorHandler_->hasErrors());
}

void Parser::nextToken() {
    currentToken_ = peekToken_;
    peekToken_ = lexer_->nextToken();
}

void Parser::expectToken(TokenType expected) {
    if (matchToken(expected)) {
        return;
    }
    
    // Check for specific unclosed delimiters to generate Python-style errors
    if (expected == TokenType::PUNCT_RIGHT_PAREN) {
        // For parenthesis not closed, we need to find the matching left parenthesis
        // and point to its location (Python style)
        // Create a diagnostic that points to the opening parenthesis
        Diagnostic diagnostic = DiagnosticBuilder::parenthesisNotClosed(currentToken_, lexer_ ? lexer_->getFilename() : std::string("<stdin>"));
        errorHandler_->report(diagnostic);
        hasErrors_ = true;
        return;
    }
    
    if (expected == TokenType::PUNCT_RIGHT_BRACKET) {
        // For bracket not closed, point to the opening bracket
        Diagnostic diagnostic = DiagnosticBuilder::bracketNotClosed(currentToken_, lexer_ ? lexer_->getFilename() : std::string("<stdin>"));
        errorHandler_->report(diagnostic);
        hasErrors_ = true;
        return;
    }
    
    if (expected == TokenType::PUNCT_RIGHT_BRACE) {
        // For brace not closed, point to the opening brace
        Diagnostic diagnostic = DiagnosticBuilder::braceNotClosed(currentToken_, lexer_ ? lexer_->getFilename() : std::string("<stdin>"));
        errorHandler_->report(diagnostic);
        hasErrors_ = true;
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

void Parser::error(const Token& token, const std::string& message) {
    errorAt(token, message);
}

std::string Parser::getSourceLine(int line) {
    if (source_.empty()) {
        return "";
    }
    
    std::istringstream iss(source_);
    std::string currentLine;
    int currentLineNum = 1;
    
    while (std::getline(iss, currentLine)) {
        if (currentLineNum == line) {
            return currentLine;
        }
        currentLineNum++;
    }
    
    return "";  // Return empty string if line not found
}

void Parser::errorAt(const Token& token, const std::string& message) {
    // Get the source line for the error location
    std::string sourceLine = getSourceLine(token.location.line);
    int column = token.location.column;
    
    // Get the filename from the lexer
    std::string filename = lexer_ ? lexer_->getFilename() : "<unknown>";
    if (filename.empty()) filename = "<stdin>";
    
    // Create a diagnostic with source line, column, and filename info
    Diagnostic diagnostic(Severity::ERROR, message, token.location,
                          ErrorCode::UNEXPECTED_TOKEN, sourceLine, column, static_cast<int>(token.location.column + token.lexeme.length()), 0, filename);
    errorHandler_->report(diagnostic);
    
    hasErrors_ = true;  // Set the error flag
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
    // Skip NEWLINE tokens as they don't represent statements
    if (checkToken(TokenType::NEWLINE)) {
        nextToken();
        return nullptr;
    }
    
    // Skip COMMENT_LINE tokens as they don't represent statements
    if (checkToken(TokenType::COMMENT_LINE)) {
        nextToken();
        return nullptr;
    }
    
    // Skip COMMENT_BLOCK tokens as they don't represent statements
    if (checkToken(TokenType::COMMENT_BLOCK)) {
        nextToken();
        return nullptr;
    }
    
    // Skip DEDENT tokens as they don't represent statements
    if (checkToken(TokenType::DEDENT)) {
        nextToken();
        return nullptr;
    }
    
    if (checkToken(TokenType::KW_DEF)) {
        return parseFunctionDefinition();
    }
    if (checkToken(TokenType::KW_IF)) {
        return parseIfStatement();
    }
    if (checkToken(TokenType::KW_WHILE)) {
        return parseWhileStatement();
    }
    if (checkToken(TokenType::KW_FOR)) {
        return parseForStatement();
    }
    if (checkToken(TokenType::KW_RETURN)) {
        return parseReturnStatement();
    }
    if (checkToken(TokenType::END_OF_FILE)) {
        return nullptr;
    }
    
    // In Python-style, all identifier = expression are treated as expression statements
    // Variable declaration is implicit through assignment
    return parseExpressionStatement();
}

std::unique_ptr<ExprStmt> Parser::parseExpressionStatement() {
    auto expr = parseExpression();
    if (!expr) {
        // Even if expression parsing fails, we need to consume the newline to avoid infinite loop
        if (checkToken(TokenType::NEWLINE)) {
            nextToken();
        }
        return nullptr;
    }
    // Only consume NEWLINE if it's present
    // This allows for expressions at the end of files or blocks
    if (checkToken(TokenType::NEWLINE)) {
        nextToken();
    }
    return std::make_unique<rglite::ExprStmt>(std::move(expr));
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
    if (!matchToken(TokenType::NEWLINE) && !checkToken(TokenType::END_OF_FILE)) {
        error("Expected newline after colon in function definition");
    }
    
    auto body = parseBlock();
    
    return std::make_unique<rglite::FunctionDeclStmt>(nameToken.lexeme, std::move(parameters), 
                                              std::move(body), defToken.location);
}

std::unique_ptr<IfStmt> Parser::parseIfStatement() {
    auto ifToken = currentToken_;
    expectToken(TokenType::KW_IF);
    
    auto condition = parseExpression();
    expectToken(TokenType::PUNCT_COLON);
    if (!matchToken(TokenType::NEWLINE) && !checkToken(TokenType::END_OF_FILE)) {
        error("Expected newline after colon in if statement");
    }
    
    auto thenBranch = parseBlock();
    
    std::unique_ptr<Stmt> elseBranch = nullptr;
    if (matchToken(TokenType::KW_ELSE)) {
        expectToken(TokenType::PUNCT_COLON);
        if (!matchToken(TokenType::NEWLINE) && !checkToken(TokenType::END_OF_FILE)) {
            error("Expected newline after colon in else clause");
        }
        elseBranch = parseBlock();
    }
    
    return std::make_unique<rglite::IfStmt>(std::move(condition), std::move(thenBranch), 
                                   std::move(elseBranch), ifToken.location);
}

std::unique_ptr<rglite::WhileStmt> Parser::parseWhileStatement() {
    auto whileToken = currentToken_;
    expectToken(TokenType::KW_WHILE);
    
    auto condition = parseExpression();
    expectToken(TokenType::PUNCT_COLON);
    if (!matchToken(TokenType::NEWLINE) && !checkToken(TokenType::END_OF_FILE)) {
        error("Expected newline after colon in while statement");
    }
    
    auto body = parseBlock();
    
    return std::make_unique<rglite::WhileStmt>(std::move(condition), std::move(body), whileToken.location);
}

std::unique_ptr<rglite::ForStmt> Parser::parseForStatement() {
    auto forToken = currentToken_;
    expectToken(TokenType::KW_FOR);
    
    // Parse the loop variable (identifier)
    std::string variableName;
    if (checkToken(TokenType::IDENTIFIER)) {
        variableName = currentToken_.lexeme;
        nextToken();
    } else {
        error("Expected loop variable name after 'for'");
        return nullptr;
    }
    
    // Expect 'in' keyword
    expectToken(TokenType::KW_IN);
    
    // Parse the iterable expression
    auto iterable = parseExpression();
    if (!iterable) {
        error("Expected iterable expression after 'in'");
        return nullptr;
    }
    
    expectToken(TokenType::PUNCT_COLON);
    if (!matchToken(TokenType::NEWLINE) && !checkToken(TokenType::END_OF_FILE)) {
        error("Expected newline after colon in for statement");
    }
    
    // Parse the loop body
    auto body = parseBlock();
    
    return std::make_unique<rglite::ForStmt>(variableName, std::move(iterable), std::move(body), forToken.location);
}

std::unique_ptr<rglite::ReturnStmt> Parser::parseReturnStatement() {
    auto returnToken = currentToken_;
    expectToken(TokenType::KW_RETURN);
    
    std::unique_ptr<Expr> value = nullptr;
    if (!checkToken(TokenType::NEWLINE) && !checkToken(TokenType::END_OF_FILE)) {
        value = parseExpression();
    }
    
    if (!matchToken(TokenType::NEWLINE) && !checkToken(TokenType::END_OF_FILE)) {
        error("Expected newline after return statement");
    }
    
    return std::make_unique<rglite::ReturnStmt>(std::move(value), returnToken.location);
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
            return std::make_unique<rglite::BinaryExpr>(std::move(expr), op, std::move(value));
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
        expr = std::make_unique<rglite::BinaryExpr>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::parseLogicalAnd() {
    auto expr = parseEquality();
    
    while (checkToken(TokenType::KW_AND)) {
        auto op = currentToken_;  // Save the operator token before consuming it
        nextToken();  // Consume the 'and' operator
        auto right = parseEquality();
        expr = std::make_unique<rglite::BinaryExpr>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::parseEquality() {
    auto expr = parseComparison();
    
    while (checkToken(TokenType::OP_EQUAL) || checkToken(TokenType::OP_NOT_EQUAL)) {
        auto op = currentToken_;  // Save the operator token before consuming it
        nextToken();  // Consume the equality operator
        auto right = parseComparison();
        expr = std::make_unique<rglite::BinaryExpr>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::parseComparison() {
    auto expr = parseTerm();
    
    while (checkToken(TokenType::OP_LESS) || checkToken(TokenType::OP_GREATER) ||
           checkToken(TokenType::OP_LESS_EQUAL) || checkToken(TokenType::OP_GREATER_EQUAL) ||
           checkToken(TokenType::KW_IN) || (checkToken(TokenType::KW_NOT) && checkNextToken(TokenType::KW_IN))) {
        if (checkToken(TokenType::KW_NOT) && checkNextToken(TokenType::KW_IN)) {
            Token not_op = currentToken_; // 'not'
            nextToken(); // consume 'not'
            Token in_op = currentToken_; // 'in'
            nextToken(); // consume 'in'
            auto right = parseTerm();
            auto inner = std::make_unique<rglite::BinaryExpr>(std::move(expr), in_op, std::move(right));
            expr = std::make_unique<rglite::UnaryExpr>(not_op.lexeme, std::move(inner), not_op.location);
        } else {
            auto op = currentToken_;  // Save the operator token before consuming it
            nextToken();  // Consume the comparison operator
            auto right = parseTerm();
            expr = std::make_unique<rglite::BinaryExpr>(std::move(expr), op, std::move(right));
        }
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::parseTerm() {
    auto expr = parseFactor();
    
    while (checkToken(TokenType::OP_PLUS) || checkToken(TokenType::OP_MINUS)) {
        auto op = currentToken_;  // Save the operator token before consuming it
        nextToken();  // Consume the term operator
        auto right = parseFactor();
        if (!right) {
            error(op, "Missing operand after operator");
            return nullptr;
        }
        expr = std::make_unique<rglite::BinaryExpr>(std::move(expr), op, std::move(right));
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
        expr = std::make_unique<rglite::BinaryExpr>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::parseUnary() {
    if (checkToken(TokenType::OP_MINUS) || checkToken(TokenType::KW_NOT)) {
        auto op = currentToken_;  // Save the operator token before consuming it
        nextToken();  // Consume the unary operator
        auto operand = parseUnary();
        return std::make_unique<rglite::UnaryExpr>(op.lexeme, std::move(operand), op.location);
    }
    
    return parseCall();
}

std::unique_ptr<Expr> Parser::parsePrimary() {
    // Skip COMMENT_LINE tokens in expression parsing
    if (checkToken(TokenType::COMMENT_LINE)) {
        nextToken();
        return nullptr; // Comments don't produce expressions
    }
    
    // Skip COMMENT_BLOCK tokens in expression parsing
    if (checkToken(TokenType::COMMENT_BLOCK)) {
        nextToken();
        return nullptr; // Comments don't produce expressions
    }
    
    if (checkToken(TokenType::IDENTIFIER)) {
        auto token = currentToken_;
        nextToken();
        return std::make_unique<rglite::IdentifierExpr>(token.lexeme, token.location);
    }
    
    if (checkToken(TokenType::INTEGER) || checkToken(TokenType::FLOAT) ||
        checkToken(TokenType::STRING) || checkToken(TokenType::KW_TRUE) ||
        checkToken(TokenType::KW_FALSE) || checkToken(TokenType::KW_NONE)) {
        // Save the current token before consuming it
        auto token = currentToken_;
        
        // Debug: Print token information for string literals
        if (token.type == TokenType::STRING) {
            
        }
        
        nextToken();
        
        auto expr = std::make_unique<rglite::LiteralExpr>(token);
        return expr;
    }
    
    if (matchToken(TokenType::PUNCT_LEFT_PAREN)) {
        // Check if this is a tuple literal by looking ahead for a comma
        // or if it's an empty tuple: ()
        if (checkToken(TokenType::PUNCT_RIGHT_PAREN)) {
            // Empty tuple: ()
            nextToken(); // Consume the right parenthesis
            return std::make_unique<rglite::TupleExpr>(std::vector<std::unique_ptr<Expr>>(), currentToken_.location);
        }
        
        // Parse the first expression
        auto firstExpr = parseExpression();
        if (!firstExpr) {
            error("Expected expression in tuple or parenthesized expression");
            // Try to recover by consuming tokens until we find a closing parenthesis
            while (!checkToken(TokenType::PUNCT_RIGHT_PAREN) && !checkToken(TokenType::END_OF_FILE)) {
                nextToken();
            }
            if (checkToken(TokenType::PUNCT_RIGHT_PAREN)) {
                nextToken();
            }
            return nullptr;
        }
        
        // Check if there's a comma after the first expression
        if (matchToken(TokenType::PUNCT_COMMA)) {
            // This is a tuple literal
            std::vector<std::unique_ptr<Expr>> elements;
            elements.push_back(std::move(firstExpr));
            
            // Parse remaining elements
            if (!checkToken(TokenType::PUNCT_RIGHT_PAREN)) {
                do {
                    auto element = parseExpression();
                    if (element) {
                        elements.push_back(std::move(element));
                    } else {
                        error("Expected expression in tuple");
                        break;
                    }
                } while (matchToken(TokenType::PUNCT_COMMA));
            }
            
            // Expect closing parenthesis
            expectToken(TokenType::PUNCT_RIGHT_PAREN);
            return std::make_unique<rglite::TupleExpr>(std::move(elements), currentToken_.location);
        } else {
            // This is a regular parenthesized expression
            expectToken(TokenType::PUNCT_RIGHT_PAREN);
            return firstExpr;
        }
    }
    
    if (matchToken(TokenType::PUNCT_LEFT_BRACKET)) {
        return parseList();
    }

    if (matchToken(TokenType::PUNCT_LEFT_BRACE)) {
        // Check if this is a set literal by looking ahead
        // Empty braces {} are a dictionary, not a set
        if (checkToken(TokenType::PUNCT_RIGHT_BRACE)) {
            // Empty dictionary: {}
            nextToken(); // Consume the right brace
            return std::make_unique<rglite::DictExpr>(std::vector<std::unique_ptr<Expr>>(), 
                                             std::vector<std::unique_ptr<Expr>>(), 
                                             currentToken_.location);
        }
        
        // Parse the first expression
        auto firstExpr = parseExpression();
        if (!firstExpr) {
            error("Expected expression in set or dictionary");
            // Try to recover by consuming tokens until we find a closing brace
            while (!checkToken(TokenType::PUNCT_RIGHT_BRACE) && !checkToken(TokenType::END_OF_FILE)) {
                nextToken();
            }
            if (checkToken(TokenType::PUNCT_RIGHT_BRACE)) {
                nextToken();
            }
            return nullptr;
        }
        
        // Check if there's a colon after the first expression
        if (matchToken(TokenType::PUNCT_COLON)) {
            // This is a dictionary
            std::vector<std::unique_ptr<Expr>> keys;
            std::vector<std::unique_ptr<Expr>> values;
            keys.push_back(std::move(firstExpr));
            
            // Parse the value for the first key
            auto firstValue = parseExpression();
            if (!firstValue) {
                error("Expected value in dictionary");
                // Skip to next comma or closing brace
                while (!checkToken(TokenType::PUNCT_COMMA) && !checkToken(TokenType::PUNCT_RIGHT_BRACE) && !checkToken(TokenType::END_OF_FILE)) {
                    nextToken();
                }
                if (checkToken(TokenType::PUNCT_COMMA)) {
                    nextToken();
                }
            } else {
                values.push_back(std::move(firstValue));
            }
            
            // Parse remaining key-value pairs
            while (matchToken(TokenType::PUNCT_COMMA)) {
                if (checkToken(TokenType::PUNCT_RIGHT_BRACE)) {
                    break; // Allow trailing comma
                }
                
                // Parse key
                auto key = parseExpression();
                if (!key) {
                    error("Expected key in dictionary");
                    // Skip to colon or next comma or closing brace
                    while (!checkToken(TokenType::PUNCT_COLON) && !checkToken(TokenType::PUNCT_COMMA) && !checkToken(TokenType::PUNCT_RIGHT_BRACE) && !checkToken(TokenType::END_OF_FILE)) {
                        nextToken();
                    }
                    if (checkToken(TokenType::PUNCT_COLON)) {
                        nextToken();
                    }
                    continue;
                }
                
                // Expect colon separator
                if (!matchToken(TokenType::PUNCT_COLON)) {
                    error("Expected ':' after dictionary key");
                    // Skip to next comma or closing brace
                    while (!checkToken(TokenType::PUNCT_COMMA) && !checkToken(TokenType::PUNCT_RIGHT_BRACE) && !checkToken(TokenType::END_OF_FILE)) {
                        nextToken();
                    }
                    if (checkToken(TokenType::PUNCT_COMMA)) {
                        nextToken();
                    }
                    continue;
                }
                
                // Parse value
                auto value = parseExpression();
                if (!value) {
                    error("Expected value in dictionary");
                    // Skip to next comma or closing brace
                    while (!checkToken(TokenType::PUNCT_COMMA) && !checkToken(TokenType::PUNCT_RIGHT_BRACE) && !checkToken(TokenType::END_OF_FILE)) {
                        nextToken();
                    }
                    if (checkToken(TokenType::PUNCT_COMMA)) {
                        nextToken();
                    }
                    continue;
                }
                
                keys.push_back(std::move(key));
                values.push_back(std::move(value));
            }
            
            // Expect closing brace
            expectToken(TokenType::PUNCT_RIGHT_BRACE);
            return std::make_unique<rglite::DictExpr>(std::move(keys), std::move(values), currentToken_.location);
        } else {
            // This is a set literal
            std::vector<std::unique_ptr<Expr>> elements;
            elements.push_back(std::move(firstExpr));
            
            // Parse remaining elements
            while (matchToken(TokenType::PUNCT_COMMA)) {
                if (checkToken(TokenType::PUNCT_RIGHT_BRACE)) {
                    break; // Allow trailing comma
                }
                
                auto element = parseExpression();
                if (element) {
                    elements.push_back(std::move(element));
                } else {
                    error("Expected expression in set");
                    // Skip to next comma or closing brace
                    while (!checkToken(TokenType::PUNCT_COMMA) && !checkToken(TokenType::PUNCT_RIGHT_BRACE) && !checkToken(TokenType::END_OF_FILE)) {
                        nextToken();
                    }
                    if (checkToken(TokenType::PUNCT_COMMA)) {
                        nextToken();
                    }
                }
            }
            
            // Expect closing brace
            expectToken(TokenType::PUNCT_RIGHT_BRACE);
            return std::make_unique<rglite::SetExpr>(std::move(elements), currentToken_.location);
        }
    }
    
    // Handle tuple keyword: tuple([1, 2, 3])
    if (checkToken(TokenType::KW_TUPLE)) {
        return parseTuple();
    }
    
    // Handle set keyword: set([1, 2, 3])
    if (checkToken(TokenType::KW_SET)) {
        return parseSet();
    }

    // If we reach here, we have an unexpected token
    error("Expected expression");
    // Advance to avoid infinite loop
    nextToken();
    return nullptr;
}

std::unique_ptr<Expr> Parser::parseCall() {
    auto expr = parsePrimary();
    
    while (true) {
        if (matchToken(TokenType::PUNCT_LEFT_PAREN)) {
            // Save the left parenthesis token for error reporting
            auto leftParenToken = currentToken_;
            auto arguments = parseArgumentList();
            
            // Check if we have the closing parenthesis
            if (!checkToken(TokenType::PUNCT_RIGHT_PAREN)) {
                // Report error pointing to the opening parenthesis (Python style)
                Diagnostic diagnostic = DiagnosticBuilder::parenthesisNotClosed(leftParenToken, lexer_ ? lexer_->getFilename() : std::string("<stdin>"));
                errorHandler_->report(diagnostic);
                hasErrors_ = true;
                // Don't consume any token, let the parser recover
            } else {
                nextToken(); // Consume the right parenthesis
            }
            
            expr = std::make_unique<rglite::CallExpr>(std::move(expr), std::move(arguments), currentToken_.location);
        } else if (matchToken(TokenType::PUNCT_DOT)) {
            // Handle member access: object.member
            if (checkToken(TokenType::IDENTIFIER)) {
                auto memberToken = currentToken_;
                nextToken();
                expr = std::make_unique<rglite::MemberAccessExpr>(std::move(expr), memberToken.lexeme, memberToken.location);
            } else {
                error("Expected identifier after '.'");
                break;
            }
        } else if (matchToken(TokenType::PUNCT_LEFT_BRACKET)) {
            // Handle index access: object[index]
            auto index = parseExpression();
            expectToken(TokenType::PUNCT_RIGHT_BRACKET);
            expr = std::make_unique<rglite::IndexAccessExpr>(std::move(expr), std::move(index), currentToken_.location);
        } else {
            break;
        }
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::parseList() {
    auto listToken = currentToken_; // Save the left bracket token for location
    
    std::vector<std::unique_ptr<Expr>> elements;
    
    // Handle empty list: []
    if (matchToken(TokenType::PUNCT_RIGHT_BRACKET)) {
        return std::make_unique<rglite::ListExpr>(std::move(elements), listToken.location);
    }
    
    // Parse list elements
    do {
        auto element = parseExpression();
        if (element) {
            elements.push_back(std::move(element));
        } else {
            error("Expected expression in list");
            break;
        }
    } while (matchToken(TokenType::PUNCT_COMMA));
    
    // Expect closing bracket
    expectToken(TokenType::PUNCT_RIGHT_BRACKET);
    
    return std::make_unique<ListExpr>(std::move(elements), listToken.location);
}

std::unique_ptr<Expr> Parser::parseDict() {
    auto dictToken = currentToken_; // Save the left brace token for location
    
    std::vector<std::unique_ptr<Expr>> keys;
    std::vector<std::unique_ptr<Expr>> values;
    
    // Handle empty dict: {}
    if (matchToken(TokenType::PUNCT_RIGHT_BRACE)) {
        return std::make_unique<DictExpr>(std::move(keys), std::move(values), dictToken.location);
    }
    
    // Parse dict key-value pairs
    do {
        // Parse key
        auto key = parseExpression();
        if (!key) {
            error("Expected expression for dictionary key");
            break;
        }
        
        // Expect colon separator
        if (!matchToken(TokenType::PUNCT_COLON)) {
            error("Expected ':' after dictionary key");
            break;
        }
        
        // Parse value
        auto value = parseExpression();
        if (!value) {
            error("Expected expression for dictionary value");
            break;
        }
        
        keys.push_back(std::move(key));
        values.push_back(std::move(value));
        
    } while (matchToken(TokenType::PUNCT_COMMA));
    
    // Expect closing brace
    expectToken(TokenType::PUNCT_RIGHT_BRACE);
    
    return std::make_unique<DictExpr>(std::move(keys), std::move(values), dictToken.location);
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

        // Skip comment tokens at block start; they are not statements
        if (checkToken(TokenType::COMMENT_LINE) || checkToken(TokenType::COMMENT_BLOCK)) {
            nextToken();
            continue;
        }
        
        // Parse statements normally
        Token beforeToken = currentToken_;
        auto stmt = parseStatement();
        if (stmt) {
            statements.push_back(std::move(stmt));
        } else {
            // Advance only if parseStatement did not consume any token
            if (beforeToken.type == currentToken_.type &&
                beforeToken.location.line == currentToken_.location.line &&
                beforeToken.location.column == currentToken_.location.column) {
                nextToken();
            }
        }
    }
    
    return std::make_unique<BlockStmt>(std::move(statements), blockToken.location);
}

std::vector<std::unique_ptr<Expr>> Parser::parseArgumentList() {
    std::vector<std::unique_ptr<Expr>> arguments;
    
    if (!checkToken(TokenType::PUNCT_RIGHT_PAREN)) {
        do {
            // Skip COMMENT_LINE tokens in argument list
            if (checkToken(TokenType::COMMENT_LINE)) {
                nextToken();
                continue; // Skip to next token
            }
            
            // Skip COMMENT_BLOCK tokens in argument list
            if (checkToken(TokenType::COMMENT_BLOCK)) {
                nextToken();
                continue; // Skip to next token
            }
            
            auto expr = parseExpression();
            if (expr) {
                arguments.push_back(std::move(expr));
            } else {
                // If expression parsing fails, break to avoid infinite loop
                break;
            }
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

std::unique_ptr<Expr> Parser::parseTuple() {
    auto tupleToken = currentToken_; // Save the tuple keyword token for location
    expectToken(TokenType::KW_TUPLE);
    
    // Expect left parenthesis
    expectToken(TokenType::PUNCT_LEFT_PAREN);
    
    std::vector<std::unique_ptr<Expr>> elements;
    
    // Handle empty tuple: tuple()
    if (matchToken(TokenType::PUNCT_RIGHT_PAREN)) {
        return std::make_unique<TupleExpr>(std::move(elements), tupleToken.location);
    }
    
    // Parse tuple elements
    do {
        auto element = parseExpression();
        if (element) {
            elements.push_back(std::move(element));
        } else {
            error("Expected expression in tuple");
            break;
        }
    } while (matchToken(TokenType::PUNCT_COMMA));
    
    // Expect closing parenthesis
    expectToken(TokenType::PUNCT_RIGHT_PAREN);
    
    return std::make_unique<TupleExpr>(std::move(elements), tupleToken.location);
}

std::unique_ptr<Expr> Parser::parseSet() {
    auto setToken = currentToken_; // Save the set keyword token for location
    expectToken(TokenType::KW_SET);
    
    // Expect left parenthesis
    expectToken(TokenType::PUNCT_LEFT_PAREN);
    
    std::vector<std::unique_ptr<Expr>> elements;
    
    // Handle empty set: set()
    if (matchToken(TokenType::PUNCT_RIGHT_PAREN)) {
        return std::make_unique<SetExpr>(std::move(elements), setToken.location);
    }
    
    // Parse set elements
    do {
        auto element = parseExpression();
        if (element) {
            elements.push_back(std::move(element));
        } else {
            error("Expected expression in set");
            break;
        }
    } while (matchToken(TokenType::PUNCT_COMMA));
    
    // Expect closing parenthesis
    expectToken(TokenType::PUNCT_RIGHT_PAREN);
    
    return std::make_unique<SetExpr>(std::move(elements), setToken.location);
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
        case TokenType::PUNCT_DOT: return "'.";
        case TokenType::NEWLINE: return "newline";
        case TokenType::INDENT: return "indent";
        case TokenType::DEDENT: return "dedent";
        case TokenType::END_OF_FILE: return "end of file";
        default: return "token";
    }
}

} // namespace rglite



