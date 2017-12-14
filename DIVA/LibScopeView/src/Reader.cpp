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
    if (auto Ty = dynamic_cast<Type *>(Obj)) {
      resolveTypeName(Ty);
      return;
    }

    if (auto ObjScope = dynamic_cast<Scope *>(Obj)) {
      // Resolve function pointer names.
      if (ObjScope->getIsSubroutineType()) {
        resolveFunctionPointerName(dynamic_cast<ScopeFunction *>(Obj));
        return;
      }

      // Resolve array names.
      if (ObjScope->getIsArrayType()) {
        resolveArrayName(dynamic_cast<ScopeArray *>(Obj));
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
      if (auto *Sym = dynamic_cast<const Symbol *>(Child)) {
        if (Sym->getIsParameter()) {
          // Make sure the parameters are resolved.
          if (Sym->getType())
            resolve(Sym->getType());
          ResolvedName << (First ? "" : ",") << Sym->getTypeName();
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

    bool Ret = Ty->setFullName(Settings);
    // setFullName keys off DWARF tags and will return false if it doesn't
    // recognise the tag.
    assert(Ret && "Unrecognised DWARF Tag in Object::setFullName");
    (void)Ret;
  }

  void resolveArrayName(ScopeArray *Array) {
    // Make sure Array's type is resolved first.
    Object *ArrayType = Array->getType();
    if (ArrayType)
      resolve(ArrayType);

    std::string ResolvedName(
        ArrayType && ArrayType->getName() ? ArrayType->getName() : "?");
    ResolvedName += " ";

    for (const Object *Child : Array->getChildren())
      if (auto *Ty = dynamic_cast<const Type *>(Child))
        if (Ty->getIsSubrangeType())
          ResolvedName += Ty->getName();
    Array->setName(ResolvedName.c_str());
  }

  const PrintSettings &Settings;
  std::unordered_set<Object *> AlreadyResolved;
};

// Visitor that sets the attributes of objects to those they reference.
class ReferenceAttributeResolver : public ScopeVisitor {
private:
  void visitImpl(Object *Obj) override {
    resolveReference(Obj);
    visitChildren(Obj);
  }

  // Get an Object's referenced Object, handling any type specifics.
  static Object *getObjectReference(Object *Obj) {
    if (auto Scp = dynamic_cast<Scope *>(Obj))
      return Scp->getReference();
    if (auto Sym = dynamic_cast<Symbol *>(Obj))
      return Sym->getReference();
    return nullptr;
  }

  static void resolveReference(Object *Obj) {
    auto *Reference = getObjectReference(Obj);
    if (!Reference)
      return;

    // Resolve the reference first.
    resolveReference(Reference);

    // Set common attribute values.
    Obj->setNameIndex(Reference->getNameIndex());
    Obj->setLineNumber(Reference->getLineNumber());
    Obj->setFileNameIndex(Reference->getFileNameIndex());
    if (Reference->getInvalidFileName())
      Obj->setInvalidFileName();

    // Set type.
    if (Reference->getType()) {
      dynamic_cast<Element *>(Obj)->setType(Reference->getType());
      Obj->setHasType();
    }

    // Cover the static function case that initScopeFromAttrs can't reach.
    auto ObjFunc = dynamic_cast<ScopeFunction *>(Obj);
    auto RefFunc = dynamic_cast<ScopeFunction *>(Reference);
    if (ObjFunc && RefFunc && RefFunc->getIsStatic())
      ObjFunc->setIsStatic();

    // Set qualified name from reference.
    if (Obj->getIsSymbol() && Reference->getIsSymbol())
      Obj->resolveQualifiedName(Reference->getParent());
  }
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
  std::unique_ptr<ScopeRoot> Root = createScopes(FileName);
  if (Root)
    postCreationActions(Root.get(), Settings);
  return Root;
}

void Reader::postCreationActions(ScopeRoot *Root,
                                 const PrintSettings &Settings) {
  assert(Root);

  NameResolver(Settings).visit(Root);
  ReferenceAttributeResolver().visit(Root);
  GlobalResolver().visit(Root);

  Root->sortScopes(Settings.SortKey);
}
