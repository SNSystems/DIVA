expected = """\
           {InputFile} "scopes_org.o"

             {CompileUnit} "scopes.cpp"

  {Source} "scopes.cpp"
     3         {Alias} "INT" -> "int"
     6         {Function} "foo" -> "void"
                   - No declaration
    11           {Variable} "a" -> "INT"

"""


def test(diva):
    assert diva('scopes_org.o') == expected
