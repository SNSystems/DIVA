//===-- UnitTests/TestLibScopeView/TestType.cpp -----------------*- C++ -*-===//
///
/// Copyright (c) 2017 by Sony Interactive Entertainment Inc.
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to
/// deal in the Software without restriction, including without limitation the
/// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
/// sell copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.
///
//===----------------------------------------------------------------------===//
///
/// \file
/// Tests for LibScopeView::Type.
///
//===----------------------------------------------------------------------===//

#include "PrintSettings.h"
#include "Scope.h"
#include "Symbol.h"
#include "Type.h"

#include "dwarf.h"
#include "gtest/gtest.h"

using namespace LibScopeView;

TEST(Type, getAsText_Enumerator) {
  PrintSettings Settings;

  TypeEnumerator TyEnumerator;

  TyEnumerator.setName("mon");
  EXPECT_EQ(TyEnumerator.getAsText(Settings), "    - \"mon\" = ");

  TyEnumerator.setValue("10");
  EXPECT_EQ(TyEnumerator.getAsText(Settings), "    - \"mon\" = 10");

  Settings.ShowDWARFOffset = true;
  TyEnumerator.setDieOffset(0x0);
  EXPECT_EQ(TyEnumerator.getAsText(Settings),
            "    - \"mon\" = 10 [0x00000000]");
}

TEST(Type, getAsYAML_Enumerator) {
  TypeEnumerator TyEnumerator;
  // Printing enumerators is handled in ScopeEnumeration.
  ASSERT_FALSE(TyEnumerator.getIsPrintedAsObject());
}

TEST(Type, getAsText_Inheritance) {
  PrintSettings Settings;

  Type Base;
  Base.setName("Base");
  ScopeAggregate ParentClass;
  ParentClass.setIsClassType();
  ScopeAggregate ParentStruct;
  ParentStruct.setIsStructType();

  // Test no type name, no access, class parent.
  TypeImport Inherit;
  Inherit.setIsInheritance();
  Inherit.setParent(&ParentClass);
  EXPECT_EQ(Inherit.getAsText(Settings), "    - private \"\"");

  // Test type name, no access, struct parent.
  Inherit.setType(&Base);
  Inherit.setParent(&ParentStruct);
  EXPECT_EQ(Inherit.getAsText(Settings), "    - public \"Base\"");

  // Test different access.
  Inherit.setParent(nullptr); // Parent should not be checked.
  Inherit.setInheritanceAccess(AccessSpecifier::Private);
  EXPECT_EQ(Inherit.getAsText(Settings), "    - private \"Base\"");
  Inherit.setInheritanceAccess(AccessSpecifier::Protected);
  EXPECT_EQ(Inherit.getAsText(Settings), "    - protected \"Base\"");
  Inherit.setInheritanceAccess(AccessSpecifier::Public);
  EXPECT_EQ(Inherit.getAsText(Settings), "    - public \"Base\"");
}

TEST(Type, getAsText_Param) {
  PrintSettings Settings;

  Type Ty;
  Ty.setName("wsx");

  // Template type.
  TypeParam TyParam;
  TyParam.setIsType();
  TyParam.setIncludeInPrint();
  TyParam.setName("qaz");
  TyParam.setType(&Ty);
  TyParam.setIsTemplateType();
  EXPECT_EQ(TyParam.getAsText(Settings),
            "{TemplateParameter} \"qaz\" <- \"wsx\"");

  Ty.setQualifiedName("base::");
  Ty.setHasQualifiedName();
  EXPECT_EQ(TyParam.getAsText(Settings),
            "{TemplateParameter} \"qaz\" <- \"base::wsx\"");

  TyParam.setQualifiedName("base::");
  TyParam.setHasQualifiedName();
  EXPECT_EQ(TyParam.getAsText(Settings),
            "{TemplateParameter} \"base::qaz\" <- \"base::wsx\"");

  // Template value.
  TypeParam TemplateValue;
  TemplateValue.setIsTemplateValue();
  TemplateValue.setName("TVal");
  TemplateValue.setValue("101");
  EXPECT_EQ(TemplateValue.getAsText(Settings),
            "{TemplateParameter} \"TVal\" <- 101");

  // Template template.
  TypeParam TemplateTemplate;
  TemplateTemplate.setIsTemplateTemplate();
  TemplateTemplate.setName("TTemp");
  TemplateTemplate.setValue("vector");
  EXPECT_EQ(TemplateTemplate.getAsText(Settings),
            "{TemplateParameter} \"TTemp\" <- \"vector\"");

  // Template packs print differently.
  Scope ScpTP;
  ScpTP.setIsScope();
  ScpTP.setIsTemplatePack();
  TyParam.setParent(&ScpTP);
  EXPECT_EQ(TyParam.getAsText(Settings), "<- \"base::wsx\"");
}

