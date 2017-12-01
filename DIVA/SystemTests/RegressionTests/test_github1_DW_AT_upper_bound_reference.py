"""
See https://github.com/SNSystems/DIVA/issues/1
"""

expected = """\
{InputFile} "input.o"
   {CompileUnit} "input.c"

{Source} "input.c"
1    {Function} "test" -> "void"
         - No declaration
1      {Parameter} "n" -> "int"
2      {Variable} "vla" -> "int [?]"
"""


def test(diva):
    assert diva("input.o") == expected
