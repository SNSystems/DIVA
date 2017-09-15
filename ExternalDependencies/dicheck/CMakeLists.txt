# DiCheck

create_target(EXE DiCheck
    OUTPUT_NAME
        "dicheck"
    SOURCE
        "Src/Distribution/dicheck.cc"
    INCLUDE
        "../DwarfDump/Src/Getopt"
        "../LibDwarf/Src/Distribution"
)
