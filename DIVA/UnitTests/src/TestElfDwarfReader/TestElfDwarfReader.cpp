//===-- ElfReader/ElfDwarfReader.h ------------------------------*- C++ -*-===//
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
/// This file contains the tests for EldDwarfReader.
///
//===----------------------------------------------------------------------===//

#include "ElfDwarfReader.h"
#include "FileUtilities.h"
#include "Line.h"
#include "Symbol.h"
#include "Type.h"
#include "UtilsForTesting.h"

#include "dwarf.h"
#include "gtest/gtest.h"

#include <memory>

using namespace ElfDwarfReader;

using testing::AssertionResult;

// IMPORTANT.
//
// These tests depend on the DWARF in the elf files in
// TestInputs/ElfDwarfReader. Changing these elf files in any way may cause
// these tests to fail.

// Reading of the following DWARF tags is not currently tested due to lack of
// appropriate DWARF test data:
//
// DWARF_TAG_catch_block
// DWARF_TAG_entry_point
// DWARF_TAG_inlined_subroutine
// DWARF_TAG_ptr_to_member
// DWARF_TAG_template_alias
// DWARF_TAG_try_block
// DWARF_TAG_unspecified_type

// Test helper functions and fixtures.
namespace {

AssertionResult checkChildCount(LibScopeView::Scope *Scp, size_t ScopeCount,
                                size_t TypeCount, size_t SymbolCount) {
  if (Scp->getScopeCount() != ScopeCount)
    return ::testing::AssertionFailure()
           << "Scope contained " << Scp->getScopeCount() << " Scopes, expected "
           << ScopeCount;
  if (Scp->getTypeCount() != TypeCount)
    return ::testing::AssertionFailure()
           << "Scope contained " << Scp->getTypeCount() << " Types, expected "
           << TypeCount;
  if (Scp->getSymbolCount() != SymbolCount)
    return ::testing::AssertionFailure()
           << "Scope contained " << Scp->getSymbolCount()
           << " Symbols, expected " << SymbolCount;
  if (Scp->getChildrenCount() != ScopeCount + TypeCount + SymbolCount)
    return ::testing::AssertionFailure()
           << "ChildrenCount != ScopeCount + TypeCount + SymbolCount";

  return ::testing::AssertionSuccess();
}

std::string getSourceFileName(LibScopeView::Element *E, bool Format = true) {
  return E->getFileName(Format);
}

// Test fixture providing helpers to load a Scope tree using the DwarfReader.
class TestElfDwarfReader : public ::testing::Test {
public:
  AssertionResult loadRootFromTestFile(std::string TestFile,
                                       LibScopeView::Scope **Root,
                                       LibScopeView::CmdOptions &Options) {
    if (!LibScopeView::doesFileExist(getTestInputFilePath(TestFile)))
      return ::testing::AssertionFailure() << "Test file does not exist";

    LibScopeView::ViewSpecification Spec(Options);
    Spec.setInputFile(getTestInputFilePath(TestFile));
    Reader = std::unique_ptr<DwarfReader>(new DwarfReader(&Spec));
    Reader->getOptions().setFormatFileName();

    if (!Reader->executeActions())
      return ::testing::AssertionFailure() << "Failed to load test file";

    *Root = Reader->getScopesRoot();
    if (!(*Root)->getIsRoot())
      return ::testing::AssertionFailure()
             << "Root in test file was not a ScopeRoot";

    return ::testing::AssertionSuccess();
  }

  AssertionResult loadRootFromTestFile(std::string TestFile,
                                       LibScopeView::Scope **Root) {
    LibScopeView::CmdOptions Options;
    return loadRootFromTestFile(TestFile, Root, Options);
  }

  AssertionResult loadSingleCUFromTestFile(std::string TestFile,
                                           LibScopeView::Scope **CU,
                                           LibScopeView::CmdOptions &Options) {
    LibScopeView::Scope *Root = nullptr;
    AssertionResult Res = loadRootFromTestFile(TestFile, &Root, Options);
    if (!Res)
      return Res;
    if (Root->getScopeCount() == 0)
      return ::testing::AssertionFailure()
             << "Test file did not contain any CUs";
    if (Root->getScopeCount() > 1)
      return ::testing::AssertionFailure()
             << "Test file contained more than 1 CU";

    *CU = Root->getScopeAt(0);
    if (!(*CU)->getIsCompileUnit())
      return ::testing::AssertionFailure()
             << "Top level scope in test file was not a CU";

    return ::testing::AssertionSuccess();
  }

  AssertionResult loadSingleCUFromTestFile(std::string TestFile,
                                           LibScopeView::Scope **CU) {
    LibScopeView::CmdOptions Options;
    return loadSingleCUFromTestFile(TestFile, CU, Options);
  }

  LibScopeView::Reader &getReader() { return *Reader; }

private:
  std::unique_ptr<DwarfReader> Reader;
};

} // namespace

TEST_F(TestElfDwarfReader, ReadStructure) {
  // Check for the following tree:
  // CompileUnit
  //   Function
  //   BaseType
  // CompileUnit
  //   Variable
  //   Class
  //     Function
  //       Parameter
  //   TypePointer
  // CompileUnit
  //   Variable
  //   BaseType

  LibScopeView::Scope *Root = nullptr;
  ASSERT_TRUE(loadRootFromTestFile("ElfDwarfReader/structure.elf", &Root));

  ASSERT_TRUE(checkChildCount(Root, 3, 0, 0));

  auto CU1 = Root->getScopes().at(0);
  ASSERT_TRUE(checkChildCount(CU1, 1, 1, 0));
  EXPECT_TRUE(checkChildCount(CU1->getScopeAt(0), 0, 0, 0));
  EXPECT_TRUE(CU1->getIsCompileUnit());
  EXPECT_TRUE(CU1->getScopeAt(0)->getIsFunction());
  EXPECT_TRUE(CU1->getTypes().at(0)->getIsBaseType());

  auto CU2 = Root->getScopes().at(1);
  ASSERT_TRUE(checkChildCount(CU2, 1, 1, 1));
  ASSERT_TRUE(checkChildCount(CU2->getScopeAt(0), 1, 0, 0));
  ASSERT_TRUE(checkChildCount(CU2->getScopeAt(0)->getScopeAt(0), 0, 0, 1));
  EXPECT_TRUE(CU2->getIsCompileUnit());
  EXPECT_TRUE(CU2->getSymbolAt(0)->getIsVariable());
  EXPECT_TRUE(CU2->getScopeAt(0)->getIsClassType());
  EXPECT_TRUE(CU2->getScopeAt(0)->getScopeAt(0)->getIsFunction());
  EXPECT_TRUE(
      CU2->getScopeAt(0)->getScopeAt(0)->getSymbolAt(0)->getIsParameter());
  EXPECT_TRUE(CU2->getTypes().at(0)->getIsPointerType());

  auto CU3 = Root->getScopes().at(2);
  ASSERT_TRUE(checkChildCount(CU3, 0, 1, 1));
  EXPECT_TRUE(CU3->getIsCompileUnit());
  EXPECT_TRUE(CU3->getSymbolAt(0)->getIsVariable());
  EXPECT_TRUE(CU3->getTypes().at(0)->getIsBaseType());
}

