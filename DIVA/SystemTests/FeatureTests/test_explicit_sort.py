"""
Test that sort by line is the default.
"""


def test(diva):
    assert (
        diva('example_16.elf --show-all') ==
        diva('example_16.elf --show-all --sort=line')
    )
