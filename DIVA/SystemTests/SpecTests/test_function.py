expected_txt = """\
{InputFile} "function.o"
    {CompileUnit} "function.cpp"
      {PrimitiveType} -> "int"
          - 4 bytes

{Source} "function.cpp"
 3    {Function} "func1" -> "int"
          - No declaration
 3      {Parameter} "x" -> "int"
 5    {Function} static "func2" -> "int"
          - No declaration
 5      {Parameter} "x" -> "int"
 7    {Function} inline "func3" -> "int"
          - No declaration
 7      {Parameter} "x" -> "int"
11    {Struct} "S"
12      {Function} "S::method" -> "int"
            - Is declaration
          {Parameter} "" -> "S *"
          {Parameter} "" -> "int"
          {Parameter} "" -> "int"
12    {Function} "method" -> "int"
          - Declaration @ function.cpp,12
        {Parameter} "this" -> "const S *"
15      {Parameter} "x" -> "int"
15      {Parameter} "y" -> "int"
19    {Function} "test" -> "void"
          - No declaration
"""


def test_txt(diva):
    assert diva('function.o --show-all --output=text') == expected_txt


expected_yaml = """\
input_file: "function.o"
output_version: "0.1"
objects:
  - object: "CompileUnit"
    name: "function.cpp"
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
      - object: "Function"
        name: "func1"
        type: "int"
        source:
          line: 3
          file: "function.cpp"
        dwarf:
          offset: 0x86
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
              line: 3
              file: "function.cpp"
            dwarf:
              offset: 0xab
              tag: "DW_TAG_formal_parameter"
            attributes: {}
            children: []
      - object: "Function"
        name: "func2"
        type: "int"
        source:
          line: 5
          file: "function.cpp"
        dwarf:
          offset: 0xb8
          tag: "DW_TAG_subprogram"
        attributes:
          declaration:
            file: null
            line: null
          is_template: false
          static: true
          inline: false
          is_inlined: false
          is_declaration: false
        children:
          - object: "Parameter"
            name: "x"
            type: "int"
            source:
              line: 5
              file: "function.cpp"
            dwarf:
              offset: 0xd9
              tag: "DW_TAG_formal_parameter"
            attributes: {}
            children: []
      - object: "Function"
        name: "func3"
        type: "int"
        source:
          line: 7
          file: "function.cpp"
        dwarf:
          offset: 0x68
          tag: "DW_TAG_subprogram"
        attributes:
          declaration:
            file: null
            line: null
          is_template: false
          static: false
          inline: true
          is_inlined: false
          is_declaration: false
        children:
          - object: "Parameter"
            name: "x"
            type: "int"
            source:
              line: 7
              file: "function.cpp"
            dwarf:
              offset: 0x7c
              tag: "DW_TAG_formal_parameter"
            attributes: {}
            children: []
      - object: "Struct"
        name: "S"
        type: null
        source:
          line: 11
          file: "function.cpp"
        dwarf:
          offset: 0x2d
          tag: "DW_TAG_structure_type"
        attributes:
          is_template: false
          inherits_from: []
        children:
          - object: "Function"
            name: "S::method"
            type: "int"
            source:
              line: 12
              file: "function.cpp"
            dwarf:
              offset: 0x37
              tag: "DW_TAG_subprogram"
            attributes:
              declaration:
                file: null
                line: null
              is_template: false
              static: false
              inline: false
              is_inlined: false
              is_declaration: true
            children:
              - object: "Parameter"
                name: null
                type: "S *"
                source:
                  line: null
                  file: null
                dwarf:
                  offset: 0x4a
                  tag: "DW_TAG_formal_parameter"
                attributes: {}
                children: []
              - object: "Parameter"
                name: null
                type: "int"
                source:
                  line: null
                  file: null
                dwarf:
                  offset: 0x4f
                  tag: "DW_TAG_formal_parameter"
                attributes: {}
                children: []
              - object: "Parameter"
                name: null
                type: "int"
                source:
                  line: null
                  file: null
                dwarf:
                  offset: 0x54
                  tag: "DW_TAG_formal_parameter"
                attributes: {}
                children: []
      - object: "Function"
        name: "method"
        type: "int"
        source:
          line: 12
          file: "function.cpp"
        dwarf:
          offset: 0xe6
          tag: "DW_TAG_subprogram"
        attributes:
          declaration:
            file: "function.cpp"
            line: 12
          is_template: false
          static: false
          inline: false
          is_inlined: false
          is_declaration: false
        children:
          - object: "Parameter"
            name: "this"
            type: "const S *"
            source:
              line: null
              file: null
            dwarf:
              offset: 0x106
              tag: "DW_TAG_formal_parameter"
            attributes: {}
            children: []
          - object: "Parameter"
            name: "x"
            type: "int"
            source:
              line: 15
              file: "function.cpp"
            dwarf:
              offset: 0x112
              tag: "DW_TAG_formal_parameter"
            attributes: {}
            children: []
          - object: "Parameter"
            name: "y"
            type: "int"
            source:
              line: 15
              file: "function.cpp"
            dwarf:
              offset: 0x11e
              tag: "DW_TAG_formal_parameter"
            attributes: {}
            children: []
      - object: "Function"
        name: "test"
        type: "void"
        source:
          line: 19
          file: "function.cpp"
        dwarf:
          offset: 0x130
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
      - object: "CodeLine"
        name: null
        type: null
        source:
          line: 3
          file: "function.cpp"
        dwarf:
          offset: 0x0
          tag: null
        attributes:
          NewStatement: true
          PrologueEnd: false
          EndSequence: false
          BasicBlock: false
          Discriminator: true
          EpilogueBegin: false
        children: []
      - object: "CodeLine"
        name: null
        type: null
        source:
          line: 3
          file: "function.cpp"
        dwarf:
          offset: 0x7
          tag: null
        attributes:
          NewStatement: true
          PrologueEnd: false
          EndSequence: false
          BasicBlock: false
          Discriminator: true
          EpilogueBegin: false
        children: []
      - object: "CodeLine"
        name: null
        type: null
        source:
          line: 5
          file: "function.cpp"
        dwarf:
          offset: 0xc
          tag: null
        attributes:
          NewStatement: true
          PrologueEnd: false
          EndSequence: false
          BasicBlock: false
          Discriminator: true
          EpilogueBegin: false
        children: []
      - object: "CodeLine"
        name: null
        type: null
        source:
          line: 5
          file: "function.cpp"
        dwarf:
          offset: 0x13
          tag: null
        attributes:
          NewStatement: true
          PrologueEnd: false
          EndSequence: false
          BasicBlock: false
          Discriminator: true
          EpilogueBegin: false
        children: []
      - object: "CodeLine"
        name: null
        type: null
        source:
          line: 15
          file: "function.cpp"
        dwarf:
          offset: 0x18
          tag: null
        attributes:
          NewStatement: true
          PrologueEnd: false
          EndSequence: false
          BasicBlock: false
          Discriminator: true
          EpilogueBegin: false
        children: []
      - object: "CodeLine"
        name: null
        type: null
        source:
          line: 16
          file: "function.cpp"
        dwarf:
          offset: 0x26
          tag: null
        attributes:
          NewStatement: true
          PrologueEnd: false
          EndSequence: false
          BasicBlock: false
          Discriminator: true
          EpilogueBegin: false
        children: []
      - object: "CodeLine"
        name: null
        type: null
        source:
          line: 17
          file: "function.cpp"
        dwarf:
          offset: 0x2e
          tag: null
        attributes:
          NewStatement: true
          PrologueEnd: false
          EndSequence: false
          BasicBlock: false
          Discriminator: true
          EpilogueBegin: false
        children: []
      - object: "CodeLine"
        name: null
        type: null
        source:
          line: 19
          file: "function.cpp"
        dwarf:
          offset: 0x30
          tag: null
        attributes:
          NewStatement: true
          PrologueEnd: false
          EndSequence: false
          BasicBlock: false
          Discriminator: true
          EpilogueBegin: false
        children: []
      - object: "CodeLine"
        name: null
        type: null
        source:
          line: 20
          file: "function.cpp"
        dwarf:
          offset: 0x38
          tag: null
        attributes:
          NewStatement: true
          PrologueEnd: false
          EndSequence: false
          BasicBlock: false
          Discriminator: true
          EpilogueBegin: false
        children: []
      - object: "CodeLine"
        name: null
        type: null
        source:
          line: 22
          file: "function.cpp"
        dwarf:
          offset: 0x49
          tag: null
        attributes:
          NewStatement: true
          PrologueEnd: false
          EndSequence: false
          BasicBlock: false
          Discriminator: true
          EpilogueBegin: false
        children: []
      - object: "CodeLine"
        name: null
        type: null
        source:
          line: 22
          file: "function.cpp"
        dwarf:
          offset: 0x4c
          tag: null
        attributes:
          NewStatement: true
          PrologueEnd: false
          EndSequence: true
          BasicBlock: false
          Discriminator: true
          EpilogueBegin: false
        children: []
"""


def test_yaml(diva):
    assert diva('function.o --output=yaml') == expected_yaml
