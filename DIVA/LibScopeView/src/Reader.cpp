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
#include "PrintContext.h"
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

namespace {
Reader *GlobalReader = nullptr;
}

Reader *LibScopeView::getReader() { return GlobalReader; }
void LibScopeView::setReader(Reader *Rdr) { GlobalReader = Rdr; }

bool Reader::executeActions() {
  destroyScopes();

  // Record current Reader, so it will be available to places where is hard
  // to access it.
  setReader(this);

  // Delegate the scope tree creation to the respective reader.
  if (!createScopes())
    return false;

  postCreationActions();
  return true;
}

// Print summary details for the Scopes Tree.
void Reader::printSummary() {
  if (!PrintedHeader) {
    getScopesRoot()->dump();
  }
  TheSummaryTable.getPrintedSummaryTable(std::cout);
}

void Reader::print() {
  // If doing any search (--filter), do not do any scope tree printing.
  if (!Settings.Filters.empty() || !Settings.FilterAnys.empty()) {
    printObjects();
  } else {
    printScopes();
  }
  std::cout << "\n";
}

void Reader::printObjects() {
  if (getPrintObjects() && ViewMatchedObjects.size()) {
    // Get the sorting callback function.
    SortFunction SortFunc = getSortFunction();
    if (SortFunc) {
      std::sort(ViewMatchedObjects.begin(), ViewMatchedObjects.end(), SortFunc);
    }

    getScopesRoot()->dump();
    PrintedHeader = true;

    for (Object *Matched : ViewMatchedObjects)
      Matched->dump();
  }

  if (Settings.ShowSummary)
    printSummary();
}

void Reader::printScopes() {
  bool DoPrint = getPrintObjects();
  if (DoPrint) {
    // Propagate any matching information into the scopes tree.
    propagatePatternMatch();

    Scope *Scp = getScopesRoot();
    bool DoSplit = Settings.SplitOutput;
    std::string SplitDir = Settings.OutputDirectory;
    if (DoSplit) {
      if (SplitDir.empty()) {
        // If no split location, use the scope root name.
        SplitDir = Scp->getName();
      }
      if (!GlobalPrintContext->createLocation(SplitDir)) {
        // If enable to create a print context location, reset the given
        // location option and swith to non-split mode.
        DoSplit = false;
      }
    }

    PrintedHeader = true;

    // We do a normal print, using the standard settings.
    bool Match = (!Settings.WithChildrenFilters.empty() ||
                  !Settings.WithChildrenFilterAnys.empty());
    Scp->print(DoSplit, Match, DoPrint);
  }

  if (Settings.ShowSummary)
    printSummary();
}

bool Reader::loadFile(const std::string &FileName) {
  InputFile = FileName;
  return executeActions();
}

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
        resolveFunctionPointerName(
            dynamic_cast<ScopeFunction *>(Obj));
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
    for (auto *Sym : Func->getSymbols()) {
      if (Sym->getIsParameter()) {
        // Make sure the parameters are resolved.
        if (Sym->getType())
          resolve(Sym->getType());
        ResolvedName << (First ? "" : ",") << Sym->getTypeName();
        First = false;
      }
    }

    ResolvedName << ")";
    Func->setName(ResolvedName.str().c_str());
  }

  void resolveTypeName(Type *Ty) {
    // Make sure Ty's type is resolved first.
    if (Ty->getType())
      resolve(Ty->getType());

    bool Ret = Ty->setFullName();
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

    for (const auto *Ty : Array->getTypes()) {
      if (Ty->getIsSubrangeType())
        ResolvedName += Ty->getName();
    }
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

// Visitor that does any modifications to the CU tree that need to be done after
// it has been created and the type names and references have been resolved.
class TreeResolver : public ScopeVisitor {
public:
  TreeResolver(Reader &Reader)
      : ReaderInstance(Reader) {}

private:
  void visitImpl(Object *Obj) override {
    if (Obj->getIsResolved())
      return;
    Obj->setIsResolved();

    // Resolve any filters.
    // TODO: Filters should be evaluated while printing.
    ReaderInstance.resolveFilterPatternMatch(Obj);
    if (auto Scp = dynamic_cast<Scope *>(Obj))
      ReaderInstance.resolveTreePatternMatch(Scp);

    // If the parent is global then mark this as global.
    if (Obj->getParent() && Obj->getParent()->getIsGlobalReference())
      Obj->setIsGlobalReference();

    visitChildren(Obj);
  }

  Reader &ReaderInstance;
};
} // namespace

void Reader::postCreationActions() {
  assert(Scopes);

  NameResolver(Settings).visit(Scopes);
  ReferenceAttributeResolver().visit(Scopes);
  TreeResolver(*this).visit(Scopes);

  Scopes->sortScopes();
}

void Reader::propagatePatternMatch() {
  // At this stage, we have finished creating the Scopes tree and we have
  // a list of objects that match the pattern specified in the command line
  // by the user. The pattern corresponds to a subtree; mark its parents and
  // children as having that pattern, before any printing is done.
  for (Scope *Matched : ViewMatchedScopes)
    Matched->traverse(&Scope::getHasPattern, &Scope::setHasPattern,
                      /*down=*/true);
}

void Reader::resolveTreePatternMatch(Scope *Scp) {
  if (Scp->isNamed() &&
      Settings.matchesWithChildrenFilterPattern(Scp->getName())) {
    ViewMatchedScopes.push_back(Scp);
  }
}

void Reader::resolveFilterPatternMatch(Object *Object) {
  if (Object->isNamed() && Settings.matchesFilterPattern(Object->getName())) {
    ViewMatchedObjects.push_back(Object);
  }
}

void Reader::resolveFilterPatternMatch(Line *Line) {
  if (Settings.matchesFilterPattern(
      trim(Line->getLineNumberAsString()).c_str())) {
    ViewMatchedObjects.push_back(Line);
  }
}
