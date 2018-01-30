expected_txt = """\
{InputFile} "using.o"
    {CompileUnit} "using.cpp"
      {PrimitiveType} -> "int"
          - 4 bytes

{Source} "using.cpp"
 3    {Struct} "A"
 5      {Member} protected "m" -> "int"
 6      {Alias} "INT" -> "int"
 7      {Function} "A::foo" -> "void"
            - Is declaration
          {Parameter} "" -> "A *"
 7    {Function} "foo" -> "void"
          - Declaration @ using.cpp,7
        {Parameter} "this" -> "const A *"
10    {Struct} "B"
          - public "A"
10      {Using} variable "A::m"
10      {Using} type "A::INT"
10      {Using} function "A::foo"
16    {Namespace} "NS"
17      {Variable} "x" -> "int"
17    {Variable} "NS::x" -> "int"
20    {Using} namespace "NS"
22    {Function} "test" -> "void"
          - No declaration
24      {Variable} "b" -> "B"
26      {Variable} "bx" -> "A::INT"
"""


def test_txt(diva):
    assert diva('using.o --show-all --output=text') == expected_txt


expected_yaml = """\
input_file: "using.o"
output_version: "0.1"
objects:
  - object: "CompileUnit"
    name: "using.cpp"
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
          offset: 0x61
          tag: "DW_TAG_base_type"
        attributes:
          size: 4
        children: []
      - object: "Struct"
        name: "A"
        type: null
        source:
          line: 3
          file: "using.cpp"
        dwarf:
          offset: 0x29
          tag: "DW_TAG_structure_type"
        attributes:
          is_template: false
          inherits_from: []
        children:
          - object: "Member"
            name: "m"
            type: "int"
            source:
              line: 5
              file: "using.cpp"
            dwarf:
              offset: 0x33
              tag: "DW_TAG_member"
            attributes:
              access_specifier: "protected"
            children: []
          - object: "Alias"
            name: "A::INT"
            type: "int"
            source:
              line: 6
              file: "using.cpp"
            dwarf:
              offset: 0x3e
              tag: "DW_TAG_typedef"
            attributes: {}
            children: []
          - object: "Function"
            name: "A::foo"
            type: "void"
            source:
              line: 7
              file: "using.cpp"
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
              is_declaration: true
            children:
              - object: "Parameter"
                name: null
                type: "A *"
                source:
                  line: null
                  file: null
                dwarf:
                  offset: 0x5a
                  tag: "DW_TAG_formal_parameter"
                attributes: {}
                children: []
      - object: "Function"
        name: "foo"
        type: "void"
        source:
          line: 7
          file: "using.cpp"
        dwarf:
          offset: 0xb3
          tag: "DW_TAG_subprogram"
        attributes:
          declaration:
            file: "using.cpp"
            line: 7
          is_template: false
          static: false
          inline: false
          is_inlined: false
          is_declaration: false
        children:
          - object: "Parameter"
            name: "this"
            type: "const A *"
            source:
              line: null
              file: null
            dwarf:
              offset: 0xd2
              tag: "DW_TAG_formal_parameter"
            attributes: {}
            children: []
      - object: "Struct"
        name: "B"
        type: null
        source:
          line: 10
          file: "using.cpp"
        dwarf:
          offset: 0x6e
          tag: "DW_TAG_structure_type"
        attributes:
          is_template: false
          inherits_from:
            - parent: "A"
              access_specifier: "public"
        children:
          - object: "Using"
            name: "A::m"
            type: null
            source:
              line: 10
              file: "using.cpp"
            dwarf:
              offset: 0x78
              tag: "DW_TAG_imported_declaration"
            attributes:
              using_type: "variable"
            children: []
          - object: "Using"
            name: "A::INT"
            type: null
            source:
              line: 10
              file: "using.cpp"
            dwarf:
              offset: 0x7f
              tag: "DW_TAG_imported_declaration"
            attributes:
              using_type: "type"
            children: []
          - object: "Using"
            name: "A::foo"
            type: null
            source:
              line: 10
              file: "using.cpp"
            dwarf:
              offset: 0x86
              tag: "DW_TAG_imported_declaration"
            attributes:
              using_type: "function"
            children: []
      - object: "Namespace"
        name: "NS"
        type: null
        source:
          line: 16
          file: "using.cpp"
        dwarf:
          offset: 0x94
          tag: "DW_TAG_namespace"
        attributes: {}
        children:
          - object: "Variable"
            name: "x"
            type: "int"
            source:
              line: 17
              file: "using.cpp"
            dwarf:
              offset: 0x9e
              tag: "DW_TAG_variable"
            attributes: {}
            children: []
      - object: "Variable"
        name: "NS::x"
        type: "int"
        source:
          line: 17
          file: "using.cpp"
        dwarf:
          offset: 0x11c
          tag: "DW_TAG_variable"
        attributes: {}
        children: []
      - object: "Using"
        name: "NS"
        type: null
        source:
          line: 20
          file: "using.cpp"
        dwarf:
          offset: 0xac
          tag: "DW_TAG_imported_module"
        attributes:
          using_type: "namespace"
        children: []
      - object: "Function"
        name: "test"
        type: "void"
        source:
          line: 22
          file: "using.cpp"
        dwarf:
          offset: 0xe4
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
            name: "b"
            type: "B"
            source:
              line: 24
              file: "using.cpp"
            dwarf:
              offset: 0x105
              tag: "DW_TAG_variable"
            attributes: {}
            children: []
          - object: "Variable"
            name: "bx"
            type: "A::INT"
            source:
              line: 26
              file: "using.cpp"
            dwarf:
              offset: 0x111
              tag: "DW_TAG_variable"
            attributes: {}
            children: []
      - object: "CodeLine"
        name: null
        type: null
        source:
          line: 7
          file: "using.cpp"
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
          line: 7
          file: "using.cpp"
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
          line: 7
          file: "using.cpp"
        dwarf:
          offset: null
          tag: null
        attributes:
          Address: 0xb
          Discriminator: 0
          NewStatement: true
          PrologueEnd: false
          EndSequence: true
          BasicBlock: false
          EpilogueBegin: false
        children: []
      - object: "CodeLine"
        name: null
        type: null
        source:
          line: 22
          file: "using.cpp"
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
          line: 22
          file: "using.cpp"
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
          line: 23
          file: "using.cpp"
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
          line: 25
          file: "using.cpp"
        dwarf:
          offset: null
          tag: null
        attributes:
          Address: 0x21
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
          line: 27
          file: "using.cpp"
        dwarf:
          offset: null
          tag: null
        attributes:
          Address: 0x28
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
          line: 28
          file: "using.cpp"
        dwarf:
          offset: null
          tag: null
        attributes:
          Address: 0x34
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
          line: 28
          file: "using.cpp"
        dwarf:
          offset: null
          tag: null
        attributes:
          Address: 0x4b
          Discriminator: 0
          NewStatement: true
          PrologueEnd: false
          EndSequence: true
          BasicBlock: false
          EpilogueBegin: false
        children: []
"""


def test_yaml(diva):
    assert diva('using.o --output=yaml') == expected_yaml
