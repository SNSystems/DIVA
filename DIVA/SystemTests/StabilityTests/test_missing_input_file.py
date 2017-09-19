expected = """\

ERR_FILE_NOT_FOUND: Unable to open file 'HelloWorld.o'.
"""


def test(diva):
    assert diva('HelloWorld.o', nonzero=True, getelfs=False) == (1, expected)
