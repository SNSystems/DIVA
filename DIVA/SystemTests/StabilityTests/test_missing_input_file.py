expected = """\

ERR_VIEW_INVALID_OPEN: Unable to open file 'HelloWorld.o'.
"""


def test(diva):
    assert diva('HelloWorld.o', nonzero=True, getelfs=False) == (1, expected)
