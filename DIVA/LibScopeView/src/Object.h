//===-- LibScopeView/Object.h -----------------------------------*- C++ -*-===//
///
/// Copyright (c) Sony Interactive Entertainment Inc.
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
/// Definition of the Object and Element classes.
///
//===----------------------------------------------------------------------===//

#ifndef OBJECT_H
#define OBJECT_H

// Disable some clang warnings for libdwarf.h.
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#pragma clang diagnostic ignored "-Wundef"
#endif

#include "libdwarf.h"

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#include <bitset>
#include <cstdint>

namespace LibScopeView {

class Object;
class PrintSettings;
class Scope;
class Type;

void printAllocationInfo();

typedef uint16_t LevelType;

/// \brief Get/set attribute functions.
typedef void (Object::*ObjSetFunction)();
typedef bool (Object::*ObjGetFunction)() const;

/// \brief Enum to represent C++ access specifiers.
enum class AccessSpecifier { Unspecified, Private, Protected, Public };

/// \brief Class to represent the basic information for a DIVA object.
class Object {
public:
  Object();
  Object(LevelType Lvl);
  virtual ~Object();

  Object(const Object &) = delete;
  Object &operator=(const Object &) = delete;

private:
  void commonConstructor();

private:
  // Flags specifying various properties of the Object.
  enum ObjectAttributes {
    IsLine,
    IsScope,
    IsSymbol,
    IsType,
    HasType,
    IsGlobalReference,
    IsResolved,
    IsInlined,
    InvalidFilename,
    HasReference,
    HasQualifiedName,
    HasPattern,
    ObjectAttributesSize
  };
  // Flags specifying various properties of the Object.
  std::bitset<ObjectAttributesSize> ObjectAttributesFlags;

public:
  /// \brief Get the object kind as a string.
  virtual const char *getKindAsString() const = 0;

public:
  /// \brief The Object is a line.
  bool getIsLine() const { return ObjectAttributesFlags[IsLine]; }
  void setIsLine() { ObjectAttributesFlags.set(IsLine); }

  /// \brief The Object is a scope.
  bool getIsScope() const { return ObjectAttributesFlags[IsScope]; }
  void setIsScope() { ObjectAttributesFlags.set(IsScope); }

  /// \brief The Object is a symbol.
  bool getIsSymbol() const { return ObjectAttributesFlags[IsSymbol]; }
  void setIsSymbol() { ObjectAttributesFlags.set(IsSymbol); }

  /// \brief The Object is a type.
  bool getIsType() const { return ObjectAttributesFlags[IsType]; }
  void setIsType() { ObjectAttributesFlags.set(IsType); }

  /// \brief The Object has a type.
  bool getHasType() const { return ObjectAttributesFlags[HasType]; }
  void setHasType() { ObjectAttributesFlags.set(HasType); }

  /// \brief The Object is referenced from other CUs.
  bool getIsGlobalReference() const {
    return ObjectAttributesFlags[IsGlobalReference];
  }
  void setIsGlobalReference() { ObjectAttributesFlags.set(IsGlobalReference); }

  /// \brief The Object has been resolved by the reader.
  bool getIsResolved() const { return ObjectAttributesFlags[IsResolved]; }
  void setIsResolved() { ObjectAttributesFlags.set(IsResolved); }

  /// \brief The Object has been inlined.
  bool getIsInlined() const { return ObjectAttributesFlags[IsInlined]; }
  void setIsInlined() { ObjectAttributesFlags.set(IsInlined); }

  /// \brief The filename associated with the object is valid.
  bool getInvalidFileName() const {
    return ObjectAttributesFlags[InvalidFilename];
  }
  void setInvalidFileName() { ObjectAttributesFlags.set(InvalidFilename); }

  /// \brief The Object has a reference to another object.
  ///
  /// DW_AT_specification, DW_AT_abstract_origin, DW_AT_extension.
  bool getHasReference() const { return ObjectAttributesFlags[HasReference]; }
  void setHasReference() { ObjectAttributesFlags.set(HasReference); }

  /// \brief If the Object has a qualified name.
  bool getHasQualifiedName() const {
    return ObjectAttributesFlags[HasQualifiedName];
  }
  void setHasQualifiedName() { ObjectAttributesFlags.set(HasQualifiedName); }

