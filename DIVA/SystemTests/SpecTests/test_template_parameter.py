expected_txt = """\
{InputFile} "template_parameter.o"
    {CompileUnit} "template_parameter.cpp"
      {PrimitiveType} -> "int"
          - 4 bytes

{Source} "template_parameter.cpp"
 7    {Function} "t_func<1, int, vector>" -> "void"
          - No declaration
          - Template
        {TemplateParameter} "TEMPLATE" <- "vector"
        {TemplateParameter} "TY" <- "int"
        {TemplateParameter} "VAL" <- 1
10    {Function} "sum<void>" -> "int"
          - No declaration
          - Template
        {TemplateParameter} "" <- "void"
13    {Function} "sum<>" -> "int"
          - No declaration
          - Template
        {TemplateParameter} "Targs"
13      {Parameter} "x" <- "int"
13    {Function} "sum<int, int>" -> "int"
          - No declaration
          - Template
        {TemplateParameter} "Targs"
            <- "int"
            <- "int"
13      {Parameter} "args" <- "int"
13      {Parameter} "args" <- "int"
13      {Parameter} "x" <- "int"
13    {Function} "sum<int>" -> "int"
          - No declaration
          - Template
        {TemplateParameter} "Targs"
            <- "int"
13      {Parameter} "args" <- "int"
13      {Parameter} "x" <- "int"
17    {Function} "test" -> "void"
          - No declaration
"""


def test_txt(diva):
    assert diva('template_parameter.o --show-all --output=text') == expected_txt


