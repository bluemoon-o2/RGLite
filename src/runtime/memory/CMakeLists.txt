# CMakeLists.txt for RGLite memory management

# Create memory management library
add_library(rglite_memory
    MemoryManager.cpp
)

# Include directories (modern style)
target_include_directories(rglite_memory
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Warnings policy
target_link_libraries(rglite_memory PRIVATE rglite_warnings)
