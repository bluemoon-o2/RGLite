#include <iostream>
#include <string>
#include "Lexer.h"
#include "Token.h"
#include "ErrorHandler.h"

using namespace rglite;

void testNestedFunctionIndent() {
    std::string source = 
        "x = 10\n"
        "def outer():\n"
        "    x = 20\n"
        "    def inner():\n"
        "        x = 30\n"
        "        return x\n"
        "    return inner()\n";
    
    auto errorHandler = std::make_shared<StandardErrorHandler>();
    auto lexer = std::make_unique<Lexer>(source, "test_input", errorHandler);
    
    std::cout << "=== Nested Function Indent Test ===" << std::endl;
    std::cout << "Source code:" << std::endl;
    std::cout << source << std::endl;
    std::cout << "=== Token Sequence ===" << std::endl;
    
    int tokenCount = 0;
    Token token;
    
    do {
        token = lexer->nextToken();
        std::cout << "Token " << tokenCount << ": " << token.toString() << std::endl;
        
        // Check DEDENT tokens at line 7
        if (token.location.line == 7 && token.type == TokenType::DEDENT) {
            std::cout << "  *** DEDENT at line 7, column " << token.location.column << " ***" << std::endl;
        }
        
        tokenCount++;
    } while (token.type != TokenType::END_OF_FILE);
    
    std::cout << "=== Analysis ===" << std::endl;
    std::cout << "Line 6: return x (indent level 2, 8 spaces)" << std::endl;
    std::cout << "Line 7: return inner() (indent level 1, 4 spaces)" << std::endl;
    std::cout << "Expected: Only one DEDENT token at line 7" << std::endl;
    std::cout << "Actual: Check DEDENT token count at line 7 above" << std::endl;
}

int main() {
    testNestedFunctionIndent();
    return 0;
}