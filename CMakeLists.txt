# RGLite Language Compiler - CMake Configuration
cmake_minimum_required(VERSION 3.15)
project(RGLite VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler options
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /WX")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -Wpedantic")
endif()

# Project configuration
set(RGLITE_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
set(RGLITE_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)

# Include directories
include_directories(
    ${RGLITE_INCLUDE_DIR}
    ${RGLITE_SOURCE_DIR}
)

# Main executable with all source files# Add executable
add_executable(rg
    src/main.cpp
    src/RGLite.cpp
    src/common/utils/StringUtils.cpp
    src/frontend/lexer/Lexer.cpp
    src/common/errors/ErrorHandler.cpp
    src/frontend/parser/Parser.cpp
    src/frontend/ast/AST.cpp
    src/common/Token.cpp
)

# Add Lexer test executable
add_executable(test_lexer
    tests/test_lexer.cpp
    src/RGLite.cpp
    src/common/utils/StringUtils.cpp
    src/frontend/lexer/Lexer.cpp
    src/common/errors/ErrorHandler.cpp
    src/common/Token.cpp
)

# Add AST test executable
add_executable(test_ast
    tests/test_ast.cpp
    src/RGLite.cpp
    src/common/utils/StringUtils.cpp
    src/frontend/lexer/Lexer.cpp
    src/frontend/parser/Parser.cpp
    src/frontend/ast/AST.cpp
    src/common/errors/ErrorHandler.cpp
    src/common/Token.cpp
)

# Add Parser test executable
add_executable(test_parser
    tests/test_parser.cpp
    src/RGLite.cpp
    src/common/utils/StringUtils.cpp
    src/frontend/lexer/Lexer.cpp
    src/frontend/parser/Parser.cpp
    src/frontend/ast/AST.cpp
    src/common/errors/ErrorHandler.cpp
    src/common/Token.cpp
)

# Add Semantic Analyzer test executable
add_executable(test_semantic_analyzer
    tests/test_semantic_analyzer.cpp
    src/RGLite.cpp
    src/common/utils/StringUtils.cpp
    src/frontend/lexer/Lexer.cpp
    src/frontend/parser/Parser.cpp
    src/frontend/ast/AST.cpp
    src/frontend/semantic/SemanticAnalyzer.cpp
    src/common/errors/ErrorHandler.cpp
    src/common/Token.cpp
)

# Add Indentation test executable
add_executable(test_indent
    tests/test_indent.cpp
    src/RGLite.cpp
    src/common/utils/StringUtils.cpp
    src/frontend/lexer/Lexer.cpp
    src/common/errors/ErrorHandler.cpp
    src/common/Token.cpp
)

add_executable(test_framework_debug
    tests/test_framework_debug.cpp
)

add_executable(test_complex_fixture
    tests/test_complex_fixture.cpp
)

# Testing configuration
if(BUILD_TESTING)
    enable_testing()
    
    # Find GoogleTest
    find_package(GTest REQUIRED)
    
    # Test executable
    add_executable(rglite_tests
        tests/test_lexer.cpp
        tests/test_parser.cpp
        tests/test_vm.cpp
    )
    
    target_link_libraries(rglite_tests
        rglite_frontend
        rglite_backend
        rglite_runtime
        rglite_common
    )
    
    add_test(NAME LexerTests COMMAND rglite_tests --gtest_filter="Lexer*")
    add_test(NAME ParserTests COMMAND rglite_tests --gtest_filter="Parser*")
    add_test(NAME VMTests COMMAND rglite_tests --gtest_filter="VM*")
endif()

# Installation
install(TARGETS rg DESTINATION bin)
install(DIRECTORY ${RGLITE_INCLUDE_DIR}/ DESTINATION include)

# Package configuration
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/RGLiteConfigVersion.cmake
    VERSION ${RGLite_VERSION}
    COMPATIBILITY AnyNewerVersion
)

# Export targets
install(TARGETS rg EXPORT RGLiteTargets DESTINATION lib)
install(EXPORT RGLiteTargets DESTINATION lib/cmake/RGLite)

# Documentation
if(BUILD_DOCS)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        set(DOXYGEN_INPUT ${RGLITE_SOURCE_DIR} ${RGLITE_INCLUDE_DIR})
        set(DOXYGEN_OUTPUT_DIR ${CMAKE_BINARY_DIR}/docs)
        doxygen_add_docs(docs ${DOXYGEN_INPUT})
    endif()
endif()