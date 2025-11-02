// RGLite Indentation Test Program
// This program tests the Lexer's indentation handling functionality using Google Test-style framework

#include <iostream>
#include <fstream>
#include <string>
#include "TestFramework.h"
#include "Lexer.h"
#include "ErrorHandler.h"

using namespace rglite;

/**
 * @brief Indentation test fixture for common setup
 */
class IndentTestFixture : public testing::TestFixture {
public:
    std::shared_ptr<StandardErrorHandler> errorHandler;
    
    void SetUp() override {
        errorHandler = std::make_shared<StandardErrorHandler>();
    }
    
    void TearDown() override {
        errorHandler.reset();
    }
    
    /**
     * @brief Helper function to test Lexer indentation handling
     */
    bool testIndentation(const std::string& source, const std::string& description) {
        (void)description; // Mark as unused to avoid warning
        auto lexer = std::make_unique<Lexer>(source, "test_input", errorHandler);
        
        int tokenCount = 0;
        rglite::Token token;
        bool hasIndent = false;
        bool hasDedent = false;
        
        do {
            token = lexer->nextToken();
            
            // Check for INDENT and DEDENT tokens
            if (token.type == rglite::TokenType::INDENT) {
                hasIndent = true;
            } else if (token.type == rglite::TokenType::DEDENT) {
                hasDedent = true;
            }
            
            tokenCount++;
        } while (token.type != rglite::TokenType::END_OF_FILE);
        
        if (errorHandler->hasErrors()) {
            return false;
        }
        
        // Verify that we have both INDENT and DEDENT tokens
        if (!hasIndent || !hasDedent) {
            return false;
        }
        
        return true;
    }
};

// Test suite for indentation functionality
TEST(IndentSuite, SimpleIndentation) {
    IndentTestFixture fixture;
    fixture.SetUp();
    
    bool result = fixture.testIndentation(
        "if x > 0:\n    print(x)",
        "Simple indentation"
    );
    
    EXPECT_TRUE(result);
    
    fixture.TearDown();
}

TEST(IndentSuite, NestedIndentation) {
    IndentTestFixture fixture;
    fixture.SetUp();
    
    bool result = fixture.testIndentation(
        "x = 10\ndef outer():\n    x = 20\n    def inner():\n        x = 30\n        return x\n    return inner()\n",
        "Nested indentation"
    );
    
    EXPECT_TRUE(result);
    
    fixture.TearDown();
}

TEST(IndentSuite, FunctionMultipleIndentedStatements) {
    IndentTestFixture fixture;
    fixture.SetUp();
    
    bool result = fixture.testIndentation(
        "def test():\n    x = 10\n    y = 20\n    return x + y",
        "Function with multiple indented statements"
    );
    
    EXPECT_TRUE(result);
    
    fixture.TearDown();
}

TEST(IndentSuite, MixedIndentationLevels) {
    IndentTestFixture fixture;
    fixture.SetUp();
    
    bool result = fixture.testIndentation(
        "if a:\n    if b:\n        print(1)\n    print(2)",
        "Mixed indentation levels"
    );
    
    EXPECT_TRUE(result);
    
    fixture.TearDown();
}

TEST(IndentSuite, EmptyIndentedBlock) {
    IndentTestFixture fixture;
    fixture.SetUp();
    
    bool result = fixture.testIndentation(
        "if condition:\n    pass",
        "Empty indented block"
    );
    
    EXPECT_TRUE(result);
    
    fixture.TearDown();
}

// Main function using the new test framework
RUN_ALL_TESTS();