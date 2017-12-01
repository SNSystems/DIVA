expected_1 = """\
{InputFile} "example_04.o"
     {CompileUnit} "example_04.cpp"
"""

expected_2 = """\
  8    {Function} static "" -> "void"
           - No declaration
"""


def test(diva):
    actual = diva('example_04.o --show-generated')
    assert expected_1 in actual
    assert expected_2 in actual
