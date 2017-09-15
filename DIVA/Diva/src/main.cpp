//===-- Diva/main.cpp -------------------------------------------*- C++ -*-===//
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
/// Contains the entry point for the DIVA executable.
///
//===----------------------------------------------------------------------===//

#include "DivaOptions.h"
#include "ElfDwarfReader.h"
#include "Error.h"
#include "ScopeYAMLPrinter.h"
#include "Utilities.h"
#include "ViewSpecification.h"

#include <memory>

namespace {

typedef std::unique_ptr<LibScopeView::Reader> ReaderUPtr;

/// \brief Allocate an appropriate reader for the given specification.
ReaderUPtr createReader(LibScopeView::ViewSpecification &Spec) {
  ReaderUPtr CreatedReader;
  switch (Spec.getReaderType()) {
  case LibScopeView::rt_libdwarf:
    CreatedReader = std::make_unique<ElfDwarfReader::DwarfReader>(&Spec);
    break;
  case LibScopeView::rt_unknown:
    // Unknown kind of reader.
    break;
  }
  return CreatedReader;
}

} // namespace

int main(int argc, char *argv[]) {
  auto StartTime = LibScopeView::getCurrentTime();

  // Library and general initialization.
  LibScopeView::initialize();

  // Argument parsing.
  const std::vector<std::string> CMDArgs(argv + 1, argv + argc);
  const DivaOptions Options(CMDArgs, /*HelpOut*/ std::cout,
                            /*VersionOut*/ std::cerr,
                            /*ErrOut*/ std::cerr);

  auto ViewSpecs = Options.convertToViewSpecs();

  std::map<std::string, std::unique_ptr<LibScopeView::Reader>> ReaderMap;

  // Create readers and load in the options.
  for (auto &Spec : ViewSpecs) {
    ReaderUPtr pReader(createReader(Spec));

    if (pReader) {
      ReaderMap.emplace(Spec.getID(), std::move(pReader));
    } else {
      // Reader was not created. Report failure.
      fatalError(LibScopeError::ErrorCode::ERR_VIEW_OBJECT_FAILURE,
                 Spec.getID());
    }
  }

  // Execute actions on readers.
  for (auto &MapPair : ReaderMap) {
    LibScopeView::Reader &AReader = *MapPair.second;
    bool Result = AReader.executeActions();

    if (!Result)
      fatalError(LibScopeError::ErrorCode::ERR_VIEW_SCOPE_FAILURE,
                 MapPair.first, *AReader.getError());
  }

  if (Options.ShowScopeAllocation)
    LibScopeView::printAllocationInfo();

  // Print the scope views in the readers.
  for (auto &MapPair : ReaderMap) {
    LibScopeView::Reader &AReader = *MapPair.second;
    // Print the Logical View.
    if (Options.OutputFormats.count(OutputFormat::TEXT)) {
      AReader.print();
    }
    // Print YAML.
    if (Options.OutputFormats.count(OutputFormat::YAML)) {
      // YAML_OUTPUT_VERSION_STR is defined by CMake.
      LibScopeView::ScopeYAMLPrinter YAMLPrinter(AReader.getInputFile(),
                                                 YAML_OUTPUT_VERSION_STR);
      if (AReader.getOptions().getViewSplit()) {
        YAMLPrinter.print(
            static_cast<LibScopeView::ScopeRoot *>(AReader.getScopesRoot()),
          AReader.getPrintSplitDir());
      } else {
        YAMLPrinter.print(AReader.getScopesRoot(), std::cout);
      }
    }
  }

  // Library termination.
  LibScopeView::terminate(Options.convertToCmdOptions());

  // Print performance data.
  if (Options.ShowPerformanceTime) {
    auto EndTime = LibScopeView::getCurrentTime();
    LibScopeView::printTimeTaken(StartTime, EndTime);
  }
  if (Options.ShowPerformanceMemory) {
    LibScopeView::printMemoryUsage(LibScopeView::getPeakMemoryUsage());
  }

  return 0;
}
