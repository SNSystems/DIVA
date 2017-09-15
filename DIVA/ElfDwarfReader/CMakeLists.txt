# ElfDwarfReader

create_target(LIB ElfDwarfReader
    SOURCE
        "src/ElfDwarfReader.cpp"
        "src/LibDwarfHelpers.cpp"
    HEADERS
        "src/ElfDwarfReader.h"
        "src/LibDwarfHelpers.h"
    INCLUDE
        "../ExternalDependencies/boost/include/boost-1_62"
        "../ExternalDependencies/DwarfDump/Includes/LibDwarf"
        "../LibScopeView/src"
)
