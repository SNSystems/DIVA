//===-- UnitTests/TestLibScopeView/TestObject.cpp ---------------*- C++ -*-===//
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
/// Tests for LibScopeView::Object.
///
//===----------------------------------------------------------------------===//

#include "Line.h"
#include "Object.h"
#include "Scope.h"
#include "Symbol.h"
#include "Type.h"

#include "dwarf.h"
#include "gtest/gtest.h"

#include <memory>

using namespace LibScopeView;

namespace {

// Object class where the properties can be set and getCommonYAML is public.
class TestObject : public Scope {
public:
  TestObject() : Scope(SV_Scope), Type(nullptr) {}

  const char *getName() const override { return Name.c_str(); }
  size_t getNameIndex() const override { return 0; }
  virtual void setNameIndex(size_t NameIndex) override {}
  void setName(const char *name) override { Name = name; }
  const char *getQualifiedName() const override { return QName.c_str(); }
  void setQualifiedName(const char *name) override {
    setHasQualifiedName();
    QName = name;
  }
  const char *getTypeName() const override { return ""; }
  std::string getFileName(bool format_options) const override {
    return FileName.c_str();
  }
  void setFileName(const char *fileName) override { FileName = fileName; }
  size_t getFileNameIndex() const override { return 0; }
  virtual void setFileNameIndex(size_t FilenameIndex) override{};
  Object *getType() const override { return Type; }
  const char *getTypeQualifiedName() const override { return nullptr; }
  bool isNamed() const override { return false; }
  void setType(Object *object) override { Type = object; }

  std::string getAsText(const PrintSettings &) const override { return ""; };
  std::string getAsYAML() const override { return ""; };

  using Object::getCommonYAML;

private:
  std::string Name;
  std::string QName;
  std::string FileName;
  Object *Type;
};

} // namespace

TEST(Object, getCommonYAML) {
  TestObject TO;
  TO.setIsBlock(); // For getKindAsString.
  EXPECT_EQ(TO.getCommonYAML(), "object: \"Block\"\n"
                                "name: null\n"
                                "type: null\n"
                                "source:\n"
                                "  line: null\n"
                                "  file: null\n"
                                "dwarf:\n"
                                "  offset: 0x0\n"
                                "  tag: null");

  TO.setName("VarName");
  EXPECT_EQ(TO.getCommonYAML(), "object: \"Block\"\n"
                                "name: \"VarName\"\n"
                                "type: null\n"
                                "source:\n"
                                "  line: null\n"
                                "  file: null\n"
                                "dwarf:\n"
                                "  offset: 0x0\n"
                                "  tag: null");

  TO.setQualifiedName("Q::");
  EXPECT_EQ(TO.getCommonYAML(), "object: \"Block\"\n"
                                "name: \"Q::VarName\"\n"
                                "type: null\n"
                                "source:\n"
                                "  line: null\n"
                                "  file: null\n"
                                "dwarf:\n"
                                "  offset: 0x0\n"
                                "  tag: null");

  Type Ty;
  Ty.setName("Ty");
  TO.setType(&Ty);
  EXPECT_EQ(TO.getCommonYAML(), "object: \"Block\"\n"
                                "name: \"Q::VarName\"\n"
                                "type: \"Ty\"\n"
                                "source:\n"
                                "  line: null\n"
                                "  file: null\n"
                                "dwarf:\n"
                                "  offset: 0x0\n"
                                "  tag: null");

  Ty.setQualifiedName("Class::");
  Ty.setHasQualifiedName();
  EXPECT_EQ(TO.getCommonYAML(), "object: \"Block\"\n"
                                "name: \"Q::VarName\"\n"
                                "type: \"Class::Ty\"\n"
                                "source:\n"
                                "  line: null\n"
                                "  file: null\n"
                                "dwarf:\n"
                                "  offset: 0x0\n"
                                "  tag: null");

  TO.setLineNumber(25);
  EXPECT_EQ(TO.getCommonYAML(), "object: \"Block\"\n"
                                "name: \"Q::VarName\"\n"
                                "type: \"Class::Ty\"\n"
                                "source:\n"
                                "  line: 25\n"
                                "  file: null\n"
                                "dwarf:\n"
                                "  offset: 0x0\n"
                                "  tag: null");

  TO.setFileName("path/file.cpp");
  EXPECT_EQ(TO.getCommonYAML(), "object: \"Block\"\n"
                                "name: \"Q::VarName\"\n"
                                "type: \"Class::Ty\"\n"
                                "source:\n"
                                "  line: 25\n"
                                "  file: \"path/file.cpp\"\n"
                                "dwarf:\n"
                                "  offset: 0x0\n"
                                "  tag: null");

  TO.setInvalidFileName();
  EXPECT_EQ(TO.getCommonYAML(), "object: \"Block\"\n"
                                "name: \"Q::VarName\"\n"
                                "type: \"Class::Ty\"\n"
                                "source:\n"
                                "  line: 25\n"
                                "  file: \"?\"\n"
                                "dwarf:\n"
                                "  offset: 0x0\n"
                                "  tag: null");

  TO.setDieOffset(0x201);
  EXPECT_EQ(TO.getCommonYAML(), "object: \"Block\"\n"
                                "name: \"Q::VarName\"\n"
                                "type: \"Class::Ty\"\n"
                                "source:\n"
                                "  line: 25\n"
                                "  file: \"?\"\n"
                                "dwarf:\n"
                                "  offset: 0x201\n"
                                "  tag: null");

  TO.setDieTag(DW_TAG_variable);
  EXPECT_EQ(TO.getCommonYAML(), "object: \"Block\"\n"
                                "name: \"Q::VarName\"\n"
                                "type: \"Class::Ty\"\n"
                                "source:\n"
                                "  line: 25\n"
                                "  file: \"?\"\n"
                                "dwarf:\n"
                                "  offset: 0x201\n"
                                "  tag: \"DW_TAG_variable\"");
}

