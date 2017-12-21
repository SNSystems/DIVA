//===-- UnitTests/TestLibScopeView/TestSymbol.cpp ---------------*- C++ -*-===//
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
/// Tests for LibScopeView::Symbol.
///
//===----------------------------------------------------------------------===//

#include "PrintSettings.h"
#include "Scope.h"
#include "Symbol.h"
#include "Type.h"

#include "dwarf.h"
#include "gtest/gtest.h"

using namespace LibScopeView;

TEST(Symbol, getAsText_Member) {
  PrintSettings Settings;

  Symbol Sym;
  Sym.setIsMember();
  Sym.setAccessSpecifier(AccessSpecifier::Private);
  EXPECT_EQ(Sym.getAsText(Settings), "{Member} private -> \"void\"");

  Settings.ShowVoid = false;
  EXPECT_EQ(Sym.getAsText(Settings), "{Member} private -> \"\"");

  Sym.setName("Var");
  EXPECT_EQ(Sym.getAsText(Settings), "{Member} private \"Var\" -> \"\"");

  Type Ty;
  Ty.setIsBaseType();
  Ty.setName("VarType");
  Sym.setType(&Ty);
  EXPECT_EQ(Sym.getAsText(Settings), "{Member} private \"Var\" -> \"VarType\"");

  Sym.setAccessSpecifier(AccessSpecifier::Protected);
  EXPECT_EQ(Sym.getAsText(Settings),
            "{Member} protected \"Var\" -> \"VarType\"");

  Sym.setAccessSpecifier(AccessSpecifier::Public);
  EXPECT_EQ(Sym.getAsText(Settings), "{Member} public \"Var\" -> \"VarType\"");

  ScopeAggregate ParentClass;
  ParentClass.setIsClassType();
  Sym.setParent(&ParentClass);
  Sym.setAccessSpecifier(AccessSpecifier::Unspecified);
  EXPECT_EQ(Sym.getAsText(Settings), "{Member} private \"Var\" -> \"VarType\"");

  ScopeAggregate ParentStruct;
  ParentStruct.setIsStructType();
  Sym.setParent(&ParentStruct);
  EXPECT_EQ(Sym.getAsText(Settings), "{Member} public \"Var\" -> \"VarType\"");

  Sym.setIsStatic();
  EXPECT_EQ(Sym.getAsText(Settings),
            "{Member} public static \"Var\" -> \"VarType\"");
}

TEST(Symbol, getAsYAML_Member) {
  Symbol Sym;
  Sym.setIsMember();
  Sym.setName("Var");
  Sym.setLineNumber(77);
  Sym.setFileName("test.cpp");
  Sym.setDieOffset(0x1234);
  Sym.setDieTag(DW_TAG_member);
  Sym.setAccessSpecifier(AccessSpecifier::Private);
  Type Ty;
  Ty.setIsBaseType();
  Ty.setName("VarType");
  Sym.setType(&Ty);

  std::string CommonExpected("object: \"Member\"\n"
                             "name: \"Var\"\n"
                             "type: \"VarType\"\n"
                             "source:\n"
                             "  line: 77\n"
                             "  file: \"test.cpp\"\n"
                             "dwarf:\n"
                             "  offset: 0x1234\n"
                             "  tag: \"DW_TAG_member\"\n"
                             "attributes:\n");

  EXPECT_EQ(Sym.getAsYAML(),
            CommonExpected + std::string("  access_specifier: \"private\""));

  EXPECT_EQ(Sym.getAsYAML(),
            CommonExpected + std::string("  access_specifier: \"private\""));

  Sym.setAccessSpecifier(AccessSpecifier::Protected);
  EXPECT_EQ(Sym.getAsYAML(),
            CommonExpected + std::string("  access_specifier: \"protected\""));

  Sym.setAccessSpecifier(AccessSpecifier::Public);
  EXPECT_EQ(Sym.getAsYAML(),
            CommonExpected + std::string("  access_specifier: \"public\""));

  // Unspecified access in a class is private.
  ScopeAggregate ParentClass;
  ParentClass.setIsClassType();
  Sym.setParent(&ParentClass);
  Sym.setAccessSpecifier(AccessSpecifier::Unspecified);
  EXPECT_EQ(Sym.getAsYAML(),
            CommonExpected + std::string("  access_specifier: \"private\""));

  // Unspecified access in a scope is public.
  ScopeAggregate ParentStruct;
  ParentStruct.setIsStructType();
  Sym.setParent(&ParentStruct);
  EXPECT_EQ(Sym.getAsYAML(),
            CommonExpected + std::string("  access_specifier: \"public\""));

  Sym.setIsStatic();
  EXPECT_EQ(Sym.getAsYAML(),
            CommonExpected + std::string("  access_specifier: \"public\""));
}