TEST_F(TestElfDwarfReader, ReadCompileUnits) {
  LibScopeView::Scope *Root = nullptr;
  ASSERT_TRUE(loadRootFromTestFile("ElfDwarfReader/structure.elf", &Root));
  ASSERT_TRUE(checkChildCount(Root, 3, 0, 0));

  auto CU1 = Root->getScopes().at(0);
  auto CU2 = Root->getScopes().at(1);
  auto CU3 = Root->getScopes().at(2);

  EXPECT_TRUE(CU1->getIsCompileUnit());
  EXPECT_EQ(CU1->getDieOffset(), 0x0bU);
  EXPECT_EQ(CU1->getDieTag(), DW_TAG_compile_unit);
  EXPECT_STREQ(CU1->getName(), "structure1.cpp");
  EXPECT_EQ(CU1->getLineNumber(), 0U);
  EXPECT_EQ(dynamic_cast<LibScopeView::Object *>(CU1)->getFileName(false), "");

  EXPECT_TRUE(CU2->getIsCompileUnit());
  EXPECT_EQ(CU2->getDieOffset(), 0x56U);
  EXPECT_EQ(CU2->getDieTag(), DW_TAG_compile_unit);
  EXPECT_STREQ(CU2->getName(), "structure2.cpp");
  EXPECT_EQ(CU2->getLineNumber(), 0U);
  EXPECT_EQ(dynamic_cast<LibScopeView::Object *>(CU2)->getFileName(false), "");

  EXPECT_TRUE(CU3->getIsCompileUnit());
  EXPECT_EQ(CU3->getDieOffset(), 0x0a9U);
  EXPECT_EQ(CU3->getDieTag(), DW_TAG_compile_unit);
  EXPECT_STREQ(CU3->getName(), "structure3.cpp");
  EXPECT_EQ(CU3->getLineNumber(), 0U);
  EXPECT_EQ(dynamic_cast<LibScopeView::Object *>(CU3)->getFileName(false), "");
}

TEST_F(TestElfDwarfReader, ReadAggregate) {
  LibScopeView::Scope *CU = nullptr;
  ASSERT_TRUE(loadSingleCUFromTestFile("ElfDwarfReader/aggregate.o", &CU));
  ASSERT_EQ(CU->getScopeCount(), 4U);

  // DW_TAG_class_type
  auto Class = CU->getScopeAt(0);
  EXPECT_TRUE(Class->getIsClassType());
  EXPECT_EQ(Class->getDieOffset(), 0x0033U);
  EXPECT_EQ(Class->getDieTag(), DW_TAG_class_type);
  EXPECT_EQ(Class->getLineNumber(), 1U);
  EXPECT_EQ(getSourceFileName(Class), "aggregate.cpp");
  EXPECT_STREQ(Class->getName(), "A");

  // DW_TAG_struct_type
  auto Struct = CU->getScopeAt(2);
  EXPECT_TRUE(Struct->getIsStructType());
  EXPECT_EQ(Struct->getDieOffset(), 0x0095U);
  EXPECT_EQ(Struct->getDieTag(), DW_TAG_structure_type);
  EXPECT_EQ(Struct->getLineNumber(), 9U);
  EXPECT_EQ(getSourceFileName(Struct), "aggregate.cpp");
  EXPECT_STREQ(Struct->getName(), "C");

  // DW_TAG_union_type
  auto Union = CU->getScopeAt(3);
  EXPECT_TRUE(Union->getIsUnionType());
  EXPECT_EQ(Union->getDieOffset(), 0x00bfU);
  EXPECT_EQ(Union->getDieTag(), DW_TAG_union_type);
  EXPECT_EQ(Union->getLineNumber(), 13U);
  EXPECT_EQ(getSourceFileName(Union), "aggregate.cpp");
  EXPECT_STREQ(Union->getName(), "D");

  // DW_TAG_inheritance
  auto SubClass = CU->getScopeAt(1);
  ASSERT_EQ(SubClass->getTypeCount(), 1U);
  EXPECT_TRUE(SubClass->getTypes().at(0)->getIsInheritance());
  EXPECT_EQ(SubClass->getTypes().at(0)->getDieOffset(), 0x006cU);
  EXPECT_EQ(SubClass->getTypes().at(0)->getDieTag(), DW_TAG_inheritance);
  EXPECT_EQ(SubClass->getTypes().at(0)->getType(), Class);
}

TEST_F(TestElfDwarfReader, ReadArray) {
  LibScopeView::Scope *CU = nullptr;
  ASSERT_TRUE(loadSingleCUFromTestFile("ElfDwarfReader/array.o", &CU));
  ASSERT_TRUE(checkChildCount(CU, 1, 2, 1));

  auto IntType = CU->getTypes().at(0);
  auto SizeType = CU->getTypes().at(1);

  // DW_TAG_array_type
  auto Array = CU->getScopeAt(0);
  EXPECT_TRUE(Array->getIsArrayType());
  EXPECT_EQ(Array->getDieOffset(), 0x0033U);
  EXPECT_EQ(Array->getDieTag(), DW_TAG_array_type);
  EXPECT_EQ(Array->getType(), IntType);

  // DW_TAG_subrange_type
  ASSERT_TRUE(checkChildCount(Array, 0, 2, 0));
  EXPECT_TRUE(Array->getTypes().at(0)->getIsSubrangeType());
  EXPECT_EQ(Array->getTypes().at(0)->getDieOffset(), 0x0038U);
  EXPECT_EQ(Array->getTypes().at(0)->getDieTag(), DW_TAG_subrange_type);
  EXPECT_EQ(Array->getTypes().at(0)->getType(), SizeType);

  // Name resolution.
  EXPECT_STREQ(Array->getName(), "int [5][10]");
}

TEST_F(TestElfDwarfReader, ReadBlocks) {
  LibScopeView::Scope *CU = nullptr;
  ASSERT_TRUE(loadSingleCUFromTestFile("ElfDwarfReader/block.o", &CU));
  ASSERT_EQ(CU->getScopeCount(), 1U);
  ASSERT_EQ(CU->getScopeAt(0)->getScopeCount(), 3U);

  // TODO: Test try/catch when DWARF input is available.

  // DW_TAG_lexical_block
  EXPECT_TRUE(CU->getScopeAt(0)->getScopeAt(2)->getIsBlock());
  EXPECT_TRUE(CU->getScopeAt(0)->getScopeAt(2)->getIsLexicalBlock());
  EXPECT_EQ(CU->getScopeAt(0)->getScopeAt(2)->getDieOffset(), 0x007bU);
  EXPECT_EQ(CU->getScopeAt(0)->getScopeAt(2)->getDieTag(),
            DW_TAG_lexical_block);
}

