expected = """\
           {InputFile} "scopes_mod.o"

             {CompileUnit} "scopes.cpp"

  {Source} "scopes.cpp"
     6         {Function} "foo" -> "void"
                   - No declaration
     9           {Alias} "INT" -> "int"
    11           {Variable} "a" -> "INT"

"""


def test(diva):
    assert diva('scopes_mod.o') == expected
