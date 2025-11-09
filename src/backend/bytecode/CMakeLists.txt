# Bytecode Module CMake Configuration

# Create bytecode library
add_library(rglite_bytecode
    Bytecode.cpp
)

# Link with common module
target_link_libraries(rglite_bytecode
    PRIVATE
        rglite_common
)

# Set include directories
target_include_directories(rglite_bytecode
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Set target properties
set_target_properties(rglite_bytecode PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Warnings policy
target_link_libraries(rglite_bytecode PRIVATE rglite_warnings)
