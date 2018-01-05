//===-- UnitTests/TestLibScopeView/TestScope.cpp ----------------*- C++ -*-===//
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
/// Tests for LibScopeView::Scope.
///
//===----------------------------------------------------------------------===//

#include "Line.h"
#include "PrintSettings.h"
#include "Scope.h"
#include "Type.h"

#include "dwarf.h"
#include "gtest/gtest.h"

using namespace LibScopeView;

TEST(Scope, getAsText_Alias) {
  PrintSettings Settings;

  ScopeAlias Alias;
  EXPECT_EQ(Alias.getAsText(Settings), "{Alias} \"\" -> \"void\"");

  Settings.ShowVoid = false;
  EXPECT_EQ(Alias.getAsText(Settings), "{Alias} \"\" -> \"\"");

  Alias.setName("test<int>");
  EXPECT_EQ(Alias.getAsText(Settings), "{Alias} \"test<int>\" -> \"\"");

  Type Ty;
  Alias.setType(&Ty);
  EXPECT_EQ(Alias.getAsText(Settings), "{Alias} \"test<int>\" -> \"\"");

  Ty.setName("foo<int, int>");
  EXPECT_EQ(Alias.getAsText(Settings),
            "{Alias} \"test<int>\" -> \"foo<int, int>\"");

  Ty.setQualifiedName("Class::");
  EXPECT_EQ(Alias.getAsText(Settings),
            "{Alias} \"test<int>\" -> \"Class::foo<int, int>\"");
}

TEST(Scope, getAsYAML_Alias) {
  ScopeAlias Alias;
  Alias.setName("test<int>");
  Alias.setLineNumber(18);
  Alias.setFilePath("test.cpp");
  Alias.setDieOffset(0x123f);
  Alias.setDieTag(DW_TAG_template_alias);
  Type Ty;
  Alias.setType(&Ty);
  Ty.setName("foo<int, int>");

  EXPECT_EQ(Alias.getAsYAML(), "object: \"Alias\"\n"
                               "name: \"test<int>\"\n"
                               "type: \"foo<int, int>\"\n"
                               "source:\n"
                               "  line: 18\n"
                               "  file: \"test.cpp\"\n"
                               "dwarf:\n"
                               "  offset: 0x123f\n"
                               "  tag: \"DW_TAG_template_alias\"\n"
                               "attributes: {}");

  Ty.setQualifiedName("Class::");
  EXPECT_EQ(Alias.getAsYAML(), "object: \"Alias\"\n"
                               "name: \"test<int>\"\n"
                               "type: \"Class::foo<int, int>\"\n"
                               "source:\n"
                               "  line: 18\n"
                               "  file: \"test.cpp\"\n"
                               "dwarf:\n"
                               "  offset: 0x123f\n"
                               "  tag: \"DW_TAG_template_alias\"\n"
                               "attributes: {}");
}

TEST(Scope, getAsText_Array) {
  ScopeArray Array;
  Array.setName("int 5,10");
  EXPECT_EQ(Array.getAsText(PrintSettings()), "{Array} \"int 5,10\"");
}

TEST(Scope, getAsText_Block) {
  PrintSettings Settings;
  Settings.ShowBlockAttributes = true;

  Scope Block;
  Block.setIsBlock();
  EXPECT_EQ(Block.getAsText(Settings), "{Block}");

  Scope TryBlock;
  TryBlock.setIsBlock();
  TryBlock.setIsTryBlock();
  EXPECT_EQ(TryBlock.getAsText(Settings), "{Block}\n    - try");

  Scope CatchBlock;
  CatchBlock.setIsBlock();
  CatchBlock.setIsCatchBlock();
  EXPECT_EQ(CatchBlock.getAsText(Settings), "{Block}\n    - catch");
}

