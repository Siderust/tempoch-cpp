cmake_minimum_required(VERSION 3.15)
project(tempoch_cpp VERSION 0.5.3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
option(TEMPOCH_BUILD_DOCS "Enable Doxygen documentation target." ON)
option(TEMPOCH_USE_CANONICAL_RUST
       "Build/link against ../../../rust/tempoch instead of the vendored snapshot."
       OFF)

# Find Cargo for building Rust library
find_program(CARGO_BIN cargo REQUIRED)

# Paths to tempoch-ffi
if(TEMPOCH_USE_CANONICAL_RUST)
    set(TEMPOCH_CANONICAL_RUST_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../rust/tempoch
        CACHE PATH "Path to the canonical rust/tempoch workspace")
    set(TEMPOCH_SUBMODULE_DIR ${TEMPOCH_CANONICAL_RUST_DIR})
    set(TEMPOCH_ARTIFACT_DIR  ${TEMPOCH_CANONICAL_RUST_DIR}/target/release)
    set(TEMPOCH_TARGET_DIR    ${TEMPOCH_CANONICAL_RUST_DIR}/target)
else()
    set(TEMPOCH_SUBMODULE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tempoch)
    set(TEMPOCH_ARTIFACT_DIR  ${TEMPOCH_SUBMODULE_DIR}/tempoch-ffi/target/release)
    set(TEMPOCH_TARGET_DIR    ${TEMPOCH_SUBMODULE_DIR}/tempoch-ffi/target)
endif()
set(TEMPOCH_FFI_DIR         ${TEMPOCH_SUBMODULE_DIR}/tempoch-ffi)
set(TEMPOCH_FFI_INCLUDE_DIR ${TEMPOCH_FFI_DIR}/include)

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
    COMMAND ${CMAKE_COMMAND} -E env
            CARGO_TARGET_DIR=${TEMPOCH_TARGET_DIR}
            ${CARGO_BIN} build --release ${_TEMPOCH_FEATURES_ARGS}
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

# qtty-cpp integration (provides unit types for duration<>)
if(NOT TARGET qtty_cpp)
    set(QTTY_BUILD_DOCS OFF CACHE BOOL "Disable qtty-cpp docs when vendored by tempoch-cpp" FORCE)
    add_subdirectory(qtty-cpp)
endif()

# Header-only C++ wrapper library
add_library(tempoch_cpp INTERFACE)
target_include_directories(tempoch_cpp INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${TEMPOCH_FFI_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(tempoch_cpp INTERFACE
    tempoch_ffi
    $<BUILD_INTERFACE:qtty_cpp>
    $<INSTALL_INTERFACE:qtty::qtty_cpp>
)
add_dependencies(tempoch_cpp build_tempoch_ffi)

# Doxygen documentation
if(TEMPOCH_BUILD_DOCS)
    find_package(Doxygen QUIET)
    if(DOXYGEN_FOUND)
        set(TEMPOCH_DOXYFILE_IN  ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
        set(TEMPOCH_DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.tempoch_cpp)
        configure_file(${TEMPOCH_DOXYFILE_IN} ${TEMPOCH_DOXYFILE_OUT} @ONLY)

        add_custom_target(tempoch_docs
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/docs/doxygen
            COMMAND ${DOXYGEN_EXECUTABLE} ${TEMPOCH_DOXYFILE_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM
        )

        if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
            if(NOT TARGET docs)
                add_custom_target(docs DEPENDS tempoch_docs)
            else()
                message(STATUS "Top-level 'docs' target already exists; skipping creation to avoid conflict with subprojects")
            endif()
        endif()
    else()
        message(STATUS "Doxygen not found; 'docs' target will not be available")
    endif()
endif()

# Set RPATH for runtime library location
if(APPLE)
    if(TEMPOCH_USE_CANONICAL_RUST)
        set(_tempoch_rpath "${TEMPOCH_ARTIFACT_DIR}")
    else()
        set(_tempoch_rpath "@loader_path/../tempoch/tempoch-ffi/target/release")
    endif()
elseif(UNIX)
    if(TEMPOCH_USE_CANONICAL_RUST)
        set(_tempoch_rpath "${TEMPOCH_ARTIFACT_DIR}")
    else()
        set(_tempoch_rpath "$ORIGIN/../tempoch/tempoch-ffi/target/release")
    endif()
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
# Prevent GTest/GMock install rules from appearing in cmake --install / CPack.
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(INSTALL_GMOCK OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Enable testing
enable_testing()

# Examples — numbered to mirror the tempoch Rust crate examples.
foreach(_ex
    01_quickstart   # Quick start: UTC → TT, JD, MJD
    02_scales       # All scale conversions from J2000.0 TT
    03_formats      # Constructor / accessor showcase for Time<S>
    04_periods      # Period list operations: complement, intersect, normalize
    06_runtime_tables # EOP table access, UT1 conversion, constants
    07_conversions  # Full chain: Unix → UTC → TAI → TT → TDB / UT1 / GPS
    08_gnss_scales  # GNSS and SPICE-compatibility scales (GPST, GST, QZSST, BDT, ET)
    09_gnss_week    # GNSS week-number decomposition (GnssWeek)
)
    add_executable(${_ex} examples/${_ex}.cpp)
    target_link_libraries(${_ex} PRIVATE tempoch_cpp)
    if(DEFINED _tempoch_rpath)
        set_target_properties(${_ex} PROPERTIES
            BUILD_RPATH ${_tempoch_rpath}
            INSTALL_RPATH ${_tempoch_rpath}
        )
    endif()
endforeach()

# Test executables with Google Test
set(TEST_SOURCES
    tests/main.cpp
    tests/test_headers.cpp
    tests/test_time.cpp
    tests/test_period.cpp
    tests/test_new_scales.cpp
    tests/test_constants.cpp
    tests/test_data_status.cpp
    tests/test_gnss_week.cpp
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
gtest_discover_tests(test_tempoch
    PROPERTIES LABELS "tempoch_cpp"
)

endif() # CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR

# ---------------------------------------------------------------------------
# Installation and Packaging (CPack — DEB and RPM)
# Only active when tempoch-cpp is the top-level project.  When used as a
# subdirectory the install rules are intentionally suppressed so that
# parent packages (siderust-cpp) do not accidentally include tempoch-cpp
# artifacts and create file conflicts on the target system.
# ---------------------------------------------------------------------------
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)

    install(DIRECTORY include/tempoch
        DESTINATION include
        COMPONENT tempoch-cpp
        FILES_MATCHING PATTERN "*.hpp"
    )

    install(DIRECTORY ${TEMPOCH_FFI_INCLUDE_DIR}/
        DESTINATION include
        COMPONENT tempoch-cpp
        FILES_MATCHING PATTERN "*.h"
    )

    install(FILES ${TEMPOCH_LIBRARY_PATH}
        DESTINATION lib
        COMPONENT tempoch-cpp
    )

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
        COMPONENT tempoch-cpp
    )

    install(TARGETS tempoch_cpp
        EXPORT tempoch_cppTargets
        INCLUDES DESTINATION include
    )

    install(EXPORT tempoch_cppTargets
        FILE tempoch_cppTargets.cmake
        NAMESPACE tempoch::
        DESTINATION lib/cmake/tempoch_cpp
        COMPONENT tempoch-cpp
    )

    set(CPACK_PACKAGE_NAME             "tempoch-cpp")
    set(CPACK_PACKAGE_VERSION          "${PROJECT_VERSION}")
    set(CPACK_PACKAGE_VENDOR           "Siderust")
    set(CPACK_PACKAGE_CONTACT          "VPRamon <vallespuigramon@gmail.com>")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
        "C++ wrapper for the tempoch astronomical time library")
    set(CPACK_PACKAGE_DESCRIPTION
        "tempoch-cpp provides a header-only C++17 API over the Rust tempoch\n"
        "library via the tempoch-ffi C ABI. It bundles the public C++ headers,\n"
        "the tempoch_ffi shared library, and CMake package configuration files.")
    set(CPACK_RESOURCE_FILE_LICENSE    "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
    set(CPACK_PACKAGE_HOMEPAGE_URL     "https://github.com/Siderust/tempoch-cpp")

    set(CPACK_PACKAGE_VERSION_MAJOR    "${PROJECT_VERSION_MAJOR}")
    set(CPACK_PACKAGE_VERSION_MINOR    "${PROJECT_VERSION_MINOR}")
    set(CPACK_PACKAGE_VERSION_PATCH    "${PROJECT_VERSION_PATCH}")

    set(CPACK_COMPONENTS_ALL "tempoch-cpp")

    # -- DEB -----------------------------------------------------------------------
    set(CPACK_DEBIAN_PACKAGE_NAME        "tempoch-cpp")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER  "VPRamon <vallespuigramon@gmail.com>")
    set(CPACK_DEBIAN_PACKAGE_SECTION     "libs")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS     "libc6 (>= 2.17), libstdc++6 (>= 9), qtty-cpp (>= 0.4.2)")
    set(CPACK_DEBIAN_FILE_NAME           DEB-DEFAULT)

    # -- RPM -----------------------------------------------------------------------
    set(CPACK_RPM_PACKAGE_NAME        "tempoch-cpp")
    set(CPACK_RPM_PACKAGE_LICENSE     "AGPL-3.0")
    set(CPACK_RPM_PACKAGE_GROUP       "Development/Libraries")
    set(CPACK_RPM_PACKAGE_REQUIRES    "glibc >= 2.17, libstdc++ >= 9, qtty-cpp >= 0.4.2")
    set(CPACK_RPM_FILE_NAME           RPM-DEFAULT)

    set(CPACK_GENERATOR "DEB;RPM")

    include(CPack)

endif() # CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR
