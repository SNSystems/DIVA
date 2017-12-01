expected = """\
{InputFile} "helloworld.o"
   {CompileUnit} "helloworld.cpp"

{Source} "helloworld.cpp"
5    {Function} "main" -> "int"
         - No declaration
"""


def test(diva):
    assert diva('helloworld.o') == expected