  /// \brief Has a filter pattern.
  bool getHasPattern() const { return ObjectAttributesFlags[HasPattern]; }
  void setHasPattern() { ObjectAttributesFlags.set(HasPattern); }

private:
  // Track source file changes while printing.
  static size_t LastFilenameIndex;
  // Filler gap for the attributes.
  static size_t IndentationSize;

protected:
  static void resetFileIndex() { LastFilenameIndex = 0; }

  // Scope level for this object.
  LevelType Level;

  // Line associated with this object.
  uint64_t LineNumber;

  // The parent of this object (nullptr if the root scope).
  Scope *Parent;

  // Information to link the object back to the DWARF.
  Dwarf_Off DieOffset; // Global Offset in Debug Info.
  Dwarf_Half DieTag;   // DWARF tag/attr for this object.

protected:
  // Print the Filename or Pathname.
  void printFileIndex();

protected:
  // Get a string representation for the given line number.
  const char *getLineAsString(uint64_t LineNumber) const;

  // Get a string representation for the given number.
  const char *getReferenceAsString(uint64_t LineNumber, bool Spaces) const;

public:
  /// \brief DWARF Die tag.
  Dwarf_Half getDieTag() const { return DieTag; }
  void setDieTag(Dwarf_Half DWTag) { DieTag = DWTag; }

  /// \brief DWARF Die offset.
  Dwarf_Off getDieOffset() const { return DieOffset; }
  void setDieOffset(Dwarf_Off Offset) { DieOffset = Offset; }

  /// \brief DWARF parent Die offset.
  Dwarf_Off getDieParent() const;

public:
  /// \brief The Object's name.
  virtual const char *getName() const = 0;
  virtual void setName(const char *Name) = 0;

  /// \brief StringPool index of the Object's name.
  virtual size_t getNameIndex() const = 0;
  virtual void setNameIndex(size_t NameIndex) = 0;

  /// \brief The Object's qualified name.
  virtual const char *getQualifiedName() const = 0;
  virtual void setQualifiedName(const char *Name) = 0;

  /// \brief The Object's type name (if any).
  virtual const char *getTypeName() const = 0;

  /// \brief The Object's filename.
  virtual std::string getFileName(bool FormatOptions) const = 0;
  virtual void setFileName(const char *FileName) = 0;

  /// \brief StringPool index of the Object's filename.
  virtual size_t getFileNameIndex() const = 0;
  virtual void setFileNameIndex(size_t FilenameIndex) = 0;

  /// \brief The Object's type qualified name.
  virtual const char *getTypeQualifiedName() const = 0;

public:
  /// \brief If the Object has a name.
  virtual bool isNamed() const = 0;
  virtual bool isUnnamed() const { return !isNamed(); }

  bool isLined() const { return LineNumber != 0; }
  bool isUnlined() const { return !isLined(); }

  /// \brief Check if the object is printable.
  bool isPrintable() const { return !(isUnnamed()); }

  /// \brief Check if the object is not printable.
  bool isNotPrintable() const { return (isUnnamed()); }

  /// \brief Set the qualified name to include the parent's name.
  void resolveQualifiedName() { resolveQualifiedName(getParent()); }
  void resolveQualifiedName(const Scope *ExplicitParent);

  /// \brief The line for the object.
  uint64_t getLineNumber() const { return LineNumber; }
  void setLineNumber(uint64_t LnNumber) { LineNumber = LnNumber; }

  /// \brief The call line for the object (Inlined Functions).
  virtual uint64_t getCallLineNumber() const { return 0; }
  virtual void setCallLineNumber(uint64_t /*CallLineNumber*/) {}

  /// \brief The access DW_AT_GNU_discriminator attribute.
  virtual Dwarf_Half getDiscriminator() const { return 0; }
  virtual void setDiscriminator(Dwarf_Half /*Discriminator*/) {}

  /// \brief The level where this object is located.
  LevelType getLevel() const { return Level; }
  void setLevel(LevelType Lvl) { Level = Lvl; }
  std::string getIndentString(const PrintSettings &Settings) const;

  bool referenceMatch(const Object *Obj) const;

  /// \brief The parent scope for this object.
  Scope *getParent() const { return Parent; }
  void setParent(Scope *ObjParent) { Parent = ObjParent; }

