# RGLite Source Modules CMake Configuration

# Add subdirectories for each module
add_subdirectory(common)
add_subdirectory(frontend)
add_subdirectory(backend)
add_subdirectory(runtime)

# Create RGLite library
add_library(rglite
    RGLite.cpp
)

# Link with submodules
target_link_libraries(rglite
    PRIVATE
        rglite_common
        rglite_frontend
        rglite_backend
        rglite_builtin
        rglite_runtime
        rglite_memory
        rglite_warnings
)

# Set include directories
target_include_directories(rglite
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Set target properties
set_target_properties(rglite PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)
