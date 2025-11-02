// RGLite Abstract Syntax Tree Definitions
// This file defines the AST node types for syntax tree representation

#ifndef AST_H
#define AST_H

#include <memory>
#include <vector>
#include <string>
#include "Token.h"

namespace rglite {

// Forward declarations
class ASTVisitor;

/**
 * @brief Expression types for type checking
 */
enum class ExprType {
    LITERAL,
    IDENTIFIER,
    BINARY,
    UNARY,
    CALL
};

/**
 * @brief Statement types for type checking
 */
enum class StmtType {
    EXPR_STMT,
    BLOCK,
    IF_STMT,
    WHILE_STMT,
    FUNC_DECL,
    RETURN_STMT
};

/**
 * @brief Base class for all AST nodes
 */
class ASTNode {
public:
    virtual ~ASTNode() = default;
    
    /**
     * @brief Accept a visitor for traversal
     * @param visitor The visitor to accept
     */
    virtual void accept(ASTVisitor& visitor) = 0;
    
    /**
     * @brief Get the source location of this node
     * @return Source location
     */
    virtual SourceLocation getLocation() const = 0;
    
    /**
     * @brief Convert node to string for representation
     * @return String representation
     */
    virtual std::string toString() const = 0;
};

/**
 * @brief Expression base class
 */
class Expr : public ASTNode {
public:
    virtual ~Expr() = default;
    
    /**
     * @brief Get the type of expression
     * @return Expression type
     */
    virtual ExprType getType() const = 0;
};

/**
 * @brief Statement base class
 */
class Stmt : public ASTNode {
public:
    virtual ~Stmt() = default;
    
    /**
     * @brief Get the type of statement
     * @return Statement type
     */
    virtual StmtType getType() const = 0;
};

// Expression nodes
class LiteralExpr : public Expr {
public:
    Token token;
    
    LiteralExpr(const Token& t) : token(t) {}
    
    void accept(ASTVisitor& visitor) override;
    SourceLocation getLocation() const override { return token.location; }
    std::string toString() const override;
    ExprType getType() const override { return ExprType::LITERAL; }
};

class IdentifierExpr : public Expr {
public:
    std::string name;
    SourceLocation location;
    
    IdentifierExpr(const std::string& n, const SourceLocation& loc) 
        : name(n), location(loc) {}
    
    void accept(ASTVisitor& visitor) override;
    SourceLocation getLocation() const override { return location; }
    std::string toString() const override;
    ExprType getType() const override { return ExprType::IDENTIFIER; }
};

class BinaryExpr : public Expr {
public:
    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;
    
    BinaryExpr(std::unique_ptr<Expr> l, const Token& o, std::unique_ptr<Expr> r)
        : left(std::move(l)), op(o), right(std::move(r)) {}
    
    void accept(ASTVisitor& visitor) override;
    SourceLocation getLocation() const override { return op.location; }
    std::string toString() const override;
    ExprType getType() const override { return ExprType::BINARY; }
};

class CallExpr : public Expr {
public:
    std::unique_ptr<Expr> callee;
    std::vector<std::unique_ptr<Expr>> arguments;
    SourceLocation location;
    
    CallExpr(std::unique_ptr<Expr> c, std::vector<std::unique_ptr<Expr>> args, 
             const SourceLocation& loc)
        : callee(std::move(c)), arguments(std::move(args)), location(loc) {}
    
    void accept(ASTVisitor& visitor) override;
    SourceLocation getLocation() const override { return location; }
    std::string toString() const override;
    ExprType getType() const override { return ExprType::CALL; }
};

class UnaryExpr : public Expr {
public:
    std::string op;
    std::unique_ptr<Expr> operand;
    SourceLocation location;
    
    UnaryExpr(const std::string& o, std::unique_ptr<Expr> expr, const SourceLocation& loc)
        : op(o), operand(std::move(expr)), location(loc) {}
    
    void accept(ASTVisitor& visitor) override;
    SourceLocation getLocation() const override { return location; }
    std::string toString() const override;
    ExprType getType() const override { return ExprType::UNARY; }
};

// Statement nodes
class ExprStmt : public Stmt {
public:
    std::unique_ptr<Expr> expression;
    
