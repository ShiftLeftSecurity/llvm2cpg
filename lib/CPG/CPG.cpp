#include "llvm2cpg/CPG/CPG.h"
#include <llvm/IR/Module.h>

using namespace llvm2cpg;

const std::vector<CPGFile> &CPG::getFiles() const {
  return files;
}

void CPG::addBitcode(llvm::Module *bitcode) {
  CPGFile file(*bitcode);
  files.push_back(std::move(file));
}
