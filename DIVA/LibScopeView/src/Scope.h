//===-- LibScopeView/Scope.h ------------------------------------*- C++ -*-===//
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
/// Definitions of the Scope class and its subclasses.
///
//===----------------------------------------------------------------------===//

#ifndef SCOPEVIEWSCOPE_H
#define SCOPEVIEWSCOPE_H

#include "Object.h"
#include "Sort.h"

#include <vector>

namespace LibScopeView {

class Line;
class Symbol;

// TODO: Make Scope pure virtual.

/// \brief Class to represent a DWARF Scope object.
class Scope : public Element {
public:
  Scope() : Scope(SV_Scope) {}
  ~Scope() override;

  /// \brief Return true if Obj is an instance of Scope.
  static bool classof(const Object *Obj) {
    return Obj->getKind() >= SV_Scope && Obj->getKind() <= SV_ScopeRoot;
  }

protected:
  Scope(ObjectKind K);

private:
  // Flags specifying various properties of the Scope.
  enum ScopeAttributes {
    IsBlock,
    IsCatchBlock,
    IsLexicalBlock,
    IsTryBlock,
    IsEntryPoint,
    IsSubprogram,
    IsSubroutineType,
    IsLabel,
    IsTemplate,
    IsClassType,
    IsStructureType,
    IsUnionType,
    HasDiscriminator,
    IsCombinedScope,
    ScopeAttributesSize
  };
  std::bitset<ScopeAttributesSize> ScopeAttributesFlags;

public:
  // Flags associated with the scope.
  bool getIsBlock() const { return ScopeAttributesFlags[IsBlock]; }
  void setIsBlock() { ScopeAttributesFlags.set(IsBlock); }

  bool getIsCatchBlock() const { return ScopeAttributesFlags[IsCatchBlock]; }
  void setIsCatchBlock() {
    ScopeAttributesFlags.set(IsCatchBlock);
    setIsBlock();
  }

  bool getIsLexicalBlock() const {
    return ScopeAttributesFlags[IsLexicalBlock];
  }
  void setIsLexicalBlock() {
    ScopeAttributesFlags.set(IsLexicalBlock);
    setIsBlock();
  }

  bool getIsTryBlock() const { return ScopeAttributesFlags[IsTryBlock]; }
  void setIsTryBlock() {
    ScopeAttributesFlags.set(IsTryBlock);
    setIsBlock();
  }

  bool getIsEntryPoint() const { return ScopeAttributesFlags[IsEntryPoint]; }
  void setIsEntryPoint() { ScopeAttributesFlags.set(IsEntryPoint); }

  bool getIsSubprogram() const { return ScopeAttributesFlags[IsSubprogram]; }
  void setIsSubprogram() { ScopeAttributesFlags.set(IsSubprogram); }

  bool getIsSubroutineType() const {
    return ScopeAttributesFlags[IsSubroutineType];
  }
  void setIsSubroutineType() { ScopeAttributesFlags.set(IsSubroutineType); }

  bool getIsLabel() const { return ScopeAttributesFlags[IsLabel]; }
  void setIsLabel() { ScopeAttributesFlags.set(IsLabel); }

  bool getIsTemplate() const { return ScopeAttributesFlags[IsTemplate]; }
  void setIsTemplate() { ScopeAttributesFlags.set(IsTemplate); }

  bool getIsClassType() const { return ScopeAttributesFlags[IsClassType]; }
  void setIsClassType() {
    ScopeAttributesFlags.set(IsClassType);
  }

  bool getIsStructType() const { return ScopeAttributesFlags[IsStructureType]; }
  void setIsStructType() {
    ScopeAttributesFlags.set(IsStructureType);
  }

  bool getIsUnionType() const { return ScopeAttributesFlags[IsUnionType]; }
  void setIsUnionType() {
    ScopeAttributesFlags.set(IsUnionType);
  }

  // Has any discriminator (DW_AT_GNU_discriminator).
  bool getHasDiscriminator() const {
    return ScopeAttributesFlags[HasDiscriminator];
  }
  void setHasDiscriminator() { ScopeAttributesFlags.set(HasDiscriminator); }

