# Tests CMakeLists.txt for RGLite

# Common include: expose test framework to subdirs
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Helper to create a labeled test target
function(_create_labeled_test test_name label)
    add_executable(${test_name} ${test_name}.cpp)
    target_link_libraries(${test_name} PRIVATE rglite)
    target_include_directories(${test_name} PRIVATE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/tests>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
    )
    set_target_properties(${test_name} PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )
    add_test(NAME ${test_name} COMMAND ${test_name})
    set_tests_properties(${test_name} PROPERTIES LABELS ${label})
endfunction()

# Public functions for subdirectories
function(create_unit_test test_name)
    _create_labeled_test(${test_name} unit)
endfunction()

function(create_perf_test test_name)
    _create_labeled_test(${test_name} perf)
endfunction()

# Subdirectories for categorized tests
add_subdirectory(unit)
add_subdirectory(perf)
