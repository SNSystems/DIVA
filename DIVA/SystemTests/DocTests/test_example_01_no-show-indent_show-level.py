expected = """\
              {InputFile} "example_01.o"

000           {CompileUnit} "example_01.cpp"

     {Source} "example_01.cpp"
001     2     {Function} "foo" -> "void"
                  - No declaration
002     2     {Parameter} "c" -> "char"
002     4     {Variable} "i" -> "int"

"""


def test(diva):
    assert diva(
        '--no-show-indent --show-level example_01.o') == expected
