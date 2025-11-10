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

// Test suite for member and index assignment parsing
TEST_F(ParserSuite, MemberAssignment, ParserTestFixture) {
    parseAndValidate("d.attr = 1\n",
        "ExprStmt(BinaryExpr(MemberAccessExpr(IdentifierExpr(d).attr) = LiteralExpr(1)))");
    EXPECT_FALSE(parser->hasErrors());
}

TEST_F(ParserSuite, IndexAssignment, ParserTestFixture) {
    parseAndValidate("a[0] = 42\n",
        "ExprStmt(BinaryExpr(IndexAccessExpr(IdentifierExpr(a)[LiteralExpr(0)]) = LiteralExpr(42)))");
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

// Test suite for 'in' and 'not in' operators
TEST_F(ParserSuite, InOperator, ParserTestFixture) {
    parseAndValidate("x in [1, 2, 3]\n", 
        "ExprStmt(BinaryExpr(IdentifierExpr(x) in ListExpr([LiteralExpr(1), LiteralExpr(2), LiteralExpr(3)])))");
    EXPECT_FALSE(parser->hasErrors());
}

TEST_F(ParserSuite, NotInOperator, ParserTestFixture) {
    parseAndValidate("x not in [1, 2, 3]\n", 
        "ExprStmt(UnaryExpr(not BinaryExpr(IdentifierExpr(x) in ListExpr([LiteralExpr(1), LiteralExpr(2), LiteralExpr(3)]))))");
    EXPECT_FALSE(parser->hasErrors());
}

TEST_F(ParserSuite, InOperatorWithString, ParserTestFixture) {
    parseAndValidate("'a' in 'hello'\n", 
        "ExprStmt(BinaryExpr(LiteralExpr(\"a\") in LiteralExpr(\"hello\")))");
    EXPECT_FALSE(parser->hasErrors());
}

TEST_F(ParserSuite, NotInOperatorWithString, ParserTestFixture) {
    parseAndValidate("'x' not in 'hello'\n", 
        "ExprStmt(UnaryExpr(not BinaryExpr(LiteralExpr(\"x\") in LiteralExpr(\"hello\"))))");
    EXPECT_FALSE(parser->hasErrors());
}

TEST_F(ParserSuite, InOperatorWithDict, ParserTestFixture) {
    parseAndValidate("'key' in {'key': 'value'}\n", 
        "ExprStmt(BinaryExpr(LiteralExpr(\"key\") in DictExpr({LiteralExpr(\"key\"): LiteralExpr(\"value\")})))");
    EXPECT_FALSE(parser->hasErrors());
}

TEST_F(ParserSuite, InOperatorComplexExpression, ParserTestFixture) {
    parseAndValidate("x in [1, 2, 3] and y not in [4, 5, 6]\n", 
        "ExprStmt(BinaryExpr(BinaryExpr(IdentifierExpr(x) in ListExpr([LiteralExpr(1), LiteralExpr(2), LiteralExpr(3)])) and UnaryExpr(not BinaryExpr(IdentifierExpr(y) in ListExpr([LiteralExpr(4), LiteralExpr(5), LiteralExpr(6)])))))");
    EXPECT_FALSE(parser->hasErrors());
}

// Test error scenarios for 'in' operator
TEST_F(ParserSuite, InOperatorErrorMissingRightOperand, ParserTestFixture) {
    createParser("x in\n");
    auto stmt = parser->parse();
    
    EXPECT_NE(stmt, nullptr);
    EXPECT_TRUE(parser->hasErrors());
}

TEST_F(ParserSuite, NotInOperatorErrorMissingRightOperand, ParserTestFixture) {
    createParser("x not in\n");
    auto stmt = parser->parse();
    
    EXPECT_NE(stmt, nullptr);
    EXPECT_TRUE(parser->hasErrors());
}

// Test suite for import statements
TEST_F(ParserSuite, ImportStatement, ParserTestFixture) {
    parseAndValidate("import math\n", "ImportStmt([math])");
    EXPECT_FALSE(parser->hasErrors());
}

TEST_F(ParserSuite, ImportStatementWithAlias, ParserTestFixture) {
    parseAndValidate("import math as m\n", "ImportStmt([math as m])");
    EXPECT_FALSE(parser->hasErrors());
}

TEST_F(ParserSuite, ImportMultipleModules, ParserTestFixture) {
    parseAndValidate("import os, sys as s\n", "ImportStmt([os, sys as s])");
    EXPECT_FALSE(parser->hasErrors());
}

TEST_F(ParserSuite, FromImportStatement, ParserTestFixture) {
    parseAndValidate("from math import sqrt\n", "FromImportStmt(module: math, names: [sqrt])");
    EXPECT_FALSE(parser->hasErrors());
}

TEST_F(ParserSuite, FromImportStatementWithAliasList, ParserTestFixture) {
    parseAndValidate("from math import sqrt as s, cos\n", "FromImportStmt(module: math, names: [sqrt as s, cos])");
    EXPECT_FALSE(parser->hasErrors());
}

TEST_F(ParserSuite, FromImportStar, ParserTestFixture) {
    parseAndValidate("from math import *\n", "FromImportStmt(module: math, names: *)");
    EXPECT_FALSE(parser->hasErrors());
}

// Main function using the new test framework
RUN_ALL_TESTS();
