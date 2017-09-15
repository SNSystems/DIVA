//===-- LibScopeView/CmdOptions.h -------------------------------*- C++ -*-===//
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
/// Definitions of command options.
///
//===----------------------------------------------------------------------===//

#ifndef CMDOPTIONS_H_
#define CMDOPTIONS_H_

#include <bitset>

namespace LibScopeView {

class CmdOptions {
private:
  // Object Attributes.
  enum ObjectAttributes {
    _AttributeSeen,
    AttributeFile,
    AttributeGlobal,
    AttributeLevel,
    AttributeOffset,
    AttributeParent,
    AttributeTag,
    AttributeType,
    ObjectAttributeSize
  };
  std::bitset<ObjectAttributeSize> ObjectAttributeFlags;

  // Format options.
  enum FormatOptions {
    _FormatSeen,
    FormatArraysEncoded,
    FormatBaseTypes,
    FormatCombineScopes,
    FormatDiscriminators,
    FormatFilename,
    FormatGenerated,
    FormatIndentation,
    FormatLine,
    FormatOnlyGlobals,
    FormatOnlyLocals,
    FormatPathname,
    FormatQualifiedName,
    FormatTemplatesEncoded,
    FormatTemplatesResolved,
    FormatUnderlyingType,
    FormatVoidType,
    FormatZeroLineNumbers,
    FormatOptionsSize
  };
  std::bitset<FormatOptionsSize> FormatOptionsFlags;

  // Internal options.
  enum InternalOptions {
    _InternalSeen,
    InternalShortcuts,
    InternalOptionsSize
  };
  std::bitset<InternalOptionsSize> InternalOptionsFlags;

  // Print options.
  enum PrintOptions {
    _PrintSeen,
    PrintFilenames,
    PrintLines,
    PrintScopes,
    PrintSymbols,
    PrintSummary,
    PrintTypes,
    PrintOptionsSize
  };
  std::bitset<PrintOptionsSize> PrintOptionsFlags;

  // General Traces.
  enum GeneralTraces {
    _TraceSeen,
    _TraceAll,
    TraceQuiet,
    TraceVerbose,
    GeneralTracesSize
  };
  std::bitset<GeneralTracesSize> GeneralTracesFlags;

  // Usage Information.
  enum UsageInformation {
    _UsageSeen,
    UsageArguments,
    UsageErrorTable,
    UsageMemoryInfo,
    UsageReaderAlloc,
    UsageStringPoolInfo,
    UsageStringPoolTable,
    UsageSwitches,
    UsageInformationSize
  };
  std::bitset<UsageInformationSize> UsageInformationFlags;

  // View Global options.
  enum GlobalOptions {
    _ViewSeen,
    ViewDualPrint,
    ViewFilter,
    ViewSort,
    ViewSplit,
    ViewSplitDir,
    ViewTree,
    GlobalOptionsSize
  };
  std::bitset<GlobalOptionsSize> GlobalOptionsFlags;

  // Help.
  enum Help {
    _HelpSeen,
    HelpAdvancedUsage,
    HelpDeveloperUsage,
    HelpIntermediateUsage,
    HelpUsage,
    HelpSize
  };
  std::bitset<HelpSize> HelpFlags;

  // Version.
  enum Version { _VersionSeen, VersionDetails, VersionSize };
  std::bitset<VersionSize> VersionFlags;

  // Operation Modes.
  enum OperationModes { AllowPrinting, OperationModesSize };
  std::bitset<OperationModesSize> OperationModesFlags;

  // Version 0.2 Object options.
  struct {
    // Default values are set when setPrintBrief is called.
    bool print_alias;
    bool print_array;
    bool print_block;
    bool print_block_attributes;
    bool print_class;
    bool print_codeline;
    bool print_enum;
    bool print_function;
    bool print_member;
    bool print_namespace;
    bool print_parameter;
    bool print_primitivetype;
    bool print_struct;
    bool print_template;
    bool print_typedef;
    bool print_union;
    bool print_using;
    bool print_variable;
  } object_options;

  // Version 0.2 Advanced object options.
  struct {
    bool print_codeline_attributes = false;
  } advanced_object_options;

  // Version 0.4 Output format options.
  struct {
    bool output_format_specified = false;
    bool output_text = false;
    bool output_yaml = false;
  } output_format_options;

public:
  CmdOptions() {}