TEST(Type, getAsYAML_Param) {
  Type Ty;
  Ty.setName("Ty");

  // Template type.
  TypeParam TempType;
  TempType.setName("TTy");
  TempType.setType(&Ty);
  TempType.setIsTemplateType();
  TempType.setLineNumber(11);
  TempType.setFileName("test.cpp");
  TempType.setDieOffset(0x11);
  TempType.setDieTag(DW_TAG_template_type_parameter);
  EXPECT_TRUE(TempType.getIsPrintedAsObject());
  EXPECT_EQ(TempType.getAsYAML(), "object: \"TemplateParameter\"\n"
                                  "name: \"TTy\"\n"
                                  "type: null\n"
                                  "source:\n"
                                  "  line: 11\n"
                                  "  file: \"test.cpp\"\n"
                                  "dwarf:\n"
                                  "  offset: 0x11\n"
                                  "  tag: \"DW_TAG_template_type_parameter\"\n"
                                  "attributes:\n"
                                  "  types:\n"
                                  "    - \"Ty\"");

  // Template value.
  TypeParam TempValue;
  TempValue.setIsTemplateValue();
  TempValue.setName("TVal");
  TempValue.setValue("101");
  TempValue.setLineNumber(12);
  TempValue.setFileName("test.cpp");
  TempValue.setDieOffset(0x12);
  TempValue.setDieTag(DW_TAG_template_value_parameter);
  EXPECT_TRUE(TempValue.getIsPrintedAsObject());
  EXPECT_EQ(TempValue.getAsYAML(), "object: \"TemplateParameter\"\n"
                                   "name: \"TVal\"\n"
                                   "type: null\n"
                                   "source:\n"
                                   "  line: 12\n"
                                   "  file: \"test.cpp\"\n"
                                   "dwarf:\n"
                                   "  offset: 0x12\n"
                                   "  tag: \"DW_TAG_template_value_parameter"
                                   "\"\n"
                                   "attributes:\n"
                                   "  types:\n"
                                   "    - 101");

  // Template template.
  TypeParam TempTemp;
  TempTemp.setIsTemplateTemplate();
  TempTemp.setName("TTemp");
  TempTemp.setValue("vector");
  TempTemp.setLineNumber(13);
  TempTemp.setFileName("test.cpp");
  TempTemp.setDieOffset(0x13);
  TempTemp.setDieTag(DW_TAG_GNU_template_template_parameter);
  EXPECT_TRUE(TempTemp.getIsPrintedAsObject());
  EXPECT_EQ(TempTemp.getAsYAML(), "object: \"TemplateParameter\"\n"
                                  "name: \"TTemp\"\n"
                                  "type: null\n"
                                  "source:\n"
                                  "  line: 13\n"
                                  "  file: \"test.cpp\"\n"
                                  "dwarf:\n"
                                  "  offset: 0x13\n"
                                  "  tag: \"DW_TAG_GNU_template_template"
                                  "_parameter\"\n"
                                  "attributes:\n"
                                  "  types:\n"
                                  "    - \"vector\"");

  // Template parameters in template packs are printed by the template pack.
  Scope Pack;
  Pack.setIsTemplatePack();
  TempType.setParent(&Pack);
  TempValue.setParent(&Pack);
  TempTemp.setParent(&Pack);
  EXPECT_EQ(TempType.getAsYAML(), "\"Ty\"");
  EXPECT_EQ(TempValue.getAsYAML(), "101");
  EXPECT_EQ(TempTemp.getAsYAML(), "\"vector\"");
  EXPECT_FALSE(TempType.getIsPrintedAsObject());
  EXPECT_FALSE(TempValue.getIsPrintedAsObject());
  EXPECT_FALSE(TempTemp.getIsPrintedAsObject());
}

