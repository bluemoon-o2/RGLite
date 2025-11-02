// RGLite AST Test Suite
// This file tests the Abstract Syntax Tree (AST) functionality using Google Test-style framework

#include "AST.h"
#include "TestFramework.h"
#include "Parser.h"
#include "Lexer.h"
#include "ErrorHandler.h"
#include <memory>
#include <sstream>
#include <iostream>
#include <vector>
#include <string>

using namespace rglite;

/**
 * @brief AST test fixture for common setup
 */
class ASTTestFixture : public testing::TestFixture {
public:
    std::shared_ptr<StandardErrorHandler> errorHandler;
    
    void SetUp() override {
        errorHandler = std::make_shared<StandardErrorHandler>();
    }
    
    void TearDown() override {
        errorHandler.reset();
    }
    
    /**
     * @brief Helper function to parse source code and return AST
     */
    std::unique_ptr<Stmt> parseSource(const std::string& source) {
        auto lexer = std::make_unique<Lexer>(source);
        Parser parser(std::move(lexer), errorHandler);
        return parser.parse();
    }
};

/**
 * @brief Test visitor for AST traversal
 */
class TestVisitor : public ASTVisitor {
public:
    std::vector<std::string> visitedNodes;
    
    void visitLiteralExpr(LiteralExpr& expr) override {
        visitedNodes.push_back("LiteralExpr(" + expr.token.lexeme + ")");
    }
    
    void visitIdentifierExpr(IdentifierExpr& expr) override {
        visitedNodes.push_back("IdentifierExpr(" + expr.name + ")");
    }
    
    void visitBinaryExpr(BinaryExpr& expr) override {
        visitedNodes.push_back("BinaryExpr(" + expr.op.lexeme + ")");
        expr.left->accept(*this);
        expr.right->accept(*this);
    }
    
    void visitCallExpr(CallExpr& expr) override {
        visitedNodes.push_back("CallExpr");
        expr.callee->accept(*this);
        for (auto& arg : expr.arguments) {
            arg->accept(*this);
        }
    }
    
    void visitUnaryExpr(UnaryExpr& expr) override {
        visitedNodes.push_back("UnaryExpr(" + expr.op + ")");
        expr.operand->accept(*this);
    }
    
    void visitExprStmt(ExprStmt& stmt) override {
        visitedNodes.push_back("ExprStmt");
        stmt.expression->accept(*this);
    }
    
    void visitBlockStmt(BlockStmt& stmt) override {
        visitedNodes.push_back("BlockStmt");
        for (auto& s : stmt.statements) {
            s->accept(*this);
        }
    }
    
    void visitIfStmt(IfStmt& stmt) override {
        visitedNodes.push_back("IfStmt");
        stmt.condition->accept(*this);
        stmt.thenBranch->accept(*this);
        if (stmt.elseBranch) {
            stmt.elseBranch->accept(*this);
        }
    }
    
    void visitWhileStmt(WhileStmt& stmt) override {
        visitedNodes.push_back("WhileStmt");
        stmt.condition->accept(*this);
        stmt.body->accept(*this);
    }
    
    void visitFunctionDeclStmt(FunctionDeclStmt& stmt) override {
        visitedNodes.push_back("FunctionDeclStmt(" + stmt.name + ")");
        stmt.body->accept(*this);
    }
    
    void visitReturnStmt(ReturnStmt& stmt) override {
        visitedNodes.push_back("ReturnStmt");
        if (stmt.value) {
            stmt.value->accept(*this);
        }
    }
};

// Test suite for AST node creation and basic functionality
TEST(ASTSuite, LiteralExprCreation) {
    Token token{TokenType::INTEGER, "42", {1, 1}};
    LiteralExpr expr(token);
    
    EXPECT_EQ(expr.toString(), "LiteralExpr(42)");
    EXPECT_EQ(expr.getLocation().line, 1);
    EXPECT_EQ(expr.getLocation().column, 1);
}

TEST(ASTSuite, IdentifierExprCreation) {
    SourceLocation loc{1, 1};
    IdentifierExpr expr("x", loc);
    
    EXPECT_EQ(expr.toString(), "IdentifierExpr(x)");
    EXPECT_EQ(expr.getLocation().line, 1);
    EXPECT_EQ(expr.getLocation().column, 1);
}

TEST(ASTSuite, BinaryExprCreation) {
    SourceLocation loc{1, 1};
    Token token{TokenType::OP_PLUS, "+", {1, 3}};
    auto left = std::make_unique<IdentifierExpr>("x", loc);
    auto right = std::make_unique<IdentifierExpr>("y", loc);
    
    BinaryExpr expr(std::move(left), token, std::move(right));
    
    EXPECT_EQ(expr.toString(), "BinaryExpr(IdentifierExpr(x) + IdentifierExpr(y))");
    EXPECT_EQ(expr.getLocation().line, 1);
    EXPECT_EQ(expr.getLocation().column, 3);
}

// Test suite for parser functionality
TEST(ASTSuite, ParseSimpleExpression) {
    ASTTestFixture fixture;
    fixture.SetUp();
    
    auto ast = fixture.parseSource("x + y\n");
    
    EXPECT_NE(ast, nullptr);
    
    fixture.TearDown();
}

TEST(ASTSuite, ParseVariableDeclaration) {
    ASTTestFixture fixture;
    fixture.SetUp();
    
    auto ast = fixture.parseSource("x = 42\n");
    
    EXPECT_NE(ast, nullptr);
    
    fixture.TearDown();
}

TEST(ASTSuite, ParseFunctionDefinition) {
    ASTTestFixture fixture;
    fixture.SetUp();
    
    auto ast = fixture.parseSource("def add(a, b):\n    return a + b\n");
    
    EXPECT_NE(ast, nullptr);
    EXPECT_TRUE(ast->toString().find("FunctionDeclStmt") != std::string::npos);
    EXPECT_TRUE(ast->toString().find("add") != std::string::npos);
    EXPECT_TRUE(ast->toString().find("ReturnStmt") != std::string::npos);
    
    fixture.TearDown();
}

TEST(ASTSuite, ParseIfStatement) {
    ASTTestFixture fixture;
    fixture.SetUp();
    
    auto ast = fixture.parseSource("if x > 0:\n    print(x)\n");
    
    EXPECT_NE(ast, nullptr);
    
    fixture.TearDown();
}

TEST(ASTSuite, ParseWhileStatement) {
    ASTTestFixture fixture;
    fixture.SetUp();
    
    auto ast = fixture.parseSource("while x < 10:\n    x = x + 1\n");
    
    EXPECT_NE(ast, nullptr);
    
    fixture.TearDown();
}

// Test suite for AST visitor functionality
TEST(ASTSuite, ASTVisitorTraversal) {
    SourceLocation loc{1, 1};
    Token token{TokenType::OP_PLUS, "+", {1, 3}};
    auto left = std::make_unique<IdentifierExpr>("x", loc);
    auto right = std::make_unique<IdentifierExpr>("y", loc);
    
    BinaryExpr expr(std::move(left), token, std::move(right));
    
    TestVisitor visitor;
    expr.accept(visitor);
    
    EXPECT_EQ(visitor.visitedNodes.size(), 3);
    EXPECT_EQ(visitor.visitedNodes[0], "BinaryExpr(+)");
    EXPECT_EQ(visitor.visitedNodes[1], "IdentifierExpr(x)");
    EXPECT_EQ(visitor.visitedNodes[2], "IdentifierExpr(y)");
}

// Main function using the new test framework
RUN_ALL_TESTS();