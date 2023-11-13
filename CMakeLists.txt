# pylarid: Load Arrays of Imaging Data
# Copyright 2023 Jeffrey Michael Engelmann

cmake_minimum_required(VERSION 3.26 FATAL_ERROR)
project(pylarid LANGUAGES CXX VERSION 0.1)
set(CMAKE_CXX_STANDARD 20)

if(NOT PYLARID_VERSION_STRING)
    set(PYLARID_VERSION_STRING "0.1.dev0")
endif()

find_package(Python COMPONENTS Interpreter Development.Module REQUIRED)

Python_add_library(pylarid MODULE "src/pylarid.cpp" WITH_SOABI)

if(MSVC)
    target_compile_options(pylarid PRIVATE /W4)
else()
    target_compile_options(pylarid PRIVATE -Wall)
endif()

target_compile_definitions(pylarid PRIVATE
        "PYLARID_VER=\"${PYLARID_VERSION_STRING}\"")

if(SKBUILD_PROJECT_NAME)
    install(TARGETS pylarid LIBRARY DESTINATION .)
else()
    if(NOT PYLARID_DEST)
        set(PYLARID_DEST "${Python_SITEARCH}")
    endif()

    # Install target
    install(TARGETS pylarid LIBRARY DESTINATION "${PYLARID_DEST}")

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
endif()

###############################################################################
