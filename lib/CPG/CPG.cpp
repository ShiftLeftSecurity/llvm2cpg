#include "llvm2cpg/CPG/CPG.h"
#include <assert.h>

using namespace llvm2cpg;

CPG::CPG(CPGLogger &log, bool inlineAP, bool simplify)
    : transforms(log, inlineAP, simplify), logger(log) {}

const std::vector<CPGFile> &CPG::getFiles() const {
  return files;
}

void CPG::addBitcode(llvm::Module *bitcode) {
  assert(bitcode);
  transforms.transformBitcode(*bitcode);
  CPGFile file(bitcode);
  files.push_back(std::move(file));
  logger.doNothing();
}