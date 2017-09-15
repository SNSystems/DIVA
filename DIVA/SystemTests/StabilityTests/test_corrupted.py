"""
corrupted.o is HelloWorld.o with a random byte removed from the middle of the
file.
"""
expected_rel = """\

ERR_INVALID_DWARF: Failed to read DWARF from 'corrupted.o'
"""
expected_dbg = """\
DW_DLE_ELF_GETIDENT_ERROR (148)
ERR_INVALID_DWARF: Failed to read DWARF from 'corrupted.o'
"""


def test(diva):
    expected = expected_dbg if '(Debug)' in diva('--version') else expected_rel

    assert diva('corrupted.o', nonzero=True) == (1, expected)
