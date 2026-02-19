cmake_minimum_required(VERSION 3.15)
project(tempoch_cpp VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find Cargo for building Rust library
find_program(CARGO_BIN cargo REQUIRED)

# Paths to tempoch-ffi
set(TEMPOCH_SUBMODULE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tempoch)
set(TEMPOCH_FFI_DIR       ${TEMPOCH_SUBMODULE_DIR}/tempoch-ffi)
set(TEMPOCH_FFI_INCLUDE_DIR ${TEMPOCH_FFI_DIR}/include)
set(TEMPOCH_ARTIFACT_DIR  ${TEMPOCH_FFI_DIR}/target/release)

# Allow enabling Cargo features for tempoch-ffi (e.g., serde)
set(TEMPOCH_FFI_FEATURES "" CACHE STRING "Cargo features for tempoch-ffi (e.g., 'serde' or empty)")

# Normalize feature args
set(_TEMPOCH_FEATURES_ARGS "")
string(TOUPPER "${TEMPOCH_FFI_FEATURES}" _TEMPOCH_FEATURES_UP)
if("${_TEMPOCH_FEATURES_UP}" STREQUAL "OFF" OR "${_TEMPOCH_FEATURES_UP}" STREQUAL "FALSE" OR "${_TEMPOCH_FEATURES_UP}" STREQUAL "0")
    # No features
elseif("${_TEMPOCH_FEATURES_UP}" STREQUAL "ON")
    set(_TEMPOCH_FEATURES_ARGS --features serde)
elseif(NOT "${TEMPOCH_FFI_FEATURES}" STREQUAL "")
    set(_TEMPOCH_FEATURES_ARGS --features ${TEMPOCH_FFI_FEATURES})
endif()

# Platform-specific library paths
if(APPLE)
    set(TEMPOCH_LIBRARY_PATH ${TEMPOCH_ARTIFACT_DIR}/libtempoch_ffi.dylib)
elseif(WIN32)
    set(TEMPOCH_LIBRARY_PATH ${TEMPOCH_ARTIFACT_DIR}/tempoch_ffi.dll)
    set(TEMPOCH_IMPORT_LIBRARY ${TEMPOCH_ARTIFACT_DIR}/tempoch_ffi.dll.lib)
else()
    set(TEMPOCH_LIBRARY_PATH ${TEMPOCH_ARTIFACT_DIR}/libtempoch_ffi.so)
endif()

# Build tempoch-ffi via Cargo
add_custom_target(
    build_tempoch_ffi
    COMMAND ${CARGO_BIN} build --release ${_TEMPOCH_FEATURES_ARGS}
    WORKING_DIRECTORY ${TEMPOCH_FFI_DIR}
    BYPRODUCTS ${TEMPOCH_LIBRARY_PATH}
    COMMENT "Building tempoch-ffi library via Cargo"
    VERBATIM
)

# Import tempoch-ffi as a library target
add_library(tempoch_ffi SHARED IMPORTED GLOBAL)
set_target_properties(tempoch_ffi PROPERTIES IMPORTED_LOCATION ${TEMPOCH_LIBRARY_PATH})
if(WIN32)
    set_target_properties(tempoch_ffi PROPERTIES IMPORTED_IMPLIB ${TEMPOCH_IMPORT_LIBRARY})
endif()
add_dependencies(tempoch_ffi build_tempoch_ffi)

# Header-only C++ wrapper library
add_library(tempoch_cpp INTERFACE)
target_include_directories(tempoch_cpp INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${TEMPOCH_FFI_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(tempoch_cpp INTERFACE tempoch_ffi)
add_dependencies(tempoch_cpp build_tempoch_ffi)

# Set RPATH for runtime library location
if(APPLE)
    set(_tempoch_rpath "@loader_path/../tempoch/tempoch-ffi/target/release")
elseif(UNIX)
    set(_tempoch_rpath "$ORIGIN/../tempoch/tempoch-ffi/target/release")
endif()

# Only build tests, examples, and Google Test when standalone (not as subdirectory)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)

# Google Test integration
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Enable testing
enable_testing()

# Example
add_executable(time_example examples/time_example.cpp)
target_link_libraries(time_example PRIVATE tempoch_cpp)
if(DEFINED _tempoch_rpath)
    set_target_properties(time_example PROPERTIES
        BUILD_RPATH ${_tempoch_rpath}
        INSTALL_RPATH ${_tempoch_rpath}
    )
endif()

# Test executables with Google Test
set(TEST_SOURCES
    tests/main.cpp
    tests/test_time.cpp
    tests/test_period.cpp
)

add_executable(test_tempoch ${TEST_SOURCES})
target_link_libraries(test_tempoch PRIVATE tempoch_cpp GTest::gtest)
if(DEFINED _tempoch_rpath)
    set_target_properties(test_tempoch PROPERTIES
        BUILD_RPATH ${_tempoch_rpath}
        INSTALL_RPATH ${_tempoch_rpath}
    )
endif()

# Add tests to CTest
include(GoogleTest)
gtest_discover_tests(test_tempoch)

endif() # CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR

# Installation rules
install(DIRECTORY include/tempoch
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)

install(DIRECTORY ${TEMPOCH_FFI_INCLUDE_DIR}/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

# Install the tempoch_ffi shared library
install(FILES ${TEMPOCH_LIBRARY_PATH}
    DESTINATION lib
)

# Install CMake config files for find_package support
include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/tempoch_cppConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/tempoch_cppConfig.cmake
    INSTALL_DESTINATION lib/cmake/tempoch_cpp
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/tempoch_cppConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/tempoch_cppConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/tempoch_cppConfigVersion.cmake
    DESTINATION lib/cmake/tempoch_cpp
)

install(TARGETS tempoch_cpp
    EXPORT tempoch_cppTargets
    INCLUDES DESTINATION include
)

install(EXPORT tempoch_cppTargets
    FILE tempoch_cppTargets.cmake
    NAMESPACE tempoch::
    DESTINATION lib/cmake/tempoch_cpp
)
