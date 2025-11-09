# RGLite Language Compiler - CMake Configuration
cmake_minimum_required(VERSION 3.15)
project(RGLite VERSION 1.0.0 LANGUAGES CXX)

include(GNUInstallDirs)

# Global settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Options
option(RGLITE_BUILD_TESTS "Build unit/perf tests" ON)
option(RGLITE_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)

# Centralized warnings target (modern target-based approach)
add_library(rglite_warnings INTERFACE)
if(MSVC)
    target_compile_options(rglite_warnings INTERFACE /W4 $<$<BOOL:${RGLITE_WARNINGS_AS_ERRORS}>:/WX> /utf-8)
else()
    target_compile_options(rglite_warnings INTERFACE -Wall -Wextra -Wpedantic $<$<BOOL:${RGLITE_WARNINGS_AS_ERRORS}>:-Werror>)
endif()

# Paths
set(RGLITE_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
set(RGLITE_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)

# Modules
add_subdirectory(src)
if(RGLITE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Main executable
add_executable(rg
    src/main.cpp
)

# Link with RGLite library
target_link_libraries(rg PRIVATE rglite rglite_warnings)

# Set target properties
set_target_properties(rg PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Add tests
# Note: tests are already added above

# Installation
install(TARGETS rg RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(TARGETS rglite
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${RGLITE_INCLUDE_DIR}/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Package configuration
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/RGLiteConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# Export targets
install(TARGETS rglite rglite_common rglite_frontend rglite_backend rglite_builtin rglite_memory rglite_bytecode rglite_codegen rglite_vm EXPORT rglite-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Export interface warnings target (no files, but required by exported targets)
install(TARGETS rglite_warnings EXPORT rglite-targets)

install(EXPORT rglite-targets
    FILE RGLiteTargets.cmake
    NAMESPACE RGLite::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/RGLite
)

# Create config file
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/RGLiteConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/RGLiteConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/RGLite
)

# Install config files
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/RGLiteConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/RGLiteConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/RGLite
)
