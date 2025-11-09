# Common Module CMake Configuration

# Create common library
add_library(rglite_common
    Token.cpp
    errors/ErrorHandler.cpp
    utils/StringUtils.cpp
)

# Set include directories
target_include_directories(rglite_common
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Set target properties
set_target_properties(rglite_common PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Warnings policy
target_link_libraries(rglite_common PRIVATE rglite_warnings)
