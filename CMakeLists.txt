###############################################################################
#                                 _                  _       _                #
#                                | |                (_)     | |               #
#                 _ __    _   _  | |   __ _   _ __   _    __| |               #
#                | '_ \  | | | | | |  / _` | | '__| | |  / _` |               #
#                | |_) | | |_| | | | | (_| | | |    | | | (_| |               #
#                | .__/   \__, | |_|  \__,_| |_|    |_|  \__,_|               #
#                | |       __/ |                                              #
#                |_|      |___/                                               #
#                                                                             #
#                         Load Arrays of Imaging Data                         #
#                                                                             #
# Copyright (c) 2021, Jeffrey M. Engelmann                                    #
#                                                                             #
# pylarid is released under the revised (3-clause) BSD license.               #
# For details, see LICENSE.txt                                                #
#                                                                             #
###############################################################################

cmake_minimum_required(VERSION 3.17 FATAL_ERROR)

project(pylarid VERSION 0.1.0 LANGUAGES C)
set(CMAKE_C_STANDARD 11)

# Version string to be displayed by the extension module
if(NOT PYLARID_VERSION_STRING)
    set(PYLARID_VERSION_STRING "0.1.0.dev0")
endif()

# Require out-of-source builds
if( EXISTS "${PROJECT_BINARY_DIR}/CMakeLists.txt" OR
    EXISTS "${PROJECT_BINARY_DIR}/larid.c" OR
    EXISTS "${PROJECT_BINARY_DIR}/test_larid.py" OR
    EXISTS "${PROJECT_BINARY_DIR}/cmake_uninstall.cmake.in" OR
    EXISTS "${PROJECT_BINARY_DIR}/Findnifti1.cmake")
    message(FATAL_ERROR "Cannot build in a source directory")
endif()

# Set the build type (default is Release)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release"
        CACHE STRING "Choose the type of build" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Look for Python and NumPy
find_package(Python3 COMPONENTS Interpreter Development NumPy REQUIRED)

# Check Python version
if(Python3_VERSION VERSION_LESS 3.9.2)
    message(FATAL_ERROR
        "Python must be at least version 3.9.2 (found ${Python3_VERSION})")
endif()

# Check NumPy version
if(Python3_NumPy_VERSION VERSION_LESS 1.20.1)
    message(FATAL_ERROR
        "NumPy must be at least version 1.20.1 "
        "(found ${Python3_NumPy_VERSION})")
endif()

# Add NumPy include directories
list(APPEND larid_includes "${Python3_NumPy_INCLUDE_DIRS}")

# Look for NIfTI headers and add NIfTI include directories
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")
find_package(nifti1 REQUIRED)
find_package(nifti2 REQUIRED)
list(APPEND larid_includes "${NIFTI1_INCLUDE_DIRS}" "${NIFTI2_INCLUDE_DIRS}")

# Compile options
if(MSVC)
    list(APPEND larid_compile_options -W3)
else()
    list(APPEND larid_compile_options -Wall -Wextra)
endif()

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    list(APPEND larid_compile_options -Wno-cast-function-type)
endif()

# Preprocessor defines
list(APPEND larid_c_defines "PYLARID_VER=\"${PYLARID_VERSION_STRING}\"")

# Source code files
list(APPEND larid_sources
    "src/larid.c"
    "src/util.c"
    "src/dset.c"
    "src/datatype.c"
    "src/morder.c"
    "src/rescale.c"
    "src/io.c")

option(PYLARID_ZLIB "Build with zlib when possible" ON)
if(PYLARID_ZLIB)
    find_package(ZLIB)
    if(ZLIB_FOUND)
        list(APPEND larid_c_defines LARID_ZLIB)
        list(APPEND larid_private_includes "${ZLIB_INCLUDE_DIRS}")
        list(APPEND larid_link_libraries "${ZLIB_LIBRARIES}")
    endif()
endif()

