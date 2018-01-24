expected_txt = """\
{InputFile} "parameter.o"
   {CompileUnit} "parameter.cpp"
     {PrimitiveType} -> "char"
         - 1 bytes
     {PrimitiveType} -> "int"
         - 4 bytes
     {PrimitiveType} -> "unsigned int"
         - 4 bytes

{Source} "parameter.cpp"
3    {Function} "func1" -> "void"
         - No declaration
4    {Function} "func2" -> "void"
         - No declaration
4      {Parameter} "x" -> "int"
5    {Function} "func3" -> "void"
         - No declaration
5      {Parameter} "x" -> "int"
5      {Parameter} "y" -> "char"
5      {Parameter} "z" -> "unsigned int"
"""


def test_txt(diva):
    assert diva('parameter.o --show-all --output=text') == expected_txt


expected_yaml = """\
input_file: "parameter.o"
output_version: "0.1"
objects:
  - object: "CompileUnit"
    name: "parameter.cpp"
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
        type: "char"
        source:
          line: null
          file: null
        dwarf:
          offset: 0xc5
          tag: "DW_TAG_base_type"
        attributes:
          size: 1
        children: []
      - object: "PrimitiveType"
        name: null
        type: "int"
        source:
          line: null
          file: null
        dwarf:
          offset: 0x78
          tag: "DW_TAG_base_type"
        attributes:
          size: 4
        children: []
      - object: "PrimitiveType"
        name: null
        type: "unsigned int"
        source:
          line: null
          file: null
        dwarf:
          offset: 0xcc
          tag: "DW_TAG_base_type"
        attributes:
          size: 4
        children: []
      - object: "Function"
        name: "func1"
        type: "void"
        source:
          line: 3
          file: "parameter.cpp"
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
        children: []
      - object: "Function"
        name: "func2"
        type: "void"
        source:
          line: 4
          file: "parameter.cpp"
        dwarf:
          offset: 0x4a
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
          - object: "Parameter"
            name: "x"
            type: "int"
            source:
              line: 4
              file: "parameter.cpp"
            dwarf:
              offset: 0x6b
              tag: "DW_TAG_formal_parameter"
            attributes: {}
            children: []
      - object: "Function"
        name: "func3"
        type: "void"
        source:
          line: 5
          file: "parameter.cpp"
        dwarf:
          offset: 0x7f
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
          - object: "Parameter"
            name: "x"
            type: "int"
            source:
              line: 5
              file: "parameter.cpp"
            dwarf:
              offset: 0xa0
              tag: "DW_TAG_formal_parameter"
            attributes: {}
            children: []
          - object: "Parameter"
            name: "y"
            type: "char"
            source:
              line: 5
              file: "parameter.cpp"
            dwarf:
              offset: 0xac
              tag: "DW_TAG_formal_parameter"
            attributes: {}
            children: []
          - object: "Parameter"
            name: "z"
            type: "unsigned int"
            source:
              line: 5
              file: "parameter.cpp"
            dwarf:
              offset: 0xb8
              tag: "DW_TAG_formal_parameter"
            attributes: {}
            children: []
      - object: "CodeLine"
        name: null
        type: null
        source:
          line: 3
          file: "parameter.cpp"
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
          line: 3
          file: "parameter.cpp"
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
          line: 4
          file: "parameter.cpp"
        dwarf:
          offset: 0x7
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
          line: 4
          file: "parameter.cpp"
        dwarf:
          offset: 0xe
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
          line: 5
          file: "parameter.cpp"
        dwarf:
          offset: 0x11
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
          line: 5
          file: "parameter.cpp"
        dwarf:
          offset: 0x20
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
          line: 5
          file: "parameter.cpp"
        dwarf:
          offset: 0x23
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
    assert diva('parameter.o --output=yaml') == expected_yaml
