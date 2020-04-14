#pragma once

#include "llvm2cpg/CPG/CPGFile.h"
#include "llvm2cpg/Logger/CPGLogger.h"
#include "llvm2cpg/Transforms/Transforms.h"
#include <vector>

namespace llvm {
class Module;
}

namespace llvm2cpg {
class CPG {
public:
  CPG(CPGLogger &logger, bool inlineAP, bool simplify, bool inlineStrings);
  const std::vector<CPGFile> &getFiles() const;
  void addBitcode(llvm::Module *bitcode);

private:
  void validateBitcode(llvm::Module *bitcode);
  Transforms transforms;
  std::vector<CPGFile> files;
  CPGLogger &logger;
};

} // namespace llvm2cpg