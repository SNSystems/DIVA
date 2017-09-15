def test(diva, tmpdir_autodel):
    split_dir = tmpdir_autodel.join('example_16_CUs')
    assert not split_dir.check(dir=True)

    # Get normal output
    command = 'example_16.elf --show-all'
    expected = diva(command)

    # Split output
    assert diva(command + ' --output-dir={}'.format(split_dir)) == '\n'
    assert split_dir.check(dir=True)

    outfiles = (
        'example_16_cpp.txt',
        'example_16_global_cpp.txt',
        'example_16_local_cpp.txt'
    )
    for outfile in outfiles:
        outfile = split_dir.join(outfile)
        assert outfile.check(file=True)
        contents = outfile.read()
        assert contents in expected
        # Remove the matched content to ensure all three files are distinct
        expected = expected.replace(contents, '')
    # The only thing left should be the InputFile header
    assert expected.strip() == '{InputFile} "example_16.elf"'