  /// \brief If the Object is a Compile Unit.
  virtual bool getIsCompileUnit() const { return false; }

  // Get some attributes as string.
  const char *getDieOffsetAsString(const PrintSettings &Settings) const;
  const char *getTypeDieOffsetAsString(const PrintSettings &Settings) const;
  const char *getTypeAsString(const PrintSettings &Settings) const;

  /// \brief String to be used for objects with no line number.
  virtual const char *getNoLineString() const;

  /// \brief The line number to display.
  ///
  /// In the case of Inlined Functions, we use the DW_AT_call_line attribute;
  /// otherwise use the DW_AT_decl_line attribute.
  virtual const char *getLineNumberAsString() const {
    return getLineAsString(getLineNumber());
  }
  virtual std::string getLineNumberAsStringStripped();

public:
  // The object class type can point to a Type or Scope.
  virtual bool getIsKindType() const { return false; }
  virtual bool getIsKindScope() const { return false; }

  virtual Object *getType() const = 0;
  virtual void setType(Object *Obj) = 0;

  /// \brief Generate the full name for the object.
  bool setFullName(const PrintSettings &Settings, Type *BaseType,
                   Scope *BaseScope, Scope *SpecScope,
                   const char *BaseText = nullptr);

public:
  virtual void printAttributes(const PrintSettings &Settings);

  /// \brief Get the attributes associated with the object as string.
  std::string getAttributesAsText(const PrintSettings &Settings);

public:
  static size_t getIndentationSize() { return IndentationSize; }

public:
  virtual void dump(const PrintSettings &Settings);
  virtual void print(bool SplitCU, bool Match, bool IsNull,
                     const PrintSettings &Settings);
  virtual uint32_t getTag() const;
  virtual void setTag();

  /// \brief Should this object be printed under children?
  virtual bool getIsPrintedAsObject() const { return true; }
  /// \brief Returns a text representation of this DIVA Object.
  virtual std::string getAsText(const PrintSettings &Settings) const = 0;
  /// \brief Returns a YAML representation of this DIVA Object.
  virtual std::string getAsYAML() const = 0;

protected:
  /// \brief Returns a text representation of attribute information.
  std::string getAttributeInfoAsText(const std::string &AttributeText,
                                     const PrintSettings &Settings) const;
  /// \brief Returns the common YAML information for this object.
  std::string getCommonYAML() const;

#ifndef NDEBUG
protected:
  uint32_t Tag;
#endif
};

/// \brief Class to represent the basic data for an object.
class Element : public Object {
public:
  Element();
  Element(LevelType Lvl);
  virtual ~Element() override {}

  Element &operator=(const Element &) = delete;
  Element(const Element &) = delete;

private:
  void CommonConstructor();

protected:
  // The name, type name, qualified name and filename in String Pool.
  size_t NameIndex;
  size_t QualifiedIndex;
  size_t FilenameIndex;

  // Type of this object.
  Object *TheType;

private:
// The strings associated with the indexes, for easy debugging.
#ifndef NDEBUG
  std::string Name;
#endif

public:
  bool isNamed() const override { return NameIndex != 0; }
  bool isUnnamed() const override { return !isNamed(); }

public:
  // The object class type can point to a Type or Scope.
  bool getIsKindType() const override {
    return TheType && TheType->getIsType();
  }

public:
  /// \brief The Object's name.
  const char *getName() const override;
  void setName(const char *Name) override;

  /// \brief The StringPool index of the Object's name.
  size_t getNameIndex() const override;
  void setNameIndex(size_t NameIndex) override;

  /// \brief The Object's qualified name.
  const char *getQualifiedName() const override;
  void setQualifiedName(const char *Name) override;

  /// \brief The Object's type name (if any).
  const char *getTypeName() const override;

  /// \brief The Object's filename.
  std::string getFileName(bool NameOnly) const override;
  void setFileName(const char *FileName) override;

  /// \brief The StringPool index of the Object's filename.
  size_t getFileNameIndex() const override;
  void setFileNameIndex(size_t FilenameIndex) override;

  /// \brief The Object's type qualified name.
  const char *getTypeQualifiedName() const override;

public:
  void setType(Object *Obj) override {
    setHasType();
    TheType = Obj;
  }
  Object *getType() const override { return TheType; }
};

} // namespace LibScopeView

#endif // OBJECT_H
