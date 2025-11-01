// RGLite Lexer Test Suite
// This file tests the Lexer functionality using Universal Test Framework
#include "TestFramework.h"
#include "Lexer.h"
#include "ErrorHandler.h"
#include <memory>
#include <sstream>
#include <vector>
#include <iostream>

// using namespace rglite; // Commented out to avoid conflicts

/**
 * @brief Lexer test fixture for common setup
 */
class LexerTestFixture : public testing::TestFixture {
protected:
    std::shared_ptr<rglite::StandardErrorHandler> errorHandler;
    std::unique_ptr<rglite::Lexer> lexer;
    
    void SetUp() override {
        errorHandler = std::make_shared<rglite::StandardErrorHandler>();
    }
    
    void TearDown() override {
        lexer.reset();
        errorHandler.reset();
    }
    
    /**
     * @brief Helper function to test Lexer with source code
     */
    void testLexerTokens(const std::string& source, const std::string& test_name) {
        lexer = std::make_unique<rglite::Lexer>(source, test_name, errorHandler);
        
        std::vector<rglite::Token> tokens;
        rglite::Token token;
        int tokenCount = 0;
        
        do {
            token = lexer->nextToken();
            tokens.push_back(token);
            tokenCount++;
            
            // Safety check to prevent infinite loop
            if (tokenCount > 100) {
                break;
            }
        } while (token.type != rglite::TokenType::END_OF_FILE);
        
        // Verify no errors occurred
        EXPECT_FALSE(errorHandler->hasErrors());
        
        // Verify we got at least some tokens (not just EOF)
        EXPECT_GT(tokens.size(), 1);
    }
    
    /**
     * @brief Helper function to test Lexer with expected token types
     */
    void testLexerTokenTypes(const std::string& source, const std::vector<rglite::TokenType>& expected_types) {
        lexer = std::make_unique<rglite::Lexer>(source, "token_type_test", errorHandler);
        
        std::vector<rglite::TokenType> actual_types;
        rglite::Token token;
        
        do {
            token = lexer->nextToken();
            actual_types.push_back(token.type);
        } while (token.type != rglite::TokenType::END_OF_FILE);
        
        // Verify no errors
        EXPECT_FALSE(errorHandler->hasErrors());
        
        // Verify token types match
        EXPECT_EQ(expected_types.size(), actual_types.size());
        
        for (size_t i = 0; i < std::min(expected_types.size(), actual_types.size()); ++i) {
            EXPECT_EQ(expected_types[i], actual_types[i]);
        }
    }
};

// Test suite for basic lexer functionality
TEST_F(LexerSuite, SimpleAssignment, LexerTestFixture) {
    testLexerTokens("x = 10", "SimpleAssignment");
}

TEST_F(LexerSuite, FunctionDefinition, LexerTestFixture) {
    testLexerTokens("def add(a, b):\n    return a + b", "FunctionDefinition");
}

TEST_F(LexerSuite, IfStatement, LexerTestFixture) {
    testLexerTokens("if x > 0:\n    print(x)", "IfStatement");
}

TEST_F(LexerSuite, WhileLoop, LexerTestFixture) {
    testLexerTokens("while x < 10:\n    x = x + 1", "WhileLoop");
}

TEST_F(LexerSuite, ArithmeticExpression, LexerTestFixture) {
    testLexerTokens("result = (a + b) * c - d / e", "ArithmeticExpression");
}

// Test suite for specific token types
TEST_F(LexerSuite, IdentifierTokens, LexerTestFixture) {
    testLexerTokenTypes("variable_name anotherVar", 
        {rglite::TokenType::IDENTIFIER, rglite::TokenType::IDENTIFIER, rglite::TokenType::END_OF_FILE});
}

TEST_F(LexerSuite, NumberTokens, LexerTestFixture) {
    testLexerTokenTypes("123 45.67 0x1F", 
        {rglite::TokenType::INTEGER, rglite::TokenType::FLOAT, rglite::TokenType::INTEGER, rglite::TokenType::END_OF_FILE});
}

TEST_F(LexerSuite, OperatorTokens, LexerTestFixture) {
    testLexerTokenTypes("+ - * / = == != < > <= >=", 
        {rglite::TokenType::OP_PLUS, rglite::TokenType::OP_MINUS, rglite::TokenType::OP_MULTIPLY, rglite::TokenType::OP_DIVIDE, 
         rglite::TokenType::OP_ASSIGN, rglite::TokenType::OP_EQUAL, rglite::TokenType::OP_NOT_EQUAL,
         rglite::TokenType::OP_LESS, rglite::TokenType::OP_GREATER, rglite::TokenType::OP_LESS_EQUAL, rglite::TokenType::OP_GREATER_EQUAL,
         rglite::TokenType::END_OF_FILE});
}

