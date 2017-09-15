include(CheckCXXCompilerFlag)

function(add_flag_if_exists flag_name)
    # Remove dashes from test variable name. The name of the variable is used
    # as a macro in the build input so it needs to be a c++ identifier to avoid
    # any errors.
    string(REPLACE "-" "_" flag_var_name ${flag_name})
    check_cxx_compiler_flag(${flag_name} "has_flag_${flag_var_name}")
    if("${has_flag_${flag_var_name}}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag_name}" PARENT_SCOPE)
    endif()
endfunction()

# GCC Flags

# Turn on gcc/clang warnings.
add_flag_if_exists(-Wall)
add_flag_if_exists(-Weverything)
add_flag_if_exists(-Wno-c++98-compat)
add_flag_if_exists(-Wno-c++98-compat-pedantic)
add_flag_if_exists(-Wno-c++14-binary-literal)
add_flag_if_exists(-Wno-padded)
add_flag_if_exists(-Wno-global-constructors)
add_flag_if_exists(-Wno-exit-time-destructors)

# Turn on warnings as errors
add_flag_if_exists(-Werror)

# Clang Flags
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    # Clang coverage.
    if (CODE_COVERAGE)
        message(STATUS "Enabling clang code coverage")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-instr-generate -fcoverage-mapping")
    endif()

    # Clang sanitizers.
    if (CLANG_ASAN)
        message(STATUS "Enabling clang address sanitizer")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
    endif()
    if (CLANG_MEMSAN)
        message(STATUS "Enabling clang memory sanitizer")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=memory")
    endif()
    if (CLANG_UBSAN)
        message(STATUS "Enabling clang undefined behaviour sanitizer")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined")
    endif()
endif()

# Visual Studio MSVC Flags

# Warnings as errors
add_flag_if_exists(/WX)

# Turn on production level warnings
add_flag_if_exists(/W3)
