expected_txt = """\
{InputFile} "member.o"
    {CompileUnit} "member.cpp"
      {PrimitiveType} -> "int"
          - 4 bytes

{Source} "member.cpp"
 3    {Struct} "A"
        {Function} "A::A" -> "void"
            - Is declaration
          {Parameter} "" -> "A *"
 5      {Member} private "m_private" -> "int"
 7      {Member} public "m_public" -> "int"
 9      {Member} protected "m_protected" -> "int"
12    {Function} "test" -> "void"
          - No declaration
13      {Variable} "a" -> "A"
"""


def test_txt(diva):
    assert diva('member.o --show-all --output=text') == expected_txt


expected_yaml = """\
input_file: "member.o"
output_version: "0.1"
objects:
  - object: "CompileUnit"
    name: "member.cpp"
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
          offset: 0x6f
          tag: "DW_TAG_base_type"
        attributes:
          size: 4
        children: []
      - object: "Struct"
        name: "A"
        type: null
        source:
          line: 3
          file: "member.cpp"
        dwarf:
          offset: 0x2d
          tag: "DW_TAG_structure_type"
        attributes:
          is_template: false
          inherits_from: []
        children:
          - object: "Function"
            name: "A::A"
            type: "void"
            source:
              line: null
              file: null
            dwarf:
              offset: 0x5d
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
                type: "A *"
                source:
                  line: null
                  file: null
                dwarf:
                  offset: 0x68
                  tag: "DW_TAG_formal_parameter"
                attributes: {}
                children: []
          - object: "Member"
            name: "m_private"
            type: "int"
            source:
              line: 5
              file: "member.cpp"
            dwarf:
              offset: 0x37
              tag: "DW_TAG_member"
            attributes:
              access_specifier: "private"
            children: []
          - object: "Member"
            name: "m_public"
            type: "int"
            source:
              line: 7
              file: "member.cpp"
            dwarf:
              offset: 0x44
              tag: "DW_TAG_member"
            attributes:
              access_specifier: "public"
            children: []
          - object: "Member"
            name: "m_protected"
            type: "int"
            source:
              line: 9
              file: "member.cpp"
            dwarf:
              offset: 0x50
              tag: "DW_TAG_member"
            attributes:
              access_specifier: "protected"
            children: []
      - object: "Function"
        name: "test"
        type: "void"
        source:
          line: 12
          file: "member.cpp"
        dwarf:
          offset: 0x7c
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
            name: "a"
            type: "A"
            source:
              line: 13
              file: "member.cpp"
            dwarf:
              offset: 0x99
              tag: "DW_TAG_variable"
            attributes: {}
            children: []
      - object: "CodeLine"
        name: null
        type: null
        source:
          line: 12
          file: "member.cpp"
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
          line: 12
          file: "member.cpp"
        dwarf:
          offset: null
          tag: null
        attributes:
          Address: 0x8
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
          file: "member.cpp"
        dwarf:
          offset: null
          tag: null
        attributes:
          Address: 0x17
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
          line: 14
          file: "member.cpp"
        dwarf:
          offset: null
          tag: null
        attributes:
          Address: 0x2c
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
          line: 14
          file: "member.cpp"
        dwarf:
          offset: null
          tag: null
        attributes:
          Address: 0x43
          Discriminator: 0
          NewStatement: true
          PrologueEnd: false
          EndSequence: true
          BasicBlock: false
          EpilogueBegin: false
        children: []
"""


def test_yaml(diva):
    assert diva('member.o --output=yaml') == expected_yaml