TEST(Type, getAsText_PrimitiveType) {
  PrintSettings Settings;

  Type Ty;
  Ty.setIsBaseType();
  Ty.setIncludeInPrint();

  EXPECT_EQ(Ty.getAsText(Settings), "{PrimitiveType} -> \"\"");

  Ty.setName("qaz");
  EXPECT_EQ(Ty.getAsText(Settings), "{PrimitiveType} -> \"qaz\"");

  Ty.setByteSize(4);
  EXPECT_EQ(Ty.getAsText(Settings), "{PrimitiveType} -> \"qaz\"\n"
                                    "    - 4 bytes");

  Ty.setByteSize(23);
  EXPECT_EQ(Ty.getAsText(Settings), "{PrimitiveType} -> \"qaz\"\n"
                                    "    - 23 bytes");
}

TEST(Type, getAsYAML_PrimitiveType) {
  Type Ty;
  Ty.setIsBaseType();
  Ty.setName("qaz");
  Ty.setDieOffset(0x33);
  Ty.setDieTag(DW_TAG_base_type);
  Ty.setByteSize(4);
  EXPECT_EQ(Ty.getAsYAML(), "object: \"PrimitiveType\"\n"
                            "name: null\n"
                            "type: \"qaz\"\n"
                            "source:\n"
                            "  line: null\n"
                            "  file: null\n"
                            "dwarf:\n"
                            "  offset: 0x33\n"
                            "  tag: \"DW_TAG_base_type\"\n"
                            "attributes:\n"
                            "  size: 4");

  Type NotBase;
  ASSERT_FALSE(NotBase.getIsPrintedAsObject());
}

TEST(Type, getAsText_Typedef) {
  PrintSettings Settings;

  TypeDefinition TyDef;
  ASSERT_TRUE(TyDef.getIsPrintedAsObject());
  TyDef.setIsTypedef();
  TyDef.setIncludeInPrint();

  EXPECT_EQ(TyDef.getAsText(Settings), "{Alias} \"\" -> \"\"");

  TyDef.setName("qaz");
  EXPECT_EQ(TyDef.getAsText(Settings), "{Alias} \"qaz\" -> \"\"");

  TypeDefinition TyDef2;
  TyDef2.setIsTypedef();
  TyDef2.setIncludeInPrint();
  TyDef2.setName("wsx");
  TyDef.setType(&TyDef2);
  EXPECT_EQ(TyDef.getAsText(Settings), "{Alias} \"qaz\" -> \"wsx\"");
}

TEST(Type, getAsYAML_Typedef) {
  TypeDefinition TD;
  TD.setIsTypedef();
  TD.setDieOffset(0x84);
  TD.setDieTag(DW_TAG_typedef);
  TD.setLineNumber(10);
  TD.setFileName("source_file.cpp");
  TD.setName("MyType");
  Type Ty;
  Ty.setName("Ty");
  TD.setType(&Ty);

  EXPECT_EQ(TD.getAsYAML(), "object: \"Alias\"\n"
                            "name: \"MyType\"\n"
                            "type: \"Ty\"\n"
                            "source:\n"
                            "  line: 10\n"
                            "  file: \"source_file.cpp\"\n"
                            "dwarf:\n"
                            "  offset: 0x84\n"
                            "  tag: \"DW_TAG_typedef\"\n"
                            "attributes: {}");
}

