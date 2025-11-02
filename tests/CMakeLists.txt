# CMakeLists.txt for tests directory

# Add test executables
add_executable(test_lexer
    test_lexer.cpp
    ../src/common/Token.cpp
    ../src/frontend/lexer/Lexer.cpp
    ../src/common/utils/StringUtils.cpp
    ../src/common/errors/ErrorHandler.cpp
)

target_include_directories(test_lexer PRIVATE
    ../include
    ../src/frontend/lexer
    ../src/common
)

target_link_libraries(test_lexer PRIVATE
    # Add any required libraries
)

# Register test
add_test(NAME LexerTest COMMAND test_lexer)

add_executable(test_parser
    test_parser.cpp
    ../src/common/Token.cpp
    ../src/frontend/lexer/Lexer.cpp
    ../src/frontend/parser/Parser.cpp
    ../src/frontend/ast/AST.cpp
    ../src/common/utils/StringUtils.cpp
    ../src/common/errors/ErrorHandler.cpp
)

target_include_directories(test_parser PRIVATE
    ../include
    ../src/frontend/lexer
    ../src/frontend/parser
    ../src/frontend/ast
    ../src/common
)

target_link_libraries(test_parser PRIVATE
    # Add any required libraries
)

# Register test
add_test(NAME ParserTest COMMAND test_parser)

add_executable(test_semantic_analyzer
    test_semantic_analyzer.cpp
    ../src/common/Token.cpp
    ../src/frontend/lexer/Lexer.cpp
    ../src/frontend/parser/Parser.cpp
    ../src/frontend/ast/AST.cpp
    ../src/frontend/semantic/SemanticAnalyzer.cpp
    ../src/common/utils/StringUtils.cpp
    ../src/common/errors/ErrorHandler.cpp
)

target_include_directories(test_semantic_analyzer PRIVATE
    ../include
    ../src/frontend/lexer
    ../src/frontend/parser
    ../src/frontend/ast
    ../src/frontend/semantic
    ../src/common
)

target_link_libraries(test_semantic_analyzer PRIVATE
    # Add any required libraries
)

# Register test
add_test(NAME SemanticAnalyzerTest COMMAND test_semantic_analyzer)

add_executable(test_ast
    test_ast.cpp
    ../src/frontend/ast/AST.cpp
    ../src/common/utils/StringUtils.cpp
    ../src/common/errors/ErrorHandler.cpp
)

target_include_directories(test_ast PRIVATE
    ../include
    ../src/frontend/ast
    ../src/common
)

target_link_libraries(test_ast PRIVATE
    # Add any required libraries
)

# Register test
add_test(NAME ASTTest COMMAND test_ast)