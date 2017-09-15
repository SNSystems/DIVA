expected_txt = """\
           {InputFile} "compile_unit.o"

             {CompileUnit} "compile_unit.cpp"

"""


def test_txt(diva):
    assert diva('compile_unit.o --show-all --output=text') == expected_txt


expected_yaml = """\
input_file: "compile_unit.o"
output_version: "0.1"
objects:
  - object: "CompileUnit"
    name: "compile_unit.cpp"
    type: null
    source:
      line: null
      file: null
    dwarf:
      offset: 0xb
      tag: "DW_TAG_compile_unit"
    attributes: {}
    children: []
"""


def test_yaml(diva):
    assert diva('compile_unit.o --output=yaml') == expected_yaml
