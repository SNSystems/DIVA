expected = """\
           {InputFile} "helloworld.o"

             {CompileUnit} "helloworld.cpp"
               {PrimitiveType} -> "int"
                   - 4 bytes

  {Source} "helloworld.cpp"
     5         {Function} "main" -> "int"
                   - No declaration

"""


def test(diva):
    assert diva('helloworld.o --show-all') == expected
