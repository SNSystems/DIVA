//===-- UnitTests/TestLibScopeView/TestScopePrinter.cpp ---------*- C++ -*-===//
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
/// Tests for LibScopeView::ScopePrinter.
///
//===----------------------------------------------------------------------===//

#include "FileUtilities.h"
#include "Scope.h"
#include "ScopePrinter.h"
#include "UtilsForTesting.h"

#include "gtest/gtest.h"

using namespace LibScopeView;

namespace {

// Test printer that prints the name of an object and its children.
class TestNamePrinter : public ScopePrinter {
public:
  using ScopePrinter::ScopePrinter;
private:
  void printImpl(const Object *Obj, std::ostream &OutputStream) override {
    OutputStream << Obj->getName() << '\n';
    printChildren(Obj);
  }
  const std::string &getFileExtension() override {
    static std::string Ext = "txt";
    return Ext;
  }
  const std::string &getHeader() override {
    static std::string Header = "HEADER\n";
    return Header;
  }
  const std::string &getFooter() override {
    static std::string Footer = "FOOTER\n";
    return Footer;
  }
};

} // end anonymous namespace

TEST(ScopePrinter, StandardPrint) {
  std::stringstream Output;
  Scope Scp1;
  Scp1.setName("Scope1");
  auto *Scp2 = new Scope;
  Scp2->setName("Scope2");
  Scp1.addObject(Scp2);

  TestNamePrinter Printer;
  Printer.print(&Scp1, Output);
  EXPECT_EQ(Output.str(), "HEADER\nScope1\nScope2\nFOOTER\n");
}

TEST(ScopePrinter, SplitPrint) {
  ScopeRoot Root;
  Root.setName("SHOULD NOT PRINT");

  auto *CU1 = new ScopeCompileUnit;
  Root.addObject(CU1);
  CU1->setIsCompileUnit();
  CU1->setName("test/cu/1");
  Scope *Child1 = new Scope;
  Scope *Child2 = new Scope;
  Child1->setName("Child1");
  Child2->setName("Child2");
  CU1->addObject(Child1);
  CU1->addObject(Child2);

  auto *CU2 = new ScopeCompileUnit;
  Root.addObject(CU2);
  CU2->setIsCompileUnit();
  CU2->setName("test.cu.2");
  Scope *Child3 = new Scope;
  Scope *Child4 = new Scope;
  Child3->setName("Child3");
  Child4->setName("Child4");
  CU2->addObject(Child3);
  CU2->addObject(Child4);

  std::string CUFilename1("test_cu_1.txt");
  std::string CUFilename2("test_cu_2.txt");

  clearTestOutputFile(CUFilename1);
  clearTestOutputFile(CUFilename2);

  TestNamePrinter().print(&Root, getTestOutputDir());

  ASSERT_TRUE(doesFileExist(getTestOutputFilePath(CUFilename1)));
  EXPECT_EQ(readTestOutputFile(CUFilename1),
            "HEADER\ntest/cu/1\nChild1\nChild2\nFOOTER\n");

  ASSERT_TRUE(doesFileExist(getTestOutputFilePath(CUFilename2)));
  EXPECT_EQ(readTestOutputFile(CUFilename2),
            "HEADER\ntest.cu.2\nChild3\nChild4\nFOOTER\n");
}
