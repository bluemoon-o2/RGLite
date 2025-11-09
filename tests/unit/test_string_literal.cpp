#include "TestFramework.h"
#include "RGLite.h"
#include <iostream>
#include <memory>

using namespace rglite;

// Test fixture for string literal tests
class StringLiteralTestFixture : public testing::TestFixture {
protected:
    std::shared_ptr<Compiler> compiler;
    
    void SetUp() override {
        compiler = createCompiler();
    }
    
    void TearDown() override {
        compiler.reset();
    }
};

// Test suite for string literals
TEST_F(StringLiteralTestFixture, SimpleStringLiteral, StringLiteralTestFixture) {
    // Test simple string literal
    std::string source = R"(
print("Hello, World!")
)";
    
    bool result = (compiler->execute(source) == 0);
    EXPECT_TRUE(result);
}

TEST_F(StringLiteralTestFixture, StringVariableAssignment, StringLiteralTestFixture) {
    // Test string variable assignment
    std::string source = R"(
message = "Hello, RGLite!"
print(message)
)";
    
    bool result = (compiler->execute(source) == 0);
    EXPECT_TRUE(result);
}

TEST_F(StringLiteralTestFixture, StringConcatenation, StringLiteralTestFixture) {
    // Test string concatenation
    std::string source = R"(
greeting = "Hello"
name = "RGLite"
message = greeting + ", " + name + "!"
print(message)
)";
    
    bool result = (compiler->execute(source) == 0);
    EXPECT_TRUE(result);
}

TEST_F(StringLiteralTestFixture, StringWithQuotes, StringLiteralTestFixture) {
    // Test string with quotes
    std::string source = R"(
message = 'Hello, "RGLite"!'
print(message)
)";
    
    bool result = (compiler->execute(source) == 0);
    EXPECT_TRUE(result);
}

TEST_F(StringLiteralTestFixture, StringWithEscapeSequences, StringLiteralTestFixture) {
    // Test string with escape sequences
    std::string source = R"(
message = "Hello\nRGLite\t!"
print(message)
)";
    
    bool result = (compiler->execute(source) == 0);
    EXPECT_TRUE(result);
}

TEST_F(StringLiteralTestFixture, TripleDoubleQuoteMultilineString, StringLiteralTestFixture) {
    // Test triple double-quoted multi-line string
    std::string source = R"(
message = """Hello
RGLite
"""
print(message)
)";
    
    bool result = (compiler->execute(source) == 0);
    EXPECT_TRUE(result);
}

TEST_F(StringLiteralTestFixture, TripleSingleQuoteMultilineString, StringLiteralTestFixture) {
    // Test triple single-quoted multi-line string
    std::string source = R"(
message = '''Hello
"RGLite"!
Line2'''
print(message)
)";
    
    bool result = (compiler->execute(source) == 0);
    EXPECT_TRUE(result);
}

TEST_F(StringLiteralTestFixture, TripleQuoteAsBlockComment, StringLiteralTestFixture) {
    // Triple-quoted block used as comment-like statement at top-level
    std::string source = R"(
"""This is a multi-line comment
Spanning multiple lines"""
x = 1
print(x + 2)
)";
    
    bool result = (compiler->execute(source) == 0);
    EXPECT_TRUE(result);
}

// Main test runner
RUN_ALL_TESTS()