TEST(Scope, getAsYAML_Block) {
  Scope Block;
  Block.setIsBlock();
  Block.setLineNumber(10);
  Block.setFilePath("test.cpp");
  Block.setDieOffset(0xff);
  Block.setDieTag(DW_TAG_lexical_block);
  EXPECT_EQ(Block.getAsYAML(), "object: \"Block\"\n"
                               "name: null\n"
                               "type: null\n"
                               "source:\n"
                               "  line: 10\n"
                               "  file: \"test.cpp\"\n"
                               "dwarf:\n"
                               "  offset: 0xff\n"
                               "  tag: \"DW_TAG_lexical_block\"\n"
                               "attributes:\n"
                               "  try: false\n"
                               "  catch: false");

  Scope TryBlock;
  TryBlock.setIsBlock();
  TryBlock.setIsTryBlock();
  TryBlock.setLineNumber(11);
  TryBlock.setFilePath("test.cpp");
  TryBlock.setDieOffset(0x100);
  TryBlock.setDieTag(DW_TAG_try_block);
  EXPECT_EQ(TryBlock.getAsYAML(), "object: \"Block\"\n"
                                  "name: null\n"
                                  "type: null\n"
                                  "source:\n"
                                  "  line: 11\n"
                                  "  file: \"test.cpp\"\n"
                                  "dwarf:\n"
                                  "  offset: 0x100\n"
                                  "  tag: \"DW_TAG_try_block\"\n"
                                  "attributes:\n"
                                  "  try: true\n"
                                  "  catch: false");

  Scope CatchBlock;
  CatchBlock.setIsBlock();
  CatchBlock.setIsCatchBlock();
  CatchBlock.setLineNumber(12);
  CatchBlock.setFilePath("test.cpp");
  CatchBlock.setDieOffset(0x111);
  CatchBlock.setDieTag(DW_TAG_catch_block);
  EXPECT_EQ(CatchBlock.getAsYAML(), "object: \"Block\"\n"
                                    "name: null\n"
                                    "type: null\n"
                                    "source:\n"
                                    "  line: 12\n"
                                    "  file: \"test.cpp\"\n"
                                    "dwarf:\n"
                                    "  offset: 0x111\n"
                                    "  tag: \"DW_TAG_catch_block\"\n"
                                    "attributes:\n"
                                    "  try: false\n"
                                    "  catch: true");
}

TEST(Scope, getAsText_Class) {
  PrintSettings Settings;

  ScopeAggregate Class;
  Class.setIsClassType();
  Class.setName("TestClass");
  EXPECT_EQ(Class.getAsText(Settings), "{Class} \"TestClass\"");

  Class.setIsTemplate();
  EXPECT_EQ(Class.getAsText(Settings), "{Class} \"TestClass\"\n"
                                       "    - Template");
}

TEST(Scope, getAsYAML_Class) {
  ScopeAggregate Class;
  Class.setIsClassType();
  Class.setName("TestClass");
  Class.setLineNumber(5);
  Class.setFilePath("testclass.cpp");
  Class.setDieOffset(0xD3ADB33F);
  Class.setDieTag(DW_TAG_class_type);

  ScopeAggregate ParentClass;
  ParentClass.setName("TestParentClass");

  TypeImport *TyImport = new TypeImport;
  TyImport->setIsInheritance();
  TyImport->setType(&ParentClass);
  TyImport->setInheritanceAccess(AccessSpecifier::Public);

  Class.addChild(TyImport);

  std::string Expected("object: \"Class\"\n"
                       "name: \"TestClass\"\n"
                       "type: null\n"
                       "source:\n"
                       "  line: 5\n"
                       "  file: \"testclass.cpp\"\n"
                       "dwarf:\n"
                       "  offset: 0xd3adb33f\n"
                       "  tag: \"DW_TAG_class_type\"\n"
                       "attributes:\n");

  EXPECT_EQ(Class.getAsYAML(),
            Expected + std::string("  is_template: false\n"
                                   "  inherits_from:\n"
                                   "    - parent: \"TestParentClass\"\n"
                                   "      access_specifier: \"public\""));

  Class.setIsTemplate();
  EXPECT_EQ(Class.getAsYAML(),
            Expected + std::string("  is_template: true\n"
                                   "  inherits_from:\n"
                                   "    - parent: \"TestParentClass\"\n"
                                   "      access_specifier: \"public\""));
}