    ExprStmt(std::unique_ptr<Expr> expr) : expression(std::move(expr)) {}
    
    void accept(ASTVisitor& visitor) override;
    SourceLocation getLocation() const override { return expression->getLocation(); }
    std::string toString() const override;
    StmtType getType() const override { return StmtType::EXPR_STMT; }
};

class BlockStmt : public Stmt {
public:
    std::vector<std::unique_ptr<Stmt>> statements;
    SourceLocation location;
    
    BlockStmt(std::vector<std::unique_ptr<Stmt>> stmts, const SourceLocation& loc)
        : statements(std::move(stmts)), location(loc) {}
    
    void accept(ASTVisitor& visitor) override;
    SourceLocation getLocation() const override { return location; }
    std::string toString() const override;
    StmtType getType() const override { return StmtType::BLOCK; }
};

class IfStmt : public Stmt {
public:
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> thenBranch;
    std::unique_ptr<Stmt> elseBranch;
    SourceLocation location;
    
    IfStmt(std::unique_ptr<Expr> cond, std::unique_ptr<Stmt> then, 
           std::unique_ptr<Stmt> elseStmt, const SourceLocation& loc)
        : condition(std::move(cond)), thenBranch(std::move(then)), 
          elseBranch(std::move(elseStmt)), location(loc) {}
    
    void accept(ASTVisitor& visitor) override;
    SourceLocation getLocation() const override { return location; }
    std::string toString() const override;
    StmtType getType() const override { return StmtType::IF_STMT; }
};

class WhileStmt : public Stmt {
public:
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> body;
    SourceLocation location;
    
    WhileStmt(std::unique_ptr<Expr> cond, std::unique_ptr<Stmt> b, 
              const SourceLocation& loc)
        : condition(std::move(cond)), body(std::move(b)), location(loc) {}
    
    void accept(ASTVisitor& visitor) override;
    SourceLocation getLocation() const override { return location; }
    std::string toString() const override;
    StmtType getType() const override { return StmtType::WHILE_STMT; }
};

class FunctionDeclStmt : public Stmt {
public:
    std::string name;
    std::vector<std::string> parameters;
    std::unique_ptr<BlockStmt> body;
    SourceLocation location;
    
    FunctionDeclStmt(const std::string& n, std::vector<std::string> params, 
                     std::unique_ptr<BlockStmt> b, const SourceLocation& loc)
        : name(n), parameters(std::move(params)), body(std::move(b)), location(loc) {}
    
    void accept(ASTVisitor& visitor) override;
    SourceLocation getLocation() const override { return location; }
    std::string toString() const override;
    StmtType getType() const override { return StmtType::FUNC_DECL; }
};

class ReturnStmt : public Stmt {
public:
    std::unique_ptr<Expr> value;
    SourceLocation location;
    
    ReturnStmt(std::unique_ptr<Expr> v, const SourceLocation& loc)
        : value(std::move(v)), location(loc) {}
    
    void accept(ASTVisitor& visitor) override;
    SourceLocation getLocation() const override { return location; }
    std::string toString() const override;
    StmtType getType() const override { return StmtType::RETURN_STMT; }
};

/**
 * @brief Visitor interface for AST traversal
 */
class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;
    
    // Expression visitors
    virtual void visitLiteralExpr(LiteralExpr& expr) = 0;
    virtual void visitIdentifierExpr(IdentifierExpr& expr) = 0;
    virtual void visitBinaryExpr(BinaryExpr& expr) = 0;
    virtual void visitCallExpr(CallExpr& expr) = 0;
    virtual void visitUnaryExpr(UnaryExpr& expr) = 0;
    
    // Statement visitors
    virtual void visitExprStmt(ExprStmt& stmt) = 0;
    virtual void visitBlockStmt(BlockStmt& stmt) = 0;
    virtual void visitIfStmt(IfStmt& stmt) = 0;
    virtual void visitWhileStmt(WhileStmt& stmt) = 0;
    virtual void visitFunctionDeclStmt(FunctionDeclStmt& stmt) = 0;
    virtual void visitReturnStmt(ReturnStmt& stmt) = 0;
};

} // namespace rglite

#endif // AST_H