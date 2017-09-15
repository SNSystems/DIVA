//===-- UnitTests/TestLibScopeView/TestSummaryTable.cpp ---------*- C++ -*-===//
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
/// Tests for LibScopeView::SummaryTable.
///
//===----------------------------------------------------------------------===//

#include "Line.h"
#include "Scope.h"
#include "SummaryTable.h"
#include "Symbol.h"
#include "Type.h"

#include "gtest/gtest.h"

#include <memory>
#include <sstream>

enum ObjectKind : uint32_t {
  Alias,
  Block,
  Class,
  CodeLine,
  CompileUnit,
  Enum,
  Function,
  Member,
  Namespace,
  Parameter,
  PrimitiveType,
  Struct,
  TemplateParameter,
  Union,
  Using,
  Variable,
  ObjectKindSize
};

std::unique_ptr<LibScopeView::Object> GenerateTestObject(uint32_t Kind) {
  switch (Kind) {
  case Alias: {
    auto Scp = std::make_unique<LibScopeView::Scope>();
    Scp->setIsTemplateAlias();
    return std::move(Scp);
  }
  case Block: {
    auto Scp = std::make_unique<LibScopeView::Scope>();
    Scp->setIsBlock();
    return std::move(Scp);
  }
  case Class: {
    auto Scp = std::make_unique<LibScopeView::Scope>();
    Scp->setIsClassType();
    return std::move(Scp);
  }
  case CodeLine: {
    auto Ln = std::make_unique<LibScopeView::Line>();
    Ln->setIsLineRecord();
    return std::move(Ln);
  }
  case CompileUnit: {
    auto Scp = std::make_unique<LibScopeView::Scope>();
    Scp->setIsCompileUnit();
    return std::move(Scp);
  }
  case Enum: {
    auto Scp = std::make_unique<LibScopeView::Scope>();
    Scp->setIsEnumerationType();
    return std::move(Scp);
  }
  case Function: {
    auto Scp = std::make_unique<LibScopeView::Scope>();
    Scp->setIsFunction();
    return std::move(Scp);
  }
  case Member: {
    auto Sym = std::make_unique<LibScopeView::Symbol>();
    Sym->setIsMember();
    return std::move(Sym);
  }
  case Namespace: {
    auto Scp = std::make_unique<LibScopeView::Scope>();
    Scp->setIsNamespace();
    return std::move(Scp);
  }
  case Parameter: {
    auto Sym = std::make_unique<LibScopeView::Symbol>();
    Sym->setIsParameter();
    return std::move(Sym);
  }
  case PrimitiveType: {
    auto Ty = std::make_unique<LibScopeView::Type>();
    Ty->setIsBaseType();
    return std::move(Ty);
  }
  case Struct: {
    auto Scp = std::make_unique<LibScopeView::Scope>();
    Scp->setIsStructType();
    return std::move(Scp);
  }
  case TemplateParameter: {
    auto Ty = std::make_unique<LibScopeView::Type>();
    Ty->setIsTemplateParam();
    Ty->setIsTemplateType();
    return std::move(Ty);
  }
  case Union: {
    auto Scp = std::make_unique<LibScopeView::Scope>();
    Scp->setIsUnionType();
    return std::move(Scp);
  }
  case Using: {
    auto Ty = std::make_unique<LibScopeView::Type>();
    Ty->setIsImported();
    return std::move(Ty);
  }
  case Variable: {
    auto Sym = std::make_unique<LibScopeView::Symbol>();
    Sym->setIsVariable();
    return std::move(Sym);
  }
  }

  return nullptr;
}

TEST(SummaryTable, EmptyStandardSummaryTable) {
  LibScopeView::SummaryTable STab;

  std::stringstream Result;
  STab.getPrintedSummaryTable(Result);

  std::string Expected = "\n"
                         "     -------------------------------------\n"
                         "     Object                 Total  Printed\n"
                         "     -------------------------------------\n"
                         "     Alias                      0        0\n"
                         "     Block                      0        0\n"
                         "     Class                      0        0\n"
                         "     CodeLine                   0        0\n"
                         "     CompileUnit                0        0\n"
                         "     Enum                       0        0\n"
                         "     Function                   0        0\n"
                         "     Member                     0        0\n"
                         "     Namespace                  0        0\n"
                         "     Parameter                  0        0\n"
                         "     PrimitiveType              0        0\n"
                         "     Struct                     0        0\n"
                         "     TemplateParameter          0        0\n"
                         "     Union                      0        0\n"
                         "     Using                      0        0\n"
                         "     Variable                   0        0\n"
                         "     -------------------------------------\n"
                         "     Totals                     0        0\n"
                         "\n";

  EXPECT_EQ(Result.str(), Expected);
}