TEST(Scope, getAsYAML_multiInheritance) {
  ScopeAggregate Class;
  Class.setIsClassType();
  Class.setName("TestClass");
  Class.setLineNumber(5);
  Class.setFilePath("testclass.cpp");
  Class.setDieOffset(0xD3ADB33F);
  Class.setDieTag(DW_TAG_class_type);

  ScopeAggregate ParentClassA;
  ParentClassA.setName("TestParentClassA");

  TypeImport *TyImportA = new TypeImport;
  TyImportA->setIsInheritance();
  TyImportA->setType(&ParentClassA);
  TyImportA->setInheritanceAccess(AccessSpecifier::Public);

  ScopeAggregate ParentClassB;
  ParentClassB.setName("TestParentClassB");

  TypeImport *TyImportB = new TypeImport;
  TyImportB->setIsInheritance();
  TyImportB->setType(&ParentClassB);
  TyImportB->setInheritanceAccess(AccessSpecifier::Protected);

  Class.addChild(TyImportA);
  Class.addChild(TyImportB);

  EXPECT_EQ(Class.getAsYAML(), "object: \"Class\"\n"
                               "name: \"TestClass\"\n"
                               "type: null\n"
                               "source:\n"
                               "  line: 5\n"
                               "  file: \"testclass.cpp\"\n"
                               "dwarf:\n"
                               "  offset: 0xd3adb33f\n"
                               "  tag: \"DW_TAG_class_type\"\n"
                               "attributes:\n"
                               "  is_template: false\n"
                               "  inherits_from:\n"
                               "    - parent: \"TestParentClassA\"\n"
                               "      access_specifier: \"public\"\n"
                               "    - parent: \"TestParentClassB\"\n"
                               "      access_specifier: \"protected\"");
}

TEST(Scope, getAsYAML_Unspecified_Class) {
  ScopeAggregate Class;
  Class.setIsClassType();
  Class.setName("TestClass");
  Class.setLineNumber(5);
  Class.setFilePath("testclass.cpp");
  Class.setDieOffset(0xD3ADB33F);
  Class.setDieTag(DW_TAG_class_type);

  ScopeAggregate ParentStruct;
  ParentStruct.setName("TestParentStruct");

  TypeImport *TyImport = new TypeImport;
  TyImport->setIsInheritance();
  TyImport->setType(&ParentStruct);
  TyImport->setInheritanceAccess(AccessSpecifier::Unspecified);

  Class.addChild(TyImport);

  EXPECT_EQ(Class.getAsYAML(), "object: \"Class\"\n"
                               "name: \"TestClass\"\n"
                               "type: null\n"
                               "source:\n"
                               "  line: 5\n"
                               "  file: \"testclass.cpp\"\n"
                               "dwarf:\n"
                               "  offset: 0xd3adb33f\n"
                               "  tag: \"DW_TAG_class_type\"\n"
                               "attributes:\n"
                               "  is_template: false\n"
                               "  inherits_from:\n"
                               "    - parent: \"TestParentStruct\"\n"
                               "      access_specifier: \"private\"");
}

TEST(Scope, getAsText_CompileUnit) {
  ScopeCompileUnit compileUnit;
  EXPECT_EQ(compileUnit.getAsText(PrintSettings()), "{CompileUnit} \"\"");

  compileUnit.setName("c:/fakedir/fakefile.cpp");

  EXPECT_EQ(compileUnit.getAsText(PrintSettings()),
            "{CompileUnit} \"c:/fakedir/fakefile.cpp\"");
}

TEST(Scope, getAsYAML_CompileUnit) {
  ScopeCompileUnit CompileUnit;
  CompileUnit.setName("c:/fakedir/fakefile.cpp");
  CompileUnit.setDieOffset(0xbeef);
  CompileUnit.setDieTag(DW_TAG_compile_unit);
  EXPECT_EQ(CompileUnit.getAsYAML(), "object: \"CompileUnit\"\n"
                                     "name: \"c:/fakedir/fakefile.cpp\"\n"
                                     "type: null\n"
                                     "source:\n"
                                     "  line: null\n"
                                     "  file: null\n"
                                     "dwarf:\n"
                                     "  offset: 0xbeef\n"
                                     "  tag: \"DW_TAG_compile_unit\"\n"
                                     "attributes: {}");
}

TEST(Scope, getAsText_Enumeration) {
  ScopeEnumeration ScpEnumeration;
  EXPECT_EQ(ScpEnumeration.getAsText(PrintSettings()), "{Enum} \"\"");

  ScpEnumeration.setName("days");
  EXPECT_EQ(ScpEnumeration.getAsText(PrintSettings()), "{Enum} \"days\"");

  ScpEnumeration.setIsClass();
  EXPECT_EQ(ScpEnumeration.getAsText(PrintSettings()), "{Enum} class \"days\"");

  Type Ty;
  Ty.setName("unsigned int");
  ScpEnumeration.setName("days");
  ScpEnumeration.setType(&Ty);
  EXPECT_EQ(ScpEnumeration.getAsText(PrintSettings()),
            "{Enum} class \"days\" -> \"unsigned int\"");
}

