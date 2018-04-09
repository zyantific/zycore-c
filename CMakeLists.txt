cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
include(GenerateExportHeader)
include(GNUInstallDirs)

project(Zycore VERSION 1.0 LANGUAGES C)

# =============================================================================================== #
# Overridable options                                                                             #
# =============================================================================================== #

# Build configuration
option(ZYCORE_BUILD_SHARED_LIB
    "Build shared library"
    OFF)
option(ZYCORE_WHOLE_PROGRAM_OPTIMIZATION
    "Enable whole program optimization"
    OFF)
option(ZYCORE_NO_LIBC
    "Don't use any C standard library functions (for exotic build-envs like kernel drivers)"
    OFF)
#option(ZYCORE_BUILD_EXAMPLES
#    "Build examples"
#    ON)
option(ZYCORE_DEV_MODE
    "Enable developer mode (-Wall, -Werror, ...)"
    OFF)

# =============================================================================================== #
# Developer mode                                                                                  #
# =============================================================================================== #

# If in developer mode, hack global compiler flags.
if (ZYCORE_DEV_MODE)
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
# Library configuration                                                                           #
# =============================================================================================== #

if (ZYCORE_BUILD_SHARED_LIB)
    add_library("Zycore" SHARED)
else ()
    add_library("Zycore" STATIC)
endif ()

set_target_properties("Zycore" PROPERTIES LINKER_LANGUAGE C)
target_include_directories("Zycore"
    PUBLIC "include" ${PROJECT_BINARY_DIR}
    PRIVATE "src")
target_compile_definitions("Zycore" PRIVATE "_CRT_SECURE_NO_WARNINGS" "ZYCORE_EXPORTS")
if (ZYCORE_WHOLE_PROGRAM_OPTIMIZATION AND MSVC)
    set_target_properties("Zycore" PROPERTIES COMPILE_FLAGS "/GL")
    set_target_properties("Zycore" PROPERTIES LINK_FLAGS_RELEASE "/LTCG")
    set_target_properties("Zycore" PROPERTIES STATIC_LIBRARY_FLAGS_RELEASE "/LTCG")
endif ()
generate_export_header("Zycore" BASE_NAME "ZYCORE" EXPORT_FILE_NAME "ZycoreExportConfig.h")

if (ZYCORE_NO_LIBC)
    target_compile_definitions("Zycore" PUBLIC "ZYCORE_NO_LIBC")
endif ()

target_sources("Zycore"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/include/Zycore/LibC.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/Zycore/Types.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/Zycore/Defines.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/Zycore/Status.h")

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

#if (ZYCORE_BUILD_EXAMPLES)
#    add_executable("Vector" "examples/Vector.c")
#    target_link_libraries("Vector" "Zycore")
#    set_target_properties("Vector" PROPERTIES FOLDER "Examples")
#    target_compile_definitions("Vector" PRIVATE "_CRT_SECURE_NO_WARNINGS")
#    if (ZYCORE_WHOLE_PROGRAM_OPTIMIZATION AND MSVC)
#        set_target_properties("Vector" PROPERTIES COMPILE_FLAGS "/GL")
#        set_target_properties("Vector" PROPERTIES LINK_FLAGS_RELEASE "/LTCG")
#    endif ()
#endif ()
