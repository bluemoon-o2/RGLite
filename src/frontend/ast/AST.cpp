// RGLite AST Implementation
// This file implements the AST node methods

#include "AST.h"
#include <sstream>

namespace rglite {

// LiteralExpr implementation
void LiteralExpr::accept(ASTVisitor& visitor) {
    visitor.visitLiteralExpr(*this);
}

std::string LiteralExpr::toString() const {
    if (token.type == TokenType::STRING) {
        return "LiteralExpr(\"" + token.lexeme + "\")";
    }
    return "LiteralExpr(" + token.lexeme + ")";
}

// IdentifierExpr implementation
void IdentifierExpr::accept(ASTVisitor& visitor) {
    visitor.visitIdentifierExpr(*this);
}

std::string IdentifierExpr::toString() const {
    return "IdentifierExpr(" + name + ")";
}

// BinaryExpr implementation
void BinaryExpr::accept(ASTVisitor& visitor) {
    visitor.visitBinaryExpr(*this);
}

std::string BinaryExpr::toString() const {
    return "BinaryExpr(" + left->toString() + " " + op.lexeme + " " + right->toString() + ")";
}

// CallExpr implementation
void CallExpr::accept(ASTVisitor& visitor) {
    visitor.visitCallExpr(*this);
}

std::string CallExpr::toString() const {
    std::stringstream ss;
    ss << "CallExpr(" << callee->toString() << "(";
    
    for (size_t i = 0; i < arguments.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << arguments[i]->toString();
    }
    
    ss << ")";
    return ss.str();
}

// UnaryExpr implementation
void UnaryExpr::accept(ASTVisitor& visitor) {
    visitor.visitUnaryExpr(*this);
}

std::string UnaryExpr::toString() const {
    return "UnaryExpr(" + op + " " + operand->toString() + ")";
}

// ExprStmt implementation
void ExprStmt::accept(ASTVisitor& visitor) {
    visitor.visitExprStmt(*this);
}

std::string ExprStmt::toString() const {
    return "ExprStmt(" + expression->toString() + ")";
}

// VarDeclStmt implementation
void VarDeclStmt::accept(ASTVisitor& visitor) {
    visitor.visitVarDeclStmt(*this);
}

std::string VarDeclStmt::toString() const {
    return "VarDeclStmt(" + name + " = " + initializer->toString() + ")";
}

// BlockStmt implementation
void BlockStmt::accept(ASTVisitor& visitor) {
    visitor.visitBlockStmt(*this);
}

std::string BlockStmt::toString() const {
    std::stringstream ss;
    ss << "BlockStmt([";
    
    for (size_t i = 0; i < statements.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << statements[i]->toString();
    }
    
    ss << "])";
    return ss.str();
}

// IfStmt implementation
void IfStmt::accept(ASTVisitor& visitor) {
    visitor.visitIfStmt(*this);
}

std::string IfStmt::toString() const {
    std::stringstream ss;
    ss << "IfStmt(condition: " << condition->toString() 
       << ", then: " << thenBranch->toString();
    
    if (elseBranch) {
        ss << ", else: " << elseBranch->toString();
    }
    
    ss << ")";
    return ss.str();
}

// WhileStmt implementation
void WhileStmt::accept(ASTVisitor& visitor) {
    visitor.visitWhileStmt(*this);
}

std::string WhileStmt::toString() const {
    return "WhileStmt(condition: " + condition->toString() + 
           ", body: " + body->toString() + ")";
}

// FunctionDeclStmt implementation
void FunctionDeclStmt::accept(ASTVisitor& visitor) {
    visitor.visitFunctionDeclStmt(*this);
}

std::string FunctionDeclStmt::toString() const {
    std::stringstream ss;
    ss << "FunctionDeclStmt(" << name << "(";
    
    for (size_t i = 0; i < parameters.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << parameters[i];
    }
    
    ss << "), body: " << body->toString() << ")";
    return ss.str();
}

// ReturnStmt implementation
void ReturnStmt::accept(ASTVisitor& visitor) {
    visitor.visitReturnStmt(*this);
}

std::string ReturnStmt::toString() const {
    if (value) {
        return "ReturnStmt(" + value->toString() + ")";
    }
    return "ReturnStmt()";
}

} // namespace rglite