  // Update the Reader command line options with the Global command line
  // options, preserving any option that is already set.
  void UpdateOptions(const CmdOptions &rhs) {
    ObjectAttributeFlags |= rhs.ObjectAttributeFlags;
    FormatOptionsFlags |= rhs.FormatOptionsFlags;
    InternalOptionsFlags |= rhs.InternalOptionsFlags;
    PrintOptionsFlags |= rhs.PrintOptionsFlags;
    GeneralTracesFlags |= rhs.GeneralTracesFlags;
    UsageInformationFlags |= rhs.UsageInformationFlags;
    GlobalOptionsFlags |= rhs.GlobalOptionsFlags;
    // Standard options: -h, -v
    HelpFlags |= rhs.HelpFlags;
    VersionFlags |= rhs.VersionFlags;
    // Operation Modes.
    OperationModesFlags |= rhs.OperationModesFlags;
    // Version 0.2 Object options.
    object_options = rhs.object_options;
    // Version 0.4 Output format options.
    output_format_options = rhs.output_format_options;
  }

  // Do nothing.
  bool getNothing() const { return false; }
  void setNothing() {}
  void resetNothing() {}

  // Object Attributes.
  bool getAttributeSeen() const { return ObjectAttributeFlags[_AttributeSeen]; }
  void setAttributeSeen() { ObjectAttributeFlags.set(_AttributeSeen); }
  void resetAttributeSeen() { ObjectAttributeFlags.set(_AttributeSeen, false); }

  bool getAttributeFile() const { return ObjectAttributeFlags[AttributeFile]; }
  void setAttributeFile() {
    ObjectAttributeFlags.set(AttributeFile);
    setAttributeSeen();
  }
  void resetAttributeFile() {
    ObjectAttributeFlags.set(AttributeFile, false);
    setAttributeSeen();
  }
  bool getAttributeGlobal() const {
    return ObjectAttributeFlags[AttributeGlobal];
  }
  void setAttributeGlobal() {
    ObjectAttributeFlags.set(AttributeGlobal);
    setAttributeSeen();
  }
  void resetAttributeGlobal() {
    ObjectAttributeFlags.set(AttributeGlobal, false);
    setAttributeSeen();
  }
  bool getAttributeLevel() const {
    return ObjectAttributeFlags[AttributeLevel];
  }
  void setAttributeLevel() {
    ObjectAttributeFlags.set(AttributeLevel);
    setAttributeSeen();
  }
  void resetAttributeLevel() {
    ObjectAttributeFlags.set(AttributeLevel, false);
    setAttributeSeen();
  }
  bool getAttributeOffset() const {
    return ObjectAttributeFlags[AttributeOffset];
  }
  void setAttributeOffset() {
    ObjectAttributeFlags.set(AttributeOffset);
    setAttributeSeen();
  }
  void resetAttributeOffset() {
    ObjectAttributeFlags.set(AttributeOffset, false);
    setAttributeSeen();
  }
  bool getAttributeParent() const {
    return ObjectAttributeFlags[AttributeParent];
  }
  void setAttributeParent() {
    ObjectAttributeFlags.set(AttributeParent);
    setAttributeSeen();
  }
  void resetAttributeParent() {
    ObjectAttributeFlags.set(AttributeParent, false);
    setAttributeSeen();
  }
  bool getAttributeTag() const { return ObjectAttributeFlags[AttributeTag]; }
  void setAttributeTag() {
    ObjectAttributeFlags.set(AttributeTag);
    setAttributeSeen();
  }
  void resetAttributeTag() {
    ObjectAttributeFlags.set(AttributeTag, false);
    setAttributeSeen();
  }
  bool getAttributeType() const { return ObjectAttributeFlags[AttributeType]; }
  void setAttributeType() {
    ObjectAttributeFlags.set(AttributeType);
    setAttributeSeen();
  }
  void resetAttributeType() {
    ObjectAttributeFlags.set(AttributeType, false);
    setAttributeSeen();
  }

  // Format options.
  bool getFormatSeen() const { return FormatOptionsFlags[_FormatSeen]; }
  void setFormatSeen() { FormatOptionsFlags.set(_FormatSeen); }
  void resetFormatSeen() { FormatOptionsFlags.set(_FormatSeen, false); }

