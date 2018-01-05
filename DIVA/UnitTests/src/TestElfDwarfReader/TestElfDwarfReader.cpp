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

using LibScopeView::cast;
using LibScopeView::dyn_cast;
using LibScopeView::isa;

using testing::AssertionResult;

// IMPORTANT.
//
// These tests depend on the DWARF in the elf files in
// TestInputs/ElfDwarfReader. Changing these elf files in any way may cause
// these tests to fail.

// Test helper functions and fixtures.
namespace {

template <class Ty>
Ty *getNthChildOfType(LibScopeView::Scope *Parent, size_t N) {
  for (auto *Child : Parent->getChildren()) {
    if (auto Res = dyn_cast<Ty>(Child)) {
      if (N == 0)
        return Res;
      --N;
    }
  }
  return nullptr;
}

LibScopeView::Scope *getNthScopeIn(LibScopeView::Scope *Parent, size_t N) {
  return getNthChildOfType<LibScopeView::Scope>(Parent, N);
}
LibScopeView::Type *getNthTypeIn(LibScopeView::Scope *Parent, size_t N) {
  return getNthChildOfType<LibScopeView::Type>(Parent, N);
}
LibScopeView::Symbol *getNthSymbolIn(LibScopeView::Scope *Parent, size_t N) {
  return getNthChildOfType<LibScopeView::Symbol>(Parent, N);
}

template <class Ty>
size_t countChildrenOfType(const LibScopeView::Scope *Parent) {
  size_t Count = 0;
  for (auto *Child : Parent->getChildren())
    if (dyn_cast<Ty>(Child))
      ++Count;
  return Count;
}

size_t countScopesIn(const LibScopeView::Scope *Parent) {
  return countChildrenOfType<LibScopeView::Scope>(Parent);
}
size_t countTypesIn(const LibScopeView::Scope *Parent) {
  return countChildrenOfType<LibScopeView::Type>(Parent);
}
size_t countSymbolsIn(const LibScopeView::Scope *Parent) {
  return countChildrenOfType<LibScopeView::Symbol>(Parent);
}

AssertionResult checkChildCount(const LibScopeView::Scope *Parent,
                                size_t ScopeCount, size_t TypeCount,
                                size_t SymbolCount) {
  size_t ScopesFound = countScopesIn(Parent);
  size_t TypesFound = countTypesIn(Parent);
  size_t SymbolsFound = countSymbolsIn(Parent);

  if (ScopesFound != ScopeCount)
    return ::testing::AssertionFailure() << "Scope contained " << ScopesFound
                                         << " Scopes, expected " << ScopeCount;
  if (TypesFound != TypeCount)
    return ::testing::AssertionFailure() << "Scope contained " << TypesFound
                                         << " Types, expected " << TypeCount;
  if (SymbolsFound != SymbolCount)
    return ::testing::AssertionFailure()
           << "Scope contained " << SymbolsFound << " Symbols, expected "
           << SymbolCount;
  if (Parent->getChildren().size() != ScopeCount + TypeCount + SymbolCount)
    return ::testing::AssertionFailure()
           << "Children.size() != ScopeCount + TypeCount + SymbolCount";

  return ::testing::AssertionSuccess();
}

// Test fixture providing helpers to load a Scope tree using the DwarfReader.
class TestElfDwarfReader : public ::testing::Test {
public:
  AssertionResult loadRootFromTestFile(
      std::string TestFile, LibScopeView::Scope **Root,
      LibScopeView::PrintSettings Settings = LibScopeView::PrintSettings()) {
    if (!LibScopeView::doesFileExist(getTestInputFilePath(TestFile)))
      return ::testing::AssertionFailure() << "Test file does not exist";

    Settings.SortKey = LibScopeView::SortingKey::OFFSET;
    DwarfReader Reader;

    ScpRoot = Reader.loadFile(getTestInputFilePath(TestFile), Settings);
    if (!ScpRoot)
      return ::testing::AssertionFailure() << "Failed to load test file";
    *Root = ScpRoot.get();

    return ::testing::AssertionSuccess();
  }

  AssertionResult loadSingleCUFromTestFile(
      std::string TestFile, LibScopeView::Scope **CU,
      LibScopeView::PrintSettings Settings = LibScopeView::PrintSettings()) {
    LibScopeView::Scope *Root = nullptr;
    AssertionResult Res = loadRootFromTestFile(TestFile, &Root, Settings);
    if (!Res)
      return Res;
    if (Root->getChildren().size() == 0)
      return ::testing::AssertionFailure()
             << "Test file did not contain any CUs";
    if (Root->getChildren().size() > 1)
      return ::testing::AssertionFailure()
             << "Test file contained more than 1 CU";

    *CU = getNthScopeIn(Root, 0);
    if (!(CU && isa<LibScopeView::ScopeCompileUnit>(**CU)))
      return ::testing::AssertionFailure()
             << "Top level scope in test file was not a CU";

    return ::testing::AssertionSuccess();
  }

private:
  std::unique_ptr<LibScopeView::ScopeRoot> ScpRoot;
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

  auto CU1 = getNthScopeIn(Root, 0);
  ASSERT_TRUE(checkChildCount(CU1, 1, 1, 0));
  EXPECT_TRUE(checkChildCount(getNthScopeIn(CU1, 0), 0, 0, 0));
  EXPECT_TRUE(isa<LibScopeView::ScopeCompileUnit>(*CU1));
  EXPECT_TRUE(isa<LibScopeView::ScopeFunction>(*getNthScopeIn(CU1, 0)));
  EXPECT_TRUE(getNthTypeIn(CU1, 0)->getIsBaseType());

  auto CU2 = getNthScopeIn(Root, 1);
  ASSERT_TRUE(checkChildCount(CU2, 1, 1, 1));
  auto *ScopeChild = getNthScopeIn(CU2, 0);
  ASSERT_TRUE(checkChildCount(ScopeChild, 1, 0, 0));
  auto *ScopeChildScopeChild = getNthScopeIn(ScopeChild, 0);
  ASSERT_TRUE(checkChildCount(ScopeChildScopeChild, 0, 0, 1));
  EXPECT_TRUE(isa<LibScopeView::ScopeCompileUnit>(*CU2));
  EXPECT_TRUE(getNthSymbolIn(CU2, 0)->getIsVariable());
  EXPECT_TRUE(getNthScopeIn(CU2, 0)->getIsClassType());
  EXPECT_TRUE(isa<LibScopeView::ScopeFunction>(*ScopeChildScopeChild));
  EXPECT_TRUE(getNthSymbolIn(ScopeChildScopeChild, 0)->getIsParameter());
  EXPECT_TRUE(getNthTypeIn(CU2, 0)->getIsPointerType());

