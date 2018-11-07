if (TARGET Zycore)
    return()
endif ()

cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
include(GenerateExportHeader)
include(GNUInstallDirs)

project(Zycore VERSION 1.0.0.0 LANGUAGES C CXX)

# =============================================================================================== #
# Overridable options                                                                             #
# =============================================================================================== #

# Global configuration ()
option(ZYAN_WHOLE_PROGRAM_OPTIMIZATION
    "Enable whole program optimization (all targets)"
    OFF)
option(ZYAN_NO_LIBC
    "Don't use any C standard library functions (for exotic build-envs like kernel drivers)"
    OFF)
option(ZYAN_DEV_MODE
    "Enable developer mode (-Wall, -Werror, ...)"
    OFF)

# Build configuration
option(ZYCORE_BUILD_SHARED_LIB
    "Build shared library"
    OFF)
option(ZYCORE_BUILD_EXAMPLES
    "Build examples"
    OFF)
option(ZYCORE_BUILD_TESTS
    "Build tests"
    OFF)

# =============================================================================================== #
# GoogleTest                                                                                      #
# =============================================================================================== #

# Download and unpack googletest
configure_file("CMakeLists.txt.in" "${CMAKE_BINARY_DIR}/gtest/download/CMakeLists.txt")
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
    RESULT_VARIABLE result
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/gtest/download")
if (result)
    message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
    RESULT_VARIABLE result
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/gtest/download")
if (result)
    message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

if (ZYCORE_BUILD_TESTS)    
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    add_subdirectory("${CMAKE_BINARY_DIR}/gtest/src" "${CMAKE_BINARY_DIR}/gtest/build"
        EXCLUDE_FROM_ALL)

    if (CMAKE_VERSION VERSION_LESS 2.8.11)
        include_directories("${gtest_SOURCE_DIR}/include")
    endif()
endif ()

# =============================================================================================== #
# Developer mode                                                                                  #
# =============================================================================================== #

# If in developer mode, hack global compiler flags.
if (ZYAN_DEV_MODE)
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    if ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU" OR
            "${CMAKE_C_COMPILER_ID}" STREQUAL "Clang" OR
            "${CMAKE_C_COMPILER_ID}" STREQUAL "AppleClang")
        set(compiler_specific "-pedantic -Wextra -Werror")
    elseif (MSVC)
        set(compiler_specific "/WX /W4 /TC")
    endif ()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${compiler_specific}"
        CACHE STRING "Flags used by the compiler during all build types." FORCE)
endif ()

# =============================================================================================== #
# Exported functions                                                                              #
# =============================================================================================== #

function (zyan_set_common_flags target)
    if ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU" OR
        "${CMAKE_C_COMPILER_ID}" STREQUAL "Clang" OR
        "${CMAKE_C_COMPILER_ID}" STREQUAL "AppleClang")
        target_compile_options("${target}" PRIVATE "-std=c99")
    endif ()
endfunction ()

function (zyan_maybe_enable_wpo target)
    if (ZYAN_WHOLE_PROGRAM_OPTIMIZATION AND MSVC)
        set_target_properties("${target}" PROPERTIES COMPILE_FLAGS "/GL")
        set_target_properties("${target}" PROPERTIES LINK_FLAGS_RELEASE "/LTCG")
    endif ()
endfunction ()

function (zyan_maybe_enable_wpo_for_lib target)
    if (ZYAN_WHOLE_PROGRAM_OPTIMIZATION AND MSVC)
        set_target_properties("${target}" PROPERTIES COMPILE_FLAGS "/GL")
        set_target_properties("${target}" PROPERTIES LINK_FLAGS_RELEASE "/LTCG")
        set_target_properties("${target}" PROPERTIES STATIC_LIBRARY_FLAGS_RELEASE "/LTCG")
    endif ()
endfunction ()

# =============================================================================================== #
# Library configuration                                                                           #
# =============================================================================================== #

if (ZYCORE_BUILD_SHARED_LIB)
    add_library("Zycore" SHARED)
else ()
    add_library("Zycore" STATIC)
endif ()

zyan_set_common_flags("Zycore")
set_target_properties("Zycore" PROPERTIES LINKER_LANGUAGE C)
target_include_directories("Zycore"
    PUBLIC "include" ${PROJECT_BINARY_DIR}
    PRIVATE "src")
target_compile_definitions("Zycore" PRIVATE "_CRT_SECURE_NO_WARNINGS" "ZYCORE_EXPORTS")
zyan_maybe_enable_wpo_for_lib("Zycore")
generate_export_header("Zycore" BASE_NAME "ZYCORE" EXPORT_FILE_NAME "ZycoreExportConfig.h")

if (ZYAN_NO_LIBC)
    target_compile_definitions("Zycore" PUBLIC "ZYAN_NO_LIBC")
endif ()

target_sources("Zycore"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/include/Zycore/Allocator.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/Zycore/Bitset.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/Zycore/Comparison.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/Zycore/Defines.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/Zycore/Format.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/Zycore/LibC.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/Zycore/Object.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/Zycore/Status.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/Zycore/String.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/Zycore/Terminal.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/Zycore/Types.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/Zycore/Vector.h"
        "src/Allocator.c"
        "src/Bitset.c"
        "src/Format.c"
        "src/String.c"
        "src/Terminal.c"
        "src/Vector.c")

if (ZYCORE_BUILD_SHARED_LIB AND WIN32)
    target_sources("Zycore" PRIVATE "src/VersionInfo.rc")
endif ()

# TODO: Install CMake config.
install(TARGETS "Zycore"
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES
    "${PROJECT_BINARY_DIR}/ZycoreExportConfig.h"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(DIRECTORY "include/" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# =============================================================================================== #
# Examples                                                                                        #
# =============================================================================================== #

if (ZYCORE_BUILD_EXAMPLES)
    add_executable("String" "examples/String.c")
    zyan_set_common_flags("String" "Zycore")
    target_link_libraries("String" "Zycore")
    set_target_properties("String" PROPERTIES FOLDER "Examples")
    target_compile_definitions("String" PRIVATE "_CRT_SECURE_NO_WARNINGS")
    zyan_maybe_enable_wpo("String")

    add_executable("Vector" "examples/Vector.c")
    zyan_set_common_flags("Vector" "Zycore")
    target_link_libraries("Vector" "Zycore")
    set_target_properties("Vector" PROPERTIES FOLDER "Examples")
    target_compile_definitions("Vector" PRIVATE "_CRT_SECURE_NO_WARNINGS")
    zyan_maybe_enable_wpo("Vector")
endif ()

# =============================================================================================== #
# Tests                                                                                           #
# =============================================================================================== #

if (ZYCORE_BUILD_TESTS)
    add_executable("TestString" "tests/String.cpp")
    zyan_set_common_flags("TestString" "Zycore")
    target_link_libraries("TestString" "Zycore")
    target_link_libraries("TestString" "gtest")
    set_target_properties("TestString" PROPERTIES FOLDER "Tests")
    target_compile_definitions("TestString" PRIVATE "_CRT_SECURE_NO_WARNINGS")
    zyan_maybe_enable_wpo("TestString")

    add_executable("TestVector" "tests/Vector.cpp")
    zyan_set_common_flags("TestVector" "Zycore")
    target_link_libraries("TestVector" "Zycore")
    target_link_libraries("TestVector" "gtest")
    set_target_properties("TestVector" PROPERTIES FOLDER "Tests")
    target_compile_definitions("TestVector" PRIVATE "_CRT_SECURE_NO_WARNINGS")
    zyan_maybe_enable_wpo("TestVector")
endif ()