  bool getFormatArraysEncoded() const {
    return FormatOptionsFlags[FormatArraysEncoded];
  }
  void setFormatArraysEncoded() {
    FormatOptionsFlags.set(FormatArraysEncoded);
    setFormatSeen();
  }
  void resetFormatArraysEncoded() {
    FormatOptionsFlags.set(FormatArraysEncoded, false);
    setFormatSeen();
  }
  bool getFormatBaseTypes() const {
    return FormatOptionsFlags[FormatBaseTypes];
  }
  void setFormatBaseTypes() {
    FormatOptionsFlags.set(FormatBaseTypes);
    setFormatSeen();
  }
  void resetFormatBaseTypes() {
    FormatOptionsFlags.set(FormatBaseTypes, false);
    setFormatSeen();
  }
  bool getFormatCombineScopes() const {
    return FormatOptionsFlags[FormatCombineScopes];
  }
  void setFormatCombineScopes() {
    FormatOptionsFlags.set(FormatCombineScopes);
    setFormatSeen();
  }
  void resetFormatCombineScopes() {
    FormatOptionsFlags.set(FormatCombineScopes, false);
    setFormatSeen();
  }
  bool getFormatDiscriminators() const {
    return FormatOptionsFlags[FormatDiscriminators];
  }
  void setFormatDiscriminators() {
    FormatOptionsFlags.set(FormatDiscriminators);
    setFormatSeen();
  }
  void resetFormatDiscriminators() {
    FormatOptionsFlags.set(FormatDiscriminators, false);
    setFormatSeen();
  }
  bool getFormatFileName() const { return FormatOptionsFlags[FormatFilename]; }
  void setFormatFileName() {
    FormatOptionsFlags.set(FormatFilename);
    FormatOptionsFlags.set(FormatPathname, false);
    setFormatSeen();
  }
  void resetFormatFileName() {
    FormatOptionsFlags.set(FormatFilename, false);
    setFormatSeen();
  }
  bool getFormatGenerated() const {
    return FormatOptionsFlags[FormatGenerated];
  }
  void setFormatGenerated() {
    FormatOptionsFlags.set(FormatGenerated);
    setFormatSeen();
  }
  void resetFormatGenerated() {
    FormatOptionsFlags.set(FormatGenerated, false);
    FormatOptionsFlags.set(FormatPathname, false);
    setFormatSeen();
  }
  bool getFormatIndentation() const {
    return FormatOptionsFlags[FormatIndentation];
  }
  void setFormatIndentation() {
    FormatOptionsFlags.set(FormatIndentation);
    setFormatSeen();
  }
  void resetFormatIndentation() {
    FormatOptionsFlags.set(FormatIndentation, false);
    setFormatSeen();
  }
  bool getFormatLine() const { return FormatOptionsFlags[FormatLine]; }
  void setFormatLine() {
    FormatOptionsFlags.set(FormatLine);
    setFormatSeen();
  }
  void resetFormatLine() {
    FormatOptionsFlags.set(FormatLine, false);
    setFormatSeen();
  }
  bool getFormatOnlyGlobals() const {
    return FormatOptionsFlags[FormatOnlyGlobals];
  }
  void setFormatOnlyGlobals() {
    FormatOptionsFlags.set(FormatOnlyGlobals);
    setFormatSeen();
  }
  void resetFormatOnlyGlobals() {
    FormatOptionsFlags.set(FormatOnlyGlobals, false);
    setFormatSeen();
  }
  bool getFormatOnlyLocals() const {
    return FormatOptionsFlags[FormatOnlyLocals];
  }
  void setFormatOnlyLocals() {
    FormatOptionsFlags.set(FormatOnlyLocals);
    setFormatSeen();
  }
  void resetFormatOnlyLocals() {
    FormatOptionsFlags.set(FormatOnlyLocals, false);
    setFormatSeen();
  }
  bool getFormatPathName() const { return FormatOptionsFlags[FormatPathname]; }
  void setFormatPathName() {
    FormatOptionsFlags.set(FormatPathname);
    FormatOptionsFlags.set(FormatFilename, false);
    setFormatSeen();
  }
  void resetFormatPathName() {
    FormatOptionsFlags.set(FormatPathname, false);
    setFormatSeen();
  }
  bool getFormatQualifiedName() const {
    return FormatOptionsFlags[FormatQualifiedName];
  }
  void setFormatQualifiedName() {
    FormatOptionsFlags.set(FormatQualifiedName);
    setFormatSeen();
  }
  void resetFormatQualifiedName() {
    FormatOptionsFlags.set(FormatQualifiedName, false);
    setFormatSeen();
  }
  bool getFormatTemplatesEncoded() const {
    return FormatOptionsFlags[FormatTemplatesEncoded];
  }
  void setFormatTemplatesEncoded() {
    FormatOptionsFlags.set(FormatTemplatesEncoded);
    setFormatSeen();
  }
  void resetFormatTemplatesEncoded() {
    FormatOptionsFlags.set(FormatTemplatesEncoded, false);
    setFormatSeen();
  }
  bool getFormatTemplatesResolved() const {
    return FormatOptionsFlags[FormatTemplatesResolved];
  }
  void setFormatTemplatesResolved() {
    FormatOptionsFlags.set(FormatTemplatesResolved);
    setFormatSeen();
  }
  void resetFormatTemplatesResolved() {
    FormatOptionsFlags.set(FormatTemplatesResolved, false);
    setFormatSeen();
  }
  bool getFormatUnderlyingType() const {
    return FormatOptionsFlags[FormatUnderlyingType];
  }
  void setFormatUnderlyingType() {
    FormatOptionsFlags.set(FormatUnderlyingType);
    setFormatSeen();
  }
  void resetFormatUnderlyingType() {
    FormatOptionsFlags.set(FormatUnderlyingType, false);
    setFormatSeen();
  }
  bool getFormatVoidType() const { return FormatOptionsFlags[FormatVoidType]; }
  void setFormatVoidType() {
    FormatOptionsFlags.set(FormatVoidType);
    setFormatSeen();
  }
  void resetFormatVoidType() {
    FormatOptionsFlags.set(FormatVoidType, false);
    setFormatSeen();
  }
  bool getFormatZeroLineNumbers() const {
    return FormatOptionsFlags[FormatZeroLineNumbers];
  }
  void setFormatZeroLineNumbers() {
    FormatOptionsFlags.set(FormatZeroLineNumbers);
    setFormatSeen();
  }
  void resetFormatZeroLineNumbers() {
    FormatOptionsFlags.set(FormatZeroLineNumbers, false);
    setFormatSeen();
  }