TEST_F(TestElfDwarfReader, ReadEnum) {
  LibScopeView::Scope *CU = nullptr;
  ASSERT_TRUE(loadSingleCUFromTestFile("ElfDwarfReader/enum.o", &CU));
  ASSERT_TRUE(checkChildCount(CU, 3, 2, 3));
  ASSERT_EQ(CU->getScopeAt(0)->getTypeCount(), 3U);
  ASSERT_EQ(CU->getScopeAt(1)->getTypeCount(), 3U);
  ASSERT_EQ(CU->getScopeAt(2)->getTypeCount(), 3U);

  auto Enum1 = CU->getScopeAt(0);
  auto Enum1Value1 = Enum1->getTypes().at(0);
  auto Enum1Value2 = Enum1->getTypes().at(1);
  auto Enum1Value3 = Enum1->getTypes().at(2);
  auto Enum2 = CU->getScopeAt(1);
  auto Enum2Value1 = Enum2->getTypes().at(0);
  auto Enum2Value2 = Enum2->getTypes().at(1);
  auto Enum2Value3 = Enum2->getTypes().at(2);
  auto Enum3 = CU->getScopeAt(2);
  auto Enum3Value1 = Enum3->getTypes().at(0);
  auto Enum3Value2 = Enum3->getTypes().at(1);
  auto Enum3Value3 = Enum3->getTypes().at(2);

  auto ShortType = CU->getTypes().at(0);
  auto IntType = CU->getTypes().at(1);

  // Enumeration (1)
  EXPECT_TRUE(Enum1->getIsEnumerationType());
  EXPECT_EQ(Enum1->getDieOffset(), 0x0033U);
  EXPECT_EQ(Enum1->getDieTag(), DW_TAG_enumeration_type);
  EXPECT_EQ(Enum1->getLineNumber(), 1U);
  EXPECT_EQ(getSourceFileName(Enum1), "enum.cpp");
  EXPECT_STREQ(Enum1->getName(), "E1");
  EXPECT_EQ(Enum1->getType(), nullptr);
  EXPECT_EQ(dynamic_cast<LibScopeView::ScopeEnumeration *>(Enum1)->getIsClass(),
            false);

  EXPECT_TRUE(Enum1Value1->getIsEnumerator());
  EXPECT_EQ(Enum1Value1->getDieOffset(), 0x003bU);
  EXPECT_EQ(Enum1Value1->getDieTag(), DW_TAG_enumerator);
  EXPECT_STREQ(Enum1Value1->getName(), "A");
  EXPECT_STREQ(Enum1Value1->getValue(), "0");

  EXPECT_TRUE(Enum1Value2->getIsEnumerator());
  EXPECT_EQ(Enum1Value2->getDieOffset(), 0x0041U);
  EXPECT_EQ(Enum1Value2->getDieTag(), DW_TAG_enumerator);
  EXPECT_STREQ(Enum1Value2->getName(), "B");
  EXPECT_STREQ(Enum1Value2->getValue(), "1");

  EXPECT_TRUE(Enum1Value3->getIsEnumerator());
  EXPECT_EQ(Enum1Value3->getDieOffset(), 0x0047U);
  EXPECT_EQ(Enum1Value3->getDieTag(), DW_TAG_enumerator);
  EXPECT_STREQ(Enum1Value3->getName(), "C");
  EXPECT_STREQ(Enum1Value3->getValue(), "2");

  // Enumeration (2)
  EXPECT_TRUE(Enum2->getIsEnumerationType());
  EXPECT_EQ(Enum2->getDieOffset(), 0x0063U);
  EXPECT_EQ(Enum2->getDieTag(), DW_TAG_enumeration_type);
  EXPECT_EQ(Enum2->getLineNumber(), 7U);
  EXPECT_EQ(getSourceFileName(Enum2), "enum.cpp");
  EXPECT_STREQ(Enum2->getName(), "E2");
  EXPECT_EQ(Enum2->getType(), ShortType);
  EXPECT_EQ(dynamic_cast<LibScopeView::ScopeEnumeration *>(Enum2)->getIsClass(),
            true);

  EXPECT_TRUE(Enum2Value1->getIsEnumerator());
  EXPECT_EQ(Enum2Value1->getDieOffset(), 0x006fU);
  EXPECT_EQ(Enum2Value1->getDieTag(), DW_TAG_enumerator);
  EXPECT_STREQ(Enum2Value1->getName(), "G");
  EXPECT_STREQ(Enum2Value1->getValue(), "-1");

  EXPECT_TRUE(Enum2Value2->getIsEnumerator());
  EXPECT_EQ(Enum2Value2->getDieOffset(), 0x0075U);
  EXPECT_EQ(Enum2Value2->getDieTag(), DW_TAG_enumerator);
  EXPECT_STREQ(Enum2Value2->getName(), "H");
  EXPECT_STREQ(Enum2Value2->getValue(), "0");

  EXPECT_TRUE(Enum2Value3->getIsEnumerator());
  EXPECT_EQ(Enum2Value3->getDieOffset(), 0x007bU);
  EXPECT_EQ(Enum2Value3->getDieTag(), DW_TAG_enumerator);
  EXPECT_STREQ(Enum2Value3->getName(), "I");
  EXPECT_STREQ(Enum2Value3->getValue(), "1");

  // Enumeration (3)
  EXPECT_TRUE(Enum3->getIsEnumerationType());
  EXPECT_EQ(Enum3->getDieOffset(), 0x009eU);
  EXPECT_EQ(Enum3->getDieTag(), DW_TAG_enumeration_type);
  EXPECT_EQ(Enum3->getLineNumber(), 13U);
  EXPECT_EQ(getSourceFileName(Enum3), "enum.cpp");
  EXPECT_STREQ(Enum3->getName(), "E3");
  EXPECT_EQ(Enum3->getType(), IntType);
  EXPECT_EQ(dynamic_cast<LibScopeView::ScopeEnumeration *>(Enum3)->getIsClass(),
            true);

  EXPECT_TRUE(Enum3Value1->getIsEnumerator());
  EXPECT_EQ(Enum3Value1->getDieOffset(), 0x00aaU);
  EXPECT_EQ(Enum3Value1->getDieTag(), DW_TAG_enumerator);
  EXPECT_STREQ(Enum3Value1->getName(), "D");
  EXPECT_STREQ(Enum3Value1->getValue(), "1");

  EXPECT_TRUE(Enum3Value2->getIsEnumerator());
  EXPECT_EQ(Enum3Value2->getDieOffset(), 0x00b0U);
  EXPECT_EQ(Enum3Value2->getDieTag(), DW_TAG_enumerator);
  EXPECT_STREQ(Enum3Value2->getName(), "E");
  EXPECT_STREQ(Enum3Value2->getValue(), "2");

  EXPECT_TRUE(Enum3Value3->getIsEnumerator());
  EXPECT_EQ(Enum3Value3->getDieOffset(), 0x00b6U);
  EXPECT_EQ(Enum3Value3->getDieTag(), DW_TAG_enumerator);
  EXPECT_STREQ(Enum3Value3->getName(), "F");
  EXPECT_STREQ(Enum3Value3->getValue(), "3");
}

