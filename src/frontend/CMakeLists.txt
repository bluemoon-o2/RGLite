# Frontend Module CMake Configuration

# Create frontend library
add_library(rglite_frontend
    lexer/Lexer.cpp
    parser/Parser.cpp
    ast/AST.cpp
    semantic/SemanticAnalyzer.cpp
)

# Link with common module
target_link_libraries(rglite_frontend
    PRIVATE
        rglite_common
        rglite_warnings
)

# Set include directories
target_include_directories(rglite_frontend
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Set target properties
set_target_properties(rglite_frontend PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)