  // Internal options.
  bool getInternalSeen() const { return InternalOptionsFlags[_InternalSeen]; }
  void setInternalSeen() { InternalOptionsFlags.set(_InternalSeen); }
  void resetInternalSeen() { InternalOptionsFlags.set(_InternalSeen, false); }

  bool getInternalShortcuts() const {
    return InternalOptionsFlags[InternalShortcuts];
  }
  void setInternalShortcuts() {
    InternalOptionsFlags.set(InternalShortcuts);
    setInternalSeen();
  }
  void resetInternalShortcuts() {
    InternalOptionsFlags.set(InternalShortcuts, false);
    setInternalSeen();
  }

  // Print options.
  bool getPrintSeen() const { return PrintOptionsFlags[_PrintSeen]; }
  void setPrintSeen() { PrintOptionsFlags.set(_PrintSeen); }
  void resetPrintSeen() { PrintOptionsFlags.set(_PrintSeen, false); }

  // Version 0.2 object options.
  bool getPrintAlias() const { return object_options.print_alias; }
  void setPrintAlias() {
    object_options.print_alias = true;
    this->setPrintTypedef();
  }
  void resetPrintAlias() {
    object_options.print_alias = false;
    this->resetPrintTypedef();
  }
  bool getPrintArray() const { return object_options.print_array; }
  void setPrintArray() { object_options.print_array = true; }
  void resetPrintArray() { object_options.print_array = false; }
  bool getPrintBlock() const { return object_options.print_block; }
  void setPrintBlock() { object_options.print_block = true; }
  void resetPrintBlock() { object_options.print_block = false; }
  bool getPrintBlockAttributes() const {
    return object_options.print_block_attributes;
  }
  void setPrintBlockAttributes() {
    object_options.print_block_attributes = true;
  }
  void resetPrintBlockAttributes() {
    object_options.print_block_attributes = false;
  }
  bool getPrintClass() const { return object_options.print_class; }
  void setPrintClass() { object_options.print_class = true; }
  void resetPrintClass() { object_options.print_class = false; }
  bool getPrintCodeline() const { return object_options.print_codeline; }
  void setPrintCodeline() { object_options.print_codeline = true; }
  void resetPrintCodeline() { object_options.print_codeline = false; }
  bool getPrintEnum() const { return object_options.print_enum; }
  void setPrintEnum() { object_options.print_enum = true; }
  void resetPrintEnum() { object_options.print_enum = false; }
  bool getPrintFunction() const { return object_options.print_function; }
  void setPrintFunction() { object_options.print_function = true; }
  void resetPrintFunction() { object_options.print_function = false; }
  bool getPrintMember() const { return object_options.print_member; }
  void setPrintMember() { object_options.print_member = true; }
  void resetPrintMember() { object_options.print_member = false; }
  bool getPrintNamespace() const { return object_options.print_namespace; }
  void setPrintNamespace() { object_options.print_namespace = true; }
  void resetPrintNamespace() { object_options.print_namespace = false; }
  bool getPrintParameter() const { return object_options.print_parameter; }
  void setPrintParameter() { object_options.print_parameter = true; }
  void resetPrintParameter() { object_options.print_parameter = false; }
  bool getPrintPrimitivetype() const {
    return object_options.print_primitivetype;
  }
  void setPrintPrimitivetype() { object_options.print_primitivetype = true; }
  void resetPrintPrimitivetype() { object_options.print_primitivetype = false; }
  bool getPrintStruct() const { return object_options.print_struct; }
  void setPrintStruct() { object_options.print_struct = true; }
  void resetPrintStruct() { object_options.print_struct = false; }
  bool getPrintTemplate() const { return object_options.print_template; }
  void setPrintTemplate() { object_options.print_template = true; }
  void resetPrintTemplate() { object_options.print_template = false; }
  bool getPrintTypedef() const { return object_options.print_typedef; }
  void setPrintTypedef() { object_options.print_typedef = true; }
  void resetPrintTypedef() { object_options.print_typedef = false; }
  bool getPrintUnion() const { return object_options.print_union; }
  void setPrintUnion() { object_options.print_union = true; }
  void resetPrintUnion() { object_options.print_union = false; }
  bool getPrintUsing() const { return object_options.print_using; }
  void setPrintUsing() { object_options.print_using = true; }
  void resetPrintUsing() { object_options.print_using = false; }
  bool getPrintVariable() const { return object_options.print_variable; }
  void setPrintVariable() { object_options.print_variable = true; }
  void resetPrintVariable() { object_options.print_variable = false; }