TEST(Scope, getAsYAML_Enumeration) {
  ScopeEnumeration Enum;
  Enum.setName("days");
  Enum.setLineNumber(42);
  Enum.setFilePath("enum.cpp");
  Enum.setDieOffset(0xfeed);
  Enum.setDieTag(DW_TAG_enumeration_type);
  Type Ty;
  Ty.setName("unsigned int");
  Enum.setName("days");
  Enum.setType(&Ty);
  std::string ExpectedYAML("object: \"Enum\"\n"
                           "name: \"days\"\n"
                           "type: \"unsigned int\"\n"
                           "source:\n"
                           "  line: 42\n"
                           "  file: \"enum.cpp\"\n"
                           "dwarf:\n"
                           "  offset: 0xfeed\n"
                           "  tag: \"DW_TAG_enumeration_type\"\n"
                           "attributes:");
  EXPECT_EQ(Enum.getAsYAML(),
            ExpectedYAML + "\n  class: false\n  enumerators: []");

  Enum.setIsClass();
  ExpectedYAML += "\n  class: true";
  EXPECT_EQ(Enum.getAsYAML(), ExpectedYAML + "\n  enumerators: []");

  auto *Monday = new TypeEnumerator;
  Monday->setName("monday");
  Monday->setValue("10");
  Enum.addChild(Monday);
  auto *Tuesday = new TypeEnumerator;
  Tuesday->setName("tuesday");
  Tuesday->setValue("20");
  Enum.addChild(Tuesday);
  ExpectedYAML += "\n  enumerators:"
                  "\n    - enumerator: \"monday\""
                  "\n      value: 10"
                  "\n    - enumerator: \"tuesday\""
                  "\n      value: 20";
  EXPECT_EQ(Enum.getAsYAML(), ExpectedYAML);
}

TEST(Scope, getAsText_Function) {
  PrintSettings Settings;

  ScopeFunction ScpFunc;
  EXPECT_EQ(ScpFunc.getAsText(Settings), "{Function} \"\" -> \"void\"\n"
                                         "    - No declaration");

  Settings.ShowVoid = false;
  EXPECT_EQ(ScpFunc.getAsText(Settings), "{Function} \"\" -> \"\"\n"
                                         "    - No declaration");

  ScpFunc.setName("qaz");
  EXPECT_EQ(ScpFunc.getAsText(Settings), "{Function} \"qaz\" -> \"\"\n"
                                         "    - No declaration");

  Type RetType;
  RetType.setName("wsx");
  ScpFunc.setType(&RetType);
  EXPECT_EQ(ScpFunc.getAsText(Settings), "{Function} \"qaz\" -> \"wsx\"\n"
                                         "    - No declaration");

  ScopeNamespace NS;
  NS.setName("TestNamespace");
  ScpFunc.setParent(&NS);
  EXPECT_EQ(ScpFunc.getAsText(Settings),
            "{Function} \"TestNamespace::qaz\" -> \"wsx\"\n"
            "    - No declaration");

  ScopeNamespace NS2;
  NS2.setName("BaseNamespace");
  NS.setParent(&NS2);
  EXPECT_EQ(ScpFunc.getAsText(Settings),
            "{Function} \"BaseNamespace::TestNamespace::qaz\" -> \"wsx\"\n"
            "    - No declaration");

  ScopeFunction StaticFunc;
  StaticFunc.setName("sf");
  StaticFunc.setIsStatic();
  EXPECT_EQ(StaticFunc.getAsText(Settings), "{Function} static \"sf\" -> \"\"\n"
                                            "    - No declaration");

  ScopeFunction DecInlineFunc;
  DecInlineFunc.setName("dif");
  DecInlineFunc.setIsDeclaredInline();
  EXPECT_EQ(DecInlineFunc.getAsText(Settings),
            "{Function} inline \"dif\" -> \"\"\n"
            "    - No declaration");

  ScopeFunction ScpQualFunc;
  ScpQualFunc.setName("qaz");
  ScpQualFunc.setType(&RetType);
  EXPECT_EQ(ScpQualFunc.getAsText(Settings), "{Function} \"qaz\" -> \"wsx\"\n"
                                             "    - No declaration");

  RetType.setQualifiedName("base::");
  EXPECT_EQ(ScpQualFunc.getAsText(Settings),
            "{Function} \"qaz\" -> \"base::wsx\"\n"
            "    - No declaration");
}