TEST(Object, ResolveQualifiedName) {
  ScopeNamespace NS1;
  NS1.setName("NS1");

  ScopeNamespace NS2;
  NS2.setName("NS2");

  ScopeFunction Func;
  Func.setName("Function");

  ScopeRoot Root;
  Root.setName("Root");

  ScopeCompileUnit CU;
  CU.setName("CU");

  Scope Block;
  Block.setIsLexicalBlock();

  // Single parent.
  {
    Symbol Sym;
    Sym.resolveQualifiedName(&NS1);
    EXPECT_TRUE(Sym.getHasQualifiedName());
    EXPECT_STREQ(Sym.getQualifiedName(), "NS1::");
  }

  // Multiple parents.
  {
    NS2.setParent(&NS1);
    Symbol Sym;
    Sym.resolveQualifiedName(&NS2);
    EXPECT_TRUE(Sym.getHasQualifiedName());
    EXPECT_STREQ(Sym.getQualifiedName(), "NS1::NS2::");
  }

  // Function parent.
  {
    Func.setParent(&NS2);
    Symbol Sym;
    Sym.resolveQualifiedName(&Func);
    EXPECT_FALSE(Sym.getHasQualifiedName());
    EXPECT_STREQ(Sym.getQualifiedName(), "");
  }

  // Stop at ScopeRoot.
  {
    NS1.setParent(&Root);
    Symbol Sym;
    Sym.resolveQualifiedName(&NS2);
    EXPECT_TRUE(Sym.getHasQualifiedName());
    EXPECT_STREQ(Sym.getQualifiedName(), "NS1::NS2::");
  }

  // Stop at CU.
  {
    NS1.setParent(&CU);
    Symbol Sym;
    Sym.resolveQualifiedName(&NS2);
    EXPECT_TRUE(Sym.getHasQualifiedName());
    EXPECT_STREQ(Sym.getQualifiedName(), "NS1::NS2::");
  }

  // Parent with no name.
  {
    NS2.setParent(&Block);
    Block.setParent(&NS1);
    Symbol Sym;
    Sym.resolveQualifiedName(&NS2);
    EXPECT_TRUE(Sym.getHasQualifiedName());
    EXPECT_STREQ(Sym.getQualifiedName(), "NS1::NS2::");
  }
}