TEST_F(TestElfDwarfReader, ReadFunction) {
  LibScopeView::Scope *CU = nullptr;
  ASSERT_TRUE(loadSingleCUFromTestFile("ElfDwarfReader/function.o", &CU));
  ASSERT_TRUE(checkChildCount(CU, 3, 2, 0));

  auto IntType = CU->getTypes().at(0);

  // DW_TAG_subprogram
  EXPECT_TRUE(CU->getScopeAt(0)->getIsFunction());
  EXPECT_TRUE(CU->getScopeAt(0)->getIsSubprogram());
  EXPECT_EQ(CU->getScopeAt(0)->getDieOffset(), 0x002aU);
  EXPECT_EQ(CU->getScopeAt(0)->getDieTag(), DW_TAG_subprogram);
  EXPECT_EQ(CU->getScopeAt(0)->getLineNumber(), 1U);
  EXPECT_EQ(getSourceFileName(CU->getScopeAt(0)), "function.cpp");
  EXPECT_STREQ(CU->getScopeAt(0)->getName(), "func1");
  EXPECT_EQ(CU->getScopeAt(0)->getType(), IntType);

  // DW_TAG_subroutine_type
  EXPECT_TRUE(CU->getScopeAt(2)->getIsFunction());
  EXPECT_TRUE(CU->getScopeAt(2)->getIsSubroutineType());
  EXPECT_EQ(CU->getScopeAt(2)->getDieOffset(), 0x0098U);
  EXPECT_EQ(CU->getScopeAt(2)->getDieTag(), DW_TAG_subroutine_type);
  EXPECT_EQ(CU->getScopeAt(2)->getType(), IntType);
  EXPECT_STREQ(CU->getScopeAt(2)->getName(), "int (*)(int)");

  // Pointers to DW_TAG_subroutine_type.
  CU = nullptr;
  ASSERT_TRUE(
      loadSingleCUFromTestFile("ElfDwarfReader/function_pointer.o", &CU));
  ASSERT_TRUE(checkChildCount(CU, 2, 5, 0));

  EXPECT_TRUE(CU->getTypes().at(0)->getIsTypedef());
  EXPECT_STREQ(CU->getTypes().at(0)->getName(), "ptr_to_fptr");
  EXPECT_STREQ(CU->getTypes().at(0)->getTypeName(), "INT (*)(INT,int) * *");

  // DW_AT_specification, Definition should reference declaration.
  CU = nullptr;
  ASSERT_TRUE(loadSingleCUFromTestFile("ElfDwarfReader/function_decls.o", &CU));
  ASSERT_TRUE(checkChildCount(CU, 2, 3, 0));
  ASSERT_TRUE(checkChildCount(CU->getScopeAt(0), 1, 0, 0));

  auto Decl = CU->getScopeAt(0)->getScopeAt(0);
  auto Defin = CU->getScopeAt(1);

  EXPECT_TRUE(Decl->getIsFunction());
  auto DeclFunc = dynamic_cast<LibScopeView::ScopeFunction *>(Decl);
  EXPECT_TRUE(Defin->getIsFunction());
  auto DefinFunc = dynamic_cast<LibScopeView::ScopeFunction *>(Defin);

  EXPECT_TRUE(DeclFunc->getIsDeclaration());
  EXPECT_FALSE(DefinFunc->getIsDeclaration());

  EXPECT_EQ(Defin->getReference(), Decl);
  EXPECT_STREQ(Defin->getName(), Decl->getName());
  EXPECT_EQ(Defin->getType(), Decl->getType());
  EXPECT_EQ(Defin->getLineNumber(), Decl->getLineNumber());
  EXPECT_EQ(Defin->getFileNameIndex(), Decl->getFileNameIndex());

  // Static and inlined
  CU = nullptr;
  ASSERT_TRUE(
      loadSingleCUFromTestFile("ElfDwarfReader/function_static_inline.o", &CU));
  ASSERT_TRUE(checkChildCount(CU, 5, 1, 0));

  auto Static = CU->getScopeAt(2);
  EXPECT_TRUE(Static->getIsFunction());
  EXPECT_STREQ(Static->getName(), "func1");
  auto StaticFunc = dynamic_cast<LibScopeView::ScopeFunction *>(Static);
  EXPECT_TRUE(StaticFunc->getIsStatic());
  EXPECT_FALSE(StaticFunc->getIsDeclaredInline());

  auto Inlined = CU->getScopeAt(0);
  EXPECT_TRUE(Inlined->getIsFunction());
  EXPECT_STREQ(Inlined->getName(), "func2");
  auto InlinedFunc = dynamic_cast<LibScopeView::ScopeFunction *>(Inlined);
  EXPECT_FALSE(InlinedFunc->getIsStatic());
  EXPECT_TRUE(InlinedFunc->getIsDeclaredInline());

  auto StaticInlined = CU->getScopeAt(1);
  EXPECT_TRUE(StaticInlined->getIsFunction());
  EXPECT_STREQ(StaticInlined->getName(), "func3");
  auto StaticInlinedFunc =
      dynamic_cast<LibScopeView::ScopeFunction *>(StaticInlined);
  EXPECT_TRUE(StaticInlinedFunc->getIsStatic());
  EXPECT_TRUE(StaticInlinedFunc->getIsDeclaredInline());

  // Check static is inherited from declaration
  auto StaticDef = CU->getScopeAt(3);
  EXPECT_TRUE(StaticDef->getIsFunction());
  EXPECT_STREQ(StaticDef->getName(), "func4");
  auto StaticDefFunc = dynamic_cast<LibScopeView::ScopeFunction *>(StaticDef);
  EXPECT_FALSE(StaticDefFunc->getIsDeclaration());
  EXPECT_TRUE(StaticDefFunc->getIsStatic());
}

TEST_F(TestElfDwarfReader, ReadGlobals) {
  LibScopeView::Scope *Root = nullptr;
  ASSERT_TRUE(loadRootFromTestFile("ElfDwarfReader/lto_cross_cu.elf", &Root));
  ASSERT_TRUE(checkChildCount(Root, 2, 0, 0));

  // CU1 should contain a struct (A) containing another struct (G). G should be
  // marked as global as there is a reference to it in CU2. The member of G
  // should also be marked as global.
  auto CU1 = Root->getScopeAt(0);
  ASSERT_TRUE(checkChildCount(CU1, 3, 1, 0));
  ASSERT_TRUE(checkChildCount(CU1->getScopeAt(2), 1, 0, 0));
  ASSERT_TRUE(checkChildCount(CU1->getScopeAt(2)->getScopeAt(0), 0, 0, 1));
  auto StructG = CU1->getScopeAt(2)->getScopeAt(0);
  EXPECT_TRUE(StructG->getIsGlobalReference());
  EXPECT_TRUE(StructG->getSymbolAt(0)->getIsGlobalReference());

  // The function in CU2 should be of type G.
  auto CU2 = Root->getScopeAt(1);
  ASSERT_TRUE(checkChildCount(CU2, 1, 0, 0));
  EXPECT_EQ(CU2->getScopeAt(0)->getType(), StructG);
}