TEST(Scope, getAsText_Function_Attributes) {
  PrintSettings Settings;
  Settings.ShowVoid = false;

  // Test attributes.
  std::string Expected("{Function} \"\" -> \"\"");
  ScopeFunction Func;

  ScopeFunction DeclFunc;
  Expected.append("\n    - ");
  EXPECT_EQ(DeclFunc.getAsText(Settings), Expected + "No declaration");

  DeclFunc.setIsDeclaration();
  EXPECT_EQ(DeclFunc.getAsText(Settings), Expected + "Is declaration");

  DeclFunc.setFilePath("test/file.h");
  DeclFunc.setLineNumber(24);
  Func.setReference(&DeclFunc);
  Expected.append("Declaration @ ");
  EXPECT_EQ(Func.getAsText(Settings), Expected + "file.h,24");

  DeclFunc.setInvalidFileName();
  Expected.append("?,24");
  EXPECT_EQ(Func.getAsText(Settings), Expected);

  Func.setIsTemplate();
  Expected.append("\n    - ").append("Template");
  EXPECT_EQ(Func.getAsText(Settings), Expected);

  Func.setIsDeclaration();
  Expected.append("\n    - ").append("Is declaration");
  EXPECT_EQ(Func.getAsText(Settings), Expected);

  Line Low1;
  Line Low2;
  Line High1;
  Line High2;
  Low1.setLineNumber(9);
  Low2.setLineNumber(4);
  High1.setLineNumber(20);
  High2.setLineNumber(30);
  EXPECT_EQ(Func.getAsText(Settings), Expected);

  // ScopeFunctionInlined.
  ScopeFunctionInlined inlined;
  inlined.setName("Foo");
  EXPECT_EQ(inlined.getAsText(Settings), "{Function} \"Foo\" -> \"\"\n"
                                         "    - No declaration\n"
                                         "    - Inlined");
}

