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
#include "PrintSettings.h"
#include "Scope.h"
#include "SummaryTable.h"
#include "Symbol.h"
#include "Type.h"

#include "gtest/gtest.h"

#include <memory>
#include <sstream>

using namespace LibScopeView;

enum ObjectKind {
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

void generateTestObject(ScopeRoot &Root, ObjectKind Kind) {
  switch (Kind) {
  case Alias:
    Root.addChild(new ScopeAlias);
    break;
  case Block: {
    auto *Scp = new Scope;
    Scp->setIsBlock();
    Root.addChild(Scp);
    break;
  }
  case Class: {
    auto *Scp = new ScopeAggregate;
    Scp->setIsClassType();
    Root.addChild(Scp);
    break;
  }
  case CodeLine:
    Root.addChild(new Line);
    break;
  case CompileUnit:
    Root.addChild(new ScopeCompileUnit);
    break;
  case Enum:
    Root.addChild(new ScopeEnumeration);
    break;
  case Function:
    Root.addChild(new ScopeFunction);
    break;
  case Member: {
    auto *Sym = new Symbol;
    Sym->setIsMember();
    Root.addChild(Sym);
    break;
  }
  case Namespace:
    Root.addChild(new ScopeNamespace);
    break;
  case Parameter: {
    auto *Sym = new Symbol;
    Sym->setIsParameter();
    Root.addChild(Sym);
    break;
  }
  case PrimitiveType: {
    auto *Ty = new Type;
    Ty->setIsBaseType();
    Root.addChild(Ty);
    break;
  }
  case Struct: {
    auto *Scp = new ScopeAggregate;
    Scp->setIsStructType();
    Root.addChild(Scp);
    break;
  }
  case TemplateParameter: {
    auto *Ty = new TypeTemplateParam;
    Ty->setIsTemplateType();
    Root.addChild(Ty);
    break;
  }
  case Union: {
    auto *Scp = new ScopeAggregate;
    Scp->setIsUnionType();
    Root.addChild(Scp);
    break;
  }
  case Using:
    Root.addChild(new TypeImport);
    break;
  case Variable: {
    auto *Sym = new Symbol;
    Sym->setIsVariable();
    Root.addChild(Sym);
    break;
  }
  case ObjectKindSize:
    assert(false && "Unreachable");
    break;
  }
}

TEST(SummaryTable, EmptySummaryTable) {
  ScopeRoot Root;

  SummaryTable STab(Root, nullptr);

  std::stringstream Result;
  STab.printSummaryTable(Result);

  std::string Expected = "     -------------------------------------\n"
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

TEST(SummaryTable, OneIncrementSummaryTable) {
  ScopeRoot Root;

  for (uint32_t Kind = 0; Kind != ObjectKindSize; ++Kind)
    generateTestObject(Root, ObjectKind(Kind));

  SummaryTable STab(Root, nullptr);

  std::stringstream Result;
  STab.printSummaryTable(Result);

  std::string Expected = "     -------------------------------------\n"
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

TEST(SummaryTable, MultipleIncrementsSummaryTable) {
  ScopeRoot Root;

  for (uint32_t Kind = 0; Kind != ObjectKindSize; ++Kind)
    generateTestObject(Root, ObjectKind(Kind));

  const auto IncrementBy = 3;
  for (uint32_t Counter = 0; Counter != IncrementBy; ++Counter)
    for (uint32_t Kind = ObjectKindSize / 2; Kind != ObjectKindSize; ++Kind)
      generateTestObject(Root, ObjectKind(Kind));

  generateTestObject(Root, ObjectKind::Block);
  generateTestObject(Root, ObjectKind::Enum);
  generateTestObject(Root, ObjectKind::Enum);

  SummaryTable STab(Root, nullptr);

  std::stringstream Result;
  STab.printSummaryTable(Result);

  std::string Expected = "     -------------------------------------\n"
                         "     Object                 Total  Printed\n"
                         "     -------------------------------------\n"
                         "     Alias                      1        1\n"
                         "     Block                      2        2\n"
                         "     Class                      1        1\n"
                         "     CodeLine                   1        1\n"
                         "     CompileUnit                1        1\n"
                         "     Enum                       3        3\n"
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
                         "     Totals                    43       43\n"
                         "\n";

  EXPECT_EQ(Result.str(), Expected);
}

TEST(SummaryTable, PrintSettingsSummaryTable) {
  ScopeRoot Root;

  for (uint32_t Kind = 0; Kind != ObjectKindSize; ++Kind)
    generateTestObject(Root, ObjectKind(Kind));

  PrintSettings Settings;
  Settings.ShowAlias = true;
  Settings.ShowBlock = false;
  Settings.ShowClass = true;
  Settings.ShowCodeline = false;
  // CompileUnits are always shown
  Settings.ShowEnum = false;
  Settings.ShowFunction = true;
  Settings.ShowMember = false;
  Settings.ShowNamespace = true;
  Settings.ShowParameter = false;
  Settings.ShowPrimitiveType = true;
  Settings.ShowStruct = false;
  Settings.ShowTemplate = true;
  Settings.ShowUnion = false;
  Settings.ShowUsing = true;
  Settings.ShowVariable = false;

  SummaryTable STab(Root, &Settings);

  std::stringstream Result;
  STab.printSummaryTable(Result);

  std::string Expected = "     -------------------------------------\n"
                         "     Object                 Total  Printed\n"
                         "     -------------------------------------\n"
                         "     Alias                      1        1\n"
                         "     Block                      1        0\n"
                         "     Class                      1        1\n"
                         "     CodeLine                   1        0\n"
                         "     CompileUnit                1        1\n"
                         "     Enum                       1        0\n"
                         "     Function                   1        1\n"
                         "     Member                     1        0\n"
                         "     Namespace                  1        1\n"
                         "     Parameter                  1        0\n"
                         "     PrimitiveType              1        1\n"
                         "     Struct                     1        0\n"
                         "     TemplateParameter          1        1\n"
                         "     Union                      1        0\n"
                         "     Using                      1        1\n"
                         "     Variable                   1        0\n"
                         "     -------------------------------------\n"
                         "     Totals                    16        8\n"
                         "\n";

  EXPECT_EQ(Result.str(), Expected);
}
