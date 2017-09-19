//===-- UnitTests/TestLibScopeView/TestAttributes.cpp -----------*- C++ -*-===//
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
/// Tests for LibScopeView::Object getAttributesAsText methods
///
//===----------------------------------------------------------------------===//

#include "Reader.h"
#include "Symbol.h"
#include "Type.h"

#include "dwarf.h"
#include "gtest/gtest.h"

using namespace LibScopeView;

TEST(ObjectAttributes, getAttributesAsText) {
  Reader R;
  setReader(&R);

  ScopeRoot Root(-1);
  Root.setIsRoot();
  Root.setDieOffset(0x0cae);
  Root.setDieTag(/*DW_TAG_file*/0);
  Root.setIsGlobalReference();

  ScopeCompileUnit *CompileUnit = new ScopeCompileUnit(0U);
  CompileUnit->setIsCompileUnit();
  CompileUnit->setDieOffset(0x0b);
  CompileUnit->setDieTag(DW_TAG_compile_unit);
  CompileUnit->setIsGlobalReference();

  ScopeNamespace *Namespace = new ScopeNamespace(1U);
  Namespace->setIsNamespace();
  Namespace->setDieOffset(0x0c);
  Namespace->setDieTag(DW_TAG_namespace);
  Namespace->setIsGlobalReference();

  Symbol *Variable = new Symbol(2U);
  Variable->setIsVariable();
  Variable->setDieOffset(0xba);
  Variable->setDieTag(DW_TAG_variable);
  Variable->setIsGlobalReference();

  TypeDefinition *Typedef = new TypeDefinition(2U);
  Typedef->setIsTypedef();
  Typedef->setDieOffset(0xce);
  Typedef->setDieTag(DW_TAG_typedef);
  Typedef->setIsGlobalReference();

  Root.addObject(CompileUnit);
  CompileUnit->addObject(Namespace);
  Namespace->addObject(Variable);
  Namespace->addObject(Typedef);
  Root.setIsGlobalReference();

  // Test the object and parent DIE offsets.
  // Test the type, level and DWARF tag.
  R.getPrintSettings().ShowDWARFOffset = true;
  R.getPrintSettings().ShowDWARFParent = true;
  R.getPrintSettings().ShowLevel = true;
  R.getPrintSettings().ShowIsGlobal = true;
  R.getPrintSettings().ShowDWARFTag = true;

  EXPECT_EQ(Root.getAttributesAsText(),         "                           X                                          ");
  EXPECT_EQ(CompileUnit->getAttributesAsText(), "[0x0000000b][0x00000cae]000X[DW_TAG_compile_unit]                     ");
  EXPECT_EQ(Namespace->getAttributesAsText(),   "[0x0000000c][0x0000000b]001X[DW_TAG_namespace]                        ");
  EXPECT_EQ(Variable->getAttributesAsText(),    "[0x000000ba][0x0000000c]002X[DW_TAG_variable]                         ");
  EXPECT_EQ(Typedef->getAttributesAsText(),     "[0x000000ce][0x0000000c]002X[DW_TAG_typedef]                          ");

  R.getPrintSettings().ShowDWARFOffset = false;
  R.getPrintSettings().ShowDWARFParent = false;
  R.getPrintSettings().ShowLevel = false;
  R.getPrintSettings().ShowIsGlobal = false;
  R.getPrintSettings().ShowDWARFTag = false;

  EXPECT_EQ(Root.getAttributesAsText(),         "");
  EXPECT_EQ(CompileUnit->getAttributesAsText(), "");
  EXPECT_EQ(Namespace->getAttributesAsText(),   "");
  EXPECT_EQ(Variable->getAttributesAsText(),    "");
  EXPECT_EQ(Typedef->getAttributesAsText(),     "");

  // Test the object and parent DIE offsets.
  R.getPrintSettings().ShowDWARFOffset = true;
  R.getPrintSettings().ShowDWARFParent = true;

  EXPECT_EQ(Root.getAttributesAsText(),         "                        ");
  EXPECT_EQ(CompileUnit->getAttributesAsText(), "[0x0000000b][0x00000cae]");
  EXPECT_EQ(Namespace->getAttributesAsText(),   "[0x0000000c][0x0000000b]");
  EXPECT_EQ(Variable->getAttributesAsText(),    "[0x000000ba][0x0000000c]");
  EXPECT_EQ(Typedef->getAttributesAsText(),     "[0x000000ce][0x0000000c]");

  R.getPrintSettings().ShowDWARFOffset = false;
  R.getPrintSettings().ShowDWARFParent = false;

  EXPECT_EQ(Root.getAttributesAsText(),         "");
  EXPECT_EQ(CompileUnit->getAttributesAsText(), "");
  EXPECT_EQ(Namespace->getAttributesAsText(),   "");
  EXPECT_EQ(Variable->getAttributesAsText(),    "");
  EXPECT_EQ(Typedef->getAttributesAsText(),     "");

  // Test the object DIE offset.
  R.getPrintSettings().ShowDWARFOffset = true;

  EXPECT_EQ(Root.getAttributesAsText(),         "            ");
  EXPECT_EQ(CompileUnit->getAttributesAsText(), "[0x0000000b]");
  EXPECT_EQ(Namespace->getAttributesAsText(),   "[0x0000000c]");
  EXPECT_EQ(Variable->getAttributesAsText(),    "[0x000000ba]");
  EXPECT_EQ(Typedef->getAttributesAsText(),     "[0x000000ce]");

  R.getPrintSettings().ShowDWARFOffset = false;

  EXPECT_EQ(Root.getAttributesAsText(),         "");
  EXPECT_EQ(CompileUnit->getAttributesAsText(), "");
  EXPECT_EQ(Namespace->getAttributesAsText(),   "");
  EXPECT_EQ(Variable->getAttributesAsText(),    "");
  EXPECT_EQ(Typedef->getAttributesAsText(),     "");

  // Test the object parent DIE offset.
  R.getPrintSettings().ShowDWARFParent = true;

  EXPECT_EQ(Root.getAttributesAsText(),         "            ");
  EXPECT_EQ(CompileUnit->getAttributesAsText(), "[0x00000cae]");
  EXPECT_EQ(Namespace->getAttributesAsText(),   "[0x0000000b]");
  EXPECT_EQ(Variable->getAttributesAsText(),    "[0x0000000c]");
  EXPECT_EQ(Typedef->getAttributesAsText(),     "[0x0000000c]");

  R.getPrintSettings().ShowDWARFParent = false;

  EXPECT_EQ(Root.getAttributesAsText(),         "");
  EXPECT_EQ(CompileUnit->getAttributesAsText(), "");
  EXPECT_EQ(Namespace->getAttributesAsText(),   "");
  EXPECT_EQ(Variable->getAttributesAsText(),    "");
  EXPECT_EQ(Typedef->getAttributesAsText(),     "");

  // Test the object level.
  R.getPrintSettings().ShowLevel = true;

  EXPECT_EQ(Root.getAttributesAsText(), "   ");
  EXPECT_EQ(CompileUnit->getAttributesAsText(), "000");
  EXPECT_EQ(Namespace->getAttributesAsText(), "001");
  EXPECT_EQ(Variable->getAttributesAsText(), "002");
  EXPECT_EQ(Typedef->getAttributesAsText(), "002");

  R.getPrintSettings().ShowLevel = false;

  EXPECT_EQ(Root.getAttributesAsText(), "");
  EXPECT_EQ(CompileUnit->getAttributesAsText(), "");
  EXPECT_EQ(Namespace->getAttributesAsText(), "");
  EXPECT_EQ(Variable->getAttributesAsText(), "");
  EXPECT_EQ(Typedef->getAttributesAsText(), "");

  // Test the object DWARF tag.
  R.getPrintSettings().ShowDWARFTag = true;

  EXPECT_EQ(Root.getAttributesAsText(),         "                                          ");
  EXPECT_EQ(CompileUnit->getAttributesAsText(), "[DW_TAG_compile_unit]                     ");
  EXPECT_EQ(Namespace->getAttributesAsText(),   "[DW_TAG_namespace]                        ");
  EXPECT_EQ(Variable->getAttributesAsText(),    "[DW_TAG_variable]                         ");
  EXPECT_EQ(Typedef->getAttributesAsText(),     "[DW_TAG_typedef]                          ");

  R.getPrintSettings().ShowDWARFTag = false;

  EXPECT_EQ(Root.getAttributesAsText(),         "");
  EXPECT_EQ(CompileUnit->getAttributesAsText(), "");
  EXPECT_EQ(Namespace->getAttributesAsText(),   "");
  EXPECT_EQ(Variable->getAttributesAsText(),    "");
  EXPECT_EQ(Typedef->getAttributesAsText(),     "");

  // Test the object 'global' status.
  R.getPrintSettings().ShowIsGlobal = true;

  EXPECT_EQ(Root.getAttributesAsText(), "X");
  EXPECT_EQ(CompileUnit->getAttributesAsText(), "X");
  EXPECT_EQ(Namespace->getAttributesAsText(), "X");
  EXPECT_EQ(Variable->getAttributesAsText(), "X");
  EXPECT_EQ(Typedef->getAttributesAsText(), "X");

  R.getPrintSettings().ShowIsGlobal = false;

  EXPECT_EQ(Root.getAttributesAsText(), "");
  EXPECT_EQ(CompileUnit->getAttributesAsText(), "");
  EXPECT_EQ(Namespace->getAttributesAsText(), "");
  EXPECT_EQ(Variable->getAttributesAsText(), "");
  EXPECT_EQ(Typedef->getAttributesAsText(), "");
}
