expected = ''


def test(diva, tmpdir_autodel):
    assert diva(
        'example_10.elf --output-dir={} --output=yaml'.format(tmpdir_autodel)
    ) == expected
    assert tmpdir_autodel.join('example_10_main_cpp.yaml').check()
    assert tmpdir_autodel.join('example_10_lib_cpp.yaml').check()