  // Version 0.2 Advanced object options.
  bool getPrintCodelineAttributes() const {
    return advanced_object_options.print_codeline_attributes;
  }
  void setPrintCodelineAttributes() {
    advanced_object_options.print_codeline_attributes = true;
  }
  void resetPrintCodelineAttributes() {
    advanced_object_options.print_codeline_attributes = false;
  }

  void setPrintAll() {
    setPrintBrief();

    setPrintBlockAttributes();
    setPrintPrimitivetype();
  }

  void setPrintBrief() {
    setPrintAlias();
    setPrintBlock();
    setPrintClass();
    setPrintEnum();
    setPrintFunction();
    setPrintMember();
    setPrintNamespace();
    setPrintParameter();
    setPrintStruct();
    setPrintTemplate();
    setPrintUnion();
    setPrintUsing();
    setPrintVariable();

    resetPrintArray();
    resetPrintBlockAttributes();
    resetPrintCodeline();
    resetPrintPrimitivetype();
  }
  void setPrintNone() {
    resetPrintAlias();
    resetPrintArray();
    resetPrintBlock();
    resetPrintClass();
    resetPrintCodeline();
    resetPrintEnum();
    resetPrintFunction();
    resetPrintMember();
    resetPrintNamespace();
    resetPrintParameter();
    resetPrintStruct();
    resetPrintTemplate();
    resetPrintUnion();
    resetPrintUsing();
    resetPrintVariable();
    resetPrintBlockAttributes();
    resetPrintPrimitivetype();
  }

  // Version 0.4 Output format options.
  void AddOutputFormatText() {
    output_format_options.output_format_specified = true;
    output_format_options.output_text = true;
  }
  void AddOutputFormatYAML() {
    output_format_options.output_format_specified = true;
    output_format_options.output_yaml = true;
  }
  bool getOutputFormatText() const {
    return !output_format_options.output_format_specified ||
           output_format_options.output_text;
  }
  bool getOutputFormatYAML() const { return output_format_options.output_yaml; }

