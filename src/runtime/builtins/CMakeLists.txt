# CMakeLists.txt for RGLite builtin functions

# Create builtin functions library
add_library(rglite_builtin
    BuiltinFunctions.cpp
)

# Include directories (modern style)
target_include_directories(rglite_builtin
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Warnings policy
target_link_libraries(rglite_builtin PRIVATE rglite_warnings)