TEST_F(TestElfDwarfReader, ReadImport) {
  LibScopeView::Scope *CU = nullptr;
  ASSERT_TRUE(loadSingleCUFromTestFile("ElfDwarfReader/import.o", &CU));
  ASSERT_TRUE(checkChildCount(CU, 4, 2, 1));

  auto Struct = CU->getScopeAt(1);
  ASSERT_EQ(Struct->getTypeCount(), 2U);

  // DW_TAG_imported_module
  EXPECT_TRUE(CU->getTypes().at(1)->getIsImportedModule());
  EXPECT_EQ(CU->getTypes().at(1)->getDieOffset(), 0x007aU);
  EXPECT_EQ(CU->getTypes().at(1)->getLineNumber(), 14U);
  EXPECT_EQ(getSourceFileName(CU->getTypes().at(1)), "import.cpp");
  EXPECT_EQ(CU->getTypes().at(1)->getDieTag(), DW_TAG_imported_module);
  // Check the imported namespace.
  ASSERT_NE(CU->getTypes().at(1)->getType(), nullptr);
  ASSERT_TRUE(CU->getTypes().at(1)->getType()->getIsScope());
  auto *ImpModule =
      dynamic_cast<LibScopeView::Scope *>(CU->getTypes().at(1)->getType());
  EXPECT_EQ(ImpModule->getDieOffset(), 0x62U);
  EXPECT_TRUE(ImpModule->getIsNamespace());
  EXPECT_STREQ(ImpModule->getName(), "NS");

  // DW_TAG_imported_declaration
  EXPECT_TRUE(Struct->getTypes().at(0)->getIsImportedDeclaration());
  EXPECT_EQ(Struct->getTypes().at(0)->getDieOffset(), 0x0054U);
  // The using statement is on line 7, but the DWARF says line 6.
  EXPECT_EQ(Struct->getTypes().at(0)->getLineNumber(), 6U);
  EXPECT_EQ(getSourceFileName(Struct->getTypes().at(0)), "import.cpp");
  EXPECT_EQ(Struct->getTypes().at(0)->getDieTag(),
            DW_TAG_imported_declaration);
  // Check the imported member.
  ASSERT_NE(Struct->getTypes().at(0)->getType(), nullptr);
  ASSERT_TRUE(Struct->getTypes().at(0)->getType()->getIsSymbol());
  auto *ImpMember = dynamic_cast<LibScopeView::Symbol *>(
      Struct->getTypes().at(0)->getType());
  EXPECT_EQ(ImpMember->getDieOffset(), 0x37U);
  EXPECT_TRUE(ImpMember->getIsMember());
  EXPECT_STREQ(ImpMember->getName(), "m");
}

TEST_F(TestElfDwarfReader, ReadInheritance) {
  LibScopeView::Scope *CU = nullptr;
  ASSERT_TRUE(loadSingleCUFromTestFile("ElfDwarfReader/inheritance.o", &CU));
  ASSERT_TRUE(checkChildCount(CU, 6, 0, 0));

  auto BaseClass = CU->getScopeAt(0);
  ASSERT_TRUE(BaseClass->getIsClassType());
  ASSERT_STREQ(BaseClass->getName(), "Base");

  auto PublicClass = CU->getScopeAt(1);
  ASSERT_TRUE(PublicClass->getIsClassType());
  ASSERT_STREQ(PublicClass->getName(), "Public");
  ASSERT_TRUE(PublicClass->getTypes().at(0)->getIsInheritance());
  EXPECT_EQ(
      dynamic_cast<LibScopeView::TypeImport *>(PublicClass->getTypes().at(0))
          ->getInheritanceAccess(),
      LibScopeView::AccessSpecifier::Public);

  auto PrivateClass = CU->getScopeAt(2);
  ASSERT_TRUE(PrivateClass->getIsClassType());
  ASSERT_STREQ(PrivateClass->getName(), "Private");
  // Compiler just leaves this as unspecified.
  EXPECT_EQ(
      dynamic_cast<LibScopeView::TypeImport *>(PrivateClass->getTypes().at(0))
          ->getInheritanceAccess(),
      LibScopeView::AccessSpecifier::Unspecified);

  auto ProtectedClass = CU->getScopeAt(3);
  ASSERT_TRUE(ProtectedClass->getIsClassType());
  ASSERT_STREQ(ProtectedClass->getName(), "Protected");
  EXPECT_EQ(dynamic_cast<LibScopeView::TypeImport *>(
                ProtectedClass->getTypes().at(0))
                ->getInheritanceAccess(),
            LibScopeView::AccessSpecifier::Protected);

  auto DefaultClass = CU->getScopeAt(4);
  ASSERT_TRUE(DefaultClass->getIsClassType());
  ASSERT_STREQ(DefaultClass->getName(), "Default");
  EXPECT_EQ(
      dynamic_cast<LibScopeView::TypeImport *>(DefaultClass->getTypes().at(0))
          ->getInheritanceAccess(),
      LibScopeView::AccessSpecifier::Unspecified);
}

TEST_F(TestElfDwarfReader, ReadLabel) {
  LibScopeView::Scope *CU = nullptr;
  ASSERT_TRUE(loadSingleCUFromTestFile("ElfDwarfReader/label.o", &CU));
  ASSERT_TRUE(checkChildCount(CU, 1, 1, 0));
  ASSERT_TRUE(checkChildCount(CU->getScopeAt(0), 1, 0, 1));

  // DW_TAG_label
  auto Label = CU->getScopeAt(0)->getScopeAt(0);
  EXPECT_TRUE(Label->getIsLabel());
  EXPECT_EQ(Label->getDieOffset(), 0x004eU);
  EXPECT_EQ(Label->getDieTag(), DW_TAG_label);
  EXPECT_EQ(Label->getLineNumber(), 3U);
  EXPECT_EQ(getSourceFileName(Label), "label.cpp");
  EXPECT_STREQ(Label->getName(), "loophead");
}