TEST(Symbol, getAsText_Parameter) {
  PrintSettings Settings;

  Symbol Sym;
  Sym.setIsParameter();
  EXPECT_EQ(Sym.getAsText(Settings), "{Parameter} -> \"void\"");

  Settings.ShowVoid = false;
  EXPECT_EQ(Sym.getAsText(Settings), "{Parameter} -> \"\"");

  Sym.setName("qaz");
  EXPECT_EQ(Sym.getAsText(Settings), "{Parameter} \"qaz\" -> \"\"");

  Type Ty;
  Ty.setIsBaseType();
  Ty.setName("wsx");
  Sym.setType(&Ty);
  EXPECT_EQ(Sym.getAsText(Settings), "{Parameter} \"qaz\" -> \"wsx\"");

  // Templates have the indicator reversed '<-'.
  Scope Scp;
  Scp.setIsTemplate();
  Sym.setParent(&Scp);
  EXPECT_EQ(Sym.getAsText(Settings), "{Parameter} \"qaz\" <- \"wsx\"");

  Symbol Unspec;
  Unspec.setIsUnspecifiedParameter();
  EXPECT_EQ(Unspec.getAsText(Settings), "{Parameter} \"...\"");
}

TEST(Symbol, getAsYAML_Parameter) {
  Symbol Sym;
  Sym.setIsParameter();
  Sym.setName("qaz");
  Sym.setLineNumber(11);
  Sym.setFileName("test.cpp");
  Sym.setDieOffset(0x1122);
  Sym.setDieTag(DW_TAG_formal_parameter);
  Type Ty;
  Ty.setIsBaseType();
  Ty.setName("wsx");
  Sym.setType(&Ty);
  EXPECT_EQ(Sym.getAsYAML(), "object: \"Parameter\"\n"
                             "name: \"qaz\"\n"
                             "type: \"wsx\"\n"
                             "source:\n"
                             "  line: 11\n"
                             "  file: \"test.cpp\"\n"
                             "dwarf:\n"
                             "  offset: 0x1122\n"
                             "  tag: \"DW_TAG_formal_parameter\"\n"
                             "attributes: {}");

  Symbol Unspec;
  Unspec.setIsUnspecifiedParameter();
  Unspec.setLineNumber(12);
  Unspec.setFileName("test.cpp");
  Unspec.setDieOffset(0x1150);
  Unspec.setDieTag(DW_TAG_formal_parameter);
  Unspec.setType(&Ty);
  EXPECT_EQ(Unspec.getAsYAML(), "object: \"Parameter\"\n"
                                "name: \"...\"\n"
                                "type: \"wsx\"\n"
                                "source:\n"
                                "  line: 12\n"
                                "  file: \"test.cpp\"\n"
                                "dwarf:\n"
                                "  offset: 0x1150\n"
                                "  tag: \"DW_TAG_formal_parameter\"\n"
                                "attributes: {}");
}

TEST(Symbol, getAsText_Variable) {
  PrintSettings Settings;

  Symbol Sym;
  Sym.setIsVariable();
  EXPECT_EQ(Sym.getAsText(Settings), "{Variable} -> \"void\"");

  Settings.ShowVoid = false;
  EXPECT_EQ(Sym.getAsText(Settings), "{Variable} -> \"\"");

  Sym.setName("Var");
  EXPECT_EQ(Sym.getAsText(Settings), "{Variable} \"Var\" -> \"\"");

  Type Ty;
  Ty.setIsBaseType();
  Ty.setName("VarType");
  Sym.setType(&Ty);
  EXPECT_EQ(Sym.getAsText(Settings), "{Variable} \"Var\" -> \"VarType\"");

  Sym.setIsStatic();
  EXPECT_EQ(Sym.getAsText(Settings),
            "{Variable} static \"Var\" -> \"VarType\"");

  Sym.setQualifiedName("Base::Class::");
  Sym.setHasQualifiedName();
  EXPECT_EQ(Sym.getAsText(Settings),
            "{Variable} static \"Base::Class::Var\" -> \"VarType\"");
}

TEST(Symbol, getAsYAML_Variable) {
  Symbol Sym;
  Sym.setIsVariable();
  Sym.setName("Var");
  Sym.setLineNumber(88);
  Sym.setFileName("bttf.cpp");
  Sym.setDieOffset(0x1955);
  Sym.setDieTag(DW_TAG_variable);
  Type Ty;
  Ty.setIsBaseType();
  Ty.setName("VarType");
  Sym.setType(&Ty);
  EXPECT_EQ(Sym.getAsYAML(), "object: \"Variable\"\n"
                             "name: \"Var\"\n"
                             "type: \"VarType\"\n"
                             "source:\n"
                             "  line: 88\n"
                             "  file: \"bttf.cpp\"\n"
                             "dwarf:\n"
                             "  offset: 0x1955\n"
                             "  tag: \"DW_TAG_variable\"\n"
                             "attributes: {}");
}
