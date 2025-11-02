// RGLite Semantic Analyzer Definitions
// This file defines the semantic analyzer for RGLite language

#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include "AST.h"
#include "ErrorHandler.h"
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <variant>
#include <stack>

namespace rglite {

// Forward declarations
class SymbolTable;
class TypeChecker;

/**
 * @brief Symbol information for variables and functions
 */
struct Symbol {
    enum class Kind {
        VARIABLE,
        FUNCTION,
        PARAMETER,
        BUILTIN
    };
    
    // Alias for backward compatibility
    using Type = Kind;
    
    Kind kind;
    std::string name;
    SourceLocation location;
    bool isInitialized = false;
    bool used = false;
    
    // For functions
    std::vector<std::string> parameters;
    
    Symbol(Kind k, const std::string& n, const SourceLocation& loc)
        : kind(k), name(n), location(loc) {}
};

// Alias for backward compatibility
using SymbolType = Symbol::Kind;

/**
 * @brief Scope information for nested scopes
 */
class Scope {
public:
    Scope(std::shared_ptr<Scope> parent = nullptr);
    ~Scope() = default;
    
    // Symbol management
    bool define(const std::string& name, std::unique_ptr<Symbol> symbol);
    Symbol* resolve(const std::string& name) const;
    
    // Scope management
    std::shared_ptr<Scope> getParent() const { return parent_; }
    size_t getSize() const { return symbols_.size(); }
    
private:
    std::shared_ptr<Scope> parent_;
    std::unordered_map<std::string, std::unique_ptr<Symbol>> symbols_;
};

/**
 * @brief Symbol table for managing symbols across scopes
 */
class SymbolTable {
public:
    SymbolTable();
    ~SymbolTable() = default;
    
    // Scope management
    void pushScope();
    void popScope();
    std::shared_ptr<Scope> getCurrentScope() const { return currentScope_; }
    
    // Symbol management
    bool define(const std::string& name, std::unique_ptr<Symbol> symbol);
    Symbol* resolve(const std::string& name) const;
    Symbol* lookup(const std::string& name) const;
    
    // Built-in functions
    void defineBuiltins();
    
private:
    std::shared_ptr<Scope> globalScope_;
    std::shared_ptr<Scope> currentScope_;
};

/**
 * @brief Type information for expressions
 */
enum class Type {
    INTEGER,
    FLOAT,
    STRING,
    BOOLEAN,
    NONE,
    FUNCTION,
    UNKNOWN
};

/**
 * @brief Type checker for RGLite expressions
 */
class TypeChecker {
public:
    TypeChecker(std::shared_ptr<ErrorHandler> errorHandler = nullptr);
    ~TypeChecker() = default;
    
    // Type checking
    Type check(const std::unique_ptr<Expr>& expr);
    Type checkBinaryOperation(Type left, Type right, const std::string& op);
    Type checkUnaryOperation(Type operand, const std::string& op);
    Type checkFunctionCall(const std::string& name, const std::vector<Type>& argTypes);
    Type checkFunctionCall(Symbol* symbol, const std::vector<std::unique_ptr<Expr>>& args);
    
    // Type utilities
    std::string typeToString(Type type) const;
    bool isAssignable(Type type) const;
    bool isNumeric(Type type) const;
    
private:
    std::shared_ptr<ErrorHandler> errorHandler_;
};

/**
 * @brief Semantic analyzer for RGLite language
 * 
 * The semantic analyzer performs type checking, symbol resolution,
 * and other semantic checks on the AST.
 */
class SemanticAnalyzer {
public:
    SemanticAnalyzer(std::shared_ptr<ErrorHandler> errorHandler = nullptr);
    ~SemanticAnalyzer() = default;
    
    /**
     * @brief Analyze the AST for semantic errors
     */
    bool analyze(const std::unique_ptr<Stmt>& ast);
    
    /**
     * @brief Check if there are any semantic errors
     */
    bool hasErrors() const;
    
    /**
     * @brief Get the symbol table for inspection
     */
    std::shared_ptr<SymbolTable> getSymbolTable() const { return symbolTable_; }
    
private:
    // Error handling
    std::shared_ptr<ErrorHandler> errorHandler_;
    bool hasErrors_ = false;
    
    // Analysis components
    std::shared_ptr<SymbolTable> symbolTable_;
    std::unique_ptr<TypeChecker> typeChecker_;
    
    // Error reporting
    void error(const std::string& message, const SourceLocation& location);
    void error(const SourceLocation& location, const std::string& message);
    
    // Statement analysis methods
    void analyzeStatement(const std::unique_ptr<Stmt>& stmt);
    void analyzeBlockStmt(BlockStmt* stmt);
    void analyzeExprStmt(ExprStmt* stmt);
    void analyzeFunctionDeclStmt(FunctionDeclStmt* stmt);
    void analyzeIfStmt(IfStmt* stmt);
    void analyzeWhileStmt(WhileStmt* stmt);
    void analyzeReturnStmt(ReturnStmt* stmt);
    
    // Expression analysis
    Type analyzeExpression(const std::unique_ptr<Expr>& expr);
    Type analyzeBinaryExpr(BinaryExpr* expr);
    Type analyzeUnaryExpr(UnaryExpr* expr);
    Type analyzeLiteralExpr(LiteralExpr* expr);
    std::shared_ptr<Symbol> analyzeIdentifierExpr(IdentifierExpr* expr);
    Type analyzeCallExpr(CallExpr* expr);
    
    // Helper methods
    void enterFunction(const std::string& name, const std::vector<std::string>& parameters);
    void exitFunction();
    bool isInFunction() const { return inFunction_; }
    const std::string& getCurrentFunction() const { return currentFunction_; }
    
    // Undefined variable checking
    void checkForUndefinedVariables(const std::unique_ptr<Expr>& expr);
    
    // State
    bool inFunction_ = false;
    std::string currentFunction_;
    std::stack<std::pair<bool, std::string>> functionStack_;
};

} // namespace rglite

#endif // SEMANTIC_ANALYZER_H