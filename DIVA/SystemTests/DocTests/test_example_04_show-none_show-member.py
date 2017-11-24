expected = """\
{InputFile} "example_04.o"
     {CompileUnit} "example_04.cpp"

{Source} "example_04.cpp"
  6      {Member} private "x" -> "int"
  6      {Member} private "y" -> "int"

{Source} "ios_base.h"
547          {Member} private "_S_refcount" -> "_Atomic_word"
548          {Member} private "_S_synced_with_stdio" -> "bool"
"""


def test(diva):
    assert diva('--show-none --show-member example_04.o') == expected
