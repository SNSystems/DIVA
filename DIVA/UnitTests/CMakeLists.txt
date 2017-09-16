set(base_lib_dir "../ExternalDependencies/DwarfDump/Libraries")

# We can't fix the warnings in googletest.
check_cxx_compiler_flag(-Wno-everything has_Wno_everything)
if(has_Wno_everything)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-everything")
endif()

if (STATIC_DWARF_LIBS)
    set(static_lib_dir "${base_lib_dir}/${platform_name}_${architecture_name}_static")
    set(static_libs "LibDwarf" "LibElf" "LibTsearch" "LibZlib")
    link_directories("${static_lib_dir}")
else()
    set(debug_lib_dir "${base_lib_dir}/${platform_name}_${architecture_name}_debug")
    set(lib_dir "${base_lib_dir}/${platform_name}_${architecture_name}")
    link_directories("${lib_dir}" "${debug_lib_dir}")
endif()

if(WIN32)
    set(windows_libraries "Psapi")
endif()

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/TestInputs")
    message(FATAL_ERROR "TestInputs directory not found")
endif()

create_target(EXE UnitTests
    OUTPUT_NAME
        "unittests"
    SOURCE
        "src/main.cpp"
        "src/UtilsForTesting.cpp"
        "src/TestDiva/TestArgumentParser.cpp"
        "src/TestDiva/TestDivaOptions.cpp"
        "src/TestLibScopeView/TestFileUtilities.cpp"
        "src/TestLibScopeView/TestLine.cpp"
        "src/TestLibScopeView/TestObject.cpp"
        "src/TestLibScopeView/TestObjectAttributes.cpp"
        "src/TestLibScopeView/TestPrintSettings.cpp"
        "src/TestLibScopeView/TestScope.cpp"
        "src/TestLibScopeView/TestScopePrinter.cpp"
        "src/TestLibScopeView/TestScopeVisitor.cpp"
        "src/TestLibScopeView/TestScopeYAMLPrinter.cpp"
        "src/TestLibScopeView/TestSummaryTable.cpp"
        "src/TestLibScopeView/TestSymbol.cpp"
        "src/TestLibScopeView/TestType.cpp"
        "src/TestElfDwarfReader/TestElfDwarfReader.cpp"
        "src/TestElfDwarfReader/TestLibDwarfHelpers.cpp"
        # Source to be tested
        "../Diva/src/ArgumentParser.cpp"
        "../Diva/src/DivaOptions.cpp"
    HEADERS
        "src/UtilsForTesting.h"
    INCLUDE
        "src"
        "../ExternalDependencies/googletest/googletest/include"
        "../ExternalDependencies/googletest/googlemock/include"
        "../Diva/src"
        "../LibScopeView/src"
        "../ElfDwarfReader/src"
        "../ExternalDependencies/DwarfDump/Includes/LibDwarf"
    LINK
        "gtest"
        "gmock"
        "ElfDwarfReader"
        "LibScopeView"
        "${static_libs}"
        "${windows_libraries}"
        "${linux_libraries}"
    DEFINE
        "-DUNIT_TEST_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\""
        # Test version defines for DivaOptions.cpp
        "-DRC_VERSION_STR=\"TEST_VERSION_STR\""
        "-DRC_COMPANYNAME_STR=\"TEST_COMPANY_NAME\""
        "-DRC_COPYYEAR_STR=\"TEST_COPYRIGHT_YEAR\""
)

if (NOT STATIC_DWARF_LIBS)
    target_link_libraries(UnitTests debug "LibDwarf_debug")
    target_link_libraries(UnitTests debug "LibElf_debug")
    target_link_libraries(UnitTests debug "LibTsearch_debug")
    target_link_libraries(UnitTests debug "LibZlib_debug")

    target_link_libraries(UnitTests optimized "LibDwarf")
    target_link_libraries(UnitTests optimized "LibElf")
    target_link_libraries(UnitTests optimized "LibTsearch")
    target_link_libraries(UnitTests optimized "LibZlib")
endif()
