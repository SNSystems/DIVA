expected = ''


def test(diva, tmpdir_autodel):
    assert diva(
        'example_16.elf --output-dir={}'.format(tmpdir_autodel)
    ) == expected
    assert tmpdir_autodel.join('example_16_cpp.txt').check()
    assert tmpdir_autodel.join('example_16_global_cpp.txt').check()
    assert tmpdir_autodel.join('example_16_local_cpp.txt').check()
