def test(diva, tmpdir_autodel):
    split_dir = tmpdir_autodel.join('example_16_CUs')
    assert not split_dir.check(dir=True)

    # Get normal output
    command = 'example_16.elf --show-all'
    expected = diva(command)

    # Split output
    assert diva(command + ' --output-dir={}'.format(split_dir)) == ''
    assert split_dir.check(dir=True)

    input_file_header = '{InputFile} "example_16.elf"'

    outfiles = (
        'example_16_cpp.txt',
        'example_16_global_cpp.txt',
        'example_16_local_cpp.txt'
    )
    for outfile in outfiles:
        outfile = split_dir.join(outfile)
        assert outfile.check(file=True)
        contents = outfile.read()

        # Each contents should start with the input file header, but expected
        # should only contain one
        assert contents.startswith(input_file_header + '\n')
        contents = contents[len(input_file_header) + 1:]

        if contents not in expected:
            print(contents)
            print("---")
            print(expected)
        assert contents in expected
        # Remove the matched content to ensure all three files are distinct
        expected = expected.replace(contents, '')
    # The only thing left should be the InputFile header
    assert expected.strip() == input_file_header
