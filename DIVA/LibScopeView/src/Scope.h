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

/// \brief Class to represent a DWARF Scope object.
class Scope : public Element {

public:
  Scope();
  Scope(LevelType Lvl);
  virtual ~Scope() override;

  Scope &operator=(const Scope &) = delete;
  Scope(const Scope &) = delete;

private:
  // Scope Kind.
  static const char *KindAggregate;
  static const char *KindArray;
  static const char *KindBlock;
  static const char *KindCatchBlock;
  static const char *KindClass;
  static const char *KindCompileUnit;
  static const char *KindEntryPoint;
  static const char *KindEnumeration;
  static const char *KindFile;
  static const char *KindFunction;
  static const char *KindFunctionType;
  static const char *KindInlinedFunction;
  static const char *KindLabel;
  static const char *KindLexicalBlock;
  static const char *KindNamespace;
  static const char *KindStruct;
  static const char *KindTemplate;
  static const char *KindTemplateAlias;
  static const char *KindTemplatePack;
  static const char *KindTryBlock;
  static const char *KindUndefined;
  static const char *KindUnion;

public:
  // Type of function to be called at each node in the Scope Tree.
  typedef void (Scope::*ScopeSetFunction)();
  typedef bool (Scope::*ScopeGetFunction)() const;

public:
  // Flags specifying various properties of the Scope.
  enum ScopeAttributes {
    IsAggregate,
    IsArrayType,
    IsBlock,
    IsCatchBlock,
    IsLexicalBlock,
    IsTryBlock,
    IsCompileUnit,
    IsFunction,
    IsInlinedSubroutine,
    IsEntryPoint,
    IsSubprogram,
    IsSubroutineType,
    IsLabel,
    IsNamespace,
    IsRoot,
    IsTemplate,
    IsTemplateAlias,
    IsClassType,
    IsStructureType,
    IsUnionType,
    IsEnumerationType,
    IsTemplatePack,
    HasDiscriminator,
    CanHaveLines,
    IsCombinedScope,
    ScopeAttributesSize
  };
  std::bitset<ScopeAttributesSize> ScopeAttributesFlags;

public:
  /// \brief Get the object kind as a string.
  const char *getKindAsString() const override;

public:
  // Flags associated with the scope.
  bool getIsAggregate() const { return ScopeAttributesFlags[IsAggregate]; }
  void setIsAggregate() { ScopeAttributesFlags.set(IsAggregate); }

  bool getIsArrayType() const { return ScopeAttributesFlags[IsArrayType]; }
  void setIsArrayType() { ScopeAttributesFlags.set(IsArrayType); }

  bool getIsBlock() const { return ScopeAttributesFlags[IsBlock]; }
  void setIsBlock() {
    ScopeAttributesFlags.set(IsBlock);
    setCanHaveLines();
  }

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

  bool getIsCompileUnit() const override {
    return ScopeAttributesFlags[IsCompileUnit];
  }
  void setIsCompileUnit() {
    ScopeAttributesFlags.set(IsCompileUnit);
    setCanHaveLines();
  }

  bool getIsInlinedSubroutine() const {
    return ScopeAttributesFlags[IsInlinedSubroutine];
  }
  void setIsInlinedSubroutine() {
    ScopeAttributesFlags.set(IsInlinedSubroutine);
    setIsFunction();
    setIsInlined();
  }

  bool getIsEntryPoint() const { return ScopeAttributesFlags[IsEntryPoint]; }
  void setIsEntryPoint() {
    ScopeAttributesFlags.set(IsEntryPoint);
    setIsFunction();
  }

  bool getIsSubprogram() const { return ScopeAttributesFlags[IsSubprogram]; }
  void setIsSubprogram() {
    ScopeAttributesFlags.set(IsSubprogram);
    setIsFunction();
  }

  bool getIsSubroutineType() const {
    return ScopeAttributesFlags[IsSubroutineType];
  }
  void setIsSubroutineType() {
    ScopeAttributesFlags.set(IsSubroutineType);
    setIsFunction();
  }

  bool getIsLabel() const { return ScopeAttributesFlags[IsLabel]; }
  void setIsLabel() {
    ScopeAttributesFlags.set(IsLabel);
    setIsFunction();
  }

