def test_output_dir(diva, tmpdir_autodel):
    output_dir = tmpdir_autodel.join('simple_split')
    diva('simple.o --output-dir={}'.format(output_dir))
    assert output_dir.check(dir=True)
    assert output_dir.join('simple_cpp.txt').check(file=True)


def test_output_dir_preexisting(diva, tmpdir_autodel):
    output_dir = tmpdir_autodel.join('already_exists')
    output_dir.ensure_dir()
    assert output_dir.check(dir=True)
    diva('simple.o --output-dir={}'.format(output_dir))
    assert output_dir.join('simple_cpp.txt').check(file=True)


expected = """\

ERR_VIEW_INVALID_OPEN: Unable to open file 'not_a_file.o'.
"""


def test_not_a_file(diva):
    assert diva('not_a_file.o', getelfs=False, nonzero=True) == (1, expected)
