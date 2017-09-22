# repro8.o came from a crtbegin.o file as described in:
# https://github.com/SNSystems/DIVA/issues/8

expected = """\

Warning: No DWARF debug data found.
           {InputFile} "repro8.o"

"""


def test(diva):
    assert diva("repro8.o") == expected
