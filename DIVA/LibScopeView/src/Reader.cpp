//===-- LibScopeView/Reader.cpp ---------------------------------*- C++ -*-===//
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
/// Implementation of methods common to all Readers.
///
//===----------------------------------------------------------------------===//

#include "Reader.h"
#include "Line.h"
#include "ScopeVisitor.h"
#include "Symbol.h"
#include "Type.h"
#include "Utilities.h"

#include <algorithm>
#include <assert.h>
#include <iostream>
#include <sstream>
#include <unordered_set>

using namespace LibScopeView;

// Visitors for post-creation actions.
namespace {

// Visitor that creates all the full type names once the CU tree has been
// created.
class NameResolver : public ScopeVisitor {
public:
  NameResolver(const PrintSettings &PrintingSettings)
      : Settings(PrintingSettings) {}

private:
  void visitImpl(Object *Obj) override {
    resolve(Obj);
    visitChildren(Obj);
  }

  void resolve(Object *Obj) {
    if (AlreadyResolved.count(Obj))
      return;
    AlreadyResolved.insert(Obj);

    // Resolve type names.
    if (auto Ty = dyn_cast<Type>(Obj)) {
      resolveTypeName(Ty);
      return;
    }

    if (auto ObjScope = dyn_cast<Scope>(Obj)) {
      // Resolve function pointer names.
      if (ObjScope->getIsSubroutineType()) {
        resolveFunctionPointerName(dyn_cast<ScopeFunction>(Obj));
        return;
      }

      // Resolve array names.
      if (isa<ScopeArray>(*ObjScope)) {
        resolveArrayName(dyn_cast<ScopeArray>(Obj));
        return;
      }
    }
  }

  void resolveFunctionPointerName(ScopeFunction *Func) {
    // Make sure the return type is resolved first.
    if (Func->getType())
      resolve(Func->getType());

    std::stringstream ResolvedName;
    ResolvedName << Func->getTypeAsString(Settings) << " (*)(";

    // Add the parameters.
    bool First = true;
    for (const Object *Child : Func->getChildren()) {
      if (auto *Sym = dyn_cast<const Symbol>(Child)) {
        if (Sym->getIsParameter()) {
          // Make sure the parameters are resolved.
          if (Sym->getType())
            resolve(Sym->getType());
          ResolvedName << (First ? "" : ",") << Sym->getTypeAsString(Settings);
          First = false;
        }
      }
    }

    ResolvedName << ")";
    Func->setName(ResolvedName.str().c_str());
  }

  void resolveTypeName(Type *Ty) {
    // Make sure Ty's type is resolved first.
    if (Ty->getType())
      resolve(Ty->getType());
    Ty->formulateTypeName(Settings);
  }

  void resolveArrayName(ScopeArray *Array) {
    // Make sure Array's type is resolved first.
    Object *ArrayType = Array->getType();
    if (ArrayType)
      resolve(ArrayType);

    std::string ResolvedName(ArrayType ? ArrayType->getName() : "?");
    ResolvedName += " ";

    for (const Object *Child : Array->getChildren())
      if (auto *Ty = dyn_cast<const Type>(Child))
        if (isa<TypeSubrange>(*Ty))
          ResolvedName += Ty->getName();
    Array->setName(ResolvedName.c_str());
  }

  const PrintSettings &Settings;
  std::unordered_set<Object *> AlreadyResolved;
};

// Visitor that sets the attributes of Objects to those of their specification.
class SpecificationAttributeResolver : public ScopeVisitor {
public:
  SpecificationAttributeResolver(
      std::unordered_map<Object *, Object *> &SpecMap)
      : ObjectSpecMap(SpecMap) {}

private:
  void visitImpl(Object *Obj) override {
    setFromSpec(Obj);
    visitChildren(Obj);
  }

  void setFromSpec(Object *Obj) {
    auto IT = ObjectSpecMap.find(Obj);
    if (IT == ObjectSpecMap.end())
      return;
    Object *Spec = IT->second;

    // Resolve the Spec first.
    setFromSpec(Spec);

    // Set common attribute values.
    Obj->setName(Spec->getNamePoolRef());
    Obj->setLineNumber(Spec->getLineNumber());
    Obj->setFilePath(Spec->getFilePathPoolRef());
    if (Spec->getInvalidFileName())
      Obj->setInvalidFileName();

    // Set type.
    if (Spec->getType())
      cast<Element>(Obj)->setType(Spec->getType());

    // Set static functions.
    auto ObjFunc = dyn_cast<ScopeFunction>(Obj);
    auto RefFunc = dyn_cast<ScopeFunction>(Spec);
    if (ObjFunc && RefFunc && RefFunc->getIsStatic())
      ObjFunc->setIsStatic();

    // Set qualified name from reference.
    if (isa<Symbol>(*Obj) && isa<Symbol>(*Spec))
      Obj->resolveQualifiedName(Spec->getParent());
  }

  std::unordered_map<Object *, Object *> &ObjectSpecMap;
};

// Visitor that sets all children of global objects as global.
class GlobalResolver : public ScopeVisitor {
private:
  void visitImpl(Object *Obj) override {
    // If the parent is global then mark this as global.
    if (Obj->getParent() && Obj->getParent()->getIsGlobalReference())
      Obj->setIsGlobalReference();
    visitChildren(Obj);
  }
};
} // namespace

Reader::~Reader() {}

std::unique_ptr<ScopeRoot> Reader::loadFile(const std::string &FileName,
                                            const PrintSettings &Settings) {
  ObjectSpecMap.clear();
  std::unique_ptr<ScopeRoot> Root = createScopes(FileName);
  if (Root)
    postCreationActions(Root.get(), Settings);
  return Root;
}

void Reader::addObjectSpecRelation(Object &Obj, Object &Spec) {
  // Function to Function.
  if (isa<ScopeFunction>(Obj) && isa<ScopeFunction>(Spec)) {
    ObjectSpecMap.emplace(&Obj, &Spec);
    cast<ScopeFunction>(Obj).setDeclaration(&cast<ScopeFunction>(Spec));
  }
  // Symbol to Symbol.
  else if (isa<Symbol>(Obj) && isa<Symbol>(Spec))
    ObjectSpecMap.emplace(&Obj, &Spec);
}

void Reader::postCreationActions(ScopeRoot *Root,
                                 const PrintSettings &Settings) {
  assert(Root);

  NameResolver(Settings).visit(Root);
  SpecificationAttributeResolver(ObjectSpecMap).visit(Root);
  GlobalResolver().visit(Root);

  Root->sortScopes(Settings.SortKey);
}
