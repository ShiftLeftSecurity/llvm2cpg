#pragma once
#include <llvm/ADT/DenseMap.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Value.h>

//#include "llvm2cpg/Logger/CPGLogger.h"

namespace llvm {
class Module;
class Function;
}

namespace llvm2cpg {
class CPGLogger;
class Transforms;

class Transforms {
public:
  void transformBitcode(llvm::Module &bitcode);
  explicit Transforms(CPGLogger &logger, bool inlineAP);

private:
  CPGLogger &logger;
  bool inlineAP; // whether to inline access paths
  void destructPHINodes(llvm::Function &function);
  void renameOpaqueObjCTypes(llvm::Module &bitcode);
};

} // namespace llvm2cpg