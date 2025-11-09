# Backend Module CMake Configuration

# Add subdirectories
add_subdirectory(bytecode)
add_subdirectory(codegen)
add_subdirectory(vm)

# Create backend library
add_library(rglite_backend INTERFACE)

# Link with submodules
target_link_libraries(rglite_backend
    INTERFACE
        rglite_bytecode
        rglite_codegen
        rglite_vm
        rglite_warnings
)

# Set include directories
target_include_directories(rglite_backend
    INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Set target properties
set_target_properties(rglite_backend PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)
