//===-- UnitTests/TestLibScopeView/TestViewSpecification.cpp ----*- C++ -*-===//
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
/// Tests for LibScopeView::PrintSettings.
///
//===----------------------------------------------------------------------===//

#include "Line.h"
#include "PrintSettings.h"
#include "Scope.h"
#include "Symbol.h"
#include "Type.h"

#include "gtest/gtest.h"

using namespace LibScopeView;

// Create a TY and set a print setting to true (e.g. ShowAlias) and check
// printObject returns true. Then set the same print setting to false and
// check printObject returns false.
#define CHECK_TY_PRINT_OPTION(TY, PRINT_FLAG_MEMBER)                           \
  do {                                                                         \
    TY Obj;                                                                    \
    Settings.PRINT_FLAG_MEMBER = true;                                         \
    EXPECT_TRUE(Settings.printObject(Obj));                                    \
    Settings.PRINT_FLAG_MEMBER = false;                                        \
    EXPECT_FALSE(Settings.printObject(Obj));                                   \
  } while (false)

TEST(PrintSettings, ShowOnlyGlobalsLocals) {
  PrintSettings Settings;
  ScopeFunction Global;
  ScopeFunction Local;
  Global.setIsGlobalReference();

  EXPECT_TRUE(Settings.printObject(Global));
  EXPECT_TRUE(Settings.printObject(Local));

  Settings.ShowOnlyLocals = false;
  Settings.ShowOnlyGlobals = true;
  EXPECT_TRUE(Settings.printObject(Global));
  EXPECT_FALSE(Settings.printObject(Local));

  Settings.ShowOnlyLocals = true;
  Settings.ShowOnlyGlobals = false;
  EXPECT_FALSE(Settings.printObject(Global));
  EXPECT_TRUE(Settings.printObject(Local));
}

TEST(PrintSettings, PrintObject_Line) {
  PrintSettings Settings;
  Line Ln;

  // Ensure code lines off by default.
  EXPECT_FALSE(Settings.printObject(Ln));

  Settings.showNone();
  Settings.ShowCodeline = true;
  EXPECT_TRUE(Settings.printObject(Ln));
  Settings.ShowCodeline = false;
  EXPECT_FALSE(Settings.printObject(Ln));
}

TEST(PrintSettings, PrintObject_Scope) {
  PrintSettings Settings;
  Settings.showNone();

  // Create a scope and set a flag (e.g. setIsTemplate()).
  // Then set a print setting to true (e.g. ShowTemplate) and check printObject
  // returns true.
  // Then set the same print setting to false and check printObject returns
  // false.
#define CHECK_SCOPE_PRINT_OPTION(TY, SCOPE_FLAG_METHOD, PRINT_FLAG_MEMBER)     \
  do {                                                                         \
    TY Scp;                                                                    \
    Scp.SCOPE_FLAG_METHOD();                                                   \
    Settings.PRINT_FLAG_MEMBER = true;                                         \
    EXPECT_TRUE(Settings.printObject(Scp));                                    \
    Settings.PRINT_FLAG_MEMBER = false;                                        \
    EXPECT_FALSE(Settings.printObject(Scp));                                   \
  } while (false)

  ScopeAlias Alias;
  Settings.ShowAlias = true;
  Settings.ShowTemplate = true;
  EXPECT_TRUE(Settings.printObject(Alias));
  Settings.ShowAlias = false;
  Settings.ShowTemplate = false;
  EXPECT_FALSE(Settings.printObject(Alias));

  CHECK_SCOPE_PRINT_OPTION(Scope, setIsTemplate, ShowTemplate);
  CHECK_TY_PRINT_OPTION(ScopeTemplatePack, ShowTemplate);
  CHECK_SCOPE_PRINT_OPTION(Scope, setIsBlock, ShowBlock);
  CHECK_SCOPE_PRINT_OPTION(ScopeAggregate, setIsClassType, ShowClass);
  CHECK_TY_PRINT_OPTION(ScopeEnumeration, ShowEnum);
  CHECK_TY_PRINT_OPTION(ScopeFunction, ShowFunction);
  CHECK_TY_PRINT_OPTION(ScopeNamespace, ShowNamespace);
  CHECK_SCOPE_PRINT_OPTION(ScopeAggregate, setIsStructType, ShowStruct);
  CHECK_SCOPE_PRINT_OPTION(ScopeAggregate, setIsUnionType, ShowUnion);
#undef CHECK_SCOPE_PRINT_OPTION

  // Never print arrays.
  ScopeArray ScpArray;
  Settings.showAll();
  EXPECT_FALSE(Settings.printObject(ScpArray));
}

