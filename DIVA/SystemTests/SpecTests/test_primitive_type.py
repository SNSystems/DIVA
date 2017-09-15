expected_txt = """\
           {InputFile} "primitive_type.o"

             {CompileUnit} "primitive_type.cpp"
               {PrimitiveType} -> "int"
                   - 4 bytes

  {Source} "primitive_type.cpp"
     3         {Variable} "g" -> "int"

"""


def test_txt(diva):
    assert diva('primitive_type.o --show-all --output=text') == expected_txt


expected_yaml = """\
input_file: "primitive_type.o"
output_version: "0.1"
objects:
  - object: "CompileUnit"
    name: "primitive_type.cpp"
    type: null
    source:
      line: null
      file: null
    dwarf:
      offset: 0xb
      tag: "DW_TAG_compile_unit"
    attributes: {}
    children:
      - object: "PrimitiveType"
        name: null
        type: "int"
        source:
          line: null
          file: null
        dwarf:
          offset: 0x30
          tag: "DW_TAG_base_type"
        attributes:
          size: 4
        children: []
      - object: "Variable"
        name: "g"
        type: "int"
        source:
          line: 3
          file: "primitive_type.cpp"
        dwarf:
          offset: 0x1d
          tag: "DW_TAG_variable"
        attributes: {}
        children: []
"""


def test_yaml(diva):
    assert diva('primitive_type.o --output=yaml') == expected_yaml