  bool getPrintFilenames() const { return PrintOptionsFlags[PrintFilenames]; }
  void setPrintFilenames() {
    PrintOptionsFlags.set(PrintFilenames);
    setPrintSeen();
  }
  void resetPrintFilenames() {
    PrintOptionsFlags.set(PrintFilenames, false);
    setPrintSeen();
  }
  bool getPrintLines() const { return PrintOptionsFlags[PrintLines]; }
  void setPrintLines() {
    PrintOptionsFlags.set(PrintLines);
    setPrintSeen();
  }
  void resetPrintLines() {
    PrintOptionsFlags.set(PrintLines, false);
    setPrintSeen();
  }
  bool getPrintScopes() const { return PrintOptionsFlags[PrintScopes]; }
  void setPrintScopes() {
    PrintOptionsFlags.set(PrintScopes);
    setPrintSeen();
  }
  void resetPrintScopes() {
    PrintOptionsFlags.set(PrintScopes, false);
    setPrintSeen();
  }
  bool getPrintSymbols() const { return PrintOptionsFlags[PrintSymbols]; }
  void setPrintSymbols() {
    PrintOptionsFlags.set(PrintSymbols);
    setPrintSeen();
  }
  void resetPrintSymbols() {
    PrintOptionsFlags.set(PrintSymbols, false);
    setPrintSeen();
  }
  bool getPrintSummary() const { return PrintOptionsFlags[PrintSummary]; }
  void setPrintSummary() {
    PrintOptionsFlags.set(PrintSummary);
    setPrintSeen();
  }
  void resetPrintSummary() {
    PrintOptionsFlags.set(PrintSummary, false);
    setPrintSeen();
  }
  bool getPrintTypes() const { return PrintOptionsFlags[PrintTypes]; }
  void setPrintTypes() {
    PrintOptionsFlags.set(PrintTypes);
    setPrintSeen();
  }
  void resetPrintTypes() {
    PrintOptionsFlags.set(PrintTypes, false);
    setPrintSeen();
  }

  // General Traces.
  bool getTraceSeen() const { return GeneralTracesFlags[_TraceSeen]; }
  void setTraceSeen() { GeneralTracesFlags.set(_TraceSeen); }
  void resetTraceSeen() { GeneralTracesFlags.set(_TraceSeen, false); }

  bool getTraceAll() const { return GeneralTracesFlags[_TraceAll]; }
  void setTraceAll() {
    GeneralTracesFlags.set(_TraceAll);
    setTraceSeen();

    // Set --trace-all.
    TraceAll(/*set=*/true);
  }
  void resetTraceAll() {
    GeneralTracesFlags.set(_TraceAll, false);
    setTraceSeen();

    // Reset --trace-all.
    TraceAll(/*set=*/false);
  }
  bool getTraceQuiet() const { return GeneralTracesFlags[TraceQuiet]; }
  void setTraceQuiet() {
    GeneralTracesFlags.set(TraceQuiet);
    setTraceSeen();
  }
  void resetTraceQuiet() {
    GeneralTracesFlags.set(TraceQuiet, false);
    setTraceSeen();
  }
  bool getTraceVerbose() const { return GeneralTracesFlags[TraceVerbose]; }
  void setTraceVerbose() {
    GeneralTracesFlags.set(TraceVerbose);
    setTraceSeen();
  }
  void resetTraceVerbose() {
    GeneralTracesFlags.set(TraceVerbose, false);
    setTraceSeen();
  }

  // Usage Information.
  bool getUsageSeen() const { return UsageInformationFlags[_UsageSeen]; }
  void setUsageSeen() { UsageInformationFlags.set(_UsageSeen); }
  void resetUsageSeen() { UsageInformationFlags.set(_UsageSeen, false); }