TEST(SummaryTable, OneIncrementStandardSummaryTable) {
  LibScopeView::SummaryTable STab;

  for (uint32_t Kind = 0; Kind != ObjectKindSize; ++Kind) {
    auto Obj = GenerateTestObject(Kind);
    STab.incrementFound(Obj.get());
    STab.incrementPrinted(Obj.get());
  }

  std::stringstream Result;
  STab.getPrintedSummaryTable(Result);

  std::string Expected = "\n"
                         "     -------------------------------------\n"
                         "     Object                 Total  Printed\n"
                         "     -------------------------------------\n"
                         "     Alias                      1        1\n"
                         "     Block                      1        1\n"
                         "     Class                      1        1\n"
                         "     CodeLine                   1        1\n"
                         "     CompileUnit                1        1\n"
                         "     Enum                       1        1\n"
                         "     Function                   1        1\n"
                         "     Member                     1        1\n"
                         "     Namespace                  1        1\n"
                         "     Parameter                  1        1\n"
                         "     PrimitiveType              1        1\n"
                         "     Struct                     1        1\n"
                         "     TemplateParameter          1        1\n"
                         "     Union                      1        1\n"
                         "     Using                      1        1\n"
                         "     Variable                   1        1\n"
                         "     -------------------------------------\n"
                         "     Totals                    16       16\n"
                         "\n";

  EXPECT_EQ(Result.str(), Expected);
}

TEST(SummaryTable, MultipleIncrementsStandardSummaryTable) {
  LibScopeView::SummaryTable STab;

  for (uint32_t Kind = 0; Kind != ObjectKindSize; ++Kind) {
    auto Obj = GenerateTestObject(Kind);
    STab.incrementFound(Obj.get());
    STab.incrementPrinted(Obj.get());
  }

  const auto IncrementBy = 3;
  for (uint32_t Counter = 0; Counter != IncrementBy; ++Counter) {
    for (uint32_t Kind = ObjectKindSize / 2; Kind != ObjectKindSize; ++Kind) {
      auto Obj = GenerateTestObject(Kind);
      STab.incrementFound(Obj.get());
      STab.incrementPrinted(Obj.get());
    }
  }

  auto Obj = GenerateTestObject(ObjectKind::Block);
  STab.incrementFound(Obj.get());
  STab.incrementPrinted(Obj.get());

  Obj = GenerateTestObject(ObjectKind::Enum);
  STab.incrementFound(Obj.get());
  STab.incrementFound(Obj.get());
  STab.incrementPrinted(Obj.get());

  std::stringstream Result;
  STab.getPrintedSummaryTable(Result);

  std::string Expected = "\n"
                         "     -------------------------------------\n"
                         "     Object                 Total  Printed\n"
                         "     -------------------------------------\n"
                         "     Alias                      1        1\n"
                         "     Block                      2        2\n"
                         "     Class                      1        1\n"
                         "     CodeLine                   1        1\n"
                         "     CompileUnit                1        1\n"
                         "     Enum                       3        2\n"
                         "     Function                   1        1\n"
                         "     Member                     1        1\n"
                         "     Namespace                  4        4\n"
                         "     Parameter                  4        4\n"
                         "     PrimitiveType              4        4\n"
                         "     Struct                     4        4\n"
                         "     TemplateParameter          4        4\n"
                         "     Union                      4        4\n"
                         "     Using                      4        4\n"
                         "     Variable                   4        4\n"
                         "     -------------------------------------\n"
                         "     Totals                    43       42\n"
                         "\n";

  EXPECT_EQ(Result.str(), Expected);
}