TEST(Scope, getAsYAML_Function) {
  ScopeFunction Func;
  Func.setName("Foo");
  Func.setLineNumber(17);
  Func.setFilePath("foo.cpp");
  Func.setDieOffset(0xce);
  Func.setDieTag(DW_TAG_subprogram);

  // Normal function.
  // No type is "void".
  EXPECT_EQ(Func.getAsYAML(), "object: \"Function\"\n"
                              "name: \"Foo\"\n"
                              "type: \"void\"\n"
                              "source:\n"
                              "  line: 17\n"
                              "  file: \"foo.cpp\"\n"
                              "dwarf:\n"
                              "  offset: 0xce\n"
                              "  tag: \"DW_TAG_subprogram\"\n"
                              "attributes:\n"
                              "  declaration:\n"
                              "    file: null\n"
                              "    line: null\n"
                              "  is_template: false\n"
                              "  static: false\n"
                              "  inline: false\n"
                              "  is_inlined: false\n"
                              "  is_declaration: false");

  Type Ty;
  Ty.setName("int");
  Func.setType(&Ty);
  EXPECT_EQ(Func.getAsYAML(), "object: \"Function\"\n"
                              "name: \"Foo\"\n"
                              "type: \"int\"\n"
                              "source:\n"
                              "  line: 17\n"
                              "  file: \"foo.cpp\"\n"
                              "dwarf:\n"
                              "  offset: 0xce\n"
                              "  tag: \"DW_TAG_subprogram\"\n"
                              "attributes:\n"
                              "  declaration:\n"
                              "    file: null\n"
                              "    line: null\n"
                              "  is_template: false\n"
                              "  static: false\n"
                              "  inline: false\n"
                              "  is_inlined: false\n"
                              "  is_declaration: false");

  // Declared inline function.
  Func.setIsDeclaredInline();
  EXPECT_EQ(Func.getAsYAML(), "object: \"Function\"\n"
                              "name: \"Foo\"\n"
                              "type: \"int\"\n"
                              "source:\n"
                              "  line: 17\n"
                              "  file: \"foo.cpp\"\n"
                              "dwarf:\n"
                              "  offset: 0xce\n"
                              "  tag: \"DW_TAG_subprogram\"\n"
                              "attributes:\n"
                              "  declaration:\n"
                              "    file: null\n"
                              "    line: null\n"
                              "  is_template: false\n"
                              "  static: false\n"
                              "  inline: true\n"
                              "  is_inlined: false\n"
                              "  is_declaration: false");

  // Declaration for Static function.
  Func.setIsStatic();
  Func.setIsDeclaration();
  EXPECT_EQ(Func.getAsYAML(), "object: \"Function\"\n"
                              "name: \"Foo\"\n"
                              "type: \"int\"\n"
                              "source:\n"
                              "  line: 17\n"
                              "  file: \"foo.cpp\"\n"
                              "dwarf:\n"
                              "  offset: 0xce\n"
                              "  tag: \"DW_TAG_subprogram\"\n"
                              "attributes:\n"
                              "  declaration:\n"
                              "    file: null\n"
                              "    line: null\n"
                              "  is_template: false\n"
                              "  static: true\n"
                              "  inline: true\n"
                              "  is_inlined: false\n"
                              "  is_declaration: true");

  // Template function.
  Func.setIsTemplate();
  EXPECT_EQ(Func.getAsYAML(), "object: \"Function\"\n"
                              "name: \"Foo\"\n"
                              "type: \"int\"\n"
                              "source:\n"
                              "  line: 17\n"
                              "  file: \"foo.cpp\"\n"
                              "dwarf:\n"
                              "  offset: 0xce\n"
                              "  tag: \"DW_TAG_subprogram\"\n"
                              "attributes:\n"
                              "  declaration:\n"
                              "    file: null\n"
                              "    line: null\n"
                              "  is_template: true\n"
                              "  static: true\n"
                              "  inline: true\n"
                              "  is_inlined: false\n"
                              "  is_declaration: true");

  // Function to be used as Reference.
  ScopeFunction Reference;
  Reference.setName("Ref");
  Reference.setLineNumber(620);
  Reference.setFilePath("ref.cpp");
  Reference.setDieOffset(0xba);

  // Reference to function.
  Func.setReference(&Reference);
  EXPECT_EQ(Func.getAsYAML(), "object: \"Function\"\n"
                              "name: \"Foo\"\n"
                              "type: \"int\"\n"
                              "source:\n"
                              "  line: 17\n"
                              "  file: \"foo.cpp\"\n"
                              "dwarf:\n"
                              "  offset: 0xce\n"
                              "  tag: \"DW_TAG_subprogram\"\n"
                              "attributes:\n"
                              "  declaration:\n"
                              "    file: \"ref.cpp\"\n"
                              "    line: 620\n"
                              "  is_template: true\n"
                              "  static: true\n"
                              "  inline: true\n"
                              "  is_inlined: false\n"
                              "  is_declaration: true");

  // Invalid Reference to function.
  Reference.setInvalidFileName();
  Reference.setLineNumber(0);
  EXPECT_EQ(Func.getAsYAML(), "object: \"Function\"\n"
                              "name: \"Foo\"\n"
                              "type: \"int\"\n"
                              "source:\n"
                              "  line: 17\n"
                              "  file: \"foo.cpp\"\n"
                              "dwarf:\n"
                              "  offset: 0xce\n"
                              "  tag: \"DW_TAG_subprogram\"\n"
                              "attributes:\n"
                              "  declaration:\n"
                              "    file: \"?\"\n"
                              "    line: 0\n"
                              "  is_template: true\n"
                              "  static: true\n"
                              "  inline: true\n"
                              "  is_inlined: false\n"
                              "  is_declaration: true");

  // ScopeFunctionInlined.
  ScopeFunctionInlined Inlined;
  Inlined.setName("Foo");
  Inlined.setDieOffset(0x100);
  Inlined.setDieTag(DW_TAG_inlined_subroutine);

  EXPECT_EQ(Inlined.getAsYAML(), "object: \"Function\"\n"
                                 "name: \"Foo\"\n"
                                 "type: \"void\"\n"
                                 "source:\n"
                                 "  line: null\n"
                                 "  file: null\n"
                                 "dwarf:\n"
                                 "  offset: 0x100\n"
                                 "  tag: \"DW_TAG_inlined_subroutine\"\n"
                                 "attributes:\n"
                                 "  declaration:\n"
                                 "    file: null\n"
                                 "    line: null\n"
                                 "  is_template: false\n"
                                 "  static: false\n"
                                 "  inline: false\n"
                                 "  is_inlined: true\n"
                                 "  is_declaration: false");
}