  bool getUsageArguments() const {
    return UsageInformationFlags[UsageArguments];
  }
  void setUsageArguments() {
    UsageInformationFlags.set(UsageArguments);
    setUsageSeen();
  }
  void resetUsageArguments() {
    UsageInformationFlags.set(UsageArguments, false);
    setUsageSeen();
  }
  bool getUsageErrorTable() const {
    return UsageInformationFlags[UsageErrorTable];
  }
  void setUsageErrorTable() {
    UsageInformationFlags.set(UsageErrorTable);
    setUsageSeen();
  }
  void resetUsageErrorTable() {
    UsageInformationFlags.set(UsageErrorTable, false);
    setUsageSeen();
  }
  bool getUsageMemoryInfo() const {
    return UsageInformationFlags[UsageMemoryInfo];
  }
  void setUsageMemoryInfo() {
    UsageInformationFlags.set(UsageMemoryInfo);
    setUsageSeen();
  }
  void resetUsageMemoryInfo() {
    UsageInformationFlags.set(UsageMemoryInfo, false);
    setUsageSeen();
  }
  bool getUsageReaderAlloc() const {
    return UsageInformationFlags[UsageReaderAlloc];
  }
  void setUsageReaderAlloc() {
    UsageInformationFlags.set(UsageReaderAlloc);
    setUsageSeen();
  }
  void resetUsageReaderAlloc() {
    UsageInformationFlags.set(UsageReaderAlloc, false);
    setUsageSeen();
  }
  bool getUsageStringPoolInfo() const {
    return UsageInformationFlags[UsageStringPoolInfo];
  }
  void setUsageStringPoolInfo() {
    UsageInformationFlags.set(UsageStringPoolInfo);
    setUsageSeen();
  }
  void resetUsageStringPoolInfo() {
    UsageInformationFlags.set(UsageStringPoolInfo, false);
    setUsageSeen();
  }
  bool getUsageStringPoolTable() const {
    return UsageInformationFlags[UsageStringPoolTable];
  }
  void setUsageStringPoolTable() {
    UsageInformationFlags.set(UsageStringPoolTable);
    setUsageSeen();
  }
  void resetUsageStringPoolTable() {
    UsageInformationFlags.set(UsageStringPoolTable, false);
    setUsageSeen();
  }
  bool getUsageSwitches() const { return UsageInformationFlags[UsageSwitches]; }
  void setUsageSwitches() {
    UsageInformationFlags.set(UsageSwitches);
    setUsageSeen();
  }
  void resetUsageSwitches() {
    UsageInformationFlags.set(UsageSwitches, false);
    setUsageSeen();
  }

  // View Global options.
  bool getViewSeen() const { return GlobalOptionsFlags[_ViewSeen]; }
  void setViewSeen() { GlobalOptionsFlags.set(_ViewSeen); }
  void resetViewSeen() { GlobalOptionsFlags.set(_ViewSeen, false); }

  bool getViewDualPrint() const { return GlobalOptionsFlags[ViewDualPrint]; }
  void setViewDualPrint() {
    GlobalOptionsFlags.set(ViewDualPrint);
    setViewSeen();
  }
  void resetViewDualPrint() {
    GlobalOptionsFlags.set(ViewDualPrint, false);
    setViewSeen();
  }
  bool getViewFilter() const { return GlobalOptionsFlags[ViewFilter]; }
  void setViewFilter() {
    GlobalOptionsFlags.set(ViewFilter);
    setViewSeen();
  }
  void resetViewFilter() {
    GlobalOptionsFlags.set(ViewFilter, false);
    setViewSeen();
  }
  bool getViewSort() const { return GlobalOptionsFlags[ViewSort]; }
  void setViewSort() {
    GlobalOptionsFlags.set(ViewSort);
    setViewSeen();
  }
  void resetViewSort() {
    GlobalOptionsFlags.set(ViewSort, false);
    setViewSeen();
  }
  bool getViewSplit() const { return GlobalOptionsFlags[ViewSplit]; }
  void setViewSplit() {
    GlobalOptionsFlags.set(ViewSplit);
    setFormatSeen();
  }
  void resetViewSplit() {
    GlobalOptionsFlags.set(ViewSplit, false);
    setFormatSeen();
  }
  bool getViewSplitDir() const { return GlobalOptionsFlags[ViewSplitDir]; }
  void setViewSplitDir() {
    GlobalOptionsFlags.set(ViewSplitDir);
    setFormatSeen();
  }
  void resetViewSplitDir() {
    GlobalOptionsFlags.set(ViewSplitDir, false);
    setFormatSeen();

    // Always use dual view print.
    resetViewDualPrint();
  }
  bool getViewTree() const { return GlobalOptionsFlags[ViewTree]; }
  void setViewTree() {
    GlobalOptionsFlags.set(ViewTree);
    setViewSeen();
  }
  void resetViewTree() {
    GlobalOptionsFlags.set(ViewTree, false);
    setViewSeen();
  }