TEST_F(LexerSuite, StringTokens, LexerTestFixture) {
    testLexerTokenTypes("\"hello\" 'world'", 
        {rglite::TokenType::STRING, rglite::TokenType::STRING, rglite::TokenType::END_OF_FILE});
}

TEST_F(LexerSuite, IndentationTokens, LexerTestFixture) {
    testLexerTokenTypes("if x:\n    y = 1\n    if y:\n        z = 2", {
        rglite::TokenType::KW_IF, rglite::TokenType::IDENTIFIER, rglite::TokenType::PUNCT_COLON, rglite::TokenType::NEWLINE,
        rglite::TokenType::INDENT, rglite::TokenType::IDENTIFIER, rglite::TokenType::OP_ASSIGN, rglite::TokenType::INTEGER,
        rglite::TokenType::NEWLINE, rglite::TokenType::KW_IF, rglite::TokenType::IDENTIFIER, rglite::TokenType::PUNCT_COLON,
        rglite::TokenType::NEWLINE, rglite::TokenType::INDENT, rglite::TokenType::IDENTIFIER, rglite::TokenType::OP_ASSIGN,
        rglite::TokenType::INTEGER, rglite::TokenType::DEDENT, rglite::TokenType::DEDENT, rglite::TokenType::END_OF_FILE
    });
}

// Test suite for error handling
TEST_F(LexerSuite, UnterminatedString, LexerTestFixture) {
    lexer = std::make_unique<rglite::Lexer>("\"unterminated", "UnterminatedString", errorHandler);
    
    rglite::Token token;
    do {
        token = lexer->nextToken();
    } while (token.type != rglite::TokenType::END_OF_FILE);
    
    // Should have error for unterminated string
    EXPECT_TRUE(errorHandler->hasErrors());
}

TEST_F(LexerSuite, InvalidNumber, LexerTestFixture) {
    lexer = std::make_unique<rglite::Lexer>("123abc", "InvalidNumber", errorHandler);
    
    rglite::Token token;
    do {
        token = lexer->nextToken();
    } while (token.type != rglite::TokenType::END_OF_FILE);
    
    // Should have error for invalid number format
    EXPECT_TRUE(errorHandler->hasErrors());
}

TEST_F(LexerSuite, UnknownCharacter, LexerTestFixture) {
    lexer = std::make_unique<rglite::Lexer>("x @ y", "UnknownCharacter", errorHandler);
    
    rglite::Token token;
    do {
        token = lexer->nextToken();
    } while (token.type != rglite::TokenType::END_OF_FILE);
    
    // Should have error for unknown character
    EXPECT_TRUE(errorHandler->hasErrors());
}

TEST_F(LexerSuite, MixedIndentation, LexerTestFixture) {
    lexer = std::make_unique<rglite::Lexer>("if x:\n  \t y = 1", "MixedIndentation", errorHandler);
    
    rglite::Token token;
    do {
        token = lexer->nextToken();
    } while (token.type != rglite::TokenType::END_OF_FILE);
    
    // Should have error for mixed indentation
    EXPECT_TRUE(errorHandler->hasErrors());
}

// Test suite for edge cases
TEST_F(LexerSuite, EmptyFile, LexerTestFixture) {
    testLexerTokenTypes("", {rglite::TokenType::END_OF_FILE});
}

TEST_F(LexerSuite, OnlyWhitespace, LexerTestFixture) {
    testLexerTokenTypes("   \n\t  ", {rglite::TokenType::END_OF_FILE});
}

TEST_F(LexerSuite, CommentsOnly, LexerTestFixture) {
    testLexerTokenTypes("# This is a comment\n# Another comment", {rglite::TokenType::END_OF_FILE});
}

TEST_F(LexerSuite, ComplexExpression, LexerTestFixture) {
    testLexerTokens("result = (a + b * c) / (d - e) if condition else default", "ComplexExpression");
}

TEST_F(LexerSuite, FunctionWithMultipleArgs, LexerTestFixture) {
    testLexerTokens("def complex_func(a, b, c=10, *args, **kwargs):\n    return a + b + c", "FunctionWithMultipleArgs");
}

TEST_F(LexerSuite, NestedBlocks, LexerTestFixture) {
    testLexerTokens("if x:\n    if y:\n        if z:\n            w = 1", "NestedBlocks");
}



// Main function using the new test framework
RUN_ALL_TESTS();