// RGLite Parser Test Suite
// This file tests the Parser functionality using Universal Test Framework
#include <memory>
#include <sstream>
#include <iostream>
#include <vector>
#include <string>
#include "TestFramework.h"
#include "Parser.h"
#include "Lexer.h"
#include "ErrorHandler.h"

using namespace rglite;

/**
 * @brief Parser test fixture for common setup
 */
class ParserTestFixture : public testing::TestFixture {
protected:
    std::shared_ptr<StandardErrorHandler> errorHandler;
    std::unique_ptr<Parser> parser;
    
    void SetUp() override {
        errorHandler = std::make_shared<StandardErrorHandler>();
    }
    
    void TearDown() override {
        parser.reset();
        errorHandler.reset();
    }
    
    /**
     * @brief Helper function to create parser from source code
     */
    void createParser(const std::string& source) {
        auto lexer = std::make_unique<Lexer>(source);
        parser = std::make_unique<Parser>(std::move(lexer), errorHandler);
    }
    
    /**
     * @brief Helper function to parse and validate statement
     */
    std::unique_ptr<Stmt> parseAndValidate(const std::string& source, const std::string& expected_str = "") {
        createParser(source);
        auto stmt = parser->parse();
        
        EXPECT_NE(stmt, nullptr);
        
        if (!expected_str.empty()) {
            EXPECT_EQ(expected_str, stmt->toString());
        }
        
        return stmt;
    }
};

// Test suite for basic expression parsing
TEST_F(ParserSuite, LiteralExpression, ParserTestFixture) {
    parseAndValidate("42\n", "ExprStmt(LiteralExpr(42))");
    EXPECT_FALSE(parser->hasErrors());
}

TEST_F(ParserSuite, IdentifierExpression, ParserTestFixture) {
    parseAndValidate("x\n", "ExprStmt(IdentifierExpr(x))");
    EXPECT_FALSE(parser->hasErrors());
}

TEST_F(ParserSuite, BinaryExpression, ParserTestFixture) {
    parseAndValidate("x + y\n", "ExprStmt(BinaryExpr(IdentifierExpr(x) + IdentifierExpr(y)))");
    EXPECT_FALSE(parser->hasErrors());
}

TEST_F(ParserSuite, ComplexExpression, ParserTestFixture) {
    parseAndValidate("a + b * c\n", 
        "ExprStmt(BinaryExpr(IdentifierExpr(a) + BinaryExpr(IdentifierExpr(b) * IdentifierExpr(c))))");
    EXPECT_FALSE(parser->hasErrors());
}

TEST_F(ParserSuite, ParenthesizedExpression, ParserTestFixture) {
    parseAndValidate("(a + b) * c\n", 
        "ExprStmt(BinaryExpr(BinaryExpr(IdentifierExpr(a) + IdentifierExpr(b)) * IdentifierExpr(c)))");
    EXPECT_FALSE(parser->hasErrors());
}

// Test suite for variable declaration parsing
TEST_F(ParserSuite, VariableDeclaration, ParserTestFixture) {
    parseAndValidate("x = 42\n", "ExprStmt(BinaryExpr(IdentifierExpr(x) = LiteralExpr(42)))");
    EXPECT_FALSE(parser->hasErrors());
}

TEST_F(ParserSuite, VariableDeclarationWithExpression, ParserTestFixture) {
    parseAndValidate("result = a + b\n", 
        "ExprStmt(BinaryExpr(IdentifierExpr(result) = BinaryExpr(IdentifierExpr(a) + IdentifierExpr(b))))");
    EXPECT_FALSE(parser->hasErrors());
}

// Test suite for function call parsing
TEST_F(ParserSuite, FunctionCall, ParserTestFixture) {
    parseAndValidate("print(\"hello\")\n", 
        "ExprStmt(CallExpr(IdentifierExpr(print)(LiteralExpr(\"hello\")))");
    EXPECT_FALSE(parser->hasErrors());
}

TEST_F(ParserSuite, FunctionCallWithMultipleArgs, ParserTestFixture) {
    parseAndValidate("add(x, y)\n", 
        "ExprStmt(CallExpr(IdentifierExpr(add)(IdentifierExpr(x), IdentifierExpr(y)))");
    EXPECT_FALSE(parser->hasErrors());
}

// Test suite for control flow parsing
TEST_F(ParserSuite, IfStatement, ParserTestFixture) {
    parseAndValidate("if x > 0:\n    print(\"positive\")\n", 
        "IfStmt(condition: BinaryExpr(IdentifierExpr(x) > LiteralExpr(0)), then: BlockStmt([ExprStmt(CallExpr(IdentifierExpr(print)(LiteralExpr(\"positive\")))]))");
    EXPECT_FALSE(parser->hasErrors());
}

TEST_F(ParserSuite, WhileStatement, ParserTestFixture) {
    parseAndValidate("while x < 10:\n    x = x + 1\n", 
        "WhileStmt(condition: BinaryExpr(IdentifierExpr(x) < LiteralExpr(10)), body: BlockStmt([ExprStmt(BinaryExpr(IdentifierExpr(x) = BinaryExpr(IdentifierExpr(x) + LiteralExpr(1))))]))");
    EXPECT_FALSE(parser->hasErrors());
}

// Test suite for function definition parsing
TEST_F(ParserSuite, FunctionDefinition, ParserTestFixture) {
    parseAndValidate("def add(a, b):\n    return a + b\n", 
        "FunctionDeclStmt(add(a, b), body: BlockStmt([ReturnStmt(BinaryExpr(IdentifierExpr(a) + IdentifierExpr(b)))]))");
    EXPECT_FALSE(parser->hasErrors());
}

// Test suite for error handling
TEST_F(ParserSuite, ErrorUnexpectedToken, ParserTestFixture) {
    createParser("x + \n");
    auto stmt = parser->parse();
    
    EXPECT_NE(stmt, nullptr);
    EXPECT_TRUE(parser->hasErrors());
}

TEST_F(ParserSuite, ErrorMissingParen, ParserTestFixture) {
    createParser("print(\"hello\"\n");
    auto stmt = parser->parse();
    
    EXPECT_NE(stmt, nullptr);
    EXPECT_TRUE(parser->hasErrors());
}

// Test suite for multiple statements
TEST_F(ParserSuite, MultipleStatements, ParserTestFixture) {
    createParser("x = 10\ny = 20\nresult = x + y\n");
    auto stmt = parser->parse();
    
    EXPECT_NE(stmt, nullptr);
    EXPECT_EQ("BlockStmt([ExprStmt(BinaryExpr(IdentifierExpr(x) = LiteralExpr(10))), ExprStmt(BinaryExpr(IdentifierExpr(y) = LiteralExpr(20))), ExprStmt(BinaryExpr(IdentifierExpr(result) = BinaryExpr(IdentifierExpr(x) + IdentifierExpr(y))))])", stmt->toString());
    EXPECT_FALSE(parser->hasErrors());
}

// Test suite for unary expressions
TEST_F(ParserSuite, UnaryExpression, ParserTestFixture) {
    parseAndValidate("-x\n", "ExprStmt(UnaryExpr(- IdentifierExpr(x)))");
    EXPECT_FALSE(parser->hasErrors());
}

TEST_F(ParserSuite, LogicalNot, ParserTestFixture) {
    parseAndValidate("not x\n", "ExprStmt(UnaryExpr(not IdentifierExpr(x)))");
    EXPECT_FALSE(parser->hasErrors());
}

// Main function using the new test framework
RUN_ALL_TESTS();