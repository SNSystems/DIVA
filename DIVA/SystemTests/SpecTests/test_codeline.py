expected_txt = """\
{InputFile} "codeline.o"
    {CompileUnit} "codeline.cpp"
      {PrimitiveType} -> "int"
          - 4 bytes

{Source} "codeline.cpp"
 3    {Function} "test" -> "void"
          - No declaration
 4      {Variable} "x" -> "int"
 5      {Variable} "y" -> "int"
 6      {Variable} "z" -> "int"
 3    {CodeLine}
 8    {CodeLine}
 9    {CodeLine}
10    {CodeLine}
12    {CodeLine}
13    {CodeLine}
13    {CodeLine}
"""


def test_txt(diva):
    assert diva(
        'codeline.o --show-all --show-codeline --output=text') == expected_txt


expected_yaml = """\
input_file: "codeline.o"
output_version: "0.1"
objects:
  - object: "CompileUnit"
    name: "codeline.cpp"
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
          offset: 0x73
          tag: "DW_TAG_base_type"
        attributes:
          size: 4
        children: []
      - object: "Function"
        name: "test"
        type: "void"
        source:
          line: 3
          file: "codeline.cpp"
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
              line: 4
              file: "codeline.cpp"
            dwarf:
              offset: 0x4e
              tag: "DW_TAG_variable"
            attributes: {}
            children: []
          - object: "Variable"
            name: "y"
            type: "int"
            source:
              line: 5
              file: "codeline.cpp"
            dwarf:
              offset: 0x5a
              tag: "DW_TAG_variable"
            attributes: {}
            children: []
          - object: "Variable"
            name: "z"
            type: "int"
            source:
              line: 6
              file: "codeline.cpp"
            dwarf:
              offset: 0x66
              tag: "DW_TAG_variable"
            attributes: {}
            children: []
      - object: "CodeLine"
        name: null
        type: null
        source:
          line: 3
          file: "codeline.cpp"
        dwarf:
          offset: 0x0
          tag: null
        attributes:
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
          line: 8
          file: "codeline.cpp"
        dwarf:
          offset: 0x4
          tag: null
        attributes:
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
          line: 9
          file: "codeline.cpp"
        dwarf:
          offset: 0xb
          tag: null
        attributes:
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
          line: 10
          file: "codeline.cpp"
        dwarf:
          offset: 0x12
          tag: null
        attributes:
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
          line: 12
          file: "codeline.cpp"
        dwarf:
          offset: 0x19
          tag: null
        attributes:
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
          line: 13
          file: "codeline.cpp"
        dwarf:
          offset: 0x2b
          tag: null
        attributes:
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
          line: 13
          file: "codeline.cpp"
        dwarf:
          offset: 0x2e
          tag: null
        attributes:
          Discriminator: 0
          NewStatement: true
          PrologueEnd: false
          EndSequence: true
          BasicBlock: false
          EpilogueBegin: false
        children: []
"""


def test_yaml(diva):
    assert diva('codeline.o --output=yaml') == expected_yaml
