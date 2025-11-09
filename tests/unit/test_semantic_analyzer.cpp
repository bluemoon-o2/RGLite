// RGLite Semantic Analyzer Test Suite
// This file tests the SemanticAnalyzer functionality using Universal Test Framework
#include <memory>
#include <sstream>
#include <iostream>
#include <vector>
#include <string>
#include "TestFramework.h"
#include "SemanticAnalyzer.h"
#include "Parser.h"
#include "Lexer.h"
#include "ErrorHandler.h"

using namespace rglite;

/**
 * @brief Semantic Analyzer test fixture for common setup
 */
class SemanticAnalyzerTestFixture : public testing::TestFixture {
protected:
    std::shared_ptr<StandardErrorHandler> errorHandler;
    std::unique_ptr<SemanticAnalyzer> analyzer;
    
    void SetUp() override {
        errorHandler = std::make_shared<StandardErrorHandler>();
        analyzer = std::make_unique<SemanticAnalyzer>(errorHandler);
    }
    
    void TearDown() override {
        analyzer.reset();
        errorHandler.reset();
    }
    
    /**
     * @brief Helper function to parse and analyze source code
     */
    bool parseAndAnalyze(const std::string& source) {
        auto lexer = std::make_unique<Lexer>(source, "test.rgb", errorHandler);
        auto parser = std::make_unique<Parser>(std::move(lexer), errorHandler);
        auto ast = parser->parse();
        
        if (errorHandler->hasErrors()) {
            return false;
        }
        
        // Always return true to indicate analysis completed successfully
        // Use hasErrors() to check if semantic errors were detected
        analyzer->analyze(ast);
        return true;
    }
    
    /**
     * @brief Helper function to check if analysis has errors
     */
    bool hasErrors() const {
        return analyzer->hasErrors();
    }
};

// Test suite for variable declaration analysis
TEST_F(SemanticAnalyzerSuite, VariableDeclaration, SemanticAnalyzerTestFixture) {
    EXPECT_TRUE(parseAndAnalyze("x = 42\n"));
    EXPECT_FALSE(hasErrors());
}

TEST_F(SemanticAnalyzerSuite, VariableDeclarationWithExpression, SemanticAnalyzerTestFixture) {
    EXPECT_TRUE(parseAndAnalyze("result = a + b\n"));
    EXPECT_TRUE(hasErrors()); // 'a' and 'b' are undefined
}

TEST_F(SemanticAnalyzerSuite, VariableRedeclaration, SemanticAnalyzerTestFixture) {
    EXPECT_TRUE(parseAndAnalyze("x = 42\nx = 10\n"));
    
    // Debug: print errors if any
    if (hasErrors()) {
        std::cout << "[DEBUG] Errors detected in VariableRedeclaration test:" << std::endl;
        const auto& diagnostics = errorHandler->getDiagnostics();
        for (const auto& diag : diagnostics) {
            std::cout << "  " << diag.toString() << std::endl;
        }
    }
    
    EXPECT_FALSE(hasErrors()); // reassignment is allowed
}

// Test suite for function definition analysis
TEST_F(SemanticAnalyzerSuite, FunctionDefinition, SemanticAnalyzerTestFixture) {
    EXPECT_TRUE(parseAndAnalyze("def add(a, b):\n    return a + b\n"));
    EXPECT_FALSE(hasErrors());
}

TEST_F(SemanticAnalyzerSuite, FunctionRedeclaration, SemanticAnalyzerTestFixture) {
    EXPECT_TRUE(parseAndAnalyze("def add(a, b):\n    return a + b\ndef add(x, y):\n    return x + y\n"));
    EXPECT_TRUE(hasErrors()); // 'add' is redeclared
}

TEST_F(SemanticAnalyzerSuite, FunctionParameterRedeclaration, SemanticAnalyzerTestFixture) {
    EXPECT_TRUE(parseAndAnalyze("def add(a, a):\n    return a + a\n"));
    EXPECT_TRUE(hasErrors()); // parameter 'a' is redeclared
}

// Test suite for function call analysis
TEST_F(SemanticAnalyzerSuite, BuiltinFunctionCall, SemanticAnalyzerTestFixture) {
    EXPECT_TRUE(parseAndAnalyze("print(\"hello\")\n"));
    EXPECT_FALSE(hasErrors());
}

TEST_F(SemanticAnalyzerSuite, UndefinedFunctionCall, SemanticAnalyzerTestFixture) {
    EXPECT_TRUE(parseAndAnalyze("undefined()\n"));
    EXPECT_TRUE(hasErrors()); // 'undefined' is not defined
}

TEST_F(SemanticAnalyzerSuite, WrongArgumentCount, SemanticAnalyzerTestFixture) {
    EXPECT_TRUE(parseAndAnalyze("def add(a, b):\n    return a + b\nadd(1)\n"));
    EXPECT_TRUE(hasErrors()); // wrong number of arguments
}

// Test suite for variable usage analysis
TEST_F(SemanticAnalyzerSuite, UndefinedVariable, SemanticAnalyzerTestFixture) {
    EXPECT_TRUE(parseAndAnalyze("x + 1\n"));
    EXPECT_TRUE(hasErrors()); // 'x' is undefined
}

TEST_F(SemanticAnalyzerSuite, UninitializedVariable, SemanticAnalyzerTestFixture) {
    EXPECT_TRUE(parseAndAnalyze("x\nx = 42\n"));
    EXPECT_TRUE(hasErrors()); // 'x' is used before initialization
}

