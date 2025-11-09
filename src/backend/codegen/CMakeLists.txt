# Code Generator Module CMake Configuration

# Create codegen library
add_library(rglite_codegen
    CodeGenerator.cpp
)

# Link with other modules
target_link_libraries(rglite_codegen
    PRIVATE
        rglite_common
        rglite_frontend
        rglite_bytecode
        rglite_warnings
)

# Set include directories
target_include_directories(rglite_codegen
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Set target properties
set_target_properties(rglite_codegen PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)
