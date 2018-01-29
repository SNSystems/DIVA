expected_txt = """\
{InputFile} "variable.o"
   {CompileUnit} "variable.cpp"
     {PrimitiveType} -> "int"
         - 4 bytes

{Source} "variable.cpp"
3    {Variable} "g" -> "int"
5    {Function} "test" -> "void"
         - No declaration
6      {Variable} "x" -> "int"
"""


def test_txt(diva):
    assert diva('variable.o --show-all --output=text') == expected_txt


expected_yaml = """\
input_file: "variable.o"
output_version: "0.1"
objects:
  - object: "CompileUnit"
    name: "variable.cpp"
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
          offset: 0x5b
          tag: "DW_TAG_base_type"
        attributes:
          size: 4
        children: []
      - object: "Variable"
        name: "g"
        type: "int"
        source:
          line: 3
          file: "variable.cpp"
        dwarf:
          offset: 0x62
          tag: "DW_TAG_variable"
        attributes: {}
        children: []
      - object: "Function"
        name: "test"
        type: "void"
        source:
          line: 5
          file: "variable.cpp"
        dwarf:
          offset: 0x2d
          tag: "DW_TAG_subprogram"
        attributes:
          declaration:
            file: null
            line: null
          is_template: false
          static: false
          inline: false
          is_inlined: false
          is_declaration: false
        children:
          - object: "Variable"
            name: "x"
            type: "int"
            source:
              line: 6
              file: "variable.cpp"
            dwarf:
              offset: 0x4e
              tag: "DW_TAG_variable"
            attributes: {}
            children: []
      - object: "CodeLine"
        name: null
        type: null
        source:
          line: 5
          file: "variable.cpp"
        dwarf:
          offset: null
          tag: null
        attributes:
          Address: 0x0
          Discriminator: 0
          NewStatement: true
          PrologueEnd: false
          EndSequence: false
          BasicBlock: false
          EpilogueBegin: false
        children: []
      - object: "CodeLine"
        name: null
        type: null
        source:
          line: 6
          file: "variable.cpp"
        dwarf:
          offset: null
          tag: null
        attributes:
          Address: 0x4
          Discriminator: 0
          NewStatement: true
          PrologueEnd: false
          EndSequence: false
          BasicBlock: false
          EpilogueBegin: false
        children: []
      - object: "CodeLine"
        name: null
        type: null
        source:
          line: 7
          file: "variable.cpp"
        dwarf:
          offset: null
          tag: null
        attributes:
          Address: 0xb
          Discriminator: 0
          NewStatement: true
          PrologueEnd: false
          EndSequence: false
          BasicBlock: false
          EpilogueBegin: false
        children: []
      - object: "CodeLine"
        name: null
        type: null
        source:
          line: 7
          file: "variable.cpp"
        dwarf:
          offset: null
          tag: null
        attributes:
          Address: 0xe
          Discriminator: 0
          NewStatement: true
          PrologueEnd: false
          EndSequence: true
          BasicBlock: false
          EpilogueBegin: false
        children: []
"""


def test_yaml(diva):
    assert diva('variable.o --output=yaml') == expected_yaml
