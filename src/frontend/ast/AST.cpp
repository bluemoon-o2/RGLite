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

// ListExpr implementation
void ListExpr::accept(ASTVisitor& visitor) {
    visitor.visitListExpr(*this);
}

std::string ListExpr::toString() const {
    std::stringstream ss;
    ss << "ListExpr([";
    
    for (size_t i = 0; i < elements.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << elements[i]->toString();
    }
    
    ss << "])";
    return ss.str();
}

// ExprStmt implementation
void ExprStmt::accept(ASTVisitor& visitor) {
    visitor.visitExprStmt(*this);
}

std::string ExprStmt::toString() const {
    return "ExprStmt(" + expression->toString() + ")";
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

// ForStmt implementation
void ForStmt::accept(ASTVisitor& visitor) {
    visitor.visitForStmt(*this);
}

std::string ForStmt::toString() const {
    return "ForStmt(variable: " + variable + 
           ", iterable: " + iterable->toString() + 
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

// ImportStmt implementation
void ImportStmt::accept(ASTVisitor& visitor) {
    visitor.visitImportStmt(*this);
}

std::string ImportStmt::toString() const {
    std::stringstream ss;
    ss << "ImportStmt([";
    for (size_t i = 0; i < items.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << items[i].module;
        if (!items[i].alias.empty()) {
            ss << " as " << items[i].alias;
        }
    }
    ss << "])";
    return ss.str();
}

// FromImportStmt implementation
void FromImportStmt::accept(ASTVisitor& visitor) {
    visitor.visitFromImportStmt(*this);
}

std::string FromImportStmt::toString() const {
    std::stringstream ss;
    ss << "FromImportStmt(module: " << module << ", names: ";
    if (importAll) {
        ss << "*";
    } else {
        ss << "[";
        for (size_t i = 0; i < names.size(); ++i) {
            if (i > 0) ss << ", ";
            ss << names[i].name;
            if (!names[i].alias.empty()) {
                ss << " as " << names[i].alias;
            }
        }
        ss << "]";
    }
    ss << ")";
    return ss.str();
}

// MemberAccessExpr implementation
void MemberAccessExpr::accept(ASTVisitor& visitor) {
    visitor.visitMemberAccessExpr(*this);
}

std::string MemberAccessExpr::toString() const {
    return "MemberAccessExpr(" + object->toString() + "." + member + ")";
}

// IndexAccessExpr implementation
void IndexAccessExpr::accept(ASTVisitor& visitor) {
    visitor.visitIndexAccessExpr(*this);
}

std::string IndexAccessExpr::toString() const {
    return "IndexAccessExpr(" + object->toString() + "[" + index->toString() + "])";
}

// DictExpr implementation
void DictExpr::accept(ASTVisitor& visitor) {
    visitor.visitDictExpr(*this);
}

std::string DictExpr::toString() const {
    std::stringstream ss;
    ss << "DictExpr({";
    
    for (size_t i = 0; i < keys.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << keys[i]->toString() << ": " << values[i]->toString();
    }
    
    ss << "})";
    return ss.str();
}

// TupleExpr implementation
void TupleExpr::accept(ASTVisitor& visitor) {
    visitor.visitTupleExpr(*this);
}

std::string TupleExpr::toString() const {
    std::stringstream ss;
    ss << "TupleExpr(";
    
    for (size_t i = 0; i < elements.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << elements[i]->toString();
    }
    
    ss << ")";
    return ss.str();
}

// SetExpr implementation
void SetExpr::accept(ASTVisitor& visitor) {
    visitor.visitSetExpr(*this);
}

std::string SetExpr::toString() const {
    std::stringstream ss;
    ss << "SetExpr({";
    
    for (size_t i = 0; i < elements.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << elements[i]->toString();
    }
    
    ss << "})";
    return ss.str();
}

} // namespace rglite