  bool getIsFunction() const { return ScopeAttributesFlags[IsFunction]; }
  void setIsFunction() {
    ScopeAttributesFlags.set(IsFunction);
    setCanHaveLines();
  }

  bool getIsNamespace() const { return ScopeAttributesFlags[IsNamespace]; }
  void setIsNamespace() { ScopeAttributesFlags.set(IsNamespace); }

  bool getIsRoot() const { return ScopeAttributesFlags[IsRoot]; }
  void setIsRoot() { ScopeAttributesFlags.set(IsRoot); }

  bool getIsTemplate() const { return ScopeAttributesFlags[IsTemplate]; }
  void setIsTemplate() { ScopeAttributesFlags.set(IsTemplate); }

  bool getIsTemplateAlias() const {
    return ScopeAttributesFlags[IsTemplateAlias];
  }
  void setIsTemplateAlias() { ScopeAttributesFlags.set(IsTemplateAlias); }

  bool getIsClassType() const { return ScopeAttributesFlags[IsClassType]; }
  void setIsClassType() {
    ScopeAttributesFlags.set(IsClassType);
    setIsAggregate();
  }

  bool getIsStructType() const { return ScopeAttributesFlags[IsStructureType]; }
  void setIsStructType() {
    ScopeAttributesFlags.set(IsStructureType);
    setIsAggregate();
  }

  bool getIsUnionType() const { return ScopeAttributesFlags[IsUnionType]; }
  void setIsUnionType() {
    ScopeAttributesFlags.set(IsUnionType);
    setIsAggregate();
  }

  bool getIsEnumerationType() const {
    return ScopeAttributesFlags[IsEnumerationType];
  }
  void setIsEnumerationType() { ScopeAttributesFlags.set(IsEnumerationType); }

  bool getIsTemplatePack() const {
    return ScopeAttributesFlags[IsTemplatePack];
  }
  void setIsTemplatePack() { ScopeAttributesFlags.set(IsTemplatePack); }

  // Has any reference (DW_AT_GNU_discriminator).
  bool getHasDiscriminator() const {
    return ScopeAttributesFlags[HasDiscriminator];
  }
  void setHasDiscriminator() { ScopeAttributesFlags.set(HasDiscriminator); }

  bool getCanHaveLines() const { return ScopeAttributesFlags[CanHaveLines]; }
  void setCanHaveLines() { ScopeAttributesFlags.set(CanHaveLines); }

  bool getIsCombinedScope() const {
    return ScopeAttributesFlags[IsCombinedScope];
  }
  void setIsCombinedScope() { ScopeAttributesFlags.set(IsCombinedScope); }

public:
  // Functions to be implemented by derived classes.

  /// \brief Get the Object's reference to another object.
  ///
  /// DW_AT_specification, DW_AT_abstract_origin, DW_AT_extension.
  virtual Scope *getReference() const { return nullptr; }
  virtual void setReference(Scope * /*Scp*/) {}

public:
  void addObject(Symbol *Sym);
  void addObject(Type *Ty);
  void addObject(Scope *Scp);
  void addObject(Line *Ln);

public:
  /// \brief Gets the child symbol at the specified index.
  Symbol *getSymbolAt(size_t Index) const { return TheSymbols.at(Index); }

  /// \brief Gets the child scope at the specified index.
  Scope *getScopeAt(size_t Index) const { return TheScopes.at(Index); }

public:
  const std::vector<Object *> &getChildren() const { return Children; }
  std::vector<Object *> &getChildren() { return Children; }

  const std::vector<Line *> &getLines() const { return TheLines; }
  std::vector<Line *> &getLines() { return TheLines; }

  const std::vector<Scope *> &getScopes() const { return TheScopes; }
  const std::vector<Symbol *> &getSymbols() const { return TheSymbols; }
  const std::vector<Type *> &getTypes() const { return TheTypes; }

  /// \brief Get the number of children.
  size_t getChildrenCount() const { return getChildren().size(); }

  /// \brief Get the number of lines.
  size_t getLineCount() const { return getLines().size(); }

  /// \brief Get the number of scopes.
  size_t getScopeCount() const { return getScopes().size(); }

  /// \brief Get the number of symbols.
  size_t getSymbolCount() const { return getSymbols().size(); }

