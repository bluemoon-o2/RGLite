// RGLite Parser Definitions
// This file defines the parser for RGLite language

#ifndef PARSER_H
#define PARSER_H

#include "AST.h"
#include "Lexer.h"
#include "ErrorHandler.h"
#include <memory>
#include <vector>

namespace rglite {

/**
 * @brief Parser for RGLite language
 * 
 * The parser converts a stream of tokens into an Abstract Syntax Tree (AST)
 * following the RGLite grammar rules.
 */
class Parser {
public:
    Parser(std::unique_ptr<Lexer> lexer, std::shared_ptr<ErrorHandler> errorHandler = nullptr);
    
    /**
     * @brief Parse the entire source code and return the AST
     */
    std::unique_ptr<Stmt> parse();
    
    /**
     * @brief Check if there are any parsing errors
     */
    bool hasErrors() const;
    
private:
    // Lexer and error handling
    std::unique_ptr<Lexer> lexer_;
    std::shared_ptr<ErrorHandler> errorHandler_;
    
    // Current token and lookahead
    Token currentToken_;
    Token peekToken_;
    
    // Error state
    bool hasErrors_ = false;
    
    // Token management
    void nextToken();
    void expectToken(TokenType expected);
    bool matchToken(TokenType expected);
    bool checkToken(TokenType expected) const;
    bool checkNextToken(TokenType expected) const;
    
    // Error reporting
    void error(const std::string& message);
    void errorAt(const Token& token, const std::string& message);
    void synchronize();
    std::string tokenTypeToString(TokenType type);
    
    // Grammar rules
    std::unique_ptr<Stmt> parseStatement();
    std::unique_ptr<ExprStmt> parseExpressionStatement();
    std::unique_ptr<VarDeclStmt> parseVariableDeclaration();
    std::unique_ptr<FunctionDeclStmt> parseFunctionDefinition();
    std::unique_ptr<IfStmt> parseIfStatement();
    std::unique_ptr<WhileStmt> parseWhileStatement();
    std::unique_ptr<ReturnStmt> parseReturnStatement();
    
    std::unique_ptr<Expr> parseExpression();
    std::unique_ptr<Expr> parseAssignment();
    std::unique_ptr<Expr> parseLogicalOr();
    std::unique_ptr<Expr> parseLogicalAnd();
    std::unique_ptr<Expr> parseEquality();
    std::unique_ptr<Expr> parseComparison();
    std::unique_ptr<Expr> parseTerm();
    std::unique_ptr<Expr> parseFactor();
    std::unique_ptr<Expr> parseUnary();
    std::unique_ptr<Expr> parsePrimary();
    
    std::unique_ptr<Expr> parseCall();
    
    // Utility functions
    std::unique_ptr<BlockStmt> parseBlock();
    std::vector<std::unique_ptr<Expr>> parseArgumentList();
    std::vector<std::string> parseParameterList();
};

} // namespace rglite

#endif // PARSER_H