TEST(Type, getAsText_Using) {
  PrintSettings Settings;

  Scope CU;
  CU.setIsCompileUnit();
  CU.setName("I should never be seen!");

  TypeImport UsingNamespace;
  UsingNamespace.setIsImportedModule();
  Scope NS;
  NS.setName("test_name");
  NS.setParent(&CU);
  UsingNamespace.setType(&NS);
  EXPECT_EQ(UsingNamespace.getAsText(Settings),
            "{Using} namespace \"test_name\"");

  TypeImport UsingType;
  UsingType.setIsImportedDeclaration();
  Type Ty;
  Ty.setName("test_type");
  Ty.setParent(&CU);
  UsingType.setType(&Ty);
  EXPECT_EQ(UsingType.getAsText(Settings), "{Using} type \"test_type\"");

  Scope Parent;
  Parent.setName("parent");
  Ty.setParent(&Parent);
  EXPECT_EQ(UsingType.getAsText(Settings),
            "{Using} type \"parent::test_type\"");

  TypeImport UsingVariable;
  UsingVariable.setIsImportedDeclaration();
  Symbol Variable;
  Variable.setIsVariable();
  Variable.setName("test_variable");
  Variable.setParent(&CU);
  UsingVariable.setType(&Variable);
  EXPECT_EQ(UsingVariable.getAsText(Settings),
            "{Using} variable \"test_variable\"");

  Variable.setParent(&Parent);
  EXPECT_EQ(UsingVariable.getAsText(Settings),
            "{Using} variable \"parent::test_variable\"");

  TypeImport UsingMember;
  UsingMember.setIsImportedDeclaration();
  Symbol Member;
  Member.setIsMember();
  Member.setName("test_member");
  Member.setParent(&CU);
  UsingMember.setType(&Member);
  EXPECT_EQ(UsingMember.getAsText(Settings),
            "{Using} variable \"test_member\"");

  Member.setParent(&Parent);
  EXPECT_EQ(UsingMember.getAsText(Settings),
            "{Using} variable \"parent::test_member\"");

  TypeImport UsingFunction;
  UsingFunction.setIsImportedDeclaration();
  Scope Func;
  Func.setIsFunction();
  Func.setName("test_function");
  Func.setParent(&CU);
  UsingFunction.setType(&Func);
  EXPECT_EQ(UsingFunction.getAsText(Settings),
            "{Using} function \"test_function\"");

  Func.setParent(&Parent);
  EXPECT_EQ(UsingFunction.getAsText(Settings),
            "{Using} function \"parent::test_function\"");

  TypeImport UsingStruct;
  UsingStruct.setIsImportedDeclaration();
  Scope ScpStruct;
  ScpStruct.setIsStructType();
  ScpStruct.setName("test_struct");
  ScpStruct.setParent(&CU);
  UsingStruct.setType(&ScpStruct);
  EXPECT_EQ(UsingStruct.getAsText(Settings), "{Using} type \"test_struct\"");

  ScpStruct.setParent(&Parent);
  EXPECT_EQ(UsingStruct.getAsText(Settings),
            "{Using} type \"parent::test_struct\"");

  // What if we have nested parents?
  Scope GrandParent;
  GrandParent.setName("grandparent");
  Parent.setParent(&GrandParent);
  EXPECT_EQ(UsingFunction.getAsText(Settings),
            "{Using} function \"grandparent::parent::test_function\"");
}