option(PYLARID_INTRINSICS "Build with compiler intrinsics when possible" ON)
if(PYLARID_INTRINSICS)
    include(CheckCSourceRuns)

    # Look for byte-swapping functions
    check_c_source_runs(
        "int main(void) { return !__builtin_bswap16(0x0123) == 0x2310; }"
        HAVE__BUILTIN_BSWAP16)

    check_c_source_runs(
        "int main(void) { return !__builtin_bswap32(0x01234567) == 0x67452301; }"
        HAVE__BUILTIN_BSWAP32)

    check_c_source_runs(
        "int main(void) { return !__builtin_bswap64(0x0123456789abcdef) == 0xfecdab8967452301; }"
        HAVE__BUILTIN_BSWAP64)

    if(WIN32)
        check_c_source_runs(
            "int main(void) { return !_byteswap_ushort(0x0123) == 0x2310; }"
            HAVE__BYTESWAP_USHORT)

        check_c_source_runs(
            "int main(void) { return !_byteswap_ulong(0x01234567) == 0x67452301; }"
            HAVE__BYTESWAP_ULONG)

        check_c_source_runs(
            "int main(void) { return !_byteswap_uint64(0x0123456789abcdef) == 0xfecdab8967452301; }"
            HAVE__BYTESWAP_UINT64)
    endif(WIN32)

    # Prefer _builtin_swap; fallback to _byteswap; second fallback to none
    if(HAVE__BUILTIN_BSWAP16)
        list(APPEND larid_c_defines "HAVE__BUILTIN_BSWAP16")
    elseif(HAVE__BYTESWAP_USHORT)
        list(APPEND larid_c_defines "HAVE__BYTESWAP_USHORT")
    endif()

    if(HAVE__BUILTIN_BSWAP32)
        list(APPEND larid_c_defines "HAVE__BUILTIN_BSWAP32")
    elseif(HAVE__BYTESWAP_ULONG)
        list(APPEND larid_c_defines "HAVE__BYTESWAP_ULONG")
    endif()

    if(HAVE__BUILTIN_BSWAP64)
        list(APPEND larid_c_defines "HAVE__BUILTIN_BSWAP64")
    elseif(HAVE__BYTESWAP_UINT64)
        list(APPEND larid_c_defines "HAVE__BYTESWAP_UINT64")
    endif()
endif()

# Build the extension module
Python3_add_library(larid MODULE ${larid_sources})
set_target_properties(larid PROPERTIES C_VISIBILITY_PRESET hidden)
if(PYLARID_SETUP)
    set_target_properties(larid PROPERTIES
        OUTPUT_NAME "${LARID_OUTPUT_NAME}"
        SUFFIX "")
else()
    set_target_properties(larid PROPERTIES
        OUTPUT_NAME "larid.${Python3_SOABI}")
endif()
target_compile_options(larid PRIVATE ${larid_compile_options})
target_compile_definitions(larid PRIVATE ${larid_c_defines})
target_include_directories(larid PRIVATE ${larid_includes})
target_link_libraries(larid PRIVATE ${larid_link_libraries})

if(NOT PYLARID_SETUP)
    # Install target
    if(NOT PYLARID_DEST)
        set(PYLARID_DEST "${Python3_SITEARCH}")
    endif()
    install(TARGETS larid LIBRARY DESTINATION "${PYLARID_DEST}")

    # Uninstall target
    if(NOT TARGET uninstall)
        configure_file(
            "${CMAKE_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
            "${CMAKE_BINARY_DIR}/cmake_uninstall.cmake"
            IMMEDIATE @ONLY)
        add_custom_target(uninstall
            COMMAND "${CMAKE_COMMAND}" -P
            "${CMAKE_BINARY_DIR}/cmake_uninstall.cmake")
    endif()

    # Build test runner?
    option(PYLARID_TEST_RUNNER "Build test runner" OFF)
    if(PYLARID_TEST_RUNNER)
        add_executable(run "debug/run.c")
        target_include_directories(run PRIVATE "${Python3_INCLUDE_DIRS}")
        target_link_libraries(run PRIVATE "${Python3_LIBRARIES}")
    endif()
endif()

###############################################################################
