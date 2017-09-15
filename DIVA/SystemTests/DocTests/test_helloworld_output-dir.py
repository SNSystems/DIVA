expected = '\n'


def test(diva, tmpdir_autodel):
    assert diva(
        'helloworld.o --output-dir={}'.format(tmpdir_autodel)
    ) == expected
    assert tmpdir_autodel.join('helloworld_cpp.txt').check()