TEST_F(TestElfDwarfReader, ReadLines) {
  LibScopeView::Scope *CU = nullptr;
  ASSERT_TRUE(loadSingleCUFromTestFile("ElfDwarfReader/lines.o", &CU));
  ASSERT_EQ(CU->getLineCount(), 7U);

  auto *Ln = CU->getLines().at(0);
  EXPECT_TRUE(Ln->getIsLineRecord());
  EXPECT_EQ(Ln->getLineNumber(), 1U);
  EXPECT_EQ(Ln->getAddress(), 0x00000000U);
  EXPECT_EQ(Ln->getDieOffset(), 0x00000000U);
  EXPECT_EQ(getSourceFileName(Ln), "lines.cpp");
  EXPECT_TRUE(Ln->getIsNewStatement());
  EXPECT_FALSE(Ln->getIsNewBasicBlock());
  EXPECT_FALSE(Ln->getIsLineEndSequence());
  EXPECT_FALSE(Ln->getIsEpilogueBegin());
  EXPECT_FALSE(Ln->getIsPrologueEnd());

  Ln = CU->getLines().at(6);
  EXPECT_TRUE(Ln->getIsLineRecord());
  EXPECT_EQ(Ln->getLineNumber(), 13U);
  EXPECT_EQ(Ln->getAddress(), 0x00000032U);
  EXPECT_EQ(Ln->getDieOffset(), 0x00000032U);
  EXPECT_EQ(getSourceFileName(Ln), "lines.cpp");
  EXPECT_TRUE(Ln->getIsNewStatement());
  EXPECT_FALSE(Ln->getIsNewBasicBlock());
  EXPECT_TRUE(Ln->getIsLineEndSequence());
  EXPECT_FALSE(Ln->getIsEpilogueBegin());
  EXPECT_FALSE(Ln->getIsPrologueEnd());
}

TEST_F(TestElfDwarfReader, ReadNamespace) {
  LibScopeView::Scope *CU = nullptr;
  ASSERT_TRUE(loadSingleCUFromTestFile("ElfDwarfReader/import.o", &CU));
  ASSERT_TRUE(checkChildCount(CU, 4, 2, 1));

  // DW_TAG_namespace
  EXPECT_TRUE(CU->getScopeAt(2)->getIsNamespace());
  EXPECT_EQ(CU->getScopeAt(2)->getDieOffset(), 0x0062U);
  EXPECT_EQ(CU->getScopeAt(2)->getDieTag(), DW_TAG_namespace);
  EXPECT_EQ(CU->getScopeAt(2)->getLineNumber(), 10U);
  EXPECT_EQ(getSourceFileName(CU->getScopeAt(2)), "import.cpp");
  EXPECT_STREQ(CU->getScopeAt(2)->getName(), "NS");
}

TEST_F(TestElfDwarfReader, ReadMembers) {
  LibScopeView::Scope *CU = nullptr;
  ASSERT_TRUE(loadSingleCUFromTestFile("ElfDwarfReader/members.o", &CU));
  ASSERT_TRUE(checkChildCount(CU, 3, 3, 0));

  auto Struct = CU->getScopeAt(0);
  ASSERT_STREQ(Struct->getName(), "A");

  // m_unspecified
  EXPECT_EQ(Struct->getSymbolAt(0)->getAccessSpecifier(),
            LibScopeView::AccessSpecifier::Unspecified);
  // m_private
  EXPECT_EQ(Struct->getSymbolAt(1)->getAccessSpecifier(),
            LibScopeView::AccessSpecifier::Private);
  // m_public - Clang just puts unspecified for structs.
  EXPECT_EQ(Struct->getSymbolAt(2)->getAccessSpecifier(),
            LibScopeView::AccessSpecifier::Unspecified);
  // m_protected
  EXPECT_EQ(Struct->getSymbolAt(3)->getAccessSpecifier(),
            LibScopeView::AccessSpecifier::Protected);
}

TEST_F(TestElfDwarfReader, ReadQualifiedNames) {
  LibScopeView::Scope *CU = nullptr;
  ASSERT_TRUE(loadSingleCUFromTestFile("ElfDwarfReader/qualified_name.o", &CU));
  ASSERT_TRUE(checkChildCount(CU, 2, 1, 1));
  ASSERT_TRUE(checkChildCount(CU->getScopeAt(0), 1, 0, 0));
  ASSERT_TRUE(checkChildCount(CU->getScopeAt(0)->getScopeAt(0), 1, 0, 0));
  ASSERT_TRUE(checkChildCount(CU->getScopeAt(0)->getScopeAt(0)->getScopeAt(0),
                              0, 1, 2));

  auto ClassC = CU->getScopeAt(0)->getScopeAt(0)->getScopeAt(0);
  EXPECT_TRUE(ClassC->getHasQualifiedName());
  EXPECT_STREQ(
      dynamic_cast<LibScopeView::Element *>(ClassC)->getQualifiedName(),
      "OuterNS::InnerNS::");

  auto Typedef = ClassC->getTypes().at(0);
  EXPECT_TRUE(Typedef->getHasQualifiedName());
  EXPECT_STREQ(
      dynamic_cast<LibScopeView::Element *>(Typedef)->getQualifiedName(),
      "OuterNS::InnerNS::C::");

  auto StaticMemDefin = CU->getSymbolAt(0);
  EXPECT_TRUE(StaticMemDefin->getHasQualifiedName());
  EXPECT_STREQ(
      dynamic_cast<LibScopeView::Element *>(StaticMemDefin)->getQualifiedName(),
      "OuterNS::InnerNS::C::");
}

TEST_F(TestElfDwarfReader, ReadSymbols) {
  LibScopeView::Scope *CU = nullptr;
  ASSERT_TRUE(loadSingleCUFromTestFile("ElfDwarfReader/symbol.o", &CU));
  ASSERT_TRUE(checkChildCount(CU, 3, 1, 1));
  ASSERT_TRUE(checkChildCount(CU->getScopeAt(0), 0, 0, 1));
  ASSERT_TRUE(checkChildCount(CU->getScopeAt(1), 0, 0, 2));
  ASSERT_TRUE(checkChildCount(CU->getScopeAt(2), 0, 0, 1));

  auto Var = CU->getSymbolAt(0);
  auto Member = CU->getScopeAt(0)->getSymbolAt(0);
  auto Param = CU->getScopeAt(1)->getSymbolAt(0);
  auto UParams = CU->getScopeAt(2)->getSymbolAt(0);

  auto IntType = CU->getTypes().at(0);

  // DW_TAG_variable
  EXPECT_TRUE(Var->getIsVariable());
  EXPECT_EQ(Var->getDieOffset(), 0x002aU);
  EXPECT_EQ(Var->getDieTag(), DW_TAG_variable);
  EXPECT_EQ(Var->getLineNumber(), 1U);
  EXPECT_EQ(getSourceFileName(Var), "symbol.cpp");
  EXPECT_STREQ(Var->getName(), "var");
  EXPECT_EQ(Var->getType(), IntType);

  // DW_TAG_member
  EXPECT_TRUE(Member->getIsMember());
  EXPECT_EQ(Member->getDieOffset(), 0x004eU);
  EXPECT_EQ(Member->getDieTag(), DW_TAG_member);
  EXPECT_EQ(Member->getLineNumber(), 4U);
  EXPECT_EQ(getSourceFileName(Member), "symbol.cpp");
  EXPECT_STREQ(Member->getName(), "mem");
  EXPECT_EQ(Member->getType(), IntType);

  // DW_TAG_formal_parameter
  EXPECT_TRUE(Param->getIsParameter());
  EXPECT_EQ(Param->getDieOffset(), 0x0074U);
  EXPECT_EQ(Param->getDieTag(), DW_TAG_formal_parameter);
  EXPECT_EQ(Param->getLineNumber(), 7U);
  EXPECT_EQ(getSourceFileName(Param), "symbol.cpp");
  EXPECT_STREQ(Param->getName(), "param");
  EXPECT_EQ(Param->getType(), IntType);

  // DW_TAG_unspecified_parameters
  EXPECT_TRUE(UParams->getIsUnspecifiedParameter());
  EXPECT_EQ(UParams->getDieOffset(), 0x00aaU);
  EXPECT_EQ(UParams->getDieTag(), DW_TAG_unspecified_parameters);
  EXPECT_STREQ(UParams->getName(), "");

  // Test Symbol to Symbol DW_TAG_specification.
  // The file import.o has such a symbol.
  ASSERT_TRUE(loadSingleCUFromTestFile("ElfDwarfReader/import.o", &CU));
  ASSERT_TRUE(checkChildCount(CU, 4, 2, 1));
  ASSERT_NE(CU->getSymbolAt(0)->getReference(), nullptr);
  EXPECT_EQ(CU->getSymbolAt(0)->getReference()->getDieOffset(), 0x6cU);
  EXPECT_STREQ(CU->getSymbolAt(0)->getReference()->getName(), "x");
}

