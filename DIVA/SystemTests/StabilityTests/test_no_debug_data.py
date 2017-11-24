# Compiled from HelloWorld.cpp with no debug info with clang
# clang -c HelloWorld.cpp -o nodebug.o

expected_txt = """\

Warning: No DWARF debug data found.
{InputFile} "nodebug.o"
"""

def test(diva):
    assert diva('nodebug.o') == expected_txt
