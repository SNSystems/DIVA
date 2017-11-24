expected = """\
                           {InputFile} "example_01.o"
[DW_TAG_compile_unit]         {CompileUnit} "example_01.cpp"

                           {Source} "example_01.cpp"
[DW_TAG_subprogram]        2    {Function} "foo" -> "void"
                                    - No declaration
[DW_TAG_formal_parameter]  2      {Parameter} "c" -> "char"
[DW_TAG_variable]          4      {Variable} "i" -> "int"
"""


def test(diva):
    assert diva('--show-DWARF-tag example_01.o') == expected