  /// \brief Get the number of types.
  size_t getTypeCount() const { return getTypes().size(); }

public:
  const char *resolveName();

  void sortScopes(const SortingKey &SortKey);
  void sortCompileUnits(const SortingKey &SortKey);

  // bring parent method getQualifiedName into scope.
  using Element::getQualifiedName;
  /// \brief Return the chain of parents as a string.
  void getQualifiedName(std::string &QualifiedName) const;

protected:
  void sortScopes(SortFunction SortFunc);

protected:
  // All the types in this scope.
  std::vector<Type *> TheTypes;

  // All the symbols in this scope.
  std::vector<Symbol *> TheSymbols;

  // All the child scopes underneath this one.
  std::vector<Scope *> TheScopes;

  // All the line information for this scope.
  std::vector<Line *> TheLines;

  // Vector of objects (types, scopes, symbols, lines).
  std::vector<Object *> Children;

public:
  /// \brief Returns a text representation of this DIVA Object.
  std::string getAsText(const PrintSettings &Settings) const override;
  /// \brief Returns a YAML representation of this DIVA Object.
  std::string getAsYAML() const override;

private:
  static uint32_t ScopesAllocated;

public:
  static uint32_t getInstanceCount() { return ScopesAllocated; }
  uint32_t getTag() const override;
  void setTag() override;
};

/// \brief Class to represent a DWARF Union/Structure/Class object.
class ScopeAggregate : public Scope {
public:
  ScopeAggregate();
  ScopeAggregate(LevelType Lvl);
  virtual ~ScopeAggregate() override;

  ScopeAggregate &operator=(const ScopeAggregate &) = delete;
  ScopeAggregate(const ScopeAggregate &) = delete;

private:
  // DW_AT_specification, DW_AT_abstract_origin.
  Scope *Reference;

public:
  Scope *getReference() const override { return Reference; }
  void setReference(Scope *Scp) override {
    Reference = Scp;
    setHasReference();
  }

public:
  /// \brief Returns a text representation of this DIVA Object.
  std::string getAsText(const PrintSettings &Settings) const override;
  /// \brief Returns a YAML representation of this DIVA Object.
  std::string getAsYAML() const override;
};

/// \brief Class to represent a DWARF Template alias object.
class ScopeAlias : public Scope {
public:
  ScopeAlias();
  ScopeAlias(LevelType Lvl);
  virtual ~ScopeAlias() override;

  ScopeAlias &operator=(const ScopeAlias &) = delete;
  ScopeAlias(const ScopeAlias &) = delete;

public:
  /// \brief Returns a text representation of this DIVA Object.
  std::string getAsText(const PrintSettings &Settings) const override;
  /// \brief Returns a YAML representation of this DIVA Object.
  std::string getAsYAML() const override;
};

/// \brief Class to represent a DWARF array object (DW_TAG_array_type).
class ScopeArray : public Scope {
public:
  ScopeArray();
  ScopeArray(LevelType Lvl);
  virtual ~ScopeArray() override;

  ScopeArray &operator=(const ScopeArray &) = delete;
  ScopeArray(const ScopeArray &) = delete;

public:
  bool getIsPrintedAsObject() const override { return false; }
  /// \brief Returns a text representation of this DIVA Object.
  std::string getAsText(const PrintSettings &Settings) const override;
};

/// \brief Class to represent a DWARF Compilation Unit (CU) object.
class ScopeCompileUnit : public Scope {
public:
  ScopeCompileUnit();
  ScopeCompileUnit(LevelType Lvl);
  virtual ~ScopeCompileUnit() override;

  ScopeCompileUnit &operator=(const ScopeCompileUnit &) = delete;
  ScopeCompileUnit(const ScopeCompileUnit &) = delete;

public:
  void setName(const char *Name) override;

public:
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
  ScopeEnumeration();
  ScopeEnumeration(LevelType Lvl);
  virtual ~ScopeEnumeration() override;

  ScopeEnumeration &operator=(const ScopeEnumeration &) = delete;
  ScopeEnumeration(const ScopeEnumeration &) = delete;

public:
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
  ScopeFunction();
  ScopeFunction(LevelType Lvl);
  virtual ~ScopeFunction() override;

