expected = """\
           {InputFile} "example_01.o"

             {CompileUnit} "example_01.cpp"

  {Source} "example_01.cpp"
     2         {Function} "foo" -> "void"
                   - No declaration
     2           {Parameter} "c" -> "char"
     4           {Variable} "i" -> "int"

     -------------------------------------
     Object                 Total  Printed
     -------------------------------------
     Alias                      0        0
     Block                      0        0
     Class                      0        0
     CodeLine                   4        0
     CompileUnit                1        1
     Enum                       0        0
     Function                   1        1
     Member                     0        0
     Namespace                  0        0
     Parameter                  1        1
     PrimitiveType              2        0
     Struct                     0        0
     TemplateParameter          0        0
     Union                      0        0
     Using                      0        0
     Variable                   1        1
     -------------------------------------
     Totals                    10        4

"""


def test(diva):
    assert diva('--show-summary example_01.o') == expected