TEST(Scope, getAsText_Namespace) {
  ScopeNamespace NS;
  EXPECT_EQ(NS.getAsText(PrintSettings()), "{Namespace}");

  NS.setName("TestNamespace");
  EXPECT_EQ(NS.getAsText(PrintSettings()), "{Namespace} \"TestNamespace\"");

  ScopeNamespace Parent;
  Parent.setName("Base");
  NS.setParent(&Parent);
  EXPECT_EQ(NS.getAsText(PrintSettings()),
            "{Namespace} \"Base::TestNamespace\"");
}

TEST(Scope, getAsYAML_Namespace) {
  ScopeNamespace NS;
  NS.setName("TestNamespace");
  NS.setLineNumber(17);
  NS.setFilePath("test.cpp");
  NS.setDieOffset(0x1212);
  NS.setDieTag(DW_TAG_namespace);
  EXPECT_EQ(NS.getAsYAML(), "object: \"Namespace\"\n"
                            "name: \"TestNamespace\"\n"
                            "type: null\n"
                            "source:\n"
                            "  line: 17\n"
                            "  file: \"test.cpp\"\n"
                            "dwarf:\n"
                            "  offset: 0x1212\n"
                            "  tag: \"DW_TAG_namespace\"\n"
                            "attributes: {}");
}

TEST(Scope, getAsText_Root) {
  ScopeRoot Root;
  EXPECT_EQ(Root.getAsText(PrintSettings()), "{InputFile} \"\"");

  Root.setName("test/file.o");
  EXPECT_EQ(Root.getAsText(PrintSettings()), "{InputFile} \"test/file.o\"");
}

TEST(Scope, getAsText_Struct) {
  PrintSettings Settings;

  ScopeAggregate Struct;
  Struct.setIsStructType();
  Struct.setName("TestStruct");
  EXPECT_EQ(Struct.getAsText(Settings), "{Struct} \"TestStruct\"");

  Struct.setIsTemplate();
  EXPECT_EQ(Struct.getAsText(Settings), "{Struct} \"TestStruct\"\n"
                                        "    - Template");
}

TEST(Scope, getAsYAML_Struct) {
  ScopeAggregate Struct;
  Struct.setIsStructType();
  Struct.setName("TestStruct");
  Struct.setLineNumber(5);
  Struct.setFilePath("teststruct.cpp");
  Struct.setDieOffset(0xD3ADB33F);
  Struct.setDieTag(DW_TAG_class_type);

  ScopeAggregate ParentClass;
  ParentClass.setName("TestParentClass");

  TypeImport *TyImport = new TypeImport;
  TyImport->setIsInheritance();
  TyImport->setType(&ParentClass);
  TyImport->setInheritanceAccess(AccessSpecifier::Private);

  Struct.addChild(TyImport);

  std::string Expected("object: \"Struct\"\n"
                       "name: \"TestStruct\"\n"
                       "type: null\n"
                       "source:\n"
                       "  line: 5\n"
                       "  file: \"teststruct.cpp\"\n"
                       "dwarf:\n"
                       "  offset: 0xd3adb33f\n"
                       "  tag: \"DW_TAG_class_type\"\n"
                       "attributes:\n");

  EXPECT_EQ(Struct.getAsYAML(),
            Expected + std::string("  is_template: false\n"
                                   "  inherits_from:\n"
                                   "    - parent: \"TestParentClass\"\n"
                                   "      access_specifier: \"private\""));

  Struct.setIsTemplate();
  EXPECT_EQ(Struct.getAsYAML(),
            Expected + std::string("  is_template: true\n"
                                   "  inherits_from:\n"
                                   "    - parent: \"TestParentClass\"\n"
                                   "      access_specifier: \"private\""));
}

TEST(Scope, getAsYAML_Unspecified_Struct) {
  ScopeAggregate Struct;
  Struct.setIsStructType();
  Struct.setName("TestStruct");
  Struct.setLineNumber(5);
  Struct.setFilePath("teststruct.cpp");
  Struct.setDieOffset(0xD3ADB33F);
  Struct.setDieTag(DW_TAG_class_type);

  ScopeAggregate ParentClass;
  ParentClass.setName("TestParentClass");
  ParentClass.setIsStructType();

  TypeImport *TyImport = new TypeImport;
  TyImport->setIsInheritance();
  TyImport->setType(&ParentClass);
  TyImport->setInheritanceAccess(AccessSpecifier::Unspecified);

  Struct.addChild(TyImport);

  EXPECT_EQ(Struct.getAsYAML(), "object: \"Struct\"\n"
                                "name: \"TestStruct\"\n"
                                "type: null\n"
                                "source:\n"
                                "  line: 5\n"
                                "  file: \"teststruct.cpp\"\n"
                                "dwarf:\n"
                                "  offset: 0xd3adb33f\n"
                                "  tag: \"DW_TAG_class_type\"\n"
                                "attributes:\n"
                                "  is_template: false\n"
                                "  inherits_from:\n"
                                "    - parent: \"TestParentClass\"\n"
                                "      access_specifier: \"public\"");
}

