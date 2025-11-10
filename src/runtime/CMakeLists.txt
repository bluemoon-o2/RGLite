# CMakeLists.txt for RGLite runtime components

# Add runtime subdirectories
add_subdirectory(builtins)
add_subdirectory(memory)

# Iterator and runtime helpers
add_library(rglite_runtime
    Iterator.cpp
)

target_include_directories(rglite_runtime
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(rglite_runtime PRIVATE rglite_warnings)