TEST(PrintSettings, PrintObject_Symbol) {
  PrintSettings Settings;
  Settings.showNone();
  Symbol SymMem;
  Symbol SymParam;
  Symbol SymVar;
  SymMem.setIsMember();
  SymParam.setIsParameter();
  SymVar.setIsVariable();

  Settings.ShowMember = true;
  EXPECT_TRUE(Settings.printObject(SymMem));
  Settings.ShowMember = false;
  EXPECT_FALSE(Settings.printObject(SymMem));

  Settings.ShowParameter = true;
  EXPECT_TRUE(Settings.printObject(SymParam));
  Settings.ShowParameter = false;
  EXPECT_FALSE(Settings.printObject(SymParam));

  Settings.ShowVariable = true;
  EXPECT_TRUE(Settings.printObject(SymVar));
  Settings.ShowVariable = false;
  EXPECT_FALSE(Settings.printObject(SymVar));
}

TEST(PrintSettings, PrintObject_Type) {
  PrintSettings Settings;
  Settings.showNone();

  // Create a type and set a flag (e.g. setIsBaseType()).
  // Then set a print setting to true (e.g. ShowPrimitivetype) and check
  // printObject returns true.
  // Then set the same print setting to false and check printObject returns
  // false.
#define CHECK_TYPE_PRINT_OPTION(TY, TYPE_FLAG_METHOD, PRINT_FLAG_MEMBER)       \
  do {                                                                         \
    TY Ty;                                                                     \
    Ty.TYPE_FLAG_METHOD();                                                     \
    Settings.PRINT_FLAG_MEMBER = true;                                         \
    EXPECT_TRUE(Settings.printObject(Ty));                                     \
    Settings.PRINT_FLAG_MEMBER = false;                                        \
    EXPECT_FALSE(Settings.printObject(Ty));                                    \
  } while (false)

  CHECK_TYPE_PRINT_OPTION(Type, setIsBaseType, ShowPrimitiveType);
  CHECK_TYPE_PRINT_OPTION(TypeParam, setIsTemplateTemplate, ShowTemplate);
  CHECK_TYPE_PRINT_OPTION(TypeParam, setIsTemplateType, ShowTemplate);
  CHECK_TYPE_PRINT_OPTION(TypeParam, setIsTemplateValue, ShowTemplate);
  CHECK_TY_PRINT_OPTION(TypeDefinition, ShowAlias);
  CHECK_TYPE_PRINT_OPTION(TypeImport, setIsInheritance, ShowClass);
  CHECK_TYPE_PRINT_OPTION(TypeImport, setIsInheritance, ShowStruct);
  CHECK_TY_PRINT_OPTION(TypeImport, ShowUsing);
  CHECK_TY_PRINT_OPTION(TypeEnumerator, ShowEnum);
#undef CHECK_TYPE_PRINT_OPTION

  // Never print other types.
  Type Ty;
  Settings.showAll();
  EXPECT_FALSE(Settings.printObject(Ty));
  Ty.setIsReferenceType();
  EXPECT_FALSE(Settings.printObject(Ty));
}

#undef CHECK_TY_PRINT_OPTION