  bool getIsCombinedScope() const {
    return ScopeAttributesFlags[IsCombinedScope];
  }
  void setIsCombinedScope() { ScopeAttributesFlags.set(IsCombinedScope); }

  void addChild(Object *Obj);

  const std::vector<Object *> &getChildren() const { return Children; }
  std::vector<Object *> &getChildren() { return Children; }

  const std::vector<Line *> &getLines() const { return TheLines; }
  std::vector<Line *> &getLines() { return TheLines; }

  void sortScopes(const SortingKey &SortKey);

  // bring parent method getQualifiedName into scope.
  using Element::getQualifiedName;
  /// \brief Return the chain of parents as a string.
  void getQualifiedName(std::string &QualifiedName) const;

private:
  void sortScopes(SortFunction SortFunc);

  // All the line information for this scope.
  std::vector<Line *> TheLines;

  // Vector of objects (types, scopes, symbols).
  std::vector<Object *> Children;

public:
  /// \brief Returns a text representation of this DIVA Object.
  std::string getAsText(const PrintSettings &Settings) const override;
  /// \brief Returns a YAML representation of this DIVA Object.
  std::string getAsYAML() const override;
};

/// \brief Class to represent a DWARF Union/Structure/Class object.
class ScopeAggregate : public Scope {
public:
  ScopeAggregate();
  ~ScopeAggregate() override = default;

  /// \brief Return true if Obj is an instance of ScopeAggregate.
  static bool classof(const Object *Obj) {
    return Obj->getKind() == SV_ScopeAggregate;
  }

  /// \brief Returns a text representation of this DIVA Object.
  std::string getAsText(const PrintSettings &Settings) const override;
  /// \brief Returns a YAML representation of this DIVA Object.
  std::string getAsYAML() const override;
};

/// \brief Class to represent a DWARF Template alias object.
class ScopeAlias : public Scope {
public:
  ScopeAlias() : Scope(SV_ScopeAlias) { setIsTemplate(); }

  /// \brief Return true if Obj is an instance of ScopeAlias.
  static bool classof(const Object *Obj) {
    return Obj->getKind() == SV_ScopeAlias;
  }

  /// \brief Returns a text representation of this DIVA Object.
  std::string getAsText(const PrintSettings &Settings) const override;
  /// \brief Returns a YAML representation of this DIVA Object.
  std::string getAsYAML() const override;
};

/// \brief Class to represent a DWARF array object (DW_TAG_array_type).
class ScopeArray : public Scope {
public:
  ScopeArray() : Scope(SV_ScopeArray) {}

  /// \brief Return true if Obj is an instance of ScopeArray.
  static bool classof(const Object *Obj) {
    return Obj->getKind() == SV_ScopeArray;
  }

  bool getIsPrintedAsObject() const override { return false; }
  /// \brief Returns a text representation of this DIVA Object.
  std::string getAsText(const PrintSettings &Settings) const override;
};

/// \brief Class to represent a DWARF Compilation Unit (CU) object.
class ScopeCompileUnit : public Scope {
public:
  ScopeCompileUnit() : Scope(SV_ScopeCompileUnit) {}

  /// \brief Return true if Obj is an instance of ScopeCompileUnit.
  static bool classof(const Object *Obj) {
    return Obj->getKind() == SV_ScopeCompileUnit;
  }

  void setName(const std::string &Name) override;

  /// \brief Returns a text representation of this DIVA Object.
  std::string getAsText(const PrintSettings &Settings) const override;
  /// \brief Returns a YAML representation of this DIVA Object.
  std::string getAsYAML() const override;
};

/// \brief Class to represent a DWARF enumerator object.
///
/// (DW_TAG_enumeration_type).
class ScopeEnumeration : public Scope {
public:
  ScopeEnumeration() : Scope(SV_ScopeEnumeration), IsClass(false) {}

  /// \brief Return true if Obj is an instance of ScopeEnumeration.
  static bool classof(const Object *Obj) {
    return Obj->getKind() == SV_ScopeEnumeration;
  }