  auto CU3 = getNthScopeIn(Root, 2);
  ASSERT_TRUE(checkChildCount(CU3, 0, 1, 1));
  EXPECT_TRUE(isa<LibScopeView::ScopeCompileUnit>(*CU3));
  EXPECT_TRUE(getNthSymbolIn(CU3, 0)->getIsVariable());
  EXPECT_TRUE(getNthTypeIn(CU3, 0)->getIsBaseType());
}

TEST_F(TestElfDwarfReader, ReadCompileUnits) {
  LibScopeView::Scope *Root = nullptr;
  ASSERT_TRUE(loadRootFromTestFile("ElfDwarfReader/structure.elf", &Root));
  ASSERT_TRUE(checkChildCount(Root, 3, 0, 0));

  auto CU1 = getNthScopeIn(Root, 0);
  auto CU2 = getNthScopeIn(Root, 1);
  auto CU3 = getNthScopeIn(Root, 2);

  EXPECT_TRUE(isa<LibScopeView::ScopeCompileUnit>(*CU1));
  EXPECT_EQ(CU1->getDieOffset(), 0x0bU);
  EXPECT_EQ(CU1->getDieTag(), DW_TAG_compile_unit);
  EXPECT_EQ(CU1->getName(), "structure1.cpp");
  EXPECT_EQ(CU1->getLineNumber(), 0U);
  EXPECT_EQ(CU1->getFilePath(), "");

  EXPECT_TRUE(isa<LibScopeView::ScopeCompileUnit>(*CU2));
  EXPECT_EQ(CU2->getDieOffset(), 0x56U);
  EXPECT_EQ(CU2->getDieTag(), DW_TAG_compile_unit);
  EXPECT_EQ(CU2->getName(), "structure2.cpp");
  EXPECT_EQ(CU2->getLineNumber(), 0U);
  EXPECT_EQ(CU2->getFilePath(), "");

  EXPECT_TRUE(isa<LibScopeView::ScopeCompileUnit>(*CU3));
  EXPECT_EQ(CU3->getDieOffset(), 0x0a9U);
  EXPECT_EQ(CU3->getDieTag(), DW_TAG_compile_unit);
  EXPECT_EQ(CU3->getName(), "structure3.cpp");
  EXPECT_EQ(CU3->getLineNumber(), 0U);
  EXPECT_EQ(CU3->getFilePath(), "");
}

TEST_F(TestElfDwarfReader, ReadAggregate) {
  LibScopeView::Scope *CU = nullptr;
  ASSERT_TRUE(loadSingleCUFromTestFile("ElfDwarfReader/aggregate.o", &CU));
  ASSERT_EQ(countScopesIn(CU), 4U);

  // DW_TAG_class_type
  auto Class = getNthScopeIn(CU, 0);
  EXPECT_TRUE(Class->getIsClassType());
  EXPECT_EQ(Class->getDieOffset(), 0x0033U);
  EXPECT_EQ(Class->getDieTag(), DW_TAG_class_type);
  EXPECT_EQ(Class->getLineNumber(), 1U);
  EXPECT_EQ(LibScopeView::getFileName(Class->getFilePath()), "aggregate.cpp");
  EXPECT_EQ(Class->getName(), "A");

  // DW_TAG_struct_type
  auto Struct = getNthScopeIn(CU, 2);
  EXPECT_TRUE(Struct->getIsStructType());
  EXPECT_EQ(Struct->getDieOffset(), 0x0095U);
  EXPECT_EQ(Struct->getDieTag(), DW_TAG_structure_type);
  EXPECT_EQ(Struct->getLineNumber(), 9U);
  EXPECT_EQ(LibScopeView::getFileName(Struct->getFilePath()), "aggregate.cpp");
  EXPECT_EQ(Struct->getName(), "C");

  // DW_TAG_union_type
  auto Union = getNthScopeIn(CU, 3);
  EXPECT_TRUE(Union->getIsUnionType());
  EXPECT_EQ(Union->getDieOffset(), 0x00bfU);
  EXPECT_EQ(Union->getDieTag(), DW_TAG_union_type);
  EXPECT_EQ(Union->getLineNumber(), 13U);
  EXPECT_EQ(LibScopeView::getFileName(Union->getFilePath()), "aggregate.cpp");
  EXPECT_EQ(Union->getName(), "D");

  // DW_TAG_inheritance
  auto SubClass = getNthScopeIn(CU, 1);
  ASSERT_EQ(countTypesIn(SubClass), 1U);
  EXPECT_TRUE(getNthTypeIn(SubClass, 0)->getIsInheritance());
  EXPECT_EQ(getNthTypeIn(SubClass, 0)->getDieOffset(), 0x006cU);
  EXPECT_EQ(getNthTypeIn(SubClass, 0)->getDieTag(), DW_TAG_inheritance);
  EXPECT_EQ(getNthTypeIn(SubClass, 0)->getType(), Class);
}

TEST_F(TestElfDwarfReader, ReadArray) {
  LibScopeView::Scope *CU = nullptr;
  ASSERT_TRUE(loadSingleCUFromTestFile("ElfDwarfReader/array.o", &CU));
  ASSERT_TRUE(checkChildCount(CU, 1, 2, 1));

  auto IntType = getNthTypeIn(CU, 0);
  auto SizeType = getNthTypeIn(CU, 1);

  // DW_TAG_array_type
  auto Array = getNthScopeIn(CU, 0);
  EXPECT_TRUE(isa<LibScopeView::ScopeArray>(*Array));
  EXPECT_EQ(Array->getDieOffset(), 0x0033U);
  EXPECT_EQ(Array->getDieTag(), DW_TAG_array_type);
  EXPECT_EQ(Array->getType(), IntType);

  // DW_TAG_subrange_type
  ASSERT_TRUE(checkChildCount(Array, 0, 2, 0));
  EXPECT_TRUE(isa<LibScopeView::TypeSubrange>(*getNthTypeIn(Array, 0)));
  EXPECT_EQ(getNthTypeIn(Array, 0)->getDieOffset(), 0x0038U);
  EXPECT_EQ(getNthTypeIn(Array, 0)->getDieTag(), DW_TAG_subrange_type);
  EXPECT_EQ(getNthTypeIn(Array, 0)->getType(), SizeType);

  // Name resolution.
  EXPECT_EQ(Array->getName(), "int [5][10]");
}

TEST_F(TestElfDwarfReader, ReadBlocks) {
  LibScopeView::Scope *CU = nullptr;
  ASSERT_TRUE(loadSingleCUFromTestFile("ElfDwarfReader/block.o", &CU));
  ASSERT_EQ(countScopesIn(CU), 1U);
  ASSERT_EQ(countScopesIn(getNthScopeIn(CU, 0)), 3U);

  // DW_TAG_lexical_block
  LibScopeView::Scope *Parent = getNthScopeIn(CU, 0);
  LibScopeView::Scope *Block = getNthScopeIn(Parent, 2);
  EXPECT_TRUE(Block->getIsBlock());
  EXPECT_TRUE(Block->getIsLexicalBlock());
  EXPECT_EQ(Block->getDieOffset(), 0x007bU);
  EXPECT_EQ(Block->getDieTag(), DW_TAG_lexical_block);

  ASSERT_TRUE(loadSingleCUFromTestFile("ElfDwarfReader/try_catch.elf", &CU));
  ASSERT_EQ(countScopesIn(CU), 2U);

  // DW_TAG_try_block
  EXPECT_TRUE(getNthScopeIn(CU, 0)->getIsBlock());
  EXPECT_TRUE(getNthScopeIn(CU, 0)->getIsTryBlock());
  EXPECT_EQ(getNthScopeIn(CU, 0)->getDieOffset(), 0xcU);
  EXPECT_EQ(getNthScopeIn(CU, 0)->getDieTag(), DW_TAG_try_block);

  // DW_TAG_catch_block
  EXPECT_TRUE(getNthScopeIn(CU, 1)->getIsBlock());
  EXPECT_TRUE(getNthScopeIn(CU, 1)->getIsCatchBlock());
  EXPECT_EQ(getNthScopeIn(CU, 1)->getDieOffset(), 0xdU);
  EXPECT_EQ(getNthScopeIn(CU, 1)->getDieTag(), DW_TAG_catch_block);
}

TEST_F(TestElfDwarfReader, ReadEntryPoint) {
  LibScopeView::Scope *CU = nullptr;
  ASSERT_TRUE(loadSingleCUFromTestFile("ElfDwarfReader/entry_point.elf", &CU));
  ASSERT_EQ(countScopesIn(CU), 1U);

  // DW_TAG_entry_point
  EXPECT_TRUE(getNthScopeIn(CU, 0)->getIsEntryPoint());
  EXPECT_EQ(getNthScopeIn(CU, 0)->getDieOffset(), 0xcU);
  EXPECT_EQ(getNthScopeIn(CU, 0)->getDieTag(), DW_TAG_entry_point);
  EXPECT_EQ(getNthScopeIn(CU, 0)->getName(), "entry");
}

TEST_F(TestElfDwarfReader, ReadEnum) {
  LibScopeView::Scope *CU = nullptr;
  ASSERT_TRUE(loadSingleCUFromTestFile("ElfDwarfReader/enum.o", &CU));
  ASSERT_TRUE(checkChildCount(CU, 3, 2, 3));
  ASSERT_EQ(countTypesIn(getNthScopeIn(CU, 0)), 3U);
  ASSERT_EQ(countTypesIn(getNthScopeIn(CU, 1)), 3U);
  ASSERT_EQ(countTypesIn(getNthScopeIn(CU, 2)), 3U);

  auto Enum1 = getNthScopeIn(CU, 0);
  auto Enum1Value1 = getNthTypeIn(Enum1, 0);
  auto Enum1Value2 = getNthTypeIn(Enum1, 1);
  auto Enum1Value3 = getNthTypeIn(Enum1, 2);
  auto Enum2 = getNthScopeIn(CU, 1);
  auto Enum2Value1 = getNthTypeIn(Enum2, 0);
  auto Enum2Value2 = getNthTypeIn(Enum2, 1);
  auto Enum2Value3 = getNthTypeIn(Enum2, 2);
  auto Enum3 = getNthScopeIn(CU, 2);
  auto Enum3Value1 = getNthTypeIn(Enum3, 0);
  auto Enum3Value2 = getNthTypeIn(Enum3, 1);
  auto Enum3Value3 = getNthTypeIn(Enum3, 2);

  auto ShortType = getNthTypeIn(CU, 0);
  auto IntType = getNthTypeIn(CU, 1);

  // Enumeration (1)
  EXPECT_TRUE(isa<LibScopeView::ScopeEnumeration>(*Enum1));
  EXPECT_EQ(Enum1->getDieOffset(), 0x0033U);
  EXPECT_EQ(Enum1->getDieTag(), DW_TAG_enumeration_type);
  EXPECT_EQ(Enum1->getLineNumber(), 1U);
  EXPECT_EQ(LibScopeView::getFileName(Enum1->getFilePath()), "enum.cpp");
  EXPECT_EQ(Enum1->getName(), "E1");
  EXPECT_EQ(Enum1->getType(), nullptr);
  EXPECT_EQ(dyn_cast<LibScopeView::ScopeEnumeration>(Enum1)->getIsClass(),
            false);

  EXPECT_TRUE(isa<LibScopeView::TypeEnumerator>(*Enum1Value1));
  EXPECT_EQ(Enum1Value1->getDieOffset(), 0x003bU);
  EXPECT_EQ(Enum1Value1->getDieTag(), DW_TAG_enumerator);
  EXPECT_EQ(Enum1Value1->getName(), "A");
  EXPECT_EQ(Enum1Value1->getValue(), "0");

  EXPECT_TRUE(isa<LibScopeView::TypeEnumerator>(*Enum1Value2));
  EXPECT_EQ(Enum1Value2->getDieOffset(), 0x0041U);
  EXPECT_EQ(Enum1Value2->getDieTag(), DW_TAG_enumerator);
  EXPECT_EQ(Enum1Value2->getName(), "B");
  EXPECT_EQ(Enum1Value2->getValue(), "1");

  EXPECT_TRUE(isa<LibScopeView::TypeEnumerator>(*Enum1Value3));
  EXPECT_EQ(Enum1Value3->getDieOffset(), 0x0047U);
  EXPECT_EQ(Enum1Value3->getDieTag(), DW_TAG_enumerator);
  EXPECT_EQ(Enum1Value3->getName(), "C");
  EXPECT_EQ(Enum1Value3->getValue(), "2");

  // Enumeration (2)
  EXPECT_TRUE(isa<LibScopeView::ScopeEnumeration>(*Enum2));
  EXPECT_EQ(Enum2->getDieOffset(), 0x0063U);
  EXPECT_EQ(Enum2->getDieTag(), DW_TAG_enumeration_type);
  EXPECT_EQ(Enum2->getLineNumber(), 7U);
  EXPECT_EQ(LibScopeView::getFileName(Enum2->getFilePath()), "enum.cpp");
  EXPECT_EQ(Enum2->getName(), "E2");
  EXPECT_EQ(Enum2->getType(), ShortType);
  EXPECT_EQ(dyn_cast<LibScopeView::ScopeEnumeration>(Enum2)->getIsClass(),
            true);

  EXPECT_TRUE(isa<LibScopeView::TypeEnumerator>(*Enum2Value1));
  EXPECT_EQ(Enum2Value1->getDieOffset(), 0x006fU);
  EXPECT_EQ(Enum2Value1->getDieTag(), DW_TAG_enumerator);
  EXPECT_EQ(Enum2Value1->getName(), "G");
  EXPECT_EQ(Enum2Value1->getValue(), "-1");

  EXPECT_TRUE(isa<LibScopeView::TypeEnumerator>(*Enum2Value2));
  EXPECT_EQ(Enum2Value2->getDieOffset(), 0x0075U);
  EXPECT_EQ(Enum2Value2->getDieTag(), DW_TAG_enumerator);
  EXPECT_EQ(Enum2Value2->getName(), "H");
  EXPECT_EQ(Enum2Value2->getValue(), "0");

  EXPECT_TRUE(isa<LibScopeView::TypeEnumerator>(*Enum2Value3));
  EXPECT_EQ(Enum2Value3->getDieOffset(), 0x007bU);
  EXPECT_EQ(Enum2Value3->getDieTag(), DW_TAG_enumerator);
  EXPECT_EQ(Enum2Value3->getName(), "I");
  EXPECT_EQ(Enum2Value3->getValue(), "1");

  // Enumeration (3)
  EXPECT_TRUE(isa<LibScopeView::ScopeEnumeration>(*Enum3));
  EXPECT_EQ(Enum3->getDieOffset(), 0x009eU);
  EXPECT_EQ(Enum3->getDieTag(), DW_TAG_enumeration_type);
  EXPECT_EQ(Enum3->getLineNumber(), 13U);
  EXPECT_EQ(LibScopeView::getFileName(Enum3->getFilePath()), "enum.cpp");
  EXPECT_EQ(Enum3->getName(), "E3");
  EXPECT_EQ(Enum3->getType(), IntType);
  EXPECT_EQ(dyn_cast<LibScopeView::ScopeEnumeration>(Enum3)->getIsClass(),
            true);

  EXPECT_TRUE(isa<LibScopeView::TypeEnumerator>(*Enum3Value1));
  EXPECT_EQ(Enum3Value1->getDieOffset(), 0x00aaU);
  EXPECT_EQ(Enum3Value1->getDieTag(), DW_TAG_enumerator);
  EXPECT_EQ(Enum3Value1->getName(), "D");
  EXPECT_EQ(Enum3Value1->getValue(), "1");

  EXPECT_TRUE(isa<LibScopeView::TypeEnumerator>(*Enum3Value2));
  EXPECT_EQ(Enum3Value2->getDieOffset(), 0x00b0U);
  EXPECT_EQ(Enum3Value2->getDieTag(), DW_TAG_enumerator);
  EXPECT_EQ(Enum3Value2->getName(), "E");
  EXPECT_EQ(Enum3Value2->getValue(), "2");

  EXPECT_TRUE(isa<LibScopeView::TypeEnumerator>(*Enum3Value3));
  EXPECT_EQ(Enum3Value3->getDieOffset(), 0x00b6U);
  EXPECT_EQ(Enum3Value3->getDieTag(), DW_TAG_enumerator);
  EXPECT_EQ(Enum3Value3->getName(), "F");
  EXPECT_EQ(Enum3Value3->getValue(), "3");
}

TEST_F(TestElfDwarfReader, ReadFunction) {
  LibScopeView::Scope *CU = nullptr;
  ASSERT_TRUE(loadSingleCUFromTestFile("ElfDwarfReader/function.o", &CU));
  ASSERT_TRUE(checkChildCount(CU, 3, 2, 0));

  auto IntType = getNthTypeIn(CU, 0);

  // DW_TAG_subprogram
  EXPECT_TRUE(isa<LibScopeView::ScopeFunction>(*getNthScopeIn(CU, 0)));
  EXPECT_TRUE(getNthScopeIn(CU, 0)->getIsSubprogram());
  EXPECT_EQ(getNthScopeIn(CU, 0)->getDieOffset(), 0x002aU);
  EXPECT_EQ(getNthScopeIn(CU, 0)->getDieTag(), DW_TAG_subprogram);
  EXPECT_EQ(getNthScopeIn(CU, 0)->getLineNumber(), 1U);
  EXPECT_EQ(LibScopeView::getFileName(getNthScopeIn(CU, 0)->getFilePath()),
            "function.cpp");
  EXPECT_EQ(getNthScopeIn(CU, 0)->getName(), "func1");
  EXPECT_EQ(getNthScopeIn(CU, 0)->getType(), IntType);

  // DW_TAG_subroutine_type
  EXPECT_TRUE(isa<LibScopeView::ScopeFunction>(*getNthScopeIn(CU, 2)));
  EXPECT_TRUE(getNthScopeIn(CU, 2)->getIsSubroutineType());
  EXPECT_EQ(getNthScopeIn(CU, 2)->getDieOffset(), 0x0098U);
  EXPECT_EQ(getNthScopeIn(CU, 2)->getDieTag(), DW_TAG_subroutine_type);
  EXPECT_EQ(getNthScopeIn(CU, 2)->getType(), IntType);
  EXPECT_EQ(getNthScopeIn(CU, 2)->getName(), "int (*)(int)");

  // Pointers to DW_TAG_subroutine_type.
  CU = nullptr;
  ASSERT_TRUE(
      loadSingleCUFromTestFile("ElfDwarfReader/function_pointer.o", &CU));
  ASSERT_TRUE(checkChildCount(CU, 2, 5, 0));

  EXPECT_TRUE(isa<LibScopeView::TypeDefinition>(*getNthTypeIn(CU, 0)));
  EXPECT_EQ(getNthTypeIn(CU, 0)->getName(), "ptr_to_fptr");
  EXPECT_EQ(getNthTypeIn(CU, 0)->getType()->getName(), "INT (*)(INT,int) * *");

  // DW_AT_specification, Definition should reference declaration.
  CU = nullptr;
  ASSERT_TRUE(loadSingleCUFromTestFile("ElfDwarfReader/function_decls.o", &CU));
  ASSERT_TRUE(checkChildCount(CU, 2, 3, 0));
  ASSERT_TRUE(checkChildCount(getNthScopeIn(CU, 0), 1, 0, 0));

  auto Decl = getNthScopeIn(getNthScopeIn(CU, 0), 0);
  auto Defin = getNthScopeIn(CU, 1);

  EXPECT_TRUE(isa<LibScopeView::ScopeFunction>(*Decl));
  auto DeclFunc = dyn_cast<LibScopeView::ScopeFunction>(Decl);
  EXPECT_TRUE(isa<LibScopeView::ScopeFunction>(*Defin));
  auto DefinFunc = dyn_cast<LibScopeView::ScopeFunction>(Defin);

  EXPECT_TRUE(DeclFunc->getIsDeclaration());
  EXPECT_FALSE(DefinFunc->getIsDeclaration());

  EXPECT_EQ(Defin->getReference(), Decl);
  EXPECT_EQ(Defin->getName(), Decl->getName());
  EXPECT_EQ(Defin->getType(), Decl->getType());
  EXPECT_EQ(Defin->getLineNumber(), Decl->getLineNumber());
  EXPECT_EQ(Defin->getFilePathPoolRef(), Decl->getFilePathPoolRef());

  // Static and inlined
  CU = nullptr;
  ASSERT_TRUE(
      loadSingleCUFromTestFile("ElfDwarfReader/function_static_inline.o", &CU));
  ASSERT_TRUE(checkChildCount(CU, 5, 1, 0));

  auto Static = getNthScopeIn(CU, 2);
  EXPECT_TRUE(isa<LibScopeView::ScopeFunction>(*Static));
  EXPECT_EQ(Static->getName(), "func1");
  auto StaticFunc = dyn_cast<LibScopeView::ScopeFunction>(Static);
  EXPECT_TRUE(StaticFunc->getIsStatic());
  EXPECT_FALSE(StaticFunc->getIsDeclaredInline());

  auto Inlined = getNthScopeIn(CU, 0);
  EXPECT_TRUE(isa<LibScopeView::ScopeFunction>(*Inlined));
  EXPECT_EQ(Inlined->getName(), "func2");
  auto InlinedFunc = dyn_cast<LibScopeView::ScopeFunction>(Inlined);
  EXPECT_FALSE(InlinedFunc->getIsStatic());
  EXPECT_TRUE(InlinedFunc->getIsDeclaredInline());

  auto StaticInlined = getNthScopeIn(CU, 1);
  EXPECT_TRUE(isa<LibScopeView::ScopeFunction>(*StaticInlined));
  EXPECT_EQ(StaticInlined->getName(), "func3");
  auto StaticInlinedFunc = dyn_cast<LibScopeView::ScopeFunction>(StaticInlined);
  EXPECT_TRUE(StaticInlinedFunc->getIsStatic());
  EXPECT_TRUE(StaticInlinedFunc->getIsDeclaredInline());

  // Check static is inherited from declaration
  auto StaticDef = getNthScopeIn(CU, 3);
  EXPECT_TRUE(isa<LibScopeView::ScopeFunction>(*StaticDef));
  EXPECT_EQ(StaticDef->getName(), "func4");
  auto StaticDefFunc = dyn_cast<LibScopeView::ScopeFunction>(StaticDef);
  EXPECT_FALSE(StaticDefFunc->getIsDeclaration());
  EXPECT_TRUE(StaticDefFunc->getIsStatic());

  // DW_TAG_inlined_subroutine
  auto WithInlined = getNthScopeIn(CU, 4);
  ASSERT_EQ(countScopesIn(WithInlined), 1U);
  ASSERT_EQ(countScopesIn(getNthScopeIn(WithInlined, 0)), 2U);
  auto InlinedSub = getNthScopeIn(getNthScopeIn(WithInlined, 0), 0);

  EXPECT_TRUE(isa<LibScopeView::ScopeFunctionInlined>(*InlinedSub));
  EXPECT_EQ(InlinedSub->getDieOffset(), 0x0112U);
  EXPECT_EQ(InlinedSub->getDieTag(), DW_TAG_inlined_subroutine);
  EXPECT_EQ(InlinedSub->getLineNumber(), 5U);
  EXPECT_EQ(LibScopeView::getFileName(InlinedSub->getFilePath()),
            "function_static_inline.cpp");
  EXPECT_EQ(InlinedSub->getName(), "func2");
  EXPECT_EQ(InlinedSub->getType()->getName(), "int");
}

TEST_F(TestElfDwarfReader, ReadGlobals) {
  LibScopeView::Scope *Root = nullptr;
  ASSERT_TRUE(loadRootFromTestFile("ElfDwarfReader/lto_cross_cu.elf", &Root));
  ASSERT_TRUE(checkChildCount(Root, 2, 0, 0));

  // CU1 should contain a struct (A) containing another struct (G). G should be
  // marked as global as there is a reference to it in CU2. The member of G
  // should also be marked as global.
  auto CU1 = getNthScopeIn(Root, 0);
  ASSERT_TRUE(checkChildCount(CU1, 3, 1, 0));

  auto StructA = getNthScopeIn(CU1, 2);
  ASSERT_TRUE(checkChildCount(StructA, 1, 0, 0));

  auto StructG = getNthScopeIn(StructA, 0);
  ASSERT_TRUE(checkChildCount(StructG, 0, 0, 1));
  EXPECT_TRUE(StructG->getIsGlobalReference());
  EXPECT_TRUE(getNthSymbolIn(StructG, 0)->getIsGlobalReference());

  // The function in CU2 should be of type G.
  auto CU2 = getNthScopeIn(Root, 1);
  ASSERT_TRUE(checkChildCount(CU2, 1, 0, 0));
  EXPECT_EQ(getNthScopeIn(CU2, 0)->getType(), StructG);
}

TEST_F(TestElfDwarfReader, ReadImport) {
  LibScopeView::Scope *CU = nullptr;
  ASSERT_TRUE(loadSingleCUFromTestFile("ElfDwarfReader/import.o", &CU));
  ASSERT_TRUE(checkChildCount(CU, 4, 2, 1));

  auto Struct = getNthScopeIn(CU, 1);
  ASSERT_EQ(countTypesIn(Struct), 2U);

  // DW_TAG_imported_module
  EXPECT_TRUE(getNthTypeIn(CU, 1)->getIsImportedModule());
  EXPECT_EQ(getNthTypeIn(CU, 1)->getDieOffset(), 0x007aU);
  EXPECT_EQ(getNthTypeIn(CU, 1)->getLineNumber(), 14U);
  EXPECT_EQ(LibScopeView::getFileName(getNthTypeIn(CU, 1)->getFilePath()),
            "import.cpp");
  EXPECT_EQ(getNthTypeIn(CU, 1)->getDieTag(), DW_TAG_imported_module);
  // Check the imported namespace.
  ASSERT_NE(getNthTypeIn(CU, 1)->getType(), nullptr);
  ASSERT_TRUE(isa<LibScopeView::Scope>(*getNthTypeIn(CU, 1)->getType()));
  auto *ImpModule =
      dyn_cast<LibScopeView::Scope>(getNthTypeIn(CU, 1)->getType());
  EXPECT_EQ(ImpModule->getDieOffset(), 0x62U);
  EXPECT_TRUE(isa<LibScopeView::ScopeNamespace>(*ImpModule));
  EXPECT_EQ(ImpModule->getName(), "NS");

  // DW_TAG_imported_declaration
  EXPECT_TRUE(getNthTypeIn(Struct, 0)->getIsImportedDeclaration());
  EXPECT_EQ(getNthTypeIn(Struct, 0)->getDieOffset(), 0x0054U);
  // The using statement is on line 7, but the DWARF says line 6.
  EXPECT_EQ(getNthTypeIn(Struct, 0)->getLineNumber(), 6U);
  EXPECT_EQ(LibScopeView::getFileName(getNthTypeIn(Struct, 0)->getFilePath()),
            "import.cpp");
  EXPECT_EQ(getNthTypeIn(Struct, 0)->getDieTag(), DW_TAG_imported_declaration);
  // Check the imported member.
  ASSERT_NE(getNthTypeIn(Struct, 0)->getType(), nullptr);
  ASSERT_TRUE(isa<LibScopeView::Symbol>(*getNthTypeIn(Struct, 0)->getType()));
  auto *ImpMember =
      dyn_cast<LibScopeView::Symbol>(getNthTypeIn(Struct, 0)->getType());
  EXPECT_EQ(ImpMember->getDieOffset(), 0x37U);
  EXPECT_TRUE(ImpMember->getIsMember());
  EXPECT_EQ(ImpMember->getName(), "m");
}

TEST_F(TestElfDwarfReader, ReadInheritance) {
  LibScopeView::Scope *CU = nullptr;
  ASSERT_TRUE(loadSingleCUFromTestFile("ElfDwarfReader/inheritance.o", &CU));
  ASSERT_TRUE(checkChildCount(CU, 6, 0, 0));

  auto BaseClass = getNthScopeIn(CU, 0);
  ASSERT_TRUE(BaseClass->getIsClassType());
  EXPECT_EQ(BaseClass->getName(), "Base");

  auto PublicClass = getNthScopeIn(CU, 1);
  ASSERT_TRUE(PublicClass->getIsClassType());
  EXPECT_EQ(PublicClass->getName(), "Public");
  ASSERT_TRUE(getNthTypeIn(PublicClass, 0)->getIsInheritance());
  EXPECT_EQ(dyn_cast<LibScopeView::TypeImport>(getNthTypeIn(PublicClass, 0))
                ->getInheritanceAccess(),
            LibScopeView::AccessSpecifier::Public);

  auto PrivateClass = getNthScopeIn(CU, 2);
  ASSERT_TRUE(PrivateClass->getIsClassType());
  EXPECT_EQ(PrivateClass->getName(), "Private");
  // Compiler just leaves this as unspecified.
  EXPECT_EQ(dyn_cast<LibScopeView::TypeImport>(getNthTypeIn(PrivateClass, 0))
                ->getInheritanceAccess(),
            LibScopeView::AccessSpecifier::Unspecified);

  auto ProtectedClass = getNthScopeIn(CU, 3);
  ASSERT_TRUE(ProtectedClass->getIsClassType());
  EXPECT_EQ(ProtectedClass->getName(), "Protected");
  EXPECT_EQ(dyn_cast<LibScopeView::TypeImport>(getNthTypeIn(ProtectedClass, 0))
                ->getInheritanceAccess(),
            LibScopeView::AccessSpecifier::Protected);

  auto DefaultClass = getNthScopeIn(CU, 4);
  ASSERT_TRUE(DefaultClass->getIsClassType());
  EXPECT_EQ(DefaultClass->getName(), "Default");
  EXPECT_EQ(dyn_cast<LibScopeView::TypeImport>(getNthTypeIn(DefaultClass, 0))
                ->getInheritanceAccess(),
            LibScopeView::AccessSpecifier::Unspecified);
}

TEST_F(TestElfDwarfReader, ReadLabel) {
  LibScopeView::Scope *CU = nullptr;
  ASSERT_TRUE(loadSingleCUFromTestFile("ElfDwarfReader/label.o", &CU));
  ASSERT_TRUE(checkChildCount(CU, 1, 1, 0));
  ASSERT_TRUE(checkChildCount(getNthScopeIn(CU, 0), 1, 0, 1));

  // DW_TAG_label
  auto Label = getNthScopeIn(getNthScopeIn(CU, 0), 0);
  EXPECT_TRUE(Label->getIsLabel());
  EXPECT_EQ(Label->getDieOffset(), 0x004eU);
  EXPECT_EQ(Label->getDieTag(), DW_TAG_label);
  EXPECT_EQ(Label->getLineNumber(), 3U);
  EXPECT_EQ(LibScopeView::getFileName(Label->getFilePath()), "label.cpp");
  EXPECT_EQ(Label->getName(), "loophead");
}

TEST_F(TestElfDwarfReader, ReadLines) {
  LibScopeView::Scope *CU = nullptr;
  ASSERT_TRUE(loadSingleCUFromTestFile("ElfDwarfReader/lines.o", &CU));
  ASSERT_EQ(CU->getLines().size(), 7U);

  auto *Ln = CU->getLines().at(0);
  EXPECT_TRUE(isa<LibScopeView::Line>(*Ln));
  EXPECT_EQ(Ln->getLineNumber(), 1U);
  EXPECT_EQ(Ln->getAddress(), 0x00000000U);
  EXPECT_EQ(Ln->getDieOffset(), 0x00000000U);
  EXPECT_EQ(LibScopeView::getFileName(Ln->getFilePath()), "lines.cpp");
  EXPECT_TRUE(Ln->getIsNewStatement());
  EXPECT_FALSE(Ln->getIsNewBasicBlock());
  EXPECT_FALSE(Ln->getIsLineEndSequence());
  EXPECT_FALSE(Ln->getIsEpilogueBegin());
  EXPECT_FALSE(Ln->getIsPrologueEnd());

  Ln = CU->getLines().at(6);
  EXPECT_TRUE(isa<LibScopeView::Line>(*Ln));
  EXPECT_EQ(Ln->getLineNumber(), 13U);
  EXPECT_EQ(Ln->getAddress(), 0x00000032U);
  EXPECT_EQ(Ln->getDieOffset(), 0x00000032U);
  EXPECT_EQ(LibScopeView::getFileName(Ln->getFilePath()), "lines.cpp");
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
  EXPECT_TRUE(isa<LibScopeView::ScopeNamespace>(*getNthScopeIn(CU, 2)));
  EXPECT_EQ(getNthScopeIn(CU, 2)->getDieOffset(), 0x0062U);
  EXPECT_EQ(getNthScopeIn(CU, 2)->getDieTag(), DW_TAG_namespace);
  EXPECT_EQ(getNthScopeIn(CU, 2)->getLineNumber(), 10U);
  EXPECT_EQ(LibScopeView::getFileName(getNthScopeIn(CU, 2)->getFilePath()),
            "import.cpp");
  EXPECT_EQ(getNthScopeIn(CU, 2)->getName(), "NS");
}

TEST_F(TestElfDwarfReader, ReadMembers) {
  LibScopeView::Scope *CU = nullptr;
  ASSERT_TRUE(loadSingleCUFromTestFile("ElfDwarfReader/members.o", &CU));
  ASSERT_TRUE(checkChildCount(CU, 3, 3, 0));

  auto Struct = getNthScopeIn(CU, 0);
  EXPECT_EQ(Struct->getName(), "A");
  ASSERT_TRUE(checkChildCount(Struct, 1, 0, 4));

  // m_unspecified
  EXPECT_EQ(getNthSymbolIn(Struct, 0)->getName(), "m_unspecified");
  EXPECT_EQ(getNthSymbolIn(Struct, 0)->getAccessSpecifier(),
            LibScopeView::AccessSpecifier::Unspecified);
  // m_private
  EXPECT_EQ(getNthSymbolIn(Struct, 1)->getName(), "m_private");
  EXPECT_EQ(getNthSymbolIn(Struct, 1)->getAccessSpecifier(),
            LibScopeView::AccessSpecifier::Private);
  // m_public - Clang just puts unspecified for structs.
  EXPECT_EQ(getNthSymbolIn(Struct, 2)->getName(), "m_public");
  EXPECT_EQ(getNthSymbolIn(Struct, 2)->getAccessSpecifier(),
            LibScopeView::AccessSpecifier::Unspecified);
  // m_protected
  EXPECT_EQ(getNthSymbolIn(Struct, 3)->getName(), "m_protected");
  EXPECT_EQ(getNthSymbolIn(Struct, 3)->getAccessSpecifier(),
            LibScopeView::AccessSpecifier::Protected);
}

TEST_F(TestElfDwarfReader, ReadQualifiedNames) {
  LibScopeView::Scope *CU = nullptr;
  ASSERT_TRUE(loadSingleCUFromTestFile("ElfDwarfReader/qualified_name.o", &CU));
  ASSERT_TRUE(checkChildCount(CU, 2, 1, 1));
  auto Parent1 = getNthScopeIn(CU, 0);
  ASSERT_TRUE(checkChildCount(getNthScopeIn(CU, 0), 1, 0, 0));
  auto Parent2 = getNthScopeIn(Parent1, 0);
  ASSERT_TRUE(checkChildCount(Parent2, 1, 0, 0));

  auto ClassC = getNthScopeIn(Parent2, 0);
  ASSERT_TRUE(checkChildCount(ClassC, 0, 1, 2));
  EXPECT_TRUE(ClassC->getHasQualifiedName());
  EXPECT_EQ(dyn_cast<LibScopeView::Element>(ClassC)->getQualifiedName(),
            "OuterNS::InnerNS::");

  auto Typedef = getNthTypeIn(ClassC, 0);
  EXPECT_TRUE(Typedef->getHasQualifiedName());
  EXPECT_EQ(dyn_cast<LibScopeView::Element>(Typedef)->getQualifiedName(),
            "OuterNS::InnerNS::C::");

  auto StaticMemDefin = getNthSymbolIn(CU, 0);
  EXPECT_TRUE(StaticMemDefin->getHasQualifiedName());
  EXPECT_EQ(dyn_cast<LibScopeView::Element>(StaticMemDefin)->getQualifiedName(),
            "OuterNS::InnerNS::C::");
}

TEST_F(TestElfDwarfReader, ReadSymbols) {
  LibScopeView::Scope *CU = nullptr;
  ASSERT_TRUE(loadSingleCUFromTestFile("ElfDwarfReader/symbol.o", &CU));
  ASSERT_TRUE(checkChildCount(CU, 3, 1, 1));
  ASSERT_TRUE(checkChildCount(getNthScopeIn(CU, 0), 0, 0, 1));
  ASSERT_TRUE(checkChildCount(getNthScopeIn(CU, 1), 0, 0, 2));
  ASSERT_TRUE(checkChildCount(getNthScopeIn(CU, 2), 0, 0, 1));

  auto Var = getNthSymbolIn(CU, 0);
  auto Member = getNthSymbolIn(getNthScopeIn(CU, 0), 0);
  auto Param = getNthSymbolIn(getNthScopeIn(CU, 1), 0);
  auto UParams = getNthSymbolIn(getNthScopeIn(CU, 2), 0);

  auto IntType = getNthTypeIn(CU, 0);

  // DW_TAG_variable
  EXPECT_TRUE(Var->getIsVariable());
  EXPECT_EQ(Var->getDieOffset(), 0x002aU);
  EXPECT_EQ(Var->getDieTag(), DW_TAG_variable);
  EXPECT_EQ(Var->getLineNumber(), 1U);
  EXPECT_EQ(LibScopeView::getFileName(Var->getFilePath()), "symbol.cpp");
  EXPECT_EQ(Var->getName(), "var");
  EXPECT_EQ(Var->getType(), IntType);

  // DW_TAG_member
  EXPECT_TRUE(Member->getIsMember());
  EXPECT_EQ(Member->getDieOffset(), 0x004eU);
  EXPECT_EQ(Member->getDieTag(), DW_TAG_member);
  EXPECT_EQ(Member->getLineNumber(), 4U);
  EXPECT_EQ(LibScopeView::getFileName(Member->getFilePath()), "symbol.cpp");
  EXPECT_EQ(Member->getName(), "mem");
  EXPECT_EQ(Member->getType(), IntType);

  // DW_TAG_formal_parameter
  EXPECT_TRUE(Param->getIsParameter());
  EXPECT_EQ(Param->getDieOffset(), 0x0074U);
  EXPECT_EQ(Param->getDieTag(), DW_TAG_formal_parameter);
  EXPECT_EQ(Param->getLineNumber(), 7U);
  EXPECT_EQ(LibScopeView::getFileName(Param->getFilePath()), "symbol.cpp");
  EXPECT_EQ(Param->getName(), "param");
  EXPECT_EQ(Param->getType(), IntType);

  // DW_TAG_unspecified_parameters
  EXPECT_TRUE(UParams->getIsUnspecifiedParameter());
  EXPECT_EQ(UParams->getDieOffset(), 0x00aaU);
  EXPECT_EQ(UParams->getDieTag(), DW_TAG_unspecified_parameters);
  EXPECT_EQ(UParams->getName(), "");

  // Test Symbol to Symbol DW_TAG_specification.
  // The file import.o has such a symbol.
  ASSERT_TRUE(loadSingleCUFromTestFile("ElfDwarfReader/import.o", &CU));
  ASSERT_TRUE(checkChildCount(CU, 4, 2, 1));
  ASSERT_NE(getNthSymbolIn(CU, 0)->getReference(), nullptr);
  EXPECT_EQ(getNthSymbolIn(CU, 0)->getReference()->getDieOffset(), 0x6cU);
  EXPECT_EQ(getNthSymbolIn(CU, 0)->getReference()->getName(), "x");
}

TEST_F(TestElfDwarfReader, ReadTemplates) {
  LibScopeView::Scope *CU = nullptr;
  ASSERT_TRUE(loadSingleCUFromTestFile("ElfDwarfReader/template.o", &CU));
  ASSERT_TRUE(checkChildCount(CU, 3, 1, 0));
  ASSERT_EQ(countTypesIn(getNthScopeIn(CU, 1)), 2U);
  ASSERT_EQ(countTypesIn(getNthScopeIn(CU, 2)), 2U);
  EXPECT_EQ(getNthScopeIn(CU, 1)->getName(), "t_func<1, int>");
  EXPECT_EQ(getNthScopeIn(CU, 2)->getName(), "t_func<-1, int>");

  auto IntType = getNthTypeIn(CU, 0);

  std::string NotTemplateFailure(
      "A function containing template parameters should be a template");
  EXPECT_TRUE(getNthScopeIn(CU, 1)->getIsTemplate()) << NotTemplateFailure;
  EXPECT_TRUE(getNthScopeIn(CU, 2)->getIsTemplate()) << NotTemplateFailure;

  // DW_TAG_template_value_parameter.
  auto ValParam = getNthTypeIn(getNthScopeIn(CU, 1), 0);
  EXPECT_TRUE(ValParam->getIsTemplateValue());
  EXPECT_EQ(ValParam->getDieOffset(), 0x005cU);
  EXPECT_EQ(ValParam->getDieTag(), DW_TAG_template_value_parameter);
  EXPECT_EQ(ValParam->getType(), IntType);
  EXPECT_EQ(ValParam->getName(), "VAL");
  EXPECT_EQ(ValParam->getValue(), "1");

  // DW_TAG_template_type_parameter.
  auto TypeParam = getNthTypeIn(getNthScopeIn(CU, 1), 1);
  EXPECT_TRUE(TypeParam->getIsTemplateType());
  EXPECT_EQ(TypeParam->getDieOffset(), 0x0066U);
  EXPECT_EQ(TypeParam->getDieTag(), DW_TAG_template_type_parameter);
  EXPECT_EQ(TypeParam->getType(), IntType);
  EXPECT_EQ(TypeParam->getName(), "TY");

  // Test negative template value.
  auto NegativeValParam = getNthTypeIn(getNthScopeIn(CU, 2), 0);
  EXPECT_TRUE(NegativeValParam->getIsTemplateValue());
  EXPECT_EQ(NegativeValParam->getDieOffset(), 0x0089U);
  EXPECT_EQ(NegativeValParam->getDieTag(), DW_TAG_template_value_parameter);
  EXPECT_EQ(NegativeValParam->getType(), IntType);
  EXPECT_EQ(NegativeValParam->getName(), "VAL");
  EXPECT_EQ(NegativeValParam->getValue(), "-1");
}

TEST_F(TestElfDwarfReader, ReadTemplatePacks) {
  LibScopeView::Scope *CU = nullptr;
  ASSERT_TRUE(loadSingleCUFromTestFile("ElfDwarfReader/template_pack.o", &CU));
  ASSERT_TRUE(checkChildCount(CU, 5, 1, 0));
  ASSERT_TRUE(checkChildCount(getNthScopeIn(CU, 1), 1, 0, 3));
  EXPECT_EQ(getNthScopeIn(CU, 1)->getName(), "sum<int, int>");
  auto *TPack2Templates = getNthScopeIn(getNthScopeIn(CU, 1), 0);
  ASSERT_TRUE(checkChildCount(getNthScopeIn(CU, 2), 1, 0, 2));
  EXPECT_EQ(getNthScopeIn(CU, 2)->getName(), "sum<int>");
  auto *TPack1Template = getNthScopeIn(getNthScopeIn(CU, 2), 0);
  ASSERT_TRUE(checkChildCount(getNthScopeIn(CU, 3), 1, 0, 1));
  EXPECT_EQ(getNthScopeIn(CU, 3)->getName(), "sum<>");
  auto *TPack0Templates = getNthScopeIn(getNthScopeIn(CU, 3), 0);

  auto *IntType = getNthTypeIn(CU, 0);

  std::string NotTemplateFailure(
      "A function containing template parameters should be a template");
  EXPECT_TRUE(getNthScopeIn(CU, 1)->getIsTemplate()) << NotTemplateFailure;
  EXPECT_TRUE(getNthScopeIn(CU, 2)->getIsTemplate()) << NotTemplateFailure;
  EXPECT_TRUE(getNthScopeIn(CU, 3)->getIsTemplate()) << NotTemplateFailure;

  // DW_TAG_GNU_template_parameter_pack.
  EXPECT_TRUE(isa<LibScopeView::ScopeTemplatePack>(*TPack2Templates));
  EXPECT_EQ(TPack2Templates->getDieOffset(), 0x008aU);
  EXPECT_EQ(TPack2Templates->getDieTag(), DW_TAG_GNU_template_parameter_pack);
  EXPECT_EQ(TPack2Templates->getName(), "Targs");
  // Two template parameters of type int.
  ASSERT_TRUE(checkChildCount(TPack2Templates, 0, 2, 0));
  EXPECT_TRUE(getNthTypeIn(TPack2Templates, 0)->getIsTemplateType());
  EXPECT_EQ(getNthTypeIn(TPack2Templates, 0)->getType(), IntType);
  EXPECT_TRUE(getNthTypeIn(TPack2Templates, 1)->getIsTemplateType());
  EXPECT_EQ(getNthTypeIn(TPack2Templates, 1)->getType(), IntType);

  EXPECT_TRUE(isa<LibScopeView::ScopeTemplatePack>(*TPack1Template));
  EXPECT_EQ(TPack1Template->getDieOffset(), 0x00d4U);
  EXPECT_EQ(TPack1Template->getDieTag(), DW_TAG_GNU_template_parameter_pack);
  EXPECT_EQ(TPack1Template->getName(), "Targs");
  // One template parameter of type int.
  ASSERT_TRUE(checkChildCount(TPack1Template, 0, 1, 0));
  EXPECT_TRUE(getNthTypeIn(TPack1Template, 0)->getIsTemplateType());
  EXPECT_EQ(getNthTypeIn(TPack1Template, 0)->getType(), IntType);

  EXPECT_TRUE(isa<LibScopeView::ScopeTemplatePack>(*TPack0Templates));
  EXPECT_EQ(TPack0Templates->getDieOffset(), 0x010bU);
  EXPECT_EQ(TPack0Templates->getDieTag(), DW_TAG_GNU_template_parameter_pack);
  EXPECT_EQ(TPack0Templates->getName(), "Targs");
  // No template parameters.
  EXPECT_TRUE(checkChildCount(TPack0Templates, 0, 0, 0));
}

TEST_F(TestElfDwarfReader, ReadTemplateTemplates) {
  LibScopeView::Scope *CU = nullptr;
  ASSERT_TRUE(
      loadSingleCUFromTestFile("ElfDwarfReader/template_template.o", &CU));
  ASSERT_TRUE(checkChildCount(CU, 3, 2, 0));
  ASSERT_EQ(countTypesIn(getNthScopeIn(CU, 2)), 2U);
  EXPECT_EQ(getNthScopeIn(CU, 2)->getName(), "foo<int, vector>");

  EXPECT_TRUE(getNthScopeIn(CU, 2)->getIsTemplate())
      << "A function containing template parameters should be a template";

  // DW_TAG_GNU_template_template_parameter.
  auto TTParam = getNthTypeIn(getNthScopeIn(CU, 2), 1);
  EXPECT_TRUE(TTParam->getIsTemplateTemplate());
  EXPECT_EQ(TTParam->getDieOffset(), 0x00acU);
  EXPECT_EQ(TTParam->getDieTag(), DW_TAG_GNU_template_template_parameter);
  EXPECT_EQ(TTParam->getName(), "V");
  EXPECT_EQ(TTParam->getValue(), "vector");
}

TEST_F(TestElfDwarfReader, ReadTypes) {
  LibScopeView::Scope *CU = nullptr;
  ASSERT_TRUE(loadSingleCUFromTestFile("ElfDwarfReader/type.o", &CU));
  ASSERT_EQ(countTypesIn(CU), 8U);

  auto RVal = getNthTypeIn(CU, 0);
  auto Base = getNthTypeIn(CU, 1);
  auto TDef = getNthTypeIn(CU, 2);
  auto Const = getNthTypeIn(CU, 3);
  auto Ptr = getNthTypeIn(CU, 4);
  auto Ref = getNthTypeIn(CU, 5);
  auto Restrict = getNthTypeIn(CU, 6);
  auto Volatile = getNthTypeIn(CU, 7);

  // DW_TAG_base_type
  EXPECT_TRUE(Base->getIsBaseType());
  EXPECT_EQ(Base->getDieOffset(), 0x00c5U);
  EXPECT_EQ(Base->getDieTag(), DW_TAG_base_type);
  EXPECT_EQ(Base->getName(), "int");
  EXPECT_EQ(Base->getType(), nullptr);
  EXPECT_EQ(Base->getByteSize(), 4U);

  // DW_TAG_const_type
  EXPECT_TRUE(Const->getIsConstType());
  EXPECT_EQ(Const->getDieOffset(), 0x00d7U);
  EXPECT_EQ(Const->getDieTag(), DW_TAG_const_type);
  EXPECT_EQ(Const->getName(), "const int");
  EXPECT_EQ(Const->getType(), Base);

  // DW_TAG_pointer_type
  EXPECT_TRUE(Ptr->getIsPointerType());
  EXPECT_EQ(Ptr->getDieOffset(), 0x00dcU);
  EXPECT_EQ(Ptr->getDieTag(), DW_TAG_pointer_type);
  EXPECT_EQ(Ptr->getName(), "int *");
  EXPECT_EQ(Ptr->getType(), Base);

  // DW_TAG_reference_type
  EXPECT_TRUE(Ref->getIsReferenceType());
  EXPECT_EQ(Ref->getDieOffset(), 0x00e1U);
  EXPECT_EQ(Ref->getDieTag(), DW_TAG_reference_type);
  EXPECT_EQ(Ref->getName(), "int &");
  EXPECT_EQ(Ref->getType(), Base);

  // DW_TAG_restrict_type
  EXPECT_TRUE(Restrict->getIsRestrictType());
  EXPECT_EQ(Restrict->getDieOffset(), 0x00e6U);
  EXPECT_EQ(Restrict->getDieTag(), DW_TAG_restrict_type);
  EXPECT_EQ(Restrict->getName(), "restrict int *");
  EXPECT_EQ(Restrict->getType(), Ptr);

  // DW_TAG_rvalue_reference_type
  EXPECT_TRUE(RVal->getIsRvalueReferenceType());
  EXPECT_EQ(RVal->getDieOffset(), 0x00c0U);
  EXPECT_EQ(RVal->getDieTag(), DW_TAG_rvalue_reference_type);
  EXPECT_EQ(RVal->getName(), "int &&");
  EXPECT_EQ(RVal->getType(), Base);

  // DW_TAG_typedef
  EXPECT_TRUE(isa<LibScopeView::TypeDefinition>(*TDef));
  EXPECT_EQ(TDef->getDieOffset(), 0x00ccU);
  EXPECT_EQ(TDef->getDieTag(), DW_TAG_typedef);
  EXPECT_EQ(TDef->getLineNumber(), 1U);
  EXPECT_EQ(LibScopeView::getFileName(TDef->getFilePath()), "type.cpp");
  EXPECT_EQ(TDef->getName(), "T_INT");
  EXPECT_EQ(TDef->getType(), Base);

  // DW_TAG_volatile_type
  EXPECT_TRUE(Volatile->getIsVolatileType());
  EXPECT_EQ(Volatile->getDieOffset(), 0x00ebU);
  EXPECT_EQ(Volatile->getDieTag(), DW_TAG_volatile_type);
  EXPECT_EQ(Volatile->getName(), "volatile int");
  EXPECT_EQ(Volatile->getType(), Base);

  ASSERT_TRUE(loadSingleCUFromTestFile("ElfDwarfReader/more_types.elf", &CU));
  ASSERT_TRUE(checkChildCount(CU, 1, 2, 0));

  auto Alias = getNthScopeIn(CU, 0);
  auto Unspec = getNthTypeIn(CU, 0);
  auto PtrToMem = getNthTypeIn(CU, 1);

  // DW_TAG_template_alias
  EXPECT_TRUE(Alias->getIsTemplate());
  EXPECT_TRUE(isa<LibScopeView::ScopeAlias>(*Alias));
  EXPECT_EQ(Alias->getDieOffset(), 0x12U);
  EXPECT_EQ(Alias->getDieTag(), DW_TAG_template_alias);
  EXPECT_EQ(Alias->getName(), "alias");
  EXPECT_EQ(Alias->getType(), nullptr);

  // DW_TAG_unspecified_type
  EXPECT_TRUE(Unspec->getIsUnspecifiedType());
  EXPECT_EQ(Unspec->getDieOffset(), 0x0cU);
  EXPECT_EQ(Unspec->getDieTag(), DW_TAG_unspecified_type);
  EXPECT_EQ(Unspec->getName(), "void");
  EXPECT_EQ(Unspec->getType(), nullptr);

  // DW_TAG_ptr_to_member_type
  EXPECT_TRUE(PtrToMem->getIsPointerMemberType());
  EXPECT_EQ(PtrToMem->getDieOffset(), 0x19U);
  EXPECT_EQ(PtrToMem->getDieTag(), DW_TAG_ptr_to_member_type);
  EXPECT_EQ(PtrToMem->getName(), "void *");
  EXPECT_EQ(PtrToMem->getType(), Unspec);
}

TEST_F(TestElfDwarfReader, ReadInvalidFileIndex) {
  LibScopeView::Scope *CU = nullptr;
  ASSERT_TRUE(
      loadSingleCUFromTestFile("ElfDwarfReader/invalid_file_index.elf", &CU));
  ASSERT_EQ(countScopesIn(CU), 1U);

  auto ScopeWithBadFile = getNthScopeIn(CU, 0);
  EXPECT_TRUE(ScopeWithBadFile->getInvalidFileName());
}