expected_yaml = """\
input_file: "template_parameter.o"
output_version: "0.1"
objects:
  - object: "CompileUnit"
    name: "template_parameter.cpp"
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
          offset: 0x166
          tag: "DW_TAG_base_type"
        attributes:
          size: 4
        children: []
      - object: "Function"
        name: "t_func<1, int, vector>"
        type: "void"
        source:
          line: 7
          file: "template_parameter.cpp"
        dwarf:
          offset: 0x43
          tag: "DW_TAG_subprogram"
        attributes:
          declaration:
            file: null
            line: null
          is_template: true
          static: false
          inline: false
          is_inlined: false
          is_declaration: false
        children:
          - object: "TemplateParameter"
            name: "TEMPLATE"
            type: null
            source:
              line: null
              file: null
            dwarf:
              offset: 0x6f
              tag: "DW_TAG_GNU_template_template_parameter"
            attributes:
              types:
                - "vector"
            children: []
          - object: "TemplateParameter"
            name: "TY"
            type: null
            source:
              line: null
              file: null
            dwarf:
              offset: 0x66
              tag: "DW_TAG_template_type_parameter"
            attributes:
              types:
                - "int"
            children: []
          - object: "TemplateParameter"
            name: "VAL"
            type: null
            source:
              line: null
              file: null
            dwarf:
              offset: 0x5c
              tag: "DW_TAG_template_value_parameter"
            attributes:
              types:
                - 1
            children: []
      - object: "Function"
        name: "sum<void>"
        type: "int"
        source:
          line: 10
          file: "template_parameter.cpp"
        dwarf:
          offset: 0x147
          tag: "DW_TAG_subprogram"
        attributes:
          declaration:
            file: null
            line: null
          is_template: true
          static: false
          inline: false
          is_inlined: false
          is_declaration: false
        children:
          - object: "TemplateParameter"
            name: null
            type: null
            source:
              line: null
              file: null
            dwarf:
              offset: 0x164
              tag: "DW_TAG_template_type_parameter"
            attributes:
              types:
                - ""
            children: []
      - object: "Function"
        name: "sum<>"
        type: "int"
        source:
          line: 13
          file: "template_parameter.cpp"
        dwarf:
          offset: 0x116
          tag: "DW_TAG_subprogram"
        attributes:
          declaration:
            file: null
            line: null
          is_template: true
          static: false
          inline: false
          is_inlined: false
          is_declaration: false
        children:
          - object: "TemplateParameter"
            name: "Targs"
            type: null
            source:
              line: null
              file: null
            dwarf:
              offset: 0x141
              tag: "DW_TAG_GNU_template_parameter_pack"
            attributes:
              types: []
            children: []
          - object: "Parameter"
            name: "x"
            type: "int"
            source:
              line: 13
              file: "template_parameter.cpp"
            dwarf:
              offset: 0x133
              tag: "DW_TAG_formal_parameter"
            attributes: {}
            children: []
      - object: "Function"
        name: "sum<int, int>"
        type: "int"
        source:
          line: 13
          file: "template_parameter.cpp"
        dwarf:
          offset: 0x79
          tag: "DW_TAG_subprogram"
        attributes:
          declaration:
            file: null
            line: null
          is_template: true
          static: false
          inline: false
          is_inlined: false
          is_declaration: false
        children:
          - object: "TemplateParameter"
            name: "Targs"
            type: null
            source:
              line: null
              file: null
            dwarf:
              offset: 0xc0
              tag: "DW_TAG_GNU_template_parameter_pack"
            attributes:
              types:
                - "int"
                - "int"
            children: []
          - object: "Parameter"
            name: "args"
            type: "int"
            source:
              line: 13
              file: "template_parameter.cpp"
            dwarf:
              offset: 0xa4
              tag: "DW_TAG_formal_parameter"
            attributes: {}
            children: []
          - object: "Parameter"
            name: "args"
            type: "int"
            source:
              line: 13
              file: "template_parameter.cpp"
            dwarf:
              offset: 0xb2
              tag: "DW_TAG_formal_parameter"
            attributes: {}
            children: []
          - object: "Parameter"
            name: "x"
            type: "int"
            source:
              line: 13
              file: "template_parameter.cpp"
            dwarf:
              offset: 0x96
              tag: "DW_TAG_formal_parameter"
            attributes: {}
            children: []
      - object: "Function"
        name: "sum<int>"
        type: "int"
        source:
          line: 13
          file: "template_parameter.cpp"
        dwarf:
          offset: 0xd1
          tag: "DW_TAG_subprogram"
        attributes:
          declaration:
            file: null
            line: null
          is_template: true
          static: false
          inline: false
          is_inlined: false
          is_declaration: false
        children:
          - object: "TemplateParameter"
            name: "Targs"
            type: null
            source:
              line: null
              file: null
            dwarf:
              offset: 0x10a
              tag: "DW_TAG_GNU_template_parameter_pack"
            attributes:
              types:
                - "int"
            children: []
          - object: "Parameter"
            name: "args"
            type: "int"
            source:
              line: 13
              file: "template_parameter.cpp"
            dwarf:
              offset: 0xfc
              tag: "DW_TAG_formal_parameter"
            attributes: {}
            children: []
          - object: "Parameter"
            name: "x"
            type: "int"
            source:
              line: 13
              file: "template_parameter.cpp"
            dwarf:
              offset: 0xee
              tag: "DW_TAG_formal_parameter"
            attributes: {}
            children: []
      - object: "Function"
        name: "test"
        type: "void"
        source:
          line: 17
          file: "template_parameter.cpp"
        dwarf:
          offset: 0x2a
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
          line: 17
          file: "template_parameter.cpp"
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
          line: 18
          file: "template_parameter.cpp"
        dwarf:
          offset: 0x4
          tag: null
        attributes:
          NewStatement: true
          PrologueEnd: true
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
          file: "template_parameter.cpp"
        dwarf:
          offset: 0x1c
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
          file: "template_parameter.cpp"
        dwarf:
          offset: 0x21
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
          file: "template_parameter.cpp"
        dwarf:
          offset: 0x2a
          tag: null
        attributes:
          NewStatement: true
          PrologueEnd: false
          EndSequence: true
          BasicBlock: false
          Discriminator: true
          EpilogueBegin: false
        children: []
      - object: "CodeLine"
        name: null
        type: null
        source:
          line: 7
          file: "template_parameter.cpp"
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
          line: 7
          file: "template_parameter.cpp"
        dwarf:
          offset: 0x4
          tag: null
        attributes:
          NewStatement: true
          PrologueEnd: true
          EndSequence: false
          BasicBlock: false
          Discriminator: true
          EpilogueBegin: false
        children: []
      - object: "CodeLine"
        name: null
        type: null
        source:
          line: 7
          file: "template_parameter.cpp"
        dwarf:
          offset: 0x6
          tag: null
        attributes:
          NewStatement: true
          PrologueEnd: false
          EndSequence: true
          BasicBlock: false
          Discriminator: true
          EpilogueBegin: false
        children: []
      - object: "CodeLine"
        name: null
        type: null
        source:
          line: 13
          file: "template_parameter.cpp"
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
          line: 14
          file: "template_parameter.cpp"
        dwarf:
          offset: 0x11
          tag: null
        attributes:
          NewStatement: true
          PrologueEnd: true
          EndSequence: false
          BasicBlock: false
          Discriminator: true
          EpilogueBegin: false
        children: []
      - object: "CodeLine"
        name: null
        type: null
        source:
          line: 14
          file: "template_parameter.cpp"
        dwarf:
          offset: 0x14
          tag: null
        attributes:
          NewStatement: false
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
          line: 14
          file: "template_parameter.cpp"
        dwarf:
          offset: 0x1a
          tag: null
        attributes:
          NewStatement: false
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
          line: 14
          file: "template_parameter.cpp"
        dwarf:
          offset: 0x22
          tag: null
        attributes:
          NewStatement: false
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
          line: 14
          file: "template_parameter.cpp"
        dwarf:
          offset: 0x27
          tag: null
        attributes:
          NewStatement: false
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
          line: 14
          file: "template_parameter.cpp"
        dwarf:
          offset: 0x2f
          tag: null
        attributes:
          NewStatement: false
          PrologueEnd: false
          EndSequence: true
          BasicBlock: false
          Discriminator: true
          EpilogueBegin: false
        children: []
      - object: "CodeLine"
        name: null
        type: null
        source:
          line: 13
          file: "template_parameter.cpp"
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
          line: 14
          file: "template_parameter.cpp"
        dwarf:
          offset: 0xe
          tag: null
        attributes:
          NewStatement: true
          PrologueEnd: true
          EndSequence: false
          BasicBlock: false
          Discriminator: true
          EpilogueBegin: false
        children: []
      - object: "CodeLine"
        name: null
        type: null
        source:
          line: 14
          file: "template_parameter.cpp"
        dwarf:
          offset: 0x11
          tag: null
        attributes:
          NewStatement: false
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
          line: 14
          file: "template_parameter.cpp"
        dwarf:
          offset: 0x14
          tag: null
        attributes:
          NewStatement: false
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
          line: 14
          file: "template_parameter.cpp"
        dwarf:
          offset: 0x1c
          tag: null
        attributes:
          NewStatement: false
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
          line: 14
          file: "template_parameter.cpp"
        dwarf:
          offset: 0x21
          tag: null
        attributes:
          NewStatement: false
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
          line: 14
          file: "template_parameter.cpp"
        dwarf:
          offset: 0x29
          tag: null
        attributes:
          NewStatement: false
          PrologueEnd: false
          EndSequence: true
          BasicBlock: false
          Discriminator: true
          EpilogueBegin: false
        children: []
      - object: "CodeLine"
        name: null
        type: null
        source:
          line: 13
          file: "template_parameter.cpp"
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
          line: 14
          file: "template_parameter.cpp"
        dwarf:
          offset: 0xb
          tag: null
        attributes:
          NewStatement: true
          PrologueEnd: true
          EndSequence: false
          BasicBlock: false
          Discriminator: true
          EpilogueBegin: false
        children: []
      - object: "CodeLine"
        name: null
        type: null
        source:
          line: 14
          file: "template_parameter.cpp"
        dwarf:
          offset: 0xe
          tag: null
        attributes:
          NewStatement: false
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
          line: 14
          file: "template_parameter.cpp"
        dwarf:
          offset: 0x16
          tag: null
        attributes:
          NewStatement: false
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
          line: 14
          file: "template_parameter.cpp"
        dwarf:
          offset: 0x1b
          tag: null
        attributes:
          NewStatement: false
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
          line: 14
          file: "template_parameter.cpp"
        dwarf:
          offset: 0x23
          tag: null
        attributes:
          NewStatement: false
          PrologueEnd: false
          EndSequence: true
          BasicBlock: false
          Discriminator: true
          EpilogueBegin: false
        children: []
      - object: "CodeLine"
        name: null
        type: null
        source:
          line: 10
          file: "template_parameter.cpp"
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
          line: 10
          file: "template_parameter.cpp"
        dwarf:
          offset: 0x6
          tag: null
        attributes:
          NewStatement: true
          PrologueEnd: true
          EndSequence: false
          BasicBlock: false
          Discriminator: true
          EpilogueBegin: false
        children: []
      - object: "CodeLine"
        name: null
        type: null
        source:
          line: 10
          file: "template_parameter.cpp"
        dwarf:
          offset: 0x8
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
    assert diva('template_parameter.o --output=yaml') == expected_yaml
