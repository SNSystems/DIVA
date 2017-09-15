expected = """\

ERR_SPLIT_UNABLE_TO_OPEN_FILE: Unable to open file 'helloworld/example_01_cpp.txt' for Logical View Split.
"""


def test(tmpdir_autodel, diva):
    cwd = tmpdir_autodel.join('split_ro')

    file1 = cwd.join('helloworld').join('example_01_cpp.txt')
    file1.ensure(file=True)
    file1.chmod(0o444)

    file2 = cwd.join('helloworld_ext').join('example_01_cpp.txt')
    file2.ensure(file=True)
    file2.chmod(0o444)

    assert (
        diva('--output-dir=helloworld example_01.o', nonzero=True, cwd=cwd) ==
        (1, expected)
    )