TEST_F(TestElfDwarfReader, ReadTemplates) {
  LibScopeView::Scope *CU = nullptr;
  ASSERT_TRUE(loadSingleCUFromTestFile("ElfDwarfReader/template.o", &CU));
  ASSERT_TRUE(checkChildCount(CU, 3, 1, 0));
  ASSERT_EQ(CU->getScopeAt(1)->getTypeCount(), 2U);
  ASSERT_EQ(CU->getScopeAt(2)->getTypeCount(), 2U);
  ASSERT_STREQ(CU->getScopeAt(1)->getName(), "t_func<1, int>");
  ASSERT_STREQ(CU->getScopeAt(2)->getName(), "t_func<-1, int>");

  auto IntType = CU->getTypes().at(0);

  std::string NotTemplateFailure(
      "A function containing template parameters should be a template");
  EXPECT_TRUE(CU->getScopeAt(1)->getIsTemplate()) << NotTemplateFailure;
  EXPECT_TRUE(CU->getScopeAt(2)->getIsTemplate()) << NotTemplateFailure;

  // DW_TAG_template_value_parameter.
  auto ValParam = CU->getScopeAt(1)->getTypes().at(0);
  EXPECT_TRUE(ValParam->getIsTemplateValue());
  EXPECT_EQ(ValParam->getDieOffset(), 0x005cU);
  EXPECT_EQ(ValParam->getDieTag(), DW_TAG_template_value_parameter);
  EXPECT_EQ(ValParam->getType(), IntType);
  EXPECT_STREQ(ValParam->getName(), "VAL");
  EXPECT_STREQ(ValParam->getValue(), "1");

  // DW_TAG_template_type_parameter.
  auto TypeParam = CU->getScopeAt(1)->getTypes().at(1);
  EXPECT_TRUE(TypeParam->getIsTemplateType());
  EXPECT_EQ(TypeParam->getDieOffset(), 0x0066U);
  EXPECT_EQ(TypeParam->getDieTag(), DW_TAG_template_type_parameter);
  EXPECT_EQ(TypeParam->getType(), IntType);
  EXPECT_STREQ(TypeParam->getName(), "TY");

  // Test negative template value.
  auto NegativeValParam = CU->getScopeAt(2)->getTypes().at(0);
  EXPECT_TRUE(NegativeValParam->getIsTemplateValue());
  EXPECT_EQ(NegativeValParam->getDieOffset(), 0x0089U);
  EXPECT_EQ(NegativeValParam->getDieTag(), DW_TAG_template_value_parameter);
  EXPECT_EQ(NegativeValParam->getType(), IntType);
  EXPECT_STREQ(NegativeValParam->getName(), "VAL");
  EXPECT_STREQ(NegativeValParam->getValue(), "-1");

  // Test showing template names.
  LibScopeView::CmdOptions Options;
  Options.setFormatTemplatesEncoded();
  ASSERT_TRUE(
      loadSingleCUFromTestFile("ElfDwarfReader/template.o", &CU, Options));
  ASSERT_TRUE(checkChildCount(CU, 3, 1, 0));
  // The compiler already added the template parameters to the name so here we
  // get it twice.
  ASSERT_STREQ(CU->getScopeAt(1)->getName(), "t_func<1, int><1,int>");
}

TEST_F(TestElfDwarfReader, ReadTemplatePacks) {
  LibScopeView::Scope *CU = nullptr;
  ASSERT_TRUE(loadSingleCUFromTestFile("ElfDwarfReader/template_pack.o", &CU));
  ASSERT_TRUE(checkChildCount(CU, 5, 1, 0));
  ASSERT_TRUE(checkChildCount(CU->getScopeAt(1), 1, 0, 3));
  ASSERT_STREQ(CU->getScopeAt(1)->getName(), "sum<int, int>");
  auto *TPack2Templates = CU->getScopeAt(1)->getScopeAt(0);
  ASSERT_TRUE(checkChildCount(CU->getScopeAt(2), 1, 0, 2));
  ASSERT_STREQ(CU->getScopeAt(2)->getName(), "sum<int>");
  auto *TPack1Template = CU->getScopeAt(2)->getScopeAt(0);
  ASSERT_TRUE(checkChildCount(CU->getScopeAt(3), 1, 0, 1));
  ASSERT_STREQ(CU->getScopeAt(3)->getName(), "sum<>");
  auto *TPack0Templates = CU->getScopeAt(3)->getScopeAt(0);

  auto *IntType = CU->getTypes().at(0);

  std::string NotTemplateFailure(
      "A function containing template parameters should be a template");
  EXPECT_TRUE(CU->getScopeAt(1)->getIsTemplate()) << NotTemplateFailure;
  EXPECT_TRUE(CU->getScopeAt(2)->getIsTemplate()) << NotTemplateFailure;
  EXPECT_TRUE(CU->getScopeAt(3)->getIsTemplate()) << NotTemplateFailure;

  // DW_TAG_GNU_template_parameter_pack.
  EXPECT_TRUE(TPack2Templates->getIsTemplatePack());
  EXPECT_EQ(TPack2Templates->getDieOffset(), 0x008aU);
  EXPECT_EQ(TPack2Templates->getDieTag(), DW_TAG_GNU_template_parameter_pack);
  EXPECT_STREQ(TPack2Templates->getName(), "Targs");
  // Two template parameters of type int.
  ASSERT_TRUE(checkChildCount(TPack2Templates, 0, 2, 0));
  EXPECT_TRUE(TPack2Templates->getTypes().at(0)->getIsTemplateType());
  EXPECT_EQ(TPack2Templates->getTypes().at(0)->getType(), IntType);
  EXPECT_TRUE(TPack2Templates->getTypes().at(1)->getIsTemplateType());
  EXPECT_EQ(TPack2Templates->getTypes().at(1)->getType(), IntType);

  EXPECT_TRUE(TPack1Template->getIsTemplatePack());
  EXPECT_EQ(TPack1Template->getDieOffset(), 0x00d4U);
  EXPECT_EQ(TPack1Template->getDieTag(), DW_TAG_GNU_template_parameter_pack);
  EXPECT_STREQ(TPack1Template->getName(), "Targs");
  // One template parameter of type int.
  ASSERT_TRUE(checkChildCount(TPack1Template, 0, 1, 0));
  EXPECT_TRUE(TPack1Template->getTypes().at(0)->getIsTemplateType());
  EXPECT_EQ(TPack1Template->getTypes().at(0)->getType(), IntType);

  EXPECT_TRUE(TPack0Templates->getIsTemplatePack());
  EXPECT_EQ(TPack0Templates->getDieOffset(), 0x010bU);
  EXPECT_EQ(TPack0Templates->getDieTag(), DW_TAG_GNU_template_parameter_pack);
  EXPECT_STREQ(TPack0Templates->getName(), "Targs");
  // No template parameters.
  EXPECT_TRUE(checkChildCount(TPack0Templates, 0, 0, 0));
}