TEST(Scope, getAsText_TemplatePack) {
  ScopeTemplatePack ScpTP;
  EXPECT_EQ(ScpTP.getAsText(PrintSettings()), "{TemplateParameter} \"\"");

  ScpTP.setName("qaz");
  EXPECT_EQ(ScpTP.getAsText(PrintSettings()), "{TemplateParameter} \"qaz\"");
}

TEST(Scope, getAsYAML_TemplatePack) {
  ScopeTemplatePack Pack;
  Pack.setName("TPack");
  Pack.setLineNumber(11);
  Pack.setFilePath("test.cpp");
  Pack.setDieOffset(0x11);
  Pack.setDieTag(DW_TAG_GNU_template_parameter_pack);

  std::string Expected("object: \"TemplateParameter\"\n"
                       "name: \"TPack\"\n"
                       "type: null\n"
                       "source:\n"
                       "  line: 11\n"
                       "  file: \"test.cpp\"\n"
                       "dwarf:\n"
                       "  offset: 0x11\n"
                       "  tag: \"DW_TAG_GNU_template_parameter_pack\"\n"
                       "attributes:\n"
                       "  types:");
  EXPECT_EQ(Pack.getAsYAML(), Expected + std::string(" []"));

  auto *TempType = new TypeTemplateParam;
  Type Ty;
  Ty.setName("Ty");
  TempType->setType(&Ty);
  TempType->setIsTemplateType();
  Pack.addChild(TempType);

  Expected.append("\n    - \"Ty\"");
  EXPECT_EQ(Pack.getAsYAML(), Expected);

  auto *TempValue = new TypeTemplateParam;
  TempValue->setIsTemplateValue();
  TempValue->setValue("101");
  Pack.addChild(TempValue);

  Expected.append("\n    - 101");
  EXPECT_EQ(Pack.getAsYAML(), Expected);

  auto *TempTemp = new TypeTemplateParam;
  TempTemp->setIsTemplateTemplate();
  TempTemp->setValue("vector");
  Pack.addChild(TempTemp);

  Expected.append("\n    - \"vector\"");
  EXPECT_EQ(Pack.getAsYAML(), Expected);
}

TEST(Scope, getAsText_Union) {
  PrintSettings Settings;

  ScopeAggregate Union;
  Union.setIsUnionType();
  Union.setName("TestUnion");
  EXPECT_EQ(Union.getAsText(Settings), "{Union} \"TestUnion\"");

  Union.setIsTemplate();
  EXPECT_EQ(Union.getAsText(Settings), "{Union} \"TestUnion\"\n"
                                       "    - Template");
}

TEST(Scope, getAsYAML_Union) {
  ScopeAggregate Union;
  Union.setIsUnionType();
  Union.setName("TestUnion");
  Union.setLineNumber(5);
  Union.setFilePath("testunion.cpp");
  Union.setDieOffset(0xD3ADB33F);
  Union.setDieTag(DW_TAG_class_type);

  std::string Expected("object: \"Union\"\n"
                       "name: \"TestUnion\"\n"
                       "type: null\n"
                       "source:\n"
                       "  line: 5\n"
                       "  file: \"testunion.cpp\"\n"
                       "dwarf:\n"
                       "  offset: 0xd3adb33f\n"
                       "  tag: \"DW_TAG_class_type\"\n"
                       "attributes:\n"
                       "  is_template: ");
  EXPECT_EQ(Union.getAsYAML(), Expected + std::string("false"));

  Union.setIsTemplate();
  EXPECT_EQ(Union.getAsYAML(), Expected + std::string("true"));
}

TEST(Scope, notPrintedAsObject) {
  EXPECT_FALSE(ScopeArray().getIsPrintedAsObject());
  EXPECT_FALSE(ScopeRoot().getIsPrintedAsObject());
}