TEST(Object, getKind) {
  std::unique_ptr<Object> Obj;

#define CHECK_GET_KIND(TY, KIND)                                               \
  Obj = std::make_unique<TY>();                                                \
  EXPECT_EQ(Obj->getKind(), KIND)

  CHECK_GET_KIND(Line, Object::SV_Line);
  CHECK_GET_KIND(Scope, Object::SV_Scope);
  CHECK_GET_KIND(ScopeAggregate, Object::SV_ScopeAggregate);
  CHECK_GET_KIND(ScopeAlias, Object::SV_ScopeAlias);
  CHECK_GET_KIND(ScopeArray, Object::SV_ScopeArray);
  CHECK_GET_KIND(ScopeCompileUnit, Object::SV_ScopeCompileUnit);
  CHECK_GET_KIND(ScopeEnumeration, Object::SV_ScopeEnumeration);
  CHECK_GET_KIND(ScopeFunction, Object::SV_ScopeFunction);
  CHECK_GET_KIND(ScopeFunctionInlined, Object::SV_ScopeFunctionInlined);
  CHECK_GET_KIND(ScopeNamespace, Object::SV_ScopeNamespace);
  CHECK_GET_KIND(ScopeRoot, Object::SV_ScopeRoot);
  CHECK_GET_KIND(ScopeTemplatePack, Object::SV_ScopeTemplatePack);
  CHECK_GET_KIND(Symbol, Object::SV_Symbol);
  CHECK_GET_KIND(Type, Object::SV_Type);
  CHECK_GET_KIND(TypeDefinition, Object::SV_TypeDefinition);
  CHECK_GET_KIND(TypeEnumerator, Object::SV_TypeEnumerator);
  CHECK_GET_KIND(TypeImport, Object::SV_TypeImport);
  CHECK_GET_KIND(TypeTemplateParam, Object::SV_TypeTemplateParam);
  CHECK_GET_KIND(TypeSubrange, Object::SV_TypeSubrange);
};

TEST(Object, classof) {
  // Create an instance of each object for testing.
  Line TestLine;
  Scope TestScope;
  ScopeAggregate TestScopeAggregate;
  ScopeAlias TestScopeAlias;
  ScopeArray TestScopeArray;
  ScopeCompileUnit TestScopeCompileUnit;
  ScopeEnumeration TestScopeEnumeration;
  ScopeFunction TestScopeFunction;
  ScopeFunctionInlined TestScopeFunctionInlined;
  ScopeNamespace TestScopeNamespace;
  ScopeRoot TestScopeRoot;
  ScopeTemplatePack TestScopeTemplatePack;
  Symbol TestSymbol;
  Type TestType;
  TypeDefinition TestTypeDefinition;
  TypeEnumerator TestTypeEnumerator;
  TypeImport TestTypeImport;
  TypeTemplateParam TestTypeParam;
  TypeSubrange TestTypeSubrange;

  // Create collections of objects for convenience.
  std::set<Object *> Scopes = {
      &TestScope,         &TestScopeAggregate,       &TestScopeAlias,
      &TestScopeArray,    &TestScopeCompileUnit,     &TestScopeEnumeration,
      &TestScopeFunction, &TestScopeFunctionInlined, &TestScopeNamespace,
      &TestScopeRoot,     &TestScopeTemplatePack};

  std::set<Object *> Functions = {&TestScopeFunction,
                                  &TestScopeFunctionInlined};

  std::set<Object *> Types = {&TestType,           &TestTypeDefinition,
                              &TestTypeEnumerator, &TestTypeImport,
                              &TestTypeParam,      &TestTypeSubrange};

  std::set<Object *> AllObjects = {&TestLine, &TestSymbol};
  AllObjects.insert(Scopes.begin(), Scopes.end());
  AllObjects.insert(Types.begin(), Types.end());

  // For each object check that TY::classof(Obj) is only true if Obj is an
  // instance of TY.
  for (auto Obj : AllObjects) {
    // Object::classof() and Element::classof() should always be true.
    EXPECT_TRUE(Object::classof(Obj));
    EXPECT_TRUE(Element::classof(Obj));

    // Line.
    EXPECT_EQ(Line::classof(Obj), Obj == &TestLine);

    // Scope.
    EXPECT_EQ(Scope::classof(Obj), Scopes.count(Obj) == 1);
    EXPECT_EQ(ScopeAggregate::classof(Obj), Obj == &TestScopeAggregate);
    EXPECT_EQ(ScopeAlias::classof(Obj), Obj == &TestScopeAlias);
    EXPECT_EQ(ScopeArray::classof(Obj), Obj == &TestScopeArray);
    EXPECT_EQ(ScopeCompileUnit::classof(Obj), Obj == &TestScopeCompileUnit);
    EXPECT_EQ(ScopeEnumeration::classof(Obj), Obj == &TestScopeEnumeration);
    EXPECT_EQ(ScopeFunction::classof(Obj), Functions.count(Obj) == 1);
    EXPECT_EQ(ScopeFunctionInlined::classof(Obj),
              Obj == &TestScopeFunctionInlined);
    EXPECT_EQ(ScopeNamespace::classof(Obj), Obj == &TestScopeNamespace);
    EXPECT_EQ(ScopeRoot::classof(Obj), Obj == &TestScopeRoot);
    EXPECT_EQ(ScopeTemplatePack::classof(Obj), Obj == &TestScopeTemplatePack);

    // Symbol.
    EXPECT_EQ(Symbol::classof(Obj), Obj == &TestSymbol);

    // Type.
    EXPECT_EQ(Type::classof(Obj), Types.count(Obj) == 1);
    EXPECT_EQ(TypeDefinition::classof(Obj), Obj == &TestTypeDefinition);
    EXPECT_EQ(TypeEnumerator::classof(Obj), Obj == &TestTypeEnumerator);
    EXPECT_EQ(TypeImport::classof(Obj), Obj == &TestTypeImport);
    EXPECT_EQ(TypeTemplateParam::classof(Obj), Obj == &TestTypeParam);
    EXPECT_EQ(TypeSubrange::classof(Obj), Obj == &TestTypeSubrange);
  }
};