TEST_F(TestElfDwarfReader, ReadTemplateTemplates) {
  LibScopeView::Scope *CU = nullptr;
  ASSERT_TRUE(
      loadSingleCUFromTestFile("ElfDwarfReader/template_template.o", &CU));
  ASSERT_TRUE(checkChildCount(CU, 3, 2, 0));
  ASSERT_EQ(CU->getScopeAt(2)->getTypeCount(), 2U);
  ASSERT_STREQ(CU->getScopeAt(2)->getName(), "foo<int, vector>");

  EXPECT_TRUE(CU->getScopeAt(2)->getIsTemplate())
      << "A function containing template parameters should be a template";

  // DW_TAG_GNU_template_template_parameter.
  auto TTParam = CU->getScopeAt(2)->getTypes().at(1);
  EXPECT_TRUE(TTParam->getIsTemplateTemplate());
  EXPECT_EQ(TTParam->getDieOffset(), 0x00acU);
  EXPECT_EQ(TTParam->getDieTag(), DW_TAG_GNU_template_template_parameter);
  EXPECT_STREQ(TTParam->getName(), "V");
  EXPECT_STREQ(TTParam->getValue(), "vector");
}

TEST_F(TestElfDwarfReader, ReadTypes) {
  LibScopeView::Scope *CU = nullptr;
  ASSERT_TRUE(loadSingleCUFromTestFile("ElfDwarfReader/type.o", &CU));
  ASSERT_EQ(CU->getTypeCount(), 8U);

  auto RVal = CU->getTypes().at(0);
  auto Base = CU->getTypes().at(1);
  auto TDef = CU->getTypes().at(2);
  auto Const = CU->getTypes().at(3);
  auto Ptr = CU->getTypes().at(4);
  auto Ref = CU->getTypes().at(5);
  auto Restrict = CU->getTypes().at(6);
  auto Volatile = CU->getTypes().at(7);

  // DW_TAG_base_type
  EXPECT_TRUE(Base->getIsBaseType());
  EXPECT_EQ(Base->getDieOffset(), 0x00c5U);
  EXPECT_EQ(Base->getDieTag(), DW_TAG_base_type);
  EXPECT_STREQ(Base->getName(), "int");
  EXPECT_EQ(Base->getType(), nullptr);
  EXPECT_EQ(Base->getByteSize(), 4U);

  // DW_TAG_const_type
  EXPECT_TRUE(Const->getIsConstType());
  EXPECT_EQ(Const->getDieOffset(), 0x00d7U);
  EXPECT_EQ(Const->getDieTag(), DW_TAG_const_type);
  EXPECT_STREQ(Const->getName(), "const int");
  EXPECT_EQ(Const->getType(), Base);

  // DW_TAG_pointer_type
  EXPECT_TRUE(Ptr->getIsPointerType());
  EXPECT_EQ(Ptr->getDieOffset(), 0x00dcU);
  EXPECT_EQ(Ptr->getDieTag(), DW_TAG_pointer_type);
  EXPECT_STREQ(Ptr->getName(), "int *");
  EXPECT_EQ(Ptr->getType(), Base);

  // DW_TAG_reference_type
  EXPECT_TRUE(Ref->getIsReferenceType());
  EXPECT_EQ(Ref->getDieOffset(), 0x00e1U);
  EXPECT_EQ(Ref->getDieTag(), DW_TAG_reference_type);
  EXPECT_STREQ(Ref->getName(), "int &");
  EXPECT_EQ(Ref->getType(), Base);

  // DW_TAG_restrict_type
  EXPECT_TRUE(Restrict->getIsRestrictType());
  EXPECT_EQ(Restrict->getDieOffset(), 0x00e6U);
  EXPECT_EQ(Restrict->getDieTag(), DW_TAG_restrict_type);
  EXPECT_STREQ(Restrict->getName(), "restrict int *");
  EXPECT_EQ(Restrict->getType(), Ptr);

  // DW_TAG_rvalue_reference_type
  EXPECT_TRUE(RVal->getIsRvalueReferenceType());
  EXPECT_EQ(RVal->getDieOffset(), 0x00c0U);
  EXPECT_EQ(RVal->getDieTag(), DW_TAG_rvalue_reference_type);
  EXPECT_STREQ(RVal->getName(), "int &&");
  EXPECT_EQ(RVal->getType(), Base);

  // DW_TAG_typedef
  EXPECT_TRUE(TDef->getIsTypedef());
  EXPECT_EQ(TDef->getDieOffset(), 0x00ccU);
  EXPECT_EQ(TDef->getDieTag(), DW_TAG_typedef);
  EXPECT_EQ(TDef->getLineNumber(), 1U);
  EXPECT_EQ(getSourceFileName(TDef), "type.cpp");
  EXPECT_STREQ(TDef->getName(), "T_INT");
  EXPECT_EQ(TDef->getType(), Base);

  // DW_TAG_volatile_type
  EXPECT_TRUE(Volatile->getIsVolatileType());
  EXPECT_EQ(Volatile->getDieOffset(), 0x00ebU);
  EXPECT_EQ(Volatile->getDieTag(), DW_TAG_volatile_type);
  EXPECT_STREQ(Volatile->getName(), "volatile int");
  EXPECT_EQ(Volatile->getType(), Base);
}
