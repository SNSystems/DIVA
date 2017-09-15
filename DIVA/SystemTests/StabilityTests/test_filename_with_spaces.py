def test(diva):
    # Pass as a list so the diva fixture doesn't try to split on the space
    assert diva(['Hello World.o'])
