expected = """\

ERR_INVALID_FILE: Invalid input file 'HelloWorld.cpp', please provide a file in a supported format.
"""


def test(diva):
    assert diva('HelloWorld.cpp', nonzero=True) == (1, expected)