  ScopeFunction &operator=(const ScopeFunction &) = delete;
  ScopeFunction(const ScopeFunction &) = delete;

private:
  // DW_AT_specification, DW_AT_abstract_origin.
  Scope *Reference;
  // Whether this function is static.
  bool IsStatic;
  // Whether the function was declared as inline.
  bool DeclaredInline;
  // If this is a declaration (not a definition).
  bool IsDeclaration;

public:
  Scope *getReference() const override { return Reference; }
  void setReference(Scope *Scp) override {
    Reference = Scp;
    setHasReference();
  }

  bool getIsStatic() const { return IsStatic; }
  void setIsStatic() { IsStatic = true; }

  bool getIsDeclaredInline() const { return DeclaredInline; }
  void setIsDeclaredInline() { DeclaredInline = true; }

  bool getIsDeclaration() const { return IsDeclaration; }
  void setIsDeclaration() { IsDeclaration = true; }

public:
  /// \brief Returns a text representation of this DIVA Object.
  std::string getAsText(const PrintSettings &Settings) const override;
  /// \brief Returns a YAML representation of this DIVA Object.
  std::string getAsYAML() const override;
};

/// \brief Class to represent a DWARF inlined function object.
class ScopeFunctionInlined : public ScopeFunction {
public:
  ScopeFunctionInlined();
  ScopeFunctionInlined(LevelType Lvl);
  virtual ~ScopeFunctionInlined() override;

  ScopeFunctionInlined &operator=(const ScopeFunctionInlined &) = delete;
  ScopeFunctionInlined(const ScopeFunctionInlined &) = delete;

private:
  // Reference to DW_AT_GNU_discriminator attribute.
  Dwarf_Half Discriminator;

  // File and Line Coordinates associated with this object.
  uint64_t CallLineNumber; // DWARF line number.

public:
  /// \brief Access the DW_AT_GNU_discriminator attribute.
  Dwarf_Half getDiscriminator() const override { return Discriminator; }
  void setDiscriminator(Dwarf_Half Discrim) override {
    Discriminator = Discrim;
    setHasDiscriminator();
  }

public:
  /// \brief Call line for the object (Inlined Functions).
  uint64_t getCallLineNumber() const override { return CallLineNumber; }
  void setCallLineNumber(uint64_t LnNumber) override {
    CallLineNumber = LnNumber;
  }
};

/// \brief Class to represent a DWARF Namespace object.
class ScopeNamespace : public Scope {
public:
  ScopeNamespace();
  ScopeNamespace(LevelType Lvl);
  virtual ~ScopeNamespace() override;

  ScopeNamespace &operator=(const ScopeNamespace &) = delete;
  ScopeNamespace(const ScopeNamespace &) = delete;

private:
  // Reference to DW_AT_extension attribute.
  Scope *Reference;

public:
  /// \brief Access to the DW_AT_extension reference.
  Scope *getReference() const override { return Reference; }
  void setReference(Scope *Scp) override {
    Reference = Scp;
    setHasReference();
  }

public:
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
  ScopeTemplatePack();
  ScopeTemplatePack(LevelType Lvl);
  virtual ~ScopeTemplatePack() override;

  ScopeTemplatePack &operator=(const ScopeTemplatePack &) = delete;
  ScopeTemplatePack(const ScopeTemplatePack &) = delete;

public:
  /// \brief Returns a text representation of this DIVA Object.
  std::string getAsText(const PrintSettings &Settings) const override;
  /// \brief Returns a YAML representation of this DIVA Object.
  std::string getAsYAML() const override;
};

/// \brief Class to represent an object file (single or multiple CUs).
class ScopeRoot : public Scope {
public:
  ScopeRoot();
  ScopeRoot(LevelType Lvl);
  virtual ~ScopeRoot() override;

  ScopeRoot &operator=(const ScopeRoot &) = delete;
  ScopeRoot(const ScopeRoot &) = delete;

public:
  void setName(const char *Name) override;

public:
  bool getIsPrintedAsObject() const override { return false; }
  /// \brief Returns a text representation of this DIVA Object.
  std::string getAsText(const PrintSettings &Settings) const override;
};

} // namespace LibScopeView

#endif // SCOPEVIEWSCOPE_H