  // Help.
  bool getHelpSeen() const { return HelpFlags[_HelpSeen]; }
  void setHelpSeen() { HelpFlags.set(_HelpSeen); }
  void resetHelpSeen() { HelpFlags.set(_HelpSeen, false); }

  bool getHelpAdvancedUsage() const { return HelpFlags[HelpAdvancedUsage]; }
  void setHelpAdvancedUsage() {
    HelpFlags.set(HelpAdvancedUsage);
    setHelpSeen();
  }
  void resetHelpAdvancedUsage() {
    HelpFlags.set(HelpAdvancedUsage, false);
    setHelpSeen();
  }
  bool getHelpDeveloperUsage() const { return HelpFlags[HelpDeveloperUsage]; }
  void setHelpDeveloperUsage() {
    HelpFlags.set(HelpDeveloperUsage);
    setHelpSeen();
  }
  void resetHelpDeveloperUsage() {
    HelpFlags.set(HelpDeveloperUsage, false);
    setHelpSeen();
  }
  bool getHelpIntermediateUsage() const {
    return HelpFlags[HelpIntermediateUsage];
  }
  void setHelpIntermediateUsage() {
    HelpFlags.set(HelpIntermediateUsage);
    setHelpSeen();
  }
  void resetHelpIntermediateUsage() {
    HelpFlags.set(HelpIntermediateUsage, false);
    setHelpSeen();
  }
  bool getHelpUsage() const { return HelpFlags[HelpUsage]; }
  void setHelpUsage() {
    HelpFlags.set(HelpUsage);
    setHelpSeen();
  }
  void resetHelpUsage() {
    HelpFlags.set(HelpUsage, false);
    setHelpSeen();
  }

  // Version.
  bool getVersionSeen() const { return VersionFlags[_VersionSeen]; }
  void setVersionSeen() { VersionFlags.set(_VersionSeen); }
  void resetVersionSeen() { VersionFlags.set(_VersionSeen, false); }

  bool getVersionDetails() const { return VersionFlags[VersionDetails]; }
  void setVersionDetails() {
    VersionFlags.set(VersionDetails);
    setVersionSeen();
  }
  void resetVersionDetails() {
    VersionFlags.set(VersionDetails, false);
    setVersionSeen();
  }

  // Enable/Disable options for --print-all.
  void PrintAllObjects(bool set) {
    if (set) {
      // Same as print the reader.
      setPrintScopes();
      setPrintSymbols();
      setPrintTypes();
    } else {
      // Same as print the reader.
      resetPrintScopes();
      resetPrintSymbols();
      resetPrintTypes();
    }
  }

  // Enable/Disable options for --trace-all.
  void TraceAll(bool set) {
    if (set) {
      setTraceQuiet();
      setTraceVerbose();
    } else {
      resetTraceQuiet();
      resetTraceVerbose();
    }
  }

  // Operation Modes.
  bool getAllowPrinting() const { return OperationModesFlags[AllowPrinting]; }
  void setAllowPrinting() { OperationModesFlags.set(AllowPrinting); }
  void resetAllowPrinting() { OperationModesFlags.set(AllowPrinting, false); }

  // Any 'attribute' option have been requested.
  bool IsAttributeRequested() { return getAttributeSeen(); }
  // Any 'format' option have been requested.
  bool IsFormatRequested() { return getFormatSeen(); }
  // Any 'help' option have been requested.
  bool IsHelpRequested() { return getHelpSeen(); }
  // Any 'print' option have been requested.
  bool IsPrintRequested() { return getPrintSeen(); }
  // Any 'reader' option have been requested.
  bool IsViewRequested() { return getViewSeen(); }
  // Any 'trace' option have been requested.
  bool IsTraceRequested() { return getTraceSeen(); }
  // Any 'usage' option have been requested.
  bool IsUsageRequested() { return getUsageSeen(); }
  // Any 'version' option have been requested.
  bool IsVersionRequested() { return getVersionSeen(); }
};

} // namespace LibScopeView

#endif // CMDOPTIONS_H_