TEST(Object, isa) {
  Line TestLine;
  ScopeFunction TestFunction;
  ScopeFunctionInlined TestFunctionInlined;

  EXPECT_TRUE(isa<Object>(TestLine));
  EXPECT_TRUE(isa<Element>(TestLine));
  EXPECT_TRUE(isa<Line>(TestLine));
  EXPECT_FALSE(isa<Scope>(TestLine));
  EXPECT_FALSE(isa<ScopeFunction>(TestLine));
  EXPECT_FALSE(isa<ScopeFunctionInlined>(TestLine));

  EXPECT_TRUE(isa<Object>(TestFunction));
  EXPECT_TRUE(isa<Element>(TestFunction));
  EXPECT_FALSE(isa<Line>(TestFunction));
  EXPECT_TRUE(isa<Scope>(TestFunction));
  EXPECT_TRUE(isa<ScopeFunction>(TestFunction));
  EXPECT_FALSE(isa<ScopeFunctionInlined>(TestFunction));

  EXPECT_TRUE(isa<Object>(TestFunctionInlined));
  EXPECT_TRUE(isa<Element>(TestFunctionInlined));
  EXPECT_FALSE(isa<Line>(TestFunctionInlined));
  EXPECT_TRUE(isa<Scope>(TestFunctionInlined));
  EXPECT_TRUE(isa<ScopeFunction>(TestFunctionInlined));
  EXPECT_TRUE(isa<ScopeFunctionInlined>(TestFunctionInlined));
}

TEST(Object, object_cast) {
  std::unique_ptr<Object> TestLine = std::make_unique<Line>();
  std::unique_ptr<Object> TestFunction = std::make_unique<ScopeFunction>();

  EXPECT_EQ(cast<Object>(TestLine.get()), TestLine.get());
  Line *CastedToLine = cast<Line>(TestLine.get());
  EXPECT_EQ(CastedToLine, TestLine.get());
  Scope *CastedToScope = cast<Scope>(TestFunction.get());
  EXPECT_EQ(CastedToScope, TestFunction.get());

  Object &TestFunctionRef = *TestFunction;
  Scope &CastedToScopeRef = cast<Scope>(TestFunctionRef);
  EXPECT_EQ(&CastedToScopeRef, &TestFunctionRef);

  const Object &TestFunctionConstRef = *TestFunction;
  const Scope &CastedToScopeConstRef = cast<Scope>(TestFunctionConstRef);
  EXPECT_EQ(&CastedToScopeConstRef, &TestFunctionConstRef);

  // Check for asserts in debug builds.
#ifndef NDEBUG
  EXPECT_DEATH(
      { cast<Line>(TestFunction.get()); },
      "Assertion failed.*Tried to cast Object instance to invalid type");
  EXPECT_DEATH(
      { cast<Scope>(TestLine.get()); },
      "Assertion failed.*Tried to cast Object instance to invalid type");
  EXPECT_DEATH(
      { cast<ScopeAlias>(TestFunction.get()); },
      "Assertion failed.*Tried to cast Object instance to invalid type");
#endif
}

TEST(Object, object_dyn_cast) {
  std::unique_ptr<Object> TestLine = std::make_unique<Line>();
  std::unique_ptr<Object> TestFunction = std::make_unique<ScopeFunction>();

  EXPECT_EQ(dyn_cast<Object>(TestLine.get()), TestLine.get());
  EXPECT_EQ(dyn_cast<Line>(TestLine.get()), TestLine.get());
  EXPECT_EQ(dyn_cast<Scope>(TestLine.get()), nullptr);

  EXPECT_EQ(dyn_cast<Object>(TestFunction.get()), TestFunction.get());
  EXPECT_EQ(dyn_cast<Line>(TestFunction.get()), nullptr);
  Scope *CastedToScope = dyn_cast<Scope>(TestFunction.get());
  EXPECT_EQ(CastedToScope, TestFunction.get());
  EXPECT_EQ(dyn_cast<ScopeAlias>(TestFunction.get()), nullptr);
}