  /// \brief Returns a text representation of this DIVA Object.
  std::string getAsText(const PrintSettings &Settings) const override;
  /// \brief Returns a YAML representation of this DIVA Object.
  std::string getAsYAML() const override;

  void setIsClass() { IsClass = true; }
  bool getIsClass() const { return IsClass; }

private:
  bool IsClass;
};

/// \brief Class to represent a DWARF Function object.
class ScopeFunction : public Scope {
public:
  ScopeFunction() : ScopeFunction(SV_ScopeFunction) {}

  /// \brief Return true if Obj is an instance of ScopeFunction.
  static bool classof(const Object *Obj) {
    return Obj->getKind() >= SV_ScopeFunction &&
           Obj->getKind() <= SV_ScopeFunctionInlined;
  }

protected:
  ScopeFunction(ObjectKind K);

private:
  // Whether this function is static.
  bool IsStatic;
  // Whether the function was declared as inline.
  bool DeclaredInline;
  // If this is a declaration (not a definition).
  bool IsDeclaration;

  // The declaration of this function.
  const ScopeFunction *Declaration;

public:
  bool getIsStatic() const { return IsStatic; }
  void setIsStatic() { IsStatic = true; }

  bool getIsDeclaredInline() const { return DeclaredInline; }
  void setIsDeclaredInline() { DeclaredInline = true; }

  bool getIsDeclaration() const { return IsDeclaration; }
  void setIsDeclaration() { IsDeclaration = true; }

  const ScopeFunction *getDeclaration() const { return Declaration; }
  void setDeclaration(const ScopeFunction *Decl) { Declaration = Decl; }

  /// \brief Returns a text representation of this DIVA Object.
  std::string getAsText(const PrintSettings &Settings) const override;
  /// \brief Returns a YAML representation of this DIVA Object.
  std::string getAsYAML() const override;
};

/// \brief Class to represent a DWARF inlined function object.
class ScopeFunctionInlined : public ScopeFunction {
public:
  ScopeFunctionInlined() : ScopeFunction(SV_ScopeFunctionInlined) {}
  ~ScopeFunctionInlined() override;

  /// \brief Return true if Obj is an instance of ScopeFunctionInlined.
  static bool classof(const Object *Obj) {
    return Obj->getKind() == SV_ScopeFunctionInlined;
  }
};

/// \brief Class to represent a DWARF Namespace object.
class ScopeNamespace : public Scope {
public:
  ScopeNamespace() : Scope(SV_ScopeNamespace) {}

  /// \brief Return true if Obj is an instance of ScopeNamespace.
  static bool classof(const Object *Obj) {
    return Obj->getKind() == SV_ScopeNamespace;
  }

  /// \brief Returns a text representation of this DIVA Object.
  std::string getAsText(const PrintSettings &Settings) const override;
  /// \brief Returns a YAML representation of this DIVA Object.
  std::string getAsYAML() const override;
};

/// \brief Class to represent a DWARF template pack.
///
/// (DW_TAG_GNU_template_parameter_pack).
class ScopeTemplatePack : public Scope {
public:
  ScopeTemplatePack() : Scope(SV_ScopeTemplatePack) {}

  /// \brief Return true if Obj is an instance of ScopeTemplatePack.
  static bool classof(const Object *Obj) {
    return Obj->getKind() == SV_ScopeTemplatePack;
  }

  /// \brief Returns a text representation of this DIVA Object.
  std::string getAsText(const PrintSettings &Settings) const override;
  /// \brief Returns a YAML representation of this DIVA Object.
  std::string getAsYAML() const override;
};

/// \brief Class to represent an object file (single or multiple CUs).
class ScopeRoot : public Scope {
public:
  ScopeRoot() : Scope(SV_ScopeRoot) {}

  /// \brief Return true if Obj is an instance of ScopeRoot.
  static bool classof(const Object *Obj) {
    return Obj->getKind() == SV_ScopeRoot;
  }

  void setName(const std::string &Name) override;

  bool getIsPrintedAsObject() const override { return false; }
  /// \brief Returns a text representation of this DIVA Object.
  std::string getAsText(const PrintSettings &Settings) const override;
};

} // namespace LibScopeView

#endif // SCOPEVIEWSCOPE_H
