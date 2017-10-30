# Test that elf files without debug lines don't cause a crash.

def test(diva):
    diva("no_lines.o")
