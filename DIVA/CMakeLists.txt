cmake_minimum_required(VERSION 3.2.2)
get_filename_component(CMAKE_MODULE_PATH "CMake" ABSOLUTE)

project(DIVA)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.8)
    message(STATUS "Adding -stdlib=libc++")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif ()

set(gtest_force_shared_crt ON CACHE BOOL
    "Use shared (DLL) run-time lib even when Google Test is built as static lib.")
add_subdirectory(ExternalDependencies/googletest)
set_target_properties(gmock PROPERTIES FOLDER GoogleTest)
set_target_properties(gmock_main PROPERTIES FOLDER GoogleTest)
set_target_properties(gtest PROPERTIES FOLDER GoogleTest)
set_target_properties(gtest_main PROPERTIES FOLDER GoogleTest)

option(STATIC_DWARF_LIBS "Link DWARF libraries statically" OFF)
option(CODE_COVERAGE "Enable code coverage" OFF)
option(CLANG_ASAN "Enable clang address sanitizer" OFF)
option(CLANG_MEMSAN "Enable clang memory sanitizer" OFF)
option(CLANG_UBSAN "Enable clang undefined behaviour sanitizer" OFF)

include(Utilities)
include(CompilerFlags)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

set(CMAKE_CXX_STANDARD "14")
set(CMAKE_CXX_STANDARD_REQUIRED "ON")

add_subdirectory(LibScopeView)
add_subdirectory(ElfDwarfReader)
add_subdirectory(Diva)
add_subdirectory(UnitTests)

