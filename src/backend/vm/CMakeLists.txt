# VM Module CMake Configuration

# Create VM library
add_library(rglite_vm
    VM.cpp
    Exception.cpp
)

# Link with other modules
target_link_libraries(rglite_vm
    PRIVATE
        rglite_common
        rglite_bytecode
        rglite_warnings
)

# Set include directories
target_include_directories(rglite_vm
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Set target properties
set_target_properties(rglite_vm PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)
