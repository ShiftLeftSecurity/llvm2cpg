#include "llvm2cpg/CPG/CPG.h"
#include <assert.h>
#include <llvm/ADT/SetVector.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <sstream>
#include <unordered_set>

using namespace llvm2cpg;

CPG::CPG(CPGLogger &log, bool inlineAP, bool simplify, bool inlineStrings)
    : transforms(log, inlineAP, simplify, inlineStrings), logger(log) {}

const std::vector<CPGFile> &CPG::getFiles() const {
  return files;
}

void CPG::addBitcode(llvm::Module *bitcode) {
  assert(bitcode);
  validateBitcode(bitcode);
  transforms.transformBitcode(*bitcode);
  CPGFile file(bitcode);
  files.push_back(std::move(file));
}

void CPG::validateBitcode(llvm::Module *bitcode) {
  assert(bitcode);
  llvm::NamedMDNode *namedUnit = bitcode->getNamedMetadata("llvm.dbg.cu");
  if (!namedUnit) {
    logger.uiWarning("There is no debug information. Recompile your code with '-g'.");
    return;
  }
  assert(namedUnit->getNumOperands());
  auto *compileUnit = llvm::cast<llvm::DICompileUnit>(namedUnit->getOperand(0));
  llvm::StringRef flags = compileUnit->getFlags();
  if (flags.empty()) {
    logger.uiWarning(
        "There are no CLI options recorded. Recompile your code with '-grecord-command-line'.");
    return;
  }

  std::vector<std::string> recommendedFlags({ "-fno-inline-functions", "-fno-builtin" });
  std::vector<std::string> missingRecommendedFlags;
  std::unordered_set<std::string> chunks;
  std::istringstream iss(flags.str());
  std::copy(std::istream_iterator<std::string>(iss),
            std::istream_iterator<std::string>(),
            std::inserter(chunks, chunks.begin()));
  for (const std::string &flag : recommendedFlags) {
    if (chunks.count(flag) == 0) {
      missingRecommendedFlags.push_back(flag);
    }
  }

  if (!missingRecommendedFlags.empty()) {
    std::stringstream message;
    message << "It is recommended to compile your code with the following flags '";
    for (size_t i = 0; i < missingRecommendedFlags.size(); i++) {
      message << missingRecommendedFlags[i];
      if (i != missingRecommendedFlags.size() - 1) {
        message << ' ';
      }
    }
    message << "'.";
    logger.uiWarning(message.str());
  }
}
