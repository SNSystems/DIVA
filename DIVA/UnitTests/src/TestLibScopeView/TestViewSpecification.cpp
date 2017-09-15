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
/// Tests for LibScopeView::ViewSpecification.
///
//===----------------------------------------------------------------------===//

#include "Line.h"
#include "Scope.h"
#include "Symbol.h"
#include "Type.h"
#include "ViewSpecification.h"

#include "gtest/gtest.h"

using namespace LibScopeView;

TEST(ViewSpecification, PrintObject_Line) {
  ViewSpecification Spec;
  Line Ln;

  // Set brief is really the init function!
  Spec.getOptions().setPrintBrief();
  EXPECT_FALSE(Spec.printObject(&Ln)); // Ensure code lines off by default.

  Spec.getOptions().setPrintNone();

  Spec.getOptions().setPrintCodeline();
  EXPECT_TRUE(Spec.printObject(&Ln));
  Spec.getOptions().resetPrintCodeline();
  EXPECT_FALSE(Spec.printObject(&Ln));
}

TEST(ViewSpecification, PrintObject_Scope) {
  ViewSpecification Spec;
  Spec.getOptions().setPrintNone();
  Scope *Scp;

// Create a scope and set a flag (e.g. setIsTemplateAlias()).
// Then set a print flag to true (e.g. setPrintAlias()) and check PrintObject
// returns true.
// Then set the same print flag to false (e.g. resetPrintAlias()) and check
// PrintObject returns false.
#define CHECK_SCOPE_PRINT_OPTION(SCOPE_FLAG_METHOD, PRINT_FLAG_METHOD)         \
  do {                                                                         \
    Scp = new Scope;                                                           \
    Scp->SCOPE_FLAG_METHOD();                                                  \
    Spec.getOptions().set##PRINT_FLAG_METHOD();                                \
    EXPECT_TRUE(Spec.printObject(Scp));                                        \
    Spec.getOptions().reset##PRINT_FLAG_METHOD();                              \
    EXPECT_FALSE(Spec.printObject(Scp));                                       \
    delete Scp;                                                                \
  } while (false)

  CHECK_SCOPE_PRINT_OPTION(setIsTemplateAlias, PrintAlias);
  CHECK_SCOPE_PRINT_OPTION(setIsTemplate, PrintTemplate);
  CHECK_SCOPE_PRINT_OPTION(setIsTemplatePack, PrintTemplate);
  CHECK_SCOPE_PRINT_OPTION(setIsArrayType, PrintArray);
  CHECK_SCOPE_PRINT_OPTION(setIsBlock, PrintBlock);
  CHECK_SCOPE_PRINT_OPTION(setIsClassType, PrintClass);
  CHECK_SCOPE_PRINT_OPTION(setIsEnumerationType, PrintEnum);
  CHECK_SCOPE_PRINT_OPTION(setIsFunction, PrintFunction);
  CHECK_SCOPE_PRINT_OPTION(setIsNamespace, PrintNamespace);
  CHECK_SCOPE_PRINT_OPTION(setIsStructType, PrintStruct);
  CHECK_SCOPE_PRINT_OPTION(setIsUnionType, PrintUnion);

#undef CHECK_SCOPE_PRINT_OPTION
}

TEST(ViewSpecification, PrintObject_Symbol) {
  ViewSpecification Spec;
  Spec.getOptions().setPrintNone();
  Symbol SymMem;
  Symbol SymParam;
  Symbol SymVar;
  SymMem.setIsMember();
  SymParam.setIsParameter();
  SymVar.setIsVariable();

  Spec.getOptions().setPrintMember();
  EXPECT_TRUE(Spec.printObject(&SymMem));
  Spec.getOptions().resetPrintMember();
  EXPECT_FALSE(Spec.printObject(&SymMem));

  Spec.getOptions().setPrintParameter();
  EXPECT_TRUE(Spec.printObject(&SymParam));
  Spec.getOptions().resetPrintParameter();
  EXPECT_FALSE(Spec.printObject(&SymParam));

  Spec.getOptions().setPrintVariable();
  EXPECT_TRUE(Spec.printObject(&SymVar));
  Spec.getOptions().resetPrintVariable();
  EXPECT_FALSE(Spec.printObject(&SymVar));
}

TEST(ViewSpecification, PrintObject_Type) {
  ViewSpecification Spec;
  Spec.getOptions().setPrintNone();
  Type *Ty;

// Create a type and set a flag (e.g. setIsBaseType()).
// Then set a print flag to true (e.g. setPrintPrimitivetype()) and check
// PrintObject returns true.
// Then set the same print flag to false (e.g. resetPrintPrimitivetype()) and
// check PrintObject returns false.
#define CHECK_TYPE_PRINT_OPTION(SCOPE_FLAG_METHOD, PRINT_FLAG_METHOD)          \
  do {                                                                         \
    Ty = new Type;                                                             \
    Ty->SCOPE_FLAG_METHOD();                                                   \
    Spec.getOptions().set##PRINT_FLAG_METHOD();                                \
    EXPECT_TRUE(Spec.printObject(Ty));                                         \
    Spec.getOptions().reset##PRINT_FLAG_METHOD();                              \
    EXPECT_FALSE(Spec.printObject(Ty));                                        \
    delete Ty;                                                                 \
  } while (false)

  CHECK_TYPE_PRINT_OPTION(setIsBaseType, PrintPrimitivetype);
  CHECK_TYPE_PRINT_OPTION(setIsTemplateTemplate, PrintTemplate);
  CHECK_TYPE_PRINT_OPTION(setIsTemplateType, PrintTemplate);
  CHECK_TYPE_PRINT_OPTION(setIsTemplateValue, PrintTemplate);
  CHECK_TYPE_PRINT_OPTION(setIsTypedef, PrintTypedef);
  CHECK_TYPE_PRINT_OPTION(setIsInheritance, PrintClass);
  CHECK_TYPE_PRINT_OPTION(setIsInheritance, PrintStruct);
  CHECK_TYPE_PRINT_OPTION(setIsImported, PrintUsing);
  CHECK_TYPE_PRINT_OPTION(setIsEnumerator, PrintEnum);
  CHECK_TYPE_PRINT_OPTION(setIsReferenceType, PrintTypes);

#undef CHECK_TYPE_PRINT_OPTION
}