TEST(Type, getAsYAML_Using) {
  Scope CU;
  CU.setIsCompileUnit();
  CU.setName("I should never be seen!");

  TypeImport UsingNamespace;
  UsingNamespace.setIsImportedModule();
  UsingNamespace.setLineNumber(50);
  UsingNamespace.setFileName("test_file.cpp");
  UsingNamespace.setDieTag(DW_TAG_imported_module);
  UsingNamespace.setDieOffset(0xdeadb33f);
  Scope NS;
  NS.setName("test_name");
  NS.setParent(&CU);
  UsingNamespace.setType(&NS);
  EXPECT_EQ(UsingNamespace.getAsYAML(), "object: \"Using\""
                                        "\nname: \"test_name\""
                                        "\ntype: null"
                                        "\nsource:"
                                        "\n  line: 50"
                                        "\n  file: \"test_file.cpp\""
                                        "\ndwarf:"
                                        "\n  offset: 0xdeadb33f"
                                        "\n  tag: \"DW_TAG_imported_module\""
                                        "\nattributes:"
                                        "\n  using_type: \"namespace\"");

  TypeImport UsingType;
  UsingType.setIsImportedDeclaration();
  UsingType.setLineNumber(50);
  UsingType.setFileName("test_file.cpp");
  UsingType.setDieTag(DW_TAG_imported_declaration);
  UsingType.setDieOffset(0xdeadb33f);
  Type Ty;
  Ty.setName("test_type");
  Ty.setParent(&CU);
  UsingType.setType(&Ty);
  EXPECT_EQ(UsingType.getAsYAML(), "object: \"Using\""
                                   "\nname: \"test_type\""
                                   "\ntype: null"
                                   "\nsource:"
                                   "\n  line: 50"
                                   "\n  file: \"test_file.cpp\""
                                   "\ndwarf:"
                                   "\n  offset: 0xdeadb33f"
                                   "\n  tag: \"DW_TAG_imported_declaration\""
                                   "\nattributes:"
                                   "\n  using_type: \"type\"");

  Scope Parent;
  Parent.setName("parent");
  Ty.setParent(&Parent);
  EXPECT_EQ(UsingType.getAsYAML(), "object: \"Using\""
                                   "\nname: \"parent::test_type\""
                                   "\ntype: null"
                                   "\nsource:"
                                   "\n  line: 50"
                                   "\n  file: \"test_file.cpp\""
                                   "\ndwarf:"
                                   "\n  offset: 0xdeadb33f"
                                   "\n  tag: \"DW_TAG_imported_declaration\""
                                   "\nattributes:"
                                   "\n  using_type: \"type\"");

  TypeImport UsingVariable;
  UsingVariable.setIsImportedDeclaration();
  UsingVariable.setLineNumber(50);
  UsingVariable.setFileName("test_file.cpp");
  UsingVariable.setDieTag(DW_TAG_imported_declaration);
  UsingVariable.setDieOffset(0xdeadb33f);
  Symbol Variable;
  Variable.setIsVariable();
  Variable.setName("test_variable");
  Variable.setParent(&CU);
  UsingVariable.setType(&Variable);
  EXPECT_EQ(UsingVariable.getAsYAML(), "object: \"Using\""
                                       "\nname: \"test_variable\""
                                       "\ntype: null"
                                       "\nsource:"
                                       "\n  line: 50"
                                       "\n  file: \"test_file.cpp\""
                                       "\ndwarf:"
                                       "\n  offset: 0xdeadb33f"
                                       "\n  tag: "
                                       "\"DW_TAG_imported_declaration\""
                                       "\nattributes:"
                                       "\n  using_type: \"variable\"");

  Variable.setParent(&Parent);
  EXPECT_EQ(UsingVariable.getAsYAML(), "object: \"Using\""
                                       "\nname: \"parent::test_variable\""
                                       "\ntype: null"
                                       "\nsource:"
                                       "\n  line: 50"
                                       "\n  file: \"test_file.cpp\""
                                       "\ndwarf:"
                                       "\n  offset: 0xdeadb33f"
                                       "\n  tag: "
                                       "\"DW_TAG_imported_declaration\""
                                       "\nattributes:"
                                       "\n  using_type: \"variable\"");

  TypeImport UsingMember;
  UsingMember.setIsImportedDeclaration();
  UsingMember.setLineNumber(50);
  UsingMember.setFileName("test_file.cpp");
  UsingMember.setDieTag(DW_TAG_imported_declaration);
  UsingMember.setDieOffset(0xdeadb33f);
  Symbol Member;
  Member.setIsMember();
  Member.setName("test_member");
  Member.setParent(&CU);
  UsingMember.setType(&Member);
  EXPECT_EQ(UsingMember.getAsYAML(), "object: \"Using\""
                                     "\nname: \"test_member\""
                                     "\ntype: null"
                                     "\nsource:"
                                     "\n  line: 50"
                                     "\n  file: \"test_file.cpp\""
                                     "\ndwarf:"
                                     "\n  offset: 0xdeadb33f"
                                     "\n  tag: \"DW_TAG_imported_declaration\""
                                     "\nattributes:"
                                     "\n  using_type: \"variable\"");

  Member.setParent(&Parent);
  EXPECT_EQ(UsingMember.getAsYAML(), "object: \"Using\""
                                     "\nname: \"parent::test_member\""
                                     "\ntype: null"
                                     "\nsource:"
                                     "\n  line: 50"
                                     "\n  file: \"test_file.cpp\""
                                     "\ndwarf:"
                                     "\n  offset: 0xdeadb33f"
                                     "\n  tag: \"DW_TAG_imported_declaration\""
                                     "\nattributes:"
                                     "\n  using_type: \"variable\"");

  TypeImport UsingFunction;
  UsingFunction.setIsImportedDeclaration();
  UsingFunction.setLineNumber(50);
  UsingFunction.setFileName("test_file.cpp");
  UsingFunction.setDieTag(DW_TAG_imported_declaration);
  UsingFunction.setDieOffset(0xdeadb33f);
  Scope Function;
  Function.setIsFunction();
  Function.setName("test_function");
  Function.setParent(&CU);
  UsingFunction.setType(&Function);
  EXPECT_EQ(UsingFunction.getAsYAML(), "object: \"Using\""
                                       "\nname: \"test_function\""
                                       "\ntype: null"
                                       "\nsource:"
                                       "\n  line: 50"
                                       "\n  file: \"test_file.cpp\""
                                       "\ndwarf:"
                                       "\n  offset: 0xdeadb33f"
                                       "\n  tag: "
                                       "\"DW_TAG_imported_declaration\""
                                       "\nattributes:"
                                       "\n  using_type: \"function\"");

  Function.setParent(&Parent);
  EXPECT_EQ(UsingFunction.getAsYAML(), "object: \"Using\""
                                       "\nname: \"parent::test_function\""
                                       "\ntype: null"
                                       "\nsource:"
                                       "\n  line: 50"
                                       "\n  file: \"test_file.cpp\""
                                       "\ndwarf:"
                                       "\n  offset: 0xdeadb33f"
                                       "\n  tag: "
                                       "\"DW_TAG_imported_declaration\""
                                       "\nattributes:"
                                       "\n  using_type: \"function\"");

  TypeImport UsingStruct;
  UsingStruct.setIsImportedDeclaration();
  UsingStruct.setLineNumber(50);
  UsingStruct.setFileName("test_file.cpp");
  UsingStruct.setDieTag(DW_TAG_imported_declaration);
  UsingStruct.setDieOffset(0xdeadb33f);
  Scope StructType;
  StructType.setIsStructType();
  StructType.setName("test_struct");
  StructType.setParent(&CU);
  UsingStruct.setType(&StructType);
  EXPECT_EQ(UsingStruct.getAsYAML(), "object: \"Using\""
                                     "\nname: \"test_struct\""
                                     "\ntype: null"
                                     "\nsource:"
                                     "\n  line: 50"
                                     "\n  file: \"test_file.cpp\""
                                     "\ndwarf:"
                                     "\n  offset: 0xdeadb33f"
                                     "\n  tag: \"DW_TAG_imported_declaration\""
                                     "\nattributes:"
                                     "\n  using_type: \"type\"");

  StructType.setParent(&Parent);
  EXPECT_EQ(UsingStruct.getAsYAML(), "object: \"Using\""
                                     "\nname: \"parent::test_struct\""
                                     "\ntype: null"
                                     "\nsource:"
                                     "\n  line: 50"
                                     "\n  file: \"test_file.cpp\""
                                     "\ndwarf:"
                                     "\n  offset: 0xdeadb33f"
                                     "\n  tag: \"DW_TAG_imported_declaration\""
                                     "\nattributes:"
                                     "\n  using_type: \"type\"");

  // Test nested parents.
  Scope Grandparent;
  Grandparent.setName("grandparent");
  Parent.setParent(&Grandparent);
  EXPECT_EQ(UsingFunction.getAsYAML(), "object: \"Using\""
                                       "\nname: \"grandparent::parent::"
                                       "test_function\""
                                       "\ntype: null"
                                       "\nsource:"
                                       "\n  line: 50"
                                       "\n  file: \"test_file.cpp\""
                                       "\ndwarf:"
                                       "\n  offset: 0xdeadb33f"
                                       "\n  tag: "
                                       "\"DW_TAG_imported_declaration\""
                                       "\nattributes:"
                                       "\n  using_type: \"function\"");
}