TEST_F(SemanticAnalyzerSuite, VariableScope, SemanticAnalyzerTestFixture) {
    EXPECT_TRUE(parseAndAnalyze("x = 10\ndef test():\n    x = 20\n    return x\n"));
    EXPECT_FALSE(hasErrors()); // different scopes
}

// Test suite for control flow analysis
TEST_F(SemanticAnalyzerSuite, IfStatement, SemanticAnalyzerTestFixture) {
    EXPECT_TRUE(parseAndAnalyze("if x > 0:\n    print(\"positive\")\n"));
    EXPECT_TRUE(hasErrors()); // 'x' is undefined
}

TEST_F(SemanticAnalyzerSuite, WhileStatement, SemanticAnalyzerTestFixture) {
    EXPECT_TRUE(parseAndAnalyze("x = 0\nwhile x < 10:\n    x = x + 1\n"));
    EXPECT_FALSE(hasErrors());
}

TEST_F(SemanticAnalyzerSuite, ReturnOutsideFunction, SemanticAnalyzerTestFixture) {
    EXPECT_TRUE(parseAndAnalyze("return 42\n"));
    EXPECT_TRUE(hasErrors()); // return outside function
}

// Test suite for type checking
TEST_F(SemanticAnalyzerSuite, ArithmeticOperations, SemanticAnalyzerTestFixture) {
    EXPECT_TRUE(parseAndAnalyze("x = 1 + 2\ny = 3.5 * 2\nz = 10 / 3\n"));
    EXPECT_FALSE(hasErrors());
}

TEST_F(SemanticAnalyzerSuite, StringConcatenation, SemanticAnalyzerTestFixture) {
    EXPECT_TRUE(parseAndAnalyze("x = \"hello\" + \" \" + \"world\"\n"));
    EXPECT_FALSE(hasErrors());
}

TEST_F(SemanticAnalyzerSuite, InvalidStringOperation, SemanticAnalyzerTestFixture) {
    EXPECT_TRUE(parseAndAnalyze("x = \"hello\" * \"world\"\n"));
    EXPECT_TRUE(hasErrors()); // invalid string operation
}

TEST_F(SemanticAnalyzerSuite, ComparisonOperations, SemanticAnalyzerTestFixture) {
    EXPECT_TRUE(parseAndAnalyze("x = 1 < 2\ny = \"a\" == \"b\"\nz = 3 <= 3\n"));
    EXPECT_FALSE(hasErrors());
}

TEST_F(SemanticAnalyzerSuite, LogicalOperations, SemanticAnalyzerTestFixture) {
    EXPECT_TRUE(parseAndAnalyze("x = true and false\ny = not true\nz = 1 or 0\n"));
    EXPECT_FALSE(hasErrors());
}

// Test suite for assignment analysis
TEST_F(SemanticAnalyzerSuite, SimpleAssignment, SemanticAnalyzerTestFixture) {
    EXPECT_TRUE(parseAndAnalyze("x = 42\nx = 10\n"));
    EXPECT_FALSE(hasErrors()); // reassignment is allowed
}

TEST_F(SemanticAnalyzerSuite, InvalidAssignment, SemanticAnalyzerTestFixture) {
    // This test is now testing that the parser correctly rejects invalid assignment targets
    // The parser should fail to parse "1 = 2" since literals cannot be assignment targets
    EXPECT_FALSE(parseAndAnalyze("1 = 2\n"));
    // Since parsing failed, we don't expect semantic analysis to run
    // The error is caught at the parser level, not the semantic analyzer level
}

TEST_F(SemanticAnalyzerSuite, AssignmentToUndefined, SemanticAnalyzerTestFixture) {
    EXPECT_TRUE(parseAndAnalyze("x = y\n"));
    EXPECT_TRUE(hasErrors()); // 'y' is undefined
}

// Test suite for nested scopes
TEST_F(SemanticAnalyzerSuite, NestedFunctionScopes, SemanticAnalyzerTestFixture) {
    EXPECT_TRUE(parseAndAnalyze("x = 10\ndef outer():\n    x = 20\n    def inner():\n        x = 30\n        return x\n    return inner()\n"));
    
    // Debug: print errors if any
    if (hasErrors()) {
        std::cout << "[DEBUG] Errors detected in NestedFunctionScopes test:" << std::endl;
        const auto& diagnostics = errorHandler->getDiagnostics();
        for (const auto& diag : diagnostics) {
            std::cout << "  " << diag.toString() << std::endl;
        }
    }
    
    EXPECT_FALSE(hasErrors());
}

TEST_F(SemanticAnalyzerSuite, VariableShadowing, SemanticAnalyzerTestFixture) {
    EXPECT_TRUE(parseAndAnalyze("x = 10\ndef test():\n    x = 20\n    return x\n"));
    EXPECT_FALSE(hasErrors()); // variable shadowing is allowed
}

// Test suite for complex expressions
TEST_F(SemanticAnalyzerSuite, ComplexArithmetic, SemanticAnalyzerTestFixture) {
    EXPECT_TRUE(parseAndAnalyze("x = (1 + 2) * (3 - 4) / 5\n"));
    EXPECT_FALSE(hasErrors());
}

TEST_F(SemanticAnalyzerSuite, ComplexLogical, SemanticAnalyzerTestFixture) {
    EXPECT_TRUE(parseAndAnalyze("x = (a > 0) and (b < 10) or not c\n"));
    EXPECT_TRUE(hasErrors()); // variables are undefined
}

// Main function using the new test framework
RUN_ALL_TESTS();